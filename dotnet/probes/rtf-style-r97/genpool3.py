#!/usr/bin/env python3
r"""The `COLL_LABEL` family, and what happens when the document redefines the intermediate.

`genpool2.py` establishes that `Figure` inherits Writer's `Caption` — italic, 12 pt, 6 pt above
and below — because `SetPropertiesToDefault` reaches only the style the entry *matched*
(`StyleSheetTable.cxx`:1111) and `Caption` is a different object. Two things follow that this
asks about:

  * `Figure` is not alone. `Text`, `Illustration`, `Table` and `Drawing` are the other four
    `COLL_LABEL_*` names (`poolfmt.cxx`:248-252), and `Text` is applied by a corpus document.
  * If the *same document* also declares a style whose converted name is `Caption`, then the
    intermediate is reset and reloaded with that entry's own properties, and `Figure` inherits
    those instead. Both corpus documents that apply a `COLL_LABEL_*` name do exactly that.

Arms, each x the two `Normal` sizes:

    plain     {\s7\sbasedon1392\snext0 <NAME>;}                    nothing else declared
    withcap   the same, plus {\s8\sbasedon0\snext0\fs20\b caption;} the document redefines it

  genpool3.py <outdir>
"""
import pathlib
import sys

OUT = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.')
OUT.mkdir(parents=True, exist_ok=True)

BASE = "{\\s1392\\sbasedon0\\snext1392\\f0\\fs18\\b Notes/Cautions Heading;}"
CAPTION = "{\\s8\\sbasedon0\\snext0\\f0\\fs20\\b caption;}"

NAMES = {
    'text': 'Text',
    'illustration': 'Illustration',
    'tablelabel': 'Table',
    'drawing': 'Drawing',
    'comment': 'Comment',
    'figure': 'Figure',
    'headercap': 'Header',
    'footercap': 'Footer',
    'tocupper': 'TOC 1',
    'index1': 'Index 1',
    'signature': 'Signature',
    'marginalia': 'Marginalia',
    'listindent': 'List Indent',
    'bodyindent': 'Text body indent',
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


count = 0
for key, name in NAMES.items():
    for arm, extra in (('plain', ''), ('withcap', CAPTION)):
        for normal in (20, 28):
            styles = ("{\\s0\\snext0\\f0\\fs%d Normal;}" % normal
                      + "{\\s7\\sbasedon1392\\snext0 %s;}" % name + BASE + extra)
            (OUT / f'p_{key}_{arm}_{normal}.rtf').write_text(doc(styles, three(7)),
                                                             encoding='ascii')
            count += 1

print('written', count, 'probes to', OUT)
