"""Reach: workbooks whose font colours name a theme slot the workbook re-themed."""
import re, zipfile, collections
from pathlib import Path

CORPUS = Path('/home/user/sample-files')
DEFAULT = {0:0xFFFFFF,1:0x000000,2:0xE7E6E6,3:0x44546A,4:0x4472C4,5:0xED7D31,6:0xA5A5A5,
           7:0xFFC000,8:0x5B9BD5,9:0x70AD47,10:0x0563C1,11:0x954F72}
SLOTS = ['lt1','dk1','lt2','dk2','accent1','accent2','accent3','accent4','accent5','accent6',
         'hlink','folHlink']
A = 'http://schemas.openxmlformats.org/drawingml/2006/main'

res = collections.defaultdict(set)
detail = []
for p in sorted(CORPUS.rglob('*')):
    if not p.is_file() or p.suffix.lower() not in {'.xlsx','.xlsm','.xltx','.xltm','.xlsb'}:
        continue
    rel = str(p.relative_to(CORPUS))
    try: z = zipfile.ZipFile(p)
    except Exception: continue
    names = z.namelist()
    st = next((n for n in names if n.lower() == 'xl/styles.xml'), None)
    if not st: continue
    s = z.read(st).decode('utf-8','replace')
    fonts = re.search(r'<fonts\b.*?</fonts>', s, re.S)
    if not fonts: continue
    used = set(int(m.group(1)) for m in re.finditer(r'<color[^>]*\btheme="(\d+)"', fonts.group(0)))
    idx65 = re.search(r'<color[^>]*\bindexed="(64|65|81)"', fonts.group(0))
    if idx65: res['font-indexed-auto'].add(rel)
    if not used: continue
    res['uses-font-theme'].add(rel)
    # find the theme part via the workbook relationship
    rels = next((n for n in names if n.lower() == 'xl/_rels/workbook.xml.rels'), None)
    target = None
    if rels:
        r = z.read(rels).decode('utf-8','replace')
        m = re.search(r'<Relationship[^>]*relationships/theme"[^>]*Target="([^"]+)"', r) \
            or re.search(r'<Relationship[^>]*Target="([^"]+)"[^>]*relationships/theme"', r)
        if m: target = m.group(1)
    cand = []
    if target:
        t = target.lstrip('/')
        if not t.startswith('xl/'): t = 'xl/' + t
        cand.append(t)
    cand.append('xl/theme/theme1.xml')
    tp = next((c for c in cand if c in names), None)
    if tp is None:
        res['no-theme-part'].add(rel); continue
    if tp != 'xl/theme/theme1.xml': res['theme-part-not-theme1'].add(rel)
    ts = z.read(tp).decode('utf-8','replace')
    sch = re.search(r'<a:clrScheme\b.*?</a:clrScheme>', ts, re.S)
    if not sch: res['no-clrscheme'].add(rel); continue
    body = sch.group(0)
    diffs = []
    for i, slot in enumerate(SLOTS):
        m = re.search(rf'<a:{slot}>\s*<a:(srgbClr val="([0-9A-Fa-f]{{6}})"|sysClr[^>]*lastClr="([0-9A-Fa-f]{{6}})")', body)
        if not m: continue
        val = int(m.group(2) or m.group(3), 16)
        if val != DEFAULT[i] and i in used:
            diffs.append((i, slot, f'{DEFAULT[i]:06X}', f'{val:06X}'))
    if diffs:
        res['font-theme-wrong'].add(rel)
        detail.append((rel, diffs))

for k in sorted(res):
    print(f'{k:26s} {len(res[k])}')
print()
print('documents whose font theme colours we currently get wrong:')
for rel, d in sorted(detail):
    print(' ', rel)
    for i, slot, was, now in d:
        print(f'      theme={i} {slot}: ours {was} -> file {now}')
