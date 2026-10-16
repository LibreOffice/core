#!/usr/bin/env python3
"""Bisect the real document down to the smallest thing that stops the header being inherited.

Two measurements that both reproduce and disagree:

* `header-link-to-previous.py` (round 42) and `header-inherit-content-shape.py` (round 43) author
  a two-section DOCX and LibreOffice puts the first section's header on the second section's page
  in **all twelve** shapes between them — naming nothing, naming empty first/even slots, and with
  the header holding text, a table, a nested table, an image or a table with an image in it.
* `header-slot-mutations.py` mutates `UG.CAO.00133 … Language.docx` itself and LibreOffice draws
  **no** header on sections 1, 2 and 4 — even after every `w:headerReference` in them is deleted.

So the variable is somewhere else in that file. This walks from the real document towards the
authored one, replacing one part at a time, and prints whether page 2 carries the head.

    header-inherit-bisect.py /abs/scratch/dir

Every variant keeps the real `word/header1.xml` unless it says otherwise, so "head on page 2"
always means the same mark.
"""
from __future__ import annotations

import re
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

SOURCE = Path("/workspace/sample-files/words/batch-014/docx/"
              "UG.CAO.00133 Foreign Part 145 approvals - Language.docx")

# Header-only, checked: absent from every footer part and from `word/document.xml`.
HEAD = "Approval Date"

SECTPR = re.compile(rb"<w:sectPr\b.*?</w:sectPr>", re.S)
BODY = re.compile(rb"<w:body>(.*)</w:body>", re.S)

