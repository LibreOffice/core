#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
from difflib import unified_diff
from pathlib import Path
from subprocess import PIPE, Popen

EXCLUDELIST = Path("solenv/clang-format/excludelist")
THRESHOLD = os.getenv("CLANG_THRESHOLD", 5)
CLANG_BINARY = Path(os.getenv("CLANG_FORMAT", "/opt/lo/bin/clang-format"))


def calculate_diff_size(diff):
    additions, removals = 0, 0
    # ignore first 2 item in the sequence
    # which are +++ and ---
    for line in diff[2:]:
        if line.startswith("+"):
            additions += 1
        elif line.startswith("-"):
            removals += 1
    return max((additions, removals))


def format_stream(path, *extra_args):
    process = Popen(
        [CLANG_BINARY, *extra_args], stdout=PIPE, stderr=PIPE, stdin=PIPE,
    )
    stdout, stderr = process.communicate(input=path.read_bytes())
    if stderr:
        print("<FAIL>", str(path))
        print(stderr.decode())
        print("<FAIL>")
        exit(1)
    stdout = stdout.decode()
    return stdout


def main(*args):
    if not CLANG_BINARY.exists():
        print("Couldn't find clang-format on {!s}".format(CLANG_BINARY))
        exit(1)

    for path in EXCLUDELIST.read_text().splitlines():
        path = Path(path)
        if not path.exists():
            continue

        original = path.read_text()
        new = format_stream(path, *args)
        originalsize = len(original.splitlines())
        diff = unified_diff(original.splitlines(), new.splitlines(), n=0)
        diffsize = calculate_diff_size(tuple(diff))
        if diffsize <= (originalsize * 5) // 100:
            print(path, "(size: {}/{})".format(diffsize, originalsize))


if __name__ == "__main__":
    import sys

    main(*sys.argv[1:])
