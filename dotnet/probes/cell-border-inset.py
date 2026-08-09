#!/usr/bin/env python3
"""Does LibreOffice charge a table cell's border line width against the cell's text width?

**No** — not for a Word table, which is every document on the words track. This probe exists
because the answer looked like "yes" for a round and a 119-document change was measured on it.

The claim under test was: `SwCellFrame::Format` insets a cell's text by
`SwBorderAttrs::CalcLeft`/`CalcRight`, which are `CalcLeftLine()`/`CalcRightLine()` plus the
box's LR spacing, and `CalcLeftLine()` is `SvxBoxItem::CalcLineSpace(LEFT, /*bEvenIfNoLine*/true)`
— the cell margin *plus the scaled border width*. So our `inner = width − padding.Horizontal`
would be a border too wide on every bordered cell in the corpus.

The citation is real and the branch above it is what decides:

    sw/source/core/layout/tabfrm.cxx, SwCellFrame::Format
        if ( pTab->IsCollapsingBorders() && !pLower->IsRowFrame() )
        {
            nLeftSpace  = rBoxItem.GetDistance( SvxBoxItemLine::LEFT );   // the margin ALONE
            nRightSpace = rBoxItem.GetDistance( SvxBoxItemLine::RIGHT );
        }
        else
        {
            nLeftSpace  = pAttrs->CalcLeft( this );    // margin + border line width
            nRightSpace = pAttrs->CalcRight( this );
        }

`RES_COLLAPSING_BORDERS` is defaulted **true** for every Writer document
(sw/source/uibase/app/docshini.cxx:300) and ODF states it per table style as
`table:border-model`. So the border-charging branch is the one a *Writer-native separating*
table takes, and a Word table never takes it.

Three sections, each measured against the installed soffice rather than against the tree,
because the source in this checkout has twice been wrong about a constant the binary got right:

  1. **The branch, isolated by one attribute.** Two flat-ODF files differing only in
     `table:border-model`, same 6 pt border, same padding. Separating loses exactly two
     border widths of text area; collapsing loses none. That is the observation that
     *separates* the two explanations, and no corpus document can provide it — a Word
     document cannot be made to take the other branch.

  2. **Which branch a Word table takes.** DOCX and DOC, one cell, a narrow cell, and the
     interior cell of a two-cell table, at border widths of 0, 0.5, 2 and 6 pt. At Word's
     default cell margin the text-area width is *identical* across all four widths. The
     border is not charged.

  3. **What is really there, and it is not nothing.** At a cell margin *below* half the
     border width the DOCX width does move — and the drawn border proves the table did not
     move, so it is an inset after all. It is neither the margin nor margin+border but

         pad_l = max(bll/2, cml)
         pad_r = max(pad_l + blr/2, cml + cmr) − pad_l

     which `sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx`'s
     `lcl_adjustBorderDistance` applies at *import*, with a comment saying it is what MS Word
     does. It is an OOXML/RTF import adjustment, not a layout rule: the ODF collapsing rows in
     section 1 stay invariant at zero padding, and `WW8TabDesc::SetTabBorders`
     (sw/source/filter/ww8/ww8par2.cxx:3020-3042) sets the distance straight from
     `sprmTCellPadding`/`nGapHalf` with no such floor, so `.doc` — 66 of the words track's 200
     documents — is not subject to it at all.

     Reduced: for the symmetric case the total charged is `2·max(m, b/2)`, so it equals the
     plain margin whenever the margin is at least half the border, which is nearly always.
     A zip census over the 134 DOCX of the 200 finds 7 documents with any table where it
     bites at all, 171 cells of 78287.

Run it with no arguments; it authors its own fixtures and prints one table per section.
Every fixture states its font in `word/styles.xml` — an authored probe with no styles part
lays out in the fallback face, and every length it then measures is meaningless.
"""
import re
import shutil
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path

OPS = Path(__file__).resolve().parents[2] / ".claude/skills/render-comparison/scripts/pdf-ops.py"

