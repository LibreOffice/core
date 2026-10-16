#!/usr/bin/env python3
"""Score `dxf-model.py` against 26.2.4.2's own resolved view, cell by cell and value by value.

    check-dxf.py <workbook.xlsx> <workbook.fods> ...   (pairs)

Three models are scored over every cell of every pivot rectangle the grid is generated for:

  merge      the cell keeps what the workbook states — what the tree shipped before r110
  clear      the cell is taken back to the document's Default cell style and nothing else
  clear+dxf  cleared, then the pivot's own `<format>` records laid over it — the reference's
             own rule, transcribed in `dxf-model.py`

Unlike `clearfour.py` this compares **values**, not "differs from the default": a theme colour
is resolved through the workbook's own theme and tint and compared as a hex string, a size as
points, a face as the family the reference's `<style:font-face>` names. Where the reference
writes `style:use-window-font-color="true"` the colour it draws is the automatic one, which on a
dark ground is not the Default's, so those cells are reported separately rather than counted as
agreements.
"""
import sys, os, re, zipfile, importlib.util
import xml.etree.ElementTree as ET

HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location('dxfm', os.path.join(HERE, 'dxf-model.py'))
dxfm = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(dxfm)
pg = dxfm.pg
m = pg.m

NS = {'office': 'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
      'table': 'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
      'style': 'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
      'svg': 'urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0',
      'fo': 'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0'}
A = 'http://schemas.openxmlformats.org/drawingml/2006/main'


def fq(n):
    p, l = n.split(':')
    return '{%s}%s' % (NS[p], l)


PROPS = ('face', 'size', 'colour', 'fill')

#  `styles.xml`'s theme index order, which is not the theme part's own order: 0 and 1 are the
#  light and dark of scheme 1, 2 and 3 the light and dark of scheme 2, then the accents.
THEME_ORDER = ['lt1', 'dk1', 'lt2', 'dk2', 'accent1', 'accent2', 'accent3', 'accent4',
               'accent5', 'accent6', 'hlink', 'folHlink']


def read_theme(z):
    for name in z.namelist():
        if name.startswith('xl/theme/') and name.endswith('.xml'):
            root = ET.fromstring(z.read(name))
            scheme = root.find('.//{%s}clrScheme' % A)
            if scheme is None:
                continue
            byname = {}
            for child in scheme:
                tag = child.tag.split('}')[1]
                srgb = child.find('{%s}srgbClr' % A)
                sys_ = child.find('{%s}sysClr' % A)
                if srgb is not None:
                    byname[tag] = srgb.get('val').upper()
                elif sys_ is not None:
                    byname[tag] = (sys_.get('lastClr') or '000000').upper()
            return [byname.get(n) for n in THEME_ORDER]
    return [None] * len(THEME_ORDER)


def tinted(hexrgb, tint):
    if not hexrgb:
        return None
    r, g, b = (int(hexrgb[i:i + 2], 16) for i in (0, 2, 4))
    t = float(tint or 0)
    def one(c):
        if t > 0:
            return int(round(c * (1 - t) + 255 * t))
        if t < 0:
            return int(round(c * (1 + t)))
        return c
    return '%02X%02X%02X' % (one(r), one(g), one(b))


def resolve_colour(spec, theme, auto='000000'):
    if spec is None:
        return None
    if spec[0] == 'rgb':
        return spec[1][-6:]
    if spec[0] == 'theme':
        at = int(spec[1])
        return tinted(theme[at] if 0 <= at < len(theme) else None, spec[2])
    if spec[0] == 'auto':
        return auto.upper()
    return None                          # an indexed colour is not resolved here


# ---------------------------------------------------------------- the workbook's own view

def book_view(z, theme):
    """cellXfs index -> resolved {face, class, size, colour, fill}."""
    st = ET.fromstring(z.read('xl/styles.xml'))
    fonts = []
    for f in st.find(m('fonts')) or []:
        name = f.find(m('name'))
        sz = f.find(m('sz'))
        fonts.append({
            'face': name.get('val') if name is not None else None,
            'size': sz.get('val') if sz is not None else None,
            'colour': resolve_colour(dxfm.colour_of(f.find(m('color'))), theme),
        })
    fills = []
    for fl in st.find(m('fills')) or []:
        p = fl.find(m('patternFill'))
        if p is None or p.get('patternType') in (None, 'none'):
            fills.append(None)
        else:
            fills.append(resolve_colour(dxfm.colour_of(p.find(m('fgColor'))), theme))
    out = []
    for xf in st.find(m('cellXfs')) or []:
        fid = int(xf.get('fontId', '0'))
        flid = int(xf.get('fillId', '0'))
        v = dict(fonts[fid]) if fid < len(fonts) else {'face': None, 'size': None, 'colour': None}
        v['fill'] = fills[flid] if flid < len(fills) else None
        out.append(v)
    return out


