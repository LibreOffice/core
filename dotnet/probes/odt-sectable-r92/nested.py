#!/usr/bin/env python3
"""What a columned `text:section` does to content that is not a paragraph, measured at 26.2.4.2.

Built on `features/odt-section-columns.odt`, which is 26.2.4.2's own conversion of a DOCX
(`probes/odt-startx-r88/gen-section-columns.py`): PARA01-08 in a one-column section, PARA09-14
and TWOEND in a two-column one whose columns are 216 pt wide at x 72 and 324, THREEAFTER below
it.  Each variant puts one element inside the two-column section, after PARA11.

The line to read is the LONG one: laid out in a column it wraps at 216 pt, laid out against the
section's own measure it wraps at 468.  `odt-sectable-r89/content-variants.py` is the ancestor of
this file; its `after_para` default names a paragraph that is not in the section it edits, so it
inserted at the section's end instead.
"""
import re, subprocess, sys, zipfile, pathlib

SOFFICE = '/opt/libreoffice26.2/program/soffice'
PROFILE = 'file:///tmp/xray-office-lo-r92nested'

LONG = ('%s lorem ipsum dolor sit amet consectetuer adipiscing elit sed diam nonummy nibh '
        'euismod tincidunt ut laoreet dolore magna aliquam erat volutpat ut wisi enim ad minim '
        'veniam quis nostrud exerci tation ullamcorper')

TOC = ('<text:table-of-content text:style-name="SectPlain" text:name="TOCX">'
       '<text:table-of-content-source text:outline-level="10"/>'
       '<text:index-body>'
       '<text:p text:style-name="Standard">' + (LONG % 'TOCBODY') + '</text:p>'
       '</text:index-body></text:table-of-content>')

NESTED = ('<text:section text:style-name="SectPlain" text:name="Nested">'
          '<text:p text:style-name="Standard">' + (LONG % 'NESTBODY') + '</text:p>'
          '</text:section>')

NESTED_TWO = ('<text:section text:style-name="SectTwo" text:name="NestedTwo">'
              '<text:p text:style-name="Standard">' + (LONG % 'NESTTWO') + '</text:p>'
              '</text:section>')

TABLE = ('<table:table table:name="TBL1" table:style-name="TblStyle">'
         '<table:table-column table:style-name="TblCol"/>'
         '<table:table-row><table:table-cell office:value-type="string">'
         '<text:p text:style-name="Standard">' + (LONG % 'CELLONE') + '</text:p>'
         '</table:table-cell></table:table-row></table:table>')

WIDETABLE = ('<table:table table:name="TBL2" table:style-name="TblWide">'
             '<table:table-column table:style-name="TblColWide"/>'
             '<table:table-row><table:table-cell office:value-type="string">'
             '<text:p text:style-name="Standard">' + (LONG % 'WIDECELL') + '</text:p>'
             '</table:table-cell></table:table-row></table:table>')


NESTED_NOSTYLE = ('<text:section text:name="NestedBare">'
                  '<text:p text:style-name="Standard">' + (LONG % 'NESTBARE') + '</text:p>'
                  '</text:section>')

NESTED_NOCOLS = ('<text:section text:style-name="SectNoCols" text:name="NestedNoCols">'
                 '<text:p text:style-name="Standard">' + (LONG % 'NESTNOCOL') + '</text:p>'
                 '</text:section>')

NESTED_OWNMARGIN = ('<text:section text:style-name="SectMargin" text:name="NestedMargin">'
                    '<text:p text:style-name="Standard">' + (LONG % 'NESTMARGIN') + '</text:p>'
                    '</text:section>')

NESTED_LEFTONLY = ('<text:section text:style-name="SectLeftOnly" text:name="NestedLeftOnly">'
                   '<text:p text:style-name="Standard">' + (LONG % 'NESTLEFT') + '</text:p>'
                   '</text:section>')

PLAIN = '<text:p text:style-name="Standard">' + (LONG % 'PLAINPARA') + '</text:p>'

