#!/usr/bin/env python3
"""Turn the two rendered probes into the variant table, cell by cell.

Each word is assigned to a column by its **right** edge, because every cell in the sweep is
right-aligned (a number's default) and a `###` that does not fit overhangs to the *left* of
its own column. Assigning by centre or by left edge mis-files exactly the cells the round is
about.
"""
import sys

from cells import words, grid

CM = 72.0 / 2.54
WIDTHS = [0.10, 0.15, 0.20, 0.25, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80,
          0.90, 1.00, 1.20, 1.40, 1.60, 1.80, 2.00, 2.50, 3.00, 4.00]
LEFT = 2.0 * CM                      # LibreOffice's default 2 cm left print margin
VARIANTS = ["general-1", "general-12345", "general-123456789012", "general-1.5",
            "general-neg1", "fixed2-1", "int0-1", "pct-0.5", "date-2022-02-28",
            "string-XX", "shrink-general-12345", "wrap-general-12345", "wrapdate",
            "left-general-12345"]


def bounds():
    xs, x = [], LEFT
    for w in WIDTHS:
        xs.append((x, x + w * CM))
        x += w * CM
    return xs


def read(pdf):
    """(variant, width) -> drawn text, over both pages of the sweep sheet."""
    xs = bounds()
    out = {}
    pages = words(pdf)
    for page, ws in enumerate(pages[:2]):
        rows = [r for y, r in grid(ws) if 70 < y < 300]
        # Rows can split when shrink-to-fit moves a baseline; fold by nearest variant slot.
        for y, r in sorted([(rr[0][1], rr) for rr in rows]):
            slot = min(range(len(VARIANTS)),
                       key=lambda i: abs(y - (78.56 + i * 12.786)))
            for w in r:
                right = w[2]
                for i, (a, b) in enumerate(xs):
                    if a - 0.5 <= right <= b + 0.5:
                        key = (VARIANTS[slot], WIDTHS[i])
                        # `########` is two joined cells; split it back by threes.
                        out.setdefault(key, w[4])
                        break
    return out


def hashes(pdf):
    """Number of ### cells per variant, counted as # characters / 3 — join-proof."""
    xs = bounds()
    counts = {v: 0 for v in VARIANTS}
    for page, ws in enumerate(words(pdf)[:2]):
        for y, r in grid(ws):
            if not (70 < y < 300):
                continue
            slot = min(range(len(VARIANTS)),
                       key=lambda i: abs(y - (78.56 + i * 12.786)))
            for w in r:
                counts[VARIANTS[slot]] += w[4].count("#")
    return {v: c // 3 for v, c in counts.items()}


if __name__ == "__main__":
    a, b = sys.argv[1], sys.argv[2]
    ra, rb = read(a), read(b)
    ha, hb = hashes(a), hashes(b)
    print("%-22s %5s  %-14s %-14s" % ("variant", "width", "reference", "ours"))
    for v in VARIANTS:
        for w in WIDTHS:
            x, y = ra.get((v, w), "-"), rb.get((v, w), "-")
            flag = "" if x == y else "   <-- differs"
            print("%-22s %5.2f  %-14s %-14s%s" % (v, w, x, y, flag))
        print("%-22s %5s  ###x%-11d ###x%-11d%s"
              % (v, "ALL", ha[v], hb[v], "   <-- differs" if ha[v] != hb[v] else ""))
