"""RETRACTED — the corpus case this was built for is reference nondeterminism.

Read this first
---------------

This probe was written to characterise why LibreOffice draws nothing for cell B16
of `sheets/unstable-001/xlsx/fse_identification_form.xlsx`. **There is no such
rule.** LibreOffice's rendering of that document is not a function of its input:
rendering the untouched file eighteen times through `soffice --convert-to pdf`
drew B16 **once** and omitted it seventeen times, at 443 and 430 extracted words
respectively. Our own renderer is byte-identical across five runs.

Every "the reference draws it / does not" result in the investigation that led
here — dropping the border, setting `vertical="top"`, raising the row height,
removing `wrapText`, shortening the text — was a single render, and therefore a
single sample of a coin flip. Two commits stated mechanisms built on those
samples; both are wrong, and the correction is in the history beside them.

The document is in `unstable-001` and that classification was right all along.

What survives
-------------

Only the measurements taken on OUR renderer, which is deterministic, and the two
observations that do not depend on the reference being stable:

- the difference is exactly cell B16 and nothing else — the reference-only token
  set is empty in every run;
- our output is stable and self-consistent, and matches the reference's *minority*
  outcome, which is the one where the cell's text is drawn. The cell does hold
  that text, so drawing it is not obviously the wrong answer.

The lesson, which is the reason this file is kept
-------------------------------------------------

**When the reference is a black box, one render is one sample.** An ablation
table built from single runs looks exactly like a mechanism and can be entirely
noise — it produced a clean, plausible, four-condition rule here that survived
several rounds of reasoning before a repeat measurement dissolved it. Repeat any
ablation against `soffice` at least a handful of times before believing a row of
it, and repeat it *first* on the unmutated file to establish whether the document
is stable at all.

What the probe itself still shows
----------------------------------

The synthetic below never reproduced the corpus behaviour, at any of its
thirty-two combinations of text length, row height and border. That is now
explained: there was nothing to reproduce. Its `customHeight` handling is sound
and was verified — measured row pitches of 14.43 / 29.00 / 43.48 against the
declared 14.45 / 29 / 43.5 — so it remains usable as a check that wrapped text in
a fixed-height row is drawn by both engines, which it is.

Usage
-----

    XRAY_OFFICE_CLI=... python3 merged-wrap-fixed-height.py /abs/workdir
"""

import os
import subprocess
import sys
import zipfile

# One line of 11 pt Calibri is about 14.4 pt; the merged B:E band is wide enough
# that these lengths wrap to roughly one, two, three and five lines.
LENGTHS = [30, 90, 170, 300]
HEIGHTS = [None, 14.45, 29.0, 43.5]

CONTENT_TYPES = (
    '<?xml version="1.0"?>'
    '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
    '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
    'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
    '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-'
    'officedocument.spreadsheetml.sheet.main+xml"/>'
    '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.'
    'openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
    '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-'
    'officedocument.spreadsheetml.styles+xml"/></Types>')

ROOT_RELS = (
    '<?xml version="1.0"?><Relationships xmlns="http://schemas.openxmlformats.org/'
    'package/2006/relationships"><Relationship Id="rId1" Type="http://schemas.'
    'openxmlformats.org/officeDocument/2006/relationships/officeDocument" '
    'Target="xl/workbook.xml"/></Relationships>')

WORKBOOK_RELS = (
    '<?xml version="1.0"?><Relationships xmlns="http://schemas.openxmlformats.org/'
    'package/2006/relationships"><Relationship Id="rId1" Type="http://schemas.'
    'openxmlformats.org/officeDocument/2006/relationships/worksheet" '
    'Target="worksheets/sheet1.xml"/><Relationship Id="rId2" Type="http://schemas.'
    'openxmlformats.org/officeDocument/2006/relationships/styles" '
    'Target="styles.xml"/></Relationships>')

WORKBOOK = (
    '<?xml version="1.0"?><workbook xmlns="http://schemas.openxmlformats.org/'
    'spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/'
    'officeDocument/2006/relationships"><sheets><sheet name="S" sheetId="1" '
    'r:id="rId1"/></sheets></workbook>')

