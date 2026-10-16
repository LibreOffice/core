#!/usr/bin/env python3
"""A flat-ODS probe for where a turned cell's glyphs land, not how tall its row is.

One sheet per (angle, wrap). Every row states its height outright — the point here
is the drawing, so both renderers must be given the same grid — and the master page
turns the header and footer off so the two page bodies start at the same y.
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
   <style:table-row-properties style:row-height="2.5in"
    style:use-optimal-row-height="false"/></style:style>
  <style:style style:name="co1" style:family="table-column">
   <style:table-column-properties style:column-width="1.5in"/></style:style>
  <style:page-layout style:name="pm1">
   <style:page-layout-properties fo:page-width="8.5in" fo:page-height="11in"
    fo:margin-top="1in" fo:margin-bottom="1in" fo:margin-left="1in" fo:margin-right="1in"/>
   <style:header-style/><style:footer-style/>
  </style:page-layout>
'''

MASTER = ''' <office:master-styles>
  <style:master-page style:name="Default" style:page-layout-name="pm1">
   <style:header style:display="false"/>
   <style:footer style:display="false"/>
  </style:master-page>
 </office:master-styles>
'''

TAIL = ''' </office:spreadsheet></office:body>
</office:document>
'''

# Short enough to stay on one line at 2.5in, long enough to need two at that width.
TEXTS = ["Ab", "Abcdefghijklmnopqrstuvwxyz Abcdefghijklmnopqrstuvwxyz"]
ANGLES = [90, 270, 45]
WRAPS = [False, True]


def main(path: str) -> None:
    styles, tables = [], []
    for angle in ANGLES:
        for wrap in WRAPS:
            name = f"a{angle}{'w' if wrap else 'n'}"
            wrapattr = ' fo:wrap-option="wrap"' if wrap else ''
            styles.append(
                f'  <style:style style:name="ce_{name}" style:family="table-cell">'
                f'<style:table-cell-properties{wrapattr} style:rotation-angle="{angle}"/>'
                f'<style:text-properties style:font-name="Calibri" fo:font-size="11pt"/>'
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
        f.write("\n </office:automatic-styles>\n")
        f.write(MASTER)
        f.write(" <office:body><office:spreadsheet>\n")
        f.write("".join(tables))
        f.write(TAIL)


if __name__ == "__main__":
    main(sys.argv[1])
