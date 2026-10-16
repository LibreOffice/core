#!/usr/bin/env python3
"""Which corpus workbooks give cellXfs[0] and the `Normal` cellStyleXf different content, and
does the difference survive the apply* flags?

Reads the document list from a file (one path a line) so a filename with a space is not split.
"""
import sys, zipfile, os
import xml.etree.ElementTree as ET

M = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'
def m(t): return '{%s}%s' % (M, t)

ALIGN_DEFAULTS = {
    'horizontal': 'general', 'vertical': 'bottom', 'textRotation': '0',
    'wrapText': '0', 'indent': '0', 'shrinkToFit': '0', 'readingOrder': '0',
    'justifyLastLine': '0', 'relativeIndent': '0',
}
def boolish(v):
    return '0' if v in ('0', 'false', 'False') else '1' if v in ('1', 'true', 'True') else v

def align_of(xf):
    """The alignment an xf effectively states, defaults folded out."""
    al = xf.find(m('alignment'))
    if al is None:
        return {}
    out = {}
    for k, v in al.attrib.items():
        vv = boolish(v) if k in ('wrapText', 'shrinkToFit', 'justifyLastLine') else v
        if ALIGN_DEFAULTS.get(k) == vv:
            continue
        out[k] = vv
    return out

def flag(xf, name, default):
    v = xf.get(name)
    if v is None:
        return default
    return v not in ('0', 'false')

def effective(xf, is_cellxf):
    """What this xf actually contributes, after the apply* rules Xf::importXf states.

    applyFont/Fill/Border/NumberFormat default to true when the matching id is non-zero
    (stylesbuffer.cxx:2176); applyAlignment is forced true by the presence of <alignment>
    (:2186).  A cellStyleXf has no apply* semantics at all in the reader — it is the base —
    so for it everything stated is contributed.
    """
    font = int(xf.get('fontId', '0') or 0)
    fill = int(xf.get('fillId', '0') or 0)
    bord = int(xf.get('borderId', '0') or 0)
    numf = int(xf.get('numFmtId', '0') or 0)
    al = align_of(xf)
    if not is_cellxf:
        return {'fontId': font, 'fillId': fill, 'borderId': bord, 'numFmtId': numf, 'align': al}
    return {
        'fontId': font if flag(xf, 'applyFont', font != 0) else 0,
        'fillId': fill if flag(xf, 'applyFill', fill != 0) else 0,
        'borderId': bord if flag(xf, 'applyBorder', bord != 0) else 0,
        'numFmtId': numf if flag(xf, 'applyNumberFormat', numf != 0) else 0,
        'align': al,
    }

def font_desc(fonts, i):
    if i >= len(fonts):
        return '(no font %d)' % i
    f = fonts[i]
    def val(tag, attr='val'):
        e = f.find(m(tag))
        return None if e is None else e.get(attr)
    bits = [val('name') or '?', (val('sz') or '?') + 'pt']
    if f.find(m('b')) is not None: bits.append('bold')
    if f.find(m('i')) is not None: bits.append('italic')
    c = f.find(m('color'))
    if c is not None:
        bits.append('colour=' + (c.get('rgb') or ('theme%s' % c.get('theme')) if (c.get('rgb') or c.get('theme')) else 'auto'))
    fam = val('family')
    if fam: bits.append('family=' + fam)
    return ' '.join(bits)

def cell_default_users(z):
    """How many cells in the workbook take the sheet default, and whether a full-width <col>
    overrides it.  A cell takes it when it states no @s, its row is not customFormat-with-s,
    and no <col> covers its column."""
    names = [n for n in z.namelist() if n.startswith('xl/worksheets/sheet') and n.endswith('.xml')]
    total_default = 0
    total_cells = 0
    fullwidth = []
    for n in sorted(names):
        try:
            ws = ET.fromstring(z.read(n))
        except Exception:
            continue
        colstyle = {}
        cols = ws.find(m('cols'))
        wide = None
        if cols is not None:
            for c in cols:
                if c.get('style') is None:
                    continue
                lo = int(c.get('min', '1')) - 1
                hi = int(c.get('max', str(lo + 1))) - 1
                if hi >= 16383:
                    wide = c.get('style')
                    hi = lo
                for a in range(max(lo, 0), min(hi, 16383) + 1):
                    colstyle[a] = c.get('style')
        if wide is not None:
            fullwidth.append((n.split('/')[-1], wide))
        sd = ws.find(m('sheetData'))
        if sd is None:
            continue
        for row in sd:
            rowfmt = row.get('s') if row.get('customFormat') not in (None, '0', 'false') else None
            for cell in row:
                total_cells += 1
                if cell.get('s') is not None:
                    continue
                if rowfmt is not None:
                    continue
                ref = cell.get('r') or ''
                col = 0
                for ch in ref:
                    if ch.isalpha():
                        col = col * 26 + (ord(ch.upper()) - 64)
                    else:
                        break
                if (col - 1) in colstyle:
                    continue
                total_default += 1
    return total_default, total_cells, fullwidth

def main(listfile):
    paths = [l.rstrip('\n') for l in open(listfile) if l.strip()]
    n = 0
    skipped = 0
    differ = []
    for path in paths:
        try:
            z = zipfile.ZipFile(path)
            if 'xl/styles.xml' not in z.namelist():
                skipped += 1
                continue
            st = ET.fromstring(z.read('xl/styles.xml'))
        except Exception as e:
            print('SKIP', os.path.basename(path), e)
            skipped += 1
            continue
        cx = list(st.find(m('cellXfs')) or [])
        sx = list(st.find(m('cellStyleXfs')) or [])
        if not cx or not sx:
            skipped += 1
            continue
        n += 1
        idx = 0
        for s in (st.find(m('cellStyles')) or []):
            if s.get('builtinId') == '0':
                idx = int(s.get('xfId', '0'))
                break
        if idx >= len(sx):
            continue
        a = effective(cx[0], True)
        b = effective(sx[idx], False)
        if a == b:
            continue
        fonts = list(st.find(m('fonts')) or [])
        d, t, fw = cell_default_users(z)
        differ.append((os.path.basename(path), a, b, fonts, d, t, fw, idx))

    print('xlsx/xlsm carrying both cellXfs and cellStyleXfs: %d   (skipped %d of %d)'
          % (n, skipped, len(paths)))
    print('effectively differing: %d' % len(differ))
    for name, a, b, fonts, d, t, fw, idx in differ:
        print('\n== %s   (Normal is cellStyleXfs[%d])' % (name, idx))
        for k in ('fontId', 'fillId', 'borderId', 'numFmtId'):
            if a[k] != b[k]:
                extra = ''
                if k == 'fontId':
                    extra = '\n        cellXfs[0] font: %s\n        Normal     font: %s' % (
                        font_desc(fonts, a[k]), font_desc(fonts, b[k]))
                print('   %-10s cellXfs[0]=%s  Normal=%s%s' % (k, a[k], b[k], extra))
        if a['align'] != b['align']:
            print('   %-10s cellXfs[0]=%s  Normal=%s' % ('alignment', a['align'] or '{}', b['align'] or '{}'))
        print('   cells taking the sheet default: %d of %d stated cells' % (d, t))
        if fw:
            print('   full-width <col> (overrides the sheet default): %s' % fw)

if __name__ == '__main__':
    main(sys.argv[1])
