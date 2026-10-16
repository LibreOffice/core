#!/usr/bin/env python3
"""Read the drawn thickness of every rule in a sweep rendering, attributed by its own label.

    read-rules.py <pdf> <manifest.tsv> > rows.tsv

Columns: label, count, thickness_pt.  The manifest supplies each row's size and which of the
three kinds it is; the label is an opaque tag drawn by the row itself.

**Both shapes are measured.**  The two renderers do not draw a rule the same way: 26.2.4.2
STROKES it -- `PDFWriterImpl::drawStraightTextLine` emits `<w> w` and a segment -- and this
tree FILLS a rectangle of the same band (C16).  A census reading only one shape scores every
rule of the other side as absent, which is what C14 and C16 are both about.  A stroke's
thickness is its `width` and a fill's is its rectangle's height.

**A rule is attributed by its offset from its own baseline and not by nearness.**  The first
cut took every horizontal rule within a line of the label's bounding box and lost 29 of 810
rows to the neighbouring row's -- a strike row three lines from a double underline collected
that row's two rules as well as its own, and the median then answered the wrong one.  The
windows below are in ems, so they scale with the row.
"""
import re, sys
import pymupdf

LABEL = re.compile(r'^Q(\d+)$')

# Fractions of the em, measured from the baseline, positive downwards.
WINDOW = {
    'single': (0.005, 0.40),
    'double': (0.005, 0.60),
    'strike': (-0.70, -0.02),
}


def horizontal_rules(page):
    out = []
    for d in page.get_drawings():
        r = d['rect']
        if r.width < 3:
            continue
        if d['type'] == 's':
            # A stroked rule is a segment, so its own rectangle is a sliver; a stroked box is
            # not a rule.
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


def rows(page):
    for block in page.get_text('dict')['blocks']:
        for line in block.get('lines', []):
            spans = line['spans']
            text = ''.join(s['text'] for s in spans).strip()
            if not LABEL.match(text):
                continue
            yield text, line['bbox'], spans[0]['origin'][1]


def main(pdf, manifest):
    kinds, sizes = {}, {}
    for line in open(manifest).read().splitlines()[1:]:
        label, _face, size, kind = line.split('\t')
        kinds[label] = kind
        sizes[label] = float(size)

    doc = pymupdf.open(pdf)
    print('label\tcount\tthickness_pt')
    for pi in range(doc.page_count):
        page = doc[pi]
        rules = horizontal_rules(page)
        for text, bbox, baseline in rows(page):
            x0, _y0, x1, _y1 = bbox
            lo, hi = WINDOW[kinds[text]]
            em = sizes[text]
            hit = [t for (rx0, rx1, ry, t) in rules
                   if rx0 < x1 - 1 and rx1 > x0 + 1
                   and baseline + lo * em <= ry <= baseline + hi * em]
            hit.sort()
            print('%s\t%d\t%.4f' % (text, len(hit), hit[len(hit) // 2] if hit else 0.0))
    doc.close()


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
