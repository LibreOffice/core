#!/usr/bin/env python3
"""Which slot decides whether LibreOffice draws a running head — measured on the real document.

Round 42 left this open. `UG.CAO.00133 Foreign Part 145 approvals - Language.docx` has five
sections; the reference draws its running head on **5 of 18 pages** and we draw it on all
eighteen. Sections 0 and 3 name a *default* header; sections 1, 2 and 4 name an *even* and a
*first* header and no default, and the parts they name are empty — a bare `<w:p/>`, which is what
Word writes into a slot the user never filled.

Round 42 authored six two-section shapes and LibreOffice inherited the first section's header in
all six, including the "names two empty ones" shape. That refutes "our inheritance rule is wrong"
and it does **not** explain the document. So this probe stops authoring and mutates the document
itself: one variable at a time, everything else held at whatever the real file says.

    header-slot-mutations.py /abs/scratch/dir

Each variant is converted with `soffice` and the running head is counted per page — a *presence*
per page, not a length, so the probe-style trap does not apply.
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

# The running head's own words. It is the only place in the document where this phrase is set on
# its own line, so counting the pages carrying it counts the pages carrying the head.
HEAD = "European Aviation Safety Agency"

SECTPR = re.compile(rb"<w:sectPr\b.*?</w:sectPr>", re.S)
HEADER_REF = re.compile(rb'<w:headerReference[^>]*/>')


def variants(document: bytes) -> dict[str, bytes]:
    """The mutations, each a single change to `word/document.xml`."""
    out: dict[str, bytes] = {"as-is": document}

    def rewrite(name, fn):
        pieces, last = [], 0
        for i, m in enumerate(SECTPR.finditer(document)):
            pieces.append(document[last:m.start()])
            pieces.append(fn(i, m.group(0)))
            last = m.end()
        pieces.append(document[last:])
        out[name] = b"".join(pieces)

    # Sections 1, 2 and 4 name only `even` and `first`. Strip those names and the section names
    # nothing at all — which is the shape §17.10.1 calls link-to-previous and the shape round 42's
    # `names-nothing` variant showed LibreOffice inheriting into.
    rewrite("no-refs-in-1-2-4",
            lambda i, s: HEADER_REF.sub(b"", s) if i in (1, 2, 4) else s)

    # The opposite direction: give those sections a *default* reference to section 0's own header
    # part (rId9). If the head then appears, nothing else about those sections is suppressing it.
    rewrite("default-added-to-1-2-4",
            lambda i, s: (s.replace(b"<w:headerReference",
                                    b'<w:headerReference w:type="default" r:id="rId9"/>'
                                    b"<w:headerReference", 1)
                          if i in (1, 2, 4) else s))

    # Keep the names and drop only the `even` ones, leaving `first`. And the mirror. Between them
    # these say whether one slot in particular is what stops the inheritance, or merely any slot.
    rewrite("only-first-in-1-2-4",
            lambda i, s: (re.sub(rb'<w:headerReference[^>]*w:type="even"[^>]*/>', b"", s)
                          if i in (1, 2, 4) else s))
    rewrite("only-even-in-1-2-4",
            lambda i, s: (re.sub(rb'<w:headerReference[^>]*w:type="first"[^>]*/>', b"", s)
                          if i in (1, 2, 4) else s))
    return out


def build(src: Path, dst: Path, document: bytes, headers: dict[str, bytes] | None = None) -> None:
    headers = headers or {}
    with zipfile.ZipFile(src) as zin, zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "word/document.xml":
                data = document
            elif item.filename in headers:
                data = headers[item.filename]
            zout.writestr(item, data)


def head_pages(pdf: Path) -> list[int]:
    out = subprocess.run(["pdftotext", "-layout", str(pdf), "-"],
                         capture_output=True, text=True).stdout
    return [i for i, page in enumerate(out.split("\f"), start=1) if HEAD in page]


def convert(docx: Path, outdir: Path, profile: Path) -> Path | None:
    shutil.rmtree(outdir, ignore_errors=True)
    outdir.mkdir(parents=True)
    subprocess.run(["soffice", f"-env:UserInstallation=file://{profile}", "--headless",
                    "--convert-to", "pdf", "--outdir", str(outdir), str(docx)],
                   capture_output=True, text=True, timeout=300)
    pdf = outdir / (docx.stem + ".pdf")
    return pdf if pdf.exists() else None


def main() -> int:
    out = Path(sys.argv[1] if len(sys.argv) > 1 else "/tmp/header-slot-mutations")
    out.mkdir(parents=True, exist_ok=True)
    profile = out / "prof"

    with zipfile.ZipFile(SOURCE) as zf:
        document = zf.read("word/document.xml")
        empty_header = zf.read("word/header2.xml")

    cases = [(name, doc, None) for name, doc in variants(document).items()]

    # One more, orthogonal to the section markup: leave every reference exactly as it is and put
    # *text* into the empty even/first parts. If a named-but-empty part is what LibreOffice reads
    # as "this section has a header, and it is blank", filling it makes that header visible and
    # says the emptiness was never the point — the naming was.
    filled = empty_header.replace(
        b"<w:p ", b"<w:p><w:r><w:t>MUTATEDSLOTTEXT</w:t></w:r></w:p><w:p ", 1)
    if filled == empty_header:  # the part may spell its paragraph `<w:p/>`
        filled = empty_header.replace(
            b"<w:p/>", b"<w:p><w:r><w:t>MUTATEDSLOTTEXT</w:t></w:r></w:p>", 1)
    cases.append(("empty-slots-filled", document,
                  {f"word/header{n}.xml": filled for n in (2, 3, 4, 5, 7, 8)}))

    print(f"{'variant':26s} {'pages':>5s}  head on")
    for name, doc, headers in cases:
        docx = out / f"{name}.docx"
        build(SOURCE, docx, doc, headers)
        pdf = convert(docx, out / name, profile)
        if pdf is None:
            print(f"{name:26s}      — no output")
            continue
        pages = head_pages(pdf)
        total = len(subprocess.run(["pdftotext", "-layout", str(pdf), "-"],
                                   capture_output=True, text=True).stdout.split("\f"))
        extra = ""
        if headers:
            slot = [i for i, page in enumerate(
                subprocess.run(["pdftotext", "-layout", str(pdf), "-"],
                               capture_output=True, text=True).stdout.split("\f"), start=1)
                if "MUTATEDSLOTTEXT" in page]
            extra = f"   filled slot on {slot}"
        print(f"{name:26s} {total:5d}  {pages}{extra}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
