#!/usr/bin/env python3
"""Score two banked PDF directories with `batch-check.sh`'s own verdict rule.

    score-bank.py <corpus-root> <glob> <ours-dir> <ref-dir> <out.tsv>

Used to re-derive the *base* column without rendering anything, since
`/home/user/gate-orig-r83/ours` already holds this round's base commit's own output for all
947 documents. Columns match `sweep-ours.sh`'s so the two are comparable side by side; the
`moved` column is `-`, there being nothing to compare against.

The verdict is on `glyphs` (alphanumeric *characters*), not on `words` (letter-or-digit
tokens), with the band `max(2%, 15)` — `batch-check.sh`:279 and the block above it.
"""
import glob as globmod
import os
import subprocess
import sys


def counts(path):
    if not os.path.exists(path):
        return None
    pages = subprocess.run(['pdfinfo', path], capture_output=True, text=True).stdout
    n = next((int(l.split()[1]) for l in pages.splitlines() if l.startswith('Pages')), 0)
    text = subprocess.run(['pdftotext', path, '-'], capture_output=True).stdout
    body = text.decode('utf-8', 'replace')
    tokens = body.split()
    words = sum(1 for w in tokens if any(c.isalnum() for c in w))
    glyphs = sum(1 for c in body if c.isalnum())
    fonts = subprocess.run(['pdffonts', path], capture_output=True, text=True).stdout.splitlines()
    rows = [r for r in fonts[2:] if r.strip()]
    unembedded = sum(1 for r in rows
                     if len(r.split()) >= 8 and r.split()[-5] == 'no')
    return n, words, len(tokens), glyphs, len(rows), unembedded


def verdict(ours, ref):
    if ours is None and ref is None:
        return 'both-failed'
    if ref is None:
        return 'ref-failed'
    if ours is None:
        return 'ours-failed'
    parts = []
    if ours[0] != ref[0]:
        parts.append('pages')
    if ref[3] > 0:
        d = abs(ours[3] - ref[3])
        if d > ref[3] * 0.02 and d > 15:
            parts.append('words')
    elif ours[3] > 15:
        parts.append('words')
    if ours[5]:
        parts.append('unembedded')
    return ','.join(parts) if parts else 'match'


def main(root, pattern, ours_dir, ref_dir, out):
    seen = set()
    files = []
    for path in sorted(globmod.glob(os.path.join(root, pattern))):
        if not os.path.isfile(path):
            continue
        key = os.stat(path).st_ino
        if key in seen:
            continue
        seen.add(key)
        files.append(path)

    tally = {}
    with open(out, 'w') as handle:
        for path in files:
            base = os.path.basename(path)
            stem, ext = os.path.splitext(base)
            ident = f'{stem}__{ext[1:].lower()}'
            a = counts(os.path.join(ours_dir, ident + '.pdf'))
            b = counts(os.path.join(ref_dir, ident + '.pdf'))
            v = verdict(a, b)
            tally[v] = tally.get(v, 0) + 1
            fmt = lambda c, i: ('-' if c is None else c[i])
            handle.write('\t'.join(str(x) for x in [
                os.path.relpath(path, root), ext[1:].lower(),
                f'{fmt(a,0)}/{fmt(b,0)}', f'{fmt(a,1)}/{fmt(b,1)}',
                f'{fmt(a,4)}/{fmt(b,4)}', fmt(a, 5), v,
                f'{fmt(a,2)}/{fmt(b,2)}', f'{fmt(a,3)}/{fmt(b,3)}', '-']) + '\n')
    for k in sorted(tally):
        print(k, tally[k])
    print('TOTAL', sum(tally.values()))


if __name__ == '__main__':
    main(*sys.argv[1:6])
