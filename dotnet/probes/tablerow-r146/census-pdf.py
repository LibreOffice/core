#!/usr/bin/env python3
"""Census, out of 26.2.4.2's OWN renderings, of what the horizontal-band rule paints.
**SUPERSEDED -- its MIXED figures are wrong by an order of magnitude; do not quote 49/770.**
Round 148 (`probes/mixedcensus-r148/results.md`) took it apart against the same renderings and
found three defects. Its second shape never compares THICKNESS at all -- it counts a narrower
rule within 6 pt below a wider one whatever their widths -- so 100 % of the 210 it scored on
`150_5300_13_chg8.doc` and the 206 on `150-5370-10H.docx` is a vector-traced departmental seal
of same-width hairlines. Its first shape, which IS the question, caps the side-by-side OVERLAP
at 1.0 pt and puts no cap on the GAP, so it rejects the mitre overlap that two bands of
different width actually make. Corrected, the reach is **16 documents and 67 boundaries**, not
49 and 770. Use `mixedcensus-r148/mixed-census.py` (thickness) or `extent-census.py` (extent,
which also catches a `double` border beside a single one, where every band is one thickness and
only the extent differs). Kept as the record of what round 146 ran.

    census-pdf.py <sweep.tsv> <out-prefix>

Two questions, both read from the PDF's path operators:

  MIXED  -- a row boundary at which the reference draws bands of MORE THAN ONE width side by
            side.  Because the reference top-aligns every horizontal table band (measured,
            results.md §3.1), such a boundary is one shared band TOP with two thicknesses on it.
            A model that centres one band per boundary cannot draw it.  This is O85's paint.
            The base rate is the boundary that has two or more side-by-side segments of the SAME
            width, which the centred model draws correctly.

  CUT    -- a table crossing a page boundary: the verticals of page p end where a horizontal rule
            sits, the verticals of page p+1 begin where a horizontal rule sits at the very top of
            the body, and at least two of their x positions agree.  A PROXY, and an upper bound:
            a table ending exactly at the foot of one page beside a different table opening the
            next would also match.
"""
import sys, collections
import pymupdf

TOL = 0.06          # y grouping, in points
XTOL = 1.0


def segs(page):
    """(horizontals, verticals) as (band_lo, band_hi, a, b, thickness)."""
    H, V = [], []
    for d in page.get_drawings():
        k = d.get('type', '')
        for it in d['items']:
            if it[0] == 're' and k in ('f', 'fs', 's'):
                r = it[1]
                if r.height <= 6.0 and r.width >= 4.0 and r.width > r.height:
                    H.append((r.y0, r.y1, r.x0, r.x1, r.height))
                elif r.width <= 6.0 and r.height >= 4.0 and r.height > r.width:
                    V.append((r.x0, r.x1, r.y0, r.y1, r.width))
            elif it[0] == 'l' and k in ('s', 'fs'):
                a, b = it[1], it[2]
                w = d.get('width') or 0.0
                if abs(a.y - b.y) <= 0.01 and abs(a.x - b.x) >= 4.0:
                    y = (a.y + b.y) / 2
                    H.append((y - w / 2, y + w / 2, min(a.x, b.x), max(a.x, b.x), w))
                elif abs(a.x - b.x) <= 0.01 and abs(a.y - b.y) >= 4.0:
                    x = (a.x + b.x) / 2
                    V.append((x - w / 2, x + w / 2, min(a.y, b.y), max(a.y, b.y), w))
    return H, V


def text_top(page):
    ys = [s['bbox'][1] for b in page.get_text('dict')['blocks']
          for ln in b.get('lines', []) for s in ln['spans'] if s['text'].strip()]
    return min(ys) if ys else None


def cover(v):
    """Total x covered by a list of (x0, x1, th), merged."""
    tot, cur0, cur1 = 0.0, None, None
    for x0, x1, _ in sorted(v):
        if cur1 is None or x0 > cur1 + 0.5:
            if cur1 is not None:
                tot += cur1 - cur0
            cur0, cur1 = x0, x1
        else:
            cur1 = max(cur1, x1)
    if cur1 is not None:
        tot += cur1 - cur0
    return tot


