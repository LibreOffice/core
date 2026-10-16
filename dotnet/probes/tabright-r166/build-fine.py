#!/usr/bin/env python3
"""The same arm at a one-point resolution, because the coarse sweep cannot discriminate.

`build.py` steps the title by two characters, which is 16.28 pt of the reference's own
advance, while its right-indent steps are 6.5 to 9 pt. So the threshold is located to
±16 pt in a variable that moves in 9 pt steps, and *no* rule can be told from another on
it — which is what made `results.md`'s first two arithmetic readings disagree.

Here the title is fixed and the paragraph's right indent is swept in 20-twip (1 pt) steps,
so the boundary edge is located to a point for each of three title lengths.
"""
import os, sys, zipfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from build import CT, RELS, DRELS, SETTINGS, STYLES, para

HERE = os.path.dirname(os.path.abspath(__file__))
TITLES = (44, 50, 56)
RIGHTS = tuple(range(0, 1240, 20))


def build(path):
    body = []
    for n in TITLES:
        for right in RIGHTS:
            body.append(para(right, "A" * n, f"r{right}n{n}"))
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
           '<w:body>' + "".join(body) +
           '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
           '<w:pgMar w:top="1080" w:right="1440" w:bottom="1080" w:left="1440"'
           ' w:header="432" w:footer="432"/></w:sectPr></w:body></w:document>')
    if os.path.exists(path):
        os.remove(path)
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/_rels/document.xml.rels", DRELS)
        z.writestr("word/settings.xml", SETTINGS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/document.xml", doc)


if __name__ == "__main__":
    out = os.path.join(HERE, "toc-fine.docx")
    build(out)
    print("built", out, len(TITLES) * len(RIGHTS), "arms")
