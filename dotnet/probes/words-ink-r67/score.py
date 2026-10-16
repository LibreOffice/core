#!/usr/bin/env python3
"""Score a track against a banked gate, using `batch-check.sh`'s own rule.

    score.py <ours-dir> <gate-dir> <track-prefix>

`ours-dir` holds one `<stem>__<ext>.pdf` per document; `gate-dir` is a banked
`batch-check.sh` run (its `ref/` PDFs and its `parity.tsv`). The reference half is reused
rather than re-rendered — nothing has touched `soffice` — so the only thing that has moved
between the two runs is our binary.

Refuses to print unless every banked path found one of our renderings.
"""
import re, subprocess, sys
from pathlib import Path

OURS, GATE, PREFIX = Path(sys.argv[1]), Path(sys.argv[2]), sys.argv[3]


def run(cmd):
    return subprocess.run(cmd, capture_output=True, text=True, timeout=600).stdout


def counts(pdf):
    """Words and alphanumeric characters, verbatim from `batch-check.sh`'s own `words_of`."""
    b = run(["pdftotext", str(pdf), "-"])
    t = b.split()
    return (sum(1 for w in t if any(c.isalnum() for c in w)),
            sum(1 for c in b if c.isalnum()))


def pages(pdf):
    for l in run(["pdfinfo", str(pdf)]).splitlines():
        if l.startswith("Pages:"):
            return int(l.split()[1])
    return 0


def fonts(pdf):
    rows = [l for l in run(["pdffonts", str(pdf)]).splitlines()[2:] if l.strip()]
    unemb = sum(1 for l in rows if len(l.split()) >= 8 and l.split()[-5] == "no")
    return len(rows), unemb


def verdict(op, rp, og, rg, unemb):
    """`batch-check.sh` of 2026-09-05: page count, then max(2%, 15) ALPHANUMERIC CHARACTERS.
    The band's input moved from tokens to characters and the floor from 3 to 15; the gate at
    2f4709c08 carries the glyph column, so it was scored this way and must be replayed so."""
    v = []
    if op != rp:
        v.append("pages")
    if rg > 0:
        d = abs(og - rg)
        if d > rg * 0.02 and d > 15:
            v.append("words")
    elif og > 15:
        v.append("words")
    if unemb:
        v.append("unembedded")
    return ",".join(v) or "match"


banked = {}
for line in (GATE / "parity.tsv").read_text().splitlines():
    if line.startswith("#") or line.startswith("path\t"):
        continue
    f = line.split("\t")
    if not f[0].startswith(PREFIX):
        continue
    stem = Path(f[0]).stem
    banked[f"{stem}__{f[1]}"] = f

missing = [k for k in banked if not (OURS / f"{k}.pdf").exists()]
if missing:
    sys.exit(f"REFUSING: {len(missing)} banked paths have no rendering: {missing[:5]}")

moved, tally = [], {}
for key, f in sorted(banked.items()):
    o = OURS / f"{key}.pdf"
    r = GATE / "ref" / f"{key}.pdf"
    op, rp = pages(o), (pages(r) if r.exists() else 0)
    ow, og = counts(o)
    rw, rg = counts(r) if r.exists() else (0, 0)
    nf, unemb = fonts(o)
    v = verdict(op, rp, og, rg, unemb) if r.exists() else "ref-failed"
    tally[v] = tally.get(v, 0) + 1
    if v != f[6]:
        moved.append((f[0], f[6], v, f"{op}/{rp}", f[2], f"{og}/{rg}", f[8] if len(f) > 8 else "-"))

print(f"TOTAL {len(banked)}  " + "  ".join(f"{k.upper()} {n}" for k, n in sorted(tally.items())))
print(f"banked: " + "  ".join(
    f"{k.upper()} {n}" for k, n in sorted(
        {f[6]: sum(1 for g in banked.values() if g[6] == f[6]) for f in banked.values()}.items())))
print()
if moved:
    print("verdicts that moved (path, before, after, pages now/gate, words now/gate):")
    for m in moved:
        print(f"  {m[0]}\n      {m[1]} -> {m[2]}   pages {m[3]} (gate {m[4]})   glyphs {m[5]} (gate {m[6]})")
else:
    print("no verdict moved")
