#!/usr/bin/env python3
"""Two re-checks of SlideTextLayout claims dated 24.2.7.2, in one deck each.

A: OnGrid (:145) -- "a shape's rectangle is an integer number of hundredths of a millimetre
   in the reference".  Ten boxes whose top edge steps by 40 EMU (1/9 of a unit).  If the
   claim holds the reference's first baseline is a staircase of 1-unit steps, not a ramp.

B: HeightToLastNonEmpty (:297) -- "only a RUN OF EMPTY PARAGRAPHS AT THE END is dropped
   from the height the shrink-to-fit search measures".  The same text in the same autofit
   box, with four empty paragraphs after it and with three of them moved into the middle.
   If the claim holds the first fits larger than the second.
"""
import sys, zipfile, importlib.util, os
d = "/c/sandbox/workdir/wt-slides-r50/dotnet/probes/slides-r52"
spec = importlib.util.spec_from_file_location("fp", os.path.join(d, "make-fit-probe.py"))
fp = importlib.util.module_from_spec(spec); spec.loader.exec_module(fp)

WORDS = fp.WORDS

def para(text, size):
    return (f'<a:p><a:pPr/><a:r><a:rPr lang="en-GB" sz="{size}">'
            f'<a:latin typeface="Liberation Sans"/></a:rPr><a:t>{text}</a:t></a:r></a:p>')

def empty(size):
    return (f'<a:p><a:pPr/><a:endParaRPr lang="en-GB" sz="{size}">'
            f'<a:latin typeface="Liberation Sans"/></a:endParaRPr></a:p>')

def slide_grid(y):
    body = para("Hxy", 4000)
    parts = [fp.shape(2, 'Spacer', 200000, 100000, 2000000, 400000,
                      fp.paragraph('spacer', 1200), '<a:noAutofit/>'),
             fp.shape(3, 'G', 200000, y, 6000000, 3600000, body, '<a:noAutofit/>')]
    return fp.HEAD + ''.join(parts) + fp.TAIL

def slide_ntp(trailing, middle):
    ps = [para(WORDS, 4000) for _ in range(3)]
    body = []
    body.append(ps[0])
    for _ in range(middle):
        body.append(empty(4000))
    body.append(ps[1]); body.append(ps[2])
    for _ in range(trailing):
        body.append(empty(4000))
    parts = [fp.shape(2, 'Spacer', 200000, 100000, 2000000, 400000,
                      fp.paragraph('spacer', 1200), '<a:noAutofit/>'),
             fp.shape(3, 'F', 200000, 700000, int(360 * fp.EMU_PT), int(240 * fp.EMU_PT),
                      ''.join(body), '<a:normAutofit/>')]
    return fp.HEAD + ''.join(parts) + fp.TAIL


def write(path, slides):
    ct = [fp.CT_HEAD]; ids = []; rels = []
    for i in range(len(slides)):
        n = i + 1
        ct.append(f'<Override PartName="/ppt/slides/slide{n}.xml" ContentType='
                  f'"application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>')
        ids.append(f'<p:sldId id="{255 + n}" r:id="rId{100 + n}"/>')
        rels.append(f'<Relationship Id="rId{100 + n}" Type="http://schemas.openxmlformats.org'
                    f'/officeDocument/2006/relationships/slide" Target="slides/slide{n}.xml"/>')
    ct.append('</Types>')
    pres = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<p:presentation xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"'
            ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"'
            ' xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">'
            '<p:sldMasterIdLst><p:sldMasterId id="2147483648" r:id="rId1"/></p:sldMasterIdLst>'
            f'<p:sldIdLst>{"".join(ids)}</p:sldIdLst>'
            f'<p:sldSz cx="{fp.SLIDE_W}" cy="{fp.SLIDE_H}"/><p:notesSz cx="6858000" cy="9144000"/>'
            '</p:presentation>')
    pres_rels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                 '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                 '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" Target="slideMasters/slideMaster1.xml"/>'
                 f'{"".join(rels)}</Relationships>')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ''.join(ct))
        z.writestr('_rels/.rels', fp.ROOT_RELS)
        z.writestr('ppt/presentation.xml', pres)
        z.writestr('ppt/_rels/presentation.xml.rels', pres_rels)
        z.writestr('ppt/slideLayouts/slideLayout1.xml', fp.LAYOUT)
        z.writestr('ppt/slideLayouts/_rels/slideLayout1.xml.rels', fp.LAYOUT_RELS)
        z.writestr('ppt/slideMasters/slideMaster1.xml', fp.MASTER)
        z.writestr('ppt/slideMasters/_rels/slideMaster1.xml.rels', fp.MASTER_RELS)
        z.writestr('ppt/theme/theme1.xml', fp.THEME)
        for i, s in enumerate(slides):
            z.writestr(f'ppt/slides/slide{i + 1}.xml', s)
            z.writestr(f'ppt/slides/_rels/slide{i + 1}.xml.rels', fp.SLIDE_RELS)

GRID_YS = [699840 + 40 * k for k in range(12)]
write(sys.argv[1], [slide_grid(y) for y in GRID_YS])
write(sys.argv[2], [slide_ntp(4, 0), slide_ntp(1, 3), slide_ntp(0, 4), slide_ntp(0, 0)])
print("grid ys:", GRID_YS)
print("ntp slides: 1=4 trailing, 2=1 trailing+3 middle, 3=4 middle, 4=none")
