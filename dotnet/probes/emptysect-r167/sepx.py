#!/usr/bin/env python3
"""Print a `.doc`'s section table: each section's character range and its `bkc`.

[MS-DOC] 2.8.26 `Plcfsed` and 2.9.4 `Sepx`. `bkc` (`sprmSBkc`, 0x3009) is the break the
section *begins* with: 0 continuous, 1 new column, 2 new page, 3 even page, 4 odd page.
"""
import struct, sys
import olefile


def read(path):
    ole = olefile.OleFileIO(path)
    doc = ole.openstream("WordDocument").read()
    fib_flags = struct.unpack_from("<H", doc, 0x000A)[0]
    table = "1Table" if (fib_flags & 0x0200) else "0Table"
    tbl = ole.openstream(table).read()

    # FibBase is 32 bytes, then csw and its FibRgW97, then cslw and its FibRgLw97, then
    # cbRgFcLcb and FibRgFcLcb97. The counts are read rather than assumed because a later
    # nFib writes more of each. `fcPlcfSed`/`lcbPlcfSed` is pair 6 ([MS-DOC] 2.5.5).
    at = 32
    csw = struct.unpack_from("<H", doc, at)[0]
    at += 2 + 2 * csw
    cslw = struct.unpack_from("<H", doc, at)[0]
    at += 2 + 4 * cslw
    at += 2                                     # cbRgFcLcb
    fc, lcb = struct.unpack_from("<II", doc, at + 6 * 8)
    if lcb == 0:
        print("no Plcfsed")
        return
    plc = tbl[fc:fc + lcb]
    # n CPs of 4 bytes, then n-1 Sed of 12 bytes.
    n = (lcb - 4) // 16
    cps = list(struct.unpack_from("<%dI" % (n + 1), plc, 0))
    out = []
    for i in range(n):
        sed = plc[4 * (n + 1) + 12 * i: 4 * (n + 1) + 12 * (i + 1)]
        fcSepx = struct.unpack_from("<i", sed, 2)[0]
        bkc, cols = None, None
        if fcSepx != -1 and fcSepx + 2 <= len(doc):
            cb = struct.unpack_from("<H", doc, fcSepx)[0]
            grp = doc[fcSepx + 2: fcSepx + 2 + cb]
            at = 0
            while at + 2 <= len(grp):
                sprm = struct.unpack_from("<H", grp, at)[0]
                at += 2
                spra = (sprm >> 13) & 7
                size = {0: 1, 1: 1, 2: 2, 3: 4, 4: 2, 5: 2, 7: 3}.get(spra)
                if size is None:      # spra 6: variable, first byte is the length
                    size = grp[at] + 1 if at < len(grp) else 0
                value = grp[at: at + size]
                if sprm == 0x3009 and size >= 1:
                    bkc = value[0]
                if sprm == 0x500B and size >= 2:   # sprmSCcolumns: columns - 1
                    cols = struct.unpack_from("<h", value, 0)[0] + 1
                at += size
        out.append((cps[i], cps[i + 1], bkc, cols))
    return out


if __name__ == "__main__":
    rows = read(sys.argv[1])
    want = [int(x) for x in sys.argv[2:]]
    print(f"{len(rows)} sections")
    print(f"{'#':>4} {'cpStart':>9} {'cpEnd':>9} {'bkc':>4} {'cols':>5}")
    names = {0: "continuous", 1: "newColumn", 2: "nextPage", 3: "evenPage", 4: "oddPage"}
    for i, (a, b, bkc, cols) in enumerate(rows):
        if want and i not in want:
            continue
        print(f"{i:>4} {a:>9} {b:>9} {str(bkc):>4} {str(cols):>5}  {names.get(bkc, '?')}")
