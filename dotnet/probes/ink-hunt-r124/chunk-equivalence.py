#!/usr/bin/env python3
"""Check that scoring a pair in 25-page chunks gives the same |ink|% as scoring it whole.

    chunk-equivalence.py <ours.pdf> <ref.pdf>
"""
import pathlib, re, shutil, subprocess, sys, tempfile
import pymupdf

TOOL = '/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py'
ROW = re.compile(r'^(\d+)\t([\d.]+)\t(-?[\d.]+)\t([\d.]+)\t(\d+)\t(\S+)$')


def run(a, b, tmp):
    out = pathlib.Path(tmp) / 'o'
    r = subprocess.run([sys.executable, TOOL, str(a), str(b), '--outdir', str(out)],
                       capture_output=True, text=True)
    shutil.rmtree(out, ignore_errors=True)
    return [(int(m.group(1)), float(m.group(4)))
            for m in (ROW.match(x) for x in r.stdout.splitlines()) if m]


def sl(src, f, l, dst):
    with pymupdf.open(src) as d:
        o = pymupdf.open(); o.insert_pdf(d, from_page=f, to_page=l); o.save(dst); o.close()


ours, ref = sys.argv[1], sys.argv[2]
with pymupdf.open(ref) as d:
    n = d.page_count
with tempfile.TemporaryDirectory() as tmp:
    whole = run(ours, ref, tmp)
    chunked = []
    for base in range(0, n, 25):
        last = min(base + 25, n) - 1
        a, b = pathlib.Path(tmp) / 'a.pdf', pathlib.Path(tmp) / 'b.pdf'
        sl(ours, base, last, a); sl(ref, base, last, b)
        chunked += [(p + base, v) for p, v in run(a, b, tmp)]
print(f'{n} pages; whole {len(whole)} rows sum {sum(v for _, v in whole):.4f}; '
      f'chunked {len(chunked)} rows sum {sum(v for _, v in chunked):.4f}')
bad = [(p, x, y) for (p, x), (q, y) in zip(whole, chunked) if p != q or abs(x - y) > 1e-9]
print('per-page disagreements:', len(bad), bad[:5])
