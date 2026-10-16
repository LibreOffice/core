#!/usr/bin/env python3
r"""Prefixes of the witness, each rendered with and without its `\htmautsp`.

    python3 gensuffix.py /abs/outdir <offset> [<offset> ...]

Round 80 argued from a synthetic head that the trigger which closes the
settings window sits after the word.  Nothing parsed after the word can
un-send a table that has not been sent yet, so the claim needs a direct
instrument: cut the real document at N, keep everything before it exactly as
the author wrote it, and render the cut twice -- once as it stands and once
with the nine bytes of `\htmautsp` removed.  Two page counts that agree say
the word did nothing; two that differ say it was honoured.

Offsets must be group depth 1 (see `snap.py`).
"""
import sys
import pathlib

WITNESS = "/home/user/corpus-odf/words/pagination-003/rtf/150-5370-10H.rtf"
WORD = rb"\htmautsp"


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
        cut = d[:n] + b"}" * depth_at(d, n)
        (out / f"n{n:08d}-word.rtf").write_bytes(cut)
        (out / f"n{n:08d}-none.rtf").write_bytes(cut.replace(WORD, b"", 1))
    print(f"wrote {2 * len(offsets)} probes to {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]), [int(x) for x in sys.argv[2:]],
         pathlib.Path(WITNESS))
