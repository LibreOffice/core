#!/usr/bin/env python3
"""Take a `box` note apart and ask what else could have produced it.

`first-divergence.py` classifies a page by the kind of note that dominates its operator diff.
Round 39 swept all 200 words documents rather than the 46 failures, so the 154 that already match
were the control in the same table — and `box` (`size WxH vs WxH` on a stroke or a fill) came out
as the only class enriched on failures: 8 of 46 against 5 of 154.

A count is not a defect until you know what else spells itself the same way. Two things do:

* **Rectangle corners.** `pdf-ops.py` pairs records by nearest neighbour within three points of an
  anchor which for a stroke is its top-left corner — and the left edge and the top edge of any
  rectangle share that corner exactly. Greedy matching will therefore pair our *vertical* rule
  against the reference's *horizontal* one whenever the true partner is displaced out of the window
  or is taken first. The note then reports the width of one rule against the height of another,
  which is not a measurement of anything.
* **Flattened curves.** A logo or a rounded box arrives as dozens of hairline segments a few points
  long. Two renderers flatten a curve at different sub-point positions, so every segment mismatches
  its neighbour, and one 12 pt square graphic can mint two hundred notes.

Each note is therefore reported with the orientation and the major axis length of both records:

    cross        the two records run in different directions — an artefact of the pairing
    hairline     neither record is longer than --rule-length; a flattened path, not a rule
    rule         same direction and at least one side is rule-scale — a candidate real difference

Usage:  box-note-anatomy.py <ours.pdf> <ref.pdf> --page N [--verbose]
        box-note-anatomy.py --list jobs.tsv          # label, ours.pdf, ref.pdf, page
"""
from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

OPS = Path("/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py")
BOX = re.compile(r"\bsize ([\d.]+)x([\d.]+) vs ([\d.]+)x([\d.]+)")
REC = re.compile(r"^\s*(text|fill|stroke|image)\b")


def axis(w: float, h: float):
    """(orientation, length along the major axis)."""
    return ("h", w) if w >= h else ("v", h)


def notes_for(ours: Path, ref: Path, page: int, rule_length: float):
    out = subprocess.run([sys.executable, str(OPS), "diff", str(ours), str(ref), "--page", str(page)],
                         capture_output=True, text=True,
                         env=dict(os.environ, SOURCE_DATE_EPOCH="1700000000")).stdout
    rows, current, section = [], None, None
    for line in out.splitlines():
        if line.startswith("=== "):
            section = line
            continue
        if REC.match(line):
            current = line.strip()
            continue
        m = BOX.search(line)
        if m and section and section.startswith("=== drawn by both"):
            aw, ah, bw, bh = (float(x) for x in m.groups())
            ao, al = axis(aw, ah)
            bo, bl = axis(bw, bh)
            if ao != bo:
                verdict = "cross"
            elif max(al, bl) < rule_length:
                verdict = "hairline"
            else:
                verdict = "rule"
            rows.append((verdict, ao, al, bo, bl, current))
    return rows


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("ours", nargs="?", type=Path)
    ap.add_argument("ref", nargs="?", type=Path)
    ap.add_argument("--page", type=int, default=1)
    ap.add_argument("--label", default="")
    ap.add_argument("--list", type=Path, help="TSV: label, ours.pdf, ref.pdf, page")
    ap.add_argument("--rule-length", type=float, default=20.0,
                    help="points below which a segment is a flattened path, not a rule")
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args()

    jobs = []
    if args.list:
        for line in args.list.read_text().splitlines():
            if not line.strip() or line.startswith("#"):
                continue
            label, o, r, p = line.split("\t")[:4]
            jobs.append((label, Path(o), Path(r), int(p)))
    else:
        jobs.append((args.label or args.ours.name, args.ours, args.ref, args.page))

    print("label\tpage\tbox_notes\tcross\thairline\trule")
    tot = {"cross": 0, "hairline": 0, "rule": 0}
    for label, o, r, p in jobs:
        rows = notes_for(o, r, p, args.rule_length)
        c = {k: sum(1 for v, *_ in rows if v == k) for k in tot}
        for k in tot:
            tot[k] += c[k]
        print(f"{label}\t{p}\t{len(rows)}\t{c['cross']}\t{c['hairline']}\t{c['rule']}")
        if args.verbose:
            for v, ao, al, bo, bl, rec in rows:
                print(f"    {v:9s} {ao}{al:8.1f} vs {bo}{bl:8.1f}   {rec}")
    print(f"TOTAL\t\t{sum(tot.values())}\t{tot['cross']}\t{tot['hairline']}\t{tot['rule']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
