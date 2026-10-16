#!/usr/bin/env python3
"""What actually decides the face 26.2.4.2 draws when the named family is not installed.

    python3 font-fallback-rule.py <outdir> [workers]

Round 53 (`probes/words-r53/font-fallback-recheck.py`) established that ten unrecognised
families all answer DejaVu **Serif** where `SystemFontResolver.GenericFallbacks` says DejaVu
Sans, and that the site's stated reason — "fontconfig's default" — is falsified, because
`fc-match Aptos` and `fc-match ""` both answer `DejaVuSans.ttf`. Ten families answering the
same thing is a *result*; it is not yet a *rule*, because every one of those ten is a name
fontconfig files under **no generic at all**, so the experiment never separated

    (a) "the answer is a fixed default that does not depend on the request", from
    (b) "fontconfig's generic decides, and only the *unfiled* default is wrong".

This probe separates them, and it can, because `/etc/fonts/conf.d/45-latin.conf` files 60
families under a generic and **none of them are installed here** (`fc-list` lists 22 families:
Caladea, Carlito, DejaVu ×3, Liberation ×3, OpenSymbol, IPA*, WenQuanYi*). So there are names
whose fontconfig generic is known and whose face is absent:

    filed sans-serif   Candara, Corbel, Century Gothic, Tahoma, Verdana, Trebuchet MS, …
    filed monospace    Consolas, Andale Mono, Inconsolata, Fixedsys, Terminal, Luxi Mono
    filed serif        Constantia, Elephant, Garamond, Georgia, MS Serif, Luxi Serif
    filed nothing      Aptos, Roboto, Lato, Montserrat, Myriad Pro, Futura, Optima, Univers

`fc-match` answers DejaVu Sans / DejaVu Sans Mono / DejaVu Serif / DejaVu Sans respectively —
checked in `main` and printed, so the discriminator is stated rather than assumed. Under (b)
the binary tracks that column. Under (a) every row answers DejaVu Serif.

Four more dimensions, one varied at a time:

  * the **request**: bold, italic, bold-italic, 8 pt, 40 pt, CJK text — does the answer for one
    fixed family move at all?
  * the **declared class**, `word/fontTable.xml`'s `w:family` and `w:pitch`, which LibreOffice's
    DOCX filter reads (`sw/source/writerfilter/dmapper/FontTable.cxx`). Two families, so a slope
    is fixed rather than a point, plus `Garamond` declared `swiss` as a **known-answer control**:
    `DocxLayoutSource.Face` records that exact case measured on 26.2.4.2 as moving the answer
    from DejaVu Serif to DejaVu Sans.
  * the **format**: the same undeclared family through a flat ODF file, whose filter is a
    different one and is documented to honour `style:font-pitch` where the DOCX one does not.
  * the **no-family** case, `SystemFontResolver.cs:439` `[24.2.7-audit: UNDECIDED]`. Round 53's
    probe was confounded: a DOCX carrying no `styles.xml` is given *Word's* default rather than
    LibreOffice's, so it never reached `DefaultFonts`. A flat ODF file declaring no font at all
    does reach it, and so does a DOCX whose `docDefaults` states an empty `w:rFonts`.

Each case is one authored file, converted by the installed `soffice`, with the drawn face read
out of the PDF by `pdffonts`. Controls whose answer is already known (Liberation Serif answers
itself; Calibri is Carlito and Cambria is Caladea by metric alias; Arial is Liberation Sans;
Courier New is Liberation Mono) are run in the same batch, so a run that gets those wrong is
measuring something else and says so.
"""
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
# The *package* relationship namespace is a different one, and a package whose
# `.rels` parts carry the officeDocument namespace instead simply fails to load.
PKG_R = 'http://schemas.openxmlformats.org/package/2006/relationships'

LATIN = 'Handgloves quick brown fox 12345'
CJK = '日本語のテキスト'


def content_types(parts):
    over = ''.join(
        f'<Override PartName="/word/{name}" ContentType="application/vnd.openxmlformats-'
        f'officedocument.wordprocessingml.{kind}+xml"/>'
        for name, kind in parts)
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
            '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
            'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
            + over + '</Types>')


