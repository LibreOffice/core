#!/usr/bin/env python3
"""Two rules this round measured but did not implement, each pinned so the next round need not.

    python3 label-and-autocolour.py <outdir> [workers]

**A. Does a list label take the level's own slant?**
Round 58 carried the italic request across glyph fallback and moved 42 of the 206 glyphs its
census said were reachable.  The 164 that did not move are, to the glyph, **the OpenSymbol
column** — 112 across ten documents — plus 52 in one `.doc`.  Those OpenSymbol glyphs are single
`<01>` draws one per line at the left margin: **list bullets**, which reach the page through
`PageDrawing`'s label branch and not through `ByFace` at all.  `A320SimNotes.doc` is the largest,
75 of them, and its body text does not lean on either side — so it is the *level's* own character
formatting that is italic, not the paragraph's.

**B. What colour is `COL_AUTO` text on a dark cell?**
`AFS-050-004-F2_0i.docx` page 2 draws five black banner rows.  The text in them is in our PDF, at
the right positions, in **black on black** — the reference draws 305 glyphs `1 1 1 rg` on that
page and we draw none.  `sw/source/core/txtnode/fntcache.cxx:2369` resolves an automatic font
colour against the frame's background brush and answers white when `Color::IsDark()`.  The
threshold is what needs measuring, because `IsDark()` is `GetWCAGLuminance() <= 87` in the tree in
this checkout and that tree is 27.2.0.0.alpha0+, not the reference.

Both are measured through authored packages with controls, and neither is implemented here.
The probe refuses to print unless every package produced both halves.
"""
import importlib.util
import os
import re
import subprocess
import sys
import zipfile
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location(
    "shearfaces", os.path.join(HERE, "..", "words-r56", "shear-faces.py"))
shearfaces = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(shearfaces)

sys.path.insert(0, "/c/sandbox/workdir/wt-words-r50/dotnet/research/probes/slides-r15")
from pdfops import objects, pages, content  # noqa: E402

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
PKG_R = 'http://schemas.openxmlformats.org/package/2006/relationships'

# U+F0B7, Symbol's bullet slot: the one every Word bullet list uses, and the one LibreOffice
# recodes into OpenSymbol -- which has no italic, so a lean on it has to be synthetic.
BULLET = '\uf0b7'


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


# ---------------------------------------------------------------- A: the list label's slant

def list_package(path, *, level_rpr, para_rpr, run_rpr):
    numbering = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                 f'<w:numbering xmlns:w="{W}">'
                 '<w:abstractNum w:abstractNumId="0"><w:lvl w:ilvl="0">'
                 '<w:start w:val="1"/><w:numFmt w:val="bullet"/>'
                 f'<w:lvlText w:val="{BULLET}"/><w:lvlJc w:val="left"/>'
                 '<w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr>'
                 f'<w:rPr><w:rFonts w:ascii="Symbol" w:hAnsi="Symbol"/>{level_rpr}</w:rPr>'
                 '</w:lvl></w:abstractNum>'
                 '<w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num>'
                 '</w:numbering>')
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
              '<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
              '<w:sz w:val="40"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>'
              '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
              '<w:name w:val="Normal"/></w:style></w:styles>')
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document xmlns:w="{W}"><w:body>'
           '<w:p><w:pPr><w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>'
           f'<w:rPr>{para_rpr}</w:rPr></w:pPr>'
           f'<w:r><w:rPr>{run_rpr}</w:rPr><w:t>Item one</w:t></w:r></w:p>'
           '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
           '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>'
           '</w:sectPr></w:body></w:document>')
    zipup(path, {('document.xml', 'document.main'): doc,
                 ('styles.xml', 'styles'): styles,
                 ('numbering.xml', 'numbering'): numbering})


