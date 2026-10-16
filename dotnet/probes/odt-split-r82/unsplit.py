#!/usr/bin/env python3
"""Which documents does 26.2.4.2 actually *split* a fly on?

`loext:may-break-between-pages` says a fly *may* break; whether one does is a property of the
document.  So the reach of fly splitting is not "how many documents state the attribute" but "how
many render differently when it is taken away", and that is measurable at the reference with no
layout reasoning at all: strip the attribute, render the patched document through 26.2.4.2, and
compare its page and glyph counts against the reference's own rendering of the original.

A document whose counts do not move is one the reference never split, and one this tree therefore
loses nothing by not splitting.
"""
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
import zipfile

HERE = pathlib.Path(__file__).parent
BANK = pathlib.Path('/home/user/gate-odf-r78/ref')
ATTR = re.compile(rb'\s(?:loext|draw):may-break-between-pages="[^"]*"')


def patched(doc, into):
    """A copy of `doc` with every may-break-between-pages attribute removed."""
    out = into / doc.name
    changed = 0
    with zipfile.ZipFile(doc) as src, zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as dst:
        for item in src.infolist():
            data = src.read(item.filename)
            if item.filename in ('content.xml', 'styles.xml'):
                data, n = ATTR.subn(b'', data)
                changed += n
            dst.writestr(item, data)
    return out, changed


def counts(pdf):
    if not pdf.exists():
        return None
    pages = subprocess.run(['pdfinfo', str(pdf)], capture_output=True, text=True).stdout
    pages = next((int(l.split()[1]) for l in pages.splitlines() if l.startswith('Pages')), None)
    text = subprocess.run(['pdftotext', str(pdf), '-'], capture_output=True).stdout
    text = text.decode('utf-8', 'replace')
    return pages, sum(1 for c in text if c.isalnum())


def main(names):
    print('document\trefpages\trefglyphs\tunsplitpages\tunsplitglyphs\tattrs\tsplits')
    for name in names:
        doc = next(pathlib.Path('/home/user/corpus-odf/words').glob(f'*/odt/{name}.odt'), None)
        if doc is None:
            print(f'! no such document: {name}', file=sys.stderr)
            continue
        before = counts(BANK / f'{name}__odt.pdf')
        with tempfile.TemporaryDirectory(prefix='unsplit-') as tmp:
            tmp = pathlib.Path(tmp)
            copy, n = patched(doc, tmp)
            subprocess.run([str(HERE / 'render.sh'), str(copy), str(tmp / 'out')], check=False)
            after = counts(tmp / 'out' / f'{copy.stem}.pdf')
        if before is None or after is None:
            print(f'{name}\t{before}\t{after}\t{n}\t?')
            continue
        moved = 'SPLITS' if before != after else 'no'
        print(f'{name}\t{before[0]}\t{before[1]}\t{after[0]}\t{after[1]}\t{n}\t{moved}')


if __name__ == '__main__':
    main([line.rstrip('\n') for line in sys.stdin if line.strip()])
