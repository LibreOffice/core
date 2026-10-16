# words-r59 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
corpus `/c/sandbox/workdir/sample-files`; worktree `wt-words-r50` on branch `wt-words-r59`, base
`e4296ee8520`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`. Two predictions, each committed before the
change it covers: `prediction.md` at `4af1d767394` before `6160a1bf475`, and
`prediction-autocolour.md` at `5ad7ed613e8` before `6d479369bb4`.

## Baseline, reproduced exactly

`batch-check.sh … 'words/*' … 8` → `TOTAL 355 MATCH 336 MISMATCH 19`. Scored against
`MANIFEST.tsv`'s own 337-path list rather than that total — the extra 18 rows are the
case-insensitive mount's alias entries — **319 match, 18 open, zero disagreements with the
manifest's status column, document for document.**

Slides and sheets were swept too, because the round's second change touches `XRay.Office.Core`:
**slides 199 of 302** and **sheets 276 of 307**, both agreeing with round 58's merge note. Slides
disagrees with the manifest on nothing. **Sheets disagrees on two documents and it is the
manifest that is stale** — see "A proposed `MANIFEST.tsv` correction" below.

## 1. The list label's slant — the rule is two rules, and round 58's table could not see either

### What the probe settled, over 16 authored packages in four formats

`label-slant.py`. Each `.docx` is round-tripped through the installed 26.2.4.2 into `.doc`, `.odt`
and `.rtf` and rendered from all four, which is round 58's varied-format axis and round 53's
lesson. Two label kinds, because they take different paths through every reader: a **bullet**
(Symbol's U+F0B7, recoded into OpenSymbol, which ships one cut so a lean there is necessarily a
sheared text matrix) and a **number** (`%1.` in Liberation Serif, whose italic *is* installed, so a
lean there is a different `/BaseFont` and no shear at all). Counting only one of the two would have
missed half the rule.

**Refutation 1 — the level wins outright when it states anything, in either direction.**

| case | reference, bullet | reference, number |
|---|---|---|
| `leveloff-markon` — `<w:i w:val="0"/>` on the level over an italic mark | **upright** | **upright** |
| `levelon-markoff` — `<w:i/>` on the level over `<w:i w:val="0"/>` on the mark | **leaning** | **leaning** |

So it is *level-if-stated, else the mark* — not *level OR mark*, which is all round 58's five rows
could distinguish. It matters: **13 of the 271 `.docx`** state `w:i w:val="0"` on a level and also
carry an italic list paragraph mark.

**Refutation 2 — a bullet and a number do not have the same base font.** The paragraph *style*'s
`w:i` leans a **number** label (10 of 10 glyphs italic) and leaves a **bullet** upright (0 sheared).
That is `#i53199` in `SwTextFormatter::NewNumberPortion` (`sw/source/core/text/txtfld.cxx`:578-590):
the bullet branch resets posture *and weight* on the base font before the level's character set is
applied, and the number branch resets only underline and overline. The paragraph *mark*'s own
`w:pPr/w:rPr` reaches the bullet anyway, through `checkApplyParagraphMarkFormatToNumbering`, which
the style chain does not.

The implemented rule, therefore:

    bullet: level-stated ?? paragraph-mark-DIRECT-stated ?? false
    number: level-stated ?? the paragraph mark's resolved posture

### The probe, before and after

Sheared OpenSymbol glyphs for the bullet arm, italic-faced Liberation Serif glyphs for the number
arm, reference/ours:

| | docx | doc | odt | rtf |
|---|---|---|---|---|
| agreeing **before**, of 8 bullet + 8 number cases | 9 of 16 | 11 of 16 | 9 of 16 | 10 of 16 |
| agreeing **after** | **16 of 16** | **16 of 16** | 12 of 16 | 10 of 16 |

`.doc` is measured through LibreOffice's own export, and that export **drops the paragraph mark's
formatting**: the `mark` case reads 0 on the reference in `.doc` where it reads 1 in `.docx`. So the
`.doc` column measures the level arm and cannot measure the mark arm at all, and it is reported as
such rather than counted as agreement.