# Style 1 is the one under test: wrapped and left-aligned, like the corpus cell's.
STYLES = (
    '<?xml version="1.0"?><styleSheet xmlns="http://schemas.openxmlformats.org/'
    'spreadsheetml/2006/main">'
    '<fonts count="1"><font><sz val="11"/><name val="Calibri"/></font></fonts>'
    '<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
    '<borders count="2"><border/>'
    '<border><left style="thin"><color rgb="FF000000"/></left>'
    '<right style="thin"><color rgb="FF000000"/></right>'
    '<top style="thin"><color rgb="FF000000"/></top>'
    '<bottom style="thin"><color rgb="FF000000"/></bottom></border></borders>'
    '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>'
    '<cellXfs count="3"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>'
    '<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyAlignment="1">'
    '<alignment horizontal="left" wrapText="1"/></xf>'
    '<xf numFmtId="0" fontId="0" fillId="0" borderId="1" xfId="0" applyBorder="1" '
    'applyAlignment="1"><alignment horizontal="left" wrapText="1"/></xf></cellXfs>'
    '</styleSheet>')


def words(length, tag):
    """Words unique to this case, so the count cannot run into a neighbouring row."""
    out = []
    index = 0
    while len(' '.join(out)) < length:
        out.append(f'{tag}x{index}')
        index += 1
    return ' '.join(out)


def build(path, cases):
    rows, merges = '', ''

    for index, (length, height, bordered, tag) in enumerate(cases):
        r = index + 1
        ht = f' ht="{height}" customHeight="1"' if height is not None else ''
        rows += (
            f'<row r="{r}" spans="1:5"{ht}>'
            f'<c r="A{r}" t="inlineStr"><is><t>row{r}</t></is></c>'
            f'<c r="B{r}" s="{2 if bordered else 1}" t="inlineStr">'
            f'<is><t>{words(length, tag)}</t></is></c>'
            '</row>')
        merges += f'<mergeCell ref="B{r}:E{r}"/>'

    sheet = (
        '<?xml version="1.0"?><worksheet xmlns="http://schemas.openxmlformats.org/'
        'spreadsheetml/2006/main">'
        f'<dimension ref="A1:E{len(cases)}"/>'
        '<cols><col min="1" max="1" width="12" customWidth="1"/>'
        '<col min="2" max="5" width="20" customWidth="1"/></cols>'
        f'<sheetData>{rows}</sheetData><mergeCells count="{len(cases)}">{merges}</mergeCells>'
        '</worksheet>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as package:
        package.writestr('[Content_Types].xml', CONTENT_TYPES)
        package.writestr('_rels/.rels', ROOT_RELS)
        package.writestr('xl/_rels/workbook.xml.rels', WORKBOOK_RELS)
        package.writestr('xl/workbook.xml', WORKBOOK)
        package.writestr('xl/styles.xml', STYLES)
        package.writestr('xl/worksheets/sheet1.xml', sheet)


def drawn(pdf, tag):
    """How many of this case's own words reached the page — they appear nowhere else."""
    text = subprocess.run(['pdftotext', pdf, '-'], capture_output=True, text=True,
                          check=True).stdout
    return len({w for w in text.split() if w.startswith(tag + 'x')})


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/merged-wrap'
    cli = os.environ.get('XRAY_OFFICE_CLI')
    if not cli:
        raise SystemExit('set XRAY_OFFICE_CLI to the tree you mean to measure')

    os.makedirs(os.path.join(out, 'ours'), exist_ok=True)

    cases = []
    for bordered in (False, True):
        for length in LENGTHS:
            for height in HEIGHTS:
                tag = ('B' if bordered else 'N') + f'C{length}H{"A" if height is None else int(height)}'
                cases.append((length, height, bordered, tag))

    xlsx = os.path.join(out, 'wrap.xlsx')
    build(xlsx, cases)

    subprocess.run(['soffice', '--headless', '--convert-to', 'pdf', '--outdir', out, xlsx],
                   capture_output=True, check=True)
    subprocess.run([cli, 'render', '--quiet', '--outdir', os.path.join(out, 'ours'), xlsx],
                   check=True)

    reference = os.path.join(out, 'wrap.pdf')
    ours = os.path.join(out, 'ours', 'wrap.pdf')
    for produced in (reference, ours):
        if not os.path.isfile(produced):
            raise SystemExit(f'{produced} was not written — nothing to compare')

    print(f"{'border':>7s} {'chars':>6s} {'height':>7s} {'total':>6s} {'ref':>6s} {'ours':>6s}  agree?")

    for length, height, bordered, tag in cases:
        r, o = drawn(reference, tag), drawn(ours, tag)
        total = len(words(length, tag).split())
        name = 'auto' if height is None else f'{height:g}'
        print(f'{"yes" if bordered else "no":>7s} {length:6d} {name:>7s} {total:6d} '
              f'{r:6d} {o:6d}  {"yes" if r == o else "NO"}')

    print('\nthe auto-height rows are the control and must always agree.')
    print('a reference count of 0 where ours is not is the corpus case.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
