# slides-paint-01 — image transparency, missing shadow, missing underline

Three two-deck clusters the user named in `dotnet/probes/user-review-slides-02/review.md`.
The prediction is in `prediction.md` beside this file and was committed as `c39a1c271c9`
**before any measurement**; nothing in it has been edited since.

- **Reference binary: LibreOffice 26.2.4.2** 620(Build:2). `check-env.sh` green on all five
  checks: soffice converts, Calibri→Carlito, Cambria→Caladea, Arial→Liberation Sans,
  Times→Liberation Serif, Courier→Liberation Mono, DejaVu Sans→DejaVu Sans; pdftoppm and
  pdftotext both 26.01.0.
- Reference PDFs: `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/slides/` (163, per-format identity).
- Ours: `wt-slides-paint` at `300b6c0980e`, built **0 warnings, 0 errors**.
- **Nothing under `dotnet/` has changed on the slides path since the review sheet's commit
  `445f253bd0f`** (`git log 445f253bd0f..HEAD -- dotnet/` is seven word-track commits and the
  review itself), so every page measured here is the page the user was looking at.

**Verdict movement: zero, as predicted.** Slides is 163/163 page-exact; a knocked-out
background, a duplicated glyph run and a 3 pt rule move no gate column. Nothing was changed.

---

## Summary

| cluster | reproduces | mechanism | one cause for both decks? |
|---|---|---|---|
| 1 image transparency | **half** | `1-secretariat` yes — Escher property 263 unread. `pres_ioc_phuket` **no** — its images are byte-for-byte right | **no**, and phuket is not a transparency defect at all |
| 2 missing shadow | **yes, both** | the `.ppt` character shadow bit `0x0010` is unread; we draw nothing | **yes** |
| 3 missing underline | **yes, both** | `Stakeholders` = hyperlink underline the importer supplies; `16 - UTM` = a gradient-filled `a:ln` we drop | **no** — two unrelated causes |

Four decks out of six reproduce as the user described. The two that do not — `pres_ioc_phuket`
and the *title* half of `16 - UTM` — are still real defects on the exact page the user looked
at; the user's words point at the right pixels and name the wrong feature. Both are located
below to file and line.

---

## Cluster 1 — image transparency

### `1-secretariat.ppt` — reproduces exactly, and the user named the right feature

**Page 1**, which is the deck's worst page (`|ink|` 5.15%, raw diff 20.93%, the only MAJOR
region on it). The IAOPA winged logo sits on a photograph of a globe. The reference shows the
globe through the logo's background; **we paint an opaque white rectangle over it**.

Measured, in the file:

- The shape carrying the logo states Escher property **263 `pictureTransparent` = `0x00FFFFFF`**
  (white), alongside `pib = 2`, `pictureContrast = 0x11057` and `pictureBrightness = 0x1EB8`.
- `pib = 2` is the second entry of the `OfficeArtBStoreContainer`, whose order is
  `JPEG, PNG, EMF×5, PNG, EMF×3`. So the picture is a **PNG**, 362×186, palette, and — measured
  from the `Pictures` stream — it has **0 transparent pixels of 67 332** as stored.
- LibreOffice's own import turns that into **51 361 of 67 332 fully transparent**: round-tripping
  the deck through `soffice --convert-to odp` at 26.2.4.2 writes
  `Pictures/100000010000016A000000BAB05F9E3D.png` as RGBA with exactly that count at alpha 0 and
  none in between. Nothing but property 263 can have done that.
- The reference PDF agrees: page 1 holds **two** 362×186 images, each with a 852-byte `/SMask`.
  Ours holds **one**, with **no** `/SMask`, and its 2869 bytes match the reference's 2899.

**Mechanism, in our tree.** There is no handling of this property anywhere:
`grep -rn "clrChange\|ClrChange\|TransparentColour\|TransparentColor\|pictureTransparent"` over
`dotnet/src` and `dotnet/tests` returns **zero lines**. The reader that would carry it is
`src/XRay.Office.MsBinary/Escher/EscherPicture.cs`, which reads 256–259 (crop) and nothing else,
and the seat for the result is `RasterImage` in
`src/XRay.Office.Core/Graphics/GlyphRun.cs:143`, which already carries two deferred recolourings
(`Duotone`, `Luminance`) applied by the decoder in `XRay.Office.Rendering` — a colour knockout is
the same shape of thing and belongs beside them. **It is a missing feature, not a wrong one.**

