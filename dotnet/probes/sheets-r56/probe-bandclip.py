#!/usr/bin/env python3
"""Does a header band CLIP its text, and is the clip per-area or per-line?

Round 55 bracketed a "text-fit threshold" for header and footer bands: nothing is drawn at
0.72 or 1.44 pt of 8 pt text, nothing at 4.32 pt of 20 pt text, and the bracket moved with the
point size at about 0.27x. It recorded that as an unexplained law and did not implement it.

The claim this probe tests instead is that there is no threshold at all -- there is a
rectangle:

  * `ScPrintFunc::PrintHF` (sc/source/ui/view/printfun.cxx:1870) sets a clip region of exactly
    `Rectangle(aStart, Size(nLineWidth, nHeight - nDistance))` before drawing the three areas.
  * `ImpEditEngine::DrawText_ToPosition` (editeng/source/editeng/impedit3.cxx:3367-3372) takes
    the area's whole primitive range, and if it does not overlap the clip it returns having
    emitted nothing at all -- not ink, not PDF text. If it overlaps only partly it wraps the
    area in a `MaskPrimitive2D`, which is a different thing.

If that is the mechanism then the "threshold" is just the distance from a line's top to the
top of its ink -- `ascent - capHeight`, which for Liberation Sans is 0.217 em: 1.74 pt at 8 pt
and 4.34 pt at 20 pt, both inside round 55's brackets with nothing fitted.

Two things follow that a threshold does not predict, and this probe is built to separate them:

  * **case F/G** -- a band far larger than round 55's bracket still draws nothing when the ink
    is pushed below it by *empty leading lines*. This is `FAA-2019-0995-0002_attachment_2`'s
    shape: a 5.67 pt band, seven empty lines, then 9 pt text.
  * **case E** -- a two-line area whose first line is inside the band and whose second is not.
    Per-area (the reading above) keeps BOTH in the PDF text; per-line keeps only the first.

Fixture shape is `probes/sheets-r53-totalsrow/audit_mkwb.py`'s, which 26.2.4.2 is known to read
correctly, and which `probes/sheets-r55/audit_pagedecoration.py` already ran a passing control
through. The control runs first here too.
"""
import os, re, shutil, subprocess, sys, zipfile

NS = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
RNS = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
WORK = "/c/sandbox/workdir/scratch-r56-sheets/bandclip"
CLI = "/c/sandbox/workdir/wt-sheets-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
ENV = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")


def workbook(path, *, top, header, header_text, size=11):
    esc = lambda s: s.replace("&", "&amp;").replace("<", "&lt;")
    hf = "<headerFooter><oddHeader>%s</oddHeader></headerFooter>" % esc(header_text)
    styles = (
        f'<styleSheet xmlns="{NS}">'
        f'<fonts count="1"><font><sz val="{size}"/><name val="Liberation Sans"/></font></fonts>'
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
        f'<pageMargins left="0.7" right="0.7" top="{top}" bottom="0.75"'
        f' header="{header}" footer="0.3"/>'
        '<pageSetup paperSize="9" orientation="portrait"/>'
        f'{hf}</worksheet>')
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
            f'<Relationship Id="rId1" Type="{RNS}/officeDocument" Target="xl/workbook.xml"/></Relationships>')
        z.writestr("xl/_rels/workbook.xml.rels",
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            f'<Relationship Id="rId1" Type="{RNS}/worksheet" Target="worksheets/sheet1.xml"/>'
            f'<Relationship Id="rId2" Type="{RNS}/styles" Target="styles.xml"/></Relationships>')
        z.writestr("xl/workbook.xml",
            f'<workbook xmlns="{NS}" xmlns:r="{RNS}"><sheets>'
            '<sheet name="Probe" sheetId="1" r:id="rId1"/></sheets></workbook>')
        z.writestr("xl/styles.xml", styles)
        z.writestr("xl/worksheets/sheet1.xml", sheet)
    return path


def render(path, out):
    prof = os.path.join(WORK, "prof-" + os.path.basename(out))
    subprocess.run(["soffice", "-env:UserInstallation=file://" + prof, "--headless",
                    "--convert-to", "pdf", "--outdir", out, path], capture_output=True, env=ENV)
    subprocess.run([CLI, "render", path, "--format", "pdf", "--outdir", out + "-ours"],
                   capture_output=True, env=ENV)


