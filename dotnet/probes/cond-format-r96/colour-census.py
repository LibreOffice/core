#!/usr/bin/env python3
"""Count the fill colours a PDF's content streams set, per document.

Why this rather than a reading of the page: a conditional format's whole effect is a colour and a
strikethrough, so *how many spans are set to the rule's colour* is a number both sides can be asked
for with no tolerance, no rasteriser and no eye. It is the corroboration `dotnet/CLAUDE.md` asks
for in a container with no uncontaminated reviewer.

**It counts operators, not area.** LibreOffice coalesces adjacent same-coloured cell fills into
one rectangle where this tree emits one per cell, so a fill count can differ by a factor while the
painted area agrees — the `901 grey fills against one` artefact `sheet-shapefill-r92` recorded.
Read a fill row as "is it emitted at all" and take the area question to `pdf-image-diff.py`.

Usage: colour-census.py <pdf...> [--colour RRGGBB ...]
"""
import collections
import re
import sys

import pymupdf

OPERATOR = re.compile(rb"([\d.]+) ([\d.]+) ([\d.]+) (rg|sc|scn)\b")


def census(path):
    counts = collections.Counter()
    document = pymupdf.open(path)
    try:
        for page in document:
            for match in OPERATOR.finditer(page.read_contents()):
                counts[tuple(round(float(match.group(i)), 2) for i in (1, 2, 3))] += 1
    finally:
        document.close()
    return counts


def main():
    args = sys.argv[1:]
    wanted = []
    while "--colour" in args:
        at = args.index("--colour")
        rgb = args[at + 1]
        wanted.append(tuple(round(int(rgb[i:i + 2], 16) / 255.0, 2) for i in (0, 2, 4)))
        del args[at:at + 2]

    for path in args:
        counts = census(path)
        if wanted:
            shown = "\t".join(str(counts.get(colour, 0)) for colour in wanted)
        else:
            shown = "\t".join(f"{c}={n}" for c, n in counts.most_common(12))
        print(f"{path}\t{shown}")


if __name__ == "__main__":
    main()
