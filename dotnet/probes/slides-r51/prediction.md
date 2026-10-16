# slides-r51 — prediction

Committed **before** anything was rendered post-change.

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
`fc-match Calibri` → `Carlito-Regular.ttf`, base `bd0f5ac1cf2`, `SOURCE_DATE_EPOCH=1700000000`,
`TZ=UTC`.

## Baseline, reproduced before anything was believed

Whole-track sweep, all 35 slides batches, reconciled document by document against `MANIFEST.tsv`
on a **case-folded** identity: **302 of 302 agree, 0 disagreements — 199 of 302 passing.** The
briefed figure.

The sweep's own `TOTAL` line says **311**, and that is the case-insensitive-mount trap, not a
corpus change: 9 documents were enumerated under two spellings each (`.pptx` and `.PPTX`, one
inode). They are listed in `results.md`. `311 − 9 = 302`.

## The change I intend to make

Route DrawingML's `a:clrChange` — PowerPoint's *Set Transparent Color*, a colour-key knockout —
into the `ColourKnockout` machinery that **already exists and already works**.

This is *a route, not a rule*, the **seventh** instance of that shape on this project:

| piece | state |
|---|---|
| `ColourKnockout` type, per-channel box match, binary alpha | exists, `Core/Graphics/GlyphRun.cs:248` |
| decoder applies it, before duotone and luminance | exists, `Rendering/Images/RasterImageDecoder.cs:185` |
| binary `.ppt` populates it from Escher property 263 | exists, `MsBinary/PptSlideLayout.cs:1061` |
| **OOXML populates it from `a:clrChange`** | **absent — `DrawingFill.ReadBlip` never reads the element** |

`grep -rn clrChange dotnet/src` returns **0 hits** across the whole tree.

## Documents I expect to change

Census method: every `a:clrChange` in the corpus resolved through its part's `.rels` to the
actual media file, the media decoded, and the `clrFrom` colour counted in its pixels. Declaring
the element is not the same as it changing anything.

**93 occurrences in 28 documents; 52 instances in 20 documents actually change pixels.**
The 41 inert ones are real: `DF1F06 → DF1F06` knockouts in layouts and masters whose image
contains no `DF1F06` pixel at all.

Every effective instance in the corpus has `clrFrom == clrTo` with `<a:alpha val="0"/>` — a pure
knockout, not a recolour. So `ColourKnockout` covers every corpus case exactly.

### slides — 11 documents (10 of them currently PASS)

| document | status | baseline abs_ink |
|---|---|---:|
| `16 - UTM - (NASA)` | open (`text`) | 20.25 |
| `171128IPAP` | pass | 13.87 |
| `County ACHS Presentaion Webinar 8-16-16 Peg` | pass | 11.80 |
| `Technical_Report_Elements[1]` | pass | 8.74 |
| `redac-nasops-201503-RIRP-portfolio-update` | pass | 7.71 |
| `social-media-app-bulletin-january` | pass | 6.21 |
| `FAAAIandtheArtandScienceofV&Vfinal` | pass | 4.14 |
| `bitesize-writing-a-report` | pass | 4.15 |
| `REDAC briefing March12-13-2014jemvbFINAL.ppt` | pass | 3.82 |
| `vv_summit_SAIC-PRESENTATION_FAA-V&V-Summit_508c` | pass | 0.71 |
| `vvsummit2022-SAIC-PRESENTATION` | pass | 0.71 |

Total baseline `abs_ink` over the eleven: **82.11**.

### words — 7 documents, sheets — 2 documents

`system_design__technical_architecture_template`, `090`, `091`, `095`, `096`, `098`, `100`
(Business_Case_Template family); `094` and `100` (Volunteer_Sign_Up_Sheet family).

**This is a shared-layer change.** The parse lands in **`XRay.Office.Ooxml`**, which reaches all
three tracks, and the wiring lands in the slides, words and sheets readers. The nine non-slides
documents above are the census's answer to what the other two tracks owe a measurement on.

## Verdict movement I expect: **ZERO**

Stated as a number, and zero is the honest answer here.

A colour knockout changes **pixels**. It cannot change the text layer, so `pdftotext`'s word
count cannot move. It cannot change page count. It cannot change font embedding. Those are the
gate's only three columns. Ten of the eleven slides documents already pass and have nothing to
win; the eleventh, `16 - UTM - (NASA)`, fails on `words` for reasons a picture's alpha cannot
reach.

I am predicting **199 → 199**.

If a verdict *does* move, the prediction is wrong and the reason must be found and written down,
not absorbed — r50 predicted 0, measured +1, and the useful part was the explanation.

## What I expect to move instead

`abs_ink` should **fall** on most of the eleven, largest on `social-media-app-bulletin-january`
(a 337.5 × 71.25 pt slab of pure black, 91.6% of the source image's pixels, currently painted
opaque and occluding the title) and on `Technical_Report_Elements[1]` (14 effective instances at
~49% of each image).

## What this census CANNOT see

Written down before the sweep, because an under-reaching census conceals itself — a low
prediction that comes true reads as well-calibrated.

1. **Layout/master instantiation.** I resolved which *part* each `clrChange` lives in, but not
   whether a slide actually instantiates that layout or master. The 41 "inert" instances are
   inert because the image lacks the colour, which is instantiation-independent — but a
   *layout* blip whose image *does* carry the colour would be counted by me and might still
   never be drawn, or vice versa.
2. **Tolerance — and this makes my count a LOWER bound.** I matched `clrFrom` at tolerance 0.
   LibreOffice's tolerance is **format-dependent** (`fillproperties.cxx`, tdf#149670): PNG and
   TIFF **1**, JPEG **15**, BMP **0**, otherwise **9**. A JPEG whose background is *near*-black
   rather than exactly black knocks out at 15 and contributes 0 to my census. So documents may
   move that I have not named, especially JPEG-bearing ones.
3. **Vector images.** The reference applies the knockout only to `GraphicType::Bitmap`, so a
   WMF/EMF carrying `clrChange` gets nothing. I have not counted them and do not intend to
   implement them.
4. **Occlusion.** A picture may be knocked out correctly and still be invisible because an
   opaque shape sits over it. That would show as a document I named which does not move.
5. **The other reader.** `.ppt`/`.doc`/`.xls` already have the Escher route and are untouched by
   this change; I have not re-verified them beyond the existing tests.
6. **`FAAAIandtheArtandScienceofV&Vfinal` is on the "we render better than the reference, do not
   work it" list** and my change touches it. I can see whether its ink moves; I cannot see from a
   census whether that movement is an improvement.
7. **Whether the reference actually applies the knockout on each of the 20.** I have read
   LibreOffice's import code and verified one document's markup and pixels. The other 19 are
   inferred from the same code path, not measured individually.

## Second target, not being implemented this round

`.ppt` autofit. Three blind readings, two documents, **opposite signs**, all naming autofit
unprompted:

- `2015-Civil-Rights-Website-training__ppt` p42 — ours ~9% **larger**, 14 body lines to 12, and
  ours **overruns the frame** while the reference fits.
- `ITE106-Chapter 4__ppt` p7 — ours ~10–20% **smaller**, 9 body lines to 10.

Both reviewers additionally, and independently, flagged **inter-paragraph spacing as a separate
effect from the font size** — the side with the smaller type has the larger gaps. I predict
nothing about it this round and am recording it so the next round starts from the readings
rather than re-deriving them.
