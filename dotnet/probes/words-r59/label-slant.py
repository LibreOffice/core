#!/usr/bin/env python3
"""Does a list label lean, and what states the lean — over all four reader formats.

    python3 label-slant.py <outdir> [workers]

Round 58 pinned the *docx* half of this over five packages: the level's own `w:rPr` leans the
bullet, the paragraph mark's leans it, a run's does not.  Two things that table cannot answer,
and both decide the implementation:

  * **Precedence.**  `technical-architecture.docx` states `<w:i w:val="0"/>` on nine of its
    levels while its list paragraphs' marks are italic, so "level if stated, else the mark" and
    "level OR mark" give different answers on a real corpus document.  Cases 6 and 7 separate
    them.
  * **The other three readers.**  The words corpus is 271 `.docx` and 66 `.doc` and nothing
    else, so `.doc`, `.odt` and `.rtf` have no found witness at all.  Each authored `.docx` is
    round-tripped through the installed 26.2.4.2 into those three formats and rendered from
    them, which is the same varied-format axis round 58 used and round 53 was caught without.
    A format where the round trip *drops* the property measures nothing about the reader and
    is reported as such rather than counted.

Two label kinds, because they take different paths through every reader:

  * a **bullet** — Symbol's U+F0B7, which LibreOffice recodes into OpenSymbol.  OpenSymbol has
    no italic, so a lean there is necessarily synthetic and shows up as a sheared text matrix.
  * a **number** — `%1.` in Liberation Serif, whose italic *is* installed, so a lean there is a
    different `/BaseFont` and no shear at all.  Counted by face name, not by shear.

Refuses to print unless every package produced both halves in every format.
"""
import importlib.util
import os
import re
import shutil
import subprocess
import sys
import zipfile
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location(
    "shearfaces", os.path.join(HERE, "..", "words-r56", "shear-faces.py"))
shearfaces = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(shearfaces)

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
PKG_R = 'http://schemas.openxmlformats.org/package/2006/relationships'

BULLET = '\uf0b7'    # Symbol's bullet slot; recoded into OpenSymbol, which has no italic.


def zipup(path, parts):
    over = ''.join(
        f'<Override PartName="/word/{n}" ContentType="application/vnd.openxmlformats-'
        f'officedocument.wordprocessingml.{k}+xml"/>' for n, k in parts.keys())
    ctypes = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
              '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
              'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
              + over + '</Types>')
    rels = ''.join(
        f'<Relationship Id="rId{i + 8}" Type="{R}/{k}" Target="{n}"/>'
        for i, (n, k) in enumerate(parts.keys()) if n != 'document.xml')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ctypes)
        z.writestr('_rels/.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}"><Relationship Id="rId1" Type="{R}'
                   '/officeDocument" Target="word/document.xml"/></Relationships>')
        z.writestr('word/_rels/document.xml.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}">{rels}</Relationships>')
        for (name, _), body in parts.items():
            z.writestr('word/' + name, body)


def package(path, *, kind, level_rpr, para_rpr, run_rpr, style_rpr=''):
    if kind == 'bullet':
        fmt = ('<w:numFmt w:val="bullet"/>'
               f'<w:lvlText w:val="{BULLET}"/>')
        fonts = '<w:rFonts w:ascii="Symbol" w:hAnsi="Symbol"/>'
    else:
        fmt = '<w:numFmt w:val="decimal"/><w:lvlText w:val="%1."/>'
        fonts = ''
    numbering = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                 f'<w:numbering xmlns:w="{W}">'
                 '<w:abstractNum w:abstractNumId="0"><w:lvl w:ilvl="0">'
                 f'<w:start w:val="1"/>{fmt}<w:lvlJc w:val="left"/>'
                 '<w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr>'
                 f'<w:rPr>{fonts}{level_rpr}</w:rPr>'
                 '</w:lvl></w:abstractNum>'
                 '<w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num>'
                 '</w:numbering>')
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
              '<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
              '<w:sz w:val="40"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>'
              '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
              '<w:name w:val="Normal"/></w:style>'
              '<w:style w:type="paragraph" w:styleId="Listed"><w:name w:val="Listed"/>'
              '<w:basedOn w:val="Normal"/>'
              f'<w:rPr>{style_rpr}</w:rPr></w:style></w:styles>')
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document xmlns:w="{W}"><w:body>'
           '<w:p><w:pPr><w:pStyle w:val="Listed"/>'
           '<w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>'
           f'<w:rPr>{para_rpr}</w:rPr></w:pPr>'
           f'<w:r><w:rPr>{run_rpr}</w:rPr><w:t>Item one</w:t></w:r></w:p>'
           '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
           '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>'
           '</w:sectPr></w:body></w:document>')
    zipup(path, {('document.xml', 'document.main'): doc,
                 ('styles.xml', 'styles'): styles,
                 ('numbering.xml', 'numbering'): numbering})


