#!/usr/bin/env python3
"""The second family: does the band's text term have a floor, and is that floor the
workbook's own default cell font?

The first family (gen.py) found a header of Liberation Sans 6, 8 or 10 pt all giving the same
band, and 12 pt and up growing with the size — so the text term is not simply the line height
of the face the header names. This varies the *default cell style's* font independently of the
header's own, which is the discriminator: if the floor is the default cell font, a 6 pt header
in a 20 pt workbook takes the 20 pt line.

    gen2.py <outdir>
"""
import os, sys

DOC = '''<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.spreadsheet">
 <office:font-face-decls>
  <style:font-face style:name="Liberation Sans" svg:font-family="&apos;Liberation Sans&apos;" style:font-family-generic="swiss"/>
  <style:font-face style:name="Liberation Serif" svg:font-family="&apos;Liberation Serif&apos;" style:font-family-generic="roman"/>
  <style:font-face style:name="DejaVu Sans" svg:font-family="&apos;DejaVu Sans&apos;" style:font-family-generic="swiss"/>
 </office:font-face-decls>
 <office:styles>
  <style:default-style style:family="table-cell">
   <style:text-properties style:font-name="{dface}" fo:font-size="{dsize}pt"/>
  </style:default-style>
  <style:style style:name="Default" style:family="table-cell">
   <style:text-properties style:font-name="{dface}" fo:font-size="{dsize}pt"/>
  </style:style>
 </office:styles>
 <office:automatic-styles>
  <style:style style:name="co1" style:family="table-column">
   <style:table-column-properties style:column-width="5cm"/>
  </style:style>
  <style:style style:name="ro1" style:family="table-row">
   <style:table-row-properties style:row-height="1.2cm" style:use-optimal-row-height="false"/>
  </style:style>
  <style:style style:name="HF" style:family="text">
   <style:text-properties style:font-name="{hface}" fo:font-size="{hsize}pt"/>
  </style:style>
  <style:page-layout style:name="pm1">
   <style:page-layout-properties fo:page-width="21cm" fo:page-height="29.7cm"
    fo:margin-left="2cm" fo:margin-right="2cm" fo:margin-top="2cm" fo:margin-bottom="2cm"
    style:print-orientation="portrait"/>
{bands}  </style:page-layout>
 </office:automatic-styles>
 <office:master-styles>
  <style:master-page style:name="Default" style:page-layout-name="pm1">
{content}  </style:master-page>
 </office:master-styles>
 <office:body>
  <office:spreadsheet>
   <table:table table:name="Probe">
    <table:table-column table:style-name="co1"/>
{rows}   </table:table>
  </office:spreadsheet>
 </office:body>
</office:document>
'''

BAND = ('   <style:header-style>\n'
        '    <style:header-footer-properties fo:min-height="0.1cm" fo:margin-left="0cm"'
        ' fo:margin-right="0cm" fo:margin-bottom="1cm"/>\n'
        '   </style:header-style>\n')


def rows(n=4):
    return "".join(
        f'    <table:table-row table:style-name="ro1">\n'
        f'     <table:table-cell office:value-type="string"><text:p>R{i}</text:p>'
        f'</table:table-cell>\n    </table:table-row>\n' for i in range(1, n + 1))


def build(path, *, dface="Liberation Sans", dsize=10, hface="Liberation Sans", hsize=10,
          spans=True, lines=1):
    if spans:
        paras = "".join(
            f'     <text:p><text:span text:style-name="HF">H{i + 1}</text:span></text:p>\n'
            for i in range(lines))
    else:
        paras = "".join(f'     <text:p>H{i + 1}</text:p>\n' for i in range(lines))
    content = f'   <style:header>\n{paras}   </style:header>\n'
    with open(path, "w", encoding="utf-8") as f:
        f.write(DOC.format(dface=dface, dsize=dsize, hface=hface, hsize=hsize,
                           bands=BAND, content=content, rows=rows()))


def main():
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    cases = {}
    # The calibration for this family: no header on an otherwise identical sheet.
    for dsize in [6, 10, 14, 20]:
        with open(os.path.join(out, f"band-none{dsize}.fods"), "w", encoding="utf-8") as f:
            f.write(DOC.format(dface="Liberation Sans", dsize=dsize, hface="Liberation Sans",
                               hsize=10, bands="", content="", rows=rows()))
    # Header size against default size, both ways round.
    for dsize in [6, 10, 14, 20]:
        for hsize in [6, 10, 14, 20]:
            cases[f"d{dsize}h{hsize}"] = dict(dsize=dsize, hsize=hsize)
    # A header paragraph carrying no run properties at all.
    for dsize in [6, 10, 20]:
        cases[f"bare_d{dsize}"] = dict(dsize=dsize, spans=False)
    # A default face whose line height differs from the header's at the same size.
    cases["dserif20_h6"] = dict(dface="Liberation Serif", dsize=20, hsize=6)
    cases["ddejavu20_h6"] = dict(dface="DejaVu Sans", dsize=20, hsize=6)
    # Two lines of different sizes, to see whether the floor is per line or per band.
    cases["mixed"] = dict(dsize=20, hsize=6, lines=2)

    for name, kw in cases.items():
        build(os.path.join(out, f"band-{name}.fods"), **kw)
    print(len(cases), "probes in", out)


if __name__ == "__main__":
    main()
