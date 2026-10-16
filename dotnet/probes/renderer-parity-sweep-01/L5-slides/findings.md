# L5-slides — findings

35 documents. Grouped by root cause. Every number below was read out of the two PDFs' own content
streams or out of the document's own record stream; nothing was inferred from a picture.

Environment note that changes how the whole lane reads: **the installed `soffice` is 24.2.7.2**
(`LibreOffice 24.2.7.2 420(Build:2)`) and the C++ tree beside it is `27.2.0.0.alpha0+`, so the tree
is *not* the source of the reference's behaviour and was used only for rules that predate both.

---

## A. Autofit is targeted at a different LibreOffice than the one that wrote the reference

**24 of 35 documents.** #030 #047 #056 #071 #079 #083 #086 #099 #103 #104 #110 #113 #118 #119 #122
#124 #125 #128 #132 #138 #149 #156 #168 #171.

### 1. What the pages show

The visible symptom is never "autofit"; it is *reflow*. The case notes call it "the text is set
wider than the reference's", "bullets lose a line", "the block ends higher", "the block runs down
into the watermark", "bullet glyphs sit high". All of it is one thing: **the two sides draw the
body at different point sizes.**

Read straight out of the `/Tf` operators on each divergent page (full table:
`probe/sizes.tsv`) — the dominant body size, reference against ours:

| doc | ref | ours | | doc | ref | ours |
|---|---:|---:|---|---|---:|---:|
| #030 | 17.01 | 18.00 | | #118 | 27.01 | 28.01 |
| #047 | 14.99 | 15.99 | | #119 | 27.01 | 28.01 |
| #056 | **9.01** | **11.99** | | #122 | 21.00 | 20.01 |
| #071 | 25.99 | 24.01 | | #124 | 17.01 | 18.00 |
| #079 | 18.99 | **22.00** | | #125 | 31.01 | 29.99 |
| #083 | 25.00 | 25.99 | | #128 | 27.01 | 25.99 |
| #086 | 14.00 | 13.01 | | #132 | 24.01 | 25.99 |
| #099 | 20.01 | 18.99 | | #138 | 18.99 | **22.99** |
| #103 | 25.00 | 24.01 | | #149 | 20.01 | **22.99** |
| #104 | 22.99 | 22.00 | | #156 | 32.00 | 29.99 |
| #110 | 22.99 | 24.01 | | #168 | 21.00 | 22.00 |
| #113 | 31.01 | 32.00 | | #171 | 10.01 | 11.00 |

It goes both ways — we under-shrink on twelve and over-shrink on twelve — which is the signature of
a *different table*, not of a missing multiplier.

The pens are not the issue. On #030 page 4 both sides draw the marker at x = 60.52 pt and the text
at x = 87.52 pt; on #129 both draw the marker at x = 31.436. **The text area, its insets and its
left edge agree.** What differs is the size, and everything downstream of it.

### 2. What the documents actually contain

Every `.pptx` in the cluster states `a:normAutofit`, most with a `fontScale` and some with a
`lnSpcReduction` (`unzip -p … 'ppt/slides/slide*.xml' | grep -o '<a:normAutofit[^/]*/>'`):

```
G-InvoicingKeithJarboe.pptx      <a:normAutofit lnSpcReduction="10000"/>   (slide 2)
Item 6b - UNSD …APIs.pptx        <a:normAutofit fontScale="92500" lnSpcReduction="10000"/> (slide 5)
manufacturing_process…2023.pptx  <a:normAutofit fontScale="25000" lnSpcReduction="20000"/> (slide 2)
```

The stated scale is a red herring and the tree already knows it: LibreOffice reads `@fontScale`
into `TextBodyProperties::mnFontScale` and never reads that field again, and does not read
`@lnSpcReduction` at all. Both sides therefore solve the fit themselves. **The question is what
answer the solver gives.**

### 3. Where it lives in the source

`dotnet/src/XRay.Office.Presentations/Layout/SlideAutofit.cs:118-131` — the twelve
`constScaleLevels` rows — and `:155`, the `0.250` floor. `Solve` at `:407-433` walks the table and
takes the first row that fits.

This is **not a defect**. `dotnet/probes/slides-r52/results.md:75-108` records the round that put it
there: the same probe shape, 36 one-slide decks, run against **26.2.4.2**, reproducing every row of
the table in order. The file's own remarks even carry the warning that fired here —
*"check which version wrote the reference before porting anything out of this tree."*

