# sheets-b-01 — prediction, committed before measuring

Written after (a) locating the seven documents, (b) rendering the seven reference PDFs, and
(c) reading `XRay.Office.Core/Charts/ChartScale.cs` — but **before opening a single PDF, before
running `pdftotext`/`pdffonts`/`pdfimages` on any of them, and before unzipping any `.xlsx`.**
Nothing below was retrofitted.

## Standing prediction on the gate

**I expect zero verdict movement from every item in this round.** The gate is page count,
extractable words (2% band) and unembedded fonts. A table border, a chart border, a chart's
vertical axis range, an axis-label rotation, a chart-area outline shape, header-text clipping,
an empty cell's shading, and a hyperlink's colour are all invisible to all three. The only two
items with any theoretical path to the gate are:

- **#5 `T0A0D0000090006XLSE.xls` — text sizing changing wrapping.** Wrapping changes line count,
  line count changes row height, row height changes pagination, pagination moves page count.
  This is the one item that can move a verdict, and it is the one I rate most likely to be a
  duplicate of the already-known row-height axis.
- **#7 `Capability_List… ` — "some cells are taller".** Same mechanism, same caveat.

Everything else is a fidelity defect that no instrument on this project can see. That is the
expected shape of this round and is not a failure of it.

## Per-item predictions

### 1. `Keywords_Mapping_Graphs_and_Charts.xlsx` — chart vertical scale "very different"

- **Prediction:** the divergence is **not** in `ChartScale.CalculateLinear`'s arithmetic. That
  file is a step-for-step port of `ScaleAutomatism::calculateExplicitIncrementAndScaleForLinear`
  and its documented probe (`chart-bar-deck.pptx`, 0–180 over ten ticks) reproduces. I predict
  the divergence is **upstream of the scale resolver**: either (a) the OOXML reader is not
  handing it the stated `c:scaling/c:min` / `c:max` / `c:majorUnit` for this chart (so a stated
  scale is being recomputed as automatic, or vice versa), or (b) the *data* fed to the automatic
  path is the wrong set — a wrong series range, a category axis treated as value, or a
  stacked/percent-stacked plot whose maximum should be the stack total rather than the largest
  single point. Of those I rate **(b), specifically stacking**, most likely, because a
  stacked chart's axis maximum differs from an unstacked one's by roughly the number of series —
  and "very different" is the word a user reaches for at a 2–3× scale error, not a 10% one.
- **Falsifier:** if the reference's axis maximum equals the largest *single* data point rounded
  by the LibreOffice rule, stacking is refuted and (a) is in play.
- I predict the **table border and chart border** items are a separate, simpler seat: a default
  border that LibreOffice draws and we omit (or vice versa) rather than a border we mis-place.

### 2. `Template Pilot Logbook JAR-FCL V3.0.xls` — angled axis drawn horizontal; chart area a rectangle

- **2a (rotation).** Prediction: this is an **automatic** rotation, not a stated one. LibreOffice
  rotates category-axis labels by itself when the labels do not fit side by side at 0°
  (`VCartesianAxis` / `AxisLabelProperties::autoRotate45`). I predict our reader honours only an
  explicitly stated rotation angle and has no auto-rotate step at all, so we draw 0° whenever the
  file states nothing. That would make the seat a *missing* stage rather than a wrong angle.
  Corollary prediction: the reference's angle is **45°** (or −45°), not an arbitrary value —
  because the auto path only ever picks 45.
- **2b (chart area rectangle vs polygons).** Prediction: unrelated to 2a. `.xls` charts carry
  their area as an Escher/`MSODRAWING` shape; I predict our `XlsChartReader` synthesises a plain
  rectangle for the chart area / plot area wall rather than reading the shape's geometry. Lower
  confidence than 2a. It is also possible the user is describing the **plot-area wall** and the
  polygons are a 3-D chart's floor/side walls, which are genuinely polygons and which a 2-D
  renderer would flatten to one rectangle. I rate the 3-D reading a real possibility and will
  check the chart's type before the geometry.

### 3. `grants-2005.xls` — header text not cropped to cell size

- **Prediction:** LibreOffice's rule is not "clip to the cell". For a text cell it is
  *overflow into the neighbour when the neighbour is empty, clip when it is not* — and the
  clipping, when it happens, is to the **column** rectangle, not to the cell's own width, and it
  is suppressed entirely when the cell is centred/rotated/merged. I predict we implement the
  empty-neighbour overflow (that is what `SheetTextOverflow.cs` is for, 7,536 bytes) but that the
  header row's cells here have a **non-empty** neighbour, or are **merged**, and we take the
  overflow branch where LibreOffice takes the clip branch. Seat: the predicate that decides
  overflow-vs-clip, not the drawing.