### Prediction against measurement

| quantity | baseline | predicted | measured |
|---|---:|---|---:|
| **words verdict** | 319 of 337 | 0 movement, downside −1 | **319, zero movement, zero regressions** ✓ |
| OpenSymbol glyphs the reference shears and we do not | 112 in 10 | 0 – 25 | **81 in 4** ✗ |
| — the `.docx` arm | 32 in 7 | 0 | **1 in 1** ✓ |
| — the `.doc` arm | 80 in 3 | 0 – 25 | **80 in 3, unmoved** ✗ |
| words renderings whose bytes change | — | 12 – 45 | **14** ✓ |
| page counts changed | — | 0 | **0** ✓ |
| extractable words changed | — | 0 | **0** ✓ |
| font lists changed | — | 0 – 4 | **0** ✓ |

**The `.doc` arm is the miss, and its cause is now measured rather than guessed at.**
`A320SimNotes.doc` is 75 of the 80. Round-tripped through 26.2.4.2's own flat-ODF export it has
**1 014 `WW8Numz` character-style references and not one `fo:font-style`** — so the reference's 75
sheared bullets do not come from the level at all. They come from `RES_PARATR_LIST_AUTOFMT`, which
`SwWW8ImplReader::FinalizeTextNode` (`ww8par.cxx`:2622) builds from the **control stack at the
paragraph mark** — direct CHPX only — and which `ww8par.cxx`:1954 enables for every `.doc`.

I tried the obvious substitute and it is wrong in both directions, which is why it is not shipped:
taking our resolved `Ww8LayoutParagraph.IsItalic` instead gives **120 against the reference's 75**
on `A320SimNotes`, **4 against 2** on `AirbusCallouts` and **0 against 3** on `SFSP_2013-02`. The
reason is in the reader: `Describe` resolves the character layout at the paragraph's **first
character** — `int at = Math.Min(Math.Max(start, 0), Math.Max(markPosition, 0))` — despite its own
comment saying the mark. **That is the seat, and the `.doc` arm is open on it.**

### What actually moved, and a trap I walked into and caught

Six `.docx` gained 31 correct bullet leans. Five documents *lost* 63 label leans, and the per-face
aggregate reads that as getting worse — `facegap.py`'s DejaVuSans SHORT column goes 1 348 → 1 384.
**It is not worse.** Every lost lean is a label the reference does not shear either:

| document | our sheared *runs* before → after | reference |
|---|---|---:|
| `UG.CAO.00133 … Language.docx` | 28 → **17** | 10 |
| `approvals-…-annex-B-B11 …docx` | 27 → **23** | 17 |
| `TE.CAO.00125 … OJT Logbook.docx` | 103 → **101** | 76 |
| `02_mcar_part-2_and_IS_v2.10.docx` | 1 327 → **1 326** | 1 216 |
| `SPA-02_mcar_part-2_and_IS_v2.9.docx` | 1 143 → **1 142** | 1 290 |

In glyph terms `UG.CAO.00133` goes 306 → **259** against the reference's **260** — an error of 46
reduced to 1. The removed runs are label-shaped strings (`1.2.3`, `(………)`); the reference's
`02_mcar` has **zero** 18-glyph sheared runs, so the `(………)` label we stopped shearing is one it
never sheared. **A per-document *net* is the wrong statistic for a per-label change** — the same
shape as round 58's summed census, arriving in my own instrument, caught by asking the reference
whether it draws the specific run rather than by reading the aggregate.

### Two things found on the way, both fixed with it

- **A `.doc` list level's `sprmCFItalic` was never read at all**, in either spelling (`0x0836`,
  and `0x0056` for Word 6/95). `Ww8ListLevel` now carries it as a **tri-state**, because *stated
  off* and *unstated* lead to different pictures.
- **The ODT reader looked its label's `FontReference` up under the family the resolver
  *answered* rather than the one the level *asked for*.** A bullet level naming `Symbol` resolves
  to `OpenSymbol`, so the lookup missed outright and the label was left with a name-only reference —
  which can be drawn with, cannot be embedded, and carries no synthetic lean either. This is the
  third time that same `emb no` failure has been found; the DOCX and WW8 readers each carry a
  paragraph about their own.

