#!/usr/bin/env python3
"""Does the print scale reach a header/footer band -- its TEXT and the BODY ORIGIN below it?

Round 56 found an 18.46 pt uniform downward translation of the body on
`fm-provider-service-measures` p36 and 18.49 pt on `FY2023-AIP-grants` p1, on pages whose band
token agrees with the reference to 0.0005 pt.  Its brief guessed "the header height is counted
twice".  It is not.  Both of those sheets are *scaled* -- `fitToHeight="17"` on one and
`scale="43"` on the other -- and:

    ScPrintFunc::GetDocPageSize (sc/source/ui/view/printfun.cxx:2999-3003)

        aPageRect.SetTop( ( aPageRect.Top() + nTopMargin ) * 100 / nZoom + aHdr.nHeight );

builds the page rectangle in *document twips*, where the margin is divided by the zoom and the
band height is NOT.  A document twip is rendered at `zoom/100` of a physical twip, so the margin
comes back out at full size and **the band comes out at `nHeight * zoom/100`**.  The same map
mode (`aTwipMode`, InitModes:2645, carrying `fZoomFract`) is what `UpdateHFHeight` measures the
band in and what `PrintHF` draws it in, so the band's TEXT is drawn at `size * zoom/100` too.

`SheetPagination.DocPageSize` already ports that arithmetic exactly -- which is why page counts
match -- but its comment says the bands "are printed at full size whatever the sheet's scale:
they are page furniture rather than content", and `SheetPrintSetup.PrintableArea`, which is what
*places* what a page holds, implements that sentence instead of the arithmetic.

Two observables, one probe, five scales:

  1. the band text's effective point size in the reference's PDF -- keyed on the Tf size times
     the matrix, not on advance widths, because a scaled run is narrower for two reasons at once;
  2. the first body token's y.

THE CONTROL IS SCALE 100, and it must show zero difference on both observables: everything this
probe could break is inert there, so a run that moves the 100% case is measuring something else.

Fixture shape is `probes/sheets-r53-totalsrow/audit_mkwb.py`'s, known to be read correctly by
26.2.4.2 and used by the round 55 and 56 band probes before this one.
"""
import os, re, subprocess, sys, zipfile

NS = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
RNS = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
WORK = "/c/sandbox/workdir/scratch-r57-sheets/bandscale"
CLI = "/c/sandbox/workdir/wt-sheets-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
OPS = "/c/sandbox/workdir/wt-sheets-r50/.claude/skills/render-comparison/scripts/pdf-ops.py"
ENV = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")


def workbook(path, *, scale, top=0.75, header=0.3, bottom=0.75, footer=0.3,
             header_text="&C&14ZZTOPZZ", footer_text="&C&10ZZFOOTZZ", size=11, rows=1):
    esc = lambda s: s.replace("&", "&amp;").replace("<", "&lt;")
    hf = ("<headerFooter><oddHeader>%s</oddHeader><oddFooter>%s</oddFooter></headerFooter>"
          % (esc(header_text), esc(footer_text)))
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
    body = ""
    for r in range(1, rows + 1):
        tok = "ZZBODYZZ" if r == 1 else ("ZZLAST%dZZ" % r)
        body += f'<row r="{r}"><c r="A{r}" t="inlineStr"><is><t>{tok}</t></is></c></row>'
    sheet = (
        f'<worksheet xmlns="{NS}" xmlns:r="{RNS}">'
        f'<sheetData>{body}</sheetData>'
        f'<pageMargins left="0.7" right="0.7" top="{top}" bottom="{bottom}"'
        f' header="{header}" footer="{footer}"/>'
        f'<pageSetup paperSize="9" scale="{scale}" orientation="portrait"/>'
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


def render(path, tag):
    ref = os.path.join(WORK, "ref", tag)
    ours = os.path.join(WORK, "ours", tag)
    os.makedirs(ref, exist_ok=True)
    os.makedirs(ours, exist_ok=True)
    prof = os.path.join(WORK, "prof-" + tag)
    subprocess.run(["soffice", "-env:UserInstallation=file://" + prof, "--headless",
                    "--convert-to", "pdf", "--outdir", ref, path], capture_output=True, env=ENV)
    subprocess.run([CLI, "render", path, "--format", "pdf", "--outdir", ours],
                   capture_output=True, env=ENV)
    stem = os.path.splitext(os.path.basename(path))[0] + ".pdf"
    r, o = os.path.join(ref, stem), os.path.join(ours, stem)
    # Assert the instrument produced output before comparing it -- CLAUDE.md rule 3.
    for p in (r, o):
        if not os.path.exists(p):
            raise SystemExit("no PDF at " + p)
    return r, o


def tokens(pdf):
    out = subprocess.run(["pdftotext", "-q", "-bbox", pdf, "-"],
                         capture_output=True, text=True).stdout
    got = {}
    for m in re.finditer(
            r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">(.*?)</word>',
            out):
        got.setdefault(m.group(5), (float(m.group(1)), float(m.group(2)),
                                    float(m.group(3)), float(m.group(4))))
    return got


def sizes(pdf):
    """Effective point size of each text show, keyed by the literal it decodes to."""
    out = subprocess.run([sys.executable, OPS, "dump", pdf, "--page", "1"],
                         capture_output=True, text=True).stdout
    got = {}
    for line in out.splitlines():
        m = re.match(r'text\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+([\d.]+)pt\s+\S+\s+'
                     r'\d+ glyphs in \d+ show\(s\)\s+"(.*)"', line)
        if m:
            got.setdefault(m.group(4).strip(), float(m.group(3)))
    return got


CASES = [100, 80, 60, 40, 25]


def main():
    os.makedirs(WORK, exist_ok=True)
    print("%-6s | %-28s | %-28s | %s" % ("scale", "band 14pt text size", "body token y", "footer token y"))
    print("-" * 108)
    for scale in CASES:
        tag = "s%03d" % scale
        wb = workbook(os.path.join(WORK, tag + ".xlsx"), scale=scale)
        r, o = render(wb, tag)
        tr, to = tokens(r), tokens(o)
        sr, so = sizes(r), sizes(o)

        def g(d, k, i=1):
            return None if k not in d else round(d[k][i], 2)

        rs = so_ = None
        for k, v in sr.items():
            if "ZZTOPZZ" in k: rs = v
        for k, v in so.items():
            if "ZZTOPZZ" in k: so_ = v
        print("%-6s | ref %-7s ours %-7s pred %-5s | ref %-8s ours %-8s d %-6s | ref %-8s ours %-8s d %s" % (
            scale, rs, so_, round(14 * scale / 100.0, 2),
            g(tr, "ZZBODYZZ"), g(to, "ZZBODYZZ"),
            None if g(tr, "ZZBODYZZ") is None or g(to, "ZZBODYZZ") is None
            else round(g(to, "ZZBODYZZ") - g(tr, "ZZBODYZZ"), 2),
            g(tr, "ZZFOOTZZ"), g(to, "ZZFOOTZZ"),
            None if g(tr, "ZZFOOTZZ") is None or g(to, "ZZFOOTZZ") is None
            else round(g(to, "ZZFOOTZZ") - g(tr, "ZZFOOTZZ"), 2)))


if __name__ == "__main__":
    main()
