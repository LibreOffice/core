#!/usr/bin/env python3
"""Three one-attribute mutations of `hdss-bulletin-issue-285-25-june-2025.docx`.

The document's section 2 is a `continuous` break naming `header2.xml` as its default header, and
26.2.4.2 draws that header on **no page at all** while we draw it on nine. Each variant changes one
thing and nothing else:

    no-titlepg      drop `<w:titlePg/>` from section 2
    nextpage        drop `<w:type w:val="continuous"/>` from section 2, so it breaks the page
    sect3-header    give section 3 a `headerReference` of its own to the same part

Usage: mutate.py <outdir>
"""
import pathlib
import sys
import zipfile

SRC = "/home/user/sample-files/words/done-013/docx/hdss-bulletin-issue-285-25-june-2025.docx"

out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.')
out.mkdir(parents=True, exist_ok=True)

archive = zipfile.ZipFile(SRC)
names = archive.namelist()
raw = {n: archive.read(n) for n in names}
doc = raw['word/document.xml'].decode('utf8')


def write(name, data):
    if data == doc:
        raise SystemExit(f"{name}: the mutation matched nothing — the file has changed")
    with zipfile.ZipFile(out / (name + '.docx'), 'w', zipfile.ZIP_DEFLATED) as zo:
        for n in names:
            zo.writestr(n, data.encode('utf8') if n == 'word/document.xml' else raw[n])


write('no-titlepg', doc.replace('<w:cols w:space="340"/><w:titlePg/>', '<w:cols w:space="340"/>'))
write('nextpage', doc.replace(
    '<w:sectPr w:rsidR="00580394" w:rsidRPr="00B519CD" w:rsidSect="00E62622">'
    '<w:headerReference w:type="default" r:id="rId15"/><w:type w:val="continuous"/>',
    '<w:sectPr w:rsidR="00580394" w:rsidRPr="00B519CD" w:rsidSect="00E62622">'
    '<w:headerReference w:type="default" r:id="rId15"/>'))
write('sect3-header', doc.replace(
    '<w:sectPr w:rsidR="00DD1E53" w:rsidSect="00E62622"><w:type w:val="continuous"/>',
    '<w:sectPr w:rsidR="00DD1E53" w:rsidSect="00E62622">'
    '<w:headerReference w:type="default" r:id="rId15"/><w:type w:val="continuous"/>'))
print('wrote', sorted(p.name for p in out.glob('*.docx')))
