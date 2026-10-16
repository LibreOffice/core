#!/usr/bin/env python3
"""Where the DOCX filter's font *family class* comes from, when the named font does not declare one.

    python3 family-inheritance.py <outdir> [workers]

Round 54 established that through a word-processing filter an unrecognised family answers
DejaVu **Serif** unless `word/fontTable.xml` declares `w:family="swiss"` for it, and shipped
exactly that rule.  One corpus document disobeys it — `24-25_FAA_Holdover_Tables.docx`, where the
reference draws DejaVu **Sans** for `Arial Bold`, a font the table files under `w:family="auto"` —
and eight one-variable edits to that package failed to find the variable.

The hypothesis this probe tests comes from `sw/source/writerfilter/dmapper`:

  * `FontTable.cxx` maps **only** `roman` and `swiss` onto `awt::FontFamily`; `auto`, `modern`,
    `script`, `decorative` and an absent entry all leave the entry at `DONTKNOW`, and `w:pitch`
    is parsed and dropped entirely.
  * `DomainMapper.cxx` `LN_CT_Fonts_ascii` inserts `PROP_CHAR_FONT_FAMILY` **only if** that
    lookup answered something other than `DONTKNOW`.  `LN_CT_Fonts_asciiTheme` — a theme font —
    inserts the *name* and never touches the family at all.

If that is what 26.2.4.2 does, then the family class is a **separately inherited property**: it
survives from whichever ancestor last named a font the table files under `roman` or `swiss`, and
the run's own font name cannot clear it.  `24-25_FAA_Holdover_Tables` fits exactly — its `Normal`
names `Arial`, which the table files `swiss`, and `Heading2`/`Heading3`/`Caption` are `basedOn`
`Normal` and name `Arial Bold`, which it files `auto`.

Round 54's refutation of "inheritance through the style" was measured on the *whole document's*
embedded font list, and that document draws DejaVu Sans for four other reasons — `Century Gothic`,
`Tahoma` and `CWFZGM+Myriad-BoldItalic` are all declared `swiss` in the same table.  So the
observable was over-determined.  Every case here is **one package, one paragraph, one run**, so
the PDF's font list has exactly one entry that can move.

Cases, one variable at a time, with the donor of the class and the consumer of it separated:

  A  controls          three installed families whose answer is already recorded
  B  no ancestor       the round-54 rule restated, so a run that gets these wrong is broken
  C  docDefaults       the class donated by `w:docDefaults/w:rPrDefault`
  D  style chain       donated by `Normal`, consumed by a style `basedOn` it
  E  run override      donated by the paragraph style, consumed by direct run formatting
  F  explicit wins     the consumer declares its own class over a disagreeing ancestor
  G  theme fonts       the consumer names its font through `w:asciiTheme`
  H  depth / pitch     two style levels; `w:pitch` alone; a font absent from the table entirely
"""
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
A = 'http://schemas.openxmlformats.org/drawingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
PKG_R = 'http://schemas.openxmlformats.org/package/2006/relationships'

TEXT = 'Handgloves quick brown fox 12345'

# The consumer: a name no font matches, that fontconfig files under no generic, and that carries
# no weight or width token a name-normaliser could strip.
TARGET = 'Zqxwv Nonesuch'
# The donors: real names, so the fixture looks like a real package, but never drawn — no run
# uses them, so they cannot appear in the PDF's font list.
SWISS_DONOR = 'Arial'
ROMAN_DONOR = 'Times New Roman'