ROOT_RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             f'<Relationships xmlns="{PKG_R}"><Relationship Id="rId1" Type="{R}/officeDocument" '
             'Target="word/document.xml"/></Relationships>')


def docx(path, family, *, bold=False, italic=False, half_points=None, text=LATIN,
         declared_family=None, declared_pitch=None, doc_default=None, styles=False,
         east_asia=False):
    """One package, one paragraph, one run.

    `family` None means no `w:rFonts` at all; '' means an `w:rFonts` whose `w:ascii` is empty.
    `declared_family`/`declared_pitch` add a `word/fontTable.xml` entry for `family`.
    `styles` adds a `styles.xml`; `doc_default` puts a family in its `docDefaults`.
    """
    import zipfile

    if family is None:
        rpr_bits = ''
    elif east_asia:
        rpr_bits = (f'<w:rFonts w:ascii="{family}" w:hAnsi="{family}" w:eastAsia="{family}" '
                    'w:hint="eastAsia"/>')
    else:
        rpr_bits = f'<w:rFonts w:ascii="{family}" w:hAnsi="{family}"/>'
    if bold:
        rpr_bits += '<w:b/>'
    if italic:
        rpr_bits += '<w:i/>'
    if half_points is not None:
        rpr_bits += f'<w:sz w:val="{half_points}"/>'
    props = f'<w:rPr>{rpr_bits}</w:rPr>' if rpr_bits else ''

    body = (f'<w:p><w:pPr>{props}</w:pPr><w:r>{props}<w:t xml:space="preserve">{text}</w:t>'
            '</w:r></w:p>')
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document xmlns:w="{W}"><w:body>{body}'
           '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
           '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>'
           '</w:sectPr></w:body></w:document>')

    parts = [('document.xml', 'document.main')]
    extra = {}
    rels = [f'<Relationship Id="rId1" Type="{R}/officeDocument" Target="document.xml"/>']

    if declared_family is not None or declared_pitch is not None:
        entry = f'<w:font w:name="{family}">'
        if declared_family is not None:
            entry += f'<w:family w:val="{declared_family}"/>'
        if declared_pitch is not None:
            entry += f'<w:pitch w:val="{declared_pitch}"/>'
        entry += '</w:font>'
        extra['word/fontTable.xml'] = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<w:fonts xmlns:w="{W}">{entry}</w:fonts>')
        parts.append(('fontTable.xml', 'fontTable'))
        rels.append(f'<Relationship Id="rId9" Type="{R}/fontTable" Target="fontTable.xml"/>')

    if styles or doc_default is not None:
        if doc_default is None:
            default_fonts = '<w:rFonts/>'
        elif doc_default == '':
            default_fonts = '<w:rFonts w:ascii="" w:hAnsi=""/>'
        else:
            default_fonts = f'<w:rFonts w:ascii="{doc_default}" w:hAnsi="{doc_default}"/>'
        extra['word/styles.xml'] = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
            f'{default_fonts}</w:rPr></w:rPrDefault></w:docDefaults></w:styles>')
        parts.append(('styles.xml', 'styles'))
        rels.append(f'<Relationship Id="rId8" Type="{R}/styles" Target="styles.xml"/>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', content_types(parts))
        z.writestr('_rels/.rels', ROOT_RELS)
        z.writestr('word/_rels/document.xml.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}">{"".join(rels)}</Relationships>')
        z.writestr('word/document.xml', doc)
        for name, blob in extra.items():
            z.writestr(name, blob)


FODT_NS = (
    'xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" '
    'xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" '
    'xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" '
    'xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"')


