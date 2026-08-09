#!/usr/bin/env python3
"""Does LibreOffice's printer reference device quantise a *glyph advance*, and on what grid?

**Yes, but not per glyph.** Measured on 96 authored rows, the printer device's width is

    width = floor( N · advance · px_em / upem )  device pixels,  px_em = round(size/72 · 300)

converted to logical units afterwards — exact on **96 of 96 rows**, where rounding each
glyph's advance separately (the obvious reading of
`GenericSalLayout::LayoutText`, `vcl/source/gdi/CommonSalLayout.cxx`:826-831) is out by up
to 6.96 pt. Two quantisations, and the *first* is the one that moves a document: at 9 pt on
a 300 dpi grid the em is 37.5 px and the device sets 38, so every advance is **1.33 %
wider** than the size asked for. The truncation is worth at most 0.24 pt on a whole portion
and pulls the other way.

The control half of the same run confirms the *vertical* rule the track already ships
(`MetricGrid` in `Paperless.Text/Fonts/LineSpacing.cs`) on eight authored face/size pairs
with the flag varied — 10.60/11.55/12.75/13.95 pt for Liberation Serif at 9/10/11/12 pt,
10.35/-/13.00/13.95 for Liberation Sans — which had only ever been fitted against corpus
documents.

## The switch, and the trap in reaching it

`WW8Dop::fUsePrinterMetrics` — Word's "use printer metrics to lay out document" — becomes
`!USE_VIRTUAL_DEVICE` in `sw/source/filter/ww8/ww8par.cxx`:2008, and
`DocumentDeviceManager::getReferenceDevice` then formats against an `SfxPrinter` rather
than the 8640 dpi virtual device.

Eight of the words track's 200 documents set it and **no** DOCX in the corpus states
`<w:usePrinterMetrics/>`, so the corpus cannot vary the flag while holding a document
still. This probe varies it, by writing the body through LibreOffice's own DOC export and
then patching the Dop.

**The compatibility dword is stored twice and the late copy wins.** `WW8Dop::WW8Dop` reads
it at 0x54 and then, `if (nRead >= 516)`, reads it again at **0x1fc** and overwrites
(`ww8scan.cxx`, "500 -> 508, Appear to be repeated here in 2000+"). Patching 0x54 alone
changes nothing at all: three documents came back byte-identical for byte-identical
content streams, which reads exactly like "the flag reaches nothing" and is really "the
patch reached nothing". Patch both.

## The instrument

Each body paragraph is one glyph repeated N times followed by a **red** one-character
marker in the same face and size. A colour change forces its own text show, so

    width(N) = x(marker) − x(text)

read straight out of the content stream. One repeated glyph keeps kerning out of it. The
candidate rules are sharply different by N = 64:

    exact        N · adv · size / upem
    exact-em     N · adv · px_em / upem, in points          (em quantised, advance not)
    per-glyph    N · round(adv · px_em / upem) pixels        (both quantised)
    whole-string round(N · adv · px_em / upem) pixels

with `px_em = round(size / 72 · dpi)`.

## The controls

1. **Baseline pitch** must differ between the two files. It is the quantity already known
   to be quantised, so if it does not separate, the patch did not take and nothing else in
   the run is evidence. (This is the control that caught the 0x54 mistake.)
2. The **virtual** file's advances must be reproduced by the unquantised rule outright,
   or the reading of the printer table is unsafe.
3. The probe measures a *length*, so it names its face in the styles and prints the PDF's
   own font list — an authored file that fell back to another face would measure nothing.

    printer-metric-advance.py --outdir /abs/scratch/pm
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import struct
import subprocess
import sys
import zlib
from dataclasses import dataclass

FACES = [("Liberation Serif", "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf"),
         ("Liberation Sans", "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")]
SIZES = [9.0, 10.0, 11.0, 12.0]
GLYPHS = ["n", "i", "M"]
COUNTS = [1, 4, 16, 64]

DPI_CANDIDATES = [300, 600, 720, 1440]

# Both places WW8Dop keeps the compatibility options. The second wins when lcbDop >= 516.
COMPAT_OFFSETS = (0x54, 0x1FC)
PRINTER_METRICS_BIT = 0x80000000

FODT = """<?xml version="1.0" encoding="UTF-8"?>
<office:document
 xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.text">
 <office:font-face-decls>
{faces}
 </office:font-face-decls>
 <office:automatic-styles>
{styles}
  <style:page-layout style:name="pm1">
   <style:page-layout-properties fo:page-width="55cm" fo:page-height="29.7cm"
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
    return [Row(face, size, glyph, n)
            for face, _ in FACES for size in SIZES for glyph in GLYPHS for n in COUNTS]


