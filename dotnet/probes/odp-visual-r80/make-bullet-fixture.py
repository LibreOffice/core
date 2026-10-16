#!/usr/bin/env python3
"""Write `odp-list-bullet-symbol.fodp`, the fixture behind `OdpBulletMarkerTests`.

Three list items differing only in the level that labels them: a Wingdings slot in the
Private Use Area with a colour of its own, the same slot with no colour, and an
OpenSymbol character that is not a slot at all.
"""
import os, sys

OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "../../tests/corpus/features/odp-list-bullet-symbol.fodp")

NS = (
 'xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" '
 'xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" '
 'xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" '
 'xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" '
 'xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" '
 'xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" '
 'xmlns:presentation="urn:oasis:names:tc:opendocument:xmlns:presentation:1.0"')

HEADER = """<?xml version="1.0" encoding="UTF-8"?>
<!-- Hand-authored. Three slides, each one bulleted item in a 14cm box at 16pt
     Liberation Sans, differing only in the list level that labels it:

       1  text:bullet-char="&#xF0FC;" style:font-name="Wingdings" fo:color="#9bbb59"
       2  the same slot, with style:use-window-font-color="true" instead of a colour
       3  text:bullet-char="&#x25CF;" fo:font-family="OpenSymbol", which is no slot

     A Private Use Area character means nothing outside the face that defines it, so a
     reader that hands it to a renderer draws .notdef and one that collapses it to U+2022
     draws a dot. LibreOffice does neither: Wingdings is not installed on Linux, so
     `ConvertChar` substitutes OpenSymbol and recodes the slot through `aWingDingsTab`
     (`unotools/source/misc/fontcvt.cxx`:185, 1325) to the StarSymbol code point holding
     the same picture. Slot F0FC is a check mark, and its recode is U+E4C2.

     The colour is the level's own and is a separate reading: `fo:color` on the level's
     `style:text-properties`, with `style:use-window-font-color="true"` meaning the item's
     own colour instead. DrawingML's `a:buClr` is the same property and the deck reader has
     read it since it was written.

     Verified against LibreOffice 26.2.4.2. Its PDF of this file draws the three markers
     from the font OpenSymbol in the colours #9bbb59, #000000 and #000000; the first two
     are one glyph, the third another.
-->
"""


def page(name, listname, text):
    return (f'<draw:page draw:name="{name}" draw:master-page-name="">'
            f'<draw:frame draw:style-name="gr" draw:text-style-name="P0" draw:layer="layout" '
            f'svg:width="14cm" svg:height="4cm" svg:x="1cm" svg:y="1cm">'
            f'<draw:text-box><text:list text:style-name="{listname}">'
            f'<text:list-item><text:p text:style-name="P0">'
            f'<text:span text:style-name="T16">{text}</text:span></text:p></text:list-item>'
            f'</text:list></draw:text-box></draw:frame></draw:page>')


def bullet_style(name, char, properties):
    return (f'<text:list-style style:name="{name}">'
            f'<text:list-level-style-bullet text:level="1" text:bullet-char="{char}">'
            f'<style:list-level-properties text:min-label-width="1cm"/>'
            f'<style:text-properties {properties}/>'
            f'</text:list-level-style-bullet></text:list-style>')


document = (
    HEADER +
    f'<office:document {NS} office:version="1.3" '
    f'office:mimetype="application/vnd.oasis.opendocument.presentation">'
    f'<office:font-face-decls>'
    f'<style:font-face style:name="Wingdings" svg:font-family="Wingdings" '
    f'style:font-pitch="variable" style:font-charset="x-symbol"/>'
    f'<style:font-face style:name="OpenSymbol" svg:font-family="OpenSymbol" '
    f'style:font-pitch="variable" style:font-charset="x-symbol"/>'
    f'</office:font-face-decls>'
    f'<office:styles>'
    f'<style:style style:name="standard" style:family="graphic">'
    f'<style:graphic-properties draw:fill="none" draw:stroke="none"/>'
    f'<style:text-properties fo:font-size="16pt" fo:font-family="Liberation Sans"/>'
    f'</style:style>'
    f'<style:style style:name="P0" style:family="paragraph">'
    f'<style:paragraph-properties fo:margin-top="0cm" fo:margin-bottom="0cm"/>'
    f'<style:text-properties fo:font-size="16pt" fo:font-family="Liberation Sans"/>'
    f'</style:style>'
    + f'</office:styles>'
    f'<office:automatic-styles>'
    + bullet_style("Lcolour", "&#xF0FC;",
                   'style:font-name="Wingdings" fo:color="#9bbb59" fo:font-size="90%"')
    + bullet_style("Lplain", "&#xF0FC;",
                   'style:font-name="Wingdings" style:use-window-font-color="true" '
                   'fo:font-size="90%"')
    + bullet_style("Lsymbol", "&#x25CF;",
                   'fo:font-family="OpenSymbol" style:use-window-font-color="true" '
                   'fo:font-size="45%"')
    + f'<style:style style:name="gr" style:family="graphic" style:parent-style-name="standard">'
    f'<style:graphic-properties draw:fill="none" draw:stroke="none" '
    f'draw:auto-grow-height="false" draw:auto-grow-width="false" '
    f'fo:padding-top="0cm" fo:padding-bottom="0cm" '
    f'fo:padding-left="0cm" fo:padding-right="0cm"/></style:style>'
    f'<style:style style:name="T16" style:family="text">'
    f'<style:text-properties fo:font-size="16pt" fo:font-family="Liberation Sans"/></style:style>'
    f'</office:automatic-styles>'
    f'<office:body><office:presentation>'
    + page("colour", "Lcolour", "Alpha")
    + page("plain", "Lplain", "Alpha")
    + page("symbol", "Lsymbol", "Alpha")
    + f'</office:presentation></office:body></office:document>')

os.makedirs(os.path.dirname(os.path.abspath(OUT)), exist_ok=True)
open(OUT, "w", encoding="utf-8").write(document)
print(OUT)
