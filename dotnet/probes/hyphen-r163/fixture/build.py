#!/usr/bin/env python3
"""Build the smallest DOCX that reproduces r163's page break.

One REF field whose cached result is short and whose bookmark covers a whole caption.
LibreOffice recomputes the field from the bookmark on load, the note gains a wrapped line,
and the sentence after it no longer fits the page. Word -- and this tree by default --
draws the cache, keeps the note at one line, and keeps the sentence on page 1.

A `word/settings.xml` part is present on purpose: without one the importer does not take
LibreOffice's OOXML compatibility defaults and the fixture answers a different question.
"""
import os, sys, zipfile
FILLERS = int(sys.argv[1]) if len(sys.argv) > 1 else 15

CAPTION_TAIL = ": Adjusted Allowance Times for SAE Type IV Propylene Glycol (PG) Fluids"
NOTE_HEAD = "Heavy snow, ice pellets and hail ("
NOTE_TAIL = " provides adjusted allowance times)."
TAIL = "The cautions that apply to the holdover times in the table above can be found on page 9."

def p(text, style=None, extra=""):
    pr = "<w:pPr>" + (f'<w:pStyle w:val="{style}"/>' if style else "") + "</w:pPr>"
    return (f"<w:p>{pr}{extra}"
            f'<w:r><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/><w:sz w:val="18"/></w:rPr>'
            f'<w:t xml:space="preserve">{text}</w:t></w:r></w:p>')

# The caption: bookmark _RefCap spans "Table 2" AND the tail, so recomputation returns both.
caption = (
    '<w:p><w:pPr></w:pPr>'
    '<w:bookmarkStart w:id="1" w:name="_RefCap"/>'
    '<w:r><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/><w:sz w:val="18"/></w:rPr>'
    '<w:t xml:space="preserve">Table 2</w:t></w:r>'
    '<w:r><w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/><w:sz w:val="18"/></w:rPr>'
    f'<w:t xml:space="preserve">{CAPTION_TAIL}</w:t></w:r>'
    '<w:bookmarkEnd w:id="1"/></w:p>')

RPR = '<w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/><w:sz w:val="18"/></w:rPr>'
note = (
    '<w:p><w:pPr></w:pPr>'
    f'<w:r>{RPR}<w:t xml:space="preserve">{NOTE_HEAD}</w:t></w:r>'
    f'<w:r>{RPR}<w:fldChar w:fldCharType="begin"/></w:r>'
    f'<w:r>{RPR}<w:instrText xml:space="preserve"> REF _RefCap \\h </w:instrText></w:r>'
    f'<w:r>{RPR}<w:fldChar w:fldCharType="separate"/></w:r>'
    f'<w:r>{RPR}<w:t xml:space="preserve">Table 2</w:t></w:r>'   # the stale cache
    f'<w:r>{RPR}<w:fldChar w:fldCharType="end"/></w:r>'
    f'<w:r>{RPR}<w:t xml:space="preserve">{NOTE_TAIL}</w:t></w:r>'
    '</w:p>')

FILLER = ("Filler line that holds the page down so the sentence below sits on the last line "
          "that the page has room for at all.")

body = caption + "".join(p(FILLER) for _ in range(FILLERS)) + note + p(TAIL)

# 8.5in x 3.6in, 0.5in margins -> a short page, so one extra wrapped line is a page break.
sect = ('<w:sectPr><w:pgSz w:w="12240" w:h="5184"/>'
        '<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720" '
        'w:header="0" w:footer="0" w:gutter="0"/></w:sectPr>')

document = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
            f'<w:body>{body}{sect}</w:body></w:document>')

settings = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
            '<w:compat>'
            '<w:compatSetting w:name="compatibilityMode" '
            'w:uri="http://schemas.microsoft.com/office/word" w:val="15"/>'
            '</w:compat></w:settings>')

content_types = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
 '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
 '<Default Extension="xml" ContentType="application/xml"/>'
 '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
 '<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>'
 '</Types>')

root_rels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
 '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>'
 '</Relationships>')

doc_rels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
 '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>'
 '</Relationships>')

out = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                   "words-reference-field-wraps-a-page.docx")
with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
    z.writestr("[Content_Types].xml", content_types)
    z.writestr("_rels/.rels", root_rels)
    z.writestr("word/_rels/document.xml.rels", doc_rels)
    z.writestr("word/document.xml", document)
    z.writestr("word/settings.xml", settings)
print(out)
