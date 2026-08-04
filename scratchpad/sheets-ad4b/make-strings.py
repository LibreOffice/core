#!/usr/bin/env python3
"""Author a flat-ODS probe holding one supplied string per row, so LibreOffice's
recomputed row height reports how many lines each took at a known column width."""
import sys
from xml.sax.saxutils import escape

out, width_twips, size, font, strings_file = sys.argv[1:6]
width_twips = int(width_twips)

lines = [l.rstrip('\n') for l in open(strings_file) if l.strip()]

p = []
p.append('''<?xml version="1.0" encoding="UTF-8"?>
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
''' % (font, font))
p.append('  <style:style style:name="co1" style:family="table-column">'
         '<style:table-column-properties style:column-width="%.4fin"/></style:style>\n'
         % (width_twips / 1440.0))
p.append('  <style:style style:name="ro1" style:family="table-row">'
         '<style:table-row-properties style:row-height="0.1783in" '
         'style:use-optimal-row-height="true"/></style:style>\n')
p.append('  <style:style style:name="ce0" style:family="table-cell">'
         '<style:table-cell-properties fo:wrap-option="wrap"/>'
         '<style:text-properties style:font-name="%s" fo:font-size="%spt"/>'
         '</style:style>\n' % (font, size))
p.append('  <style:style style:name="ce1" style:family="table-cell">'
         '<style:table-cell-properties fo:wrap-option="wrap"/>'
         '<style:text-properties style:font-name="%s" fo:font-size="%spt" '
         'fo:font-weight="bold"/></style:style>\n' % (font, size))
p.append(' </office:automatic-styles>\n <office:body><office:spreadsheet>\n')
p.append('  <table:table table:name="S">\n   <table:table-column table:style-name="co1"/>\n')
for s in lines:
    bold = s.startswith('*')
    if bold: s = s[1:]
    paras = ''.join('<text:p>%s</text:p>' % escape(x) for x in s.split('\\n'))
    p.append('   <table:table-row table:style-name="ro1">'
             '<table:table-cell table:style-name="ce%d" office:value-type="string">%s'
             '</table:table-cell></table:table-row>\n' % (1 if bold else 0, paras))
p.append('  </table:table>\n </office:spreadsheet></office:body>\n</office:document>\n')
open(out, 'w').write(''.join(p))
print('wrote', out, len(lines), 'rows')