def package(path, *, fonts, docdefault=None, docdefault_theme=None, styles=(),
            para_style=None, run_font=None, run_theme=None, theme_minor=None,
            theme_major=None, run_hansi_only=None):
    """One package, one paragraph, one run.

    fonts            {name: family-or-None} written into word/fontTable.xml
    docdefault       w:ascii for w:docDefaults/w:rPrDefault
    docdefault_theme w:asciiTheme for the same
    styles           ((styleId, basedOn-or-None, ascii-or-None, asciiTheme-or-None), …)
    para_style       w:pStyle for the one paragraph
    run_font         w:ascii on the one run's own w:rPr
    run_theme        w:asciiTheme on the one run's own w:rPr
    """
    import zipfile

    entries = ''
    for name, fam in fonts.items():
        body = ''
        if fam is not None:
            body += f'<w:family w:val="{fam}"/>' if not fam.startswith('pitch:') \
                else f'<w:pitch w:val="{fam[6:]}"/>'
        entries += f'<w:font w:name="{name}">{body}</w:font>'
    font_table = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                  f'<w:fonts xmlns:w="{W}">{entries}</w:fonts>')

    if docdefault_theme:
        dd = f'<w:rFonts w:asciiTheme="{docdefault_theme}" w:hAnsiTheme="{docdefault_theme}"/>'
    elif docdefault:
        dd = f'<w:rFonts w:ascii="{docdefault}" w:hAnsi="{docdefault}"/>'
    else:
        dd = ''
    style_xml = ''
    for sid, based, ascii_, theme in styles:
        rpr = ''
        if theme:
            rpr = f'<w:rFonts w:asciiTheme="{theme}" w:hAnsiTheme="{theme}"/>'
        elif ascii_:
            rpr = f'<w:rFonts w:ascii="{ascii_}" w:hAnsi="{ascii_}"/>'
        style_xml += (f'<w:style w:type="paragraph" w:styleId="{sid}"><w:name w:val="{sid}"/>'
                      + (f'<w:basedOn w:val="{based}"/>' if based else '')
                      + (f'<w:rPr>{rpr}</w:rPr>' if rpr else '')
                      + '</w:style>')
    styles_xml = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                  f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>{dd}</w:rPr>'
                  f'</w:rPrDefault><w:pPrDefault/></w:docDefaults>{style_xml}</w:styles>')

    if run_theme:
        run_rpr = f'<w:rPr><w:rFonts w:asciiTheme="{run_theme}" w:hAnsiTheme="{run_theme}"/></w:rPr>'
    elif run_hansi_only:
        run_rpr = f'<w:rPr><w:rFonts w:hAnsi="{run_hansi_only}"/></w:rPr>'
    elif run_font:
        run_rpr = f'<w:rPr><w:rFonts w:ascii="{run_font}" w:hAnsi="{run_font}"/></w:rPr>'
    else:
        run_rpr = ''
    ppr = f'<w:pPr><w:pStyle w:val="{para_style}"/></w:pPr>' if para_style else ''
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document xmlns:w="{W}"><w:body>'
           f'<w:p>{ppr}<w:r>{run_rpr}<w:t xml:space="preserve">{TEXT}</w:t></w:r></w:p>'
           '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
           '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>'
           '</w:sectPr></w:body></w:document>')

    parts = [('document.xml', 'document.main'), ('styles.xml', 'styles'),
             ('fontTable.xml', 'fontTable')]
    rels = [f'<Relationship Id="rId1" Type="{R}/officeDocument" Target="document.xml"/>',
            f'<Relationship Id="rId8" Type="{R}/styles" Target="styles.xml"/>',
            f'<Relationship Id="rId9" Type="{R}/fontTable" Target="fontTable.xml"/>']
    extra = {'word/styles.xml': styles_xml, 'word/fontTable.xml': font_table}

    theme_override = ''
    if theme_minor or theme_major:
        theme = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                 f'<a:theme xmlns:a="{A}" name="probe"><a:themeElements>'
                 '<a:clrScheme name="p"><a:dk1><a:sysClr val="windowText" lastClr="000000"/></a:dk1>'
                 '<a:lt1><a:sysClr val="window" lastClr="FFFFFF"/></a:lt1>'
                 '<a:dk2><a:srgbClr val="000000"/></a:dk2><a:lt2><a:srgbClr val="FFFFFF"/></a:lt2>'
                 '<a:accent1><a:srgbClr val="000000"/></a:accent1>'
                 '<a:accent2><a:srgbClr val="000000"/></a:accent2>'
                 '<a:accent3><a:srgbClr val="000000"/></a:accent3>'
                 '<a:accent4><a:srgbClr val="000000"/></a:accent4>'
                 '<a:accent5><a:srgbClr val="000000"/></a:accent5>'
                 '<a:accent6><a:srgbClr val="000000"/></a:accent6>'
                 '<a:hlink><a:srgbClr val="000000"/></a:hlink>'
                 '<a:folHlink><a:srgbClr val="000000"/></a:folHlink></a:clrScheme>'
                 f'<a:fontScheme name="p"><a:majorFont><a:latin typeface="{theme_major or ROMAN_DONOR}"/>'
                 '<a:ea typeface=""/><a:cs typeface=""/></a:majorFont>'
                 f'<a:minorFont><a:latin typeface="{theme_minor or ROMAN_DONOR}"/>'
                 '<a:ea typeface=""/><a:cs typeface=""/></a:minorFont></a:fontScheme>'
                 '<a:fmtScheme name="p"><a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/>'
                 '</a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
                 '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:fillStyleLst>'
                 '<a:lnStyleLst><a:ln><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>'
                 '<a:ln><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>'
                 '<a:ln><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln></a:lnStyleLst>'
                 '<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle>'
                 '<a:effectStyle><a:effectLst/></a:effectStyle>'
                 '<a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>'
                 '<a:bgFillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
                 '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
                 '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:bgFillStyleLst>'
                 '</a:fmtScheme></a:themeElements></a:theme>')
        extra['word/theme/theme1.xml'] = theme
        rels.append(f'<Relationship Id="rId7" Type="{R}/theme" Target="theme/theme1.xml"/>')
        theme_override = ('<Override PartName="/word/theme/theme1.xml" ContentType='
                          '"application/vnd.openxmlformats-officedocument.theme+xml"/>')

    over = ''.join(
        f'<Override PartName="/word/{name}" ContentType="application/vnd.openxmlformats-'
        f'officedocument.wordprocessingml.{kind}+xml"/>' for name, kind in parts)
    ctypes = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
              '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
              'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
              + over + theme_override + '</Types>')
    root_rels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                 f'<Relationships xmlns="{PKG_R}"><Relationship Id="rId1" Type="{R}/officeDocument" '
                 'Target="word/document.xml"/></Relationships>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ctypes)
        z.writestr('_rels/.rels', root_rels)
        z.writestr('word/_rels/document.xml.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}">{"".join(rels)}</Relationships>')
        z.writestr('word/document.xml', doc)
        for name, blob in extra.items():
            z.writestr(name, blob)


