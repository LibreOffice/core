# Prediction, committed before the sweep

Written at `0fa9bf89f`, before any post-change rendering.

| | predicted |
|---|---|
| sheets renderings byte-changed | **40–80 of 171** |
| verdicts moved on `batch-check.sh` | **0**, and 0–1 if the band is generous |
| words renderings byte-changed | 0 of 200 |
| slides renderings byte-changed | 0 of 163 |

## Why those bands

The reach band is high, and deliberately so, because this census is not the usual
"documents declaring a property" ceiling. The condition it counts — a merge with a covered
cell whose style paints a border or a fill — very nearly *implies* a changed rendering,
because the commonest way a file states a merge is to write the same box on every cell of
it. That is exactly the case whose interior edges we used to draw and now suppress. So I
expect reach to land near the census's 47 over the OOXML half rather than at a fraction of
it, plus some part of the 61 `.xls` the census cannot read — where merged title blocks are
if anything commoner.

The verdict band is 0 and I expect 0. **The gate cannot see this change.** `batch-check.sh`
decides on page count, extractable words and unembedded fonts; a suppressed border line and
a recoloured fill move none of the three. Nothing here touches measurement, so no line
breaks, no row heights, no pagination. If a verdict moves it will be an accident of the
reference's own instability, not this change.

The cross-track figures are 0 by construction and will be checked rather than argued: every
line of the diff is in `src/Paperless.Spreadsheets/Layout/`, which neither other family
compiles against a single call into. They are still measured, because "by construction" has
been wrong here before.
