#!/usr/bin/env python3
"""Verdict a rendered track against the banked reference PDFs.

    verdict.py <ours-dir> <ref-dir> [out.tsv]

`batch-check.sh`'s three checks, column for column, but reading the reference off
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/` instead of re-running `soffice` for it — which is
what `dotnet/CLAUDE.md` asks for, and which also removes the reference from the comparison as a
source of variation between two runs of the same measurement.

The band is `d > b*0.02 && d > 3` — an AND, so max(2%, 3) and not their sum. Copied from
`batch-check.sh:195` rather than restated, because a round found a document sitting at exactly
27 against a 25.98 band and the difference decides it.
"""
import os, subprocess, sys, concurrent.futures

OURS, REFS = sys.argv[1], sys.argv[2]
OUT = sys.argv[3] if len(sys.argv) > 3 else None

def words(pdf):
    text = subprocess.run(["pdftotext", pdf, "-"], capture_output=True).stdout
    toks = text.decode("utf-8", "replace").split()
    return sum(1 for w in toks if any(c.isalnum() for c in w)), len(toks)

def pages(pdf):
    out = subprocess.run(["pdfinfo", pdf], capture_output=True, text=True).stdout
    for line in out.splitlines():
        if line.startswith("Pages:"):
            return int(line.split()[1])
    return 0

def unembedded(pdf):
    out = subprocess.run(["pdffonts", pdf], capture_output=True, text=True).stdout.splitlines()[2:]
    n = 0
    for line in out:
        f = line.split()
        if len(f) >= 8 and f[-5] == "no":
            n += 1
    return n

def one(ident):
    o = os.path.join(OURS, ident + ".pdf")
    r = os.path.join(REFS, ident + ".pdf")
    if not os.path.exists(r):
        return ident, "ref-missing", 0, 0, 0, 0
    op, rp = pages(o), pages(r)
    ow, oraw = words(o)
    rw, rraw = words(r)
    un = unembedded(o)
    v = []
    if op != rp:
        v.append("pages")
    d = abs(ow - rw)
    if rw > 0:
        if d > rw * 0.02 and d > 3:
            v.append("words")
    elif ow > 3:
        v.append("words")
    if un:
        v.append("unembedded")
    return ident, ",".join(v) or "match", op, rp, ow, rw

idents = sorted(n[:-4] for n in os.listdir(OURS) if n.endswith(".pdf"))
rows = []
with concurrent.futures.ThreadPoolExecutor(8) as pool:
    rows = list(pool.map(one, idents))

lines = ["ident\tverdict\tours_pages\tref_pages\tours_words\tref_words"]
for ident, v, op, rp, ow, rw in rows:
    lines.append(f"{ident}\t{v}\t{op}\t{rp}\t{ow}\t{rw}")
text = "\n".join(lines)
if OUT:
    open(OUT, "w").write(text + "\n")
print(text)
match = sum(1 for r in rows if r[1] == "match")
print(f"\nTOTAL {len(rows)}  MATCH {match}  MISMATCH {len(rows) - match}")
