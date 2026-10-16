#!/usr/bin/env python3
"""Census, from 26.2.4.2's own resolved view, of the cells whose text carries a rule.

For every spreadsheet in the sheets track: convert with `--convert-to fods` through
/opt/libreoffice26.2, then count, per document,

  link_cells      cells holding a <text:a> with non-empty text  (the hyperlink-field half:
                  Calc paints these through ScEditUtil::GetCellFieldValue, which forces
                  LINESTYLE_SINGLE and the LINKS colour at paint time)
  ul_cells        cells with non-empty text whose resolved cell style states
                  style:text-underline-style other than "none"   (the font-attribute half)
  ul_link_cells   cells that are both
  shape_ul        underlined spans inside a drawing shape's text (counted apart)

The two halves are counted separately on purpose: the fods export writes the cell's
resolved *style* and NOT the field's paint-time attributes -- ScXMLExport calls
GetCellFieldValue(*pField, &rDoc, nullptr, nullptr) (sc/source/filter/xml/xmlexprt.cxx:3062),
so neither the LINKS colour nor the underline reaches the flat file.  A hyperlink cell
therefore reads `underline=none` here while the reference's own PDF strokes a rule under it.
"""
import os, re, subprocess, sys, tempfile, shutil
import xml.etree.ElementTree as ET
from concurrent.futures import ThreadPoolExecutor

SOFFICE = '/opt/libreoffice26.2/program/soffice'
ROOT = sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files/sheets'
OUT = sys.argv[2] if len(sys.argv) > 2 else 'fods-census.tsv'
JOBS = int(sys.argv[3]) if len(sys.argv) > 3 else 3

NS = {'office': 'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
      'style': 'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
      'table': 'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
      'text': 'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
      'draw': 'urn:oasis:names:tc:opendocument:xmlns:drawing:1.0',
      'fo': 'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0'}
Q = lambda p, l: '{%s}%s' % (NS[p], l)


def styles_of(root):
    out = {}
    for scope in ('office:styles', 'office:automatic-styles'):
        el = root.find(scope, NS)
        if el is None:
            continue
        for st in el.findall('style:style', NS):
            tp = st.find('style:text-properties', NS)
            out[st.get(Q('style', 'name'))] = (
                st.get(Q('style', 'parent-style-name')),
                tp.get(Q('style', 'text-underline-style')) if tp is not None else None)
    return out


def resolve(styles, name):
    seen = 0
    while name and seen < 16:
        v = styles.get(name)
        if v is None:
            return None
        if v[1] is not None:
            return v[1]
        name, seen = v[0], seen + 1
    return None


def measure(fods):
    root = ET.parse(fods).getroot()
    styles = styles_of(root)
    link = ul = both = shape = 0
    body = root.find('office:body', NS)
    for cell in body.iter(Q('table', 'table-cell')):
        text = ''.join(cell.itertext()).strip()
        if not text:
            continue
        has_a = cell.find('.//text:a', NS) is not None
        u = resolve(styles, cell.get(Q('table', 'style-name')))
        # a span inside the cell may state its own underline
        if u in (None, 'none'):
            for span in cell.iter(Q('text', 'span')):
                if resolve(styles, span.get(Q('text', 'style-name'))) not in (None, 'none'):
                    u = 'solid'
                    break
        under = u not in (None, 'none')
        link += has_a
        ul += under
        both += has_a and under
    for frame in body.iter(Q('draw', 'frame')):
        for span in frame.iter(Q('text', 'span')):
            if resolve(styles, span.get(Q('text', 'style-name'))) not in (None, 'none'):
                shape += 1
    return link, ul, both, shape


def one(path):
    tmp = tempfile.mkdtemp(prefix='fodscensus-')
    try:
        r = subprocess.run(
            ['timeout', '-k', '30', '600', SOFFICE,
             '-env:UserInstallation=file://' + tmp + '/profile',
             '--headless', '--norestore', '--convert-to', 'fods',
             '--outdir', tmp, path],
            capture_output=True, text=True)
        got = [f for f in os.listdir(tmp) if f.endswith('.fods')]
        if not got:
            return (path, 'ref-failed', 0, 0, 0, 0)
        return (path, 'ok') + measure(os.path.join(tmp, got[0]))
    except Exception as exc:                                  # noqa: BLE001
        return (path, 'error:' + type(exc).__name__, 0, 0, 0, 0)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


docs = sorted(os.path.join(d, f)
              for d, _, fs in os.walk(ROOT) for f in fs
              if os.path.splitext(f)[1].lower() in ('.xlsx', '.xlsm', '.xls', '.xlsb', '.ods'))
print('%d documents' % len(docs), file=sys.stderr)
with open(OUT, 'w') as fh:
    fh.write('path\tstatus\tlink_cells\tul_cells\tul_link_cells\tshape_ul\n')
    with ThreadPoolExecutor(JOBS) as pool:
        for i, row in enumerate(pool.map(one, docs)):
            fh.write('\t'.join(str(x) for x in row) + '\n')
            fh.flush()
            if i % 25 == 0:
                print('%d/%d' % (i, len(docs)), file=sys.stderr)
