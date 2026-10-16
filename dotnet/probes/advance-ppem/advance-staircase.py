#!/usr/bin/env python3
"""The first cut, kept because it records why the PDF's own `TJ` channel cannot answer this.

The idea was to read the reference's inter-glyph gaps straight out of the PDF. One paragraph
per (face, character, size) holds the *same* character repeated N times with kerning off, so
every gap in the run is the same quantity, and `drawHorizontalGlyphs`
(`vcl/source/pdf/pdfwriter_impl.cxx`) states each one as the integer

    adj = trunc(W_prev - advance*1000/ppem + 0.5)

where `W_prev` is the /Widths entry the PDF declares. Recovering the advance as
`W - mean(adj)` looks as though averaging over 149 gaps buys three more digits.

**It buys none.** When the gaps are equal the adjustment is the same integer at every one of
them, so the mean is that integer and the recovered advance is `W` exactly -- the declared
width, which is `floor(hmtx * 1000 / upem)` and not the advance at all. The resolution is
+/-0.5 thousandths of an em per gap however many gaps there are, and half a thousandth of an
em is 0.083% of a Liberation Mono digit and 0.18% of a Liberation Serif `i`: **the
instrument's floor is the size of the defect it was built to measure, or larger.** Every
"the reference does not draw the design advance" figure in this project's history came from a
channel with that floor, including the `pdftotext -bbox` one, which reconstructs the same
integers.

Two things it did establish, and both matter:

- **The declared widths are truncated, not rounded** -- so the deficit is systematic and
  one-signed, which is what lets it accumulate along a run instead of cancelling.
- **26.2.4.2 emits an adjustment at nearly every position where 24.2.7.2 emits a handful.**
  That is a change in the PDF writer, not in the layout: the `Td` origins are identical.

`advance-width.py` is the instrument that answers the question, by differencing two
right-aligned lines and never touching a glyph position at all.

Measured 2026-09-06 against LibreOffice 24.2.7.2 (`/usr/bin/soffice`) and 26.2.4.2
(`/opt/libreoffice26.2/program/soffice`, bundled Latin duplicates and Latin Noto aside),
system fonts from `/usr/share/fonts`.

Usage:  XRAY_OFFICE_CLI=... python3 advance-staircase.py <workdir>
"""

import os
import re
import subprocess
import sys
import zlib
from ttf import Face

FACES = ('Liberation Mono', 'Liberation Serif', 'Liberation Sans', 'Carlito', 'DejaVu Sans')
CHARS = 'oi'
# Half-point steps through the sizes documents actually use, then a coarse tail.
HALF_POINTS = [12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 28, 30,
               32, 36, 40, 44, 48, 56, 64, 72, 96]
LINE_POINTS = 1400.0            # keep every run on one line of a 22-inch page
MAX_GLYPHS = 150

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-'
      'officedocument.wordprocessingml.document.main+xml"/></Types>')
RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
        'relationships/officeDocument" Target="word/document.xml"/></Relationships>')
W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'


def face_file(name: str) -> str:
    return subprocess.run(['fc-match', '-f', '%{file}', name],
                          capture_output=True, text=True, check=True).stdout.strip()


def plan() -> list[dict]:
    rows = []
    for name in FACES:
        face = Face(face_file(name))
        for ch in CHARS:
            design_em = face.advance(ch) / face.upem
            for half in HALF_POINTS:
                size = half / 2.0
                count = min(MAX_GLYPHS, int(LINE_POINTS / (design_em * size)))
                if count < 20:
                    continue
                rows.append({'face': name, 'file': face.path, 'ch': ch, 'half': half,
                             'size': size, 'count': count, 'units': face.advance(ch),
                             'upem': face.upem, 'design_em': design_em})
    return rows


def build(path: str, rows: list[dict]) -> None:
    body = ''
    for row in rows:
        rpr = (f'<w:rFonts w:ascii="{row["face"]}" w:hAnsi="{row["face"]}" w:cs="{row["face"]}"/>'
               f'<w:sz w:val="{row["half"]}"/><w:szCs w:val="{row["half"]}"/>'
               '<w:kern w:val="0"/><w:spacing w:val="0"/>')
        body += (f'<w:p><w:pPr><w:jc w:val="left"/><w:rPr>{rpr}</w:rPr></w:pPr>'
                 f'<w:r><w:rPr>{rpr}</w:rPr>'
                 f'<w:t xml:space="preserve">{row["ch"] * row["count"]}</w:t></w:r></w:p>')
    document = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>'
                f'{body}<w:sectPr><w:pgSz w:w="31680" w:h="20160"/>'
                '<w:pgMar w:top="284" w:right="284" w:bottom="284" w:left="284"/>'
                '</w:sectPr></w:body></w:document>')
    import zipfile
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml', document)


