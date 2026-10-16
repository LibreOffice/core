#!/usr/bin/env python3
"""The 24.2.7.2 re-check for `SheetPageDecoration.cs`'s header/footer band claim.

The site (`SheetPageDecoration.cs:378`) still names **24.2.7.2** in the sentence that
records what the old claim was, and the claim now standing beside it is:

  * a header or footer band of **exactly zero** stated height draws **nothing** -- not the
    space and not the ink;
  * any stated band **above** zero draws its text.

That was written from six authored variants of one corpus workbook. This re-checks it from
scratch on the installed **26.2.4.2**, on fixtures authored here, and adds the two things
the earlier reading could not separate: what happens when the stated band is **negative**
(`header` beyond `top`), and where the text lands vertically when the band is positive --
which is the *other* calibrated number in the same method's remarks (1.5 pt of centring
above a 9.05 pt ascent, from `sheet-decor-ods.ods`).

Method notes that cost earlier probes a round:

  * The fixture generator is `probes/sheets-r53-totalsrow/audit_mkwb.py`'s shape, which is
    known to be read correctly by 26.2.4.2 -- a minimal `.xlsx` with **no `<cellStyles>`**
    has its `cellXf` font discarded entirely and every font-size probe reads a constant.
  * **The control runs first.** A workbook with a plainly positive band must print its
    footer, or nothing else the probe says means anything.
  * The observable has one degree of freedom: the footer's only text is a token
    (`ZZFOOTERZZ`) that appears nowhere else in the document, so "is it on the page" is a
    substring test that no other part of the rendering can answer.
"""
import os, re, shutil, subprocess, sys, zipfile

NS = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
RNS = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
WORK = "/c/sandbox/workdir/scratch-r55-sheets/pagedecor"
CLI = "/c/sandbox/workdir/wt-sheets-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
ENV = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")


def workbook(path, *, top, bottom, header, footer, header_text, footer_text, size=11):
    hf = ""
    if header_text or footer_text:
        hf = "<headerFooter>"
        if header_text:
            hf += "<oddHeader>&amp;C%s</oddHeader>" % header_text
        if footer_text:
            hf += "<oddFooter>&amp;C%s</oddFooter>" % footer_text
        hf += "</headerFooter>"

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
        f'<pageMargins left="0.7" right="0.7" top="{top}" bottom="{bottom}"'
        f' header="{header}" footer="{footer}"/>'
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
                    "--convert-to", "pdf", "--outdir", out, path],
                   capture_output=True, env=ENV)
    subprocess.run([CLI, "render", path, "--format", "pdf", "--outdir", out + "-ours"],
                   capture_output=True, env=ENV)


def words(pdf):
    if not os.path.exists(pdf):
        return None
    return subprocess.run(["pdftotext", "-q", "-bbox", pdf, "-"],
                          capture_output=True, text=True).stdout


def find(bbox, token):
    if bbox is None:
        return None
    m = re.search(r'<word xMin="[\d.]+" yMin="([\d.]+)" xMax="[\d.]+" yMax="([\d.]+)">%s</word>'
                  % token, bbox)
    return None if m is None else (round(float(m.group(1)), 2), round(float(m.group(2)), 2))


def main():
    shutil.rmtree(WORK, ignore_errors=True)
    os.makedirs(WORK)

    # (name, top, bottom, header, footer). The band is `top - header` for the header and
    # `bottom - footer` for the footer, in inches.
    cases = [
        # THE CONTROL, first: an ordinary Excel default. Both bands are 0.4in.
        ("control-default", 0.75, 0.75, 0.3, 0.3),
        # The band shrinks towards zero.
        ("band-0.20in", 0.75, 0.75, 0.55, 0.55),
        ("band-0.10in", 0.75, 0.75, 0.65, 0.65),
        ("band-0.05in", 0.75, 0.75, 0.70, 0.70),
        ("band-0.01in", 0.75, 0.75, 0.74, 0.74),
        # Exactly zero, which is the claim.
        ("band-zero", 0.75, 0.75, 0.75, 0.75),
        # And beyond, which the earlier reading could not separate from zero.
        ("band-negative", 0.75, 0.75, 1.00, 1.00),
    ]

    print("%-16s %-30s %-30s" % ("case", "reference footer y (min,max)", "ours"))
    rows = []
    for name, top, bottom, header, footer in cases:
        path = workbook(os.path.join(WORK, name + ".xlsx"), top=top, bottom=bottom,
                        header=header, footer=footer,
                        header_text="ZZHEADERZZ", footer_text="ZZFOOTERZZ")
        render(path, os.path.join(WORK, "ref"))
        ref = words(os.path.join(WORK, "ref", name + ".pdf"))
        ours = words(os.path.join(WORK, "ref-ours", name + ".pdf"))
        rows.append((name, find(ref, "ZZFOOTERZZ"), find(ours, "ZZFOOTERZZ"),
                     find(ref, "ZZHEADERZZ"), find(ours, "ZZHEADERZZ"),
                     find(ref, "ZZBODYZZ"), find(ours, "ZZBODYZZ")))
        print("%-16s %-30s %-30s" % (name, rows[-1][1], rows[-1][2]))

    print()
    print("%-16s %-22s %-22s %-22s %-22s" % ("case", "ref header", "ours header",
                                             "ref body", "ours body"))
    for name, _, _, rh, oh, rb, ob in rows:
        print("%-16s %-22s %-22s %-22s %-22s" % (name, rh, oh, rb, ob))

    agree = sum(1 for r in rows if (r[1] is None) == (r[2] is None)
                and (r[3] is None) == (r[4] is None))
    print()
    print("presence agrees on %d of %d cases, header and footer both" % (agree, len(rows)))


main()
