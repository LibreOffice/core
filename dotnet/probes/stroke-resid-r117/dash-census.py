#!/usr/bin/env python3
"""Census: which corpus spreadsheets state a dashed or dotted CELL BORDER.

Two levels per document:
  stated   -- a patterned style name appears in the workbook's style table
  used     -- some cell/row/column actually references a format carrying one

Zip (xlsx/xlsm) is read out of xl/styles.xml + the worksheets.
OLE (xls) is read out of the Workbook stream's BIFF XF and cell records.
Neither renders anything: this is a statement census, not an ink census.
"""
import os, re, sys, zipfile, struct, collections
import xml.etree.ElementTree as ET
import olefile

NS = '{http://schemas.openxmlformats.org/spreadsheetml/2006/main}'

# SpreadsheetML border style names that reach a dash array in LibreOffice.
XLSX_PATTERNED = {
    'dotted', 'dashed', 'dashDot', 'dashDotDot',
    'mediumDashed', 'mediumDashDot', 'mediumDashDotDot', 'slantDashDot',
}
XLSX_SOLID = {'hair', 'thin', 'medium', 'thick', 'double'}

# BIFF line styles 0..13; ppnLineParam (sc/source/filter/excel/xistyle.cxx).
BIFF_PATTERNED = {3, 4, 8, 9, 10, 11, 12, 13}
BIFF_NAME = {0:'none',1:'thin',2:'medium',3:'dashed',4:'dotted',5:'thick',6:'double',
             7:'hair',8:'mediumDashed',9:'dashDot',10:'mediumDashDot',11:'dashDotDot',
             12:'mediumDashDotDot',13:'slantDashDot'}


def tag(e):
    return e.tag.split('}')[-1]


def xlsx_census(path):
    """(stated set, used set, error)"""
    stated = collections.Counter()
    used = collections.Counter()
    with zipfile.ZipFile(path) as z:
        names = z.namelist()
        sty = next((n for n in names if n.lower().endswith('styles.xml')), None)
        if sty is None:
            return stated, used, None
        root = ET.fromstring(z.read(sty))
        borders = []
        bs = root.find(NS + 'borders')
        if bs is not None:
            for b in bs.findall(NS + 'border'):
                edges = set()
                for e in b:
                    if tag(e) in ('left', 'right', 'top', 'bottom', 'start', 'end', 'diagonal'):
                        s = e.get('style')
                        if s:
                            edges.add(s)
                borders.append(edges)
        for edges in borders:
            for s in edges:
                if s in XLSX_PATTERNED:
                    stated[s] += 1
        # dxf borders (conditional formats) count as stated too
        dxfs = root.find(NS + 'dxfs')
        dxf_pat = False
        if dxfs is not None:
            for d in dxfs.iter():
                if tag(d) in ('left','right','top','bottom','start','end') and d.get('style') in XLSX_PATTERNED:
                    stated['dxf:' + d.get('style')] += 1
                    dxf_pat = True
        # cellXfs -> borderId
        xfs = []
        cx = root.find(NS + 'cellXfs')
        stylexfs = []
        sx = root.find(NS + 'cellStyleXfs')
        if sx is not None:
            for x in sx.findall(NS + 'xf'):
                stylexfs.append(x)
        if cx is not None:
            for x in cx.findall(NS + 'xf'):
                bid = x.get('borderId')
                ab = x.get('applyBorder')
                if ab is not None and ab in ('0', 'false') and x.get('xfId') is not None:
                    i = int(x.get('xfId'))
                    if 0 <= i < len(stylexfs):
                        bid = stylexfs[i].get('borderId')
                xfs.append(int(bid) if bid is not None and bid.isdigit() else None)
        # which cellXf indices are actually referenced
        refd = set()
        for n in names:
            ln = n.lower()
            if '/worksheets/' not in ln or not ln.endswith('.xml'):
                continue
            data = z.read(n)
            for m in re.finditer(rb'<(?:c|col|row)\b[^>]*>', data):
                s = m.group(0)
                if s.startswith(b'<col'):
                    mm = re.search(rb'style="(\d+)"', s)
                elif s.startswith(b'<row'):
                    if b'customFormat' not in s:
                        continue
                    mm = re.search(rb'\bs="(\d+)"', s)
                else:
                    mm = re.search(rb'\bs="(\d+)"', s)
                if mm:
                    refd.add(int(mm.group(1)))
        for i in refd:
            if 0 <= i < len(xfs) and xfs[i] is not None:
                bi = xfs[i]
                if 0 <= bi < len(borders):
                    for s in borders[bi]:
                        if s in XLSX_PATTERNED:
                            used[s] += 1
        if dxf_pat:
            used['dxf'] += 0  # counted separately; conditional borders are not resolved here
    return stated, used, None


