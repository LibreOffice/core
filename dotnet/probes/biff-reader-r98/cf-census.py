#!/usr/bin/env python3
"""Every BIFF `CF` record in the corpus's 64 `.xls`, decoded the way
`XclImpCondFormat::ReadCF` decodes it (`sc/source/filter/excel/xicontent.cxx`:526-713).

Prints one row per CF record: which document and sheet substream it is in, the rule's type
and operator, the flag word's format blocks, and the raw bytes of its two RPN formulas.  The
point is to see what the corpus actually states before a reader is written for it.
"""
import glob, os, sys, struct
import olefile

CORPUS = "/home/user/sample-files"

CONDFMT, CF, CF12, BOF, EOF, CONTINUE = 0x01B0, 0x01B1, 0x087A, 0x0809, 0x000A, 0x003C

BLOCK = [(0x02000000, "numfmt"), (0x04000000, "font"), (0x08000000, "align"),
         (0x10000000, "border"), (0x20000000, "area"), (0x40000000, "prot")]


def workbook_stream(path):
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
    n = len(data)
    while at + 4 <= n:
        rid, size = struct.unpack_from("<HH", data, at)
        at += 4
        if at + size > n:
            return
        yield rid, data[at:at + size]
        at += size


def main():
    rows = []
    files = sorted(p for p in glob.glob(os.path.join(CORPUS, "**", "*.[xX][lL][sS]"), recursive=True)
                   if os.path.splitext(p)[1].lower() == ".xls")
    for path in files:
        data = workbook_stream(path)
        if data is None:
            continue
        sheet = -1
        cur = None            # (count, index, nranges)
        for rid, body in records(data):
            if rid == BOF:
                if len(body) >= 4:
                    kind = struct.unpack_from("<H", body, 2)[0]
                    if kind == 0x0010:
                        sheet += 1
                    elif kind != 0x0005:
                        sheet = sheet  # chart/macro substream: keep numbering
                cur = None
            elif rid == CONDFMT:
                cnt = struct.unpack_from("<H", body)[0]
                nr = struct.unpack_from("<H", body, 12)[0] if len(body) >= 14 else 0
                cur = [cnt, 0, nr]
            elif rid == CF12:
                rows.append((os.path.basename(path), sheet, "CF12", "", "", "", 0, 0, ""))
            elif rid == CF and len(body) >= 12:
                typ, op = body[0], body[1]
                f1, f2, flags = struct.unpack_from("<HHI", body, 2)
                blocks = ",".join(n for m, n in BLOCK if flags & m)
                tail = body[12:]
                # skip the format blocks to find where the formulas start
                at = 0
                if flags & 0x02000000:
                    # CFFormat: a length byte then the string, or a 2-byte id.
                    if flags & 0x1:
                        ln = tail[at] if at < len(tail) else 0
                        at += 1 + ln * 2 + 1
                    else:
                        at += 2
                if flags & 0x04000000: at += 118
                if flags & 0x08000000: at += 8
                if flags & 0x10000000: at += 8
                if flags & 0x20000000: at += 4
                if flags & 0x40000000: at += 2
                fm1 = tail[at:at + f1]
                fm2 = tail[at + f1:at + f1 + f2]
                rows.append((os.path.basename(path), sheet, "CF", typ, op, blocks,
                             f1, f2, fm1.hex() + "|" + fm2.hex()))
                if cur: cur[1] += 1

    print("document\tsheet\trec\ttype\top\tblocks\tfmla1\tfmla2\tbytes")
    for r in rows:
        print("\t".join(str(x) for x in r))
    print(f"# {len(rows)} CF records in {len({r[0] for r in rows})} documents "
          f"of {len(files)} .xls", file=sys.stderr)


if __name__ == "__main__":
    main()
