#!/usr/bin/env python3
r"""What closes the `\htmautsp` window, on hand-built documents.

    python3 genwindow.py /abs/outdir

Every probe ends in the same three paragraphs.  A->B near 24 pt is the larger
of two adjacent spacings winning -- `\htmautsp` was read; near 48 pt is the two
adding -- the word was not.  What varies is where the word stands and what
stands between it and the paragraphs.
"""
import sys
import pathlib

HEAD = (r"{\rtf1\ansi\ansicpg1252\deff0{\fonttbl{\f0\froman\fcharset0 Liberation Serif;}}"
        r"{\stylesheet{\s0\snext0\ql Normal;}}"
        r"\paperw12240\paperh15840\margl1440\margr1440\margt1440\margb1440")
BODY = ("\n" + r"\pard\plain\fs24\sa480 {AAAA}\par" + "\n"
        + r"\pard\plain\fs24\sb480 {BBBB}\par" + "\n"
        + r"\pard\plain\fs24 {CCCC}\par" + "\n}")
W = r"\htmautsp"
PARA = "\n" + r"\pard\plain\fs24 {filler}\par"
ROW = ("\n" + r"\trowd\trql\trleft0\cellx4000\cellx8000"
       + r"\pard\plain\intbl\fs24 {one}\cell {two}\cell\row")
ROWDEF = "\n" + r"\trowd\trql\trleft0\cellx4000\cellx8000"
PICT = ("\n" + r"\pard\plain\fs24 {\pict\wmetafile8\picw100\pich100\picwgoal100\pichgoal100 "
        + "01000900000" + r"}\par")
FOOT = "\n" + r"{\footer\pard\plain\fs24 {foot}\par }"

CASES = {
    "w-absent":       HEAD + BODY,
    "w-early":        HEAD + W + BODY,
    "w-afterpara":    HEAD + PARA + W + BODY,
    "w-afterfooter":  HEAD + FOOT + W + BODY,
    "w-beforerow":    HEAD + W + ROW + BODY,
    "w-beforerowdef": HEAD + W + ROWDEF + BODY,
    "w-afterrow":     HEAD + ROW + W + BODY,
    "w-beforepict":   HEAD + W + PICT + BODY,
    "w-rowthenpara":  HEAD + W + ROW + PARA + BODY,
    "w-parathenrow":  HEAD + W + PARA + ROW + BODY,
}


def main(out: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    for name, text in CASES.items():
        (out / f"{name}.rtf").write_bytes(text.encode("latin-1"))
    print(f"wrote {len(CASES)} probes to {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]))
