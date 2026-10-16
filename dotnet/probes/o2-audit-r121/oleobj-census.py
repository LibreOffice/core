#!/usr/bin/env python3
"""Which corpus presentations put an embedded OLE object on a slide, and at what scale.

Round 94 described `NAS-Infrastructure-Roadmaps-Weather.pptx` page 11 as "a `p:graphicFrame`
table we lay out nominally and stretch".  It is not a table: the frame's `graphicData` is
`.../presentationml/2006/ole` holding a `p:oleObj progId="Excel.Sheet.12"`.  So the reach
question is "how many slides carry an embedded OLE object", not "how many carry a table",
and this counts it with the base rate C9 wants.

For each `p:oleObj` it prints the frame extent and the `imgW`/`imgH` the file states for the
cached presentation, whose ratio is the only scale visible without rendering.
"""
import csv, pathlib, re, zipfile

CORPUS = pathlib.Path('/home/user/sample-files')
rows = list(csv.DictReader(open(CORPUS / 'MANIFEST.tsv'), delimiter='\t'))

pres = [r for r in rows if r['ext'].lower() in
        ('pptx', 'pptm', 'potx', 'potm', 'ppsx', 'ppsm', 'ppt', 'pot', 'pps',
         'odp', 'otp', 'fodp', 'sxi')]
zipped = [r for r in pres if r['ext'].lower() in ('pptx', 'pptm', 'potx', 'potm', 'ppsx', 'ppsm')]

FRAME = re.compile(rb'<p:graphicFrame>.*?</p:graphicFrame>', re.S)
XFRM = re.compile(rb'<p:xfrm[^>]*>.*?<a:off x="(-?\d+)" y="(-?\d+)"/>\s*'
                  rb'<a:ext cx="(\d+)" cy="(\d+)"/>', re.S)
OLE = re.compile(rb'<p:oleObj[^>]*>')

docs = frames = oles = 0
print('doc\tslide\tprogId\tframe_pt\timgW_pt\timgH_pt\tratio_w\tratio_h')
for r in zipped:
    p = CORPUS / r['path']
    if not p.is_file(): continue
    hit = False
    try:
        z = zipfile.ZipFile(p)
    except Exception:
        continue
    for nm in z.namelist():
        if not re.match(r'ppt/slides/slide\d+\.xml$', nm): continue
        try: d = z.read(nm)
        except Exception: continue
        for fm in FRAME.finditer(d):
            frames += 1
            body = fm.group(0)
            om = OLE.search(body)
            if not om: continue
            oles += 1; hit = True
            xm = XFRM.search(body)
            cx = cy = 0
            if xm: cx, cy = int(xm.group(3)), int(xm.group(4))
            prog = re.search(rb'progId="([^"]*)"', om.group(0))
            iw = re.search(rb'imgW="(\d+)"', om.group(0))
            ih = re.search(rb'imgH="(\d+)"', om.group(0))
            iwv = int(iw.group(1)) if iw else 0
            ihv = int(ih.group(1)) if ih else 0
            print('%s\t%s\t%s\t%.1fx%.1f\t%.1f\t%.1f\t%s\t%s' % (
                p.name, nm.rsplit('/', 1)[1], prog.group(1).decode() if prog else '',
                cx / 12700, cy / 12700, iwv / 12700, ihv / 12700,
                '%.4f' % (cx / iwv) if iwv else '-',
                '%.4f' % (cy / ihv) if ihv else '-'))
    if hit: docs += 1

print('# zip presentations scanned : %d' % len(zipped))
print('# presentations of any kind : %d' % len(pres))
print('# corpus documents          : %d' % len(rows))
print('# p:graphicFrame seen       : %d' % frames)
print('# p:oleObj seen             : %d in %d documents' % (oles, docs))
