#!/usr/bin/env python3
"""Which of the corpus's xlsx-family renderings move, byte for byte, between two builds.

Rendered under `SOURCE_DATE_EPOCH`, one output directory per document (never one per worker
slot), so a PDF is byte-comparable and two live renders cannot collide.

    o36-confine.py BEFOREDIR AFTERDIR
"""
import hashlib, os, sys

def digest(path):
    h = hashlib.sha256()
    with open(path, 'rb') as fh:
        for block in iter(lambda: fh.read(1 << 20), b''):
            h.update(block)
    return h.hexdigest()

def collect(root):
    out = {}
    for name in sorted(os.listdir(root)):
        d = os.path.join(root, name)
        if not os.path.isdir(d):
            continue
        pdfs = sorted(f for f in os.listdir(d) if f.endswith('.pdf'))
        out[name] = digest(os.path.join(d, pdfs[0])) if pdfs else None
    return out

def main():
    before, after = collect(sys.argv[1]), collect(sys.argv[2])
    names = sorted(set(before) | set(after))
    moved = [n for n in names if before.get(n) != after.get(n)]
    print('document\tbefore\tafter')
    for n in moved:
        print(f'{n}\t{(before.get(n) or "-")[:12]}\t{(after.get(n) or "-")[:12]}')
    print(f'# {len(names)} renderings, {len(moved)} moved, {len(names) - len(moved)} byte-identical',
          file=sys.stderr)

main()