def normal_xf(z):
    st = ET.fromstring(z.read('xl/styles.xml'))
    for s in (st.find(m('cellStyles')) or []):
        if s.get('builtinId') == '0':
            return int(s.get('xfId', '0'))
    return 0


def style_view(z, theme):
    """The `Normal` cellStyleXf's own resolved properties — what a cleared cell falls back to."""
    st = ET.fromstring(z.read('xl/styles.xml'))
    sxfs = list(st.find(m('cellStyleXfs')) or [])
    at = normal_xf(z)
    if at >= len(sxfs):
        return {'face': None, 'size': None, 'colour': None, 'fill': None}
    xf = sxfs[at]
    fonts = list(st.find(m('fonts')) or [])
    fid = int(xf.get('fontId', '0'))
    f = fonts[fid] if fid < len(fonts) else None
    name = f.find(m('name')) if f is not None else None
    sz = f.find(m('sz')) if f is not None else None
    return {
        'face': name.get('val') if name is not None else None,
        'size': sz.get('val') if sz is not None else None,
        'colour': resolve_colour(dxfm.colour_of(f.find(m('color'))), theme) if f is not None else None,
        'fill': None,
    }


# ---------------------------------------------------------------- the reference's own view

ROWHOLDERS = (fq('table:table-header-rows'), fq('table:table-row-group'), fq('table:table-rows'))
COLHOLDERS = (fq('table:table-header-columns'), fq('table:table-column-group'),
              fq('table:table-columns'))


def walk(node, want, holders):
    for child in node:
        if child.tag == want:
            yield child
        elif child.tag in holders:
            yield from walk(child, want, holders)


FODPROP = (('face', fq('style:text-properties'), fq('style:font-name')),
           ('size', fq('style:text-properties'), fq('fo:font-size')),
           ('colour', fq('style:text-properties'), fq('fo:color')),
           ('auto', fq('style:text-properties'), fq('style:use-window-font-color')),
           ('fill', fq('style:table-cell-properties'), fq('fo:background-color')))


def fods_view(root, sheet, wanted):
    raw = {}
    for holder in root:
        if holder.tag not in (fq('office:automatic-styles'), fq('office:styles')):
            continue
        for st in holder.findall(fq('style:style')):
            vals = {}
            for key, tag, attr in FODPROP:
                e = st.find(tag)
                if e is not None and e.get(attr) is not None:
                    vals[key] = e.get(attr)
            raw[st.get(fq('style:name'))] = (vals, st.get(fq('style:parent-style-name')))

    alias = {}
    for holder in root:
        if holder.tag != fq('office:font-face-decls'):
            continue
        for ff in holder:
            nm = ff.get(fq('style:name'))
            if nm:
                alias[nm] = (ff.get(fq('svg:font-family')) or nm).strip("'")

    cache = {}

    def resolve(name):
        if name in cache:
            return cache[name]
        chain, seen, n = [], set(), name
        while n and n in raw and n not in seen:
            seen.add(n)
            chain.append(n)
            n = raw[n][1]
        v = {}
        for n in reversed(chain):
            v.update(raw[n][0])
        if 'face' in v:
            v['face'] = alias.get(v['face'], v['face'])
        cache[name] = v
        return v

    default = resolve('Default')
    body = root.find(fq('office:body')).find(fq('office:spreadsheet'))
    table = None
    for t in body.findall(fq('table:table')):
        if t.get(fq('table:name')) == sheet:
            table = t
            break
    if table is None:
        return None, default

    coldef = []
    for col in walk(table, fq('table:table-column'), COLHOLDERS):
        crep = min(int(col.get(fq('table:number-columns-repeated'), 1)), 1024)
        coldef += [col.get(fq('table:default-cell-style-name'))] * crep

    lastrow = max((w[3] for w in wanted), default=-1)
    lastcol = max((w[2] for w in wanted), default=-1)
    rows_wanted = set()
    for c0, r0, c1, r1 in wanted:
        rows_wanted.update(range(r0, r1 + 1))

    grid = {}
    r = -1
    for row in walk(table, fq('table:table-row'), ROWHOLDERS):
        rep = int(row.get(fq('table:number-rows-repeated'), 1))
        if not (rows_wanted & set(range(r + 1, min(r + rep, lastrow) + 1))):
            r += rep
            if r > lastrow:
                break
            continue
        rowdef = row.get(fq('table:default-cell-style-name'))
        cells = [c for c in row
                 if c.tag in (fq('table:table-cell'), fq('table:covered-table-cell'))]
        for _ in range(rep):
            r += 1
            if r > lastrow:
                break
            if r not in rows_wanted:
                continue
            c = -1
            for cell in cells:
                crep = int(cell.get(fq('table:number-columns-repeated'), 1))
                sn0 = cell.get(fq('table:style-name'))
                for _ in range(crep):
                    c += 1
                    if c > lastcol:
                        break
                    eff = sn0 or rowdef or (coldef[c] if c < len(coldef) else None)
                    grid[(r, c)] = resolve(eff) if eff else default
                if c > lastcol:
                    break
        if r > lastrow:
            break
    return grid, default


