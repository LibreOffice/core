"""Charts whose horizontal axis is a catAx/dateAx while a scatter or bubble group is present."""
import re, zipfile, collections
from pathlib import Path
CORPUS = Path('/home/user/sample-files')
res = collections.defaultdict(set)
det = []
for p in sorted(CORPUS.rglob('*')):
    if not p.is_file() or p.suffix.lower() not in {'.xlsx','.xlsm','.pptx','.pptm','.docx','.docm','.xltx','.potx'}:
        continue
    rel = str(p.relative_to(CORPUS))
    try: z = zipfile.ZipFile(p)
    except Exception: continue
    for n in z.namelist():
        if not re.search(r'charts?/chart\d*\.xml$', n, re.I): continue
        try: s = z.read(n).decode('utf-8','replace')
        except Exception: continue
        scatter = 'c:scatterChart' in s or 'c:bubbleChart' in s
        if not scatter: continue
        res['scatter-parts'].add(rel)
        cat = re.search(r'<c:(catAx|dateAx)>', s)
        vals = len(re.findall(r'<c:valAx>', s))
        if cat and vals < 2:
            res['scatter-with-catax'].add(rel)
            axm = re.search(r'<c:(catAx|dateAx)>.*?</c:\1>', s, re.S)
            fmt = re.search(r'<c:numFmt[^>]*formatCode="([^"]*)"', axm.group(0)) if axm else None
            code = fmt.group(1) if fmt else None
            if code and code.lower() != 'general':
                res['scatter-catax-with-format'].add(rel)
                det.append((rel, n, cat.group(1), code))
for k in sorted(res): print(f'{k:28s} {len(res[k])}')
print()
for d in det: print('  ', d)
