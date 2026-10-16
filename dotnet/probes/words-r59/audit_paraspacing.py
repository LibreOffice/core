#!/usr/bin/env python3
"""24.2.7-audit: does a DOCX with no settings part still *add* its paragraph spacings?

    python3 audit_paraspacing.py <outdir> [workers]

`WordCompatibility.AddsParagraphSpacing` is `!HasSettingsPart || DoNotUseHtmlParagraphAutoSpacing`,
and its remarks record a measurement taken on **24.2.7.2**: eight paragraphs carrying 12 pt of
space-before and 8 pt of space-after on 12 pt exact lines, so a boundary is 24 pt collapsed and
32 pt added. With a settings part the reference put every boundary at 24.00 pt; with the part
removed entirely, at 32.00 pt; and with `w:doNotUseHTMLParagraphAutoSpacing` added back to a
document that has the part, at 32.00 pt again.

The reference binary is now 26.2.4.2 and the claim is a claim about a superseded one. The three
arms are re-authored here and measured by the **baseline pitch** in the reference's own PDF, which
is what a boundary is: consecutive first-baselines differ by 12 pt within a paragraph and by
12 + spacing at a boundary.

Four packages rather than three, because the second arm has a trap in it. The setting is written
by `DomainMapper_Impl::ApplySettingsTable`, which returns at its first line when there is no
settings table — so "no settings part" and "an empty settings part" are *different* inputs and the
whole claim is about which of them LibreOffice sees. A package with `word/settings.xml` present
but holding an empty `w:settings` is the control that separates them.
"""
import os
import re
import shutil
import subprocess
import sys
import zipfile
from concurrent.futures import ThreadPoolExecutor

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
PKG_R = 'http://schemas.openxmlformats.org/package/2006/relationships'

PARAGRAPHS = 8


def package(path, settings):
    """`settings` is None for no part at all, else the body of `w:settings`."""
    parts = {('document.xml', 'document.main'): None, ('styles.xml', 'styles'): None}
    if settings is not None:
        parts[('settings.xml', 'settings')] = None

    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
              '<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
              '<w:sz w:val="24"/></w:rPr></w:rPrDefault><w:pPrDefault><w:pPr>'
              '<w:spacing w:before="240" w:after="160" w:line="240" w:lineRule="exact"/>'
              '</w:pPr></w:pPrDefault></w:docDefaults>'
              '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
              '<w:name w:val="Normal"/></w:style></w:styles>')
    body = ''.join(f'<w:p><w:r><w:t>Line {i}</w:t></w:r></w:p>' for i in range(PARAGRAPHS))
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document xmlns:w="{W}"><w:body>{body}'
           '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
           '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>'
           '</w:sectPr></w:body></w:document>')

    over = ''.join(
        f'<Override PartName="/word/{n}" ContentType="application/vnd.openxmlformats-'
        f'officedocument.wordprocessingml.{k}+xml"/>' for n, k in parts)
    ctypes = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
              '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
              'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
              + over + '</Types>')
    rels = ''.join(
        f'<Relationship Id="rId{i + 8}" Type="{R}/{k}" Target="{n}"/>'
        for i, (n, k) in enumerate(parts) if n != 'document.xml')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ctypes)
        z.writestr('_rels/.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}"><Relationship Id="rId1" Type="{R}'
                   '/officeDocument" Target="word/document.xml"/></Relationships>')
        z.writestr('word/_rels/document.xml.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}">{rels}</Relationships>')
        z.writestr('word/document.xml', doc)
        z.writestr('word/styles.xml', styles)
        if settings is not None:
            z.writestr('word/settings.xml',
                       '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                       f'<w:settings xmlns:w="{W}">{settings}</w:settings>')


CASES = [
    ('no-settings-part', None, 'the claim: no part at all, so the application default adds'),
    ('empty-settings-part', '', 'a part present but empty — the discriminator'),
    ('compatibility-only', '<w:compat><w:compatSetting w:name="compatibilityMode" '
     'w:uri="http://schemas.microsoft.com/office/word" w:val="15"/></w:compat>',
     'a part naming only compatibilityMode'),
    ('flag-set', '<w:compat><w:doNotUseHTMLParagraphAutoSpacing/></w:compat>',
     'a part with the flag on, which adds by the rule itself'),
]


def render_ref(src, outdir, slot):
    subprocess.run(
        ['soffice', '-env:UserInstallation=file://' + os.path.join(outdir, f'prof{slot}'),
         '--headless', '--norestore', '--convert-to', 'pdf',
         '--outdir', os.path.join(outdir, 'ref'), src], capture_output=True, timeout=300)


def render_ours(cli, src, outdir):
    subprocess.run([cli, 'render', src, '--outdir', os.path.join(outdir, 'ours')],
                   capture_output=True, timeout=300)


def baselines(pdf):
    """Every text origin's y, page by page, in drawing order."""
    sys.path.insert(0, "/c/sandbox/workdir/wt-words-r50/dotnet/research/probes/slides-r15")
    from pdfops import objects, pages, content
    data = open(pdf, 'rb').read()
    objs = objects(data)
    out = []
    token = re.compile(rb'([-\d.]+)\s+([-\d.]+)\s+Td|'
                       rb'([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+'
                       rb'([-\d.]+)\s+Tm')
    for pnum in pages(data, objs):
        for m in token.finditer(content(data, objs, pnum)):
            out.append(round(float(m.group(2) if m.group(2) else m.group(8)), 3))
    return out


def pitch(pdf):
    ys = baselines(pdf)
    return [round(a - b, 2) for a, b in zip(ys, ys[1:])]


if __name__ == '__main__':
    out = os.path.abspath(sys.argv[1])
    workers = int(sys.argv[2]) if len(sys.argv) > 2 else 4
    cli = os.environ.get('XRAY_OFFICE_CLI') or (
        '/c/sandbox/workdir/wt-words-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/'
        'linux-x64/XRay.Office.Cli')
    if os.path.isdir(out):
        shutil.rmtree(out)
    for d in ('in', 'ref', 'ours'):
        os.makedirs(os.path.join(out, d), exist_ok=True)

    built = []
    for index, (name, settings, why) in enumerate(CASES):
        stem = '%02d-%s' % (index, name)
        path = os.path.join(out, 'in', stem + '.docx')
        package(path, settings)
        built.append((name, stem, path, why))

    with ThreadPoolExecutor(workers) as pool:
        list(pool.map(lambda t: render_ref(t[2], out, built.index(t) % workers), built))
    for t in built:
        render_ours(cli, t[2], out)

    missing = [f'{t[1]}: no {side}' for t in built for side in ('ref', 'ours')
               if not os.path.exists(os.path.join(out, side, t[1] + '.pdf'))]
    if missing:
        print('REFUSING TO PRINT — %d halves missing:' % len(missing))
        for m in missing:
            print('   ', m)
        sys.exit(2)

    print('%d packages, %d halves, all present' % (len(built), 2 * len(built)))
    print('collapsed = 24.00 pt between baselines, added = 32.00 pt\n')
    print('%-22s %-30s %-30s %s' % ('case', 'reference pitches', 'our pitches', 'why'))
    for name, stem, path, why in built:
        r = pitch(os.path.join(out, 'ref', stem + '.pdf'))
        o = pitch(os.path.join(out, 'ours', stem + '.pdf'))
        agree = 'AGREE' if r == o else 'DIFFER'
        print('%-22s %-30s %-30s %-7s %s'
              % (name, sorted(set(r)), sorted(set(o)), agree, why))
