#!/usr/bin/env python3
r"""One minimal `.rtf` per `\charscalex` value, each a single RIGHT-ALIGNED line.

Right-aligned deliberately: the PDF writer then states the line's origin as `margin − width(line)`,
so differencing two arms cancels every fixed term and the drawn advance is read out of the text
positioning operator rather than reconstructed from glyph positions, which are quantised to whole
thousandths of an em (`dotnet/CLAUDE.md`: four rounds lost to that channel).

One control word separates the arms and nothing else differs, so an arm that does not move is
evidence about that control word alone.
"""
import pathlib
import sys

DOC = (r'{\rtf1\ansi\deff0'
       r'{\fonttbl{\f0\froman Liberation Serif;}}'
       r'\paperw11906\paperh16838\margl1134\margr1134\margt1134\margb1134'
       r'\pard\qr\plain\f0\fs24{scale} Hamburgefonstiv\par'
       '}\n')

ARMS = {
    'none': '',
    's100': r'\charscalex100',
    's60': r'\charscalex60',
    's99': r'\charscalex99',
    's130': r'\charscalex130',
    # Out of `ST_TextScale`'s 1..600: the C++ importer replaces both with 100.
    's0': r'\charscalex0',
    's900': r'\charscalex900',
    'sneg': r'\charscalex-50',
    # A bare control word: the tokeniser's own default value for `charscalex` is 100
    # (`rtftokenizer.cxx`:253), so this must draw as the unscaled arm.
    'sbare': r'\charscalex',
}


def main():
    outdir = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'rtf-fixtures')
    outdir.mkdir(parents=True, exist_ok=True)
    for name, scale in ARMS.items():
        path = outdir / f'{name}.rtf'
        path.write_text(DOC.replace('{scale}', scale), encoding='ascii')
        print(path, path.stat().st_size)


if __name__ == '__main__':
    main()
