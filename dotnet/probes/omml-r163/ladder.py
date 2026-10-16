#!/usr/bin/env python3
"""Per-construct verdict table for `words-formula-complexity.docx`.

Each rung's inline line reads `<id> inline: before <FORMULA> after (<label>).`
The formula is isolated geometrically: every drawn character whose x lies
between the end of that line's "before" and the start of its "after", and
whose baseline is within +/- 24 pt of the label's own baseline (a formula's
glyphs sit off the text baseline by design, which is the whole point).

Positions come from PyMuPDF's per-character `origin`, which is the baseline
the writer's Td/Tm stated -- not the font-descriptor ink box `pdftotext -bbox`
reports, whose constant per-face offset looks exactly like a layout defect.

Usage:  ladder.py REF.pdf OURS.pdf
"""
import sys
import pymupdf

IDS = ["sub", "sup", "subsup", "presub", "frac", "fracsub", "rad", "radn",
       "narysum", "naryint", "delim", "acc", "func", "matrix", "mixed"]
BAND = 20.0   # covers every row the reference draws (max |19.64| pt) and
              # excludes the neighbouring display paragraph's own rows


def load(path):
    doc = pymupdf.open(path)
    pages = []
    for page in doc:
        words = page.get_text("words")          # x0 y0 x1 y1 word block line wordno
        chars = []
        for block in page.get_text("rawdict")["blocks"]:
            if block.get("type") != 0:
                continue
            for line in block["lines"]:
                for span in line["spans"]:
                    for c in span["chars"]:
                        chars.append((c["origin"][0], c["origin"][1],
                                      round(span["size"], 3), c["c"]))
        pages.append((words, chars))
    return pages


def band_for(pages, ident):
    """(base_y, [(x, y, size, char)]) for the formula on `ident`'s inline line."""
    for words, chars in pages:
        # The label word is the identifier immediately followed by "inline:".
        for i, w in enumerate(words):
            if w[4] != ident or i + 1 >= len(words) or words[i + 1][4] != "inline:":
                continue
            label_y = w[3]                       # bottom of the label word
            row = [v for v in words if abs(v[3] - label_y) < 2.0]
            before = next((v for v in row if v[4] == "before"), None)
            after = next((v for v in row if v[4].startswith("after")), None)
            if before is None or after is None:
                return None, []
            lo, hi = before[2], after[0]
            # The line's own BASELINE, taken from the characters of the word
            # "before" -- not the word bbox's bottom, which is a descender
            # below it and would put every comparison out by a constant.
            inside = [c for c in chars
                      if before[0] - 0.5 <= c[0] <= before[2]
                      and before[1] <= c[1] <= before[3] + 0.5]
            if not inside:
                return None, []
            text_y = min(c[1] for c in inside)
            # A baseline that also carries ink OUTSIDE the formula's x-range
            # belongs to a neighbouring text line, not to the formula: the
            # label text and the display paragraph under it both extend past
            # [lo, hi], while a formula's own raised and lowered rows do not.
            # The line's own baseline is exempt -- a formula that draws every
            # glyph on it (which is exactly the defect under test) shares it.
            outside = {round(c[1], 1) for c in chars
                       if not c[3].isspace() and not (lo - 2 <= c[0] < hi + 2)}
            picked = [c for c in chars
                      if lo <= c[0] < hi and abs(c[1] - text_y) < BAND
                      and not c[3].isspace()
                      and (abs(c[1] - text_y) < 1.0
                           or round(c[1], 1) not in outside)]
            picked.sort(key=lambda c: (round(c[1], 1), c[0]))
            return text_y, picked
    return None, []


def describe(picked):
    if not picked:
        return "-", "-", "-", ""
    sizes = sorted({c[2] for c in picked}, reverse=True)
    base = sizes[0]
    base_y = min(c[1] for c in picked if c[2] == base)
    lines = sorted({round(c[1] - base_y, 2) for c in picked})
    glyphs = "".join(c[3] for c in picked)
    return ("/".join(f"{s:.2f}" for s in sizes),
            "/".join(f"{v:+.2f}" for v in lines),
            str(len(lines)),
            glyphs)


def main():
    ref, ours = load(sys.argv[1]), load(sys.argv[2])
    print("construct\tside\tsizes(pt)\tbaselines rel. base(pt)\trows\tglyphs")
    for ident in IDS:
        for name, pages in (("ref", ref), ("ours", ours)):
            _, picked = band_for(pages, ident)
            s, b, n, g = describe(picked)
            print(f"{ident}\t{name}\t{s}\t{b}\t{n}\t{g}")


if __name__ == "__main__":
    main()
