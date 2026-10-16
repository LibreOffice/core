#!/usr/bin/env python3
"""Write the one-shape RTF probes this round measured `{\\shp}` placement with.

    python3 gen.py /abs/outdir

Four families, all on A4 unless stated:

  posrel-*   nine files: `posrelh`/`posrelv` absent and every value 0..7, on a page with a
             2 inch left margin and a 1 inch paragraph indent, so the page, the body area and
             the indented column are three different origins.
  clamp-*    fifteen files: five `\\shpwr` values x {fits, overflows right, overflows left},
             plus `far-*` (begins entirely off the sheet) and `below/above-*` (the vertical).
  wrap-*     six files: five `\\shpwr` values and one stating none, each with a long paragraph
             beside a 4000-twip box, which is what separates parallel from through from
             top-and-bottom.
  pv-*       five files: the shape anchored in the SECOND paragraph, which is what separates
             the anchor paragraph from the body area vertically.

Render each through both binaries and read the shape's own word `ZZQ` out of the PDF.
"""
import sys, pathlib

A4 = (r"\paperw11906\paperh16838\margl3139\margr3281\margt1440\margb1440")
LETTER_INDENT = (r"\paperw12240\paperh15840\margl2880\margr1440\margt1440\margb1440")
LETTER = (r"\paperw12240\paperh15840\margl1440\margr1440\margt1440\margb1440")


def head(page):
    return (r"{\rtf1\ansi\ansicpg1252\deff0"
            r"{\fonttbl{\f0\froman\fcharset0 Liberation Serif;}}" + page + "\n")


def shape(left, top, right, bottom, wrap=None, props="", text=r"\pard\plain\f0\fs20 ZZQ\par"):
    s = (r"{\shp{\*\shpinst"
         + fr"\shpleft{left}\shptop{top}\shpright{right}\shpbottom{bottom}")
    if wrap is not None:
        s += fr"\shpwr{wrap}"
    s += r"\shpbxignore\shpbyignore\shpz1"
    s += r"{\sp{\sn shapeType}{\sv 1}}" + props
    s += r"{\sp{\sn fLine}{\sv 0}}"
    if text is not None:
        s += "{\\shptxt" + text + "}"
    return s + "}}"


ZERO_INSET = (r"{\sp{\sn dxTextLeft}{\sv 0}}{\sp{\sn dxTextRight}{\sv 0}}"
              r"{\sp{\sn dyTextTop}{\sv 0}}{\sp{\sn dyTextBottom}{\sv 0}}")

PROSE = ("Alpha bravo charlie delta echo foxtrot golf hotel india juliet kilo lima mike november "
         "oscar papa quebec romeo sierra tango uniform victor whisky xray yankee zulu ") * 2


def files():
    out = {}

    # 1. posrelh / posrelv, on a page whose margin and column differ.
    for name, value in [("absent", None)] + [(str(v), v) for v in range(8)]:
        props = ZERO_INSET
        if value is not None:
            props += "{\\sp{\\sn posrelh}{\\sv %d}}{\\sp{\\sn posrelv}{\\sv %d}}" % (value, value)
        out["posrel-" + name] = (head(LETTER_INDENT) + r"\pard\plain\f0\fs20\li1440 MARKER" + "\n"
                                 + shape(1000, 1000, 4000, 1600, wrap=3, props=props) + r"\par" + "\n}")

    # 2. the capture, on A4 with a 3139-twip left margin.
    box = ZERO_INSET + r"{\sp{\sn posrelh}{\sv 3}}{\sp{\sn posrelv}{\sv 2}}"
    for wr in range(1, 6):
        out[f"clamp-fits-wr{wr}"] = (head(A4) + r"\pard\plain\f0\fs20 MARKER" + "\n"
                                     + shape(591, 1000, 5591, 1600, wr, box) + r"\par" + "\n}")
        out[f"clamp-right-wr{wr}"] = (head(A4) + r"\pard\plain\f0\fs20 MARKER" + "\n"
                                      + shape(591, 1000, 11314, 1600, wr, box) + r"\par" + "\n}")
        out[f"clamp-left-wr{wr}"] = (head(A4) + r"\pard\plain\f0\fs20 MARKER" + "\n"
                                     + shape(-4000, 1000, 1000, 1600, wr, box) + r"\par" + "\n}")
    for wr in (2, 3):
        out[f"clamp-far-wr{wr}"] = (head(A4) + r"\pard\plain\f0\fs20 MARKER" + "\n"
                                    + shape(12000, 1000, 15000, 1600, wr, box) + r"\par" + "\n}")
        out[f"clamp-below-wr{wr}"] = (head(A4) + r"\pard\plain\f0\fs20 MARKER" + "\n"
                                      + shape(591, 18000, 3591, 18600, wr, box) + r"\par" + "\n}")
        out[f"clamp-above-wr{wr}"] = (head(A4) + r"\pard\plain\f0\fs20 MARKER" + "\n"
                                      + shape(591, -2000, 3591, -1400, wr, box) + r"\par" + "\n}")

    # 3. the wrap, with a paragraph long enough to run past the box.
    dist = (r"{\sp{\sn dxWrapDistLeft}{\sv 0}}{\sp{\sn dxWrapDistRight}{\sv 0}}"
            r"{\sp{\sn dyWrapDistTop}{\sv 0}}{\sp{\sn dyWrapDistBottom}{\sv 0}}"
            r"{\sp{\sn posrelh}{\sv 3}}{\sp{\sn posrelv}{\sv 2}}")
    for name, wr in [("none", None)] + [(str(w), w) for w in range(1, 6)]:
        out["wrap-" + name] = (head(LETTER) + r"\pard\plain\f0\fs20 "
                               + shape(0, 200, 4000, 1200, wr, dist) + PROSE + r"\par" + "\n}")

    # 4. posrelv against the SECOND paragraph.
    for name, value in [("absent", None)] + [(str(v), v) for v in (0, 1, 2, 3)]:
        props = ZERO_INSET + r"{\sp{\sn posrelh}{\sv 3}}"
        if value is not None:
            props += "{\\sp{\\sn posrelv}{\\sv %d}}" % value
        out["pv-" + name] = (head(LETTER) + r"\pard\plain\f0\fs20 FIRST\par" + "\n"
                             + r"\pard\plain\f0\fs20 SECOND"
                             + shape(500, 1000, 4500, 1600, 3, props) + r"\par" + "\n}")
    return out


def main():
    out = pathlib.Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    written = files()
    for name, body in written.items():
        (out / (name + ".rtf")).write_text(body, encoding="latin-1")
    print(f"{len(written)} probe files in {out}")


if __name__ == "__main__":
    main()
