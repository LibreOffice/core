#!/usr/bin/env python3
"""Walk a BIFF workbook's Escher stream sheet by sheet, the way the reader assembles it.

    escher-walk.py <workbook.xls> [sheet index] [all]

Concatenates the `MSODRAWING` records of each *worksheet* substream -- and, with `all`, the
records of the embedded chart substreams nested inside it as well -- then walks the result as a
record tree and reports, per sheet, how many `SpContainer`s it holds, how many of those carry
client data, and how many `OBJ` and `TXO` records the substream states. A client-data count
below the `OBJ` count is a drawing the reader will run short on; a container whose declared
length runs past the buffer is a drawing the reader has not got all of.

It deliberately does NOT join a `CONTINUE` that follows an `OBJ` or a `TXO`, which is what the
reader did before round 103 -- so the numbers it prints are the base reader's own. On
`EHEST-Pre-departure-checklist.xls` that is 11 `OBJ` against 10 client-data shapes, and a
declared `DgContainer` length 54 bytes past the buffer, on each of the five sheets that hold a
group.

Given a sheet index it prints that sheet's whole record tree, one line per record, with
`OVERRUN` against any whose declared end is past what the buffer holds.
"""
import sys
import struct
import olefile

path = sys.argv[1]
want = int(sys.argv[2]) if len(sys.argv) > 2 else None
mode = sys.argv[3] if len(sys.argv) > 3 else "sheet"        # sheet | all

ole = olefile.OleFileIO(path)
name = [e for e in ole.listdir() if e[-1].lower() in ("workbook", "book")][0]
data = ole.openstream(name).read()

pos, recs = 0, []
while pos + 4 <= len(data):
    rid, rlen = struct.unpack_from("<HH", data, pos)
    recs.append((pos, rid, rlen))
    pos += 4 + rlen

BOF, EOFR, MSO, OBJ, TXO, CONT = 0x0809, 0x000A, 0x00EC, 0x005D, 0x01B6, 0x003C

sheets, cur, depth, prev = [], None, 0, None
for (pos, rid, rlen) in recs:
    if rid == BOF:
        # The substream type: 0x0010 is a worksheet, 0x0020 an embedded chart nested in one.
        dt = struct.unpack_from("<H", data, pos + 6)[0]
        depth += 1
        if dt == 0x0010:
            cur = {"idx": len(sheets), "dff": bytearray(), "dffsheet": bytearray(),
                   "obj": 0, "txo": 0, "depth": depth}
            sheets.append(cur)
        prev = rid
        continue
    if rid == EOFR:
        depth -= 1
        prev = rid
        continue
    if cur is None:
        continue

    if rid == MSO:
        cur["dff"] += data[pos + 4:pos + 4 + rlen]
        if depth == cur["depth"]:
            cur["dffsheet"] += data[pos + 4:pos + 4 + rlen]
        prev = rid
    elif rid == CONT and prev == MSO:
        cur["dff"] += data[pos + 4:pos + 4 + rlen]
        if depth == cur["depth"]:
            cur["dffsheet"] += data[pos + 4:pos + 4 + rlen]
    elif rid == OBJ:
        if depth == cur["depth"]:
            cur["obj"] += 1
        prev = rid
    elif rid == TXO:
        if depth == cur["depth"]:
            cur["txo"] += 1
        prev = rid
    elif rid != CONT:
        prev = rid


def walk(buf, start, end, level, out):
    p = start
    while p + 8 <= end:
        vi, typ, ln = struct.unpack_from("<HHI", buf, p)
        body = p + 8
        declared = body + ln
        out.append((level, typ, ln, p, declared <= end))
        if (vi & 0xF) == 0xF:
            walk(buf, body, min(declared, end), level + 1, out)
        p = declared
        if ln == 0 and typ == 0:
            break
    return out


for sheet in sheets:
    if want is not None and sheet["idx"] != want:
        continue
    buf = bytes(sheet["dff"] if mode == "all" else sheet["dffsheet"])
    if not buf:
        print(f"sheet {sheet['idx']}: no drawing, obj={sheet['obj']}")
        continue

    out = walk(buf, 0, len(buf), 0, [])
    shapes = sum(1 for level, t, ln, p, ok in out if t == 0xF004)
    client = sum(1 for level, t, ln, p, ok in out if t == 0xF011)
    overrun = sum(1 for x in out if not x[4])
    print(f"sheet {sheet['idx']}: bytes={len(buf)} obj={sheet['obj']} txo={sheet['txo']} "
          f"spContainer={shapes} clientData={client} overrun={overrun}")

    if want is not None:
        for level, t, ln, p, ok in out:
            print("  " * level, hex(t), ln, "@", p, "" if ok else "OVERRUN")