TBL = {TARGET: 'auto', SWISS_DONOR: 'swiss', ROMAN_DONOR: 'roman'}


def cases():
    """(group, name, kwargs, expected-or-None). `expected` is a *stated* prediction."""
    out = []

    # A — controls whose answer is already recorded, in this fixture shape.
    out.append(('A control', 'control:Arial', dict(fonts=TBL, run_font='Arial'),
                'LiberationSans'))
    out.append(('A control', 'control:Calibri',
                dict(fonts={**TBL, 'Calibri': 'swiss'}, run_font='Calibri'), 'Carlito'))
    out.append(('A control', 'control:LiberationSerif',
                dict(fonts={**TBL, 'Liberation Serif': 'roman'}, run_font='Liberation Serif'),
                'LiberationSerif'))

    # B — the round-54 rule restated: no ancestor names anything.
    out.append(('B no ancestor', 'target:auto', dict(fonts=TBL, run_font=TARGET), 'DejaVuSerif'))
    out.append(('B no ancestor', 'target:swiss',
                dict(fonts={**TBL, TARGET: 'swiss'}, run_font=TARGET), 'DejaVuSans'))
    out.append(('B no ancestor', 'target:roman',
                dict(fonts={**TBL, TARGET: 'roman'}, run_font=TARGET), 'DejaVuSerif'))
    out.append(('B no ancestor', 'target:modern',
                dict(fonts={**TBL, TARGET: 'modern'}, run_font=TARGET), 'DejaVuSerif'))

    # C — the class donated by docDefaults, consumed by the run.
    out.append(('C docDefaults', 'dd-swiss/run-auto',
                dict(fonts=TBL, docdefault=SWISS_DONOR, run_font=TARGET), None))
    out.append(('C docDefaults', 'dd-roman/run-auto',
                dict(fonts=TBL, docdefault=ROMAN_DONOR, run_font=TARGET), None))
    out.append(('C docDefaults', 'dd-swiss/run-none',
                dict(fonts={**TBL, SWISS_DONOR: 'swiss'}, docdefault=TARGET,
                     run_font=None), None))

    # D — donated by `Normal`, consumed by a style `basedOn` it. This is the shape of
    #     24-25_FAA_Holdover_Tables: Normal names Arial (swiss), Heading2 names Arial Bold (auto).
    out.append(('D style chain', 'normal-swiss/derived-auto',
                dict(fonts=TBL, styles=(('Normal', None, SWISS_DONOR, None),
                                        ('Derived', 'Normal', TARGET, None)),
                     para_style='Derived'), None))
    out.append(('D style chain', 'normal-roman/derived-auto',
                dict(fonts=TBL, styles=(('Normal', None, ROMAN_DONOR, None),
                                        ('Derived', 'Normal', TARGET, None)),
                     para_style='Derived'), None))
    out.append(('D style chain', 'normal-none/derived-auto',
                dict(fonts=TBL, styles=(('Normal', None, None, None),
                                        ('Derived', 'Normal', TARGET, None)),
                     para_style='Derived'), None))

    # E — donated by the paragraph style, consumed by *direct run formatting*. Round 54's
    #     authored refutation had this shape and answered Serif.
    out.append(('E run override', 'style-swiss/run-auto',
                dict(fonts=TBL, styles=(('Normal', None, SWISS_DONOR, None),),
                     para_style='Normal', run_font=TARGET), None))
    out.append(('E run override', 'style-roman/run-auto',
                dict(fonts=TBL, styles=(('Normal', None, ROMAN_DONOR, None),),
                     para_style='Normal', run_font=TARGET), None))
    out.append(('E run override', 'dd-swiss/style-none/run-auto',
                dict(fonts=TBL, docdefault=SWISS_DONOR,
                     styles=(('Normal', None, None, None),),
                     para_style='Normal', run_font=TARGET), None))

    # F — the consumer declares its own class over a disagreeing ancestor.
    out.append(('F explicit', 'dd-swiss/target-roman',
                dict(fonts={**TBL, TARGET: 'roman'}, docdefault=SWISS_DONOR,
                     run_font=TARGET), None))
    out.append(('F explicit', 'dd-roman/target-swiss',
                dict(fonts={**TBL, TARGET: 'swiss'}, docdefault=ROMAN_DONOR,
                     run_font=TARGET), None))
    out.append(('F explicit', 'normal-swiss/derived-roman',
                dict(fonts={**TBL, TARGET: 'roman'},
                     styles=(('Normal', None, SWISS_DONOR, None),
                             ('Derived', 'Normal', TARGET, None)), para_style='Derived'), None))

    # G — the consumer names its font through the theme, which sets no class at all.
    out.append(('G theme', 'normal-swiss/derived-theme',
                dict(fonts=TBL, theme_minor=TARGET,
                     styles=(('Normal', None, SWISS_DONOR, None),
                             ('Derived', 'Normal', None, 'minorHAnsi')),
                     para_style='Derived'), None))
    out.append(('G theme', 'normal-roman/derived-theme',
                dict(fonts=TBL, theme_minor=TARGET,
                     styles=(('Normal', None, ROMAN_DONOR, None),
                             ('Derived', 'Normal', None, 'minorHAnsi')),
                     para_style='Derived'), None))
    out.append(('G theme', 'dd-theme-only',
                dict(fonts=TBL, theme_minor=TARGET, docdefault_theme='minorHAnsi'), None))
    out.append(('G theme', 'dd-swiss/run-theme',
                dict(fonts=TBL, theme_minor=TARGET, docdefault=SWISS_DONOR,
                     run_theme='minorHAnsi'), None))
    # the theme font *is* declared swiss in the table — does naming it via the theme still
    # pick that up, or is the table only consulted for w:ascii?
    out.append(('G theme', 'dd-roman/run-theme-swiss-in-table',
                dict(fonts={**TBL, TARGET: 'swiss'}, theme_minor=TARGET,
                     docdefault=ROMAN_DONOR, run_theme='minorHAnsi'), None))

    # H — depth, pitch, and a font that is not in the table at all.
    out.append(('H other', 'two-levels-deep',
                dict(fonts={**TBL, 'Middle Name': 'auto'},
                     styles=(('Normal', None, SWISS_DONOR, None),
                             ('Mid', 'Normal', 'Middle Name', None),
                             ('Derived', 'Mid', TARGET, None)), para_style='Derived'), None))
    out.append(('H other', 'dd-swiss/target-absent-from-table',
                dict(fonts={SWISS_DONOR: 'swiss'}, docdefault=SWISS_DONOR,
                     run_font=TARGET), None))
    out.append(('H other', 'dd-swiss/target-pitch-fixed-only',
                dict(fonts={**TBL, TARGET: 'pitch:fixed'}, docdefault=SWISS_DONOR,
                     run_font=TARGET), None))
    out.append(('H other', 'dd-swiss/run-hAnsi-only',
                dict(fonts=TBL, docdefault=SWISS_DONOR, run_hansi_only=TARGET), None))
    return out


