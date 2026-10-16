"""Rounds three to five: what a cell has to hold to count as empty, and on the real document.

Round one (`make.py`) showed `w:hideMark` on every cell of an *empty* row is what fixes its height,
and round two (`make2.py`) ruled out the four things the corpus's graph paper carries that round
one's table did not — `w:vAlign`, `w:shd`, 256-twip columns and a floating `w:tblpPr`. None of them
explains `084_Printable_Graph_Paper_Template_Editable_Layout`, whose cells each hold one no-break
space and whose rows are nonetheless exactly `w:trHeight` tall.

`out4` asks what content counts as empty, `out7` crosses that with the declared compatibility mode,
and `out3` asks the same three questions of the real document by changing one thing in it at a time.
The answer is the pair of rules recorded in `results.md`.
"""
import os
import re
import sys
import zipfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import make  # noqa: E402

NBSP = " "
CORPUS = "/home/user/sample-files/words/chartset-005/docx"
REAL = f"{CORPUS}/084_Printable_Graph_Paper_Template_Editable_Layout_d66c6820.docx"


def settings(mode):
    """`word/settings.xml`, optionally declaring a compatibility mode."""
    if mode is None:
        return make.SETTINGS

    return make.SETTINGS.replace(
        "/>",
        '><w:compat><w:compatSetting w:name="compatibilityMode"'
        f' w:uri="http://schemas.microsoft.com/office/word" w:val="{mode}"/>'
        "</w:compat></w:settings>")


def build(folder, name, document, mode=None):
    os.makedirs(folder, exist_ok=True)
    with zipfile.ZipFile(f"{folder}/{name}.docx", "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", make.CT)
        z.writestr("_rels/.rels", make.RELS)
        z.writestr("word/_rels/document.xml.rels", make.DRELS)
        z.writestr("word/settings.xml", settings(mode))
        z.writestr("word/styles.xml", make.STYLES)
        z.writestr("word/document.xml", document)
    print(f"{folder}/{name}.docx")


# out4: which cell content is "empty", with no compatibility mode stated.
for name, text in {
    "c-none": None, "c-space": " ", "c-nbsp": NBSP, "c-nbsp2": NBSP * 2,
    "c-tab": "\t", "c-x": "x", "c-dot": ".",
}.items():
    build("out4", name, make.document(True, text))

# out7: the same question crossed with the declared compatibility mode.
for mode in (None, 12, 14, 15):
    for hide in (True, False):
        for label, text in (("none", None), ("nbsp", NBSP), ("x", "x")):
            build("out7", f"f-{mode}-{'hide' if hide else 'plain'}-{label}",
                  make.document(hide, text), mode)


# out3: the real document with exactly one thing changed.
def variant(name, change):
    os.makedirs("out3", exist_ok=True)
    source = zipfile.ZipFile(REAL)
    with zipfile.ZipFile(f"out3/{name}.docx", "w", zipfile.ZIP_DEFLATED) as out:
        for item in source.infolist():
            data = source.read(item.filename)
            if item.filename == "word/document.xml":
                data = change(data.decode("utf-8")).encode("utf-8")
            out.writestr(item, data)
    print(f"out3/{name}.docx")


CELL_RUN = (
    '<w:r w:rsidRPr="00724C50"><w:rPr><w:rFonts w:ascii="Calibri"'
    ' w:eastAsia="Times New Roman" w:hAnsi="Calibri" w:cs="Calibri"/>'
    f'<w:color w:val="000000"/><w:lang w:val="en-US"/></w:rPr><w:t>{NBSP}</w:t></w:r>')

variant("real-nohidemark", lambda d: d.replace("<w:hideMark/>", ""))
variant("real-textcells", lambda d: d.replace(f"<w:t>{NBSP}</w:t>", "<w:t>x</w:t>"))
variant("real-emptycells", lambda d: d.replace(CELL_RUN, ""))
