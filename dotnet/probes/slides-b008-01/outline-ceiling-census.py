#!/usr/bin/env python3
"""Census the *outline* ceiling: pages where the reference draws glyphs as vector paths.

    outline-ceiling-census.py <sweepdir> <refdir>

TODO.raster-ceiling.md's page test asks `pdfimages` whether the reference drew a raster where
we drew text. That test is structurally blind to a second way the reference can put ink on a
page without putting anything in its text layer: **outlining the glyphs into filled paths.**
`pdfimages` reports no image, so the page looks like a plain word-count defect of ours.

Measured on `slides/batch-008/…/8_P-Pavese…pptx` page 16, where the reference draws the twenty
rotated date-axis tick labels of its first chart as **120 filled paths of 4-6 pt in a single
grey**, with zero text-showing operators anywhere in that band, while we draw them as 103 real
glyphs. Two blind reviewers read the labels in the reference; `pdftotext` reads none of them.

The signature this looks for, on a page whose document is already page-exact:

  1. we extract at least 8 more raw words on that page than the reference does;
  2. the reference page carries at least 20 glyph-sized filled paths — smaller than 12x12 pt —
     sharing one non-white fill colour;
  3. we draw materially more text glyphs on that page than the reference does.

Condition 2 is the whole point and is deliberately loose about *what* was outlined: a chart
label, a WordArt run and a rotated table header would all satisfy it, and the census should
find out which rather than assume.

Reads a finished sweep's `ours/` directory against the banked references. No re-rendering.
"""

import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

# `.claude/` is not tracked and does not exist inside a worktree, so this resolves against
# the primary checkout. Read-only use of a shared script; nothing is written there.
OPS = os.environ.get(
    "PDF_OPS",
    "/c/sandbox/workdir/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py")

MIN_EXTRA_WORDS = 8
MIN_SMALL_FILLS = 20
MAX_GLYPH_PT = 12.0
MIN_EXTRA_GLYPHS = 20

BOX = re.compile(r"\(\s*([\d.-]+),\s*([\d.-]+)\)\s*-\s*\(\s*([\d.-]+),\s*([\d.-]+)\)\s*(#\w+)")
TXT = re.compile(r"(\d+) glyphs in")


def run(command, timeout=300):
    try:
        return subprocess.run(command, capture_output=True, timeout=timeout,
                              check=False).stdout.decode("utf8", "replace")
    except Exception:
        return ""


def page_count(pdf):
    for line in run(["pdfinfo", pdf]).splitlines():
        if line.startswith("Pages:"):
            return int(line.split()[1])
    return -1


def raw_words(pdf, page):
    return len(run(["pdftotext", "-f", str(page), "-l", str(page), pdf, "-"]).split())


def glyphs(pdf, page):
    out = run(["python3", OPS, "dump", pdf, "--page", str(page), "--only", "text"])
    return sum(int(m.group(1)) for m in TXT.finditer(out))


def small_fills(pdf, page):
    """(count, colour) for the largest single-colour family of glyph-sized fills."""
    out = run(["python3", OPS, "dump", pdf, "--page", str(page), "--only", "fill"])
    by_colour = {}
    for line in out.splitlines():
        m = BOX.search(line)
        if not m:
            continue
        x0, y0, x1, y1, colour = m.groups()
        w, h = abs(float(x1) - float(x0)), abs(float(y1) - float(y0))
        if 0 < w < MAX_GLYPH_PT and 0 < h < MAX_GLYPH_PT and colour.upper() != "#FFFFFF":
            by_colour[colour] = by_colour.get(colour, 0) + 1
    if not by_colour:
        return 0, "-"
    colour = max(by_colour, key=by_colour.get)
    return by_colour[colour], colour


def examine(pair):
    ours, reference, name = pair
    rows = []
    a, b = page_count(ours), page_count(reference)
    if a != b or a < 1:
        return rows
    for page in range(1, a + 1):
        ow, rw = raw_words(ours, page), raw_words(reference, page)
        if ow - rw < MIN_EXTRA_WORDS:
            continue
        count, colour = small_fills(reference, page)
        if count < MIN_SMALL_FILLS:
            continue
        og, rg = glyphs(ours, page), glyphs(reference, page)
        if og - rg < MIN_EXTRA_GLYPHS:
            continue
        rows.append((name, page, ow, rw, ow - rw, count, colour, og, rg))
    return rows


def main():
    sweep, refdir = sys.argv[1], sys.argv[2]
    ourdir = os.path.join(sweep, "ours")
    pairs = []
    for name in sorted(os.listdir(ourdir)):
        if not name.endswith(".pdf"):
            continue
        reference = os.path.join(refdir, name)
        if os.path.exists(reference):
            pairs.append((os.path.join(ourdir, name), reference, name[:-4]))
    print(f"# {len(pairs)} page-comparable document pairs", file=sys.stderr)
    print("document\tpage\tours_raw\tref_raw\texcess\tref_small_fills\tcolour"
          "\tours_glyphs\tref_glyphs")
    with ThreadPoolExecutor(max_workers=6) as pool:
        for rows in pool.map(examine, pairs):
            for row in rows:
                print("\t".join(str(v) for v in row), flush=True)


if __name__ == "__main__":
    main()
