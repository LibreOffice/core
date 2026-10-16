#!/usr/bin/env python3
"""Slice AC-150's page-186/187 boundary out of the real document and ablate it.

Why this exists
---------------

See `README.md`. In short: reference page 187 is the first of the document's
three lost pages, both renderings agree line for line to y=583.0 on page 186,
and then the reference moves a caption and its whole ten-row table to the next
page where we place the caption and split the table.

A synthetic probe of "the same shape" does *not* reproduce that, so the shape
has to come out of the real file. This keeps every part of the package
byte-for-byte and rewrites only the body's top-level children, so styles,
numbering, theme, fonts and settings are exactly the document's own.

Modes
-----

    reproduce.py <workdir>            baseline: 13 blocks, both renderings
    reproduce.py <workdir> --ablate   strip one feature at a time
    reproduce.py <workdir> --sweep    vary how much room the table is given

Reading the output
------------------

`rows p1` is how many of the table's data rows landed on the first page. The
reference's is **0** whenever the table does not fit — it moves the table whole
— and ours is not. `lastY` is the last body line, so a reference `lastY` that
moves when a flag is ablated is that flag doing the work.

Two traps, both of which cost a run
-----------------------------------

`ElementTree` reserialisation invents `ns2:` prefixes unless every prefix from
the original `<w:document>` tag is registered first; pasting the original root
tag back over the output instead yields a file LibreOffice rejects with only
"source file could not be loaded". And reading from a `ZipFile` while writing
derived copies in a loop raises `BadZipFile: Bad CRC-32` — read the package into
a dict up front.

Usage
-----

    XRAY_OFFICE_CLI=... python3 reproduce.py /abs/workdir [--ablate|--sweep]
"""

import os
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
import zipfile

SRC = ('/c/sandbox/workdir/sample-files/words/pagination-002/docx/'
       'AC-150-5370-10G-updated-201604.docx')

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'

# The block window, found by locating the `w:tbl` whose text holds both
# "Sieve Designation" and "Percentage by weight" and walking back to the
# `Item P-217 Aggregate-Turf Pavement` heading that opens its page.
FIRST_BLOCK, LAST_BLOCK = 2143, 2155


def load():
    with zipfile.ZipFile(SRC) as z:
        names = [i.filename for i in z.infolist()]
        blob = {n: z.read(n) for n in names}
    return names, blob


def build(path, names, blob, start=FIRST_BLOCK, mutate_body=None, mutate_styles=None):
    raw = blob['word/document.xml'].decode('utf-8')
    header = re.search(r'<w:document[^>]*>', raw, re.S).group(0)
    for prefix, uri in re.findall(r'xmlns:([A-Za-z0-9]+)="([^"]+)"', header):
        ET.register_namespace(prefix, uri)

    root = ET.fromstring(raw)
    body = root.find(W + 'body')
    kids = list(body)
    keep = kids[start:LAST_BLOCK] + [kids[-1]]

    for kid in list(body):
        body.remove(kid)
    for kid in keep:
        body.append(kid)
    if mutate_body:
        mutate_body(body)

    document = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\r\n'
                + ET.tostring(root, encoding='unicode')).encode('utf-8')
    styles = blob['word/styles.xml'].decode('utf-8')
    if mutate_styles:
        styles = mutate_styles(styles)

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as out:
        for name in names:
            if name == 'word/document.xml':
                out.writestr(name, document)
            elif name == 'word/styles.xml':
                out.writestr(name, styles.encode('utf-8'))
            else:
                out.writestr(name, blob[name])


def render(path, outdir):
    subprocess.run(['soffice', '--headless', '--convert-to', 'pdf', '--outdir', outdir, path],
                   capture_output=True, check=True)
    cli = os.environ.get('XRAY_OFFICE_CLI')
    if not cli:
        raise SystemExit('set XRAY_OFFICE_CLI to the tree you mean to measure')
    subprocess.run([cli, 'render', '--quiet', '--outdir', os.path.join(outdir, 'ours'), path],
                   check=True)
    ref = os.path.join(outdir, os.path.basename(path)[:-5] + '.pdf')
    ours = os.path.join(outdir, 'ours', os.path.basename(path)[:-5] + '.pdf')
    for produced in (ref, ours):
        if not os.path.isfile(produced):
            raise SystemExit(f'{produced} was not written — nothing to compare')
    return ref, ours


def measure(pdf):
    """Per page: the last body line, and how many of the table's data rows are on it."""
    text = subprocess.run(['pdftotext', '-bbox', pdf, '-'],
                          capture_output=True, text=True, check=True).stdout
    pages = []
    for page in re.findall(r'<page width[^>]*>(.*?)</page>', text, re.S):
        ys = [float(y) for y in re.findall(r'yMin="([\d.]+)"', page)]
        body = [y for y in ys if y < 720]
        # every data row's first cell is `<n> inch (...)` or `No. <n> (...)`
        rows = len(re.findall(r'>inch</word>|>No\.</word>', page))
        pages.append((round(max(body), 1) if body else 0.0, rows))
    return pages


def strip_headers(body):
    for row_properties in body.iter(W + 'trPr'):
        for header in row_properties.findall(W + 'tblHeader'):
            row_properties.remove(header)


def strip_borders(body):
    for table_properties in body.iter(W + 'tblPr'):
        for borders in table_properties.findall(W + 'tblBorders'):
            table_properties.remove(borders)


def strip_caption_keep(styles):
    return re.sub(r'(<w:style [^>]*w:styleId="Caption".*?)<w:keepNext/>', r'\1', styles,
                  flags=re.S)


def report(label, ref, ours):
    r, o = measure(ref), measure(ours)
    print(f'{label:14s} | ref {len(r):2d}pp lastY {r[0][0]:7.1f} rows p1 {r[0][1]:3d} '
          f'| our {len(o):2d}pp lastY {o[0][0]:7.1f} rows p1 {o[0][1]:3d}')


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/ac-reproduce'
    mode = sys.argv[2] if len(sys.argv) > 2 else ''
    names, blob = load()

    if mode == '--ablate':
        cases = [('baseline', None, None),
                 ('no-tblHeader', strip_headers, None),
                 ('no-keepNext', None, strip_caption_keep),
                 ('neither', strip_headers, strip_caption_keep),
                 ('no-borders', strip_borders, None)]
        for label, mb, ms in cases:
            directory = os.path.join(out, label)
            os.makedirs(os.path.join(directory, 'ours'), exist_ok=True)
            docx = os.path.join(directory, 't.docx')
            build(docx, names, blob, mutate_body=mb, mutate_styles=ms)
            report(label, *render(docx, directory))
        return 0

    starts = range(FIRST_BLOCK, LAST_BLOCK - 1) if mode == '--sweep' else [FIRST_BLOCK]
    for start in starts:
        directory = os.path.join(out, str(start))
        os.makedirs(os.path.join(directory, 'ours'), exist_ok=True)
        docx = os.path.join(directory, 't.docx')
        build(docx, names, blob, start=start)
        report(f'{LAST_BLOCK - start} blocks', *render(docx, directory))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
