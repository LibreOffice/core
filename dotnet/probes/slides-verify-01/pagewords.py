#!/usr/bin/env python3
"""Per-page word / character accounting for a pair of PDFs.

    pagewords.py ours.pdf ref.pdf [--chars]

Emits one row per page: gate-words ours/ref, delta, raw words, and (with --chars)
the whitespace-stripped character-multiset difference in each direction plus the
one-character token counts.
"""
import subprocess, sys, collections, argparse

def pages(pdf):
    out = subprocess.run(["pdfinfo", pdf], capture_output=True, text=True).stdout
    for line in out.splitlines():
        if line.startswith("Pages:"):
            return int(line.split()[1])
    return 0

def text(pdf, page):
    r = subprocess.run(["pdftotext", "-f", str(page), "-l", str(page), pdf, "-"],
                       capture_output=True)
    return r.stdout.decode("utf-8", "replace")

def gate(t):
    toks = t.split()
    return sum(1 for w in toks if any(c.isalnum() for c in w)), len(toks), toks

ap = argparse.ArgumentParser()
ap.add_argument("ours"); ap.add_argument("ref")
ap.add_argument("--chars", action="store_true")
a = ap.parse_args()

po, pr = pages(a.ours), pages(a.ref)
print(f"# pages ours={po} ref={pr}")
n = min(po, pr)
to_, tr_ = 0, 0
rows = []
allo = collections.Counter(); allr = collections.Counter()
for p in range(1, n + 1):
    o, r = text(a.ours, p), text(a.ref, p)
    go, ro, toko = gate(o)
    gr, rr, tokr = gate(r)
    to_ += go; tr_ += gr
    co = collections.Counter(c for c in o if not c.isspace())
    cr = collections.Counter(c for c in r if not c.isspace())
    allo += co; allr += cr
    oonly = co - cr; ronly = cr - co
    s1o = sum(1 for w in toko if len(w) == 1)
    s1r = sum(1 for w in tokr if len(w) == 1)
    rows.append((p, go, gr, go - gr, ro, rr, sum(co.values()), sum(cr.values()),
                 sum(oonly.values()), sum(ronly.values()), s1o, s1r,
                 "".join(sorted(oonly.elements()))[:60],
                 "".join(sorted(ronly.elements()))[:60]))

print("page\tow\trw\tdw\toraw\trraw\tochar\trchar\tconly_o\tconly_r\t1tok_o\t1tok_r"
      + ("\toonly\tronly" if a.chars else ""))
for row in rows:
    base = "\t".join(str(x) for x in row[:12])
    print(base + ("\t" + row[12] + "\t" + row[13] if a.chars else ""))
print(f"# TOTAL gate ours={to_} ref={tr_} delta={to_-tr_}")
print(f"# TOTAL chars ours={sum(allo.values())} ref={sum(allr.values())} "
      f"delta={sum(allo.values())-sum(allr.values())}")
oo = allo - allr; rr2 = allr - allo
print(f"# chars ours-only={sum(oo.values())} ref-only={sum(rr2.values())}")
print("# ours-only:", "".join(f"{c}x{n} " for c, n in oo.most_common(25)))
print("# ref-only :", "".join(f"{c}x{n} " for c, n in rr2.most_common(25)))
