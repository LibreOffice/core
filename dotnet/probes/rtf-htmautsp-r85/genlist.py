#!/usr/bin/env python3
r"""Is it the list table's picture, or its list levels, that costs the word?

    python3 genlist.py /abs/outdir

`genhead.py` puts the flip between byte 178110 (before `{\*\listtable}`) and
260222 (after it).  These probes vary the list table alone, on the witness's
own bytes, with the word after it and one two-cell table row between the word
and the measured paragraphs.  `l-norow` repeats the deciding case without the
row, because without a row every one of these honours the word.
"""
import sys
import pathlib

WITNESS = "/home/user/corpus-odf/words/pagination-003/rtf/150-5370-10H.rtf"
ROW = ("\n" + r"\trowd\trql\trleft0\cellx4000\cellx8000"
       + r"\pard\plain\intbl\fs24 {a}\cell {b}\cell\row")
BODY = ("\n" + r"\pard\plain\fs24\sa480 {AAAA}\par" + "\n"
        + r"\pard\plain\fs24\sb480 {BBBB}\par" + "\n"
        + r"\pard\plain\fs24 {CCCC}\par" + "\n")
W = rb"\htmautsp"
LT, LT_END = 178110, 260222
LP, LP_END = 178123, 178999      # {\*\listpicture{\*\shppict{\pict …}}}
L1_END = 180078                  # the first {\list …}


def main(out: pathlib.Path, witness: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    d = witness.read_bytes()
    head = d[:LT]                                   # everything before the list table
    lt = d[LT:LT_END]
    cases = {
        "l-nolisttable": head,
        "l-full": head + lt,
        "l-pictonly": head + b"{\\*\\listtable" + d[LP:LP_END] + b"}",
        "l-nopict": head + b"{\\*\\listtable" + d[LP_END:LT_END],
        "l-onelist": head + b"{\\*\\listtable" + d[LP_END:L1_END] + b"}",
    }
    for name, pre in cases.items():
        (out / f"{name}.rtf").write_bytes(pre + W + ROW.encode() + BODY.encode() + b"}")
    (out / "l-norow.rtf").write_bytes(head + lt + W + BODY.encode() + b"}")
    print(f"wrote {len(cases) + 1} probes to {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]),
         pathlib.Path(sys.argv[2]) if len(sys.argv) > 2 else pathlib.Path(WITNESS))