### 4. The measurement

I ran the same probe against **the binary that wrote this sweep's references**: 36 single-slide
decks differing only in text-box height, 60…480 pt in 12 pt steps, one 40 pt Liberation Sans
paragraph under `a:normAutofit`, zero insets, one slide per file so the reference's shared-outliner
state leak cannot reach it. Source `probe/fit/make.py`, raw output `probe/fit/fitcurve.tsv`.

The nine distinct sizes 24.2.7.2 draws are **22, 25, 29, 32, 33, 34, 35, 37, 40** — that is
40 × `0.550, 0.625, 0.725, 0.800, 0.825, 0.850, 0.875, 0.925, 1.000`.

**Four of the nine — 0.725, 0.800, 0.825, 0.875 — are not rows of the table and cannot be produced
by it at any rounding.** (The table's neighbours are 0.850→34 and 0.925→37; it has no way to reach
29, 32, 33 or 35.) The answer set is a 2.5 % grid.

Three further differences, each measured:

- **No 0.250 floor.** #056 (`manufacturing…2023.pptx` p2) states 48 pt and 16 pt runs in one body.
  The reference draws them at **9.014** and **3.005 pt** — a scale of 0.1875. We stop at the floor,
  draw 12 pt, and run the last bullets off the bottom of the slide. This is the whole of that
  document's `overlap-clip`.
- **The leading is never tightened at full size.** No probe box answers `{1.000, 0.900}`, the
  table's first row. On the corpus, 24.2.7.2's line pitch is a clean 1.2 em wherever it does not
  shrink: #124's reference draws 17.008 pt text on a 20.410 pt pitch (1.1999), #168's 21.005 on
  25.200 (1.1997).
- **Which side each document falls on is version-consistent.** `Lepore.ppt` page 2 is the case that
  settles it. `SlideAutofit.cs:250-255` records, as a 26.2.4.2 measurement, that the reference
  there draws *"eleven text baselines at 20.013 pt … and six bullets at 20.409"*. **Our render
  draws exactly 20.013 and 20.409. This container's reference draws 21.005 and 21.005.** We
  reproduce 26.2.4.2 to three decimals and the reference is a different binary.

### 5. This is where the "bullet glyphs sit high" cluster goes — it is not a bullet bug

#079 #124 #138 #168 were briefed as a probable baseline-alignment fault. They are not.

`SlideTextLayout.EmitMarker` (`:435-459`) places a symbol marker at
`top + line.Height − line.TextHeight/2 + (ascent − descent)/2`, which is
`ImpCalcBulletArea` + `StripBullet`. The arithmetic is right. Measured on #124 page 2, in hundredths
of a millimetre (Carlito text, Liberation Sans marker, font-independent line spacing so
`Ascent = em` and `Height = TextHeight = 1.2 em`):

| | ref | ours |
|---|---:|---:|
| text em | 600 (17.008 pt) | 635 (18 pt) |
| marker em | 593 (16.809) | 635 (18) |
| line Height | 720 | **686** — 1.2 × 635 × **0.900** |
| line TextHeight | 720 | 762 |
| marker above text baseline, predicted | 741 − 889 + 444.5 − 254.5 = 42 → **1.19 pt** | 635 − 686 + 381 − 220 = 110 → **3.11 pt** |
| marker above text baseline, **measured in the PDF** | **1.162 pt** | **3.104 pt** |

The formula predicts both sides to a hundredth of a point. The whole 2.14 pt is `FitLevels` row 0's
spacing scale, applied where the reference applies none. `Spaced()` (`:1357-1370`) scales
`Height` and leaves `TextHeight` alone — correctly, that is EditEngine — and the bullet is centred
on the text height inside the line's, so a leading reduction lifts it. **Fix the table and the
bullets fall into place; there is nothing else to fix here.**

Same arithmetic on #168 page 5: ref +1.162 pt at 21.005 pt text, ours +3.756 at 22.0.

### 6. The proposed change, and the decision it is gated on

`patches/autofit-version-divergence.diff` — replaces `FitLevels` with the measured 2.5 % grid at
spacing 1.0 and drops the floor to one grid step.

