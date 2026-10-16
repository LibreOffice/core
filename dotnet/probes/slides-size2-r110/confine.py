#!/usr/bin/env python3
"""Render a list of documents with one CLI and record, per document:
pages (from the real page tree), alphanumeric characters, and the md5 of the PDF.

One directory per DOCUMENT, never per worker slot -- two live renders in one
directory and one deletes the other's output, silently.
"""
import hashlib, os, shutil, subprocess, sys, tempfile
from concurrent.futures import ThreadPoolExecutor
import pymupdf

CLI, ROOT, LIST, OUT, JOBS = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], int(sys.argv[5])
WORK = tempfile.mkdtemp(prefix='confine-', dir='/home/user/r110-work/sweep')

def one(item):
    i, rel = item
    stem = os.path.splitext(os.path.basename(rel))[0]
    ext = os.path.splitext(rel)[1].lstrip('.')
    ident = f'{stem}__{ext}'
    d = os.path.join(WORK, f'd{i}')
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d, exist_ok=True)
    try:
        subprocess.run([CLI, 'render', os.path.join(ROOT, rel), '--format', 'pdf', '--outdir', d],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=600)
        pdf = os.path.join(d, stem + '.pdf')
        if not os.path.exists(pdf):
            return f'{ident}\tFAILED\t\t'
        data = open(pdf, 'rb').read()
        doc = pymupdf.open(pdf)
        pages = doc.page_count
        alnum = sum(1 for p in doc for c in p.get_text() if c.isalnum())
        doc.close()
        return f'{ident}\t{pages}\t{alnum}\t{hashlib.md5(data).hexdigest()}'
    except Exception as exc:
        return f'{ident}\tERROR\t{type(exc).__name__}\t'
    finally:
        shutil.rmtree(d, ignore_errors=True)

rows = [l.strip() for l in open(LIST) if l.strip()]
with ThreadPoolExecutor(max_workers=JOBS) as pool:
    out = list(pool.map(one, enumerate(rows)))
with open(OUT, 'w') as f:
    f.write('\n'.join(out) + '\n')
shutil.rmtree(WORK, ignore_errors=True)
print(f'{OUT}: {len(out)} rows')
