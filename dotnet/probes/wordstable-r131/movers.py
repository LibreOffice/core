#!/usr/bin/env python3
"""Which words renderings moved, and did anything the gate scores move with them?

    movers.py <base-dir> <after-dir> <ref-dir> <out.tsv>

`base-dir` is one PDF per document (gate r129's `ours`), `after-dir` one DIRECTORY per document.
Byte identity first, because a change confined to the drawing path must leave most of the track
alone; then page count and alphanumeric characters against the reference, which is what says the
move was a drawing move and not a layout one.
"""
import pathlib
import re
import sys

import pymupdf

# gate r129's own half was rendered with no `SOURCE_DATE_EPOCH`, so every PDF differs from a pinned
# one in its `/CreationDate` and nothing else -- `CLAUDE.md`, *a reference PDF differs byte for byte
# between two sweeps and it means nothing*. Masked here rather than re-rendering the base leg.
DATE = re.compile(rb'/CreationDate\s*\([^)]*\)')

base, after, ref, out = (pathlib.Path(p) for p in sys.argv[1:5])
ALNUM = re.compile(r'[^0-9A-Za-zÀ-ɏͰ-῿぀-퟿]')


def counts(path):
    with pymupdf.open(path) as d:
        text = ''.join(p.get_text() for p in d)
        return d.page_count, len(ALNUM.sub('', text))


rows = []
for d in sorted(after.iterdir()):
    if not d.is_dir():
        continue
    pdfs = list(d.glob('*.pdf'))
    if not pdfs:
        continue
    a = pdfs[0]
    b = base / f'{d.name}.pdf'
    r = ref / f'{d.name}.pdf'
    if not b.exists():
        continue
    same = DATE.sub(b'', a.read_bytes()) == DATE.sub(b'', b.read_bytes())
    rows.append((d.name, same, a, b, r))

with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\tmoved\tbase_pages\tafter_pages\tref_pages\tbase_glyphs\tafter_glyphs\tref_glyphs\n')
    moved = 0
    for name, same, a, b, r in rows:
        if same:
            fh.write(f'{name}\tno\t-\t-\t-\t-\t-\t-\n')
            continue
        moved += 1
        bp, bg = counts(b)
        ap, ag = counts(a)
        rp, rg = counts(r) if r.exists() else ('-', '-')
        fh.write(f'{name}\tyes\t{bp}\t{ap}\t{rp}\t{bg}\t{ag}\t{rg}\n')
print(f'{len(rows)} documents, {moved} moved, {len(rows) - moved} byte-identical')
