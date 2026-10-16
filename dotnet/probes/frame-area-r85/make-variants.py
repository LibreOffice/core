#!/usr/bin/env python3
"""One-attribute variants of a corpus document's `wp:positionV/@relativeFrom`.

The fixtures in `make-relfrom.py` are all `wp:wrapNone`, and a wrap-through object escapes the
capture that `ImplAdjustVertRelPos` applies. So they measure the *origin* rule cleanly and say
nothing about the clamp — which is what decides the only three corpus documents that state a
margin band. Rewriting one attribute of a real document and rendering each variant through the
reference is the instrument that separates the two, because everything else about the page is
held fixed by construction.

On `008_Free_Genogram_Diagram_Template_Green_and_Yellow_Theme.docx`, whose one `wp:wrapSquare`
title box states `topMargin` with a 20.71 pt offset, 26.2.4.2 draws that box's text at

    as `page`        y  83.56   = page top + the offset
    as `margin`      y 155.56   = body top + the offset
    as `topMargin`   y 134.86   = neither -- the page-relative placement clamped to the body's top

The `-both` variants rewrite the VML fallback's `mso-position-vertical-relative` as well, and are
the control that says the reference reads the `mc:Choice` branch: they reproduce their `mc:Choice`
twins exactly.

Usage:  python3 make-variants.py <outdir> [source.docx]
"""
import sys, zipfile
from pathlib import Path

DEFAULT = ("/home/user/sample-files/words/chartset-012/docx/"
           "008_Free_Genogram_Diagram_Template_Green_and_Yellow_Theme_9017c1a8.docx")

VARIANTS = {
    "orig": [],
    "as-page": [(b'<wp:positionV relativeFrom="topMargin">',
                 b'<wp:positionV relativeFrom="page">')],
    "as-margin": [(b'<wp:positionV relativeFrom="topMargin">',
                   b'<wp:positionV relativeFrom="margin">')],
    "as-page-both": [(b'<wp:positionV relativeFrom="topMargin">',
                      b'<wp:positionV relativeFrom="page">'),
                     (b'mso-position-vertical-relative:top-margin-area',
                      b'mso-position-vertical-relative:page')],
    "as-margin-both": [(b'<wp:positionV relativeFrom="topMargin">',
                        b'<wp:positionV relativeFrom="margin">'),
                       (b'mso-position-vertical-relative:top-margin-area',
                        b'mso-position-vertical-relative:margin')],
}


def main():
    out = Path(sys.argv[1])
    src = Path(sys.argv[2]) if len(sys.argv) > 2 else Path(DEFAULT)
    out.mkdir(parents=True, exist_ok=True)
    for name, edits in VARIANTS.items():
        source = zipfile.ZipFile(src)
        with zipfile.ZipFile(out / f"{name}.docx", "w", zipfile.ZIP_DEFLATED) as z:
            for item in source.infolist():
                data = source.read(item.filename)
                if item.filename == "word/document.xml":
                    for a, b in edits:
                        data = data.replace(a, b)
                z.writestr(item, data)
        print("wrote", name + ".docx")


main()
