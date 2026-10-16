#!/usr/bin/env python3
r"""What Writer's pool gives four more style names whose \sbasedon does not resolve.

Round 87 closed `heading 1`..`heading 9`, whose pool parent is Writer's *Heading*
(COLL_HEADLINE_BASE: 14 pt, 12 pt above, 6 pt below, keep-with-next), and left four
names measured-but-not-implemented: Body Text, caption, Title and Subtitle.

The four are not one case. Reading the pool rather than the brief's four numbers:

  Title     -> COLL_DOC_TITLE     28 pt bold centred, parent COLL_HEADLINE_BASE
  Subtitle  -> COLL_DOC_SUBTITLE  18 pt centred,      parent COLL_HEADLINE_BASE
  caption   -> COLL_LABEL         10 pt italic 6/6,   parent COLL_STANDARD
  Body Text -> COLL_TEXT          no size, 0/7, 115%, parent COLL_STANDARD

and the entry's *own* properties are reset by the import, so what survives is the
parent's.  So Title and Subtitle should answer Heading's 14 pt exactly as the nine
headings do, while caption and Body Text should answer whatever Standard answers.

The discriminator is the Normal entry's own size: each probe is written twice, with
`Normal` at \fs20 (10 pt) and at \fs28 (14 pt).  A name that tracks it inherits from
Standard; a name that does not is holding a pool value of its own.

  genpool.py <outdir>
"""
import pathlib
import sys

OUT = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.')
OUT.mkdir(parents=True, exist_ok=True)

# The style the entries name and which is declared *after* them, so \sbasedon does not
# resolve -- round 80's forward reference.
BASE = "{\\s1392\\sbasedon0\\snext1392\\f0\\fs18\\b Notes/Cautions Heading;}"

NAMES = {
    'h4': 'heading 4',
    'title': 'Title',
    'subtitle': 'Subtitle',
    'bodytext': 'Body Text',
    'caption': 'caption',
    'quote': 'Quote',
    'widget': 'Widget Heading',
}


def doc(styles, body):
    return ("{\\rtf1\\ansi\\ansicpg1252\\deff0\n"
            "{\\fonttbl{\\f0\\froman\\fcharset0 Liberation Serif;}}\n"
            "{\\stylesheet" + styles + "}\n"
            "\\paperw12240\\paperh15840\\margl1440\\margr1440\\margt1440\\margb1440\n"
            + body + "}\n")


def three(sid):
    return ("\\pard\\plain \\fs20{\nAAA}\\par\n"
            "\\pard\\plain \\s%d{\nHEAD}\\par\n" % sid +
            "\\pard\\plain \\fs20{\nBBB}\\par\n")


for key, name in NAMES.items():
    for normal in (20, 28):
        styles = ("{\\s0\\snext0\\f0\\fs%d Normal;}" % normal
                  + "{\\s7\\sbasedon1392\\snext0 %s;}" % name + BASE)
        (OUT / f'p_{key}_{normal}.rtf').write_text(doc(styles, three(7)), encoding='ascii')

print('written', 2 * len(NAMES), 'probes to', OUT)
