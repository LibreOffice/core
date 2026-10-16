# sheets-overflow-01 — prediction, written before measuring

Date: 2026-08-14. Branch `wt-sheet-overflow`.

## What I read (27.2-alpha tree, `sc/source/ui/view/output2.cxx`)

`ScOutputData::LayoutStrings`, :1541-1543

```cpp
SCCOL nLoopStartX = mnX1;
if ( mnX1 > 0  && !bTaggedPDF )
    --nLoopStartX;          // start before mnX1 for rest of long text to the left
```

and :1498 `bool bTaggedPDF = pPDF && pPDF->GetIsExportTaggedPDF();`

The leftward reach itself is *unbounded* (:1640-1657): at the one extra column it scans left from
`mnX1` over `IsEmptyCellText` cells until it finds `oFirstNonEmptyCellX`, however far that is. So
the reach is NOT "one column" — but the whole mechanism is gated on `!bTaggedPDF`.

`officecfg/.../Common.xcs:4318-4323` gives `UseTaggedPDF` a default of **`true`**.

## Prediction

1. `soffice --convert-to pdf` exports **tagged** PDF by default, so `bTaggedPDF` is true and
   `nLoopStartX == mnX1`. **An overflow run anchored left of a page's first column is drawn on
   ZERO later pages.** Reach left = 0 columns, not 1, not unbounded.
2. Therefore the correct rule for our painter is: draw an overflow run **only on the page whose
   column block contains its anchor cell** — the `IsOutside` test must be on the *anchor cell's
   own box*, not on the widened output area.
3. On the anchor's own page the run is NOT clipped to the block; it may run past the right page
   edge (consistent with the brief's essd page 1 xMax 617.0 vs 617.7 on a 612 pt page).
4. Reach across the sheets track: I expect a **small** number of documents to move, because most
   sheets fit one column block wide. Guess: 8-20 of 171 documents move word counts, 0 move page
   counts (pagination is decided by `SheetTextOverflow.ExtendedLastColumn`, upstream of painting).
5. `RCO_VOR_Master_List_082824.xlsx`: reference is 80 pages. If ours is also 80, page 73 is the
   same defect. If ours differs, the reviewer's page-index caveat holds and RCO is out of class.

## How I will score it

- Tagged: `grep /StructTreeRoot` in a banked reference PDF, and a `--convert-to
  'pdf:calc_pdf_Export'` probe with `UseTaggedPDF` forced off, to confirm the flag is what moves it.
- Rule: an authored probe workbook with one very long string at A1 and a narrow print area, so the
  string's extended width crosses a page break; count words on page 2 of the reference.
