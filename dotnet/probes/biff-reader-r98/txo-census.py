#!/usr/bin/env python3
"""Every `TXO` in the corpus's `.xls`, and how many carry more than one formatting run.

A `TXO` states its character count and the byte length of its formatting-run array; the
characters arrive in the `CONTINUE` after it and the runs in the one after that
(`XclImpDrawing::ReadTxo`, `sc/source/filter/excel/xiescher.cxx`:4242-4271).  Each run is
eight bytes -- a character index and a `FONT` index -- and the last names the index one past
the string, so a box whose text is all one face states exactly two.

Prints one row per text box and a summary to stderr.
"""
import glob
import os
import struct
import sys

import olefile

CORPUS = "/home/user/sample-files"

TXO, CONTINUE, BOF = 0x01B6, 0x003C, 0x0809


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
    print("document\tsheet\tchars\truns\tfonts")
    boxes = runs_total = mixed = 0
    documents, mixed_documents = set(), set()

    for path in sorted(glob.glob(os.path.join(CORPUS, "**", "*.[xX][lL][sS]"), recursive=True)):
        data = workbook(path)
        if data is None:
            continue

        sheet, pending = -1, None
        for rid, body in records(data):
            if rid == BOF and len(body) >= 4 and struct.unpack_from("<H", body, 2)[0] == 0x0010:
                sheet += 1

            if rid == TXO and len(body) >= 14:
                chars, size = struct.unpack_from("<HH", body, 10)
                pending = [chars, size, 0]
                continue

            if rid == CONTINUE and pending is not None:
                if pending[2] == 0 and pending[0] > 0:
                    pending[2] = 1          # the characters
                    continue

                count = pending[1] // 8
                fonts = {struct.unpack_from("<H", body, at * 8 + 2)[0]
                         for at in range(count) if at * 8 + 8 <= len(body)}
                if pending[0] > 0:
                    boxes += 1
                    runs_total += count
                    documents.add(os.path.basename(path))
                    # The terminator is a run of its own, so "mixed" is three or more.
                    if count > 2:
                        mixed += 1
                        mixed_documents.add(os.path.basename(path))
                    print(f"{os.path.basename(path)}\t{sheet}\t{pending[0]}\t{count}\t"
                          f"{','.join(str(f) for f in sorted(fonts))}")
                pending = None
                continue

            if rid != CONTINUE:
                pending = None

    print(f"# {boxes} text boxes with text in {len(documents)} .xls, {runs_total} runs; "
          f"{mixed} boxes in {len(mixed_documents)} state more than the opening run "
          f"and its terminator", file=sys.stderr)


if __name__ == "__main__":
    main()
