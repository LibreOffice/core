# Slides-E round 01 — the prediction, committed before any post-change measurement

Written after the census and after the code change built, and **before** any sweep, any gate
run and any pixel comparison. Unedited afterwards; scored in `results.md`.

## What the fix is

`DrawingChartAutoFormat.ColourOf` gains a `DrawingStyleMatrix? styles`, and for `stroke: true`
puts the accent it resolves through the theme's `THEMED_STYLE_SUBTLE` line style with
`DrawingStyleMatrix.Substitute`, exactly as `LineFormatter::convertFormatting` pushes
`getPhColor(nSeriesIdx)` into the themed `LineProperties`
(`oox/source/drawingml/chart/objectformatter.cxx:857-864`).

## The numbers

| # | prediction |
|---|---|
| **P1** | slides renderings changed: **1 of 163**, and it is `Demick_JetBlue.pptx`. The census walks parts and parses them, and says exactly one deck has a chart series that resolves a stroke through a theme line style whose `phClr` carries a transform. |
| **P2** | words: **0 of 200**. sheets: **0 of 171**. Zero *by construction* rather than by luck — `DocxPictures.cs:208` passes no matrix at all and `XlsxDrawings.cs:272` passes `styles: null`, and with a null matrix the new path returns the placeholder it was given. |
| **P3** | verdicts moved: **0 of 163**, so slides stays **144 of 163** on the corrected word metric and **163 of 163** page-exact. The gate asks how many pages, how many extractable words and are the fonts embedded; a colour is none of the three. `Demick_JetBlue` is a `words` failure at 713 against 608 and will still be one. |
| **P4** | direction: on `Demick_JetBlue`'s **5 chart pages**, `\|ink\|%` moves **down** on every one and no page moves up. Ranked by `\|ink\|%`, decided by `ink%`. The three accents on page 4 should land on `#B45D03`, `#761D26`, `#12415C` — the reference's own values, read out of its PDF. |
| **P5** | the fill half is **not** implemented, because its measured census reach is **0 of 163**: every automatic filled series in the corpus resolves through a `fillStyleLst` entry that is a bare `<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>`. |
| **C1** | the control that decides whether the fix is right rather than merely different: `Sector_Skills_Insights_Advanced_Manufacturing_summary_slide_pack.pptx` is the *other* deck with automatic-stroke series, and its subtle line style is a bare `phClr` with no transforms. **It must not change.** If it does, the substitution is doing something the theme did not ask for. |

## Blind spots, named before they can be excused afterwards

1. **The census follows charts from slides only** — not from `slideLayout` or `slideMaster` parts,
   and not `cx:chartSpace` (chartEx). A chart reached only from a layout would be invisible to it,
   so P1 is a ceiling that could be one *low* as well as one high.
2. **"Auto stroke" is decided by the absence of a fill under `c:ser/c:spPr/a:ln`.** A series whose
   colour arrives some other way — a `c:dPt`, a `c:extLst` — is counted as automatic when it is not.
   That direction inflates the ceiling rather than deflating it.
3. **`pdf-image-diff.py` rasterises at 512 px.** A 0.75 pt line at that resolution is roughly a
   third of a pixel, so the colour change on the *thinnest* strokes may be partly blended away and
   the measured improvement will understate the change in the PDF operators. The operator-level
   colour census is therefore the primary evidence for P4 and the pixel diff the secondary one.
4. **`pdf-ops.py` does not report `sh`.** Not expected to bite — a stroke colour is `RG`, not a
   shading — but the chart backgrounds on these pages are not what is being counted.
5. **I have not measured what the *legend key* does.** The claim that it follows the series colour
   is read from the handover, not from this round's own output.
6. **The theme entry could state a literal colour rather than a `phClr`.** LibreOffice would then
   draw that literal colour and ignore the accent; the implementation follows it, and no corpus
   theme does it, so that branch is unexercised by the corpus and is covered by a test only.

## What would refute the fix

- `Sector_Skills` changing (C1).
- Any words or sheets rendering changing.
- `Demick_JetBlue`'s three page-4 accents landing anywhere other than `B45D03` / `761D26` /
  `12415C`.
- More than one slides rendering changing without the census having named it.
