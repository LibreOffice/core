#!/usr/bin/env python3
"""Does clearing the pivot range's face, size, colour and fill agree with 26.2.4.2, or not?

    clearfour.py <workbook> <workbook.fods> ...   (pairs)

For every cell of every *worksheet-cached* pivot's stated rectangle this scores two models of
what the four properties should be, against 26.2.4.2's own resolved view of the same cell:

  merge   the cell keeps what the workbook states — what this tree ships
  clear   the cell is taken back to the sheet's default `cellXf`, which is what
          `clearContents(… HARDATTR | STYLES …)` (`pivottablebuffer.cxx`:1331-1336) and
          `DeleteAreaTab(…, ALL)` (`dpoutput.cxx`:1226) would leave

The comparison is the boolean *does this property differ from the document's own default*,
taken on both sides — from the workbook's `cellXfs` on ours and from the `Default` cell style
of the reference's `.fods` on the reference's. That avoids resolving a theme colour through two
different pipelines to compare them, and it is exactly the question the two models disagree
about: `clear` predicts "never differs" for every cell, `merge` predicts "differs iff the
workbook states something".

Where the two models disagree the reference's own value is printed, so a disagreement can be
read rather than only counted.
"""
import sys, os, re, zipfile
import xml.etree.ElementTree as ET

M = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
NS = {'office': 'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
      'table': 'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
      'style': 'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
      'fo': 'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0'}


def m(t):
    return '{%s}%s' % (M, t)


def fq(n):
    p, l = n.split(':')
    return '{%s}%s' % (NS[p], l)


PROPS = ('face', 'class', 'size', 'colour', 'fill')
FODPROP = (('face', fq('style:text-properties'), fq('style:font-name')),
           ('size', fq('style:text-properties'), fq('fo:font-size')),
           ('colour', fq('style:text-properties'), fq('fo:color')),
           ('fill', fq('style:table-cell-properties'), fq('fo:background-color')))


def colnum(s):
    n = 0
    for ch in s:
        n = n * 26 + ord(ch) - 64
    return n - 1


def colname(c):
    s = ''
    c += 1
    while c:
        c, r = divmod(c - 1, 26)
        s = chr(65 + r) + s
    return s


def parse_ref(ref):
    a, _, b = ref.partition(':')
    b = b or a
    ma = re.match(r'\$?([A-Z]+)\$?(\d+)', a)
    mb = re.match(r'\$?([A-Z]+)\$?(\d+)', b)
    return colnum(ma.group(1)), int(ma.group(2)) - 1, colnum(mb.group(1)), int(mb.group(2)) - 1


# ---------------------------------------------------------------- the workbook

def colour_of(e):
    if e is None:
        return None
    if e.get('rgb'):
        return ('rgb', e.get('rgb').upper())
    if e.get('theme') is not None:
        return ('theme', e.get('theme'), e.get('tint') or '0')
    if e.get('indexed') is not None:
        return ('indexed', e.get('indexed'))
    return None


def book_styles(z):
    st = ET.fromstring(z.read('xl/styles.xml'))
    fonts = []
    for f in st.find(m('fonts')) or []:
        name = f.find(m('name'))
        sz = f.find(m('sz'))
        fam = f.find(m('family'))
        fonts.append((name.get('val') if name is not None else None,
                      fam.get('val') if fam is not None else None,
                      sz.get('val') if sz is not None else None,
                      colour_of(f.find(m('color')))))
    fills = []
    for fl in st.find(m('fills')) or []:
        p = fl.find(m('patternFill'))
        if fl.find(m('gradientFill')) is not None:
            fills.append(('gradient',))
        elif p is None or p.get('patternType') in (None, 'none'):
            fills.append(None)
        else:
            fills.append((p.get('patternType'), colour_of(p.find(m('fgColor'))),
                          colour_of(p.find(m('bgColor')))))
    out = []
    for xf in st.find(m('cellXfs')) or []:
        fid = int(xf.get('fontId', '0'))
        flid = int(xf.get('fillId', '0'))
        face, cls, size, col = (fonts[fid] if fid < len(fonts) else (None, None, None, None))
        out.append((face, cls, size, col, fills[flid] if flid < len(fills) else None))
    return out


# ---------------------------------------------------------------- the reference

ROWHOLDERS = (fq('table:table-header-rows'), fq('table:table-row-group'), fq('table:table-rows'))
COLHOLDERS = (fq('table:table-header-columns'), fq('table:table-column-group'),
              fq('table:table-columns'))


def walk(node, want, holders):
    for child in node:
        if child.tag == want:
            yield child
        elif child.tag in holders:
            yield from walk(child, want, holders)


def font_alias(root):
    """`style:font-name` -> the family it actually names.

    The reference declares a second `<style:font-face>` for the same family whenever two
    styles disagree about its generic class — `Consolas` swiss and `Consolas1` modern in one
    file, `Gill Sans MT` and `Gill Sans MT1` in another — so comparing the *names* reads one
    family as two faces.
    """
    out = {}
    for holder in root:
        if holder.tag != fq('office:font-face-decls'):
            continue
        for ff in holder:
            nm = ff.get(fq('style:name'))
            fam = ff.get('{urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0}font-family')
            gen = ff.get(fq('style:font-family-generic'))
            if nm:
                out[nm] = ((fam or nm).strip("'"), gen)
    return out


