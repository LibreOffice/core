#!/usr/bin/env python3
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Reads options from autogen.input, one option at a time, so you don't have to run autogen.sh or
# configure with parameters.

import os
import re
import subprocess
import sys

def main():
    cmdline = [sys.argv[0].replace(".py", ".sh")]
    with open("autogen.input") as stream:
        for line in stream.readlines():
            line = line.strip()
            if line.startswith("#"):
                continue
            line = os.path.expandvars(line)
            # If an environment variable doesn't expand to a result, expand it to an empty string,
            # like the shell does.
            line = re.sub(r"\$[A-Za-z_][A-Za-z0-9_]*", "", line)
            cmdline.append(line)
    cmdline.append("--enable-option-checking=fatal")
    print("Running '{}'".format("' '".join(cmdline)))
    subprocess.run(cmdline, check=True)

    # On success, write here too in case this is invoked manually.
    with open("autogen-input.mk", "w") as stream:
        stream.write("# Marks the autogen.input that the build tree was configured from.\n")

if __name__ == "__main__":
    main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
