#!/usr/bin/env python3
"""Show what a word's position inside a LibreOffice PDF line is actually made of.

LibreOffice's PDF writer declares every glyph's advance as an **integer thousandth of an
em**, and `registerGlyph` truncates rather than rounds, so each declared width is up to one
thousandth *short* of the advance the layout used. `drawHorizontalGlyphs`
(`vcl/source/pdf/pdfwriter_impl.cxx`:5814) then corrects a gap only when

    trunc(declared_width - actual_gap*1000/ppem + 0.5)

is non-zero, which a systematic sub-unit deficit never makes it. So the pen a reader
reconstructs inside one text object falls behind the pen the layout intended by about half a
thousandth of an em per glyph, and it resets at every `Td`/`Tm`.

That is the whole of the "~0.1% advance divergence": half a thousandth of an em is 0.05% of
a 1000-unit glyph, it accumulates along a run, and every fidelity comparison that reads the
reference through `pdftotext` measures it. `advance-width.py` measures the same two stacks
through the `Td` pen instead and finds them equal to a few parts per million.

Usage:  XRAY_OFFICE_CLI=... python3 pdf-width-quantisation.py <workdir> [document...]
"""

import math
import os
import re
import subprocess
import sys
import zlib
from ttf import Face

CORPUS = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                      '..', '..', 'tests', 'corpus', 'features')
DEFAULT = ('paginated.docx', 'list-label-overrun.docx', 'tabbed.docx')
TARGETS = {'24.2.7.2': '/usr/bin/soffice', '26.2.4.2': '/opt/libreoffice26.2/program/soffice'}


STYLES = ('BoldItalic', 'BoldOblique', 'Bold', 'Italic', 'Oblique', 'Regular', 'Roman')


def face_file(base_font: str) -> str:
    """The file behind a PDF `/BaseFont` name.

    Not `fc-match "<BaseFont>"`: fontconfig's `FcNameParse` reads a `-` in a pattern as the
    start of a size, so `fc-match "Carlito-Bold"` answers Carlito *Regular* and every width
    then compares against the wrong face. The name is split into a family and a style and the
    style is passed as one, which is what fontconfig is actually being asked.
    """
    name, style = base_font, 'Regular'
    for candidate in STYLES:
        if name.endswith('-' + candidate) or (name.endswith(candidate) and name != candidate):
            style = candidate
            name = name[:-len(candidate)].rstrip('-')
            break
    family = re.sub(r'(?<=[a-z])(?=[A-Z])', ' ', name)
    return subprocess.run(['fc-match', '-f', '%{file}', f'{family}:style={style}'],
                          capture_output=True, text=True, check=True).stdout.strip()


def convert(binary: str, document: str, directory: str) -> str:
    os.makedirs(directory, exist_ok=True)
    subprocess.run([binary, f'-env:UserInstallation=file://{directory}/profile', '--headless',
                    '--convert-to', 'pdf', '--outdir', directory, document],
                   capture_output=True, check=True)
    pdf = os.path.join(directory, os.path.splitext(os.path.basename(document))[0] + '.pdf')
    if not os.path.isfile(pdf):
        raise SystemExit(f'{binary} wrote no PDF for {document} -- nothing to compare')
    return pdf


def words(pdf: str) -> list[tuple[float, float, float, str]]:
    text = subprocess.run(['pdftotext', '-bbox', '-f', '1', '-l', '1', pdf, '-'],
                          capture_output=True, text=True, check=True).stdout
    return [(float(m.group(1)), float(m.group(2)), float(m.group(3)), m.group(4))
            for m in re.finditer(
                r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="[\d.]+">(.*?)</word>',
                text)]


