#!/usr/bin/env python3
"""Render one half (ours or the reference) of the converted corpus's .odp column,
one output directory per document, and score it exactly as batch-check.sh does.

Environment is recorded in the header of the TSV this writes, because 253 of 256
stored TSVs in this repository do not record theirs.
"""
import argparse, concurrent.futures as cf, csv, hashlib, os, shutil, subprocess, sys, time
from pathlib import Path

def glyphs_of(pdf):
    try:
        out = subprocess.run(['pdftotext', str(pdf), '-'], capture_output=True, timeout=180).stdout
    except subprocess.TimeoutExpired:
        return None, None, None
    b = out.decode('utf-8', 'replace')
    t = b.split()
    return (sum(1 for w in t if any(c.isalnum() for c in w)), len(t),
            sum(1 for c in b if c.isalnum()))

def pages_of(pdf):
    try:
        out = subprocess.run(['pdfinfo', str(pdf)], capture_output=True, timeout=120).stdout.decode('utf-8','replace')
    except subprocess.TimeoutExpired:
        return None
    for line in out.splitlines():
        if line.startswith('Pages'): return int(line.split()[1])
    return None

def fonts_of(pdf):
    try:
        out = subprocess.run(['pdffonts', str(pdf)], capture_output=True, timeout=120).stdout.decode('utf-8','replace')
    except subprocess.TimeoutExpired:
        return 0, 0
    rows = [r for r in out.splitlines()[2:] if r.strip()]
    unemb = sum(1 for r in rows if len(r.split()) >= 8 and r.split()[-5] == 'no')
    return len(rows), unemb

def render_ours(cli, doc, outdir, timeout):
    r = subprocess.run([cli, 'render', str(doc), '--outdir', str(outdir)],
                       capture_output=True, timeout=timeout)
    return r.returncode

def render_ref(soffice, doc, outdir, timeout):
    prof = outdir / 'prof'
    r = subprocess.run([soffice, '--headless', '-env:UserInstallation=file://%s' % prof,
                        '--convert-to', 'pdf', '--outdir', str(outdir), str(doc)],
                       capture_output=True, timeout=timeout)
    shutil.rmtree(prof, ignore_errors=True)
    return r.returncode

def one(args, rel):
    doc = Path(args.corpus) / rel
    # Keyed on a hash of the document path, not on the path itself and not on a worker
    # slot. Two reasons, both paid for: a thread pool does not work consecutive indices, so
    # a slot index collides and one render rm -rf's another's output; and a corpus path
    # holding a space breaks `-env:UserInstallation=file://...`, which soffice truncates at
    # the space -- it then scatters profile directories through the sweep root while the
    # document silently fails to convert. A hex digest has neither problem.
    outdir = Path(args.out) / hashlib.sha1(rel.encode()).hexdigest()[:16]
    shutil.rmtree(outdir, ignore_errors=True)
    outdir.mkdir(parents=True, exist_ok=True)
    rc = -1
    try:
        if args.side == 'ours':
            rc = render_ours(args.cli, doc, outdir, args.timeout)
        else:
            rc = render_ref(args.soffice, doc, outdir, args.timeout)
    except subprocess.TimeoutExpired:
        rc = 124
    pdfs = sorted(outdir.glob('*.pdf'))
    if not pdfs:
        return [rel, 'FAILED', rc, '', '', '', '', '']
    pdf = pdfs[0]
    p = pages_of(pdf)
    w, raw, g = glyphs_of(pdf)
    f, un = fonts_of(pdf)
    if args.keep:
        keep = Path(args.keep); keep.mkdir(parents=True, exist_ok=True)
        shutil.copy2(pdf, keep / (rel.replace('/', '__')))
    shutil.rmtree(outdir, ignore_errors=True)
    return [rel, 'OK', rc, p, w, raw, g, f, un]

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--corpus', required=True)
    ap.add_argument('--list', required=True, help='file of corpus-relative paths')
    ap.add_argument('--side', choices=['ours', 'ref'], required=True)
    ap.add_argument('--cli', default='')
    ap.add_argument('--soffice', default='/opt/libreoffice26.2/program/soffice')
    ap.add_argument('--out', required=True)
    ap.add_argument('--tsv', required=True)
    ap.add_argument('--jobs', type=int, default=4)
    ap.add_argument('--timeout', type=int, default=240)
    ap.add_argument('--keep', default='')
    args = ap.parse_args()

    rels = [l.strip() for l in open(args.list) if l.strip()]
    Path(args.out).mkdir(parents=True, exist_ok=True)

    binary = args.cli if args.side == 'ours' else args.soffice
    ver = ''
    if args.side == 'ref':
        ver = subprocess.run([args.soffice, '--version'], capture_output=True).stdout.decode().strip()
    else:
        ver = subprocess.run(['git', '-C', str(Path(args.cli).resolve().parents[5]), 'rev-parse', 'HEAD'],
                             capture_output=True).stdout.decode().strip()

    rows = []
    t0 = time.time()
    with cf.ThreadPoolExecutor(max_workers=args.jobs) as ex:
        futs = {ex.submit(one, args, r): r for r in rels}
        done = 0
        for fu in cf.as_completed(futs):
            rows.append(fu.result())
            done += 1
            if done % 25 == 0:
                print('%d/%d  %.0fs' % (done, len(rels), time.time() - t0), file=sys.stderr, flush=True)

    rows.sort()
    with open(args.tsv, 'w', newline='') as fh:
        fh.write('# side=%s binary=%s version=%s\n' % (args.side, binary, ver))
        w = csv.writer(fh, delimiter='\t')
        w.writerow(['path', 'status', 'rc', 'pages', 'words', 'rawwords', 'glyphs', 'fonts', 'unemb'])
        w.writerows(rows)
    ok = sum(1 for r in rows if r[1] == 'OK')
    print('TOTAL %d rows, %d rendered, %d failed, %.0fs' % (len(rows), ok, len(rows) - ok, time.time() - t0))

if __name__ == '__main__':
    main()
