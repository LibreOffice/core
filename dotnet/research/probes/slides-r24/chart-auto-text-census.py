import sys, zipfile, re, os, glob
from xml.etree import ElementTree as ET
C='{http://schemas.openxmlformats.org/drawingml/2006/chart}'
A='{http://schemas.openxmlformats.org/drawingml/2006/main}'

def size_of(el):
    if el is None: return None
    for p in el.iter():
        if p.tag in (A+'defRPr', A+'rPr'):
            sz = p.get('sz')
            if sz and sz.isdigit() and int(sz) > 0: return int(sz)/100.0
    return None

def bold_of(el):
    if el is None: return None
    for p in el.iter():
        if p.tag in (A+'defRPr', A+'rPr'):
            b = p.get('b')
            if b is not None: return b in ('1','true')
    return None

rows=[]
for root in sys.argv[1:]:
    for f in sorted(glob.glob(root+'/**/*', recursive=True)):
        if not os.path.isfile(f): continue
        try: z=zipfile.ZipFile(f)
        except Exception: continue
        for n in z.namelist():
            if not re.match(r'.*charts?/chart\d*\.xml$', n): continue
            try: t=ET.fromstring(z.read(n))
            except Exception: continue
            if not t.tag.endswith('chartSpace'): continue
            chart=t.find(C+'chart')
            if chart is None: continue
            gtx = t.find(C+'txPr')
            gsz = size_of(gtx)
            title = chart.find(C+'title')
            tsz, tb = size_of(title), bold_of(title)
            pa = chart.find(C+'plotArea')
            axtitles=[]
            if pa is not None:
                for ax in pa:
                    if not ax.tag.startswith(C) or not ax.tag.endswith('Ax'): continue
                    at = ax.find(C+'title')
                    if at is not None: axtitles.append((size_of(at), bold_of(at)))
            leg = chart.find(C+'legend')
            rows.append((os.path.basename(f), n, gsz, tsz, tb, len(axtitles),
                         sum(1 for s,b in axtitles if s is not None),
                         sum(1 for s,b in axtitles if b is True),
                         sum(1 for s,b in axtitles if b is False),
                         1 if leg is not None else 0))
print("file\tpart\tglobalsz\ttitlesz\ttitleb\tnaxt\taxt_sz\taxt_bold\taxt_notbold\tlegend")
for r in rows: print("\t".join('' if x is None else str(x) for x in r))
