# slides-r54 — prediction

Committed **before** anything is built or rendered post-change. Environment: LibreOffice
**26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`, base `4b50291b09d`,
branch `wt-slides-r54`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

## The baseline reproduced exactly

Whole-track sweep at the base commit (our side re-rendered, reference PDFs reused from
`scratch-r53-slides/ink-after/ref`, which is legitimate because nothing has touched `soffice`).

| | briefed | measured |
|---|---|---|
| passing, scored over `MANIFEST.tsv` | 199 of 302 | **199 of 302, 0 disagreements** |
| `abs_ink` / signed / major | 1233.54 / — / 432 | **1233.54 / 913.68 / 432** |
| `tf-agreement` mean | 0.75210 | **0.75210** |
| pages with an exact `/Tf` multiset | 1558 of 4515 | **1558 of 4515** |

The sweep's own `TOTAL` is 311 rows / MATCH 201, as always; the scoreboard is the manifest join.

## The mechanism, measured on the reference before any change was made

Round 53 read the chain out of the C++ and did not measure it. It is now measured, on an
**authored known-answer `.ppt`**, and it holds.

Authoring it needed one thing round 53's plan did not have: **`soffice --convert-to ppt` does
not preserve `a:normAutofit`, because autofit is not spelled anywhere in the binary format.**
`svdfppt.cxx:1030-1039` infers it from the TextHeaderAtom's *instance* — Body, HalfBody or
QuarterBody are fitted and every other kind is not — and a round-tripped text box comes out
instance 4 (TextInShape). Measured: the first cut of the probe drew all fifteen slides at the
stated 40 pt over 21 overflowing lines. `ppt-patch-kind.py` flips the chosen TextHeaderAtoms to
instance 1; the record body is a single `uint32`, so the edit is length-preserving.

`make-ppt-fit-probe.py` → `soffice --convert-to ppt` → `ppt-patch-kind.py` → render both halves.
Fifteen slides, one 360 × H pt box each, H from 60 to 200 pt, three paragraphs of 40 pt text.
`ppt-style-dump.py` on the result: every paragraph states `lf=100` **and** every character run
states `font=1` — so `bIsHardAttribute` is set by both of `svdfppt.cxx:6267-6271`' routes.

| box | `.pptx` `/Tf` | pitch | pitch/em | `.ppt` `/Tf` | pitch | pitch/em |
|---:|---:|---:|---:|---:|---:|---:|
| 60–110 pt | 10.006 | 9.609 | **0.960** | 10.006 | 12.087 | **1.208** |
| 150 pt | 15.987 | 15.364 | **0.961** | 13.011 | 15.661 | **1.204** |
| 190 pt | 15.987 | 15.364 | **0.961** | 15.987 | 19.233 | **1.203** |

**The `.pptx` half draws `1.2 × 0.8 × em` and the `.ppt` half draws `1.2 × em`.** Same box, same
text, same fit table — the spacing reduction is applied to one and not the other. And because the
`.ppt` lines are taller, the fit *search* lands on a different row: at 150 pt the reference draws
13.011 on the binary side against 15.987 on the OOXML side.

The asymmetry has a second, independent confirmation in the source, which round 53 did not have:
**the UNO route sets `Off` at exactly 100 where the binary route sets `Prop`.**
`SvxLineSpacingItem::PutValue` (`editeng/source/items/paraitem.cxx:194-202`) reads
`style::LineSpacingMode::PROP` and writes `eInterLineSpaceRule = Off` **when the height is exactly
100** and `Prop` otherwise. That is the path every OOXML and ODF line spacing takes. The `.ppt`
importer instead calls `SetPropLineSpace(100)` directly, and `lspcitem.hxx:86-91` makes that
setter write `Prop` unconditionally. So the defect is `.ppt`-only by construction.

Ours at the base commit renders the `.ppt` half exactly like the `.pptx` half — spacing 0.8
applied, and consequently a larger font on 9 of the 15 slides.

## What ships

One flag, confined to `XRay.Office.Presentations`: a `.ppt` paragraph records whether its line
spacing is **stated** in `svdfppt.cxx`'s sense — the paragraph's mask names `PPT_ParaAttr_LineFeed`,
**or** the character run containing its first character names `PPT_CharAttr_Font`, **or** it sits
at depth > 0 in a `TextInShape`/`Subtitle` text object (`svdfppt.cxx:5953-5957`, `:5488-5492`).
`SlideTextLayout`'s last arm — the `SvxInterLineSpaceRule::Off` transcription, the only place the
fit's `fSpacingY` is applied — is skipped for such a paragraph.

`.pptx` and `.odp` are untouched by type: the flag defaults false and only `PptTextBody` sets it.

## The census, and what it cannot see

Two censuses of the **rendering** (`ppt-spacing-census.py`), pairing our base renderings against
the reference's, per page, per `/Tf` bucket, over constant-pitch blocks of three or more
baselines:

| | blocks | documents | pages |
|---|---:|---:|---:|
| ours 0.96 or 1.08, reference 1.20, **same `/Tf`** | 35 | 18 | 34 |
| ours 0.96/1.08 and reference 1.20 at **different `/Tf`** | 65 | 17 | 64 |
| union of the two document lists | | **24** | |
| both sides 0.96/1.08 (must **not** move) | 219 | 33 | |

And one census of the **record** (`ppt-hardness-census.py`), over the 42 of 51 corpus `.ppt`
documents its parser reads: **3736 paragraphs, 2156 hard (436 by line feed, 1947 by font index),
1580 soft.** So the rule is emphatically not "no `.ppt` shape gets the reduction" — 42% of
paragraphs stay on the `::Off` arm.

**What neither census can see, written down before the sweep:**

1. **Whether a shape is autofitted at all, and which `constScaleLevels` row it lands on.** Both
   are properties of the layout and invisible in the record. The block counts are therefore a
   count of *symptoms already visible*, not of shapes the change reaches.
2. **A block of fewer than three baselines contributes nothing**, so a two-line fitted body is
   invisible to the pitch census in both directions.
3. **A page whose two sides break lines differently produces no comparable block at all**, which
   is exactly the page where the change is likely to be largest.
4. **The 219 "both shrunk" blocks are not proved safe by this census.** They are consistent with
   a *stated* line feed of 80 or 90 — `2015-Civil-Rights-Website-training.ppt` alone states
   `lf=80` on 96 paragraphs and `lf=90` on 216 corpus-wide — and the reference's `Prop` arm does
   multiply by `fSpacingY`, so those keep today's arithmetic. But the census cannot separate a
   stated 90% from a fitted 0.9, and if any of the 219 is a *soft* paragraph currently agreeing by
   accident, it will move.
5. **Nine of the 51 `.ppt` documents fail the hardness census's parser** (a short property record),
   so the 3736/2156 split is over 42 documents, not 51.
6. **Two of `svdfppt.cxx`'s hardness terms are deliberately not modelled**: `nDestinationInstance
   == TSS_Type::Unknown`, and the comparison of the source instance's master level against the
   destination instance's when a Body-kind text object carries no `OEPlaceholderAtom`. Both make
   *more* paragraphs hard, so this round's rule under-reaches rather than over-reaches, and an
   under-reaching census conceals itself — that is why it is named here.
7. **The soft half of the same mechanism is NOT implemented this round.** When a paragraph is
   soft, LibreOffice puts no `SvxLineSpacingItem` at all, so the master level's line feed is
   *never applied* — `PPT_ParaAttr_LineFeed` is consumed at exactly one site in the whole tree
   (`svdfppt.cxx:6267`) and reaches no style sheet. We do apply it. That is a second, larger
   change reaching ~1580 paragraphs and it needs its own known-answer deck; this round measures
   the first half only and reports the second as open.

## The predictions

| # | prediction |
|---|---|
| 1 | **Verdict movement: 0.** 199 → 199. A surprise is anything outside −1…+2. |
| 2 | **Page counts: 0 of 302 change.** A deck's page count is its slide count. |
| 3 | **The known-answer deck lands exactly.** After the change, our `fitbody.ppt` matches the reference's `/Tf` and pitch on **15 of 15** slides, `/Tf` to 0.01 pt and pitch to 0.03 pt. Today it matches 6 (the six that sit on `FitLevels`' floor by accident). |
| 4 | **Documents whose rendering moves: 18–30, all `.ppt`.** The union of the two censuses is 24. |
| 5 | **`abs_ink` −5 to −25.** Stated as the weakest number here. Round 53's `abs_ink` point estimate was its worst call for exactly this reason — a census of *candidates* is an upper bound on changes, not an estimate of them — and the largest single contributor, `gfopportunitiesforlinkagespres_2010_en.ppt`, carries only 16.11 in total. |
| 6 | **`tf-agreement` rises**, 0.75210 → 0.753–0.762, and exact pages rise from 1558. This is the quantity the change controls most directly: 65 of the 100 censused blocks disagree on the drawn size, not only on the pitch. |
| 7 | **`gfopportunitiesforlinkagespres_2010_en.ppt` p6 draws 26 / 22 / 17 pt** where we draw 28 / 24 / 18 today, at a pitch of `1.2 × em`. This is the brief's worked case and it is a named, falsifiable page. |
| 8 | **The 219 "both shrunk" blocks do not move**, measured as: no document in the 33-document list that is *absent* from the 24-document change list moves on `tf-agreement` by more than 0.001. This is the control, and it is the prediction most likely to fail. |
| 9 | **No cross-track reach.** The diff is confined to `XRay.Office.Presentations`; `XRay.Office.Text`'s `LineSpacingRule` is not touched, so words and sheets are unreachable by type. |

## Blind spots in the predictions themselves

- Prediction 3 is a **known-answer** test on a deck this round authored, so it is the strongest
  claim here and also the one whose failure would be most informative.
- Prediction 5's sign is much safer than its magnitude. Round 53 measured five documents whose
  unsigned ink *rose* while every one of their baselines moved closer to the reference's, so
  `abs_ink` will be reported alongside `baseline-agreement.py` and neither will be netted.
- Prediction 8 cannot be checked block-by-block, only document-by-document, because
  `tf-agreement` is a per-document figure. A document appearing on both lists is uninformative.
