"""All 41 text-warp presets, in two arms.

Arm `default` states no `a:avLst`, so LibreOffice applies each preset's own default
adjustment -- a path nothing in the corpus exercises, because every warped shape on the
catalogue states its adjustments explicitly.

Arm `adjusted` uses the catalogue's own values for the 24 presets it carries, and the
preset default elsewhere. Both arms use the catalogue's warped-shape box, 4389120 x 914400
EMU: the first cut of this fixture cloned a `textNoShape` shape at 640080 and every warp
came out mild, which is the "minimal enough to answer a different question" trap in
`.claude/skills/office-corpus`.
"""
import zipfile, pathlib, re, sys

SRC = pathlib.Path('/home/user/sample-files/words/drawingset-001/docx/WordArt_Shapes_Arrows_Catalog1.docx')
PRESETS = ["textNoShape","textPlain","textStop","textTriangle","textTriangleInverted",
 "textChevron","textChevronInverted","textRingInside","textRingOutside","textArchUp",
 "textArchDown","textCircle","textButton","textArchUpPour","textArchDownPour",
 "textCirclePour","textButtonPour","textCurveUp","textCurveDown","textCanUp","textCanDown",
 "textWave1","textWave2","textDoubleWave1","textWave4","textInflate","textDeflate",
 "textInflateBottom","textDeflateBottom","textInflateTop","textDeflateTop",
 "textDeflateInflate","textDeflateInflateDeflate","textFadeRight","textFadeLeft",
 "textFadeUp","textFadeDown","textSlantUp","textSlantDown","textCascadeUp","textCascadeDown"]

z = zipfile.ZipFile(SRC)
doc = z.read('word/document.xml').decode('utf8')

# The catalogue's own adjustment values, per preset.
ADJ = {}
for m in re.finditer(r'<a:prstTxWarp prst="([A-Za-z0-9]+)">(<a:avLst>.*?</a:avLst>)</a:prstTxWarp>', doc):
    ADJ[m.group(1)] = m.group(2)

# A warped shape, verbatim, as the container's own writer emits it -- 914400 tall.
i = next(m.start() for m in re.finditer(r'prstTxWarp prst="(?!textNoShape)', doc))
a = doc.rfind('<mc:AlternateContent', 0, i)
b = doc.find('</mc:AlternateContent>', i) + len('</mc:AlternateContent>')
TEMPLATE = doc[a:b]
BASE = re.search(r'<a:prstTxWarp prst="[A-Za-z0-9]+">.*?</a:prstTxWarp>', TEMPLATE).group(0)

CAPTION = ('<w:p><w:pPr><w:spacing w:before="160" w:after="0"/></w:pPr><w:r><w:rPr>'
           '<w:rFonts w:ascii="Arial" w:hAnsi="Arial"/><w:sz w:val="16"/><w:color w:val="1F4E79"/>'
           '</w:rPr><w:t xml:space="preserve">P{n:03d}  {prst}{note}</w:t></w:r></w:p>')

def build(arm, path):
    body = []
    for n, prst in enumerate(PRESETS, 1):
        av = ADJ.get(prst, '') if arm == 'adjusted' else ''
        warp = f'<a:prstTxWarp prst="{prst}">{av}</a:prstTxWarp>' if av else f'<a:prstTxWarp prst="{prst}"/>'
        s = TEMPLATE.replace(BASE, warp)
        s = re.sub(r'name="[^"]*"', f'name="P{n:03d} {prst}"', s, count=1)
        s = re.sub(r'descr="[^"]*"', f'descr="{prst} {arm}"', s, count=1)
        note = ('  [catalogue adj]' if prst in ADJ else '  [preset default]') if arm == 'adjusted' else '  [preset default]'
        body.append(CAPTION.format(n=n, prst=prst, note=note))
        body.append('<w:p><w:pPr><w:spacing w:before="0" w:after="0"/></w:pPr><w:r>' + s + '</w:r></w:p>')
    head = doc[:doc.find('<w:body>') + len('<w:body>')]
    sect = doc[doc.rfind('<w:sectPr'):doc.rfind('</w:body>')]
    new = head + ''.join(body) + sect + '</w:body></w:document>'
    # Reopen the source per arm: reusing one handle across two writes trips
    # "Bad magic number for file header" on the second pass.
    with zipfile.ZipFile(SRC) as src, zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as o:
        for item in src.infolist():
            o.writestr(item, new.encode('utf8') if item.filename == 'word/document.xml' else src.read(item.filename))
    print(f"  {path.name}: {path.stat().st_size} bytes")

print(f"catalogue states adjustments for {len(ADJ)} presets")
build('default',  pathlib.Path('/home/user/fixtures/fontwork-presets-default.docx'))
build('adjusted', pathlib.Path('/home/user/fixtures/fontwork-presets-adjusted.docx'))
