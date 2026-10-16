#!/usr/bin/env python3
"""Render every words document with a given CLI, in parallel, into one directory."""
import os, sys, subprocess, concurrent.futures, pathlib

CLI = sys.argv[1]
OUT = sys.argv[2]
ROOT = '/c/sandbox/workdir/sample-files/words'
os.makedirs(OUT, exist_ok=True)
env = dict(os.environ, SOURCE_DATE_EPOCH='1700000000')

docs = []
for dirpath, _, names in os.walk(ROOT):
    for n in names:
        docs.append(os.path.join(dirpath, n))
docs.sort()


def one(path):
    # Per-format identity, as batch-check.sh does: report__docx, not report.
    stem = pathlib.Path(path).stem
    ext = pathlib.Path(path).suffix.lstrip('.').lower()
    target = os.path.join(OUT, f'{stem}__{ext}')
    os.makedirs(target, exist_ok=True)
    r = subprocess.run([CLI, 'render', '--quiet', '--outdir', target, path],
                       capture_output=True, text=True, env=env, timeout=900)
    return (stem, ext, r.returncode, (r.stderr or '').strip()[:200])


with concurrent.futures.ThreadPoolExecutor(max_workers=6) as ex:
    results = list(ex.map(one, docs))

bad = [r for r in results if r[2] != 0]
print(f'rendered {len(results)}, failed {len(bad)}')
for b in bad[:20]:
    print('  FAIL', b[0], b[1], b[2], b[3])
