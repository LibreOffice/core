#!/usr/bin/env python3
"""Rank the corpus by |ink|% against one banked reference, per page AND per document.

    score-ink.py <rows.tsv> <ours-dir> <ref-dir> <per-page.tsv> <per-doc.tsv> [jobs]

`rows.tsv` is a banked gate's row file: column 1 the corpus-relative path, 2 the extension,
7 the gate verdict. Only rows whose verdict is `match` are scored -- this round's whole point
is the passing set.

Per page is as much of the ranking as per document: a 700-page workbook with a small per-page
divergence outranks a 4-page one with the same total and means something else.

The row parser requires the tool's six numeric/verdict columns behind a leading integer, so the
trailing "N pages, M with major differences" summary cannot be eaten as a data row -- the defect
that inverted a stored aggregate two rounds running.

**Long documents are scored in page CHUNKS, and that is a disk measure rather than a metric
change.** `pdf-image-diff.py` rasterises the whole document into one temporary directory before
it diffs anything, so the corpus's largest workbook -- 4372 pages -- costs several gigabytes in
one go and this container has single-digit gigabytes free. Its `|ink|%` is computed per page
from that page's own two rasters and nothing crosses a page boundary, so splitting the pair at
the same page numbers and summing gives the identical answer; `chunk-equivalence.txt` is the
check that it does.
"""
import concurrent.futures as cf
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile

TOOL = pathlib.Path('/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py')
ROW = re.compile(r'^(\d+)\t([\d.]+)\t(-?[\d.]+)\t([\d.]+)\t(\d+)\t(\S+)$')
CHUNK = 25


def run_pair(ours, ref, tmp):
    out = tmp / 'out'
    r = subprocess.run([sys.executable, str(TOOL), str(ours), str(ref), '--outdir', str(out)],
                       capture_output=True, text=True, timeout=3600)
    shutil.rmtree(out, ignore_errors=True)
    pages = []
    for line in r.stdout.splitlines():
        m = ROW.match(line)
        if m:
            pages.append([int(m.group(1)), float(m.group(2)), float(m.group(3)),
                          float(m.group(4)), int(m.group(5)), m.group(6), ''])
        elif pages and line.startswith((' ', '\t')) and line.strip():
            if not pages[-1][6]:
                pages[-1][6] = line.strip()[:160]
    if not pages:
        tail = (r.stdout.strip().splitlines() or r.stderr.strip().splitlines() or ['no-rows'])[-1]
        return None, tail[:100]
    return pages, 'ok'


def npages(pdf):
    import pymupdf
    with pymupdf.open(pdf) as d:
        return d.page_count


def slice_pdf(src, first, last, dst):
    import pymupdf
    with pymupdf.open(src) as d:
        out = pymupdf.open()
        out.insert_pdf(d, from_page=first, to_page=last)
        out.save(dst)
        out.close()


def score(ours, ref):
    no, nr = npages(ours), npages(ref)
    if no != nr:
        return None, f'page-count {no} vs {nr}'
    tmp = pathlib.Path(tempfile.mkdtemp(prefix='inkr124-'))
    try:
        if no <= CHUNK:
            return run_pair(ours, ref, tmp)
        allpages, note = [], 'ok'
        for base in range(0, no, CHUNK):
            last = min(base + CHUNK, no) - 1
            a, b = tmp / 'a.pdf', tmp / 'b.pdf'
            slice_pdf(ours, base, last, a)
            slice_pdf(ref, base, last, b)
            got, note = run_pair(a, b, tmp)
            a.unlink(missing_ok=True)
            b.unlink(missing_ok=True)
            if got is None:
                return None, f'chunk@{base + 1}: {note}'
            for p in got:
                p[0] += base
            allpages.extend(got)
        return allpages, note
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


def main():
    rowsfile, oursdir, refdir, pageout, docout = sys.argv[1:6]
    jobs = int(sys.argv[6]) if len(sys.argv) > 6 else 4
    oursdir, refdir = pathlib.Path(oursdir), pathlib.Path(refdir)

    items = []
    for line in pathlib.Path(rowsfile).read_text(encoding='utf-8').splitlines():
        p = line.split('\t')
        if len(p) < 7 or p[6] != 'match':
            continue
        ident = f'{pathlib.PurePosixPath(p[0]).stem}__{p[1]}'
        items.append((ident, p[0], p[1], p[0].split('/')[0]))
    # Largest first, so the long tail of small documents fills the workers behind them rather
    # than four multi-thousand-page workbooks landing in flight together.
    items.sort(key=lambda it: -(refdir / f'{it[0]}.pdf').stat().st_size
               if (refdir / f'{it[0]}.pdf').exists() else 0)

    def one(it):
        ident, rel, ext, track = it
        o, f = oursdir / f'{ident}.pdf', refdir / f'{ident}.pdf'
        if not o.exists() or not f.exists():
            return it, None, 'missing'
        try:
            pages, note = score(o, f)
        except Exception as exc:                      # noqa: BLE001 - one bad pair must not kill the sweep
            return it, None, f'{type(exc).__name__}: {exc}'[:100]
        return it, pages, note

    results = []
    with cf.ThreadPoolExecutor(jobs) as ex:
        for n, r in enumerate(ex.map(one, items), 1):
            results.append(r)
            if n % 25 == 0:
                print(f'{n}/{len(items)}', flush=True)

    with open(pageout, 'w', encoding='utf-8') as pf, open(docout, 'w', encoding='utf-8') as df:
        pf.write('identity\text\ttrack\tpage\tnpages\tdiff\tink_signed\tink_abs\tregions'
                 '\tverdict\thint\n')
        df.write('identity\text\ttrack\tpath\tnpages\tink_sum\tink_per_page\tink_max_page'
                 '\tmax_page_no\tmajor_pages\tnote\tmax_page_hint\n')
        for (ident, rel, ext, track), pages, note in sorted(results, key=lambda x: x[0][0]):
            if pages is None:
                df.write(f'{ident}\t{ext}\t{track}\t{rel}\t\t\t\t\t\t\t{note}\t\n')
                continue
            n = len(pages)
            total = sum(p[3] for p in pages)
            mx = max(pages, key=lambda p: p[3])
            major = sum(1 for p in pages if p[5] == 'MAJOR')
            for p in pages:
                pf.write(f'{ident}\t{ext}\t{track}\t{p[0]}\t{n}\t{p[1]:.2f}\t{p[2]:.2f}'
                         f'\t{p[3]:.2f}\t{p[4]}\t{p[5]}\t{p[6]}\n')
            df.write(f'{ident}\t{ext}\t{track}\t{rel}\t{n}\t{total:.2f}\t{total / n:.4f}'
                     f'\t{mx[3]:.2f}\t{mx[0]}\t{major}\tok\t{mx[6]}\n')


main()
