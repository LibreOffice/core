#!/usr/bin/env python3
"""Whether the two FAA Holdover Tables are wholly explained by the recorded word-parity divergence.

`TODO.word-parity.md`'s first entry is a `TOC \\t` switch voiding a built-in heading's direct
paragraph formatting: 26.2.4.2 discards it, Word honours it, this tree follows Word, and
`XRAY_OFFICE_LIBREOFFICE_QUIRKS=1` switches the reference's reading back on.  The entry says it costs
one page on `24-25_FAA_Holdover_Tables`.  Both Holdover documents head the words ink ranking, so
the question is whether the switch closes them outright or leaves a residual worth a round.
"""
import hashlib, os, pathlib, subprocess, sys

CLI = "/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
DOCUMENTS = [
    "/home/user/sample-files/words/pagination-001/docx/24-25_FAA_Holdover_Tables.docx",
    "/home/user/sample-files/words/pagination-001/docx/FAA 2025-26 Holdover Tables.docx",
]


def render(document, outdir, quirks):
    outdir.mkdir(parents=True, exist_ok=True)
    pdf = next(outdir.glob("*.pdf"), None)
    if pdf:
        return pdf
    environment = {**os.environ, "SOURCE_DATE_EPOCH": "0"}
    if quirks:
        environment["XRAY_OFFICE_LIBREOFFICE_QUIRKS"] = "1"
    subprocess.run([CLI, "render", str(document), "--outdir", str(outdir)], check=True,
                   capture_output=True, timeout=1800, env=environment)
    return next(outdir.glob("*.pdf"), None)


def load(pdf, pages):
    import fitz
    with fitz.open(pdf) as doc:
        text = "".join(page.get_text() for page in doc)
        ink = []
        for index in range(pages):
            if index >= doc.page_count:
                ink.append(0.0)
                continue
            pixmap = doc[index].get_pixmap(dpi=36, colorspace=fitz.csGRAY)
            ink.append(sum(255 - b for b in pixmap.samples) / (255.0 * len(pixmap.samples)))
        return doc.page_count, sum(c.isalnum() for c in text), ink


def main():
    root = pathlib.Path(sys.argv[1])
    import fitz
    print("pages\tquirkPages\trefPages\tglyphs\tquirkGlyphs\trefGlyphs\tink\tquirkInk\tdocument")
    for name in DOCUMENTS:
        document = pathlib.Path(name)
        key = hashlib.md5(name.encode()).hexdigest()[:16]
        reference = next(pathlib.Path(f"/home/user/r174-words/ref/{key}").glob("*.pdf"))
        with fitz.open(reference) as d:
            pages = d.page_count
        rp, rg, ri = load(reference, pages)
        plain = next(pathlib.Path(f"/home/user/r176-words/ours/{key}").glob("*.pdf"))
        op, og, oi = load(plain, pages)
        quirked = render(document, root / key, quirks=True)
        qp, qg, qi = load(quirked, pages)
        oink = sum(abs(a - b) for a, b in zip(oi, ri)) * 100
        qink = sum(abs(a - b) for a, b in zip(qi, ri)) * 100
        print(f"{op}\t{qp}\t{rp}\t{og}\t{qg}\t{rg}\t{oink:.2f}\t{qink:.2f}\t{document.name}",
              flush=True)


if __name__ == "__main__":
    main()
