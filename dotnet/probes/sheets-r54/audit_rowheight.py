#!/usr/bin/env python3
"""24.2.7.2 re-check: SheetOptimalRowHeights.WrappedHeight, against the installed 26.2.4.2.

The site claims thirty exact reproductions of a wrapped row's optimal height, fitted to a flat-ODF
round trip through LibreOffice **24.2.7.2**.  This re-runs that shape on the reference binary that
actually decides this corpus.

Instrument, and why it is this one rather than a reading of the C++:

  * one workbook per font size, six rows, row *i* holding a wrapping cell whose text is *i* words
    that cannot share a line, so the row is *i* lines tall before any measurement;
  * every cell is `vertical="top"`, and column B carries an identical six-point marker in every
    row, so **the y of the marker in row i+1 less the y of the marker in row i is row i's height**
    — the same trick round 53's column-width probe used across columns;
  * no `ht` and no `customHeight` anywhere, which is what makes Calc recompute rather than honour;
  * the reference's own `--convert-to fods` `style:row-height` is read back as a second,
    independent reading of the same number, so the PDF instrument is checked before it is believed.

THE CONTROL THAT MUST PASS FIRST.  Row 1 of the twelve-point workbook is a single-line cell whose
answer is already known and written down at the site: `trunc(240 x 1.18) = 283`, plus 40 twips of
pool-default margin, less `STD_ROWHEIGHT_DIFF` = 23, is **300 twips**, which is the `0.2083in`
LibreOffice's own export writes for `National-Reports.xlsx`.  If that row does not read 300, the
fixture is not being read the way the site's fixture was and no other number here means anything.
Round 53's device probe read a constant 101.08 pt at every size because a workbook with no
`<cellStyles>` has its `cellXf` font discarded; this generator carries one.
"""
import os, re, subprocess, sys, zipfile

NS = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
RNS = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"

SIZES = [8, 10, 11, 12, 14, 18]
LINES = [1, 2, 3, 4, 5]
COLUMN_WIDTH = 12.0          # characters, so two nine-letter words can never share a line
WORDS = ["Wednesday", "Blackbird", "Clockwork", "Dartmouth", "Elephants"]
MARKER_SIZE = 6


def workbook(path, size):
    """One probe workbook: five wrapping rows at `size`, plus a sentinel row after them."""
    fonts = (f'<font><sz val="{size}"/><name val="Liberation Sans"/></font>'
             f'<font><sz val="{MARKER_SIZE}"/><name val="Liberation Sans"/></font>')

    # 0: the wrapping, top-aligned cell.  1: the marker, top-aligned, not wrapping.
    xfs = ('<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyFont="1"'
           ' applyAlignment="1"><alignment wrapText="1" vertical="top"/></xf>'
           '<xf numFmtId="0" fontId="1" fillId="0" borderId="0" xfId="0" applyFont="1"'
           ' applyAlignment="1"><alignment vertical="top"/></xf>')

    rows = ""
    for at, count in enumerate(LINES, start=1):
        text = " ".join(WORDS[:count])
        rows += (f'<row r="{at}">'
                 f'<c r="A{at}" s="0" t="inlineStr"><is><t>{text}</t></is></c>'
                 f'<c r="B{at}" s="1" t="inlineStr"><is><t>x</t></is></c>'
                 f'</row>')
    # The sentinel: one marker, so the last probe row's height is a difference like the others.
    at = len(LINES) + 1
    rows += f'<row r="{at}"><c r="B{at}" s="1" t="inlineStr"><is><t>x</t></is></c></row>'

    styles = (f'<styleSheet xmlns="{NS}">'
              f'<fonts count="2">{fonts}</fonts>'
              '<fills count="2"><fill><patternFill patternType="none"/></fill>'
              '<fill><patternFill patternType="gray125"/></fill></fills>'
              '<borders count="1"><border/></borders>'
              '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
              f'<cellXfs count="2">{xfs}</cellXfs>'
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
            f'<cols><col min="1" max="1" width="{COLUMN_WIDTH}" customWidth="1"/>'
            '<col min="2" max="2" width="6" customWidth="1"/></cols>'
            f'<sheetData>{rows}</sheetData></worksheet>')
    return path