def bbox(pdf):
    if not os.path.exists(pdf):
        return None
    return subprocess.run(["pdftotext", "-q", "-bbox", pdf, "-"],
                          capture_output=True, text=True).stdout


def find(b, token):
    if b is None:
        return None
    m = re.search(r'<word xMin="[\d.]+" yMin="([\d.]+)" xMax="[\d.]+" yMax="([\d.]+)">%s</word>'
                  % token, b)
    return None if m is None else round(float(m.group(1)), 2)


# (name, top in, header in, header string, cell size, what the rectangle reading predicts)
CASES = [
    # THE CONTROL, first. An ordinary 0.4 in band with one 11 pt line: the ink starts 2.39 pt
    # below the band's top and the band is 28.8 pt, so both tokens must be drawn.
    ("A-control",        0.75, 0.35, "&RZZTOPZZ",             11, "TOP drawn"),
    # Round 55's bracket, re-read as `ascent - capHeight` = 0.217 em = 1.74 pt at 8 pt.
    ("B-8pt-band1.44",   0.32, 0.30, "&R&8ZZTOPZZ",            8, "TOP absent (1.44 < 1.74)"),
    ("B-8pt-band2.16",   0.33, 0.30, "&R&8ZZTOPZZ",            8, "TOP drawn  (2.16 > 1.74)"),
    # ... and at 20 pt, where the same 0.217 em is 4.34 pt.
    ("C-20pt-band4.32",  0.36, 0.30, "&R&20ZZTOPZZ",          20, "TOP absent (4.32 < 4.34)"),
    ("C-20pt-band5.76",  0.38, 0.30, "&R&20ZZTOPZZ",          20, "TOP drawn  (5.76 > 4.34)"),
    # THE DISCRIMINATOR. Two 11 pt lines, band 14.4 pt. Line 1's ink starts at 2.39 and is
    # inside; line 2's starts at 12.21 + 2.39 = 14.60 and is outside. Per-area keeps both in
    # the PDF text; per-line keeps only ZZTOPZZ.
    ("D-two-line-14.4",  0.50, 0.30, "&RZZTOPZZ\nZZBOTZZ",    11, "TOP drawn; BOT decides"),
    # A band so large that both lines are inside: the control for D.
    ("D-two-line-36",    0.80, 0.30, "&RZZTOPZZ\nZZBOTZZ",    11, "both drawn"),
    # THE CORPUS SHAPE. `FAA-2019-0995-0002_attachment_2`'s sheet 10: 5.67 pt band, seven
    # empty leading lines, then 9 pt text on two lines. A 5.67 pt band is well above every
    # threshold round 55 bracketed, and the rectangle reading says nothing is drawn.
    ("E-faa-shape",      0.27559055118110237, 0.19685039370078741,
     "&R\n\n\n\n\n\n\n&9ZZTOPZZ\nZZBOTZZ", 11, "neither drawn"),
    # The same document with room made for the header: the positive control for E.
    ("E-faa-roomy",      1.60, 0.19685039370078741,
     "&R\n\n\n\n\n\n\n&9ZZTOPZZ\nZZBOTZZ", 11, "both drawn"),
]


def main():
    shutil.rmtree(WORK, ignore_errors=True)
    os.makedirs(WORK)
    print("%-18s %-8s %-30s %-12s %-12s %-12s %-12s" %
          ("case", "band pt", "predicted", "ref TOP", "ref BOT", "our TOP", "our BOT"))
    for name, top, header, text, size, predicted in CASES:
        path = workbook(os.path.join(WORK, name + ".xlsx"), top=top, header=header,
                        header_text=text, size=size)
        render(path, os.path.join(WORK, "out"))
        ref = bbox(os.path.join(WORK, "out", name + ".pdf"))
        ours = bbox(os.path.join(WORK, "out-ours", name + ".pdf"))
        print("%-18s %-8.2f %-30s %-12s %-12s %-12s %-12s" %
              (name, (top - header) * 72, predicted,
               find(ref, "ZZTOPZZ"), find(ref, "ZZBOTZZ"),
               find(ours, "ZZTOPZZ"), find(ours, "ZZBOTZZ")))


main()
