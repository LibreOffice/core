#!/usr/bin/env python3
"""Build `box-note-anatomy.py --list` jobs from a divergence TSV.

One row per document whose first divergent page carries any box-shaped note — `box=` under the
published matcher, `box=` or `hairline=` under round 43's, since the note was split in two.

    box-jobs.py div.tsv OUT/ours OUT/ref > jobs.tsv
"""
from __future__ import annotations

import sys
from pathlib import Path


def ident(rel: str) -> str:
    name = rel.rsplit("/", 1)[-1]
    stem, _, ext = name.rpartition(".")
    return f"{stem}__{ext.lower()}"


def main() -> int:
    src, ours, ref = Path(sys.argv[1]), Path(sys.argv[2]), Path(sys.argv[3])
    lines = src.read_text().splitlines()
    col = {name: i for i, name in enumerate(lines[0].split("\t"))}
    for line in lines[1:]:
        f = line.split("\t")
        if len(f) <= col["counts"]:
            continue
        counts = f[col["counts"]]
        if "box=" not in counts and "hairline=" not in counts:
            continue
        page = f[col["first_page"]]
        if not page.isdigit():
            continue
        pid = ident(f[0])
        o, r = ours / f"{pid}.pdf", ref / f"{pid}.pdf"
        if o.exists() and r.exists():
            print(f"{f[0]}\t{o}\t{r}\t{page}\t{f[col['verdict']]}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
