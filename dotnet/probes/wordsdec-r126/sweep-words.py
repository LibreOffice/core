#!/usr/bin/env python3
"""Render our half of the WORDS track with one CLI, one output directory per DOCUMENT.

    sweep-words.py <cli> <outdir> [jobs]

One directory per document, not per worker slot: a thread pool does not hand out consecutive
indices and two live renders in one directory lose each other's output silently
(`CLAUDE.md`, *A parallel sweep must give each document its own directory*).

`SOURCE_DATE_EPOCH` is pinned so two legs are byte-comparable; without it every rendering moves
and the reach figure reads as the whole set.
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
out.mkdir(parents=True, exist_ok=True)

rows = []
with (CORPUS / 'MANIFEST.tsv').open() as fh:
    for row in csv.DictReader(fh, delimiter='\t'):
        if row['family'] == 'words':
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


done = 0
fail = []
with ThreadPoolExecutor(max_workers=jobs) as pool:
    for key, ok in pool.map(one, rows):
        done += 1
        if not ok:
            fail.append(key)
print('rendered %d of %d, %d failed' % (done - len(fail), len(rows), len(fail)))
for f in fail:
    print('  FAILED', f)
