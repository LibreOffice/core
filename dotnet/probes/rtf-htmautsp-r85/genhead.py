#!/usr/bin/env python3
r"""Which part of the witness's 266 KB preamble costs `\htmautsp` its effect
once the body opens with a table row.

    python3 genhead.py /abs/outdir

Each probe is `witness[0:N]`, closed back to the document group, then the word,
then one minimal two-cell table row, then the three spaced paragraphs.  The
same tail on a hand-built preamble honours the word (`genrow.py`'s
`b-cellrow`), so the smallest N that loses it names the piece of preamble that
matters.
"""
import sys
import pathlib

WITNESS = "/home/user/corpus-odf/words/pagination-003/rtf/150-5370-10H.rtf"
ROW = ("\n" + r"\trowd\trql\trleft0\cellx4000\cellx8000"
       + r"\pard\plain\intbl\fs24 {a}\cell {b}\cell\row")
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


def main(out: pathlib.Path, offsets: list[int], witness: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    d = witness.read_bytes()
    for n in offsets:
        head = d[:n] + b"}" * (depth_at(d, n) - 1)
        if rb"\htmautsp" not in head:
            head += rb"\htmautsp"
        (out / f"h{n:08d}.rtf").write_bytes(head + ROW.encode() + BODY.encode() + b"}")
    print(f"wrote {len(offsets)} probes to {out}")


if __name__ == "__main__":
    off = [int(x) for x in sys.argv[2:]] or [31, 2027, 2582, 178110, 260222, 263584,
                                             263693, 264659, 264960, 265349, 265481,
                                             265661, 265871, 266121, 266209, 266229]
    main(pathlib.Path(sys.argv[1]), off, pathlib.Path(WITNESS))
