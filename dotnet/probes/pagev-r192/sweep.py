#!/usr/bin/env python3
"""Render our half of the words track, one output directory per document, under a pinned clock.

    XRAY_OFFICE_CLI=<...>/XRay.Office.Cli ./sweep.py <targets.txt> <outdir> [jobs]

The two rules this obeys are both scars written up in `dotnet/CLAUDE.md`: a parallel sweep gives each
*document* its own directory rather than each worker slot, and `SOURCE_DATE_EPOCH` is pinned or every
date-bearing document moves for nothing. Run it twice -- once at the round's base, once with the
change -- and compare with `confine.py`; the reference half is not rendered at all, which is sound
because a diff confined to `dotnet/src` cannot reach `soffice`.
"""
import concurrent.futures
import hashlib
import os
import pathlib
import subprocess
import sys

CLI = os.environ.get('XRAY_OFFICE_CLI')
if not CLI:
    sys.exit('set XRAY_OFFICE_CLI')

TARGETS = [line.strip() for line in open(sys.argv[1]) if line.strip()]
OUT = pathlib.Path(sys.argv[2])
JOBS = int(sys.argv[3]) if len(sys.argv) > 3 else 3
CORPUS = pathlib.Path(os.environ.get('XRAY_OFFICE_CORPUS', '/home/user/sample-files'))

OUT.mkdir(parents=True, exist_ok=True)


def render(rel: str) -> tuple[str, bool]:
    ident = hashlib.md5(rel.encode()).hexdigest()[:12]
    target = OUT / ident
    target.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH='0')
    done = subprocess.run(
        [CLI, 'render', str(CORPUS / rel), '--outdir', str(target), '--format', 'pdf'],
        capture_output=True, text=True, env=env, timeout=900)
    return rel, done.returncode == 0 and any(target.glob('*.pdf'))


with concurrent.futures.ThreadPoolExecutor(max_workers=JOBS) as pool:
    failed = [rel for rel, ok in pool.map(render, TARGETS) if not ok]

print(f'{len(TARGETS)} documents, {len(failed)} failed')
for rel in failed:
    print('  failed', rel)
