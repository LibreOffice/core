"""Arbitrate H1 (poppler 26.01.0 tokenises differently) vs H2 (the renderers emit more text).

pdfminer.six is an independent extractor. Calibrate it on the documents whose stored word count
did NOT move between containers, then read it on the movers: if pdfminer sits at the OLD stored
value on the movers while poppler 26 sits high, the PDF text did not change and the tool did.
Run over the small documents only, for cost; selection is by page count, not by outcome.
"""
import sys, os, io, subprocess, contextlib
sys.path.insert(0, "/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/words2")
from decompose import *
from pdfminer.high_level import extract_text

REF = "/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words"
def i(x):
    try: return int(x)
    except: return 0
def idof(k):
    b = os.path.basename(k); s, e = b.rsplit(".", 1); return f"{s}__{e.lower()}.pdf"

keys = [k for k in sorted(ref_F) if i(ref_F[k][0]) <= 3]
movers    = [k for k in keys if i(ref_F[k][1]) != i(ref_24[k][1])]
nonmovers = [k for k in keys if i(ref_F[k][1]) == i(ref_24[k][1])]
print(f"{len(keys)} documents of <=3 pages: {len(movers)} movers, {len(nonmovers)} non-movers\n")

def measure(k):
    p = f"{REF}/{idof(k)}"
    pop = len(subprocess.run(["pdftotext", p, "-"], capture_output=True).stdout.decode("utf-8","replace").split())
    with contextlib.redirect_stderr(io.StringIO()):
        pm = len(extract_text(p).split())
    return pop, pm

print(f"{'document':38s} {'old':>7s} {'poppler26':>10s} {'pdfminer':>9s}   pdfminer nearer")
tot = {"old": 0, "new": 0, "tie": 0}
for label, sel in (("NON-MOVERS", nonmovers[:14]), ("MOVERS", movers[:18])):
    print(f"-- {label}")
    for k in sel:
        old = i(ref_24[k][1]); new = i(ref_F[k][1])
        pop, pm = measure(k)
        if old == new: near = "n/a"
        else:
            near = "OLD" if abs(pm-old) < abs(pm-new) else ("NEW" if abs(pm-new) < abs(pm-old) else "tie")
            tot["old" if near=="OLD" else "new" if near=="NEW" else "tie"] += 1
        print(f"{os.path.basename(k)[:38]:38s} {old:7d} {pop:10d} {pm:9d}   {near}")
print(f"\nmovers where pdfminer is nearer the OLD stored value: {tot['old']}, nearer the NEW: {tot['new']}, tie: {tot['tie']}")
