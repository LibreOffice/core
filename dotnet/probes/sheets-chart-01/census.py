#!/usr/bin/env python3
"""Census: which BIFF charts in the corpus resolve to a non-General axis number format.

Reads every OLE2 workbook found under a corpus root, walks each chart substream, and
answers per chart:
  * does the value axis carry a CHFORMAT (0x104E)?  (explicit format, precedence 1)
  * what is the number format of the FIRST non-empty numeric cell of the first series'
    value range / category range?  (precedence 2 — LibreOffice's link-to-source)
"""
import sys, os, struct, glob
import olefile

BUILTIN = {
    0: 'General', 1: '0', 2: '0.00', 3: '#,##0', 4: '#,##0.00', 5: None, 6: None, 7: None,
    8: None, 9: '0%', 10: '0.00%', 11: '0.00E+00', 12: '# ?/?', 13: '# ??/??',
    14: 'MM/DD/YY', 15: 'D-MMM-YY', 16: 'D-MMM', 17: 'MMM-YY', 18: 'h:mm AM/PM',
    19: 'h:mm:ss AM/PM', 20: 'h:mm', 21: 'h:mm:ss', 22: 'M/D/YY h:mm',
    37: '#,##0;(#,##0)', 38: '#,##0;[Red](#,##0)', 39: '#,##0.00;(#,##0.00)',
    40: '#,##0.00;[Red](#,##0.00)', 45: 'mm:ss', 46: '[h]:mm:ss', 47: 'mm:ss.0',
    48: '##0.0E+0', 49: '@',
}

CELL_RECS = {0x0203, 0x027E, 0x0006, 0x00FD, 0x0205, 0x0207, 0x00BD, 0x0201, 0x00BE, 0x0004, 0x0002, 0x0003}


def recs(d, pos=0):
    while pos + 4 <= len(d):
        rid, ln = struct.unpack_from("<HH", d, pos)
        pos += 4
        yield pos - 4, rid, d[pos:pos + ln]
        pos += ln


def rk(raw):
    cents = raw & 2
    if raw & 1:
        v = float(raw >> 2)
    else:
        v = struct.unpack("<d", struct.pack("<Q", (raw & 0xFFFFFFFC) << 32))[0]
    return v / 100.0 if cents else v


def read(path):
    try:
        ole = olefile.OleFileIO(path)
    except Exception:
        return None
    name = "Workbook" if ole.exists("Workbook") else ("Book" if ole.exists("Book") else None)
    if name is None:
        return None
    try:
        return ole.openstream(name).read()
    except Exception:
        return None


