#!/usr/bin/env python3
"""Four one-attribute arms of `style:table-centering`, and the fixture built from the same source.

One 4 cm column of one row on a 21 cm page with 2 cm margins, so the printable width is 17 cm and
the block is 4 -- the slack is 13 cm and half of it, 6.5 cm = 184.252 pt, is what horizontal
centring moves the block by. Vertical is the same shape down the page.

The arms differ in ONE attribute and nothing else, which is the only instrument in this project
that settles a rule: `none` states no `style:table-centering` at all, and the other three state
each of the values `PageMasterPropHdl.cxx`:320-376 accepts.
"""
import pathlib

DOC = '''<?xml version="1.0" encoding="UTF-8"?>
<!--
  style:table-centering: Calc's "centre the printed block on the page", horizontally, vertically
  or both. ODF spells it the American way and it is NOT ours to anglicise — asking for
  `style:table-centring`, which this project's British house style makes the natural spelling,
  matches nothing in any real file and silently never centres anything.

  [src] xmloff/source/core/xmltoken.cxx:1992 interns the token; PageMasterStyleMap.cxx:97-98 maps
  the one attribute onto BOTH PROP_CenterHorizontally and PROP_CenterVertically with
  MID_FLAG_MERGE_ATTRIBUTE; ScPrintFunc::PrintPage (sc/source/ui/view/printfun.cxx:2144-2191) then
  adds (pageWidth - blockWidth)/2 to the left space, UNCLAMPED, so an over-wide block hangs off
  both edges rather than being left-aligned.

  One 4 cm block on a 17 cm printable width: the slack is 13 cm and half of it is 6.5 cm, which is
  184.252 pt. LibreOffice 26.2.4.2's own PDF of the four arms (probes/odscentre-r150/reference.txt):

    none         x  56.800   y  56.800
    horizontal   x 241.052   y  56.800
    vertical     x  56.800   y 405.520
    both         x 241.052   y 405.520

  The `none` arm is the control: an absent attribute must leave the block where the margins put it.
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
  <style:page-layout style:name="pm1">
   <style:page-layout-properties fo:page-width="21cm" fo:page-height="29.7cm"
    fo:margin-left="2cm" fo:margin-right="2cm" fo:margin-top="2cm" fo:margin-bottom="2cm"
    style:print-orientation="portrait"%(centring)s/>
  </style:page-layout>
 </office:automatic-styles>
 <office:master-styles>
  <style:master-page style:name="Default" style:page-layout-name="pm1"/>
 </office:master-styles>
 <office:body>
  <office:spreadsheet>
   <table:table table:name="Probe">
    <table:table-column table:style-name="co1"/>
    <table:table-row table:style-name="ro1">
     <table:table-cell office:value-type="string"><text:p>BLOCK</text:p></table:table-cell>
    </table:table-row>
   </table:table>
  </office:spreadsheet>
 </office:body>
</office:document>
'''

ARMS = {'none': '', 'horizontal': ' style:table-centering="horizontal"',
        'vertical': ' style:table-centering="vertical"', 'both': ' style:table-centering="both"'}


def main():
    out = pathlib.Path(__file__).with_name('fixtures')
    out.mkdir(exist_ok=True)
    for name, centring in ARMS.items():
        (out / (name + '.fods')).write_text(DOC % {'centring': centring})
    print('wrote %d arms' % len(ARMS))


if __name__ == '__main__':
    main()
