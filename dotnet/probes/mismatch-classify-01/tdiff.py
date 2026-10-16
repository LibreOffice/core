#!/usr/bin/env python3
"""For each document, the tokens present in one rendering and not the other.

Reads the layout text dumps under txt/ours and txt/ref, compares them as
multisets of whitespace-separated tokens, and prints the top surplus and
deficit tokens.  A cheap way to see *what* text a rendering is missing without
looking at a page, which is the step before looking at one.
"""
import collections
import pathlib
import sys

here = pathlib.Path(__file__).parent
want = sys.argv[1] if len(sys.argv) > 1 else ""

for p in sorted((here / "txt" / "ours").glob("*.txt")):
    if want and want not in p.name:
        continue
    o = collections.Counter((here / "txt" / "ours" / p.name).read_text(errors="replace").split())
    r = collections.Counter((here / "txt" / "ref" / p.name).read_text(errors="replace").split())
    surplus = o - r
    deficit = r - o
    og = sum(len([c for c in w if c.isalnum()]) * n for w, n in o.items())
    rg = sum(len([c for c in w if c.isalnum()]) * n for w, n in r.items())
    print(f"### {p.stem}   glyphs ours={og} ref={rg} delta={og-rg:+d}")
    print(f"  ours-only ({sum(surplus.values())} tokens): "
          + " | ".join(f"{w}×{n}" if n > 1 else w for w, n in surplus.most_common(18)))
    print(f"  ref-only  ({sum(deficit.values())} tokens): "
          + " | ".join(f"{w}×{n}" if n > 1 else w for w, n in deficit.most_common(18)))
    print()
