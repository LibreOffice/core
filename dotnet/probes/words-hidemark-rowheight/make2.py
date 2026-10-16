"""Second round: what makes the corpus's graph-paper rows fixed when a bare `w:hideMark` does not.

`084_Printable_Graph_Paper_Template_Editable_Layout` draws 9.00 pt rows against a `w:trHeight` of
180 twips while every cell holds a no-break space, and round one showed a no-break space is enough
to make LibreOffice treat the row as non-empty. So something else in that table is doing it. The
four things its cells and table carry that round one's did not: `w:vAlign bottom`, `w:shd`, a cell
width of 256 twips (12.8 pt, narrower than the 216 twips of default cell margin), and a floating
`w:tblpPr` anchored to the page.
"""
import zipfile, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from make import CT, RELS, DRELS, SETTINGS, STYLES, W

def cell(width, hide, text, valign, shd):
    bits = f'<w:tcW w:w="{width}" w:type="dxa"/>'
    if shd: bits += '<w:shd w:val="clear" w:color="auto" w:fill="auto"/>'
    bits += '<w:noWrap/>'
    if valign: bits += '<w:vAlign w:val="bottom"/>'
    if hide: bits += '<w:hideMark/>'
    run = "" if text is None else (
        '<w:r><w:rPr><w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/></w:rPr>'
        f'<w:t xml:space="preserve">{text}</w:t></w:r>')
    return (f'<w:tc><w:tcPr>{bits}</w:tcPr>'
            '<w:p><w:pPr><w:spacing w:after="0" w:line="240" w:lineRule="auto"/></w:pPr>'
            f'{run}</w:p></w:tc>')

def document(width, hide, text, valign, shd, floating, columns=3):
    rows = "".join(
        '<w:tr><w:trPr><w:trHeight w:val="180"/></w:trPr>'
        + "".join(cell(width, hide, text, valign, shd) for _ in range(columns))
        + "</w:tr>" for _ in range(10))
    pos = ('<w:tblpPr w:leftFromText="180" w:rightFromText="180" w:vertAnchor="page"'
           ' w:horzAnchor="margin" w:tblpY="1025"/>') if floating else ""
    grid = "".join(f'<w:gridCol w:w="{width}"/>' for _ in range(columns))
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {W}><w:body>
<w:tbl><w:tblPr>{pos}<w:tblW w:w="{width * columns}" w:type="dxa"/>
<w:tblBorders>
<w:top w:val="single" w:sz="8" w:space="0" w:color="44546A"/>
<w:left w:val="single" w:sz="8" w:space="0" w:color="44546A"/>
<w:bottom w:val="single" w:sz="8" w:space="0" w:color="44546A"/>
<w:right w:val="single" w:sz="8" w:space="0" w:color="44546A"/>
<w:insideH w:val="single" w:sz="8" w:space="0" w:color="44546A"/>
<w:insideV w:val="single" w:sz="8" w:space="0" w:color="44546A"/>
</w:tblBorders></w:tblPr>
<w:tblGrid>{grid}</w:tblGrid>
{rows}</w:tbl>
<w:p/>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134" w:header="709" w:footer="709" w:gutter="0"/>
</w:sectPr></w:body></w:document>"""

NBSP = " "
CASES = {
    # width hide  text   valign shd   floating cols
    "b-nbsp-valign":   (1200, True,  NBSP, True,  False, False, 3),
    "b-nbsp-shd":      (1200, True,  NBSP, False, True,  False, 3),
    "b-nbsp-float":    (1200, True,  NBSP, False, False, True,  3),
    "b-nbsp-narrow":   (256,  True,  NBSP, False, False, False, 36),
    "b-nbsp-all":      (256,  True,  NBSP, True,  True,  True,  36),
    "b-plain-narrow":  (256,  False, NBSP, True,  True,  True,  36),
    "b-empty-narrow":  (256,  True,  None, True,  True,  True,  36),
}

os.makedirs("out2", exist_ok=True)
for name, args in CASES.items():
    with zipfile.ZipFile(f"out2/{name}.docx", "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CT)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/_rels/document.xml.rels", DRELS)
        z.writestr("word/settings.xml", SETTINGS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/document.xml", document(*args))
    print("out2/" + name + ".docx")
