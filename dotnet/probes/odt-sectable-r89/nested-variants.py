#!/usr/bin/env python3
"""Where 26.2.4.2 lays out a section nested inside a columned one, and how wide.

`content-variants.py` showed that an index or a nested `text:section` inside a two-column
section is drawn between the two halves of that section rather than inside a column.  These
variants measure the *width* it is laid out against -- a long line's wrap is the only thing
that says it -- and whether the outer section's own margins reach it, plus what
`text:dont-balance-text-columns` does to the flow that follows the section.
"""
import re, subprocess, sys, zipfile, pathlib

SOFFICE = '/opt/libreoffice26.2/program/soffice'
PROFILE = 'file:///tmp/xray-office-lo-r89nested'
LONG = ('LONGLINE lorem ipsum dolor sit amet consectetuer adipiscing elit sed diam nonummy nibh '
        'euismod tincidunt ut laoreet dolore magna aliquam erat volutpat ut wisi enim ad minim '
        'veniam quis nostrud exerci tation ullamcorper suscipit lobortis nisl')

TOC = ('<text:table-of-content text:name="TOCX">'
       '<text:table-of-content-source text:outline-level="10"/>'
       '<text:index-body>'
       '<text:p text:style-name="Standard">TOC%s</text:p>'
       '</text:index-body></text:table-of-content>') % LONG

NESTED = ('<text:section text:style-name="SectPlain" text:name="Nested">'
          '<text:p text:style-name="Standard">NEST%s</text:p></text:section>') % LONG

PLAIN = '<text:p text:style-name="Standard">PLAIN%s</text:p>' % LONG

STYLES = ('<style:style style:name="SectPlain" style:family="section">'
          '<style:section-properties style:editable="false"><style:columns fo:column-count="1" '
          'fo:column-gap="0in"/></style:section-properties></style:style>')

SECT = re.compile(r'(<text:section text:style-name="Sect2" text:name="Section1">)(.*?)(</text:section>)', re.S)
SECT2STYLE = re.compile(r'(<style:style style:name="Sect2" style:family="section"><style:section-properties)([^>]*)(>)')


def after_para(blob, para='PARA07'):
    def apply(xml):
        def one(m):
            body = m.group(2)
            i = body.find('<text:p', body.find(para))
            return m.group(1) + body[:i] + blob + body[i:] + m.group(3)
        return SECT.sub(one, xml, count=1)
    return apply


def on_section_props(extra):
    return lambda xml: SECT2STYLE.sub(lambda m: m.group(1) + extra + m.group(2) + m.group(3), xml, count=1)


def chain(*fns):
    def apply(xml):
        for f in fns:
            xml = f(xml)
        return xml
    return apply


VARIANTS = {
    'long-plain': after_para(PLAIN),
    'long-toc': after_para(TOC),
    'long-nested': after_para(NESTED),
    'long-toc-margins': chain(after_para(TOC), on_section_props(' fo:margin-left="0.5in" fo:margin-right="0.75in"')),
    'long-plain-margins': chain(after_para(PLAIN), on_section_props(' fo:margin-left="0.5in" fo:margin-right="0.75in"')),
    'dontbalance': on_section_props(' text:dont-balance-text-columns="true"'),
    'dontbalance-toc': chain(after_para(TOC), on_section_props(' text:dont-balance-text-columns="true"')),
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
        subprocess.run(['timeout', '-k', '30', '240', SOFFICE, '--headless', '--norestore',
                        f'-env:UserInstallation={PROFILE}', '--convert-to', 'pdf',
                        '--outdir', str(out), str(f)], capture_output=True)
        print(name, (out / f'{name}.pdf').exists(), flush=True)
