# Round 190 — a slide's line break charges its trailing blank, and this tree hangs it

Measured 2026-09-28 against **LibreOffice 26.2.4.2** (`/opt/libreoffice26.2/program/soffice`),
corpus `/home/user/sample-files`, `SOURCE_DATE_EPOCH=0`.

The seat is task #23, `slides/done-012/pptx/Intersil_Italy_CAN_Bus_Transceiver_Presentation_Final.pptx`,
which the r186 chart sweep put at 18.08 `diff%`. **That figure is not a chart's.** The deck holds
one chart part and it is on slide 30, which scores 5.38; the 18.08 is slide **37**, a bulleted
text slide, and 15.91 is slide 13. A sweep that ranks a document on its worst page attributes
that page to whatever the document was selected for, and three task descriptions in this queue
have now turned out stale for reasons of that shape.

## What the page shows

A blind reading of the composed pair (`page-vision`, a fresh subagent, no repo access) named two
differences and got both right: the reference wraps bullet 3 one word earlier and needs three
lines where we need two, and **our leading is 9-11% looser at the same glyph size**. It also
named the right candidate for the second — *"`normAutofit` `lnSpcReduction` honoured by one side
and ignored by the other … the reference's text is the taller content (13 lines) yet the tighter
spacing"*.

The operators give both exactly. Baseline pitch: ours **31.27 pt**, the reference's **28.15**, at
25.99 pt in both — and `31.27 × 0.9 = 28.14`. That 0.9 is `constScaleLevels[0]`'s fourth column
(`editeng/source/editeng/impedit3.cxx`:286-300), whose first column is `1.000`: the reference's
first answer to an overflow is tighter leading at full size, applied as
`nHeight = round(GetHeight() * fSpacingY)` in the `InterLineSpaceRule::Off` arm (`:1584-1600`).
So **the reference is in autofit and we are not**, and the reason is the extra line.

## It is NOT a trailing blank, and the first cut of this probe said it was

Identical lines measure identically on the two sides, so it is not a metric difference at the
scale of a word — over five lines the widths agree to **0.6 pt**:

| line | ours | reference |
|---|---:|---:|
| `applications and single point failures must be avoided` | 798.01 | 797.68 |
| `will use two CAN transceivers in parallel` | 591.77 | 591.57 |
| `transceiver malfunctions` | 365.60 | 365.53 |
| `system supply rail > 2MΩ (typical)` | 505.84 | 506.44 |
| the title | 363.88 | 363.46 |

The body placeholder is `<a:off x="609599"/><a:ext cx="11012681"/>` — x 48.00 pt, right edge
**915.14**. Our accepted line `• One transceiver will be active while the other transceiver` ends
at **906.82**; the reference refuses it and ends at 740.57 after `other `.

`widths.py` rewrites `cx` alone:

| placeholder right edge | the reference's line | its right edge |
|---:|---|---:|
| 907.0, 915.0, 915.14 (as authored) | `…while the other ` | 740.57 |
| 915.6, 916.14, 924.0 | `…the other transceiver ` | 915.70 |

**Read on its own that table says the trailing blank is charged** — the boundary is between
915.0 and 915.6, the candidate's own extent with its blank is 915.70, and without the blank it is
906.82, which is below every edge at which the reference still breaks. That is what the first cut
of this probe concluded, and **it is wrong, because the placeholder is not the text area.**

### The master states a 7.2 pt right inset, and both renderers honour it

`<a:bodyPr vert="horz" lIns="0" tIns="45720" rIns="91440" bIns="45720" rtlCol="0">` on the
master's body placeholder. The slide's own and the layout's are bare `<a:bodyPr/>`. So the text
area is 48.00 … 907.94 and the available width is **859.94**, against our visible candidate of
858.83: it fits by **1.11 pt** and the reference breaks it anyway.

