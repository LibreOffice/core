#!/usr/bin/env python3
r"""Does a body `{\pict}` end the settings window, or was it the `\par` beside it?

    python3 genbodypict.py /abs/outdir

`probes/rtf-resid-r80`'s `p-bodypict` writes
`\pard\plain{\pict …}\par\htmautsp` and reads the summed spacing that comes
back as the picture closing the window.  The group is followed by a `\par`,
which is `checkFirstRun`'s caller at `rtfdispatchsymbol.cxx`:133, so that probe
cannot tell the two apart.  These three separate them, each with its own
no-word control.

    q-pict     the picture group and nothing else before the word
    q-pictpar  the picture and a `\par`      -- round 80's shape
    q-par      the `\par` alone
"""
import sys
import pathlib

HEAD = (r"{\rtf1\ansi\ansicpg1252\deff0{\fonttbl{\f0\froman\fcharset0 Liberation Serif;}}"
        r"{\stylesheet{\s0\snext0\ql Normal;}}"
        r"\paperw12240\paperh15840\margl1440\margr1440\margt1440\margb1440")
BODY = ("\n" + r"\pard\plain\fs24\sa480 {AAAA}\par" + "\n"
        + r"\pard\plain\fs24\sb480 {BBBB}\par" + "\n"
        + r"\pard\plain\fs24 {CCCC}\par" + "\n}")
W = r"\htmautsp"
PIC = (r"\pard\plain{\pict\pngblip\picw10\pich10\picwgoal180\pichgoal180"
       r" 89504e470d0a1a0a}")

CASES = {
    "q-pict": HEAD + "\n" + PIC + W + BODY,
    "q-pictpar": HEAD + "\n" + PIC + r"\par" + W + BODY,
    "q-par": HEAD + "\n" + r"\pard\plain\par" + W + BODY,
}


def main(out: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    for name, text in CASES.items():
        (out / f"{name}.rtf").write_bytes(text.encode("latin-1"))
        (out / f"{name}-noword.rtf").write_bytes(text.replace(W, "").encode("latin-1"))
    print(f"wrote {2 * len(CASES)} probes to {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]))
