#!/usr/bin/env python3
"""Two questions about a section's *first* page, both asked with one-paragraph DOCX.

A. Does an absent first-page header still cost the body its height?
   `w:titlePg` with only a `default` headerReference declares that the first page has its own
   header and does not state one.  `SectionPropertyMap::CloseSectionGroup` sets
   `PROP_HEADER_NO_FIRST` in that case (`sw/source/writerfilter/dmapper/PropertyMap.cxx`:601-607)
   rather than turning the header off, so the question is what Writer's layout does with a header
   that is on and empty.

B. Is a paragraph whose first line is taller than the body moved to the next page?
   `SwFlowFrame::IsFwdMoveAllowed` is `GetIndPrev() != nullptr` (`sw/source/core/inc/flowfrm.hxx`
   :243-246) and `SwFlowFrame::MoveFwd` returns false on it when there is no page break
   (`sw/source/core/layout/flowfrm.cxx`:2116-2145); `WidowsAndOrphans::FindBreak` returns false
   when the break lands on line 1 because `PrevLine()` fails (`widorp.cxx`:423-441).  So the
   prediction is: overflow when the paragraph is the body's first, move when it is not.
"""
import struct, sys, zlib, pathlib

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
A = 'http://schemas.openxmlformats.org/drawingml/2006/main'
WP = 'http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing'
PIC = 'http://schemas.openxmlformats.org/drawingml/2006/picture'

EMU = 12700  # per point


def png(w=8, h=8):
    def chunk(tag, data):
        return struct.pack('>I', len(data)) + tag + data + struct.pack('>I', zlib.crc32(tag + data))
    raw = b''.join(b'\x00' + b'\x40\x80\xc0' * w for _ in range(h))
    return (b'\x89PNG\r\n\x1a\n'
            + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0))
            + chunk(b'IDAT', zlib.compress(raw))
            + chunk(b'IEND', b''))


def ct(parts):
    over = ''.join(f'<Override PartName="/{p}" ContentType="{c}"/>' for p, c in parts)
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
            '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
            '<Default Extension="xml" ContentType="application/xml"/>'
            '<Default Extension="png" ContentType="image/png"/>' + over + '</Types>')


ROOT_RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
             f'<Relationship Id="rId1" Type="{R}/officeDocument" Target="word/document.xml"/></Relationships>')


def header(text):
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<w:hdr xmlns:w="{W}"><w:p><w:r><w:t>{text}</w:t></w:r></w:p></w:hdr>')


def drawing(cx, cy, rid):
    return (f'<w:drawing xmlns:wp="{WP}"><wp:inline distT="0" distB="0" distL="0" distR="0">'
            f'<wp:extent cx="{cx}" cy="{cy}"/><wp:docPr id="1" name="p"/>'
            f'<a:graphic xmlns:a="{A}"><a:graphicData uri="{PIC}">'
            f'<pic:pic xmlns:pic="{PIC}"><pic:nvPicPr><pic:cNvPr id="1" name="p"/><pic:cNvPicPr/></pic:nvPicPr>'
            f'<pic:blipFill><a:blip xmlns:r="{R}" r:embed="{rid}"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill>'
            f'<pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
            f'<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></pic:spPr></pic:pic>'
            '</a:graphicData></a:graphic></wp:inline></w:drawing>')


def build(path, body, rels, parts):
    import zipfile
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ct(parts))
        z.writestr('_rels/.rels', ROOT_RELS)
        z.writestr('word/document.xml', body)
        z.writestr('word/_rels/document.xml.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                   + rels + '</Relationships>')
        for name, data in EXTRA:
            z.writestr(name, data)


DOCPART = ('word/document.xml',
           'application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml')
HDRPART = 'application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml'

out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.')
out.mkdir(parents=True, exist_ok=True)
IMG = png()

# ---------------------------------------------------------------- A: the header cases
def sect(extra):
    return ('<w:sectPr>' + extra +
            '<w:pgSz w:w="12240" w:h="15840"/>'
            '<w:pgMar w:top="0" w:right="0" w:bottom="0" w:left="0" w:header="0" w:footer="0" w:gutter="0"/>'
            '</w:sectPr>')


def doc(bodyparas, sectextra):
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<w:document xmlns:w="{W}" xmlns:r="{R}"><w:body>'
            + bodyparas + '<w:p><w:pPr>' + sect(sectextra) + '</w:pPr></w:p></w:body></w:document>')


PARA = '<w:p><w:r><w:t>BODYTOP</w:t></w:r></w:p>'

EXTRA = []
cases = {
    'hdr-none':          ('', '', []),
    'hdr-default':       (f'<w:headerReference w:type="default" r:id="rId9"/>', 'default', ['h']),
    'hdr-titlepg':       (f'<w:headerReference w:type="default" r:id="rId9"/><w:titlePg/>', 'titlepg', ['h']),
    'hdr-titlepg-first': (f'<w:headerReference w:type="default" r:id="rId9"/>'
                          f'<w:headerReference w:type="first" r:id="rId10"/><w:titlePg/>', 'both', ['h', 'f']),
}
for name, (sx, _kind, hdrs) in cases.items():
    EXTRA = []
    rels = ''
    parts = [DOCPART]
    if 'h' in hdrs:
        EXTRA.append(('word/header1.xml', header('DEFAULT-HEADER')))
        rels += f'<Relationship Id="rId9" Type="{R}/header" Target="header1.xml"/>'
        parts.append(('word/header1.xml', HDRPART))
    if 'f' in hdrs:
        EXTRA.append(('word/header2.xml', header('FIRST-HEADER')))
        rels += f'<Relationship Id="rId10" Type="{R}/header" Target="header2.xml"/>'
        parts.append(('word/header2.xml', HDRPART))
    build(out / f'{name}.docx', doc(PARA, sx), rels, parts)

# ---------------------------------------------------------------- B: the tall-line cases
def picdoc(before, cy_pt):
    cy = int(round(cy_pt * EMU))
    cx = int(round(612.5 * EMU))
    pic = ('<w:p><w:r>' + drawing(cx, cy, 'rId8') + '</w:r></w:p>')
    after = '<w:p><w:r><w:t>AFTER</w:t></w:r></w:p>'
    return doc((before or '') + pic + after, '')


for name, before, cy in [
        ('pic-first-fits', '', 700.0),
        ('pic-first-over', '', 792.65),
        ('pic-second-over', '<w:p><w:r><w:t>BEFORE</w:t></w:r></w:p>', 792.65),
        ('pic-first-huge', '', 1200.0),
]:
    EXTRA = [('word/media/image1.png', IMG)]
    rels = f'<Relationship Id="rId8" Type="{R}/image" Target="media/image1.png"/>'
    build(out / f'{name}.docx', picdoc(before, cy), rels, [DOCPART])

print('wrote', len(list(out.glob('*.docx'))), 'documents to', out)
