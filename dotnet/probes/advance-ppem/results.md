# The ~0.1% advance divergence is LibreOffice's PDF width encoding, not its layout

**Round `advance` — 2026-09-06, base `531e9a1f3`.** Measured against LibreOffice **24.2.7.2**
(`/usr/bin/soffice`) and **26.2.4.2** (`/opt/libreoffice26.2/program/soffice`, with its bundled
Latin metric duplicates and its Latin `NotoSans-*`/`NotoSerif-*` moved aside so it resolves the
system faces), system fonts from `/usr/share/fonts`, `fc-match "DejaVu Sans"` answering
`DejaVuSans.ttf`, and `XRay.Office.Cli` built from this worktree.

## The claim under test

`CLAUDE.md`'s rule 3 has said, for four rounds, that there is a real ~0.1% divergence in the
*advance widths* the two stacks use; that ours is exactly `hmtx × size / upem` and the
reference's is not, differing per glyph by up to 0.3%; and that closing it means reproducing
FreeType's hinted advance at LibreOffice's ppem.

**All three parts are wrong, and the same measurement error produced all of them.** Every figure
behind them was read out of a LibreOffice PDF's *glyph positioning* — from `pdftotext -bbox`,
which reconstructs a pen from the PDF's declared widths, or from the `TJ` integers directly. That
channel is quantised to whole thousandths of an em. One thousandth of an em is **0.36% of a
Liberation Serif `i` and 0.17% of a Liberation Mono digit**: the instrument's own resolution is
several times the defect it was used to measure.

## What the reference actually does

Two facts from `vcl`, both confirmed in the files themselves:

- **Shaping is unhinted and is ours.** `LogicalFontInstance::InitHbFont`
  (`vcl/source/font/LogicalFontInstance.cxx`:94-103) creates the HarfBuzz font with
  `hb_font_set_scale(pHbFont, nUPEM, nUPEM)` and `hb_ot_font_set_funcs` — HarfBuzz's own
  OpenType functions, which read `hmtx`. There is no FreeType font-funcs object and no hinting
  anywhere in the advance path. The advance LibreOffice lays out with *is* the design advance,
  scaled by `mnHeight / upem` (`LogicalFontInstance::GetScale`).
- **The PDF writer truncates.** `registerGlyph` records `XUnits(pFace->UnitsPerEm(), nGlyphWidth)`,
  and `XUnits` is `inline int XUnits(int nUPEM, int n) { return (n * 1000) / nUPEM; }`
  (`vcl/inc/fontsubset.hxx`:29) — integer division, so the declared width is
  `floor(hmtx × 1000 / upem)` by construction. `drawHorizontalGlyphs`
  (`vcl/source/pdf/pdfwriter_impl.cxx`:5814) then emits a `TJ` correction only when
  `trunc(declared − actual·1000/ppem + 0.5)` is non-zero, which a systematic sub-unit deficit
  never makes it.

Measured, over every glyph of every subset in three corpus documents
(`word-pens.tsv`): **every declared width is `floor(hmtx × 1000 / upem)`**, in both binaries,
with a mean deficit of **0.48 to 0.65 thousandths of an em per glyph**. So a reader
reconstructing a pen inside one text object falls behind the pen the layout intended by about
half a thousandth of an em per glyph, and resets at every `Td`/`Tm`.

Half a thousandth of an em is 0.05% of a 1000-unit glyph and 0.18% of a Liberation Serif `i`. It
accumulates along a run and resets at every stated pen. That is the whole of "~0.1%, and it
accumulates *between* the tab stops".

## The layouts agree, to a hundredth of a percent

`advance-width.py` measures the same two stacks through a channel that is *not* quantised. A
right-aligned line puts its right edge on the margin, so the `Td` the writer emits is
`margin − width(line)`; two lines built from the same unit, one N₀ repeats long and one N₁,
differ in pen by exactly (N₁ − N₀) unit widths, and every fixed term cancels. Dividing by the
repeat count divides the pen's own rounding by it too.

**5 faces × 6 units × up to 11 sizes = 314 cases, three stacks each** (`width.tsv`):

| | ours against 26.2.4.2 | ours against 24.2.7.2 |
|---|---|---|
| worst case over all 314 | **0.0077%** | **0.0107%** |
| mean, per face and unit | ≤ 0.002% | ≤ 0.002% |

