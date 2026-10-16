# words-r59 — prediction 2: the automatic font colour, and what makes a background

Committed before any post-change rendering, after the list-label change was measured
(`6160a1bf475`, words 319 of 337, zero movement, zero regressions).

## What the probe settled before the change (`autocolour.py`, 20 authored packages)

Round 58 pinned the threshold over 22 fills. Three things it could not see, each one a branch:

1. **`Color::IsDark()` is not one formula, and 26.2.4.2 still has the second one.**
   `tools/source/generic/color.cxx`:52 special-cases `COL_DEFAULT_SHAPE_FILLING` (`0x729FCF`) and
   asks `GetLuminance() <= 62` for it instead of `GetWCAGLuminance() <= 87`. That colour's WCAG
   luminance is 83 — *dark*, so white text — and its perceived luminance is 151 — *bright*, so
   black. **The reference draws it black**, and draws `6F9BCB` one step away **white**. It is the
   only input in the whole domain that separates the two functions, and round 58's ramp did not
   contain it.
2. **A character highlight is not a background.** A yellow `w:highlight` on a run in a black cell
   is drawn **white**, and a `darkBlue` highlight in a white cell is drawn **black** — the
   highlight is ignored in both directions. `SwDrawTextInfo::ApplyAutoColor` asks the font's
   *back colour*, which is `RES_CHRATR_BACKGROUND` (character shading), and `w:highlight` is
   `RES_CHRATR_HIGHLIGHT`, a different item.
3. **A paragraph shade beats the cell it is in, in both directions.** White paragraph shade in a
   black cell → black text; black paragraph shade in a white cell → white text.
4. **`w:shd` is a pattern, not a fill**, and the blend is exact:
   `clear` 0, `solid` 1000, `pctN` N×10 (with 12→125, 15→150, 37→375, 62→625, 87→875), every
   striped and crossed value 333; `w:color="auto"` is **black**, `w:fill="auto"` is **white**;
   integer division by 1000 per channel. Eight cases, all reproduced: `pct50` auto/auto →
   `#7F7F7F`, `pct25` → `#BFBFBF`, `pct75` → `#3F3F3F`, `diagStripe` and `thinDiagCross` →
   `#AAAAAA`, `pct50` red-over-blue → `#7F007F`. **`w:val="nil"` is not "no fill"**: it is not in
   `CellColorHandler`'s table at all, so it takes the clear branch and paints its `w:fill` — the
   reference fills `nil` + `fill="000000"` black and reverses its text out. We return null for it.
5. **The control holds**: a run stating `w:color="FF0000"` in a black cell is red on both sides.

## The measurement this change is aimed at

`whiteglyphs.py`, over the 337 words paths of the sweep at `6160a1bf475`:

| | glyphs | documents |
|---|---:|---:|
| **SHORT** — the reference draws white and we do not | **5 145** | **48** |
| **LONG** — we draw white and the reference does not | 34 | 2 |

The largest are `Annex-10-…-GCAA` 1 128, `AFS-050-004-F2_0i` **571** (page 2 is 305 of them),
`draft-variation-notice-airbus-…` 451, `SPA-06_mcar_part-6` 415, `028_Unit_Circle_Chart…` 367.

## What I expect

| quantity | baseline | predicted |
|---|---:|---|
| words verdict | 319 of 337 | **0 movement — 319**, band −2 to +1 |
| white glyphs SHORT | 5 145 in 48 | **1 200 – 3 800** |
| white glyphs LONG | 34 in 2 | **34 – 400** — this is the risk, see below |
| filled rectangles on `AFS-050-004-F2_0i` p2 | 5 against 8 | **8 against 8** |
| words renderings whose bytes change | — | **45 – 110** |
| page counts changed | — | **0** |
| extractable words changed | — | **0** |
| font lists changed | — | **0** |

## The reach census — three arms, and it partitions

`darkbg-census.py` resolves every `w:shd` through the blend above and then asks `IsDark()`:

- **A** — dark and we already paint it: **12 497** elements in **86** `.docx`.
- **B** — dark and we paint nothing today, because we read only `w:fill`: **3** elements in
  **1** document, and it is `AFS-050-004-F2_0i`.
- **C** — bright, the control that must not move: 47 686 in 165.

Separately, `shd-census.py` counts the pattern fix's own reach: **156** elements in **15**
documents where we paint nothing today, and **244** elements in **10** where we paint the `w:fill`
and should paint the blend. Those two are different repairs and are counted separately.

## What the censuses cannot see, before the measurement

1. **The 66 `.doc` documents are not examined**, in either census. WW8 states shading as binary
   descriptors neither script parses. `SPA-06_mcar_part-6_and_IS_v2.9` is a `.docx`, but
   whatever the `.doc` share of the 5 145 is, the census reads it as zero.
2. **Neither census can see whether a shaded cell holds text, or whether that text states a
   colour of its own.** A run with `w:color` is not automatic and never moves, so arm A's 12 497
   is an upper bound by a large and unknown factor — which is why the predicted movement is far
   below it.
3. **Table-style conditional shading is invisible to both**, and it points the other way: a cell
   whose fill comes from `w:tblStylePr` states no `w:shd` of its own, so those are *under*-counted.
4. **Chart and frame text is a different code path.** Nine of the 48 short documents are
   `chartset` templates whose white text may be inside a `FrameChart`, which `PageDrawing.RunsIn`
   never sees. If they are, they cannot move and the measured figure will sit at the top of the
   predicted band for the wrong reason.
5. **The page's own background is not modelled at all.** Writer's fallback when no frame supplies
   a brush is `aGlobalRetoucheColor`, the application's document background; we assume white.
   A document setting a dark page background would be reversed out by the reference and not by us,
   and nothing here would say so.

## The risk, stated in advance

The LONG column is the one that can do damage the gate cannot see: turning black text white on a
background that is *not* dark paints it out of the page, and page count, word count and font list
are all unchanged by it. 86 documents carry a dark fill somewhere, so the surface is large. The
control for it is the LONG column itself, which is measured per document and reported unnetted; a
rise beyond a few hundred glyphs is a defect in this change and not a partial success.
