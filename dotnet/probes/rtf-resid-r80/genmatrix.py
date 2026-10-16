#!/usr/bin/env python3
r"""The own-against-inherited matrix for a paragraph style's *paragraph* formatting.

    python3 genmatrix.py /abs/outdir

For each property, two probes:

    own-P    the paragraph names `\s10`, which states P
    inh-P    the paragraph names `\s11`, which states nothing and is `\sbasedon10`

Every probe is one page holding

    AAAA    a plain paragraph
    BBBB…   the paragraph under test, three lines of it so an indent, a hanging indent,
            a right indent and a line spacing are all readable
    CCCC    a plain paragraph

so one rendering answers alignment (BBBB's x), space before (AAAA -> BBBB), space after
(BBBB's last line -> CCCC), the left indent (BBBB line 2's x), the first-line indent
(line 1's x against line 2's), the right indent (BBBB's widest right edge) and the line
spacing (line 1 -> line 2).
"""
import sys
import pathlib

FONTS = r"{\fonttbl{\f0\froman\fcharset0 Liberation Serif;}}"
LETTER = r"\paperw12240\paperh15840\margl1440\margr1440\margt1440\margb1440"
NORMAL = r"{\s0\snext0\ql Normal;}"

LONG = ("BBBB " + " ".join(["quick brown foxes jump over the lazy dog and then some more"] * 4))

PROPS = {
    "qc": r"\qc",
    "qr": r"\qr",
    "qj": r"\qj",
    "sb480": r"\sb480",
    "sa480": r"\sa480",
    "li720": r"\li720\lin720",
    "ri1440": r"\ri1440\rin1440",
    "fihang": r"\li720\lin720\fi-360",
    "fiindent": r"\fi720",
    "sl360": r"\sl360\slmult0",
    "sl240mult": r"\sl240\slmult1",
    "keepn": r"\keepn",
    "tx2880": r"\tx2880",
}


def doc(sheet: str, use: str) -> str:
    return (r"{\rtf1\ansi\ansicpg1252\deff0" + FONTS
            + r"{\stylesheet" + sheet + "}" + LETTER + "\n"
            + r"\pard\plain {AAAA}\par" + "\n"
            + r"\pard\plain " + use + "{" + LONG + r"}\par" + "\n"
            + r"\pard\plain {CCCC}\par" + "\n}")


def main(out: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    n = 0
    for name, words in PROPS.items():
        parent = NORMAL + r"{\s10\sbasedon0\snext10" + words + r" Foo;}"
        (out / f"own-{name}.rtf").write_text(doc(parent, r"\s10 "), encoding="latin-1")
        (out / f"inh-{name}.rtf").write_text(
            doc(parent + r"{\s11\sbasedon10\snext11 Bar;}", r"\s11 "), encoding="latin-1")

        # The same property on style zero, which is the floor under every paragraph:
        # `zero-` names no style at all, `s0-` names an empty style based on zero.
        zero = r"{\s0\snext0\ql" + words + r" Normal;}"
        (out / f"zero-{name}.rtf").write_text(doc(zero, ""), encoding="latin-1")
        (out / f"s0-{name}.rtf").write_text(
            doc(zero + r"{\s10\sbasedon0\snext10 Foo;}", r"\s10 "), encoding="latin-1")
        n += 4
    (out / "control-none.rtf").write_text(doc(NORMAL, ""), encoding="latin-1")
    print(f"{n + 1} probes in {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]))
