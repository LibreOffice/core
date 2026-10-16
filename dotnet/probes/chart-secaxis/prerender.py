#!/usr/bin/env python3
"""Render the sweep's `ours` half in parallel, from the end of its path list backwards.

sweep.py renders one document at a time and skips any whose PDF is already in place, so a
helper that fills the same directory from the other end simply halves the wall clock. Each
render goes to a worker-private directory and is moved into place with os.replace, which is
atomic, so the sweep can never read a half-written file.
"""
import os, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

CORPUS = Path('/home/user/sample-files')
GATE = Path('/home/user/gate-2f47')
OUT = Path('/home/user/tmp-secaxis/sweep')
CLI = ('/home/user/wt-secaxis/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/'
       'XRay.Office.Cli')

def identity(p): return f"{p.stem}__{p.suffix.lstrip('.').lower()}"

paths = []
for line in (GATE / 'parity.tsv').read_text().splitlines():
    if line.startswith('#') or line.startswith('path\t'): continue
    paths.append(line.split('\t')[0])
paths = sorted(paths)[::-1]

(OUT / 'ours').mkdir(parents=True, exist_ok=True)

def one(i_rel):
    i, rel = i_rel
    src = CORPUS / rel
    dest = OUT / 'ours' / f'{identity(src)}.pdf'
    if dest.exists(): return
    work = Path(f'/home/user/tmp-secaxis/pre/{i}')
    subprocess.run(['rm', '-rf', str(work)])
    work.mkdir(parents=True, exist_ok=True)
    try:
        subprocess.run([CLI, 'render', str(src), '--format', 'pdf', '--outdir', str(work)],
                       capture_output=True, timeout=600)
    except subprocess.TimeoutExpired:
        return
    made = work / (src.stem + '.pdf')
    if made.exists() and not dest.exists():
        os.replace(made, dest)

with ThreadPoolExecutor(max_workers=6) as pool:
    list(pool.map(one, enumerate(paths)))
print('prerender done')
