#!/usr/bin/env python3
"""24.2.7.2 audit re-check: `SheetText.MeasurePixels`, the per-glyph pixel rounding.

The site claims a string's width on Calc's measuring device is the **sum of the rounded
glyph advances** and not the rounded sum, and it was fitted against LibreOffice 24.2.7.2's
own row heights for turned cells — six string lengths at ten, eleven and twelve point,
read out of its flat-ODF round trip of `sheet-row-height-rotated.fods`.

This runs the same round trip through the installed 26.2.4.2 and compares all 216 heights
against the stored figures, which live in `SheetRotatedRowHeightTests` and are quoted here
so the probe does not depend on the test binary.

The discriminator is built in: fourteen of the eighteen distinct widths agree under either
reading of the rounding and four do not, so the twelve-point 90-degree rows are the ones
that decide it.  The 45/30/60-degree sheets are the control — they are capped by
SC_ROT_BREAK_FACTOR and would move for a different reason if they moved at all.

Refuses to report unless every sheet in the fixture produced a height.
"""
import os, re, subprocess, sys, tempfile, zipfile
import xml.etree.ElementTree as ET

ROOT = "/c/sandbox/workdir/wt-sheets-r50"
FIXTURE = ROOT + "/dotnet/tests/corpus/features/sheet-row-height-rotated.fods"

# The stored 24.2.7.2 figures, verbatim from SheetRotatedRowHeightTests.
STORED = {
    "p10a90n": (149, 522, 865, 1776, 3552, 6373),
    "p10a90w": (149, 522, 865, 1776, 3552, 6373),
    "p10a45n": (257, 522, 776, 1417, 2671, 4671),
    "p10a45w": (257, 522, 776, 1194, 1194, 1194),
    "p10a30n": (257, 462, 626, 1089, 1970, 3388),
    "p10a30w": (257, 462, 626, 1089, 1194, 1194),
    "p10a60n": (257, 567, 850, 1641, 3179, 5626),
    "p10a60w": (257, 567, 850, 1194, 1194, 1194),
    "p10a270n": (149, 522, 865, 1776, 3552, 6373),
    "p10a270w": (149, 522, 865, 1776, 3552, 6373),
    "p10a315n": (257, 522, 776, 1417, 2671, 4671),
    "p10a315w": (257, 522, 776, 1194, 1194, 1194),
    "p11a90n": (164, 597, 999, 2044, 4104, 7358),
    "p11a90w": (164, 597, 999, 2044, 4104, 7358),
    "p11a45n": (313, 611, 895, 1641, 3089, 5388),
    "p11a45w": (313, 611, 895, 1373, 1373, 1373),
    "p11a30n": (328, 537, 746, 1268, 2298, 3925),
    "p11a30w": (328, 537, 746, 1268, 1373, 1373),
    "p11a60n": (276, 641, 999, 1895, 3686, 6507),
    "p11a60w": (276, 641, 999, 1373, 1373, 1373),
    "p11a270n": (164, 597, 999, 2044, 4104, 7358),
    "p11a270w": (164, 597, 999, 2044, 4104, 7358),
    "p11a315n": (313, 611, 895, 1641, 3089, 5388),
    "p11a315w": (313, 611, 895, 1373, 1373, 1373),
    "p12a90n": (164, 626, 1059, 2149, 4328, 7731),
    "p12a90w": (164, 626, 1059, 2149, 4328, 7731),
    "p12a45n": (313, 641, 955, 1716, 3268, 5671),
    "p12a45w": (313, 641, 955, 1462, 1462, 1462),
    "p12a30n": (328, 567, 776, 1328, 2417, 4119),
    "p12a30w": (328, 567, 776, 1328, 1462, 1462),
    "p12a60n": (300, 686, 1059, 1999, 3880, 6835),
    "p12a60w": (300, 686, 1059, 1462, 1462, 1462),
    "p12a270n": (164, 626, 1059, 2149, 4328, 7731),
    "p12a270w": (164, 626, 1059, 2149, 4328, 7731),
    "p12a315n": (313, 641, 955, 1716, 3268, 5671),
    "p12a315w": (313, 641, 955, 1462, 1462, 1462),
}

