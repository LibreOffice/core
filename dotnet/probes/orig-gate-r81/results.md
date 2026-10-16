# The original corpus at `e6be864e4` — seven merges later, unmoved

    ours = XRay.Office.Cli @ e6be864e4     ref = /usr/bin/soffice, 24.2.7.2
    rule = batch-check.sh of 2026-09-05

24.2.7.2 deliberately, and not the calibration target: this run answers *did seven merges break
anything*, and that is only meaningful with the reference held fixed against the banked baseline.
`odf-gate-r80` next door measures fidelity and uses 26.2.4.2.

    TOTAL 947  MATCH 871  MISMATCH 76  REF-CANNOT-RENDER 0

| track | match | of |
|---|---:|---:|
| `docx` | 257 | 272 |
| `pptx` | 243 | 251 |
| `xlsx` | 206 | 241 |
| `doc` | 58 | 66 |
| `xls` | 57 | 64 |
| `ppt` | 49 | 51 |
| `xlsm` | 1 | 2 |

## The set, not the count

Every figure matches `orig-gate-r76` exactly, and the *set* does too:

    matched before and not now: 0
    matched now and not before: 0

That matters more than the total here, because two of the seven merges changed code every format
runs through — `FontSubsetter`'s kept-table set (found while chasing an ODF-only symptom, and
general) and `CellBreaks` in `XRay.Office.Text`. Neither moved a single original-corpus row.

`REF-CANNOT-RENDER 0`, against 13 in the contended converted-corpus sweep of the same morning,
also says this run was not fighting three rounds for the box — so its zero is a real zero.
