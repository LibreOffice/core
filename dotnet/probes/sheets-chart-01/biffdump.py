#!/usr/bin/env python3
"""Dump BIFF records of an .xls workbook stream, with chart substream nesting."""
import sys, struct, olefile

NAMES = {
    0x1002: "CHCHART", 0x1003: "CHSERIES", 0x1006: "CHDATAFORMAT", 0x1007: "CHLINEFORMAT",
    0x100A: "CHAREAFORMAT", 0x100B: "CHPIEFORMAT", 0x100C: "CHMARKERFORMAT",
    0x100D: "CHSTRING", 0x1014: "CHTYPEGROUP", 0x1015: "CHLEGEND", 0x1016: "CHSERIESLINK",
    0x1017: "CHBAR", 0x1018: "CHLINE", 0x1019: "CHPIE", 0x101A: "CHAREA",
    0x101B: "CHSCATTER", 0x101C: "CHCHARTLINE", 0x101D: "CHAXIS", 0x101E: "CHTICK",
    0x101F: "CHVALUERANGE", 0x1020: "CHLABELRANGE", 0x1021: "CHAXISLINE",
    0x1022: "CHCHARTFORMAT", 0x1023: "CHSERIESLIST", 0x1024: "CHDEFAULTTEXT",
    0x1025: "CHTEXT", 0x1026: "CHFONT", 0x1027: "CHOBJECTLINK", 0x1032: "CHFRAME",
    0x1033: "CHBEGIN", 0x1034: "CHEND", 0x1035: "CHPLOTFRAME", 0x103A: "CH3D",
    0x103C: "CHPICFORMAT", 0x103D: "CHDROPBAR", 0x103E: "CHRADARLINE",
    0x103F: "CHSURFACE", 0x1040: "CHRADARAREA", 0x1041: "CHAXESSET",
    0x1043: "CHATTACHEDLABEL", 0x1044: "CHCHARTFORMAT?", 0x1045: "CHSERIESTEXT",
    0x1044: "CHPROPERTIES", 0x1045: "CHSERTRENDLINE", 0x1048: "CHPIVOTREF",
    0x104A: "CHSERERRORBAR", 0x104B: "CHSERPARENT", 0x104E: "CHFORMAT(ifmt)",
    0x104F: "CH3DDATAFORMAT", 0x1050: "CHFRLEGENDENTRY", 0x1051: "CHSOURCELINK",
    0x105B: "CHSERINDEX?", 0x105D: "CHSERIESFORMAT", 0x1060: "CHFBI",
    0x1061: "CHFBI2", 0x1062: "CHDATERANGE", 0x1063: "CHFRAMEPOS?",
    0x1064: "CHESCHERFORMAT?", 0x1066: "CHESCHERFORMAT",
    0x0809: "BOF", 0x000A: "EOF", 0x041E: "FORMAT", 0x00E0: "XF", 0x0031: "FONT",
    0x0085: "BOUNDSHEET", 0x005D: "OBJ", 0x00EC: "MSODRAWING",
}

def recs(data):
    pos = 0
    while pos + 4 <= len(data):
        rid, ln = struct.unpack_from("<HH", data, pos)
        pos += 4
        yield rid, data[pos:pos+ln]
        pos += ln

def rdstr8(b, off):
    # BIFF8 short string: cch (1 byte), grbit, chars
    cch = b[off]; off += 1
    grbit = b[off]; off += 1
    if grbit & 1:
        s = b[off:off+cch*2].decode("utf-16-le", "replace")
    else:
        s = b[off:off+cch].decode("cp1252", "replace")
    return s

def main(path):
    ole = olefile.OleFileIO(path)
    name = "Workbook" if ole.exists("Workbook") else "Book"
    data = ole.openstream(name).read()
    depth = 0
    formats = {}
    charts = 0
    incharts = False
    for rid, body in recs(data):
        if rid == 0x041E and len(body) >= 3:
            ifmt = struct.unpack_from("<H", body, 0)[0]
            try:
                formats[ifmt] = rdstr8(body, 2) if body[3] in (0,1) else None
            except Exception:
                pass
            # BIFF8 FORMAT: ifmt(2) + XLUnicodeString(cch 2 bytes, grbit 1)
            cch = struct.unpack_from("<H", body, 2)[0]
            grbit = body[4]
            s = body[5:5+cch*2].decode("utf-16-le","replace") if grbit & 1 else body[5:5+cch].decode("cp1252","replace")
            formats[ifmt] = s
        if rid == 0x0809 and len(body) >= 4:
            dt = struct.unpack_from("<H", body, 2)[0]
            incharts = (dt == 0x0020)
            if incharts:
                charts += 1
                print(f"\n===== CHART SUBSTREAM #{charts} =====")
                depth = 0
        if not incharts:
            continue
        if rid == 0x1034:
            depth -= 1
        nm = NAMES.get(rid, f"0x{rid:04X}")
        extra = ""
        if rid == 0x104E:
            extra = f"  ifmt={struct.unpack_from('<H', body, 0)[0]}"
        elif rid == 0x101D:
            extra = f"  axisType={struct.unpack_from('<H', body, 0)[0]}"
        elif rid == 0x1015 and len(body) >= 20:
            x,y,w,h = struct.unpack_from("<iiii", body, 0)
            docked = body[16]; spacing = body[17]
            flags = struct.unpack_from("<H", body, 18)[0]
            extra = f"  pos=({x},{y},{w},{h}) docked={docked} spacing={spacing} flags=0x{flags:04X}"
        elif rid == 0x1026:
            extra = f"  ifnt={struct.unpack_from('<H', body, 0)[0]}"
        elif rid == 0x100D:
            try: extra = "  " + repr(rdstr8(body, 2))
            except Exception: pass
        elif rid == 0x1027:
            extra = f"  link={struct.unpack_from('<H', body, 0)[0]}"
        elif rid == 0x1051 and len(body) >= 6:
            dest, lt, flags, ifmt = body[0], body[1], struct.unpack_from("<H", body,2)[0], struct.unpack_from("<H", body,4)[0]
            extra = f"  dest={dest} linkType={lt} flags=0x{flags:04X} ifmt={ifmt}"
        elif rid == 0x1062 and len(body) >= 8:
            extra = f"  daterange={struct.unpack_from('<HHHHHHHHH', body, 0) if len(body)>=18 else body.hex()} raw={body.hex()}"
        elif rid == 0x1020 and len(body) >= 6:
            extra = f"  crossing={struct.unpack_from('<HHH', body,0)}"
        elif rid == 0x101E:
            extra = f"  tick body={body.hex()}"
        elif rid == 0x1024:
            extra = f"  id={struct.unpack_from('<H', body, 0)[0]}"
        elif rid == 0x101F and len(body) >= 42:
            vals = struct.unpack_from("<5d", body, 0)
            fl = struct.unpack_from("<H", body, 40)[0]
            extra = f"  min={vals[0]} max={vals[1]} major={vals[2]} minor={vals[3]} cross={vals[4]} flags=0x{fl:04X}"
        print("  " * max(depth,0) + f"{nm}({rid:#06x}) len={len(body)}{extra}")
        if rid == 0x1033:
            depth += 1
    print("\n=== FORMAT records ===")
    for k in sorted(formats):
        print(f"  ifmt={k}: {formats[k]!r}")

main(sys.argv[1])
