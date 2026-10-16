#!/usr/bin/env python3
"""Which token *class* does a built-in `NAME` formula use?

`XlsNameRanges.Read` folds a reference token's class with `opcode & 0x3F`, which is not the
fold: the base identifier is the low five bits with bit 5 put back, so a value-class `tArea3d`
(0x5B) reads as 0x1B, falls through the switch and stops the walk with whatever ranges it has.
Whether that matters is a question about the corpus, so it is asked of the corpus.

Prints every opcode that appears in a built-in `NAME` record's token array over the 64 `.xls`,
and how many of them carry a class above the reference one.
"""
import collections
import glob
import os
import struct
import sys

import olefile

CORPUS = "/home/user/sample-files"

NAME, BUILTIN_FLAG = 0x0018, 0x20

# How many bytes follow each token this walker knows, keyed by the folded base identifier.
SIZES = {0x3B: 10, 0x3A: 6, 0x25: 8, 0x24: 4, 0x26: 6, 0x28: 6, 0x29: 2, 0x19: 3,
         0x1E: 2, 0x1F: 8, 0x10: 0, 0x0F: 0, 0x11: 0, 0x15: 0}


def workbook(path):
    try:
        ole = olefile.OleFileIO(path)
    except Exception:
        return None
    for name in ("Workbook", "Book"):
        if ole.exists(name):
            return ole.openstream(name).read()
    return None


def records(data):
    at = 0
    while at + 4 <= len(data):
        rid, size = struct.unpack_from("<HH", data, at)
        body = at + 4
        if body + size > len(data):
            return
        yield rid, data[body:body + size]
        at = body + size


def main():
    seen = collections.Counter()
    high = collections.Counter()
    formulas = 0

    for path in sorted(glob.glob(os.path.join(CORPUS, "**", "*.[xX][lL][sS]"), recursive=True)):
        data = workbook(path)
        if data is None:
            continue

        for rid, body in records(data):
            if rid != NAME or len(body) < 15:
                continue
            if not struct.unpack_from("<H", body)[0] & BUILTIN_FLAG:
                continue

            characters, size = body[3], struct.unpack_from("<H", body, 4)[0]
            wide = body[14] & 1
            tokens = body[15 + characters * (2 if wide else 1):][:size]
            if not tokens:
                continue

            formulas += 1
            at = 0
            while at < len(tokens):
                opcode = tokens[at]
                at += 1
                seen[opcode] += 1
                if opcode >= 0x40:
                    high[opcode] += 1
                step = SIZES.get((opcode & 0x1F) | 0x20 if opcode >= 0x20 else opcode)
                if step is None:
                    break
                at += step

    print(f"{formulas} built-in NAME formulas")
    for opcode, count in seen.most_common():
        print(f"  0x{opcode:02x}\t{count}")
    print(f"value- or array-class reference tokens (opcode >= 0x40): {sum(high.values())}",
          file=sys.stderr)


if __name__ == "__main__":
    main()
