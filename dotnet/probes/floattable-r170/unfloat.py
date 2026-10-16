#!/usr/bin/env python3
"""What a positioned table's wrap is worth, measured at the reference and not at us.

`Paginator.PlaceFloatedTable` leaves a `w:tblpPr` table with room beside it in the flow, so
this tree draws the following prose *under* the whole table where 26.2.4.2 wraps it level with
the first row.  Implementing the wrap is an architectural change -- obstacles are keyed by page
before pagination places a floated table -- so the prize is worth measuring first.

The instrument is the reference's own two answers.  Strip every `w:tblpPr` from the document and
the reference lays that table in the flow, which is exactly what this tree does today; render
both and the difference between them is the whole of what a wrap implementation could buy, with
no renderer of ours in the loop and no tolerance anywhere in it.
"""
import pathlib, re, shutil, subprocess, sys, tempfile, zipfile

SOFFICE = "/opt/libreoffice26.2/program/soffice"
W = "{http://schemas.openxmlformats.org/wordprocessingml/2006/main}"
TBLPPR = re.compile(rb"<w:tblpPr\b[^>]*/>|<w:tblpPr\b.*?</w:tblpPr>", re.S)


def unfloat(source: pathlib.Path, target: pathlib.Path) -> int:
    """Copy the package with every `w:tblpPr` removed; answer how many were removed."""
    removed = 0
    with zipfile.ZipFile(source) as zin, zipfile.ZipFile(target, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename.endswith(".xml") and b"tblpPr" in data:
                data, n = TBLPPR.subn(b"", data)
                removed += n
            zout.writestr(item, data)
    return removed


def render(document: pathlib.Path, outdir: pathlib.Path) -> pathlib.Path | None:
    outdir.mkdir(parents=True, exist_ok=True)
    profile = outdir / "profile"
    try:
        subprocess.run(
            [SOFFICE, f"-env:UserInstallation=file://{profile}", "--headless", "--norestore",
             "--convert-to", "pdf", "--outdir", str(outdir), str(document)],
            check=True, capture_output=True, timeout=600)
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired):
        return None
    pdfs = list(outdir.glob("*.pdf"))
    return pdfs[0] if pdfs else None


def score(pdf: pathlib.Path) -> tuple[int, int]:
    """Pages, and alphanumeric characters -- the gate's own two columns."""
    import fitz
    with fitz.open(pdf) as doc:
        text = "".join(page.get_text() for page in doc)
        return doc.page_count, sum(c.isalnum() for c in text)


def main() -> None:
    paths = [pathlib.Path(line.strip()) for line in open(sys.argv[1]) if line.strip()]
    root = pathlib.Path(sys.argv[2])
    root.mkdir(parents=True, exist_ok=True)
    print("removed\tfloatPages\tflowPages\tfloatGlyphs\tflowGlyphs\tdocument")
    for path in paths:
        work = root / re.sub(r"\W+", "-", path.stem)[:60]
        work.mkdir(parents=True, exist_ok=True)
        flat = work / path.name
        shutil.copy(path, flat)
        plain = work / ("unfloated-" + path.name)
        removed = unfloat(path, plain)
        a = render(flat, work / "float")
        b = render(plain, work / "flow")
        if a is None or b is None:
            print(f"{removed}\tFAILED\tFAILED\t-\t-\t{path.name}")
            continue
        ap, ag = score(a)
        bp, bg = score(b)
        mark = " *" if (ap, ag) != (bp, bg) else ""
        print(f"{removed}\t{ap}\t{bp}\t{ag}\t{bg}\t{path.name}{mark}", flush=True)


if __name__ == "__main__":
    main()
