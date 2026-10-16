#!/usr/bin/env python3
"""Render the corpus's legacy binary word-processing documents with one CLI.

    sweep-doc.py <cli> <outdir> [jobs] [ext]

One directory per DOCUMENT, never per worker slot (`CLAUDE.md`, *A parallel sweep must give each
document its own directory*), and `SOURCE_DATE_EPOCH` pinned so two legs are byte-comparable.

`ext` defaults to `doc`, which is the whole population this round's change can reach: every file
it touches is under `XRay.Office.WordProcessing/Ww8`, and `git grep` says the only type any other
reader borrows from that folder is `Ww8DateTime`.
"""
import csv
import os
import pathlib
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

CORPUS = pathlib.Path('/home/user/sample-files')
EPOCH = '1757462400'

cli, out = sys.argv[1], pathlib.Path(sys.argv[2])
jobs = int(sys.argv[3]) if len(sys.argv) > 3 else 3
want = (sys.argv[4] if len(sys.argv) > 4 else 'doc').lower()
out.mkdir(parents=True, exist_ok=True)

rows = []
with (CORPUS / 'MANIFEST.tsv').open() as fh:
    for row in csv.DictReader(fh, delimiter='\t'):
        if row['ext'].lower() == want:
            rows.append(row['path'])

env = dict(os.environ, SOURCE_DATE_EPOCH=EPOCH)


def one(path):
    src = CORPUS / path
    key = src.stem + '__' + src.suffix.lstrip('.').lower()
    d = out / key
    d.mkdir(parents=True, exist_ok=True)
    try:
        r = subprocess.run([cli, 'render', str(src), '--outdir', str(d)],
                           capture_output=True, text=True, timeout=900, env=env)
        ok = r.returncode == 0
    except subprocess.TimeoutExpired:
        ok = False
    return key, ok


done = failed = 0
with ThreadPoolExecutor(max_workers=jobs) as pool:
    for key, ok in pool.map(one, rows):
        done += 1
        if not ok:
            failed += 1
            print('FAILED %s' % key)

print('%d of %d rendered, %d failed' % (done - failed, len(rows), failed))
