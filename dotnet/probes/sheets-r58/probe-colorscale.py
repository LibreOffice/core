#!/usr/bin/env python3
"""What LibreOffice 26.2.4.2 draws for a `cfRule type="colorScale"`.

Authored variants, one thing varied at a time, with two controls that run FIRST:

  00-control-none   the same sheet with no <conditionalFormatting> at all
                    -> the instrument must find ZERO scale fills
  01-control-solid  the same sheet with an ordinary solid fill on B2:B13
                    -> the instrument must find TWELVE fills of one stated colour

Without both, "the reference draws N fills" is a property of the reader.  The corpus
half of this measurement is `003_advanced_excel_pie`, whose reference PDF carries
twelve interpolated fills we draw none of.

Usage:  probe-colorscale.py <outdir>
"""
import collections, os, re, subprocess, sys, zipfile

NS = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
RNS = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
ROOT = "/c/sandbox/workdir/wt-sheets-r50"
PDFOPS = ROOT + "/.claude/skills/render-comparison/scripts/pdf-ops.py"


def workbook(path, values, cf_xml, *, extra_fill=None, first_row=2, col="B"):
    """One sheet, one column of numbers at `col`, plus whatever conditional XML is given.

    `extra_fill` is an ARGB string; when set every value cell states a solid fill of it,
    so the scale can be measured against a cell that already has a background.
    """
    rowxml = ""
    for i, v in enumerate(values):
        r = first_row + i
        s = ' s="1"' if extra_fill else ""
        cell = "" if v is None else (
            f'<c r="{col}{r}"{s} t="inlineStr"><is><t>{v[1:]}</t></is></c>' if isinstance(v, str)
            else f'<c r="{col}{r}"{s}><v>{v}</v></c>')
        rowxml += f'<row r="{r}">{cell}</row>'

    fills = ('<fills count="2"><fill><patternFill patternType="none"/></fill>'
             '<fill><patternFill patternType="gray125"/></fill></fills>')
    cellxfs = ('<cellXfs count="1">'
               '<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyFont="1"/>'
               '</cellXfs>')
    if extra_fill:
        fills = ('<fills count="3"><fill><patternFill patternType="none"/></fill>'
                 '<fill><patternFill patternType="gray125"/></fill>'
                 f'<fill><patternFill patternType="solid"><fgColor rgb="{extra_fill}"/>'
                 '<bgColor indexed="64"/></patternFill></fill></fills>')
        cellxfs = ('<cellXfs count="2">'
                   '<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyFont="1"/>'
                   '<xf numFmtId="0" fontId="0" fillId="2" borderId="0" xfId="0" applyFont="1" '
                   'applyFill="1"/>'
                   '</cellXfs>')

    styles = (
        f'<styleSheet xmlns="{NS}">'
        '<fonts count="1"><font><sz val="10"/><name val="Liberation Sans"/></font></fonts>'
        f'{fills}'
        '<borders count="1"><border/></borders>'
        '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
        f'{cellxfs}'
        '<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>'
        '<dxfs count="0"/>'
        '</styleSheet>')

    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml",
            '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
            '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
            '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
            '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
            '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>'
            '</Types>')
        z.writestr("_rels/.rels",
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            f'<Relationship Id="rId1" Type="{RNS}/officeDocument" Target="xl/workbook.xml"/>'
            '</Relationships>')
        z.writestr("xl/_rels/workbook.xml.rels",
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            f'<Relationship Id="rId1" Type="{RNS}/worksheet" Target="worksheets/sheet1.xml"/>'
            f'<Relationship Id="rId2" Type="{RNS}/styles" Target="styles.xml"/>'
            '</Relationships>')
        z.writestr("xl/workbook.xml",
            f'<workbook xmlns="{NS}" xmlns:r="{RNS}"><sheets>'
            '<sheet name="Probe" sheetId="1" r:id="rId1"/></sheets></workbook>')
        z.writestr("xl/styles.xml", styles)
        z.writestr("xl/worksheets/sheet1.xml",
            f'<worksheet xmlns="{NS}" xmlns:r="{RNS}">'
            '<cols><col min="1" max="4" width="12" customWidth="1"/></cols>'
            f'<sheetData>{rowxml}</sheetData>{cf_xml}</worksheet>')
    return path


def scale(sqref, entries):
    cfvo = "".join(entries[0])
    colours = "".join(entries[1])
    return (f'<conditionalFormatting sqref="{sqref}">'
            f'<cfRule type="colorScale" priority="1"><colorScale>{cfvo}{colours}</colorScale>'
            '</cfRule></conditionalFormatting>')


RED, YEL, GRN = 'FFF8696B', 'FFFFEB84', 'FF63BE7B'
C = lambda *r: [f'<color rgb="{x}"/>' for x in r]
V = lambda *t: [f'<cfvo type="{a}"{"" if b is None else f" val=\"{b}\""}/>' for a, b in t]

VALS12 = [93, 100, 107, 114, 121, 128, 135, 142, 149, 156, 163, 170]
VALS11 = list(range(0, 11))

