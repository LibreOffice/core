#!/usr/bin/env python3
r"""Read every rule of a position probe with its offset from its OWN baseline.

    read-pos.py <pdf> <pos-manifest.tsv> > rows.tsv

Columns: label, count, then for each rule found, its stroke CENTRE's distance below the
baseline and its thickness, in points, sorted by depth.

The reference STROKES a text line and this tree FILLS it (C16), so both shapes are read and
a fill's centre is its rectangle's mid-height.  Attribution is by the label's own baseline
and an em-scaled window, exactly as `quantise-r120/read-rules.py` does it, because a strike
row three lines above a double-underline row otherwise collects that row's two rules.
"""
import re, sys
import pymupdf

LABEL = re.compile(r'^P(\d+)$')
WINDOW = {'single': (0.005, 0.40), 'double': (0.005, 0.60), 'strike': (-0.70, -0.02)}


def horizontal_rules(page):
    out = []
    for d in page.get_drawings():
        r = d['rect']
        if r.width < 3:
            continue
        if d['type'] == 's':
            if r.height > 1.5:
                continue
            thickness = d.get('width') or 0.0
        else:
            if r.height > 8:
                continue
            thickness = r.height
        if thickness <= 0:
            continue
        out.append((r.x0, r.x1, (r.y0 + r.y1) / 2.0, thickness))
    return out


def main(pdf, manifest):
    kinds, sizes = {}, {}
    for line in open(manifest).read().splitlines()[1:]:
        label, _face, size, kind = line.split('\t')
        kinds[label], sizes[label] = kind, float(size)

    doc = pymupdf.open(pdf)
    print('label\tcount\trules')
    for pi in range(doc.page_count):
        page = doc[pi]
        rules = horizontal_rules(page)
        for block in page.get_text('dict')['blocks']:
            for line in block.get('lines', []):
                text = ''.join(s['text'] for s in line['spans']).strip()
                if not LABEL.match(text):
                    continue
                x0, _y0, x1, _y1 = line['bbox']
                baseline = line['spans'][0]['origin'][1]
                lo, hi = WINDOW[kinds[text]]
                em = sizes[text]
                hit = sorted((round(ry - baseline, 4), round(t, 4))
                             for (rx0, rx1, ry, t) in rules
                             if rx0 < x1 - 1 and rx1 > x0 + 1
                             and baseline + lo * em <= ry <= baseline + hi * em)
                print('%s\t%d\t%s' % (text, len(hit),
                                      ' '.join('%.4f@%.4f' % (d, t) for d, t in hit)))
    doc.close()


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
