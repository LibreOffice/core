#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
from difflib import context_diff
from pathlib import Path
from subprocess import PIPE, Popen

BLACKLIST = Path("solenv/clang-format/blacklist")
THRESHOLD = os.getenv("CLANG_THRESHOLD", 5)
CLANG_BINARY = Path(os.getenv("CLANG_FORMAT", "/opt/lo/bin/clang-format"))


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
    print(CLANG_BINARY)
    if not CLANG_BINARY.exists():
        print(
            "Couldn't find clang-format on /opt/lo/bin (set CLANG_FORMAT for overriding default path."
        )
        exit(1)

    for path in BLACKLIST.read_text().splitlines():
        path = Path(path)
        if not path.exists():
            continue

        original = path.read_text()
        new = format_stream(path, *args)
        originalsize = len(original.splitlines())
        diff = context_diff(original.splitlines(), new.splitlines(), n=0)
        diffsize = len(tuple(filter(lambda line: line.startswith("!"), diff)))
        if diffsize != 0 and (diffsize // 2) < (originalsize * 5) // 100:
            print(path)


if __name__ == "__main__":
    import sys

    main(*sys.argv[1:])
