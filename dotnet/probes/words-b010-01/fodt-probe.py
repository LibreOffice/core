#!/usr/bin/env python3
"""Ask LibreOffice what it computed, by changing one thing at a time and re-rendering.

The instrument this round's law was measured with, and the reason it is committed: inferring a
header's height from two PDFs gives you one number and no mechanism, while perturbing the flat ODF
LibreOffice itself wrote and asking it again gives you the derivative.

    soffice --headless --convert-to fodt --outdir <dir> <document>      # once, to get the flat XML
    python3 fodt-probe.py <that.fodt>                                   # the sweep below

Each row renders one edited copy through the installed `soffice` and reports the page count and the
`yMin` of the first body line of page 2 — the body's top, which is what a running head's height
decides. Reading the *ink* box is safe here because the same line is measured on every variant, so
the ascent offset cancels; it would not be safe for comparing two different lines.

Measured on `words/batch-010/docx/5709.16 ch.40_mgfinal.docx` against LibreOffice 26.2.4.2:

    base                         pages= 32  body top 134.69
    frame margin-bottom 0                31            114.54  <- our page count, from the reference
    frame margin-bottom 1in              35            186.54
    frame margin-top    1in              32            134.69
    anchor paragraph 20pt, empty         32            134.69
    anchor paragraph 8pt with text       32            134.69  (the text draws at yMin 36.26 —
    anchor paragraph 60pt with text   34/35            174.39   the top of the head, over the frame)
    row min-height 1.5in                 35            182.09
    header min-height 1.5in              33            144.39
    no keep-with-next                    32            134.69  (and page 7 then ends as ours does)

which is: the body top moves one for one with the frame's LOWER spacing, not at all with its upper
spacing, and not with the anchor paragraph until the paragraph is taller than the frame. Raising the
empty paragraph's size alone never reaches that, which is why the 60 pt row needs text in it.

The one figure that did not reproduce between two runs is that row's *page count*, 34 once and 35
once, with the body top identical at 174.39 both times. It is a grotesque synthetic variant and the
number that carries the law is the body top; the page count is downstream of it. Noted rather than
explained, because "render it twice before believing it" is the standing rule and this is what it
looks like when it fires on something harmless.
"""

import os
import re
import subprocess
import sys
import xml.etree.ElementTree as ET

OUT = "/tmp/fodt-probe"
PROFILE = "file:///tmp/fodt-probe/profile"
NS = "{http://www.w3.org/1999/xhtml}"


def render(tag, text):
    """Write an edited flat ODF, convert it, and report pages and the first body line of page 2."""
    os.makedirs(OUT, exist_ok=True)
    path = f"{OUT}/{tag}.fodt"
    with open(path, "w", encoding="utf8") as handle:
        handle.write(text)

    pdf = f"{OUT}/{tag}.pdf"
    if os.path.exists(pdf):
        os.remove(pdf)

    subprocess.run(
        ["soffice", "--headless", f"-env:UserInstallation={PROFILE}",
         "--convert-to", "pdf", "--outdir", OUT, path],
        capture_output=True, timeout=300, check=False)

    if not os.path.exists(pdf):
        print(f"{tag:28s} FAILED TO CONVERT")
        return

    info = subprocess.run(["pdfinfo", pdf], capture_output=True, check=False).stdout.decode()
    pages = info.split("Pages:")[1].split()[0]

    # -bbox-layout, not -bbox: the latter emits words with no enclosing <line>.
    box = subprocess.run(
        ["pdftotext", "-bbox-layout", "-f", "2", "-l", "2", pdf, "-"],
        capture_output=True, check=False).stdout.decode("utf8", "replace")

    lines = [(float(line.get("yMin")), "".join(w.text or "" for w in line.iter(NS + "word")))
             for line in ET.fromstring(box).iter(NS + "line")]

    shown = "  ".join(f"{y:.2f}:{t[:18]}" for y, t in lines[:8])
    print(f"{tag:28s} pages={pages:>3s}  {shown}")


def style(text, name, replacements):
    """Rewrite attributes inside one <style:style> element, failing loudly on a no-op."""
    start = text.find(f'<style:style style:name="{name}"')
    assert start > 0, f"no style named {name}"
    end = text.find("</style:style>", start)
    edited = text[start:end]
    for old, new in replacements.items():
        edited = edited.replace(old, new)
    assert edited != text[start:end], f"{name}: edit changed nothing"
    return text[:start] + edited + text[end:]


def withtext(text):
    """Put text into the paragraph the frame is anchored to, right after the frame."""
    marker = "</draw:frame></text:p>"
    at = text.find(marker)
    assert at > 0, "no frame-anchoring paragraph"
    return text[:at] + "</draw:frame>ANCHORTEXTMARKER" + text[at + len("</draw:frame>"):]


def main():
    source = sys.argv[1]
    with open(source, encoding="utf8") as handle:
        base = handle.read()

    # The frame is the positioned table, `fr1` in the header LibreOffice wrote.
    frame = "fr1"
    lower = re.search(r'fo:margin-bottom="([^"]+)"', base[base.find(f'"{frame}"'):]).group(1)

    render("base", base)
    render("frame-lower-0", style(base, frame, {f'fo:margin-bottom="{lower}"': 'fo:margin-bottom="0in"'}))
    render("frame-lower-1in", style(base, frame, {f'fo:margin-bottom="{lower}"': 'fo:margin-bottom="1in"'}))
    render("frame-upper-1in", style(base, frame, {'fo:margin-top="0in"': 'fo:margin-top="1in"'}))
    render("anchor-20pt", style(base, "Header", {'fo:font-size="8pt"': 'fo:font-size="20pt"'}))

    # The anchor paragraph is empty, so raising its size alone gives it one taller line and nothing
    # else — which is the point of the 20 pt row. To make it out-grow the frame it needs text as
    # well: at 60 pt this wraps to two 69 pt lines, 138 pt, and only then does it decide the height.
    render("anchor-text-8pt", withtext(base))
    render("anchor-text-60pt",
           style(withtext(base), "Header", {'fo:font-size="8pt"': 'fo:font-size="60pt"'}))
    render("row-1.5in", base.replace('style:min-row-height="0.691in"', 'style:min-row-height="1.5in"'))
    render("head-min-1.5in", base.replace('fo:min-height="0.5in"', 'fo:min-height="1.5in"', 1))

    # The control that decides the keep-with-next lead in §6 of results.md: with this gone,
    # LibreOffice produces our page break exactly.
    render("no-keep-with-next", base.replace(' fo:keep-with-next="always"', ""))


if __name__ == "__main__":
    main()