def faces(pdf):
    if not os.path.exists(pdf):
        return []
    text = subprocess.run(['pdffonts', pdf], capture_output=True).stdout.decode('utf-8', 'replace')
    return [re.sub(r'^[A-Z]{6}\+', '', line.split()[0])
            for line in text.splitlines()[2:] if line.strip()]


def convert(out, index, name, kwargs):
    safe = re.sub(r'[^A-Za-z0-9]+', '_', name)
    source = os.path.join(out, 'src', safe + '.docx')
    package(source, **kwargs)
    env = dict(os.environ, SOURCE_DATE_EPOCH='1700000000', TZ='UTC')
    profile = os.path.join(out, 'prof', f'p{index}')
    subprocess.run(['soffice', '--headless', '-env:UserInstallation=file://' + profile,
                    '--convert-to', 'pdf', '--outdir', os.path.join(out, 'pdf'), source],
                   capture_output=True, env=env, timeout=300)
    return faces(os.path.join(out, 'pdf', safe + '.pdf'))


def main():
    out = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else '.')
    workers = int(sys.argv[2]) if len(sys.argv) > 2 else 8
    for sub in ('src', 'pdf', 'prof'):
        os.makedirs(os.path.join(out, sub), exist_ok=True)
    print(subprocess.run(['soffice', '--version'], capture_output=True).stdout.decode().strip())
    print(f'fc-match "{TARGET}" -> '
          + subprocess.run(['fc-match', TARGET], capture_output=True).stdout.decode().strip())
    print()

    todo = cases()
    with ThreadPoolExecutor(max_workers=workers) as pool:
        drawn = [f.result() for f in
                 [pool.submit(convert, out, i, n, k) for i, (_g, n, k, _e) in enumerate(todo)]]

    print(f"{'group':16s} {'case':38s} {'26.2.4.2 draws':34s} note")
    bad = 0
    for (group, name, _k, expect), got in zip(todo, drawn):
        note = ''
        if expect is not None:
            ok = any(expect == one.replace('-', '').replace(' ', '').split(',')[0]
                     for one in got)
            note = 'as predicted' if ok else 'PREDICTION MISSED'
            bad += not ok
        print(f'{group:16s} {name:38s} {(", ".join(got) or "(nothing embedded)"):34s} {note}')
    stated = sum(1 for c in todo if c[3] is not None)
    print(f'\n{bad} of {stated} stated predictions missed')


if __name__ == '__main__':
    main()
