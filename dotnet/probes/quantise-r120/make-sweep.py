#!/usr/bin/env python3
"""Build the O64 sweep fixtures: one line per (face, size, decoration), three modules.

The claim under test is that a rule's thickness is decided by the PDF *writer's* own
reference device -- 720 dpi -- and quantised to a whole hundredth of a millimetre by the map
unit the metafile is recorded in, and that both halves are the same for Writer, Calc and
Impress because `PDFExport::ExportSelection` records every page the same way
(`filter/source/pdf/pdfexport.cxx`:168-179).  So the sweep is authored three times over the
same grid; if the three agree cell for cell, the module is not a variable.

Flat ODF rather than a zip, because it is one file per module and there is nothing in these
fixtures that flat ODF gets wrong -- no `draw:frame`, no custom shape, no automatic row
height.  Every paragraph names its face and size in its own text, so a row can be attributed
from the drawn text alone rather than from its position.

  faces      three Liberation (the descent branch -- and Mono's descent trips #i55341's clamp)
             and three that are not (the HarfBuzz branch; Caladea is 1000 upem and asks for
             its typographic metrics, so it separates the two metric precedences as well)
  sizes      fifteen, chosen so that the predicted pixel count steps several times per face
  kinds      single underline, double underline, strikethrough -- the three sizes
             `ImplInitTextLineSize` computes separately
"""
import sys

FACES = ['Liberation Sans', 'Liberation Serif', 'Liberation Mono',
         'Carlito', 'Caladea', 'DejaVu Sans']
SIZES = [6, 7, 8, 9, 10, 11, 12, 14, 16, 18, 20, 24, 28, 36, 48]
KINDS = ['single', 'double', 'strike']

DECOR = {
    'single': 'style:text-underline-style="solid" style:text-underline-width="auto" '
              'style:text-underline-color="font-color" style:text-underline-type="single"',
    'double': 'style:text-underline-style="solid" style:text-underline-width="auto" '
              'style:text-underline-color="font-color" style:text-underline-type="double"',
    'strike': 'style:text-line-through-style="solid" style:text-line-through-type="single"',
}

NS = (
 'xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" '
 'xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" '
 'xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" '
 'xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" '
 'xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" '
 'xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" '
 'xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" '
 'xmlns:presentation="urn:oasis:names:tc:opendocument:xmlns:presentation:1.0" '
 'xmlns:calcext="urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0"')


def rows():
    for face in FACES:
        for size in SIZES:
            for kind in KINDS:
                yield face, size, kind


def label(index):
    """A short opaque tag, joined to (face, size, kind) through the manifest.

    Short because the label is drawn AT the size under test: a 48 pt row spelling its own
    face out ran off the page in Calc and was clipped, which silently lost fourteen rows of
    the first cut of this sweep.
    """
    return 'Q%d' % index


def fontfaces():
    return ''.join('<style:font-face style:name="%s" svg:font-family="&apos;%s&apos;"/>' % (f, f)
                   for f in FACES)


def textstyles(family):
    out = []
    for i, (face, size, kind) in enumerate(rows()):
        out.append(
            '<style:style style:name="T%d" style:family="%s">'
            '<style:text-properties style:font-name="%s" fo:font-size="%dpt" %s '
            'style:font-name-asian="%s" style:font-size-asian="%dpt" '
            'style:font-name-complex="%s" style:font-size-complex="%dpt"/>'
            '</style:style>'
            % (i, family, face, size, DECOR[kind], face, size, face, size))
    return ''.join(out)


def writer(path):
    paras = ''.join(
        '<text:p><text:span text:style-name="T%d">%s</text:span></text:p>' % (i, label(i))
        for i, r in enumerate(rows()))
    xml = (
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        '<office:document %s office:version="1.3" '
        'office:mimetype="application/vnd.oasis.opendocument.text">'
        '<office:font-face-decls>%s</office:font-face-decls>'
        '<office:automatic-styles>'
        '<style:style style:name="PM" style:family="paragraph">'
        '<style:text-properties fo:font-size="10pt"/></style:style>'
        '%s</office:automatic-styles>'
        '<office:body><office:text>%s</office:text></office:body>'
        '</office:document>' % (NS, fontfaces(), textstyles('text'), paras))
    open(path, 'w', encoding='utf-8').write(xml)


