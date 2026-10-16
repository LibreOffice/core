#!/usr/bin/env python3
r"""`\super` inside a list level is what sends the settings table early.

    python3 gensuper.py /abs/outdir

The 39th `{\list}` of `150-5370-10H.rtf` opens its first level with

    {\listlevel …{\leveltext \'01\'00;}{\levelnumbers\'01;}\super\rtlch…}

and `RTFDocumentImpl::dispatchFlag`'s `RTFKeyword::SUPER` case calls
`checkFirstRun()` for anything that is not a style-sheet import
(`sw/source/writerfilter/rtftok/rtfdispatchflag.cxx`:891-895).  A list level is
not one, so the settings table goes out at byte 222750 and the `\htmautsp` at
266199 is never in it.

Each probe keeps exactly one list, with and without that one control word, and
with and without a table row after the word, on the witness's own preamble and
on a hand-built one.
"""
import sys
import pathlib

WITNESS = "/home/user/corpus-odf/words/pagination-003/rtf/150-5370-10H.rtf"
LT = 178110
SUPER_LIST = (222652, 223886)     # the 39th {\list …}
PLAIN_LIST = (221536, 222651)     # the 38th, identical in shape and without \super

HEAD = (r"{\rtf1\ansi\ansicpg1252\deff0{\fonttbl{\f0\froman\fcharset0 Liberation Serif;}}"
        r"{\stylesheet{\s0\snext0\ql Normal;}}"
        r"\paperw12240\paperh15840\margl1440\margr1440\margt1440\margb1440")
ROW = ("\n" + r"\trowd\trql\trleft0\cellx4000\cellx8000"
       + r"\pard\plain\intbl\fs24 {a}\cell {b}\cell\row")
BODY = ("\n" + r"\pard\plain\fs24\sa480 {AAAA}\par" + "\n"
        + r"\pard\plain\fs24\sb480 {BBBB}\par" + "\n"
        + r"\pard\plain\fs24 {CCCC}\par" + "\n")
W = rb"\htmautsp"
MINI = (r"{\*\listtable{\list\listtemplateid1{\listlevel\levelnfc0\leveljc0\levelstartat1"
        r"\levelfollow0{\leveltext \'01\'00;}{\levelnumbers\'01;}%s\fi-360\li540}"
        r"\listid1}}")


def main(out: pathlib.Path, witness: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    d = witness.read_bytes()
    head = d[:LT]
    sup = d[SUPER_LIST[0]:SUPER_LIST[1]]
    plain = d[PLAIN_LIST[0]:PLAIN_LIST[1]]
    cases = {
        "x-super-row": head + b"{\\*\\listtable" + sup + b"}" + W + ROW.encode(),
        "x-super-norow": head + b"{\\*\\listtable" + sup + b"}" + W,
        "x-nosuper-row": head + b"{\\*\\listtable" + sup.replace(rb"\super", b"") + b"}" + W + ROW.encode(),
        "x-nosuper-norow": head + b"{\\*\\listtable" + sup.replace(rb"\super", b"") + b"}" + W,
        "x-plain-row": head + b"{\\*\\listtable" + plain + b"}" + W + ROW.encode(),
        "m-super-row": (HEAD + MINI % r"\super").encode("latin-1") + W + ROW.encode(),
        "m-super-norow": (HEAD + MINI % r"\super").encode("latin-1") + W,
        "m-nosuper-row": (HEAD + MINI % "").encode("latin-1") + W + ROW.encode(),
        "m-nosuper-norow": (HEAD + MINI % "").encode("latin-1") + W,
        # The two places a `\super` does not count: a group nothing is dispatched
        # inside, and a style-sheet entry the caller excludes by name.
        "m-superheader": (HEAD + r"{\header\pard\plain\fs24\super H\par }").encode("latin-1")
                         + W + ROW.encode(),
        "m-superstyle": (HEAD + r"{\stylesheet{\s0\snext0\ql Normal;}{\s1\super Note;}}")
                        .encode("latin-1") + W + ROW.encode(),
    }
    for name, pre in cases.items():
        (out / f"{name}.rtf").write_bytes(pre + BODY.encode() + b"}")
        # Its own control: the same document with the word removed.  A case is
        # only evidence about `\htmautsp` where it differs from this twin.
        (out / f"{name}-noword.rtf").write_bytes(pre.replace(W, b"") + BODY.encode() + b"}")
    print(f"wrote {2 * len(cases)} probes to {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]),
         pathlib.Path(sys.argv[2]) if len(sys.argv) > 2 else pathlib.Path(WITNESS))
