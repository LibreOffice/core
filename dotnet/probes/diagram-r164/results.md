# diagram-r164 — the eight words-track diagram/chart templates that "draw more text"

**Reference throughout: `/opt/libreoffice26.2/program/soffice`, LibreOffice 26.2.4.2
(`0229ac93fcf0d7cbc6376066c6f35021cef002dc`), put FIRST on `PATH` for every call**
(`sweep.sh` prints the resolved path and version on every run; `/usr/bin/soffice`, 24.2.7.2,
was never used). Our half is the frozen snapshot `/home/user/cli-frozen-r164/XRay.Office.Cli`;
nothing in the tree was built, and no C# file was changed.

## Headline

**All eight are one cause, and it is not "we invent text".** Our text layout is
**character-for-character identical to the reference's** on all eight pages. The entire gate
delta is a **PDF clip path the reference emits around a fixed-height shape's text and we do
not** — the reference lays out exactly the same lines we do, then masks the part that falls
outside the shape's text area. A clip-honouring extractor (MuPDF/PyMuPDF, which the gate uses)
drops the masked glyphs; a clip-blind one (poppler) does not.

This is a **real rendering defect**, not a ceiling: the reference paints less ink than we do
(measured below). But it is the *opposite* of the gate's reading — nothing is invented, nothing
is duplicated, no line is wrongly formatted.

## 1. Counts confirmed `[bin]`

`census.py` over `sweep.sh`'s renders reproduces the brief's table exactly: 885/832, 470/400,
1282/1253, 402/300, 130/102, 150/120, 184/153, 493/475. One page each side, on all eight.

## 2. The outline census — **hypothesis refuted, all eight** `[bin]`

`census.py`, after `probes/odt-split-r82/overdraw.py` and `probes/odp-chart-r72/classify.py`.
`ref_fill` / `our_fill` are glyph-sized one-colour filled paths (`<20pt` box); `residual` is
`ours − ref_chars − (ref_fills − our_fills)`, which would be ~0 if the reference were outlining.

| document | ours | ref | delta | ref fills | our fills | fill delta | residual | our rot | ref rot |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 011_Project_Timeline_Beautiful | 885 | 832 | +53 | 0 | 0 | 0 | **+53** | 0 | 0 |
| 017_Project_Timeline_Customizable | 470 | 400 | +70 | 14 | 14 | 0 | **+70** | 0 | 0 |
| 050_Visual_Product_Roadmap | 1282 | 1253 | +29 | 0 | 5 | −5 | **+34** | 0 | 0 |
| 055_Organogram_Horizontal | 402 | 300 | +102 | 28 | 28 | 0 | **+102** | 0 | 0 |
| 059_Disease_Concept_Map | 130 | 102 | +28 | 0 | 0 | 0 | **+28** | 0 | 0 |
| 061_Nursing_Concept_Map | 150 | 120 | +30 | 4 | 0 | 4 | **+26** | 0 | 0 |
| 064_Foot_Reflexology_Chart | 184 | 153 | +31 | 0 | 0 | 0 | **+31** | 0 | 0 |
| 068_Work_Breakdown_Green | 493 | 475 | +18 | 0 | 0 | 0 | **+18** | 0 | 0 |

`ours ≈ ref_chars + ref_fills` holds **nowhere**. The reference has essentially the same small-fill
count as we do, and **zero** rotated text lines on either side — the outlining rule
(`VclProcessor2D::RenderTextSimpleOrDecoratedPortionPrimitive2D`, `abs(fontScaling.getY() *
fShearX) < 1`) needs a sheared run and there is not one on any of the eight pages.
`census-sens.py` re-runs the fill count at thresholds 8/14/20/30/50 pt and at every threshold the
two sides agree; total path counts are close too. **No document here is the outlining ceiling.**

## 3. What it actually is — clip-aware vs clip-blind extraction `[bin]`

`poppler-census.py`. Same PDFs, read by MuPDF (honours PDF clip paths when extracting) and by
poppler `pdftotext` (does not).

| document | ourMu | refMu | delta | ourPop | refPop | delta |
|---|---:|---:|---:|---:|---:|---:|
| 011_Project_Timeline_Beautiful | 885 | 832 | +53 | 885 | 885 | **0** |
| 017_Project_Timeline_Customizable | 470 | 400 | +70 | 470 | 470 | **0** |
| 050_Visual_Product_Roadmap | 1282 | 1253 | +29 | 1282 | 1282 | **0** |
| 055_Organogram_Horizontal | 402 | 300 | +102 | 402 | 402 | **0** |
| 059_Disease_Concept_Map | 130 | 102 | +28 | 130 | 130 | **0** |
| 061_Nursing_Concept_Map | 150 | 120 | +30 | 150 | 150 | **0** |
| 064_Foot_Reflexology_Chart | 184 | 153 | +31 | 184 | 184 | **0** |
| 068_Work_Breakdown_Green | 493 | 475 | +18 | 493 | 493 | **0** |