def marker_tops(pdf):
    """The y of every 'x' marker on page 1, in points, top down."""
    out = subprocess.run(["pdftotext", "-q", "-f", "1", "-l", "1", "-bbox", pdf, "-"],
                         capture_output=True, text=True).stdout
    ys = [float(m.group(1))
          for m in re.finditer(r'<word xMin="[\d.]+" yMin="([\d.]+)"[^>]*>x</word>', out)]
    return sorted(ys)


def fods_heights(path, outdir, profile):
    subprocess.run(["soffice", f"-env:UserInstallation=file://{profile}", "--headless",
                    "--convert-to", "fods", "--outdir", outdir, path],
                   capture_output=True)
    flat = os.path.join(outdir, os.path.splitext(os.path.basename(path))[0] + ".fods")
    if not os.path.exists(flat):
        return []
    text = open(flat, encoding="utf-8").read()
    styles = dict(re.findall(
        r'<style:style style:name="([^"]+)" style:family="table-row"[^>]*>'
        r'\s*<style:table-row-properties[^>]*style:row-height="([^"]+)"', text))
    order = re.findall(r'<table:table-row table:style-name="([^"]+)"'
                       r'(?: table:number-rows-repeated="(\d+)")?', text)
    heights = []
    for name, repeat in order:
        for _ in range(int(repeat or 1)):
            heights.append(styles.get(name))
    return heights


def to_twips(value):
    if value is None:
        return None
    m = re.match(r"([\d.]+)(in|cm|mm|pt)$", value)
    if not m:
        return None
    n, unit = float(m.group(1)), m.group(2)
    return {"in": n * 1440, "cm": n * 1440 / 2.54, "mm": n * 144 / 2.54, "pt": n * 20}[unit]


def main(work, cli):
    os.makedirs(work, exist_ok=True)
    rows = []
    control = None

    for size in SIZES:
        path = workbook(os.path.join(work, f"rh-{size}.xlsx"), size)
        profile = os.path.join(work, f"prof-{size}")
        subprocess.run(["rm", "-rf", profile])

        subprocess.run(["soffice", f"-env:UserInstallation=file://{profile}", "--headless",
                        "--convert-to", "pdf", "--outdir", work, path], capture_output=True)
        subprocess.run([cli, "render", path, "--format", "pdf", "--outdir",
                        os.path.join(work, "ours")], capture_output=True)

        ref_pdf = os.path.join(work, f"rh-{size}.pdf")
        our_pdf = os.path.join(work, "ours", f"rh-{size}.pdf")
        ref_y, our_y = marker_tops(ref_pdf), marker_tops(our_pdf)
        flat = fods_heights(path, os.path.join(work, "flat"), profile)

        for at, count in enumerate(LINES):
            ref = (ref_y[at + 1] - ref_y[at]) * 20 if len(ref_y) > at + 1 else None
            ours = (our_y[at + 1] - our_y[at]) * 20 if len(our_y) > at + 1 else None
            stated = to_twips(flat[at]) if len(flat) > at else None
            rows.append((size, count, ref, ours, stated))
            if size == 12 and count == 1:
                control = stated

    print(f"{'size':>5} {'words':>6} {'ref pdf':>9} {'ref fods':>9} {'ours pdf':>9} {'delta':>8}")
    exact = near = 0
    for size, count, ref, ours, stated in rows:
        delta = None if (ref is None or ours is None) else ours - ref
        if delta is not None and abs(delta) < 0.5:
            exact += 1
        if delta is not None and abs(delta) <= 15:      # one device pixel
            near += 1
        print(f"{size:>5} {count:>6} "
              f"{'-' if ref is None else f'{ref:9.1f}'} "
              f"{'-' if stated is None else f'{stated:9.1f}'} "
              f"{'-' if ours is None else f'{ours:9.1f}'} "
              f"{'-' if delta is None else f'{delta:8.1f}'}")

    print(f"\ncontrol: twelve-point single line reads {control} twips "
          f"(the site says 300) -> {'PASS' if control and abs(control - 300) < 1 else 'FAIL'}")
    print(f"exact (<0.5 twip): {exact} of {len(rows)}; within one device pixel: {near} of {len(rows)}")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