def declared_widths(pdf: str) -> list[tuple[str, str, float]]:
    """Every embedded font's declared widths, as (face, character, thousandths of an em).

    The `ToUnicode` map is read per font object rather than over the whole file: a document
    setting two faces gives each subset its own code space, and one map built across both
    silently reports one face's widths against the other's characters.
    """
    data = open(pdf, 'rb').read()

    def stream_of(number: int) -> bytes:
        m = re.search(rb'(?<![0-9])' + str(number).encode() + rb'\s+0\s+obj\b', data)
        if m is None:
            return b''
        start = data.find(b'stream', m.end())
        if start < 0:
            return b''
        start = data.find(b'\n', start) + 1
        try:
            return zlib.decompress(data[start:data.find(b'endstream', start)])
        except zlib.error:
            return b''

    found = []
    for m in re.finditer(
            rb'/Type\s*/Font/Subtype/TrueType/BaseFont/(?:[A-Z]{6}\+)?([A-Za-z0-9\-]+)\s*'
            rb'/FirstChar\s+(\d+)\s*/LastChar\s+\d+\s*/Widths\s*\[([^\]]*)\](.{0,400}?)>>',
            data, re.S):
        face = m.group(1).decode()
        first = int(m.group(2))
        to_unicode = re.search(rb'/ToUnicode\s+(\d+)\s+0\s+R', m.group(4))
        if to_unicode is None:
            continue
        mapping = {int(e.group(1), 16): chr(int(e.group(2), 16))
                   for e in re.finditer(rb'<([0-9A-Fa-f]{2})>\s*<([0-9A-Fa-f]{4})>',
                                        stream_of(int(to_unicode.group(1))))}
        for i, value in enumerate(m.group(3).split()):
            character = mapping.get(first + i)
            if character:
                found.append((face, character, float(value)))
    return found


def main() -> int:
    out = sys.argv[1]
    documents = sys.argv[2:] or [os.path.abspath(os.path.join(CORPUS, name))
                                 for name in DEFAULT]
    cli = os.environ.get('XRAY_OFFICE_CLI')
    os.makedirs(out, exist_ok=True)

    print('# pdf-width-quantisation. Measured ' + subprocess.run(
        ['date', '-u', '+%Y-%m-%d'], capture_output=True, text=True).stdout.strip())
    for label, binary in TARGETS.items():
        print(f'# {label}: {binary} -> ' + subprocess.run(
            [binary, '--version'], capture_output=True, text=True).stdout.strip())
    if cli:
        print(f'# ours: {cli}')
    print('# fonts: system /usr/share/fonts, the tarball\'s Latin duplicates and Latin Noto aside')

    for document in documents:
        name = os.path.basename(document)
        pdfs = {label: convert(binary, document, os.path.join(out, label, name))
                for label, binary in TARGETS.items()}
        if cli:
            directory = os.path.join(out, 'ours', name)
            os.makedirs(directory, exist_ok=True)
            subprocess.run([cli, 'render', '--quiet', '--outdir', directory, document], check=True)
            pdfs['ours'] = os.path.join(
                directory, os.path.splitext(name)[0] + '.pdf')

        print(f'\n## {name}')
        print('# the reference\'s declared /Widths against the face\'s own hmtx')
        print('binary\tface\tglyphs\tmean_declared_deficit_per_mille\tall_truncated')
        for label in TARGETS:
            for face_name in {f for f, _, _ in declared_widths(pdfs[label])}:
                face = Face(face_file(face_name))
                rows = [(c, w) for f, c, w in declared_widths(pdfs[label]) if f == face_name]
                deficits = []
                truncated = True
                for character, declared in rows:
                    try:
                        exact = face.advance(character) * 1000.0 / face.upem
                    except SystemExit:
                        continue
                    deficits.append(exact - declared)
                    truncated &= declared == math.floor(exact)
                if deficits:
                    print(f'{label}\t{face_name}\t{len(deficits)}\t'
                          f'{sum(deficits) / len(deficits):.4f}\t{truncated}')

        print('# where each word of the first line starts, in points')
        print('word\t' + '\t'.join(pdfs))
        rows = {label: words(pdf) for label, pdf in pdfs.items()}
        first = list(rows)[0]
        baseline = rows[first][0][1]
        line = [i for i, w in enumerate(rows[first]) if abs(w[1] - baseline) < 1.0]
        for i in line:
            texts = {rows[label][i][3] for label in rows}
            if len(texts) != 1:
                print(f'# WARNING: word {i} reads {texts} -- the lines differ, stopping')
                break
            print(rows[first][i][3] + '\t'
                  + '\t'.join(f'{rows[label][i][0]:.3f}' for label in pdfs))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
