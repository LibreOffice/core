#!/usr/bin/env python3
"""Does a column with no stated width come out the same width as the reference's?"""
import os, re, subprocess, sys, zipfile
sys.path.insert(0, "/c/sandbox/workdir/wt-sheets-r50/dotnet/probes/sheets-r53-totalsrow")

NS = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
RNS = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"

def wb(path, font, size, default=None, sheetfmt=""):
    cells = "".join(f'<c r="{c}1" t="inlineStr"><is><t>|</t></is></c>' for c in "ABCDEF")
    fmt = sheetfmt or (f'<sheetFormatPr defaultRowHeight="15"{default or ""}/>')
    styles = (f'<styleSheet xmlns="{NS}">'
              f'<fonts count="1"><font><sz val="{size}"/><name val="{font}"/></font></fonts>'
              '<fills count="2"><fill><patternFill patternType="none"/></fill>'
              '<fill><patternFill patternType="gray125"/></fill></fills>'
              '<borders count="1"><border/></borders>'
              '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
              '<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyFont="1"/></cellXfs>'
              '<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>'
              '<dxfs count="0"/></styleSheet>')
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
        z.writestr("xl/worksheets/sheet1.xml",
            f'<worksheet xmlns="{NS}" xmlns:r="{RNS}">{fmt}'
            f'<sheetData><row r="1">{cells}</row></sheetData></worksheet>')
    return path

def xs(pdf):
    out = subprocess.run(["pdftotext","-q","-f","1","-l","1","-bbox",pdf,"-"],
                         capture_output=True, text=True).stdout
    return sorted(float(m.group(1)) for m in
                  re.finditer(r'<word xMin="([\d.]+)"[^>]*>\|</word>', out))

work = "/c/sandbox/workdir/scratch-r54-sheets/dc"
cli = "/c/sandbox/workdir/wt-sheets-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
os.makedirs(work, exist_ok=True)

cases = [("calibri11-none", "Calibri", 11, None),
         ("calibri11-843",  "Calibri", 11, ' defaultColWidth="8.43"'),
         ("libsans10-none", "Liberation Sans", 10, None),
         ("calibri11-noformat", "Calibri", 11, "SKIP")]
print(f"{'case':>20} {'ref widths':>34} {'our widths':>34}")
for name, font, size, default in cases:
    fmt = "" if default == "SKIP" else None
    path = wb(os.path.join(work, name + ".xlsx"), font, size,
              None if default in (None, "SKIP") else default,
              sheetfmt="<sheetFormatPr defaultRowHeight=\"15\"/>" if default == "SKIP" else "")
    prof = os.path.join(work, "prof-" + name)
    subprocess.run(["rm","-rf",prof])
    subprocess.run(["soffice", f"-env:UserInstallation=file://{prof}", "--headless",
                    "--convert-to","pdf","--outdir",work,path], capture_output=True)
    subprocess.run([cli,"render",path,"--format","pdf","--outdir",os.path.join(work,"ours")],
                   capture_output=True)
    r = xs(os.path.join(work, name + ".pdf"))
    o = xs(os.path.join(work, "ours", name + ".pdf"))
    rw = [round(r[i+1]-r[i],2) for i in range(len(r)-1)]
    ow = [round(o[i+1]-o[i],2) for i in range(len(o)-1)]
    print(f"{name:>20} {str(rw):>34} {str(ow):>34}")
