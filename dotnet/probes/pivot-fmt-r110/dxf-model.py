#!/usr/bin/env python3
"""`FormatOutput`, transcribed — which cells each `<format>`'s `dxf` lands on.

    dxf-model.py <workbook.xlsx> <workbook.fods> ...   (pairs)

`ScDPOutput::Output` clears the whole output range and then, as its last act, calls
`maFormatOutput.apply` (`dpoutput.cxx`:1190), which lays the pivot's own `<format>` records over
the generated styles. This is that matcher, transcribed from
`sc/source/core/data/PivotTableFormatOutput.cxx` and `sc/source/filter/oox/PivotTableFormat.cxx`
in the C++ tree read here, and scored against 26.2.4.2's own `.fods`.

**The clearing is total, and this file's null proves it.** `033_Event_planning_tracker` with its
`<formats>` element deleted and nothing else changed comes back from 26.2.4.2 with every one of
the 91 cells of its pivot rectangle at the `Default` cell style — no face, no size, no colour, no
fill — and only the generated bold on the grand-total row. Every one of the properties this round
censused is therefore cleared, and everything the reference draws over them afterwards is a
`dxf`.

Five rules of the C++ that are not obvious and that each change the answer:

  * `PivotAreaType` is parsed and **never read**. `PivotTableFormat::finalizeImport` uses
    `dataOnly`, `labelOnly`, `outline`, `grandRow`, `grandCol`, `fieldPosition`, `offset` and the
    references, and nothing else — so `type="all"` and `type="button"` behave as `normal`.
  * The format's kind is `Data` when `dataOnly` (default **true**), else `Label` when
    `labelOnly`, else `None` — and a `None` entry applies nothing at all, because
    `applyMatchedLines` has a branch for `Label` and a branch for `Data` and no third one. Six of
    `033`'s eighty-four are `dataOnly="0"` with no `labelOnly`, and all six are inert.
  * A format with no references matches **every** line through the broad path, so a bare
    `<pivotArea outline="0"/>` paints the whole data area.
  * `grandRow`/`grandCol` short-circuit before any line matching (`tryHandleGrandTotals`).
  * A `dxf` fill that states a `bgColor` and **no** `patternType` becomes a *solid* fill whose
    colour is the pattern colour, which for an automatic colour is the window **text** colour —
    black. `Fill::finalizeImport`, `stylesbuffer.cxx`:1978-2000. That, and not the workbook, is
    where `033`'s black band comes from; `patternType="none"` beside a `bgColor`, which the same
    file also states, applies nothing.
"""
import sys, os, zipfile, importlib.util
import xml.etree.ElementTree as ET

HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location(
    'pg', os.path.join(HERE, '..', 'pivot-res-r108', 'pivot-grid.py'))
pg = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(pg)

m = pg.m
children = pg.children
colname = pg.colname
HASMEMBER, SUBTOTAL, CONTINUE = pg.HASMEMBER, pg.SUBTOTAL, pg.CONTINUE

DATA_DIM = -2


# ---------------------------------------------------------------- the format records

class Selection:
    def __init__(self, field, selected, indices, has_subtotal):
        self.field = field
        self.selected = selected
        self.indices = indices
        self.has_subtotal = has_subtotal


