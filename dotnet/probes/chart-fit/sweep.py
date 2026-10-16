#!/usr/bin/env python3
"""Re-score the whole corpus against the banked gate at `/home/user/gate-2f47/`.

Only our half is rendered: the diff under test is confined to `dotnet/src` and cannot
reach `soffice`, so the reference columns are the bank's own. The verdict rule is
`batch-check.sh`:260-284 — pages, then glyphs inside max(2%, 15), then unembedded fonts —
and it is validated against the bank's stored verdicts before anything is scored.

Each document renders into its own directory, keyed on a hex digest of its corpus-relative
path, and the directory is removed as soon as the row is scored.

    sweep.py --cli <XRay.Office.Cli> --out <dir> > sweep.tsv
"""
import argparse, concurrent.futures as cf, csv, hashlib, shutil, subprocess, sys, tempfile
from pathlib import Path

ap = argparse.ArgumentParser()
ap.add_argument('--cli', required=True)
ap.add_argument('--out', required=True)
ap.add_argument('--gate', default='/home/user/gate-2f47')
ap.add_argument('--corpus', default='/home/user/sample-files')
ap.add_argument('--jobs', type=int, default=3)
ap.add_argument('--only', default=None, help='a file of paths to render; the rest reuse the bank')
args = ap.parse_args()

CORPUS = Path(args.corpus)
GATE = Path(args.gate)
OUT = Path(args.out)
OUT.mkdir(parents=True, exist_ok=True)


def counts(pdf):
    """(pages, glyphs, unembedded) of one PDF, exactly as batch-check.sh reads them."""
    info = subprocess.run(['pdfinfo', str(pdf)], capture_output=True, text=True).stdout
    pages = next((int(l.split()[1]) for l in info.splitlines() if l.startswith('Pages:')), 0)
    text = subprocess.run(['pdftotext', str(pdf), '-'], capture_output=True).stdout
    body = text.decode('utf-8', 'replace')
    glyphs = sum(1 for c in body if c.isalnum())
    fonts = subprocess.run(['pdffonts', str(pdf)], capture_output=True, text=True).stdout
    unemb = 0
    for line in fonts.splitlines()[2:]:
        parts = line.split()
        if len(parts) >= 8 and parts[-5] == 'no':
            unemb += 1
    return pages, glyphs, unemb


def verdict(op, rp, og, rg, un):
    v = []
    if op != rp:
        v.append('pages')
    if rg > 0:
        if abs(og - rg) > rg * 0.02 and abs(og - rg) > 15:
            v.append('words')
    elif og > 15:
        v.append('words')
    if un:
        v.append('unembedded')
    return ','.join(v) if v else 'match'


# ---- validate the rule against the bank's own stored columns ---------------
stored = {}
with open(GATE / 'parity.tsv') as fh:
    for line in fh:
        if line.startswith('#') or line.startswith('path\t'):
            continue
        f = line.rstrip('\n').split('\t')
        if len(f) < 9:
            continue
        op, rp = f[2].split('/')
        og, rg = f[8].split('/')
        stored[f[0]] = dict(op=op, rp=rp, og=og, rg=rg, un=f[5], verdict=f[6])

agree = sum(
    1 for p, r in stored.items()
    if r['op'] != '-' and r['og'] != '-'
    and verdict(int(r['op']), int(r['rp']), int(r['og']), int(r['rg']), int(r['un']))
    == r['verdict'])
print(f'# rule reproduces {agree} of {len(stored)} banked verdicts', file=sys.stderr)
if agree != len(stored):
    print('# REFUSING TO SCORE: the rule does not reproduce the bank', file=sys.stderr)
    sys.exit(1)

paths = [
    row['path'] for row in csv.DictReader(open(CORPUS / 'MANIFEST.tsv'), delimiter='\t')]
only = None
if args.only:
    only = {l.strip() for l in open(args.only) if l.strip()}


def score(rel):
    row = stored.get(rel)
    if row is None:
        return rel, 'NOT-IN-BANK', None
    if only is not None and rel not in only:
        return rel, row['verdict'], row
    src = CORPUS / rel
    work = OUT / hashlib.sha1(rel.encode()).hexdigest()[:16]
    work.mkdir(parents=True, exist_ok=True)
    try:
        subprocess.run([args.cli, 'render', str(src), '--format', 'pdf', '--outdir', str(work)],
                       capture_output=True, timeout=900)
        pdf = work / (src.stem + '.pdf')
        if not pdf.exists():
            return rel, 'ours-failed', row
        op, og, un = counts(pdf)
    except subprocess.TimeoutExpired:
        return rel, 'ours-failed', row
    finally:
        shutil.rmtree(work, ignore_errors=True)
    if row['rp'] == '-':
        return rel, 'ref-failed', row
    return rel, verdict(op, int(row['rp']), og, int(row['rg']), un), dict(row, new=(op, og, un))


print('path\tbanked\tnow\tpages_ref\tglyphs_ref\tpages_ours\tglyphs_ours\tunemb')
moved = 0
with cf.ThreadPoolExecutor(max_workers=args.jobs) as pool:
    for rel, now, row in pool.map(score, paths):
        if row is None:
            print(f'{rel}\t-\t{now}\t-\t-\t-\t-\t-')
            continue
        new = row.get('new')
        print(f'{rel}\t{row["verdict"]}\t{now}\t{row["rp"]}\t{row["rg"]}'
              f'\t{new[0] if new else row["op"]}\t{new[1] if new else row["og"]}'
              f'\t{new[2] if new else row["un"]}')
        if now != row['verdict']:
            moved += 1
        sys.stdout.flush()
print(f'# {moved} verdicts differ from the bank', file=sys.stderr)
