#!/usr/bin/env python3
"""Re-score our half of the whole 947-document corpus against a banked reference.

The change under test is confined to `dotnet/src`, which cannot reach `soffice`, so the
reference bytes of `/home/user/gate-orig-r83` are the same bytes this run would produce
and re-rendering them buys nothing.  Each of our PDFs is scored and then deleted: this
box has under half a gigabyte free, and 947 renderings do not fit on it.
"""
import re, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

import os
CORPUS = Path(os.environ.get('CORPUS', '/home/user/sample-files'))
BANK = Path(os.environ.get('BANK', '/home/user/gate-orig-r83/ref'))
ALNUM = re.compile(r'[^\W_]', re.UNICODE)


def counts(pdf):
    if not pdf.exists():
        return None, None
    p = subprocess.run(['pdfinfo', str(pdf)], capture_output=True, text=True)
    pages = next((int(l.split()[1]) for l in p.stdout.splitlines()
                  if l.startswith('Pages')), None)
    t = subprocess.run(['pdftotext', str(pdf), '-'], capture_output=True, text=True)
    return pages, len(ALNUM.findall(t.stdout))


def identity(rel):
    return f'{Path(rel).stem}__{Path(rel).suffix.lstrip(".").lower()}'


def one(cli, rel, out):
    ident = identity(rel)
    work = out / ident
    work.mkdir(parents=True, exist_ok=True)
    subprocess.run(['timeout', '-k', '30', '240', cli, 'render', str(CORPUS / rel),
                    '--format', 'pdf', '--outdir', str(work)], capture_output=True)
    got = sorted(work.glob('*.pdf'))
    op, og = counts(got[0]) if got else (None, None)
    rp, rg = counts(BANK / f'{ident}.pdf')
    for f in work.glob('*'):
        f.unlink()
    work.rmdir()

    if rp is None:
        return f'{rel}\t{op}/-\t{og}/-\tref-missing'
    if op is None:
        return f'{rel}\t-/{rp}\t-/{rg}\tours-failed'
    verdict = []
    if op != rp:
        verdict.append('pages')
    if rg > 0:
        if abs(og - rg) > max(rg * 0.02, 15):
            verdict.append('words')
    elif og > 15:
        verdict.append('words')
    return f'{rel}\t{op}/{rp}\t{og}/{rg}\t{",".join(verdict) or "match"}'


def main():
    cli, listing, out, jobs = sys.argv[1], Path(sys.argv[2]), Path(sys.argv[3]), int(sys.argv[4])
    rels = [l.strip() for l in listing.read_text().splitlines() if l.strip()]
    with ThreadPoolExecutor(jobs) as ex:
        for row in ex.map(lambda r: one(cli, r, out), rels):
            print(row, flush=True)


if __name__ == '__main__':
    main()