CASES = [
    ('control',          dict(level_rpr='', para_rpr='', run_rpr=''),
     'nothing states italic anywhere'),
    ('level',            dict(level_rpr='<w:i/>', para_rpr='', run_rpr=''),
     "round 58: the level's rPr leans the label"),
    ('mark',             dict(level_rpr='', para_rpr='<w:i/>', run_rpr=''),
     "round 58: the paragraph mark's rPr leans it"),
    ('run',              dict(level_rpr='', para_rpr='', run_rpr='<w:i/>'),
     "round 58: a run's does not"),
    ('all',              dict(level_rpr='<w:i/>', para_rpr='<w:i/>', run_rpr='<w:i/>'),
     'all three'),
    ('leveloff-markon',  dict(level_rpr='<w:i w:val="0"/>', para_rpr='<w:i/>', run_rpr=''),
     'DISCRIMINATOR: does an explicit level "off" beat the mark?'),
    ('levelon-markoff',  dict(level_rpr='<w:i/>', para_rpr='<w:i w:val="0"/>', run_rpr=''),
     'DISCRIMINATOR: the other way round'),
    ('style',            dict(level_rpr='', para_rpr='', run_rpr='', style_rpr='<w:i/>'),
     "the paragraph style's rPr, which is where the corpus witnesses state it"),
]

FORMATS = [('docx', None), ('doc', 'doc'), ('odt', 'odt'), ('rtf', 'rtf')]


def convert(src, fmt, outdir, slot):
    profile = os.path.join(outdir, f'prof{slot}')
    subprocess.run(
        ['soffice', '-env:UserInstallation=file://' + profile, '--headless', '--norestore',
         '--convert-to', fmt, '--outdir', os.path.join(outdir, 'in'), src],
        capture_output=True, timeout=300)


def render_ref(src, outdir, slot, sub):
    profile = os.path.join(outdir, f'prof{slot}')
    subprocess.run(
        ['soffice', '-env:UserInstallation=file://' + profile, '--headless', '--norestore',
         '--convert-to', 'pdf', '--outdir', os.path.join(outdir, 'ref', sub), src],
        capture_output=True, timeout=300)


def render_ours(cli, src, outdir, sub):
    subprocess.run([cli, 'render', src, '--outdir', os.path.join(outdir, 'ours', sub)],
                   capture_output=True, timeout=300)


def label_of(pdf, kind):
    """(sheared glyphs, italic-faced glyphs, face names) restricted to the label's own face."""
    lean, flat = shearfaces.census(pdf)
    faces = {shearfaces.strip(k): (lean[k], flat[k]) for k in set(lean) | set(flat)}
    if kind == 'bullet':
        want = [n for n in faces if 'OpenSymbol' in n]
    else:
        want = [n for n in faces if 'LiberationSerif' in n or 'Liberation_Serif' in n]
    shear = sum(faces[n][0] for n in want)
    italicfaced = sum(faces[n][0] + faces[n][1] for n in want if 'Italic' in n)
    return shear, italicfaced, ','.join(sorted(f'{n}:{faces[n][0]}/{faces[n][1]}' for n in want))


