#!/usr/bin/env python3
r"""Shrink the difference between the witness cut that honours `\htmautsp` and
the one that does not.

    python3 gendel.py /abs/outdir

`150-5370-10H.rtf[0:266229]` honours the word; `[0:313259]` -- the same bytes
plus the document's first table row -- does not.  Each probe here is the
second cut with one piece of that row removed, so the piece whose removal
brings the word back is the piece that costs it.

    266229..266680   the row and cell definitions and the cell's paragraph
    266680..311632   the whole `{\field}` group
    266968..311625   the 44 KB `{\pict}` inside the field's result
"""
import sys
import pathlib

WITNESS = "/home/user/corpus-odf/words/pagination-003/rtf/150-5370-10H.rtf"
BODY = ("\n" + r"\pard\plain\fs24\sa480 {AAAA}\par" + "\n"
        + r"\pard\plain\fs24\sb480 {BBBB}\par" + "\n"
        + r"\pard\plain\fs24 {CCCC}\par" + "\n")
CUT = 313259


def main(out: pathlib.Path, witness: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    d = witness.read_bytes()
    row = d[:CUT]
    cases = {
        "d-none": row,                                   # control: ignores the word
        "d-nopict": row[:266968] + row[311625:],         # the picture removed
        "d-nofield": row[:266680] + row[311632:],        # the whole field removed
        "d-nocell": d[:266229] + rb"\trowd\trql\trleft0\cellx5939\cellx9475"
                    + rb"\pard\plain\intbl\fs24 {a}\cell {b}\cell\row",
        "d-empty": d[:266229],                           # control: honours the word
    }
    for name, body in cases.items():
        (out / f"{name}.rtf").write_bytes(body + BODY.encode() + b"}")
    print(f"wrote {len(cases)} probes to {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]),
         pathlib.Path(sys.argv[2]) if len(sys.argv) > 2 else pathlib.Path(WITNESS))
