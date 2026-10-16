#!/usr/bin/env python3
"""Is it the TABLE in front of the break paragraph that costs the heading its direct spacing?

The witness's shape in full: `</w:tbl>`, an otherwise-empty paragraph whose only content is
`<w:br w:type="page"/>`, then a `Heading3` paragraph stating `<w:spacing w:after="0"/>`.  The
earlier probes reproduced everything but the table and could not reproduce the defect.
"""
import sys, zipfile, importlib.util

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'

TABLE = ('<w:tbl><w:tblPr><w:tblW w:w="5000" w:type="pct"/></w:tblPr>'
         '<w:tblGrid><w:gridCol w:w="4680"/></w:tblGrid>'
         '<w:tr><w:tc><w:tcPr><w:tcW w:w="5000" w:type="pct"/></w:tcPr>'
         '<w:p><w:r><w:t>cell</w:t></w:r></w:p></w:tc></w:tr></w:tbl>')
BREAK_PARA = ('<w:p><w:pPr><w:rPr><w:sz w:val="20"/></w:rPr></w:pPr>'
              '<w:r><w:br w:type="page"/></w:r></w:p>')
EMPTY_PARA = '<w:p><w:pPr><w:rPr><w:sz w:val="20"/></w:rPr></w:pPr></w:p>'

# (label, what precedes the heading)
ARMS = [
    ('T-table-break',        TABLE + BREAK_PARA),
    ('U-table-empty',        TABLE + EMPTY_PARA),
    ('V-table-only',         TABLE),
    ('W-break-only',         BREAK_PARA),
    ('X-para-break',         '<w:p><w:r><w:t>plain</w:t></w:r></w:p>' + BREAK_PARA),
    ('Y-table-break-break',  TABLE + BREAK_PARA + BREAK_PARA),
]

paras = []
for label, preceding in ARMS:
    paras.append('<w:p><w:r><w:t>lead in</w:t></w:r></w:p>')
    paras.append(preceding)
    paras.append('<w:p><w:pPr><w:pStyle w:val="H3"/><w:spacing w:after="0"/></w:pPr>'
                 f'<w:r><w:t>{label}</w:t></w:r></w:p>')
    paras.append('<w:p><w:r><w:t>follower</w:t></w:r></w:p>')

DOCUMENT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<w:document xmlns:w="{W}"><w:body>' + ''.join(paras)
            + '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
              '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/></w:sectPr>'
            '</w:body></w:document>')

spec = importlib.util.spec_from_file_location(
    'hs', '/home/user/libreoffice-core/dotnet/probes/headspace-r160/make-probe.py')
hs = importlib.util.module_from_spec(spec)
spec.loader.exec_module(hs)
PARTS = dict(hs.PARTS)
PARTS['word/document.xml'] = DOCUMENT


def main(path):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, text in PARTS.items():
            z.writestr(name, text)
    print(path)


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else '/tmp/claude-0/o110/headtable.docx')