# --- reading the PDF -------------------------------------------------------------------

SHOW = re.compile(rb'/(F\d+)\s+([0-9.]+)\s+Tf\s*(\[.*?\]TJ|<[0-9A-Fa-f]*>Tj)', re.S)
WIDTHS = re.compile(rb'/Type\s*/Font.*?/FirstChar\s+(\d+).*?/Widths\s*\[([^\]]*)\]', re.S)


def streams(pdf: str) -> list[bytes]:
    data = open(pdf, 'rb').read()
    out = []
    for m in re.finditer(rb'stream\r?\n', data):
        try:
            out.append(zlib.decompress(data[m.end():data.find(b'endstream', m.end())]))
        except zlib.error:
            pass
    return out


def runs(pdf: str) -> list[tuple[str, float, list[int], list[int]]]:
    """Every text show in the file, as (resource, size, glyph codes, adjustments)."""
    found = []
    for stream in streams(pdf):
        for m in SHOW.finditer(stream):
            resource = m.group(1).decode('latin1')
            size = float(m.group(2))
            body = m.group(3)
            codes, adjustments = [], []
            for token in re.finditer(rb'<([0-9A-Fa-f]*)>|(-?\d+)', body):
                if token.group(1) is not None:
                    hexes = token.group(1)
                    for i in range(0, len(hexes), 2):
                        codes.append(int(hexes[i:i + 2], 16))
                        adjustments.append(0)
                else:
                    if adjustments:
                        adjustments[-1] = int(token.group(2))
            if codes:
                found.append((resource, size, codes, adjustments))
    return found


def main() -> int:
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    rows = plan()
    docx = os.path.join(out, 'staircase.docx')
    build(docx, rows)

    binaries = {'24.2.7.2': '/usr/bin/soffice', '26.2.4.2': '/opt/libreoffice26.2/program/soffice'}
    pdfs = {}
    for label, binary in binaries.items():
        directory = os.path.join(out, label)
        os.makedirs(directory, exist_ok=True)
        subprocess.run([binary, f'-env:UserInstallation=file://{directory}/profile',
                        '--headless', '--convert-to', 'pdf', '--outdir', directory, docx],
                       capture_output=True, check=True)
        pdf = os.path.join(directory, 'staircase.pdf')
        if not os.path.isfile(pdf):
            raise SystemExit(f'{binary} wrote no PDF -- nothing to compare')
        pdfs[label] = pdf

    cli = os.environ.get('XRAY_OFFICE_CLI')
    if cli:
        directory = os.path.join(out, 'ours')
        os.makedirs(directory, exist_ok=True)
        subprocess.run([cli, 'render', '--quiet', '--outdir', directory, docx], check=True)
        pdfs['ours'] = os.path.join(directory, 'staircase.pdf')

    print('# advance-staircase, measured ' + subprocess.run(
        ['date', '-u', '+%Y-%m-%d'], capture_output=True, text=True).stdout.strip())
    print('# fonts: system /usr/share/fonts; TDF tarball Latin duplicates and Latin Noto moved aside')
    print('binary\tface\tch\tsize_pt\tglyphs\tunits\tupem\tdesign_per_mille\tref_per_mille\tratio')

    for label, pdf in pdfs.items():
        found = [r for r in runs(pdf) if len(r[2]) >= 20]
        if len(found) != len(rows):
            print(f'# WARNING {label}: {len(found)} runs read against {len(rows)} paragraphs',
                  file=sys.stderr)
        for row, (_, size, codes, adjustments) in zip(rows, found):
            if len(codes) != row['count'] or len(set(codes)) != 1:
                print(f'# WARNING {label}: run for {row["face"]} {row["ch"]} {row["size"]}pt '
                      f'has {len(codes)} glyphs of {len(set(codes))} kinds', file=sys.stderr)
                continue
            # floor, not round: that is what the writer declares, and using round
            # here silently reported a defect of up to one whole unit.
            declared = int(row['units'] * 1000.0 // row['upem'])
            gaps = len(codes) - 1
            measured = declared - sum(adjustments[:gaps]) / gaps
            design = row['units'] * 1000.0 / row['upem']
            print(f'{label}\t{row["face"]}\t{row["ch"]}\t{size:g}\t{len(codes)}\t{row["units"]}\t'
                  f'{row["upem"]}\t{design:.4f}\t{measured:.4f}\t{measured / design:.6f}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
