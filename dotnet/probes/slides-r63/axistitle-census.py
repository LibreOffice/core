#!/usr/bin/env python3
"""Corpus documents whose chart parts state a title on the axis that runs along the bottom.

`changePositionOfAxisTitle`'s ALIGN_BOTTOM arm centres that title on the diagram-plus-axes
rectangle and puts it `pageHeight x 2%` below it; we centre it on the inner plot rectangle and
put it flush against the reserved band.  Both terms reach every chart that draws one.

What this census CANNOT see, stated because an under-reaching census conceals itself:
  * a title stated in a `c:layout` with a manual position -- the reference then skips
    `changePositionOfAxisTitle` entirely (`mbAutoPosTitleX`), so such a chart must NOT move
    and this census counts it as reach;
  * an ODF chart (`chart:title` under `chart:axis`), which this counts separately;
  * a `.ppt`/`.xls` binary chart, which states its titles in records this does not read;
  * which of the two axes ends up at the bottom -- that is the bar/column direction, and a
  * horizontal bar chart puts the VALUE axis' title there.
"""
import collections, os, re, sys, zipfile

CORPUS = "/c/sandbox/workdir/sample-files"

def parts(path):
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return
    for n in z.namelist():
        if re.search(r'charts?/chart\d*\.xml$', n) or n.endswith('/chart.xml'):
            try:
                yield n, z.read(n).decode('utf-8', 'replace')
            except Exception:
                pass

AX = re.compile(r'<c:(catAx|valAx|dateAx)>.*?</c:\1>', re.S)

if __name__ == '__main__':
    fam = sys.argv[1] if len(sys.argv) > 1 else None
    rows = collections.Counter()
    manual = collections.Counter()
    for f, b, p, ext, *_ in (l.split('\t') for l in open(f"{CORPUS}/MANIFEST.tsv") if not l.startswith('family\t')):
        if fam and f != fam: continue
        full = os.path.join(CORPUS, p)
        for name, xml in parts(full):
            for m in AX.finditer(xml):
                blk = m.group(0)
                if '<c:title>' not in blk: continue
                rows[(f, p)] += 1
                if re.search(r'<c:title>.*?<c:layout>\s*<c:manualLayout>', blk, re.S):
                    manual[(f, p)] += 1
    print(f"{len(rows)} documents state an axis title, {sum(rows.values())} axes in all")
    for (f, p), n in sorted(rows.items()):
        print(f"  {f:7s} {n} axis titles ({manual.get((f,p),0)} manual)  {p}")
    byfam = collections.Counter(f for f, p in rows)
    print("by family:", dict(byfam))
