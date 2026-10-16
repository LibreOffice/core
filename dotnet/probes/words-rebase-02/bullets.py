"""Decide between two explanations for ours and ref word counts moving in lockstep:
   (H1) pdftotext 26.01.0 tokenises differently from the old container's poppler;
   (H2) both renderers now emit bullet glyphs into the text layer.
H2 predicts the per-document word delta equals the count of bullet-only tokens now present.
H1 predicts no such correspondence. Run over ALL 200, matching documents included.
"""
import sys, os, subprocess, unicodedata
sys.path.insert(0, "/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/words2")
from decompose import *

SPD = "/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/words2"
REF = "/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words"

def is_bullet_tok(t):
    """A whitespace-delimited token carrying no letter and no digit."""
    return not any(c.isalnum() for c in t)

def counts(pdf):
    try:
        txt = subprocess.run(["pdftotext", pdf, "-"], capture_output=True, timeout=300).stdout.decode("utf-8", "replace")
    except Exception:
        return None
    toks = txt.split()
    pua = sum(1 for t in toks if all(0xE000 <= ord(c) <= 0xF8FF for c in t))
    bul = sum(1 for t in toks if is_bullet_tok(t))
    return len(toks), pua, bul

def i(x):
    try: return int(x)
    except: return 0

keys = sorted(ref_F)
rows = []
for k in keys:
    base = os.path.basename(k); stem, ext = base.rsplit(".", 1)
    idf = f"{stem}__{ext.lower()}.pdf"
    o = counts(f"{SPD}/gate/ours/{idf}"); r = counts(f"{REF}/{idf}")
    if o is None or r is None: continue
    dO = i(ours_head_F[k][1]) - i(ours_r47[k][1])
    dR = i(ref_F[k][1]) - i(ref_24[k][1])
    rows.append((k, dO, dR, o, r))

print(f"{len(rows)} documents measured\n")
lock = [x for x in rows if x[1] == x[2] and x[1] != 0]
print(f"lockstep documents (identical nonzero delta): {len(lock)}")
def report(sel, name):
    if not sel: return
    exact_p = sum(1 for _,dO,_,o,r in sel if o[1] == dO)
    exact_b = sum(1 for _,dO,_,o,r in sel if o[2] == dO)
    near_b  = sum(1 for _,dO,_,o,r in sel if abs(o[2]-dO) <= max(3, 0.1*abs(dO)))
    tot_d   = sum(dO for _,dO,_,_,_ in sel)
    tot_b   = sum(o[2] for _,_,_,o,_ in sel)
    tot_p   = sum(o[1] for _,_,_,o,_ in sel)
    print(f"  {name}: n={len(sel)}  sum(delta)={tot_d}  sum(bullet-only toks in OURS)={tot_b}  sum(PUA)={tot_p}")
    print(f"      delta == PUA count exactly: {exact_p};  delta == bullet-only count exactly: {exact_b};  within 10%/3: {near_b}")
report(lock, "lockstep")
report(rows, "all 200")
print()
print("the three pinned documents, in detail (ours toks/PUA/bullet-only ; ref same):")
for name in ("GLACIERBG.ETT.doc", "LENTOBUSSIAIKATAULU", "FAA-High-Level-Org-Chart"):
    for k, dO, dR, o, r in rows:
        if name in k:
            print(f"   {os.path.basename(k)[:40]:42s} delta {dO:+5d}   ours {o}   ref {r}")
