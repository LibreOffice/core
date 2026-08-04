#!/usr/bin/env python3
"""Author a flat-ODS probe: one long word per row, so LibreOffice's recomputed row
height reports how many lines it thought the word took at a known column width."""
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
  <style:font-face style:name="Liberation Sans" svg:font-family="&apos;Liberation Sans&apos;"/>
 </office:font-face-decls>
 <office:automatic-styles>
'''

TAIL = ''' </office:body>
</office:document>
'''


def main():
    out = sys.argv[1]
    width_twips = int(sys.argv[2]) if len(sys.argv) > 2 else 4000
    word = sys.argv[3] if len(sys.argv) > 3 else 'n'
    sizes = [float(s) for s in (sys.argv[4].split(',') if len(sys.argv) > 4 else ['11'])]
    font = sys.argv[5] if len(sys.argv) > 5 else 'Calibri'
    kmax = int(sys.argv[6]) if len(sys.argv) > 6 else 90

    parts = [HEAD]
    parts.append('  <style:style style:name="co1" style:family="table-column">'
                 '<style:table-column-properties style:column-width="%.4fin"/>'
                 '</style:style>\n' % (width_twips / 1440.0))
    parts.append('  <style:style style:name="ro1" style:family="table-row">'
                 '<style:table-row-properties style:row-height="0.1783in" '
                 'style:use-optimal-row-height="true"/></style:style>\n')
    for i, s in enumerate(sizes):
        parts.append(
            '  <style:style style:name="ce%d" style:family="table-cell">'
            '<style:table-cell-properties fo:wrap-option="wrap"/>'
            '<style:text-properties style:font-name="%s" fo:font-size="%gpt"/>'
            '</style:style>\n' % (i, font, s))
    parts.append(' </office:automatic-styles>\n <office:body><office:spreadsheet>\n')

    for i, s in enumerate(sizes):
        parts.append('  <table:table table:name="S%g">\n' % s)
        parts.append('   <table:table-column table:style-name="co1"/>\n')
        for k in range(1, kmax + 1):
            parts.append(
                '   <table:table-row table:style-name="ro1">'
                '<table:table-cell table:style-name="ce%d" office:value-type="string">'
                '<text:p>%s</text:p></table:table-cell></table:table-row>\n'
                % (i, word * k))
        parts.append('  </table:table>\n')

    parts.append(' </office:spreadsheet>')
    parts.append(TAIL)
    open(out, 'w').write(''.join(parts))
    print('wrote', out)


main()
