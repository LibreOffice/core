#!/usr/bin/env python3
"""Recover a stack's own laid-out string width by differencing two right-aligned lines.

A right-aligned line puts its *right* edge on the text area's right margin, so the pen
position the PDF states is `margin - width(line)`. Two lines built from the same unit at the
same size, one holding the unit N0 times and one N1 times, therefore differ in pen position
by exactly (N1 - N0) unit widths -- every fixed term (the margin, the page offset, the
trailing side bearing, the text origin, the first glyph's left bearing) cancels. Dividing by
(N1 - N0) divides the pen's own rounding by it as well, so on the 22-inch page this uses the
width is known to about a hundredth of a percent. Twenty-two inches, not more, and measured:
given a 100-inch `w:pgSz` LibreOffice 24.2.7.2 writes a 7200-point MediaBox and then lays the
text out in a 9-inch column anyway, wrapping every line and saying nothing. 26.2.4.2 honours it.

That is the quantity that decides line breaking. It is *not* what a PDF's TJ array states:
`drawHorizontalGlyphs` writes integer thousandths of an em, which cannot resolve a 0.1%
defect at all -- 1/1000 em is 0.36% of a `Liberation Serif` `i`. An instrument that reads
advances out of PDF glyph positioning, whether from the TJ integers or from poppler's
reconstruction of them in `pdftotext -bbox`, measures that quantisation and not the layout.

Measured 2026-09-06 against LibreOffice 24.2.7.2 (`/usr/bin/soffice`) and 26.2.4.2
(`/opt/libreoffice26.2/program/soffice`, with its bundled Latin duplicates and its Latin
`NotoSans-*`/`NotoSerif-*` moved aside so it resolves the system faces), system fonts from
`/usr/share/fonts`, `fc-match "DejaVu Sans"` answering `DejaVuSans.ttf`.

Usage:  XRAY_OFFICE_CLI=... python3 advance-width.py <workdir>
"""

import os
import re
import subprocess
import sys
import zipfile
import zlib
from ttf import Face

FACES = ('Liberation Mono', 'Liberation Serif', 'Liberation Sans', 'Carlito', 'DejaVu Sans')

# A single repeated glyph isolates the base advance; the phrases put shaping, kerning and
# the space glyph into the same instrument, so the two questions are answered side by side.
UNITS = {
    'o': 'o',
    'i': 'i',
    'space_o': ' o',
    'hamburgefonstiv': 'Hamburgefonstiv',
    # Both phrases lead with their space rather than trailing it: a trailing space at a
    # line end is trimmed by one stack and drawn by the other, which changes the glyph
    # count without changing the width and makes the two sides look incomparable.
    'kerned': ' AVATAR Wave To. Yes,',
    'prose': ' the quick brown fox jumps over the lazy dog',
}

HALF_POINTS = [12, 16, 20, 21, 24, 28, 32, 40, 48, 64, 96]
SHORT = 2
LINE_POINTS = 1540.0            # the 22-inch page below, less its margins
MAX_UNITS = 240

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
ESCAPE = {'&': '&amp;', '<': '&lt;', '>': '&gt;'}


def face_file(name: str) -> str:
    return subprocess.run(['fc-match', '-f', '%{file}', name],
                          capture_output=True, text=True, check=True).stdout.strip()


def plan() -> list[dict]:
    rows = []
    for name in FACES:
        face = Face(face_file(name))
        for key, unit in UNITS.items():
            units = sum(face.advance(c) for c in unit)
            design_em = units / face.upem
            for half in HALF_POINTS:
                size = half / 2.0
                # Both lines must stay on one line, and neither may exceed a 256-glyph show.
                long = min(MAX_UNITS, int(LINE_POINTS / (design_em * size)))
                if (long - SHORT) * design_em * size < 200:
                    continue
                rows.append({'face': name, 'unit': key, 'text': unit, 'half': half, 'size': size,
                             'long': long, 'units': units, 'upem': face.upem,
                             'glyphs': len(unit)})
    return rows


def build(path: str, rows: list[dict]) -> None:
    body = ''
    for row in rows:
        rpr = (f'<w:rFonts w:ascii="{row["face"]}" w:hAnsi="{row["face"]}" w:cs="{row["face"]}"/>'
               f'<w:sz w:val="{row["half"]}"/><w:szCs w:val="{row["half"]}"/>'
               '<w:kern w:val="0"/><w:spacing w:val="0"/>')
        for count in (SHORT, row['long']):
            text = ''.join(ESCAPE.get(c, c) for c in row['text'] * count)
            body += (f'<w:p><w:pPr><w:jc w:val="right"/><w:rPr>{rpr}</w:rPr></w:pPr>'
                     f'<w:r><w:rPr>{rpr}</w:rPr>'
                     f'<w:t xml:space="preserve">{text}</w:t></w:r></w:p>')
    document = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>'
                f'{body}<w:sectPr><w:pgSz w:w="31680" w:h="20160"/>'
                '<w:pgMar w:top="284" w:right="284" w:bottom="284" w:left="284"/>'
                '</w:sectPr></w:body></w:document>')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml', document)


PLACE = re.compile(
    rb"(?:([-0-9.]+)\s+([-0-9.]+)\s+(?:Td|TD)|"
    rb"([-0-9.]+)\s+([-0-9.]+)\s+([-0-9.]+)\s+([-0-9.]+)\s+([-0-9.]+)\s+([-0-9.]+)\s+Tm)"
    rb"[^A-Za-z]*(?:/F\d+\s+[0-9.]+\s+Tf\s*)?"
    rb"(\[.*?\]TJ|<[0-9A-Fa-f]*>Tj|\((?:\\.|[^\\)])*\)Tj)", re.S)


