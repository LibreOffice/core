#!/usr/bin/env python3
"""24.2.7.2 audit: are a VML note anchor's offsets 96-dpi screen pixels on 26.2.4.2?

The site is `XRay.Office.Spreadsheets/Ooxml/XlsxNoteCaptions.cs` and the sentence under test is

    "The anchor's offsets are screen pixels, not EMUs.  ShapeAnchor::importVmlAnchor sets
     CellAnchorType::Pixel (sc/source/filter/oox/drawingbase.cxx:152-155) and calcCellAnchorEmu
     scales them through Unit::ScreenX, which is 96 per inch.  Checked against LibreOffice
     24.2.7.2's own export ..."

which is a claim about a binary that is no longer the reference, and it is the last *furniture*
claim in `XRay.Office.Spreadsheets` -- both sheets sites found wrong so far were furniture.

METHOD.  Authored workbooks varying ONE number: the anchor's row offset, at 0, 48, 96 and 144.
A slope needs two points and this has four, which is what separates the three candidate readings
outright rather than by plausibility:

    96 dpi   -> +15.0 pt per 20 px     (the site's claim)
    72 dpi   -> +20.0 pt per 20 px
    EMU      -> +0.0  pt per 20 px     (i.e. nothing)

The first cut of this probe used a 20 pt row and offsets of 48, 96 and 144 px, and read
"neither" at every step -- 39.996, 59.897, 59.897, 59.897.  That is not a third law, it is the
offset being CLAMPED to the anchor row's own height: a 20 pt row is 26.7 px at 96 dpi, so 48, 96
and 144 all saturate at exactly one row.  The rows here are 60 pt (80 px) and the offsets stay
inside them, which is the difference between measuring the rule and measuring the clamp.

THE CONTROL RUNS FIRST: an anchor whose row offset is 0 must put the caption's top exactly on the
top of the row the anchor names, which is a number known in advance from the row heights alone.
A fixture LibreOffice read wrongly would fail that before any slope is fitted.

The instrument is `soffice --convert-to fods` and the observable is the annotation's own
`svg:y`, not a rendered position: a shown comment is drawn with a shadow and a border, so reading
it off the PDF measures the caption's decoration as much as its anchor.
"""
import os, re, shutil, subprocess, sys, zipfile

WORK = "/c/sandbox/workdir/scratch-r57-sheets/vmlanchor"
ENV = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")

NS = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
RNS = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
CT = "http://schemas.openxmlformats.org/package/2006/content-types"

# Row 1..8 all at a stated 20 pt so the row grid is known without measuring a font.
ROW_POINTS = 60.0
# Column A..H all at a stated width so the column grid is known too.
COL_CHARS = 12.0

VML = """<xml xmlns:v="urn:schemas-microsoft-com:vml"
 xmlns:o="urn:schemas-microsoft-com:office:office"
 xmlns:x="urn:schemas-microsoft-com:office:excel">
<o:shapelayout v:ext="edit"><o:idmap v:ext="edit" data="1"/></o:shapelayout>
<v:shapetype id="_x0000_t202" coordsize="21600,21600" o:spt="202"
 path="m,l,21600r21600,l21600,xe"><v:stroke joinstyle="miter"/>
<v:path gradientshapeok="t" o:connecttype="rect"/></v:shapetype>
<v:shape id="_x0000_s1025" type="#_x0000_t202"
 style='position:absolute;margin-left:400pt;margin-top:400pt;width:96pt;height:48pt;z-index:1;visibility:visible'
 fillcolor="#ffffe1" o:insetmode="auto">
<v:fill color2="#ffffe1"/><v:shadow on="t" color="black" obscured="t"/>
<v:path o:connecttype="none"/>
<v:textbox style='mso-direction-alt:auto'><div style='text-align:left'></div></v:textbox>
<x:ClientData ObjectType="Note"><x:MoveWithCells/><x:SizeWithCells/>
<x:Anchor>%d, %d, %d, %d, %d, %d, %d, %d</x:Anchor>
<x:AutoFill>False</x:AutoFill><x:Row>2</x:Row><x:Column>1</x:Column></x:ClientData>
</v:shape></xml>"""

COMMENTS = ("""<comments xmlns="%s"><authors><author>P</author></authors>"""
            """<commentList><comment ref="B3" authorId="0">"""
            """<text><r><rPr><sz val="9"/><rFont val="Tahoma"/></rPr>"""
            """<t>ZZNOTEZZ</t></r></text></comment></commentList></comments>""" % NS)