def analyse(path):
    d = read(path)
    if d is None:
        return []

    formats, xfs, sheets = {}, [], []
    sub = -1
    # globals
    for off, rid, b in recs(d):
        if rid == 0x0809:
            sub += 1
            if sub > 0:
                break
        if rid == 0x041E and len(b) >= 4:
            i = struct.unpack_from("<H", b, 0)[0]
            try:
                cch = struct.unpack_from("<H", b, 2)[0]
                grbit = b[4]
                s = b[5:5 + cch * 2].decode("utf-16-le", "replace") if grbit & 1 else b[5:5 + cch].decode("cp1252", "replace")
            except Exception:
                s = None
            formats[i] = s
        elif rid == 0x00E0 and len(b) >= 4:
            xfs.append(struct.unpack_from("<HH", b, 0)[1])
        elif rid == 0x0085 and len(b) >= 8:
            pos = struct.unpack_from("<I", b, 0)[0]
            try:
                cch = b[6]; grbit = b[7]
                nm = b[8:8 + cch * 2].decode("utf-16-le") if grbit & 1 else b[8:8 + cch].decode("cp1252")
            except Exception:
                nm = "?"
            sheets.append((pos, nm))

    def fmt_of_xf(xf):
        if xf < 0 or xf >= len(xfs):
            return 'General'
        i = xfs[xf]
        return formats.get(i, BUILTIN.get(i, f'<{i}>'))

    # sheet cell index, lazily per sheet
    cellcache = {}

    def cells_of(sheet):
        if sheet in cellcache:
            return cellcache[sheet]
        out = {}
        if 0 <= sheet < len(sheets):
            pos = sheets[sheet][0]
            for off, rid, b in recs(d, pos):
                if rid == 0x000A:
                    break
                if rid == 0x0809 and off != pos:
                    pass
                try:
                    if rid == 0x0203 and len(b) >= 14:
                        r, c, xf = struct.unpack_from("<HHH", b, 0)
                        out[(r, c)] = (xf, struct.unpack_from("<d", b, 6)[0])
                    elif rid == 0x027E and len(b) >= 10:
                        r, c, xf = struct.unpack_from("<HHH", b, 0)
                        out[(r, c)] = (xf, rk(struct.unpack_from("<I", b, 6)[0]))
                    elif rid == 0x00BD and len(b) >= 6:
                        r, c1 = struct.unpack_from("<HH", b, 0)
                        n = (len(b) - 6) // 6
                        for i in range(n):
                            xf, raw = struct.unpack_from("<HI", b, 4 + i * 6)
                            out[(r, c1 + i)] = (xf, rk(raw))
                    elif rid == 0x0006 and len(b) >= 20:
                        r, c, xf = struct.unpack_from("<HHH", b, 0)
                        raw = b[6:14]
                        if raw[6:8] != b'\xff\xff':
                            out[(r, c)] = (xf, struct.unpack("<d", raw)[0])
                except Exception:
                    pass
        cellcache[sheet] = out
        return out

    # externsheet
    xtis = []
    supbooks = []
    for off, rid, b in recs(d):
        if rid == 0x0809 and off != 0:
            break
        if rid == 0x01AE:
            try:
                supbooks.append(len(b) == 4 and struct.unpack_from("<H", b, 2)[0] == 0x0401)
            except Exception:
                supbooks.append(False)
        if rid == 0x0017 and len(b) >= 2:
            n = struct.unpack_from("<H", b, 0)[0]
            n = min(n, (len(b) - 2) // 6)
            xtis = [struct.unpack_from("<HHH", b, 2 + i * 6) for i in range(n)]

    def sheet_of(ixti):
        if ixti < 0 or ixti >= len(xtis):
            return None
        sb, first, last = xtis[ixti]
        if sb >= len(supbooks) or not supbooks[sb]:
            return None
        if first == 0xFFFF:
            return None
        return first

    # chart substreams
    out = []
    inch = False
    stack = []
    header = 0
    cur = None
    axis = -1
    for off, rid, b in recs(d):
        if rid == 0x0809 and len(b) >= 4:
            inch = struct.unpack_from("<H", b, 2)[0] == 0x0020
            if inch:
                cur = {"file": path, "valueFmtRec": None, "catFmtRec": None,
                       "values": None, "cats": None, "axis_font": None}
                stack = []
                axis = -1
            elif cur:
                out.append(cur); cur = None
        if not inch or cur is None:
            continue
        if rid == 0x1033:
            stack.append(header); continue
        if rid == 0x1034:
            if stack: stack.pop()
            continue
        if 0x1000 <= rid <= 0x10FF:
            header = rid
        if rid == 0x101D and len(b) >= 2:
            axis = struct.unpack_from("<H", b, 0)[0]
        elif rid == 0x104E and len(b) >= 2:
            v = struct.unpack_from("<H", b, 0)[0]
            if axis == 1:
                cur["valueFmtRec"] = v
            elif axis == 0:
                cur["catFmtRec"] = v
        elif rid == 0x1051 and len(b) > 8 and stack and stack[-1] == 0x1003:
            dest, lt = b[0], b[1]
            fl = struct.unpack_from("<H", b, 6)[0]
            tok = b[8:8 + fl]
            rng = None
            if tok and (tok[0] & 0x3F) == 0x3B and len(tok) >= 11:
                ixti, r1, r2, c1, c2 = struct.unpack_from("<HHHHH", tok, 1)
                rng = (ixti, r1, r2, c1 & 0x3FFF, c2 & 0x3FFF)
            if rng:
                if dest == 1 and cur["values"] is None:
                    cur["values"] = rng
                elif dest == 2 and cur["cats"] is None:
                    cur["cats"] = rng
    if cur:
        out.append(cur)

    # resolve
    for c in out:
        for key, rngkey in (("valueFmt", "values"), ("catFmt", "cats")):
            rng = c[rngkey]
            c[key] = None
            if not rng:
                continue
            sh = sheet_of(rng[0])
            if sh is None:
                continue
            cells = cells_of(sh)
            ixti, r1, r2, c1, c2 = rng
            found = None
            for r in range(r1, min(r2, r1 + 70000) + 1):
                for col in range(c1, c2 + 1):
                    v = cells.get((r, col))
                    if v is not None:
                        found = fmt_of_xf(v[0])
                        break
                if found is not None:
                    break
            c[key] = found
    return out


def main(root):
    paths = []
    for dirpath, dirnames, filenames in os.walk(root):
        for f in filenames:
            paths.append(os.path.join(dirpath, f))
    docs_with_charts = 0
    charts = 0
    changed_value = 0
    changed_cat = 0
    changed_docs = set()
    for p in sorted(paths):
        try:
            with open(p, 'rb') as fh:
                if fh.read(8) != b'\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1':
                    continue
        except Exception:
            continue
        res = analyse(p)
        if not res:
            continue
        docs_with_charts += 1
        for c in res:
            charts += 1
            vf = c["valueFmt"]
            cf = c["catFmt"]
            vchanged = (c["valueFmtRec"] is not None) or (vf not in (None, 'General'))
            cchanged = (c["catFmtRec"] is not None) or (cf not in (None, 'General'))
            if vchanged:
                changed_value += 1
            if cchanged:
                changed_cat += 1
            if vchanged or cchanged:
                changed_docs.add(p)
            print(f"{os.path.basename(p)}\tchart\tvalRec={c['valueFmtRec']}\tvalFmt={vf!r}\tcatRec={c['catFmtRec']}\tcatFmt={cf!r}")
    print(f"\n# OLE2 docs holding a chart substream: {docs_with_charts}")
    print(f"# chart substreams: {charts}")
    print(f"# charts whose VALUE axis resolves to a non-General format: {changed_value}")
    print(f"# charts whose CATEGORY axis resolves to a non-General format: {changed_cat}")
    print(f"# documents affected: {len(changed_docs)}")
    for p in sorted(changed_docs):
        print("   ", p)


main(sys.argv[1])