def streams(pdf: str) -> list[bytes]:
    data = open(pdf, "rb").read()
    out = []
    for m in re.finditer(rb"stream\r?\n", data):
        try:
            out.append(zlib.decompress(data[m.end():data.find(b"endstream", m.end())]))
        except zlib.error:
            pass
    return out or [data]


def _glyphs(token: bytes) -> int:
    if token.endswith(b"]TJ") or token.endswith(b">Tj"):
        return sum(len(h) // 2 for h in re.findall(rb"<([0-9A-Fa-f]*)>", token))
    return len(re.sub(rb"\\.", b".", token[1:-3]))


def lines(pdf: str) -> list[tuple[float, int]]:
    """Every drawn line, as (leftmost pen x, glyph count), in page then top-to-bottom order.

    A line is not one text object: both writers split a long one into several shows, and
    LibreOffice gives each its own `Tm`. Grouping the shows of one page by their baseline
    recovers the line whichever way it was split. The grouping has to stay inside a page --
    a baseline recurs on every page, and merging two pages' lines is silent and total.
    """
    found: list[tuple[float, int]] = []
    for stream in streams(pdf):
        rows: dict[float, list[float]] = {}
        for m in PLACE.finditer(stream):
            if m.group(1) is not None:
                x, y = float(m.group(1)), float(m.group(2))
            else:
                x, y = float(m.group(7)), float(m.group(8))
            count = _glyphs(m.group(9))
            if not count:
                continue
            row = rows.setdefault(round(y, 1), [x, 0])
            row[0] = min(row[0], x)
            row[1] += count
        found.extend((v[0], int(v[1])) for _, v in sorted(rows.items(), reverse=True))
    return found


def main() -> int:
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    rows = plan()
    docx = os.path.join(out, 'width.docx')
    build(docx, rows)

    targets = {'24.2.7.2': '/usr/bin/soffice', '26.2.4.2': '/opt/libreoffice26.2/program/soffice'}
    pdfs = {}
    for label, binary in targets.items():
        directory = os.path.join(out, label)
        os.makedirs(directory, exist_ok=True)
        subprocess.run([binary, f'-env:UserInstallation=file://{directory}/profile', '--headless',
                        '--convert-to', 'pdf', '--outdir', directory, docx],
                       capture_output=True, check=True)
        pdf = os.path.join(directory, 'width.pdf')
        if not os.path.isfile(pdf):
            raise SystemExit(f'{binary} wrote no PDF -- nothing to compare')
        pdfs[label] = pdf

    cli = os.environ.get('XRAY_OFFICE_CLI')
    if cli:
        directory = os.path.join(out, 'ours')
        os.makedirs(directory, exist_ok=True)
        subprocess.run([cli, 'render', '--quiet', '--outdir', directory, docx], check=True)
        pdfs['ours'] = os.path.join(directory, 'width.pdf')

    version = lambda b: subprocess.run([b, '--version'], capture_output=True, text=True).stdout.strip()
    print('# advance-width: the laid-out width of a repeated unit, by differencing two')
    print('# right-aligned lines. Measured ' + subprocess.run(
        ['date', '-u', '+%Y-%m-%d'], capture_output=True, text=True).stdout.strip())
    for label, binary in targets.items():
        print(f'# {label}: {binary} -> {version(binary)}')
    print('# fonts: system /usr/share/fonts; the TDF tarball\'s bundled Latin duplicates and its')
    print('# Latin NotoSans-*/NotoSerif-* are moved aside, so 26.2.4.2 resolves the system faces.')
    print('# fc-match "DejaVu Sans" -> ' + subprocess.run(
        ['fc-match', 'DejaVu Sans'], capture_output=True, text=True).stdout.strip())
    if cli:
        print(f'# ours: {cli}')
    print('binary\tface\tunit\tsize_pt\treps\tglyphs_per_unit\tdesign_pt\t'
          'measured_pt\tratio_to_design')
    for label, pdf in pdfs.items():
        found = lines(pdf)
        if len(found) != 2 * len(rows):
            print(f'# WARNING {label}: {len(found)} text objects read against '
                  f'{2 * len(rows)} lines', file=sys.stderr)
        for i, row in enumerate(rows):
            (x0, n0), (x1, n1) = found[2 * i], found[2 * i + 1]
            # Glyphs per unit rather than characters per unit: a face may form ligatures,
            # and the two stacks need not form the same ones. What must hold is that both
            # lines are whole repeats of the same shaping, or the difference is not N units.
            if n0 % SHORT or n1 % row['long'] or n0 // SHORT != n1 // row['long']:
                print(f'# WARNING {label}: {row["face"]} {row["unit"]} {row["size"]}pt '
                      f'shows {n0}/{n1} glyphs, not whole repeats of one shaping',
                      file=sys.stderr)
                continue
            measured = (x0 - x1) / (row['long'] - SHORT)
            design = row['units'] * row['size'] / row['upem']
            print(f'{label}\t{row["face"]}\t{row["unit"]}\t{row["size"]:g}\t'
                  f'{row["long"] - SHORT}\t{n0 // SHORT}\t{design:.6f}\t{measured:.6f}\t'
                  f'{measured / design:.6f}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
