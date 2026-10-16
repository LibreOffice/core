#!/usr/bin/env python3
"""Build minimal DOCX fixtures for the blankpage-r163 probe.

Parts are copied verbatim from the real document (styles, settings, fontTable,
numbering, theme) so the fixtures inherit exactly the same OOXML compatibility
defaults; only word/document.xml is synthesised.  See office-corpus/SKILL.md:
a hand-built DOCX without word/settings.xml answers a different question.
"""
import os, re, shutil, zipfile, sys

SRC = "/home/user/sample-files/words/ceiling-001/docx/ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx"
OUT = "/home/user/libreoffice-core/dotnet/probes/blankpage-r163/fixtures"

with zipfile.ZipFile(SRC) as _z:
    PARTS = {i.filename: _z.read(i.filename) for i in _z.infolist()}
doc = PARTS["word/document.xml"].decode("utf-8")

# the single sectPr of the real document, reused verbatim minus header/footer refs
sect = re.search(r"<w:sectPr [^>]*>.*?</w:sectPr>", doc, re.S).group(0)
sect = re.sub(r"<w:(?:header|footer)Reference[^>]*/>", "", sect)

# the display-math paragraph "VH = 130 kts" from the real document, verbatim
paras = [m.group(0) for m in re.finditer(r"<w:p [^>]*>.*?</w:p>", doc, re.S)]
MATHP = None
for p in paras:
    if "m:oMath" in p and "".join(re.findall(r"<m:t(?: [^>]*)?>(.*?)</m:t>", p, re.S)).startswith("VH=130"):
        MATHP = p
        break
assert MATHP, "math paragraph not found"

HDR = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
       '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
       ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"'
       ' xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math"'
       ' xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"'
       ' xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"'
       ' xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml"'
       ' xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006"'
       ' mc:Ignorable="w14"><w:body>')

def para(text):
    return "<w:p><w:r><w:t xml:space=\"preserve\">%s</w:t></w:r></w:p>" % text

BREAKP = '<w:p><w:r><w:br w:type="page"/></w:r></w:p>'

def build(name, body):
    path = os.path.join(OUT, name + ".docx")
    if os.path.exists(path):
        os.remove(path)
    zout = zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED)
    for n, data in PARTS.items():
        if n in ("word/document.xml", "[Content_Types].xml", "word/_rels/document.xml.rels"):
            continue
        if n.startswith("customXml/") or n.startswith("docProps/"):
            continue
        if n.startswith("word/") and not re.match(
                r"word/(styles|settings|fontTable|numbering|theme/[^/]+|webSettings|stylesWithEffects)\.xml$", n):
            continue
        zout.writestr(n, data)
    # rewrite document.xml.rels to reference only the kept parts
    r = PARTS["word/_rels/document.xml.rels"].decode("utf-8")
    kept = []
    for m in re.finditer(r"<Relationship [^>]*/>", r):
        t = m.group(0)
        tgt = re.search(r'Target="([^"]*)"', t).group(1)
        if re.match(r"(styles|settings|fontTable|numbering|theme/[^/]+|webSettings)\.xml$", tgt):
            kept.append(t)
    zout.writestr("word/_rels/document.xml.rels",
                  '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
                  '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                  + "".join(kept) + "</Relationships>")
    # content types: strip the parts we dropped
    ct = PARTS["[Content_Types].xml"].decode("utf-8")
    ct = re.sub(r'<Override PartName="/word/(header|footer|charts|drawings|embeddings|endnotes|footnotes)[^"]*"[^>]*/>', "", ct)
    ct = re.sub(r'<Override PartName="/customXml[^"]*"[^>]*/>', "", ct)
    ct = re.sub(r'<Override PartName="/docProps[^"]*"[^>]*/>', "", ct)
    zout.writestr("[Content_Types].xml", ct)
    zout.writestr("word/document.xml", HDR + body + sect + "</w:body></w:document>")
    zout.close()
    print("built", path)

FILL = ("Filler line that is short enough to occupy exactly one line of text. ")

# --- family A: does the empty break-only paragraph fit at the bottom of page 1?
for n in range(24, 34):
    body = "".join(para("A%02d %s" % (i, FILL)) for i in range(n)) + BREAKP + para("AFTER")
    build("breakpara-fill%02d" % n, body)

# --- family B: line height of a display-math paragraph, verbatim from the real doc
build("mathline-plain", "".join(para("L%02d plain" % i) for i in range(6)))
build("mathline-math", para("L00 plain") + para("L01 plain") + MATHP
      + para("L02 plain") + para("L03 plain") + para("L04 plain"))
