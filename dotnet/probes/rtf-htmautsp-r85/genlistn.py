#!/usr/bin/env python3
r"""How many of the witness's 70 `{\list}` entries it takes to cost the word.

    python3 genlistn.py /abs/outdir [count ...]

The list table's `{\*\listpicture}` is not what does it and one list is not
either, so the probe varies how many of the remaining entries are kept.
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


def group_end(d: bytes, start: int) -> int:
    depth, i, n = 0, start, len(d)
    while i < n:
        c = d[i]
        if c == 0x5C:
            i += 2 if i + 1 < n and d[i + 1] in b"\\{}'" else 1
            continue
        if c == 0x7B:
            depth += 1
        elif c == 0x7D:
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    raise ValueError("unbalanced")


def children(d: bytes) -> list[tuple[int, int]]:
    out, i, end = [], LT + 1, LT_END - 1
    while i < end:
        if d[i] == 0x7B:
            j = group_end(d, i)
            out.append((i, j))
            i = j
            continue
        if d[i] == 0x5C:
            i += 2 if d[i + 1] in b"\\{}'" else 1
            continue
        i += 1
    return out


def main(out: pathlib.Path, counts: list[int], witness: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    d = witness.read_bytes()
    kids = children(d)
    lists = kids[1:]                       # kids[0] is {\*\listpicture …}
    print(f"{len(lists)} list entries", file=sys.stderr)
    for k in counts:
        lt = b"{\\*\\listtable" + b"".join(d[a:b] for a, b in lists[:k]) + b"}"
        (out / f"k{k:03d}.rtf").write_bytes(
            d[:LT] + lt + W + ROW.encode() + BODY.encode() + b"}")
    print(f"wrote {len(counts)} probes to {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]),
         [int(x) for x in sys.argv[2:]] or [1, 2, 4, 8, 16, 32, 70],
         pathlib.Path(WITNESS))
