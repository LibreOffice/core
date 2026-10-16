#!/usr/bin/env python3
"""Render every file of one extension under a directory tree with one CLI.

    sweep-tree.py <cli> <root> <ext> <outdir> [jobs]

`sweep-doc.py`'s sibling for the converted ODF corpus, which has no manifest of its own: same
one-directory-per-document rule and the same pinned `SOURCE_DATE_EPOCH`, with the population taken
from the tree instead.
"""
import os
import pathlib
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

EPOCH = '1757462400'

cli, root, ext, out = sys.argv[1], pathlib.Path(sys.argv[2]), sys.argv[3].lower(), pathlib.Path(sys.argv[4])
jobs = int(sys.argv[5]) if len(sys.argv) > 5 else 3
out.mkdir(parents=True, exist_ok=True)

files = sorted(p for p in root.rglob('*') if p.suffix.lower() == '.' + ext)
env = dict(os.environ, SOURCE_DATE_EPOCH=EPOCH)


def one(src):
    key = src.stem + '__' + src.suffix.lstrip('.').lower()
    d = out / key
    d.mkdir(parents=True, exist_ok=True)
    try:
        r = subprocess.run([cli, 'render', str(src), '--outdir', str(d)],
                           capture_output=True, text=True, timeout=900, env=env)
        return key, r.returncode == 0
    except subprocess.TimeoutExpired:
        return key, False


done = failed = 0
with ThreadPoolExecutor(max_workers=jobs) as pool:
    for key, ok in pool.map(one, files):
        done += 1
        if not ok:
            failed += 1
            print('FAILED %s' % key)

print('%d of %d rendered, %d failed' % (done - failed, len(files), failed))