### Deferred, with the measurement rather than a guess

- **`.rtf`.** Our reader draws a bullet in the paragraph's own Liberation Serif and **not in
  OpenSymbol at all** — `01-bullet-level.rtf` is `LiberationSerif 0 lean / 9 flat` against the
  reference's `LiberationSerif 8 flat + OpenSymbol 1 lean`. Both the label's face and its slant live
  in the `{\listtext}` destination's own character formatting, which the reader discards. Larger
  than the slant, and **zero witnesses**: the words corpus is 271 `.docx` and 66 `.doc`.
- **The extra ODT glyph.** Our `.odt` bullet draws **two** OpenSymbol glyphs where the reference
  draws one, in all eight bullet packages including the control. The cause is now named:
  LibreOffice's ODF export writes `loext:num-list-format=""` **and** `style:num-suffix=""`
  on the same level, and `loext:num-list-format` *replaces* prefix and suffix rather than joining
  them. `OdfListStyle` concatenates. Not fixed: `OdfListStyle` is shared with the presentation
  reader and neither corpus holds an ODF document, so it would ship unmeasured.

## 2. The automatic font colour — 305 glyphs on one page, and two rules no source reading gives you

### What the probe settled, over 20 authored packages

`autocolour.py`. Round 58 pinned the threshold over a 22-fill ramp. Three things it could not see:

**Refutation 3 — `Color::IsDark()` is not one formula, and 26.2.4.2 still has the second.**
`tools/source/generic/color.cxx`:52 special-cases `COL_DEFAULT_SHAPE_FILLING` (`0x729FCF`) and asks
the *perceived* luminance `<= 62` for it, where every other colour is asked WCAG `<= 87`. That
colour has WCAG luminance **83** — dark — and perceived luminance **151** — bright. **The reference
draws its text black**, and draws `6F9BCB` one sRGB step away **white**. It is the only input in the
whole domain that separates the two functions, and round 58's ramp did not contain it.

**Refutation 4 — a character highlight is not a background, in both directions.** A `yellow`
`w:highlight` on a run in a **black** cell is drawn **white**; a `darkBlue` highlight in a **white**
cell is drawn **black**. The brush `SwDrawTextInfo::ApplyAutoColor` asks for is the font's *back*
colour, `RES_CHRATR_BACKGROUND`; `w:highlight` is `RES_CHRATR_HIGHLIGHT`, a different item. Reading
the highlight as the background is the obvious wrong answer and both cases refuse it.

**Refutation 5 — `w:shd` is a pattern and not a fill, and `w:val="nil"` is not "no fill".**
`CellColorHandler::getProperties` turns `w:val` into a per-mille weight — `clear` 0, `solid` 1000,
`pctN` N×10 (12→125, 15→150, 37→375, 62→625, 87→875), every striped and crossed value a flat 333 —
and blends `w:color` over `w:fill` at it, where `w:color="auto"` is **black** and `w:fill="auto"` is
**white**. `nil` is absent from that table, takes the zero-weight branch and paints its fill. All
eight patterns reproduce byte for byte after: `pct50` auto/auto `#7F7F7F`, `pct25` `#BFBFBF`,
`pct75` `#3F3F3F`, `diagStripe` and `thinDiagCross` `#AAAAAA`, `pct50` red-over-blue `#7F007F`.

A paragraph shade beats the cell it sits in, in both directions; a run stating `w:color="FF0000"` in
a black cell stays red — the control.

**4 of 20 packages agreed on glyph colour before; 20 of 20 after.**

### Prediction against measurement

