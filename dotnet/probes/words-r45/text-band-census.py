#!/usr/bin/env python3
"""How tall is the band of text each renderer puts on a page, and where does it start?

A document that paginates one page short or one page long is fitting a different amount of
text between the same two margins, and there are only two ways to do that: a taller band, or a
tighter line pitch inside it.  This measures the first directly, from the rendered PDFs, and it
is the cheapest thing that separates the two.

For each page it reads every word's box out of `pdftotext -bbox` and reports

    top     the smallest yMin on the page   — where the first mark starts
    bot     the largest  yMax on the page   — where the last mark ends
    band    bot - top

in PDF points from the top of the page.  Comparing ours against the reference page by page
answers "is our text area taller" without any reference to what is in it.

Two properties worth stating, because a census that does not say what it counted over has
misled this project before:

  * it includes running heads and footers, which is deliberate — a header that is a
    millimetre short moves the body's top and is exactly the defect this is looking for;
  * it can only compare pages that both sides have, so on a document that paginates
    differently only the common prefix means anything, and only the pages *before* the first
    moved break mean anything at all.  Pass `--upto N` to cut it there.
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

WORD = re.compile(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">')
PAGE = re.compile(r'<page width="([\d.]+)" height="([\d.]+)">')


def bands(pdf: Path):
    """[(page_height, top, bot)] per page; top/bot None on a page with no words."""
    xml = subprocess.run(["pdftotext", "-bbox", str(pdf), "-"],
                         capture_output=True, text=True).stdout
    out = []
    for chunk in xml.split("<page ")[1:]:
        m = PAGE.search("<page " + chunk.split(">")[0] + ">")
        h = float(m.group(2)) if m else 0.0
        ys = [(float(a), float(b)) for a, _, b, _ in
              [(g[1], g[0], g[3], g[2]) for g in WORD.findall("<page " + chunk)]]
        if not ys:
            out.append((h, None, None))
        else:
            out.append((h, min(y for y, _ in ys), max(y2 for _, y2 in ys)))
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("ours")
    ap.add_argument("ref")
    ap.add_argument("--upto", type=int, default=0)
    ap.add_argument("--quiet", action="store_true")
    args = ap.parse_args()

    o, r = bands(Path(args.ours)), bands(Path(args.ref))
    n = min(len(o), len(r))
    if args.upto:
        n = min(n, args.upto)
    dtop = dbot = dband = 0.0
    cnt = 0
    for i in range(n):
        (ho, to, bo), (hr, tr, br) = o[i], r[i]
        if to is None or tr is None:
            continue
        cnt += 1
        dtop += to - tr
        dbot += bo - br
        dband += (bo - to) - (br - tr)
        if not args.quiet:
            print(f"page {i+1:>4}  h {ho:7.2f}/{hr:7.2f}  top {to:7.2f}/{tr:7.2f} "
                  f"({to-tr:+6.2f})  bot {bo:7.2f}/{br:7.2f} ({bo-br:+6.2f})  "
                  f"band {bo-to:7.2f}/{br-tr:7.2f} ({(bo-to)-(br-tr):+6.2f})")
    if cnt:
        print(f"mean over {cnt} pages: top {dtop/cnt:+.2f}  bot {dbot/cnt:+.2f} "
              f"band {dband/cnt:+.2f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
