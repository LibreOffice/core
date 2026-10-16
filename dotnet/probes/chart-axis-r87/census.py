import re, sys, zipfile
from pathlib import Path
root=Path('/home/user/sample-files')
rows=[]
for p in sorted(root.rglob('*')):
    if not p.is_file() or p.suffix.lower() not in ('.xlsx','.xlsm'): continue
    try:
        with zipfile.ZipFile(p) as z:
            names=z.namelist()
            charts=[n for n in names if re.match(r'xl/charts?/chart\d+\.xml$',n)]
            if not charts: continue
            scales=set(); fit=False
            for n in names:
                if re.match(r'xl/worksheets/sheet\w*\.xml$',n):
                    t=z.read(n).decode('utf-8','replace')
                    for m in re.finditer(r'<pageSetup[^>]*>',t):
                        s=re.search(r'scale="(\d+)"',m.group(0))
                        scales.add(int(s.group(1)) if s else 100)
                    if 'fitToPage="1"' in t: fit=True
            rows.append((p.relative_to(root), len(charts), sorted(scales), fit))
    except Exception as e:
        pass
print(f"{len(rows)} xlsx/xlsm with a chart part")
n_scaled=sum(1 for r in rows if r[3] or any(s!=100 for s in r[2]))
print(f"{n_scaled} of them state fitToPage or a scale != 100")
for r in rows:
    if r[3] or any(s!=100 for s in r[2]):
        print(f"  {r[0]}\tcharts={r[1]}\tscales={r[2]}\tfitToPage={r[3]}")
