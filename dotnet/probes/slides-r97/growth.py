#!/usr/bin/env python3
"""How many .ppt shapes does 26.2.4.2 grow to fit their text?

Left half: every live slide's Escher client/child anchors, in 1/100 mm — the height the file
states.  Right half: every drawing shape in the reference's own flat-ODP export of the same
document, which is the height 26.2.4.2 resolved.  A shape is matched on (x, y, width) — the
three coordinates `SdrObjCustomShape::AdjustTextFrameWidthAndHeight` leaves alone for a
top-anchored grow (`svx/source/svdraw/svdoashp.cxx`:2398-2401) — and a match whose heights
differ by more than 0.2 mm is a growth.

A shape whose vertical adjust is BOTTOM or CENTER moves its top as well, so it is matched on
(x, width) alone in a second pass and only when that pair is unique in the document.
"""
import re, sys, pathlib, collections
sys.path.insert(0, str(pathlib.Path(__file__).parent))
from shapes import read, shapes_of, SLIDE

MU = 2540.0 / 576.0          # a PowerPoint master unit in 1/100 mm

UNIT = {'cm': 1000.0, 'mm': 100.0, 'in': 2540.0, 'pt': 2540.0 / 72.0}
def leng(s):
    m = re.match(r'^(-?[\d.]+)(cm|mm|in|pt)$', s or '')
    return None if not m else float(m.group(1)) * UNIT[m.group(2)]

STYLE = re.compile(r'<style:style style:name="([^"]+)"[^>]*style:family="graphic"[^>]*>(.*?)</style:style>', re.S)
SHAPE = re.compile(r'<draw:(custom-shape|frame|rect|text-box)\s([^>]*)>')
ATTR  = re.compile(r'([\w:-]+)="([^"]*)"')

def fodp_shapes(path):
    x = path.read_text(encoding='utf-8', errors='replace')
    grow = {}
    for m in STYLE.finditer(x):
        grow[m.group(1)] = 'draw:auto-grow-height="true"' in m.group(2)
    out = []
    for m in SHAPE.finditer(x):
        a = dict(ATTR.findall(m.group(2)))
        w, h = leng(a.get('svg:width')), leng(a.get('svg:height'))
        px, py = leng(a.get('svg:x')), leng(a.get('svg:y'))
        if None in (w, h, px, py): continue
        out.append((px, py, w, h, grow.get(a.get('draw:style-name'), False), m.group(1)))
    return out

def ppt_anchors(path):
    buf, roots = read(str(path))
    out = []
    for rt, off, end in roots:
        if rt != SLIDE: continue
        for s in shapes_of(buf, off + 8, end):
            a = s['anchor']
            if not a: continue
            l, t, r, b = a
            out.append((l * MU, t * MU, (r - l) * MU, (b - t) * MU, s['text'], s['type']))
    return out

def near(a, b, tol=25.0): return abs(a - b) <= tol

def main(root, fodpdir):
    tot = collections.Counter()
    rows = []
    for p in sorted(q for q in pathlib.Path(root).rglob('*') if q.suffix.lower() == '.ppt'):
        f = pathlib.Path(fodpdir) / (p.stem + '.fodp')
        if not f.exists(): continue
        try:
            anchors = ppt_anchors(p)
        except Exception as e:
            print(f'SKIP {p.name}: {e}', file=sys.stderr); continue
        shapes = fodp_shapes(f)
        tot['docs'] += 1
        tot['fodp_shapes'] += len(shapes)
        tot['grow_flag'] += sum(1 for s in shapes if s[4])
        grown = 0
        for px, py, w, h, g, kind in shapes:
            if not g: continue
            cands = [a for a in anchors if near(a[0], px) and near(a[1], py) and near(a[2], w)]
            if len(cands) != 1: 
                cands = [a for a in anchors if near(a[0], px) and near(a[2], w)]
                if len(cands) != 1:
                    tot['unmatched'] += 1
                    continue
            tot['matched'] += 1
            if abs(cands[0][3] - h) > 20.0:
                grown += 1
                rows.append((p.name, px, py, w, cands[0][3], h, h - cands[0][3]))
        tot['grown'] += grown
        if grown: tot['grown_docs'] += 1
    for r in sorted(rows, key=lambda r: -abs(r[6])):
        print(f'{r[0][:52]:52s} x={r[1]/1000:7.3f} y={r[2]/1000:7.3f} w={r[3]/1000:7.3f} '
              f'stated={r[4]/1000:7.3f} resolved={r[5]/1000:7.3f} grow={r[6]/1000:+7.3f} cm')
    print('', file=sys.stderr)
    for k in ('docs','fodp_shapes','grow_flag','matched','unmatched','grown','grown_docs'):
        print(f'{k:14s} {tot[k]}', file=sys.stderr)

if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
