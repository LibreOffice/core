#!/usr/bin/env python3
"""Render one track with one engine and bank three numbers per document.

    sweep.py ref  <root> <out.tsv> [jobs]
    sweep.py <path-to-XRay.Office.Cli> <root> <out.tsv> [jobs]

Columns: path, status, pages, glyphs, sha256 of the PDF.

`glyphs` is `batch-check.sh`'s column 9 -- alphanumeric *characters* over `pdftotext`'s
output, the figure the gate's second check compares -- reproduced here so that a leg run
by this script is readable against a gate row.

The PDF is deleted as soon as the three numbers are taken: the whole track twice over does
not fit in this container, and a measurement that has been banked does not need its render.

ONE DIRECTORY PER DOCUMENT, not per worker slot: a thread pool does not hand out
consecutive indices, so `out / f'w{i % jobs}'` puts two live renders in one directory and
one of them deletes the other's output, silently (`dotnet/CLAUDE.md`, "A parallel sweep
must give each document its own directory").

The reference leg is run ONCE and its numbers are reused for both of ours.  That is not a
shortcut, it is what removes C13: our own renders evaluate no volatile function and are
pinned by SOURCE_DATE_EPOCH, so they are identical whatever day they run on, while the
reference's are not -- and a reference column that is literally the same bytes on both legs
cannot move a verdict between them.
"""
import hashlib, os, shutil, subprocess, sys, tempfile
from concurrent.futures import ThreadPoolExecutor

MODE = sys.argv[1]
ROOT = sys.argv[2]
OUT = sys.argv[3]
JOBS = int(sys.argv[4]) if len(sys.argv) > 4 else 3
SOFFICE = '/opt/libreoffice26.2/program/soffice'
EPOCH = '1700000000'
EXTS = ('.xlsx', '.xlsm', '.xls', '.xlsb', '.xltx', '.xltm', '.ods', '.ots', '.fods', '.csv')


def glyphs(pdf):
    try:
        txt = subprocess.run(['pdftotext', pdf, '-'], capture_output=True, timeout=300).stdout
    except subprocess.SubprocessError:
        return -1
    return sum(1 for c in txt.decode('utf-8', 'replace') if c.isalnum())


def pages(pdf):
    try:
        info = subprocess.run(['pdfinfo', pdf], capture_output=True, timeout=120,
                              text=True).stdout
    except subprocess.SubprocessError:
        return -1
    for line in info.splitlines():
        if line.startswith('Pages:'):
            return int(line.split()[1])
    return -1


def one(path):
    tmp = tempfile.mkdtemp(prefix='sweep-')
    stem = os.path.splitext(os.path.basename(path))[0]
    try:
        if MODE == 'ref':
            cmd = ['timeout', '-k', '30', '900', SOFFICE,
                   '-env:UserInstallation=file://' + tmp + '/profile',
                   '--headless', '--norestore', '--convert-to', 'pdf', '--outdir', tmp, path]
            env = dict(os.environ)
        else:
            cmd = ['timeout', '-k', '30', '900', MODE, 'render', path,
                   '--format', 'pdf', '--outdir', tmp]
            env = dict(os.environ, SOURCE_DATE_EPOCH=EPOCH)
        subprocess.run(cmd, capture_output=True, env=env)
        pdf = os.path.join(tmp, stem + '.pdf')
        if not os.path.exists(pdf):
            return (path, 'failed', -1, -1, '-')
        with open(pdf, 'rb') as fh:
            sha = hashlib.sha256(fh.read()).hexdigest()
        return (path, 'ok', pages(pdf), glyphs(pdf), sha)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


docs = sorted(os.path.join(d, f)
              for d, _, fs in os.walk(ROOT) for f in fs
              if os.path.splitext(f)[1].lower() in EXTS)
print('%s: %d documents' % (MODE, len(docs)), file=sys.stderr)
with open(OUT, 'w') as fh:
    fh.write('path\tstatus\tpages\tglyphs\tsha256\n')
    with ThreadPoolExecutor(JOBS) as pool:
        for i, row in enumerate(pool.map(one, docs)):
            fh.write('\t'.join(str(x) for x in row) + '\n')
            fh.flush()
            if i % 25 == 0:
                print('%d/%d' % (i, len(docs)), file=sys.stderr)
print('done', file=sys.stderr)
