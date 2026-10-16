#!/usr/bin/env python3
r"""A POSITION probe for a double underline: where the two lines sit, not only how thick.

    make-posprobe.py <outdir>

Round 120's sweep scored a rule's THICKNESS and nothing else -- `read-rules.py` takes the
median of the rules it finds in a window and never reports where they are -- so nothing in
its bank distinguishes a second line drawn one thickness below the first from one drawn two
or three thicknesses below.  This authors the same shape of fixture at a reduced grid and
reads each rule's own offset from its own baseline.

Grid: six faces (three on the descent branch, three on the HarfBuzz branch) x seven sizes x
three kinds.  The sizes reach down to 6 pt on purpose: the descent branch floors the gap
between the two lines at `1 + DPIY/150` device pixels (`fontmetric.cxx`:303-306), which is
FIVE at the PDF writer's 720 dpi, so the floor binds at every size where the double
underline is thinner than five pixels and does not bind above it.  A probe that only looks
at large text cannot see that term at all.

Two modules, not three: Writer, whose map unit is the twip, and Calc, whose map unit is the
hundredth of a millimetre.  The Impress leg is deliberately absent -- the slides half of O68
is closed as nil reach (0 of 302 `.odp` state a double underline and `.ppt` cannot state
one), so authoring a fixture for it would be measuring a path no document takes.
"""
import sys

FACES = ['Liberation Sans', 'Liberation Serif', 'Liberation Mono',
         'Carlito', 'Caladea', 'DejaVu Sans']
SIZES = [6, 8, 10, 12, 16, 24, 48]
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
 'xmlns:calcext="urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0"')


def rows():
    for face in FACES:
        for size in SIZES:
            for kind in KINDS:
                yield face, size, kind


def label(i):
    """Short, because the label is drawn AT the size under test and a long one at 48 pt is
    clipped away -- which silently lost fourteen rows of round 120's first cut."""
    return 'P%d' % i


def fontfaces():
    return ''.join('<style:font-face style:name="%s" svg:font-family="&apos;%s&apos;"/>' % (f, f)
                   for f in FACES)


def textstyles():
    return ''.join(
        '<style:style style:name="T%d" style:family="text">'
        '<style:text-properties style:font-name="%s" fo:font-size="%dpt" %s '
        'style:font-name-asian="%s" style:font-size-asian="%dpt" '
        'style:font-name-complex="%s" style:font-size-complex="%dpt"/>'
        '</style:style>' % (i, face, size, DECOR[kind], face, size, face, size)
        for i, (face, size, kind) in enumerate(rows()))


def writer(path):
    paras = ''.join('<text:p><text:span text:style-name="T%d">%s</text:span></text:p>'
                    % (i, label(i)) for i, _ in enumerate(rows()))
    open(path, 'w', encoding='utf-8').write(
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        '<office:document %s office:version="1.3" '
        'office:mimetype="application/vnd.oasis.opendocument.text">'
        '<office:font-face-decls>%s</office:font-face-decls>'
        '<office:automatic-styles>%s</office:automatic-styles>'
        '<office:body><office:text>%s</office:text></office:body>'
        '</office:document>' % (NS, fontfaces(), textstyles(), paras))


def calc(path):
    cells = ''.join(
        '<table:table-row><table:table-cell office:value-type="string">'
        '<text:p><text:span text:style-name="T%d">%s</text:span></text:p>'
        '</table:table-cell></table:table-row>' % (i, label(i))
        for i, _ in enumerate(rows()))
    open(path, 'w', encoding='utf-8').write(
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
        '</office:document>' % (NS, fontfaces(), textstyles(), cells))


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    writer(out + '/pos-fodt.fodt')
    calc(out + '/pos-fods.fods')
    with open(out + '/pos-manifest.tsv', 'w') as fh:
        fh.write('label\tface\tsize\tkind\n')
        for i, (face, size, kind) in enumerate(rows()):
            fh.write('%s\t%s\t%d\t%s\n' % (label(i), face, size, kind))
    print('%d rows: %d faces x %d sizes x %d kinds'
          % (len(list(rows())), len(FACES), len(SIZES), len(KINDS)))
