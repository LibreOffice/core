#!/usr/bin/env python3
"""The six-arm character-width fixture, as RTF, to be converted to `.doc` by the reference.

The same six arms as `tests/corpus/features/odt-text-scale.fodt`, so the four word-processing
readers can be asserted side by side and a disagreement between them is visible in one place:
absent, a stated 100, 99, 60, 130, and an unscaled paragraph holding a scaled span.

EVERY ARM DRAWS THE SAME WORD, so the ratios are exact rather than a per-character proxy. Round
148's first cut of the ODF fixture used six different words and had to assert at 6 % where this
asserts at 0.05 %.

`\\charscalex` is the RTF spelling and `sprmCCharScale` the WW8 one; the `.doc` is the reference's
own conversion of this file, and `dump-chpx.py` in `probes/charscale-r149/` is what verifies the
sprm survived it -- a fixture converted and not checked measures nothing and reports agreement.
"""
import pathlib

WORD = 'Hamburgefonstiv'

# \qr so the drawn width is readable from the line's own origin: the right text edge is fixed, so
# the origin is `edge - width` and every fixed term cancels. Glyph boxes are quantised to whole
# thousandths of an em and `dotnet/CLAUDE.md` records four rounds lost to measuring through them.
ARMS = [
    ('',              WORD),                      # absent
    (r'\charscalex100', WORD),                    # stated identity
    (r'\charscalex99',  WORD),                    # the corpus's commonest value
    (r'\charscalex60',  WORD),                    # squeezed
    (r'\charscalex130', WORD),                    # stretched
]


def main():
    body = []
    for control, text in ARMS:
        body.append(r'\pard\qr\plain\f0\fs24' + control + ' ' + text + r'\par')

    # The uniform-paragraph shortcut's arm: two runs of the same word, the second scaled, in a
    # paragraph that states nothing. A reader that folds the run list away measures both at the
    # paragraph's width.
    body.append(r'\pard\ql\plain\f0\fs24 ' + WORD + r' {\charscalex60 ' + WORD + r'}\par')

    out = pathlib.Path(__file__).with_name('words-char-scale.rtf')
    out.write_text(
        r'{\rtf1\ansi\deff0{\fonttbl{\f0\froman Liberation Serif;}}'
        r'\paperw11906\paperh16838\margl1134\margr1134\margt1134\margb1134'
        + ''.join(body) + '}')
    print('wrote %s (%d bytes)' % (out, out.stat().st_size))


if __name__ == '__main__':
    main()
