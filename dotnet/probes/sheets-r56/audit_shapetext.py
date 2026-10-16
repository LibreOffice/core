#!/usr/bin/env python3
"""The 24.2.7.2 re-check for `SheetShapeText.DefaultSize`.

The site says: a DrawingML shape run that states no `sz` and inherits none is drawn at
**12 pt**, not at the 18 pt the shape's own default character height states. Its evidence is a
probe workbook of three text boxes round-tripped through LibreOffice **24.2.7.2**'s flat-ODS
export, where the bare run came back `fo:font-size="12pt"`.

This re-runs that on the installed **26.2.4.2**, and adds the arm the original could not have:
the *rendered* size, off the PDF, so that the answer does not depend on the exporter agreeing
with the layout.

Method notes:
  * **The control runs first and is stated first.** Box B states `sz="1100"` on its only run;
    if that does not come back as 11 pt then nothing else here means anything.
  * Box C is the site's own two-span case: a body that states 1100 and a trailing run that
    states nothing, which the site says comes back as 11 pt and 12 pt in two spans.
  * Box D states `sz="1800"` explicitly, so that "12" and "18" are separated by a case where
    18 is definitely reachable -- otherwise a reader that always answers 12 and a reader that
    answers the shape default cannot be told apart on a rendering.
  * The rendered figures are ink-box heights of a single capital-and-descender token, which
    scale with the em; the ratio between boxes is what carries the finding, not the absolute.
"""
import os, re, shutil, subprocess, sys, zipfile

NS = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
RNS = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
XDR = "http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"
A = "http://schemas.openxmlformats.org/drawingml/2006/main"
WORK = "/c/sandbox/workdir/scratch-r56-sheets/shapetext"
ENV = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")

# (token, the `sz` attribute on the run, the `sz` on the paragraph's endParaRPr / defRPr)
BOXES = [
    ("ZZBARERUN", None),      # the claim: no size anywhere on the run
    ("ZZELEVENP", "1100"),    # THE CONTROL
    ("ZZEIGHTEEN", "1800"),   # 18 pt is reachable, so 12 and 18 are separable
]


def anchor(index, token, size):
    rpr = '<a:rPr lang="en-US"%s/>' % ("" if size is None else ' sz="%s"' % size)
    row = index * 6
    return (
        '<xdr:twoCellAnchor editAs="oneCell">'
        f'<xdr:from><xdr:col>0</xdr:col><xdr:colOff>0</xdr:colOff>'
        f'<xdr:row>{row}</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from>'
        f'<xdr:to><xdr:col>6</xdr:col><xdr:colOff>0</xdr:colOff>'
        f'<xdr:row>{row + 5}</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to>'
        f'<xdr:sp macro="" textlink=""><xdr:nvSpPr>'
        f'<xdr:cNvPr id="{index + 2}" name="TextBox {index + 1}"/>'
        '<xdr:cNvSpPr txBox="1"/></xdr:nvSpPr>'
        '<xdr:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="3000000" cy="800000"/></a:xfrm>'
        '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></xdr:spPr>'
        '<xdr:txBody><a:bodyPr vertOverflow="clip" horzOverflow="clip" wrap="square"'
        ' rtlCol="0" anchor="t"/><a:lstStyle/>'
        f'<a:p><a:r>{rpr}<a:t>{token}</a:t></a:r></a:p>'
        '</xdr:txBody></xdr:sp><xdr:clientData/></xdr:twoCellAnchor>')