| quantity | baseline | predicted | measured |
|---|---:|---|---:|
| **words verdict** | 319 of 337 | 0 movement, band −2 to +1 | **319, zero movement, zero regressions** ✓ |
| white glyphs **SHORT** (reference draws white, we do not) | 5 145 in 48 | 1 200 – 3 800 | **2 728 in 38** ✓ |
| white glyphs **LONG** (we draw white, the reference does not) | 34 in 2 | 34 – 400 | **34 in 2** ✓ |
| filled rectangles on `AFS-050-004-F2_0i` p2 | 5 against 8 | 8 against 8 | **8 against 8** ✓ |
| words renderings whose bytes change | — | 45 – 110 | **35** ✗ |
| page counts / extractable words / font lists changed | — | 0 / 0 / 0 | **0 / 0 / 0** ✓ |

`AFS-050-004-F2_0i` goes from **0 white glyphs to 569** against the reference's 571, and its page 2
now fills the same eight rectangles the reference does, the three header cells included, matching x
and width to 0.05 pt.

**The byte-change count is the miss and it is a real one**: 35 against a predicted 45–110. The
prediction's reach census counted 12 497 dark-background elements in 86 documents and said in
advance that it could not see whether a shaded cell holds text or whether that text states a colour
of its own. It over-reached by more than the band allowed for — most shaded cells in this corpus
already state white text explicitly.

### The LONG column caught a defect the gate cannot see, which is what it was for

**The first cut of this change passed the frame's own fill down as the background**, because
`SwFrame::GetBackgroundBrush` walks fly frames and that is the reading the source invites. Measured,
it turned **383 glyphs white that the reference draws black** — 371 in
`docs-quality-MA.IMS.00001-Integrated-Management-System-manual.docx` page 9, whose shape is filled
`#0070C0` (WCAG luminance 38, dark by every rule this code knows), and 12 in
`069_Work_Breakdown_Structure_Template_Professional_Format` at `#8496B0`. **Page count, word count
and font list are all unchanged by painting text out of a page**, so nothing in the gate would have
said a word. The prediction named that risk and named the LONG column as its control, and the
control fired: 34 → 417.

The arm is removed. A `PageFrame` here is a Writer text frame *and* a DrawingML shape alike, and a
shape's text is drawn by the drawing layer, where `COL_AUTO` is resolved by editeng and never
reaches `ApplyAutoColor`. Nothing in this layer can yet tell the two apart, so the arm stays off
until a probe separates them. After removal the LONG column is **34 in 2 — exactly its pre-round
value**, so the shipped change introduces no wrongly-white glyph anywhere in the corpus.

### What it took, and it is three separate things

1. The readers turned an unstated colour into **opaque black**, so nothing downstream could tell
   `COL_AUTO` from a document that asked for black. All four now keep it transparent.
2. Nothing carried the background *to* the drawing pass. `PageDrawing` threads it from the page
   through the table, the cell and the paragraph, in the order `GetBackgroundBrush` walks them.
3. `w:shd`'s pattern, above.

This touches `XRay.Office.Core`: `Colour` gains `IsDark`, `WcagLuminance` and `PerceivedLuminance`.
**Additive — `git grep` finds no consumer outside `XRay.Office.Core` and `XRay.Office.WordProcessing`** —
and both other tracks were swept anyway: slides 199 of 302 and sheets 276 of 307, each agreeing
with round 58.

## 3. The fallback-*order* census, all three tracks — and the largest item on slides is not fallback order at all

`facechoice.py` asks, per document and **per face**, which `/BaseFont`s each side draws that the
other never opens. Printed as three sections that are never netted: unique to the reference, unique
to us, and **paired** — documents in both, which is one substitution decision made two ways and is
the list a fallback-order fix is aimed at.

**Slides — and the headline is a different defect.** `Sean Monogue.pptx` is 5 527 glyphs, the
largest face divergence on the track, and it is **not** an ordering mistake: the package carries
four `ppt/fonts/*.fntdata` parts and a `p:embeddedFontLst`, the reference draws Verdana out of them,
and `fc-match Verdana` on this machine answers `DejaVuSans.ttf`. Same for
`Liturgical-Commission-2025-Convention-Presentation.pptx` and its embedded `Play`.
`embeddedfonts.py` censuses all three tracks: **six slides documents embed a font; no words or
sheets document does.**

