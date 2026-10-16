#!/usr/bin/env python3
"""Where `w:w` (character width scaling) is stated: on a run, or on a style this tree ignores.

    stylescale-census.py <corpus-root> <manifest.tsv> <out.tsv>

`WordParagraphFormats.WidthOf` reads `w:rPr/w:w` and the class' own remarks say it was *"read
by nothing at all"* before that; what it is still read by nothing from is a paragraph or
character STYLE's `w:rPr`.  Measured on `Regulations Governing the Status…`: changing the
`SL` style's `w:w` from 96 to 60 and nothing else changes no width at all, while the same
value on the run changes every one.

So the census splits the corpus's `w:w` statements by the part they are in.
"""
import pathlib
import re
import sys
import zipfile

W = re.compile(r'<w:w w:val="(\d+)"')
STORY = re.compile(r'^word/(document|header\d*|footer\d*|footnotes|endnotes)\.xml$')

root, man, out = sys.argv[1:4]
rows = []
for line in pathlib.Path(man).read_text(encoding='utf-8').splitlines()[1:]:
    p = line.split('\t')
    if len(p) < 4 or p[3] not in ('docx', 'docm', 'dotx'):
        continue
    story = styles = 0
    try:
        z = zipfile.ZipFile(pathlib.Path(root) / p[2])
    except (zipfile.BadZipFile, OSError):
        continue
    with z:
        for name in z.namelist():
            if name == 'word/styles.xml':
                styles += sum(1 for m in W.finditer(z.read(name).decode('utf-8', 'replace'))
                              if m.group(1) != '100')
            elif STORY.match(name):
                story += sum(1 for m in W.finditer(z.read(name).decode('utf-8', 'replace'))
                             if m.group(1) != '100')
    if story or styles:
        rows.append((p[2], story, styles))

rows.sort(key=lambda r: -r[2])
with open(out, 'w', encoding='utf-8') as fh:
    fh.write('path\tw_on_a_run\tw_in_styles_xml\n')
    for r in rows:
        fh.write('\t'.join(str(x) for x in r) + '\n')
print(f'{len(rows)} docx state a non-100 w:w; '
      f'{sum(1 for r in rows if r[2])} state one in styles.xml ({sum(r[2] for r in rows)} statements, '
      f'unread), {sum(1 for r in rows if r[1])} state one on a run ({sum(r[1] for r in rows)}, read)')
