#!/usr/bin/env python3
"""Content variants of one document's columned `text:section`, rendered through 26.2.4.2.

`sect-variants.py` of the previous round varied the section's *style* and found no switch.  This
varies what the section *holds* -- which is where that round's measurement pointed -- one element
at a time, and reads the x histogram of the result.

The section of `absrc-pac-01-info-note-en.odt` is
`[table:table][8 empty text:p][text:p INFORMATION...][text:table-of-content][text:p]`.
"""
import re, subprocess, sys, zipfile, pathlib

SOFFICE = '/opt/libreoffice26.2/program/soffice'
PROFILE = 'file:///tmp/xray-office-lo-r89absrc'
OPEN = '<text:section text:style-name="Sect1" text:name="Section1">'

PARA = '<text:p text:style-name="Standard">FILLER%d lorem ipsum dolor sit amet consectetuer adipiscing elit sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna</text:p>'


def section(xml):
    i = xml.find(OPEN)
    j = xml.find('</text:section>', i)
    return i + len(OPEN), j


def edit(fn):
    def apply(xml):
        a, b = section(xml)
        return xml[:a] + fn(xml[a:b]) + xml[b:]
    return apply


def drop(tag):
    def fn(body):
        return re.sub(r'<%s\b.*?</%s>' % (tag, tag), '', body, flags=re.S)
    return fn


def replace(tag, blob):
    def fn(body):
        return re.sub(r'<%s\b.*?</%s>' % (tag, tag), blob, body, flags=re.S)
    return fn


def keep_only(tag):
    def fn(body):
        m = re.search(r'<%s\b.*?</%s>' % (tag, tag), body, flags=re.S)
        return m.group(0) if m else ''
    return fn


def strip_images(body):
    return re.sub(r'<draw:frame\b.*?</draw:frame>', '', body, flags=re.S)


FILL = ''.join(PARA % i for i in range(1, 40))

VARIANTS = {
    'base': lambda x: x,
    'no-table': edit(drop('table:table')),
    'no-toc': edit(drop('text:table-of-content')),
    'no-either': edit(lambda b: drop('text:table-of-content')(drop('table:table')(b))),
    'table-only': edit(keep_only('table:table')),
    'toc-only': edit(keep_only('text:table-of-content')),
    'table-plus-fill': edit(lambda b: keep_only('table:table')(b) + FILL),
    'toc-plus-fill': edit(lambda b: keep_only('text:table-of-content')(b) + FILL),
    'fill-only': edit(lambda b: FILL),
    'table-noimages': edit(lambda b: strip_images(b)),
    'table-to-para': edit(replace('table:table', PARA % 99)),
    'toc-to-para': edit(replace('text:table-of-content', PARA % 98)),
}

if __name__ == '__main__':
    src = pathlib.Path(sys.argv[1])
    out = pathlib.Path(sys.argv[2]); out.mkdir(parents=True, exist_ok=True)
    for name in (sys.argv[3:] or list(VARIANTS)):
        with zipfile.ZipFile(src) as z:
            items = z.infolist()
            data = {i.filename: z.read(i.filename) for i in items}
        data['content.xml'] = VARIANTS[name](data['content.xml'].decode('utf-8')).encode('utf-8')
        f = out / f'{name}.odt'
        with zipfile.ZipFile(f, 'w', zipfile.ZIP_DEFLATED) as o:
            for i in items:
                o.writestr(i, data[i.filename])
        subprocess.run(['timeout', '-k', '30', '240', SOFFICE, '--headless', '--norestore',
                        f'-env:UserInstallation={PROFILE}', '--convert-to', 'pdf',
                        '--outdir', str(out), str(f)], capture_output=True)
        print(name, (out / f'{name}.pdf').exists(), flush=True)
