#!/usr/bin/env python3
"""How many .ppt paragraphs are 'hard' in the sense svdfppt.cxx:6267-6271 uses?

`PPTParagraphObj::ApplyTo` puts an SvxLineSpacingItem -- and with it
`SvxInterLineSpaceRule::Prop` -- when the paragraph states a line feed OR its FIRST portion
states a hard `PPT_CharAttr_Font`.  At a resolved proportion of exactly 100 the Prop arm of
`impedit3.cxx:1553-1602` does nothing, and the ::Off arm that applies the autofit's
`fSpacingY` is then unreachable.

This counts the record's own two bits, per paragraph, over every .ppt in the corpus:
  * paragraph mask 0x00001000 -- states a line feed
  * first character run's mask 0x00010000 -- states a typeface index

It reads the record, so it cannot see what the MASTER's level resolves the line feed to
(that decides WHICH arm, not whether the item is put), and it cannot see whether the shape
is autofitted or whether its text overflows.  Both are stated in the round's prediction file.

    ppt-hardness-census.py <corpus-root>
"""
import collections, os, struct, sys
import olefile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) + "/../slides-r53")
import importlib.util
_spec = importlib.util.spec_from_file_location(
    "psd", os.path.dirname(os.path.abspath(__file__)) + "/../slides-r53/ppt-style-dump.py")
psd = importlib.util.module_from_spec(_spec)
_argv = sys.argv
sys.argv = ["x", "/dev/null"]
try:
    _spec.loader.exec_module(psd)
except SystemExit:
    pass
sys.argv = _argv

MASK_LINEFEED = 0x00001000
MASK_FONT = 0x00010000


def census(path):
    ole = olefile.OleFileIO(path)
    data = ole.openstream("PowerPoint Document").read()
    n = collections.Counter()
    for body, stop in psd.slides(data):
        text = None
        for ver, inst, rtype, b, e, depth in psd.walk(data, body, stop):
            if rtype == psd.TEXT_CHARS_ATOM:
                text = data[b:e].decode("utf-16-le", "replace")
            elif rtype == psd.TEXT_BYTES_ATOM:
                text = data[b:e].decode("latin-1", "replace")
            elif rtype == psd.STYLE_TEXT_PROP and text is not None:
                paras, pos = psd.read_paras(data[b:e], len(text))
                chars = psd.read_chars(data[b:e], pos, len(text))
                # walk paragraphs, find the character run that CONTAINS each start
                bounds, off = [], 0
                for c in chars:
                    bounds.append((off, off + max(c["count"], 0), c))
                    off += max(c["count"], 0)
                off = 0
                for p in paras:
                    first = None
                    for s, en, c in bounds:
                        if s <= off < en or (off == s):
                            first = c
                            break
                    hard_lf = bool(p["mask"] & MASK_LINEFEED)
                    hard_font = bool(first and (first["mask"] & MASK_FONT))
                    n["paragraphs"] += 1
                    n["hard_lf"] += hard_lf
                    n["hard_font"] += hard_font
                    n["hard_either"] += (hard_lf or hard_font)
                    if hard_lf:
                        n[f"lf={p.get('lineFeed')}"] += 1
                    off += max(p["count"], 0)
    return n


if __name__ == "__main__":
    root = sys.argv[1] if len(sys.argv) > 1 else "/c/sandbox/workdir/sample-files"
    total = collections.Counter()
    docs = soft_docs = 0
    for dirpath, _, names in os.walk(os.path.join(root, "slides")):
        for name in sorted(names):
            if not name.lower().endswith(".ppt"):
                continue
            p = os.path.join(dirpath, name)
            try:
                n = census(p)
            except Exception as exc:
                print(f"  !! {name}: {exc}")
                continue
            docs += 1
            soft = n["paragraphs"] - n["hard_either"]
            if soft:
                soft_docs += 1
            total.update(n)
            print(f"{n['paragraphs']:6d} paras  hard_lf {n['hard_lf']:6d}  "
                  f"hard_font {n['hard_font']:6d}  hard_either {n['hard_either']:6d}  "
                  f"soft {soft:6d}   {name}")
    print()
    print(f"{docs} .ppt documents, {soft_docs} with at least one soft paragraph")
    for k in ("paragraphs", "hard_lf", "hard_font", "hard_either"):
        print(f"  {k:12s} {total[k]}")
    print("  stated line feeds:",
          ", ".join(f"{k}×{v}" for k, v in sorted(total.items()) if k.startswith("lf=")))
