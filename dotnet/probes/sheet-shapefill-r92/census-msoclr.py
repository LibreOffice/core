#!/usr/bin/env python3
"""Census the MSO_CLR forms every `.xls` shape states for fillColor and lineColor.

An Escher colour's top byte decides how the other three are read (MSO_CLR_ToColor,
filter/source/msfilter/msdffimp.cxx:3420). This counts the forms actually present, so a
round implements the branches the corpus has rather than the whole specification.

Usage: census-msoclr.py <corpus-root>
"""
import collections
import pathlib
import struct
import sys

import olefile

MSODRAWING = 0x00EC
MSODRAWINGGROUP = 0x00EB
CONTINUE = 0x003C
BOF = 0x0809

FILL = 385
FILLBACK = 387
LINE = 448
SHADOW = 513
FILLBOOLS = 443
LINEBOOLS = 511


def records(data):
    at = 0
    while at + 4 <= len(data):
        rid, size = struct.unpack_from("<HH", data, at)
        at += 4
        yield rid, data[at:at + size]
        at += size


def walk(buf, out):
    at = 0
    while at + 8 <= len(buf):
        ver_inst, rtype, size = struct.unpack_from("<HHI", buf, at)
        if size < 0 or at + 8 + size > len(buf) + 8:
            return
        body = buf[at + 8:at + 8 + size]
        if rtype == 0xF004:
            recs = []
            flat(body, recs)
            out.append(recs)
            walk(body, out)
        elif (ver_inst & 0x0F) == 0x0F:
            walk(body, out)
        at += 8 + size


def flat(buf, out):
    at = 0
    while at + 8 <= len(buf):
        ver_inst, rtype, size = struct.unpack_from("<HHI", buf, at)
        body = buf[at + 8:at + 8 + size]
        if (ver_inst & 0x0F) == 0x0F:
            flat(body, out)
        else:
            out.append((rtype, ver_inst >> 4, body))
        at += 8 + size


def properties(body, inst):
    props = {}
    at = 0
    for _ in range(inst):
        if at + 6 > len(body):
            break
        pid_raw, value = struct.unpack_from("<HI", body, at)
        at += 6
        props[pid_raw & 0x3FFF] = value
    return props


def form(value):
    """The branch MSO_CLR_ToColor would take."""
    if (value & 0xFE000000) == 0xFE000000:
        return "0xfe text"
    upper = value >> 24
    if upper & 0x19:
        if (upper & 0x08) or not (upper & 0x10):
            return f"scheme idx={value & 0xFFFF}" if upper & 0x08 else f"scheme upper={upper}"
        return "syscolor"
    if (upper & 4) and (value & 0xFFFFF8) == 0:
        return f"scheme upper={upper}"
    return "literal"


def main():
    root = pathlib.Path(sys.argv[1])
    counts = collections.Counter()
    docs = collections.defaultdict(set)
    files = [p for p in root.rglob("*") if p.suffix.lower() == ".xls" and p.is_file()]
    for path in sorted(files):
        try:
            ole = olefile.OleFileIO(str(path))
            name = "Workbook" if ole.exists("Workbook") else "Book"
            data = ole.openstream(name).read()
        except Exception as exc:  # noqa: BLE001
            print(f"!! {path.name}: {exc}", file=sys.stderr)
            continue

        dff = bytearray()
        last = None
        for rid, body in records(data):
            if rid in (MSODRAWING, MSODRAWINGGROUP):
                dff += body
                last = rid
            elif rid == CONTINUE and last in (MSODRAWING, MSODRAWINGGROUP):
                dff += body
            else:
                last = None
        if not dff:
            continue
        shapes = []
        try:
            walk(bytes(dff), shapes)
        except Exception as exc:  # noqa: BLE001
            print(f"!! {path.name}: walk {exc}", file=sys.stderr)
            continue
        for recs in shapes:
            props = {}
            for rt, inst, body in recs:
                if rt == 0xF00B:
                    props.update(properties(body, inst))
            for pid, label in ((FILL, "fill"), (LINE, "line")):
                if pid in props:
                    key = f"{label} {form(props[pid])}"
                    counts[key] += 1
                    docs[key].add(path.name)

    for key, n in counts.most_common():
        print(f"{n:6d}  {len(docs[key]):3d} docs  {key:24s}  {', '.join(sorted(docs[key]))}")


if __name__ == "__main__":
    main()
