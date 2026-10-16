# slides-missing-01 — prediction, committed before measuring

Group `slides/missing-001`, one document:
`NWD-GLA-Community-Outreach-Day-Oct-2025.pptx`. Page-exact 13/13, 529 words against 638.

## What is already established, and is therefore not a prediction

Stated here so the scoring below cannot claim credit for it.

1. **Ours draws nothing at all for the `subTitle` body on slides 5, 6 and 12.** Measured, not
   inferred: page 12's content stream in our PDF holds the background rectangle, one image, and
   the title's `/F2 43.0016 Tf` run. There is no second `BT…ET`. Per page, `pdftotext`:
   5 → ours 5 / ref 72, 6 → ours 3 / ref 27, 12 → ours 3 / ref 32, and page 4 → 39 / 39.
2. **The seat is `SlideAutofit.Solve`.** Traced through a temporary `Console.Error` in `Solve`
   (`SlideAutofit.traced.cs` in the scratch dir, not committed): the three bodies solve to
   `bestFont` = 0.006666, 0.003410 and 0.003897 against grid font heights of 60.009, 87.987 and
   76.989 pt. `Scaling.Scaled` rounds each to a whole point *before* converting back, so
   60 × 0.006666 = 0.4 pt → `Rounded(0.4)` = 0 → **an em of 0 hundredths of a millimetre**. The
   body is not dropped by a filter; every run in it is scaled to nothing.
3. **The reference's answer is exactly font scale 0.250 on all three.** Read off the banked
   26.2.4.2 PDF: page 12 draws `18.992` where the runs state 77 pt (77 × 0.25 = 19.25 → 19);
   page 5's glyph boxes are 15.9 and 18.3 units tall for stated 52 and 60 pt (13 and 15);
   page 6's are 26.8 / 22.0 / 18.3 for stated 88 / 72 / 60 (22, 18, 15). That is
   `constScaleLevels`' last row and nothing else.

So the brief's steer — *"a stated scale is making us discard the body entirely"* — is **refuted
before this round starts**. `@fontScale` is never read on these shapes: `Scaling.Stated` is
reached only when `body.AutoFit` is false, and `a:normAutofit` sets it true. The stated 25000 is
a red herring; what makes 25000 the marker of the failing slides is that a document whose author
had to scale to a quarter is a document whose overflow is large enough to drive *our* bisection
off the bottom of its range. Slide 4 states 90000 and renders because its overflow is small.

## The predictions

| | prediction |
|---|---|
| **P1** | The minimal fix is a **floor of 0.250 on the search's font scale**, not a change to how `@fontScale` is read and not a guard against a zero em. With the floor in place the three bodies land on 0.250 exactly, because nothing at or above it fits. |
| **P2** | With the floor, our drawn em matches the reference **exactly** on all three slides: 15 and 13 pt on slide 5; 22, 18, 15 and 7 pt on slide 6; 19 pt on slide 12. |
| **P3** | The document's extracted words move from 529 to **within the 2%+3 band of 638** — I predict 620–660 — and its verdict moves to pass. Page count stays 13/13. |
| **P4** | **Reach is small.** Rendering all 163 slides before and after, I predict **3 to 8 decks** change a byte, and **1 to 3** move verdict. A deck has to overflow its box by more than a factor of four before the floor can bite, which is rare outside a deliberately-overfilled placeholder. |
| **P5** | No regression anywhere in `slides/done-*` (15 groups). |
| **P6** | The fidelity suite is unchanged at the baseline established on the unfixed tree. |
| **P7** | The stale ceiling row for this document is **not the only one**. I predict **at least two** further `ceiling` rows in `raster-ceiling-pages.tsv` now measure with ours *below* ref — i.e. have flipped sign — because the table was written before several rounds of missing-content fixes and before the reference binary moved from 24.2.7.2 to 26.2.4.2. |
| **P8** | This fix is **subsumed by, not in conflict with**, the twelve-level `constScaleLevels` walk landing on `wt-slides-text`: that table's last level is 0.250, so a correct port of it makes the floor redundant rather than wrong. I predict the merge is a small textual conflict in one method and no behavioural disagreement on these three slides. |

## What would refute the approach

If the reference's drawn size on any of the three slides is *not* stated × 0.250 — if it were,
say, a continuous fit — then a floor is the wrong shape of fix and the whole twelve-level table
has to come across. Point 3 above already rules that out on this document; P2 is the test of
whether the floor alone reproduces it through our own metrics.
