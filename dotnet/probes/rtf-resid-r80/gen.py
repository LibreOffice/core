#!/usr/bin/env python3
r"""Write the RTF probes this round measured a paragraph *style*'s paragraph formatting with.

    python3 gen.py /abs/outdir

Every probe is one letter page holding three paragraphs:

    AAAA   plain, `\pard\plain` and nothing else
    BBBB   the paragraph under test, naming a style
    CCCC   plain again

so the style's contribution is readable as three numbers off the PDF: `BBBB`'s x (its
alignment), `AAAA` -> `BBBB` (its space before, plus `AAAA`'s space after) and `BBBB` -> `CCCC`
(its space after, plus `CCCC`'s space before).

`\s0 Normal` states `\sb120\sa120` in every probe, which is what LibreOffice's own export
writes and what makes "does an unstyled paragraph take style zero" answerable at all.

The families:

  direct-*   the named style states the property itself
  chain-*    the named style states nothing and its `\sbasedon` parent states the property,
             which is the witness document's own shape (`\s3274\sbasedon3155` where
             `\s3155 Centered` is `\qc\sb480\sa120\keepn`)
  pool-*     the named style is called `heading 5`, which `StyleSheetTable::ConvertStyleName`
             maps onto Writer's built-in `Heading 5` -- the case a previous round measured a
             regression on and generalised from
  zero-*     no style is named at all, so `\pard` alone decides
"""
import sys
import pathlib

LETTER = r"\paperw12240\paperh15840\margl1440\margr1440\margt1440\margb1440"

FONTS = r"{\fonttbl{\f0\froman\fcharset0 Liberation Serif;}{\f1\fswiss\fcharset0 Liberation Sans;}}"

# `\s0` always states these two, exactly as LibreOffice's export does.
NORMAL = r"{\s0\snext0\ql\sb120\sa120\loch\f0\fs22 Normal;}"


def doc(styles, body):
    return (r"{\rtf1\ansi\ansicpg1252\deff0" + FONTS
            + r"{\stylesheet" + NORMAL + styles + "}"
            + LETTER + "\n" + body + "}\n")


def para(text, prefix=r"\pard\plain "):
    return prefix + "{" + text + r"}\par" + "\n"


def three(styled_prefix, styles):
    """AAAA plain, BBBB under test, CCCC plain."""
    return doc(styles,
               para("AAAA") + para("BBBB", styled_prefix) + para("CCCC"))


CASES: dict[str, tuple[str, str]] = {}

# --- direct: the style states the property itself ---------------------------------------
CASES["direct-qc"] = (r"\pard\plain \s10 ", r"{\s10\sbasedon0\snext10\qc Centred;}")
CASES["direct-sb480"] = (r"\pard\plain \s10 ", r"{\s10\sbasedon0\snext10\sb480 SpaceBefore;}")
CASES["direct-sa480"] = (r"\pard\plain \s10 ", r"{\s10\sbasedon0\snext10\sa480 SpaceAfter;}")
CASES["direct-li720"] = (r"\pard\plain \s10 ", r"{\s10\sbasedon0\snext10\li720\lin720 Indented;}")
CASES["direct-ri1440"] = (r"\pard\plain \s10 ", r"{\s10\sbasedon0\snext10\ri1440\rin1440 RightIn;}")
CASES["direct-fi-360"] = (r"\pard\plain \s10 ", r"{\s10\sbasedon0\snext10\li720\lin720\fi-360 Hanging;}")
CASES["direct-qr"] = (r"\pard\plain \s10 ", r"{\s10\sbasedon0\snext10\qr RightAligned;}")
CASES["direct-sl480"] = (r"\pard\plain \s10 ", r"{\s10\sbasedon0\snext10\sl480\slmult0 Leading;}")

# The witness's own combination.
CASES["direct-centred"] = (
    r"\pard\plain \s10 ",
    r"{\s10\sbasedon0\snext10\qc\sb480\sa120\keepn\caps\b Centered;}")

# --- chain: the style states nothing, its parent states it -------------------------------
CASES["chain-centred"] = (
    r"\pard\plain \s11 ",
    r"{\s10\sbasedon0\snext10\qc\sb480\sa120\keepn\caps\b Centered;}"
    r"{\s11\sbasedon10\snext11 Centered bold KWN;}")
CASES["chain-override"] = (
    r"\pard\plain \s11 ",
    r"{\s10\sbasedon0\snext10\qc\sb480 Centered;}"
    r"{\s11\sbasedon10\snext11\ql\sb0 Flat;}")

# --- the style's property against the paragraph's own -----------------------------------
CASES["direct-overridden"] = (
    r"\pard\plain \s10\ql\sb0 ",
    r"{\s10\sbasedon0\snext10\qc\sb480 Centered;}")

# --- pool: the style is named for a Writer built-in ---------------------------------------
CASES["pool-h5-inherit"] = (
    r"\pard\plain \s5 ",
    r"{\s5\sbasedon0\snext0 heading 5;}")
CASES["pool-h5-own"] = (
    r"\pard\plain \s5 ",
    r"{\s5\sbasedon0\snext0\sb480\sa480\qc heading 5;}")
CASES["pool-h5-parent"] = (
    r"\pard\plain \s5 ",
    r"{\s10\sbasedon0\snext10\qc\sb480\sa480 Centered;}"
    r"{\s5\sbasedon10\snext0 heading 5;}")
CASES["pool-normal-named"] = (
    r"\pard\plain \s10 ",
    r"{\s10\sbasedon0\snext10\qc\sb480 Normal;}")

# --- zero: no `\s` at all -----------------------------------------------------------------
CASES["zero-plain"] = (r"\pard\plain ", "")


def main(out: pathlib.Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    for name, (prefix, styles) in CASES.items():
        (out / f"{name}.rtf").write_text(three(prefix, styles), encoding="latin-1")
    print(f"{len(CASES)} probes in {out}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]))
