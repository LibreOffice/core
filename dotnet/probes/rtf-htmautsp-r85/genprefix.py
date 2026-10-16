#!/usr/bin/env python3
r"""Bisect a real document to find what closes the `\htmautsp` window.

    python3 genprefix.py /abs/outdir [witness.rtf] [byte-offsets...]

`RTFDocumentImpl::checkFirstRun` calls `outputSettingsTable` exactly once
(`sw/source/writerfilter/rtftok/rtfdocumentimpl.cxx`:414-435), so a `\htmautsp`
parsed after the first call is stored in an `m_aSettingsTableSprms` nobody reads
again.  Round 80 measured that the window is open on
`head + 150-5370-10H.rtf[178110:266208]` and shut on the whole document, and
concluded that the trigger sits *after* the word.  It cannot: nothing parsed
after the word can un-send a table that has not been sent yet.  What the round
actually varied was the 178 KB it replaced with a synthetic head.

Each probe here is `witness[0:N]`, closed back to the document group, then
`\htmautsp` and three spaced paragraphs.  A→B of ~24 pt is the word honoured
(the larger of two adjacent spacings wins) and ~48 pt is the word arriving too
late (they add).  The smallest N that sums names the byte that closed the
window.
"""
import sys
import pathlib

WITNESS = "/home/user/corpus-odf/words/pagination-003/rtf/150-5370-10H.rtf"

LETTER = r"\paperw12240\paperh15840\margl1440\margr1440\margt1440\margb1440"
BODY = ("\n" + r"\pard\plain\fs24\sa480 {AAAA}\par" + "\n"
        + r"\pard\plain\fs24\sb480 {BBBB}\par" + "\n"
        + r"\pard\plain\fs24 {CCCC}\par" + "\n")


def depth_at(d: bytes, n: int) -> int:
    """Group depth after consuming `d[:n]`, honouring backslash escapes."""
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
    depth = depth_at(d, n)
    if depth < 1:
        raise ValueError(f"offset {n} is not inside the document group")
    tail = b"}" * (depth - 1)
    return d[:n] + tail + rb"\htmautsp" + LETTER.encode() + BODY.encode() + b"}"


def main(out: pathlib.Path, witness: pathlib.Path, offsets: list[int]) -> None:
    out.mkdir(parents=True, exist_ok=True)
    d = witness.read_bytes()
    for n in offsets:
        (out / f"n{n:08d}.rtf").write_bytes(probe(d, n))
    print(f"wrote {len(offsets)} probes to {out}")


if __name__ == "__main__":
    out = pathlib.Path(sys.argv[1])
    witness = pathlib.Path(sys.argv[2]) if len(sys.argv) > 2 else pathlib.Path(WITNESS)
    offsets = [int(x) for x in sys.argv[3:]] or [31, 2027, 2582, 178110, 260222,
                                                 263584, 263693, 264659, 264960, 266199]
    main(out, witness, offsets)
