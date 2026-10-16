#!/usr/bin/env python3
r"""A bare `\row` after `\htmautsp` costs the word; a well-formed row does not.

    python3 genrow.py /abs/outdir

Cutting `150-5370-10H.rtf` at the byte before its first `\trowd` and appending
`\row` loses the word, while the same cut without the `\row` keeps it.  These
probes ask what about a row does that, on documents small enough to read.
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
PIC = (r"{\pict\wmetafile8\picw1000\pich1000\picwgoal1000\pichgoal1000 "
       + "01000900" + "00" * 40 + r"}")

CASES = {
    "b-barerow":    HEAD + W + "\n" + r"\row" + BODY,
    "b-defrow":     HEAD + W + "\n" + r"\trowd\trql\trleft0\cellx4000\row" + BODY,
    "b-cellrow":    HEAD + W + "\n" + r"\trowd\trql\trleft0\cellx4000\pard\plain\intbl\fs24 {x}\cell\row" + BODY,
    "b-pictcell":   HEAD + W + "\n" + r"\trowd\trql\trleft0\cellx4000\pard\plain\intbl\fs24 " + PIC + r"\cell\row" + BODY,
    "b-fieldcell":  HEAD + W + "\n" + r"\trowd\trql\trleft0\cellx4000\pard\plain\intbl\fs24 "
                    + r"{\field{\*\fldinst {IMPORT x \\* mergeformat}}{\fldrslt\plain {" + PIC + r"}}}\cell\row" + BODY,
    "b-fieldpara":  HEAD + W + "\n" + r"\pard\plain\fs24 "
                    + r"{\field{\*\fldinst {IMPORT x \\* mergeformat}}{\fldrslt\plain {" + PIC + r"}}}\par" + BODY,
    "b-picpara":    HEAD + W + "\n" + r"\pard\plain\fs24 " + PIC + r"\par" + BODY,
}


def main(out: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    for name, text in CASES.items():
        (out / f"{name}.rtf").write_bytes(text.encode("latin-1"))
    print(f"wrote {len(CASES)} probes to {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]))
