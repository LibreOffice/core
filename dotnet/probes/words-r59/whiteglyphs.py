#!/usr/bin/env python3
"""Glyphs drawn white, per document, on both sides.

    whiteglyphs.py <ours-dir> <ref-dir> [threshold]

The direct measurement of the automatic-font-colour defect and of its risk, which are the same
number read in two directions: a document where the reference draws white glyphs and we draw none
is text we have painted into its own background, and a document where *we* draw white glyphs and
the reference does not is text we have painted out. Both are printed, and neither is netted
against the other.

Refuses to print unless every reference rendering has an `ours` counterpart.
"""
import glob, os, re, sys
sys.path.insert(0, "/c/sandbox/workdir/wt-words-r50/dotnet/research/probes/slides-r15")
from pdfops import objects, pages, content  # noqa: E402

GLYPH = re.compile(
    rb'(?:([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+rg)|(?:([-\d.]+)\s+g\b)'
    rb'|(\((?:\\.|[^\\()])*\))\s*Tj|(<[0-9A-Fa-f\s]*>)\s*Tj'
    rb'|(\[(?:\\.|[^\\\[\]])*\])\s*TJ')


def white(pdf, threshold=0.99):
    data = open(pdf, 'rb').read()
    objs = objects(data)
    total = 0
    for pnum in pages(data, objs):
        isWhite = False
        for m in GLYPH.finditer(content(data, objs, pnum)):
            if m.group(1) is not None:
                isWhite = all(float(v) >= threshold for v in m.group(1, 2, 3))
            elif m.group(4) is not None:
                isWhite = float(m.group(4)) >= threshold
            elif isWhite:
                body = m.group(5) or m.group(6) or m.group(7)
                parts = (re.finditer(rb'\((?:\\.|[^\\()])*\)|<[0-9A-Fa-f\s]*>', body)
                         if m.group(7) else [m])
                for part in parts:
                    s = part.group(0) if m.group(7) else (m.group(5) or m.group(6))
                    total += (len(re.sub(rb'\\(\d{1,3}|.)', b'x', s[1:-1])) if s[:1] == b'('
                              else len(re.sub(rb'\s', b'', s[1:-1])) // 2)
    return total


if __name__ == '__main__':
    ours_dir, ref_dir = sys.argv[1], sys.argv[2]
    refs = sorted(glob.glob(os.path.join(ref_dir, '*.pdf')))
    missing = [os.path.basename(p) for p in refs
               if not os.path.exists(os.path.join(ours_dir, os.path.basename(p)))]
    if missing:
        print('REFUSING: %d of %d reference renderings have no ours' % (len(missing), len(refs)))
        sys.exit(2)
    rows = []
    for p in refs:
        ident = os.path.basename(p)[:-4]
        try:
            rows.append((ident, white(os.path.join(ours_dir, ident + '.pdf')), white(p)))
        except Exception as exc:
            print('  !! %s: %s' % (ident, exc))
    short = [r for r in rows if r[2] > r[1]]
    long = [r for r in rows if r[1] > r[2]]
    print('%d documents compared' % len(rows))
    print('SHORT — the reference draws white where we do not: %d glyphs in %d documents'
          % (sum(r[2] - r[1] for r in short), len(short)))
    for ident, o, r in sorted(short, key=lambda t: t[1] - t[2])[:40]:
        print('   %6d   ours %6d  ref %6d   %s' % (r - o, o, r, ident))
    print('LONG — we draw white where the reference does not: %d glyphs in %d documents'
          % (sum(r[1] - r[2] for r in long), len(long)))
    for ident, o, r in sorted(long, key=lambda t: t[2] - t[1])[:40]:
        print('   %6d   ours %6d  ref %6d   %s' % (o - r, o, r, ident))