class Format:
    """`sc::PivotTableFormat`, built the way `PivotTableFormat::finalizeImport` builds it."""

    SUBTOTAL_FLAGS = ('defaultSubtotal', 'sumSubtotal', 'countASubtotal', 'avgSubtotal',
                      'maxSubtotal', 'minSubtotal', 'productSubtotal', 'countSubtotal',
                      'stdDevSubtotal', 'stdDevPSubtotal', 'varSubtotal', 'varPSubtotal')

    def __init__(self, el):
        self.dxf = int(el.get('dxfId', '-1'))
        area = el.find(m('pivotArea'))
        self.data_only = pg.flag(area, 'dataOnly', True) if area is not None else True
        self.label_only = pg.flag(area, 'labelOnly', False) if area is not None else False
        self.grand_row = pg.flag(area, 'grandRow', False) if area is not None else False
        self.grand_col = pg.flag(area, 'grandCol', False) if area is not None else False
        self.offset = area.get('offset') if area is not None else None
        #  FormatType: Data when dataOnly, else Label when labelOnly, else None.
        self.kind = 'data' if self.data_only else ('label' if self.label_only else 'none')

        self.selections = []
        refs = area.find(m('references')) if area is not None else None
        for ref in (refs if refs is not None else []):
            if ref.get('field') is None:
                continue
            #  An unsigned 4294967294 read back as a signed 32-bit value is -2, the data
            #  dimension, which is how the file spells "the data-layout field".
            field = int(ref.get('field'))
            if field >= 1 << 31:
                field -= 1 << 32
            indices = [int(x.get('v')) for x in ref if x.get('v') is not None]
            self.selections.append(Selection(
                field,
                pg.flag(ref, 'selected', True),
                indices,
                any(pg.flag(ref, f, False) for f in Format.SUBTOTAL_FLAGS)))

    def selection(self, dimension):
        for s in self.selections:
            if s.field == dimension:
                return s
        return None


class OutField:
    """`sc::FormatOutputField`."""

    def __init__(self, dimension):
        self.dimension = dimension
        self.index = -1
        self.matches_all = False
        self.selected = True
        self.has_subtotal = False
        self.is_set = False


class FieldData:
    """`sc::FieldData`, one field of one output line."""

    def __init__(self, dimension):
        self.dimension = dimension
        self.index = -1
        self.is_set = False
        self.is_member = False
        self.subtotal = False
        self.cont = False


class Line:
    def __init__(self, count, dims):
        self.line = None
        self.position = None
        self.fields = [FieldData(dims[i]) for i in range(count)]


# ---------------------------------------------------------------- the matcher

def find_matching_lines(lines, out_fields, kind):
    """`findMatchingLines`."""
    matches, broad = [], []
    for line in lines:
        all_match = True
        has_subtotal_match = False
        has_wildcard = False
        for i, fd in enumerate(line.fields):
            fe = out_fields[i]
            if fd.dimension != fe.dimension:
                all_match = False
                break
            if not fe.is_set:
                continue
            ok = False
            if fe.has_subtotal:
                if fd.subtotal and fd.index == fe.index:
                    ok = True
                    has_subtotal_match = True
                elif not fe.selected and not fd.subtotal and fd.index == fe.index:
                    ok = True
            elif fe.matches_all and not fd.subtotal:
                ok = True
            elif fd.index == fe.index and not (fe.dimension != DATA_DIM and fd.subtotal):
                ok = True
            if not ok:
                all_match = False
                break
        if not all_match:
            continue
        for i, fd in enumerate(line.fields):
            fe = out_fields[i]
            if fe.is_set:
                continue
            if kind == 'label':
                if not has_subtotal_match and not fd.is_member and not fd.cont:
                    all_match = False
                    break
            elif kind == 'data':
                if not has_subtotal_match and (fd.is_member or fd.cont):
                    has_wildcard = True
        if not all_match:
            continue
        (broad if has_wildcard else matches).append(line)
    if kind == 'data' and not matches and broad:
        matches = broad
    return matches


def matched_field_index(out_fields):
    for i, f in enumerate(out_fields):
        if f.is_set:
            return i
    return 0


def has_set_fields(fields):
    return any(f.is_set for f in fields)


