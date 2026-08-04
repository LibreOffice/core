#!/usr/bin/env python3
"""A flat-ODS holding one unwrapped string per row in a very wide column, so the PDF
shows how wide LibreOffice actually draws it."""
import sys
from xml.sax.saxutils import escape

out, size, font, strings_file = sys.argv[1:5]
lines = [l.rstrip('\n') for l in open(strings_file) if l.strip()]

p = ['''<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.spreadsheet">
 <office:font-face-decls>
  <style:font-face style:name="%s" svg:font-family="%s"/>
 </office:font-face-decls>
 <office:automatic-styles>
  <style:style style:name="co1" style:family="table-column">
   <style:table-column-properties style:column-width="9in"/></style:style>
  <style:style style:name="ce0" style:family="table-cell">
   <style:text-properties style:font-name="%s" fo:font-size="%spt"/></style:style>
 </office:automatic-styles>
 <office:body><office:spreadsheet>
  <table:table table:name="D">
   <table:table-column table:style-name="co1"/>
''' % (font, font, font, size)]
for s in lines:
    if s.startswith('*'): s = s[1:]
    p.append('   <table:table-row><table:table-cell table:style-name="ce0" '
             'office:value-type="string"><text:p>%s</text:p></table:table-cell>'
             '</table:table-row>\n' % escape(s))
p.append('  </table:table>\n </office:spreadsheet></office:body>\n</office:document>\n')
open(out, 'w').write(''.join(p))
print('wrote', out)
