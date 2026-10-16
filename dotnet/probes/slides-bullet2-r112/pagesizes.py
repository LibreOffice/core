#!/usr/bin/env python3
"""Per-page dominant drawn text size, and the alphanumeric count behind it.

The register O15 scores is the page's dominant size: every span's alphanumeric
characters bucketed by its drawn size, largest bucket wins.  `slides-r107`'s
`sizescore.py` is the scorer; this is the reader, kept here so the three legs of
one page can be printed side by side without a sweep.

Usage: pagesizes.py A.pdf B.pdf [C.pdf ...]   -- prints one row per page.
"""
import sys, collections
import pymupdf


def page_sizes(path):
    out = []
    with pymupdf.open(path) as doc:
        for page in doc:
            counts = collections.Counter()
            for block in page.get_text('dict')['blocks']:
                if block['type'] != 0:
                    continue
                for line in block['lines']:
                    for span in line['spans']:
                        text = ''.join(c for c in span['text'] if c.isalnum())
                        if text:
                            counts[round(span['size'], 2)] += len(text)
            out.append(counts)
    return out


def main():
    legs = [page_sizes(p) for p in sys.argv[1:]]
    names = [p.split('/')[-1] for p in sys.argv[1:]]
    print('page\t' + '\t'.join(f'{n} dom/alnum' for n in names))
    for i in range(max(len(l) for l in legs)):
        cells = []
        for leg in legs:
            if i >= len(leg) or not leg[i]:
                cells.append('-')
                continue
            size, _ = leg[i].most_common(1)[0]
            cells.append(f'{size}/{sum(leg[i].values())}')
        if len(set(c.split("/")[0] for c in cells)) > 1:
            print(f'{i + 1}\t' + '\t'.join(cells))
    print('pages', [len(l) for l in legs])


if __name__ == '__main__':
    main()
