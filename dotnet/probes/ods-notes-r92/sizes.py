#!/usr/bin/env python3
"""What 26.2.4.2 computes for a *standard* row, over twelve font sizes.

    python3 sizes.py /tmp/out

One column of Calibri cells, one size each, every row stating a height and the optimal flag so
that Calc recomputes it. Read back through the reference's own `--convert-to fods`.

Measured 2026-09-10 against /opt/libreoffice26.2/program/soffice, LibreOffice 26.2.4.2:

    6 pt   256.3      10.5 pt  264.2      14 pt  346.9      20 pt  489.3
    8 pt   256.3      11   pt  276.0      16 pt  394.0      24 pt  582.8
    9 pt   256.3      12   pt  300.0      18 pt  441.1
    10 pt  256.3

Every one of the twelve is `trunc(20 x size x 1.18) + 40 - 23` floored at `ScGlobal::nStdRowHeight`
of 256 — `lcl_GetAttribHeight` (`sc/source/core/data/column2.cxx:866-892`) with the pool's 20-twip
top and bottom margins. So the arithmetic branch is exact and the 298 twips a real 11 pt row comes
back with is the *other* branch, not a different arithmetic. The four sizes at the floor are the
control that fixes the margin pair at 40: with no vertical margin at all, 11 pt would land on the
floor too.
"""
import re
import subprocess
import sys
import tempfile
from pathlib import Path

SOFFICE = '/opt/libreoffice26.2/program/soffice'
SIZES = [6, 8, 9, 10, 10.5, 11, 12, 14, 16, 18, 20, 24]


def build(path: Path):
    cells, rows = [], []
    for index, size in enumerate(SIZES):
        cells.append(
            f'<style:style style:name="ceP{index}" style:family="table-cell" '
            f'style:parent-style-name="Default"><style:text-properties '
            f'style:font-name="Calibri" fo:font-size="{size}pt" '
            f'style:font-size-asian="{size}pt" style:font-size-complex="{size}pt"/></style:style>')
        rows.append(
            f'<table:table-row table:style-name="roP"><table:table-cell '
            f'table:style-name="ceP{index}" office:value-type="string">'
            f'<text:p>Size {size}</text:p></table:table-cell></table:table-row>')

    path.write_text(
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        '<office:document '
        'xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" '
        'xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" '
        'xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" '
        'xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" '
        'xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" '
        'office:version="1.3" '
        'office:mimetype="application/vnd.oasis.opendocument.spreadsheet">'
        '<office:automatic-styles>'
        '<style:style style:name="roP" style:family="table-row">'
        '<style:table-row-properties style:row-height="0.2083in" fo:break-before="auto" '
        'style:use-optimal-row-height="true"/></style:style>'
        + ''.join(cells) +
        '</office:automatic-styles><office:body><office:spreadsheet>'
        '<table:table table:name="Probe"><table:table-column/>'
        + ''.join(rows) +
        '</table:table></office:spreadsheet></office:body></office:document>',
        encoding='utf8')


def main(outdir: str):
    out = Path(outdir)
    out.mkdir(parents=True, exist_ok=True)
    source = out / 'sizes.fods'
    build(source)

    subprocess.run(
        ['timeout', '-k', '30', '900', SOFFICE, '--headless', '--norestore',
         '-env:UserInstallation=file://' + str(out / 'profile'),
         '--convert-to', 'fods', '--outdir', str(out / 'back'), str(source)],
        check=False, capture_output=True)

    text = (out / 'back' / 'sizes.fods').read_text(encoding='utf8')
    styles = {}
    for match in re.finditer(
            r'<style:style style:name="(ro\d+)" style:family="table-row">\s*'
            r'<style:table-row-properties ([^/]*?)/>', text):
        stated = re.search(r'style:row-height="([\d.]+)in"', match.group(2))
        styles[match.group(1)] = round(float(stated.group(1)) * 1440, 1) if stated else None

    for match in re.finditer(
            r'<table:table-row table:style-name="(ro\d+)"[^>]*>(.*?)</table:table-row>',
            text, re.S):
        label = re.findall(r'<text:p>(.*?)</text:p>', match.group(2))
        if label:
            print(f'{label[0]}\t{styles.get(match.group(1))} twips')


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else tempfile.mkdtemp())
