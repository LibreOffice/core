#!/usr/bin/env python3
"""A flat-ODS probe over a list of families: letters in A, digits in B, row order identifies."""
import html, sys

LETTERS = "Hamburgefonstiv"
DIGITS = "0123456789"

families = [l.rstrip('\n') for l in open(sys.argv[1], encoding='utf8') if l.strip()]
styles = "".join(
    f'<style:style style:name="c{i}" style:family="table-cell">'
    f'<style:text-properties style:font-name="f{i}" fo:font-size="10pt"/></style:style>'
    for i in range(len(families)))
fonts = "".join(
    f'<style:font-face style:name="f{i}" svg:font-family="&quot;{html.escape(f)}&quot;"/>'
    for i, f in enumerate(families))
rows = "".join(
    f'<table:table-row>'
    f'<table:table-cell table:style-name="ix" office:value-type="string">'
    f'<text:p>R{i:04d}</text:p></table:table-cell>'
    f'<table:table-cell table:style-name="c{i}" office:value-type="string">'
    f'<text:p>{LETTERS}</text:p></table:table-cell>'
    f'<table:table-cell table:style-name="c{i}" office:value-type="string">'
    f'<text:p>{DIGITS}</text:p></table:table-cell></table:table-row>'
    for i in range(len(families)))

doc = f'''<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.spreadsheet">
<office:font-face-decls>{fonts}<style:font-face style:name="fix" svg:font-family="&quot;Liberation Sans&quot;"/></office:font-face-decls>
<office:automatic-styles>
<style:style style:name="co1" style:family="table-column">
<style:table-column-properties style:column-width="1.4in"/></style:style>
{styles}
<style:style style:name="ix" style:family="table-cell">
<style:text-properties style:font-name="fix" fo:font-size="10pt"/></style:style>
</office:automatic-styles>
<office:body><office:spreadsheet><table:table table:name="probe">
<table:table-column table:style-name="co1" table:number-columns-repeated="3"/>
{rows}
</table:table></office:spreadsheet></office:body></office:document>'''

open(sys.argv[2], 'w', encoding='utf8').write(doc)
print(f"{len(families)} families -> {sys.argv[2]}")
