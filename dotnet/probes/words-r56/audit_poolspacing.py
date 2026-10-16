#!/usr/bin/env python3
"""The 24.2.7.2 audit: does `WriterPoolSpacing.Pool` still hold on 26.2.4.2?

    audit_poolspacing.py <outdir> [workers]

`XRay.Office.WordProcessing/Ooxml/WriterPoolSpacing.cs` is one of the eleven open sites in this
project, and it says outright what it is: *"Each row is one rendered probe against LibreOffice
24.2.7.2"*.  It also already carries a warning from a later round -- that lower-case `body text`
measures nought below on 26.2.4.2 where `Body Text` measures 140 -- left alone deliberately because
correcting one row belongs to whichever round re-measures the whole table.  This is that round.

**The reading, and why this shape.**  The table is consumed by
`WordStyles.CompleteOneSidedSpacing`: a style stating only one of `w:spacing/@w:before` and
`@w:after` has the other half frozen at whatever the style resolves to at that point in the import,
and for a style based on a built-in name that is the *parent's* pool row.  So each case is a child
style with a **custom** name -- so nothing but the parent can donate -- based on a parent whose
`w:name` is the built-in one under test, with the parent **declared after the child**, which is the
condition the whole mechanism turns on.  The child states exactly one margin and the other is read
back off `soffice --convert-to fodt`, which is the importer's own answer with no layout in it.

The stated value is **480 twips**, a number that appears in no pool row, so "mirror the stated
value" is refuted by every case rather than assumed away.

Two controls run first and their answers are already known: a parent with a name Writer has never
heard of must read nought on both sides, and `heading 1` must read 240 above and 120 below.  If
either misses, nothing else here is believed.
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

STATED = 480

# Every distinct name `Pool` lists, plus the four `ChildKeeps` mentions and the names the site's
# own prose says measured nought, so a row that has quietly gained or lost an answer is visible.
NAMES = [
    'heading 1', 'Heading 1', 'heading 2', 'Heading 2', 'heading 3', 'Heading 3',
    'heading 4', 'Heading 4', 'heading 5', 'Heading 5', 'heading 6', 'Heading 6',
    'heading 7', 'Heading 7', 'heading 8', 'Heading 8', 'heading 9', 'Heading 9',
    'Title', 'Subtitle', 'caption', 'Caption', 'Body Text', 'body text', 'List',
    'Quote', 'Normal', 'List Paragraph',
]

# What the site claims today, as (above, below) in twips. None means "not in Pool", i.e. (0, 0).
CLAIMED = {n: (240, 120) for n in NAMES if n.lower().startswith('heading ')}
CLAIMED.update({'Title': (240, 120), 'Subtitle': (240, 120),
                'caption': (120, 120), 'Caption': (120, 120),
                'Body Text': (0, 140), 'body text': (0, 140), 'List': (0, 140)})


def package(path, *, parent_name, side):
    """A custom child based on a parent carrying `parent_name`, stating one margin only."""
    spacing = (f'<w:spacing w:before="{STATED}"/>' if side == 'before'
               else f'<w:spacing w:after="{STATED}"/>')
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<w:styles xmlns:w="{W}">'
              '<w:docDefaults><w:rPrDefault><w:rPr/></w:rPrDefault>'
              '<w:pPrDefault><w:pPr/></w:pPrDefault></w:docDefaults>'
              # The child comes first, so the parent's own definition has not been read yet.
              '<w:style w:type="paragraph" w:styleId="Child">'
              '<w:name w:val="Zqxwv Child"/><w:basedOn w:val="Parent"/>'
              f'<w:pPr>{spacing}</w:pPr></w:style>'
              '<w:style w:type="paragraph" w:styleId="Parent">'
              f'<w:name w:val="{parent_name}"/></w:style>'
              '</w:styles>')
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document xmlns:w="{W}"><w:body>'
           '<w:p><w:pPr><w:pStyle w:val="Child"/></w:pPr><w:r><w:t>probe</w:t></w:r></w:p>'
           '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/></w:sectPr></w:body></w:document>')
    ctypes = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
              '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
              'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
              '<Override PartName="/word/document.xml" ContentType="application/vnd.'
              'openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
              '<Override PartName="/word/styles.xml" ContentType="application/vnd.'
              'openxmlformats-officedocument.wordprocessingml.styles+xml"/></Types>')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ctypes)
        z.writestr('_rels/.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}"><Relationship Id="rId1" Type="{R}'
                   '/officeDocument" Target="word/document.xml"/></Relationships>')
        z.writestr('word/_rels/document.xml.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}"><Relationship Id="rId8" Type="{R}'
                   '/styles" Target="styles.xml"/></Relationships>')
        z.writestr('word/document.xml', doc)
        z.writestr('word/styles.xml', styles)


def convert(docx, outdir, slot):
    """Convert, and retry once -- the first conversion into a fresh profile can come back empty."""
    target = os.path.join(
        outdir, 'out', os.path.basename(docx)[:-len('.docx')] + '.fodt')
    for _ in range(3):
        subprocess.run(
            ['soffice', '-env:UserInstallation=file://' + os.path.join(outdir, f'prof{slot}'),
             '--headless', '--norestore', '--convert-to', 'fodt',
             '--outdir', os.path.join(outdir, 'out'), docx],
            capture_output=True, timeout=300)
        if os.path.exists(target):
            return


STYLE = re.compile(r'<style:style[^>]*style:name="Zqxwv_20_Child".*?</style:style>', re.S)
TOP = re.compile(r'fo:margin-top="([-\d.]+)(in|cm|mm|pt)"')
BOTTOM = re.compile(r'fo:margin-bottom="([-\d.]+)(in|cm|mm|pt)"')

UNITS = {'in': 1440.0, 'cm': 1440.0 / 2.54, 'mm': 144.0 / 2.54, 'pt': 20.0}


def twips(match):
    return None if match is None else round(float(match.group(1)) * UNITS[match.group(2)])


def read(path):
    if not os.path.exists(path):
        return None, None
    blob = open(path, encoding='utf-8', errors='replace').read()
    m = STYLE.search(blob)
    if not m:
        return None, None
    body = m.group(0)
    return twips(TOP.search(body)), twips(BOTTOM.search(body))


if __name__ == '__main__':
    out = os.path.abspath(sys.argv[1])
    workers = int(sys.argv[2]) if len(sys.argv) > 2 else 3
    os.makedirs(os.path.join(out, 'in'), exist_ok=True)
    os.makedirs(os.path.join(out, 'out'), exist_ok=True)

    jobs = []
    controls = [('Zqxwv Nonesuch Parent', (0, 0)), ('heading 1', (240, 120))]
    order = [n for n, _ in controls] + [n for n in NAMES if n not in dict(controls)]

    for i, name in enumerate(order):
        for side in ('before', 'after'):
            # Case is part of the case: `heading 5` and `Heading 5` are two different rows, and
            # this mount is case-insensitive -- naming the two packages `heading-5` and
            # `Heading-5` makes them the same file and the same output. The first run of this
            # probe lost 28 of 58 conversions that way and reported nine rows "DISAGREES" that
            # were nothing but a missing output file read as nought. Numbered instead.
            safe = f'{i:02d}-{re.sub(r"[^a-z0-9]+", "-", name.lower()).strip("-")}-{side}'
            path = os.path.join(out, 'in', safe + '.docx')
            package(path, parent_name=name, side=side)
            jobs.append((name, side, safe, path, i))

    with ThreadPoolExecutor(workers) as pool:
        list(pool.map(lambda t: convert(t[3], out, t[4] % workers), jobs))

    # Assert the instrument produced output before reading a single number out of it. Half of
    # this probe's first run was missing files, and a missing file reads as nought, which reads
    # as a finding.
    lost = [t[2] for t in jobs if not os.path.exists(os.path.join(out, 'out', t[2] + '.fodt'))]
    if lost:
        print(f'!! {len(lost)} of {len(jobs)} conversions produced no output: '
              f'{", ".join(lost[:8])}{" …" if len(lost) > 8 else ""}')
        sys.exit(2)

    measured = {}
    for name, side, safe, path, _ in jobs:
        top, bottom = read(os.path.join(out, 'out', safe + '.fodt'))
        measured.setdefault(name, {})[side] = (top, bottom)

    print(f"{'parent w:name':18s} {'stated':>8} {'filled above':>13} {'filled below':>13} "
          f"{'site claims':>13}  verdict")
    bad = 0
    for name in order:
        # The child states `before`, so the *filled* half is `below`, and the other way round.
        below = measured[name]['before'][1]
        above = measured[name]['after'][0]
        claim = CLAIMED.get(name, (0, 0)) if name in NAMES else (0, 0)
        got = (above if above is not None else 0, below if below is not None else 0)
        ok = got == claim
        if name in NAMES and not ok:
            bad += 1
        mark = 'agrees' if ok else '*** DISAGREES ***'
        if name not in NAMES:
            mark = f'control, expected {claim}' + ('' if ok else '  *** CONTROL FAILED ***')
        print(f'{name:18s} {STATED:8d} {got[0]:13d} {got[1]:13d} {str(claim):>13}  {mark}')
    print(f'\nrows of the site\'s table that disagree on 26.2.4.2: {bad} of {len(NAMES)}')