FODT = """<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.text">
 <office:automatic-styles>
  <style:style style:name="Tbl" style:family="table">
   <style:table-properties style:width="16cm" table:align="left"
     fo:margin-left="0cm" table:border-model="{model}"/>
  </style:style>
  <style:style style:name="Col" style:family="table-column">
   <style:table-column-properties style:column-width="16cm"/>
  </style:style>
  <style:style style:name="Cell" style:family="table-cell">
   <style:table-cell-properties fo:padding-left="{pad}" fo:padding-right="{pad}"
     fo:padding-top="0cm" fo:padding-bottom="0cm"
     fo:border-left="{bw} solid #000000" fo:border-right="{bw} solid #000000"
     fo:border-top="{bw} solid #000000" fo:border-bottom="{bw} solid #000000"/>
  </style:style>
  <style:style style:name="PL" style:family="paragraph">
   <style:paragraph-properties fo:text-align="start" fo:margin-left="0cm"
     fo:margin-right="0cm"/>
   <style:text-properties style:font-name="Liberation Serif" fo:font-size="10pt"/>
  </style:style>
  <style:style style:name="PR" style:family="paragraph">
   <style:paragraph-properties fo:text-align="end" fo:margin-left="0cm"
     fo:margin-right="0cm"/>
   <style:text-properties style:font-name="Liberation Serif" fo:font-size="10pt"/>
  </style:style>
 </office:automatic-styles>
 <office:body><office:text>
  <table:table table:name="T" table:style-name="Tbl">
   <table:table-column table:style-name="Col"/>
   <table:table-row>
    <table:table-cell table:style-name="Cell" office:value-type="string">
     <text:p text:style-name="PL">LLL</text:p>
     <text:p text:style-name="PR">RRR</text:p>
    </table:table-cell>
   </table:table-row>
  </table:table>
  <text:p text:style-name="PL">TAIL</text:p>
 </office:text></office:body>
</office:document>
"""

DOCX_DOC = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
 <w:body>
  <w:tbl>
   <w:tblPr>
    <w:tblW w:w="{tw}" w:type="dxa"/><w:tblLayout w:type="fixed"/>
    {borders}
    <w:tblCellMar>
     <w:top w:w="0" w:type="dxa"/><w:left w:w="{cm}" w:type="dxa"/>
     <w:bottom w:w="0" w:type="dxa"/><w:right w:w="{cm}" w:type="dxa"/>
    </w:tblCellMar>
   </w:tblPr>
   <w:tblGrid><w:gridCol w:w="{c1}"/>{col2}</w:tblGrid>
   <w:tr><w:tc>
    <w:tcPr><w:tcW w:w="{c1}" w:type="dxa"/></w:tcPr>
    <w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:t>LLL</w:t></w:r></w:p>
    <w:p><w:pPr><w:jc w:val="right"/></w:pPr><w:r><w:t>RRR</w:t></w:r></w:p>
   </w:tc>{cell2}</w:tr>
  </w:tbl>
  <w:p><w:r><w:t>TAIL</w:t></w:r></w:p>
  <w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
   <w:pgMar w:top="1417" w:right="1417" w:bottom="1417" w:left="1417"
    w:header="708" w:footer="708" w:gutter="0"/></w:sectPr>
 </w:body></w:document>
"""

CELL2 = """<w:tc><w:tcPr><w:tcW w:w="{c2}" w:type="dxa"/></w:tcPr>
    <w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:t>MMM</w:t></w:r></w:p>
    <w:p><w:pPr><w:jc w:val="right"/></w:pPr><w:r><w:t>NNN</w:t></w:r></w:p>
   </w:tc>"""

BORDERS = """<w:tblBorders>
     <w:top w:val="single" w:sz="{sz}" w:space="0" w:color="000000"/>
     <w:left w:val="single" w:sz="{sz}" w:space="0" w:color="000000"/>
     <w:bottom w:val="single" w:sz="{sz}" w:space="0" w:color="000000"/>
     <w:right w:val="single" w:sz="{sz}" w:space="0" w:color="000000"/>
    </w:tblBorders>"""

NO_BORDERS = """<w:tblBorders>
     <w:top w:val="none" w:sz="0" w:space="0" w:color="auto"/>
     <w:left w:val="none" w:sz="0" w:space="0" w:color="auto"/>
     <w:bottom w:val="none" w:sz="0" w:space="0" w:color="auto"/>
     <w:right w:val="none" w:sz="0" w:space="0" w:color="auto"/>
    </w:tblBorders>"""

CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
 <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
 <Default Extension="xml" ContentType="application/xml"/>
 <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
 <Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
</Types>"""

RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
 <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DRELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
 <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>"""

STYLES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
 <w:docDefaults><w:rPrDefault><w:rPr>
  <w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"
   w:eastAsia="Liberation Serif" w:cs="Liberation Serif"/>
  <w:sz w:val="20"/><w:szCs w:val="20"/></w:rPr></w:rPrDefault>
  <w:pPrDefault><w:pPr><w:spacing w:after="0" w:line="240" w:lineRule="auto"/>
   </w:pPr></w:pPrDefault></w:docDefaults>
 <w:style w:type="paragraph" w:default="1" w:styleId="Normal">
  <w:name w:val="Normal"/><w:qFormat/></w:style></w:styles>"""


def write_docx(path, sz, cellmar, c1=9072, c2=0):
    borders = NO_BORDERS if sz == 0 else BORDERS.format(sz=sz)
    doc = DOCX_DOC.format(
        borders=borders, cm=cellmar, tw=c1 + c2, c1=c1,
        col2=(f'<w:gridCol w:w="{c2}"/>' if c2 else ""),
        cell2=(CELL2.format(c2=c2) if c2 else ""))
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/_rels/document.xml.rels", DRELS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/document.xml", doc)


def convert(src, outdir, fmt="pdf"):
    prof = outdir / "prof"
    prof.mkdir(parents=True, exist_ok=True)
    subprocess.run(["soffice", f"-env:UserInstallation=file://{prof}", "--headless",
                    "--convert-to", fmt, "--outdir", str(outdir), str(src)],
                   capture_output=True, timeout=180)
    out = outdir / (src.stem + "." + fmt)
    return out if out.exists() else None


