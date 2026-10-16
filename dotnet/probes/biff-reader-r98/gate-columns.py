#!/usr/bin/env python3
"""The gate's own two columns — page count and alphanumeric characters — for a list of
renderings, ours against the banked reference.

`batch-check.sh` fails a document when the character distance exceeds max(2 %, 15); column 9,
`glyphs`, is the verdict column and the word counts beside it decide nothing.  This applies
that rule to one directory of our renderings so a change's verdict movement can be read
without re-rendering the reference half.

Usage: gate-columns.py <ours-dir> <ref-dir> <identity...>
"""
import pathlib
import sys

import pymupdf


def measure(path):
    doc = pymupdf.open(path)
    text = "".join(page.get_text() for page in doc)
    return doc.page_count, sum(1 for c in text if c.isalnum())


def main():
    ours_dir, ref_dir = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
    names = sys.argv[3:]
    if names == ["-"] or not names:
        names = [line.strip() for line in sys.stdin if line.strip()]

    print("identity\tours_pages\tref_pages\tours_glyphs\tref_glyphs\tverdict")
    for ident in names:
        ours, ref = ours_dir / f"{ident}.pdf", ref_dir / f"{ident}.pdf"
        if not ours.exists() or not ref.exists():
            print(f"{ident}\t-\t-\t-\t-\tmissing")
            continue

        op, og = measure(ours)
        rp, rg = measure(ref)
        distance = abs(og - rg)
        verdict = ("pages" if op != rp
                   else "glyphs" if distance > rg * 0.02 and distance > 15
                   else "match")
        print(f"{ident}\t{op}\t{rp}\t{og}\t{rg}\t{verdict}", flush=True)


if __name__ == "__main__":
    main()