def workbook(path, *, from_col, from_off, from_row, from_roff,
             to_col, to_off, to_row, to_roff):
    cols = "".join(f'<col min="{i}" max="{i}" width="{COL_CHARS}" customWidth="1"/>'
                   for i in range(1, 9))
    rows = "".join(f'<row r="{r}" ht="{ROW_POINTS}" customHeight="1">'
                   f'<c r="A{r}" t="inlineStr"><is><t>R{r}</t></is></c></row>'
                   for r in range(1, 9))
    sheet = (f'<worksheet xmlns="{NS}" xmlns:r="{RNS}">'
             f'<cols>{cols}</cols><sheetData>{rows}</sheetData>'
             '<pageMargins left="0.7" right="0.7" top="0.75" bottom="0.75"'
             ' header="0.3" footer="0.3"/>'
             '<pageSetup paperSize="9" orientation="portrait"/>'
             '<legacyDrawing r:id="rIdV"/></worksheet>')
    styles = (f'<styleSheet xmlns="{NS}">'
              '<fonts count="1"><font><sz val="10"/><name val="Liberation Sans"/></font></fonts>'
              '<fills count="2"><fill><patternFill patternType="none"/></fill>'
              '<fill><patternFill patternType="gray125"/></fill></fills>'
              '<borders count="1"><border/></borders>'
              '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
              '<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyFont="1"/></cellXfs>'
              '<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>'
              '<dxfs count="0"/></styleSheet>')

    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml",
            f'<Types xmlns="{CT}">'
            '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
            '<Default Extension="vml" ContentType="application/vnd.openxmlformats-officedocument.vmlDrawing"/>'
            '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
            '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
            '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>'
            '<Override PartName="/xl/comments1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml"/>'
            '</Types>')
        z.writestr("_rels/.rels",
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            f'<Relationship Id="rId1" Type="{RNS}/officeDocument" Target="xl/workbook.xml"/></Relationships>')
        z.writestr("xl/_rels/workbook.xml.rels",
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            f'<Relationship Id="rId1" Type="{RNS}/worksheet" Target="worksheets/sheet1.xml"/>'
            f'<Relationship Id="rIdS" Type="{RNS}/styles" Target="styles.xml"/></Relationships>')
        z.writestr("xl/worksheets/_rels/sheet1.xml.rels",
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            f'<Relationship Id="rIdV" Type="{RNS}/vmlDrawing" Target="../drawings/vmlDrawing1.vml"/>'
            f'<Relationship Id="rIdC" Type="{RNS}/comments" Target="../comments1.xml"/></Relationships>')
        z.writestr("xl/workbook.xml",
            f'<workbook xmlns="{NS}" xmlns:r="{RNS}"><sheets>'
            '<sheet name="Probe" sheetId="1" r:id="rId1"/></sheets></workbook>')
        z.writestr("xl/styles.xml", styles)
        z.writestr("xl/worksheets/sheet1.xml", sheet)
        z.writestr("xl/comments1.xml", COMMENTS)
        z.writestr("xl/drawings/vmlDrawing1.vml",
                   VML % (from_col, from_off, from_row, from_roff,
                          to_col, to_off, to_row, to_roff))
    return path


def fods(path, tag):
    out = os.path.join(WORK, "fods")
    os.makedirs(out, exist_ok=True)
    prof = os.path.join(WORK, "prof-" + tag)
    subprocess.run(["soffice", "-env:UserInstallation=file://" + prof, "--headless",
                    "--convert-to", "fods", "--outdir", out, path],
                   capture_output=True, env=ENV)
    got = os.path.join(out, os.path.splitext(os.path.basename(path))[0] + ".fods")
    if not os.path.exists(got):
        raise SystemExit("no fods at " + got)
    return open(got, encoding="utf8", errors="replace").read()


def annotation(text):
    """The annotation's svg:x / svg:y, in points."""
    m = re.search(r"<office:annotation\b[^>]*>", text)
    if not m:
        return None
    tag = m.group(0)

    def val(name):
        v = re.search(name + r'="([-\d.]+)(cm|mm|in|pt)"', tag)
        if not v:
            return None
        n, unit = float(v.group(1)), v.group(2)
        return {"cm": 28.3464567, "mm": 2.83464567, "in": 72.0, "pt": 1.0}[unit] * n

    return val("svg:x"), val("svg:y")


def main():
    if os.path.isdir(WORK):
        shutil.rmtree(WORK)
    os.makedirs(WORK)

    print("Row grid: eight rows of %.1f pt.  Column grid: eight columns of %.1f chars."
          % (ROW_POINTS, COL_CHARS))
    print()
    print("%-16s %-10s %-10s %-10s %s"
          % ("row offset px", "svg:y pt", "step pt", "implied dpi", "reading"))
    print("-" * 74)

    previous = None
    for off in (0, 20, 40, 60):
        tag = "roff%03d" % off
        wb = workbook(os.path.join(WORK, tag + ".xlsx"),
                      from_col=1, from_off=0, from_row=2, from_roff=off,
                      to_col=4, to_off=0, to_row=5, to_roff=off)
        got = annotation(fods(wb, tag))
        if got is None or got[1] is None:
            print("%-16d NO ANNOTATION IN THE EXPORT" % off)
            previous = None
            continue
        y = got[1]
        step = None if previous is None else y - previous
        per = None if step is None else step / 72.0
        per = None if step is None else step / 20.0 * 96.0   # implied dpi denominator
        reading = ""
        if step is not None:
            dpi = None if step == 0 else 20.0 / step * 72.0
            reading = ("96 dpi" if dpi and abs(dpi - 96) < 2
                       else "72 dpi" if dpi and abs(dpi - 72) < 2
                       else "neither")
        print("%-16d %-10.3f %-10s %-10s %s"
              % (off, y,
                 "-" if step is None else "%.3f" % step,
                 "-" if step in (None, 0) else "%.1f" % (20.0 / step * 72.0),
                 reading))
        previous = y

    print()
    print("The control: an anchor naming row 2 (zero-based) with offset 0 puts the caption's top")
    print("at 2 x %.1f = %.1f pt, measured from the sheet's own origin." % (ROW_POINTS, 2 * ROW_POINTS))
    print()
    print("And the clamp, kept because it is what the first cut of this probe mistook for a law:")
    for off in (200, 400):
        tag = "clamp%03d" % off
        wb = workbook(os.path.join(WORK, tag + ".xlsx"),
                      from_col=1, from_off=0, from_row=2, from_roff=off,
                      to_col=4, to_off=0, to_row=5, to_roff=off)
        got = annotation(fods(wb, tag))
        print("  row offset %-5d -> svg:y %.3f pt  (row 2 top %.1f, row 3 top %.1f)"
              % (off, got[1], 2 * ROW_POINTS, 3 * ROW_POINTS))


if __name__ == "__main__":
    main()