| track | document | the reference draws | we draw |
|---|---|---|---|
| slides | `Sean Monogue.pptx` | **Verdana 5 405 + Bold 90 + Italic 32** (embedded) | DejaVuSans 5 333 + Bold 90 |
| slides | `Liturgical-Commission-2025-…pptx` | **Play-Regular 93** (embedded) | DejaVuSans 92 |
| slides | `outlook_of_nigerian_pension_sector.ppt` | WenQuanYiZenHei 355 | LiberationSerif 6 |
| slides | `1-secretariat.ppt` | DejaVuSans 121 | LiberationSans 60 |
| slides | `010605Vul.ppt` | OpenSymbol 11 | LiberationSerif 80 |
| words | `AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc` | **Carlito 42 395 + Bold 4 242** | LiberationSerif-Bold 4 318 |
| words | `1228841571067_2009_TPPT_13….doc` | WenQuanYiZenHei 2 460 | DejaVuSans 13 |
| words | `HC-Bulletin-template.docx` | LiberationSans 1 050 + Bold 1 265 | LiberationSerif 1 044 + Bold 1 263 |
| words | `2024-12_Comlux_opens_….docx` | LiberationSans-Italic 663 | DejaVuSans 3 759 + Bold 11 |
| words | `f111.doc` | Carlito 275 | LiberationSerif 275 |
| words | `AFS-050-004-F2_0i.docx` | DejaVuSerif-Bold 42 | DejaVuSans 50 + Bold 42 |
| words | `021_Unit_Circle_Chart_3D_Pie_Chart….docx` | Carlito-**Bold** 40 | Carlito-**Regular** 40 |
| words | `template---tpr-technical-progress-report….docx` | DejaVuSans 23 | DejaVuSerif 23 |
| words | `150_5300_13_chg12.doc` | OpenSymbol 21 | LiberationSans 360 |
| sheets | `049_expenses_calculator….xlsx` | LiberationSerif 648 | LiberationSans 636 |
| sheets | `TDA_Smoke-Detectors.xlsx` / `Part_375_Operators` / `Part_129_Operators` | LiberationSerif 466 each | LiberationSans 448–450 |
| sheets | `037_Personal_money_tracker….xlsx` | Carlito-Bold 257 | LiberationSans 657 |
| sheets | `Air_Boss_Master_List.xlsx` | Carlito-Bold 176 | LiberationSans 172 |
| sheets | `dynamicbubblechart.xlsx` | LiberationSerif 214 | LiberationSans 208 |
| sheets | `062_Run_chart….xlsx` | DejaVuSans 115 + Bold 34 | DejaVuSerif 32 |
| sheets | `063_sales_pipeline….xlsx` | DejaVuSans 92 | DejaVuSans-**Bold** 143 |
| sheets | `REDAC_SCHEDULE_RPD_137.xls`, `021_Control_Chart_Template….xlsx` | OpenSymbol 13 / 11 | DejaVuSans 13 / 11 |

**Three classes fall out of it, and they are not one problem:**

1. **A font embedded in the package.** 5 620 glyphs, 2 slides documents (6 documents carry parts).
   Nothing to do with ordering. **This is the largest single item in the whole census.**
2. **Serif against sans — the generic *class*, not the family.** `HC-Bulletin-template.docx`
   (2 315), `template---tpr` (23) and **five sheets documents** (~2 240) all differ by exactly one
   step of the roman/swiss decision, in both directions. It is on all three tracks and it is the
   most repeated shape.
3. **Carlito against Liberation Serif.** `AAC-AD-…-MAX.doc` alone is **46 637 glyphs**, the single
   biggest number in the words census and one round 58's slant-only view never surfaced; plus
   `f111.doc` 275, `SPA-06` 313, `t_TEMPforInvProgs` 250, and two sheets documents. Calibri
   resolving to Liberation Serif where the reference resolves it to Carlito.

Round 58's item — `outlook_of_nigerian_pension_sector.ppt`, 355 WQY glyphs — is real and is
**seventh** by size on this list.

