#!/usr/bin/env python3
"""Does a synthetic oblique survive a paragraph that is otherwise uniform?

    python3 oblique-uniform.py <outdir> [workers]

`shear-chars.py` says the words track under-shears on 16 documents by drawing **nothing**
sheared where the reference shears something, and over-shears on a handful of others.  The
under-shear class has a candidate seat that costs nothing to test: all four word-processing
readers build a paragraph's `PageRun` list only when the paragraph's formatting *varies*, and
the list of properties that count as varying is written out at each of the four sites.  Slant
is not on it, and it does not have to be — for `Arial` an italic run resolves to
`LiberationSans-Italic`, a **different `OpenTypeFace`**, so `face != paragraphFace` fires and
the run survives.  The families with **no italic installed at all** are exactly the fallback
faces — DejaVu Sans and DejaVu Serif have Book and Bold and nothing else — so for those an
italic run resolves to the *same* face as its upright neighbour, every other test passes, and
the whole paragraph is folded into one upright run.

Each package is **one paragraph of two runs**, identical but for the one thing being varied, so
the sheared-glyph count of the second run is the only quantity that can move.  The cases are
built to *discriminate* rather than to confirm:

  * `nonesuch/i` is the claim.  `nonesuch/i+sz` is the same paragraph with a second difference
    the shortcut already tests, so if the shortcut is the seat these two disagree on our side
    and agree on the reference's — which no other hypothesis about italic predicts.
  * `arial/i` and `courier/i` are families whose italic *is* installed, so nothing should shear
    on either side; they separate "we never shear" from "we lose it here".
  * `nonesuch/para-i` puts the italic on the paragraph mark as well, so the paragraph's own face
    request is the italic one and there is no run to lose.
  * `nonesuch/iCs` states complex-script italic only, which bears on the *over*-shear class.
"""
import os
import re
import subprocess
import sys
import zipfile
from concurrent.futures import ThreadPoolExecutor

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import importlib.util
_spec = importlib.util.spec_from_file_location(
    "shearfaces", os.path.join(os.path.dirname(os.path.abspath(__file__)), "shear-faces.py"))
shearfaces = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(shearfaces)

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
PKG_R = 'http://schemas.openxmlformats.org/package/2006/relationships'

PLAIN = 'Handgloves upright aaaa '
LEAN = 'Handgloves slanted bbbb'
TARGET = 'Zqxwv Nonesuch'


def package(path, *, family, table_class=None, run2='<w:i/>', para_rpr='', style_i=False):
    entries = f'<w:font w:name="{family}">' + (
        f'<w:family w:val="{table_class}"/>' if table_class else '') + '</w:font>'
    font_table = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                  f'<w:fonts xmlns:w="{W}">{entries}</w:fonts>')
    normal = ('<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
              '<w:name w:val="Normal"/>'
              + ('<w:rPr><w:i/></w:rPr>' if style_i else '') + '</w:style>')
    styles_xml = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                  f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
                  f'<w:rFonts w:ascii="{family}" w:hAnsi="{family}"/><w:sz w:val="40"/>'
                  '</w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>'
                  f'{normal}</w:styles>')
    ppr = f'<w:pPr><w:rPr>{para_rpr}</w:rPr></w:pPr>' if para_rpr else ''
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document xmlns:w="{W}"><w:body>'
           f'<w:p>{ppr}'
           f'<w:r><w:t xml:space="preserve">{PLAIN}</w:t></w:r>'
           f'<w:r><w:rPr>{run2}</w:rPr><w:t xml:space="preserve">{LEAN}</w:t></w:r>'
           '</w:p>'
           '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
           '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>'
           '</w:sectPr></w:body></w:document>')
    parts = [('document.xml', 'document.main'), ('styles.xml', 'styles'),
             ('fontTable.xml', 'fontTable')]
    over = ''.join(
        f'<Override PartName="/word/{n}" ContentType="application/vnd.openxmlformats-'
        f'officedocument.wordprocessingml.{k}+xml"/>' for n, k in parts)
    ctypes = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
              '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
              'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
              + over + '</Types>')
    rels = [f'<Relationship Id="rId1" Type="{R}/officeDocument" Target="document.xml"/>',
            f'<Relationship Id="rId8" Type="{R}/styles" Target="styles.xml"/>',
            f'<Relationship Id="rId9" Type="{R}/fontTable" Target="fontTable.xml"/>']
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ctypes)
        z.writestr('_rels/.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}"><Relationship Id="rId1" Type="{R}'
                   '/officeDocument" Target="word/document.xml"/></Relationships>')
        z.writestr('word/_rels/document.xml.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}">{"".join(rels)}</Relationships>')
        z.writestr('word/document.xml', doc)
        z.writestr('word/styles.xml', styles_xml)
        z.writestr('word/fontTable.xml', font_table)


