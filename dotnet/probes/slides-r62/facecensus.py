#!/usr/bin/env python3
"""Which typeface each text run on a rendered page is set in, ours against the reference's.

The legend question needs the *face* a run is drawn in, not its pen: 26.2.4.2 draws
001_advanced_powerpoint_bar's axis labels in LiberationSans (the axes' stated Arial) and its
legend in Carlito (the theme's Calibri, because c:legend states no c:txPr), and we draw both in
LiberationSans.  Resolves /Fn through the page's own resource dictionary to the embedded
/BaseFont name, strips the six-letter subset tag, and reports a (face, size) histogram plus the
runs in a named x/y window.
"""
import re, sys
sys.path.insert(0, '/c/sandbox/workdir/wt-slides-r50/dotnet/research/probes/slides-r15')
sys.path.insert(0, '/c/sandbox/workdir/scratch-r62-slides')
import pdfops
from tfpos import runs

NUMOBJ = re.compile(rb'(\d+)\s+\d+\s+R')


def fontmap(path, idx):
    d = open(path, 'rb').read()
    objs = pdfops.objects(d)
    page = objs[pdfops.pages(d, objs)[idx]]
    res = page
    m = re.search(rb'/Resources\s+(\d+)\s+\d+\s+R', page)
    if m:
        res = objs[int(m.group(1))]
    fm = re.search(rb'/Font\s*<<(.*?)>>', res, re.S)
    if fm:
        table = fm.group(1)
    else:
        fr = re.search(rb'/Font\s+(\d+)\s+\d+\s+R', res)
        table = objs[int(fr.group(1))] if fr else b''
    out = {}
    for name, num in re.findall(rb'/(F\d+)\s+(\d+)\s+\d+\s+R', table):
        o = objs[int(num)]
        bf = re.search(rb'/BaseFont\s*/([^\s/>\]]+)', o)
        if not bf:
            df = re.search(rb'/DescendantFonts\s*\[?\s*(\d+)', o)
            if df:
                bf = re.search(rb'/BaseFont\s*/([^\s/>\]]+)', objs[int(df.group(1))])
        face = bf.group(1).decode() if bf else '?'
        out['/' + name.decode()] = re.sub(r'^[A-Z]{6}\+', '', face)
    return out


def faces(path, idx):
    from pg import page_stream
    fm = fontmap(path, idx)
    return [(x, y, fm.get(f, str(f)), round(s, 2), t) for x, y, f, s, t in runs(page_stream(path, idx))]


if __name__ == '__main__':
    import collections
    sys.path.insert(0, '/c/sandbox/workdir/scratch-r56-slides')
    ours, ref, name, page = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
    for label, root in (("ours", ours), ("ref", ref)):
        rs = faces(f"{root}/{name}.pdf", page - 1)
        tal = collections.Counter((f, s) for _, _, f, s, _ in rs)
        print(f"== {label}: {len(rs)} runs")
        for (f, s), n in sorted(tal.items(), key=lambda kv: (-kv[1], kv[0])):
            print(f"   {n:4d}  {f:28s} {s}")
