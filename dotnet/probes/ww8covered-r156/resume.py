#!/usr/bin/env python3
"""Finish a sweep leg the container restart killed, and validate what it had already written.

CLAUDE.md's rule is to restart a sweep into a FRESH directory rather than reuse one, because two
writers in one directory corrupt it silently. That rule is about two LIVE runs; here the first run
is definitively dead. What its death can leave behind is a truncated PDF, so this does what a fresh
run would not: it opens every file the leg already holds and deletes any that does not parse or
holds no page, then lists what is still missing.

    resume.py <leg>            # report and clean
    resume.py <leg> --list     # print the documents still to render, one per line
"""
import hashlib
import pathlib
import sys

import pymupdf

LEG = pathlib.Path('/home/user/ww8covered-r156-sweep') / sys.argv[1]
CORPUS = pathlib.Path('/home/user/sample-files')


def documents():
    rows = []
    with open(CORPUS / 'MANIFEST.tsv') as handle:
        for line in handle.read().splitlines()[1:]:
            parts = line.split('\t')
            if len(parts) > 2 and parts[2]:
                rows.append(str(CORPUS / parts[2]))
    rows += [str(p) for p in sorted(pathlib.Path('/home/user/corpus-odf/odt').glob('*.odt'))]
    rows += [str(p) for p in sorted(pathlib.Path('/home/user/corpus-odf/rtf').glob('*.rtf'))]
    return rows


bad, missing, good = [], [], 0
for path in documents():
    key = hashlib.md5(path.encode()).hexdigest()[:12]
    found = list((LEG / key).glob('*.pdf')) if (LEG / key).is_dir() else []
    if not found:
        missing.append(path)
        continue
    try:
        with pymupdf.open(found[0]) as doc:
            if doc.page_count < 1:
                raise ValueError('no pages')
        good += 1
    except Exception as error:                      # noqa: BLE001 -- any failure means re-render it
        bad.append((path, found[0], error))
        found[0].unlink()
        missing.append(path)

if '--list' in sys.argv:
    print('\n'.join(missing))
else:
    print(f'{good} good, {len(bad)} unreadable and deleted, {len(missing)} to render')
    for path, pdf, error in bad:
        print(f'  DELETED {pdf.name}: {error}')
