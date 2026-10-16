#!/usr/bin/env python3
"""One-attribute variants of `02_mcar`'s TOC styles, to find what decides the wrap.

`TOC1`..`TOC9` put a right stop with a dot leader at 9360 twips -- the frame's right edge -- while
stating `w:ind w:right="720"`, so the stop is 720 twips *past* the paragraph's own line end.  This
tree wraps a dozen entries' page numbers onto a second line where 26.2.4.2 keeps them on the line
by shrinking the leader, and one extra page of table of contents is the whole of that document's
313 against 312.

Each variant changes exactly one thing in `word/styles.xml` and nothing else.
"""
import pathlib, re, shutil, subprocess, sys, zipfile

SOURCE = pathlib.Path(
    "/home/user/sample-files/words/metrics-001/docx/02_mcar_part-2_and_IS_v2.10.docx")
SOFFICE = "/opt/libreoffice26.2/program/soffice"
CLI = "/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"

TOC = re.compile(r'(<w:style [^>]*w:styleId="TOC\d".*?</w:style>)', re.S)


def rewrite(name, styles):
    """Return `word/styles.xml` with one attribute of every TOC style changed."""
    def one(match):
        body = match.group(1)
        if name == "no-right-indent":
            return re.sub(r'(<w:ind\b[^>]*?)\s*w:right="\d+"', r'\1', body)
        if name == "stop-at-line-end":
            # Move the right stop back to where the paragraph's own line ends.
            def move(m):
                return m.group(0).replace('w:pos="9360"', 'w:pos="8640"')
            return re.sub(r'<w:tab w:val="right"[^>]*/>', move, body)
        if name == "no-leader":
            return re.sub(r'(<w:tab w:val="right")\s*w:leader="dot"', r'\1', body)
        return body

    return TOC.sub(one, styles)


ARMS = ["as-authored", "no-right-indent", "stop-at-line-end", "no-leader"]


def build(name, out):
    target = out / f"{name}.docx"
    with zipfile.ZipFile(SOURCE) as zin, zipfile.ZipFile(target, "w", zipfile.ZIP_DEFLATED) as zo:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "word/styles.xml" and name != "as-authored":
                data = rewrite(name, data.decode("utf-8")).encode("utf-8")
            zo.writestr(item, data)
    return target


def render(document, outdir, reference):
    outdir.mkdir(parents=True, exist_ok=True)
    pdf = next(outdir.glob("*.pdf"), None)
    if pdf:
        return pdf
    if reference:
        subprocess.run([SOFFICE, f"-env:UserInstallation=file://{outdir / 'p'}", "--headless",
                        "--norestore", "--convert-to", "pdf", "--outdir", str(outdir),
                        str(document)], check=True, capture_output=True, timeout=900)
    else:
        import os
        subprocess.run([CLI, "render", str(document), "--outdir", str(outdir)], check=True,
                       capture_output=True, timeout=900,
                       env={**os.environ, "SOURCE_DATE_EPOCH": "0"})
    return next(outdir.glob("*.pdf"), None)


def measure(pdf):
    """Total pages, and how many pages the front matter takes before arabic page 1."""
    import fitz, re as regex
    with fitz.open(pdf) as doc:
        body = None
        for number, page in enumerate(doc):
            text = regex.sub(r"\s+", " ", page.get_text())
            if "PART 2 – PERSONNEL LICENSING" in text.upper() and "2.1 GENERAL" in text.upper():
                body = number + 1
                break
        return doc.page_count, body


def main():
    out = pathlib.Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    print("arm\toursPages\trefPages\toursBodyAt\trefBodyAt")
    for name in ARMS:
        document = build(name, out)
        a = render(document, out / name / "ours", reference=False)
        b = render(document, out / name / "ref", reference=True)
        op, ob = measure(a)
        rp, rb = measure(b)
        print(f"{name}\t{op}\t{rp}\t{ob}\t{rb}", flush=True)


if __name__ == "__main__":
    main()
