#!/usr/bin/env python3
"""One-attribute variants of an ODF document's text:section style, rendered through 26.2.4.2.

The question a variant answers is what the reference itself does when the attribute is absent:
if removing style:columns makes 26.2.4.2 draw the layout this tree draws, the section's columns
are the whole of the divergence and nothing else needs looking for.
"""
import re, shutil, subprocess, sys, zipfile, pathlib

SOFFICE = '/opt/libreoffice26.2/program/soffice'

def rewrite(src, dst, fn):
    with zipfile.ZipFile(src) as z:
        items = z.infolist()
        data = {i.filename: z.read(i.filename) for i in items}
    data['content.xml'] = fn(data['content.xml'].decode('utf-8')).encode('utf-8')
    with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as out:
        for i in items:
            out.writestr(i, data[i.filename])

def drop_columns(x):
    return re.sub(r'<style:columns\b.*?</style:columns>', '', x, flags=re.S)

def drop_margins(x):
    def f(m):
        s = m.group(0)
        s = re.sub(r'\sfo:margin-left="[^"]*"', '', s)
        s = re.sub(r'\sfo:margin-right="[^"]*"', '', s)
        return s
    return re.sub(r'<style:section-properties\b[^>]*>', f, x)

VARIANTS = {
    'base':      lambda x: x,
    'nocolumns': drop_columns,
    'nomargins': drop_margins,
    'neither':   lambda x: drop_margins(drop_columns(x)),
}

src = pathlib.Path(sys.argv[1])
out = pathlib.Path(sys.argv[2]); out.mkdir(parents=True, exist_ok=True)
for name, fn in VARIANTS.items():
    f = out / f'{name}.odt'
    rewrite(src, f, fn)
    subprocess.run(['timeout', '-k', '30', '240', SOFFICE, '--headless', '--norestore',
                    '-env:UserInstallation=file:///tmp/xray-office-lo-var88',
                    '--convert-to', 'pdf', '--outdir', str(out), str(f)],
                   capture_output=True)
    print(name, (out / f'{name}.pdf').exists())
