#!/usr/bin/env python3
r"""Does a word-processing decoration span a TAB, and how far exactly?

    make-tabprobe.py <outdir>

O71 says it does, and names `SwTabPortion::Paint` (`sw/source/core/text/txttab.cxx`:626-641),
which paints `Width()/GetTextSize(' ')` literal blanks in the current font whenever
`SwFont::IsPaintBlank()`.  Two things about that are NOT settled by reading it, and both change
what a reader has to implement:

  A. **How long is the rule?**  `nChar` blanks at their own advances would fall up to one space
     short of the stop; the tab's full width would not.  The call passes `bKern = true`, which
     `SwTextPaintInfo::DrawText_` turns into `aDrawInf.SetKern(rPor.Width())` and routes to
     `SwSubFont::DrawStretchText_` -> `OutputDevice::DrawStretchText(aPos, rInf.GetWidth(), ...)`
     (`inftxt.cxx`:798-808, `swfont.cxx`:1289-1345), so the blanks are STRETCHED onto the
     portion's own width and the answer should be the full tab.  Measured here rather than
     inferred.
  B. **What happens when the tab is narrower than one space?**  `nChar` is then 0, the string is
     empty and `DrawText_` returns on `!nLength` -- so no rule at all.  The Mono group below
     brackets that cliff on purpose: Liberation Mono at 10 pt is 120 twips per character, so tab
     widths of 30/90/119 twips must draw nothing and 121/180 must draw the tab.

and one control:

  C. **`IsWordLineMode()` turns the whole thing off** -- Word's `w:u w:val="words"`, RTF's `\ulw`,
     ODF's `style:text-*-mode="skip-white-space"`.  Corpus reach of that attribute is nil
     (`wordline-census.py`), so this group is the only witness there will ever be for it.

All three decorations, because the switch is
`(underline || overline || strikeout) && !IsWordLineMode()` (`fntcache.cxx`:106-109) and a probe
that only underlines cannot see the other two.
"""
import sys

FACES = ['Liberation Serif', 'Liberation Sans', 'Carlito', 'DejaVu Sans']
SIZES = [8, 10, 14]
KINDS = ['under', 'over', 'strike']
# Tab stops in inches, chosen so the residue of the tab width modulo one space width sweeps.
STOPS = [2.5, 3.517, 4.531, 5.549]

MONO_STOPS_TWIPS = [30, 90, 119, 121, 180, 361]   # against a 120-twip space
MONO_LABEL = 'MMMMMMMMMM'                          # 10 chars x 120 twips = 1200 twips

# D. Truncate or round?  Liberation Sans' space at 10 pt is 569/2048 x 10 pt = 55.566 twips, so a
# tab exactly 55 twips wide draws a rule under `floor` and none under `round`.  The paragraph opens
# WITH the tab, so the stop position IS the tab's width and no glyph advance has to be predicted.
CLIFF_TWIPS = [54, 55, 56, 57]

# E. The discriminator between the two REPRESENTATIONS, which is the question a fix gets wrong if it
# takes the witness at its word.  Model A: the blanks are stretched onto the tab's own width, so the
# rule reaches the stop.  Model B: `nChar` blanks are set at their own advances, so the rule stops
# `Width() mod nCharWidth` short of it.  Liberation Mono at 10 pt is 120 twips per blank, so these
# five tab widths leave residues of 80, 20, 100, 100 and 100 twips -- 4.0, 1.0, 5.0, 5.0 and 5.0 pt,
# against a reader that merges two rules at 0.5 pt.  The two models are several merge tolerances
# apart at four of the five, which is what makes this a measurement and not an inference.
RESID_TWIPS = [200, 260, 340, 460, 580]

DECOR = {
    'under': 'style:text-underline-style="solid" style:text-underline-width="auto" '
             'style:text-underline-color="font-color" style:text-underline-type="single"',
    'over': 'style:text-overline-style="solid" style:text-overline-width="auto" '
            'style:text-overline-color="font-color" style:text-overline-type="single"',
    'strike': 'style:text-line-through-style="solid" style:text-line-through-type="single"',
}
SKIP = {
    'under': ' style:text-underline-mode="skip-white-space"',
    'over': ' style:text-overline-mode="skip-white-space"',
    'strike': ' style:text-line-through-mode="skip-white-space"',
}

