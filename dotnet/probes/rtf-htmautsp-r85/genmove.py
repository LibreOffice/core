#!/usr/bin/env python3
r"""Three copies of the witness, differing only in where `\htmautsp` stands.

    python3 genmove.py /abs/outdir [witness.rtf]

    w-asis      the document unchanged
    w-none      the word deleted
    w-early     the word moved to the top of the document group, before the
                font table -- so that no reading of `checkFirstRun` can call it
                late.

If `w-early` and `w-asis` render alike, the window is not what decides this
document and the settings-table timing is a dead end for it.
"""
import sys
import pathlib

WITNESS = "/home/user/corpus-odf/words/pagination-003/rtf/150-5370-10H.rtf"
WORD = rb"\htmautsp"


def main(out: pathlib.Path, witness: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    d = witness.read_bytes()
    i = d.find(WORD)
    if i < 0 or d.find(WORD, i + 1) >= 0:
        raise SystemExit(f"expected exactly one {WORD!r}")
    stripped = d[:i] + d[i + len(WORD):]
    # byte 31 is the first `{` after the document group's own control words.
    at = stripped.find(b"{", 1)
    (out / "w-asis.rtf").write_bytes(d)
    (out / "w-none.rtf").write_bytes(stripped)
    (out / "w-early.rtf").write_bytes(stripped[:at] + WORD + stripped[at:])
    print(f"word at {i}; wrote three probes to {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]),
         pathlib.Path(sys.argv[2]) if len(sys.argv) > 2 else pathlib.Path(WITNESS))