class Applier:
    """`FormatOutput::apply` over one accepted pivot."""

    def __init__(self, geometry, row_lines, column_lines, row_dims, column_dims,
                 grand_row, grand_column):
        self.g = geometry
        self.row_lines = row_lines
        self.column_lines = column_lines
        self.row_dims = row_dims
        self.column_dims = column_dims
        self.grand_row = grand_row
        self.grand_column = grand_column

    def entries(self, fmt):
        """`FormatOutput::prepare` — one entry per selection index."""
        #  `nMaxNumberOfIndices` is assigned rather than maxed: where two references each name
        #  several members, the last one decides.  No corpus pivot states two.
        widest = 1
        for s in fmt.selections:
            if len(s.indices) > 1:
                widest = len(s.indices)
        out = []
        for at in range(widest):
            rows = [OutField(d) for d in self.row_dims]
            cols = [OutField(d) for d in self.column_dims]
            for fields in (rows, cols):
                for f in fields:
                    s = fmt.selection(f.dimension)
                    if s is None:
                        continue
                    if not s.indices:
                        f.matches_all = True
                    else:
                        f.index = s.indices[at] if len(s.indices) > 1 and at < len(s.indices) \
                            else s.indices[0]
                    f.selected = s.selected
                    f.has_subtotal = s.has_subtotal
                    f.is_set = True
            out.append((rows, cols))
        return out

    #  **26.2.4.2 has no grand-total short circuit.** `tryHandleGrandTotals`
    #  (`PivotTableFormatOutput.cxx`:582) is in the C++ tree read here, which declares
    #  27.2.0.0.alpha0+, and the binary does not behave as if it were there: applied one at a
    #  time to `033_Event_planning_tracker`, its four `grandRow="1"` data formats paint the
    #  *whole* data area rather than the grand-total row — `#9` and `#22` white over `C6:N11`,
    #  `#29` black — and its `grandRow="1"` *label* formats paint nothing at all, which is what
    #  ordinary matching gives a label with no references. Four formats, four predictions, and
    #  the grand-total path predicts none of them. So the entries fall through, and this flag
    #  records that it is a measurement of the binary rather than a reading of the tree.
    GRAND_TOTAL_SHORT_CIRCUIT = False

    LABEL_MATCHES_BOTH = False

    def apply(self, fmt, put):
        """`put(column, row, fmt)` for every cell the format lands on."""
        for rows, cols in self.entries(fmt):
            if self.GRAND_TOTAL_SHORT_CIRCUIT and self.grand_totals(fmt, rows, cols, put):
                continue

            hit_rows, hit_columns = [], []
            match_rows = fmt.kind != 'label' or has_set_fields(rows) or self.LABEL_MATCHES_BOTH
            match_columns = fmt.kind != 'label' or has_set_fields(cols) or self.LABEL_MATCHES_BOTH

            header_start = self.g['dsr']
            if self.column_lines:
                header_start -= len(self.column_lines[0].fields)

            if match_rows:
                for line in find_matching_lines(self.row_lines, rows, fmt.kind):
                    self.place(fmt, line, rows, 'row', hit_rows, hit_columns, header_start, put)
            if match_columns:
                for line in find_matching_lines(self.column_lines, cols, fmt.kind):
                    self.place(fmt, line, cols, 'column', hit_rows, hit_columns, header_start, put)

            if hit_rows and hit_columns and fmt.kind == 'data':
                for r in hit_rows:
                    for c in hit_columns:
                        put(c, r, fmt)

    def place(self, fmt, line, out_fields, direction, hit_rows, hit_columns, header_start, put):
        """`applyMatchedLines` for one line."""
        if line.line is None or line.position is None:
            return
        if fmt.kind == 'label':
            at = matched_field_index(out_fields)
            if direction == 'row':
                put(self.g['tsc'] + at, line.line, fmt)
            else:
                put(line.line, header_start + at, fmt)
        elif fmt.kind == 'data':
            (hit_rows if direction == 'row' else hit_columns).append(line.line)

    def grand_totals(self, fmt, rows, cols, put):
        """`tryHandleGrandTotals`."""
        if fmt.grand_row and fmt.grand_col and self.grand_row >= 0 and self.grand_column >= 0:
            put(self.grand_column, self.grand_row, fmt)
            return True
        if fmt.grand_row and self.grand_row >= 0:
            if fmt.kind == 'data':
                for line in self.grand_lines(self.column_lines, cols, self.grand_column):
                    put(line, self.grand_row, fmt)
            elif fmt.kind == 'label':
                for c in range(self.g['tsc'], self.g['dsc']):
                    put(c, self.grand_row, fmt)
            return True
        if fmt.grand_col and self.grand_column >= 0:
            if fmt.kind == 'data':
                for line in self.grand_lines(self.row_lines, rows, self.grand_row):
                    put(self.grand_column, line, fmt)
            elif fmt.kind == 'label':
                for row in sorted({l.position for l in self.column_lines if l.position is not None}):
                    put(self.grand_column, row, fmt)
            return True
        return False

    def grand_lines(self, lines, out_fields, skip):
        chosen = find_matching_lines(lines, out_fields, 'none') if has_set_fields(out_fields) \
            else lines
        return [l.line for l in chosen if l.line is not None and l.line != skip]


