#!/usr/bin/env python3
"""How many corpus charts name a range whose rows or columns the sheet hides.

`ScChart2DataSequence::BuildDataCache` drops a cell in a hidden row or column unless the
chart's diagram says IncludeHiddenCells, which oox sets to `!c:plotVisOnly`
(chartspaceconverter.cxx:264).  So a hidden row inside a chart's own range takes a point
off the chart.
"""
import pathlib
import re
import sys
import zipfile

CORPUS = pathlib.Path('/home/user/sample-files')
COL = re.compile(r'<col ([^>]*)/>')
ROW = re.compile(r'<row ([^>]*?)/?>')
F = re.compile(r'<c:f>([^<]*)</c:f>')
REF = re.compile(r"^(?:'([^']*)'|([^!]*))!\$?([A-Z]+)\$?(\d+)(?::\$?([A-Z]+)\$?(\d+))?$")


def colnum(s):
    n = 0
    for ch in s:
        n = n * 26 + (ord(ch) - 64)
    return n


def sheet_hidden(data):
    """(hidden column numbers, hidden row numbers) of one worksheet part."""
    cols = set()
    for m in COL.finditer(data):
        a = m.group(1)
        if 'hidden="1"' not in a:
            continue
        lo = int(re.search(r'min="(\d+)"', a).group(1))
        hi = int(re.search(r'max="(\d+)"', a).group(1))
        cols.update(range(lo, min(hi, 16384) + 1))
    rows = set()
    for m in ROW.finditer(data):
        a = m.group(1)
        if 'hidden="1"' not in a:
            continue
        rows.add(int(re.search(r'r="(\d+)"', a).group(1)))
    return cols, rows


def main():
    hits = []
    for path in sorted(CORPUS.rglob('*')):
        if not path.is_file() or path.suffix.lower() not in ('.xlsx', '.xlsm'):
            continue
        try:
            z = zipfile.ZipFile(path)
        except Exception:
            continue
        charts = [n for n in z.namelist() if re.match(r'xl/charts/chart\d+\.xml$', n)]
        if not charts:
            continue
        # sheet name -> worksheet part, via workbook.xml order and the rels
        try:
            wb = z.read('xl/workbook.xml').decode('utf-8', 'replace')
            rels = z.read('xl/_rels/workbook.xml.rels').decode('utf-8', 'replace')
        except Exception:
            continue
        target = {}
        for rel in re.findall(r'<Relationship\b[^>]*/>', rels):
            rid = re.search(r'Id="([^"]+)"', rel)
            tgt = re.search(r'Target="([^"]+)"', rel)
            if rid and tgt:
                target[rid.group(1)] = tgt.group(1)
        sheets = {}
        for m in re.finditer(r'<sheet[^>]*name="([^"]*)"[^>]*r:id="([^"]*)"', wb):
            t = target.get(m.group(2), '')
            sheets[m.group(1)] = ('xl/' + t.lstrip('/')).replace('xl/xl/', 'xl/')
        cache = {}
        for c in charts:
            body = z.read(c).decode('utf-8', 'replace')
            visonly = '<c:plotVisOnly val="0"/>' not in body
            for f in F.findall(body):
                m = REF.match(f.strip())
                if not m:
                    continue
                name = m.group(1) or m.group(2)
                part = sheets.get(name)
                if not part:
                    continue
                if part not in cache:
                    try:
                        cache[part] = sheet_hidden(z.read(part).decode('utf-8', 'replace'))
                    except Exception:
                        cache[part] = (set(), set())
                hcols, hrows = cache[part]
                c1, r1 = colnum(m.group(3)), int(m.group(4))
                c2 = colnum(m.group(5)) if m.group(5) else c1
                r2 = int(m.group(6)) if m.group(6) else r1
                cells = (c2 - c1 + 1) * (r2 - r1 + 1)
                hidden = sum(1 for cc in range(c1, c2 + 1) for rr in range(r1, r2 + 1)
                             if cc in hcols or rr in hrows)
                if hidden:
                    hits.append((str(path.relative_to(CORPUS)), c, f, cells, hidden, visonly))
    print('path\tchart\tformula\tcells\thidden\tplotVisOnly')
    for h in hits:
        print('%s\t%s\t%s\t%d\t%d\t%s' % h)
    docs = {h[0] for h in hits}
    allhidden = {h[0] for h in hits if h[3] == h[4]}
    print(f'# {len(hits)} sequences in {len(docs)} documents; '
          f'{len(allhidden)} documents have a sequence entirely hidden', file=sys.stderr)


if __name__ == '__main__':
    main()
