#!/usr/bin/env python3
"""The 24.2.7.2 audit: does LibreOffice still refuse to break after a hyphen that opens a number?

    python3 audit_hyphenbreak.py <outdir> [workers]

`XRay.Office.Text/Layout/LineBreaker.cs`:473 says, of `MatchNumber`'s leading-sign clause:

    UAX #14 lets a hyphen open a number here as well, so that "-5" holds together.  LibreOffice
    does not, and the difference is visible on any narrow frame: measured against LibreOffice
    24.2.7.2, "E-22", "$-22", "10-19" and a hyphen that begins its own token in "A -222" all
    break *after* the hyphen, while "(222" -- an opening bracket, which this grammar still
    admits -- does not break after the bracket.

That is a claim about a superseded binary, in a **shared layer**: `LineBreaker` decides where
every line of every document on all three tracks ends, so one wrong calibration here is three
tracks' worth of error.  `TODO.24-2-7-audit.md` says to take the shared-layer sites first.

METHOD, AND WHY IT NEEDS NO WIDTH TUNING
----------------------------------------
Each case is one paragraph holding one token **longer than the line**, in a column about six
characters wide.  The token then has to break somewhere, and *where* separates the two rules
without any need to find a width at which one candidate fits and the next does not:

    break after the hyphen permitted   -> the first line is the two characters "E-"
    not permitted                      -> the whole token is broken for want of room, and the
                                          first line is as many characters as fit, "E-2222"

`word/hyphen-word` is the control whose answer is already known: an ordinary hyphenated word
breaks after its hyphen in every implementation of UAX #14, so if *that* comes back unbroken the
instrument is measuring the column width and not the rule.  `bracket/open` is the negative
control the site itself names.

Refuses to print unless every package produced both halves.
"""
import os
import re
import subprocess
import sys
import zipfile
from concurrent.futures import ThreadPoolExecutor

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
PKG_R = 'http://schemas.openxmlformats.org/package/2006/relationships'

# A4 wide, with margins leaving about 60 pt of text -- six characters of 20 pt Liberation Serif.
PAGE_W, MARGIN = 11906, 5350


def package(path, text):
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
              '<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
              '<w:sz w:val="40"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>'
              '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
              '<w:name w:val="Normal"/></w:style></w:styles>')
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document xmlns:w="{W}"><w:body>'
           f'<w:p><w:r><w:t xml:space="preserve">{text}</w:t></w:r></w:p>'
           f'<w:sectPr><w:pgSz w:w="{PAGE_W}" w:h="16838"/>'
           f'<w:pgMar w:top="1440" w:right="{MARGIN}" w:bottom="1440" w:left="{MARGIN}"/>'
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


def cases():
    return [
        ('word/hyphen-word',  'abcd-efghijklmnop', 'CONTROL: breaks after the hyphen'),
        ('number/E-hyphen',   'E-222222222222',    'the site says: breaks after the hyphen'),
        ('number/dollar',     '$-222222222222',    'the site says: breaks after the hyphen'),
        ('number/range',      '10-1922222222222',  'the site says: breaks after the hyphen'),
        ('number/own-token',  'A -222222222222',   'the site says: breaks after the hyphen'),
        ('bracket/open',      '(222222222222222',  'CONTROL: does NOT break after the bracket'),
        # Added after the first run: the first five points are all explained by one rule -- a
        # hyphen opens a number, EXCEPT between two digits, where LibreOffice's own i#83229
        # customisation puts a break back. These four separate that rule from "the class before
        # the hyphen is what matters" and from "a hyphen at a token start is special".
        ('alpha/hyphen-num',  'abc-222222222222',  'AL HY NU: no break after the hyphen'),
        ('digit/hyphen-num',  '5-2222222222222',   'NU HY NU: breaks after the hyphen'),
        ('start/hyphen-num',  '-2222222222222',    'a hyphen opening the line'),
        ('num/hyphen-alpha',  '222-abcdefghijkl',  'NU HY AL: does it break?'),
    ]


def render_ref(src, outdir, slot):
    profile = os.path.join(outdir, f'prof{slot}')
    subprocess.run(
        ['soffice', '-env:UserInstallation=file://' + profile, '--headless', '--norestore',
         '--convert-to', 'pdf', '--outdir', os.path.join(outdir, 'ref'), src],
        capture_output=True, timeout=300)


def render_ours(cli, src, outdir):
    subprocess.run([cli, 'render', src, '--outdir', os.path.join(outdir, 'ours')],
                   capture_output=True, timeout=300)


def lines_of(pdf):
    out = subprocess.run(['pdftotext', '-layout', pdf, '-'],
                         capture_output=True, timeout=120).stdout.decode('utf-8', 'replace')
    return [line.strip() for line in out.splitlines() if line.strip()]


if __name__ == '__main__':
    out = os.path.abspath(sys.argv[1])
    workers = int(sys.argv[2]) if len(sys.argv) > 2 else 6
    cli = os.environ.get('XRAY_OFFICE_CLI') or (
        '/c/sandbox/workdir/wt-words-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/'
        'linux-x64/XRay.Office.Cli')
    for d in ('in', 'ref', 'ours'):
        os.makedirs(os.path.join(out, d), exist_ok=True)

    built = []
    for i, (name, text, expect) in enumerate(cases()):
        stem = '%02d-%s' % (i, re.sub(r'[^A-Za-z0-9]+', '-', name))
        path = os.path.join(out, 'in', stem + '.docx')
        package(path, text)
        built.append((name, text, stem, path, expect))

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

    print(f"{'case':18s} {'token':18s} {'reference lines':34s} {'our lines':34s} expected")
    for name, text, stem, path, expect in built:
        r = lines_of(os.path.join(out, 'ref', stem + '.pdf'))
        o = lines_of(os.path.join(out, 'ours', stem + '.pdf'))
        print(f'{name:18s} {text:18s} {" | ".join(r):34s} {" | ".join(o):34s} {expect}')
