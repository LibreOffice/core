#!/usr/bin/env python3
r"""Does a heading-parented style inherit the document's own `Normal`, and in which properties?

`RtfPoolParent.Heading` has been a *constant* in this tree since round 87: `FormattingOf` folds
`HeadingPool`'s four values in and stops the walk, so a document's own `Normal` stating bold, a
colour or an alignment never reaches `heading 1`..`heading 9`, `Title` or `Subtitle`. Rounds 87
and 95 both modelled it that way and neither measured it. `COLL_HEADLINE_BASE`'s own pool parent
*is* `COLL_STANDARD` (`sw/source/core/doc/poolfmt.cxx`:279-289, verified line for line), so the
structural claim says the walk should continue -- but structure is not reach: the intermediate
states a font, a size, an upper/lower space and keep-with-next of its own
(`DocumentStylePoolManager.cxx`:769-820), and every property it states shadows `Normal`.

So the question is not *whether* the chain reaches `Standard` -- `readfodt.py` prints the chain and
already shows `Heading_20_4>Heading>Standard` -- but *which properties survive the intermediate*.
That is what these probes separate, one property class per arm so nothing is confounded:

    plain   {\s0\snext0\f0\fs20 Normal;}                 the baseline
    chars   ...\fs20\b\i\cf1                             bold, italic, colour: none stated by
                                                         `COLL_HEADLINE_BASE`
    paras   ...\fs20\qc\li720\sb400                      alignment and left indent: not stated;
                                                         space-before: stated (PT_12)
    size    ...\fs36                                     18 pt against the intermediate's PT_14
    marks   ...\fs20\ul\strike\caps                       underline, strike-through and capitals:
                                                         none stated by the intermediate either

`\fs36` rather than `\fs28`, because `PT_14` *is* `\fs28` and a probe that cannot tell the two
apart measures nothing.

Each name's entry carries a forward `\sbasedon1392` that cannot resolve (round 80's rule), which is
the condition under which the pool parent decides anything -- and the same condition
`FormattingOf`'s `switch` requires.

  genheading.py <outdir>
"""
import pathlib
import sys

OUT = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.')
OUT.mkdir(parents=True, exist_ok=True)

BASE = "{\\s1392\\sbasedon0\\snext1392\\f0\\fs18\\b Notes/Cautions Heading;}"

NAMES = {
    'h1': 'heading 1',
    'h2': 'heading 2',
    'h3': 'heading 3',
    'h4': 'heading 4',
    'h5': 'heading 5',
    'h6': 'heading 6',
    'h7': 'heading 7',
    'h8': 'heading 8',
    'h9': 'heading 9',
    'H1upper': 'Heading 1',
    'title': 'Title',
    'subtitle': 'Subtitle',
    # controls
    'bodytext': 'Body Text',      # `Standard`-parented: round 95's answer, must inherit
    'quote': 'Quote',             # no Writer style at all: must inherit nothing
    'headingbare': 'Heading',     # *is* `COLL_HEADLINE_BASE`, so reset -- round 97's trap
}

ARMS = {
    'plain': '\\f0\\fs20',
    'chars': '\\f0\\fs20\\b\\i\\cf1',
    'paras': '\\f0\\fs20\\qc\\li720\\sb400',
    'size': '\\f0\\fs36',
    'marks': '\\f0\\fs20\\ul\\strike\\caps',
}


def doc(styles, body):
    return ("{\\rtf1\\ansi\\ansicpg1252\\deff0\n"
            "{\\fonttbl{\\f0\\froman\\fcharset0 Liberation Serif;}}\n"
            "{\\colortbl;\\red255\\green0\\blue0;}\n"
            "{\\stylesheet" + styles + "}\n"
            "\\paperw12240\\paperh15840\\margl1440\\margr1440\\margt1440\\margb1440\n"
            + body + "}\n")


def three(sid):
    return ("\\pard\\plain \\fs20{\nAAA}\\par\n"
            "\\pard\\plain \\s%d{\nHEAD}\\par\n" % sid +
            "\\pard\\plain \\fs20{\nBBB}\\par\n")


count = 0
for key, name in NAMES.items():
    for arm, normal in ARMS.items():
        styles = ("{\\s0\\snext0%s Normal;}" % normal
                  + "{\\s7\\sbasedon1392\\snext0 %s;}" % name + BASE)
        (OUT / f'p_{key}_{arm}.rtf').write_text(doc(styles, three(7)), encoding='ascii')
        count += 1

print('written', count, 'probes to', OUT)
