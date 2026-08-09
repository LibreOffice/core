#!/usr/bin/env python3
"""Every corpus chart's plot rectangle: ours against the one LibreOffice states.

One row per chart object, per edge error, and a per-document and whole-track total. The
rectangle is read from our own gridlines, so a chart that draws neither a grid nor a value
axis with three ticks is skipped and said so — the point of the model side is that it is
never the missing half.
"""
import sys, os, re, glob, subprocess, collections
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from region import regions
from inkrect import rect, _dump

SP = os.path.dirname(os.path.abspath(__file__))
ODP = os.path.join(SP, "corpus-odp")
OURS = os.path.join(SP, "sweep-base", "ours")
REF = ("/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/"
       "scratchpad/slides-3e093938/sweep-base/ref")

def heights(pdf):
    out = subprocess.run(["pdfinfo", "-f", "1", "-l", "9999", pdf],
                         capture_output=True, text=True).stdout
    h = {}
    for m in re.finditer(r"Page\s+(\d+)\s+size:\s+([\d.]+) x ([\d.]+)", out):
        h[int(m.group(1))] = float(m.group(3))
    if not h:
        m = re.search(r"Page size:\s+([\d.]+) x ([\d.]+)", out)
        if m: h[0] = float(m.group(2))
    return h

def main():
    verbose = "-v" in sys.argv
    grand = collections.Counter(); rows = 0; skipped = 0
    per = {}
    for odp in sorted(glob.glob(os.path.join(ODP, "*.odp"))):
        stem = os.path.basename(odp)[:-4]
        ours = os.path.join(OURS, stem + "__pptx.pdf")
        ref = os.path.join(REF, stem + "__pptx.pdf")
        if not (os.path.exists(ours) and os.path.exists(ref)): 
            print("missing pdf for", stem); continue
        H = heights(ref)
        tot = collections.Counter(); n = 0; sk = 0; cache = {}
        for r in regions(odp):
            p = r["page"]
            ph = H.get(p) or H.get(0)
            x, y, w, h = r["region"]
            model = (x, ph-(y+h), x+w, ph-y)
            fx, fy, fw, fh = r["frame"]
            box = (fx, ph - (fy + fh), fx + fw, ph - fy)
            a = rect(ours, p, within=box, dump=cache.setdefault((ours, p), _dump(ours, p)))
            if a is None: sk += 1; skipped += 1; continue
            n += 1; rows += 1
            for i, name in enumerate(("left","bottom","right","top")):
                d = a[i]-model[i]
                tot[name] += abs(d); grand[name] += abs(d)
                if verbose:
                    print(f'  {stem[:34]:<34} p{p:<3} {name:>6} model {model[i]:8.2f} ours {a[i]:8.2f} d {d:7.2f}')
        per[stem] = (n, sk, sum(tot.values()))
        if n: print(f'{stem[:44]:<44} charts {n:>2} skipped {sk} '
                    f'|err| L{tot["left"]:7.2f} B{tot["bottom"]:7.2f} '
                    f'R{tot["right"]:7.2f} T{tot["top"]:7.2f}  total {sum(tot.values()):8.2f}')
    print()
    print(f'{rows} charts measured, {skipped} skipped (no gridline rectangle in ours)')
    print(f'total |error|  L {grand["left"]:.2f}  B {grand["bottom"]:.2f}  '
          f'R {grand["right"]:.2f}  T {grand["top"]:.2f}   all {sum(grand.values()):.2f}')
    if rows: print(f'mean per edge {sum(grand.values())/(4*rows):.2f} pt')

main()
