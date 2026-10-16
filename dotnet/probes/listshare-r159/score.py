#!/usr/bin/env python3
"""Score every document a sweep moved against the 26.2.4.2 bank, before and after.

Pages and alphanumeric characters, which is what the gate reads, plus its verdict:
`match` when the page counts agree and the character difference is under both 2 % and 15.
"""
import pathlib, sys, pymupdf

BANK = {p.stem: p for p in pathlib.Path('/home/user/refpdfs-words-26.2.4.2').rglob('*.pdf')}


def counts(path):
    d = pymupdf.open(path)
    return len(d), sum(sum(1 for c in p.get_text() if c.isalnum()) for p in d)


def verdict(ref, ours):
    if ref[0] != ours[0]:
        return 'pages'
    delta = abs(ref[1] - ours[1])
    return 'match' if not (delta > ref[1] * 0.02 and delta > 15) else 'glyphs'


def main(before_root, after_root, rows):
    moved = [line.split('\t')[1].strip()
             for line in pathlib.Path(rows).read_text().splitlines()
             if line.startswith('MOVED')]
    print(f'{"document":58} {"ref":>10} {"before":>12} {"after":>12}  verdict')
    for stem in moved:
        ref = BANK.get(stem)
        if ref is None:
            print(f'{stem[:58]:58} (not in the reference bank)')
            continue
        r = counts(ref)
        b = next(iter(pathlib.Path(before_root).rglob(f'{stem}.pdf')), None)
        a = next(iter(pathlib.Path(after_root).rglob(f'{stem}.pdf')), None)
        bc, ac = counts(b), counts(a)
        print(f'{stem[:58]:58} {r[0]:4}p{r[1]:6} {bc[0]:4}p{bc[1]:7} {ac[0]:4}p{ac[1]:7}  '
              f'{verdict(r, bc)} -> {verdict(r, ac)}')


if __name__ == '__main__':
    main(*sys.argv[1:4])
