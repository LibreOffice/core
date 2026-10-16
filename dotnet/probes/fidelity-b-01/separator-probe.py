#!/usr/bin/env python3
"""fidelity-b-01 — what the installed LibreOffice draws above a page's footnotes.

Authors one minimal document, converts it to every spelling LibreOffice can write,
renders each with the *installed* binary and reads the note separator straight out of
the PDF content stream. The question is which formats get Word's fixed 2-inch rule and
which keep Writer's proportional one — the positive case for DOCX and DOC, and the
negative case for ODT/FODT (and for RTF, which XRay.Office routes through the same
`PaginationOptions.Word` preset and which therefore decides whether the condition can be
that preset at all).

The rule is read as a filled path. LibreOffice's PDF export does **not** write it as
`x y w h re f`, which is what one would guess and what cost this probe its first run: it
writes an explicit closed polygon — `x y m x y l x y l x y l x y l h B*` — so the reader
has to take the bounding box of a `m`/`l` subpath ended by a painting operator. On a page
holding nothing else, the flattest such box is the rule.

Run:  python3 separator-probe.py <outdir>
"""

import os
import re
import subprocess
import sys
import zlib

# A minimal document: one body paragraph, one footnote, A4, stated margins.
# Newly authored. Nothing is copied from the corpus or from any real document.
FODT = """<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.text">
 <office:styles>
  <style:default-style style:family="paragraph">
   <style:text-properties fo:font-size="{size}"/>
  </style:default-style>
 </office:styles>
 <office:automatic-styles>
  <style:page-layout style:name="pm1">
   <style:page-layout-properties fo:page-width="21cm" fo:page-height="29.7cm"
    fo:margin-top="2cm" fo:margin-bottom="2cm"
    fo:margin-left="{margin}" fo:margin-right="{margin}"/>
  </style:page-layout>
 </office:automatic-styles>
 <office:master-styles>
  <style:master-page style:name="Standard" style:page-layout-name="pm1"/>
 </office:master-styles>
 <office:body><office:text>
  <text:p>Body text with a note<text:note text:id="ftn1" text:note-class="footnote"
   ><text:note-citation>1</text:note-citation
   ><text:note-body><text:p>The note.</text:p></text:note-body></text:note>.</text:p>
 </office:text></office:body>
</office:document>
"""


def convert(source, target_filter, extension, outdir, profile):
    subprocess.run(
        ["soffice", "--headless", f"-env:UserInstallation=file://{profile}",
         "--convert-to", target_filter, "--outdir", outdir, source],
        check=True, capture_output=True, timeout=180)
    produced = os.path.join(
        outdir, os.path.splitext(os.path.basename(source))[0] + extension)
    if not os.path.exists(produced):
        raise RuntimeError(f"{source} -> {target_filter} produced nothing")
    return produced


def streams(pdf_path):
    """Every content stream in a PDF, inflated."""
    with open(pdf_path, "rb") as handle:
        raw = handle.read()

    out = []
    for match in re.finditer(rb"stream\r?\n", raw):
        start = match.end()
        end = raw.find(b"endstream", start)
        if end < 0:
            continue
        body = raw[start:end]
        try:
            out.append(zlib.decompress(body).decode("latin-1"))
        except zlib.error:
            continue
    return out


NUMBER = r"-?[\d.]+"
SUBPATH = re.compile(
    rf"(?P<body>(?:{NUMBER}\s+{NUMBER}\s+[ml]\s+)+)h\s+(?P<paint>B\*|B|f\*|f)\b")
POINT = re.compile(rf"({NUMBER})\s+({NUMBER})\s+[ml]")


def fills(pdf_path):
    """Every painted closed subpath, as an (x, y, width, height) box in points."""
    found = []
    for stream in streams(pdf_path):
        for match in SUBPATH.finditer(stream):
            points = [(float(x), float(y))
                      for x, y in POINT.findall(match.group("body"))]
            if len(points) < 3:
                continue
            xs = [point[0] for point in points]
            ys = [point[1] for point in points]
            found.append((min(xs), min(ys), max(xs) - min(xs), max(ys) - min(ys)))
        if found:
            break
    return found


def separator(pdf_path):
    """The note rule: the flattest filled rectangle on the page."""
    found = fills(pdf_path)
    if not found:
        return None
    return min(found, key=lambda rect: abs(rect[3]))


PEN = re.compile(rf"({NUMBER})\s+({NUMBER})\s+Td\s*/F\d+\s+({NUMBER})\s+Tf")


def note_baseline(pdf_path, note_size):
    """The topmost baseline drawn at the note's em size, in PDF points."""
    for stream in streams(pdf_path):
        pens = [(float(y), float(size))
                for _, y, size in PEN.findall(stream)]
        at_size = [y for y, size in pens if abs(size - note_size) < 0.01]
        return max(at_size) if at_size else None
    return None


def run(label, margin, text_width, size, outdir, profile):
    """Renders one authored document in every spelling and reads its rule."""
    source = os.path.join(outdir, f"sep-{label}.fodt")
    with open(source, "w", encoding="utf-8") as handle:
        handle.write(FODT.format(margin=margin, size=size))

    spellings = [("fodt", source)]
    for target_filter, extension in (
            ("odt", ".odt"), ("docx", ".docx"),
            ("doc:MS Word 97", ".doc"), ("rtf", ".rtf")):
        spellings.append(
            (extension.lstrip("."),
             convert(source, target_filter, extension, outdir, profile)))

    column = text_width / 2.54 * 72
    print(f"=== column {column:.3f} pt (margins {margin}), "
          f"default paragraph style {size} ===")
    for name, path in spellings:
        pdf = convert(path, "pdf", ".pdf", outdir, profile)
        kept = os.path.join(outdir, f"{label}-{name}.pdf")
        os.replace(pdf, kept)

        rect = separator(kept)
        if rect is None:
            print(f"  {name:5s} no rule")
            continue
        x, y, w, h = rect
        notes = note_baseline(kept, 10.0)
        gap = "" if notes is None else f"  rule-to-note {(y + h) - notes:7.3f}"
        print(f"  {name:5s} width {w:8.3f} pt ({w / column * 100:5.1f}% of column, "
              f"{w / 72:.4f} in)  x {x:7.3f}  top {y + h:8.3f}{gap}")


def main():
    outdir = os.path.abspath(sys.argv[1])
    os.makedirs(outdir, exist_ok=True)
    profile = os.path.join(outdir, "profile")

    # Two text widths, so that "2 inches" can be told from "a proportion of the column",
    # then two default-paragraph font sizes at one width, because the *vertical* rule is
    # stated as a proportion of a height derived from that style and a single size cannot
    # tell a proportion from a constant either.
    run("wide", "2cm", 17.0, "12pt", outdir, profile)
    run("narrow", "6cm", 9.0, "12pt", outdir, profile)
    run("big", "2cm", 17.0, "24pt", outdir, profile)
    run("small", "2cm", 17.0, "8pt", outdir, profile)


if __name__ == "__main__":
    main()
