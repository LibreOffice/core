#!/usr/bin/env python3
r"""The five smaller probe families this round used, beside the two matrices.

    python3 genextra.py /abs/outdir

  chain-*   where in the `\sbasedon` chain a property has to be stated to reach the paragraph,
            including a child declared *before* its parent
  keep-*    `\keepn`, which the three-paragraph matrix cannot see: the page is filled to its last
            line, so a kept paragraph moves to the next page and an unkept one does not
  sum-*     whether `\sa` above and `\sb` below add or collapse, which is the control the
            `\htmautsp` family needs
  htm-*     when `\htmautsp` is still readable -- before the body, after it, and after a
            `{\header}` group
  deff-*    which face a run takes when `\deff`, the style's `\f` and `Times New Roman` are three
            different answers
"""
import sys
import pathlib

SERIF = r"{\f0\froman\fcharset0 Liberation Serif;}"
SANS = r"{\f1\fswiss\fcharset0 Liberation Sans;}"
MONO = r"{\f2\fmodern\fcharset0 Liberation Mono;}"
FONTS = "{\\fonttbl" + SERIF + SANS + MONO + "}"
LETTER = r"\paperw12240\paperh15840\margl1440\margr1440\margt1440\margb1440"
NORMAL = r"{\s0\snext0\ql Normal;}"
COLOURS = r"{\colortbl;\red0\green0\blue0;\red255\green0\blue0;}"


def head(deff=r"\deff0", colours=False):
    return r"{\rtf1\ansi\ansicpg1252" + deff + FONTS + (COLOURS if colours else "")


def three(sheet, use, first=r"\pard\plain "):
    return (r"{\stylesheet" + sheet + "}" + LETTER + "\n"
            + first + r"{AAAA}\par" + "\n"
            + r"\pard\plain " + use + r"{BBBB}\par" + "\n"
            + r"\pard\plain {CCCC}\par" + "\n}")


CHAIN = {
    # the property is stated here ->                 the paragraph names this
    "chain-own": (NORMAL + r"{\s10\sbasedon0\snext10\qc\sb480 Foo;}", r"\s10 "),
    "chain-child": (NORMAL + r"{\s10\sbasedon0\snext10\qc\sb480 Foo;}"
                    r"{\s11\sbasedon10\snext11 Bar;}", r"\s11 "),
    "chain-sibling": (NORMAL + r"{\s10\sbasedon0\snext10\qc\sb480 Foo;}"
                      r"{\s11\sbasedon0\snext11 Bar;}", r"\s11 "),
    "chain-fromzero": (r"{\s0\snext0\qc\sb480 Normal;}{\s10\sbasedon0\snext10 Foo;}", r"\s10 "),
    "chain-childfirst": (NORMAL + r"{\s11\sbasedon10\snext11 Bar;}"
                         r"{\s10\sbasedon0\snext10\qc\sb480 Foo;}", r"\s11 "),
    "chain-grandchild": (NORMAL + r"{\s10\sbasedon0\snext10\qc\sb480 Foo;}"
                         r"{\s11\sbasedon10\snext11 Bar;}{\s12\sbasedon11\snext12 Baz;}", r"\s12 "),
    "chain-restated": (NORMAL + r"{\s10\sbasedon0\snext10\qc\sb480 Foo;}"
                       r"{\s11\sbasedon10\snext11\qc\sb480 Bar;}", r"\s11 "),
    "chain-gap": (NORMAL + r"{\s10\sbasedon0\snext10\qc\sb480 Foo;}"
                  r"{\s20\sbasedon0\snext20\li1440 Mid;}{\s11\sbasedon10\snext11 Bar;}", r"\s11 "),
    "chain-paraoverrides": (NORMAL + r"{\s10\sbasedon0\snext10\qc\sb480 Foo;}"
                            r"{\s11\sbasedon10\snext11 Bar;}", r"\s11\ql "),
}

SUM = {
    "sum-none": ("", ""),
    "sum-after": (r"\sa480 ", ""),
    "sum-before": ("", r"\sb480 "),
    "sum-both": (r"\sa480 ", r"\sb480 "),
}

