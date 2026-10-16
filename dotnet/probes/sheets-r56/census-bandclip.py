#!/usr/bin/env python3
"""Which corpus worksheets have a header or footer whose text the band CLIPS AWAY.

The mechanism, read out of the reference's own filter and print code and confirmed on the
binary by `probe-bandclip.py`:

  * `PageSettingsConverter::convertHeaderFooterData` (sc/source/filter/oox/pagesettings.cxx
    :1015-1041) measures the band's text with `HeaderFooterParser` -- one line's height is the
    largest *stated point size* on it -- and sets `mnBodyDist = statedBand - textHeight`.
    A negative distance sets `mbDynamicHeight = false` and pins `mnHeight` at the stated band.
  * `ScPrintFunc::UpdateHFHeight` (printfun.cxx:789-793) returns immediately for a band that is
    not dynamic, so the pinned height survives to print time.
  * `ScPrintFunc::PrintHF` (printfun.cxx:1870) sets a CLIP REGION of exactly
    `Rectangle(aStart, Size(nLineWidth, nHeight - nDistance))` and then draws each of the three
    areas into it.
  * `ImpEditEngine::DrawText_ToPosition` (editeng/source/editeng/impedit3.cxx:3367-3372) asks
    whether the area's own primitive range overlaps that clip and **returns without emitting
    anything at all** when it does not.

So a band that is shorter than its text does not "overflow downwards": whatever falls outside
the rectangle is not drawn, and an area whose ink is entirely outside it is not drawn at all --
not the ink and not the PDF text.

This counts, per worksheet and per band, the areas whose topmost inked line starts below the
band's own bottom edge. Those are the areas the reference drops and we currently draw.

Blind spots, stated because an under-reaching census conceals itself:
  * **xlsx/xlsm only.** 64 of the 307 sheets documents are `.xls`, whose bands come through
    `XclImpHFConverter` -- the same arithmetic, a different reader, and not visible here.
  * The ink-top offset is modelled as `(ascent - capHeight) x size`, 0.217 em for Liberation
    Sans, rather than measured per glyph. Right at the boundary that is an estimate.
  * `evenHeader`/`evenFooter` are folded into the pin decision (Calc takes the max of the
    three) but the per-page area that actually prints on an even page is not separated here.
"""
import csv, os, re, sys, zipfile
import xml.etree.ElementTree as ET

CORPUS = "/c/sandbox/workdir/sample-files"
NS = "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}"

# Liberation Sans, the face Calc's furniture is drawn in.
LINE = 1.11          # ascent + descent, in ems -- SheetBandText.LineHeightAt
INK_TOP = 0.217      # ascent - capHeight, in ems: how far below a line's top its ink starts


def default_size(z):
    """The workbook's own default cell font size, which is what a band with no `&<n>` uses."""
    try:
        root = ET.fromstring(z.read("xl/styles.xml"))
    except Exception:
        return 10.0
    fonts = root.find(NS + "fonts")
    if fonts is None or len(fonts) == 0:
        return 10.0
    sz = fonts[0].find(NS + "sz")
    return float(sz.get("val")) if sz is not None and sz.get("val") else 10.0