## 4. The 24.2.7.2 audit — one site taken, verified

`WordCompatibility.AddsParagraphSpacing` (`!HasSettingsPart || DoNotUseHtmlParagraphAutoSpacing`),
whose remarks recorded a 24.2.7.2 measurement. `audit_paraspacing.py`: four authored packages,
measured by the **baseline pitch** in the reference's own PDF rather than by a stated spacing,
because a boundary is exactly what a pitch is — 24.00 pt collapsed, 32.00 pt added.

| arm | reference | ours |
|---|---|---|
| no `word/settings.xml` at all | **32.00** | 32.00 |
| the part present but **empty** — the discriminator | **24.00** | 24.00 |
| the part naming only `compatibilityMode` | **24.00** | 24.00 |
| the flag `doNotUseHTMLParagraphAutoSpacing` set | **32.00** | 32.00 |

**VERIFIED**, four of four to 0.00 pt. The second arm is the whole of the property: *absent* and
*empty* are different inputs, because the write lives in `DomainMapper_Impl::ApplySettingsTable`
which returns at its first line when there is no table.

Counts re-derived with the file's own commands rather than quoted. At the base commit: **38 open
sites, 23 markers — 19 VERIFIED / 3 FIXED / 1 WRONG**. At this tree: **38 open, 24 markers —
20 VERIFIED / 3 FIXED / 1 WRONG**. The open count does not fall, per the file's convention: the
site keeps its original 24.2.7.2 prose as the record of what the code was fitted to. (Round 58's
results quote "17 VERIFIED"; the command above gives 19 at that same tree, so one of the two
countings is off by two and the commands in `TODO.24-2-7-audit.md` are the ones to trust.)

## 5. The vision reading

Three blind readings, each handed one composed image and nothing else, each forbidden from reading
any file but that image or running any command, each asked to describe the halves separately before
comparing and to give the direction.

### `AFS-050-004-F2_0i.docx` page 2, **before** the change — chosen because it is the round's item 2

Not `--worst`: chosen because it is the page the brief names and the one the change is aimed at.

**The reviewer named the defect blind.** "All five black separator bands on the left are blank black
bars… the corresponding five bands on the right carry the section titles." Direction: content
present on the reference, absent on ours. Its first candidate cause, unprompted, is the right one —
"the text runs exist but are drawn in a colour equal to the fill" — and it names the discriminating
measurement: extract the text layer and grep for `CE-1`; if the string is there, it is colour or
paint order, and comparing the fill operator with the text-fill operator separates those two.

**Second instrument, confirmed**: the text-layer extraction round 58 ran says exactly that — both
strings at the reference's own positions, five banner rectangles matching to a twip, and 305 white
glyphs on the reference against our nought.

**One thing the reading got wrong, and it is a calibration point.** The reviewer described the
reference's band text as "dark glyphs on a black fill… low-contrast dark-on-dark, readable with
effort". It is white. At 120 dpi in a composed half, a 9 pt reversed-out line reads as *present* and
its *colour* is not recoverable. **Presence right, colour wrong** — which is precisely the split the
skill file predicts, and worth keeping: the reading that led to the fix would have been just as
useful if it had said only "there is text there and there is none here".

### The same page, **after** the change — the confirmation, and two new leads

The reviewer lists under *identical*: "The black banner rows: identical fill (solid black),
identical white bold centred text, identical wording for all six bands." The defect is gone by a
blind reading of the fixed page, which is the strongest confirmation this instrument gives.

Two differences it raises that no metric in this round had named, both **pre-existing**:

- **We draw blue where the reference draws black**, in the header table's `Title:` / `Effective
  Date:` / `Page … of …` row. Second instrument, measured on the page's own content stream: ours
  draws **9 runs** of `#0000FF` on page 2 and the reference **2** — identical before and after this
  round's change, so it is not something this round introduced. Open.
