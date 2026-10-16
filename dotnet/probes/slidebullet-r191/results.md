# A slide bullet's label is measured at the UNSHRUNK size, and it pushes the first line right

Measured 2026-09-28 against `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2** —
with the tarball's five bundled font confounds moved aside. **Diagnosed and not implemented.**

## 0. Where the question came from, and the census rule it cost

`Sector_Skills_Insights_Advanced_Manufacturing_summary_slide_pack.pptx` sits at 15.93 worst-page
`diff%` in the chart ranking, so a brief could easily read it as a chart seat. It is not: its worst
page is **13**, which holds no chart at all — the chart is elsewhere and pages 11 and 21 are the
document's only MAJOR pages, at 6.70 and 6.13. *Census which page the feature is on before working
the number.*

Page 13's own numbers say what kind of defect it is: `diff%` **15.93** with `|ink|%` **0.14** over
14 regions and a verdict of `shifted`. Almost nothing is drawn differently; a great deal is drawn
in the wrong place.

## 1. What the page does, read blind and then measured

A subagent read the composed pair with no access to the repository (`page-vision`) and reported one
substantive divergence: *"the reference applies a +16 px (≈1/8 inch) first-line indent to every
level-3 (dash) paragraph that ours does not … the dash marker itself does not move"*, cascading into
one extra wrap, one extra line, everything below pushed down one line pitch, and the last line
clipped by the slide's bottom edge in the reference and not in ours. It measured the shift by
cross-correlating line strips: **+16 px on every such first line and exactly 0 on every other line**,
and it confirmed the two renderings use the same face at the same size (the identical string is
1055 px wide in ours and 1056 in the reference).

The operators agree and put a number on it. The master's body list states
`lvl2 marL="742950" indent="-285750"` — a 58.5 pt margin with the bullet 22.5 pt to its left — and
the text area's left edge is at x = 21.37:

| | bullet x | first line x | continuation x |
|---|---:|---:|---:|
| ours | 57.37 | **79.87** | 79.87 |
| 26.2.4.2 | 57.37 | **88.50** | 79.88 |

A flat **8.63 pt**, on every second-level paragraph of the page, with the bullet glyph itself in the
same place on both sides. The cascade is visible in the operators too: our line fits 94 glyphs where
the reference's fits 92, so the reference needs three lines for a paragraph we set in two and every
paragraph below it is one pitch lower.

## 2. The rule, pinned by ten one-attribute variants

`variants.py` rewrites one attribute at a time. Reading the same three x off each rendering
(`firstline.py`), with everything relative to the text area's left edge:

| variant | marL | indent | bullet | first line | predicted |
|---|---:|---:|---:|---:|---:|
| as authored | 58.5 | −22.5 | 36.0 | **67.13** | 67.12 |
| `ind0` | 58.5 | 0 | 58.5 | **89.63** | 89.62 |
| `ind-9pt` | 58.5 | −9 | 49.5 | **80.62** | 80.62 |
| `ind-36pt` | 58.5 | −36 | 22.5 | **58.5** (= marL) | 58.5 |
| `marL-78.7pt` | 78.75 | −22.5 | 56.25 | **87.37** | 87.37 |
| `buW` | 58.5 | −22.5 | 36.0 | **88.87** | 88.85 |
| `buI` | 58.5 | −22.5 | 36.0 | **58.5** (= marL) | 58.5 |

**Five of five exact and two clamped as predicted**, with no free parameter, under

```
firstLineTextX = max(marL, marL + indent + advance(bullet))
```

and the advance taken **at the run's stated size, 56 pt, not at the 14 pt the text is drawn at**:
the en dash is 0.5562 em → 31.15 against 31.13 measured, `W` 0.9438 em → 52.85 against 52.87, and
`i` 0.2222 em → 12.43, which lands inside the 22.5 pt hanging width and is therefore clamped away.

***The unshrunk size is the finding.*** The body states
`<a:normAutofit fontScale="25000" lnSpcReduction="20000"/>` and its runs state `sz="5600"`, so the
text is drawn at 56 × 0.25 = **14 pt** — and the bullet's *label width* is 56 pt's. The paragraph's
own geometry is unscaled too (`marL` 58.5 and the bullet at 36.0 are the stated values on both
sides), which is right; what is not obvious is that the label's width is measured in the unshrunk
font while the glyph is drawn in the shrunk one.

**Two variants that decide nothing, and both are worth knowing.** The bullet's own stated size —
`<a:defRPr sz="2800">` on the same `lvl2pPr` — moves the offset **not at all** at 14, 28 or 56 pt,
so the label takes the *run's* size and not the level's. And rewriting the slide's `fontScale` to
50 % or deleting it outright also moves nothing, because **LibreOffice recomputes the autofit rather
than honouring the stated scale** — all three renderings draw the text at 14.00 pt. So a probe that
tries to vary the shrink through the file cannot; the unshrunk size has to be read off the runs.

## 3. A generated NUMBER is the exception, and one corpus deck is what found it

The rule above holds for a fixed character. It does **not** hold for a generated number, and the
first confinement sweep is what said so: of the ten slides documents that moved, nine improved and
`30-04-2021 merged NDoH and NICD_Presentation HBV BD meeting 05May2021_1.pptx` went **85.02 → 90.72**
summed `diff%`, all of it on page 9 (5.00 → 10.70). That page numbers its second level, and its five
first lines move from 63.01 to 65.74 where 26.2.4.2 draws **62.90** — so the reference measured that
label at the size it *drew* it, not at the unfitted one.

The reading that fits is the cache's own key. `Paragraph::IsBulletInvalid` compares the bullet's
**text** as well as the scaling parameters, so a generated number — whose text differs on every
paragraph — refills the cache while the search already has a scale, where a fixed character never
changes and is measured once, before it. Restricting the rule to a character bullet keeps every gain
and removes the loss: `Sector_Skills` stays at 131.47 and `30-04-2021` returns to 85.02, exactly its
base figure.

## 4. What it is worth

`sweep.py` renders one document per directory under `SOURCE_DATE_EPOCH=0`, three workers, at the
round's base and with the change; `movers.py` diffs the two legs and refuses to print a total unless
every document on the list rendered in both. The slides track is the whole reach by construction:
`SlideTextLayout` has no consumer outside `XRay.Office.Presentations`, and the two references to it
from `XRay.Office.Text` and `XRay.Office.WordProcessing` are doc comments.

**9 of the 302 slides documents move and 293 are byte-identical**, and all nine improve: summed
`diff%` over them **1148.05 → 1121.17**, MAJOR pages 14 → 14. The number-bulleted deck of §3 is not
in the mover set at all — its rendering is byte-identical to the base — which is the restriction
working rather than an absence of coverage.

Scored against a freshly rendered 26.2.4.2 with `pdf-image-diff.py`, over the first sweep's ten
movers (the unrestricted rule, so the one number-bulleted deck is in it):

| document | worst `diff%` | summed `diff%` |
|---|---|---|
| `Sector_Skills_Insights_Advanced_Manufacturing…` | 15.93 → **13.13** | 139.74 → **131.47** |
| `Liturgical-Commission-2025-Convention-Presentation` | 15.40 → **11.27** | 28.31 → **24.18** |
| `chapter_4_0` | 14.79 → **14.46** | 220.18 → **218.76** |
| `5b_upasana_dasgupta_-_liability_and_registration` | 9.98 → 10.27 | 50.46 → **47.42** |
| `FAA_Form_337` | 11.60 → 11.60 | 346.68 → **342.92** |
| `joint_user_outcomes_michael_fullerton_29.06.12` | 18.93 → 18.93 | 99.75 → **98.03** |
| `RESPA_-_Section_8_Webinar` | 8.98 → 8.98 | 96.66 → **94.89** |
| `ghgp-supply-chain-initiative_20100323_wri` | 13.94 → 13.94 | 143.77 → **143.07** |
| `BMFE-06-03 (Gerflor) Smoke Density and Toxicity` | 13.72 → 13.72 | 22.50 → **20.43** |
| `30-04-2021 merged NDoH and NICD…` | 14.94 → 14.94 | 85.02 → 90.72 → **85.02** |
| **total over the nine the shipped rule moves** | | **1148.05 → 1121.17** |

MAJOR pages 15 → 15. One worst page rises — `5b_upasana` 9.98 → 10.27 — while that document's sum
falls by 3.04, which is a paragraph whose wrap changed on a page that was already the best of its
document.

**No gate column can see any of it.** A changed wrap keeps the same characters, and a slide's page
count is its slide count.

## 5. And a second lead from the same deck's page 11, read blind and not measured

A second uncontaminated reading of page 11 (6.70 `diff%`, MAJOR) found the text identical — *"same
text, same line breaks, same line positions to ±1 px, same box widths, same colours"* — and three
differences that are all in the drawing of shapes:

- **Our drop shadow is a hard, flat, straight-down bar.** On the page's third teal box ours is a
  uniform RGB 153 band 4 rows deep with **zero** spread to the left, the right or above, where
  26.2.4.2's ramps 157 → 249 over about 11 rows below, 8 px to the right, 7 px to the left and 3 px
  above — a blur of roughly 4–6 pt offset a little down **and to the right**. Ours is offset
  straight down by ~2.2 pt with no blur at all, which is what an ignored `a:outerShdw/@blurRad` and
  `@dist`/`@dir` look like.
- **A shadowed white container around the body paragraph is drawn by the reference and not by us** —
  or is drawn and invisible, because under a no-blur no-lateral-offset shadow model the only strip
  that could show is covered by the box below it. The reading names the discriminator: re-render on
  a non-white ground, or hide the covering box.
- **Our first teal box is 11 px (6.1 pt) taller than the reference's**, which makes ours the better
  page: the reference's box is too short and its last line's white glyph bottoms fall onto white
  paper, leaving only the orange hyperlink underline visible below the fill. A shape-height or
  autofit question, and the one difference on that page where the reference is the wrong one.

None of the three is measured yet. `dotnet/CLAUDE.md` already records that **a blurred shadow is
rasterised by LibreOffice** — its PDF holds a picture with no words where a hard shadow stays vector
and searchable — so the blur is not only cosmetic and the text layer is the cheap instrument for it.
