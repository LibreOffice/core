#!/usr/bin/env python3
"""Per-page extractable-word counts for one document, ours against the reference.

The word gate reports one number per document, which cannot distinguish "we draw a whole
page the reference rasterises" from "we lose a little text everywhere". This splits it.

    page-words.py <sweepdir> <id-substring>
"""
import os
import subprocess
import sys


def words(path, page):
    out = subprocess.run(['pdftotext', '-f', str(page), '-l', str(page), path, '-'],
                         capture_output=True, check=False)
    return len(out.stdout.split())


def pages(path):
    out = subprocess.run(['pdfinfo', path], capture_output=True, check=False)
    for line in out.stdout.decode('utf-8', 'replace').splitlines():
        if line.startswith('Pages:'):
            return int(line.split()[1])
    return 0


def main(sweepdir, needle):
    ours_dir = os.path.join(sweepdir, 'ours')
    for name in sorted(os.listdir(ours_dir)):
        if not name.endswith('.pdf') or needle not in name:
            continue
        o = os.path.join(ours_dir, name)
        r = os.path.join(sweepdir, 'ref', name)
        if not os.path.exists(r):
            continue
        n = pages(o)
        rows = [(p, words(o, p), words(r, p)) for p in range(1, n + 1)]
        over = sum(max(0, a - b) for _, a, b in rows)
        under = sum(max(0, b - a) for _, a, b in rows)
        print(f'\n{name[:-4]}  {n} pages   over-draw {over}   under-draw {under}')
        for p, a, b in sorted(rows, key=lambda t: -(t[1] - t[2])):
            if a - b == 0:
                continue
            print(f'  page {p:4d}  ours {a:5d}  ref {b:5d}  {a - b:+5d}')


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else '')