def list_cases():
    return [
        ('label/nothing-italic', dict(level_rpr='', para_rpr='', run_rpr=''),
         'no lean anywhere — the control'),
        ('label/level-italic', dict(level_rpr='<w:i/>', para_rpr='', run_rpr=''),
         'the bullet leans, the text does not'),
        ('label/para-mark-italic', dict(level_rpr='', para_rpr='<w:i/>', run_rpr=''),
         'does the paragraph mark reach the bullet?'),
        ('label/run-italic', dict(level_rpr='', para_rpr='', run_rpr='<w:i/>'),
         'the text leans; does the bullet?'),
        ('label/all-italic', dict(level_rpr='<w:i/>', para_rpr='<w:i/>', run_rpr='<w:i/>'),
         'both lean'),
    ]


# ------------------------------------------------- B: automatic font colour on a dark cell

def shade_package(path, fill):
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
              '<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
              '<w:sz w:val="40"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>'
              '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
              '<w:name w:val="Normal"/></w:style></w:styles>')
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document xmlns:w="{W}"><w:body>'
           '<w:tbl><w:tblPr><w:tblW w:w="9000" w:type="dxa"/></w:tblPr>'
           '<w:tblGrid><w:gridCol w:w="9000"/></w:tblGrid>'
           '<w:tr><w:tc><w:tcPr><w:tcW w:w="9000" w:type="dxa"/>'
           f'<w:shd w:val="clear" w:color="auto" w:fill="{fill}"/></w:tcPr>'
           '<w:p><w:r><w:t>Reversed out</w:t></w:r></w:p></w:tc></w:tr></w:tbl>'
           '<w:p/>'
           '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
           '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>'
           '</w:sectPr></w:body></w:document>')
    zipup(path, {('document.xml', 'document.main'): doc, ('styles.xml', 'styles'): styles})


def shade_cases():
    # A ramp through grey, plus the primaries, so the threshold is bracketed rather than assumed.
    # `Color::IsDark()` in the 27.2 tree is `GetWCAGLuminance() <= 87`; the reference is 26.2.4.2
    # and this probe does not assume the two agree.
    greys = ['000000', '202020', '404040', '505050', '606060', '707070', '808080',
             '909090', 'A0A0A0', 'C0C0C0', 'FFFFFF',
             # The boundary the first run bracketed to 0x90 <= grey < 0xA0, and where
             # `GetWCAGLuminance() <= 87` puts it exactly: 0.3412 relative luminance is
             # sRGB 157.9, so 0x9D must still be white and 0x9E must already be black.
             # A one-step prediction with no free parameter left in it.
             '9C9C9C', '9D9D9D', '9E9E9E', '9F9F9F']
    colours = ['FF0000', '00FF00', '0000FF', '008000', '000080', 'FFFF00', '00FFFF']
    return [(f'shade/{c}', c) for c in greys + colours]


# ---------------------------------------------------------------------------- rendering

def render_ref(src, outdir, slot):
    profile = os.path.join(outdir, f'prof{slot}')
    subprocess.run(
        ['soffice', '-env:UserInstallation=file://' + profile, '--headless', '--norestore',
         '--convert-to', 'pdf', '--outdir', os.path.join(outdir, 'ref'), src],
        capture_output=True, timeout=300)


def render_ours(cli, src, outdir):
    subprocess.run([cli, 'render', src, '--outdir', os.path.join(outdir, 'ours')],
                   capture_output=True, timeout=300)


GLYPH = re.compile(
    rb'(?:([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+rg)|(?:([-\d.]+)\s+g\b)'
    rb'|(\((?:\\.|[^\\()])*\))\s*Tj|(<[0-9A-Fa-f\s]*>)\s*Tj'
    rb'|(\[(?:\\.|[^\\\[\]])*\])\s*TJ')


