#!/usr/bin/env python3
"""Split the sheared-glyph disagreement by direction, per page and per document.

`shear-chars.py` reports one signed total per document, which hides that the words track is
simultaneously **short** on some documents and **long** on others: those are two defects, and a
fix for one can make the other worse.  This separates them.

    shear-split.py <ours-dir> <ref-dir>
"""
import glob, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import importlib.util
_s = importlib.util.spec_from_file_location(
    "shearchars", os.path.join(os.path.dirname(os.path.abspath(__file__)), "shear-chars.py"))
sc = importlib.util.module_from_spec(_s)
_s.loader.exec_module(sc)

if __name__ == "__main__":
    ours_dir, ref_dir = sys.argv[1], sys.argv[2]
    short_docs = long_docs = 0
    short_g = long_g = 0
    p_none = p_short = p_long = p_extra = p_agree = 0
    rows = []
    for pdf in sorted(glob.glob(os.path.join(ref_dir, "*.pdf"))):
        ident = os.path.basename(pdf)[:-4]
        ours = os.path.join(ours_dir, ident + ".pdf")
        if not os.path.exists(ours):
            continue
        try:
            a = [sc.count(s) for s in sc.streams(ours)]
            b = [sc.count(s) for s in sc.streams(pdf)]
        except Exception as exc:
            print(f"  !! {ident}: {exc}")
            continue
        for i in range(min(len(a), len(b))):
            if a[i] == b[i]:
                p_agree += 1
            elif b[i] and not a[i]:
                p_none += 1
            elif a[i] and not b[i]:
                p_extra += 1
            elif a[i] < b[i]:
                p_short += 1
            else:
                p_long += 1
        d = sum(b) - sum(a)
        if d > 0:
            short_docs += 1; short_g += d
        elif d < 0:
            long_docs += 1; long_g += -d
        if sum(a) or sum(b):
            rows.append((d, ident, sum(a), sum(b)))
    print(f"documents the reference shears more of : {short_docs}  ({short_g} glyphs)")
    print(f"documents we shear more of            : {long_docs}  ({long_g} glyphs)")
    print()
    print(f"pages where the reference shears and we draw none : {p_none}")
    print(f"pages where we shear and the reference draws none : {p_extra}")
    print(f"pages where both shear, we fewer                  : {p_short}")
    print(f"pages where both shear, we more                   : {p_long}")
    print(f"pages that agree                                  : {p_agree}")
    print()
    print("documents where we draw NO sheared glyph and the reference does:")
    for d, ident, x, y in sorted(rows, key=lambda t: -t[0]):
        if x == 0 and y:
            print(f"  {y:7d}  {ident}")
