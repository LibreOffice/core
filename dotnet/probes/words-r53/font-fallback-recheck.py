#!/usr/bin/env python3
"""Re-check the three `SystemFontResolver` claims calibrated to LibreOffice 24.2.7.2.

    python3 font-fallback-recheck.py <outdir>

`dotnet/TODO.24-2-7-audit.md` lists 48 sites carrying claims measured against the superseded
24.2.7.2 binary. Three of them are in `XRay.Office.Text/Fonts/SystemFontResolver.cs`, which sits
upstream of the font resolution deciding 267 of the reference renderings, and all three are the
same experiment: hand the reference a family name and read back what it drew with.

  site :406  a chain naming nothing installed lands on **DejaVu**, never Liberation
  site :435  a document declaring **no** family renders in **Liberation Serif**
  site :629  an **unrecognised** family resolves to **DejaVu Sans** — the site names Aptos,
             Segoe UI, Roboto, Lato, Montserrat, Myriad Pro, Futura, Optima and Univers

Each case is one authored DOCX holding one line in one declared family, converted by the
installed `soffice` and read back with `pdffonts`. The controls are families whose answer is
already known — Calibri is Carlito and Cambria is Caladea by metric substitution, Liberation Serif
is installed and must answer itself — so a run that gets those wrong is measuring something else.
"""
import os, re, subprocess, sys, zipfile

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-'
      'officedocument.wordprocessingml.document.main+xml"/></Types>')
RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
        'relationships/officeDocument" Target="word/document.xml"/></Relationships>')

# name -> (declared family or None, what the site under test predicts)
CASES = {
    'control-liberation-serif': ('Liberation Serif', 'Liberation Serif'),
    'control-calibri':          ('Calibri', 'Carlito'),
    'control-cambria':          ('Cambria', 'Caladea'),
    'control-arial':            ('Arial', 'Liberation Sans'),
    'no-family':                (None, 'Liberation Serif'),
    'unknown-aptos':            ('Aptos', 'DejaVu Sans'),
    'unknown-segoe-ui':         ('Segoe UI', 'DejaVu Sans'),
    'unknown-roboto':           ('Roboto', 'DejaVu Sans'),
    'unknown-lato':             ('Lato', 'DejaVu Sans'),
    'unknown-montserrat':       ('Montserrat', 'DejaVu Sans'),
    'unknown-myriad-pro':       ('Myriad Pro', 'DejaVu Sans'),
    'unknown-futura':           ('Futura', 'DejaVu Sans'),
    'unknown-optima':           ('Optima', 'DejaVu Sans'),
    'unknown-univers':          ('Univers', 'DejaVu Sans'),
    # A name that carries a serif hint and is installed nowhere: the :629 site says the shape of
    # the *name* decides only where the table has heard of the family, and that this path answers
    # sans regardless.
    'unknown-serif-hint':       ('Nonesuch Serif MT', 'DejaVu Sans'),
    'unknown-nonsense':         ('Zzqqxx Nonesuch', 'DejaVu Sans'),
    # The blank-family case stated a second way: an empty w:ascii rather than no w:rFonts at all.
    'empty-family':             ('', 'Liberation Serif'),
}


def build(out):
    for name, (family, _) in CASES.items():
        if family is None:
            props = ''
        else:
            props = f'<w:rPr><w:rFonts w:ascii="{family}" w:hAnsi="{family}"/></w:rPr>'
        body = (f'<w:p><w:pPr>{props}</w:pPr><w:r>{props}'
                '<w:t>Handgloves quick brown fox 12345</w:t></w:r></w:p>')
        doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
               f'<w:document xmlns:w="{W}"><w:body>{body}'
               '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
               '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>'
               '</w:sectPr></w:body></w:document>')
        with zipfile.ZipFile(os.path.join(out, name + '.docx'), 'w', zipfile.ZIP_DEFLATED) as z:
            z.writestr('[Content_Types].xml', CT)
            z.writestr('_rels/.rels', RELS)
            z.writestr('word/document.xml', doc)


def faces(pdf):
    if not os.path.exists(pdf):
        return []
    text = subprocess.run(['pdffonts', pdf], capture_output=True).stdout.decode('utf-8', 'replace')
    names = []
    for line in text.splitlines()[2:]:
        if not line.strip():
            continue
        base = line.split()[0]
        names.append(re.sub(r'^[A-Z]{6}\+', '', base))
    return names


def main():
    out = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else '.')
    os.makedirs(os.path.join(out, 'ref'), exist_ok=True)
    build(out)

    env = dict(os.environ, SOURCE_DATE_EPOCH='1700000000', TZ='UTC')
    version = subprocess.run(['soffice', '--version'], capture_output=True,
                             env=env).stdout.decode().strip()
    print(version)

    agree = disagree = 0
    print(f"{'case':26s} {'declared':20s} {'site says':18s} {'26.2.4.2 draws':28s} verdict")
    for name, (family, expected) in CASES.items():
        source = os.path.join(out, name + '.docx')
        subprocess.run(['soffice', '--headless',
                        '-env:UserInstallation=file://' + os.path.join(out, 'prof'),
                        '--convert-to', 'pdf', '--outdir', os.path.join(out, 'ref'), source],
                       capture_output=True, env=env)
        drawn = faces(os.path.join(out, 'ref', name + '.pdf'))
        got = ', '.join(drawn) if drawn else '(nothing embedded)'
        ok = any(expected.replace(' ', '') in one.replace('-', '').replace(' ', '')
                 for one in drawn)
        agree += ok
        disagree += not ok
        print(f'{name:26s} {str(family):20s} {expected:18s} {got:28s} '
              f'{"agrees" if ok else "DISAGREES"}')

    print(f'\n{agree} agree, {disagree} disagree, of {len(CASES)}')


if __name__ == '__main__':
    main()