def colours_of(pdf):
    """Glyphs per non-stroking colour, over every page."""
    data = open(pdf, 'rb').read()
    objs = objects(data)
    out = {}
    for pnum in pages(data, objs):
        colour = 'initial'
        for m in GLYPH.finditer(content(data, objs, pnum)):
            if m.group(1) is not None:
                colour = ' '.join(x.decode() for x in m.group(1, 2, 3))
            elif m.group(4) is not None:
                g = m.group(4).decode()
                colour = f'{g} {g} {g}'
            else:
                body = m.group(5) or m.group(6) or m.group(7)
                n = 0
                for part in re.finditer(rb'\((?:\\.|[^\\()])*\)|<[0-9A-Fa-f\s]*>', body) \
                        if m.group(7) else [m]:
                    s = part.group(0) if m.group(7) else (m.group(5) or m.group(6))
                    n += (len(re.sub(rb'\\(\d{1,3}|.)', b'x', s[1:-1])) if s[:1] == b'('
                          else len(re.sub(rb'\s', b'', s[1:-1])) // 2)
                out[colour] = out.get(colour, 0) + n
    return out


def lean_of(pdf):
    lean, flat = shearfaces.census(pdf)
    return (sum(lean.values()), sum(flat.values()),
            {shearfaces.strip(k): v for k, v in lean.items() if v})


if __name__ == '__main__':
    out = os.path.abspath(sys.argv[1])
    workers = int(sys.argv[2]) if len(sys.argv) > 2 else 6
    cli = os.environ.get('XRAY_OFFICE_CLI') or (
        '/c/sandbox/workdir/wt-words-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/'
        'linux-x64/XRay.Office.Cli')
    for d in ('in', 'ref', 'ours'):
        os.makedirs(os.path.join(out, d), exist_ok=True)

    built = []
    n = 0
    for name, kw, expect in list_cases():
        stem = '%02d-%s' % (n, re.sub(r'[^A-Za-z0-9]+', '-', name))
        path = os.path.join(out, 'in', stem + '.docx')
        list_package(path, **kw)
        built.append(('A', name, stem, path, expect))
        n += 1
    for name, fill in shade_cases():
        stem = '%02d-%s' % (n, re.sub(r'[^A-Za-z0-9]+', '-', name))
        path = os.path.join(out, 'in', stem + '.docx')
        shade_package(path, fill)
        built.append(('B', name, stem, path, fill))
        n += 1

    with ThreadPoolExecutor(workers) as pool:
        list(pool.map(lambda t: render_ref(t[3], out, built.index(t) % workers), built))
    for t in built:
        render_ours(cli, t[3], out)

    missing = [f'{t[2]}: no {side}' for t in built for side in ('ref', 'ours')
               if not os.path.exists(os.path.join(out, side, t[2] + '.pdf'))]
    if missing:
        print('REFUSING TO PRINT — %d of %d halves missing:' % (len(missing), 2 * len(built)))
        for m in missing:
            print('   ', m)
        sys.exit(2)
    print('%d packages, %d halves, all present\n' % (len(built), 2 * len(built)))

    print('=== A. the list label\'s slant')
    print(f"{'case':24s} {'ref lean':>8} {'our lean':>8} {'ref flat':>8} {'our flat':>8}  "
          f"{'ref leaning faces':28s} expected")
    for kind, name, stem, path, expect in built:
        if kind != 'A':
            continue
        rl, rf, rfaces = lean_of(os.path.join(out, 'ref', stem + '.pdf'))
        ol, of, _ = lean_of(os.path.join(out, 'ours', stem + '.pdf'))
        fr = ','.join(f'{k}:{v}' for k, v in sorted(rfaces.items())) or '-'
        print(f'{name:24s} {rl:8d} {ol:8d} {rf:8d} {of:8d}  {fr:28s} {expect}')

    print('\n=== B. an automatic font colour on a shaded cell')
    print(f"{'fill':10s} {'reference':28s} {'ours':28s}")
    for kind, name, stem, path, fill in built:
        if kind != 'B':
            continue
        r = colours_of(os.path.join(out, 'ref', stem + '.pdf'))
        o = colours_of(os.path.join(out, 'ours', stem + '.pdf'))
        fmt = lambda d: ','.join(f'{k}:{v}' for k, v in sorted(d.items())) or '-'
        print(f'{fill:10s} {fmt(r):28s} {fmt(o):28s}')
