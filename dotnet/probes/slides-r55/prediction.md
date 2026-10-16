# slides-r55 — prediction

Committed **before** anything is built or rendered post-change. Environment: LibreOffice
**26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`, base `89798814dda`,
branch `wt-slides-r55`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

## The baseline reproduced

| | briefed | measured |
|---|---|---|
| passing over `MANIFEST.tsv` | 199 of 302 | **199 of 302, 0 disagreements** |
| `abs_ink` | 1147.17 | **1147.14** |
| major pages | 403 | **402** |
| `tf-agreement` mean | 0.77053 | **0.77054** |
| exact `/Tf` pages | 1709 of 4515 | **1709 of 4515** |

The 0.03 of ink and the one major page are a **single document**,
`035_Chemistry_Column_PowerPoint_Chart_45bf8a76.pptx` (1.77 → 1.74, 2 major → 1); every other
row of `ink.tsv` is byte-identical to round 54's final sweep. Reproduced.

## What the round is doing, and why it is not what the brief said

The brief's item 1 is "rotated text — 197 pages where the reference rotates and we do not". That
number is an **artefact of the instrument that produced it**, and this is the *second* artefact in
the same instrument: round 54 fixed a `Tm`-only count that missed our own `cm` route, and left in
place the test that calls a matrix rotated whenever `b` **or** `c` is non-zero. A synthetic-oblique
text matrix is `[1 0 tan(θ) 1]` — `b` zero, `c` not — so **every fake-italic run counted as
rotated**. On `section_1_our_rights_presentation`, the census's #2 document, all 81 "rotated" blocks
are `c = 0.3462535606` and nothing on that deck is turned at all.

`probes/slides-r55/turn-census.py` separates the two. Over the same 302 documents and the same
PDFs:

| | ours | reference | pages ref does and we do not |
|---|---:|---:|---:|
| **turned** text blocks | 1097 | 1318 | **43** |
| **sheared** text blocks | **0** | **587** | **157** |

So 197 = 43 + 157 minus overlap, and the larger half is a different defect: **we never synthesise
an oblique, anywhere, on any track.** Cross-track, over the other two tracks' current reference
PDFs:

| track | reference sheared blocks | ours | pages |
|---|---:|---:|---:|
| slides | 587 | 0 | 157 |
| **words** | **5420** | 0 | **759** |
| sheets | 464 | 0 | 106 |

**6471 blocks, 1022 pages, zero on our side.** That is the largest single measured rendering
defect on the project and it is what this round ships.

The mechanism, with the constant nailed: `LogicalFontInstance::NeedsArtificialItalic()` is
`m_aFontSelData.GetItalic() != ITALIC_NONE && m_pFontFace->GetItalic() == ITALIC_NONE`, and
`pdfwriter_impl.cxx:5767` then does `aMat.skew(0.0, ARTIFICIAL_ITALIC_SKEW)`.
`ARTIFICIAL_ITALIC_SKEW` is `float((1<<16)/3) / (1<<16)` = `0.3333333432674408`, and `Matrix3::skew`
takes it as an **angle**, so the PDF gets `tan(0.3333333432674408)` = **0.3462535606** — which is
the one and only shear value in all 587 slides occurrences.

**Known-answer deck, authored by hand and not round-tripped** (round 54's rule: a fixture built
through the reference inherits the reference's defaults). `make-oblique-probe.py`, five slides ×
three sizes × {roman, italic}, built to separate three rules:

| | H1 face has no italic | H2 by stated family | H3 real italics shear too |
|---|---|---|---|
| Liberation Sans (italic installed) | no shear | — | shear |
| DejaVu Sans (no oblique installed) | shear | — | shear |
| Verdana (not installed → DejaVu Sans) | shear | *its own answer* | shear |
| Liberation Serif | no shear | — | shear |
| DejaVu Serif | shear | — | shear |

Measured: **slides 2, 3 and 5 shear, 1 and 4 do not, on all three sizes — 15 of 15 for H1**, H2 and
H3 refuted. `Verdana` maps to the same `/F3` as `DejaVu Sans` and shears with it, so the answer
follows the **resolved** face and not the stated name. And the `TJ` arrays and the pen origins of
the roman and italic halves are **identical**, which is the fact the whole risk assessment rests
on: a synthetic slant changes no advance and no origin, so **nothing reflows**.

## The predictions

Round 54's own recommendation was to predict a **sign and a rank**, not an `abs_ink` range, because
two rounds running have missed a range by more than 3× in opposite directions. Taken.

1. **Verdict movement on slides: 0.** 199 → 199. Surprise band −1 … +1. A shear changes no page
   count, no embedded font and no glyph advance, so all three gate checks should be inert.
2. **Page counts: 0 of 302 change.**
3. **`abs_ink`: down.** No range. The documents that move are exactly the ones the census names,
   ranked by sheared blocks: `section_1_our_rights_presentation` (81 blocks / 11 pages / 3.36 ink),
   `ws_prod…M.017-(French)-France` (73 / 12 / 18.80), `Structural Testing` (39 / 13 / 6.58),
   `2015-Civil-Rights-Website-training` (39 / 10 / **30.32**, the track's #2 document),
   `attendance-updates-for-governors` (36 / 8), `Fundamentals_Module_1_basics` (25 / 1),
   `Employment-Based_I-485` (22 / 9), `PRM_training` (22 / 4), `outlook_of_nigerian_pension_sector`
   (18 / 12), `ws_prod…Aercap` (16 / 12), `berlin` (15 / 5), `RESPA` (13 / 4), `pods05` (17 / 5),
   `undp` (10 / 4).
4. **Documents moved: 40–52.** 46 slides documents carry at least one reference shear. Fewer than
   40 means our faces at those runs are not the reference's; more than 52 means the change reaches
   runs the census cannot see.
5. **`tf-agreement` and exact-`/Tf` pages do not move**, ±0.0005 and ±5 pages. This is a
   **control**, not a hope: a shear changes no font size, so if this column moves, something other
   than the intended change did it. (Round 54's equivalent control was refuted by its own sweep;
   this one is stated so it can be.)
6. **Cross-track, and it is owed a measurement rather than an argument** — the diff touches
   `XRay.Office.Core` and `XRay.Office.Text`. Both other tracks improve in **sign**.
   Words: 5420 blocks over 759 pages; sheets: 464 over 106. Words verdict movement **−2 … +2** —
   a shear moves no pen origin, but `pdftotext`'s column heuristics see the *glyph* positions, so a
   token boundary could in principle move. Sheets verdict movement **0**.
7. **`a:bodyPr/@vert`, if the round reaches it.** 14 slides documents state a non-horizontal text
   direction on a **slide** part (`NAS-Infrastructure-Roadmaps-v16.0` 118 `eaVert` + 55 `vert270`;
   `-Weather` 16, `-HSI` 11, `Snowbirds_High_Show` 8, `Wildlife for REDAC` 4, `ghgp` 3,
   `Technical_Report_Elements` 2, `Sylva` 2 ×2 copies, `7-Zulkefli` 1, `16 - UTM - (NASA)` 1),
   and 4 `.ppt` documents carry a non-zero Escher `txflTextFlow` (`ws_prod…Approval-of-Flight-
   Conditions` 22, `introduction_to_bea_tuxedo` 4, `Thailand17` 4, `hofman` 1). **250 further
   `eaVert` hits sit on slideLayout parts and are the stock PowerPoint "Vertical Text" layouts,
   almost certainly unused** — counting them would over-reach by a factor of three.

## What these censuses cannot see

Written down before the sweep, per `HANDOVER.md` § 7.

- **Whether our runs at those positions resolve to the face the reference resolved to.** The
  census counts the *reference's* shears. Where we pick a real italic face and the reference picks
  roman-plus-shear, the fix cannot help and the font lists already disagree. Spot-checked on
  `Employment-Based_I-485`, where both sides embed exactly `{DejaVuSans, DejaVuSans-Bold,
  OpenSymbol}` — but that is one document of 46.
- **Whether we will shear where the reference does not.** The census has no column for it, because
  our side is zero everywhere today. This is the regression direction and only the sweep can see it.
- **How much ink a shear is worth.** A shear displaces a glyph by `0.346 × y` above the baseline —
  about 0.12 em at mid-cap-height — so whether a page crosses `pdf-image-diff`'s major threshold is
  not something a block count predicts. This is the term that made rounds 53 and 54 wrong in
  opposite directions and it is still unmodelled. Hence a sign and a rank and no number.
- **The raster backend.** `pdf-image-diff` rasterises our *PDF*, so a Skia text path that does not
  get the same skew would be invisible to every metric in this round.
- **Embedded faces.** A document that embeds its own roman face and asks for italic is a fourth
  case the probe does not cover; the resolver's embedded arm asserts the request's italic onto the
  reference and is left alone.
- **`@vert`'s reach**, if it is reached: the census reads the attribute, not whether the shape
  carrying it draws any text, and it cannot see `wordArtVert` (LibreOffice draws that STACKED, not
  turned) or ODF's `style:writing-mode`.
