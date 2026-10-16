#!/usr/bin/env python3
"""Census, from the files' own markup, of underlined and hyperlinked cells.

Zip spreadsheets only (.xlsx/.xlsm/.xltx/.xltm) -- the 243 of the sheets track's 307 whose
styles are readable without a BIFF parser.  Per document:

  hl_cells   cells covered by a worksheet's <hyperlinks> element that hold a non-empty value
  ul_cells   cells whose *resolved* font carries <u/>, following applyFont over cellXfs and
             cellStyleXfs exactly as ECMA-376 18.8.45 states it
  hl_ul      cells that are both

The point of the pair is the disagreement with fods-census.py, not the agreement: the
underline the reference draws under a hyperlink cell is in neither column.
"""
import os, re, sys, zipfile
import xml.etree.ElementTree as ET

M = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
Q = lambda l: '{%s}%s' % (M, l)


def col_of(ref):
    n = 0
    for ch in ref:
        if not ch.isalpha():
            break
        n = n * 26 + (ord(ch.upper()) - 64)
    return n


def row_of(ref):
    m = re.search(r'\d+', ref)
    return int(m.group(0)) if m else 0


def ranges(text):
    out = []
    for part in text.split():
        a, _, b = part.partition(':')
        b = b or a
        out.append((row_of(a), col_of(a), row_of(b), col_of(b)))
    return out


def one(path):
    with zipfile.ZipFile(path) as z:
        names = set(z.namelist())
        base = 'xl/' if 'xl/styles.xml' in names else next(
            (n[:-len('styles.xml')] for n in names if n.endswith('/styles.xml')), None)
        if base is None:
            return None
        st = ET.fromstring(z.read(base + 'styles.xml'))
        fonts = [f.find(Q('u')) is not None for f in st.find(Q('fonts')) or []]
        def xfs(tag):
            el = st.find(Q(tag))
            return list(el) if el is not None else []
        style_xfs = xfs('cellStyleXfs')
        cell_xfs = xfs('cellXfs')

        def underlined(s):
            try:
                xf = cell_xfs[s]
            except (IndexError, TypeError):
                return False
            fid = xf.get('fontId')
            if xf.get('applyFont') in ('0', 'false') or fid is None:
                xid = xf.get('xfId')
                if xid is not None and int(xid) < len(style_xfs):
                    fid = style_xfs[int(xid)].get('fontId')
            if fid is None:
                return False
            fid = int(fid)
            return fonts[fid] if fid < len(fonts) else False

        wb = ET.fromstring(z.read(base + 'workbook.xml'))
        hl = ul = both = 0
        for sheet_name in sorted(n for n in names
                                 if n.startswith(base + 'worksheets/') and n.endswith('.xml')):
            ws = ET.fromstring(z.read(sheet_name))
            links = []
            for e in ws.find(Q('hyperlinks')) or []:
                links += ranges(e.get('ref', ''))
            for c in ws.iter(Q('c')):
                v = c.find(Q('v'))
                inline = c.find(Q('is'))
                if (v is None or not (v.text or '').strip()) and inline is None:
                    continue
                ref = c.get('r') or ''
                r, col = row_of(ref), col_of(ref)
                s = c.get('s')
                u = underlined(int(s)) if s is not None else False
                linked = any(r0 <= r <= r1 and c0 <= col <= c1 for r0, c0, r1, c1 in links)
                hl += linked
                ul += u
                both += linked and u
        return hl, ul, both


docs = sorted(os.path.join(d, f)
              for d, _, fs in os.walk(sys.argv[1] if len(sys.argv) > 1
                                      else '/home/user/sample-files/sheets')
              for f in fs
              if os.path.splitext(f)[1].lower() in ('.xlsx', '.xlsm', '.xltx', '.xltm', '.xlsb'))
print('path\thl_cells\tul_cells\thl_ul')
for p in docs:
    try:
        got = one(p)
    except Exception as exc:                                  # noqa: BLE001
        print('%s\tERR:%s\t0\t0' % (p, type(exc).__name__))
        continue
    if got is None:
        print('%s\tNOSTYLES\t0\t0' % p)
    else:
        print('%s\t%d\t%d\t%d' % ((p,) + got))
