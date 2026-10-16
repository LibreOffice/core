#!/usr/bin/env python3
"""Does a deferred page break cost the paragraph it lands on its own direct spacing?

The witness's shape: an otherwise-empty paragraph whose only content is `<w:br w:type="page"/>`,
followed by a `Heading3` paragraph stating `<w:spacing w:after="0"/>`.  26.2.4.2's own
`--convert-to fodt` gives that heading `fo:break-before="page"` and *no* `fo:margin-bottom`, so it
keeps the style's 6 pt -- while the same heading with no break before it keeps its own 0.
"""
import sys, zipfile

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'

BREAK_PARA = ('<w:p><w:pPr><w:rPr><w:sz w:val="20"/></w:rPr></w:pPr>'
              '<w:r><w:br w:type="page"/></w:r></w:p>')
TEXT_BREAK_PARA = ('<w:p><w:r><w:t>tail text</w:t></w:r>'
                   '<w:r><w:br w:type="page"/></w:r></w:p>')

# (label, what precedes the heading, the heading's own spacing)
ARMS = [
    ('L-break-then-after0',      BREAK_PARA,      '<w:spacing w:after="0"/>'),
    ('M-break-then-after40',     BREAK_PARA,      '<w:spacing w:after="40"/>'),
    ('N-nobreak-then-after0',    '',              '<w:spacing w:after="0"/>'),
    ('O-textbreak-then-after0',  TEXT_BREAK_PARA, '<w:spacing w:after="0"/>'),
    ('P-break-then-before0',     BREAK_PARA,      '<w:spacing w:before="0"/>'),
    ('Q-break-then-both',        BREAK_PARA,      '<w:spacing w:before="0" w:after="0"/>'),
    ('R-break-then-nothing',     BREAK_PARA,      ''),
    ('S-pagebreakbefore-after0', '',              '<w:pageBreakBefore/><w:spacing w:after="0"/>'),
]

paras = []
for label, preceding, spacing in ARMS:
    paras.append('<w:p><w:r><w:t>lead in</w:t></w:r></w:p>')
    paras.append(preceding)
    paras.append(f'<w:p><w:pPr><w:pStyle w:val="H3"/>{spacing}</w:pPr>'
                 f'<w:r><w:t>{label}</w:t></w:r></w:p>')
    paras.append('<w:p><w:r><w:t>follower</w:t></w:r></w:p>')

DOCUMENT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<w:document xmlns:w="{W}"><w:body>' + ''.join(paras)
            + '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
              '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/></w:sectPr>'
            '</w:body></w:document>')

import importlib.util
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
    main(sys.argv[1] if len(sys.argv) > 1 else '/tmp/claude-0/o110/headbreak.docx')
