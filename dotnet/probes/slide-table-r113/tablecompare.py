#!/usr/bin/env python3
"""Per document, how far a leg's table pages are from the reference's.

Two quantities, both read out of the PDFs and neither of them a token count:

* `stroke`   -- the sum over pages of |sum of stroked lengths x width| difference,
                i.e. stroke INK, in square points.  A renderer may draw a rule as a
                thin FILL rather than a stroke, so a page's stroke ink alone is not
                its rule ink; both of these renderers stroke a `.ppt` table's rules,
                which is checked by the item counts printed beside it.
* `baseline` -- the mean and max |dy| between the two legs' text span origins, over
                the pages where both legs draw the same number of spans.  A page
                where they do not is counted separately and contributes nothing,
                because the pairing would be meaningless.

Usage: tablecompare.py <leg-dir> <ref-dir>
"""
import sys, os, math
import pymupdf


def strokes(page):
    out = []
    for p in page.get_drawings():
        if p['type'] not in ('s', 'fs'):
            continue
        w = p.get('width') or 0
        length = 0.0
        for item in p['items']:
            if item[0] == 'l':
                length += math.dist((item[1].x, item[1].y), (item[2].x, item[2].y))
            elif item[0] == 're':
                length += 2 * (item[1].width + item[1].height)
        out.append((w, length))
    return out


def spans(page):
    out = []
    for b in page.get_text('dict')['blocks']:
        for l in b.get('lines', []):
            for s in l['spans']:
                out.append((round(s['origin'][0], 2), round(s['origin'][1], 2)))
    out.sort()
    return out


def one(a, b):
    da, db = pymupdf.open(a), pymupdf.open(b)
    ink_a = ink_b = 0.0
    items_a = items_b = 0
    dys, unpaired = [], 0
    for i in range(min(da.page_count, db.page_count)):
        sa, sb = strokes(da[i]), strokes(db[i])
        ink_a += sum(w * l for w, l in sa)
        ink_b += sum(w * l for w, l in sb)
        items_a += len(sa)
        items_b += len(sb)
        pa, pb = spans(da[i]), spans(db[i])
        if len(pa) != len(pb):
            unpaired += 1
            continue
        dys += [abs(x[1] - y[1]) for x, y in zip(pa, pb)]
    return (da.page_count, db.page_count, ink_a, ink_b, items_a, items_b,
            (sum(dys) / len(dys) if dys else 0.0), (max(dys) if dys else 0.0), unpaired)


def main():
    leg, ref = sys.argv[1], sys.argv[2]
    print('doc\tpages\tink_leg\tink_ref\titems_leg\titems_ref\tmean_dy\tmax_dy\tunpaired')
    tot = [0.0, 0.0, 0, 0, 0.0, 0.0, 0]
    for name in sorted(os.listdir(leg)):
        if not name.endswith('.pdf') or not os.path.exists(os.path.join(ref, name)):
            continue
        pa, pb, ia, ib, na, nb, mean, mx, un = one(
            os.path.join(leg, name), os.path.join(ref, name))
        print(f'{name[:44]}\t{pa}/{pb}\t{ia:.1f}\t{ib:.1f}\t{na}\t{nb}\t{mean:.3f}\t{mx:.3f}\t{un}')
        tot[0] += ia; tot[1] += ib; tot[2] += na; tot[3] += nb
        tot[4] = max(tot[4], mx); tot[5] += mean; tot[6] += un
    print(f'TOTAL\tink {tot[0]:.1f} vs {tot[1]:.1f}\titems {tot[2]} vs {tot[3]}'
          f'\tworst dy {tot[4]:.3f}\tunpaired pages {tot[6]}')


if __name__ == '__main__':
    main()