- **Our intra-group separator rules are solid where the reference's are dotted**, four or five
  occurrences. Not yet checked by a second instrument; the reviewer names the right control
  (re-render at 2–4× and see whether the reference's dots resolve into periodic gaps or fill in).

It also reports that **the reference pushes the last row `4.701 – 4.705` off the page and we keep
it** — a pagination divergence on a document the gate scores as a match.

### `097_Business_Case_Template_Elegant_Layout_3ba9cbf2.docx` page 1 — chosen for the brief's
line-height item, not for its ink

**The reviewer's first-ranked difference, blind: "the reference creeps down the page relative to
ours, and the gap grows monotonically from top to bottom… equivalently, the reference's rows are on
average a hair taller."** That is the 1.7 pt deficit the brief names, found without being told it
exists, with the direction. Its fifth item localises it: "the reference's six empty rows [in the
History table] are each a fraction taller".

**Second instrument — confirms the direction, refutes the shape, and refutes the mechanism.**
Measuring the top edge of each full-width blue band on page 1 of both sides: `+0.24`, `+0.89`,
`−0.06`, **`−2.06`** pt. So the reference *is* 2.06 pt further down by the last band, but the ramp
is **not monotonic** — the whole deficit sits between bands 3 and 4, which is the History table's
six empty rows, exactly where the reviewer's fifth item put it and not where its first item did.

**And a refutation of my own first reading of the failure.** `097` fails the gate at 1 page against
2, and the obvious story is that the deficit lets one row fit that should not. It is not:
`pdftotext -bbox` shows the reference's **page 2 is completely empty** — every word is on page 1 on
both sides. The failure is a **trailing empty page** the reference emits and we do not, and the
per-empty-paragraph deficit is what decides whether the document's final empty paragraph fits.
That makes `097`, `012_Project_Timeline_Template…` and `015_Project_Timeline_Template…` — all three
scored 1 page against 2 — one class, and it is the class the next round should take.

## Refutations, collected

1. **The list label's rule is not "the level OR the mark".** The level wins outright wherever it
   states a posture, in either direction — `leveloff-markon` and `levelon-markoff` disagree on both
   label kinds. 13 of 271 `.docx` write the first shape.
2. **A bullet's base font and a number's are not the same.** `#i53199` resets posture and weight for
   a bullet and not for a number, so a paragraph style's `w:i` leans a number label and leaves a
   bullet upright.
3. **`Color::IsDark()` is two formulas on 26.2.4.2**, and `0x729FCF` is the only input that
   separates them: WCAG says dark, perceived says bright, and the reference draws it **black**.
4. **A character highlight is not a background**, in both directions.
5. **`w:shd` is a pattern, not a fill**, and `w:val="nil"` is not "no fill" — it paints its fill.
6. **A floating frame's own fill is not the background either.** The one arm of the chain I inferred
   rather than measured, and the corpus refutes it at 383 glyphs across two documents.
7. **A per-document net is the wrong statistic for a per-label change.** Five documents whose
   aggregate lean count fell are five documents that moved *toward* the reference; the aggregate
   said the opposite. Round 58's summed census, arriving in my own instrument.
8. **The `.doc` bullet slant does not come from the level.** `A320SimNotes.doc` has 1 014
   `WW8Numz` style references and not one `fo:font-style` through the reference's own export.
9. **The largest face-selection divergence on the slides track is an embedded font**, not a
   fallback order — and no words or sheets document embeds one at all.
10. **`097` does not fail because content overflows.** The reference's page 2 is empty; the failure
    is a trailing empty page.

## Tests

```
Core 358   Containers 109   Text 624   Vector 298   Rendering 153(1 skipped)   Markup 259
OpenDocument 125   WordProcessing 1220   Spreadsheets 980   Presentations 836     = 4962
0 failed, 1 skipped
```

**4 909 → 4 962, delta +53**, re-derived rather than quoted: Core 337 → 358 (+21,
`ColourDarknessTests`), WordProcessing 1 188 → 1 220 (+32 — 16 `ListLabelSlantTests`,
5 `Ww8ListLevelTests`, 11 `AutomaticFontColourTests`). `dotnet build -v q -nologo`: **0 warnings,
0 errors.**

