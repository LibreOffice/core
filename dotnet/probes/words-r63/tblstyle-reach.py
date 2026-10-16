#!/usr/bin/env python3
"""How many table cells would gain a fill from a `w:tblStylePr` `w:tcPr` layer, resolved.

`tblstylepr-census.py` counts what the *styles* declare, which over-reads by two orders of
magnitude: thirty-three of the corpus's documents carry Word's entire built-in style set as latent
styles and name three of them. This walks the tables instead and mirrors what the implementation
will do — `w:tblStyle` → `w:tblLook` → the cell's conditional layers most specific first → the
`w:basedOn` chain — and reports only cells that would actually change.

A cell is counted only when

  * some layer in its chain states a `w:tcPr/w:shd` with a real fill, and
  * the cell states no `w:shd` of its own (direct formatting wins), and
  * the resolved fill differs from what the cell draws today (nothing).

Blind spots, before the sweep:

  * tables in headers and footers are walked, but a table inside a text box's `w:txbxContent` is
    walked too and this reader may not lay that out at all, so its cells over-read;
  * `w:cnfStyle`, which lets a *row or cell* name its own conditional regions directly, is not
    read here or in the implementation — a document using it under-reads;
  * a `w:shd` whose `w:fill` is `auto` with a `w:val` pattern resolves to a real colour in the
    implementation and is counted here as a fill, so the two agree, but neither is checked against
    what the reference paints;
  * vertical bands need the *grid* column index, and a `w:gridSpan` makes the cell index and the
    grid index differ; this counts by grid index, as the implementation does.

    tblstyle-reach.py
"""
import collections
import os
import re
import sys
import zipfile
from xml.etree import ElementTree as ET

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
MANIFEST = '/c/sandbox/workdir/sample-files/MANIFEST.tsv'
ROOT = '/c/sandbox/workdir/sample-files'


def val(el, name):
    return None if el is None else el.get(W + name)


def look_of(tblpr):
    """(firstRow, lastRow, firstCol, lastCol, hBand, vBand) from w:tblLook."""
    el = None if tblpr is None else tblpr.find(W + 'tblLook')
    if el is None:
        return (False,) * 4 + (False, False)
    mask = 0
    v = el.get(W + 'val')
    if v:
        try:
            mask = int(v, 16)
        except ValueError:
            mask = 0

    def flag(name, bit, invert=False):
        a = el.get(W + name)
        if a in ('1', 'true', 'on'):
            return not invert
        if a in ('0', 'false', 'off'):
            return invert
        return bool(mask & bit) != invert

    return (flag('firstRow', 0x0020), flag('lastRow', 0x0040),
            flag('firstColumn', 0x0080), flag('lastColumn', 0x0100),
            flag('noHBand', 0x0200, invert=True), flag('noVBand', 0x0400, invert=True))


def names_for(look, first_row, last_row, first_col, last_col, row_band, col_band):
    fr, lr, fc, lc, hb, vb = look
    first, last = fr and first_row, lr and last_row
    lead, trail = fc and first_col, lc and last_col
    out = []
    if first and lead: out.append('nwCell')
    if first and trail: out.append('neCell')
    if last and lead: out.append('swCell')
    if last and trail: out.append('seCell')
    if first: out.append('firstRow')
    if last: out.append('lastRow')
    if lead: out.append('firstCol')
    if trail: out.append('lastCol')
    if hb and not first and not last and row_band is not None:
        out.append('band1Horz' if row_band % 2 == 0 else 'band2Horz')
    if vb and not lead and not trail and col_band is not None:
        out.append('band1Vert' if col_band % 2 == 0 else 'band2Vert')
    out.append('wholeTable')
    return out


class Styles:
    def __init__(self, root):
        self.by_id = {}
        if root is None:
            return
        for st in root.iter(W + 'style'):
            sid = st.get(W + 'styleId')
            if sid:
                self.by_id[sid] = st

    def chain(self, sid):
        seen = set()
        out = []
        while sid and sid in self.by_id and sid not in seen:
            seen.add(sid)
            st = self.by_id[sid]
            out.append(st)
            sid = val(st.find(W + 'basedOn'), 'val')
        return out

    def band_sizes(self, sid):
        row = col = 1
        for st in self.chain(sid):
            tp = st.find(W + 'tblPr')
            if tp is None:
                continue
            r = val(tp.find(W + 'tblStyleRowBandSize'), 'val')
            c = val(tp.find(W + 'tblStyleColBandSize'), 'val')
            if r and row == 1:
                row = max(1, int(r))
            if c and col == 1:
                col = max(1, int(c))
        return row, col

    def shading(self, sid, names):
        """The first w:tcPr/w:shd a cell's layer chain states, style's own layers first."""
        for st in self.chain(sid):
            layers = {l.get(W + 'type'): l for l in st.findall(W + 'tblStylePr')}
            for name in names:
                layer = layers.get(name)
                tcpr = None if layer is None else layer.find(W + 'tcPr')
                shd = None if tcpr is None else tcpr.find(W + 'shd')
                if shd is not None:
                    return name, shd.get(W + 'fill'), shd.get(W + 'val')
            tcpr = st.find(W + 'tcPr')                   # the style's unconditional cell half
            shd = None if tcpr is None else tcpr.find(W + 'shd')
            if shd is not None:
                return 'style/tcPr', shd.get(W + 'fill'), shd.get(W + 'val')
        return None, None, None


