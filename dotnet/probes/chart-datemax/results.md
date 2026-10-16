# `055_Project_timeline`'s date-axis maximum is `TODAY()`

Measured 2026-09-07 in `/home/user/wt-chartfit`, branch `agent/chartfit`, base `fdab86b6a`.
Reference `/opt/libreoffice26.2/program/soffice` **26.2.4.2**, its Latin metric duplicates, its
Latin `NotoSans`/`NotoSerif` and its `LiberationSansNarrow` moved aside. Corpus
`/home/user/sample-files`. The document is
`sheets/chartset-008/xlsx/055_Project_timeline_with_milestones_Use_this_template_546cecc0.xlsx`.

## The question

`probes/chart-secaxis/results.md` §2.3 left it open:

> The categories are serials 45021 … 45169 — 5 April to 31 August 2023, 148 days — and the axis
> states `c:majorUnit val="10"` with `baseTimeUnit days`. We draw 5 Apr … 23 Aug at that ten-day
> step, which is the data's own range. The reference draws **5 Apr 2023 … 19 Apr 2026 at a 30-day
> step**, 38 labels … so its axis maximum is about serial 46132, a thousand days past the last
> category … Four of the seventeen rows the range covers are **empty**, and neither the cells nor
> the caches hold anything near 2026, so where its maximum comes from is not yet established.

## The answer, in one line

**The DATE column is twelve volatile formulas `DATE(YEAR(TODAY()),m,d)`.** LibreOffice
recalculates them on load and we print the value cached in the file, so its dates are this year's
and ours are 2023's. Nothing in `ScaleAutomatism` or in `ChartDateScale` is involved.

```xml
<c r="C20" s="25"><f ca="1">DATE(YEAR(TODAY()),4,5)</f><v>45021</v></c>
```

`xl/worksheets/sheet11.xml`, rows 20-32, twelve such cells: `4,5`, `4,24`, `4,24`, `5,1`, `5,15`,
`6,15`, `6,30`, `7,15`, `7,30`, `8,11`, `8,23`, `8,31`.

## The measurement that settles it

`probe2.py` rewrites one thing in the workbook and renders it through 26.2.4.2; the axis' labels
are read off the PDF's own text. The axis' number format is `[$-409]d\ mmm;@`, which writes no
year, so `fmtprobe.py` renders the same two documents with `[$-409]d\ mmm\ yyyy;@` on the axis and
nothing else changed — the years below are read, not inferred.

| the workbook's `YEAR(TODAY())` replaced by | 26.2.4.2 draws | labels | step |
|---|---|---:|---:|
| *(nothing — the file as it is)* | 5 Apr **2023** → 19 Apr **2026** | 38 | 30 days |
| `2026` | 5 Apr 2023 → 19 Apr 2026 | 38 | 30 days |
| `2023` | **5 Apr 2023 → 23 Aug 2023** | **15** | **10 days** |
| `2030` | 5 Apr 2023 → 28 Apr 2030 | 44 | 60 days |

**The `2023` row is what this tree draws, label for label.** So there is no defect in the axis'
range, its increment or its label rhythm: pin the volatile formulas and the two stacks agree.

## What the step is, and why it is not the stated one

`c:majorUnit val="10"` **is** honoured — the ticks are ten days apart in every one of those rows.
What varies is how many of them get a *label*: `VCartesianAxis::createTextShapes` increments
`rAxisLabelProperties.m_nRhythm` and calls `removeShapesAtWrongRhythm` until the labels stop
overlapping (`chart2/source/view/axes/VCartesianAxis.cxx`:948-955), so a 1110-day axis at a
ten-day tick draws every third label and reads as a 30-day step. Measured by varying only the
axis' font size, with the data untouched:

| `a:defRPr sz` | labels | step |
|---:|---:|---:|
| 600 | 57 | 20 days |
| 1200 *(the file's)* | 38 | 30 days |
| 2400 | 19 | 60 days |

A stated interval the axis *cannot* thin far enough is replaced instead — `c:majorUnit val="40"`
draws 29 labels 40 days apart, which is `nMainIncrementCount > nMaxMainIncrementCount` at
`ScaleAutomatism.cxx`:630-632.

## What was ruled out on the way

Every one of these changes **nothing at all** — the axis stays 5 Apr 2023 → 19 Apr 2026:

- **Every edit to the chart part's caches.** `c:numCache`/`c:strCache` values, `c:ptCount` (17 → 13,
  which is the four empty rows the previous round suspected), and the cached last date.
  *An `.xlsx` chart's data comes from the worksheet; the caches are not read at all*, which is why
  `probe.py`'s eight variants are eight null results.
- **The cell values themselves**, when the formula is left in place: `<v>` is overwritten by the
  recalculation. Editing the C column's cached values moves the axis only because it moves what the
  chart reads *before* the volatile pass, which is the mixture the section below describes.
- The bar group's categories (`$D$20:$D$36` → the dates), the two groups sharing one axis pair,
  dropping the bar group's `c:cat`, the ranges (`$20:$36` → `$20:$32` or `$20:$40`),
  `c:crossBetween` (`midCat` → `between`), `c:baseTimeUnit`, and both series' `c:errBars`.
- The duration column, the milestone column and the position column, at 10× and at zero.

## The one thing left unexplained, and it is small

The axis' **minimum** is the *loaded* 2023 value while its maximum reaches 2026, so the sequence
the chart reads is a mixture of the two passes rather than one or the other: with all twelve
formulas at 2026 the recalculated range would be 5 Apr 2026 … 31 Aug 2026, 148 days, and the axis
would be fifteen labels again. Pinning any *single* row to 2026 and the rest to 2023 leaves the
axis at the 2023 range in all eleven cases, so no one cell is the maximum; it takes the whole
column. That is a recalculation-ordering detail inside Calc — `ScChart2DataSequence::BuildDataCache`
against the volatile pass — and it is not reproducible from the file.

## What it means for this tree

This is `CLAUDE.md`'s known volatile-formula class — *"the reference recalculates the formula on
open and we print the value cached in the file. Six documents are affected and they drift further
apart every day"* — and this document is one of the six. It is not a chart defect: **the same
thirteen dates are wrong in the sheet's own DATE column**, which reads `4/5/2026` in the reference
and `4/5/2023` here, and that is 13 cells of text before the chart is reached. Closing it means
evaluating `TODAY()`, which is a spreadsheet-engine decision and not a chart one.

## Files

| file | what it is |
|---|---|
| `probe.py`, `cache-variants.tsv` | eight variations of the chart part's caches, all null |
| `probe2.py`, `sheet-variants.tsv` | thirty-eight variations of the worksheet and the chart, including the `YEAR(TODAY())` substitutions and the label-size sweep |
| `fmtprobe.py` | the same two documents with a four-digit year on the axis, so the years are read rather than inferred |