def mixed(H):
    """A boundary this tree cannot draw at one y, in two shapes.

    (a) one band TOP carrying segments of MORE THAN ONE thickness side by side -- singles of
        different widths meeting at one boundary;
    (b) a NARROWER group of segments hanging 0 < dy <= 6 pt below a wider group and contained in
        it -- the inner rule of a multi-line border present on some columns only, which is the
        `150_5335_5a.doc` shape.

    The base rate is a boundary drawn as two or more side-by-side segments all of ONE width,
    which a model that centres one band per boundary draws correctly.
    """
    g = collections.defaultdict(list)
    for lo, hi, x0, x1, th in H:
        key = round(lo / TOL)
        seg = (round(x0, 2), round(x1, 2), round(th, 3))
        if seg not in g[key]:                       # the writer emits some rules twice
            g[key].append(seg)
    keys = sorted(g)
    nmix = nsame = 0
    flagged = set()
    for k in keys:
        v = sorted(g[k])
        if len(v) >= 2:
            side = all(v[i][1] - v[i + 1][0] <= 1.0 for i in range(len(v) - 1))
            if side and len({t for _, _, t in v}) > 1:
                nmix += 1
                flagged.add(k)
                continue
            if side:
                nsame += 1
    # (b): a narrower group just below a wider one
    for i, k in enumerate(keys):
        if k in flagged:
            continue
        cv = cover(g[k])
        for k2 in keys[i + 1:]:
            dy = (k2 - k) * TOL
            if dy <= 0 or dy > 6.0:
                break
            if cover(g[k2]) < cv - 1.0 and all(
                    any(a[0] >= b[0] - 1.0 and a[1] <= b[1] + 1.0 for b in g[k]) for a in g[k2]):
                nmix += 1
                flagged.add(k)
                break
    return nmix, nsame


def cuts(doc):
    n = 0
    for p in range(doc.page_count - 1):
        Ha, Va = segs(doc[p])
        Hb, Vb = segs(doc[p + 1])
        if not Va or not Vb or not Ha or not Hb:
            continue
        yb = max(v[3] for v in Va)
        yt = min(v[2] for v in Vb)
        if not any(h[0] - 0.7 <= yb <= h[1] + 0.7 for h in Ha):
            continue
        if not any(h[0] - 0.7 <= yt <= h[1] + 0.7 for h in Hb):
            continue
        tt = text_top(doc[p + 1])
        if tt is None or yt > tt + 2.0:
            continue
        xa = sorted((v[0] + v[1]) / 2 for v in Va if abs(v[3] - yb) < 0.7)
        xb = sorted((v[0] + v[1]) / 2 for v in Vb if abs(v[2] - yt) < 0.7)
        common = sum(1 for a in xa if any(abs(a - b) < XTOL for b in xb))
        if common >= 2:
            n += 1
    return n


rows = []
for line in open(sys.argv[1]):
    st, rel, pdf = line.rstrip('\n').split('\t')
    if st != 'ok':
        continue
    try:
        doc = pymupdf.open(pdf)
    except Exception as e:
        rows.append((rel, -1, -1, -1, f'open:{e}')); continue
    nmix = nsame = 0
    for p in range(doc.page_count):
        H, _ = segs(doc[p])
        a, b = mixed(H)
        nmix += a; nsame += b
    rows.append((rel, nmix, nsame, cuts(doc), ''))
    doc.close()

with open(sys.argv[2] + '.tsv', 'w') as f:
    f.write('path\tmixed_boundaries\tsame_width_boundaries\tpage_cuts\tnote\n')
    for r in rows:
        f.write('\t'.join(str(x) for x in r) + '\n')

docs = len(rows)
dmix = sum(1 for r in rows if r[1] > 0)
dsame = sum(1 for r in rows if r[2] > 0)
dcut = sum(1 for r in rows if r[3] > 0)
print(f'documents read                                   {docs}')
print(f'  with >=1 MIXED-width row boundary              {dmix}   ({sum(r[1] for r in rows)} boundaries)')
print(f'  with >=1 per-column boundary of ONE width      {dsame}   ({sum(r[2] for r in rows)} boundaries)')
print(f'  with >=1 table crossing a page boundary        {dcut}   ({sum(r[3] for r in rows)} cuts)')