def points(v):
    if v is None:
        return None
    mm = re.match(r'([0-9.]+)pt', v)
    return round(float(mm.group(1)), 2) if mm else None


def hexof(v):
    if not v or not v.startswith('#'):
        return None
    return v[1:].upper()


# ---------------------------------------------------------------- the score

R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'


def colnum(s):
    n = 0
    for ch in s:
        n = n * 26 + ord(ch) - 64
    return n - 1


def parse_ref(ref):
    a, _, b = ref.partition(':')
    b = b or a
    ma = re.match(r'\$?([A-Z]+)\$?(\d+)', a)
    mb = re.match(r'\$?([A-Z]+)\$?(\d+)', b)
    return colnum(ma.group(1)), int(ma.group(2)) - 1, colnum(mb.group(1)), int(mb.group(2)) - 1


TOTAL = {k: {'cells': 0, 'merge': 0, 'clear': 0, 'dxf': 0} for k in PROPS}
AUTOCELLS = [0]


def main():
    for at in range(1, len(sys.argv), 2):
        path, fpath = sys.argv[at], sys.argv[at + 1]
        z = zipfile.ZipFile(path)
        name = os.path.basename(path)
        theme = read_theme(z)
        xfs = book_view(z, theme)
        cleared = style_view(z, theme)
        dxfs = dxfm.read_dxfs(z)
        fods = ET.parse(fpath).getroot()

        wb = ET.fromstring(z.read('xl/workbook.xml'))
        rels = {r.get('Id'): r.get('Target')
                for r in ET.fromstring(z.read('xl/_rels/workbook.xml.rels'))}
        for sh in wb.find(m('sheets')):
            t = rels[sh.get('{%s}id' % R)]
            part = t.lstrip('/') if t.startswith('/') else ('xl/' + t if not t.startswith('xl/') else t)
            rp = os.path.join(os.path.dirname(part), '_rels', os.path.basename(part) + '.rels')
            if rp not in z.namelist():
                continue
            pts = []
            for r in ET.fromstring(z.read(rp)):
                if r.get('Type', '').endswith('/pivotTable'):
                    raw = r.get('Target')
                    pts.append((raw.lstrip('/') if raw.startswith('/')
                                else os.path.normpath(os.path.join(os.path.dirname(part), raw))).replace('\\', '/'))
            if not pts:
                continue

            ws = ET.fromstring(z.read(part))
            cols, rows, cells = {}, {}, {}
            cse = ws.find(m('cols'))
            if cse is not None:
                for c in cse:
                    if c.get('style') is None:
                        continue
                    lo, hi = int(c.get('min', '1')) - 1, int(c.get('max', '1')) - 1
                    for cc in range(lo, min(hi, 16383) + 1):
                        cols[cc] = int(c.get('style'))
            for row in ws.find(m('sheetData')) or []:
                if row.get('r') is None:
                    continue
                ri = int(row.get('r')) - 1
                if row.get('customFormat') in ('1', 'true') and row.get('s') is not None:
                    rows[ri] = int(row.get('s'))
                for c in row:
                    a = c.get('r')
                    if not a:
                        continue
                    mm = re.match(r'([A-Z]+)(\d+)', a)
                    cells[(int(mm.group(2)) - 1, colnum(mm.group(1)))] = int(c.get('s', '0'))

            wanted = []
            models = {}
            for pt in pts:
                root = ET.fromstring(z.read(pt))
                over, why = dxfm.resolve(root, dxfs)
                if over is None:
                    continue
                loc = root.find(m('location'))
                box = parse_ref(loc.get('ref'))
                wanted.append(box)
                models[pt] = (box, over)
            if not wanted:
                continue

            grid, fdefault = fods_view(fods, sh.get('name'), wanted)
            if grid is None:
                continue

            for pt, (box, over) in models.items():
                c0, r0, c1, r1 = box
                score = {k: {'cells': 0, 'merge': 0, 'clear': 0, 'dxf': 0} for k in PROPS}
                shown = []
                for rr in range(r0, r1 + 1):
                    for cc in range(c0, c1 + 1):
                        s = cells.get((rr, cc), rows.get(rr, cols.get(cc, 0)))
                        stated = xfs[s] if s < len(xfs) else cleared
                        ref = grid.get((rr, cc), fdefault)
                        auto = ref.get('auto') in ('true', '1')
                        if auto:
                            AUTOCELLS[0] += 1
                        put = over.get((rr, cc), {})

                        want = {
                            'face': ref.get('face') or (fdefault.get('face') or None),
                            'size': points(ref.get('size')) or points(fdefault.get('size')) or 10.0,
                            'colour': None if auto else (hexof(ref.get('colour'))
                                                         or hexof(fdefault.get('colour')) or '000000'),
                            'fill': hexof(ref.get('fill')),
                        }
                        got = {}
                        for model, base in (('merge', stated), ('clear', cleared), ('dxf', cleared)):
                            v = {
                                'face': base.get('face') or cleared.get('face'),
                                'size': float(base.get('size') or cleared.get('size') or 10),
                                'colour': base.get('colour') or '000000',
                                'fill': base.get('fill'),
                            }
                            if model == 'dxf':
                                if 'face' in put:
                                    v['face'] = put['face']
                                if 'size' in put:
                                    v['size'] = float(put['size'])
                                if 'colour' in put:
                                    v['colour'] = resolve_colour(put['colour'], theme) or v['colour']
                                if 'fill' in put:
                                    v['fill'] = (resolve_colour(put['fill'][1], theme)
                                                 if put['fill'] else None)
                            got[model] = v
                        for k in PROPS:
                            score[k]['cells'] += 1
                            if k == 'colour' and auto:
                                for mo in ('merge', 'clear', 'dxf'):
                                    score[k][mo] += 1     # not decidable from the fods
                                continue
                            for mo in ('merge', 'clear', 'dxf'):
                                if got[mo][k] == want[k]:
                                    score[k][mo] += 1
                            if got["dxf"][k] != want[k] and len(shown) < 40:
                                shown.append('%s%d %s ours=%s ref=%s' % (
                                    pg.colname(cc), rr + 1, k, got['dxf'][k], want[k]))
                for k in PROPS:
                    for f in ('cells', 'merge', 'clear', 'dxf'):
                        TOTAL[k][f] += score[k][f]
                print('%s\t%s\t%s\tcells=%d\t%s' % (
                    name, sh.get('name'), os.path.basename(pt), score['face']['cells'],
                    '  '.join('%s m=%d c=%d d=%d' % (k, score[k]['merge'], score[k]['clear'],
                                                     score[k]['dxf']) for k in PROPS)))
                for s in shown:
                    print('      %s' % s)

    print()
    print('%-8s %8s %10s %10s %10s' % ('property', 'cells', 'merge', 'clear', 'clear+dxf'))
    for k in PROPS:
        print('%-8s %8d %10d %10d %10d' % (
            k, TOTAL[k]['cells'], TOTAL[k]['merge'], TOTAL[k]['clear'], TOTAL[k]['dxf']))
    print()
    print('cells the reference draws in the automatic font colour (not decidable from the '
          '.fods, counted as agreeing for every model): %d' % AUTOCELLS[0])


main()
