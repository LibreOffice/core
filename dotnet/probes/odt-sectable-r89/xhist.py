#!/usr/bin/env python3
"""Every text line of a PDF as (page, x, y, text), and the histogram of its line starts.

Two corrections over a naive reader, and the second is this round's:

* **A line is a baseline, not a text object** -- this tree writes a justified line as one text
  object per stretch and 26.2.4.2 as one per line (`probes/odt-startx-r88/startx.py`).
* **But a baseline crosses a column.**  Merging every span of one baseline turns the two
  columns of a two-column section into one line starting at the left column, so a histogram
  taken that way says "one column" for a document drawn in two.  Spans on a baseline are cut
  into segments wherever the horizontal gap exceeds `--gap` points (default 20, well under a
  column gap and well over a tab), and each segment is a line.
"""
import collections, sys, pymupdf

def lines(path, gap=20.0):
    rows = []
    with pymupdf.open(path) as doc:
        for pno, page in enumerate(doc):
            byline = collections.defaultdict(list)
            for b in page.get_text('dict')['blocks']:
                for l in b.get('lines', []):
                    for s in l['spans']:
                        if s['text'].strip():
                            byline[(pno, round(s['origin'][1], 1))].append(s)
            for key in sorted(byline):
                spans = sorted(byline[key], key=lambda s: s['origin'][0])
                seg = []
                for s in spans:
                    if seg and s['origin'][0] - seg[-1]['bbox'][2] > gap:
                        rows.append((pno, round(seg[0]['origin'][0], 2), key[1],
                                     ''.join(t['text'] for t in seg).strip()))
                        seg = []
                    seg.append(s)
                if seg:
                    rows.append((pno, round(seg[0]['origin'][0], 2), key[1],
                                 ''.join(t['text'] for t in seg).strip()))
    return [r for r in rows if r[3]]

if __name__ == '__main__':
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    hist = '--hist' in sys.argv
    gap = 20.0
    for a in sys.argv[1:]:
        if a.startswith('--gap='):
            gap = float(a.split('=')[1])
    for path in args:
        rows = lines(path, gap)
        print(f'== {path}  {len(rows)} lines')
        if hist:
            c = collections.Counter(round(x) for _, x, _, _ in rows)
            print('   ' + '  '.join(f'{x}:{n}' for x, n in sorted(c.items())))
        else:
            for pno, x, y, text in rows:
                print(f'{pno:>3} {x:>8.2f} {y:>8.2f}  {text[:70]}')
