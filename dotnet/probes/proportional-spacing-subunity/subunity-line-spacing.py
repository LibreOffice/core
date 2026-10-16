#!/usr/bin/env python3
"""Proportional line spacing *below* 100%, which an earlier probe never tested.

Why this exists
---------------

An earlier probe on this project measured `w:line` with `w:lineRule="auto"` at
240, 264, 276, 288, 300 and 360 twentieths and found our line pitch agreeing
with the reference to **0.000 pt on every one**. That result was used to rule
proportional line spacing out as a cause on `OM template`, and it was sound as
far as it went.

Every value it tested was **at or above 100%**. The regime below was never
looked at, and it is wrong.

What this measures, 2026-08-15, Liberation Serif 11 pt, eleven lines on a page
of its own per value, in whole twips on both sides:

    w:line  pct     ref  ours  delta        w:line  pct      ref  ours  delta
       120  50.00   126   127   +1             186  77.50   197   198   +1
       126  52.50   131   132   +1             192  80.00   202   203   +1
       132  55.00   139   140   +1             198  82.50   209   208   -1
       138  57.50   144   145   +1             204  85.00   215   216   +1
       144  60.00   151   152   +1             210  87.50   220   223   +3
       150  62.50   159   157   -2             216  90.00   227   228   +1
       156  65.00   164   165   +1             222  92.50   235   233   -2
       162  67.50   172   173   +1             228  95.00   240   241   +1
       168  70.00   177   178   +1             234  97.50   247   248   +1
       174  72.50   184   183   -1             240 100.00   253   253    0

**We are exactly one twip too tall on sixteen of the twenty-one**, and the five
exceptions are 62.5, 72.5, 82.5, 87.5 and 92.5 — all half-percents, though five
other half-percents (52.5, 57.5, 67.5, 77.5, 97.5) behave normally.

Two earlier readings of this were wrong and both are worth knowing about:

- A **narrow sweep** (180, 200, 220, 233, 239, 240, 260) found every sub-unity
  value at exactly +1 twip. That is a clean off-by-one and would have been
  implemented as one. It is the majority behaviour and not the rule.
- A **wider sweep that grouped values by vertical gap on one page** reported
  62.5% and 87.5% as anomalies and, separately, silently dropped everything past
  the eighth group once the content outgrew the page. One value per page removes
  the heuristic entirely; the anomalies survived that change, so they are real.

**Do not subtract a twip.** It would make sixteen exact and the other five worse
— 62.5% would go from −2 to −3 and 92.5% from −2 to −3 — and it is curve-fitting
to a majority rather than a rule.

The reference is **not** a simple scaling of its own 100% height either. Against
`253 × pct` the residuals swing from −1.83 (52.5%) to +0.88 (62.5%), so 253 is
not the number being scaled, and whatever LibreOffice multiplies is not what it
reports at 100%.

The rule, found in the C++ after curve fitting was proved impossible
-------------------------------------------------------------------

Successive differences in the table above swing by **3** twips, where rounding
any smooth function of a *single* scale can only ever swing by 1. That is a
proof, not a hunch: the reference is not scaling one number, so no amount of
further sampling would have produced the rule. It had to come from the source.

`SwTextFormatter::CalcRealHeight`, `sw/source/core/text/itrform2.cxx:2367-2394`:

    tools::Long nTmp = pSpace->GetPropLineSpace();
    if( nTmp < 50 ) nTmp = nTmp ? 50 : 100;      // clamped at 50%
    if (nTmp < 100) {
        nTmp *= nLineHeight;
        nTmp /= 100;                              // TRUNCATING integer division
        if( !nTmp ) ++nTmp;
        nLineHeight = nTmp;
        SwTwips nAsc = (4 * nLineHeight) / 5;    // ascent forced to 80%
        m_pCurr->SetAscent( nAsc );
        m_pCurr->Height( nLineHeight, false );
    }

Four things there that no amount of measuring would have produced:

1. **Truncating integer division**, not rounding.
2. **The ascent is forced to exactly 80% of the shrunk height** — it is not
   scaled proportionally with it. That alone breaks any model that treats the
   line as one quantity.
3. **A 50% floor**, with a curious `nTmp ? 50 : 100` so that a stated 0 means
   100 and anything else below 50 means 50.
4. It is **gated on `DocumentSettingId::PROP_LINE_SPACING_SHRINKS_FIRST_LINE`**
   and on `IsParaLine()`, so it does not apply uniformly.

And a *second*, opposite branch at `itrform2.cxx:2337-2347`, on the fixed
line-height path: `if( nTmp < 100 ) nTmp = 100;` — there, sub-unity proportional
spacing is **ignored entirely**. So which of the two a line takes decides
whether shrinking happens at all.

The percentage itself is `round(w:line * 100.0 / 240)` at
`sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx:5399` — a floating-point
round, half away from zero, which is why 99.58% returns the 100% answer and
takes neither branch.

**What is still open, and a trap to not fall into on the way.** Composing the
above into `trunc(253 * p / 100)` and solving for the `p` that reproduces each
of the twenty-one reference heights gives a unique `p` for every one — which
looks like a fit and is worth nothing, because it is twenty-one free parameters
against twenty-one points. The model is only evidence if some *single* rule
predicts `p` from `w:line`, and none does:

    pct   50.0  52.5  55.0  57.5  60.0  62.5  65.0  67.5  70.0  72.5
    p       50    52    55    57    60    63    65    68    70    73
    p-pct  0.0  -0.5   0.0  -0.5   0.0  +0.5   0.0  +0.5   0.0  +0.5

    pct   77.5  80.0  82.5  85.0  87.5  90.0  92.5  95.0  97.5 100.0
    p       78    80    83    85    87    90    93    95    98   100
    p-pct +0.5   0.0  +0.5   0.0  -0.5   0.0  +0.5   0.0  +0.5   0.0

52.5 rounds **down**, 62.5 rounds **up**, 87.5 rounds **down** again. That is
not round-half-up, round-half-even, floor or ceil. So `trunc(253 * p / 100)` is
**not** the mechanism either, and the three sub-unity halves that go the wrong
way are the same three the residuals always flag.

What that means for the next round: the open question is not "which rounding
converts the percentage" but **what quantity `nLineHeight` actually holds at
that point in `CalcRealHeight`**. 253 twips is what the reference *reports* at
100%, and the round-trip through the 600 dpi reference device means the value
being scaled need not be the value emitted. Answer that from the source before
composing anything, and do not solve for a per-point parameter again.

Where it matters
----------------

`SPA-02_mcar_part-2_and_IS_v2.9.docx` and `02_mcar_part-2_and_IS_v2.10.docx`
set their table body paragraphs at `w:line="233" w:lineRule="auto"` — 97.08% —
and only 12 of 305 rows declare a `w:trHeight`, so for the rest the line height
*is* the row height. The +0.05 pt here is the +0.0525 pt per row measured
directly on those tables by `probes/row-boundary-drift/`, and it accumulates to
+1.08 pt over 24 rows, which is what decides where a long table breaks across a
page. Both documents fail on page count alone with words inside the band.

The rule is **not derived**, and the two corrections above are why that matters:
the narrow sweep gave a clean wrong answer, and the grouped sweep gave a
different wrong answer for a reason that was in the instrument rather than in
the renderer. The next round should start from the twenty-one points above and
work out what quantity is actually being scaled, rather than fitting the 100%
height.

Usage
-----

    XRAY_OFFICE_CLI=... python3 subunity-line-spacing.py /abs/workdir
"""

