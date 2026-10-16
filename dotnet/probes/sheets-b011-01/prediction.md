# sheets-b011-01 — prediction, written before any reach was measured

Round of 2026-08-14 on branch `wt-sheets-b011`. Reference binary **LibreOffice 26.2.4.2
620(Build:2)**. Written and committed **before** the corpus sweep, the regression sweep and the
test run. Scored honestly in `results.md`.

The document is `sheets/batch-011/xls/T0A0D0000090006XLSE.xls` — 162/162 pages, **42471 words
against the reference's 40382**, +2089, +5.2%.

## 0. What was already established when this was written, and how

Stated separately from the predictions, because a prediction that repeats a measurement is not one.

- **This is not a raster-ceiling round.** `pdfimages -list` on the banked reference finds **zero**
  images in the whole 162-page file, so `TODO.raster-ceiling.md`'s first shape cannot apply.
- **This is not a determinism round.** The reference was converted four times by the same
  26.2.4.2 in the same session: **40696 raw words and 162 pages every time**, and the banked
  reference at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/sheets/` is the same 40696/162.
- **This is not a tokenisation round.** The whitespace-stripped character streams do **not**
  match: 258 687 non-space characters ours against 245 196 the reference's, **+13 491**. Real
  content, not `pdftotext` fragmenting. 126 of 162 pages are character-exact; the excess is on 36
  pages, all between 55 and 104.
- **The rule was measured before this was written**, on an authored probe through 26.2.4.2 and on
  the corpus document's own content stream. It is stated in §1 as a finding, not a prediction.

## 1. The rule (measured, not predicted)

A wrapping cell whose vertical alignment is **Top or Standard** is formatted only as far down as
the room it has, and **the lines past that are never formatted, never drawn, and are not in the
PDF's text layer.** The seat is not a clip at all:

```cpp
rParam.mpEngine->EnableSkipOutsideFormat(rParam.meVerJust==SvxCellVerJustify::Top
    || rParam.meVerJust==SvxCellVerJustify::Standard);   // output2.cxx:3115
```

```cpp
if( mbSkipOutsideFormat && nLine > 2
    && !maStatus.AutoPageHeight() && maPaperSize.Height() < nCurrentPosY )
    break;                                               // impedit3.cxx:1801-1806
