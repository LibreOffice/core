#!/usr/bin/env python3
"""Render a list of .ppt with one CLI and record `sizes.py`'s per-page dominant drawn
text size, deleting each PDF as it goes.  One directory per document."""
import collections, os, shutil, subprocess, sys, tempfile
from concurrent.futures import ThreadPoolExecutor
import pymupdf

CLI, LIST, OUT, JOBS = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
WORK = tempfile.mkdtemp(prefix='size-', dir='/home/user/r110-work/sweep')

def rows_of(path, ident):
    out = []
    doc = pymupdf.open(path)
    for i, page in enumerate(doc, 1):
        per = collections.Counter()
        for block in page.get_text("dict")["blocks"]:
            for line in block.get("lines", []):
                for span in line.get("spans", []):
                    n = sum(1 for c in span["text"] if c.isalnum())
                    if n:
                        per[round(span["size"], 2)] += n
        size, n = max(per.items(), key=lambda kv: kv[1]) if per else (0.0, 0)
        out.append(f'{ident}\t{i}\t{size}\t{n}')
    doc.close()
    return out

def one(item):
    i, rel = item
    stem = os.path.splitext(os.path.basename(rel))[0]
    ident = f'{stem}__ppt'
    d = os.path.join(WORK, f'd{i}')
    shutil.rmtree(d, ignore_errors=True); os.makedirs(d, exist_ok=True)
    try:
        subprocess.run([CLI, 'render', os.path.join('/home/user/sample-files', rel),
                        '--format', 'pdf', '--outdir', d],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=600)
        pdf = os.path.join(d, stem + '.pdf')
        return rows_of(pdf, ident) if os.path.exists(pdf) else [f'{ident}\tFAILED\t0\t0']
    finally:
        shutil.rmtree(d, ignore_errors=True)

rows = [l.strip() for l in open(LIST) if l.strip()]
with ThreadPoolExecutor(max_workers=JOBS) as pool:
    out = [line for chunk in pool.map(one, enumerate(rows)) for line in chunk]
with open(OUT, 'w') as f:
    f.write('\n'.join(out) + '\n')
shutil.rmtree(WORK, ignore_errors=True)
print(f'{OUT}: {len(out)} rows')