A hundredth of a percent is the instrument's floor — the reference's pen is rounded to a logical unit
(1/100 mm) before it is written. The units are not only isolated glyphs: they include
`Hamburgefonstiv`, ` AVATAR Wave To. Yes,` (six kern pairs), ` the quick brown fox jumps over
the lazy dog` and ` o`, so shaping, kerning, ligature formation and the space glyph are all
inside the agreement. The one case where both sides leave the design metric together is
Carlito's `hamburgefonstiv`, −0.104% from the unkerned sum in *both* stacks: a kern pair we and
the reference apply identically.

**So neither stack grid-fits anything, no ppem quantisation is visible at any of eleven sizes,
and there is nothing to reproduce in `XRay.Office.Text`.**

## The control that settles it

`TabStopComparisonTests` runs the same assertion at the same 0.1 pt tolerance over two
documents. `tabbed.docx` passes and `list-label-overrun.docx` fails, and the discriminator is
visible in the PDFs (`word-pens.tsv`):

- In `tabbed.docx` every stretch after a tab is its own text object with its own `Td`, so the
  reference *states* each position. All three renderings agree to 0.100 pt — the constant PDF pen
  offset the tests already model — at every word.
- In `list-label-overrun.docx` the whole line is one text object, so every position after the
  first has to be reconstructed from declared widths. The three renderings drift apart word by
  word: 0.000, 0.011, 0.044, 0.066, 0.088, 0.099 pt between the two reference binaries alone.

Same test, same tolerance, same document family. What changes is whether the number was stated or
reconstructed.

## Why 26.2.4.2 looks "further from the design metric" than 24.2.7.2

It is not further from anything. The `Td` origins of every portion are **identical** between the
two binaries on `list-label-overrun.docx` and on `tabbed.docx` — the layout did not move. What
moved is the `TJ` arrays: 24.2.7.2 emits an adjustment at a handful of positions, 26.2.4.2 emits
one at nearly every position. 26.2.4.2 encodes the kerning the layout applied; 24.2.7.2 dropped
some of it, which made its lines *wider* and happened to cancel part of the truncation deficit.

On `paginated.docx` line 1 — 100 glyphs of 11 pt Carlito, the case
`PageDrawingComparisonTests` fails on — the arithmetic closes:

| | line 1 width |
|---|---:|
| ours (fractional `/Widths`, exact) | 459.573 pt |
| 24.2.7.2 | 459.239 pt |
| 26.2.4.2 | 458.953 pt |
| **predicted truncation deficit**, Σ(exact − floor) over those 100 glyphs at 11 pt | **0.461 pt** |
| observed deficit, ours − 26.2.4.2 | 0.620 pt |

The 0.520 pt the test's own remark records is that deficit, and the remark has the two sides the
wrong way round: **ours is 530.423 and the reference 529.903**, not the reverse.

## What this means for the ten failing methods this was said to reach

Eight of them are this family — `PageDrawingComparisonTests` (×4) and
`TabStopComparisonTests` (×4) — and each compares an absolute position that lies N glyphs
deep inside one reference text object. The instrument's resolution there is
**N × 0.5/1000 em × size**, which for `paginated.docx` line 1 is 0.55 pt against a 0.5 pt
tolerance: *the tolerance is below the noise floor of the channel it is measured through.* No
change to our layout can close that, because our layout is already right; only writing our own
PDF with LibreOffice's truncated integer widths would, and that would make our output worse in
order to make a test greener.

They are left failing, with their remarks corrected to say what they measure.

## The one that was reachable, and was a rounding after all

`SheetTextComparisonTests.EveryCellIsDrawnWhereLibreOfficeDrawsIt` is *not* this family. Its own
note already says so: 17 of the 24 runs in the document are identical across all three renderings
to a thousandth of a point, and the failure is an **indent**.

An OOXML `indent` level is three spaces of the workbook's default font
(`sc/source/filter/oox/stylesbuffer.cxx`:1263), and one space is `xFont->getCharWidth(' ')`
(`sc/source/filter/oox/unitconverter.cxx`:139) — which is `OutputDevice::GetTextWidth` cast to
`sal_Int16` (`toolkit/source/awt/vclxfont.cxx`:77), so it reaches the multiplication as a **whole
number of twips**. The only open question is which way the fraction goes, and we truncated.

`indent-twip-rounding.py` builds one workbook per default font size, each stating an indented and
an unindented cell in the same column so the pen difference is the indent and nothing else, and
reports the six sizes at which Liberation Sans' 5.5566 twips per point separate `floor` from
`round` (`indent.tsv`):

