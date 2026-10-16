# Prediction — the print band's right edge follows the last closed column run

Committed before any post-change rendering of the corpus. The only thing rendered so far is
`GridProbe` on `CSJU List of Recipients of funds 2013-2020.xlsx`, which is where the rule was
derived, and five edited copies of that one workbook put through **LibreOffice** rather than
through us.

## What the change is

`SheetDecorationArea` now bounds its scan by, and extends the print block to, the last column
the file *materialises*: the last `<col>`/`COLINFO` run that stops short of the sheet's last
column, or `first - 1` for a run that reaches it. Calc's print-area scan loops over `aCol`, and
`ScTable::ApplyPatternArea` materialises nothing for a range ending at `MaxCol`.

## What fixed the rule

Five edited copies of the CSJU workbook, each converted by `soffice`, against the arithmetic the
fit-to-width search does on the widened band:

| variant | predicted band | predicted pages | measured |
|---|---|---:|---:|
| as found (`…E`, `F`, `G–XFD`) | A–F, 21724 tw, zoom 46 | 96 | **96** |
| `F` merged into the open run | A–D, 19615 tw, zoom 52 | 97 | **97** |
| a closed run added at `J` | A–J, ~26169 tw, zoom 38 | 95 | **95** |
| `F` widened to 50 characters | A–F, 26186 tw, zoom 38 | 95 | **95** |
| `E` widened, `F` merged away | A–E, 25171 tw, zoom 40 | 95 | **95** |
| `F` keeps its run, loses its `style` | A–F, unchanged | 96 | **96** |

The fifth separates column E from column D; the sixth is what rules the column's own fill out as
the cause, and is why the extension is unconditional on what the column paints.

## The census, and what it counted over

`census.py` reads `xl/worksheets/sheet*.xml`, so it can read **109 of the track's 171
documents**; the other 62 are `.xls`, whose `COLINFO` records no zip-level census can see and
which the change reaches identically. Over the 109: **23 documents have at least one sheet whose
last closed column run is past its last data column.** The extension ranges from one column
(`CSJU`, `jobs-bulletin-51`) to 16126 (`SIL_TDB648`'s third sheet).

That is a ceiling, not a reach, and it should overstate badly: `StopAtEqualColumns` cuts the
block back before the first run of thirty visually equal columns behind the data, which is what
every one of the large extensions above looks like.

## The prediction

| | predicted |
|---|---|
| sheets renderings byte-changed | **10–35 of 171** |
| verdicts moved | **+1**, band **−2 to +3** |
| `CSJU List of Recipients of funds 2013-2020.xlsx` | 97/96 `pages` → **96/96 `match`** |
| words renderings byte-changed | 0 of 200 |
| slides renderings byte-changed | 0 of 163 |

The CSJU line is not really a prediction — `GridProbe` already reports 96 pages with the band at
zoom 46 and 97 rows on page 1, against the reference's 97 rows on page 1. It is stated so that
the round can be scored on it.

The band on the verdict total is deliberately two-sided and the downside is the real risk. This
change widens a print block on documents that **already match**, and a widened block can add a
column band. The control that decides it is the whole-track sweep: if matches fall, the rule is
measured on the corpus rather than argued from six renders of one workbook, and it comes out.
