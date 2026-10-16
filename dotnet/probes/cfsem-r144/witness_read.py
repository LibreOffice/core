"""Read the reference's painted conditional fills for H5:BO30 of the witness.

Anchors: the period numbers 1..60 printed in row 4 give the x centre of each
column H..BO; the 'Activity nn' labels in column B give the y centre of each
row 5..30."""
import sys, collections, pymupdf

def grid(page):
    words = page.get_text('words')
    # row 4: the y-band that contains the most integer words 1..60
    byy = collections.defaultdict(list)
    for x0, y0, x1, y1, t, *_ in words:
        byy[round((y0 + y1) / 2, 1)].append((x0, x1, t))
    band = max(byy.items(), key=lambda kv: sum(1 for w in kv[1] if w[2].isdigit()))
    cols = {}
    for x0, x1, t in band[1]:
        if t.isdigit():
            n = int(t)
            if 1 <= n <= 60:
                cols[n] = (x0 + x1) / 2      # period n  ->  column H+n-1
    rows = {}
    for x0, y0, x1, y1, t, *_ in words:
        if t == 'Activity':
            rows[len(rows)] = (y0 + y1) / 2
    # activity k (0-based) is worksheet row 5+k
    rows = {5 + k: v for k, v in sorted(rows.items(), key=lambda kv: kv[1])}
    rows = {5 + i: y for i, (_, y) in enumerate(sorted(rows.items(), key=lambda kv: kv[1]))}
    return cols, rows

def fills(page):
    out = []
    for d in page.get_drawings():
        if d.get('fill') is None:
            continue
        r = d['rect']
        if r.width < 1 or r.height < 1:
            continue
        out.append((r, tuple(d['fill'])))
    return out

def read(path):
    doc = pymupdf.open(path)
    page = doc[0]
    cols, rows = grid(page)
    fl = fills(page)
    res = {}
    for n, x in sorted(cols.items()):
        for r, y in sorted(rows.items()):
            hit = None
            for rect, col in fl:
                if rect.x0 <= x <= rect.x1 and rect.y0 <= y <= rect.y1:
                    if hit is None or rect.get_area() < hit[0].get_area():
                        hit = (rect, col)
            res[(n, r)] = None if hit is None else '#%02X%02X%02X' % tuple(round(c * 255) for c in hit[1])
    return cols, rows, res

if __name__ == '__main__':
    cols, rows, res = read(sys.argv[1])
    print('columns found: %d  rows found: %d' % (len(cols), len(rows)))
    for r in sorted(rows):
        print('row %2d: %s' % (r, ' '.join((res[(n, r)] or '-------')[1:4] for n in sorted(cols))))
