#!/usr/bin/env python3
"""Re-classify a divergence TSV's first pages with a different `first-divergence.py`.

`divergence-from-pdfs.py` does two things: it finds each document's first materially divergent
page with the image diff, and it classifies that page with the operator diff. Round 43 changes only
the second — `pdf-ops.py`'s pairing — so re-running the first would cost twenty minutes to
reproduce numbers that cannot have moved, and would invite the two columns being measured on
different runs.

This reads a finished TSV, keeps the `first_page` column exactly, and re-runs the classification.

    FIRST_DIVERGENCE=/abs/worktree/.claude/…/first-divergence.py \\
        reclassify.py --in div-published.tsv --ours-dir OUT/ours --ref-dir OUT/ref \\
                      --out div-cleaned.tsv
"""
from __future__ import annotations

import argparse
import importlib.util
import os
import sys
from pathlib import Path

SPEC = importlib.util.spec_from_file_location(
    "firstdiv",
    os.environ.get(
        "FIRST_DIVERGENCE",
        "/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/first-divergence.py"))
fd = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(fd)


def ident(rel: str) -> str:
    name = rel.rsplit("/", 1)[-1]
    stem, _, ext = name.rpartition(".")
    return f"{stem}__{ext.lower()}"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--in", dest="src", type=Path, required=True)
    ap.add_argument("--ours-dir", type=Path, required=True)
    ap.add_argument("--ref-dir", type=Path, required=True)
    ap.add_argument("--out", type=Path, required=True)
    args = ap.parse_args()

    lines = args.src.read_text().splitlines()
    header = lines[0].split("\t")
    col = {name: i for i, name in enumerate(header)}
    out = ["\t".join(header)]
    for line in lines[1:]:
        if not line.strip():
            continue
        f = line.split("\t")
        page = f[col["first_page"]]
        if not page.isdigit():
            out.append(line)
            continue
        pid = ident(f[0])
        ours, ref = args.ours_dir / f"{pid}.pdf", args.ref_dir / f"{pid}.pdf"
        if not ours.exists() or not ref.exists():
            out.append(line)
            continue
        counts, oo, orr, dom, gd, br = fd.classify(ours, ref, int(page))
        f[col["dominant"]] = dom
        f[col["gdelta"]] = str(gd)
        f[col["only_ours"]] = str(oo)
        f[col["only_ref"]] = str(orr)
        f[col["blank_ref"]] = str(br)
        f[col["counts"]] = ",".join(f"{k}={v}" for k, v in counts.items() if v)
        out.append("\t".join(f))
        print(f"  {f[0]} p{page} {dom} {f[col['counts']]}", flush=True)
    args.out.write_text("\n".join(out) + "\n")
    print(f"wrote {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
