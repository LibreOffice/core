#!/usr/bin/env python3
"""Variants of the real document that push the empty break paragraph down by X pt.

The last paragraph of page 6 ("Considering the horizontal tail ...") gets an extra
w:spacing w:after of X pt.  Everything else is byte-identical to the original, so the
sweep measures exactly how much slack each renderer has left at the foot of page 6.
"""
import os, re, zipfile

SRC = "/home/user/sample-files/words/ceiling-001/docx/ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx"
OUT = "/home/user/libreoffice-core/dotnet/probes/blankpage-r163/slack"
os.makedirs(OUT, exist_ok=True)

with zipfile.ZipFile(SRC) as z:
    PARTS = {i.filename: z.read(i.filename) for i in z.infolist()}
doc = PARTS["word/document.xml"].decode("utf-8")

# the paragraph that ends page 6, immediately before the break-only paragraph
anchor = '<w:p w:rsidR="00A479C9" w:rsidRPr="00EF44A8" w:rsidRDefault="00A479C9"><w:r w:rsidRPr="00EF44A8"><w:br w:type="page"/></w:r></w:p>'
i = doc.find(anchor)
assert i > 0, "break paragraph not found"
j = doc.rfind("<w:p ", 0, i)
prev = doc[j:i]
assert "Considering" not in prev or True
# it has a pPr with <w:spacing w:line="240" w:lineRule="auto"/> — add w:after
m = re.search(r"<w:pPr>(.*?)</w:pPr>", prev, re.S)
print("prev pPr:", m.group(0) if m else "(none)")

for pt in [x/4 for x in range(32,73)]:
    tw = int(round(pt * 20))
    if m:
        newp = prev[:m.start(1)] + ('<w:spacing w:after="%d" w:line="240" w:lineRule="auto"/>' % tw) \
            + re.sub(r'<w:spacing [^>]*/>', '', m.group(1)) + prev[m.end(1):]
    else:
        newp = prev.replace(">", '><w:pPr><w:spacing w:after="%d"/></w:pPr>' % tw, 1)
    out = doc[:j] + newp + doc[i:]
    path = os.path.join(OUT, "slack-%06.2fpt.docx" % pt)
    if os.path.exists(path): os.remove(path)
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zo:
        for n, data in PARTS.items():
            zo.writestr(n, out.encode("utf-8") if n == "word/document.xml" else data)
    print("built", path)
