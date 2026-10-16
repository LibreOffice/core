#!/usr/bin/env python3
"""How many legacy form checkboxes the `.doc` half of the words corpus carries.

    python3 doc-checkbox-census.py <corpus-root> [convert-dir]

Round 56 implemented `w:ffData/w:checkBox` for OOXML and censused it exactly: **675 boxes in 12
documents**, read over every `.xml` part of every package.  It recorded that figure as a *floor*
for the corpus, because WW8 spells the same field as a `PLCF` of field characters and RTF as
`\\*\\formfield`, and neither had been counted.

This counts the WW8 half.  **The RTF half needs no probe: the words corpus holds no `.rtf` at
all** — 271 `.docx` and 66 `.doc`, from `MANIFEST.tsv` — so the RTF arm has zero witnesses and
"675 in 12" can only be raised by the 66 `.doc` files.

Two independent instruments, because either alone would be a guess:

  1. **The bytes.**  A WW8 field instruction is stored in the text stream, which is 8-bit or
     16-bit depending on the piece, so `FORMCHECKBOX` is searched for in both encodings across
     the whole compound file.  This over-counts a document that mentions the word in its prose
     and under-counts nothing.
  2. **LibreOffice's own reader**, when a convert directory is given: each `.doc` is converted
     to `.docx` by the reference binary and the result is counted with round 56's rule —
     `w:checkBox` elements over every part.  This is the authority on what the fields *are*,
     and it is the number that matters, because it is what our reader would have to find.

Printing both is the point.  A single number here would be indistinguishable from a wrong one.
"""
import glob
import os
import re
import subprocess
import sys
import zipfile
from concurrent.futures import ThreadPoolExecutor

NEEDLE = b'FORMCHECKBOX'
NEEDLE_16 = NEEDLE.decode().encode('utf-16-le')


def by_bytes(path):
    data = open(path, 'rb').read()
    return data.count(NEEDLE), data.count(NEEDLE_16)


def by_reader(docx):
    total = 0
    with zipfile.ZipFile(docx) as z:
        for name in z.namelist():
            if not name.lower().endswith('.xml'):
                continue
            total += len(re.findall(rb'<w:checkBox[ />]', z.read(name)))
    return total


def convert(path, outdir, slot):
    profile = os.path.join(outdir, f'prof{slot}')
    subprocess.run(
        ['soffice', '-env:UserInstallation=file://' + profile, '--headless', '--norestore',
         '--convert-to', 'docx:MS Word 2007 XML', '--outdir', outdir, path],
        capture_output=True, timeout=600)


if __name__ == '__main__':
    root = sys.argv[1]
    convert_dir = os.path.abspath(sys.argv[2]) if len(sys.argv) > 2 else None

    manifest = os.path.join(root, 'MANIFEST.tsv')
    docs = []
    with open(manifest, encoding='utf-8') as handle:
        head = handle.readline().rstrip('\n').split('\t')
        for line in handle:
            row = dict(zip(head, line.rstrip('\n').split('\t')))
            if row.get('family') == 'words' and row.get('ext') == 'doc':
                docs.append(os.path.join(root, row['path']))

    unreadable = [p for p in docs if not os.path.exists(p)]
    if unreadable:
        print('REFUSING — %d manifest paths do not exist:' % len(unreadable))
        for p in unreadable[:10]:
            print('   ', p)
        sys.exit(2)

    print('%d `.doc` documents from the manifest\n' % len(docs))

    rows = []
    for path in docs:
        eight, sixteen = by_bytes(path)
        if eight or sixteen:
            rows.append((os.path.basename(path), eight, sixteen))

    print('--- instrument 1: the bytes')
    print('%-64s %8s %9s' % ('document', '8-bit', '16-bit'))
    for name, a, b in sorted(rows, key=lambda t: -(t[1] + t[2])):
        print('%-64s %8d %9d' % (name[:64], a, b))
    print('%d of %d documents, %d hits'
          % (len(rows), len(docs), sum(a + b for _, a, b in rows)))

    if convert_dir is None:
        print('\n(no convert directory given — instrument 2 not run)')
        sys.exit(0)

    os.makedirs(convert_dir, exist_ok=True)
    with ThreadPoolExecutor(6) as pool:
        list(pool.map(lambda t: convert(t[1], convert_dir, t[0] % 6), list(enumerate(docs))))

    produced = {os.path.basename(p)[:-5]: p
                for p in glob.glob(os.path.join(convert_dir, '*.docx'))}
    lost = [os.path.basename(p) for p in docs
            if os.path.splitext(os.path.basename(p))[0] not in produced]
    if lost:
        print('\nREFUSING instrument 2 — %d of %d conversions produced nothing:'
              % (len(lost), len(docs)))
        for p in lost[:10]:
            print('   ', p)
        sys.exit(2)

    print('\n--- instrument 2: LibreOffice\'s own reader, via `.doc` -> `.docx`')
    total = 0
    seen = 0
    for path in docs:
        stem = os.path.splitext(os.path.basename(path))[0]
        n = by_reader(produced[stem])
        if n:
            seen += 1
            total += n
            print('%-64s %6d' % (os.path.basename(path)[:64], n))
    print('%d of %d documents, %d boxes' % (seen, len(docs), total))