**What LibreOffice does, read from source and consistent with the measurement.**
`filter/source/msfilter/msdffimp.cxx:3894-3903` → `Bitmap::CombineMaskOr`
(`vcl/source/bitmap/bitmap.cxx:2517`) → `Bitmap::CreateAlphaMask`
(`vcl/source/bitmap/bitmappaint.cxx:684`). An **independent per-channel box of ±9** around the
stated colour — `nMinR ≤ R ≤ nMaxR && nMinG ≤ G ≤ nMaxG && nMinB ≤ B ≤ nMaxB` — producing
**binary** alpha, OR-combined with any alpha the picture already had, and applied **only when
`aGraf.GetType() == GraphicType::Bitmap`** (so a WMF or EMF carrying property 263 gets nothing).
The 51 361 / 0-or-255 measurement above is exactly binary alpha and is consistent with a ±9 box,
but does not on its own pin the tolerance to 9 rather than 0 — that number is read from source
(and from the 27.2 tree), so treat **±9 as inferred and "binary, on white, on a bitmap only" as
measured**.

**A second debit on the same shape, from the same missing property.** The reference's two
362×186 images are the picture (`r-003`) and a **grey silhouette of it** (`r-001`, flat
`#808080`, same 852-byte mask), drawn offset — the Escher shadow, `shadowOffsetX/Y = 76 200 EMU`
= 6 pt, `shadowOpacity = 0x8000` = 0.5, colour resolving to the `0x00808080` default. We draw no
shadow, and `SlideDrawing.DrawShadow`
(`src/XRay.Office.Presentations/Layout/SlideDrawing.cs:199-215`) says why in its own comment: it
casts a picture's silhouette only when the raster is an opaque JPEG, because a PNG's alpha is
not visible at that layer and a logo would gain a black rectangle. That rule is right and it
loses this shadow — and **implementing 263 is what would give this picture the alpha the rule is
waiting for**. The two halves of page 1 are one fix.

### `pres_ioc_phuket.ppt` — does **not** reproduce; the deck has no transparent colour at all

This is where the prediction was wrong and it is worth stating plainly.

- The deck states property **263 nowhere**. Its `OfficeArtFOPT` picture properties across the
  whole `PowerPoint Document` stream are 257, 259, 260, 261, 262, 324, 327, 328 — crop, `pib`,
  `pibName`, `pibFlags` and geometry. No 263.
- Its five blips are three palette PNGs, one RGB PNG and one JPEG. Two of the PNGs carry **their
  own** transparency (72 649 / 207 835 and 11 984 / 58 322 pixels), and LibreOffice's ODP
  round-trip writes back exactly those counts — it added nothing.
- **We honour it.** Comparing image XObjects page by page over all 26 pages, ours and the
  reference's agree in count, kind and size everywhere, `/SMask` included, with the masks
  identical in byte length (7 920 B and 694 B). There is exactly **one** difference in the whole
  deck: page 26 has an 851×46 image in the reference that we do not draw.

**What the user actually saw**, on page 26 — which is the deck's worst page by `|ink|` (5.23%)
and therefore the page on the review sheet. The slide's title band is a shape with a
**gradient fill and text**. The reference paints the gradient *clipped to the glyph outlines*
and PDF-exports the clipped gradient as that 851×46 image (extracted and read: a plain
yellow→orange horizontal ramp), leaving the navy band and the yellow title legible. We paint the
gradient over the shape's **whole rectangle** — ~100 banded fills from `#FFFF00` at
`(12.00, 491.12)-(693.38, 528.00)` down to orange — plus two `#C0C0C0` rectangles for its shadow,
and the title disappears under a solid block.

