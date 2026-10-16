#!/usr/bin/env python3
"""Fixture B -- the same advance channel, over PROPORTIONS other than 58.

`w:vertAlign` can only ever mean DFLT_ESC_PROP (58), so a DOCX cannot say whether the quantisation
rule is a property of 58 or of the arithmetic.  ODF's `style:text-position="<rise>% <proportion>%"`
states the proportion outright, so this sweeps it at three base sizes.

    make-prop.py <out.fodt>
"""
import sys

FACE = 'Liberation Sans'
DIGIT = '0'
N1, N2 = 2, 42
SIZES = [8.0, 10.0, 11.0, 13.0]
PROPS = list(range(25, 101, 5))

styles, body = [], []
for si, size in enumerate(SIZES):
    for prop in PROPS:
        for n in (N1, N2):
            name = f'S{si}P{prop}N{n}'
            styles.append(
                f'<style:style style:name="T{name}" style:family="text">'
                f'<style:text-properties fo:font-size="{size}pt" style:font-name="LS" '
                f'style:text-position="33% {prop}%"/></style:style>')
            body.append(
                f'<text:p text:style-name="R">'
                f'<text:span text:style-name="L">ARM {name} </text:span>'
                f'<text:span text:style-name="T{name}">{DIGIT * n}</text:span></text:p>')

doc = f'''<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.text">
<office:font-face-decls>
<style:font-face style:name="LS" svg:font-family="{FACE}"/>
</office:font-face-decls>
<office:automatic-styles>
<style:style style:name="R" style:family="paragraph">
<style:paragraph-properties fo:text-align="end" style:justify-single-word="false"/>
<style:text-properties fo:font-size="9pt" style:font-name="LS"/></style:style>
<style:style style:name="L" style:family="text">
<style:text-properties fo:font-size="9pt" style:font-name="LS"/></style:style>
<style:style style:name="PM" style:family="page-layout">
<style:page-layout-properties fo:page-width="22in" fo:page-height="11in"
 fo:margin-top="0.5in" fo:margin-bottom="0.5in" fo:margin-left="0.5in" fo:margin-right="0.5in"
 style:print-orientation="landscape"/></style:style>
{''.join(styles)}
</office:automatic-styles>
<office:master-styles>
<style:master-page style:name="Standard" style:page-layout-name="PM"/>
</office:master-styles>
<office:body><office:text>
{''.join(body)}
</office:text></office:body></office:document>'''

out = sys.argv[1]
open(out, 'w').write(doc)
print(f'{out}: {len(body)} arms, {len(SIZES)} sizes x {len(PROPS)} proportions')
