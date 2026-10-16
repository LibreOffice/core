#!/usr/bin/env python3
r"""Which pictures close the window on a document setting.

    python3 genpicture.py /abs/outdir [witness.rtf]

`RTFDocumentImpl::resolvePict` ends with `checkFirstRun()`
(`sw/source/writerfilter/rtftok/rtfdocumentimpl.cxx`:1296), which is what sends the settings table
— so a picture written before a `\htmautsp` ought to make the word too late to read. Four probes
say which pictures count, and the answer is not "all of them":

    p-control        no `\htmautsp` at all               -> the spacings add
    p-none           `\htmautsp` and no picture          -> the larger wins
    p-bodypict       a bare `{\pict}` in a paragraph     -> the word is too late; they add
    p-listpicture    the witness's own
                     `{\*\listtable{\*\listpicture …}}`  -> the word still counts

The list-table group is lifted verbatim out of `150-5370-10H.rtf`, because that document is why
the question was asked: it opens its list table with an 876-byte PNG 88 KB before its
`\htmautsp`.
"""
import sys
import pathlib

WITNESS = "/home/user/corpus-odf/words/pagination-003/rtf/150-5370-10H.rtf"

HEAD = (r"{\rtf1\ansi\ansicpg1252\deff0{\fonttbl{\f0\froman\fcharset0 Liberation Serif;}}"
        r"{\stylesheet{\s0\snext0\ql Normal;}}")
LETTER = r"\paperw12240\paperh15840\margl1440\margr1440\margt1440\margb1440"
BODY = ("\n" + r"\pard\plain\sa480 {AAAA}\par" + "\n"
        + r"\pard\plain\sb480 {BBBB}\par" + "\n"
        + r"\pard\plain {CCCC}\par" + "\n}")


def group(text: str, start: int) -> str:
    """The balanced RTF group beginning at `start`."""
    depth, i = 0, start
    while i < len(text):
        c = text[i]
        if c == "\\":
            i += 2
            continue
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return text[start:i + 1]
        i += 1
    raise ValueError("unbalanced group")


def main(out: pathlib.Path, witness: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    text = witness.read_text("latin-1")
    picture = group(text, text.find(r"{\*\listpicture", text.find(r"{\*\listtable")))

    cases = {
        "p-control": HEAD + LETTER + BODY,
        "p-none": HEAD + LETTER + r"\htmautsp" + BODY,
        "p-bodypict": HEAD + LETTER + "\n"
                      + r"\pard\plain{\pict\pngblip\picw10\pich10\picwgoal180\pichgoal180"
                      + r" 89504e470d0a1a0a}\par" + r"\htmautsp" + BODY,
        "p-listpicture": HEAD + r"{\*\listtable" + picture + "}" + LETTER + r"\htmautsp" + BODY,
    }
    for name, body in cases.items():
        (out / f"{name}.rtf").write_text(body, encoding="latin-1")
    print(f"{len(cases)} probes in {out} ({len(picture)} bytes of list picture)")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]),
         pathlib.Path(sys.argv[2] if len(sys.argv) > 2 else WITNESS))
