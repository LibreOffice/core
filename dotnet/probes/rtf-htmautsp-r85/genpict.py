#!/usr/bin/env python3
r"""Cut the witness either side of the picture in its first table cell.

    python3 genpict.py /abs/outdir [witness.rtf]

`150-5370-10H.rtf` states `\htmautsp` at byte 266199 and opens its body at
266229 with a table row whose first cell holds
`{\field{\fldrslt{\pict …}}}` -- a 44 KB WMF at 266968-311625.  Cut before
that picture and the word is honoured; cut after it and the word is lost.
Every probe is closed back to the document group, flushed with `\row` because
the cut lands inside a half-built row, and given three spaced paragraphs whose
A->B distance says which rule the reference applied:

    ~38 pt   the larger of two adjacent spacings wins -- the word was read
    ~62 pt   they add -- the word arrived after the settings table was sent

`c-beforerow` and `c-afterrow` are the controls whose answers are already
known from the coarse bisect, and they must reproduce it or the `\row` flush
is perturbing the measurement.
"""
import sys
import pathlib

WITNESS = "/home/user/corpus-odf/words/pagination-003/rtf/150-5370-10H.rtf"

LETTER = r"\paperw12240\paperh15840\margl1440\margr1440\margt1440\margb1440"
BODY = ("\n" + r"\pard\plain\fs24\sa480 {AAAA}\par" + "\n"
        + r"\pard\plain\fs24\sb480 {BBBB}\par" + "\n"
        + r"\pard\plain\fs24 {CCCC}\par" + "\n")


def depth_at(d: bytes, n: int) -> int:
    depth = i = 0
    while i < n:
        c = d[i]
        if c == 0x5C:
            i += 2 if i + 1 < n and d[i + 1] in b"\\{}'" else 1
            continue
        if c == 0x7B:
            depth += 1
        elif c == 0x7D:
            depth -= 1
        i += 1
    return depth


def probe(d: bytes, n: int) -> bytes:
    return (d[:n] + b"}" * (depth_at(d, n) - 1) + rb"\row " + rb"\pard\plain "
            + LETTER.encode() + BODY.encode() + b"}")


def main(out: pathlib.Path, witness: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    d = witness.read_bytes()
    cases = {
        "c-beforerow": 266229,    # the word, and nothing of the body yet
        "c-beforefield": 266680,  # inside the first cell, before the field
        "c-beforepict": 266968,   # inside the field result, before the picture
        "c-afterpict": 311626,    # the picture group closed
        "c-afterrow": 313259,     # the first row finished
    }
    for name, n in cases.items():
        (out / f"{name}.rtf").write_bytes(probe(d, n))
    print(f"wrote {len(cases)} probes to {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]),
         pathlib.Path(sys.argv[2]) if len(sys.argv) > 2 else pathlib.Path(WITNESS))
