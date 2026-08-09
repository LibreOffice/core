#!/usr/bin/env python3
"""Where does a document's *page boundary* first stop agreeing with the reference's?

`first-divergence.py` answers "which page first looks different", which on a document that
paginates differently is the page after the fault as often as the page holding it.  This asks a
narrower question that only makes sense for a document whose text is nearly the same on both
sides: **at which page break do the two renderers first cut the token stream in a different
place, and by how much text**.

Method, and why it is not a per-page word count.  A per-page count `[503, 246, …]` against
`[451, 199, …]` says the pages differ and cannot say whether the cause is on page 1 or was
inherited from it — every later page is displaced by the same break.  So instead the two token
streams are aligned once, whole, with `difflib`, giving a monotone map from a position in our
stream to a position in the reference's.  Then for each page *k*

    drift[k] = map(end of our page k) − (end of the reference's page k)

in reference tokens.  Positive means we put **more** on the page than the reference did.  The
first *k* with |drift| over a floor is the break that moved; everything after it is consequence.

Two earlier designs are recorded here because each failed in a way that read as a clean result:

  * **A local anchor** — the last eight tokens of our page, searched for near the expected
    position in the reference — cannot resolve a page whose last tokens are its **footer**,
    because a footer holds the page number and that number differs on every page after the
    pagination diverges.  The unresolved pages were then reported as *aligned*.  Measured on
    `33004.docx`: 47 pages compared, five unresolved, and the single extra page sat inside them,
    so the document was scored "every break agrees" while being a page long.
  * Retrying the anchor further back from the page end fixes that document and introduces its
    own artefact: an anchor that resolves to the wrong instance of a repeated running head
    reports a drift of ±80 tokens on a page whose neighbours both read zero.

A whole-stream alignment has neither failure mode.  It costs seconds on an ordinary document
and 83 s on the largest in this corpus, which is the price of not having to trust a window.

Run it over the documents that already match before believing anything it says about the ones
that do not: `--corpus` does that, one row per document.
"""

from __future__ import annotations

import argparse
import difflib
import re
import subprocess
import sys
from pathlib import Path

TOKEN = re.compile(r"\S+")
FLOOR = 4           # a drift smaller than this is extraction noise, not a moved break
# difflib is quadratic in the bad case.  95 000 tokens takes 83 s, which is the price of the
# method; the corpus' two 700-page outliers hold 285 000 and 298 000 and are not worth an hour
# each.  Skipped rather than approximated, and named in the output so the gap is visible.
MAX_TOKENS = 120_000


def pages(pdf: Path) -> list[list[str]]:
    """Tokens per page, in reading order."""
    out = subprocess.run(["pdftotext", str(pdf), "-"], capture_output=True, text=True).stdout
    parts = out.split("\f")[:-1]
    return [TOKEN.findall(p) for p in parts] if parts else [TOKEN.findall(out)]


def flatten(per_page: list[list[str]]):
    toks: list[str] = []
    ends: list[int] = []          # ends[k] = index one past the last token of page k
    for p in per_page:
        toks.extend(p)
        ends.append(len(toks))
    return toks, ends


def mapper(ours: list[str], ref: list[str]):
    """A monotone map from a position in `ours` to the position in `ref` of the first token
    shared from there on.  Built from difflib's matching blocks, so it is defined everywhere."""
    blocks = difflib.SequenceMatcher(None, ours, ref, autojunk=False).get_matching_blocks()

    def m(x: int) -> int:
        for a, b, size in blocks:
            if x < a:
                return b
            if x < a + size:
                return b + (x - a)
        return len(ref)

    return m


def analyse(ours_pdf: Path, ref_pdf: Path):
    op, rp = pages(ours_pdf), pages(ref_pdf)
    ot, oe = flatten(op)
    rt, re_ = flatten(rp)
    if max(len(ot), len(rt)) > MAX_TOKENS:
        return None, len(op), len(rp)
    m = mapper(ot, rt)
    rows = []
    for k in range(min(len(op), len(rp))):
        hit = m(oe[k])
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
    ap.add_argument("--per-page")
    args = ap.parse_args()

    if args.ours:
        rows, no, nr = analyse(Path(args.ours), Path(args.ref))
        print(f"pages {no} vs {nr}")
        if rows is None:
            print("too large to align")
            return 0
        for page, drift, endpage in rows:
            print(f"  page {page:>4}  drift {drift:+6d}  ends on ref page {endpage}")
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
        if rows is None:
            print("\t".join([f[0], f[6], f"{no}/{nr}", "", "", "", "", "too-large"]), file=out)
            out.flush()
            continue
        page, drift = first_moved(rows)
        moved = sum(1 for _, d, _ in rows if abs(d) >= FLOOR)
        # The categorical signal, and the one that matters for a document a page out: the first
        # of our pages that ends inside a *different* reference page.
        offpage = next((p for p, _, e in rows if e != p), None)
        print("\t".join([f[0], f[6], f"{no}/{nr}",
                         "" if page is None else str(page),
                         "" if drift is None else f"{drift:+d}",
                         str(moved), str(len(rows)),
                         "" if offpage is None else str(offpage)]), file=out)
        out.flush()
        if args.per_page:
            with open(args.per_page, "a") as pp:
                for p, d, e in rows:
                    print(f"{f[0]}\t{p}\t{d}\t{e}", file=pp)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
