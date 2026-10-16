#!/usr/bin/env python3
r"""The one probe arm this round could not reproduce, and where it comes from.

    python3 genresidual.py /abs/outdir

With the witness's own preamble, a list level stating `\super`, **no
`\htmautsp` anywhere**, and no table in the body, 26.2.4.2 collapses two
adjacent paragraph spacings where every other arrangement adds them.  Adding
one table row makes it add them again.  Whatever that is, it is not a question
about the word -- the arm has no word in it -- so this family varies how much
of the preamble stands, to say which part of it is involved.

    r-fonttbl    witness[0:2027]     the font table alone
    r-colortbl   witness[0:2582]     and the colour table
    r-styles     witness[0:178110]   and the 175 KB style sheet   <- the full head
    r-mini       a hand-built preamble                            <- adds, like everything else
"""
import sys
import pathlib

WITNESS = "/home/user/corpus-odf/words/pagination-003/rtf/150-5370-10H.rtf"
SUPER_LIST = (222652, 223886)
HEAD = (r"{\rtf1\ansi\ansicpg1252\deff0{\fonttbl{\f0\froman\fcharset0 Liberation Serif;}}"
        r"{\stylesheet{\s0\snext0\ql Normal;}}")
BODY = ("\n" + r"\pard\plain\fs24\sa480 {AAAA}\par" + "\n"
        + r"\pard\plain\fs24\sb480 {BBBB}\par" + "\n"
        + r"\pard\plain\fs24 {CCCC}\par" + "\n")
ROW = ("\n" + r"\trowd\trql\trleft0\cellx4000\cellx8000"
       + r"\pard\plain\intbl\fs24 {a}\cell {b}\cell\row")


def main(out: pathlib.Path, witness: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    d = witness.read_bytes()
    sup = b"{\\*\\listtable" + d[SUPER_LIST[0]:SUPER_LIST[1]] + b"}"
    heads = {
        "r-fonttbl": d[:2027],
        "r-colortbl": d[:2582],
        "r-styles": d[:178110],
        "r-mini": HEAD.encode("latin-1"),
    }
    for name, head in heads.items():
        (out / f"{name}.rtf").write_bytes(head + sup + BODY.encode() + b"}")
        (out / f"{name}-row.rtf").write_bytes(head + sup + ROW.encode() + BODY.encode() + b"}")
    print(f"wrote {2 * len(heads)} probes to {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]),
         pathlib.Path(sys.argv[2]) if len(sys.argv) > 2 else pathlib.Path(WITNESS))
