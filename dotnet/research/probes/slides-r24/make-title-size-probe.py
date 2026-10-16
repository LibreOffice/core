#!/usr/bin/env python3
"""Restate one corpus deck's chart title size, so the reference can be asked what it draws.

`Demick_JetBlue.pptx` states no `sz` and no `b` anywhere in any of its five chart parts, which
makes it the corpus's cleanest reading of the OOXML auto-text defaults — and the cleanest
carrier for a *stated* size, because everything else on the page stays put while one number
moves. Rewriting the one `<a:defRPr/>` inside `c:title/c:tx/c:rich` leaves the file valid and
leaves the axes, the legend and the slide's own text at their original sizes, which is what
makes them a control: they draw 9.889 pt and 18 pt in every variant below.

    python3 make-title-size-probe.py /workspace/sample-files/slides/batch-017/pptx/Demick_JetBlue.pptx out
    soffice --headless --convert-to pdf --outdir out/ref out/jb-*.pptx

Measured this way at 7, 10, 14, 18, 30 and 40 points — see TODO.md, "the chart's own text
scale" — the reference draws 6.987, 9.889, 13.889, 17.890, 29.807 and 39.808. The residual is
neither a constant offset nor a constant factor, which is what this probe exists to have shown.
"""
import re
import sys
import zipfile


def restate(source: str, out: str, hundredths: int) -> str:
    """Writes a copy of *source* whose first chart part states *hundredths* on its title."""
    path = f"{out}/jb-{hundredths}.pptx"
    with zipfile.ZipFile(source) as zin, \
            zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "ppt/charts/chart1.xml":
                text = data.decode("utf-8")
                found = re.search(r"<c:title>.*?</c:title>", text, re.S)
                title = found.group(0)
                # One a:pPr, replaced rather than added: two of them in one a:p is invalid and
                # the importer's answer to invalid markup is not what is under study here.
                if title.count("<a:pPr><a:defRPr/></a:pPr>") != 1:
                    raise SystemExit("the title is not the shape this probe rewrites")
                title = title.replace(
                    "<a:pPr><a:defRPr/></a:pPr>",
                    f'<a:pPr><a:defRPr sz="{hundredths}"/></a:pPr>')
                data = (text[:found.start()] + title + text[found.end():]).encode("utf-8")
            zout.writestr(item, data)
    return path


if __name__ == "__main__":
    src, outdir = sys.argv[1], sys.argv[2]
    for size in (700, 1400, 1800, 3000, 4000):
        print(restate(src, outdir, size))