STYLES = ('<style:style style:name="TblStyle" style:family="table">'
          '<style:table-properties style:width="2in" table:align="left"/></style:style>'
          '<style:style style:name="TblCol" style:family="table-column">'
          '<style:table-column-properties style:column-width="2in"/></style:style>'
          '<style:style style:name="TblWide" style:family="table">'
          '<style:table-properties style:width="6in" table:align="left"/></style:style>'
          '<style:style style:name="TblColWide" style:family="table-column">'
          '<style:table-column-properties style:column-width="6in"/></style:style>'
          '<style:style style:name="SectPlain" style:family="section">'
          '<style:section-properties style:editable="false"><style:columns fo:column-count="1" '
          'fo:column-gap="0in"/></style:section-properties></style:style>'
          '<style:style style:name="SectTwo" style:family="section">'
          '<style:section-properties style:editable="false"><style:columns fo:column-count="2" '
          'fo:column-gap="0.25in"/></style:section-properties></style:style>'
          '<style:style style:name="SectNoCols" style:family="section">'
          '<style:section-properties style:editable="false"/></style:style>'
          '<style:style style:name="SectMargin" style:family="section">'
          '<style:section-properties style:editable="false" fo:margin-left="1.5in" '
          'fo:margin-right="0.25in"><style:columns fo:column-count="1" fo:column-gap="0in"/>'
          '</style:section-properties></style:style>'
          '<style:style style:name="SectLeftOnly" style:family="section">'
          '<style:section-properties style:editable="false" fo:margin-left="1.5in">'
          '<style:columns fo:column-count="1" fo:column-gap="0in"/>'
          '</style:section-properties></style:style>')

SECT = re.compile(r'(<text:section text:style-name="Sect2" text:name="Section1">)(.*?)(</text:section>)', re.S)
SECT2PROPS = re.compile(r'(<style:style style:name="Sect2" style:family="section"><style:section-properties)([^>]*)(>)')


def after_para(blob, para='PARA11'):
    def apply(xml):
        def one(m):
            body = m.group(2)
            at = body.find(para)
            assert at >= 0, f'{para} is not inside the section being edited'
            i = body.find('<text:p', at)
            assert i > at
            return m.group(1) + body[:i] + blob + body[i:] + m.group(3)
        return SECT.sub(one, xml, count=1)
    return apply


def on_section_props(extra):
    return lambda xml: SECT2PROPS.sub(lambda m: m.group(1) + extra + m.group(2) + m.group(3), xml, count=1)


def chain(*fns):
    def apply(xml):
        for f in fns:
            xml = f(xml)
        return xml
    return apply


VARIANTS = {
    'base': lambda x: x,
    'plain': after_para(PLAIN),
    'toc': after_para(TOC),
    'nested': after_para(NESTED),
    'nested-two': after_para(NESTED_TWO),
    'table': after_para(TABLE),
    'widetable': after_para(WIDETABLE),
    'nested-bare': after_para(NESTED_NOSTYLE),
    'nested-nocols': after_para(NESTED_NOCOLS),
    'nested-ownmargin': after_para(NESTED_OWNMARGIN),
    'nested-ownmargin-outer': chain(after_para(NESTED_OWNMARGIN),
                                    on_section_props(' fo:margin-left="0.5in" fo:margin-right="0.75in"')),
    'nested-leftonly': chain(after_para(NESTED_LEFTONLY),
                             on_section_props(' fo:margin-left="0.5in" fo:margin-right="0.75in"')),
    'nested-margins': chain(after_para(NESTED),
                            on_section_props(' fo:margin-left="0.5in" fo:margin-right="0.75in"')),
    'toc-margins': chain(after_para(TOC),
                         on_section_props(' fo:margin-left="0.5in" fo:margin-right="0.75in"')),
    'plain-margins': chain(after_para(PLAIN),
                           on_section_props(' fo:margin-left="0.5in" fo:margin-right="0.75in"')),
    'table-margins': chain(after_para(TABLE),
                           on_section_props(' fo:margin-left="0.5in" fo:margin-right="0.75in"')),
}

if __name__ == '__main__':
    src = pathlib.Path(sys.argv[1])
    out = pathlib.Path(sys.argv[2]); out.mkdir(parents=True, exist_ok=True)
    for name in (sys.argv[3:] or list(VARIANTS)):
        with zipfile.ZipFile(src) as z:
            items = z.infolist()
            data = {i.filename: z.read(i.filename) for i in items}
        xml = VARIANTS[name](data['content.xml'].decode('utf-8'))
        xml = xml.replace('</office:automatic-styles>', STYLES + '</office:automatic-styles>')
        data['content.xml'] = xml.encode('utf-8')
        f = out / f'{name}.odt'
        with zipfile.ZipFile(f, 'w', zipfile.ZIP_DEFLATED) as o:
            for i in items:
                o.writestr(i, data[i.filename])
        subprocess.run(['timeout', '-k', '30', '600', SOFFICE, '--headless', '--norestore',
                        f'-env:UserInstallation={PROFILE}', '--convert-to', 'pdf',
                        '--outdir', str(out), str(f)], capture_output=True)
        print(name, (out / f'{name}.pdf').exists())