- **Alternative I rate second:** the cells are wrap-enabled and we ignore wrap, so the text runs
  wide instead of wrapping — which a reviewer would also describe as "not cropped".

### 4. `sectors-defense-and-aerospace.xlsx` — empty cells missing shading

- **Prediction, and this is the item I am most confident about:** we drop the cell **before** the
  style is resolved. A `<c r="B7" s="42"/>` with no `<v>` and no `<is>` is a styled empty cell and
  is the single commonest way a fill reaches a blank region in real files. I predict our
  `XlsxSheetReader` skips cells with no value, so the `s=` index never reaches the decoration
  pass. Second candidate, only if the first is refuted: the fill is inherited through
  `cellStyleXfs` (a named cell style, e.g. a table style or "Accent1") rather than stated on the
  `cellXfs` entry, and our resolver reads only `cellXfs`. Third: row-level (`<row s= customFormat=>`)
  or column-level (`<col style=>`) formatting, which paints a fill with **no `<c>` element at all**
  — that one cannot be fixed by any change to cell handling.
- Note in advance: a census that counts `<c>` elements carrying `s=` **cannot see** the third
  case, and a census that reads `cellXfs` only cannot see the second. I will resolve to the
  effective fill, not to the declaration.

### 5. `T0A0D0000090006XLSE.xls` — text sizing causes different wrapping

- **Prediction:** this is *not* a new axis. I predict it resolves to font metrics — either the
  known fallback-face divergence (we do not pick the reference's `IPAGothic` + `WenQuanYiZenHei`
  pair) or a missing per-character advance rounding — and that the wrapping difference is the
  *visible consequence* of an advance-width difference of a fraction of a point accumulated over
  a long string. I predict the row **heights** on this document are nonetheless exact, exactly as
  they were on the fallback-face cluster, because the height comes from the line count and the
  line count only breaks when the accumulated error crosses a word boundary.
- **Falsifier:** if row heights here are *not* exact, this is a different seat and worth more.

### 6. `ans_mappings_of_eccairs_terms.xlsx` — link colour

- **Prediction:** the smallest item here and probably a one-line default. LibreOffice colours a
  hyperlink from the **document's** link colour, and for a spreadsheet cell the hyperlink
  character colour comes from the `Hyperlink` **named cell style** if the file defines one, else
  from the application default (`#0000EE`-ish blue, underlined). I predict we hard-code a blue
  that is not the reference's, or — more likely and more interesting — we colour from the file's
  `Hyperlink` cellStyle where the reference *ignores* it, or the reverse. I predict the reference
  colour is **not** `#0000FF`.

### 7. `Capability_List…xlsx` — "some cells are taller"

- **Prediction:** I predict this document **is** covered by the known 14-document row-height
  cluster and is not a separate axis — the user's phrase is verbatim the one that identified that
  cluster. Specifically I predict at least one row here is a non-wrapping multi-line row where
  the confirmed-and-unimplemented **3.4 twip** `bStdAllowed = false` adjustment applies.
- **Falsifier, which I will actually test rather than assume:** if no row on this sheet is a
  multi-line non-wrapping row, the 3.4 twip rule cannot be the seat and this is a second axis.
  I will check that before claiming the cluster covers it.

## What my instruments cannot see, stated in advance

- **No CLI.** I cannot render our own output for anything. Every statement about what *we* do is
  therefore **inferred from source**, never measured, and I will label it so throughout. Any item
  whose diagnosis needs our rendered geometry is unresolvable this round and I will say so rather
  than guess.
- **PDF-side blindness.** `pdftotext` gives me text and positions; it does not give me fill
  colours reliably, and a shading drawn as a vector rectangle is invisible to it. For item 4 and
  item 6 I need colour, so I will read the PDF content stream directly (`rg`/`zlib` over the
  decompressed page stream) rather than trust a text extractor.
- **A cell's `s=` index is a declaration, not a resolution.** Stated above per item 4.
- **The reference binary is 26.2.4.2, not the 24.2.7.2 every stored figure was measured against.**
  Any figure I quote is measured fresh from 26.2.4.2 today; I will not compare against a stored
  number without saying that the two binaries differ.
