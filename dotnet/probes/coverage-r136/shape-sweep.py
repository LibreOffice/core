#!/usr/bin/env python3
"""Render our half of the corpus and measure each page's SHAPE against the banked reference.

    shape-sweep.py <corpus-root> <rows.tsv> <ref-bank> <outdir> [jobs]

Ink is deliberately not computed.  Round 124 ranked on summed |ink|% and its two most
valuable documents scored 0.71 and 1.67, because a pale frame drawn 8x too wide is almost
no ink at all.  This measures instead, per page and with no rasterisation:

    pages            how many, on each side
    page size        the media box, to a tenth of a point -- the quantity whose non-numeric
                     row `pdf-image-diff.py` silently drops (r124 s5)
    characters       `get_text` with whitespace removed, per page, both sides
    drawings         `page.get_drawings()` -- one entry per path construction
    fills / strokes  the same, split by what the path actually paints
    images           `page.get_image_info()` COUNT only.  Round 128 established that the
                     BOX it reports is the image XObject's declared placement rectangle,
                     which for a cropped picture is deliberately far larger than the frame;
                     the count of placements is not affected by that and the box is not read
                     here.

Our PDF is deleted as soon as it is measured, so the sweep's peak disk cost is one document.
"""
import concurrent.futures
import os
import pathlib
import subprocess
import sys
import time

import pymupdf

CLI = os.environ["XRAY_OFFICE_CLI"]
EPOCH = os.environ.get("SOURCE_DATE_EPOCH", "1757462400")

COLS = ('pages', 'chars', 'draws', 'fills', 'strokes', 'images')


def measure(path):
    """Per-page tuples for one PDF, or None if it cannot be opened."""
    try:
        doc = pymupdf.open(path)
    except Exception:
        return None
    out = []
    for page in doc:
        r = page.rect
        try:
            txt = page.get_text('text')
        except Exception:
            txt = ''
        chars = len(''.join(txt.split()))
        fills = strokes = 0
        try:
            dr = page.get_drawings()
        except Exception:
            dr = []
        for d in dr:
            t = d.get('type', '')
            if t in ('f', 'fs'):
                fills += 1
            if t in ('s', 'fs'):
                strokes += 1
        try:
            imgs = len(page.get_image_info())
        except Exception:
            imgs = 0
        out.append((round(r.width, 1), round(r.height, 1), chars, len(dr), fills, strokes, imgs))
    doc.close()
    return out


def render(args):
    root, rel, ext, refbank, outdir = args
    ident = f'{pathlib.PurePosixPath(rel).stem}__{ext}'
    work = pathlib.Path(outdir) / 'work' / ident
    work.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH=EPOCH)
    ours = None
    status = 'ok'
    try:
        r = subprocess.run([CLI, 'render', str(pathlib.Path(root) / rel), '--outdir', str(work)],
                           capture_output=True, timeout=1200, env=env, check=False)
    except subprocess.TimeoutExpired:
        status = 'timeout'
    else:
        pdfs = sorted(work.glob('*.pdf'))
        if r.returncode != 0 or not pdfs:
            status = 'ours-failed'
        else:
            ours = measure(pdfs[0])
            if ours is None:
                status = 'ours-unreadable'
    # Defensive: one document's render left this directory gone under us, and an
    # unguarded cleanup then killed the whole sweep 321 rows in.
    try:
        for f in work.iterdir():
            f.unlink()
        work.rmdir()
    except OSError:
        pass

    refpdf = pathlib.Path(refbank) / f'{ident}.pdf'
    ref = measure(refpdf) if refpdf.exists() else None
    if ref is None and status == 'ok':
        status = 'ref-missing'
    return ident, rel, ext, status, ours, ref


def main():
    root, rowsfile, refbank, outdir = sys.argv[1:5]
    jobs = int(sys.argv[5]) if len(sys.argv) > 5 else 3

    items = []
    for line in pathlib.Path(rowsfile).read_text(encoding='utf-8').splitlines():
        p = line.split('\t')
        if len(p) < 2:
            continue
        items.append((root, p[0], p[1], refbank, outdir))

    outdir = pathlib.Path(outdir)
    outdir.mkdir(parents=True, exist_ok=True)
    started, done = time.time(), 0
    fp = open(outdir / 'pages.tsv', 'w', encoding='utf-8')
    fd = open(outdir / 'docs.tsv', 'w', encoding='utf-8')
    fp.write('identity\tpage\tw_ours\th_ours\tw_ref\th_ref\t' +
             '\t'.join(f'{c}_ours\t{c}_ref' for c in COLS[1:]) + '\n')
    fd.write('identity\tpath\text\tstatus\tpages_ours\tpages_ref\tsize_diff_pages\t' +
             '\t'.join(f'{c}_ours\t{c}_ref' for c in COLS[1:]) + '\n')
    with concurrent.futures.ThreadPoolExecutor(jobs) as pool:
        for ident, rel, ext, status, ours, ref in pool.map(render, items):
            po, pr = (len(ours) if ours else 0), (len(ref) if ref else 0)
            tot_o = [0] * 5
            tot_r = [0] * 5
            sized = 0
            if ours and ref:
                for i in range(max(po, pr)):
                    a = ours[i] if i < po else None
                    b = ref[i] if i < pr else None
                    if a and b and (a[0], a[1]) != (b[0], b[1]):
                        sized += 1
                    fp.write('\t'.join([ident, str(i + 1)] +
                                       [f'{a[0]}' if a else '', f'{a[1]}' if a else '',
                                        f'{b[0]}' if b else '', f'{b[1]}' if b else ''] +
                                       [x for k in range(2, 7)
                                        for x in (str(a[k]) if a else '', str(b[k]) if b else '')]
                                       ) + '\n')
                for a in ours:
                    for k in range(2, 7):
                        tot_o[k - 2] += a[k]
                for b in ref:
                    for k in range(2, 7):
                        tot_r[k - 2] += b[k]
            fd.write('\t'.join([ident, rel, ext, status, str(po), str(pr), str(sized)] +
                               [str(x) for pair in zip(tot_o, tot_r) for x in pair]) + '\n')
            fp.flush()
            fd.flush()
            done += 1
            if done % 50 == 0:
                print(f'{done}/{len(items)}  {time.time() - started:.0f}s', flush=True)
    fp.close()
    fd.close()
    print(f'TOTAL {done} of {len(items)} in {time.time() - started:.0f}s')


main()
