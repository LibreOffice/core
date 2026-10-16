#!/usr/bin/env python3
"""Does the reference keep a table row whole where we split it? — twenty files that answer no.

    python3 rowsplit.py /abs/outdir

Each file is `n` one-line rows followed by a three-line row and then one more, on a letter page
with one-inch margins, so `n` walks the tall row across the page boundary. Render both ways and
read which page `AAAA`, `CCCC` and `AFTER` land on: if a renderer splits the tall row, `AAAA` and
`CCCC` are on different pages.

Measured against 26.2.4.2 over n = 44..63: both split in the same place at every one, and the
two sides disagree only at n = 51 and 52, where the reference breaks the page and we fit one
more row on it. So the difference at a page bottom is a one-row capacity difference and NOT a
keep-together rule.
"""
import sys, pathlib

HEAD = (r"{\rtf1\ansi\deff0{\fonttbl{\f0\froman Liberation Serif;}}"
        r"\paperw12240\paperh15840\margl1440\margr1440\margt1440\margb1440\sectd" "\n")


def row(cells):
    s = r"\trowd\trgaph0\cellx3000\cellx7000"
    for c in cells:
        s += r"\pard\intbl\plain\f0\fs20 " + c + r"\cell"
    return s + r"\row" + "\n"


def doc(nfill):
    body = HEAD
    for i in range(nfill):
        body += row([f"R{i}", f"row {i}"])
    body += row([r"AAAA\line BBBB\line CCCC", "tall row"])
    body += row(["AFTER", "after row"])
    return body + "}"


def main():
    out = pathlib.Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    for n in range(44, 64):
        (out / f"n{n}.rtf").write_text(doc(n), encoding="latin-1")
    print(f"20 files in {out}")


if __name__ == "__main__":
    main()