def build_fodt() -> str:
    faces = "\n".join(
        f'  <style:font-face style:name="F{i}" svg:font-family="{name}"/>'
        for i, (name, _) in enumerate(FACES))
    face_index = {name: i for i, (name, _) in enumerate(FACES)}
    styles, body = [], []
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
    return FODT.format(faces=faces, styles="\n".join(styles), body="\n".join(body))


# ------------------------------------------------------------------ the Dop patch

def set_printer_metrics(path: str, wanted: bool) -> list[str]:
    """Set or clear fUsePrinterMetrics in *both* copies of the Dop's compatibility dword."""
    import olefile

    ole = olefile.OleFileIO(path, write_mode=True)
    try:
        fib = ole.openstream("WordDocument").read(0x400)
        table = "1Table" if struct.unpack_from("<H", fib, 0x0A)[0] & 0x0200 else "0Table"
        fc_dop, lcb_dop = struct.unpack_from("<ii", fib, 0x192)
        data = bytearray(ole.openstream(table).read())
        was = []
        for off in COMPAT_OFFSETS:
            if lcb_dop < off + 4:
                continue
            at = fc_dop + off
            value = struct.unpack_from("<I", data, at)[0]
            was.append(f"0x{off:03x}={value:08x}")
            struct.pack_into("<I", data, at,
                             value | PRINTER_METRICS_BIT if wanted
                             else value & ~PRINTER_METRICS_BIT)
        ole.write_stream(table, bytes(data))
        return was
    finally:
        ole.close()


# ---------------------------------------------------------------- PDF reading

TOKEN = re.compile(rb"([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+Tm"
                   rb"|([-\d.]+)\s+([-\d.]+)\s+Td"
                   rb"|(\d+(?:\.\d+)?)\s+(\d+(?:\.\d+)?)\s+(\d+(?:\.\d+)?)\s+rg"
                   rb"|\[(?:[^\]]*)\]\s*TJ"
                   rb"|<[0-9A-Fa-f]*>\s*Tj"
                   rb"|\((?:[^()\\]|\\.)*\)\s*Tj")


@dataclass
class Show:
    x: float
    y: float
    red: bool


def shows(stream: bytes) -> list[Show]:
    out: list[Show] = []
    red = False
    x = y = 0.0
    for m in TOKEN.finditer(stream):
        if m.group(1) is not None:
            x, y = float(m.group(5)), float(m.group(6))
        elif m.group(7) is not None:
            x, y = float(m.group(7)), float(m.group(8))
        elif m.group(9) is not None:
            r, g, b = float(m.group(9)), float(m.group(10)), float(m.group(11))
            red = r > 0.9 and g < 0.1 and b < 0.1
        else:
            out.append(Show(x, y, red))
    return out


def page_streams(pdf: bytes) -> list[bytes]:
    out = []
    for m in re.finditer(rb"stream\r?\n", pdf):
        start = m.end()
        end = pdf.find(b"endstream", start)
        raw = pdf[start:end]
        try:
            out.append(zlib.decompress(raw))
        except zlib.error:
            out.append(raw)
    return out


def convert(src: str, outdir: str, profile: str, fmt: str = "pdf") -> str:
    subprocess.run(["soffice", "--headless", f"-env:UserInstallation=file://{profile}",
                    "--convert-to", fmt, "--outdir", outdir, src],
                   check=False, capture_output=True, timeout=600)
    out = os.path.join(outdir, os.path.splitext(os.path.basename(src))[0] + "." + fmt)
    if not os.path.exists(out):
        sys.exit(f"soffice produced no {fmt} for {src}")
    return out


# ---------------------------------------------------------------- font metrics

def advance(path: str, glyph: str) -> tuple[int, int]:
    from fontTools.ttLib import TTFont
    font = TTFont(path, lazy=True)
    upem = font["head"].unitsPerEm
    adv = font["hmtx"][font.getBestCmap()[ord(glyph)]][0]
    font.close()
    return adv, upem


def px_em(size: float, dpi: int) -> int:
    return round(size / 72.0 * dpi)


def model_exact(adv, upem, size, n, dpi=None):
    return n * adv * size / upem


def model_exact_em(adv, upem, size, n, dpi):
    return n * adv * (px_em(size, dpi) * 72.0 / dpi) / upem


def model_per_glyph(adv, upem, size, n, dpi):
    return n * round(adv * px_em(size, dpi) / upem) * 72.0 / dpi


def model_whole(adv, upem, size, n, dpi):
    return round(n * adv * px_em(size, dpi) / upem) * 72.0 / dpi