def fods_sheet(root, sheet, wanted):
    """(row, col) -> resolved {face,size,colour,fill}; plus the Default's own.

    `wanted` bounds the rectangle materialised: a `.fods` states a trailing row repeated tens
    of thousands of times over a thousand columns, and materialising that is minutes of
    nothing.
    """
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

    cache = {}

    def resolve(name):
        if name in cache:
            return cache[name]
        chain, seen, n = [], set(), name
        while n and n in raw and n not in seen:
            seen.add(n)
            chain.append(n)
            n = raw[n][1]
        out = {}
        for n in reversed(chain):
            out.update(raw[n][0])
        cache[name] = out
        return out

    alias = font_alias(root)

    def normalise(v):
        v = dict(v)
        if 'face' in v:
            family, generic = alias.get(v['face'], (v['face'], None))
            v['face'] = family
            v['class'] = generic
        return v

    default = normalise(resolve('Default'))
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
    rowsWanted = set()
    for c0, r0, c1, r1 in wanted:
        rowsWanted.update(range(r0, r1 + 1))

    grid = {}
    r = -1
    for row in walk(table, fq('table:table-row'), ROWHOLDERS):
        rep = int(row.get(fq('table:number-rows-repeated'), 1))
        if not (rowsWanted & set(range(r + 1, min(r + rep, lastrow) + 1))):
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
            if r not in rowsWanted:
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
                    grid[(r, c)] = normalise(resolve(eff)) if eff else default
                if c > lastcol:
                    break
        if r > lastrow:
            break
    return grid, default


# ---------------------------------------------------------------- the score

TOT = {k: {'cells': 0, 'merge': 0, 'clear': 0} for k in PROPS}
JOINT = {'merge': 0, 'clear': 0}
SHOWN = 0

for i in range(1, len(sys.argv), 2):
    path, fpath = sys.argv[i], sys.argv[i + 1]
    z = zipfile.ZipFile(path)
    name = os.path.basename(path)
    xfs = book_styles(z)
    base = xfs[0] if xfs else (None, None, None, None, None)
    fodsroot = ET.parse(fpath).getroot()

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
        cols = {}
        cse = ws.find(m('cols'))
        if cse is not None:
            for c in cse:
                if c.get('style') is None:
                    continue
                lo, hi = int(c.get('min', '1')) - 1, int(c.get('max', '1')) - 1
                for cc in range(lo, min(hi, 16383) + 1):
                    cols[cc] = int(c.get('style'))
        rows, cells = {}, {}
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
        for pt in pts:
            loc0 = ET.fromstring(z.read(pt)).find(m('location'))
            if loc0 is not None:
                wanted.append(parse_ref(loc0.get('ref')))
        grid, fdefault = fods_sheet(fodsroot, sh.get('name'), wanted)
        if grid is None:
            print('%s\t%s\tNO SUCH SHEET IN THE FODS' % (name, sh.get('name')))
            continue

        for pt in pts:
            root = ET.fromstring(z.read(pt))
            prp = os.path.join(os.path.dirname(pt), '_rels', os.path.basename(pt) + '.rels')
            kind = '?'
            if prp in z.namelist():
                for r in ET.fromstring(z.read(prp)):
                    if r.get('Type', '').endswith('/pivotCacheDefinition'):
                        raw = r.get('Target')
                        cp = (raw.lstrip('/') if raw.startswith('/')
                              else os.path.normpath(os.path.join(os.path.dirname(pt), raw))).replace('\\', '/')
                        if cp in z.namelist():
                            src = ET.fromstring(z.read(cp)).find(m('cacheSource'))
                            kind = src.get('type') if src is not None else '?'
                        break
            if kind != 'worksheet':
                continue

            loc = root.find(m('location'))
            c0, r0, c1, r1 = parse_ref(loc.get('ref'))
            score = {k: {'cells': 0, 'merge': 0, 'clear': 0} for k in PROPS}
            examples = []
            for rr in range(r0, r1 + 1):
                for cc in range(c0, c1 + 1):
                    s = cells.get((rr, cc), rows.get(rr, cols.get(cc, 0)))
                    v = xfs[s] if s < len(xfs) else base
                    ref = grid.get((rr, cc), fdefault)
                    jm = jc = True
                    for k, key in enumerate(PROPS):
                        stated_differs = v[k] != base[k]
                        ref_differs = ref.get(key) != fdefault.get(key)
                        score[key]['cells'] += 1
                        if stated_differs == ref_differs:
                            score[key]['merge'] += 1
                        elif key in ('face', 'class'):
                            jm = False
                        if not ref_differs:
                            score[key]['clear'] += 1
                        elif key in ('face', 'class'):
                            jc = False
                        if stated_differs != ref_differs and len(examples) < 4:
                            examples.append('%s%d %s ours-states=%s ref=%s (default %s)' % (
                                colname(cc), rr + 1, key, stated_differs,
                                ref.get(key), fdefault.get(key)))
                    JOINT['merge'] += 1 if jm else 0
                    JOINT['clear'] += 1 if jc else 0
            for key in PROPS:
                for f in ('cells', 'merge', 'clear'):
                    TOT[key][f] += score[key][f]
            print('%s\t%s\t%s\tref=%s\tcells=%d\t%s' % (
                name, sh.get('name'), os.path.basename(pt), loc.get('ref'),
                score['face']['cells'],
                '  '.join('%s merge=%d clear=%d' % (k, score[k]['merge'], score[k]['clear'])
                          for k in PROPS)))
            for e in examples:
                print('      %s' % e)

print()
print('%-8s %8s %10s %10s' % ('property', 'cells', 'merge', 'clear'))
for k in PROPS:
    print('%-8s %8d %10d %10d' % (k, TOT[k]['cells'], TOT[k]['merge'], TOT[k]['clear']))
print()
print('%-8s %8d %10d %10d   (face and class together: a face named without its declared'
      % ('font', TOT['face']['cells'], JOINT['merge'], JOINT['clear']))
print('%-8s %8s %10s %10s    generic class is a different font to resolve, so the two are'
      % ('', '', '', ''))
print('%-8s %8s %10s %10s    one property and are scored as one)' % ('', '', '', ''))
