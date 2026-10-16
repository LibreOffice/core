> **Answered by round 115 — the width is INVENTED, and this file's last section is the one that
> mattered.** 26.2.4.2 lays this deck's 14 pt table text out at DejaVu Sans' advances while drawing
> Liberation Sans' glyphs: the seventh confound (`L1`) reaching the binary format through
> `lfPitchAndFamily = 0x22`. A `.ppt` cannot state character spacing at all — the reference's own
> reader has no field for one. Two one-attribute variants of the deck collapse 26.2.4.2 onto this
> tree to 0.042–0.069 pt. See `probes/ppt-spacing-r115`. **Two numbers below are wrong and are
> corrected there**: the 24 pt heading row compares 39 characters against 40 and the heading is not
> affected at all, and the justified-measure figures are origin-to-origin with a different final
> glyph on each side (like for like it is 3.3 pt, not 7.1).

# O57 is not a line-breaking bug — the reference draws the same string wider

Round 113 seated O57 as *"26.2.4.2 fits 22 lines into a `.ppt` table cell where this tree fits 19,
at the same font and the same size"* and named `architecture6` page 10 as the page worth an
independent reading. A round cannot spawn one here, so the parent did.

## The blind reading

A fresh agent, given `pair10.png` and nothing else — no documentation, no source, no results, not
told what was wrong. Its reading, before any measurement:

- **The reference fits fewer words per line than ours, in every multi-line cell**, at the same
  cell width. Description ours 7 lines / reference 8; Advantages ours 3 / reference 4; and in the
  cells where both take the same number of lines, the reference still breaks earlier.
- **The reference's glyphs look letter-spaced** — it read words as `Nam e`, `com ponent`,
  `Exam ple`, `Advant ages`, `sim ple`.
- It declined to claim a font-size difference from the image, and named the measurement that would
  separate size from advance width.

## The measurement it pointed at

Glyph `origin` values out of both PDFs, so this is drawn position, not extracted grouping.
`widths.tsv`.

**Both sides subset the same five faces** — `LiberationSans`, `-Bold`, `-Italic`, `Carlito`,
`OpenSymbol` — so it is not a substitution.

| identical string | chars | pt | ours | reference | Δ | Δ/char |
|---|---:|---:|---:|---:|---:|---:|
| `Description` | 11 | 14.0 | 77.03 | 89.19 | **+12.15** | +1.105 |
| `Advantages` | 10 | 14.0 | 79.37 | 92.97 | **+13.60** | +1.360 |
| `Disadvantages` | 13 | 14.0 | 98.83 | 116.34 | **+17.50** | +1.346 |
| heading, whole | 39 | 24.01 | 453.10 | 468.00 | **+14.90** | +0.382 |
| heading, first 12 chars | 12 | 24.01 | 148.98 | 149.24 | **+0.26** | +0.022 |

And the reference has **more** room, not less: the justified body lines fill
**478.76–479.51 pt** against our **471.70–472.51**, from the same left edge at 199.05. So the
reference's text box is ~7.1 pt wider and it still breaks earlier — because its text is ~3.3 %
wider than that.

That is the whole of O57: **not a wrap rule, an advance width.** The reference needs more lines
because its own text is wider, and it is self-consistent in doing so.

## A hypothesis this refutes

The obvious mechanism is that the reference measures on a 96 dpi device and rounds each glyph
advance up to a whole pixel — 0.375 pt on average, which over the heading's 39 characters predicts
+14.6 against the measured +14.90, a 2 % match. **It is wrong**, and the same table kills it: the
heading's *first twelve characters* differ by **+0.26 pt in total**, not the ~4.5 pt a uniform
per-glyph rounding would give. The divergence is not spread evenly; it accumulates at particular
points, which matches exactly where the blind reader saw gaps — around `M` and `m`.

## What is not yet established

Whether the extra width is stated or invented. The reference's own flat ODP of this deck contains
**zero** occurrences of `letter-spacing` — so in its own resolved view it is not honouring a stated
spacing. That is suggestive and not conclusive: an ODF round trip can drop what the `.ppt` states,
so the `.ppt`'s own character records still have to be read before anyone says the reference
invented it.

If it did invent it, this is the same family as **L1** (the draw layer measuring in one face and
drawing in another) and as round 111's `RRM 16` finding (the reference drawing DejaVu Sans at
DejaVu Serif's advances, with ours the self-consistent output) — in which case ours is right and
O57 becomes a nil-reach confound rather than a defect. If the `.ppt` does state it, ours is wrong
and it is a reader gap. **Nobody should implement anything until that is settled**, and it is one
record read away.
