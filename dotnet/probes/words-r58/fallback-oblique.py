#!/usr/bin/env python3
"""Does a run whose glyph comes from a *fallback* face keep its lean?

    python3 fallback-oblique.py <outdir> [workers]

Round 56 fixed synthetic oblique at the four word-processing readers and left 1 611 sheared
glyphs short on 39 documents, of which 289 sit in faces **no document names** — WenQuanYi Zen
Hei 177, OpenSymbol 112.  Those faces arrive through `FontItemiser.Split`, and the reference
naming them is built by `SystemFontResolver.ReferenceFor`, a *reverse lookup from a face with
no request to compare against*.  It therefore cannot set `SyntheticOblique` and never has.

WHAT THIS VARIES, AND WHAT IT HOLDS FIXED
-----------------------------------------
`SystemFontResolver.GenericFallbacks` was recorded WRONG by round 53 and VERIFIED by round 54,
because round 53's probe was DOCX-only: it held the **document format** fixed without noticing
that the format was the variable.  So the format is a *varied* axis here, over four filters
that reach the shared resolver by four different routes and cover all three tracks:

    .docx   OOXML word processing      words
    .fodt   ODF word processing        words
    .fodp   ODF presentation           slides
    .fods   ODF spreadsheet            sheets

Held fixed, deliberately, and each stated so it can be attacked:

  * **The installed font set** (35 files).  Both stacks see the same one, so it cannot make the
    two sides differ — but it *does* decide which faces have italics at all, and every claim
    below is a claim about *this* machine.  The families with no italic here are DejaVu Sans,
    DejaVu Serif, OpenSymbol, WenQuanYi Zen Hei and IPA Gothic; the ones with an italic are
    Liberation Sans/Serif/Mono, Carlito, Caladea and DejaVu Sans Mono.
  * **The point size** (20 pt).  A shear is scale-free in the text matrix, so this cannot be
    the answer; it is fixed only to keep the counts readable.
  * **The weight** — except in `cjk-bold-italic`, which varies it on purpose, because a bold
    italic request is the case where the fallback search has two attributes to satisfy and
    might satisfy the wrong one.

CASES, AND WHY EACH IS HERE
---------------------------
Every case is one paragraph of two runs whose only difference is the thing named, so the second
run's sheared-glyph count is the only quantity that can move.

  latin-italic      control with a known answer: italic Latin in Arial resolves to
                    LiberationSans-Italic, a real italic, so **nothing shears** on either side.
                    Separates "we never shear" from "we lose it at the fallback".
  cjk-upright       control with a known answer: the same fallback face, no italic asked for,
                    so **nothing shears** on either side.  Separates "the fallback face always
                    leans" from the claim.
  cjk-italic        THE CLAIM.  Italic Arial + CJK: the primary face is a real italic, the
                    fallback face has none.
  sym-italic        the same claim through a different fallback face (OpenSymbol / DejaVu Sans)
                    and a different script class, so a positive result is not a fact about CJK.
  cjk-italic-none   italic in a family nobody has installed, so the *primary* is already
                    synthetic-oblique and the fallback has to inherit a lean the primary only
                    has synthetically.  This is the arm our fix cannot reconstruct from
                    `face.IsItalic` alone.
  cjk-bold-italic   weight and slant together.
  cjk-italic-eastasia  (docx only) the eastAsia slot names the same missing family, which is
                    how the corpus witnesses are actually written.

The probe REFUSES TO PRINT unless every package produced both a reference and an ours PDF.
A missing input read as zero reads as a finding — round 56 lost 28 of 58 conversions that way.
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

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
PKG_R = 'http://schemas.openxmlformats.org/package/2006/relationships'

PLAIN = 'Upright aaaa '
CJK = '手机免提系统'          # 6, in WQY / IPA Gothic only
SYM = '☐☒➢✦'                       # 4, in DejaVu Sans / OpenSymbol only
HEB = 'אבגד'                       # 4, in Liberation Sans (italic installed) and DejaVu Sans (none)
LATIN = 'Slanted bbbb'
NONESUCH = 'Zqxwv Nonesuch'

# ------------------------------------------------------------------ the case table

def cases():
    """(name, family, text, italic, bold, eastasia, expected, ics)

    `ics` states `w:iCs` beside `w:i` in the DOCX arm.  It is needed for the Hebrew cases and
    for nothing else: OOXML files right-to-left text under the *complex script* slot, and
    round 56 established that `w:i` does not lean it — so a Hebrew DOCX case without `w:iCs`
    measures the reader's slot rule, not the resolver's.  The flat-ODF builders already set
    `style:font-style-complex` alongside `fo:font-style`, so they need no equivalent.
    """
    return [
        ('latin-italic',       'Arial',  LATIN, True,  False, False, 'nothing shears', False),
        ('cjk-upright',        'Arial',  CJK,   False, False, False, 'nothing shears', False),
        ('cjk-italic',         'Arial',  CJK,   True,  False, False, 'shears run 2 (the claim)', False),
        ('sym-italic',         'Arial',  SYM,   True,  False, False, 'shears run 2 (the claim)', False),
        ('cjk-italic-none',    NONESUCH, CJK,   True,  False, False, 'shears run 2', False),
        ('cjk-bold-italic',    'Arial',  CJK,   True,  True,  False, 'shears run 2', False),
        ('cjk-italic-eastasia','Arial',  CJK,   True,  False, True,  'shears run 2 (docx only)', False),
        # --- the face-choice discriminator, added after the first run came back clean.
        # OpenSymbol is installed and has no italic, so the primary is already synthetic-oblique;
        # Hebrew is missing from it and is covered by BOTH DejaVu Sans (no italic) and Liberation
        # Sans (italic installed).  If the reference prefers the italic-bearing family it draws
        # LiberationSans-Italic UPRIGHT; if it prefers its own fallback order it draws DejaVu Sans
        # SHEARED.  Setting SyntheticOblique on a fallback reference is only right in the second
        # case, so this is the one case that can refute the change rather than confirm it.
        ('heb-italic-opensym', 'OpenSymbol', HEB, True,  False, False, 'DISCRIMINATOR', True),
        ('heb-upright-opensym','OpenSymbol', HEB, False, False, False, 'nothing shears', True),
        # The same discriminator with a primary that is a REAL italic rather than a synthetic
        # one: Carlito has an italic installed and no Hebrew, so the request reaching fallback
        # is unambiguously italic and the choice between DejaVu Sans (no italic, must be
        # sheared) and Liberation Sans (italic installed, must not be) is forced.
        ('heb-italic-carlito', 'Carlito',    HEB, True,  False, False, 'DISCRIMINATOR', True),
        ('heb-upright-carlito','Carlito',    HEB, False, False, False, 'nothing shears', True),
        # --- round 56's fix, as a regression control: a family with no italic anywhere.
        ('latin-italic-none',  NONESUCH, LATIN, True,  False, False, 'shears run 2 (round 56)', False),
    ]


# ------------------------------------------------------------------ DOCX

def docx(path, *, family, text, italic, bold, eastasia, ics):
    rpr = ''
    if eastasia:
        rpr += f'<w:rFonts w:eastAsia="{family}"/>'
    if bold:
        rpr += '<w:b/><w:bCs/>' if ics else '<w:b/>'
    if italic:
        rpr += '<w:i/><w:iCs/>' if ics else '<w:i/>'
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
              f'<w:rFonts w:ascii="{family}" w:hAnsi="{family}" w:eastAsia="{family}" '
              f'w:cs="{family}"/><w:sz w:val="40"/><w:szCs w:val="40"/>'
              '</w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>'
              '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
              '<w:name w:val="Normal"/></w:style></w:styles>')
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document xmlns:w="{W}"><w:body><w:p>'
           f'<w:r><w:t xml:space="preserve">{PLAIN}</w:t></w:r>'
           f'<w:r><w:rPr>{rpr}</w:rPr><w:t xml:space="preserve">{text}</w:t></w:r>'
           '</w:p><w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
           '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>'
           '</w:sectPr></w:body></w:document>')
    parts = [('document.xml', 'document.main'), ('styles.xml', 'styles')]
    over = ''.join(
        f'<Override PartName="/word/{n}" ContentType="application/vnd.openxmlformats-'
        f'officedocument.wordprocessingml.{k}+xml"/>' for n, k in parts)
    ctypes = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
              '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
              'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
              + over + '</Types>')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ctypes)
        z.writestr('_rels/.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}"><Relationship Id="rId1" Type="{R}'
                   '/officeDocument" Target="word/document.xml"/></Relationships>')
        z.writestr('word/_rels/document.xml.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}">'
                   f'<Relationship Id="rId8" Type="{R}/styles" Target="styles.xml"/>'
                   '</Relationships>')
        z.writestr('word/document.xml', doc)
        z.writestr('word/styles.xml', styles)


# ------------------------------------------------------------------ flat ODF

NS = ('xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" '
      'xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" '
      'xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" '
      'xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" '
      'xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" '
      'xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" '
      'xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" '
      'xmlns:presentation="urn:oasis:names:tc:opendocument:xmlns:presentation:1.0"')


def _text_props(family, italic, bold):
    slant = 'italic' if italic else 'normal'
    weight = 'bold' if bold else 'normal'
    return (f'<style:text-properties style:font-name="{family}" '
            f'style:font-name-asian="{family}" style:font-name-complex="{family}" '
            f'fo:font-size="20pt" style:font-size-asian="20pt" style:font-size-complex="20pt" '
            f'fo:font-style="{slant}" style:font-style-asian="{slant}" '
            f'style:font-style-complex="{slant}" '
            f'fo:font-weight="{weight}" style:font-weight-asian="{weight}" '
            f'style:font-weight-complex="{weight}"/>')


def _faces(family):
    return (f'<office:font-face-decls><style:font-face style:name="{family}" '
            f'svg:font-family="&apos;{family}&apos;"/></office:font-face-decls>')


def _autostyles(family, italic, bold, extra=''):
    return ('<office:automatic-styles>'
            '<style:style style:name="Tplain" style:family="text">'
            + _text_props(family, False, False) + '</style:style>'
            '<style:style style:name="Tlean" style:family="text">'
            + _text_props(family, italic, bold) + '</style:style>'
            + extra + '</office:automatic-styles>')


def _spans(text):
    return (f'<text:span text:style-name="Tplain">{PLAIN}</text:span>'
            f'<text:span text:style-name="Tlean">{text}</text:span>')


def fodt(path, *, family, text, italic, bold, eastasia, ics):
    body = f'<office:body><office:text><text:p>{_spans(text)}</text:p></office:text></office:body>'
    xml = ('<?xml version="1.0" encoding="UTF-8"?>'
           f'<office:document {NS} office:version="1.3" '
           'office:mimetype="application/vnd.oasis.opendocument.text">'
           + _faces(family) + _autostyles(family, italic, bold) + body + '</office:document>')
    open(path, 'w', encoding='utf-8').write(xml)


def fods(path, *, family, text, italic, bold, eastasia, ics):
    body = ('<office:body><office:spreadsheet>'
            '<table:table table:name="S1">'
            '<table:table-column/>'
            '<table:table-row><table:table-cell office:value-type="string">'
            f'<text:p>{_spans(text)}</text:p>'
            '</table:table-cell></table:table-row>'
            '</table:table></office:spreadsheet></office:body>')
    xml = ('<?xml version="1.0" encoding="UTF-8"?>'
           f'<office:document {NS} office:version="1.3" '
           'office:mimetype="application/vnd.oasis.opendocument.spreadsheet">'
           + _faces(family) + _autostyles(family, italic, bold) + body + '</office:document>')
    open(path, 'w', encoding='utf-8').write(xml)


def fodp(path, *, family, text, italic, bold, eastasia, ics):
    extra = ('<style:style style:name="gr1" style:family="graphic">'
             '<style:graphic-properties draw:fill="none" draw:stroke="none"/></style:style>')
    body = ('<office:body><office:presentation>'
            '<draw:page draw:name="page1">'
            '<draw:frame draw:style-name="gr1" svg:width="20cm" svg:height="4cm" '
            'svg:x="1cm" svg:y="2cm"><draw:text-box>'
            f'<text:p>{_spans(text)}</text:p>'
            '</draw:text-box></draw:frame>'
            '</draw:page></office:presentation></office:body>')
    xml = ('<?xml version="1.0" encoding="UTF-8"?>'
           f'<office:document {NS} office:version="1.3" '
           'office:mimetype="application/vnd.oasis.opendocument.presentation">'
           + _faces(family) + _autostyles(family, italic, bold, extra) + body + '</office:document>')
    open(path, 'w', encoding='utf-8').write(xml)


BUILDERS = [('docx', docx), ('fodt', fodt), ('fodp', fodp), ('fods', fods)]


# ------------------------------------------------------------------ rendering

def render_ref(src, outdir, slot):
    profile = os.path.join(outdir, f'prof{slot}')
    subprocess.run(
        ['soffice', '-env:UserInstallation=file://' + profile, '--headless', '--norestore',
         '--convert-to', 'pdf', '--outdir', os.path.join(outdir, 'ref'), src],
        capture_output=True, timeout=300)


def render_ours(cli, src, outdir):
    subprocess.run([cli, 'render', src, '--outdir', os.path.join(outdir, 'ours')],
                   capture_output=True, timeout=300)


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
    for name, family, text, italic, bold, eastasia, expect, ics in cases():
        for ext, build in BUILDERS:
            if eastasia and ext != 'docx':
                continue
            # Numbered, so two packages can never collide on a case-insensitive mount.
            stem = '%02d-%s-%s' % (n, re.sub(r'[^A-Za-z0-9]+', '-', name), ext)
            path = os.path.join(out, 'in', stem + '.' + ext)
            build(path, family=family, text=text, italic=italic, bold=bold,
                  eastasia=eastasia, ics=ics)
            built.append((name, ext, stem, path, expect))
            n += 1

    with ThreadPoolExecutor(workers) as pool:
        list(pool.map(lambda t: render_ref(t[3], out, built.index(t) % workers), built))
    for t in built:
        render_ours(cli, t[3], out)

    # --- refuse to print unless every package produced both halves
    missing = []
    for name, ext, stem, path, expect in built:
        for side in ('ref', 'ours'):
            if not os.path.exists(os.path.join(out, side, stem + '.pdf')):
                missing.append(f'{stem}: no {side}')
    if missing:
        print('REFUSING TO PRINT — %d of %d halves missing:' % (len(missing), 2 * len(built)))
        for m in missing:
            print('   ', m)
        sys.exit(2)

    print('%d packages, %d halves, all present\n' % (len(built), 2 * len(built)))
    print(f"{'case':22s} {'fmt':5s} {'ref lean':>8} {'our lean':>8} {'ref flat':>8} "
          f"{'our flat':>8}  {'ref leaning faces':34s} {'our leaning faces':34s} expected")
    for name, ext, stem, path, expect in built:
        rl, rf, rfaces = lean_of(os.path.join(out, 'ref', stem + '.pdf'))
        ol, of, ofaces = lean_of(os.path.join(out, 'ours', stem + '.pdf'))
        fr = ','.join(f'{k}:{v}' for k, v in sorted(rfaces.items())) or '-'
        fo = ','.join(f'{k}:{v}' for k, v in sorted(ofaces.items())) or '-'
        print(f'{name:22s} {ext:5s} {rl:8d} {ol:8d} {rf:8d} {of:8d}  {fr:34s} {fo:34s} {expect}')