WORD_RE = re.compile(
    r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>')


def measure(pdf, pair=("LLL", "RRR"), want_frame=False):
    """The text area's two edges, from a flush-left and a flush-right word.

    A wrap moves only when the change crosses a token boundary, so measuring a wrap answers
    this question only sometimes; the two edges answer it always.
    """
    if pdf is None:
        return None
    out = subprocess.run(["pdftotext", "-bbox", str(pdf), "-"],
                         capture_output=True, text=True).stdout
    words = {}
    for a, b, c, d, t in WORD_RE.findall(out):
        words.setdefault(t, (float(a), float(c)))
    if pair[0] not in words or pair[1] not in words:
        return None
    r = {"left": words[pair[0]][0], "right": words[pair[1]][1],
         "tail": words.get("TAIL", (float("nan"),))[0]}
    r["width"] = r["right"] - r["left"]
    if want_frame:
        # With collapsing borders Writer paints a cell's border centred on the frame edge, so
        # the stroke's own coordinate *is* the frame edge — which is how a reposition is told
        # apart from an inset without trusting any arithmetic about table placement.
        dump = subprocess.run([sys.executable, str(OPS), "dump", str(pdf), "--only", "stroke"],
                              capture_output=True, text=True).stdout
        vert = sorted({
            float(m.group(1))
            for m in re.finditer(r"stroke\s+\S+\s+\(\s*([\d.]+),\s*([\d.]+)\)-\(\s*([\d.]+),", dump)
            if abs(float(m.group(1)) - float(m.group(3))) < 0.01})
        r["frameL"] = vert[0] if vert else None
        r["frameR"] = vert[-1] if vert else None
    return r


def table(rows, header):
    w = max(len(n) for n, _ in rows)
    print("  " + "fixture".ljust(w) + "  " + header)
    for name, m in rows:
        if m is None:
            print("  " + name.ljust(w) + "  (no rendering)")
        else:
            print("  " + name.ljust(w) + "  " + m)


def section1(tmp):
    print("\n1. THE BRANCH, isolated by one attribute (`table:border-model`).")
    print("   Same 6 pt border, same padding; only the model differs.\n")
    rows = []
    for model in ("collapsing", "separating"):
        for bw in ("0.02pt", "2pt", "6pt"):
            for pad in ("0cm", "0.19cm"):
                src = tmp / f"odf-{model}-{bw}-{pad}.fodt".replace(".pt", "pt")
                src.write_text(FODT.format(model=model, bw=bw, pad=pad))
                m = measure(convert(src, tmp))
                rows.append((f"odf {model:<10} border={bw:<7} pad={pad:<7}",
                             None if m is None else
                             f"textL {m['left']:7.2f}  textR {m['right']:7.2f}  WIDTH {m['width']:7.2f}"))
    table(rows, "")
    print("\n   Collapsing: the width does not move with the border. Separating: it loses 2 x border.")


def section2(tmp):
    print("\n2. WHICH BRANCH A WORD TABLE TAKES.  Cell margin 108 twips, which is Word's default.\n")
    rows = []
    for sz in (0, 4, 16, 48):
        for label, kw in (("one cell, 9072tw", {}),
                          ("narrow, 5000tw", {"c1": 5000}),
                          ("cell 2 of 2", {"c1": 4536, "c2": 4536})):
            src = tmp / f"d2-{sz}-{label.replace(' ', '')}.docx"
            write_docx(src, sz, 108, **kw)
            pair = ("MMM", "NNN") if "c2" in kw else ("LLL", "RRR")
            m = measure(convert(src, tmp), pair)
            rows.append((f"docx border={sz / 8:.2f}pt  {label:<16}",
                         None if m is None else f"WIDTH {m['width']:8.2f}"))
    # The binary half of the track reaches the layout through a different reader.
    for sz in (0, 48):
        src = tmp / f"dd-{sz}.docx"
        write_docx(src, sz, 108)
        doc = convert(src, tmp, "doc")
        m = measure(convert(doc, tmp), ("LLL", "RRR")) if doc else None
        rows.append((f"doc  border={sz / 8:.2f}pt  one cell, 9072tw",
                     None if m is None else f"WIDTH {m['width']:8.2f}"))
    table(rows, "")
    print("\n   Every width is the same at every border width. The border is not charged.")


def section3(tmp):
    print("\n3. WHAT IS REALLY THERE: the inset is max(cell margin, half the border).")
    print("   frameL/frameR are the border strokes, which are centred on the cell frame edge.\n")
    rows = []
    for sz in (48, 16, 4):
        for cm in (0, 20, 40, 60, 80, 100, 120):
            src = tmp / f"d3-{sz}-{cm}.docx"
            write_docx(src, sz, cm, c1=5000)
            m = measure(convert(src, tmp), want_frame=True)
            if m is None or m["frameL"] is None:
                rows.append((f"docx border={sz / 8:.2f}pt  margin={cm / 20:.2f}pt", None))
                continue
            rows.append((
                f"docx border={sz / 8:.2f}pt  margin={cm / 20:.2f}pt",
                f"insetL {m['left'] - m['frameL']:6.2f}  insetR {m['frameR'] - m['right']:6.2f}"
                f"   predicted {max(cm / 20, sz / 16):5.2f}"))
        rows.append((f"{'':<38}", ""))
    table([(n, v) for n, v in rows if v != ""], "")
    print("\n   insetL carries the left side bearing of 'L' (about +0.05 pt) and insetR the right")
    print("   side bearing of 'R' (about -0.09 pt); the steps between rows are exact.")


def main():
    if not shutil.which("soffice"):
        print("no soffice on PATH", file=sys.stderr)
        return 2
    tmp = Path(tempfile.mkdtemp(prefix="cell-border-inset-"))
    print(__doc__.split("\n\n")[0])
    section1(tmp)
    section2(tmp)
    section3(tmp)
    print(f"\nfixtures kept in {tmp}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
