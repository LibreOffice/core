#!/usr/bin/env python3
"""Score a sheets-track rows.tsv: matches, page error, exact page counts, word error."""
import sys, os, re
from collections import defaultdict


def load(path):
    rows = {}
    for line in open(path):
        parts = line.rstrip('\n').split('\t')
        if len(parts) < 7:
            continue
        rows[parts[0]] = parts
    return rows


def score(rows, label):
    match = exact = 0
    page_err = word_err = 0
    per_batch = defaultdict(lambda: [0, 0])
    for path, p in sorted(rows.items()):
        batch = path.split('/')[1]
        per_batch[batch][1] += 1
        if p[6] == 'match':
            match += 1
            per_batch[batch][0] += 1
        m = re.match(r'(\d+)/(\d+)$', p[2])
        if m:
            ours, ref = int(m.group(1)), int(m.group(2))
            page_err += abs(ours - ref)
            if ours == ref:
                exact += 1
        w = re.match(r'(\d+)/(\d+)$', p[3])
        if w:
            word_err += abs(int(w.group(1)) - int(w.group(2)))
    print(f'{label}: {match}/{len(rows)} matches   abs page error {page_err}   '
          f'exact page counts {exact}   abs word error {word_err}')
    print('  per batch: ' + '  '.join(
        f'{b.split("-")[-1]} {v[0]}/{v[1]}' for b, v in sorted(per_batch.items())))
    return rows


if __name__ == '__main__':
    base = load(sys.argv[1])
    for splice in sys.argv[3:]:
        for k, v in load(splice).items():
            if k in base:
                base[k] = v
    score(base, sys.argv[2])
