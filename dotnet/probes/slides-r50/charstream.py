#!/usr/bin/env python3
"""The charstream test: strip ALL whitespace from both pdftotext extractions and compare.

Same characters + failing word count  = a tokenisation ceiling; our output may be the better one.
Different characters                  = a real content or layout defect.

Usage: charstream.py <ours.pdf> <ref.pdf>            -> one TSV row
       charstream.py --dir <ourdir> <refdir>         -> a row per id present in both

Columns: id  ourchars refchars  equal  common_prefix  ourwords refwords  jaccard  firstdiff
`equal` is the verdict this test exists to give.  `jaccard` is over the multiset of
characters, so a deck that merely reorders scores 1.000 while one that is missing a block
of text does not -- it separates "same text, tokenised differently" from "text is absent"
even when the streams are not byte-equal.
"""
import subprocess, sys, pathlib, unicodedata
from collections import Counter

def stream(pdf):
    t = subprocess.run(["pdftotext", str(pdf), "-"], capture_output=True, timeout=300)
    s = t.stdout.decode("utf-8", "replace")
    return "".join(s.split())

def words(pdf):
    t = subprocess.run(["pdftotext", str(pdf), "-"], capture_output=True, timeout=300)
    toks = t.stdout.decode("utf-8", "replace").split()
    return sum(1 for w in toks if any(c.isalnum() for c in w))

def row(ident, o, r):
    a, b = stream(o), stream(r)
    n = 0
    for x, y in zip(a, b):
        if x != y: break
        n += 1
    ca, cb = Counter(a), Counter(b)
    inter = sum((ca & cb).values()); union = sum((ca | cb).values())
    j = inter / union if union else 1.0
    fd = ""
    if a != b:
        fd = repr(a[n:n+40]) + " | " + repr(b[n:n+40])
    return "\t".join([ident, str(len(a)), str(len(b)), "SAME" if a == b else "DIFF",
                      str(n), str(words(o)), str(words(r)), f"{j:.4f}", fd])

if __name__ == "__main__":
    if sys.argv[1] == "--dir":
        od, rd = pathlib.Path(sys.argv[2]), pathlib.Path(sys.argv[3])
        only = set(sys.argv[4:]) or None
        print("id\tourchars\trefchars\tequal\tprefix\tourwords\trefwords\tjaccard\tfirstdiff")
        for op in sorted(od.glob("*.pdf")):
            rp = rd / op.name
            if not rp.is_file(): continue
            if only and op.stem not in only: continue
            print(row(op.stem, op, rp), flush=True)
    else:
        print(row("doc", pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])))
