"""Scan the corpus for multi-range sqrefs, and for the ones where Calc's
(tab,col,row) minimum differs from the componentwise minimum Excel writes
formulas against."""
import glob, zipfile, re, sys
ROOT = sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files'

def parse(a):
    m = re.match(r'\$?([A-Z]+)\$?(\d+)$', a)
    col = 0
    for ch in m.group(1):
        col = col * 26 + ord(ch) - 64
    return col, int(m.group(2))

def start(t):
    p = t.split(':')
    a, b = parse(p[0]), parse(p[-1])
    return (min(a[0], b[0]), min(a[1], b[1]))

dis, tot = [], 0
for f in glob.glob(ROOT + '/**/*.xlsx', recursive=True):
    try:
        z = zipfile.ZipFile(f)
    except Exception:
        continue
    for nm in z.namelist():
        if not re.match(r'xl/worksheets/sheet.*\.xml$', nm):
            continue
        try:
            s = z.read(nm).decode('utf-8', 'replace')
        except Exception:
            continue
        for m in re.finditer(r'<conditionalFormatting[^>]*sqref="([^"]+)"[^>]*>(.*?)</conditionalFormatting>', s, re.S):
            sq = m.group(1).split()
            if len(sq) < 2:
                continue
            tot += 1
            try:
                st = [start(t) for t in sq]
            except Exception:
                continue
            cw = (min(c for c, _ in st), min(r for _, r in st))          # componentwise minimum
            sa = min(st, key=lambda t: (t[0], t[1]))                      # ScAddress ordering
            if cw != sa:
                dis.append((f.split('/')[-1], nm, m.group(1)[:80], cw, sa,
                            re.findall(r'<formula>(.*?)</formula>', m.group(2))[:2]))
print('multi-range sqrefs:', tot, ' disagreeing:', len(dis))
for d in dis:
    print(d)
