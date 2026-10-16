#!/usr/bin/env python3
r"""The own-against-inherited matrix for a paragraph style's *character* formatting.

    python3 genchar.py /abs/outdir

The paragraph half of this round found that a style's own paragraph statements are replaced by
RTF's defaults where `getDefaultSPRM` has one. That function has a character arm as well
(`rtfsprm.cxx`:156-176: `sz` and `szCs` -> 24, `b` and `i` -> 0, the underline -> none, the three
`rFonts` slots -> Times New Roman, the colour -> 0), so this asks the same question of it.

Each probe holds one paragraph, `BBBB Hamburgefonstiv`, whose measurable properties are its
height, its width and its ink; `own-P` names the style that states P and `inh-P` names an empty
child of it.
"""
import sys
import pathlib

FONTS = (r"{\fonttbl{\f0\froman\fcharset0 Liberation Serif;}"
         r"{\f1\fswiss\fcharset0 Liberation Sans;}"
         r"{\f2\fmodern\fcharset0 Liberation Mono;}}")
LETTER = r"\paperw12240\paperh15840\margl1440\margr1440\margt1440\margb1440"
NORMAL = r"{\s0\snext0\ql Normal;}"

PROPS = {
    "fs36": r"\fs36",
    "f1": r"\f1",
    "f2": r"\f2",
    "b": r"\b",
    "i": r"\i",
    "ul": r"\ul",
    "caps": r"\caps",
}


def doc(sheet: str, use: str) -> str:
    return (r"{\rtf1\ansi\ansicpg1252\deff0" + FONTS
            + r"{\stylesheet" + sheet + "}" + LETTER + "\n"
            + r"\pard\plain " + use + r"{BBBB Hamburgefonstiv}\par" + "\n}")


def main(out: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    for name, words in PROPS.items():
        parent = NORMAL + r"{\s10\sbasedon0\snext10" + words + r" Foo;}"
        (out / f"own-{name}.rtf").write_text(doc(parent, r"\s10 "), encoding="latin-1")
        (out / f"inh-{name}.rtf").write_text(
            doc(parent + r"{\s11\sbasedon10\snext11 Bar;}", r"\s11 "), encoding="latin-1")
        zero = r"{\s0\snext0\ql" + words + r" Normal;}"
        (out / f"zero-{name}.rtf").write_text(doc(zero, ""), encoding="latin-1")
        (out / f"s0-{name}.rtf").write_text(
            doc(zero + r"{\s10\sbasedon0\snext10 Foo;}", r"\s10 "), encoding="latin-1")
    (out / "control-none.rtf").write_text(doc(NORMAL, ""), encoding="latin-1")
    print(f"{4 * len(PROPS) + 1} probes in {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]))
