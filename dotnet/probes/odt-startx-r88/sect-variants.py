#!/usr/bin/env python3
"""One-attribute variants of a document's own section style, rendered through 26.2.4.2.

`variants.py` for a named section style rather than for every one: what 26.2.4.2 does when the
attribute is absent is the only way to tell an unimplemented feature from a feature the reference
declines to apply.
"""
import re, subprocess, sys, zipfile, pathlib

SOFFICE = '/opt/libreoffice26.2/program/soffice'

def rewrite(src, dst, fn):
    with zipfile.ZipFile(src) as z:
        items = z.infolist()
        data = {i.filename: z.read(i.filename) for i in items}
    data['content.xml'] = fn(data['content.xml'].decode('utf-8')).encode('utf-8')
    with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as out:
        for i in items:
            out.writestr(i, data[i.filename])

def on_style(name, fn):
    def apply(xml):
        pattern = re.compile(
            r'(<style:style style:name="%s" style:family="section">)(.*?)(</style:style>)' % re.escape(name),
            re.S)
        return pattern.sub(lambda m: m.group(1) + fn(m.group(2)) + m.group(3), xml, count=1)
    return apply

def one_column(body):
    body = re.sub(r'<style:columns\b.*?</style:columns>', '', body, flags=re.S)
    return re.sub(r'<style:columns\b[^>]*/>', '', body)

def even_columns(body):
    return re.sub(r'<style:column\b[^>]*/>', '', body)

def with_gap(body):
    return body.replace('<style:columns ', '<style:columns fo:column-gap="0.5in" ', 1)

src = pathlib.Path(sys.argv[1])
style = sys.argv[2]
out = pathlib.Path(sys.argv[3]); out.mkdir(parents=True, exist_ok=True)

for name, fn in (('base', lambda x: x),
                 ('onecolumn', on_style(style, one_column)),
                 ('evencolumns', on_style(style, even_columns)),
                 ('withgap', on_style(style, with_gap))):
    f = out / f'{name}.odt'
    rewrite(src, f, fn)
    subprocess.run(['timeout', '-k', '30', '240', SOFFICE, '--headless', '--norestore',
                    '-env:UserInstallation=file:///tmp/xray-office-lo-r88sect',
                    '--convert-to', 'pdf', '--outdir', str(out), str(f)], capture_output=True)
    print(name, (out / f'{name}.pdf').exists())
