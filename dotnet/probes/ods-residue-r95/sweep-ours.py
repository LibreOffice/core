#!/usr/bin/env python3
"""Render our half of a column and score it against a banked 26.2.4.2 reference.

The gate's own rule, transcribed from `batch-check.sh` of 2026-09-05: a row is a `match`
when the page counts agree, the alphanumeric character counts are within max(2 %, 15), and
our PDF names no unembedded font. Column 9, `glyphs`, is what decides the text half.

Reusing a bank is sound exactly when the diff under test cannot reach `soffice`, which a
change confined to `dotnet/src` cannot.

Usage: sweep-ours.py <corpus-root> <bank-ref-dir> <outdir> [jobs]

Writes `<outdir>/rows.tsv` with batch-check.sh's columns and deletes each rendering once it
has been scored, so a whole column costs one PDF of disk at a time per worker.
"""
import os
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

SUFFIXES = {'.ods', '.fods', '.ots', '.xlsx', '.xls', '.xlsm', '.xlsb', '.xltx', '.csv',
            '.docx', '.doc', '.rtf', '.odt', '.pptx', '.ppt', '.odp'}


def counts(pdf):
    """(pages, glyphs, fonts, unembedded) of a PDF, or None when it is not readable."""
    try:
        info = subprocess.run(['pdfinfo', str(pdf)], capture_output=True, text=True, timeout=120)
        pages = next((int(line.split()[1]) for line in info.stdout.splitlines()
                      if line.startswith('Pages')), None)
        text = subprocess.run(['pdftotext', str(pdf), '-'],
                              capture_output=True, timeout=300).stdout.decode('utf8', 'replace')
        glyphs = sum(1 for c in text if c.isalnum())
        fonts = subprocess.run(['pdffonts', str(pdf)], capture_output=True, text=True, timeout=120)
        rows = [r for r in fonts.stdout.splitlines()[2:] if r.strip()]
        unembedded = sum(1 for r in rows if len(r.split()) >= 8 and r.split()[-5] == 'no')
    except Exception:                               # noqa: BLE001
        return None
    if pages is None:
        return None
    return pages, glyphs, len(rows), unembedded


def verdict(ours, ref):
    if ref is None and ours is None:
        return 'both-failed'
    if ref is None:
        return 'ref-failed'
    if ours is None:
        return 'ours-failed'
    op, og, _, un = ours
    rp, rg, _, _ = ref
    reasons = []
    if op != rp:
        reasons.append('pages')
    if rg > 0:
        if abs(og - rg) > rg * 0.02 and abs(og - rg) > 15:
            reasons.append('words')
    elif og > 15:
        reasons.append('words')
    if un:
        reasons.append('unembedded')
    return ','.join(reasons) or 'match'


def main():
    root = Path(sys.argv[1])
    bank = Path(sys.argv[2])
    out = Path(sys.argv[3])
    jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 3
    cli = os.environ['XRAY_OFFICE_CLI']

    out.mkdir(parents=True, exist_ok=True)
    rows = out / 'rows.tsv'
    documents = sorted(p for p in root.rglob('*')
                       if p.is_file() and p.suffix.lower() in SUFFIXES)

    env = dict(os.environ, SOURCE_DATE_EPOCH='1700000000')
    lines = []

    def one(path):
        identity = f'{path.stem}__{path.suffix.lower().lstrip(".")}'
        work = out / 'work' / identity
        work.mkdir(parents=True, exist_ok=True)
        subprocess.run(['timeout', '-k', '30', '600', cli, 'render', str(path),
                        '--format', 'pdf', '--outdir', str(work)],
                       capture_output=True, env=env)
        produced = work / f'{path.stem}.pdf'
        ours = counts(produced) if produced.exists() else None
        banked = bank / f'{identity}.pdf'
        ref = counts(banked) if banked.exists() else None
        if produced.exists():
            produced.unlink()
        try:
            work.rmdir()
        except OSError:
            pass
        op, og, of, un = ours if ours else ('-', '-', '-', '-')
        rp, rg, rf, _ = ref if ref else ('-', '-', '-', '-')
        return (f'{path.relative_to(root)}\t{path.suffix.lower().lstrip(".")}\t{op}/{rp}\t'
                f'-/-\t{of}/{rf}\t{un}\t{verdict(ours, ref)}\t-/-\t{og}/{rg}')

    with ThreadPoolExecutor(max_workers=jobs) as pool:
        for line in pool.map(one, documents):
            lines.append(line)

    rows.write_text('\n'.join(lines) + '\n', encoding='utf8')
    tally = {}
    for line in lines:
        tally[line.split('\t')[6]] = tally.get(line.split('\t')[6], 0) + 1
    print(f'TOTAL {len(lines)}  ' + '  '.join(f'{k} {v}' for k, v in sorted(tally.items())))


if __name__ == '__main__':
    main()
