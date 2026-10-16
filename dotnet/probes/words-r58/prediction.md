# words-r58 — prediction, committed before the change

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
corpus `/c/sandbox/workdir/sample-files`; worktree `wt-words-r50` on branch `wt-words-r58`, base
`32f946bf612`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

**Baseline reproduced before anything was written.** `batch-check.sh … 'words/*' … 8` →
`TOTAL 355 MATCH 336 MISMATCH 19`; scored against `MANIFEST.tsv`'s own 337-path list rather than
that total (the extra 18 rows are the case-insensitive mount's alias entries): **319 match, 18 open,
zero disagreements with the manifest's status column, document for document.** The residual shear
figures reproduce round 56's exactly — 39 documents / 1 611 glyphs short, 5 / 916 long, 148 pages
where the reference shears and we draw none, per-face short DejaVuSans 1 348 / WenQuanYiZenHei 177 /
OpenSymbol 112 / DejaVuSerif 86 / DejaVuSans-Bold 46 / DejaVuSerif-Bold 15.

## The claim, and it is measured before it is implemented

**A run whose glyph comes from a fallback face loses its lean.** `SystemFontResolver.ReferenceFor`
is a reverse lookup from a face with no request to compare against, so it cannot set
`SyntheticOblique` and never has; the three call sites that name a substituted face —
`PageDrawing.ByFace` (words), `SlideTextLayout.Block.FontFor` (slides), `SheetFonts.ForFallback`
(sheets) — all go through it.

`fallback-oblique.py` and `fallback-oblique-ooxml.py`: **41 authored packages over six filters**
(`.docx`, `.fodt`, `.fodp`, `.fods`, `.pptx`, `.xlsx`), each one paragraph of two runs so the second
run's sheared-glyph count is the only quantity that can move.

* **The format is a varied axis, deliberately.** `GenericFallbacks` was recorded WRONG by one round
  and VERIFIED by the next because the first probe was DOCX-only — it held the format fixed without
  noticing the format was the variable. Here the answer is **identical in all six**.
* **Every italic case: the reference shears the fallback face, we shear none.** CJK → WenQuanYi Zen
  Hei 6 of 6; symbols → DejaVu Sans 4 of 4; Hebrew → DejaVu Sans 4 of 4; bold+italic the same;
  and the same when the *primary* is itself only synthetically oblique.
* **Four negative controls, sixteen rows, nought on both sides in every format**: italic Latin in a
  family whose italic is installed, and the identical fallback text with no italic asked for.
* **The discriminator refuses the alternative.** Hebrew from an italic **Carlito** run (a real
  italic primary) is covered by DejaVu Sans (no italic) *and* Liberation Sans (italic installed).
  The reference draws **DejaVu Sans, sheared** — it does not go looking for an italic face. So our
  fallback *order* is already right and only the shear is missing.
* **Round 56's fix is a regression control** and agrees to the glyph: `latin-italic-none` is 12
  sheared on both sides in every format, in DejaVu **Serif** for `.docx` and DejaVu **Sans** for the
  other five, which is round 54's `WordFallbackClass` roman default arriving again.

What is held fixed and could still be the answer: **the installed font set** (35 files — both stacks
see it, but every claim here is a claim about *this* machine's italic-less families), the point size
(20 pt; a shear is scale-free in the text matrix), and the weight except where varied.

## The change

`XRay.Office.Text/Fonts/GlyphFallback.cs` gains a **default interface method**
`ReferenceFor(OpenTypeFace face, bool isItalicRequested)` which is the one-argument reverse lookup
plus `SyntheticOblique = isItalicRequested && !face.IsItalic`. Neither implementer changes. Three
call sites pass the request, reconstructed as *the primary face is italic, or the primary run was
itself being sheared*.

**This is `XRay.Office.Text` and it reaches all three tracks.** Census below; no reach is assumed.

## The census, and what it cannot see

`fallbackfaces.py` over all three baselines. A **pure-fallback face** is one that can only have
arrived through `FontItemiser` on this machine because no corpus document names the family —
WenQuanYi Zen Hei, OpenSymbol, IPA Gothic. DejaVu Sans and DejaVu Serif are deliberately **not** on
that list: they are also the substitution answer for an unrecognised family, and a PDF cannot tell
the two routes apart.

| track | documents drawing a pure-fallback face | our leans there | the reference's | documents where it leans and we lean none |
|---|---:|---:|---:|---:|
| words | 141 | **0** of 6 616 | **289** of 9 391 | 14 |
| slides | 101 | **4** of 5 530 | **345** of 5 242 | 1 |
| sheets | 10 | **0** of 16 663 | **4** of 17 159 | 1 |

Named, with the glyph counts the reference leans and we do not:

* **words** — `手机免提系统TSB.DOC` 82, `A320SimNotes.DOC` 75,
  `1228841571067_2009_TPPT_13__2007_TPPT_102__R-108535759.doc` 74,
  `P200904290238_0238_51880.doc` 12, `1257259179492_2007_TPPT_102_Supporting_Doc_2-434003080.doc` 9,
  `technical-architecture.docx` 9, `technical-memo-format.docx` 8, `33004.docx` 5,
  `Agile_Arc_SysDes.docx` 4, `CRIF - Spécification technique - Socle applicatif.docx` 3,
  `SFSP_2013-02_Bulletin.doc` 3, `AirbusCallouts.doc` 2,
  `Contract_Check_Pilot_Sample_LOA.docx` 2, `02_mcar_part-2_and_IS_v2.10.docx` 1.
* **slides** — `outlook_of_nigerian_pension_sector.ppt` **341**. Beside it the same census's
  face table names `section_1_our_rights_presentation.pptx` 158 and
  `ws_prod-g-doc-Events-2007-september-M.017-(French)-France.ppt` 52 as short in **DejaVu** faces,
  which the pure-fallback list cannot attribute.
* **sheets** — `dragon-175066A.xlsx` **4**.

**Every one of those documents passes the gate today**, on all three tracks. So the exposure here is
regression, not gain.

**What this census cannot see, stated so a low prediction that comes true is not read as skill:**

1. **The DejaVu share.** Words is 1 348 glyphs short in DejaVu Sans, 86 in DejaVu Serif and 61 in
   the bold cuts; slides 150 / 182 / 12 / 3. An unknown part of that is glyph fallback (the probe
   shows symbols fall back to DejaVu Sans) and will also move; the rest is family substitution and
   will not. **The census cannot separate them and I am not going to pretend a number.**
2. **The two words documents where we draw *none* of the fallback face at all** —
   `1228841571067…doc` (74) and `1257259179492…doc` (9), 83 glyphs — are a *face-selection*
   divergence, not a shear one, and this change cannot touch them.
3. **The direction away from the reference.** A run our reader believes italic where the reference
   does not would now gain a shear it should not have. Six filters found no such case, but the
   probe authored its own runs; corpus readers are not the probe.
4. **`.ppt` and `.xls`.** The rule was measured through `.pptx` and `.xlsx`; the binary filters were
   not authored, and `outlook_of_nigerian_pension_sector.ppt` — the largest single item in the whole
   census — is a `.ppt`.

## Predicted movement

| quantity | instrument | baseline | predicted |
|---|---|---:|---|
| words: sheared glyphs, ours (reference 154 501) | `shear-chars.py` | 153 806 | **154 000 – 154 700** |
| words: documents the reference shears more of | `shear-split.py` | 39 | **28 – 37** |
| words: glyphs in that direction | `shear-split.py` | 1 611 | **900 – 1 350** |
| words: pages the reference shears and we draw none | `shear-split.py` | 148 | **132 – 145** |
| words: documents **we** shear more of | `shear-split.py` | 5 | **5 – 8** |
| words: our leans in pure-fallback faces | `fallbackfaces.py` | 0 | **190 – 210** |
| slides: our leans in pure-fallback faces | `fallbackfaces.py` | 4 | **340 – 350** |
| sheets: our leans in pure-fallback faces | `fallbackfaces.py` | 0 | **4** |
| **words verdict movement** | `batch-check.sh` vs `MANIFEST.tsv` | 319 of 337 | **0** |
| **slides verdict movement** | `batch-check.sh` | 202 of 311 rows | **0** |
| **sheets verdict movement** | `batch-check.sh` | 290 of 325 rows | **0** |
| page counts changed, any track | `rows.tsv` | — | **0** |
| extractable words changed, any track | `rows.tsv` | — | **0** |
| font lists changed, any track | `rows.tsv` | — | **0** |
| renderings whose bytes change: words | byte diff | — | **14 – 40** |
| renderings whose bytes change: slides | byte diff | — | **1 – 20** |
| renderings whose bytes change: sheets | byte diff | — | **1 – 3** |

The three "changed" rows are zero because a synthetic oblique is a text-matrix `c` term and changes
no advance, no face key and no embedded program: the probe's own totals are the control, where
`flat 19` becomes `flat 13 + lean 6` and the sum does not move.

The byte-reach bands are wider than the census's document counts because the DejaVu share is
invisible to the census (blind spot 1) and can only add documents, never remove them.