def parse(codes, default):
    """Walk the `&`-code string exactly as `HeaderFooterParser` does.

    Returns, per area (L, C, R): the nominal height Calc's filter measures, and the list of
    (line top in ems-of-line-height, line size, has ink) for the lines the area holds.
    """
    areas = {0: [], 1: [], 2: []}          # (size, inked)
    cur = {0: [0.0, False], 1: [0.0, False], 2: [0.0, False]}
    part, size = 1, default
    i = 0
    while i < len(codes):
        c = codes[i]
        if c != "&" or i + 1 >= len(codes):
            if c == "\n":
                areas[part].append(tuple(cur[part]) if cur[part][0] else (size, cur[part][1]))
                cur[part] = [0.0, False]
            elif not c.isspace():
                cur[part][0] = max(cur[part][0], size)
                cur[part][1] = True
            else:
                # A space is text to the parser -- it raises the line height -- but it puts no
                # ink on the page, so it does not make the line visible.
                cur[part][0] = max(cur[part][0], size)
            i += 1
            continue
        code = codes[i + 1]
        i += 2
        up = code.upper()
        if up in "LCR":
            part = {"L": 0, "C": 1, "R": 2}[up]
            size = default
        elif up in "PNDTAFZ":
            cur[part][0] = max(cur[part][0], size)
            cur[part][1] = True
        elif code == "&":
            cur[part][0] = max(cur[part][0], size)
            cur[part][1] = True
        elif code == "\n":
            areas[part].append(tuple(cur[part]) if cur[part][0] else (size, cur[part][1]))
            cur[part] = [0.0, False]
        elif code == '"':
            end = codes.find('"', i)
            i = len(codes) if end < 0 else end + 1
        elif code.isdigit():
            start = i - 1
            while i < len(codes) and codes[i].isdigit():
                i += 1
            size = float(codes[start:i])
        elif up == "K":
            i = min(len(codes), i + 6)
    for p in (0, 1, 2):
        areas[p].append(tuple(cur[p]) if cur[p][0] else (size, cur[p][1]))
    return areas


def nominal(areas):
    return max(sum(s for s, _ in a) for a in areas.values())


def clipped_areas(areas, band_pt):
    """Areas whose topmost inked line begins below the band's bottom edge."""
    out = []
    for p, lines in areas.items():
        y = 0.0
        first = None
        for size, inked in lines:
            if inked:
                first = y + INK_TOP * size
                break
            y += LINE * size
        if first is None:
            continue                       # no ink in this area at all
        if first >= band_pt:
            out.append((p, round(first, 2)))
    return out


def main():
    hits = []
    pinned = 0
    sheets = 0
    for r in csv.DictReader(open(os.path.join(CORPUS, "MANIFEST.tsv"), newline=""), delimiter="\t"):
        if r["family"] != "sheets" or not r["path"].lower().endswith((".xlsx", ".xlsm")):
            continue
        p = os.path.join(CORPUS, r["path"])
        if not os.path.exists(p):
            continue
        try:
            z = zipfile.ZipFile(p)
        except Exception:
            continue
        with z:
            dflt = default_size(z)
            for name in sorted(z.namelist()):
                if not re.match(r"xl/worksheets/sheet\d+\.xml$", name):
                    continue
                try:
                    root = ET.fromstring(z.read(name))
                except Exception:
                    continue
                m = root.find(NS + "pageMargins")
                hf = root.find(NS + "headerFooter")
                if m is None or hf is None:
                    continue
                g = lambda k, d: float(m.get(k, d))
                top, bottom = g("top", 0.75), g("bottom", 0.75)
                head, foot = g("header", 0.3), g("footer", 0.3)
                different_first = hf.get("differentFirst") in ("1", "true")
                for label, band_in, tags in (
                        ("header", top - head, ("oddHeader", "evenHeader", "firstHeader")),
                        ("footer", bottom - foot, ("oddFooter", "evenFooter", "firstFooter"))):
                    texts = []
                    for t in tags:
                        e = hf.find(NS + t)
                        if e is None or not (e.text or "").strip():
                            continue
                        if t.startswith("first") and not different_first:
                            continue
                        texts.append((t, e.text))
                    if not texts:
                        continue
                    sheets += 1
                    band = band_in * 72
                    parsed = [(t, parse(x, dflt)) for t, x in texts]
                    nom = max(nominal(a) for _, a in parsed)
                    if nom <= band:
                        continue               # dynamic: the band grows to the text
                    pinned += 1
                    for t, a in parsed:
                        c = clipped_areas(a, band)
                        if c:
                            hits.append((r["status"], r["path"], name, t,
                                         round(band, 2), round(nom, 2), c))
    print("worksheet bands with content: %d;  pinned (text taller than band): %d" % (sheets, pinned))
    print("bands with at least one area clipped away entirely: %d" % len(hits))
    for h in sorted(hits):
        print("  %-5s %-62s %-18s %-12s band=%6.2f nominal=%7.2f %s"
              % (h[0], h[1].split("/")[-1][:62], h[2].split("/")[-1], h[3], h[4], h[5], h[6]))


main()
