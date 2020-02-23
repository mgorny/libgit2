/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */

#include <stdio.h>
#include <git2.h>
#include "cli.h"
#include "cmd.h"

static int show_help = 0;
static int show_version = 0;
static char *command = NULL;
static char **args = NULL;

const cli_opt_spec cli_common_opts[] = {
	{ CLI_OPT_TYPE_SWITCH,    "help",      0, &show_help,    1,
	  CLI_OPT_USAGE_DEFAULT,   NULL,      "display help information" },
	{ CLI_OPT_TYPE_SWITCH,    "version",   0, &show_version, 1,
	  CLI_OPT_USAGE_DEFAULT,   NULL,      "display the version" },
	{ CLI_OPT_TYPE_ARG,       "command",   0, &command,      0,
	  CLI_OPT_USAGE_REQUIRED, "command", "the command to run" },
	{ CLI_OPT_TYPE_ARGS,      "args",      0, &args,         0,
	  CLI_OPT_USAGE_DEFAULT,  "args",    "arguments for the command" },
	{ 0 }
};

const cli_cmd_spec cli_cmds[] = {
	{ "help", cmd_help, "Display help information" },
	{ NULL }
};

int main(int argc, char **argv)
{
	const cli_cmd_spec *cmd;
	cli_opt_parser optparser;
	cli_opt opt;
	int args_len = 0;
	int ret = 0;

	if (git_libgit2_init() < 0) {
		cli_error("failed to initialize libgit2");
		exit(CLI_EXIT_GIT);
	}

	cli_opt_parser_init(&optparser, cli_common_opts, argv + 1, argc - 1);

	/* Parse the top-level (common) options and command information */
	while (cli_opt_parser_next(&opt, &optparser)) {
		if (!opt.spec) {
			cli_opt_status_fprint(stderr, PROGRAM_NAME, &opt);
			cli_opt_usage_fprint(stderr, PROGRAM_NAME, NULL, cli_common_opts);
			ret = CLI_EXIT_USAGE;
			goto done;
		}

		/*
		 * When we see a command, stop parsing and capture the
		 * remaining arguments as args for the command itself.
		 */
		if (command) {
			args = &argv[optparser.idx];
			args_len = (int)(argc - optparser.idx);
			break;
		}
	}

	if (show_version) {
		printf("%s version %s\n", PROGRAM_NAME, LIBGIT2_VERSION);
		goto done;
	}

	/* If there was no command, we want to invoke "help" */
	if (!command || show_help) {
		cli_opt_usage_fprint(stdout, PROGRAM_NAME, NULL, cli_common_opts);
		goto done;
	}

	if ((cmd = cli_cmd_spec_byname(command)) == NULL) {
		ret = cli_error("'%s' is not a %s command. See '%s help'.",
		                command, PROGRAM_NAME, PROGRAM_NAME);
		goto done;
	}

	ret = cmd->fn(args_len, args);

done:
	git_libgit2_shutdown();
	return ret;
}
