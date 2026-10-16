#!/usr/bin/env python3
r"""Does a *Standard*-parented pool name still reach `Normal` when the entry states its own \fs?

`probes/rtf-bookmark-r88/genpool.py` establishes that `Body Text` and `caption` inherit the
document's own `Normal` when the entry states nothing. Both of the two corpus `.rtf` that apply
`Body Text` state an `\fs` **on the entry itself**, and round 87's trap says a style's own `\fs`
reaches no paragraph at all — `getDefaultSPRM` writes the reset back over it — so the two rules
meet here and only a measurement says which wins.

Six probes, three arms × the two `Normal` sizes that discriminate inheritance from a constant:

    plain   {\s7\sbasedon1392\snext0 Body Text;}          the entry states nothing
    own     {\s7\sbasedon1392\snext0\fs28 Body Text;}     the entry states its own size
    quote   {\s7\sbasedon1392\snext0\fs28 Quote;}         a name Writer has no style for

`\sbasedon1392` is a forward reference to an entry declared after it, so it does not resolve —
round 80's rule — and the pool parent is what is left.

  genstandard.py <outdir>

Then render through 26.2.4.2 and read with `probes/rtf-bookmark-r88/readpool.py`.
"""
import pathlib
import sys

OUT = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.')
OUT.mkdir(parents=True, exist_ok=True)

BASE = "{\\s1392\\sbasedon0\\snext1392\\f0\\fs18\\b Notes/Cautions Heading;}"

ARMS = {
    'plain': ('Body Text', ''),
    'own': ('Body Text', '\\fs28'),
    'caption-own': ('caption', '\\fs28'),
    'quote-own': ('Quote', '\\fs28'),
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


for key, (name, own) in ARMS.items():
    for normal in (20, 28):
        styles = ("{\\s0\\snext0\\f0\\fs%d Normal;}" % normal
                  + "{\\s7\\sbasedon1392\\snext0%s %s;}" % (own, name) + BASE)
        (OUT / f's_{key}_{normal}.rtf').write_text(doc(styles, three(7)), encoding='ascii')

print('written', 2 * len(ARMS), 'probes to', OUT)
