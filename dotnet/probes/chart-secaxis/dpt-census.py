#!/usr/bin/env python3
"""Which chart parts state a per-point fill or varyColors on a series that is NOT a pie."""
import os, re, sys, zipfile, csv
CORPUS = sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files'
PIE = ('pieChart', 'pie3DChart', 'doughnutChart', 'ofPieChart')
rows = []
for root, _, files in os.walk(CORPUS):
    for f in files:
        if not f.lower().endswith(('.xlsx','.xlsm','.pptx','.docx','.xltx','.potx')): continue
        p = os.path.join(root, f); rel = os.path.relpath(p, CORPUS)
        try: z = zipfile.ZipFile(p)
        except Exception: continue
        for name in z.namelist():
            if '/charts/' not in name or not name.endswith('.xml'): continue
            if 'colors' in name or 'style' in name: continue
            try: x = z.read(name).decode('utf-8','replace')
            except Exception: continue
            if 'chartSpace' not in x: continue
            for m in re.finditer(r'<c:(\w+Chart)\b(.*?)</c:\1>', x, re.S):
                g, body = m.group(1), m.group(2)
                if g in PIE: continue
                nser = body.count('<c:ser>')
                # dPt entries carrying an actual spPr fill
                dpt = len(re.findall(r'<c:dPt>(?:(?!</c:dPt>).)*?<c:spPr>(?:(?!</c:dPt>).)*?</c:dPt>', body, re.S))
                vary = 1 if '<c:varyColors val="1"/>' in body else 0
                if dpt or vary:
                    rows.append(dict(doc=rel, part=name, group=g, series=nser, dpt=dpt, vary=vary))
with open('/home/user/wt-secaxis/dotnet/probes/chart-secaxis/census-dpt.tsv','w',newline='') as fh:
    w=csv.DictWriter(fh,fieldnames=['doc','part','group','series','dpt','vary'],delimiter='\t'); w.writeheader()
    for r in rows: w.writerow(r)
byd={}
for r in rows: byd.setdefault(r['doc'],[]).append(r)
print('non-pie groups with a per-point fill or varyColors:',len(rows),'in',len(byd),'documents')
d=[r for r in rows if r['dpt']]
print('  with c:dPt fills:',len(d),'groups in',len({r['doc'] for r in d}),'documents')
v=[r for r in rows if r['vary'] and r['series']==1]
print('  varyColors=1 with a single series:',len(v),'groups in',len({r['doc'] for r in v}),'documents')