import os
import re
import subprocess
import sys
import zipfile
from collections import defaultdict

LINES = [120, 126, 132, 138, 144, 150, 156, 162, 168, 174,
         180, 186, 192, 198, 204, 210, 216, 222, 228, 234, 240]
FACE = 'Liberation Serif'
HALF_POINTS = 22

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


def build(path: str) -> list[str]:
    """One `w:line` value per page, eleven lines each.

    Deliberately one per page rather than groups separated by a blank. The
    first two cuts of this probe put every group on one page and split them by
    looking for a vertical gap, and that heuristic silently dropped groups once
    the content outgrew the page and mis-measured the ones that straddled a
    break — which is where two of the "anomalies" in the earlier readings came
    from. A page break is unambiguous and costs nothing.
    """
    body, labels = '', []
    run = f'<w:rFonts w:ascii="{FACE}" w:hAnsi="{FACE}"/><w:sz w:val="{HALF_POINTS}"/>'

    for index, line in enumerate(LINES):
        if index:
            body += '<w:p><w:pPr><w:pageBreakBefore/></w:pPr></w:p>'
        body += ''.join(
            f'<w:p><w:pPr><w:spacing w:line="{line}" w:lineRule="auto" w:before="0" w:after="0"/>'
            f'<w:rPr>{run}</w:rPr></w:pPr><w:r><w:rPr>{run}</w:rPr>'
            f'<w:t>L{j}</w:t></w:r></w:p>'
            for j in range(11))
        labels.append(f'w:line={line} ({line / 240 * 100:.2f}%)')

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


def baselines(pdf: str, page: int) -> list[float]:
    text = subprocess.run(['pdftotext', '-bbox', '-f', str(page), '-l', str(page), pdf, '-'],
                          capture_output=True, text=True, check=True).stdout
    found = sorted({round(float(m.group(1)), 3)
                    for m in re.finditer(r'yMin="([\d.]+)"', text)})
    if not found:
        raise SystemExit(f'no text in {pdf} — did it render?')
    return found


def pitch(pdf: str, page: int) -> float:
    """The baseline pitch on one page, which holds one `w:line` value and nothing else."""
    ys = baselines(pdf, page)
    if len(ys) < 11:
        raise SystemExit(f'{pdf} page {page}: {len(ys)} lines, expected 11')
    return (ys[-1] - ys[0]) / (len(ys) - 1)


def main() -> int:
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/subunity-line-spacing'
    cli = os.environ.get('XRAY_OFFICE_CLI')
    if not cli:
        raise SystemExit('set XRAY_OFFICE_CLI to the tree you mean to measure')

    os.makedirs(os.path.join(out, 'ours'), exist_ok=True)
    docx = os.path.join(out, 'sub.docx')
    labels = build(docx)

    subprocess.run(['soffice', '--headless', '--convert-to', 'pdf', '--outdir', out, docx],
                   capture_output=True, check=True)
    subprocess.run([cli, 'render', '--quiet', '--outdir', os.path.join(out, 'ours'), docx],
                   check=True)

    reference, ours = os.path.join(out, 'sub.pdf'), os.path.join(out, 'ours', 'sub.pdf')
    for path in (reference, ours):
        if not os.path.isfile(path):
            raise SystemExit(f'{path} was not written — nothing to compare')

    print(f"{'case':22s} {'ref tw':>7s} {'our tw':>7s} {'delta tw':>9s}")
    for page, label in enumerate(labels, start=1):
        ref = pitch(reference, page) * 20
        our = pitch(ours, page) * 20
        print(f'{label:22s} {ref:7.1f} {our:7.1f} {our - ref:+9.1f}')

    return 0


if __name__ == '__main__':
    raise SystemExit(main())