if __name__ == '__main__':
    out = os.path.abspath(sys.argv[1])
    workers = int(sys.argv[2]) if len(sys.argv) > 2 else 6
    cli = os.environ.get('XRAY_OFFICE_CLI') or (
        '/c/sandbox/workdir/wt-words-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/'
        'linux-x64/XRay.Office.Cli')
    if os.path.isdir(out):
        shutil.rmtree(out)
    for d in ['in'] + [os.path.join(s, f) for s in ('ref', 'ours') for f, _ in FORMATS]:
        os.makedirs(os.path.join(out, d), exist_ok=True)

    built = []
    n = 0
    for kind in ('bullet', 'number'):
        for name, kw, why in CASES:
            stem = '%02d-%s-%s' % (n, kind, name)
            package(os.path.join(out, 'in', stem + '.docx'), kind=kind, **kw)
            built.append((kind, name, stem, why))
            n += 1

    # The three non-native formats, written by the reference itself.
    jobs = [(t[2], fmt) for t in built for fmt, conv in FORMATS if conv]
    with ThreadPoolExecutor(workers) as pool:
        list(pool.map(
            lambda j: convert(os.path.join(out, 'in', j[0] + '.docx'), j[1], out,
                              jobs.index(j) % workers), jobs))

    renders = [(t[2], fmt) for t in built for fmt, _ in FORMATS]
    with ThreadPoolExecutor(workers) as pool:
        list(pool.map(
            lambda j: render_ref(os.path.join(out, 'in', f'{j[0]}.{j[1]}'), out,
                                 renders.index(j) % workers, j[1]), renders))
    for stem, fmt in renders:
        render_ours(cli, os.path.join(out, 'in', f'{stem}.{fmt}'), out, fmt)

    missing = []
    for stem, fmt in renders:
        if not os.path.exists(os.path.join(out, 'in', f'{stem}.{fmt}')):
            missing.append(f'{stem}.{fmt}: not converted')
            continue
        for side in ('ref', 'ours'):
            if not os.path.exists(os.path.join(out, side, fmt, stem + '.pdf')):
                missing.append(f'{stem}.{fmt}: no {side}')
    if missing:
        print('REFUSING TO PRINT — %d of %d halves missing:' % (len(missing), 2 * len(renders)))
        for m in missing:
            print('   ', m)
        sys.exit(2)
    print('%d packages x %d formats, %d halves, all present\n'
          % (len(built), len(FORMATS), 2 * len(renders)))

    for kind in ('bullet', 'number'):
        metric = 'sheared OpenSymbol glyphs' if kind == 'bullet' \
            else 'glyphs in an italic Liberation Serif'
        print(f'=== the {kind} label — {metric}, ref/ours')
        head = f"{'case':18s}"
        for fmt, _ in FORMATS:
            head += f' {fmt:>9s}'
        print(head + '   why')
        for k, name, stem, why in built:
            if k != kind:
                continue
            row = f'{name:18s}'
            for fmt, _ in FORMATS:
                rs, ri, _ = label_of(os.path.join(out, 'ref', fmt, stem + '.pdf'), kind)
                os_, oi, _ = label_of(os.path.join(out, 'ours', fmt, stem + '.pdf'), kind)
                a, b = (rs, os_) if kind == 'bullet' else (ri, oi)
                row += f' {a:4d}/{b:<4d}'
            print(row + f'   {why}')
        print()

    print('=== the faces each side draws the label in (docx only, for the record)')
    for k, name, stem, why in built:
        _, _, rf = label_of(os.path.join(out, 'ref', 'docx', stem + '.pdf'), k)
        _, _, of = label_of(os.path.join(out, 'ours', 'docx', stem + '.pdf'), k)
        print(f'{stem:26s} ref {rf:52s} ours {of}')