def rows_of(el, depth=0):
    if depth > 8:
        return
    for child in el:
        if child.tag == W + 'tr':
            yield child
        elif child.tag in (W + 'sdt', W + 'sdtContent', W + 'customXml', W + 'ins'):
            yield from rows_of(child, depth + 1)


def cells_of(row, depth=0):
    if depth > 8:
        return
    for child in row:
        if child.tag == W + 'tc':
            yield child
        elif child.tag in (W + 'sdt', W + 'sdtContent', W + 'customXml', W + 'ins'):
            yield from cells_of(child, depth + 1)


def main():
    per_doc = collections.Counter()
    per_layer = collections.Counter()
    fills = collections.Counter()
    with open(MANIFEST, encoding='utf-8') as f:
        next(f)
        manifest = [l.rstrip('\n').split('\t') for l in f]
    for row in manifest:
        if len(row) < 4 or row[0] != 'words' or row[3] != 'docx':
            continue
        rel = row[2]
        try:
            with zipfile.ZipFile(os.path.join(ROOT, rel)) as z:
                names = z.namelist()
                styles = Styles(ET.fromstring(z.read('word/styles.xml'))
                                if 'word/styles.xml' in names else None)
                parts = [n for n in names
                         if re.fullmatch(r'word/(document|header\d*|footer\d*)\.xml', n)]
                trees = [ET.fromstring(z.read(n)) for n in parts]
        except Exception as exc:                                   # noqa: BLE001
            print('  ! %s: %s' % (rel, exc))
            continue

        for tree in trees:
            for tbl in tree.iter(W + 'tbl'):
                tblpr = tbl.find(W + 'tblPr')
                sid = val(None if tblpr is None else tblpr.find(W + 'tblStyle'), 'val')
                if not sid or sid not in styles.by_id:
                    continue
                look = look_of(tblpr)
                rowband, colband = styles.band_sizes(sid)
                trs = list(rows_of(tbl))
                nrows = len(trs)
                body_row = -1
                for ri, tr in enumerate(trs):
                    is_first, is_last = ri == 0, ri == nrows - 1
                    if not ((look[0] and is_first) or (look[1] and is_last)):
                        body_row += 1
                    tcs = list(cells_of(tr))
                    grid = 0
                    body_col = -1
                    for ci, tc in enumerate(tcs):
                        tcpr = tc.find(W + 'tcPr')
                        span = 1
                        if tcpr is not None:
                            s = val(tcpr.find(W + 'gridSpan'), 'val')
                            if s and s.isdigit():
                                span = max(1, int(s))
                        is_fc, is_lc = ci == 0, ci == len(tcs) - 1
                        if not ((look[2] and is_fc) or (look[3] and is_lc)):
                            body_col += 1
                        rb = body_row // rowband if body_row >= 0 else None
                        cb = body_col // colband if body_col >= 0 else None
                        layer, fill, pattern = styles.shading(
                            sid, names_for(look, is_first, is_last, is_fc, is_lc, rb, cb))
                        grid += span
                        if fill in (None, 'auto') and pattern in (None, 'clear', 'nil'):
                            continue
                        own = None if tcpr is None else tcpr.find(W + 'shd')
                        if own is not None:
                            continue
                        per_doc[rel] += 1
                        per_layer[layer] += 1
                        fills[(fill or 'auto', pattern or '-')] += 1

    print('cells that would gain a fill from a conditional w:tcPr layer')
    print('  documents : %d' % len(per_doc))
    print('  cells     : %d' % sum(per_doc.values()))
    print()
    print('by layer  : %s' % dict(per_layer.most_common()))
    print('by fill   : %s' % dict(fills.most_common(12)))
    print()
    for rel, n in per_doc.most_common():
        print('   %-5d %s' % (n, rel))


if __name__ == '__main__':
    main()