Both sides honour the inset, which is what makes the next sweep a statement about the break rule
rather than about the box — at `rIns="182880"` (14.4 pt) the reference ends the line at 740.57
and this tree at 731.68, and at `rIns="0"` the reference ends it at 915.70 and this tree at
906.82.

### How much more than its visible text a line must be given: (1.11, 2.01] pt

`insets.py` rewrites that one attribute:

| `rIns` | available | the reference |
|---:|---:|---|
| 0.0 | 867.14 | keeps `transceiver` |
| 0.5 | 866.64 | keeps |
| 1.0 | 866.14 | keeps |
| 3.6 | 863.54 | keeps |
| 4.5 | 862.64 | keeps |
| 5.4 | 861.74 | keeps |
| **6.3** | **860.84** | **keeps** |
| **7.2** | **859.94** | **breaks** |
| 14.4 | 852.74 | breaks |

So the reference needs between **1.11 and 2.01 pt** more room than the line's visible text — about
**0.2% of the line**. The trailing blank is **8.26 pt** at 25.99 pt in this face, four times that
and outside the bracket. *"The blank is charged"* is refuted by the same document that suggested
it, once the inset is in the arithmetic.

## The trailing-blank rule was implemented anyway, and the slides track refuted it

Before the inset was found, `LineFiller` was given a `chargesTrailingBlanks` mode — the candidate
line judged by its width *to the break opportunity* rather than to `VisibleEnd` — and
`SlideTextLayout` set it. On slide 37 it is exact: all thirteen of our baselines become the
reference's, to the hundredth.

Over the whole slides track it is a clear regression. Our half rendered twice, at the round's base
and with the mode on, one output directory per document: **210 of 302 documents move**, and
scoring every mover against a freshly rendered 26.2.4.2:

| | before | after |
|---|---:|---:|
| worst-page `diff%`, summed over 210 | 1750.8 | **1950.8** |
| every page's `diff%`, summed | 15405.8 | **16839.0** |
| MAJOR pages | 280 | **401** |
| documents whose worst page improves | — | 16 |
| … worsens | — | **109** |
| … level | — | 85 |

Both formats regress (`.pptx` 8 better / 60 worse, `.ppt` 6 / 27 over the first 175), and the
worst movers are large: `ws_prod-g-doc-Events-2007-may-presentation` 4.79 → 16.01,
`attendance-updates-for-governors` 8.45 → 18.68, `dhs-293364` 9.63 → 19.30 — the signature of an
extra line tipping a body into autofit where the reference does not shrink.

**Reverted.** Nothing in `dotnet/src` carries it.

## What is left, and what not to re-derive

- **Do not re-derive "the reference charges the trailing blank" from slide 37.** The width sweep
  alone says it and the inset sweep refutes it; the two together are the finding.
- The residue is **1.11 to 2.01 pt on an 859.94 pt line**. Candidates the measurements here
  cannot separate: EditEngine's whole-1/100 mm arithmetic (`GetCharPosArray` is integer, and
  `ImpBreakLine`'s test is strict `<`); a per-line reserve; or simply that the reference measured
  this one line ~0.2% wider than we do, which is the same order as the ±0.6 pt spread the five
  agreeing lines already show and would mean there is no rule here at all. The third would explain
  the track result exactly.
- **The leading difference on this page is a consequence, not a defect.** The extra line tips the
  body past its autofit box, the reference takes `constScaleLevels[0]` — `{1.000, 1.000, 1.0,
  0.9}`, font unscaled and spacing 0.9, applied as `nHeight = round(GetHeight() * fSpacingY)` in
  the `InterLineSpaceRule::Off` arm (`editeng/source/editeng/impedit3.cxx`:1584-1600) — and draws
  a 28.15 pt pitch where we draw 31.27 at the same 25.99 pt size. Widen the placeholder by 1 pt
  and the reference draws our two lines at our exact baselines, 241.97 and 274.68. **Measure a
  page's line count before its pitch.**
