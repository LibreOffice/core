# slides/extra-001 — prediction, written before the fix is measured

One document: `FAAAIandtheArtandScienceofV&Vfinal.pptx`. Baseline, measured with the
unfixed binary at `d4a0e5097b3`:

```
slides/extra-001/pptx/FAAAIandtheArtandScienceofV&Vfinal.pptx  pptx  30/30  1189/1133  6/6  0  words  1275/1219
```

Per page, ours against the banked 26.2.4.2 reference: **only pages 13 and 14 differ, by
exactly +28 each**. Every other one of the 30 is exact. 1189 − 56 = 1133.

## The seat, and where the brief is wrong

The brief says we draw the five gauge labels **twice**, once arc-warped and once unwarped,
and that the arc-warped copy is ours and correct. Measured, that is not what happens.

Slides 13 and 14 each hold **two genuinely different sets of shapes**:

* **Set A** — three plain rotated text boxes (`p:sp` id 2 `Analysis`, id 8 `Assistance`,
  id 9 `Augmentation`), no warp, `a:xfrm/@rot` only. `pdftotext -bbox` puts our glyphs for
  these within **0.3 pt** of the reference's, fragment for fragment. These are the "arc-warped"
  copy of the brief — they are not warped, they are rotated, and they are already right.
* **Set B** — four text boxes (ids 42–45) whose `a:bodyPr` carries
  `<a:prstTxWarp prst="textArchUp"/>` or `textArchDown`, holding the five words
  `Assistance`, `Analysis`, `Automation`, `Autonomy`, `Augmentation` — 48 characters.

We read `a:prstTxWarp` **nowhere** (`grep -rn warp dotnet/src` is three unrelated hits), so
set B is laid out as an ordinary text body and drawn as ordinary glyphs. The reference does
something else, and it is not "draws it once":

* `oox/source/drawingml/textbodypropertiescontext.cxx:215-226` — a `prstTxWarp` whose
  `@prst` is anything other than `textNoShape` opens a `PresetTextShapeContext`.
* `oox/source/drawingml/shape.cxx:2202-2211` — the shape is then put into text-path mode by
  `FontworkHelpers::putCustomShapeIntoTextPathMode`.
* Fontwork text is converted to `tools::PolyPolygon` outlines
  (`svx/source/customshapes/EnhancedCustomShapeFontWork.cxx`), so it reaches the PDF as
  **filled paths, not as text**.

Measured on the installed 26.2.4.2 rather than assumed from the 27.2-alpha tree:

* the reference's page-13 content stream holds **597 `c` curve operators** where ours holds 4;
* `pdftotext -raw` on the reference page 13 yields set A's fragments and **no `Automation`
  and no `Autonomy` anywhere** — those two words exist only in set B;
* on the *other* corpus document with a real warp,
  `slides/done-009/…/redac-sas-201403-ppt-portfolio-rev-sim.pptx`, the WordArt strings
  `Fuselage Panel Test` and `Fractographic Examinations` are likewise **absent** from the
  reference's text layer (`Fractographic`: 0 occurrences in the whole PDF), including for
  `prst="textPlain"` — so the rule really is `!= textNoShape`, not "only the curved ones".

So the defect is not a duplicated emission. It is that **a Fontwork body is ink in the
reference and text in ours**, and the 28 words a page are the whole of set B's 48 characters
as `pdftotext` tokenises them from per-glyph rotated placement.

## What I intend to do

Read `a:prstTxWarp/@prst` in `PptxTextBody`, carry it on `SlideTextBody`, and stop emitting
a warped body's glyph runs as text.

I have **not** implemented the arch geometry and do not intend to in this round. The reason
is measured, not budgetary: the reference's four Fontwork outlines on page 13 sit at

| label | reference outline bbox (pt) | our unwarped text bbox (pt) | offset |
|---|---|---|---|
| `Assistance` | 507.9–578.6 × 325.0–391.4 | 482.3–552.6 × 288.9–366.2 | (+25.8, +30.6) |
| `Analysis` | 514.7–569.4 × 217.0–275.2 | 502.0–565.9 × 230.5–284.7 | (+8.1, −11.5) |
| `Automation`/`Autonomy` | 393.4–474.2 × 209.4–290.0 | 416.4–514.3 × 226.7–303.6 | (−31.6, −15.5) |
| `Augmentation` | 390.6–478.0 × 314.1–395.2 | 420.4–496.6 × 281.1–364.5 | (−24.2, −31.9) |

— every offset points **outward along the box's own local up (`textArchUp`) or down
(`textArchDown`)**, which is the arch's radial displacement and nothing else. Reproducing it
needs per-glyph outline warping and a per-run transform the IR does not carry. Emitting the
glyphs as outlines *at the positions above* would leave the ink exactly where it is today and
buy only the word count, at the cost of a change to `IDrawingSink`'s contract and glyph-to-path
machinery in the PDF backend.

## Predictions, to be scored

1. **P1 — the seat.** The two pages' +28 is set B and nothing else. Suppressing warped bodies
   gives **1189 → 1133 exactly**, and pages 13 and 14 go to 102/102 and 71/71.
2. **P2 — the gate.** `slides/extra-001` becomes `TOTAL 1 MATCH 1 MISMATCH 0`, 30/30 pages.
3. **P3 — reach, from what resolves.** A grep for `prstTxWarp` over the 163 slides documents
   hits **67**. Sixty-five of those use `textNoShape` only, which is the "no warp" value and
   changes nothing. **I predict exactly 2 of the 163 renderings change** — the extra-001
   document and `done-009/…/redac-sas-201403-ppt-portfolio-rev-sim.pptx` (2 arch shapes on
   slide 6, 3 `textPlain` shapes on slide 7, 11 words in all). Everything else byte-identical
   under a fixed `SOURCE_DATE_EPOCH`.
4. **P4 — no regression.** `slides/done-*` keeps its baseline verdicts. In particular
   `redac-sas-201403` passes today and must still pass with ~11 fewer words; the reference
   does not have those words either, so the fix moves it **towards** the reference.
5. **P5 — ink.** Suppressing set B *reduces* the page-13 raster difference against the
   reference, because misplaced ink counts twice in a diff and absent ink counts once. I will
   measure this rather than assert it; if the diff gets worse, the fix is wrong and I will
   emit outlines at the current positions instead.
6. **P6 — tests.** Fidelity baseline **30 failed of 550** before and after; no other project
   changes count. New tests fail against the unfixed tree.
7. **P7 — the ceiling table.** `TODO.raster-ceiling.md:212` and
   `raster-ceiling-pages.tsv:68` list page 14 of this document as a raster ceiling. It is not
   one — it is this defect — and page 13, which carries the identical defect, is listed
   nowhere. The detector scores condition 3 on the **net** per-page delta, so a defect of ours
   on a page can fake a ceiling (page 14) or, where a real ceiling and an opposite-signed
   defect coincide, hide one. I predict the general fault is measurable: other rows in the
   table are pages whose delta is a sum of a ceiling and something of ours.

## What would overturn this

* If `done-009/redac-sas-201403` fails after the fix, P4 is wrong and the `textPlain` case
  needs separating from the arch cases.
* If a third slides document changes, P3's resolution-based reach was still too optimistic.
* If the page-13 ink diff grows, P5 is wrong and the fix must emit outlines instead.
