#!/usr/bin/env python3
"""The ODF half of the table-cell first-baseline claim — `OdpSlideLayout.cs:302`.

Round 54 settled the OOXML half (`PptxSlideLayout.cs:763`) and said plainly that its probe did
not cover this one.  It cannot be settled the same way, and that is the finding: **the exported
fixture states the very attribute under test.**  `soffice --convert-to odp` writes
`style:font-independent-line-spacing="true"` onto every drawing cell it emits, so a single
round-tripped file measures the reference's own default and not ODF's.

So this renders a **discriminating pair**: the exported `.odp` as it comes, and a byte-identical
copy with that one attribute deleted from the cell style.  Six stated sizes each.

Needs `probes/slides-r54/make-cell-baseline-probe.py` beside it for the source deck.

    odp-cell-baseline.py <workdir>
"""
import os, re, shutil, subprocess, sys, zipfile

sys.path.insert(0, "/c/sandbox/workdir/wt-slides-r50/dotnet/research/probes/slides-r15")
import pdfops  # noqa: E402
from pdfops import objects, pages  # noqa: E402

SIZES = [10, 12, 18, 24, 32, 40]
TOP_PT = 100.0                     # the cell's top edge, from the slide's top
PAGE_PT = 540.0
ATTR = ' style:font-independent-line-spacing="true"'
MAKER = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                     "..", "slides-r54", "make-cell-baseline-probe.py")


def soffice(target, path, outdir):
    subprocess.run(
        ["soffice", f"-env:UserInstallation=file://{os.path.abspath(outdir)}/prof",
         "--headless", "--convert-to", target, "--outdir", outdir, path],
        check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
        env={**os.environ, "SOURCE_DATE_EPOCH": "1700000000", "TZ": "UTC"})


def strip(src, dst):
    zin = zipfile.ZipFile(src)
    removed = 0
    with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "content.xml":
                text = data.decode("utf-8")
                removed = text.count(ATTR)
                data = text.replace(ATTR, "").encode("utf-8")
            zout.writestr(item, data)
    return removed


def ascents(pdf):
    """First-baseline ascent below the cell's top edge, per slide, in points."""
    data = open(pdf, "rb").read()
    objs = objects(data)
    out = {}
    for pno, page in enumerate(pages(data, objs), 1):
        stream = pdfops.content(data, objs, page)
        pen = None
        size = None
        for m in re.finditer(
                rb"(-?[\d.]+) (-?[\d.]+) Td|/F\d+ ([\d.]+) Tf|\((?:\\.|[^\\()])*\)\s*Tj"
                rb"|\[[^\]]*\]\s*TJ", stream):
            if m.group(3):
                size = float(m.group(3))
            elif m.group(1):
                pen = float(m.group(2))
            # `pen <= PAGE_PT - TOP_PT` is not decoration: the deck carries a "spacer" shape
            # near the top of every slide, and on three of the six its run is close enough in
            # size to be picked instead of the cell's, which reads as a negative ascent.
            elif pen is not None and size is not None \
                    and abs(size - SIZES[pno - 1]) < 1.5 \
                    and pen <= PAGE_PT - TOP_PT + 0.5:
                out[SIZES[pno - 1]] = round(PAGE_PT - TOP_PT - pen, 3)
                break
    return out


if __name__ == "__main__":
    work = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else "odp-cell")
    os.makedirs(work, exist_ok=True)
    pptx = os.path.join(work, "cellprobe.pptx")
    subprocess.run([sys.executable, os.path.abspath(MAKER), pptx], check=True)
    soffice("odp", pptx, work)

    odp = os.path.join(work, "cellprobe.odp")
    for name in ("stated", "absent"):
        os.makedirs(os.path.join(work, name), exist_ok=True)
    stated = os.path.join(work, "stated", "cell.odp")
    absent = os.path.join(work, "absent", "cell.odp")
    shutil.copy(odp, stated)
    n = strip(odp, absent)
    print(f"the exporter wrote {n} `font-independent-line-spacing` attributes")

    for name, path in (("stated", stated), ("absent", absent)):
        soffice("pdf", path, os.path.join(work, name))

    a = ascents(os.path.join(work, "stated", "cell.pdf"))
    b = ascents(os.path.join(work, "absent", "cell.pdf"))
    print(f"{'size':>5} {'stated true':>12} {'em':>7} | {'absent':>9} {'em':>7}")
    for s in SIZES:
        if s in a and s in b:
            print(f"{s:5d} {a[s]:12.3f} {a[s] / s:7.4f} | {b[s]:9.3f} {b[s] / s:7.4f}")
