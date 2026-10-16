# sheets-wrap-01 — prediction, written before measuring

## The defect

`sheets/batch-006/xlsx/Published_Issuances_2024.xlsx`, verdict `words`, 457/479.
Column F ("LINK") holds a bare URL in each of 22 rows. The reference wraps each onto
two lines, breaking mid-token (`…Published%2` / `0Issuances…`); we draw one line and
clip it at the cell border.

## What the file actually contains

`xl/worksheets/sheet1.xml` carries `<hyperlinks>` with 22 `<hyperlink ref="Fn" r:id="…"/>`
entries — sheet-level hyperlinks, not inline runs. Every F cell's `xf` (9, 11, 19, 20)
sets `wrapText="1"`. Calc's OOXML import (`sc/source/filter/oox/worksheethelper.cxx:1062`
`insertHyperlink`) turns each such string cell into an **edit cell holding one
`SvxURLField`** — a single `EE_FEATURE_FIELD` feature character in the content node.

## The seat in X-Ray.Office

`dotnet/src/XRay.Office.Spreadsheets/Layout/SheetTextLayout.cs:420`

```csharp
bool breaks = Breaks(format, isValue) && !cell.IsField;
```

A field cell never wraps, whatever its format says. The comment above it cites
`output2.cxx:2560-2567` — `readCellContent`'s `//  Fields aren't wrapped, so clipping is
enabled to prevent a field from being drawn beyond the cell size`, which sets
`rWrapFields = true`. So the port took the C++ comment at its word.

## Prediction

1. **The C++ comment is describing the clip, not a suppression of wrapping.** The installed
   26.2.4.2 binary *will* wrap a wrap-enabled cell whose whole content is a hyperlink field.
   `bWrapFields` only forces `bClip`; `mbBreak` is untouched, so the EditEngine paper stays
   the column width and the text still has to fit it.

2. **The break inside a field is a pure character-level chop at the fitting limit**, with no
   break opportunity honoured at all — not at `/`, not at `-`, not at `%`. Mechanism: the
   break iterator is handed the *content node's* string, and for a field cell that string is
   a single feature character, so it offers no interior opportunity; EditEngine then falls
   through to `ImpEditEngine::ImpBreakLine`'s `// No separator in line => Chop!`
   (`editeng/source/editeng/impedit3.cxx:2236-2247`) and cuts at `nMaxBreakPos`, the last
   character position under the remaining width. That is exactly what
   `…Published%2` / `0Issuances…` looks like — a cut at an arbitrary character where a
   break-opportunity rule would have cut after `Regulations/`.

3. **Therefore the reviewer's probe will separate the candidates as follows:**
   - long space-free token, **plain** cell → wraps in *both* renderers (our
     `TextMeasurer.Chop` already does this), so this arm passes today;
   - long space-free token, **hyperlink** cell → wraps in the reference, **one clipped line**
     in ours. This arm is the defect.
   - long *spaced* text, **plain** cell → wraps at the spaces in both.
   - long *spaced* text, **hyperlink** cell → the reference wraps it, and I predict it
     **chops mid-word rather than breaking at its spaces**, because the field is atomic to
     the break iterator. Ours draws one clipped line.

   So: *not* "the character-break fallback is missing" (it is present and correct in
   `XRay.Office.Text`), but "the run is being treated as atomic" — and atomic in a stronger
   sense than the reviewer's framing, since the reference is atomic to the *breaker* while
   still being divisible by the *chop*.

4. **Reach.** `IsField` is set only for spreadsheet cells that resolve to a hyperlink/URL
   field, so the change cannot touch the words or slides tracks. Within sheets I expect a
   small number of documents to move — hyperlink-bearing workbooks with wrap on and a
   column too narrow for the URL. I predict **1 to 4 documents** change page count or word
   count across the 171, and that `Published_Issuances_2024` is the one whose verdict flips.

5. **The border defects on the same page do not share a seat.** Wrapping decides where glyphs
   go; the grid rules are drawn by `SheetGrid`/`SheetDecoration`, which never consults
   `IsField`. I predict the border findings are unchanged by this fix.

## What would refute each

- (1) refuted if the reference draws one clipped line for a hyperlink-field cell in the probe.
- (2) refuted if the reference's probe break lands after a `/` or a `-` rather than at the
  fitting limit.
- (4) refuted if any words or slides document moves, or if more than ~6 sheets move.