def cases():
    return [
        # name, kwargs, what the reference is expected to do (stated in advance)
        ('nonesuch/i', dict(family=TARGET), 'shears run 2'),
        ('nonesuch/i+sz', dict(family=TARGET, run2='<w:i/><w:sz w:val="32"/>'), 'shears run 2'),
        ('nonesuch/sz-only', dict(family=TARGET, run2='<w:sz w:val="32"/>'), 'shears nothing'),
        ('nonesuch-swiss/i', dict(family=TARGET, table_class='swiss'), 'shears run 2'),
        ('arial/i', dict(family='Arial'), 'shears nothing — Liberation Sans has an italic'),
        ('courier/i', dict(family='Courier New'), 'shears nothing — Liberation Mono has one'),
        ('nonesuch/para-i', dict(family=TARGET, para_rpr='<w:i/>'), 'shears run 2 only'),
        ('nonesuch/style-i', dict(family=TARGET, run2='', style_i=True), 'shears both runs'),
        ('nonesuch/iCs', dict(family=TARGET, run2='<w:iCs/>'), 'shears nothing'),
        ('nonesuch/b', dict(family=TARGET, run2='<w:b/>'), 'shears nothing'),
    ]


def render_ref(docx, outdir, slot):
    profile = os.path.join(outdir, f'prof{slot}')
    subprocess.run(
        ['soffice', '-env:UserInstallation=file://' + profile, '--headless', '--norestore',
         '--convert-to', 'pdf', '--outdir', os.path.join(outdir, 'ref'), docx],
        capture_output=True, timeout=300)


def render_ours(cli, docx, outdir):
    subprocess.run([cli, 'render', docx, '--outdir', os.path.join(outdir, 'ours')],
                   capture_output=True, timeout=300)


def lean_of(pdf):
    lean, flat = shearfaces.census(pdf)
    return sum(lean.values()), sum(flat.values()), {shearfaces.strip(k): v for k, v in lean.items()}


if __name__ == '__main__':
    out = os.path.abspath(sys.argv[1])
    workers = int(sys.argv[2]) if len(sys.argv) > 2 else 6
    cli = os.environ.get('XRAY_OFFICE_CLI') or (
        '/c/sandbox/workdir/wt-words-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/'
        'linux-x64/XRay.Office.Cli')
    os.makedirs(os.path.join(out, 'in'), exist_ok=True)
    os.makedirs(os.path.join(out, 'ref'), exist_ok=True)
    os.makedirs(os.path.join(out, 'ours'), exist_ok=True)

    built = []
    for i, (name, kw, expect) in enumerate(cases()):
        safe = re.sub(r'[^A-Za-z0-9]+', '-', name).strip('-')
        path = os.path.join(out, 'in', safe + '.docx')
        package(path, **kw)
        built.append((name, safe, path, expect, i))

    with ThreadPoolExecutor(workers) as pool:
        list(pool.map(lambda t: render_ref(t[2], out, t[4] % workers), built))
    for t in built:
        render_ours(cli, t[2], out)

    print(f"{'case':22s} {'ref lean':>9} {'ref flat':>9} {'our lean':>9} {'our flat':>9}  "
          f"{'ref face':22s} expected")
    for name, safe, path, expect, _ in built:
        rp = os.path.join(out, 'ref', safe + '.pdf')
        op = os.path.join(out, 'ours', safe + '.pdf')
        if not os.path.exists(rp) or not os.path.exists(op):
            print(f'{name:22s}  !! missing {"ref" if not os.path.exists(rp) else "ours"}')
            continue
        rl, rf, rfaces = lean_of(rp)
        ol, of, ofaces = lean_of(op)
        print(f'{name:22s} {rl:9d} {rf:9d} {ol:9d} {of:9d}  '
              f'{",".join(sorted(rfaces)) or "-":22s} {expect}')
