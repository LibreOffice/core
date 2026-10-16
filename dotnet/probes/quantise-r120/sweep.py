#!/usr/bin/env python3
"""Render the whole corpus with one binary and bank four numbers per document.

    sweep.py <path-to-XRay.Office.Cli> <corpus-root> <out.tsv> [jobs]

Columns: path, status, pages, glyphs, sha256 of the PDF.

`glyphs` is `batch-check.sh`'s column 9 -- alphanumeric *characters* over `pdftotext`'s
output, which is the figure the gate's second check compares -- so a leg run by this script
is readable against a gate row.  Columns 4 and 8 decide nothing.

The PDF is deleted as soon as the four numbers are taken; the corpus twice over does not fit
in this container.  ONE DIRECTORY PER DOCUMENT, not per worker slot -- a thread pool does not
hand out consecutive indices, and two live renders in one directory silently delete each
other's output (`dotnet/CLAUDE.md`).

`SOURCE_DATE_EPOCH` is pinned, so our own renders are byte-identical whatever day they run
on and a base/after diff of the sha column is a diff of the drawing and nothing else (C13).
There is no reference leg here on purpose: a rule's thickness is ink, so the gate columns
cannot move, and what this measures is which documents *we* draw differently.
"""
import hashlib, os, shutil, subprocess, sys, tempfile
from concurrent.futures import ThreadPoolExecutor

CLI = sys.argv[1]
ROOT = sys.argv[2]
OUT = sys.argv[3]
JOBS = int(sys.argv[4]) if len(sys.argv) > 4 else 3
EPOCH = '1700000000'
EXTS = ('.docx', '.docm', '.dotx', '.doc', '.dot', '.rtf', '.odt', '.ott', '.fodt',
        '.xlsx', '.xlsm', '.xltx', '.xltm', '.xlsb', '.xls', '.xlt', '.ods', '.ots',
        '.fods', '.csv',
        '.pptx', '.pptm', '.potx', '.potm', '.ppsx', '.ppsm', '.ppt', '.pot', '.pps',
        '.odp', '.otp', '.fodp', '.sxw', '.sxc', '.sxi')


def counts(pdf):
    pages = glyphs = -1
    try:
        info = subprocess.run(['pdfinfo', pdf], capture_output=True, timeout=120,
                              text=True).stdout
        for line in info.splitlines():
            if line.startswith('Pages:'):
                pages = int(line.split()[1])
    except (subprocess.SubprocessError, ValueError):
        pass
    try:
        txt = subprocess.run(['pdftotext', pdf, '-'], capture_output=True, timeout=300).stdout
        glyphs = sum(1 for c in txt.decode('utf-8', 'replace') if c.isalnum())
    except subprocess.SubprocessError:
        pass
    return pages, glyphs


def one(path):
    tmp = tempfile.mkdtemp(prefix='q120-')
    stem = os.path.splitext(os.path.basename(path))[0]
    try:
        subprocess.run(['timeout', '-k', '30', '900', CLI, 'render', path,
                        '--format', 'pdf', '--outdir', tmp],
                       capture_output=True, env=dict(os.environ, SOURCE_DATE_EPOCH=EPOCH))
        pdf = os.path.join(tmp, stem + '.pdf')
        if not os.path.exists(pdf):
            return (path, 'failed', -1, -1, '-')
        with open(pdf, 'rb') as fh:
            sha = hashlib.sha256(fh.read()).hexdigest()
        pages, glyphs = counts(pdf)
        return (path, 'ok', pages, glyphs, sha)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


docs = sorted(os.path.join(d, f)
              for d, _, fs in os.walk(ROOT) for f in fs
              if os.path.splitext(f)[1].lower() in EXTS)
print('%d documents' % len(docs), file=sys.stderr)
with open(OUT, 'w') as fh:
    fh.write('path\tstatus\tpages\tglyphs\tsha256\n')
    with ThreadPoolExecutor(JOBS) as pool:
        for i, row in enumerate(pool.map(one, docs)):
            fh.write('\t'.join(str(x) for x in row) + '\n')
            fh.flush()
            if i % 50 == 0:
                print('%d/%d' % (i, len(docs)), file=sys.stderr)
print('done', file=sys.stderr)
