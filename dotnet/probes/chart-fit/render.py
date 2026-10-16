#!/usr/bin/env python3
"""Render a list of corpus-relative paths, in parallel, one directory per document.

The directory is keyed on a hex digest of the document's corpus-relative path — not on a
worker slot, because a thread pool does not work consecutive indices and two renders would
share a directory; and not on the path itself, because `soffice` truncates
`-env:UserInstallation=file://…` at the first space.

    render.py --out <dir> --cli <XRay.Office.Cli>     < paths
    render.py --out <dir> --soffice /opt/…/soffice  < paths
"""
import argparse, concurrent.futures as cf, hashlib, os, subprocess, sys
from pathlib import Path

ap = argparse.ArgumentParser()
ap.add_argument('--out', required=True)
ap.add_argument('--cli')
ap.add_argument('--soffice')
ap.add_argument('--corpus', default='/home/user/sample-files')
ap.add_argument('--jobs', type=int, default=4)
args = ap.parse_args()

CORPUS = Path(args.corpus)
OUT = Path(args.out)
OUT.mkdir(parents=True, exist_ok=True)
rows = [l.strip() for l in sys.stdin if l.strip() and not l.startswith('#')]


def key(rel):
    return hashlib.sha1(rel.encode('utf-8')).hexdigest()[:16]


def render(rel):
    src = CORPUS / rel
    out = OUT / key(rel)
    out.mkdir(parents=True, exist_ok=True)
    pdf = out / (src.stem + '.pdf')
    if pdf.exists():
        return rel, 'cached'
    if args.cli:
        cmd = [args.cli, 'render', str(src), '--format', 'pdf', '--outdir', str(out)]
    else:
        prof = out / 'prof'
        cmd = [args.soffice, '-env:UserInstallation=file://' + str(prof), '--headless',
               '--norestore', '--convert-to', 'pdf', '--outdir', str(out), str(src)]
    try:
        subprocess.run(cmd, capture_output=True, timeout=900)
    except subprocess.TimeoutExpired:
        return rel, 'timeout'
    return rel, 'ok' if pdf.exists() else 'FAILED'


done = 0
with cf.ThreadPoolExecutor(max_workers=args.jobs) as pool:
    for rel, state in pool.map(render, rows):
        done += 1
        if state != 'ok' and state != 'cached':
            print(f'{state}\t{rel}', flush=True)
print(f'# rendered {done} of {len(rows)}', flush=True)