A reviewer seeing a bright opaque slab where a title should be, on a deck whose other slides are
full of logos, will call that "missing transparent color handling for many images". The words are
a fair description of the pixels and the wrong name for the cause. **It is a WordArt/gradient-fill
defect and belongs to whoever owns shape text, not to this cluster.** I am recording it rather
than folding it in.

### Reach and direction, cluster 1

Censused by **walking records** over all 163 decks — `olefile` for the CFB then a recursive walk
of the `PowerPoint Document` record tree (`recVer == 0xF` means container) for `.ppt`, `zipfile`
+ `ElementTree` for `.pptx`. **No regex.** 163 documents, 51 `.ppt` + 112 `.pptx`, **zero read
errors**. Script: `census.py`, reproduced under `probes/slides-paint-01/`.

| | decks | instances |
|---|---:|---:|
| `.ppt` with Escher property 263 | **8 of 51** | 91 |
| `.pptx` with `a:clrChange` | **19 of 112** | 81 |
| either | **27 of 163** | |

The `.ppt` leaders are `Airport Planning 09112013` (26), `71393_pp7` (21), `EG1_dsrc tech` (16),
`Lepore` (13), `Fundamentals_Module_1_basics` (9), `Employment-Based_I-485` (4), then
`1-secretariat` and one `ws_prod-g-doc` deck at 1 each. The `.pptx` leaders are
`16 - UTM - (NASA)` (15 — the same deck as cluster 3, which the user did not flag for it),
`Technical_Report_Elements[1]` (15), `BasicMed_AME_Presentation` (14) and
`redac-nasops-201503-RIRP-portfolio-update` (13).

Two honest limits on those counts, both named in the prediction: an instance count is **not** a
shape count, because a `.ppt` written by incremental save keeps superseded `OfficeArtFOPT`
records in the stream and the walk sees all of them; and a stated transparent colour that the
artwork never uses is inert. Both make these **upper bounds**. The deck counts are sound.

**Direction — and the prediction was wrong here.** I predicted we draw *more* ink. On
`1-secretariat` page 1 we draw **less**: the knocked-out colour is white, what shows through is a
dark blue globe, so the reference is the darker page and the missing grey shadow costs us more
ink again. The correct general statement is that direction is not fixed — we always paint the
knocked-out colour where the reference paints whatever is behind it, and which is darker is a
property of the slide. On the one deck that reproduces, it is ink missing from ours.

---

## Cluster 2 — missing shadow

**Both decks reproduce, both share one cause, and we draw nothing at all rather than drawing it
wrongly.** That is the distinction the review could not make, and it is the first half.

### It is the legacy character bit, not `a:outerShdw`

Both decks are `.ppt`. `Thailand17.ppt` page 32 (its worst page, `|ink|` 8.02%) has the header
**HAWAII BULLETIN CRITERIA**. The reference draws that 24-glyph run **twice**; we draw it once.
Rasterised at 300 dpi the reference shows a hard-edged black shadow down and right of the red
title, with no blur; ours has none.

### The rule, measured at three font sizes

| deck / page | size | reference text at | reference shadow at | offset |
|---|---:|---|---|---:|
| `Thailand17` p32 | 32.00 pt | (138.16, 486.31) | (139.55, 484.92) | **1.39 pt** |
| `Aerospace_Journey` p16 | 33.99 pt | (105.93, 544.62) | (107.43, 543.12) | **1.50 pt** |
| `Aerospace_Journey` p5 | 38.01 pt | (76.88, 521.32) | (78.58, 519.62) | **1.70 pt** |

In every case ours has one record and it sits on the reference's *text*, not its shadow — on
p5 to 0.00 pt in both coordinates. The shadow is down and to the right on the page, drawn first.

`vcl/source/outdev/text.cxx:394-407` gives the rule:

```cpp
tools::Long nOff = 1 + ((mpFontInstance->mnLineHeight-24)/24);
if ( maFont.IsOutline() ) nOff++;
if ( (GetTextColor() == COL_BLACK) || (GetTextColor().GetLuminance() < 8) )
     SetTextColor( COL_LIGHTGRAY );
else SetTextColor( COL_BLACK );
rSalLayout.DrawBase() += basegfx::B2DPoint( nOff, nOff );
```