Not just equal counts: under `pdftotext -layout` the **alphanumeric character multiset is
identical on all eight**, and the **word multiset is identical on seven** (068 differs only
because the reference's own extraction splits `Management` into `M` + `anagement`).

`clips.py` shows the mechanism directly — the reference emits one clip rectangle per shape text
body and we emit almost none:

| document | ref clips | our clips |
|---|---:|---:|
| 011 | 27 | 0 |
| 017 | 17 | 0 |
| 050 | 10 | 4 |
| 055 | 34 | 0 |
| 059 | 21 | 0 |
| 061 | 12 | 0 |
| 064 | 29 | 1 |
| 068 | 5 | 0 |

The glyph-level fingerprint (`cmp.py`, and the character dump in the round's log): on a line the
clip edge cuts through, the reference keeps **exactly the glyphs with a cap or an ascender** and
drops the x-height and descender ones — `Symptoms` → `S`,`t`; `Treatment` → `T`,`t`,`t`;
`Etiology` → `E`,`t`,`i`,`l`; `Education` → `E`,`d`,`t`,`i`. Same font, same size, same x to
0.1 pt. Lines entirely below the edge are absent altogether. That is a mask, not a layout
difference.

## 4. The clip is visible, so this is a rendering defect too `[bin]`

Fixture `wrap-1para-h24` (a 24 pt box whose second line straddles the bottom edge). Row ink
profile at 400 dpi across the reference's clip bottom (92.4 pt):

```
   y(pt)    ref  ours
   92.10    200   201
   92.64     69   225     <-- reference clips here, we paint through
   93.18     53    53     <-- body text below the shape; identical again
```

The reference loses ~156 px of shape-text ink at that row and we do not. On the corpus pages the
overflow often happens to be white-on-white (059) and so invisible, but the mechanism paints ink
the reference hides. Whole-page pixel diffs (`inkdiff.py`) run 0.6–2.9 % of pixels.

## 5. The exact rule, measured `[bin]`

Fixtures in `fixtures/`, built with `probes/words-extra-01/mkdocx.py` so that **every fixture
carries a `word/settings.xml` part** (verified in the zip listing).

**When the clip is emitted.** Only when a fixed-height body's content exceeds its text area:

| fixture | refMu | refPop | ref clips |
|---|---:|---:|---:|
| `dml-h15-n6-overflow` (overflows) | 1 | 1 | 2 |
| `dml-h15-n6-clip` | 1 | 1 | 2 |
| `dml-h15-n6-noattr` | 1 | 1 | 2 |
| `dml-h15-n6-normauto` | 1 | 1 | 2 |
| `dml-h15-n6-spauto` (grows to text) | 6 | 6 | **1** |
| `dml-h60-n6-overflow` (overflows) | 6 | 6 | 2 |
| `wrap-1para-h60` (fits) | 12 | 12 | **1** |

(`1` is the page clip alone.) So: a body that **grows to text** (`a:spAutoFit`) is never clipped,
and a fixed-height body is clipped only when its laid-out text does not fit. This matches
LibreOffice's own `if (!aClipRange.isInside(aContentRange))` guard
(`editeng/source/editeng/impedit3.cxx`:4174-4181) `[src]`.

**The rectangle.** Box at x 72..172, y 72..96 pt; `ins-*` fixtures vary `lIns/tIns/rIns/bIns`:

| fixture | insets (h/v) | reference clip rect |
|---|---|---|
| `ins-zero` | 0 / 0 | (72.0, **72.0**, 172.0, **96.0**) |
| `ins-default` | 7.2 / 3.6 | (72.0, **75.6**, 172.0, **92.4**) |
| `ins-big` | 18 / 9 | (72.0, **81.0**, 172.0, **87.0**) |

**Vertical insets are applied; horizontal insets are not.** The clip is
`(shape.Left, shape.Top + tIns, shape.Right, shape.Bottom − bIns)`. Only the vertical bound is
load-bearing on the corpus.

**`vertOverflow` is irrelevant.** `overflow`, `clip` and the attribute being absent all render
identically (rows above) — and all eight corpus documents state `vertOverflow="overflow"`, never
`clip`. `normAutofit` does not shrink; it behaves exactly as `noAutofit`. Both confirm the
existing notes on `PageFrame.HasFixedHeight` `[src]`.

## 6. Our line truncation is already right

`FrameLayout.Content` (`dotnet/src/XRay.Office.WordProcessing/Layout/FrameLayout.cs`:1264) applies
`FlowLayouter.Truncated(flow, inside.Height)` when `PageFrame.HasFixedHeight` `[src]`, and the
DrawingML readers set it (`DocxFrames.cs`:338 and :839, `box is not null && !GrowsWithText`).
Every DrawingML fixture above shows **ours == reference** on the number of lines formatted. On the
corpus pages neither side truncates, and that is correct: e.g. 059's "Signs and Symptoms" shape has
a 25.1 pt text area and an 18.6 pt line height, so line 2's top offset (18.6) is below the content
height (25.1) and the rule keeps it — it is then the *clip*, not truncation, that removes it from
the reference's text layer.

## 7. Second, separate defect: the VML reader never sets `HasFixedHeight` `[src]` `[bin]`

| fixture | refMu | refPop | ourMu | ourPop |
|---|---:|---:|---:|---:|
| `vml-h15-n6` (overflows) | 1 | 1 | **6** | **6** |
| `vml-h60-n6` (fits) | 6 | 6 | 6 | 6 |

`DocxVmlFrames.cs` builds `PageFrame` in three places (:351, :467, :567) and **none sets
`HasFixedHeight`**, so a VML `v:textbox` never truncates. None of the eight corpus documents
exercises it (068 is VML, but both of its lines pass the "top offset < content height" test, so
no truncation is due there and its whole delta is the missing clip). Worth its own round.

Also noted at `DocxVmlFrames.cs`:598: `Padding = box is null ? default : default` — a VML text box
gets zero padding whatever `v:textbox/@inset` states.

## 8. Refuted / ruled out

- **Outlining ceiling** — refuted for all eight; section 2. Zero sheared or rotated runs.
- **`vertOverflow="clip"` / `style:overflow-behavior`** — no document states it (all say
  `overflow`), and it is measured to make no difference in 26.2.4.2.
- **Autofit we do not apply** — no `a:normAutofit` and no `a:spAutoFit` anywhere in the eight;
  all their bodies are `a:noAutofit`. And `normAutofit` does not shrink in the reference anyway.
- **Placeholder / hidden / `<w:vanish/>` text** — zero `<w:vanish>`, zero `<w:placeholder>`, zero
  hidden-layer shapes across all eight.
- **SmartArt double-draw (`dsp:spTree` vs the data model)** — no `diagram*` parts in any of the
  eight; none is SmartArt.
- **Our text being wrong in any way** — refuted by exact character-multiset identity under a
  clip-blind extractor on all eight.

## 9. Not settled

- 068's shape sits ~3.5 pt higher and ~7 pt further left in our render than in the reference
  (header box at y 13.5/x 27.0 vs 17.1/34.3). Unrelated to the character count and not diagnosed.
- 064's band around y 360–375 has a very different background between the two renders (bg 196 vs
  0 in a crop) — a fill difference, not text. Not diagnosed.
- Whether the clip's horizontal bound is really the shape width or is unbounded in x and merely
  reported as the shape width by MuPDF's scissor. Indistinguishable on this corpus.
- No page was read visually, by me or by a subagent: every claim here is arithmetic over the two
  PDFs (character multisets under two extractors, clip rectangles, row ink profiles). No visual
  reading is relied on, so no reading needs corroborating.

## Files

| file | what it is |
|---|---|
| `sweep.sh` | Renders both sides; prints the resolved `soffice` and its version. |
| `docs.tsv` | The eight stems and their corpus paths. |
| `census.py` | The outline census (section 2). Importable; `main()` guarded. |
| `census-sens.py` | The same at five glyph-size thresholds, plus total path counts. |
| `poppler-census.py` | **The decisive measurement**: MuPDF vs poppler on both sides. |
| `clips.py` | Clip rectangles on each side. |
| `cmp.py` | Tolerant line-level glyph diff; shows the ascender-only survival pattern. |
| `dumptext.py` | One text line per row, with position, for eyeballing a single page. |
| `inkdiff.py` | Whole-page pixel diff and the worst 20-row band. |
| `fixture.py` | Builds `fixtures/` (via `words-extra-01/mkdocx.py`, so `word/settings.xml` is present). |