def biff_records(data):
    i = 0
    n = len(data)
    while i + 4 <= n:
        rid, ln = struct.unpack_from('<HH', data, i)
        i += 4
        if i + ln > n:
            break
        yield rid, data[i:i + ln]
        i += ln


def xls_census(path):
    stated = collections.Counter()
    used = collections.Counter()
    if not olefile.isOleFile(path):
        return None, None, 'not-ole'
    ole = olefile.OleFileIO(path)
    stream = None
    for cand in (['Workbook'], ['Book']):
        if ole.exists(cand[0]):
            stream = ole.openstream(cand[0]).read()
            break
    if stream is None:
        ole.close()
        return None, None, 'no-workbook-stream'
    ole.close()

    xf_edges = []      # list of tuple(l,r,t,b) line styles
    biff = None
    in_globals = True
    used_idx = set()
    for rid, body in biff_records(stream):
        if rid == 0x0809 and len(body) >= 4:      # BOF
            ver = struct.unpack_from('<H', body, 0)[0]
            typ = struct.unpack_from('<H', body, 2)[0]
            if biff is None:
                biff = ver
            in_globals = (typ == 0x0005)
        elif rid == 0x00E0:                        # XF
            if biff == 0x0600 and len(body) >= 20:
                b1, b2 = struct.unpack_from('<II', body, 10)
                xf_edges.append((b1 & 0xF, (b1 >> 4) & 0xF, (b1 >> 8) & 0xF, (b1 >> 12) & 0xF))
            elif len(body) >= 16:                  # BIFF5/7
                area, bor = struct.unpack_from('<II', body, 8)
                xf_edges.append(((bor >> 3) & 0x7, (bor >> 6) & 0x7, bor & 0x7, (area >> 22) & 0x7))
        elif rid in (0x0201, 0x0203, 0x027E, 0x00FD, 0x0006, 0x0205, 0x027E, 0x0204, 0x0202):
            # BLANK, NUMBER, RK, LABELSST, FORMULA, BOOLERR, LABEL, INTEGER: ixfe at offset 4
            if len(body) >= 6:
                used_idx.add(struct.unpack_from('<H', body, 4)[0])
        elif rid == 0x00BE and len(body) >= 6:     # MULBLANK
            k = (len(body) - 6) // 2
            for j in range(k):
                used_idx.add(struct.unpack_from('<H', body, 4 + 2 * j)[0])
        elif rid == 0x00BD and len(body) >= 6:     # MULRK
            k = (len(body) - 6) // 6
            for j in range(k):
                used_idx.add(struct.unpack_from('<H', body, 4 + 6 * j)[0])
        elif rid == 0x0208 and len(body) >= 16:    # ROW
            flags = struct.unpack_from('<I', body, 12)[0]
            if flags & 0x00000080:                 # fGhostDirty / has explicit format
                used_idx.add(struct.unpack_from('<H', body, 14)[0] & 0xFFF)

    for edges in xf_edges:
        for s in edges:
            if s in BIFF_PATTERNED:
                stated[BIFF_NAME[s]] += 1
    for i in used_idx:
        if 0 <= i < len(xf_edges):
            for s in xf_edges[i]:
                if s in BIFF_PATTERNED:
                    used[BIFF_NAME[s]] += 1
    return stated, used, None


def main(root, out):
    rows = []
    for dirpath, _, files in os.walk(os.path.join(root, 'sheets')):
        for f in sorted(files):
            rows.append(os.path.join(dirpath, f))
    rows.sort()
    with open(out, 'w') as fh:
        fh.write('path\text\tkind\tstated\tused\tstated_styles\tused_styles\terr\n')
        for p in rows:
            ext = os.path.splitext(p)[1].lstrip('.').lower()
            err = ''
            stated = used = None
            try:
                if zipfile.is_zipfile(p):
                    kind = 'zip'
                    stated, used, err = xlsx_census(p)
                elif olefile.isOleFile(p):
                    kind = 'ole'
                    stated, used, err = xls_census(p)
                else:
                    kind = 'other'
                    err = 'not-zip-not-ole'
            except Exception as e:
                kind = kind if 'kind' in dir() else '?'
                err = type(e).__name__ + ':' + str(e)[:80]
            st = sum(stated.values()) if stated else 0
            us = sum(used.values()) if used else 0
            fh.write('%s\t%s\t%s\t%d\t%d\t%s\t%s\t%s\n' % (
                os.path.relpath(p, root), ext, kind, st, us,
                ','.join('%s=%d' % kv for kv in sorted((stated or {}).items())),
                ','.join('%s=%d' % kv for kv in sorted((used or {}).items())),
                err or ''))
    print('wrote', out, len(rows), 'documents')


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