HTM = {
    "htm-absent": "",
    "htm-before": r"\htmautsp",
    "htm-afterparagraph": None,          # written out below
    "htm-afterheader": r"{\header\pard\plain {H}\par }" + "\n" + r"\htmautsp",
}

DEFF = {
    "deff-own-plain": (NORMAL + r"{\s10\sbasedon0\snext10\f1 Foo;}", r"\plain \s10 "),
    "deff-inherited-plain": (NORMAL + r"{\s10\sbasedon0\snext10\f1 Foo;}"
                             r"{\s11\sbasedon10\snext11 Bar;}", r"\plain \s11 "),
    "deff-own-noplain": (NORMAL + r"{\s10\sbasedon0\snext10\f1 Foo;}", r" \s10 "),
    "deff-inherited-noplain": (NORMAL + r"{\s10\sbasedon0\snext10\f1 Foo;}"
                               r"{\s11\sbasedon10\snext11 Bar;}", r" \s11 "),
    "deff-control": (NORMAL, r"\plain "),
}


def keep(sheet, use, fillers):
    """`BBBB` is the last line that fits, so keep-with-next moves it to the next page."""
    fill = "".join(r"\pard\plain {F%03d}\par" % i + "\n" for i in range(fillers))
    return (r"{\stylesheet" + sheet + "}" + LETTER + "\n" + fill
            + r"\pard\plain " + use + r"{BBBB}\par" + "\n"
            + r"\pard\plain {CCCC}\par" + "\n}")


def main(out: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    n = 0
    for name, (sheet, use) in CHAIN.items():
        (out / f"{name}.rtf").write_text(head() + three(sheet, use), encoding="latin-1")
        n += 1

    for name, (after, before) in SUM.items():
        (out / f"{name}.rtf").write_text(
            head() + r"{\stylesheet" + NORMAL + "}" + LETTER + "\n"
            + r"\pard\plain " + after + r"{AAAA}\par" + "\n"
            + r"\pard\plain " + before + r"{BBBB}\par" + "\n"
            + r"\pard\plain {CCCC}\par" + "\n}", encoding="latin-1")
        n += 1

    for name, prologue in HTM.items():
        body = (r"\pard\plain {ZERO}\par" + "\n" + r"\htmautsp" + "\n"
                if prologue is None else (prologue + "\n" if prologue else ""))
        (out / f"{name}.rtf").write_text(
            head() + r"{\stylesheet" + NORMAL + "}" + LETTER + "\n" + body
            + r"\pard\plain \sa480 {AAAA}\par" + "\n"
            + r"\pard\plain \sb480 {BBBB}\par" + "\n"
            + r"\pard\plain {CCCC}\par" + "\n}", encoding="latin-1")
        n += 1

    # `\deff2` is Liberation Mono, the style names `\f1` Liberation Sans, and the reset default
    # `Times New Roman` resolves to Liberation Serif -- three distinguishable answers.
    for name, (sheet, use) in DEFF.items():
        (out / f"{name}.rtf").write_text(
            head(r"\deff2") + r"{\stylesheet" + sheet + "}" + LETTER + "\n"
            + r"\pard" + use + r"{BBBB Hamburgefonstiv}\par" + "\n}", encoding="latin-1")
        n += 1

    for fillers in (44, 45, 46):
        for tag, (sheet, use) in {
                "control": (NORMAL, ""),
                "own": (NORMAL + r"{\s10\sbasedon0\snext10\keepn Foo;}", r"\s10 "),
                "inherited": (NORMAL + r"{\s10\sbasedon0\snext10\keepn Foo;}"
                              r"{\s11\sbasedon10\snext11 Bar;}", r"\s11 "),
                "direct": (NORMAL, r"\keepn ")}.items():
            (out / f"keep{fillers}-{tag}.rtf").write_text(
                head() + keep(sheet, use, fillers), encoding="latin-1")
            n += 1

    print(f"{n} probes in {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]))
