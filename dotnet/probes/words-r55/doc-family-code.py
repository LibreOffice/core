#!/usr/bin/env python3
"""What the WW8 filter does with a font whose `FFN.ff` says nothing.

    python3 doc-family-code.py <outdir>

Round 54 probed the DOC arm by converting an authored DOCX to Word 97 with `soffice` and back, and
**refuted its own probe in the same round**: LibreOffice's DOCX *import* applies the roman default
before the export runs, so the `.doc` it wrote declared `ff=roman` and the probe measured
"declared roman", which was never in doubt. The honest state of that arm has been "unmeasured"
since.

A flat ODF file defeats the confound, because the ODF filter has no roman default: a
`style:font-face` with **no** `style:font-family-generic` leaves `SvxFontItem`'s family at
`FAMILY_DONTKNOW`, and `wwFont::Write` (`sw/source/filter/ww8/wrtw8sty.cxx`:821) maps that onto
`ff = 0`. So `.fodt` → `.doc` → `.pdf` produces a genuine undeclared `FFN` and reads back what
26.2.4.2 draws for it. The generic-bearing variants are the controls: they must come back roman,
swiss and modern, and a run that gets those wrong is measuring the export rather than the import.

`SwWW8ImplReader::GetFontParams` (`sw/source/filter/ww8/ww8par6.cxx`:3767) also carries a
**name-override list** that has no counterpart in the DOCX filter: seven prefixes forced to
`FAMILY_ROMAN` (`Tms Rmn`, `Timmons`, `CG Times`, `MS Serif`, `Garamond`, `Times Roman`,
`Times New Roman`) and seven to `FAMILY_SWISS` (`Helv`, `Arial`, `Univers`, `LinePrinter`,
`Lucida Sans`, `Small Fonts`, `MS Sans Serif`). Two of those are probed here as well, and they
discriminate whichever way the undeclared case comes out.
"""
import os
import re
import subprocess
import sys

TEXT = 'Handgloves quick brown fox 12345'
NS = ('xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" '
      'xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" '
      'xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" '
      'xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" '
      'xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"')


def fodt(path, family, generic):
    face = f'style:name="probe" svg:font-family="&apos;{family}&apos;"'
    if generic:
        face += f' style:font-family-generic="{generic}"'
    with open(path, 'w', encoding='utf-8') as handle:
        handle.write(
            '<?xml version="1.0" encoding="UTF-8"?>'
            f'<office:document {NS} office:version="1.3" '
            'office:mimetype="application/vnd.oasis.opendocument.text">'
            f'<office:font-face-decls><style:font-face {face}/></office:font-face-decls>'
            '<office:automatic-styles>'
            '<style:style style:name="P1" style:family="paragraph">'
            '<style:text-properties style:font-name="probe"/></style:style>'
            '</office:automatic-styles>'
            f'<office:body><office:text><text:p text:style-name="P1">{TEXT}</text:p>'
            '</office:text></office:body></office:document>')


def run(out, index, args, source, target_dir):
    profile = os.path.join(out, 'prof', f'p{index}')
    subprocess.run(['soffice', '--headless', '-env:UserInstallation=file://' + profile,
                    '--convert-to', args, '--outdir', target_dir, source],
                   capture_output=True, timeout=300,
                   env=dict(os.environ, SOURCE_DATE_EPOCH='1700000000', TZ='UTC'))


def faces(pdf):
    if not os.path.exists(pdf):
        return []
    text = subprocess.run(['pdffonts', pdf], capture_output=True).stdout.decode('utf-8', 'replace')
    return [re.sub(r'^[A-Z]{6}\+', '', line.split()[0])
            for line in text.splitlines()[2:] if line.strip()]


def ffn_bytes(doc):
    """Every `FFN` byte that could be a family code, so the fixture is checked and not assumed.

    Not a parse of the table stream — just the raw bytes of the file, searched for the font name so
    the `ff` nibble immediately before it can be printed. Enough to show that two fixtures differ in
    the byte this probe claims to vary.
    """
    with open(doc, 'rb') as handle:
        blob = handle.read()
    name = 'Zqxwv Nonesuch'.encode('utf-16-le')
    at = blob.find(name)
    if at < 0:
        return '(name not found in the .doc)'
    # FFN: [cbFfnM1][aFFNBase byte with prg:2 fTrueType:1 _:1 ff:3][wWeight]…[xszFfn]
    return f'ff={(blob[at - 38] >> 4) & 0x07} (byte 0x{blob[at - 38]:02x} at -38)'


CASES = [
    ('Zqxwv Nonesuch', None, 'the question: no generic at all'),
    ('Zqxwv Nonesuch', 'roman', 'control: ff=1'),
    ('Zqxwv Nonesuch', 'swiss', 'control: ff=2'),
    ('Zqxwv Nonesuch', 'modern', 'control: ff=3'),
    ('Zqxwv Nonesuch', 'decorative', 'control: ff=5'),
    ('Garamond', None, 'name-override list: forced FAMILY_ROMAN'),
    ('Univers', None, 'name-override list: forced FAMILY_SWISS'),
    ('Helvetica', None, 'name-override list: "Helv" prefix, forced FAMILY_SWISS'),
    ('Aptos', None, 'no override, no generic'),
]


def main():
    out = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else '.')
    for sub in ('src', 'doc', 'pdf', 'prof'):
        os.makedirs(os.path.join(out, sub), exist_ok=True)
    print(subprocess.run(['soffice', '--version'], capture_output=True).stdout.decode().strip())
    print()
    print(f"{'family':18s} {'generic':12s} {'FFN':28s} {'26.2.4.2 draws':26s} what it is")

    for index, (family, generic, note) in enumerate(CASES):
        safe = re.sub(r'[^A-Za-z0-9]+', '_', f'{family}-{generic}')
        source = os.path.join(out, 'src', safe + '.fodt')
        fodt(source, family, generic)
        run(out, index, 'doc', source, os.path.join(out, 'doc'))
        doc = os.path.join(out, 'doc', safe + '.doc')
        if not os.path.exists(doc):
            print(f'{family:18s} {str(generic):12s} {"(export failed)":28s}')
            continue
        run(out, index, 'pdf', doc, os.path.join(out, 'pdf'))
        drawn = ', '.join(faces(os.path.join(out, 'pdf', safe + '.pdf'))) or '(nothing embedded)'
        code = ffn_bytes(doc) if family == 'Zqxwv Nonesuch' else '-'
        print(f'{family:18s} {str(generic):12s} {code:28s} {drawn:26s} {note}')


if __name__ == '__main__':
    main()
