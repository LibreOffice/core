#!/usr/bin/env python3
"""Apply `batch-check.sh`'s verdict rule to a rendered pair of directories.

The reference half of the gate costs about ten times what our half costs and cannot be
touched by a diff confined to `dotnet/src`, so a round that wants a *before* and an *after*
verdict should render the reference once and score two of our own sweeps against it. This is
that scorer, and it is deliberately a transcription of the shell rather than an improvement
on it:

* page count must be equal;
* the **alphanumeric character** count -- column 9, `glyphs`, not column 4, `words` -- must be
  within `max(2%, 15)` of the reference's;
* no font may be unembedded.

`words` is a letter-or-digit *token* count kept only so an older scoreboard reconciles. The
band has applied to `glyphs` since 2026-09-05 and reading the wrong column moves verdicts that
did not move.

Usage:  python3 score-gate.py <ref-dir> <ours-dir> [<ours-dir> ...]
"""
import re, subprocess, sys
from pathlib import Path

ALNUM = re.compile(r"[^\W_]", re.UNICODE)


def pages(pdf):
    out = subprocess.run(["pdfinfo", str(pdf)], capture_output=True, text=True).stdout
    for line in out.splitlines():
        if line.startswith("Pages:"):
            return int(line.split()[1])
    return None


def glyphs(pdf):
    text = subprocess.run(["pdftotext", str(pdf), "-"], capture_output=True, text=True).stdout
    return len(ALNUM.findall(text))


def unembedded(pdf):
    out = subprocess.run(["pdffonts", str(pdf)], capture_output=True, text=True).stdout
    rows = out.splitlines()[2:]
    return sum(1 for r in rows if len(r.split()) >= 8 and r.split()[-5] == "no")


def verdict(ref, ours):
    if not ours.exists():
        return "ours-failed", None, None
    op, rp = pages(ours), pages(ref)
    og, rg = glyphs(ours), glyphs(ref)
    bad = []
    if op != rp:
        bad.append("pages")
    if rg > 0:
        if abs(og - rg) > rg * 0.02 and abs(og - rg) > 15:
            bad.append("words")
    elif og > 15:
        bad.append("words")
    if unembedded(ours):
        bad.append("unembedded")
    return (",".join(bad) or "match"), (op, rp), (og, rg)


def main():
    ref = Path(sys.argv[1])
    banks = [Path(p) for p in sys.argv[2:]]
    ids = sorted(p.stem for p in ref.glob("*.pdf"))
    print(f"reference {ref}: {len(ids)} documents")
    rows = {}
    for i in ids:
        rows[i] = [verdict(ref / f"{i}.pdf", b / f"{i}.pdf") for b in banks]
    for n, b in enumerate(banks):
        m = sum(1 for i in ids if rows[i][n][0] == "match")
        print(f"{b}: MATCH {m} of {len(ids)}")
    if len(banks) == 2:
        moved = [i for i in ids if rows[i][0][0] != rows[i][1][0]]
        print(f"\nverdicts that moved: {len(moved)}")
        for i in moved:
            a, b = rows[i][0], rows[i][1]
            print(f"  {i}\n    before {a[0]:12s} pages {a[1]} glyphs {a[2]}"
                  f"\n    after  {b[0]:12s} pages {b[1]} glyphs {b[2]}")


main()
