#!/usr/bin/env python3
r"""What an RTF style name reaches when Writer already has a style of that name — and what the
*intermediate* pool style between it and `COLL_STANDARD` contributes on the way.

`probes/words-close-r95/genstandard.py` settles the one-hop case: `Body Text` and `caption` have
`COLL_STANDARD` for a pool parent, so a paragraph in them takes the document's own `Normal`, and
the entry's own `\fs` reaches nothing. This asks the same question of the names whose pool parent
is *not* `COLL_STANDARD` directly:

    header, footer   ->  Header, Footer          COLL_HEADER/COLL_FOOTER -> COLL_HEADERFOOTER
    toc 1..toc 3     ->  Contents 1..3           COLL_TOX_CNTNT1.. -> COLL_REGISTER_BASE
    Figure           ->  Figure                  COLL_LABEL_FIGURE -> COLL_LABEL
    Heading          ->  Heading                 COLL_HEADLINE_BASE -> COLL_STANDARD

`Figure` and `Heading` are in `ConvertStyleName`'s map for *neither* spelling, so what reaches
Writer's own style is `xStyles->hasByName` on the name as written
(`StyleSheetTable.cxx`:1099-1101). The intermediates are separate style objects, so
`SetPropertiesToDefault` (`:305-331`, called at `:1111`) cannot reach them: whatever
`COLL_HEADERFOOTER`, `COLL_REGISTER_BASE` and `COLL_LABEL` state for themselves survives the
import and is inherited.

Each name is written in three arms x the two `Normal` sizes that tell inheritance from a constant:

    plain   {\s7\sbasedon1392\snext0 <NAME>;}          the entry states nothing
    own     {\s7\sbasedon1392\snext0\fs28 <NAME>;}     the entry states its own size
    tab     as `plain`, with a \tab in the paragraph   so an inherited tab stop is visible

`\sbasedon1392` is a forward reference to an entry declared *after* it, so it does not resolve
(round 80's rule) and the pool parent is what is left.

  genpool2.py <outdir>
"""
import pathlib
import sys

OUT = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.')
OUT.mkdir(parents=True, exist_ok=True)

BASE = "{\\s1392\\sbasedon0\\snext1392\\f0\\fs18\\b Notes/Cautions Heading;}"

NAMES = {
    'header': 'header',
    'footer': 'footer',
    'toc1': 'toc 1',
    'toc2': 'toc 2',
    'toc3': 'toc 3',
    'figure': 'Figure',
    'headingbare': 'Heading',
    # controls, both already settled by round 95
    'bodytext': 'Body Text',
    'quote': 'Quote',
    'heading4': 'heading 4',
}


def doc(styles, body):
    return ("{\\rtf1\\ansi\\ansicpg1252\\deff0\n"
            "{\\fonttbl{\\f0\\froman\\fcharset0 Liberation Serif;}}\n"
            "{\\stylesheet" + styles + "}\n"
            "\\paperw12240\\paperh15840\\margl1440\\margr1440\\margt1440\\margb1440\n"
            + body + "}\n")


def three(sid, tab=False):
    mid = "\\tab HEAD\\tab TAIL" if tab else "\nHEAD"
    return ("\\pard\\plain \\fs20{\nAAA}\\par\n"
            "\\pard\\plain \\s%d{%s}\\par\n" % (sid, mid) +
            "\\pard\\plain \\fs20{\nBBB}\\par\n")


count = 0
for key, name in NAMES.items():
    for arm, own, tab in (('plain', '', False), ('own', '\\fs28', False), ('tab', '', True)):
        for normal in (20, 28):
            styles = ("{\\s0\\snext0\\f0\\fs%d Normal;}" % normal
                      + "{\\s7\\sbasedon1392\\snext0%s %s;}" % (own, name) + BASE)
            (OUT / f'p_{key}_{arm}_{normal}.rtf').write_text(
                doc(styles, three(7, tab)), encoding='ascii')
            count += 1

print('written', count, 'probes to', OUT)