# ---------------------------------------------------------------- the dxf table

def colour_of(e):
    if e is None:
        return None
    if e.get('rgb'):
        return ('rgb', e.get('rgb').upper())
    if e.get('theme') is not None:
        return ('theme', e.get('theme'), e.get('tint') or '0')
    if e.get('indexed') is not None:
        return ('indexed', e.get('indexed'))
    if e.get('auto') in ('1', 'true'):
        return ('auto',)
    return None


def read_dxfs(z):
    """Index -> the five properties a `dxf` states, or None where it states none."""
    if 'xl/styles.xml' not in z.namelist():
        return []
    root = ET.fromstring(z.read('xl/styles.xml'))
    out = []
    for dxf in (root.find(m('dxfs')) if root.find(m('dxfs')) is not None else []):
        p = {}
        font = dxf.find(m('font'))
        if font is not None:
            name = font.find(m('name'))
            fam = font.find(m('family'))
            sz = font.find(m('sz'))
            if name is not None:
                p['face'] = name.get('val')
            if fam is not None:
                p['class'] = fam.get('val')
            if sz is not None:
                p['size'] = sz.get('val')
            if font.find(m('color')) is not None:
                p['colour'] = colour_of(font.find(m('color')))
            b = font.find(m('b'))
            if b is not None:
                p['weight'] = 400 if b.get('val') in ('0', 'false') else 700
        fill = dxf.find(m('fill'))
        if fill is not None:
            p['fill'] = dxf_fill(fill)
        out.append(p)
    return out


def dxf_fill(fill):
    """`Fill::finalizeImport`'s dxf arm, which is where `033`'s black band comes from.

    A `<patternFill>` that states a `bgColor` and no `patternType` is turned into a **solid**
    fill whose colour is the *pattern* colour — and an unstated pattern colour is automatic,
    which resolves to the window text colour, black. A `patternType="none"` beside the same
    `bgColor` applies nothing at all.
    """
    pattern = fill.find(m('patternFill'))
    if pattern is None:
        return None
    fg = pattern.find(m('fgColor'))
    bg = pattern.find(m('bgColor'))
    pattern_used = pattern.get('patternType') is not None
    kind = pattern.get('patternType')
    patt_colour_used = fg is not None
    fill_colour_used = bg is not None

    if fill_colour_used and (not pattern_used or kind == 'solid'):
        patt = colour_of(bg)
        kind = 'solid'
        patt_colour_used = True
    elif not fill_colour_used and not patt_colour_used and pattern_used and kind == 'solid':
        return None                      # mbPatternUsed = false
    else:
        patt = colour_of(fg)

    if not pattern_used and not fill_colour_used:
        return None
    if kind == 'none':
        return None                      # "XML_none should not apply any color"
    if not patt_colour_used:
        patt = ('auto',)
    return ('solid' if kind == 'solid' else kind, patt)


# ---------------------------------------------------------------- the driver

