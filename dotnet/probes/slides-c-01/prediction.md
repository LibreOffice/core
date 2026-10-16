# Slides-C round 01 — prediction

Written and committed **before any post-change measurement**. Base commit `eab6499c860`
(slides re-baselined to 132/163 at 26.2.4.2 with the corrected font set).

Two changes are in scope:

1. **Escher picture cropping on the `.ppt` path**, with `SlideImages.Uncropped`/`Inset` moved
   down to `XRay.Office.Core` first so `.doc` and `.xls` inherit the arithmetic.
2. **A run's colour from `a:gradFill`**, alpha included, in `PptxTextBody`.

## Verdicts

**Zero verdict movement, on all three tracks.** Slides stays **132 of 163**; words and sheets
stay wherever they are.

This is not hedging, it is what the gate measures. The three checks are page count, extractable
words inside a 2%+3 band, and unembedded fonts. A crop changes where a picture is drawn and how
large; an alpha changes the ink a glyph is painted with. Neither adds or removes a page, a word
or a font. On slides specifically, check 1 is a slide count and is structurally immune. A round
that predicts no verdict movement and turns out to be right is a round that did its arithmetic
before it spent the compute, and I will say so plainly rather than hunting for a verdict to
claim.

The one way this prediction fails is a **cascade** — a crash or a layout exception on a deck
that previously rendered, which would drop its page count to zero. I predict none.

## Renderings changed

Measured by rendering a whole track at the base build and at the branch build and diffing byte
for byte, with `SOURCE_DATE_EPOCH=1700000000 TZ=UTC` so `/CreationDate` is already equal.
(Instrument control run first: `slides/batch-001` rendered twice with the same binary is 9 of 9
byte-identical, so a non-zero count is the change and not the harness.)

| | predicted |
|---|---|
| slides, from the crop (`.ppt` only) | **12–16 of 51 `.ppt` decks** |
| slides, from the run gradient (`pptx` only) | **5–16 of 112 `pptx` decks** |
| slides, total | **17–32 of 163** |
| **words**, from the `XRay.Office.Core` move | **0 of 200** |
| **sheets**, from the `XRay.Office.Core` move | **0 of 171** |

The crop band is set below the previous round's census ceiling of 16 decks / 100 shapes on
purpose. That census counts shapes stating a non-zero crop property in the `PowerPoint Document`
stream, and it cannot see whether the shape is on a master or a notes page, whether it is behind
something else, or whether the deck lays out at all. Every census on this project so far has
been a ceiling and several have overshot the measured floor by a factor of two or more (round
45's 87-deck census produced 2 changed renderings; the previous round's 2-deck corner-gradient
census produced 1). So I expect the crop to land at the **top** of its band — the shapes carry a
`pib`, which is the strongest census this track has had — and the gradient to land at the
**bottom** of its, because 40 declarations across 16 decks are `defRPr` entries in list styles
that may be inherited by no run at all.

The words and sheets zeroes are the load-bearing prediction of this round, because
`XRay.Office.Core` is shared by everything. A pure code move with no behavioural edit must move
nothing. If either is non-zero, the move is not pure and I have to find out why before anything
else. Precedent cuts both ways here: round 45 moved code in `XRay.Office.Text` and swept 334 of
334 byte-identical, round 44 changed `PdfImages` and every affected deck moved. The difference
is that round 44 changed behaviour and round 45 did not; this is a round-45-shaped change.

## What I expect to refute

Every round on this project has refuted its brief's central claim, and I expect to refute
something here too. The three candidates, in the order I think them likely:

1. **The `+1` in `lcl_ApplyCropping`.** The brief instructs "note the `+1`" —
   `(height + 1) × factor + 0.5`. I predict it is **irrelevant to the destination rectangle**
   and must not be ported. It is applied to a *pixel bitmap crop* inside LibreOffice's own
   graphic handling, not to the shape's placed rectangle, and the previous round's own
   measurement is the evidence: plain fraction arithmetic reconciles the reference's
   `733.92 × 586.97` to 0.03 pt with no `+1` anywhere. Porting it would be copying a rounding
   rule out of the wrong coordinate space.
2. **The reach of the crop.** I predict at least one of the 14 "unmistakable" decks does not
   change its rendering at all, because its cropped shape is on a master or notes page we do
   not draw, or is a group child we place differently.
3. **The gradient stop LibreOffice picks.** The brief says the `OnTrac` case is two identical
   stops so the choice does not matter. It does matter for the other 15 decks, and
   `getBestSolidColor` is **not** "the first stop" and **not** "the middle stop" — it is the
   first stop when there are one or two, and the *second* when there are more than two
   (`fillproperties.cxx:410-418`). `DrawingChartPlot.FillOf` already implements a different
   rule — nearest to position 0.5 — deliberately and for charts. I predict the correct rule for
   text is LibreOffice's and that copying the chart heuristic across would be wrong.

## Tests

I expect to add tests that are verifiable by reintroduction — deleting the fix must fail them —
for: `Uncropped` in its new Core home, the four Escher crop property reads, the `.ppt` picture
destination for a known crop, and the `a:gradFill` run colour with its alpha. I expect the
whole-file "the build still has zero warnings" and the per-project counts to be drift guards
only, and I will label them as such.

## Final state expected

`dotnet build -v q -nologo` at 0 warnings, 0 errors; the ten non-Fidelity test projects green at
or above their current counts — Core 284, Containers 109, Text 287 (14 skipped), Vector 295,
Rendering 121 (1 skipped), Markup 259, OpenDocument 125, WordProcessing 761, Spreadsheets 621,
Presentations 592.
