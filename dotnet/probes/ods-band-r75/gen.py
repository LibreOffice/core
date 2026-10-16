#!/usr/bin/env python3
"""Author one .fods per header-band case, so that Calc's own band rule can be read off a
rendering rather than inferred from one document.

Every probe is the same sheet — one 5 cm column, five 0.5 cm rows of one word each, on an A4
page with 2 cm margins — and differs only in its header: whether there is one at all, its
declared `fo:min-height`, its `fo:margin-bottom`, and the face, size and number of lines of
its text. The band is then read as the *shift of the first cell row* against the no-header
probe, which is the one quantity that needs no assumption about where a row's text sits
inside its row.

    gen.py <outdir>
"""
import os, sys

HEAD = '''<?xml version="1.0" encoding="UTF-8"?>
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
  <style:font-face style:name="Liberation Mono" svg:font-family="&apos;Liberation Mono&apos;" style:font-family-generic="modern"/>
  <style:font-face style:name="DejaVu Sans" svg:font-family="&apos;DejaVu Sans&apos;" style:font-family-generic="swiss"/>
  <style:font-face style:name="Carlito" svg:font-family="Carlito" style:font-family-generic="swiss"/>
 </office:font-face-decls>
 <office:automatic-styles>
  <style:style style:name="co1" style:family="table-column">
   <style:table-column-properties style:column-width="5cm"/>
  </style:style>
  <style:style style:name="ro1" style:family="table-row">
   <style:table-row-properties style:row-height="0.5cm" style:use-optimal-row-height="false"/>
  </style:style>
  <style:style style:name="HF" style:family="text">
   <style:text-properties style:font-name="{face}" fo:font-size="{size}pt"/>
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
'''

TAIL = '''   </table:table>
  </office:spreadsheet>
 </office:body>
</office:document>
'''


def rows(n=5):
    out = []
    for i in range(1, n + 1):
        out.append(
            f'    <table:table-row table:style-name="ro1">\n'
            f'     <table:table-cell office:value-type="string"><text:p>R{i}</text:p>'
            f'</table:table-cell>\n    </table:table-row>\n')
    return "".join(out)


def build(path, *, header=True, min_height=None, height=None, margin_bottom="0cm",
          face="Liberation Sans", size=10, lines=1):
    bands = ""
    content = ""
    if header:
        h = (f'svg:height="{height}"' if height is not None
             else f'fo:min-height="{min_height}"')
        bands = (f'   <style:header-style>\n'
                 f'    <style:header-footer-properties {h} fo:margin-left="0cm"'
                 f' fo:margin-right="0cm" fo:margin-bottom="{margin_bottom}"/>\n'
                 f'   </style:header-style>\n')
        paras = "".join(
            f'     <text:p><text:span text:style-name="HF">H{i + 1}</text:span></text:p>\n'
            for i in range(lines))
        content = f'   <style:header>\n{paras}   </style:header>\n'
    with open(path, "w", encoding="utf-8") as f:
        f.write(HEAD.format(face=face, size=size, bands=bands, content=content))
        f.write(rows())
        f.write(TAIL)


def main():
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    cases = {}

    # The calibration: no header at all, so the first row sits at the top margin.
    cases["none"] = dict(header=False)

    # One line, a band the text comfortably fits, varying only the gap. If the band is
    # `max(minHeight, text + gap)` the first row moves point for point with the gap once the
    # sum passes the declared height, and not at all before it.
    for mm in ["0cm", "0.25cm", "0.5cm", "1cm", "1.5cm", "2cm"]:
        cases[f"gap_{mm}"] = dict(min_height="0.75cm", margin_bottom=mm)

    # The declared height alone, with no gap at all.
    for h in ["0.2cm", "0.4cm", "0.6cm", "0.75cm", "1cm", "1.5cm", "2cm"]:
        cases[f"min_{h}"] = dict(min_height=h, margin_bottom="0cm")

    # A fixed band: svg:height rather than fo:min-height, which imports as
    # HeaderIsDynamicHeight = false and must not grow.
    for h in ["0.2cm", "0.75cm", "1.5cm"]:
        cases[f"fix_{h}"] = dict(height=h, margin_bottom="0.25cm")

    # The face and the size, at a gap large enough that the text term always decides.
    for face in ["Liberation Sans", "Liberation Serif", "Liberation Mono", "DejaVu Sans",
                 "Carlito"]:
        for size in [6, 8, 10, 12, 16, 24]:
            key = f"font_{face.replace(' ', '')}_{size}"
            cases[key] = dict(min_height="0.1cm", margin_bottom="1cm", face=face, size=size)

    # Several lines, which is the other half of the text term.
    for n in [1, 2, 3, 5]:
        cases[f"lines_{n}"] = dict(min_height="0.1cm", margin_bottom="0.5cm", lines=n)

    for name, kw in cases.items():
        build(os.path.join(out, f"band-{name}.fods"), **kw)
    print(len(cases), "probes in", out)


if __name__ == "__main__":
    main()
