"""Which of svg:width and table:end-cell-address decides a sheet shape's box.

One flat ODS per case: a right-aligned text box anchored in A2, so the drawn x of its
one line is the box's right edge less the inset less the string's width. Columns are
1 inch each, so an end cell of E2 states 4 inches where svg:width states 1.
"""
import os, subprocess, sys

HEAD = '''<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
 xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.spreadsheet">
 <office:automatic-styles>
  <style:style style:name="co1" style:family="table-column">
   <style:table-column-properties style:column-width="1in"/></style:style>
  <style:style style:name="ro1" style:family="table-row">
   <style:table-row-properties style:row-height="0.5in"/></style:style>
  <style:style style:name="gr1" style:family="graphic" style:parent-style-name="Default">
   <style:graphic-properties draw:stroke="none" draw:fill="none"
    draw:textarea-horizontal-align="right" draw:textarea-vertical-align="top"
    fo:padding-top="0in" fo:padding-bottom="0in" fo:padding-left="0in" fo:padding-right="0in"
    fo:wrap-option="wrap"/></style:style>
  <style:style style:name="T1" style:family="text">
   <style:text-properties fo:font-family="Liberation Serif" fo:font-size="10pt"/></style:style>
  <style:page-layout style:name="pm1">
   <style:page-layout-properties fo:page-width="21cm" fo:page-height="29.7cm"
    fo:margin-left="1cm" fo:margin-right="1cm" fo:margin-top="1cm" fo:margin-bottom="1cm"/>
  </style:page-layout>
 </office:automatic-styles>
 <office:master-styles>
  <style:master-page style:name="Default" style:page-layout-name="pm1"/>
 </office:master-styles>
 <office:body><office:spreadsheet>
  <table:table table:name="Probe">
   <table:table-column table:style-name="co1" table:number-columns-repeated="8"/>
   <table:table-row table:style-name="ro1">
    <table:table-cell office:value-type="string"><text:p>top</text:p></table:table-cell>
   </table:table-row>
   <table:table-row table:style-name="ro1">
    <table:table-cell>
'''
TAIL = '''    </table:table-cell>
   </table:table-row>
  </table:table>
 </office:spreadsheet></office:body>
</office:document>
'''

def shape(extra):
    return ('     <draw:custom-shape draw:name="Box" draw:style-name="gr1" '
            'svg:width="1in" svg:height="0.4in" svg:x="0in" svg:y="0in"' + extra + '>\n'
            '      <text:p><text:span text:style-name="T1">EDGE</text:span></text:p>\n'
            '     </draw:custom-shape>\n')

CASES = {
    'stated_only': shape(''),
    'end_e2': shape(' table:end-cell-address="Probe.E2" table:end-x="0in" table:end-y="0.4in"'),
    'end_a2': shape(' table:end-cell-address="Probe.A2" table:end-x="1in" table:end-y="0.4in"'),
}

os.makedirs('endcell/var', exist_ok=True)
for name, body in CASES.items():
    open(f'endcell/var/{name}.fods', 'w', encoding='utf-8').write(HEAD + body + TAIL)
print(' '.join(CASES))
