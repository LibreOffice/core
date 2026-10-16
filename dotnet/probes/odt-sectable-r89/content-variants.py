#!/usr/bin/env python3
"""What a columned `text:section` does when its content is not only paragraphs.

The previous round measured that `absrc-pac-01-info-note-en`'s two-column section is drawn by
26.2.4.2 in ONE column, that no attribute of its style is the switch, and that replacing the
section's table *and* its table-of-content with paragraphs restores the two columns.  This puts
each candidate into the fixture 26.2.4.2 itself produced (`features/odt-section-columns.odt`),
one at a time, and reads the x histogram of the result.
"""
import re, subprocess, sys, zipfile, pathlib

SOFFICE = '/opt/libreoffice26.2/program/soffice'
PROFILE = 'file:///tmp/xray-office-lo-r89variants'

TABLE = ('<table:table table:name="TBL%(n)s" table:style-name="TblStyle">'
         '<table:table-column table:style-name="TblCol" table:number-columns-repeated="2"/>'
         '<table:table-row>'
         '<table:table-cell office:value-type="string"><text:p text:style-name="Standard">CELLA%(n)s</text:p></table:table-cell>'
         '<table:table-cell office:value-type="string"><text:p text:style-name="Standard">CELLB%(n)s</text:p></table:table-cell>'
         '</table:table-row>'
         '<table:table-row>'
         '<table:table-cell office:value-type="string"><text:p text:style-name="Standard">CELLC%(n)s</text:p></table:table-cell>'
         '<table:table-cell office:value-type="string"><text:p text:style-name="Standard">CELLD%(n)s</text:p></table:table-cell>'
         '</table:table-row>'
         '</table:table>')

TOC = ('<text:table-of-content text:name="TOCX">'
       '<text:table-of-content-source text:outline-level="10"/>'
       '<text:index-body>'
       '<text:p text:style-name="Standard">TOCONE first entry of the index body</text:p>'
       '<text:p text:style-name="Standard">TOCTWO second entry of the index body</text:p>'
       '</text:index-body></text:table-of-content>')

NESTED = ('<text:section text:style-name="SectPlain" text:name="Nested">'
          '<text:p text:style-name="Standard">NESTONE inside a nested plain section</text:p>'
          '<text:p text:style-name="Standard">NESTTWO inside a nested plain section</text:p>'
          '</text:section>')

FRAME = ('<text:p text:style-name="Standard"><draw:frame draw:style-name="FrStyle" '
         'text:anchor-type="as-char" svg:width="1in" svg:height="0.4in">'
         '<draw:text-box><text:p text:style-name="Standard">FRAMEONE</text:p></draw:text-box>'
         '</draw:frame></text:p>')

STYLES = ('<style:style style:name="TblStyle" style:family="table">'
          '<style:table-properties style:width="2in" table:align="left"/></style:style>'
          '<style:style style:name="TblCol" style:family="table-column">'
          '<style:table-column-properties style:column-width="1in"/></style:style>'
          '<style:style style:name="FrStyle" style:family="graphic" style:parent-style-name="Frame">'
          '<style:graphic-properties fo:padding="0in" fo:border="none"/></style:style>'
          '<style:style style:name="SectPlain" style:family="section">'
          '<style:section-properties style:editable="false"><style:columns fo:column-count="1" '
          'fo:column-gap="0in"><style:column style:rel-width="65535*"/></style:columns>'
          '</style:section-properties></style:style>')


def rewrite(src, dst, fn):
    with zipfile.ZipFile(src) as z:
        items = z.infolist()
        data = {i.filename: z.read(i.filename) for i in items}
    xml = fn(data['content.xml'].decode('utf-8'))
    xml = xml.replace('</office:automatic-styles>', STYLES + '</office:automatic-styles>')
    data['content.xml'] = xml.encode('utf-8')
    with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as out:
        for i in items:
            out.writestr(i, data[i.filename])


SECT = re.compile(r'(<text:section text:style-name="Sect2" text:name="Section1">)(.*?)(</text:section>)', re.S)


def at_start(blob):
    return lambda x: SECT.sub(lambda m: m.group(1) + blob + m.group(2) + m.group(3), x, count=1)


def after_para(blob, para='PARA07'):
    def apply(xml):
        def one(m):
            body = m.group(2)
            i = body.find('<text:p', body.find(para))
            return m.group(1) + body[:i] + blob + body[i:] + m.group(3)
        return SECT.sub(one, xml, count=1)
    return apply


VARIANTS = {
    'base': lambda x: x,
    'table-start': at_start(TABLE % {'n': '1'}),
    'table-mid': after_para(TABLE % {'n': '1'}),
    'toc-mid': after_para(TOC),
    'toc-start': at_start(TOC),
    'nested-mid': after_para(NESTED),
    'frame-mid': after_para(FRAME),
    'table-and-toc': lambda x: after_para(TOC)(at_start(TABLE % {'n': '1'})(x)),
}

if __name__ == '__main__':
    src = pathlib.Path(sys.argv[1])
    out = pathlib.Path(sys.argv[2]); out.mkdir(parents=True, exist_ok=True)
    only = sys.argv[3:] or list(VARIANTS)
    for name in only:
        f = out / f'{name}.odt'
        rewrite(src, f, VARIANTS[name])
        subprocess.run(['timeout', '-k', '30', '240', SOFFICE, '--headless', '--norestore',
                        f'-env:UserInstallation={PROFILE}', '--convert-to', 'pdf',
                        '--outdir', str(out), str(f)], capture_output=True)
        print(name, (out / f'{name}.pdf').exists())