```

with the engine's paper height being the cell's own,
`rAlignRect.GetHeight() - nTopM - nBottomM` (`calcPaperSize`, `output2.cxx:2684-2700`), and the
paper only bounded at all when the cell wraps (`output2.cxx:3074-3085`; otherwise it is
1 000 000).

Read as arithmetic: **lines drawn = max(4, floor(paperHeight / lineHeight) + 1)**, where
`paperHeight = cellHeight − topMargin − bottomMargin`.

Fitted against a 12-row authored sweep rendered through 26.2.4.2 — row heights 0.4 cm to 3.2 cm,
Liberation Sans 10 pt, pitch 11.20 pt — **12 of 12 exact**:

| row height | paperH/pitch | lines drawn | max(4, floor+1) |
|---:|---:|---:|---:|
| 11.310 pt | 0.83 | 4 | 4 |
| 22.677 | 1.85 | 4 | 4 |
| 33.987 | 2.86 | 4 | 4 |
| 39.713 | 3.37 | 4 | 4 |
| 45.298 | 3.87 | 4 | 4 |
| 50.995 | 4.37 | 5 | 5 |
| 56.665 | 4.88 | 5 | 5 |
| 68.003 | 5.89 | 6 | 6 |
| 79.313 | 6.90 | 7 | 7 |
| 90.652 | 7.92 | 8 | 8 |

and against four further authored cases:

| case | reference | why |
|---|---|---|
| vertical `bottom`, row too short | **all 60 words drawn** | skip is not enabled for Bottom |
| vertical `middle`, row too short | **all 60 words drawn** | nor for Centre |
| vertical unstated (Standard) | **truncated to 4 lines**, placed bottom | Standard *is* in the guard |
| no wrap, twenty hard-break paragraphs | **all 20 drawn** | no wrap ⇒ paper height 1 000 000 |
| exact multiple: paperH = 5 × pitch | **6 lines** | the comparison is strict, so floor+1 not ceil |

and on the corpus document's own decisive row — page 55's last, 427.21→286.58 pt, margin 40 twips
(the BIFF filter's), pitch 11.197: `floor(136.63/11.197)+1 = 13`, and the reference draws
**exactly 13**.

**It is not the `ManualSize`/optimal-height branch and not a clip.** That branch
(`output2.cxx:3255-3261`) decides only whether a hard clip rectangle is emitted; both sides of it
truncate. The corpus document's reference page 55 carries **no clip operator at all** beyond the
page's own, and still stops after 13 lines.

## 2. What the round will do

**2.1** The fix is confined to `SheetTextLayout.Place`: after `Wrap` and before placement, drop
the lines past the budget. Nothing else in the tree changes. `PdfContentSink`,
`ClipPathKeepingText` and the horizontal clip are untouched.

**2.2** `T0A0D0000090006XLSE.xls` will land inside the 2 %+3 band. I predict **40382 ± 150** and
verdict `words` → `match`. If it lands short of the reference the budget is one line too tight;
if it stays over, some of the excess is a second mechanism.

**2.3** Page counts move on **0 of 171**. Pagination is `SheetOptimalRowHeights` and
`SheetPagination`, both upstream of drawing; a line that is not drawn cannot change a row height.
**If a page count moves at all I have changed something I did not mean to.**

**2.4** Word counts move on **15 to 45 of 171** documents, essentially all of them downwards.
Every wrapping cell in the corpus that is taller than its row is a candidate, and the row heights
we compute are LibreOffice's own coarse 96 dpi answer while the text is drawn at printer
resolution — so the two disagree by a line routinely rather than exceptionally.

**2.5** Verdicts: **+1 to +4** documents reach `match`, **0 to 2** move away from it. The risk is
one-sided and I am naming it: the minimum of four lines is what stops a one-line row from losing
its text, and if I get that minimum wrong the loss is silent and large.

**2.6** Fidelity stays at **30 failed of 550, 0 skipped** — the baseline measured on this branch
before anything was changed. None of the 23 fidelity documents is a spreadsheet with an
overflowing wrapped cell.

**2.7** `sheets/batch-001`–`011` ends at **no worse than the 96/99 batches 001–010 stand at
today**, plus batch-011's own row. The four documents named as ceilings in earlier rounds
(`2017-04-27`/`2020-01-29-Lease-Transition`, `Published_Issuances_2024`,
`fse_identification_form`) keep the same two numbers on each side.

**2.8** The three earlier sheets defects are **not** this one. This is not `sheets-overflow-01`
(which page a rightward spill is painted on), not `sheets-clip-01` (the horizontal ink clip,
which never touches the text layer), and not `sheets-wrap-01` (a field's pitch). The
distinguishing measurement is already made: those three move ink or geometry, and this one moves
**13 491 characters out of the text layer**, which none of them can.

## 3. What I expect to be wrong about

- The **minimum of four lines** is fitted to a probe, not read off 26.2.4.2's own source (the tree
  here is 27.2-alpha, where the guard reads `nLine > 2`). If the two disagree the error shows as
  a one-line difference on short rows only.
- **Fields.** A wrapping hyperlink cell advances by its face's ascent rather than by the line
  height (`sheets-wrap-01`), and I am accumulating the budget with the same pitch on the reasoning
  that the pitch is a formatting property. Untested; the four-line minimum hides it on every
  corpus row I have looked at.
- **Rich cells** whose lines are not all the same height. The walk accumulates per line, so it
  should be right, but nothing measured pins it.