def axis_lines(items, dims, count):
    """One `LineData` per output position, with `FieldData` per field."""
    nfields = len(dims)
    flags = pg.read_axis(items, nfields, count)
    lines = [Line(nfields, dims) for _ in range(count)]
    for pos in range(min(count, len(items))):
        it = items[pos]
        repeated = max(0, min(int(it.get('r', 0)), nfields))
        stated = [int(x.get('v', 0)) for x in it.findall(m('x'))] or [0]
        for f in range(nfields):
            fd = lines[pos].fields[f]
            fl = flags[f][pos]
            fd.is_member = bool(fl & HASMEMBER)
            fd.subtotal = bool(fl & SUBTOTAL)
            fd.cont = bool(fl & CONTINUE)
            fd.is_set = bool(fl)
            if dims[f] == DATA_DIM:
                #  `rFieldData.nIndex = nMemberIndex`, the position, which for the data-layout
                #  dimension is the data field's own index.
                fd.index = pos
            elif f >= repeated and f - repeated < len(stated):
                fd.index = stated[f - repeated]
        #  A CONTINUE field takes the name and index of the last line that stated one.
        for f in range(nfields):
            if lines[pos].fields[f].cont and pos > 0:
                at = pos - 1
                while at >= 0 and lines[at].fields[f].cont:
                    at -= 1
                if at >= 0:
                    lines[pos].fields[f].index = lines[at].fields[f].index
    return lines


def build(root, geometry):
    """The row and column lines, positioned, and the applier over them."""
    rowfields = children(root.find(m('rowFields')), 'field')
    colfields = children(root.find(m('colFields')), 'field')
    rowitems = children(root.find(m('rowItems')), 'i')
    colitems = children(root.find(m('colItems')), 'i')
    datafields = children(root.find(m('dataFields')), 'dataField')

    def dim(f):
        x = int(f.get('x', '-1'))
        return DATA_DIM if x < 0 else x

    row_dims = [dim(f) for f in rowfields]
    column_dims = [dim(f) for f in colfields]
    if len(column_dims) == 1 and column_dims[0] == DATA_DIM and len(datafields) <= 1:
        column_dims = [DATA_DIM]         # the one synthetic data column of `insertEmptyDataColumn`

    row_lines = axis_lines(rowitems, row_dims, len(rowitems))
    column_lines = axis_lines(colitems, column_dims, len(colitems))
    for n, line in enumerate(row_lines):
        line.line = geometry['dsr'] + n
        line.position = geometry['tsc']
    for n, line in enumerate(column_lines):
        line.line = geometry['dsc'] + n
        line.position = geometry['dsr'] - len(column_dims)

    #  `nGrandTotalRow`/`nGrandTotalColumn` are the table's own last row and column whenever the
    #  data pilot's RowGrand/ColumnGrand say so, and the OOXML import takes those from
    #  `rowGrandTotals`/`colGrandTotals`, both defaulting to true.
    grand_row = geometry['ter'] if pg.flag(root, 'rowGrandTotals', True) else -1
    grand_column = geometry['tec'] if pg.flag(root, 'colGrandTotals', True) else -1

    return Applier(geometry, row_lines, column_lines, row_dims, column_dims,
                   grand_row, grand_column)


def resolve(root, dxfs):
    """(row, column) -> the properties the `<format>` records leave on that cell."""
    out = {}
    grid, why = pg.generate(root)
    if grid is None:
        return None, why
    geometry = {'tsc': grid.tsc, 'tsr': grid.tsr, 'dsc': grid.dsc, 'dsr': grid.dsr,
                'tec': grid.tec, 'ter': grid.ter}
    applier = build(root, geometry)

    formats = children(root.find(m('formats')), 'format')
    for el in formats:
        fmt = Format(el)
        if fmt.kind == 'none':
            continue
        props = dxfs[fmt.dxf] if 0 <= fmt.dxf < len(dxfs) else {}
        if not props:
            continue

        def put(column, row, _fmt, props=props):
            out.setdefault((row, column), {}).update(props)

        applier.apply(fmt, put)
    return out, None
