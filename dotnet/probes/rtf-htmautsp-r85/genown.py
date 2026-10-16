#!/usr/bin/env python3
r"""Does the witness's *own* `\htmautsp` survive?  Cut the document at N and
append nothing but three spaced paragraphs.

    python3 genown.py /abs/outdir <offset> [<offset> ...]

This is the companion to `genprefix.py` and the two ask different questions.
`genprefix.py` appends a word of its own, so it measures whether the settings
window is still OPEN after N bytes.  This one appends no word, so it measures
whether the word the author wrote at byte 266199 was READ.  Confusing the two
is what made round 80 conclude that the trigger sits after the word.
"""
import sys
import pathlib

WITNESS = "/home/user/corpus-odf/words/pagination-003/rtf/150-5370-10H.rtf"
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
        (out / f"o{n:08d}.rtf").write_bytes(
            d[:n] + b"}" * (depth_at(d, n) - 1) + BODY.encode() + b"}")
    print(f"wrote {len(offsets)} probes to {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]), [int(x) for x in sys.argv[2:]], pathlib.Path(WITNESS))
