#!/usr/bin/env python3
"""One DOCX per OMML shape, so 26.2.4.2's own resolved svg:height can be read for each."""
import os, re, zipfile
SRC = "/home/user/sample-files/words/ceiling-001/docx/ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx"
OUT = "/home/user/libreoffice-core/dotnet/probes/blankpage-r163/mathsizes"
os.makedirs(OUT, exist_ok=True)
with zipfile.ZipFile(SRC) as z:
    PARTS = {i.filename: z.read(i.filename) for i in z.infolist()}
doc = PARTS["word/document.xml"].decode("utf-8")
sect = re.sub(r"<w:(?:header|footer)Reference[^>]*/>", "",
              re.search(r"<w:sectPr [^>]*>.*?</w:sectPr>", doc, re.S).group(0))
HDR = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
       '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
       ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"'
       ' xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math"><w:body>')
def mr(t, sz=None):
    rpr = '<w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/>' \
          + (('<w:sz w:val="%d"/>' % (sz * 2)) if sz else '') + '</w:rPr>'
    return '<m:r>%s<m:t>%s</m:t></m:r>' % (rpr, t)
SHAPES = {
 "plain":      mr("x"),
 "sub":        '<m:sSub><m:e>%s</m:e><m:sub>%s</m:sub></m:sSub>' % (mr("x"), mr("1")),
 "sup":        '<m:sSup><m:e>%s</m:e><m:sup>%s</m:sup></m:sSup>' % (mr("x"), mr("2")),
 "subsup":     '<m:sSubSup><m:e>%s</m:e><m:sub>%s</m:sub><m:sup>%s</m:sup></m:sSubSup>' % (mr("x"), mr("1"), mr("2")),
 "frac":       '<m:f><m:num>%s</m:num><m:den>%s</m:den></m:f>' % (mr("a"), mr("b")),
 "fracnest":   '<m:f><m:num><m:f><m:num>%s</m:num><m:den>%s</m:den></m:f></m:num><m:den>%s</m:den></m:f>' % (mr("a"), mr("b"), mr("c")),
 "rad":        '<m:rad><m:deg/><m:e>%s</m:e></m:rad>' % mr("x"),
 "plain-sz8":  mr("x", 8),
 "plain-sz20": mr("x", 20),
 "sub-sz20":   '<m:sSub><m:e>%s</m:e><m:sub>%s</m:sub></m:sSub>' % (mr("x", 20), mr("1", 20)),
}
for name, body in SHAPES.items():
    p = '<w:p><m:oMath>%s</m:oMath></w:p>' % body
    xml = HDR + '<w:p><w:r><w:t>before</w:t></w:r></w:p>' + p + '<w:p><w:r><w:t>after</w:t></w:r></w:p>' + sect + "</w:body></w:document>"
    path = os.path.join(OUT, "math-%s.docx" % name)
    if os.path.exists(path): os.remove(path)
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zo:
        for n, data in PARTS.items():
            if n.startswith(("customXml/", "docProps/")): continue
            if n.startswith("word/") and not re.match(r"word/(styles|settings|fontTable|numbering|theme/[^/]+|webSettings)\.xml$", n): continue
            if n in ("[Content_Types].xml", "word/_rels/document.xml.rels"): continue
            zo.writestr(n, data)
        r = PARTS["word/_rels/document.xml.rels"].decode()
        kept = [t.group(0) for t in re.finditer(r"<Relationship [^>]*/>", r)
                if re.match(r"(styles|settings|fontTable|numbering|theme/[^/]+|webSettings)\.xml$",
                            re.search(r'Target="([^"]*)"', t.group(0)).group(1))]
        zo.writestr("word/_rels/document.xml.rels",
                    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                    + "".join(kept) + "</Relationships>")
        ct = PARTS["[Content_Types].xml"].decode()
        ct = re.sub(r'<Override PartName="/(word/(header|footer|charts|drawings|embeddings|endnotes|footnotes)[^"]*|customXml[^"]*|docProps[^"]*)"[^>]*/>', "", ct)
        zo.writestr("[Content_Types].xml", ct)
        zo.writestr("word/document.xml", xml)
    print("built", path)
