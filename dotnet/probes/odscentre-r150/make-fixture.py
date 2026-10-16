#!/usr/bin/env python3
"""The four arms as one fixture: four sheets, each naming its own master page.

A page layout is per master page, so four sheets with four masters put the whole factorial in one
file and one rendering -- which is what a test wants, because the arms then share every other
input by construction rather than by four files agreeing.
"""
import pathlib

HEAD = '''<?xml version="1.0" encoding="UTF-8"?>
<!--
  style:table-centering: Calc's "centre the printed block on the page", horizontally, vertically or
  both. ODF spells it the American way and it is not ours to anglicise; this project's British house
  style makes `style:table-centring` the natural spelling, it matches nothing in any real file, and
  a reader asking for it silently never centres anything. That was the whole of seat O100.

  [src] xmloff/source/core/xmltoken.cxx:1992 interns the token; PageMasterStyleMap.cxx:97-98 maps
  the ONE attribute onto BOTH PROP_CenterHorizontally and PROP_CenterVertically with
  MID_FLAG_MERGE_ATTRIBUTE, its value being horizontal, vertical or both;
  ScPrintFunc::PrintPage (sc/source/ui/view/printfun.cxx:2144-2191) then adds half the slack to the
  left and top space, UNCLAMPED, so an over-wide block hangs off both edges instead of being left
  aligned.

  Four sheets, each with its own master page, differing in that one attribute and nothing else. One
  4 cm column on a 21 cm page with 2 cm margins gives a 17 cm printable width, so the slack is 13 cm
  and half of it is 6.5 cm = 184.252 pt, predicted with no free parameter.

  LibreOffice 26.2.4.2 draws the four arms at (probes/odscentre-r150/reference.txt):

    none         x  57.685   y  57.276
    horizontal   x 241.937   y  57.276      dx +184.252, exactly 6.5 cm
    vertical     x  57.685   y 415.121      dy +357.845
    both         x 241.937   y 415.121

  `none` is the control: an absent attribute must leave the block where the margins put it.
-->
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.spreadsheet">
 <office:automatic-styles>
  <style:style style:name="co1" style:family="table-column">
   <style:table-column-properties style:column-width="4cm"/>
  </style:style>
  <style:style style:name="ro1" style:family="table-row">
   <style:table-row-properties style:row-height="0.45cm"/>
  </style:style>
'''

LAYOUT = '''  <style:page-layout style:name="pm%(n)d">
   <style:page-layout-properties fo:page-width="21cm" fo:page-height="29.7cm"
    fo:margin-left="2cm" fo:margin-right="2cm" fo:margin-top="2cm" fo:margin-bottom="2cm"
    style:print-orientation="portrait"%(centring)s/>
  </style:page-layout>
'''

MASTER = '  <style:master-page style:name="mp%(n)d" style:page-layout-name="pm%(n)d"/>\n'

CELL = '''  <style:style style:name="ta%(n)d" style:family="table" style:master-page-name="mp%(n)d">
   <style:table-properties table:display="true"/>
  </style:style>
'''

SHEET = '''   <table:table table:name="%(name)s" table:style-name="ta%(n)d">
    <table:table-column table:style-name="co1"/>
    <table:table-row table:style-name="ro1">
     <table:table-cell office:value-type="string"><text:p>%(name)s</text:p></table:table-cell>
    </table:table-row>
   </table:table>
'''

ARMS = [('none', ''), ('horizontal', ' style:table-centering="horizontal"'),
        ('vertical', ' style:table-centering="vertical"'), ('both', ' style:table-centering="both"')]


def main():
    parts = [HEAD]
    for n, (name, centring) in enumerate(ARMS, 1):
        parts.append(LAYOUT % {'n': n, 'centring': centring})
        parts.append(CELL % {'n': n})
    parts.append(' </office:automatic-styles>\n <office:master-styles>\n')
    for n, _ in enumerate(ARMS, 1):
        parts.append(MASTER % {'n': n})
    parts.append(' </office:master-styles>\n <office:body>\n  <office:spreadsheet>\n')
    for n, (name, _) in enumerate(ARMS, 1):
        parts.append(SHEET % {'n': n, 'name': name})
    parts.append('  </office:spreadsheet>\n </office:body>\n</office:document>\n')

    out = pathlib.Path(__file__).resolve().parents[2] / 'tests/corpus/features/sheet-print-centring.fods'
    out.write_text(''.join(parts))
    print('wrote %s (%d bytes)' % (out, out.stat().st_size))


if __name__ == '__main__':
    main()