def calc(path):
    cells = ''.join(
        '<table:table-row><table:table-cell office:value-type="string">'
        '<text:p><text:span text:style-name="T%d">%s</text:span></text:p>'
        '</table:table-cell></table:table-row>' % (i, label(i))
        for i, r in enumerate(rows()))
    xml = (
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        '<office:document %s office:version="1.3" '
        'office:mimetype="application/vnd.oasis.opendocument.spreadsheet">'
        '<office:font-face-decls>%s</office:font-face-decls>'
        '<office:automatic-styles>%s'
        '<style:style style:name="co1" style:family="table-column">'
        '<style:table-column-properties style:column-width="12cm"/></style:style>'
        '</office:automatic-styles>'
        '<office:body><office:spreadsheet><table:table table:name="Probe">'
        '<table:table-column table:style-name="co1"/>%s'
        '</table:table></office:spreadsheet></office:body>'
        '</office:document>' % (NS, fontfaces(), textstyles('text'), cells))
    open(path, 'w', encoding='utf-8').write(xml)


def impress(path):
    """One slide per (face, size), three lines on it -- the three kinds at one size.

    Grouped that way rather than by a fixed row count because a text box does not grow and a
    slide of fifteen 48 pt lines is taller than the slide: the overflow is clipped away and
    the rows vanish without a word."""
    per = len(KINDS)
    all_rows = list(rows())
    pages = []
    for start in range(0, len(all_rows), per):
        chunk = all_rows[start:start + per]
        lines = ''.join(
            '<text:p><text:span text:style-name="T%d">%s</text:span></text:p>'
            % (start + j, label(start + j))
            for j, r in enumerate(chunk))
        pages.append(
            '<draw:page draw:name="p%d" draw:master-page-name="M">'
            '<draw:frame draw:style-name="gr" draw:layer="layout" svg:width="24cm" '
            'svg:height="16cm" svg:x="1cm" svg:y="1cm">'
            '<draw:text-box>%s</draw:text-box></draw:frame></draw:page>'
            % (start // per, lines))
    xml = (
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        '<office:document %s office:version="1.3" '
        'office:mimetype="application/vnd.oasis.opendocument.presentation">'
        '<office:font-face-decls>%s</office:font-face-decls>'
        '<office:automatic-styles>%s'
        '<style:style style:name="gr" style:family="graphic">'
        '<style:graphic-properties draw:fill="none" draw:stroke="none" '
        'draw:auto-grow-height="false" fo:padding-top="0cm" fo:padding-bottom="0cm" '
        'fo:padding-left="0cm" fo:padding-right="0cm"/></style:style>'
        '</office:automatic-styles>'
        '<office:master-styles><style:master-page style:name="M" '
        'style:page-layout-name="PL"/></office:master-styles>'
        '<office:body><office:presentation>%s</office:presentation></office:body>'
        '</office:document>' % (NS, fontfaces(), textstyles('text'), ''.join(pages)))
    open(path, 'w', encoding='utf-8').write(xml)


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    writer(out + '/sweep-fodt.fodt')
    calc(out + '/sweep-fods.fods')
    impress(out + '/sweep-fodp.fodp')
    with open(out + '/manifest.tsv', 'w') as fh:
        fh.write('label\tface\tsize\tkind\n')
        for i, (face, size, kind) in enumerate(rows()):
            fh.write('%s\t%s\t%d\t%s\n' % (label(i), face, size, kind))
    print('%d rows: %d faces x %d sizes x %d kinds'
          % (len(list(rows())), len(FACES), len(SIZES), len(KINDS)))
