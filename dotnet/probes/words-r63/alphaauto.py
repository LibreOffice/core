#!/usr/bin/env python3
"""Does a shape's fill *transparency* change what an automatic font colour resolves to?

Round 62 established, on `012` with four one-variable arms, that a `wps` text box's own fill wins
when it has one and that the walk continues to the anchor's background when it does not. Round 59
measured two counter-witnesses where the reference draws **black** text on a fill that is dark by
`Color::IsDark` — `docs-quality-MA.IMS.00001-…docx` at `#0070C0` (WCAG 39) and
`069_Work_Breakdown_Structure_Template_Professional_Format` at `#8496B0` (WCAG 76).

Both of those fills are **semi-transparent** — `<a:alpha val="52941"/>` and
`<v:fill opacity="26214f"/>` — and `SwDrawTextInfo::ApplyAutoColor` does not ask the fill for its
colour, it asks `SdrAllFillAttributesHelper::getAverageColor(aGlobalRetoucheColor)`, which
interpolates the fill toward the application's retouche colour by the transparency.

So the rival hypotheses are

  H-alpha : the shape's own fill decides, blended toward white by its transparency;
  H-shape : the shape's own fill is never consulted for these two documents, because a VML /
            DrawingML shape's text is drawn by editeng and not by `ApplyAutoColor` at all.

Every arm below is one substitution in one corpus document, and the arms are chosen so that the
two hypotheses answer *differently* — the trap round 62 recorded, where an arm whose colour is
dark under both readings looks like a confirmation and proves nothing.

    alphaauto.py <outdir>
"""
import os
import re
import shutil
import subprocess
import sys
import zipfile

CORPUS = '/c/sandbox/workdir/sample-files/words'
DOCS = {
    '069': f'{CORPUS}/chartset-013/docx/069_Work_Breakdown_Structure_Template_Professional_Format_1e02dce1.docx',
    'ims': f'{CORPUS}/pagination-002/docx/docs-quality-MA.IMS.00001-Integrated-Management-System-manual.docx',
    '012': f'{CORPUS}/chartset-008/docx/012_Project_Timeline_Template_Black_and_Brown_Theme_35c76550.docx',
}

# The one shape each arm touches, and the substitution, applied to `word/document.xml` only.
#   (document, page to read, needle, replacement, what each hypothesis predicts)
S1026 = '<v:rect id="_x0000_s1026"'
IMS_FILL = '<a:solidFill><a:srgbClr val="0070C0"><a:alpha val="52941"/></a:srgbClr></a:solidFill>'
T012_NOFILL = '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/><a:ln w="6350"><a:noFill/></a:ln>'


def rect_fill(xml, sub):
    """Rewrite `_x0000_s1026`'s fill: `sub` is (fillcolor value, opacity element or '')."""
    i = xml.index(S1026)
    j = xml.index('><v:textbox>', i)
    head = xml[i:j + 1]
    colour, opacity = sub
    head = re.sub(r'fillcolor="[^"]*"', f'fillcolor="{colour}"', head)
    head = head.replace('<v:fill opacity="26214f"/>', '')
    return xml[:i] + head + opacity + xml[j + 1:]


ARMS = [
    # id                doc     page  transform                                        H-alpha  H-shape
    ('069-base',        '069', 1, lambda x: x,                                         'black', 'black'),
    ('069-noalpha',     '069', 1, lambda x: rect_fill(x, ('#8496b0 [1951]', '')),      'WHITE', 'black'),
    ('069-blackopaque', '069', 1, lambda x: rect_fill(x, ('black', '')),               'WHITE', 'black'),
    ('069-whiteopaque', '069', 1, lambda x: rect_fill(x, ('white', '')),               'black', 'black'),
    ('069-blackalpha',  '069', 1, lambda x: rect_fill(x, ('black', '<v:fill opacity="13107f"/>')),
                                                                                       'black', 'black'),
    ('ims-base',        'ims', 9, lambda x: x,                                         'black', 'black'),
    ('ims-noalpha',     'ims', 9,
     lambda x: x.replace(IMS_FILL, '<a:solidFill><a:srgbClr val="0070C0"/></a:solidFill>'),
                                                                                       'WHITE', 'black'),
    ('012-base',        '012', 1, lambda x: x,                                         'WHITE', '?'),
    ('012-blackopaque', '012', 1,
     lambda x: x.replace(T012_NOFILL,
                         '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>'
                         '<a:solidFill><a:srgbClr val="000000"/></a:solidFill>'
                         '<a:ln w="6350"><a:noFill/></a:ln>', 1),
                                                                                       'WHITE', '?'),
    ('012-blackalpha',  '012', 1,
     lambda x: x.replace(T012_NOFILL,
                         '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>'
                         '<a:solidFill><a:srgbClr val="000000"><a:alpha val="20000"/></a:srgbClr>'
                         '</a:solidFill><a:ln w="6350"><a:noFill/></a:ln>', 1),
                                                                                       'black', '?'),
]


def author(src, dst, transform):
    with zipfile.ZipFile(src) as zin, zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == 'word/document.xml':
                text = data.decode('utf-8')
                after = transform(text)
                if after == text and 'base' not in dst:
                    raise SystemExit(f'{dst}: substitution matched nothing')
                data = after.encode('utf-8')
            zout.writestr(item, data)


def render(path, outdir, profile):
    env = dict(os.environ, SOURCE_DATE_EPOCH='1700000000', TZ='UTC')
    subprocess.run(
        ['soffice', '--headless', f'-env:UserInstallation=file://{profile}',
         '--convert-to', 'pdf', '--outdir', outdir, path],
        check=True, capture_output=True, env=env, timeout=600)


def main(outdir):
    os.makedirs(outdir, exist_ok=True)
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    profile = os.path.join(outdir, 'prof')
    rows = []
    for name, doc, page, transform, h_alpha, h_shape in ARMS:
        src = DOCS[doc]
        docx = os.path.join(outdir, f'{name}.docx')
        author(src, docx, transform)
        render(docx, outdir, profile)
        pdf = os.path.join(outdir, f'{name}.pdf')
        out = subprocess.run([sys.executable,
                              os.path.join(os.path.dirname(os.path.abspath(__file__)), 'textcolour.py'),
                              pdf, str(page)], capture_output=True, text=True).stdout
        counts = dict(re.findall(r'#([0-9A-F]{6})\s+(\d+)', out))
        white = int(counts.get('FFFFFF', 0))
        black = int(counts.get('000000', 0))
        rows.append((name, h_alpha, h_shape, white, black,
                     'WHITE' if white else 'black'))
        print('%-16s H-alpha %-5s  H-shape %-5s   white %4d  black %4d   -> %s'
              % (name, h_alpha, h_shape, white, black, rows[-1][5]))
    print()
    for name, ha, hs, w, b, got in rows:
        if hs == '?':
            continue
        mark = ('alpha' if got.lower() == ha.lower() else '') or ('shape' if got.lower() == hs.lower() else 'NEITHER')
        print('%-16s decided by: %s' % (name, mark))


if __name__ == '__main__':
    main(sys.argv[1])
