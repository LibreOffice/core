#!/usr/bin/env python3
"""The distinct left edges a page's text is drawn from, clustered, with a count each.

Written because the two instruments this round had to hand both lie about a columned page in a
way that hides exactly the defect being measured:

* `odt-startx-r88/startx.py` merges every span of one baseline and reports the leftmost, so a
  two-column page contributes one x per baseline — the *first* column's — and a wrong second
  column is invisible in it.
* `odt-sectable-r89/xhist.py`'s docstring records the same trap the other way round: a merged
  histogram reads a two-column page as one column.

This merges nothing.  Every text span is a sample of the left edge it starts from, samples
within `tol` are one cluster, and the cluster's count says how much of the page sits there.  A
section indented from the body shows up as a whole cluster in the wrong place, which is the
reading this round needed.

    columnx.py <pdf> [<pdf> ...] [--tol 1.0] [--min 2]
"""
import sys, pymupdf

def clusters(path, tol=1.0, floor=2):
    xs = []
    with pymupdf.open(path) as doc:
        for page in doc:
            for b in page.get_text('dict')['blocks']:
                for l in b.get('lines', []):
                    for s in l['spans']:
                        if s['text'].strip():
                            xs.append(s['origin'][0])
    xs.sort()
    out = []
    for x in xs:
        if out and x - out[-1][-1] <= tol:
            out[-1].append(x)
        else:
            out.append([x])
    return [(sum(c) / len(c), len(c)) for c in out if len(c) >= floor]

def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    tol = 1.0
    floor = 2
    for i, a in enumerate(sys.argv):
        if a == '--tol': tol = float(sys.argv[i + 1])
        if a == '--min': floor = int(sys.argv[i + 1])
    for p in args:
        print(p)
        for x, n in clusters(p, tol, floor):
            print(f"   {x:9.2f}  {n:>4} spans")

main()