MINIMAL_SETTINGS = (
    b'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    b'<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>')


def sections(document: bytes):
    return [m.group(0) for m in SECTPR.finditer(document)]


def two_section_body(document: bytes) -> bytes:
    """The real document cut to its first two sections, section 1's `sectPr` moved to the body.

    Section 0 keeps its own paragraph-level `sectPr` — the shape the file actually has. Section 1
    becomes the final section, so its properties are stated exactly once, at body level.
    """
    body = BODY.search(document).group(1)
    sects = sections(document)
    # The paragraph carrying section 0's sectPr, and everything up to the next sectPr's paragraph.
    end0 = body.find(sects[0]) + len(sects[0])
    close = body.find(b"</w:p>", end0) + len(b"</w:p>")
    head = body[:close]
    return (b"<w:body>" + head
            + b"<w:p><w:r><w:t>SECONDSECTIONBODY</w:t></w:r></w:p>"
            + sects[1] + b"</w:body>")


def strip_last_section_headers(document: bytes) -> bytes:
    """Delete every `w:headerReference` from the document's final `w:sectPr`."""
    last = sections(document)[-1]
    return document.replace(last, re.sub(rb"<w:headerReference[^>]*/>", b"", last))


PLAIN_HEADER = (
    b'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    b'<w:hdr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
    b"<w:p><w:r><w:t>Approval Date PLAINHEAD</w:t></w:r></w:p></w:hdr>")


def drop(document: bytes, pattern: bytes) -> bytes:
    return re.sub(pattern, b"", document, flags=re.S)


def untable(header: bytes) -> bytes:
    """The same header with every table taken out and its paragraphs left in document order.

    The three tables are nested, so the wrapper elements are simply deleted rather than matched as
    a tree — every `w:p` survives, in order, with its own properties.
    """
    for tag in (rb"</?w:tbl>", rb"<w:tblPr>.*?</w:tblPr>", rb"<w:tblGrid>.*?</w:tblGrid>",
                rb"<w:tr\b[^>]*>", rb"</w:tr>", rb"<w:tc>", rb"</w:tc>",
                rb"<w:tcPr>.*?</w:tcPr>"):
        header = re.sub(tag, b"", header, flags=re.S)
    return header


def plain_first_body(document: bytes) -> bytes:
    """Everything section 0 draws, replaced by one paragraph; its `sectPr` kept exactly."""
    sect0 = sections(document)[0]
    body = BODY.search(document).group(1)
    end = body.find(sect0) + len(sect0)
    close = body.find(b"</w:p>", end) + len(b"</w:p>")
    kept = (b"<w:p><w:pPr>" + sect0 + b"</w:pPr><w:r><w:t>FIRSTSECTIONBODY</w:t></w:r></w:p>")
    return document.replace(body[:close], kept)


def repack(src: Path, dst: Path, replace: dict[str, bytes]) -> None:
    with zipfile.ZipFile(src) as zin, zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = replace.get(item.filename, zin.read(item.filename))
            zout.writestr(item, data)


def head_pages(pdf: Path) -> tuple[int, list[int]]:
    out = subprocess.run(["pdftotext", "-layout", str(pdf), "-"],
                         capture_output=True, text=True).stdout
    pages = out.split("\f")
    return len(pages), [i for i, p in enumerate(pages, 1) if HEAD in p]


def main() -> int:
    out = Path(sys.argv[1] if len(sys.argv) > 1 else "/tmp/header-inherit-bisect")
    out.mkdir(parents=True, exist_ok=True)
    profile = out / "prof"

    with zipfile.ZipFile(SOURCE) as zf:
        document = zf.read("word/document.xml")
        names = set(zf.namelist())

    doc2 = re.sub(rb"<w:body>.*</w:body>", lambda _m: two_section_body(document), document,
                  count=1, flags=re.S)

    cases: list[tuple[str, dict[str, bytes]]] = [
        ("real-as-is", {}),
        ("cut-to-two-sections", {"word/document.xml": doc2}),
        ("two-sections-minimal-settings",
         {"word/document.xml": doc2, "word/settings.xml": MINIMAL_SETTINGS}),
        # Section 1 names an empty `even` and an empty `first` header. Deleting both names — from
        # the *final* sectPr only, which is section 1's — makes it the shape §17.10.1 calls
        # link-to-previous outright.
        ("two-sections-no-refs", {"word/document.xml": strip_last_section_headers(doc2)}),
        # The real header is a table holding a logo and six fields. Replacing it with one line of
        # text keeps every other byte of the document.
        ("two-sections-plain-header", {"word/document.xml": doc2, "word/header1.xml": PLAIN_HEADER}),
        # Section 0 names a footer and section 1 names none, which the authored probes never do —
        # and `copyHeaderFooter` reads the header and footer link flags into one decision.
        ("two-sections-no-footer-ref",
         {"word/document.xml": drop(doc2, rb'<w:footerReference[^>]*/>')}),
        # The rest of what the real sectPr carries and the authored one does not.
        ("two-sections-no-cols-noendnote",
         {"word/document.xml": drop(doc2, rb"<w:cols[^>]*/>|<w:noEndnote\s*/>")}),
        # Section 0's own body, replaced by one paragraph.
        ("two-sections-plain-first-body", {"word/document.xml": plain_first_body(doc2)}),
    ]

    # Stage two, entered once stage one says the header's *content* is the variable: bisect the
    # real `word/header1.xml` rather than the document.
    with zipfile.ZipFile(SOURCE) as zf:
        header = zf.read("word/header1.xml")
    cases += [
        ("hdr-no-drawing",
         {"word/document.xml": doc2,
          "word/header1.xml": drop(header, rb"<w:drawing>.*?</w:drawing>")}),
        ("hdr-no-tables",
         {"word/document.xml": doc2, "word/header1.xml": untable(header)}),
        ("hdr-no-tables-no-drawing",
         {"word/document.xml": doc2,
          "word/header1.xml": untable(drop(header, rb"<w:drawing>.*?</w:drawing>"))}),
        # The header's last element is `</w:tbl>`: Word wrote no paragraph after the table. This is
        # the one byte-level difference from every authored table header, all of which inherit.
        ("hdr-trailing-paragraph",
         {"word/document.xml": doc2,
          "word/header1.xml": header.replace(b"</w:hdr>", b"<w:p/></w:hdr>")}),
        # And a paragraph at the *front* instead. The authored probe says a header holding a
        # paragraph and then a table copies fine, so what the copy cannot survive is a header with
        # no top-level paragraph at all — either end rescues it.
        ("hdr-leading-paragraph",
         {"word/document.xml": doc2,
          "word/header1.xml": re.sub(rb"(<w:hdr\b[^>]*>)", rb"\1<w:p/>", header, count=1)}),
    ]

    if "word/styles.xml" in names:
        cases.append(("two-sections-minimal-styles", {
            "word/document.xml": doc2,
            "word/styles.xml": (
                b'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                b'<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
                b"<w:docDefaults><w:rPrDefault><w:rPr>"
                b'<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
                b'<w:sz w:val="22"/></w:rPr></w:rPrDefault></w:docDefaults></w:styles>')}))

    print(f"{'variant':32s} {'pages':>5s}  head on")
    for name, replace in cases:
        docx = out / f"{name}.docx"
        repack(SOURCE, docx, replace)
        pdfdir = out / name
        shutil.rmtree(pdfdir, ignore_errors=True)
        pdfdir.mkdir()
        subprocess.run(["soffice", f"-env:UserInstallation=file://{profile}", "--headless",
                        "--convert-to", "pdf", "--outdir", str(pdfdir), str(docx)],
                       capture_output=True, text=True, timeout=300)
        pdf = pdfdir / f"{name}.pdf"
        if not pdf.exists():
            print(f"{name:32s}     — no output")
            continue
        total, pages = head_pages(pdf)
        print(f"{name:32s} {total:5d}  {pages}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