def fodt(path, family, *, generic=None, pitch=None, text=LATIN):
    """A flat ODF text file. `family` None declares no font anywhere."""
    if family is None:
        decls, style = '', ''
    else:
        bits = f'style:name="probe" svg:font-family="&apos;{family}&apos;"'
        if generic:
            bits += f' style:font-family-generic="{generic}"'
        if pitch:
            bits += f' style:font-pitch="{pitch}"'
        decls = ('<office:font-face-decls>'
                 f'<style:font-face xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:'
                 f'svg-compatible:1.0" {bits}/></office:font-face-decls>')
        style = ('<style:style style:name="P1" style:family="paragraph">'
                 f'<style:text-properties style:font-name="probe"/></style:style>')
    body = ('<text:p text:style-name="P1">' if family is not None else '<text:p>') + text + '</text:p>'
    with open(path, 'w', encoding='utf-8') as handle:
        handle.write(
            '<?xml version="1.0" encoding="UTF-8"?>'
            f'<office:document {FODT_NS} office:version="1.3" '
            'office:mimetype="application/vnd.oasis.opendocument.text">'
            + decls +
            f'<office:automatic-styles>{style}</office:automatic-styles>'
            f'<office:body><office:text>{body}</office:text></office:body></office:document>')


# ---------------------------------------------------------------- the cases

# family -> the generic 45-latin.conf files it under, and what `fc-match` answers because of it.
FILED = {
    'Candara': 'sans', 'Corbel': 'sans', 'Century Gothic': 'sans', 'Tahoma': 'sans',
    'Verdana': 'sans', 'Trebuchet MS': 'sans', 'Britannic': 'sans', 'Luxi Sans': 'sans',
    'Consolas': 'mono', 'Andale Mono': 'mono', 'Inconsolata': 'mono', 'Fixedsys': 'mono',
    'Terminal': 'mono', 'Luxi Mono': 'mono',
    'Constantia': 'serif', 'Elephant': 'serif', 'Garamond': 'serif', 'Georgia': 'serif',
    'MS Serif': 'serif', 'Luxi Serif': 'serif', 'Palatino Linotype': 'serif',
    'Impact': 'fantasy', 'Cooper Std': 'fantasy',
    'Comic Sans MS': 'cursive', 'Zapfino': 'cursive',
    'Segoe UI': 'system-ui', 'Cantarell': 'system-ui',
    'Aptos': 'none', 'Roboto': 'none', 'Lato': 'none', 'Montserrat': 'none',
    'Myriad Pro': 'none', 'Futura': 'none', 'Optima': 'none', 'Univers': 'none',
    'Zzqqxx Nonesuch': 'none', 'Nonesuch Serif MT': 'none', 'Nonesuch Mono': 'none',
    'Nonesuch Gothic': 'none',
}

CONTROLS = {
    'Liberation Serif': 'LiberationSerif',
    'Calibri': 'Carlito',
    'Cambria': 'Caladea',
    'Arial': 'LiberationSans',
    'Courier New': 'LiberationMono',
}


def cases():
    """(group, name, builder) triples."""
    out = []
    for family, want in CONTROLS.items():
        out.append(('A control', f'control:{family}', family,
                    lambda p, f=family: docx(p, f), want))
    for family, filed in FILED.items():
        out.append((f'B filed={filed}', f'plain:{family}', family,
                    lambda p, f=family: docx(p, f), None))

    # C — vary the request, family held at one unfiled name.
    base = 'Aptos'
    for label, kwargs in [
        ('bold', dict(bold=True)),
        ('italic', dict(italic=True)),
        ('bold-italic', dict(bold=True, italic=True)),
        ('8pt', dict(half_points=16)),
        ('40pt', dict(half_points=80)),
        ('cjk-text', dict(text=CJK)),
        ('eastasia-hint', dict(east_asia=True)),
    ]:
        out.append(('C request', f'request:{base}:{label}', base,
                    lambda p, k=kwargs: docx(p, base, **k), None))
    # …and one second point, on a filed name, so a difference is a slope not a point.
    out.append(('C request', 'request:Candara:bold', 'Candara',
                lambda p: docx(p, 'Candara', bold=True), None))

    # D — the declared class, two families plus the recorded known-answer control.
    for family in ('Aptos', 'Candara', 'Consolas', 'Garamond'):
        for label, kwargs in [
            ('swiss', dict(declared_family='swiss')),
            ('roman', dict(declared_family='roman')),
            ('modern', dict(declared_family='modern')),
            ('script', dict(declared_family='script')),
            ('decorative', dict(declared_family='decorative')),
            ('auto', dict(declared_family='auto')),
            ('pitch-fixed', dict(declared_pitch='fixed')),
            ('swiss+fixed', dict(declared_family='swiss', declared_pitch='fixed')),
        ]:
            out.append((f'D declared', f'declared:{family}:{label}', family,
                        lambda p, f=family, k=kwargs: docx(p, f, **k), None))

    # E — the no-family case, four DOCX statements of it and one ODF.
    out.append(('E no-family', 'nofamily:docx-no-styles', None, lambda p: docx(p, None), None))
    out.append(('E no-family', 'nofamily:docx-empty-docdefaults', None,
                lambda p: docx(p, None, styles=True), None))
    out.append(('E no-family', 'nofamily:docx-docdefault-unknown', None,
                lambda p: docx(p, None, doc_default='Aptos'), None))
    out.append(('E no-family', 'nofamily:docx-empty-ascii', '',
                lambda p: docx(p, ''), None))

    # F — the ODF filter, same questions.
    out.append(('F odf', 'odf:no-font-at-all', None, lambda p: fodt(p, None), None))
    for family in ('Aptos', 'Candara', 'Consolas', 'Garamond'):
        out.append(('F odf', f'odf:{family}', family, lambda p, f=family: fodt(p, f), None))
    for label, kwargs in [('swiss', dict(generic='swiss')), ('roman', dict(generic='roman')),
                          ('modern', dict(generic='modern')), ('fixed', dict(pitch='fixed'))]:
        out.append(('F odf', f'odf:Aptos:{label}', 'Aptos',
                    lambda p, k=kwargs: fodt(p, 'Aptos', **k), None))
    return out


