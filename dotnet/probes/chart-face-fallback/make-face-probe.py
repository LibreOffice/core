#!/usr/bin/env python3
"""One deck whose theme names a family nothing has, to read each engine's fallback out of a PDF.

`fc-match` is not ground truth for what LibreOffice resolves — the project's notes already say
so — and neither is either soffice on this container ground truth for the other. This builds the
smallest thing that answers the question: a chart deck whose theme minor face is
`ZzzNoSuchFamily`, so every piece of its chart text asks for a family no font database holds.
Render it through both binaries and read `pdffonts`.

    python3 make-face-probe.py <out dir>
    soffice --headless --convert-to pdf --outdir <out> <out>/face.pptx
    /opt/libreoffice26.2/program/soffice --headless --convert-to pdf --outdir <out2> <out>/face.pptx
    pdffonts <out>/face.pdf ; pdffonts <out2>/face.pdf
"""
import os, sys, zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = os.path.join(HERE, "..", "..", "tests", "corpus", "features",
                      "chart-face-theme-minor.pptx")


def main(out):
    os.makedirs(out, exist_ok=True)
    target = os.path.join(out, "face.pptx")
    with zipfile.ZipFile(SOURCE) as src, \
            zipfile.ZipFile(target, "w", zipfile.ZIP_DEFLATED) as dst:
        for item in src.infolist():
            data = src.read(item.filename)
            if item.filename == "ppt/theme/theme1.xml":
                data = data.decode("utf-8").replace(
                    'typeface="Liberation Mono"', 'typeface="ZzzNoSuchFamily"').encode("utf-8")
            dst.writestr(item, data)
    print(target)


if __name__ == "__main__":
    main(sys.argv[1])
