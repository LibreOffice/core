#!/usr/bin/env python3
"""Our plot rectangle against LibreOffice's own, per chart, per edge.

Three readings, deliberately: LibreOffice's *model* (chart:coordinate-region out of an
odp export, which is calculateDiagramPositionExcludingAxes), LibreOffice's *ink* (the
gridline rectangle in its reference PDF) and ours (the same read of our PDF). The first
two agreeing is what licenses using the model where no grid is drawn; the third against
the first is the measurement.
"""
import subprocess, sys, os, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from region import regions
from inkrect import rect

def pageheight(pdf, page):
    out = subprocess.run(["pdfinfo", "-f", str(page), "-l", str(page), pdf],
                         capture_output=True, text=True).stdout
    m = re.search(r"Page\s+\d+\s+size:\s+([\d.]+) x ([\d.]+)", out)
    return float(m.group(2)) if m else None

def run(odp, ours, ref, label=""):
    rows = regions(odp)
    print(f"{'chart':>9} {'pg':>3} {'edge':>7} "
          f"{'LOmodel':>9} {'LOink':>9} {'ours':>9} {'ours-model':>11} {'ours-ink':>9}")
    tot = 0.0; n = 0
    for r in rows:
        p = r["page"]
        H = pageheight(ref, p)
        x, y, w, h = r["region"]
        model = (x, H - (y + h), x + w, H - y)          # left, bottom, right, top
        a, b = rect(ours, p), rect(ref, p)
        for i, name in enumerate(("left", "bottom", "right", "top")):
            am = a[i] - model[i] if a else float("nan")
            ai = a[i] - b[i] if a and b else float("nan")
            bm = b[i] - model[i] if b else float("nan")
            print(f'{r["obj"]:>9} {p:>3} {name:>7} {model[i]:>9.2f} '
                  f'{(b[i] if b else float("nan")):>9.2f} {(a[i] if a else float("nan")):>9.2f} '
                  f'{am:>11.2f} {ai:>9.2f}')
            if a: tot += abs(am); n += 1
    if n: print(f"mean |ours-model| over {n} edges: {tot/n:.2f} pt")

if __name__ == "__main__":
    run(*sys.argv[1:4])
