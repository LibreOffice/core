#!/usr/bin/env python3
"""Rank the slides track by unaccounted ink, with the two known ceilings subtracted per page.

    ink-ranking.py <sweep-cmp-dir> [alternate-content.tsv] [raster-pages.tsv]

Three things this does that `probes/slides-r22/slides-ink-ranking.py` did not:

* **It takes its inputs as arguments.** Its predecessor hardcoded `sweep-base/cmp` beside
  itself and read `altcontent.tsv` and `inkrank-pages.tsv`, neither of which is the name the
  committed data went in under (`alternate-content-oleobj.tsv`, `raster-pages.tsv`), so it
  could not be run against the tree at all.
* **It reports both ink columns and asserts the invariant between them.** `pdf-image-diff.py`
  prints a signed `ink%` and an unsigned `|ink|%`; a sum of the first can never exceed a sum
  of the second, and a round has already been spent on a "correction" that violated it.
  The predecessor ranked on the signed column while the track's headline quoted the unsigned
  one, so the two numbers in circulation were never the same measurement.
* **It carries residual *per page*.** Ranking on a document's total puts a long deck with a
  uniform sub-half-per-cent drift above a short one with three ruined pages.  Measured:
  `Reporting_responsibilities_matrix__pptx` ranks third on residual with 54.27 and is
  268 pages of 0.20 each, with no mechanism behind it bigger than a page-background fill;
  `Wildlife for REDAC September 11__pptx` ranks second with 54.78 of which 47.07 is three
  pages.  Same rank, entirely different kind of work.
"""
import collections, os, sys


def ceiling_pages(alt_path, raster_path):
    """The pages the track has already established it cannot win, keyed by sweep id."""
    ceiling = collections.defaultdict(set)

    if alt_path and os.path.exists(alt_path):
        for line in open(alt_path):
            parts = line.rstrip("\n").split("\t")
            if len(parts) < 2 or not parts[1].isdigit():
                continue
            name = parts[0].split("/")[-1]
            stem, _, ext = name.rpartition(".")
            ceiling[f"{stem}__{ext.lower()}"].add(int(parts[1]))

    if raster_path and os.path.exists(raster_path):
        for line in open(raster_path):
            parts = line.rstrip("\n").split("\t")
            # doc, page, strict, loose -- strict is the one the track treats as a ceiling
            if len(parts) < 3 or not parts[1].isdigit() or parts[2] != "1":
                continue
            ceiling[parts[0]].add(int(parts[1]))

    return ceiling


def main(cmp_dir, alt_path=None, raster_path=None):
    ceiling = ceiling_pages(alt_path, raster_path)
    rows = []

    for name in sorted(os.listdir(cmp_dir)):
        if not name.endswith(".txt"):
            continue
        doc = name[:-4]
        pages = {}
        for line in open(os.path.join(cmp_dir, name), errors="replace"):
            f = line.rstrip("\n").split("\t")
            # page, diff%, ink%, |ink|%, regions, verdict
            if len(f) >= 6 and f[0].isdigit():
                pages[int(f[0])] = (float(f[2]), float(f[3]), f[5] == "MAJOR")
        if not pages:
            continue

        ceil = ceiling[doc]
        signed = sum(v[0] for v in pages.values())
        total = sum(v[1] for v in pages.values())
        resid = sum(v for p, (_, v, _) in pages.items() if p not in ceil)
        live = len(pages) - len(ceil & set(pages))
        rows.append((doc, len(pages), signed, total, resid,
                     resid / live if live else 0.0,
                     len(ceil & set(pages)),
                     sum(1 for _, _, m in pages.values() if m),
                     sum(1 for p, (_, _, m) in pages.items() if m and p in ceil)))

    rows.sort(key=lambda r: -r[4])
    print("doc\tpages\tink_signed\tink_abs\tresidual\tresidual_per_page"
          "\tceiling_pages\tmajor\tmajor_on_ceiling")
    for r in rows:
        print(f"{r[0]}\t{r[1]}\t{r[2]:.2f}\t{r[3]:.2f}\t{r[4]:.2f}\t{r[5]:.3f}"
              f"\t{r[6]}\t{r[7]}\t{r[8]}")

    signed = sum(r[2] for r in rows)
    total = sum(r[3] for r in rows)
    print(f"# TOTAL |ink| {total:.2f}  signed {signed:.2f}  residual {sum(r[4] for r in rows):.2f}"
          f"  ceiling pages {sum(r[6] for r in rows)}  major {sum(r[7] for r in rows)}"
          f"  major on ceiling {sum(r[8] for r in rows)}", file=sys.stderr)

    # The free correctness check: the unsigned sum cannot be the smaller of the two.
    if abs(signed) > total + 1e-6:
        print(f"# INVARIANT VIOLATED: |signed| {abs(signed):.2f} > |ink| {total:.2f} -- "
              "something is parsing a line it was not meant to see", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))
