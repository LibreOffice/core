#!/usr/bin/env python3
"""Emit a flat-ODS probe: one column of cells, each at a different font size,
   the sheet printed at a stated scale."""
import sys

SIZES = [6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 24, 28, 36, 48]

def build(scale_pct):
    styles = []
    for i, s in enumerate(SIZES):
        styles.append(
            f'<style:style style:name="ce{i}" style:family="table-cell" '
            f'style:parent-style-name="Default">'
            f'<style:text-properties fo:font-size="{s}pt" style:font-name="Liberation Sans"/>'
            f'</style:style>')
    rows = []
    for i, s in enumerate(SIZES):
        rows.append(
            f'<table:table-row><table:table-cell table:style-name="ce{i}" '
            f'office:value-type="string"><text:p>Size{i:02d}Probe</text:p>'
            f'</table:table-cell></table:table-row>')
    return f'''<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.spreadsheet">
<office:automatic-styles>
{"".join(styles)}
<style:page-layout style:name="pm1">
<style:page-layout-properties fo:page-width="21.001cm" fo:page-height="29.7cm"
 fo:margin-top="2cm" fo:margin-bottom="2cm" fo:margin-left="2cm" fo:margin-right="2cm"
 style:print-orientation="portrait" style:scale-to="{scale_pct}%"/>
</style:page-layout>
<style:style style:name="ta1" style:family="table" style:master-page-name="Default">
<style:table-properties table:display="true"/>
</style:style>
</office:automatic-styles>
<office:master-styles>
<style:master-page style:name="Default" style:page-layout-name="pm1"/>
</office:master-styles>
<office:body><office:spreadsheet>
<table:table table:name="Probe" table:style-name="ta1">
<table:table-column/>
{"".join(rows)}
</table:table>
</office:spreadsheet></office:body>
</office:document>
'''

if __name__ == '__main__':
    out = sys.argv[1]
    scale = int(sys.argv[2])
    with open(out, 'w', encoding='utf-8') as f:
        f.write(build(scale))
