#!/usr/bin/env python3
"""The first span's corner and the lines drawn, for each fixture, from each renderer.

The box is at 72 pt across and 144 pt down of the page, so the left inset is `x - 72` and the
upper one is the first baseline's top less 144.  How many of the four lines survive says whether
the height was treated as fixed.
"""
import os, pathlib, subprocess, sys

CLI = "/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
SOFFICE = "/opt/libreoffice26.2/program/soffice"
WORDS = {"Alpha", "Bravo", "Charlie", "Delta"}


def read(pdf):
    import fitz
    found = []
    with fitz.open(pdf) as doc:
        for page in doc:
            for block in page.get_text("dict")["blocks"]:
                for line in block.get("lines", []):
                    for span in line["spans"]:
                        text = span["text"].strip()
                        if text in WORDS:
                            found.append((text, span["bbox"][0], span["bbox"][1]))
    return found


def main():
    fixtures = pathlib.Path(sys.argv[1])
    work = pathlib.Path(sys.argv[2])
    print("left\ttop\tlines\twords\tarm\trenderer")
    for docx in sorted(fixtures.glob("*.docx")):
        for label, command in (("reference", None), ("ours", None)):
            outdir = work / docx.stem / label
            outdir.mkdir(parents=True, exist_ok=True)
            pdf = next(outdir.glob("*.pdf"), None)
            if pdf is None:
                if label == "reference":
                    subprocess.run([SOFFICE, f"-env:UserInstallation=file://{outdir / 'p'}",
                                    "--headless", "--norestore", "--convert-to", "pdf",
                                    "--outdir", str(outdir), str(docx)],
                                   check=True, capture_output=True, timeout=300)
                else:
                    subprocess.run([CLI, "render", str(docx), "--outdir", str(outdir)],
                                   check=True, capture_output=True, timeout=300,
                                   env={**os.environ, "SOURCE_DATE_EPOCH": "0"})
                pdf = next(outdir.glob("*.pdf"), None)
            found = read(pdf) if pdf else []
            if not found:
                print(f"-\t-\t0\t\t{docx.stem}\t{label}")
                continue
            _, x, y = found[0]
            print(f"{x - 72:.2f}\t{y - 144:.2f}\t{len(found)}\t"
                  f"{','.join(t for t, _, _ in found)}\t{docx.stem}\t{label}", flush=True)


if __name__ == "__main__":
    main()
