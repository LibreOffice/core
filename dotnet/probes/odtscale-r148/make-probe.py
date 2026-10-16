#!/usr/bin/env python3
"""One flat-ODF document per `style:text-scale` value, each a right-aligned line.

Right-aligned deliberately: the writer then states the line's origin as `margin − width(line)`,
so differencing two arms cancels every fixed term and the drawn advance can be read out of the
PDF's own text-positioning operator rather than reconstructed from glyph positions, which are
quantised to thousandths of an em. `dotnet/CLAUDE.md` records four rounds lost to that channel.
"""
import pathlib
import sys

DOC = '''<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
    xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
    xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
    xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
    xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
    office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.text">
<office:automatic-styles>
<style:page-layout style:name="pm1"><style:page-layout-properties
    fo:page-width="21cm" fo:page-height="29.7cm" fo:margin-top="2cm" fo:margin-bottom="2cm"
    fo:margin-left="2cm" fo:margin-right="2cm"/></style:page-layout>
<style:style style:name="P1" style:family="paragraph">
  <style:paragraph-properties fo:text-align="end"/>
  <style:text-properties style:font-name="Liberation Serif" fo:font-size="12pt"{scale}/>
</style:style>
</office:automatic-styles>
<office:master-styles><style:master-page style:name="Standard"
    style:page-layout-name="pm1"/></office:master-styles>
<office:body><office:text>
<text:p text:style-name="P1">Hamburgefonstiv</text:p>
</office:text></office:body>
</office:document>'''

ARMS = {
    'none': '',
    's100': ' style:text-scale="100%"',
    's60': ' style:text-scale="60%"',
    's99': ' style:text-scale="99%"',
    's130': ' style:text-scale="130%"',
}


def main():
    outdir = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'fixtures')
    outdir.mkdir(parents=True, exist_ok=True)
    for name, scale in ARMS.items():
        path = outdir / f'{name}.fodt'
        path.write_text(DOC.format(scale=scale), encoding='utf8')
        print(path, path.stat().st_size)


if __name__ == '__main__':
    main()
