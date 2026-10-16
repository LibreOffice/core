#!/usr/bin/env python3
"""Does the BIFF `CF` reader reach `EHEST-Pre-departure-checklist`, or does it miss it?

That workbook states **120 `CONDFMT` and 144 `CF`** — more than the other four witnesses put
together — and reading them moves not one byte of its rendering.  Two explanations fit: the
reader never sees the records, or it sees them and no cell satisfies a rule.  The second is
what a blank checklist would look like (126 of the 144 rules are `cellIs equal 2` over cells
that hold 0 or nothing), but "no cell matches" is exactly the sentence a reader that read
nothing would also produce.

The discriminator is a one-attribute variant, and it is on the *rule* rather than on a cell,
so that neither side's recalculation enters it.  The first `CONDFMT` of the second worksheet
substream covers `E10:E12` and its single `CF` ends with the RPN token `1e 0200` — `tInt 2`.
Patched to `1e 0000` the rule becomes `cellIs equal 0`, which those three cells do satisfy.

Rendered at 26.2.4.2 and at this tree, original against patched:

    REF   page 7   |ink|% 0.05   one region
    OURS  page 7   |ink|% 0.05   one region

Both sides paint the same fill in the same place, so the reader reaches the records and the
nil result on the unpatched workbook is a fact about the corpus.
"""
import glob
import shutil
import struct
import sys

import olefile

SRC = glob.glob("/home/user/sample-files/**/EHEST-Pre-departure-checklist-Rev.-1-06-12-2016.xls",
                recursive=True)


def first_cf(data):
    """The first CF record of the second worksheet substream: (offset, size, payload)."""
    at, sheet, header = 0, -1, None
    while at + 4 <= len(data):
        rid, size = struct.unpack_from("<HH", data, at)
        body = at + 4
        if rid == 0x0809 and size >= 4 and struct.unpack_from("<H", data, body + 2)[0] == 0x0010:
            sheet += 1
        if sheet == 1 and rid == 0x01B0 and header is None:
            header = data[body:body + size]
        if sheet == 1 and rid == 0x01B1 and header is not None:
            return header, data[body:body + size]
        at = body + size
    return None, None


def main():
    if not SRC:
        print("witness not found", file=sys.stderr)
        return 1

    out = sys.argv[1] if len(sys.argv) > 1 else "ehest-patched.xls"
    header, payload = first_cf(olefile.OleFileIO(SRC[0]).openstream("Workbook").read())
    if payload is None or payload[-3:] != bytes.fromhex("1e0200"):
        print("the record this patches has moved; re-locate it", file=sys.stderr)
        return 1

    print("CONDFMT", header.hex())
    print("CF     ", payload.hex())

    shutil.copy(SRC[0], out)
    raw = bytearray(open(out, "rb").read())
    if raw.count(payload) != 1:
        print("the CF payload is not unique in the file", file=sys.stderr)
        return 1

    at = raw.find(payload)
    raw[at + len(payload) - 3:at + len(payload)] = bytes.fromhex("1e0000")
    open(out, "wb").write(raw)
    print(f"wrote {out}: cellIs equal 2 -> cellIs equal 0 over E10:E12")
    return 0


if __name__ == "__main__":
    sys.exit(main())
