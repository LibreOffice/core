#!/usr/bin/env python3
"""Which corpus word-processing documents carry a TOC field, and does its result state a
character style.

    toc-census.py <rows.tsv> <corpus-root> <out.tsv>

A declaration census, and therefore an UPPER bound on reach -- a `TOC` instruction in the file
is a candidate the layout must still select. The drawn measurement is `rule-census.py`.

Both container families are walked, because the rule is not OOXML's: `ww8par5.cxx`:2334, 3362
and 3653 dress a WW8 index link in the same `Index Link` pool style that
`DomainMapper_Impl.cxx`:9358 gives a DOCX one.
"""
import pathlib
import re
import sys
import zipfile

rowsfile, root, out = sys.argv[1:4]
root = pathlib.Path(root)

with open(out, 'w', encoding='utf-8') as fh:
    fh.write('path\text\ttoc_fields\thyperlink_rstyle_runs\tnote\n')
    for line in pathlib.Path(rowsfile).read_text(encoding='utf-8').splitlines():
        p = line.split('\t')
        if len(p) < 7 or p[6] != 'match' or p[1] not in ('docx', 'doc'):
            continue
        src = root / p[0]
        toc = hyp = 0
        note = ''
        try:
            if p[1] == 'docx':
                with zipfile.ZipFile(src) as z:
                    blob = b''
                    for n in z.namelist():
                        if n.startswith('word/') and n.endswith('.xml'):
                            blob += z.read(n)
                text = blob.decode('utf-8', 'replace')
                toc = len(re.findall(r'<w:instrText[^>]*>\s*TOC\b', text)) \
                    + len(re.findall(r'w:instr="[^"]*\bTOC\b', text))
                hyp = len(re.findall(r'<w:rStyle w:val="Hyperlink"\s*/>', text))
            else:
                data = src.read_bytes()
                # A WW8 field instruction is stored as UTF-16LE text in the WordDocument
                # stream; the marker is the literal " TOC " with the field-begin 0x13 before
                # it. Counting the UTF-16 spelling of "TOC" is coarse and is why this column
                # is only ever read as an upper bound.
                toc = data.count(' \x00T\x00O\x00C\x00')
                note = 'utf16-scan'
        except Exception as exc:                      # noqa: BLE001
            note = f'{type(exc).__name__}'
        if toc or hyp:
            fh.write(f'{p[0]}\t{p[1]}\t{toc}\t{hyp}\t{note}\n')
print('done')
