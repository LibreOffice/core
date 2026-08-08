#!/usr/bin/env python3
"""Decks that separate the terms of the band *under* a chart's plot area.

    make-band-probe.py <outdir>

`chart-face-theme-minor.pptx` is the carrier because its plot area states its own fill
(`d9d9d9`), so the rectangle under study is a filled box in both renderings and can be read
straight off the PDF rather than inferred from gridlines. Everything varies one element of the
bottom stack at a time — the legend, the category axis' title, the category labels — so a
disagreement can be attributed to a term rather than to their sum.

Read LibreOffice's own answer out of `chart:coordinate-region` in its `odp` export as well as
off the ink: that element carries the *computed* plot rectangle excluding axes
(`SchXMLExportHelper_Impl::exportCoordinateRegion`, `xmloff/source/chart/SchXMLExport.cxx:2274`),
which is the quantity in question with none of the ink's rounding.
"""
import os, re, shutil, sys, zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = "/home/user/libreoffice-core/.claude/worktrees/slides-r29/dotnet/tests/corpus/features/chart-face-theme-minor.pptx"

CAT_TITLE = re.compile(r"(<c:catAx>.*?)<c:title>.*?</c:title>(.*?</c:catAx>)", re.S)
VAL_TITLE = re.compile(r"(<c:valAx>.*?)<c:title>.*?</c:title>(.*?</c:valAx>)", re.S)
MAIN_TITLE = re.compile(r"<c:title>.*?</c:title>\s*<c:autoTitleDeleted val=\"0\"/>", re.S)
LEGEND = re.compile(r"<c:legend>.*?</c:legend>", re.S)


def variant(text, *, legend, cattitle, valtitle, maintitle, catlabels, cats=None, labelsize=None):
    if not maintitle:
        text = MAIN_TITLE.sub('<c:autoTitleDeleted val="1"/>', text, count=1)
    if not cattitle:
        text = CAT_TITLE.sub(r"\1\2", text, count=1)
    if not valtitle:
        text = VAL_TITLE.sub(r"\1\2", text, count=1)
    if legend is None:
        text = LEGEND.sub("", text, count=1)
    else:
        text = text.replace('<c:legendPos val="r"/>', f'<c:legendPos val="{legend}"/>')
    if not catlabels:
        head, sep, tail = text.partition("<c:catAx>")
        tail = tail.replace('<c:tickLblPos val="nextTo"/>',
                            '<c:tickLblPos val="none"/>', 1)
        text = head + sep + tail
    if cats is not None:
        block = re.search(r"<c:cat>.*?</c:cat>", text, re.S).group(0)
        pts = "".join(f"<c:pt idx=\"{i}\"><c:v>{c}</c:v></c:pt>" for i, c in enumerate(cats))
        new = ('<c:cat><c:strRef><c:f>categories</c:f><c:strCache>'
               f'<c:ptCount val="{len(cats)}"/>{pts}</c:strCache></c:strRef></c:cat>')
        text = text.replace(block, new)
        # The value series must have as many points, or the axis is not the length under study.
        for f in ("0", "1"):
            vals = re.search(r"<c:val><c:numRef><c:f>" + f + r"</c:f>.*?</c:val>", text, re.S)
            base = [120, 95, 143, 168, 88, 132, 101, 121]
            pts = "".join(f"<c:pt idx=\"{i}\"><c:v>{base[i % 8]}</c:v></c:pt>"
                          for i in range(len(cats)))
            text = text.replace(vals.group(0),
                                f'<c:val><c:numRef><c:f>{f}</c:f><c:numCache>'
                                f'<c:formatCode>General</c:formatCode>'
                                f'<c:ptCount val="{len(cats)}"/>{pts}</c:numCache></c:numRef></c:val>')
    if labelsize is not None:
        head, sep, tail = text.partition("<c:catAx>")
        tail = tail.replace('sz="1000"', f'sz="{labelsize}"', 1)
        text = head + sep + tail
    return text


VARIANTS = {
    # name                legend cattitle valtitle maintitle catlabels
    "full-b":            dict(legend="b", cattitle=True,  valtitle=True,  maintitle=True,  catlabels=True),
    "nocattitle":        dict(legend="b", cattitle=False, valtitle=True,  maintitle=True,  catlabels=True),
    "noleg":             dict(legend=None, cattitle=True, valtitle=True,  maintitle=True,  catlabels=True),
    "bare":              dict(legend=None, cattitle=False, valtitle=True, maintitle=True,  catlabels=True),
    "bare-nolabels":     dict(legend=None, cattitle=False, valtitle=True, maintitle=True,  catlabels=False),
    "bare-notitle":      dict(legend=None, cattitle=False, valtitle=False, maintitle=False, catlabels=True),
    "bare-nothing":      dict(legend=None, cattitle=False, valtitle=False, maintitle=False, catlabels=False),
    "labels-14":         dict(legend=None, cattitle=False, valtitle=True, maintitle=True,  catlabels=True, labelsize=1400),
    "labels-07":         dict(legend=None, cattitle=False, valtitle=True, maintitle=True,  catlabels=True, labelsize=700),
    "cats-26":           dict(legend=None, cattitle=False, valtitle=True, maintitle=True,  catlabels=True,
                              cats=[f"{2000 + i // 4}-{i % 4 + 1}" for i in range(26)]),
    "cats-12":           dict(legend=None, cattitle=False, valtitle=True, maintitle=True,  catlabels=True,
                              cats=[f"{2000 + i // 4}-{i % 4 + 1}" for i in range(12)]),
}


def main():
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    for name, kw in VARIANTS.items():
        target = os.path.join(out, f"band-{name}.pptx")
        with zipfile.ZipFile(SOURCE) as src, \
                zipfile.ZipFile(target, "w", zipfile.ZIP_DEFLATED) as dst:
            for item in src.infolist():
                data = src.read(item.filename)
                if re.match(r"ppt/charts/chart\d+\.xml$", item.filename):
                    data = variant(data.decode("utf-8"), **kw).encode("utf-8")
                dst.writestr(item, data)
        print(target)
    return 0


if __name__ == "__main__":
    sys.exit(main())
