#!/usr/bin/env python3
"""Probes for the gap between a `Heading3` paragraph and the paragraph after it.

The FAA holdover documents put a two-line `Heading3` title above every table and follow
it with a 9 pt centred note. 26.2.4.2 leaves 6 pt more between the two than this tree
does, and that 6 pt is what displaces a section by one page. Each probe varies one thing
so the 6 pt can be attributed:

  A  as the file states it -- style `after=120`, direct `after=0`, a `w:br` in the title
     and one 12 pt space among 13 pt runs;
  B  no direct `after` at all, so the style's 120 applies (the control that says 6 pt is
     what the style would have given);
  C  one line -- the `w:br` removed;
  D  no 12 pt run, so the second line is uniformly 13 pt;
  E  direct `after="0"` written as `after="0" line="240" lineRule="auto"`;
  F  the following paragraph's `widowControl` removed;
  G  the title's paragraph-mark `rPr` removed.

Usage: mk-heading-gap.py <outdir>
"""
import os
import sys
import zipfile

RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>
'''

DOC_RELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>
'''

TYPES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
</Types>
'''

STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="Times New Roman" w:eastAsia="Times New Roman" w:hAnsi="Times New Roman" w:cs="Times New Roman"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/><w:pPr><w:jc w:val="both"/></w:pPr><w:rPr><w:rFonts w:ascii="Arial" w:eastAsia="Calibri" w:hAnsi="Arial"/><w:szCs w:val="22"/></w:rPr></w:style>
<w:style w:type="paragraph" w:styleId="Heading3"><w:name w:val="heading 3"/><w:basedOn w:val="Normal"/><w:next w:val="Normal"/><w:qFormat/>
<w:pPr><w:keepNext/><w:spacing w:after="120"/><w:jc w:val="center"/><w:outlineLvl w:val="2"/></w:pPr>
<w:rPr><w:rFonts w:ascii="Arial Bold" w:hAnsi="Arial Bold"/><w:caps/><w:sz w:val="26"/></w:rPr></w:style>
</w:styles>
'''

DOCUMENT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:body>
<w:p><w:pPr><w:pStyle w:val="Heading3"/>{spacing}<w:rPr>{markrpr}</w:rPr></w:pPr>
<w:r><w:t>Table 49:</w:t></w:r>{brk}<w:r><w:t xml:space="preserve">Type I</w:t></w:r>{small}<w:r><w:t>Fluids Tested for Anti-Icing Performance and Aerodynamic Acceptance</w:t></w:r></w:p>
<w:p><w:pPr>{widow}<w:spacing w:after="120"/><w:jc w:val="center"/><w:rPr><w:rFonts w:cs="Arial"/><w:b/><w:bCs/><w:sz w:val="18"/></w:rPr></w:pPr>
<w:r><w:rPr><w:rFonts w:cs="Arial"/><w:b/><w:bCs/><w:sz w:val="18"/></w:rPr><w:t>(see cautions and notes on pages 77 and 78)</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="15840" w:h="12240" w:orient="landscape"/>
<w:pgMar w:top="1151" w:right="1151" w:bottom="1151" w:left="1151" w:header="431" w:footer="431" w:gutter="0"/>
</w:sectPr>
</w:body></w:document>
'''

BR = '<w:r><w:br/></w:r>'
SMALL = '<w:r><w:rPr><w:sz w:val="24"/></w:rPr><w:t xml:space="preserve"> </w:t></w:r>'
MARK = '<w:rFonts w:eastAsia="Times New Roman"/><w:szCs w:val="20"/>'
WIDOW = '<w:widowControl w:val="0"/>'

CASES = {
    'gapA-as-authored': dict(spacing='<w:spacing w:after="0"/>', brk=BR, small=SMALL,
                             markrpr=MARK, widow=WIDOW),
    'gapB-style-after': dict(spacing='', brk=BR, small=SMALL, markrpr=MARK, widow=WIDOW),
    'gapC-one-line': dict(spacing='<w:spacing w:after="0"/>', brk='', small=SMALL,
                          markrpr=MARK, widow=WIDOW),
    'gapD-no-small-run': dict(spacing='<w:spacing w:after="0"/>', brk=BR, small='',
                              markrpr=MARK, widow=WIDOW),
    'gapE-after-with-line': dict(spacing='<w:spacing w:after="0" w:line="240" w:lineRule="auto"/>',
                                 brk=BR, small=SMALL, markrpr=MARK, widow=WIDOW),
    'gapF-no-widow': dict(spacing='<w:spacing w:after="0"/>', brk=BR, small=SMALL,
                          markrpr=MARK, widow=''),
    'gapG-no-mark-rpr': dict(spacing='<w:spacing w:after="0"/>', brk=BR, small=SMALL,
                             markrpr='', widow=WIDOW),
}


def main():
    outdir = sys.argv[1]
    os.makedirs(outdir, exist_ok=True)
    for name, fields in CASES.items():
        path = os.path.join(outdir, name + '.docx')
        with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
            z.writestr('[Content_Types].xml', TYPES)
            z.writestr('_rels/.rels', RELS)
            z.writestr('word/_rels/document.xml.rels', DOC_RELS)
            z.writestr('word/styles.xml', STYLES)
            z.writestr('word/document.xml', DOCUMENT.format(**fields))
        print(path)


main()