CASES = [
    # (name, values, cf xml, extra solid fill)
    ("00-control-none",  VALS12, "", None),
    ("01-control-solid", VALS12, "", "FF00FF00"),
    ("02-two-minmax",    VALS11, scale("B2:B12", (V(("min", None), ("max", None)), C(RED, GRN))), None),
    ("03-three-mid50",   VALS12, scale("B2:B13", (V(("min", None), ("percentile", 50), ("max", None)),
                                                  C(RED, YEL, GRN))), None),
    ("04-num-2-8",       VALS11, scale("B2:B12", (V(("num", 2), ("num", 8)), C(RED, GRN))), None),
    ("05-percent-25-75", VALS11, scale("B2:B12", (V(("percent", 25), ("percent", 75)), C(RED, GRN))), None),
    ("06-percentile-90", VALS11, scale("B2:B12", (V(("min", None), ("percentile", 90)), C(RED, GRN))), None),
    ("07-with-own-fill", VALS11, scale("B2:B12", (V(("min", None), ("max", None)), C(RED, GRN))), "FF00FF00"),
    ("08-text-in-range", ["s" + x for x in "abcdefghijk"],
                         scale("B2:B12", (V(("min", None), ("max", None)), C(RED, GRN))), None),
    ("09-mixed-blanks",  [0, None, 2, None, 4, None, 6, None, 8, None, 10],
                         scale("B2:B12", (V(("min", None), ("max", None)), C(RED, GRN))), None),
    ("10-range-past-data", VALS11,
                         scale("B2:B40", (V(("min", None), ("max", None)), C(RED, GRN))), None),
    ("11-formula-cfvo",  VALS11,
                         scale("B2:B12", (V(("formula", 2), ("formula", 8)), C(RED, GRN))), None),
    ("12-three-num",     VALS11, scale("B2:B12", (V(("num", 0), ("num", 5), ("num", 10)),
                                                  C(RED, YEL, GRN))), None),
    ("13-negatives",     [-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5],
                         scale("B2:B12", (V(("min", None), ("max", None)), C(RED, GRN))), None),
]

BLU, MAG = 'FF0000FF', 'FFFF00FF'


def two_scales():
    """Two colorScale rules over one range: which one paints, priority or document order?

    The first element in document order carries priority 9 (the *lower* priority in
    SpreadsheetML, where 1 is highest); the second carries priority 1.
    """
    a = ('<conditionalFormatting sqref="B2:B12"><cfRule type="colorScale" priority="9">'
         f'<colorScale><cfvo type="min"/><cfvo type="max"/><color rgb="{BLU}"/>'
         f'<color rgb="{MAG}"/></colorScale></cfRule></conditionalFormatting>')
    b = ('<conditionalFormatting sqref="B2:B12"><cfRule type="colorScale" priority="1">'
         f'<colorScale><cfvo type="min"/><cfvo type="max"/><color rgb="{RED}"/>'
         f'<color rgb="{GRN}"/></colorScale></cfRule></conditionalFormatting>')
    return a + b


CASES.append(("14-two-scales", VALS11, two_scales(), None))
def two_scales_swapped():
    """The discriminating half of the pair: the same two rules, document order reversed.

    Case 14 alone cannot separate "highest priority wins" from "last in document order
    wins", because its winner was both.  Here priority 1 comes *first*.
    """
    b = ('<conditionalFormatting sqref="B2:B12"><cfRule type="colorScale" priority="1">'
         f'<colorScale><cfvo type="min"/><cfvo type="max"/><color rgb="{RED}"/>'
         f'<color rgb="{GRN}"/></colorScale></cfRule></conditionalFormatting>')
    a = ('<conditionalFormatting sqref="B2:B12"><cfRule type="colorScale" priority="9">'
         f'<colorScale><cfvo type="min"/><cfvo type="max"/><color rgb="{BLU}"/>'
         f'<color rgb="{MAG}"/></colorScale></cfRule></conditionalFormatting>')
    return b + a


CASES.append(("15-two-scales-swapped", VALS11, two_scales_swapped(), None))


def main():
    outdir = os.path.abspath(sys.argv[1])
    os.makedirs(outdir, exist_ok=True)
    made = {}
    for name, values, cf, extra in CASES:
        p = os.path.join(outdir, name + ".xlsx")
        workbook(p, values, cf, extra_fill=extra)
        made[name] = p

    env = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")
    rc = subprocess.run(
        [ROOT + "/.claude/skills/libreoffice-reference/scripts/lo-convert.sh",
         "--pdf", "--quiet", "--outdir", os.path.join(outdir, "ref")] + list(made.values()),
        env=env, capture_output=True, text=True)
    if rc.returncode != 0:
        print(rc.stdout, rc.stderr, file=sys.stderr)

    missing = [n for n in made
               if not os.path.exists(os.path.join(outdir, "ref", n + ".xlsx", n + ".pdf"))]
    if missing:
        print("REFUSING TO REPORT — %d of %d fixtures produced no PDF: %s"
              % (len(missing), len(made), ", ".join(sorted(missing))), file=sys.stderr)
        sys.exit(2)

    print("fixtures: %d authored, %d rendered, 0 failures\n" % (len(made), len(made)))

    for name, values, cf, extra in CASES:
        pdf = os.path.join(outdir, "ref", name + ".xlsx", name + ".pdf")
        out = subprocess.run([sys.executable, PDFOPS, "dump", pdf],
                             capture_output=True, text=True).stdout
        fills = []
        for line in out.splitlines():
            if not line.startswith("fill"):
                continue
            m = re.search(r"\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(#\w{6})",
                          line)
            if m:
                fills.append((float(m.group(2)), m.group(5), float(m.group(1))))
        # keep only the ones in column B's x band, drop full-page white
        band = [f for f in fills if 60 < f[2] < 200 and f[1] != "#FFFFFF"]
        band.sort(key=lambda f: -f[0])
        print("%-20s %2d fills  %s" % (name, len(band), " ".join(c for _, c, _ in band)))
        print("%-20s values   %s" % ("", " ".join("-" if v is None else str(v) for v in values)))


if __name__ == "__main__":
    main()
