#!/usr/bin/env python3
"""Score a rendering against the reference on the *axes* rather than on the glyph count.

`dotnet/CLAUDE.md`'s sixth confound makes the gate's glyph column unusable on this
cluster: ten of the fourteen `chartset` failures recalculate `TODAY()`, so their
deltas move on their own every day the gate is run.  What does not move is which
tick labels a value axis draws, and that is what this round changed.

For each document it reads every vertical run of numeric spans that reads as a
value axis (`axisticks.axes`) out of both PDFs and compares the *label sets* page
by page, matched on the axis' x position.  A document is `same` when every axis
found on one side has a partner on the other drawing the same labels.

    score-axes.py --ours DIR --ref DIR [--only SUBSTRING]
"""
import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from axisticks import axes                                    # noqa: E402


def signature(path):
    """(page, x, labels) for every axis-shaped run in the PDF."""
    return [(a["page"], a["x"], tuple(a["labels"])) for a in axes(path)]


def compare(ours_pdf, ref_pdf, near=40.0):
    """Pair the two sides' axes by page and nearest x, then compare the labels.

    Nearest-x rather than a bucket, because the two renderings' charts are not
    the same width to the point: on `040_Blood_pressure_tracker` the secondary
    axis is at 534.74 here and 543.09 in the reference — the same axis, 1.2%
    apart, which a grid keyed on 40 pt puts either side of a boundary and reads
    as two axes each missing from the other side.
    """
    ours, ref = signature(ours_pdf), signature(ref_pdf)
    if not ours and not ref:
        return "none", []

    remaining = list(ref)
    diffs = []
    for page, x, labels in ours:
        candidates = [r for r in remaining if r[0] == page and abs(r[1] - x) <= near]
        if not candidates:
            diffs.append(((page, round(x, 1)), labels, None))
            continue
        best = min(candidates, key=lambda r: abs(r[1] - x))
        remaining.remove(best)
        if best[2] != labels:
            diffs.append(((page, round(x, 1)), labels, best[2]))
    for page, x, labels in remaining:
        diffs.append(((page, round(x, 1)), None, labels))
    return ("same" if not diffs else "differ"), diffs


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ours", required=True)
    ap.add_argument("--ref", required=True)
    ap.add_argument("--only", default="")
    ap.add_argument("--quiet", action="store_true")
    args = ap.parse_args()

    ours_dir, ref_dir = Path(args.ours), Path(args.ref)
    tally = {}
    for pdf in sorted(ours_dir.glob("*.pdf")):
        if args.only and args.only not in pdf.name:
            continue
        ref_pdf = ref_dir / pdf.name
        if not ref_pdf.is_file():
            tally["no-ref"] = tally.get("no-ref", 0) + 1
            continue
        try:
            state, diffs = compare(str(pdf), str(ref_pdf))
        except Exception as exc:                                # noqa: BLE001
            print(f"ERROR\t{pdf.name}\t{exc}")
            tally["error"] = tally.get("error", 0) + 1
            continue
        tally[state] = tally.get(state, 0) + 1
        if state == "differ" and not args.quiet:
            print(f"DIFFER\t{pdf.name}")
            for key, a, b in diffs:
                print(f"    p{key[0]} x~{key[1]}\n      ours {a}\n      ref  {b}")
    print("TOTAL " + "  ".join(f"{k} {v}" for k, v in sorted(tally.items())))


if __name__ == "__main__":
    main()
