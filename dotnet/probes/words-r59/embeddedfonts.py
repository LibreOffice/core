#!/usr/bin/env python3
"""Which corpus documents embed a font program in the package, on all three tracks.

    embeddedfonts.py <manifest>

Found while censusing the fallback *order*, and it is a different defect: the largest
face-selection divergence on the slides track is not an ordering mistake at all.
`Sean Monogue.pptx` carries four `ppt/fonts/*.fntdata` parts and a `p:embeddedFontLst`; the
reference draws **5 527 glyphs of Verdana** from them and we draw DejaVu Sans, because
`fc-match Verdana` on this machine answers `DejaVuSans.ttf` — the family is not installed and the
only copy of it in existence is inside the package.

Counted per format, because the three spell it differently and a reader has to open a different
part for each:

  * OOXML presentations: `ppt/fonts/*.fntdata`, listed in `p:embeddedFontLst`. Obfuscated the
    same way DOCX's are.
  * OOXML word processing: `word/fonts/*.odttf`, listed in `w:embedRegular`/`w:embedBold`/…
    inside `w:settings`. The `.odttf` is a TTF whose first 32 bytes are XORed with the GUID in
    the part name.
  * ODF: `Fonts/*` inside the package, declared by `style:font-face-src`.

What it cannot see: whether the embedded family is *also* installed, in which case nothing moves.
Every row here was cross-checked with `fc-match` before being called a divergence.
"""
import csv, os, re, sys, zipfile, collections

PATTERNS = [
    ('pptx-fntdata', re.compile(r'^ppt/fonts/.+\.fntdata$', re.I)),
    ('docx-odttf', re.compile(r'^word/fonts/.+\.odttf$', re.I)),
    ('odf-fonts', re.compile(r'^Fonts/.+$', re.I)),
]

if __name__ == '__main__':
    man = sys.argv[1]
    root = os.path.dirname(os.path.abspath(man))
    rows = collections.defaultdict(list)
    scanned = collections.Counter()
    with open(man, newline='', encoding='utf-8') as f:
        for r in csv.DictReader(f, delimiter='\t'):
            path = os.path.join(root, r['path'])
            scanned[r['family']] += 1
            try:
                z = zipfile.ZipFile(path)
            except Exception:
                continue                     # a binary .doc/.ppt/.xls is not a zip
            names = z.namelist()
            for kind, pattern in PATTERNS:
                hits = [n for n in names if pattern.match(n)]
                if hits:
                    rows[r['family']].append((len(hits), kind, r['path']))

    for family in ('words', 'slides', 'sheets'):
        found = sorted(rows[family], reverse=True)
        print('=== %s: %d documents scanned, %d embed a font'
              % (family, scanned[family], len(found)))
        for n, kind, path in found:
            print('   %2d parts  %-14s %s' % (n, kind, path))
        print()
