#!/usr/bin/env python3
"""Compare the `/Tf` sizes two renderings of the same deck actually use, page by page.

The autofit search's whole output is a font multiplier, so a disagreement about it is
visible in the content stream as a different `/Tf` size on the same page — and *only*
there, because `pdftotext` reports the same words either way and the image diff sees a
reflow it cannot attribute. This counts show operators per rounded size, per page, on
both sides, and reports the pages whose distributions differ.

    size-census.py <sweepdir>                     every document in a batch-check outdir
    size-census.py <ours.pdf> <ref.pdf>           one pair, per page
    size-census.py <sweepdir> --filter Civil      only documents whose id matches

Why counts of show operators rather than distinct sizes: a deck sets its furniture — page
numbers, footers, a logo caption — at sizes no autofit touches, and those are the majority
of the distinct sizes on a slide while being a handful of the operators. Weighting by
operator count puts the body text first, which is what the search decides.

`ours > ref` on the dominant size is the signature of the search settling on a larger font
than the reference did; `ours < ref` is the reverse. Both are reported, because the round
that assumed the sign was fixed had it backwards.
"""
import argparse
import collections
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', 'slides-r15'))
from pdfops import dump  # noqa: E402


def sizes(path):
    """page number -> Counter of rounded /Tf size -> show-operator count."""
    out = {}
    page = 1
    while True:
        try:
            ops = dump(path, page)
        except (IndexError, KeyError, ValueError, ZeroDivisionError):
            break
        counts = collections.Counter()
        for font, _base, sz, _tm, _td in ops:
            if font == 'TL' or not sz:
                continue
            counts[round(sz, 2)] += 1
        out[page] = counts
        page += 1
        if page > 4000:
            break
    return out


def dominant(counts):
    """The size carrying the most show operators, and its share."""
    if not counts:
        return (0.0, 0, 0)
    size, n = counts.most_common(1)[0]
    return (size, n, sum(counts.values()))


def compare(ours, ref, label, verbose=False):
    a, b = sizes(ours), sizes(ref)
    pages = sorted(set(a) & set(b))
    rows = []
    for p in pages:
        da, db = dominant(a[p]), dominant(b[p])
        if abs(da[0] - db[0]) < 0.01:
            continue
        rows.append((p, da, db))
    if rows:
        print(f'{label}: {len(rows)} of {len(pages)} pages differ on the dominant size')
        for p, da, db in rows:
            direction = 'ours LARGER ' if da[0] > db[0] else 'ours smaller'
            print(f'  page {p:4d}  ours {da[0]:7.3f} x{da[1]:<4d}  '
                  f'ref {db[0]:7.3f} x{db[1]:<4d}  {direction} '
                  f'({(da[0] / db[0] - 1) * 100:+.2f}%)')
            if verbose:
                print(f'      ours {sorted(a[p].items(), reverse=True)}')
                print(f'      ref  {sorted(b[p].items(), reverse=True)}')
    return len(rows), len(pages)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('a')
    ap.add_argument('b', nargs='?')
    ap.add_argument('--filter', default='')
    ap.add_argument('--verbose', action='store_true')
    args = ap.parse_args()

    if args.b:
        compare(args.a, args.b, os.path.basename(args.a), args.verbose)
        return

    ours_dir = os.path.join(args.a, 'ours')
    ref_dir = os.path.join(args.a, 'ref')
    total_pages = total_diff = docs = 0
    for name in sorted(os.listdir(ours_dir)):
        if not name.endswith('.pdf') or args.filter not in name:
            continue
        r = os.path.join(ref_dir, name)
        if not os.path.exists(r):
            continue
        try:
            d, p = compare(os.path.join(ours_dir, name), r, name[:-4], args.verbose)
        except Exception as exc:                       # noqa: BLE001 — a probe, not a library
            print(f'{name[:-4]}: unreadable ({exc})')
            continue
        docs += 1
        total_diff += d
        total_pages += p
    print(f'\nTOTAL {total_diff} of {total_pages} pages differ on the dominant '
          f'/Tf size, over {docs} documents')


if __name__ == '__main__':
    main()