def model_floor(adv, upem, size, n, dpi):
    """The rule this probe measures: exact advances at an integer-pixel em, the *total*
    truncated to a whole device pixel, and only then converted to logical units."""
    import math
    return math.floor(n * adv * px_em(size, dpi) / upem) * 72.0 / dpi


# ---------------------------------------------------------------- main

def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--outdir", required=True, help="absolute scratch directory")
    args = ap.parse_args()
    if not os.path.isabs(args.outdir):
        sys.exit("--outdir must be absolute")
    os.makedirs(args.outdir, exist_ok=True)
    profile = os.path.join(args.outdir, "prof")

    src = os.path.join(args.outdir, "pm-body.fodt")
    with open(src, "w", encoding="utf-8") as f:
        f.write(build_fodt())
    base_doc = convert(src, args.outdir, profile, "doc")

    metrics = {(face, g): advance(path, g) for face, path in FACES for g in GLYPHS}
    measured: dict[tuple[str, Row], float] = {}
    pitch: dict[tuple[str, str, float], list[float]] = {}

    for tag, wanted in (("printer", True), ("virtual", False)):
        doc = os.path.join(args.outdir, f"pm-{tag}.doc")
        shutil.copy(base_doc, doc)
        was = set_printer_metrics(doc, wanted)
        print(f"--- {tag}: fUsePrinterMetrics={wanted}, patched {', '.join(was)}")
        pdf = convert(doc, os.path.join(args.outdir, tag), profile, "pdf")
        print(subprocess.run(["pdffonts", pdf], capture_output=True, text=True).stdout.strip())

        allshows = []
        for st in page_streams(open(pdf, "rb").read()):
            allshows.extend(shows(st))
        pairs, i = [], 0
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
        for k in range(len(expect) - 1):
            if expect[k].face == expect[k + 1].face and expect[k].size == expect[k + 1].size:
                pitch.setdefault((tag, expect[k].face, expect[k].size), []).append(
                    pairs[k][0].y - pairs[k + 1][0].y)

    print("\n=== control: baseline pitch (the quantity already known to differ)")
    separated = 0
    for face, _ in FACES:
        for size in SIZES:
            # Positive gaps only: a page break puts the next baseline above this one.
            p = min(x for x in pitch[("printer", face, size)] if x > 0)
            v = min(x for x in pitch[("virtual", face, size)] if x > 0)
            if abs(p - v) > 0.01:
                separated += 1
            print(f"  {face:17s} {size:5.1f}pt  printer {p:7.3f}  virtual {v:7.3f}  "
                  f"{'differs' if abs(p - v) > 0.01 else 'SAME'}")
    if separated == 0:
        print("\n!! the two files did not separate on a quantity known to differ.")
        print("!! the Dop patch did not take; nothing here is evidence.")
        return 1

    print("\n=== advance widths, printer device against virtual device")
    print(f"{'face':17s} {'size':>5s} {'g':>2s} {'N':>3s} "
          f"{'printer':>9s} {'virtual':>9s} {'delta':>7s} {'exact':>9s}")
    moved = 0
    for r in rows():
        adv, upem = metrics[(r.face, r.glyph)]
        p, v = measured[("printer", r)], measured[("virtual", r)]
        if abs(p - v) > 0.005:
            moved += 1
        if r.count == 64:
            print(f"{r.face:17s} {r.size:5.1f} {r.glyph:>2s} {r.count:3d} "
                  f"{p:9.3f} {v:9.3f} {p - v:7.3f} {model_exact(adv, upem, r.size, r.count):9.3f}")
    n = len(rows())
    print(f"\n{moved} of {n} rows have a different advance on the two devices")

    named = [("exact", model_exact, None)]
    for d in DPI_CANDIDATES:
        named += [(f"exact-em@{d}", model_exact_em, d),
                  (f"per-glyph@{d}", model_per_glyph, d),
                  (f"whole@{d}", model_whole, d),
                  (f"floor@{d}", model_floor, d)]

    for tag, title in (("printer", "PRINTER"), ("virtual", "VIRTUAL (control)")):
        print(f"\n=== which rule reproduces the {title} device's advances")
        print(f"{'model':18s} {'mean|err|pt':>12s} {'max|err|pt':>11s} {'exact rows':>12s}")
        for name, fn, dpi in named:
            errs = [abs(measured[(tag, r)] - fn(*metrics[(r.face, r.glyph)], r.size, r.count, dpi))
                    for r in rows()]
            print(f"{name:18s} {sum(errs) / len(errs):12.4f} {max(errs):11.4f} "
                  f"{sum(1 for e in errs if e < 0.026):8d}/{n}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
