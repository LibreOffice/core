#!/usr/bin/env python3
"""Build tests/corpus/features/printer-metrics.docx and printer-metrics-off.docx.

One paragraph of several lines in 12 pt Arial, which is where the 300 dpi rounding shows
largest: LibreOffice 24.2.7.2 sets it at a 13.80 pt pitch printer-independently and at
13.95 pt with `w:usePrinterMetrics`.  The two packages differ in that element and nothing
else, so the pair is its own control.

10 pt Times New Roman is the face and size *not* to use here, and the reason is worth
recording: our printer-independent pitch for it is 11.50 against LibreOffice's 11.55, a
separate and older defect, so both packages come out at 11.55 and the fixture appears to
prove the flag does nothing.  Six other face/size pairs separate cleanly — see
research/probes/words-r13/probe-grid.py.
"""
import os
import sys
import zipfile

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
  <Default Extension="xml" ContentType="application/xml"/>
  <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
  <Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
  <Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>"""

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Target="word/document.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
</Relationships>"""

DOC_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Target="styles.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"/>
  <Relationship Id="rId2" Target="settings.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"/>
</Relationships>"""

STYLES = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {W}>
<w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Arial" w:hAnsi="Arial"/><w:sz w:val="24"/>
<w:lang w:val="en-US"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/><w:qFormat/></w:style>
</w:styles>"""

LINE = ('The quick brown fox jumps over the lazy dog while the printer rounds every metric '
        'onto its own pixel grid and the line grows by a fraction of a point. ')


def settings(printer):
    flag = '<w:usePrinterMetrics/>' if printer else ''
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings {W}>
  <w:defaultTabStop w:val="720"/>
  <w:compat>{flag}</w:compat>
</w:settings>"""


DOCUMENT = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {W}><w:body>
<w:p><w:r><w:t xml:space="preserve">{LINE * 6}</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720" w:gutter="0"/>
</w:sectPr></w:body></w:document>"""


def build(path, printer):
    parts = {
        '[Content_Types].xml': CONTENT_TYPES,
        '_rels/.rels': ROOT_RELS,
        'word/_rels/document.xml.rels': DOC_RELS,
        'word/document.xml': DOCUMENT,
        'word/styles.xml': STYLES,
        'word/settings.xml': settings(printer),
    }
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, data in parts.items():
            z.writestr(name, data)
    print('wrote', os.path.abspath(path))


here = os.path.dirname(__file__)
features = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
    here, '..', '..', '..', 'tests', 'corpus', 'features')
build(os.path.join(features, 'printer-metrics.docx'), printer=True)
build(os.path.join(features, 'printer-metrics-off.docx'), printer=False)
