#!/usr/bin/env python3
"""Summed |ink|% against a banked reference, for one document or a whole sweep directory.

    ink.py <sweep-dir> <ref-dir> <out.tsv> [identity ...]

A thin driver over `.claude/skills/render-comparison/scripts/pdf-image-diff.py`, which is the
instrument this project scores a rendering change on when no gate column can see it: it renders
both sides at 512 pixels on the long edge and reports, per page, how much ink one side has that
the other does not. The sum over a document's pages and the count of MAJOR pages are the two
numbers; both are directionless, so a document is better when they fall.

Named identities restrict the run; with none, every document in the sweep directory is scored.
"""
import pathlib
import re
import subprocess
import sys

DIFF = pathlib.Path(__file__).resolve().parents[3] \
    / '.claude/skills/render-comparison/scripts/pdf-image-diff.py'

sweep, refdir, out = (pathlib.Path(p) for p in sys.argv[1:4])
only = set(sys.argv[4:])

rows = []
for d in sorted(p for p in sweep.iterdir() if p.is_dir()):
    if only and d.name not in only:
        continue
    pdfs = sorted(d.glob('*.pdf'))
    ref = refdir / (d.name + '.pdf')
    if not pdfs or not ref.exists():
        continue

    run = subprocess.run(
        [sys.executable, str(DIFF), str(pdfs[0]), str(ref), '--outdir', '/tmp/ink-scratch'],
        capture_output=True, text=True, timeout=1800)

    total = 0.0
    major = pages = 0
    for line in run.stdout.splitlines():
        cells = line.split('\t')
        if len(cells) >= 6 and re.fullmatch(r'\d+', cells[0]):
            pages += 1
            try:
                total += float(cells[3])
            except ValueError:
                continue
            if cells[5].strip() == 'MAJOR':
                major += 1

    rows.append((d.name, pages, total, major))

with open(out, 'w', encoding='utf-8') as fh:
    fh.write('identity\tpages\tsum_abs_ink_pct\tmajor_pages\n')
    for name, pages, total, major in rows:
        fh.write('%s\t%d\t%.2f\t%d\n' % (name, pages, total, major))

print('%d documents, summed |ink|%% %.2f, MAJOR pages %d'
      % (len(rows), sum(r[2] for r in rows), sum(r[3] for r in rows)))
