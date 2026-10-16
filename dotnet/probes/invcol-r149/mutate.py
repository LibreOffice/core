#!/usr/bin/env python3
"""One-attribute variants of the invoice, to measure 26.2.4.2 against the attribute itself.

Rewrites only `style:table-centering` on page-layout Mpm4 (the Invoice sheet's), leaving
every other byte of the .ods alone, and writes one file per variant into variants/.
"""
import re, shutil, zipfile, pathlib, sys

SRC = '/home/user/corpus-odf/ods/b7fde7cdac59-084_Service_invoice_Use_this_template_c92b43dc.ods'
OUT = pathlib.Path('variants'); OUT.mkdir(exist_ok=True)

VARIANTS = {
    'asfound':    'horizontal',   # byte-identical rewrite, the control on the rewriter
    'none':       None,           # attribute removed entirely
    'vertical':   'vertical',
    'both':       'both',
}

src = zipfile.ZipFile(SRC)
styles = src.read('styles.xml').decode()
lay = re.search(r'<style:page-layout style:name="Mpm4">.*?/>', styles, re.S).group(0)
assert 'style:table-centering="horizontal"' in lay

for name, val in VARIANTS.items():
    new = (lay.replace(' style:table-centering="horizontal"', '') if val is None
           else lay.replace('style:table-centering="horizontal"',
                            f'style:table-centering="{val}"'))
    dst = OUT / f'invoice-{name}.ods'
    with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as z:
        for info in src.infolist():
            if info.filename.endswith('/'):
                continue          # directory entries; this .ods overlaps two of them
            data = (styles.replace(lay, new).encode() if info.filename == 'styles.xml'
                    else src.read(info.filename))
            z.writestr(info.filename, data,
                       compress_type=zipfile.ZIP_STORED if info.filename == 'mimetype'
                       else zipfile.ZIP_DEFLATED)
    print(dst, '->', val)
