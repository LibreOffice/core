#!/usr/bin/env python3
"""Write a flat-ODS probe of rotated cells, one sheet per (size, angle, wrap).

Each sheet holds one column and several rows, each row a string of a different
length. Every row asks for an optimal height, so LibreOffice's own flat-ODF
round trip states the height it computed for a rotated cell — which is the
number this probe exists to read.
"""
import sys

HEAD = '''<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.spreadsheet">
 <office:font-face-decls>
  <style:font-face style:name="Calibri" svg:font-family="Calibri"/>
 </office:font-face-decls>
 <office:automatic-styles>
  <style:style style:name="ro1" style:family="table-row">
   <style:table-row-properties style:row-height="0.1783in"
    style:use-optimal-row-height="true"/></style:style>
'''

TAIL = ''' </office:spreadsheet></office:body>
</office:document>
'''

# Strings of increasing length. The rotated height is the text's *width* put
# through the angle, so the length is the independent variable here.
TEXTS = [
    "A",
    "Abcde",
    "Abcdefghij",
    "Abcdefghijklmnopqrst",
    "Abcdefghijklmnopqrstuvwxyzabcdefghijklmn",
    "Abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrst",
]

SIZES = [10, 11, 12]
ANGLES = [90, 45, 30, 60, 270, 315]
WRAPS = [False, True]

# One narrow column, so a wrapping cell would break if the wrap width were the
# column's — which is one of the questions this probe answers.
COLUMN_WIDTH_IN = 0.6


def main(path: str) -> None:
    styles, tables = [], []
    styles.append(
        '  <style:style style:name="co1" style:family="table-column">'
        f'<style:table-column-properties style:column-width="{COLUMN_WIDTH_IN}in"/></style:style>')

    for size in SIZES:
        for angle in ANGLES:
            for wrap in WRAPS:
                name = f"p{size}a{angle}{'w' if wrap else 'n'}"
                wrapattr = ' fo:wrap-option="wrap"' if wrap else ''
                styles.append(
                    f'  <style:style style:name="ce_{name}" style:family="table-cell">'
                    f'<style:table-cell-properties{wrapattr} style:rotation-angle="{angle}"/>'
                    f'<style:text-properties style:font-name="Calibri" fo:font-size="{size}pt"/>'
                    '</style:style>')
                rows = "".join(
                    '   <table:table-row table:style-name="ro1">'
                    f'<table:table-cell table:style-name="ce_{name}" office:value-type="string">'
                    f'<text:p>{t}</text:p></table:table-cell></table:table-row>\n'
                    for t in TEXTS)
                tables.append(
                    f'  <table:table table:name="{name}">\n'
                    '   <table:table-column table:style-name="co1"/>\n'
                    f'{rows}'
                    '  </table:table>\n')

    with open(path, "w", encoding="utf-8") as f:
        f.write(HEAD)
        f.write("\n".join(styles))
        f.write("\n </office:automatic-styles>\n <office:body><office:spreadsheet>\n")
        f.write("".join(tables))
        f.write(TAIL)


if __name__ == "__main__":
    main(sys.argv[1])