| default size | space, twips | floor | round | 24.2.7.2 | 26.2.4.2 | ours, before | ours, after |
|---|---:|---:|---:|---:|---:|---:|---:|
| 10 pt | 55.566 | 16.500 | 16.800 | 16.498 | **16.781** | 16.498 | **16.781** |
| 12 pt | 66.680 | 19.800 | 20.100 | 19.786 | **20.098** | 19.786 | **20.098** |
| 14 pt | 77.793 | 23.100 | 23.400 | 23.386 | **23.386** | 23.074 | **23.386** |
| 16 pt | 88.906 | 26.400 | 26.700 | 26.702 | **26.702** | 26.391 | **26.674** |
| 28 pt | 155.586 | 46.500 | 46.800 | 46.800 | **46.800** | 46.488 | **46.800** |
| 30 pt | 166.699 | 49.800 | 50.100 | 50.088 | **50.088** | 49.776 | **50.088** |

26.2.4.2 rounds at six of six, 24.2.7.2 at four of six, and truncating was wrong at every one of
the six against the target. Truncation had been calibrated on the two sizes where 24.2.7.2 agrees
with it.

The fix is `Length.Twips` — which already divides rounded — in `XlsxCellFormats.IndentUnit`. It
is asserted as a mechanism in `SheetIndentUnitTests`, which checks at each of the six sizes that
the case genuinely separates the two rules *before* checking that the reader chose rounding.

`SheetCellMarginTests.AnIndentIsMeasuredOnTopOfTheCellsOwnMargin` pinned the `.xlsx` at 261.38,
which is 24.2.7.2's answer. Rendering all three spellings of that workbook through both binaries:
the `.xls` lands at 265.720 and the flat ODF at 264.869 under each, and only the `.xlsx` moves —
**261.383 under 24.2.7.2 and 261.666 under 26.2.4.2**. Ours is now 261.666.

## Not this family, and left alone

- **`SheetDrawingComparisonTests`.** Its own remark already classifies it as 26.2.4.2 clamping a
  full-cell anchor offset to `cellSize − 5` (1/100 mm), with 34 probe renderings behind it, and
  the picture's height is EMU arithmetic within one row with no font in it. `CLAUDE.md` listed
  it under rule 3's reach and that was already stale when it was written; rule 3 is now
  withdrawn outright.
- **`SlideChartFaceComparisonTests`.** It passes. But its 5.839 pt digit is **not** this family
  either, and is worth a round of its own: the `TJ` adjustment there is **16 at every
  inter-glyph position** on a monospaced face, which is 2.7% of the advance — thirty times the
  quantisation, and far too large to be it. The chart's own `Tm` origins also move between the
  two binaries (the value labels sit at 89.542 under 26.2.4.2 and 89.713 under 24.2.7.2) where
  Writer's do not move at all, so the chart text really is laid out differently by the two
  binaries. The shape of the data answers the question `CLAUDE.md` poses about it: **per-position
  and constant, not per-glyph and outline-dependent.** A constant relative scale on a monospaced
  face cannot be outline hinting. The seat is somewhere in the metafile the chart is drawn into
  and replayed from — `tdf#168002` and `GetSubpixelPositioning` (`vcl/source/outdev/text.cxx`:1258)
  are in that area and changed in this window — and it is not established here.

## What would have to be true for this to be a real advance defect

For completeness, since three rounds have now believed it was: our line breaks would have to
differ from LibreOffice's. They do not. On `paginated.docx` line 1 all three renderings put the
same fourteen words on the line, and `PageDrawingComparisonTests` asserts the line *count* and
the word sequence before it asserts any position — those assertions pass. A stack whose advances
were 0.1% out over a 430 pt line would break a line early somewhere in a 100-document corpus, and
the failure would be a page count, not a fifth decimal place.

## Files

| | |
|---|---|
| `ttf.py` | minimal `head`/`hhea`/`cmap`/`hmtx` reader; fontTools is not installed here |
| `advance-width.py` | the differencing instrument — 314 cases, three stacks → `width.tsv` |
| `advance-staircase.py` | the first cut, kept as the record of *why* the `TJ` channel cannot answer this: its resolution is ±0.5/1000 em per gap |
| `pdf-width-quantisation.py` | the declared-width census and the per-word pen table → `word-pens.tsv` |
| `indent-twip-rounding.py` | the Calc indent rule, six discriminating sizes → `indent.tsv` |