**It is a retarget, and it must not be applied on my say-so.** The tree is *correct for 26.2.4.2*.
Applying this makes it correct for this sweep's reference and wrong for the project's, and it will
fail `SlideAutofitTests`, `SlideMarkerScaleTests` and both `SlideAutofit*ComparisonTests`, all of
which pin 26.2.4.2 answers. The alternative — **re-render the reference bank at 26.2.4.2** — leaves
the tree alone and re-scores these 24 documents instead. `dotnet/CLAUDE.md`'s own table says slides
were the family *least* disturbed by the version move, but that is a statement about **page
counts** (a deck's page count is its slide count) and is silent about shrink; the same table records
that **160 of 163** slides documents moved on *words*, which is exactly what a different autofit
does.

What the patch does *not* reproduce, stated so nobody mistakes it for complete: 24.2.7.2 tightens
leading on 8 of the 36 probe boxes (0.9 or 0.8 of natural pitch, always alongside a reduced font),
and reproducing *which* boxes needs 24.2.7.2's own `autoFitTextForCompatibility`, whose source is
in neither the installed binary nor this 27.2 tree. I did not guess at it. 24.2 also allowed
`aCurrentTextBoxSize.extendBy(0, -50)` — 1.417 pt of slack — where `Solve` now uses `+1`; that too
is unreproduced and would shift a handful of boundary cases.

### 7. The probe that would refute me

One deck, one 40 pt paragraph, box height stepped 1 pt across a boundary the grid predicts and the
table does not — 40 × 0.825 = 33 pt is the sharpest, since the table has no row between 0.850 and
0.775. If the reference never draws 33 pt at any box height, the answer set is not a 2.5 % grid and
my nine sizes were an artefact of the 12 pt sampling. `probe/fit/make.py` takes the step as its only
edit. The cheaper refutation: if the installed `soffice` were 26.2.4.2, `Lepore.ppt` p2 would draw
20.013/20.409 and it draws 21.005/21.005.

### 8. Confidence, and what I did not establish

**High** that the reference's answer set is off the table and that this drives the 24 documents.
**High** that the bullet-height cluster is this and not a placement bug (the formula predicts both
sides to 0.03 pt). **Not established:** the exact 24.2.7.2 search, the spacing dimension, and
whether the 0.025 grid holds below 0.250 (I have one corpus point, 0.1875, and no probe box there).

---

## B. `PPT_PST_TextRulerAtom` is declared and read by nobody

**#126 and #129 measured; latent in all 14 `.ppt` documents in the lane.**

### 1. What the pages show

#126 (`Aerospace_Journey_of_Flight_Chapter_*.ppt` p5): the bullets sit "flush against the text with
no gap after them". #129 (`iep-amount-frequency-for-webinar.ppt` p2): "the indented explanatory
paragraph is set with a deeper left indent than the reference's". Both sides draw **the same point
sizes** on both documents, so root cause A is not in play here.

Text pens, from the content streams:

| | ref marker x | ref text x | our marker x | our text x |
|---|---:|---:|---:|---:|
| #126 level 0 | 7.625 | **26.617** | 7.625 | **13.580** |
| #126 level 1 | 45.638 | **60.009** | 43.625 | **49.580** |
| #129 level 0 | 31.436 | **55.672** | 31.436 | **58.436** |
| #129 deeper | — | **58.564** | — | **89.936** |

Our text sits at `marker x + the marker's own advance` — i.e. the hanging indent collapses onto the
glyph, which is `MarkerReach` (`SlideTextLayout.cs:661-690`) doing its job because nothing else
gave the first line anywhere to be.

### 2. What the document actually contains

`Aerospace…ppt`'s body shape carries a 24-byte `TextRulerAtom` (record type 4006), raw:

```
f91e0000 6102 9800 a301 3001 6102 6102 9103 9103 c204 c204
flags = 0x00001EF9
  bit 0  -> defaultTab   = 0x0261 = 609 master units
  bit 3  -> textOfs[0]   = 0x0098 = 152          bit 8  (bulletOfs[0]) NOT set
  bit 4  -> textOfs[1]   = 0x01A3 = 419          bit 9  -> bulletOfs[1] = 0x0130 = 304
  bit 5  -> textOfs[2]   = 609                   bit 10 -> bulletOfs[2] = 609
  bit 6  -> textOfs[3]   = 913                   bit 11 -> bulletOfs[3] = 913
  bit 7  -> textOfs[4]   = 1218                  bit 12 -> bulletOfs[4] = 1218
```

The master's Body levels say `textOfs 228 / buOfs 0` and `textOfs 495 / buOfs 304` — **different
numbers.** In points (576 master units to the inch): the ruler's 152 → 18.99 pt, 419 → 52.375,
304 → 38.00. The reference draws that page's level-zero text **18.99 pt** from the text area's left
edge, its level-one text **52.384**, and its level-one bullet **38.013**. Three independent numbers,
the ruler's to a hundredth of a point, and none of the master's.

Every `.ppt` in this lane carries these (record count / stated level offsets):
`FAA_Form_337` 491/1962, `iep-amount-frequency` 400/1570, `2015-Civil-Rights` 232/934,
`Lepore` 122/485, `RRM-training-syllabus` 119/472, `Aerospace` 117/1018,
`joint_user_outcomes` 123/479, `RESPA` 111/442, `ws_prod-…France` 54/270,
`undp_presentation` 50/241, `Inducement-to-Insurance` 25/98, `berlin` 14/94,
`BUS-Chapter 05` 8/28, `Employment-Based_I-485` 1/5. (`probe/` scan.)

### 3. Where it lives in the source

`dotnet/src/XRay.Office.Presentations/MsBinary/PptRecordTypes.cs:84` declares
`TextRulerAtom = 4006` with a doc comment describing exactly what it holds — and
`git grep TextRulerAtom -- 'dotnet/src/**/*.cs'` returns **that line and nothing else**. It is the
"read but never consumed" pattern in its purest form: the constant is the whole of the support.

The consumer that should exist is `PptTextBody.Paragraph` at `:178-182`, which resolves
`textOffset`/`bulletOffset` as `paragraph-stated ?? master level`, with no rung between them.
LibreOffice's rung is `ReadParaProps`' tail (`svdfppt.cxx:5062-5068`): the ruler's value is written
into the property set **and its mask bit is set with it**, so a level the ruler speaks for never
reaches the master at all.

### 4. The proposed change

`patches/ppt-text-ruler.diff`. Adds `PptTextRuler` to `PptTextReader` (flags word, then the
interleaved `8 << level` / `256 << level` fields in file order — a flag missed is not one value lost
but everything after it read from the wrong offset), hangs it on `PptTextRun`, and consults it in
`PptTextBody.Paragraph` only where the paragraph itself stated nothing. The ruler's `defaultTab`
wins over the master outright, as `GetDefaultTab` is asked unconditionally (`svdfppt.cxx:5069`).

**Deliberately not in the patch:** the ruler's explicit tab-stop list. It is parsed past rather than
kept, because `SlideParagraph` has no `TabStops` at all and no slide reader ever populates
`ParagraphFormat.TabStops` — see the follow-on note at the end.

### 5. The probe that would refute me

A one-slide `.ppt` with a body whose ruler states `textOfs[0]` different from its master's, rendered
both ways: if the reference follows the master rather than the ruler, the rung does not exist. The
cheaper version already exists in the corpus — `Aerospace…ppt` p5 is that probe, and its three
numbers are the ruler's.

### 6. Confidence

**High** for the mechanism and the field order (three exact matches on one document, a fourth
signature on a second). **Medium** for reach: I established that all 14 `.ppt` documents carry
rulers, not that all 14 rulers disagree with their masters.

---

## C. `PPT_PST_ExtendedParagraphAtom` is read by nobody, so numbered and picture bullets vanish

**#137, #086, #053.**

### 1. What the pages show

- **#137** `undp_presentation_revised_17_may.ppt` p2: the reference numbers the outline
  `I. II. III. IV. V. VI.`; we draw six round bullets.
- **#086** `RESPA_-_Section_8_Webinar.ppt` p4: the reference labels four paragraphs
  `(a) (b) (c) (d)` in green and leaves the fifth unlabelled; we draw four round bullets. Confirmed
  from the reference's own text layer, which extracts `(b) No person shall give…`.
- **#053** `ws_prod-…France.ppt` p5: the reference marks each item with an orange arrow and the
  sub-item with a star; we draw no marker at all. **Those are not glyphs.** The reference page has
  six `Do` operators, and four of them place `Im29` at 22.309 pt at x = 43.087 on the four bullet
  lines, with `Im98` at 14.258 pt on the sub-item. They are *images*.

### 2. What the documents actually contain

The paragraph property runs say nothing about any of it. #086's shape states, for all four
paragraphs, `buFlags=15 buChar=0xF0A1 buFont=0` — an ordinary bullet in Gill Sans MT.

The numbering is in the shape's private data. `#086`'s body shape carries, in its `ClientData`
under `ProgTags` → `ProgBinaryTag "___PPT9"` → `BinaryTagData`, a 52-byte record **4012**
(`PPT_PST_ExtendedParagraphAtom`) whose entries decode as:

```
ext[0]  mask=0x03800000  buBlip=0xFFFF  hasAnm=1  anmScheme=0x00010008   -> scheme 8, start 1
ext[1]  mask=0x00000000                                                  -> nothing
ext[2]  mask=0x00000000
```

Scheme 8 is `SVX_NUM_CHARS_LOWER_LETTER` with `"("`/`")"` (`svdfppt.cxx:3526-3531`) — `(a)`. The
character runs select the entry: their flags carry `extIdx` 0, 0, 1, 2, so entry 0 covers the first
1278 characters and entry 1 covers the rest, which is exactly why the reference labels four
paragraphs and not the fifth.

`#137` carries one entry, raw body
`00008003 ffff 0100 07000100` → `mask=0x03800000 buBlip=0xFFFF hasAnm=1 anmScheme=0x00010007` —
**scheme 7, start 1**, which is `SVX_NUM_ROMAN_UPPER` with a `"."` suffix (`svdfppt.cxx:3550-3554`),
i.e. `I. II. III.` — exactly what the reference's own text layer extracts
(`I. UNDP's Position / II. UNDP Mandate… / VI. Cost structure`) and exactly what we replace with
six `•`. It is reached through the document-level presentation-rules route (a
`PPT_PST_ExtendedParagraphHeaderAtom`, 4015, keyed by slide id and outline-ref number) rather than
through the shape, and both routes exist in one corpus.

`#053` carries five 4012 records whose entries state `buBlip=0` — **a picture bullet**, index 0 into
the BLIP store — with `hasAnm=0`.

Record counts, from `probe/pptdump.py` (which must recurse into `BinaryTagData`, an *atom* that
holds records — a walker that only recurses into containers sees none of this):

| doc | 4012 | 4013 | 4015 |
|---|---:|---:|---:|
| FAA_Form_337.ppt | 92 | – | – |
| RESPA…ppt | 75 | – | – |
| ws_prod…France.ppt | 5 | 6 | 5 |
| undp_presentation…ppt | 1 | – | 1 |

### 3. Where it lives in the source

Nowhere. `PptRecordTypes.cs` has no 4012/4013/4015 and no `ProgBinaryTag`/`BinaryTagData`;
`PptTextReader.Read` walks only the client text box's own siblings, which is a range the atom is
never in. `PptTextBody.Marker` (`:252-301`) therefore sees only the paragraph's stated bullet, and
its `?? '•'` default is what reaches the page.

### 4. The proposed change

`patches/ppt-extended-paragraph-numbering.diff`. Adds the three record types, a
`PptExtendedParagraph` record and its parser (three masks with optional fields hanging off each, in
`StyleTextProp9::Read`'s order), the per-character-run `ExtendedIndex` from flags bits 10–13, the
`ClientData → ProgTags → ___PPT9 → BinaryTagData` lookup, and a numbered-marker branch in
`PptTextBody.Marker` that outranks the paragraph's stated bullet.

The number itself is **not** computed here: `PptNumbering` maps PowerPoint's scheme number onto the
`ST_TextAutonumberScheme` *name* and hands the counters to `DrawingTextBody.AutoNumber`, which is
public for precisely this reason and already owns "where the run starts, what breaks it, how a
nested level restarts". A scheme with no DrawingML name (the CJK, Hebrew, full-width and circled
families) draws no number and leaves the paragraph's own bullet, rather than substituting a Latin
numeral for a Chinese one.

**Scope limits, stated rather than buried.** (i) The patch covers the *shape* route only; #137's
presentation-rules route (4015, matched on slide id and outline-ref number) is not implemented, so
#137 is diagnosed but not fixed by it. (ii) Picture bullets are parsed (`BulletBlip`) and not drawn:
`SlideMarker` has no image, and giving it one is a layout-model change I would not make blind — so
#053 is diagnosed, not fixed.

### 5. The probe that would refute me

Grep the four decks for a `buAutoNum`-equivalent in their *paragraph* properties: there is none, and
that is already the check. The refutation that matters is the other way round — if the reference
numbered #086 from something in the paragraph runs, the scheme byte would not have to be 8, and
scheme 8's decode (`(a)`) matching the reference's own extracted text layer character-for-character
would be a coincidence.

### 6. Confidence

**High** for the mechanism and the two decoded schemes. **Medium** for the patch, which is 200
lines of untestable record navigation: the field order is `StyleTextProp9::Read`'s and the entry
index is `ReadCharProps`', but neither has been run.

---

## D. A marker never asks for glyph fallback, so a recoded slot draws `.notdef`

**#156** `FAA_Form_337.ppt` p4.

### 1. What the page shows

The reference marks five items with filled circled numerals ❶–❺ in yellow. We draw no marker, and
the text starts where the reference's text starts — so the marker is absent, not misplaced.

### 2. What the document actually contains

Five paragraphs, each stating `mask=0x000000FE buFlags=15 buChar=0xB6..0xBA buFont=3
buHeight=105 buColour=0xFE00CCFF`, with font 3 = **Monotype Sorts, charset 2**. The master's Body
level 0 states `buFlags=15` (bullet on), so the bullets are on.

`0xB6` in Monotype Sorts recodes to **U+2776** — `aMonotypeSortsTab`'s `F0B0` block,
`unotools/source/misc/fontcvt.cxx:606-610`, which our `SymbolFontRecode.Tables.cs` ports verbatim.
That is the right answer, and it is where it goes wrong.

### 3. Where it lives in the source

We *do* emit the marker, correctly recoded. Our page-4 stream holds
`1 0.8 0 rg BT 55.2472 354.9402 Td /F4 31.0961 Tf <01>Tj ET` five times, and `pdftotext` extracts
`❶` (U+2776) from all five.

**OpenSymbol has no glyph for U+2776.** Read out of `opens___.ttf`'s own `cmap` (3,1):
U+2776…U+277A → glyph **0**. So all five shape to `.notdef`, our subsetter folds them onto one
code, and nothing is drawn. The reference resolves the same U+F0B6 through the same table and then
**falls back**: its page-4 `/F1` is `BAAAAA+DejaVuSans` and it draws glyph codes 02, 03, 04, 05, 06
— five distinct glyphs — while keeping U+F0B6…U+F0BA in its `ToUnicode`.

The seat is `SlideTextLayout.Shaped` (`:520-589`): it resolves a face, optionally swaps in
OpenSymbol for a recode, and calls `TextShaper.Default.Shape(face, text, default)` **with no
fallback resolver**. Twenty-five lines further down, the run path does the opposite —
`MeasuredParagraph.Measure(..., new ItemisationOptions { GlyphFallback = fonts.Fallback })`
(`:759-760`). `SlideFonts.Fallback`'s own remarks (`SlideText.cs:623-645`) describe this exact
failure — *"the text is invisible and no gate column sees it"* — for runs. The marker path was
never wired to it. This is the "an outline bullet resolves on a separate path from the runs"
warning in `XRay.Office.Presentations/TODO.md:1014-1015`, arriving from the other direction.

`Wingdings 2` slot `0xF4` (#053's master bullet) recodes to U+E5CD, which no installed face holds,
so the same hole is reachable from a second table.

### 4. The proposed change

`patches/marker-glyph-fallback.diff` — five lines in `Shaped`: if the resolved face has no glyph for
the marker's code point, take `fonts.Fallback.FallbackFor(...)` and its `ReferenceFor(..., italic)`,
which is the same pair `Block.FontFor` uses so a fallback face reaches the PDF writer *with its
font program*.

### 5. The probe that would refute me

A one-slide deck with `a:buChar char="&#xF0B6;"` and `a:buFont typeface="Monotype Sorts"`. If the
reference draws nothing there, LibreOffice is not falling back and the missing marker is elsewhere.
It is already answered on the corpus: the reference's page-4 font dictionary names DejaVu Sans for
the marker run and OpenSymbol for nothing on that page.

### 6. Confidence

**High.** Both halves are read directly out of the two files — our `.notdef`, the font's `cmap`, and
the reference's font resource — and the fix reuses machinery already proven on the run path.

---

## E. The PPT bullet's face ignores `PPT_ParaAttr_BuHardFont`

**#126**, alongside root cause B on the same page.

### 1. What the page shows

`Aerospace…ppt` p5: the reference draws every bullet from `BAAAAA+LiberationSans` — the face of the
Arial text beside them. We draw them from `CAAAAA+LiberationSerif`. The case note's "drawn smaller
than the reference's" is the same thing: a serif bullet at the same em is a smaller mark.

### 2. What the document actually contains

The master's Body level 0 states `buFlags=1` — `BulletOn` and nothing else, so `BuHardFont` is
**clear** — beside a `buFont` of 0, which the font collection resolves to *Times New Roman*. The
deck's text is font 1, Arial.

### 3. Where it lives in the source

`PptTextBody.Marker:265`:

```csharp
ushort font = properties.States(StatesBulletFont) ? properties.BulletFont : level.BulletFont;
```

The word is taken unconditionally. LibreOffice gates it:
`PPTParagraphObj::GetAttrib`, `svdfppt.cxx:5918-5942`, in *both* the hard and the inherited branch —
if `BuHardFont` is clear, `rRetValue` is *"the font used which assigned to the first character of the
following text"*.

The tell that this is the "read but never consumed" pattern rather than an oversight is eight lines
below, at `:275-277`: the identical gate **is** implemented for the colour, with a paragraph of
commentary and a corpus citation. And `PptParagraphRun.BulletFlags`' own XML remarks
(`PptTextReader.cs:49-58`) name all three flags — *"bits 1, 2 and 3 are `PPT_ParaAttr_BuHardFont`,
`BuHardColor` and `BuHardHeight`"* — while only bit 2 has a consumer.

### 4. The proposed change

`patches/ppt-bullet-hard-font.diff` — the mirror of the colour gate, plus a `TextFontAt` helper that
finds the character run covering the paragraph's first character, which is what `GetAttrib` falls
back to.

### 5. The probe that would refute me

A `.ppt` whose master states `BuHardFont` *set* with a symbol bullet font: if the reference then
draws the level's face rather than the text's, the gate reads the other way. The corpus half is
already done — the flag is clear on `Aerospace…ppt` and the reference takes the text's face.

### 6. Confidence

**High** on the rule and the document; **medium** on reach, since I checked the flag on one deck.

---

## F. `lo-broken` — confirmed and filed, not chased

- **#162** `NAS-Infrastructure-Roadmaps-v16.0.pptx` p4. Read directly: the reference renders a title,
  a footer and an empty page; we draw all four quarter tables with their headers, entries, totals
  and rules. `ink ×11.78` is the whole story. Ours is the correct output.
- **#084** `Ramp Up Campaign - French.pptx` p3. Read directly: the reference's title runs straight
  over *L'importance d'une campagne* and *Soyez Prêts signifie* overlaps its own body; ours is
  clean. The deck states only `a:spAutoFit` (34 of them, no `normAutofit`), so this is not the
  autofit cluster. One real difference remains on our side and is worth recording rather than
  hiding: our headings are set in a heavier face than the reference's, which is a font-substitution
  question, not a layout one.
- **#130** `AATF-Fact-Sheet-2025.pptx` p2. Both sides draw identical size histograms
  (17.01×287, 14.0×11, 11.99×5, 36.0×3); the divergence is the reference clipping its last bullet
  under the footer band. Filed as described.

---

## Refutations — mechanisms named in the brief that the files do not use

1. **#113 "A tab stop is not honoured" is wrong about the file.** The reference pushes
   *Air Canada - YYZ* right with **thirteen literal spaces**, not a tab:
   `'Mr. Gerry Pipe, Mgr. Corporate Safety             Air Canada - YYZ'`. There is no tab character
   in the paragraph. The gap closes on our side because our text is 32 pt against the reference's
   31.01 — root cause A — so the line no longer fits and the affiliation wraps. Chasing tab stops
   here would have cost a day and fixed nothing.
2. **"Bullet glyphs sitting high" is not a baseline-alignment bug.** See A §5: `EmitMarker`'s
   centring predicts both sides to a hundredth of a point once the fit's spacing scale is in the
   arithmetic.
3. **"Placeholder measure" — no measure defect found.** On #030 both sides draw the marker at
   x = 60.52 and the text at x = 87.52; on #129 both draw the marker at x = 31.436. The layout and
   master inheritance chain delivers the same text area on both sides. What was read as a wider
   measure is (a) the font size, root cause A, and (b) on the `.ppt` documents, the *indent* inside
   that area, root cause B.
4. **The "list markers" cluster is three faults, not one.** Numbered/lettered markers are C
   (a record we never read), a missing glyph is D (fallback), and a wrong bullet face is E (a flag
   we never consume). They share no code.

---

## Not attributed to this lane

**#096, #120, #147, #172** — both sides draw the same sizes at the same pens and differ only in
where lines break. `#147` and `#172` are described as marginal in the brief and measure that way
(`REF 28.01×9, 29.99×8` against `OUR 28.01×9, 29.99×8`; `REF 15.99×26` against `OUR 15.99×35`, the
count difference being run splitting rather than size). This is the advance-width divergence
recorded in `dotnet/CLAUDE.md` rule 3 — grid-fitted against unhinted advances — which is neither a
presentations defect nor mine to fix.

---

## Follow-on, filed but not patched

**Explicit tab stops never reach the slide layouter, in any of the three readers.**
`ParagraphFormat.TabStops` exists and `ParagraphLayouter` consumes it, but `SlideParagraph` has no
`TabStops` property and `SlideTextLayout`'s `ParagraphFormat` (`:743-752`) sets only
`DefaultTabInterval`. The three sources are all discarded: `PptStyleSheet.SkipTabStops` (`:432`)
steps over the master level's list, `PptTextRuler` (this lane's patch B) steps over the shape's, and
`PptxTextBody` never looks for `a:tabLst`. LibreOffice's mapping is not in doubt —
`svdfppt.cxx:6338-6367`, style 1 centre, 2 right, 3 decimal, else left — but **no document in this
lane needs it**: #113 was the candidate and it uses spaces. Four `.ppt` decks here do carry ruler
tab stops (`undp` 14, `2015-Civil-Rights` 4, `RRM` 1, `Lepore` 1), so it will surface. I did not
write the patch because it spans three readers' tab vocabularies with nothing here to validate two
of them against, and `AGENTS.md` forbids a fix tuned to one sample.

---

## Cross-lane dependencies

**None in source.** All five patches touch only `dotnet/src/XRay.Office.Presentations/**`.

Two things outside my ownership that the applier must handle:

1. **Tests pin the autofit's 26.2.4.2 answers.** If `autofit-version-divergence.diff` is applied,
   `dotnet/tests/XRay.Office.Presentations.Tests/SlideAutofitTests.cs`,
   `SlideMarkerScaleTests.cs`, and
   `dotnet/tests/XRay.Office.Fidelity.Tests/SlideAutofit{DeviceGrid,ParagraphSpace}ComparisonTests.cs`
   must move with it. They are correct as they stand for the project's reference.
2. **The reference-bank decision.** Root cause A is a fork in the road for the whole sweep, not just
   for slides: either the tree is retargeted to 24.2.7.2, or `/data/bench/lo/` is re-rendered at
   26.2.4.2. Lane L2 reports the same shape of finding on `w:trHeight` and section-break spacing, so
   this needs deciding once, above the lanes.

## Patch inventory and apply order

| patch | root cause | files | `git apply --check` |
|---|---|---|---|
| `ppt-text-ruler.diff` | B | `PptTextReader.cs`, `PptTextBody.cs` | clean |
| `ppt-bullet-hard-font.diff` | E | `PptTextBody.cs` | clean |
| `marker-glyph-fallback.diff` | D | `SlideTextLayout.cs` | clean |
| `ppt-extended-paragraph-numbering.diff` | C | `PptRecordTypes.cs`, `PptTextReader.cs`, `PptSlideLayout.cs`, `PptTextBody.cs`, **new** `PptNumbering.cs` | clean |
| `autofit-version-divergence.diff` | A | `SlideAutofit.cs` | clean — **gated on the decision above** |

Verified in a scratch clone: `ppt-text-ruler` → `ppt-bullet-hard-font` → `marker-glyph-fallback` →
`autofit-version-divergence` apply cumulatively without conflict.
`ppt-extended-paragraph-numbering` applies alone but collides with `ppt-text-ruler` in exactly two
hunks — both add a trailing optional parameter to the `PptTextRun` record and to
`PptTextReader.Read`. Keep both parameters (`Ruler` and `Extended`) and both arguments at the two
call sites; there is no semantic overlap.

## Reproducing anything here

```
probe/pptdump.py <file> [types]   record-type census, recursing into BinaryTagData
probe/extpara.py <file>           StyleTextProp9 entries: buBlip / hasAnm / scheme / start
probe/stp.py, stp2.py <file> <needle>   StyleTextPropAtom paragraph + character runs
probe/master.py <file>            TxMasterStyleAtom levels: bullet flags, char, font, offsets
probe/fonts.py <file>             FontEntityAtom collection with charsets
probe/sizes.tsv                   drawn /Tf sizes on every divergent page, both sides
probe/fit/make.py, fitcurve.tsv   the 36-deck autofit probe against the installed 24.2.7.2
```
