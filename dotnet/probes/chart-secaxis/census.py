#!/usr/bin/env python3
"""Reach of every mechanism this round names, over the whole corpus.

BIFF (.xls/.ppt/.doc chart substreams) and OOXML (chart parts inside a zip) are counted
separately because they are different readers with different defects.
"""
import os, re, struct, sys, zipfile, csv
import olefile

CORPUS = sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files'

# ---- BIFF -----------------------------------------------------------------
def biff_records(data):
    p = 0
    n = len(data)
    while p + 4 <= n:
        rid, ln = struct.unpack_from('<HH', data, p)
        yield rid, data[p+4:p+4+ln]
        p += 4 + ln

def scan_biff(path):
    """Per chart substream: axes-set count, type-group count, showVisibleOnly, legend, frame."""
    out = []
    try:
        ole = olefile.OleFileIO(path)
    except Exception:
        return out
    names = ['/'.join(e) for e in ole.listdir()]
    streams = [n for n in names if n.split('/')[-1] in ('Workbook', 'Book', 'PowerPoint Document', 'WordDocument')]
    for nm in streams:
        try:
            data = ole.openstream(nm).read()
        except Exception:
            continue
        cur = None
        hidden_rows = {}           # sheet substream not tracked here; done separately
        for rid, b in biff_records(data):
            if rid == 0x0809 and len(b) >= 4:
                dt = struct.unpack_from('<H', b, 2)[0]
                if dt == 0x0020:
                    cur = dict(axessets=0, usedaxes=0, typegroups=0, visonly=None,
                               legend=0, legend_notdocked=0, valranges=0, chart=0)
                    out.append(cur)
                else:
                    cur = None
            if cur is None:
                continue
            if rid == 0x1002: cur['chart'] += 1
            elif rid == 0x1041: cur['axessets'] += 1
            elif rid == 0x1046 and len(b) >= 2: cur['usedaxes'] = struct.unpack_from('<H', b, 0)[0]
            elif rid == 0x1014: cur['typegroups'] += 1
            elif rid == 0x101F: cur['valranges'] += 1
            elif rid == 0x1044 and len(b) >= 2:
                cur['visonly'] = bool(struct.unpack_from('<H', b, 0)[0] & 0x0002)
            elif rid == 0x1015 and len(b) >= 20:
                cur['legend'] += 1
                if b[16] == 7: cur['legend_notdocked'] += 1
    try: ole.close()
    except Exception: pass
    return out

def biff_hidden_rows(path):
    """True when the file has any hidden row or column at all."""
    try:
        ole = olefile.OleFileIO(path)
    except Exception:
        return False
    for nm in ('Workbook', 'Book'):
        if not ole.exists(nm):
            continue
        data = ole.openstream(nm).read()
        for rid, b in biff_records(data):
            if rid == 0x0208 and len(b) >= 14:
                grbit = struct.unpack_from('<H', b, 12)[0]
                if grbit & 0x20: return True
            if rid == 0x007D and len(b) >= 10:
                if struct.unpack_from('<H', b, 8)[0] & 0x0001: return True
    return False

# ---- OOXML ----------------------------------------------------------------
C = '{http://schemas.openxmlformats.org/drawingml/2006/chart}'

def scan_ooxml(path):
    rows = []
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return rows
    for name in z.namelist():
        if '/charts/chart' not in name and '/charts/' not in name:
            continue
        if not name.endswith('.xml') or 'colors' in name or 'style' in name:
            continue
        try:
            x = z.read(name).decode('utf-8', 'replace')
        except Exception:
            continue
        if '<c:chartSpace' not in x and 'chartSpace' not in x:
            continue
        groups = re.findall(r'<c:(\w+Chart)\b', x)
        groups = [g for g in groups if g not in ()]
        # axis ids per group, in document order
        gaxes = []
        for m in re.finditer(r'<c:(\w+Chart)\b(.*?)</c:\1>', x, re.S):
            ids = re.findall(r'<c:axId val="(-?\d+)"/>', m.group(2))
            sers = m.group(2).count('<c:ser>')
            gaxes.append((m.group(1), ids, sers))
        axorder = re.findall(r'<c:(catAx|valAx|dateAx|serAx)>\s*<c:axId val="(-?\d+)"/>', x)
        pairs = {tuple(ids) for _, ids, s in gaxes if s > 0}
        rows.append(dict(
            part=name,
            groups=len(gaxes),
            axpairs=len(pairs),
            valax=x.count('<c:valAx>'),
            dateax=x.count('<c:dateAx>'),
            catax=x.count('<c:catAx>'),
            plotvisonly=('<c:plotVisOnly val="0"/>' not in x),
            varycolors=len(re.findall(r'<c:varyColors val="1"/>', x)),
            dpt=x.count('<c:dPt>'),
            cellrange=x.count('type="CELLRANGE"'),
            dlblrange=x.count('c15:datalabelsRange'),
            strlit=x.count('<c:strLit>'),
            numlit=x.count('<c:numLit>'),
            spacebold=1 if re.search(r'</c:plotArea>.*<c:txPr>.*?<a:defRPr[^>]*\bb="1"', x, re.S) else 0,
            axorder=';'.join(f'{t}:{i}' for t, i in axorder),
            gaxorder='|'.join(f'{g}:{",".join(i)}:{s}' for g, i, s in gaxes),
        ))
    return rows

def main():
    biff_out = []
    ooxml_out = []
    for root, _, files in os.walk(CORPUS):
        if '/.git' in root: continue
        for f in files:
            p = os.path.join(root, f)
            low = f.lower()
            rel = os.path.relpath(p, CORPUS)
            if low.endswith(('.xls', '.doc', '.ppt', '.xlt', '.pot', '.pps')):
                subs = scan_biff(p)
                if subs:
                    hid = biff_hidden_rows(p)
                    for s in subs:
                        s['doc'] = rel; s['hiddenrows'] = hid
                        biff_out.append(s)
            elif low.endswith(('.xlsx', '.xlsm', '.pptx', '.docx', '.xltx', '.potx')):
                for r in scan_ooxml(p):
                    r['doc'] = rel
                    ooxml_out.append(r)
    for name, rows in (('biff', biff_out), ('ooxml', ooxml_out)):
        if not rows: continue
        keys = list(rows[0].keys())
        with open(f'/home/user/wt-secaxis/dotnet/probes/chart-secaxis/census-{name}.tsv', 'w', newline='') as fh:
            w = csv.DictWriter(fh, fieldnames=keys, delimiter='\t')
            w.writeheader()
            for r in rows: w.writerow(r)
        print(name, len(rows), 'rows')

main()
