#!/usr/bin/env python3
"""Test the one sub-unity line-spacing prediction that has no free parameters.

Why this exists
---------------

`subunity-line-spacing.py` measured the *pitch* — baseline to baseline — at
twenty-one values of `w:line` and could not be turned into a rule. Fitting is
provably exhausted there: successive differences swing by 3 twips where rounding
any smooth function of a single scale can only swing by 1, and solving the
obvious composed model for a per-point percentage gives a unique answer for all
twenty-one points, which is twenty-one parameters against twenty-one points and
therefore evidence of nothing.

`SwTextFormatter::CalcRealHeight` (`sw/source/core/text/itrform2.cxx:2367-2394`)
carries a second statement that pitch cannot see:

    nLineHeight = nTmp;
    SwTwips nAsc = (4 * nLineHeight) / 5;    // ascent forced to 80%
    m_pCurr->SetAscent( nAsc );

**The ascent is forced to exactly four fifths of the shrunk height rather than
scaled with it.** That is a prediction with no fitted quantity in it, and it is
measurable: the first baseline on a page sits one ascent below the top margin,
so as the line height changes from page to page the first line's ink must move
down the page at a slope of exactly **0.8** against it.

The two hypotheses separate cleanly, and neither is a fit:

- **the 80% rule is live** — ink top moves at slope 0.8 against the height
- **the ascent is the font's own and untouched** — ink top does not move at all

A third outcome, some other slope, says the rule is live but `nLineHeight` is
not the quantity the pitch reports, which is the standing open question.

This also measures the reference and ours side by side, so it says whether the
80% rule is something we already reproduce by accident or something missing.

What it found, 2026-08-15
-------------------------

    least squares over the 20 sub-unity pages, ink top against line height
      reference slope   0.8030
      our slope         0.0000

**The reference shrinks the ascent and we do not touch it.** Our first baseline
sits at exactly the same place on all twenty-one pages — the font's own ascent,
whatever the spacing says.

Taking the reference's *measured* line height on each page and predicting the
shrink as `205 - (4 * H) / 5` in truncating integer arithmetic, where 205 twips
is the 100% ascent, is **exact on all twenty of them — residual 0.000 pt
everywhere**:

    H tw   4H/5   predicted   measured      H tw   4H/5   predicted   measured
     126    100       5.250      5.250       189    151       2.700      2.700
     131    104       5.050      5.050       197    157       2.400      2.400
     139    111       4.700      4.700       202    161       2.200      2.200
     144    115       4.500      4.500       209    167       1.900      1.900
     151    120       4.250      4.250       215    172       1.650      1.650
     159    127       3.900      3.900       220    176       1.450      1.450
     164    131       3.700      3.700       227    181       1.200      1.200
     172    137       3.400      3.400       235    188       0.850      0.850
     177    141       3.200      3.200       240    192       0.650      0.650
     184    147       2.900      2.900       247    197       0.400      0.400

This is the opposite of the pitch probe's situation and the contrast is the
point. There, twenty-one points needed twenty-one fitted percentages. Here the
shape of the model has **no** free parameter — `4H/5` is read straight off the
source — and the single constant is the 100% ascent, which is a font metric
rather than something tuned. Twenty points, one parameter, zero residual.

So `SwTextFormatter::CalcRealHeight`'s `nAsc = (4 * nLineHeight) / 5` is
**confirmed live**, with truncation, and it is a defect we have. It is also
independent of the open question about what `nLineHeight` holds: the prediction
above consumes the reference's own measured height, so it stands whatever that
turns out to be.

The instrument
--------------

One `w:line` value per page, exactly as the pitch probe does, and for the same
reason: the first two cuts of that probe grouped values on one page and split
them by looking for a vertical gap, which silently dropped groups once the
content outgrew the page. A page break is unambiguous and costs nothing.

`pdftotext -bbox` reports a **font-descriptor ink box, not a baseline** (see the
`page-vision` skill — a flat 2.1 pt Caladea offset turned out to be
`usWinAscent - sTypoAscent` with identical `Td` baselines). That is harmless
here because the face, the size and the text are identical on every page, so the
ink-to-baseline offset is one constant that cancels out of every *difference*
between pages. Only slopes and deltas below are trustworthy; the absolute
intercept is not, and is not used.

Usage
-----

    XRAY_OFFICE_CLI=... python3 ascent-eighty-percent.py /abs/workdir
"""

import os
import re
import subprocess
import sys
import zipfile

LINES = [120, 126, 132, 138, 144, 150, 156, 162, 168, 174,
         180, 186, 192, 198, 204, 210, 216, 222, 228, 234, 240]
FACE = 'Liberation Serif'
HALF_POINTS = 22
TOP_MARGIN_PT = 567 / 20.0

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-'
      'officedocument.wordprocessingml.document.main+xml"/></Types>')
RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
        'relationships/officeDocument" Target="word/document.xml"/></Relationships>')
W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'


