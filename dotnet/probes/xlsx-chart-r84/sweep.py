#!/usr/bin/env python3
"""Render a list of workbooks through X-Ray.Office and through the reference, and score
each pair by the gate's own rule.

One output directory per *document*, keyed on an MD5 of its path — the trap
`dotnet/CLAUDE.md` records — and the LibreOffice profile is keyed on the same digest,
because `soffice` truncates `-env:UserInstallation` at the first space.
"""
import hashlib, os, re, shutil, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

REF = os.environ.get('REF_SOFFICE', '/opt/libreoffice26.2/program/soffice')
ALNUM = re.compile(r'[^\W_]', re.UNICODE)


def counts(pdf):
    if not pdf.exists():
        return None, None
    p = subprocess.run(['pdfinfo', str(pdf)], capture_output=True, text=True)
    pages = next((int(l.split()[1]) for l in p.stdout.splitlines()
                  if l.startswith('Pages')), None)
    t = subprocess.run(['pdftotext', str(pdf), '-'], capture_output=True, text=True)
    return pages, len(ALNUM.findall(t.stdout))


def one(cli, doc, out):
    d = hashlib.md5(str(doc).encode()).hexdigest()
    work = out / d
    work.mkdir(parents=True, exist_ok=True)
    ours, ref = out / 'ours' / f'{d}.pdf', out / 'ref' / f'{d}.pdf'
    for p in (ours.parent, ref.parent):
        p.mkdir(parents=True, exist_ok=True)
    if not ours.exists():
        subprocess.run(['timeout', '-k', '30', '240', cli, 'render', str(doc),
                        '--format', 'pdf', '--outdir', str(work)],
                       capture_output=True)
        got = list(work.glob('*.pdf'))
        if got:
            got[0].replace(ours)
    for f in work.glob('*.pdf'):
        f.unlink()
    prof = Path(f'/tmp/plo-{d}')
    if not ref.exists():
        subprocess.run(['timeout', '-k', '30', '240', REF,
                        f'-env:UserInstallation=file://{prof}',
                        '--headless', '--convert-to', 'pdf',
                        '--outdir', str(work), str(doc)], capture_output=True)
        got = list(work.glob('*.pdf'))
        if got:
            got[0].replace(ref)
    # The box this runs on has under 200 MB free; a LibreOffice profile is ten of
    # those and there is one per document, so it goes as soon as the render is done.
    shutil.rmtree(prof, ignore_errors=True)
    shutil.rmtree(work, ignore_errors=True)
    op, og = counts(ours)
    rp, rg = counts(ref)
    band = max((rg or 0) * 0.02, 15)
    verdict = []
    if op != rp:
        verdict.append('pages')
    if rg and abs((og or 0) - rg) > band:
        verdict.append('words')
    return f'{doc.name}\t{op}/{rp}\t{og}/{rg}\t{(og or 0)-(rg or 0)}\t{band:.1f}\t' \
           f'{",".join(verdict) or "match"}'


def main():
    cli, listing, out, jobs = sys.argv[1], Path(sys.argv[2]), Path(sys.argv[3]), int(sys.argv[4])
    docs = sorted(listing.glob('*.xlsx')) if listing.is_dir() else \
        [Path(l.strip()) for l in listing.read_text().splitlines() if l.strip()]
    with ThreadPoolExecutor(jobs) as ex:
        for row in ex.map(lambda d: one(cli, d, out), docs):
            print(row, flush=True)


if __name__ == '__main__':
    main()
