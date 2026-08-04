#!/usr/bin/env python3
"""The device fixture: one sheet per (font size, column width) pair, each holding the same
four strings, with the column widths sitting either side of a break point so that the two
device quantisations each decide a row on their own."""
import sys
from xml.sax.saxutils import escape

OUT = sys.argv[1]

STRINGS = [
    'Statethereasonswhichcouldprobablyleadtothe',
    'n' * 42,
    'M' * 42,
    'State the reasons which could probably lead to the decision to use another '
    'take-off or landing direction than the one into the wind.',
]

# (sheet name, font size in points, column width in twips)
SHEETS = [
    ('p10w3800', 10, 3800),
    ('p10w3850', 10, 3850),
    ('p11w4400', 11, 4400),
    ('p11w4450', 11, 4450),
    ('p12w4700', 12, 4700),
    ('p12w4750', 12, 4750),
]

p = ['''<?xml version="1.0" encoding="UTF-8"?>
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
''']
for name, size, width in SHEETS:
    p.append('  <style:style style:name="co_%s" style:family="table-column">'
             '<style:table-column-properties style:column-width="%.4fin"/></style:style>\n'
             % (name, width / 1440.0))
    p.append('  <style:style style:name="ce_%s" style:family="table-cell">'
             '<style:table-cell-properties fo:wrap-option="wrap"/>'
             '<style:text-properties style:font-name="Calibri" fo:font-size="%dpt"/>'
             '</style:style>\n' % (name, size))
p.append(' </office:automatic-styles>\n <office:body><office:spreadsheet>\n')
for name, size, width in SHEETS:
    p.append('  <table:table table:name="%s">\n   <table:table-column '
             'table:style-name="co_%s"/>\n' % (name, name))
    for s in STRINGS:
        p.append('   <table:table-row table:style-name="ro1">'
                 '<table:table-cell table:style-name="ce_%s" office:value-type="string">'
                 '<text:p>%s</text:p></table:table-cell></table:table-row>\n'
                 % (name, escape(s)))
    p.append('  </table:table>\n')
p.append(' </office:spreadsheet></office:body>\n</office:document>\n')
open(OUT, 'w').write(''.join(p))
print('wrote', OUT)