`nOff` is in device units and PDF export runs at **720 dpi**, so one unit is **0.1 pt**. With the
integer division that gives 14, 15 and 17 units for the three rows above — **1.4, 1.5, 1.7 pt,
reproducing all three measurements to the digit** (1.39 is 1.4 through the PDF text matrix).

*Measured*: the three offsets, the integer step, the 0.1 pt unit, and that the shadow is
**black** — both decks have coloured titles (red, blue) and both shadows are black, which is the
`else` branch. *Inferred*: which line-height metric feeds `mnLineHeight`. The three points bound
the ascent+descent factor to **[1.073, 1.122]** of the em, which contains both Liberation Sans's
hhea sum (1.1172) and its OS/2 typo sum (1.088); they do not separate them. The `COL_LIGHTGRAY`
branch for black text is untested by these two decks.

### Mechanism, in our tree

`PptCharacterStyle.ToEmphasis`
(`src/XRay.Office.Presentations/MsBinary/PptStyleSheet.cs:528-542`) reads `0x0001` bold, `0x0002`
italic, `0x0004` underline and `0x0100` strikethrough — and **not `0x0010`, shadow**, which the
same file names in its own header comment at line 11. `PptCharacterStyle.Stated` (line 553-567)
omits it from the mask for the same reason. There is nowhere for it to go: `RunEmphasis`
(`src/XRay.Office.Core/Extraction/Content.cs:177-199`) has no `Shadow` member, so
`PptTextBody.cs:337` cannot carry one to `SlideTextRun`, `SlideTextLayout.cs:634` cannot put one
in a `RunStyle`, and `SlideDrawing` never draws a second run. The bit is read from the file and
discarded four layers before the painter.

This is **not** the `SlideShadow` we already implement. That is a *shape* shadow, read on this
path at `PptSlideLayout.cs:980-1013` from the Escher `Shadowed`/`ShadowOffsetX/Y`/`ShadowColour`
properties, and it is a different feature with a different offset — the same file, 6 pt and
grey, is what casts `1-secretariat`'s picture silhouette in cluster 1.

### Reach and direction

Censused by parsing `StyleTextPropAtom` (`0x0FA1`) properly: paragraph half skipped, then
`TextCFRun` = `count(4)` + `TextCFException`, whose optional fields are sized off the `CFMasks`
bits, accepting only the parse that consumes the atom exactly. Shadow counts a run where mask
bit 4 **and** style bit 4 are both set.

**36 of the 51 `.ppt` decks**, 843 runs in total. The two decks the user named are the top two:

| runs with shadow | of total runs | deck |
|---:|---:|---|
| 135 | 983 | **Thailand17** |
| 98 | 1762 | *pres_ioc_phuket* |
| 90 | 302 | joint_user_outcomes_michael_fullerton |
| 77 | 993 | iep-amount-frequency-for-webinar |
| 60 | 356 | **Aerospace_Journey_of_Flight_Chapter** |
| 57 | 731 | ITE106-Chapter 4 |
| 40 | 1148 | 2015-Civil-Rights-Website-training |

That the user picked out two of the top five by an unaided visual read is worth recording. The
same persistence caveat applies as in cluster 1 — a run count is an upper bound on drawn runs,
and a master default inherited by an empty placeholder draws nothing.

The `.pptx` counterpart is separate and larger still: **30 of 112** decks put an
`a:effectLst/a:outerShdw` inside an `a:rPr` or `a:defRPr`, 1 543 of them. That is a different
model (offset, blur, colour, alpha) and was **not** measured here; it is named so nobody reads
the 36/51 as the whole reach.

**Direction: ink missing from ours, always.** A shadow only ever adds marks.

---

## Cluster 3 — missing underline

**Both reproduce. They do not share a cause** — the prediction that they would not is upheld,
though the reason given for `16 - UTM` was wrong.

### `Stakeholders-v08052017 - v5.pptx` — the underline comes from the hyperlink, not the run