def build(path):
    drawing = (f'<xdr:wsDr xmlns:xdr="{XDR}" xmlns:a="{A}">'
               + "".join(anchor(n, t, s) for n, (t, s) in enumerate(BOXES))
               + '</xdr:wsDr>')
    styles = (
        f'<styleSheet xmlns="{NS}">'
        '<fonts count="1"><font><sz val="10"/><name val="Liberation Sans"/></font></fonts>'
        '<fills count="2"><fill><patternFill patternType="none"/></fill>'
        '<fill><patternFill patternType="gray125"/></fill></fills>'
        '<borders count="1"><border/></borders>'
        '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
        '<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyFont="1"/></cellXfs>'
        '<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>'
        '<dxfs count="0"/></styleSheet>')
    sheet = (
        f'<worksheet xmlns="{NS}" xmlns:r="{RNS}">'
        '<sheetData><row r="1"><c r="A1" t="inlineStr"><is><t>ZZBODYZZ</t></is></c></row></sheetData>'
        '<pageMargins left="0.7" right="0.7" top="0.75" bottom="0.75" header="0.3" footer="0.3"/>'
        '<pageSetup paperSize="9" orientation="portrait"/>'
        '<drawing r:id="rId9"/></worksheet>')
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml",
            '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
            '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
            '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
            '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
            '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>'
            '<Override PartName="/xl/drawings/drawing1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawing+xml"/>'
            '</Types>')
        z.writestr("_rels/.rels",
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            f'<Relationship Id="rId1" Type="{RNS}/officeDocument" Target="xl/workbook.xml"/></Relationships>')
        z.writestr("xl/_rels/workbook.xml.rels",
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            f'<Relationship Id="rId1" Type="{RNS}/worksheet" Target="worksheets/sheet1.xml"/>'
            f'<Relationship Id="rId2" Type="{RNS}/styles" Target="styles.xml"/></Relationships>')
        z.writestr("xl/worksheets/_rels/sheet1.xml.rels",
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            f'<Relationship Id="rId9" Type="{RNS}/drawing" Target="../drawings/drawing1.xml"/></Relationships>')
        z.writestr("xl/workbook.xml",
            f'<workbook xmlns="{NS}" xmlns:r="{RNS}"><sheets>'
            '<sheet name="Shapes" sheetId="1" r:id="rId1"/></sheets></workbook>')
        z.writestr("xl/styles.xml", styles)
        z.writestr("xl/worksheets/sheet1.xml", sheet)
        z.writestr("xl/drawings/drawing1.xml", drawing)
    return path


def run(*args):
    return subprocess.run(args, capture_output=True, text=True, env=ENV)


def main():
    shutil.rmtree(WORK, ignore_errors=True)
    os.makedirs(WORK)
    book = build(os.path.join(WORK, "shapes.xlsx"))
    prof = "file://" + os.path.join(WORK, "prof")

    run("soffice", "-env:UserInstallation=" + prof, "--headless",
        "--convert-to", "fods", "--outdir", WORK, book)
    run("soffice", "-env:UserInstallation=" + prof, "--headless",
        "--convert-to", "pdf", "--outdir", WORK, book)

    fods = os.path.join(WORK, "shapes.fods")
    text = open(fods, encoding="utf8", errors="replace").read() if os.path.exists(fods) else ""

    # Each box becomes a draw:frame holding a text:p whose span names a text style; the style's
    # fo:font-size is the answer. Keyed on the token so the three cannot be confused.
    styles = dict(re.findall(
        r'<style:style style:name="([^"]+)"[^>]*style:family="text".*?fo:font-size="([^"]+)"',
        text, re.S))
    print("== the flat-ODS export, the same instrument the site used ==")
    for token, stated in BOXES:
        m = re.search(r'<text:span text:style-name="([^"]+)">%s</text:span>' % token, text)
        got = styles.get(m.group(1)) if m else None
        if m is None and token in text:
            got = "(no span: inherits the paragraph)"
        print("   %-12s states %-6s -> %s" % (token, stated or "nothing", got))

    # And the rendering, which does not depend on the exporter.
    pdf = os.path.join(WORK, "shapes.pdf")
    bbox = run("pdftotext", "-q", "-bbox", pdf, "-").stdout
    print("== the rendering: ink-box height of each token ==")
    for token, stated in BOXES:
        m = re.search(r'<word xMin="[\d.]+" yMin="([\d.]+)" xMax="[\d.]+" yMax="([\d.]+)">%s</word>'
                      % token, bbox)
        h = round(float(m.group(2)) - float(m.group(1)), 3) if m else None
        print("   %-12s states %-6s -> ink height %s pt" % (token, stated or "nothing", h))


main()