def build(path):
    """One `w:line` value per page, eleven identical lines each."""
    body, labels = '', []
    run = f'<w:rFonts w:ascii="{FACE}" w:hAnsi="{FACE}"/><w:sz w:val="{HALF_POINTS}"/>'

    for index, line in enumerate(LINES):
        # The break goes on the group's OWN first paragraph, not on a separate
        # empty one. A separate empty paragraph carries the document default
        # spacing and so adds a full extra line above the ink on every page but
        # the first — which made page 1 alone incomparable and produced a
        # spurious 13.65 pt jump the first time this was run.
        for j in range(11):
            brk = '<w:pageBreakBefore/>' if index and not j else ''
            body += (
                f'<w:p><w:pPr>{brk}'
                f'<w:spacing w:line="{line}" w:lineRule="auto" w:before="0" w:after="0"/>'
                f'<w:rPr>{run}</w:rPr></w:pPr><w:r><w:rPr>{run}</w:rPr>'
                f'<w:t>L{j}</w:t></w:r></w:p>')
        labels.append((line, line / 240 * 100))

    document = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>{body}'
        '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="567" w:right="567" w:bottom="567" w:left="567"/></w:sectPr>'
        '</w:body></w:document>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml', document)
    return labels


def tops(pdf, page):
    """Every distinct line-ink top on one page, in points from the page top."""
    text = subprocess.run(['pdftotext', '-bbox', '-f', str(page), '-l', str(page), pdf, '-'],
                          capture_output=True, text=True, check=True).stdout
    found = sorted({round(float(m.group(1)), 3)
                    for m in re.finditer(r'yMin="([\d.]+)"', text)})
    if len(found) < 11:
        raise SystemExit(f'{pdf} page {page}: {len(found)} lines, expected 11')
    return found


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/ascent-eighty'
    cli = os.environ.get('XRAY_OFFICE_CLI')
    if not cli:
        raise SystemExit('set XRAY_OFFICE_CLI to the tree you mean to measure')

    os.makedirs(os.path.join(out, 'ours'), exist_ok=True)
    docx = os.path.join(out, 'asc.docx')
    labels = build(docx)

    subprocess.run(['soffice', '--headless', '--convert-to', 'pdf', '--outdir', out, docx],
                   capture_output=True, check=True)
    subprocess.run([cli, 'render', '--quiet', '--outdir', os.path.join(out, 'ours'), docx],
                   check=True)

    reference, ours = os.path.join(out, 'asc.pdf'), os.path.join(out, 'ours', 'asc.pdf')
    for path in (reference, ours):
        if not os.path.isfile(path):
            raise SystemExit(f'{path} was not written — nothing to compare')

    print(f"{'w:line':>6} {'pct':>6} | {'ref h':>7} {'ref top':>8} | "
          f"{'our h':>7} {'our top':>8} | {'Δtop':>7}")

    rows = []
    for page, (line, pct) in enumerate(labels, start=1):
        r, o = tops(reference, page), tops(ours, page)
        rh, oh = (r[-1] - r[0]) / 10 * 20, (o[-1] - o[0]) / 10 * 20
        rows.append((line, pct, rh, r[0], oh, o[0]))
        print(f'{line:6d} {pct:6.2f} | {rh:7.1f} {r[0]:8.3f} | '
              f'{oh:7.1f} {o[0]:8.3f} | {o[0] - r[0]:+7.3f}')

    print()
    # Regress ink top on line height across the SUB-UNITY pages only. The 100%
    # page must be excluded: it is the one page that does not take the shrink
    # branch at all, so its ascent is the font's own and it does not lie on the
    # line the other twenty do. Using it as the baseline is what made the first
    # run of this probe report ~0.83 instead of 0.80.
    sub = [r for r in rows if r[1] < 100.0]
    full = next(r for r in rows if r[1] == 100.0)

    def slope(points, h_index, top_index):
        n = len(points)
        xs = [p[h_index] / 20.0 for p in points]
        ys = [p[top_index] for p in points]
        mx, my = sum(xs) / n, sum(ys) / n
        num = sum((x - mx) * (y - my) for x, y in zip(xs, ys))
        den = sum((x - mx) ** 2 for x in xs)
        return num / den if den else float('nan')

    print(f'least squares over the {len(sub)} sub-unity pages, ink top against line height:')
    print(f'  reference slope  {slope(sub, 2, 3):.4f}')
    print(f'  our slope        {slope(sub, 4, 5):.4f}')
    print()
    print('how far our first baseline sits below where the reference puts it, '
          'net of the\nconstant ink-box offset measured on the 100% page '
          f'({full[5] - full[3]:+.3f} pt):')
    print(f"{'pct':>6} {'ref shrink':>11} {'our shrink':>11} {'error pt':>9}")
    for line, pct, rh, rt, oh, ot in sub:
        print(f'{pct:6.2f} {rt - full[3]:+11.3f} {ot - full[5]:+11.3f} '
              f'{(ot - ot) + (full[3] - rt) - (full[5] - ot):+9.3f}')
    print()
    print('slope 0.800 means the ascent is forced to 4/5 of the shrunk height;')
    print('slope 0.000 means the ascent is the font\'s own and untouched.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
