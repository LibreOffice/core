"""Reach census over the whole corpus for this round's number-format questions."""
import re, sys, zipfile, struct, collections
from pathlib import Path
import olefile

CORPUS = Path('/home/user/sample-files')
docs = sorted(p for p in CORPUS.rglob('*') if p.is_file())

xlsx_like = {'.xlsx', '.xlsm', '.xltx', '.xltm'}
xls_like = {'.xls', '.xlt'}

res = collections.defaultdict(set)
codes_aaa = collections.defaultdict(set)

def txt(z, name):
    try: return z.read(name).decode('utf-8', 'replace')
    except KeyError: return None

for p in docs:
    ext = p.suffix.lower()
    rel = str(p.relative_to(CORPUS))
    if ext in xlsx_like:
        try: z = zipfile.ZipFile(p)
        except Exception: continue
        names = z.namelist()
        st = next((n for n in names if n.lower().endswith('xl/styles.xml')), None)
        if not st: continue
        s = txt(z, st) or ''
        # custom codes
        custom = dict((int(m.group(1)), m.group(2)) for m in
                      re.finditer(r'<numFmt[^>]*numFmtId="(\d+)"[^>]*formatCode="([^"]*)"', s))
        for i, c in custom.items():
            u = c.replace('&quot;', '"')
            # strip quoted literals and backslash escapes before looking for keys
            bare = re.sub(r'"[^"]*"', '', u)
            bare = re.sub(r'\\.', '', bare)
            if re.search(r'a{3,4}', bare, re.I): codes_aaa['aaa'].add(rel)
            if re.search(r'(?<![a-z])n{1,4}(?![a-z])', bare, re.I): codes_aaa['nnn'].add(rel)
        # cellStyleXfs[0] / Normal cellStyle
        m = re.search(r'<cellStyleXfs[^>]*>(.*?)</cellStyleXfs>', s, re.S)
        csx = [int(x) for x in re.findall(r'<xf[^>]*numFmtId="(\d+)"', m.group(1))] if m else []
        m2 = re.search(r'<cellXfs[^>]*>(.*?)</cellXfs>', s, re.S)
        cx_raw = re.findall(r'<xf\b[^>]*?/?>', m2.group(1)) if m2 else []
        cx = [int(re.search(r'numFmtId="(\d+)"', x).group(1)) if 'numFmtId=' in x else 0
              for x in cx_raw]
        normal = re.search(r'<cellStyle[^>]*builtinId="0"[^>]*/>', s)
        nx = 0
        if normal:
            mm = re.search(r'xfId="(\d+)"', normal.group(0))
            if mm: nx = int(mm.group(1))
        default_fmt = csx[nx] if nx < len(csx) else 0
        cx0 = cx[0] if cx else 0
        for n in names:
            if not re.search(r'xl/worksheets/sheet[^/]*\.xml$', n, re.I): continue
            ws = txt(z, n) or ''
            has_bare = re.search(r'<c r="[A-Z]+\d+"(?:\s*/?>|\s+t=)', ws) is not None
            if has_bare:
                if default_fmt != 0: res['no-s-default-nonzero'].add(rel)
                if cx0 != default_fmt: res['no-s-cellxf0-differs'].add(rel)
                if cx0 != default_fmt and default_fmt != 0: res['no-s-both-matter'].add(rel)
            for cm in re.finditer(r'<col\b[^>]*style="(\d+)"', ws):
                if int(cm.group(1)) < len(cx) and cx[int(cm.group(1))] != 0:
                    res['col-style-fmt'].add(rel)
            for rm in re.finditer(r'<row\b[^>]*>', ws):
                t = rm.group(0)
                if 'customFormat' in t and re.search(r'\ss="(\d+)"', t):
                    sv = int(re.search(r'\ss="(\d+)"', t).group(1))
                    if sv < len(cx) and cx[sv] != 0: res['row-custom-fmt'].add(rel)
    elif ext in xls_like:
        try:
            f = olefile.OleFileIO(str(p))
            if not f.exists('Workbook'):
                f.close(); continue
            data = f.openstream('Workbook').read(); f.close()
        except Exception: continue
        i = 0; fmts = {}; xfs = []
        while i + 4 <= len(data):
            rec, ln = struct.unpack_from('<HH', data, i); i += 4
            body = data[i:i+ln]; i += ln
            if rec == 0x041E and len(body) >= 5:
                idx = struct.unpack_from('<H', body, 0)[0]
                cch = struct.unpack_from('<H', body, 2)[0]
                flags = body[4]
                try:
                    fmts[idx] = (body[5:5+cch*2].decode('utf-16le') if flags & 1
                                 else body[5:5+cch].decode('latin-1'))
                except Exception: pass
            elif rec == 0x00E0 and len(body) >= 4:
                xfs.append(struct.unpack_from('<HH', body, 0)[1])
        used = set(xfs)
        for c in fmts.values():
            bare = re.sub(r'"[^"]*"', '', c); bare = re.sub(r'\\.', '', bare)
            if re.search(r'a{3,4}', bare, re.I): codes_aaa['aaa'].add(rel)
            if re.search(r'(?<![a-z])n{1,4}(?![a-z])', bare, re.I): codes_aaa['nnn'].add(rel)
        for b in (14, 15, 16, 20, 21, 22, 37, 38, 39, 40, 63, 64, 65, 66, 5, 6, 7, 8, 41, 42, 43, 44):
            if b in used and b not in fmts:
                res[f'xls-builtin-{b}'].add(rel)

print('== xlsx default-style questions ==')
for k in ('no-s-default-nonzero', 'no-s-cellxf0-differs', 'no-s-both-matter',
          'col-style-fmt', 'row-custom-fmt'):
    print(f'{k:26s} {len(res[k])}')
print('== format-code keys ==')
for k, v in codes_aaa.items():
    print(f'{k:6s} {len(v)}  {sorted(v)[:8]}')
print('== xls builtin ids used without a FORMAT record ==')
for k in sorted(res):
    if k.startswith('xls-builtin'):
        print(f'{k:22s} {len(res[k])}')
import json
json.dump({k: sorted(v) for k, v in res.items()} | {'code_' + k: sorted(v) for k, v in codes_aaa.items()},
          open('/home/user/numfmt-work/census.json', 'w'), indent=1)
