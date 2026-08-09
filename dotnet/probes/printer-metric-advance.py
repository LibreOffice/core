#!/usr/bin/env python3
"""Does LibreOffice quantise a *glyph advance* on the printer's pixel grid?

The words track already knows that a document asking for printer metrics —
`WW8Dop::fUsePrinterMetrics`, which becomes `!USE_VIRTUAL_DEVICE` in
`sw/source/filter/ww8/ww8par.cxx`:2008 — has its **vertical** metrics rounded through a
300 dpi device grid: `MetricGrid` in `Paperless.Text/Fonts/LineSpacing.cs`, fitted against
LibreOffice's own line pitches on two corpus documents.

What was never measured is whether the same grid reaches the *horizontal* direction.
`GenericSalLayout::LayoutText` rounds every glyph advance to whole device pixels unless
subpixel positioning is on (`vcl/source/gdi/CommonSalLayout.cxx`:826-831), and 1/300 in is
0.24 pt — the right order for what several printer-metric documents are missing.

The corpus cannot answer it: eight of the 200 words documents set the flag and all eight
are real documents where a hundred other things also differ. So this authors the pair.

## The instrument

Two flat-ODF files identical in every byte except

    <config:config-item config:name="PrinterIndependentLayout" config:type="string">…</config:config-item>

`disabled` is the printer reference device; `high-resolution` is the 8640 dpi virtual one.
`xmloff/source/core/DocumentSettingsContext.cxx`:493-506 maps the strings, and
`SwXDocumentSettings` (`sw/source/uibase/uno/SwXDocumentSettings.cxx`:1454-1462) shows it is
the same `USE_VIRTUAL_DEVICE` the WW8 importer sets from the Dop bit. So this is the DOC
document's own switch, reachable in a file we can author.

Each body paragraph is `<glyph>×N` followed by a **red** one-character marker in the same
face and size. A colour change forces its own text show, so the marker's x is the running
advance of the N glyphs, read straight out of the content stream. Repeating one glyph
removes kerning from the question entirely.

    width(N) = x(marker) − x(text)

and the three candidate rules are sharply different at N = 64:

    exact        N · adv · size / upem
    per-glyph    N · round(adv · px_em / upem) device pixels, back to length
    whole-string round(N · adv · px_em / upem) device pixels, back to length

where `px_em = round(size / 72 · dpi)` is the em as an integer-pixel device can set it.

## The control that has to pass first

If the setting were ignored, every width would agree and the honest reading of that is
"the file did not switch", not "advances are not quantised". So the run also measures
**baseline pitch** on both files and checks it against `MetricGrid`'s vertical rule — a
quantity already known to differ between the two devices. A run whose pitch control does
not separate is reported as inconclusive and nothing else in it may be believed.

The probe measures a *length*, so it names its face explicitly in the styles and confirms
with the PDF's own font list that nothing was substituted.

    printer-metric-advance.py --outdir /abs/scratch/pm
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
import zlib
from dataclasses import dataclass

# Faces, sizes and glyph counts. One repeated glyph per row: no kerning pairs, so the
# expected width is exactly N advances and every model below is a closed form.
FACES = [("Liberation Serif", "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf"),
         ("Liberation Sans", "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")]
SIZES = [9.0, 10.0, 11.0, 12.0]
GLYPHS = ["n", "i", "M"]
COUNTS = [1, 4, 16, 64]

DPI_CANDIDATES = [300, 600, 720, 1440]

FODT = """<?xml version="1.0" encoding="UTF-8"?>
<office:document
 xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.text">
 <office:settings>
  <config:config-item-set config:name="ooo:configuration-settings">
   <config:config-item config:name="PrinterIndependentLayout" config:type="string">{layout}</config:config-item>
  </config:config-item-set>
 </office:settings>
 <office:font-face-decls>
{faces}
 </office:font-face-decls>
 <office:automatic-styles>
{styles}
  <style:page-layout style:name="pm1">
   <style:page-layout-properties fo:page-width="21cm" fo:page-height="29.7cm"
     fo:margin-top="1cm" fo:margin-bottom="1cm" fo:margin-left="1cm" fo:margin-right="1cm"/>
  </style:page-layout>
 </office:automatic-styles>
 <office:master-styles>
  <style:master-page style:name="Standard" style:page-layout-name="pm1"/>
 </office:master-styles>
 <office:body>
  <office:text>
{body}
  </office:text>
 </office:body>
