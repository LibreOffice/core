#!/usr/bin/env python3
"""Which value axis is the primary one, by LibreOffice's rule and by ours.

LibreOffice groups the type groups into axes sets by identical c:axId lists and then decides
which set is index 0 (plotareaconverter.cxx:466-468); ours takes the first c:valAx in the plot
area's own element order. The two agree for every combined chart and can differ otherwise.
"""
import os, re, sys, zipfile, csv
CORPUS = sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files'
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
            if '<c:chartSpace' not in x: continue
            plot = x.split('<c:plotArea>')[-1].split('</c:plotArea>')[0]
            groups = []
            for m in re.finditer(r'<c:(\w+Chart)\b(.*?)</c:\1>', plot, re.S):
                ids = re.findall(r'<c:axId val="(-?\d+)"/>', m.group(2))
                groups.append((m.group(1), ids, m.group(2).count('<c:ser>')))
            axes = re.findall(r'<c:(catAx|valAx|dateAx|serAx)>\s*<c:axId val="(-?\d+)"/>', plot)
            valax_order = [i for t, i in axes if t == 'valAx']
            # axes sets: distinct axId lists, in document order, of groups holding series
            sets = []
            for g, ids, ser in groups:
                if ser == 0 or not ids: continue
                if not any(s[0] == ids for s in sets): sets.append((ids, g))
            if len(sets) < 2: continue
            real = [ids[1] for _, ids, ser in groups if len(ids) > 1]
            valids = [i for i in valax_order if i in real]
            combined = len(groups) == 2 and groups[0][0] != groups[1][0]
            start = 1 if (combined and len(valids) > 1 and len(sets) > 0
                          and len(sets[0][0]) > 1 and sets[0][0][1] != valids[0]) else 0
            lo_primary_set = next(k for k in range(len(sets)) if (start + k) % 2 == 0)
            lo_primary_y = sets[lo_primary_set][0][1] if len(sets[lo_primary_set][0]) > 1 else ''
            ours_primary_y = valids[0] if valids else (valax_order[0] if valax_order else '')
            rows.append(dict(doc=rel, part=name, groups=len(groups), sets=len(sets),
                             combined=int(combined), start=start,
                             lo=lo_primary_y, ours=ours_primary_y,
                             agree=int(lo_primary_y == ours_primary_y),
                             lo_cat_group=sets[lo_primary_set][1],
                             first_group=groups[0][0]))
with open('/home/user/wt-secaxis/dotnet/probes/chart-secaxis/census-axesset.tsv','w',newline='') as fh:
    w=csv.DictWriter(fh,fieldnames=list(rows[0].keys()),delimiter='\t'); w.writeheader()
    for r in rows: w.writerow(r)
print('chart parts with two axes sets:',len(rows),'in',len({r['doc'] for r in rows}),'documents')
d=[r for r in rows if not r['agree']]
print('  where the two rules name a different primary value axis:',len(d),'in',len({r['doc'] for r in d}),'docs')
c=[r for r in rows if r['lo_cat_group']!=r['first_group']]
print("  where LibreOffice's category group is not the first group:",len(c),'in',len({r['doc'] for r in c}),'docs')
for r in c: print('    ',r['doc'].split('/')[-1], r['part'], r['first_group'],'->',r['lo_cat_group'],'combined',r['combined'],'start',r['start'])