def faces(pdf):
    if not os.path.exists(pdf):
        return []
    text = subprocess.run(['pdffonts', pdf], capture_output=True).stdout.decode('utf-8', 'replace')
    names = []
    for line in text.splitlines()[2:]:
        if not line.strip():
            continue
        names.append(re.sub(r'^[A-Z]{6}\+', '', line.split()[0]))
    return names


def convert(out, index, name, build, ext):
    safe = re.sub(r'[^A-Za-z0-9]+', '_', name)
    source = os.path.join(out, 'src', safe + ext)
    build(source)
    env = dict(os.environ, SOURCE_DATE_EPOCH='1700000000', TZ='UTC')
    profile = os.path.join(out, 'prof', f'p{index}')
    subprocess.run(['soffice', '--headless', '-env:UserInstallation=file://' + profile,
                    '--convert-to', 'pdf', '--outdir', os.path.join(out, 'pdf'), source],
                   capture_output=True, env=env, timeout=300)
    return faces(os.path.join(out, 'pdf', safe + '.pdf'))


def fc(family):
    if family is None:
        return '(no family)'
    line = subprocess.run(['fc-match', family], capture_output=True).stdout.decode().strip()
    return line.split(':')[0]


def main():
    out = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else '.')
    workers = int(sys.argv[2]) if len(sys.argv) > 2 else 8
    for sub in ('src', 'pdf', 'prof'):
        os.makedirs(os.path.join(out, sub), exist_ok=True)

    print(subprocess.run(['soffice', '--version'], capture_output=True).stdout.decode().strip())
    print()

    todo = cases()
    with ThreadPoolExecutor(max_workers=workers) as pool:
        futures = []
        for index, (group, name, family, build, expect) in enumerate(todo):
            ext = '.fodt' if name.startswith('odf:') else '.docx'
            futures.append(pool.submit(convert, out, index, name, build, ext))
        drawn = [f.result() for f in futures]

    print(f"{'group':14s} {'case':34s} {'fc-match':22s} {'26.2.4.2 draws':34s} note")
    bad = 0
    for (group, name, family, _build, expect), got in zip(todo, drawn):
        note = ''
        if expect is not None:
            ok = any(expect in one.replace('-', '').replace(' ', '') for one in got)
            note = 'control agrees' if ok else 'CONTROL DISAGREES'
            bad += not ok
        print(f'{group:14s} {name:34s} {fc(family):22s} '
              f'{(", ".join(got) or "(nothing embedded)"):34s} {note}')
    print(f'\n{bad} control(s) disagree of {len(CONTROLS)}')


if __name__ == '__main__':
    main()
