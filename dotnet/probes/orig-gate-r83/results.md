# The original corpus against the calibration target — the first such measurement

    ours = XRay.Office.Cli @ 6966b6a47
    ref  = /opt/libreoffice26.2/program/soffice, 26.2.4.2, via $REF_SOFFICE
    rule = batch-check.sh of 2026-09-05

    TOTAL 947  MATCH 914  MISMATCH 33  REF-CANNOT-RENDER 0

**Every earlier original-corpus figure in this repository is against 24.2.7.2.** `orig-gate-r81`
scores the same tree at **871**. Neither number is wrong; they answer different questions, and the
gap between them is not our fidelity:

    rows failing at 24.2 that MATCH at 26.2:  46

| track | match | of | |
|---|---:|---:|---|
| `xls` | 64 | 64 | clean |
| `xlsm` | 2 | 2 | clean |
| `docx` | 266 | 272 | |
| `pptx` | 244 | 251 | |
| `doc` | 63 | 66 | |
| `ppt` | 49 | 51 | |
| `xlsx` | 226 | 241 | the weakest column |

## Which to use, and when

- **Regression checks keep using 24.2.7.2**, because the banked baseline was taken against it and a
  comparison only means something with the reference held fixed. That is `orig-gate-r81`.
- **Fidelity, and any picture of a defect, uses 26.2.4.2**, because that is the build the tree is
  calibrated to. Drawing a "defect" against 24.2 shows a version difference instead.

## A worked example of getting this wrong

`sectors-defense-and-aerospace.xlsx` was classified from the 24.2 rows as a font-resolution defect:
449 pages against 227, with our PDF embedding DejaVu Sans where the reference embedded Carlito
alone, and the wider face taken to explain the doubled page count.

**Against 26.2.4.2 the document is `449/449` — an exact match**, and the character counts agree to
11 in 22 986. The 227 is 24.2's own pagination. The font-embedding difference is real and changes
nothing measurable. `dotnet/CLAUDE.md`'s round-66 screen had already placed this document in the
version gap; the classification was made from the wrong table anyway.

## The 33, classified

| class | rows | |
|---|---:|---|
| charts in workbooks | 14 | 13 are `chartset-*`; almost all page-exact, differing in characters alone |
| no shared cause established | 10 | six are pagination off by one or two pages on long documents |
| **measured ceiling — not a defect** | 9 | 8 in `ceiling-*`, every one *positive* (we draw more extractable text than the reference, replaying a metafile as text where it rasterises); 1 glyph-exact row failing on `unembedded` alone |

Side-by-side renders of all 33, each at its worst-diverging page, were composed for review.