NS = (
 'xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" '
 'xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" '
 'xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" '
 'xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" '
 'xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" '
 'xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" '
 'xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"')


def rows():
    n = 0
    for face in FACES:
        for size in SIZES:
            for kind in KINDS:
                for stop in STOPS:
                    yield ('L%03d' % n, face, size, kind, '%gin' % stop, 'span', 0)
                    n += 1
    # C: the word-line-mode control, one per face and kind at 10 pt
    for face in FACES:
        for kind in KINDS:
            yield ('L%03d' % n, face, 10, kind, '%gin' % STOPS[1], 'skip', 0)
            n += 1
    # B: the sub-space cliff, in a monospaced face whose space is exactly 120 twips at 10 pt
    for tw in MONO_STOPS_TWIPS:
        yield ('L%03d' % n, 'Liberation Mono', 10, 'under',
               '%.6fin' % ((1200 + tw) / 1440.0), 'mono', tw)
        n += 1
    # D: floor against round, on a face whose space is not a whole twip
    for tw in CLIFF_TWIPS:
        yield ('L%03d' % n, 'Liberation Sans', 10, 'under', '%.6fin' % (tw / 1440.0), 'cliff', tw)
        n += 1
    # E: stretched onto the tab, or set at their own advances?
    for tw in RESID_TWIPS:
        yield ('L%03d' % n, 'Liberation Mono', 10, 'under', '%.6fin' % (tw / 1440.0), 'resid', tw)
        n += 1


ROWS = list(rows())


def fontfaces():
    faces = FACES + ['Liberation Mono']
    return ''.join('<style:font-face style:name="%s" svg:font-family="&apos;%s&apos;"/>' % (f, f)
                   for f in faces)


def styles():
    out = []
    for i, (label, face, size, kind, stop, group, _tw) in enumerate(ROWS):
        decor = DECOR[kind] + (SKIP[kind] if group == 'skip' else '')
        out.append(
            '<style:style style:name="T%d" style:family="text">'
            '<style:text-properties style:font-name="%s" fo:font-size="%dpt" %s '
            'style:font-name-asian="%s" style:font-size-asian="%dpt" '
            'style:font-name-complex="%s" style:font-size-complex="%dpt"/>'
            '</style:style>' % (i, face, size, decor, face, size, face, size))
        out.append(
            '<style:style style:name="P%d" style:family="paragraph">'
            '<style:paragraph-properties fo:margin-top="0cm" fo:margin-bottom="14pt">'
            '<style:tab-stops><style:tab-stop style:position="%s"/></style:tab-stops>'
            '</style:paragraph-properties>'
            '<style:text-properties style:font-name="%s" fo:font-size="%dpt"/>'
            '</style:style>' % (i, stop, face, size))
    return ''.join(out)


def body():
    out = []
    for i, (label, face, size, kind, stop, group, _tw) in enumerate(ROWS):
        left = MONO_LABEL if group == 'mono' else ('' if group in ('cliff', 'resid') else label)
        right = 'Q' if group == 'cliff' else ('Z' if group == 'resid' else 'R')
        out.append('<text:p text:style-name="P%d"><text:span text:style-name="T%d">'
                   '%s<text:tab/>%s</text:span></text:p>' % (i, i, left, right))
    return ''.join(out)


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    open(out + '/tabprobe.fodt', 'w', encoding='utf-8').write(
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        '<office:document %s office:version="1.3" '
        'office:mimetype="application/vnd.oasis.opendocument.text">'
        '<office:font-face-decls>%s</office:font-face-decls>'
        '<office:automatic-styles>%s</office:automatic-styles>'
        '<office:body><office:text>%s</office:text></office:body>'
        '</office:document>' % (NS, fontfaces(), styles(), body()))
    with open(out + '/tabprobe-manifest.tsv', 'w') as fh:
        fh.write('row\tlabel\tface\tsize\tkind\tstop\tgroup\ttabtwips\n')
        for i, r in enumerate(ROWS):
            fh.write('%d\t%s\t%s\t%d\t%s\t%s\t%s\t%d\n' % ((i,) + r))
    print('%d rows' % len(ROWS))


if __name__ == '__main__':
    main()