</office:document>
"""


@dataclass(frozen=True)
class Row:
    face: str
    size: float
    glyph: str
    count: int


def rows() -> list[Row]:
    out = []
    for face, _ in FACES:
        for size in SIZES:
            for glyph in GLYPHS:
                for n in COUNTS:
                    out.append(Row(face, size, glyph, n))
    return out


def build_fodt(layout: str) -> str:
    faces = "\n".join(
        f'  <style:font-face style:name="F{i}" svg:font-family="{name}" style:font-family-generic="roman"/>'
        for i, (name, _) in enumerate(FACES))

    styles, body = [], []
    face_index = {name: i for i, (name, _) in enumerate(FACES)}
    for j, r in enumerate(rows()):
        fi = face_index[r.face]
        styles.append(
            f'  <style:style style:name="P{j}" style:family="paragraph">'
            f'<style:paragraph-properties fo:margin-top="0cm" fo:margin-bottom="0cm" '
            f'fo:line-height="100%" fo:text-align="start"/>'
            f'<style:text-properties style:font-name="F{fi}" fo:font-size="{r.size}pt" '
            f'fo:language="en" fo:country="GB" fo:color="#000000"/></style:style>')
        styles.append(
            f'  <style:style style:name="T{j}" style:family="text">'
            f'<style:text-properties style:font-name="F{fi}" fo:font-size="{r.size}pt" '
            f'fo:color="#ff0000"/></style:style>')
        body.append(
            f'   <text:p text:style-name="P{j}">{r.glyph * r.count}'
            f'<text:span text:style-name="T{j}">|</text:span></text:p>')

    return FODT.format(layout=layout, faces=faces,
                       styles="\n".join(styles), body="\n".join(body))


# ---------------------------------------------------------------- PDF reading

def page_streams(pdf: bytes) -> list[bytes]:
    """Every uncompressed content stream in the file, in file order."""
    out = []
    for m in re.finditer(rb"stream\r?\n", pdf):
        start = m.end()
        end = pdf.find(b"endstream", start)
        if end < 0:
            continue
        raw = pdf[start:end]
        try:
            out.append(zlib.decompress(raw))
        except zlib.error:
            out.append(raw)
    return out


TOKEN = re.compile(rb"([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+Tm"
                   rb"|(\d+(?:\.\d+)?)\s+(\d+(?:\.\d+)?)\s+(\d+(?:\.\d+)?)\s+rg"
                   rb"|\[(.*?)\]\s*TJ"
                   rb"|\((?:[^()\\]|\\.)*\)\s*Tj")


@dataclass
class Show:
    x: float
    y: float
    red: bool


def shows(stream: bytes) -> list[Show]:
    """Text shows with their text-matrix origin and whether the fill colour is pure red."""
    out: list[Show] = []
    red = False
    x = y = 0.0
    for m in TOKEN.finditer(stream):
        if m.group(1) is not None:
            x, y = float(m.group(5)), float(m.group(6))
        elif m.group(7) is not None:
            r, g, b = float(m.group(7)), float(m.group(8)), float(m.group(9))
            red = r > 0.9 and g < 0.1 and b < 0.1
        else:
            out.append(Show(x, y, red))
    return out


def convert(src: str, outdir: str, tag: str) -> str:
    profile = os.path.join(outdir, f"prof-{tag}")
    cmd = ["soffice", "--headless", f"-env:UserInstallation=file://{profile}",
           "--convert-to", "pdf", "--outdir", outdir, src]
    subprocess.run(cmd, check=False, capture_output=True, timeout=300)
    pdf = os.path.join(outdir, os.path.splitext(os.path.basename(src))[0] + ".pdf")
    if not os.path.exists(pdf):
        sys.exit(f"soffice produced no PDF for {src}")
    return pdf


# ---------------------------------------------------------------- font metrics

def advance(path: str, glyph: str) -> tuple[int, int]:
    from fontTools.ttLib import TTFont
    font = TTFont(path, lazy=True)
    upem = font["head"].unitsPerEm
    name = font.getBestCmap()[ord(glyph)]
    adv = font["hmtx"][name][0]
    font.close()
    return adv, upem


def model_exact(adv: int, upem: int, size: float, n: int) -> float:
    return n * adv * size / upem


def model_per_glyph(adv: int, upem: int, size: float, n: int, dpi: int) -> float:
    px_em = round(size / 72.0 * dpi)
    return n * round(adv * px_em / upem) * 72.0 / dpi


def model_whole(adv: int, upem: int, size: float, n: int, dpi: int) -> float:
    px_em = round(size / 72.0 * dpi)
    return round(n * adv * px_em / upem) * 72.0 / dpi


def model_exact_em(adv: int, upem: int, size: float, n: int, dpi: int) -> float:
    """Exact advances, but at the em size an integer-pixel device can set."""
    px_em = round(size / 72.0 * dpi)
    return n * adv * (px_em * 72.0 / dpi) / upem


# ---------------------------------------------------------------- main

def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--outdir", required=True, help="absolute scratch directory")
    ap.add_argument("--keep", action="store_true", help="keep the generated files")
    args = ap.parse_args()

    if not os.path.isabs(args.outdir):
        sys.exit("--outdir must be absolute")
    os.makedirs(args.outdir, exist_ok=True)

    metrics = {}
    for face, path in FACES:
        for g in GLYPHS:
            metrics[(face, g)] = advance(path, g)

    measured = {}
    pitch = {}
    for tag, layout in (("printer", "disabled"), ("virtual", "high-resolution")):
        src = os.path.join(args.outdir, f"pm-{tag}.fodt")
        with open(src, "w", encoding="utf-8") as f:
            f.write(build_fodt(layout))
        pdf = convert(src, args.outdir, tag)

        fonts = subprocess.run(["pdffonts", pdf], capture_output=True, text=True).stdout
        print(f"--- {tag} ({layout}) fonts")
        print(fonts.strip())

        with open(pdf, "rb") as f:
            data = f.read()
        allshows = []
        for st in page_streams(data):
            allshows.extend(shows(st))

        # Pair each black show with the red marker that follows it.
        pairs = []
        i = 0
        while i < len(allshows) - 1:
            if not allshows[i].red and allshows[i + 1].red:
                pairs.append((allshows[i], allshows[i + 1]))
                i += 2
            else:
                i += 1
        expect = rows()
        if len(pairs) != len(expect):
            print(f"!! {tag}: {len(pairs)} text/marker pairs for {len(expect)} rows", file=sys.stderr)
            return 2
        for r, (a, b) in zip(expect, pairs):
            measured[(tag, r)] = b.x - a.x
        # Baseline pitch between consecutive rows sharing a face and size.
        for k in range(len(expect) - 1):
            r0, r1 = expect[k], expect[k + 1]
            if r0.face == r1.face and r0.size == r1.size:
                pitch.setdefault((tag, r0.face, r0.size), []).append(
                    pairs[k][0].y - pairs[k + 1][0].y)

    # ------------------------------------------------ control: vertical pitch
    print("\n=== control: baseline pitch (the quantity already known to differ)")
    separated = 0
    for face, _ in FACES:
        for size in SIZES:
            p = min(pitch[("printer", face, size)])
            v = min(pitch[("virtual", face, size)])
            flag = "differs" if abs(p - v) > 0.01 else "SAME"
            if abs(p - v) > 0.01:
                separated += 1
            print(f"  {face:17s} {size:5.1f}pt  printer {p:8.3f}  virtual {v:8.3f}  {flag}")
    if separated == 0:
        print("\n!! the two files did not separate on a quantity known to differ.")
        print("!! the PrinterIndependentLayout switch did not take; nothing here is evidence.")
        return 1

    # ------------------------------------------------ the measurement
    print("\n=== advance widths, printer device against virtual device")
    print(f"{'face':17s} {'size':>5s} {'g':>2s} {'N':>3s} "
          f"{'printer':>9s} {'virtual':>9s} {'delta':>7s} {'exact':>9s}")
    moved = 0
    resid = {("exact", 0): 0.0}
    for name in ["exact"] + [f"per-glyph@{d}" for d in DPI_CANDIDATES] + \
                [f"whole@{d}" for d in DPI_CANDIDATES] + [f"exact-em@{d}" for d in DPI_CANDIDATES]:
        resid[name] = []
    for r in rows():
        adv, upem = metrics[(r.face, r.glyph)]
        p = measured[("printer", r)]
        v = measured[("virtual", r)]
        e = model_exact(adv, upem, r.size, r.count)
        if abs(p - v) > 0.005:
            moved += 1
        if r.count in (1, 64):
            print(f"{r.face:17s} {r.size:5.1f} {r.glyph:>2s} {r.count:3d} "
                  f"{p:9.3f} {v:9.3f} {p - v:7.3f} {e:9.3f}")
        resid["exact"].append(p - e)
        for d in DPI_CANDIDATES:
            resid[f"per-glyph@{d}"].append(p - model_per_glyph(adv, upem, r.size, r.count, d))
            resid[f"whole@{d}"].append(p - model_whole(adv, upem, r.size, r.count, d))
            resid[f"exact-em@{d}"].append(p - model_exact_em(adv, upem, r.size, r.count, d))

    n = len(rows())
    print(f"\n{moved} of {n} rows have a different advance on the two devices")

    print("\n=== which rule reproduces the PRINTER device's advances")
    print(f"{'model':18s} {'mean|err|pt':>12s} {'max|err|pt':>11s} {'exact rows':>11s}")
    for name in ["exact"] + [f"exact-em@{d}" for d in DPI_CANDIDATES] + \
                [f"per-glyph@{d}" for d in DPI_CANDIDATES] + [f"whole@{d}" for d in DPI_CANDIDATES]:
        errs = [abs(x) for x in resid[name]]
        hits = sum(1 for x in errs if x < 0.01)
        print(f"{name:18s} {sum(errs) / len(errs):12.4f} {max(errs):11.4f} {hits:7d}/{n}")

    # And the same table against the virtual device, as a second control: there the
    # unquantised rule must win outright, or the reading of the printer table is unsafe.
    print("\n=== control: which rule reproduces the VIRTUAL device's advances")
    vres = {"exact": []}
    for d in DPI_CANDIDATES:
        vres[f"per-glyph@{d}"] = []
    for r in rows():
        adv, upem = metrics[(r.face, r.glyph)]
        v = measured[("virtual", r)]
        vres["exact"].append(v - model_exact(adv, upem, r.size, r.count))
        for d in DPI_CANDIDATES:
            vres[f"per-glyph@{d}"].append(v - model_per_glyph(adv, upem, r.size, r.count, d))
    for name, errs in vres.items():
        errs = [abs(x) for x in errs]
        hits = sum(1 for x in errs if x < 0.01)
        print(f"{name:18s} {sum(errs) / len(errs):12.4f} {max(errs):11.4f} {hits:7d}/{n}")

    if not args.keep:
        for tag in ("printer", "virtual"):
            shutil.rmtree(os.path.join(args.outdir, f"prof-{tag}"), ignore_errors=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
