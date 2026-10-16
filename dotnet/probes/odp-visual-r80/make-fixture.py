#!/usr/bin/env python3
"""Write `odp-hyperlink-field.fodp`, the fixture behind `OdpHyperlinkFieldTests`.

Four slides differing only in how one over-long URL is written and how the box is
anchored.  Everything else is held constant so that the only thing that can move a
baseline is the field rule.
"""
import os, sys

OUT = sys.argv[1] if len(sys.argv) > 1 else \
    os.path.join(os.path.dirname(os.path.abspath(__file__)),
                 "../../tests/corpus/features/odp-hyperlink-field.fodp")

NS = (
 'xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" '
 'xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" '
 'xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" '
 'xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" '
 'xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" '
 'xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" '
 'xmlns:presentation="urn:oasis:names:tc:opendocument:xmlns:presentation:1.0" '
 'xmlns:xlink="http://www.w3.org/1999/xlink"')

URL = ("https://www.example.org/working-with-us/hr-connect/organisational-development/"
       "career-and-development-planning-framework/leadership/")
LINK = f'<text:a xlink:href="{URL}" xlink:type="simple">{URL}</text:a>'

HEADER = """<?xml version="1.0" encoding="UTF-8"?>
<!-- Hand-authored. Four slides differing only in how one over-long URL is written and
     how its box is anchored. Every box is 14cm x 5cm at (1cm, 1cm), 16pt Liberation
     Sans, no paragraph margins, top-left aligned, with all four text insets at zero.

       1  the URL as a text:a           - a FIELD
       2  the same characters as text   - the control
       3  the field, middle-anchored    - does the block height count the spill?
       4  a short text:a that fits      - the control for "a field changes nothing"

     A `text:a` inside a draw shape's text is imported as an EditEngine field rather than
     as a character property, because xmloff/source/text/txtparai.cxx:1352-1370 builds an
     XMLUrlFieldImportContext whenever the cursor has no HyperLinkURL property, which is
     every Draw and Impress text and no Writer one. Two rules follow:

       * a field wider than the room left on its line is filled to the CELL rather than
         moved down or broken at a separator (impedit3.cxx:1112-1200), so a URL breaks at
         any character;
       * each line it spills onto is drawn one ASCENT below the last rather than one line
         height (impedit3.cxx:3784-3795, "only use GetMaxAscent(), pLine->GetHeight() will
         not proceed as needed ... a compressed look"), and the spill is invisible to the
         formatter, so the block's measured height counts the field's line once.

     Verified against LibreOffice 26.2.4.2. Its PDF of this file draws, in points from the
     page top:

       1  42.747  60.521  74.921  89.321  107.094
          ordinary pitch 17.773; the two spill lines 14.400 apart, which is the ascent
          Liberation Sans reports at 16pt through the draw layer's reference device.
          The lines read:
             Alpha beta gamma.
             https://www.example.org/working-with-us/hr-connect/or
             ganisational-development/career-and-development-plan
             ning-framework/leadership/
             Omega.
       2  42.747  60.521  78.294  96.067  113.840
          every pitch 17.773, and the breaks fall at the solidi:
             https://www.example.org/working-with-us/hr-connect/
             organisational-development/career-and-development-
             planning-framework/leadership/
       3  95.840 113.614 128.014 142.414
          the box is 28.35-170.08 pt, centred on 99.2. Two EditLines of 17.773 = 35.55,
          so the block's top is 81.43 and its first baseline 95.83; the two spill lines
          are drawn below the box and are not in the height that centred it.
       4  42.747  60.521  78.294 = a field that fits changes nothing.
-->
"""


def style(name, anchor):
    return (
        f'<style:style style:name="{name}" style:family="graphic" '
        f'style:parent-style-name="standard">'
        f'<style:graphic-properties draw:fill="none" draw:stroke="none" '
        f'draw:auto-grow-height="false" draw:auto-grow-width="false" '
        f'draw:textarea-vertical-align="{anchor}" '
        f'fo:padding-top="0cm" fo:padding-bottom="0cm" '
        f'fo:padding-left="0cm" fo:padding-right="0cm"/>'
        f'<style:paragraph-properties fo:margin-top="0cm" fo:margin-bottom="0cm"/>'
        f'<style:text-properties fo:font-size="16pt" fo:font-family="Liberation Sans"/>'
        f'</style:style>')


def page(name, gr, body):
    return (f'<draw:page draw:name="{name}" draw:master-page-name="">'
            f'<draw:frame draw:style-name="{gr}" draw:text-style-name="P0" '
            f'draw:layer="layout" svg:width="14cm" svg:height="5cm" svg:x="1cm" svg:y="1cm">'
            f'<draw:text-box>{body}</draw:text-box></draw:frame></draw:page>')


def p(inner):
    return f'<text:p text:style-name="P0"><text:span text:style-name="T16">{inner}</text:span></text:p>'


def pa(inner):
    return f'<text:p text:style-name="P0">{inner}</text:p>'


SHORT = f'<text:a xlink:href="{URL}" xlink:type="simple">short.example</text:a>'

document = (
    HEADER +
    f'<office:document {NS} office:version="1.3" '
    f'office:mimetype="application/vnd.oasis.opendocument.presentation">'
    f'<office:styles>'
    f'<style:style style:name="standard" style:family="graphic">'
    f'<style:graphic-properties draw:fill="none" draw:stroke="none"/>'
    f'<style:text-properties fo:font-size="16pt" fo:font-family="Liberation Sans"/>'
    f'</style:style>'
    f'<style:style style:name="P0" style:family="paragraph">'
    f'<style:paragraph-properties fo:margin-top="0cm" fo:margin-bottom="0cm"/>'
    f'<style:text-properties fo:font-size="16pt" fo:font-family="Liberation Sans"/>'
    f'</style:style>'
    f'</office:styles>'
    f'<office:automatic-styles>'
    + style("grTop", "top") + style("grMiddle", "middle") +
    f'<style:style style:name="T16" style:family="text">'
    f'<style:text-properties fo:font-size="16pt" fo:font-family="Liberation Sans"/>'
    f'</style:style>'
    f'</office:automatic-styles>'
    f'<office:body><office:presentation>'
    + page("field", "grTop", p("Alpha beta gamma.") + pa(LINK) + p("Omega."))
    + page("text", "grTop", p("Alpha beta gamma.") + p(URL) + p("Omega."))
    + page("middle", "grMiddle", p("Alpha beta.") + pa(LINK))
    + page("short", "grTop", p("Alpha beta gamma.") + pa(SHORT) + p("Omega."))
    + f'</office:presentation></office:body></office:document>')

os.makedirs(os.path.dirname(os.path.abspath(OUT)), exist_ok=True)
open(OUT, "w", encoding="utf-8").write(document)
print(OUT)