OFFICE = "{urn:oasis:names:tc:opendocument:xmlns:office:1.0}"
TABLE = "{urn:oasis:names:tc:opendocument:xmlns:table:1.0}"
STYLE = "{urn:oasis:names:tc:opendocument:xmlns:style:1.0}"
FO = "{urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0}"


def twips(length):
    """An ODF length string to twips."""
    m = re.match(r"^([\d.]+)(cm|mm|in|pt|pc)$", length)
    if not m:
        return None
    v = float(m.group(1))
    inches = {"cm": v / 2.54, "mm": v / 25.4, "in": v, "pt": v / 72.0, "pc": v / 6.0}[m.group(2)]
    return inches * 1440.0


def roundtrip(path, outdir):
    env = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")
    profile = tempfile.mkdtemp()
    rc = subprocess.run(
        ["soffice", "--headless", "-env:UserInstallation=file://" + profile,
         "--convert-to", "fods:OpenDocument Spreadsheet Flat XML", "--outdir", outdir, path],
        env=env, capture_output=True, text=True, timeout=300)
    out = os.path.join(outdir, os.path.basename(path))
    if not os.path.exists(out):
        print(rc.stdout, rc.stderr, file=sys.stderr)
        return None
    return out


def heights(fods):
    root = ET.parse(fods).getroot()
    styles = {}
    for auto in root.iter(STYLE + "style"):
        if auto.get(STYLE + "family") != "table-row":
            continue
        props = auto.find(STYLE + "table-row-properties")
        if props is None:
            continue
        h = props.get(STYLE + "row-height") or props.get(STYLE + "min-row-height")
        if h:
            styles[auto.get(STYLE + "name")] = twips(h)

    out = {}
    body = root.find(OFFICE + "body")
    for table in body.find(OFFICE + "spreadsheet").findall(TABLE + "table"):
        name = table.get(TABLE + "name")
        rows = []
        for row in table.findall(TABLE + "table-row"):
            repeat = int(row.get(TABLE + "number-rows-repeated") or 1)
            h = styles.get(row.get(TABLE + "style-name"))
            for _ in range(min(repeat, 8)):
                rows.append(h)
        out[name] = rows
    return out


def main():
    outdir = sys.argv[1] if len(sys.argv) > 1 else tempfile.mkdtemp()
    os.makedirs(outdir, exist_ok=True)

    fods = roundtrip(FIXTURE, outdir)
    if fods is None:
        print("REFUSING TO REPORT — the fixture did not round-trip", file=sys.stderr)
        sys.exit(2)

    got = heights(fods)
    missing = [s for s in STORED if s not in got or len(got[s]) < 6 or None in got[s][:6]]
    if missing:
        print("REFUSING TO REPORT — %d of %d sheets produced no height: %s"
              % (len(missing), len(STORED), ", ".join(sorted(missing))), file=sys.stderr)
        sys.exit(2)

    print("inputs: %d sheets in the fixture, %d produced heights, 0 failures\n"
          % (len(STORED), len(STORED)))
    print("%-10s %s" % ("sheet", "  ".join("%12s" % n for n in
                                           ("1 char", "5", "10", "20", "40", "72"))))
    wrong = 0
    for sheet in sorted(STORED):
        row = []
        for i, want in enumerate(STORED[sheet]):
            have = int(round(got[sheet][i]))
            if have == want:
                row.append("%12s" % want)
            else:
                row.append("%12s" % ("%d->%d" % (want, have)))
                wrong += 1
        print("%-10s %s" % (sheet, "  ".join(row)))

    total = sum(len(v) for v in STORED.values())
    print("\n%d of %d row heights reproduce on 26.2.4.2; %d moved." % (total - wrong, total, wrong))
    quarter = [s for s in STORED if "a90" in s or "a270" in s]
    qw = sum(1 for s in quarter for i, w in enumerate(STORED[s])
             if int(round(got[s][i])) != w)
    print("of the %d quarter-turn heights — the ones that measure GetTextWidth directly — "
          "%d moved." % (sum(len(STORED[s]) for s in quarter), qw))


if __name__ == "__main__":
    main()
