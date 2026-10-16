#!/usr/bin/env python3
"""Corpus chart parts whose series draw markers, and what size each states.

`TypeGroupConverter::convertMarker` (`typegroupconverter.cxx:652-654`) makes the symbol
`convertPointToMm100(c:marker/c:size)`, defaulting to `mnMarkerSize(5)`
(`seriesmodel.cxx:118`).  We draw `labelSize x 0.7`, which is 7.00 pt on the 10 pt labels
nearly every corpus chart uses and is a transcription of chart2's *unset* 250x250 default --
right for ODF and binary charts, wrong for every OOXML one.

What this census CANNOT see:
  * whether the series actually draws a marker at all -- `c:symbol val="none"`, a
    `seriesFrameFormat` chart type (bar/pie/area, where `convertMarker` returns at once), and
    a line chart whose `c:marker val="0"` suppresses them are all excluded here by string
    match only, so this over-counts;
  * an ODF or binary chart, which must NOT move: it has no `c:marker` and keeps the 250;
  * a marker whose size is stated on a single `c:dPt` rather than on the series.
"""
import collections, os, re, sys, zipfile
CORPUS = "/c/sandbox/workdir/sample-files"
FRAME = ('barChart', 'bar3DChart', 'pieChart', 'pie3DChart', 'doughnutChart',
         'areaChart', 'area3DChart', 'ofPieChart', 'surfaceChart', 'surface3DChart')

def parts(path):
    try: z = zipfile.ZipFile(path)
    except Exception: return
    for n in z.namelist():
        if re.search(r'charts?/chart\d*\.xml$', n) or n.endswith('/chart.xml'):
            try: yield n, z.read(n).decode('utf-8', 'replace')
            except Exception: pass

if __name__ == '__main__':
    want = sys.argv[1] if len(sys.argv) > 1 else None
    docs = collections.Counter()
    sizes = collections.Counter()
    for f, b, p, ext, *_ in (l.split('\t') for l in open(f"{CORPUS}/MANIFEST.tsv")
                             if not l.startswith('family\t')):
        if want and f != want: continue
        for name, xml in parts(os.path.join(CORPUS, p)):
            if not any(f'<c:{k}>' in xml for k in
                       ('lineChart', 'line3DChart', 'scatterChart', 'radarChart', 'stockChart')):
                continue
            if '<c:symbol val="none"/>' in xml and '<c:symbol' not in xml.replace('<c:symbol val="none"/>', ''):
                continue
            docs[(f, p)] += 1
            for m in re.finditer(r'<c:marker>(.*?)</c:marker>', xml, re.S):
                blk = m.group(1)
                if '<c:symbol val="none"' in blk: continue
                s = re.search(r'<c:size val="(\d+)"', blk)
                sizes[s.group(1) if s else 'unstated(5)'] += 1
    print(f"{len(docs)} documents hold a marker-capable OOXML chart part")
    print("stated c:size over their series markers:", dict(sizes))
    for (f, p), n in sorted(docs.items()):
        print(f"  {f:7s} {n} parts  {p}")
    print("by family:", dict(collections.Counter(f for f, p in docs)))
