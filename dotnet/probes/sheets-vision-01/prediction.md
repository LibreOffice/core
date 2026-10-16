# sheets-vision-01 — prediction, committed before measuring

Written before the sweep was run. Recorded so the census can be scored against it, and
so what the census *cannot see* is named up front rather than discovered afterwards.

## What I am about to measure

Rank all 171 sheets documents by `|ink|%` on their most divergent page, using look.py's
Rec. 601 luma ink mask at 60 dpi, against the banked 26.2.4.2 references. Then open the
top pages as images and describe them before consulting the record.

## Predictions

**P1 — Ranking composition.** The top of the ranking is dominated by (a) chart-bearing
documents and (b) documents with large filled/shaded regions, not by text reflow. Text
reflow on sheets moves cells within a fixed grid, so it costs little ink; a missing or
mis-sized chart costs a lot. I predict >= 5 of the top 10 are chart or fill defects.

**P2 — `apron-area.xls` page 1.** The standing lead says we draw no grid where the
reference draws 70 vertical and 56 horizontal hairlines. Hairlines are thin, so the ink
cost is small in absolute terms. I predict it scores between 0.3% and 3% `|ink|%`, with a
strongly **negative** signed figure (we draw less), and that it does **not** reach the top
10 — i.e. the largest known lead on this track is invisible to the very ranking I am
building. If it does rank top-10 I was wrong about hairline ink mass.

**P3 — THE BLIND SPOT, and the one that matters most for the stated goal.**
The ink mask is **binary dark/light and therefore colour-blind**. Two charts whose series
colours are swapped, or whose fills are the wrong hue at the same luma, produce an ink
delta of approximately **zero**. The user's goal names colours explicitly ("the axis,
shapes, labels and colours of the chart must match"), so a luma ranking is structurally
unable to rank the defect class I was asked to prioritise.

I therefore predict: **at least 3 documents show a large colour-only chart difference
while scoring `|ink|% < 0.5%`**, and I commit in advance to building a second, chroma-aware
comparator rather than trusting the luma ranking alone. If I only run the luma sweep, I
will have measured the wrong thing carefully.

**P4 — Size mismatches.** Between 0 and 3 documents report a page-dimension mismatch
(look.py returns NaN rather than a percentage for these). These are their own bug and must
not be read as 100% different.

**P5 — Signed direction.** Across the corpus the signed ink figure is net **negative**
(we draw less than the reference) rather than positive, because the standing leads are all
omissions: the missing grid, the overflow grid hole, the accounting-format blanks.

## What this census cannot see at all

- **Colour** (P3): swapped series, wrong fill hue, wrong gridline colour at equal luma.
- **Metadata**: `/BaseFont` naming, embedded-program identity. Zero pixels.
- **Text-layer granularity**: `Tj` vs `TJ` operator splitting. Zero pixels, moves word counts.
- **Sub-pixel geometry**: an 0.08 pt page-size or a 0.5 pt element offset can round to the
  same pixels at 60 dpi, or to wildly different ones. 60 dpi is a ranking instrument, not
  a measurement; anything I intend to *fix* gets re-measured at 150 dpi or in PDF operators.
- **Which side is right.** Ink difference is symmetric about correctness. Only looking says
  which page is the good one.
