# A justified cell's lines reach the right edge of its column

Round `clip`, 2026-09-05. Environment: this container, `/usr/bin/soffice` **24.2.7.2** and
`/opt/libreoffice26.2/program/soffice` **26.2.4.2** (TDF tarball, its 33 duplicate font files
moved aside), `fc-match "DejaVu Sans"` → DejaVu, Carlito and Caladea installed.

## What was asked

`SheetTextLayout.Resolve` mapped `horizontal="justify"` and `horizontal="distributed"` to `Left`
with the remark "the stretch is not reproduced". Calc maps both to `SvxAdjust::Block`, and
EditEngine then shares each line's spare width among its **blanks** —
`ImpEditEngine::ImpAdjustBlocks` (`editeng/source/editeng/impedit3.cxx:2306-2420`), called from
`CreateLines` for every line with room left over (`:1694-1701`). A paragraph's last line is
exempt (`!bEOC`), and `distributed` is exactly the setting that lifts that exemption:
`bDistLastLine = GetJustifyMethod(nPara) == SvxCellJustifyMethod::Distribute` (`:1696`).

`probe.py` authors a one-cell workbook — 40 characters wide, Liberation Sans 11 pt, wrapping,
three lines of text — three times over at `justify`, `distributed` and `left`, and reads the
right edge of the last word on each line out of the rendered page.

## Result

Right edge of each line, in points from the page's left edge.

| alignment | side | line 1 | line 2 | line 3 |
|---|---|---:|---:|---:|
| justify | 24.2.7.2 | 293.02 | 293.13 | 285.86 |
| justify | 26.2.4.2 | 293.02 | 293.04 | 285.83 |
| justify | ours, after | 293.39 | 293.39 | 286.16 |
| distributed | 24.2.7.2 | 293.02 | 293.13 | 293.08 |
| distributed | 26.2.4.2 | 293.02 | 293.04 | 293.00 |
| distributed | ours, after | 293.39 | 293.39 | 293.39 |
| left (control) | 24.2.7.2 | 285.20 | 284.10 | 285.86 |
| left (control) | 26.2.4.2 | 285.22 | 284.02 | 285.83 |
| left (control) | ours, after | 285.51 | 284.31 | 286.16 |

Both references agree with each other and with the port on every cell: under `justify` the first
two lines reach the column's right edge and the last does not; under `distributed` all three do;
under `left` none does. The constant **0.3 pt** by which our edges sit right of the references'
is a column-width difference and not this rule — it is the same 0.3 pt on the `left` control,
where nothing is stretched.

Before the change, all three alignments produced the `left` row.

## What is not reproduced

`ImpAdjustBlocks` also opens a gap at every boundary between an Asian character and its
neighbour (`:2334-2343`) and inserts Kashidas for Arabic (`:2350`). Only the Latin blank arm is
ported. Nothing in the corpus has a justified cell of Asian or Arabic text.

Calc additionally zeroes the width of a blank that is a line's own last character, so that the
line's reported width lands exactly on the paper's (`:2361-2380`). Here the trailing blanks keep
their advances and hang past the right edge, which is where they already were: a justified cell
is placed from the left in every case, so nothing is positioned by that width.

## Reach

Measured by rendering the 63 sheets documents whose styles carry `horizontal="justify"`,
`horizontal="distributed"` or `fo:text-align="justify"`, or that are not a zip (so every `.xls`
and `.ods` is included), through the binary either side of the change and comparing the PDFs byte
for byte: **6 of 63 moved**. Scored against both references over up to six pages each, mean ink
at 60 dpi grayscale:

| | 24.2 before | 24.2 after | 26.2 before | 26.2 after |
|---|---:|---:|---:|---:|
| `2017-04-27-Lease-Transition-Records-Checklist-FINAL-1.xlsx` | 14.736 | **13.914** | 41.862 | 41.884 |
| `2020-01-29-Lease-Transition-Records-Checklist-FINAL-1.xlsx` | 14.205 | **13.340** | 42.325 | 42.349 |
| `PBN Matrix NAAs (V01).xlsx` | 2.418 | **1.817** | 2.258 | **1.645** |
| `Global_Market_Forecast_2016-2035_Airbus_Data_Set.xlsx` | 0.992 | **0.755** | 1.002 | **0.768** |
| `Aviation_Abbreviations.xlsx` | 0.216 | 0.216 | 0.216 | 0.216 |
| `flightstandards-doc-Cross-reference-table_version02.xlsx` | 5.520 | 5.520 | 5.348 | 5.348 |

The two lease checklists move 0.02 the wrong way against 26.2.4.2 and that figure is not
evidence: **the tarball substitutes a font for them that this machine does not have.** Their
cells are set in `Bell MT`; 24.2.7.2 and X-Ray.Office both resolve it through fontconfig to DejaVu
Sans, while the TDF 26.2.4.2 finds its own bundled `NotoSerif-Regular.ttf`, whose line is 23
device pixels at 13 pt against DejaVu Sans's 20. The reference's own flat-ODF export shows the
consequence in the row heights: 626 twips against 716 for a two-line row, 89 535 twips against
92 353 over the sheet, and a sixth page that neither 24.2 nor we produce. Against the reference
that reads the same fonts we do, both documents improve by about 0.85.
