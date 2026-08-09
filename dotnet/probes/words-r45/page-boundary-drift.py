#!/usr/bin/env python3
"""Where does a document's *page boundary* first stop agreeing with the reference's?

`first-divergence.py` answers "which page first looks different", which on a document that
paginates differently is the page after the fault as often as the page holding it.  This asks
a narrower question that only makes sense for a document whose text is nearly the same on both
sides: **at which page break do the two renderers first cut the token stream in a different
place, and by how much text**.

Method, and why it is not a per-page word count.  A per-page count `[503, 246, …]` against
`[451, 199, …]` says the pages differ and cannot say whether the cause is on page 1 or was
inherited from it — every later page is displaced by the same break.  So instead:

  * take each side's token stream in reading order, remembering which page each token is on;
  * for our page *k*, take the last `K` tokens of that page as an anchor and find that anchor
    in the reference's stream, searching only a window around the expected position (running
    heads and footers repeat every page, so a global search is ambiguous by construction);
  * `drift[k]` is where that anchor lands in the reference's stream minus where the
    reference's own page *k* ends.  Positive means we put *more* on the page than it did.

The first `k` with |drift| over a floor is the break that moved; everything after it is
consequence.  `endpage[k]`, the reference page our page *k* ends on, says the same thing
categorically.

Run it over the documents that already match before believing anything it says about the ones
that do not — a per-document reading of a page break is exactly the kind of signal that fires
everywhere.  `--corpus` does that: one row per document, `first` empty when every break agrees.
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

TOKEN = re.compile(r"\S+")
ANCHOR = 8          # tokens per anchor; long enough to be locally unique in body text
WINDOW = 600        # reference tokens searched either side of the expected position
FLOOR = 4           # a drift smaller than this is extraction noise, not a moved break


def pages(pdf: Path) -> list[list[str]]:
    """Tokens per page, in reading order."""
    out = subprocess.run(["pdftotext", str(pdf), "-"], capture_output=True, text=True).stdout
    return [TOKEN.findall(p) for p in out.split("\f")[:-1]] or [TOKEN.findall(out)]


def flatten(per_page: list[list[str]]):
    toks: list[str] = []
    ends: list[int] = []          # ends[k] = index one past the last token of page k
    for p in per_page:
        toks.extend(p)
        ends.append(len(toks))
    return toks, ends


def locate(anchor: list[str], ref: list[str], expect: int) -> int | None:
    """Index one past the best occurrence of `anchor` in `ref`, nearest to `expect`."""
    if not anchor:
        return None
    lo, hi = max(0, expect - WINDOW), min(len(ref), expect + WINDOW)
    n = len(anchor)
    best = None
    first = anchor[0]
    for i in range(lo, hi - n + 1):
        if ref[i] != first:
            continue
        if ref[i:i + n] == anchor:
            end = i + n
            if best is None or abs(end - expect) < abs(best - expect):
                best = end
    return best


def analyse(ours_pdf: Path, ref_pdf: Path):
    op, rp = pages(ours_pdf), pages(ref_pdf)
    ot, oe = flatten(op)
    rt, re_ = flatten(rp)
    rows = []
    for k in range(min(len(op), len(rp))):
        anchor = ot[max(0, oe[k] - ANCHOR):oe[k]]
        # A page with almost no text (a picture page, a section filler) has no usable anchor.
        if len(anchor) < ANCHOR:
            rows.append((k + 1, None, None))
            continue
        hit = locate(anchor, rt, re_[k])
        if hit is None:
            rows.append((k + 1, None, None))
            continue
        drift = hit - re_[k]
        endpage = next((j + 1 for j in range(len(re_)) if hit <= re_[j]), len(re_))
        rows.append((k + 1, drift, endpage))
    return rows, len(op), len(rp)


def first_moved(rows):
    for page, drift, _ in rows:
        if drift is not None and abs(drift) >= FLOOR:
            return page, drift
    return None, None


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("ours", nargs="?")
    ap.add_argument("ref", nargs="?")
    ap.add_argument("--corpus", help="rows.tsv from batch-check.sh")
    ap.add_argument("--pdfs", help="the sweep's outdir holding ours/ and ref/")
    ap.add_argument("--out")
    args = ap.parse_args()

    if args.ours:
        rows, no, nr = analyse(Path(args.ours), Path(args.ref))
        print(f"pages {no} vs {nr}")
        for page, drift, endpage in rows:
            print(f"  page {page:>4}  drift {'?' if drift is None else f'{drift:+d}'}"
                  f"  ends on ref page {endpage}")
        page, drift = first_moved(rows)
        print(f"first moved break: page {page} drift {drift}")
        return 0

    out = open(args.out, "w") if args.out else sys.stdout
    for line in Path(args.corpus).read_text().splitlines():
        f = line.split("\t")
        if len(f) < 7:
            continue
        stem = Path(f[0]).stem + "__" + Path(f[0]).suffix.lstrip(".").lower()
        ours = Path(args.pdfs) / "ours" / f"{stem}.pdf"
        ref = Path(args.pdfs) / "ref" / f"{stem}.pdf"
        if not ours.exists() or not ref.exists():
            continue
        rows, no, nr = analyse(ours, ref)
        page, drift = first_moved(rows)
        nomatch = sum(1 for _, d, e in rows if d is not None and abs(d) >= FLOOR)
        print("\t".join([f[0], f[6], f"{no}/{nr}",
                         "" if page is None else str(page),
                         "" if drift is None else f"{drift:+d}",
                         str(nomatch), str(len(rows))]), file=out)
        out.flush()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