Measured in the file: `slide11.xml` has one and `slide12.xml` three `a:r` whose `a:rPr` holds an
`a:hlinkClick` with an `r:id` and **no `u` attribute at all**. `slide13.xml` has two that state
both.

Measured in the PDFs:

| page | reference | ours |
|---|---|---|
| 11 | two `#0000FF` strokes at (456, 291)–(678) and (80, 276)–(281) | **nothing** |
| 12 | four `#0000FF` strokes | **nothing** |
| 13 | three `#0000FF` strokes | three fills, right place, **`#0070C0`** |

So where the file states `u="sng"` we draw the rule and get the *colour* wrong; where the file
states nothing we draw no rule at all. One cause explains both, and it is in the reference's
importer rather than in the file — `oox/source/drawingml/textrun.cxx:161-166`:

```cpp
if (!maTextCharacterProperties.maHyperlinkPropertyMap.hasProperty(PROP_CharColor))
    aTextCharacterProps.maFillProperties.maFillColor.setSchemeClr(XML_hlink);
aTextCharacterProps.maFillProperties.moFillType = XML_solidFill;
if ( !maTextCharacterProperties.moUnderline.has_value() )
    aTextCharacterProps.moUnderline = XML_sng;
```

**A hyperlink run gets `u="sng"` and the theme's `hlink` colour supplied by the importer, and the
colour is applied whether or not the run stated an underline.** `PptxTextBody.cs:628-654` reads
`a:rPr/@u` faithfully and correctly and never sees this, because it is not in the file. The
`a:hlinkClick` is read for the link but contributes no formatting.

The colour is measured, not assumed: `#0000FF` is what the reference draws on all three pages of
this deck and `#6B9F25` is what it draws on `16 - UTM` page 3 — each deck's own `a:hlink` scheme
colour.

### `16 - UTM - (NASA).pptx` — the *title* half is a line shape, not a text underline

The user's "Title missing underline" is on **page 9** (`|ink|` 1.71%, second worst). Under
"UTM Research Goals and Characteristics" the reference draws a thin crimson rule and ours draws
none that survives rasterisation. It is not `a:rPr/@u`. It is
`p:cxnSp` "Straight Connector 10" in `ppt/slideMasters/slideMaster1.xml`, so it is on every slide:

```xml
<a:xfrm><a:off x="237744" y="895604"/><a:ext cx="10515600" cy="1588"/></a:xfrm>
<a:prstGeom prst="line"/>
<a:solidFill><a:schemeClr val="accent1"/></a:solidFill>          <!-- accent1 = F07F09 -->
<a:ln w="38100" …><a:gradFill …>
    <a:gs pos="0"><a:srgbClr val="C60C30"/></a:gs>
    <a:gs pos="59000"><a:srgbClr val="C60C30"/></a:gs>
    <a:gs pos="100000"><a:prstClr val="white"/></a:gs>
</a:gradFill></a:ln>
```

The shape is **1 588 EMU tall — 0.125 pt** — and its visible mark is entirely its **3 pt**
(`w="38100"`) outline, whose colour is a *gradient* fill inside the `a:ln`.

Reference page 9: `stroke (18.71, 469.36)-(846.71, 469.50) #C60C30`.
Ours page 9: `fill (18.72, 469.36)-(846.72, 469.48) #F07F09` — the shape's 0.12 pt body in
`accent1`, and **no stroke at all**.

Mechanism, one function: `PptxSlideLayout.Pen`
(`src/XRay.Office.Presentations/Ooxml/PptxSlideLayout.cs:1678-1698`) opens with
`if (SolidFill(line, theme, placeholder: null) is not { } paint) return null;` — **an `a:ln` whose
paint is an `a:gradFill` yields no `Stroke`**, so the width, the cap, the dash and the colour are
all discarded together. What is left is a sliver 0.12 pt tall, which is under a fifth of a pixel
at 300 dpi and invisible at any resolution a reader uses. Both of the user's words are right:
the rule under the title is missing, and it is missing as a *line*.

That deck has the hyperlink defect as well — page 3's two URLs are underlined `#6B9F25` in the
reference and unmarked in ours — so `16 - UTM` carries **both** cluster-3 causes, and the one the
user reported is the connector.