**`XRay.Office.Vector` reported a phantom failure twice** — once 1 of 298, once 21 of 298 — while
another test project was running beside it, and passed 298 of 298 alone both times. That is the
trap `CLAUDE.md` names, and it now has two more instances.

Run through `verify-test.sh`, tree clean before each and restored after — **ten mutations, nine
detected, one not, and the one is reported as what it is**:

| mutation | detected by |
|---|---|
| the DOCX bullet never takes a posture (`Symbol(…, false)`) | 3 `ListLabelSlantTests` |
| the number branch drops `levelItalic ??` | 3 `ListLabelSlantTests` |
| the `.doc` level's `sprmCFItalic` never reads *on* | 2 `Ww8ListLevelTests` |
| a toggle byte of 128/129 is read as *on* | `AToggleThatNamesAStyleIsNotAValue` |
| `Colour.IsDark` drops the `COL_DEFAULT_SHAPE_FILLING` arm | `TheDefaultShapeFillingIs…`, `TheExceptionIgnoresAlpha` |
| `Colour.IsDark` asks the perceived luminance for everything | 4, including the grey ramp |
| `PageRun.ColourOn` ignores its background | 5 `AutomaticFontColourTests` |
| `DrawTable` passes the outer background instead of the cell's fill | 3 `AutomaticFontColourTests` |
| `ShadingWeight("solid")` returns 0 | `AnUnstatedColourIsResolvedAgainstWhatIsBehindIt` |
| the DOCX reader forces an unstated colour to black again | 5 `AutomaticFontColourTests` |
| **the ODT label's reference key goes back to the resolved family** | **NOT DETECTED** |

The last one is honest and not a hole to paper over: the only ODF document in the test corpus,
`text-features.odt`, states its bullets as a real U+2022 in the paragraph's own face, so the
requested family and the resolved family are the same and the mutation is an *equivalent
formulation* there. The change is measured only by `label-slant.py`'s `.odt` column, where the
bullet-level arm goes from `0` sheared against the reference's `1` to `2` against `1` — the right
direction, and long by one because of the separate `loext:num-list-format` defect above.

## A proposed `MANIFEST.tsv` correction (not committed — the corpus repo is not mine to write)

`MANIFEST.tsv`'s sheets status column records **278 done**; this round measures **276**, as did
round 58. The two it over-counts are:

```
sheets/chartset-002/xlsx/003_advanced_excel_pie.xlsx   done -> open   (ours 138 words, ref 143)
sheets/chartset-004/xlsx/019_advanced_excel_pie.xlsx   done -> open   (ours 135 words, ref 140)
```

Neither moved this round and neither can have: `git grep` finds no consumer of the new `Colour`
members outside `XRay.Office.Core` and `XRay.Office.WordProcessing`. Both are stable across repeated
renderings on **both** sides — the reference gives 143 twice — so this is not the date-volatility
trap round 57 named. The missing tokens are chart labels (`07;`, `17%`, `93;`, `Actual;`, `M1;`),
five of them, which puts the gap just outside the 2%+3 band. Words (319) and slides (199) agree
with the manifest exactly.

## Files

- `prediction.md`, `prediction-autocolour.md` — each committed before the change it covers.
- `label-slant.py` — 16 packages × 4 formats, both label kinds, the two precedence discriminators.
- `label-italic-census.py` — three arms, printed as arms and not as a total.
- `autocolour.py` — 20 packages: the `0x729FCF` discriminator, the two highlight controls, the
  paragraph-versus-cell pair, eight `w:shd` patterns and two controls.
- `darkbg-census.py` — resolves every `w:shd` through the blend and asks `IsDark`, in three arms.
- `shd-census.py` — the pattern fix's own reach, split by what we paint today.
- `whiteglyphs.py` — white glyphs per document on both sides, SHORT and LONG, never netted.
- `facechoice.py` — the fallback-*order* census, per face, for any track.
- `embeddedfonts.py` — the census that turned the slides headline into a different defect.
- `audit_paraspacing.py` — four packages, measured by baseline pitch.