### Reach and direction, cluster 3

| | decks | instances |
|---|---:|---:|
| `.pptx` with a hyperlink run stating no `u` | **41 of 112** | 297 runs |
| `.pptx` with an `a:ln` carrying only a `gradFill` | **8 of 112** | 17 lines |

The hyperlink cause is the wide one — a third of the `.pptx` half of the track, led by
`_1___Opatrny_Ales_United_Kingdom…` (33), `solog_orientation_august_2019` (29),
`County ACHS Presentaion Webinar` (25). The gradient-line cause is narrow by document count and
its per-deck reach is not: on `16 - UTM` the two lines live in the master and therefore appear on
**every one of its 30 pages**. Its eight decks include `8_P-Pavese_AIRBUS-ATB-journee-CRATB` and
`Demick_JetBlue`, both of which the Slides-Chart agent is holding.

**Direction: ink missing from ours** for both causes — an undrawn stroke and an undrawn rule.
The one exception is `Stakeholders` page 13, where the ink is present and the colour is wrong.

---

## Tests

**None were added, and no code was changed.** This round is diagnosis only, so the
reintroduction-verified / drift-guard split has nothing to report rather than something weak to
report. `verify-test.sh` was not run because there is no test to run it against.

That is a deliberate call rather than an overrun. Cluster 2 is the one I could have shipped —
its rule reproduces to the digit at three font sizes — but landing it means an additive member on
`RunEmphasis` in `XRay.Office.Core` and a flag threaded through `SlideTextRun`, `RunStyle`,
`PlacedGlyphRun` and `SlideDrawing`, and the offset has to be recomputed from a line height in
720-dpi device units whose *source metric* the three measurements above do **not** pin down
(they bound it to [1.073, 1.122] em, which contains both candidates). Shipping a device-unit
rounding rule on an unresolved metric is how a fixture passes and seven corpus documents fail.
The next round can settle it with one probe — a `.ppt` authored with the shadow bit at a size
where hhea and OS/2 straddle a 24-unit boundary — and then implement against a rule that is
measured end to end.

## What is measured and what is inferred

**Measured** (against 26.2.4.2, or read out of the files themselves):

- `1-secretariat`'s property 263 = `0x00FFFFFF` on `pib = 2`; that blip is a PNG with 0
  transparent pixels stored and 51 361 of 67 332 after LibreOffice's import; the reference PDF's
  two `/SMask`ed images against our one unmasked one.
- `pres_ioc_phuket` states no 263 anywhere; its image XObjects match the reference on all 26
  pages; the single difference is page 26's 851×46 gradient, extracted and read.
- The three shadow offsets (1.39 / 1.50 / 1.70 pt at 32.00 / 33.99 / 38.01 pt) and that both
  shadows are black and hard-edged; that we emit one text record where the reference emits two.
- Every underline figure in cluster 3: the strokes present and absent per page, `#0000FF`,
  `#6B9F25`, `#C60C30` against our `#0070C0` and `#F07F09`, and the connector's own XML.
- All census counts: 163 documents, 51 `.ppt` + 112 `.pptx`, zero read errors.

**Inferred** (read from the 27.2.0.0.alpha0+ tree, which is *not* the reference binary):

- The ±9 tolerance and its per-channel box shape. Binary alpha and bitmap-only are consistent
  with the measurement; the number 9 is not tested by it.
- `nOff = 1 + ((lineHeight − 24) / 24)` as the *formula* — the three offsets fit it exactly, but
  three points do not exclude every other integer rule, and they do not identify the line-height
  metric.
- The `COL_LIGHTGRAY` branch for black text: untested, no deck here has a black shadowed run.
- That `oox/source/drawingml/textrun.cxx:161-166` is the code 26.2.4.2 runs. The *behaviour* it
  describes is measured on nine hyperlinks across two decks.

## Files touched

None outside `dotnet/probes/slides-paint-01/`. `XRay.Office.Core`, `Containers`, `Text`, `Vector`,
`Rendering` and `Markup` are untouched, so the Slides-Text and Slides-Chart tracks are owed
nothing.
