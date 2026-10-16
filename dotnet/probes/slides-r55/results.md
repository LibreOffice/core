# slides-r55 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
base `89798814dda`, branch `wt-slides-r55`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.
`prediction.md` beside this file was committed as `6c8559c6ad5`, before anything was built or
rendered post-change.

## Baseline, and what did not quite reproduce

| | briefed | measured |
|---|---|---|
| passing over `MANIFEST.tsv` | 199 of 302 | **199 of 302, 0 disagreements** |
| `abs_ink` | 1147.17 | **1147.14** |
| major pages | 403 | **402** |
| `tf-agreement` mean | 0.77053 | **0.77054** |
| exact `/Tf` pages | 1709 of 4515 | **1709 of 4515** |

The 0.03 and the one major page are **a single document** —
`035_Chemistry_Column_PowerPoint_Chart_45bf8a76.pptx`, 1.77 → 1.74 with 2 major pages → 1. Every
other row of `ink.tsv` is byte-identical to round 54's final sweep. It is a page sitting on the
major threshold, not a different tree.

## The whole round

| | base | final |
|---|---:|---:|
| passing | **199 of 302** | **199 of 302** |
| page counts changed | | **0 of 302** |
| `abs_ink` | 1147.14 | **1136.53** |
| signed ink | 831.99 | 831.39 |
| major pages | 402 | **395** |
| `tf-agreement` | 0.77054 | **0.77061** |
| exact `/Tf` pages | 1709 of 4515 | 1709 of 4515 |
| **differing-pixel % summed over 4530 pages** | 19823.86 | **19731.15 (−92.71)** |
| **turned** text blocks (reference 1318) | 1097 | **1385** |
| pages the reference turns and we do not | 43 | **7** |
| **sheared** glyphs (reference 16008) | **0** | **16740** |
| pages the reference shears and we draw none | 157 | **3** |

32 documents moved on ink: **24 better, 8 worse.** The eight, named rather than netted:

| Δ ink | document | before → after |
|---:|---|---|
| **+0.27** | `Sylva introduction session.pptx` | 2.99 → 3.26 |
| **+0.25** | `ws_prod…M.017-(French)-France.ppt` | 18.80 → 19.05 |
| +0.07 | `joint_user_outcomes…29.06.12.ppt` | 4.47 → 4.54 |
| +0.07 | `HENTZEN_COMPOSITE_MATERIALS…pptx` | 5.19 → 5.26 |
| +0.05 | `Sean Monogue.pptx` | 2.98 → 3.03 |
| +0.05 | `Employment-Based_I-485.ppt` | 6.21 → 6.26 |
| +0.01 | `nabiyeleul_1_aviation_service…pptx` | 3.43 → 3.44 |
| +0.01 | `16 - UTM - (NASA).pptx` | 15.10 → 15.11 |

**Two of the eight are the round-53 shape rather than real, and one of them is the largest.**
`Sylva introduction session.pptx` is the clearest case this project has produced: its unsigned ink
rose 2.99 → 3.26 while its **differing pixels fell 50.97 → 48.81** *and* its turn count went from
**34 blocks the reference does not turn to 0, which is the reference's own figure exactly*. It has
a body stating `@rot` and `@vert` that cancel; before this round we turned that text and the
reference did not. Three instruments, two of which are not the ink column, say the page improved.
`Employment-Based_I-485` is the same shape at a smaller scale (215.52 → 212.77 differing pixels
against +0.05 ink).

**By differing pixels — the instrument that can see this change — 8 documents worsen and only one
by more than a quarter of a point**: `ws_prod…M.017-(French)-France.ppt` **+1.04** over 55 pages,
on a document already at 738.13 (13.4% of every page). Its shear placement is right to within a
few glyphs a page (3778 to the reference's 3830, no page off by more than 12), so the residue is
ink moving inside a page that is wrong for other reasons. The rest: `2014BSA_Sunday_Killion` +0.21
and `16 - UTM - (NASA)` +0.11 (both the font-resolution case in §1), `NAS-…-Weather` +0.07,
`ws_prod…Part-M` +0.06, `joint_user_outcomes` +0.02, `nabiyeleul` +0.01, `_1___Opatrny_Ales` +0.01.

Largest improvements: `NAS-Infrastructure-Roadmaps-v16.0.pptx` **159.88 → 151.76** with major
pages **55 → 49**, `2015-Civil-Rights-Website-training.ppt` 30.32 → 29.64,
`NAS-…-Weather` 11.93 → 11.42, `attendance-updates-for-governors` 3.80 → 3.29, `undp` 20.57 →
20.24, `NAS-…-HSI` 6.64 → 6.32, `section_1_our_rights_presentation` 3.36 → 3.05.

**The ink column is the wrong instrument for most of this round and the round says so with a
number.** The synthetic oblique on its own is worth **−1.92 `abs_ink`** and **−76.89 differing
pixels** over the same pages: a leaning glyph and an upright one cover almost the same area, so
an ink metric barely sees a change that moves 16740 glyphs onto the reference's. This is the same
shape as round 54's bullet fix (−0.41 ink, +144 exact-`/Tf` pages), arriving in a third column.

## 1. The brief's item 1 is an artefact of its own instrument, and the larger half of it is a different defect

The brief's headline — *197 pages where the reference rotates text and we rotate none* — is
**wrong, and it is the second artefact in the same instrument.** Round 54 fixed a `Tm`-only count
that could not see our own `cm` route and left in place the test that calls a matrix rotated
whenever `b` **or** `c` is non-zero. A synthetic-oblique text matrix is `[1 0 tan(θ) 1]` — `b`
zero, `c` not — so **every fake-italic run counted as rotated**.

The document that gives it away is the census's own **number two**:
`section_1_our_rights_presentation.pptx`, 81 "rotated" blocks over 11 pages, of which the reference
turns **zero**. All 81 read `c = 0.3462535606`, and nothing on that deck is turned at all.

`turn-census.py` separates the two, over the same 302 documents and the same PDFs:

| | ours | reference | pages ref does and we do not |
|---|---:|---:|---:|
| **turned** blocks | 1097 | 1318 | **43** |
| **sheared** blocks | **0** | **587** | **157** |

So 197 = 43 + 157 less the overlap, and the larger part is: **we never synthesise an oblique.
Anywhere. On any track.**

| track | reference sheared **glyphs** | ours | pages | documents |
|---|---:|---:|---:|---:|
| slides | 16 008 | **0** | 157 | 44 |
| **words** | **154 501** | **0** | **759** | 55 |
| sheets | 15 497 | **0** | 106 | 10 |

**186 006 glyphs over 1022 pages in 109 documents**, drawn upright where the reference leans them.

`shear-chars.py` exists because `turn-census.py`'s successor makes the *same class* of mistake one
level down: it counts text **blocks**, and blocks are not comparable between the two stacks —
LibreOffice writes one `BT … ET` per text object with a `Tm` per run inside it, we write one per
glyph run. A paragraph of three italic runs is one sheared block on their side and three on ours,
which reads as a 20% over-shear that is **entirely granularity**. Counting glyphs removes it. The
block-level reading after the fix says ours 705 to their 587; the glyph-level reading says 16740 to
16008, and 1057 of that surplus is three named documents (below).

### The mechanism, and the constant

`LogicalFontInstance::NeedsArtificialItalic()` is `m_aFontSelData.GetItalic() != ITALIC_NONE &&
m_pFontFace->GetItalic() == ITALIC_NONE` — *the request is italic and the face is not* — and
`pdfwriter_impl.cxx:5767` then does `aMat.skew(0.0, ARTIFICIAL_ITALIC_SKEW)`.

**The constant is not a third and the digits are the proof.** `ARTIFICIAL_ITALIC_SKEW` is
`float((1<<16)/3) / (1<<16)` = `0.3333333432674408`
(`vcl/inc/font/LogicalFontInstance.hxx:52-53`), and `Matrix3::skew` takes its arguments as
**angles** and writes their tangent. So the page gets `tan(0.3333333432674408)` =
**0.3462535606** — and a scan of every text matrix in all 302 reference renderings finds
**exactly one** shear value, 0.346254, 587 times.

### The known-answer deck, and it was built to discriminate

Round 54's rule — *a fixture round-tripped through the reference inherits the reference's defaults
and can make two rival rules indistinguishable* — was taken seriously. `make-oblique-probe.py`
writes the `.pptx` **by hand**, and its columns are chosen so that three candidate rules give
three different answers:

| slide | H1 *face has no italic* | H2 *by stated family* | H3 *real italics lean too* |
|---|---|---|---|
| Liberation Sans | no shear | — | shear |
| DejaVu Sans | shear | — | shear |
| **Verdana** (not installed → DejaVu Sans) | shear | *its own answer* | shear |
| Liberation Serif | no shear | — | shear |
| DejaVu Serif | shear | — | shear |

Measured on 26.2.4.2, three sizes each: slides 2, 3 and 5 shear and 1 and 4 do not — **15 of 15
for H1**, H2 and H3 refuted. `Verdana` maps to the same `/F3` resource as `DejaVu Sans` and leans
with it, so the answer follows the **resolved** face and not the stated name.

**And the fact the whole risk assessment rests on**: on the shearing slides the roman and the
italic halves carry the **same `TJ` array and the same pen origin** at 12, 24 and 40 pt. The
reference passes the slant to HarfBuzz as `hb_font_set_synthetic_slant`, which moves outlines and
mark attachments and leaves advances alone. **No advance and no line break moves.** Our own
rendering of the same deck: 15 of 15, shears in the same three places and nowhere else, origins on
the reference's to 0.03 pt.

### What the fix exposed that was already wrong

Three documents now shear where the reference shears **nothing**, and the shear is not the defect:

| document | our sheared glyphs | reference |
|---|---:|---:|
| `2014BSA_Sunday_Killion.pptx` | 948 | **0** |
| `16 - UTM - (NASA).pptx` | 77 | **0** |
| `Sean Monogue.pptx` | 32 | **0** |

On `2014BSA_Sunday_Killion` the reference embeds no `DejaVuSans` roman at all and we do: we send
an italic request to a family with no italic where the reference sends it to one that has one.
That is a **font-resolution divergence that existed before this round and was invisible**, because
a wrong face and a right face both drew upright. It is now visible, and it costs +0.21 and +0.11
differing pixels on two of the three. Left open deliberately: gating the shear to hide it would be
fitting to the corpus.

## 2. `a:bodyPr/@vert`, and the insets that travel with it

`vert` and `eaVert` add a quarter turn clockwise, `vert270` a quarter the other way, folded into
`SlideTextBody.Rotation` so that the existing transposed-frame path (`PptxSlideLayout.Turned`)
carries them. Worth **−8.69 `abs_ink`** and **major pages 402 → 395** on six documents.

**`vert` and `eaVert` are the same rendering and that is measured, not assumed.** The importer
sends them down different paths — `WritingMode2::TB_RL90` against `TB_RL`, with a swapped pair of
adjusts (`textbodypropertiescontext.cxx:126-200`) — and an authored deck holding everything else
fixed over three anchors draws **165 identical glyph matrices at identical positions** for the
two. This matters for reach: 118 of `NAS-Infrastructure-Roadmaps-v16.0`'s 173 vertical shapes are
`eaVert`.

**The insets rotate with the turn, and a symmetric fixture cannot see it.**
`TextBodyProperties::pushTextDistances` walks the four insets into slots starting at 3 for
vert/eaVert and at 1 for vert270. A second authored box with **10 / 20 / 30 / 40 pt** insets
settles the direction:

```
vert, eaVert:  transposed (left, top, right, bottom) <- (tIns, rIns, bIns, lIns)
vert270:       transposed (left, top, right, bottom) <- (bIns, lIns, tIns, rIns)
```

This is **not neutral on a body that states nothing**: DrawingML's defaults are 0.1 inch across
and 0.05 inch down, so a turned body gets 0.05 across and 0.1 down — 3.6 pt on every edge of every
vertical shape. The first derivation of the mapping written from the C++ was **wrong** (it had
the transposed-left taking `rIns`); the probe corrected it.

Agreement on the authored decks: **9 of 9 line origins on `vert` and 9 of 9 on `vert270`, over all
three anchors, to 0.05 pt**, and the inset deck to 0.045 pt with the same three-line break.

Two values of `ST_TextVerticalType` are deliberately **not** turns, and that was measured on the
same deck rather than assumed: `mongolianVert` draws **horizontally** (the importer's own comment
says the rendering is not implemented for shape text) and `wordArtVert` stacks one upright glyph
per line (`WritingMode2::STACKED`). Neither appears on any slide part of the corpus. We draw both
horizontally, which is right for the first and **wrong for the second** — recorded, not fixed.

`NAS-Infrastructure-Roadmaps-v16.0.pptx`: turned blocks **111 → 393** against the reference's 403,
pages where the reference turns and we do not **32 → 0**, `abs_ink` 159.88 → 151.76, major 55 → 49.

## Measured against the prediction

| # | predicted | measured |
|---|---|---|
| 1 | verdict movement **0**, band −1…+1 | **0** — 199 → 199, `MANIFEST` agrees on all 302 |
| 2 | page counts 0 of 302 | **0 of 302** |
| 3 | `abs_ink` **down**, no range; a named rank | **down, −10.61.** The rank was **wrong in its top half**: `section_1_our_rights` (81 blocks) is worth −0.31 and `M.017-France` (73) went **up** 0.25, while `NAS` — ranked nowhere near the top for shears — carries −8.12 of the total through the *other* fix |
| 4 | 40–52 documents move | **32**, below the band. See below: the band was wrong for a reason the census could have stated |
| 5 | `tf-agreement` ±0.0005 and exact pages ±5, **as a control** | **held exactly for the oblique**: 0.77054 → 0.77054 and 1709 → 1709, unmoved to the digit. The `@vert` fix then moved it to 0.77061 with 1709 unchanged |
| 6 | both other tracks improve in sign; words verdict −2…+2, sheets 0 | **words 0**, 319 of 337 with 0 manifest disagreements; sheets pending at the time of writing |
| 7 | `@vert` reach: 14 slide-part documents + 4 `.ppt` | **6 of the 14 moved.** The other eight state `@vert` on a shape that draws no text or whose text is empty. The `.ppt` half was not implemented |

**Prediction 4 is the round's bad call and it is the *third* consecutive miss on "how many
documents move".** Round 53 over-shot by extrapolating candidates, round 54 under-shot by
censusing visible symptoms, and this round over-shot by **censusing the reference's own output**,
which is the most direct census available and still wrong — because a document with one or two
sheared runs on a thirty-page deck moves its ink by less than the 0.005 the column prints. 46 of
the 302 carry a reference shear; 27 moved on ink and 43 moved on differing pixels. **The census
was right about the documents and wrong about the instrument**: it predicted movement in a column
that cannot resolve it. Stating a *column* alongside a count is the missing discipline, not a
better census.

## Refutations

1. **The brief's item 1**, quantitatively. `rotated-text-census.py` conflates a shear with a
   rotation; 157 of the 197 pages are synthesised italic, not rotation. Two instruments say so
   (a matrix classification, and the fact that the corpus's whole reference-side "rotation"
   population contains exactly one non-zero `c` value repeated 587 times).
2. **`turn-census.py`'s own block counting**, refuted by `shear-chars.py` in the same round: our
   705 sheared blocks against the reference's 587 is granularity, not over-shearing. This is
   recorded because the round nearly shipped "we over-shear by 20%" as a finding.
3. **My own first derivation of the inset rotation**, read from `pushTextDistances` and wrong by
   two slots. Caught by an asymmetric fixture.
4. **The audit site `OdpSlideLayout.cs:302`** — see below.
5. **`24.2.7.2`'s own trap, still live in the file that describes it**: round 54's marker prose
   named the superseded version and kept a cleared site in the open count.

## The 24.2.7.2 audit

`OpenDocument/OdpSlideLayout.cs:302` — the ODF half of the claim round 54 settled for OOXML, which
round 54 explicitly recorded as not covered.

**WRONG on 26.2.4.2, reported and not fixed.** The site says a drawing cell's first baseline sits
at the face's own ascent "whatever `tablecellcontext.cxx:61` sets". The rule is no longer fixed at
all: the reference obeys `style:font-independent-line-spacing` **as stated**.

**The method is the finding.** `soffice --convert-to odp` writes
`style:font-independent-line-spacing="true"` onto every drawing cell it emits — so a single
rendering of a round-tripped fixture measures the exporter's habit, not the rule. That is round
54's lesson arriving in a second form, and it is why this needed a **discriminating pair**: the
exported `.odp`, and a byte-identical copy with that one attribute deleted.

| stated size | attribute present | absent |
|---:|---:|---:|
| 10 pt | 10.013 (1.0013 em) | 9.020 (0.9020 em) |
| 12 pt | 11.997 (0.9998 em) | 10.920 (0.9100 em) |
| 18 pt | 18.006 (1.0003 em) | 16.334 (0.9074 em) |
| 40 pt | 40.003 (1.0001 em) | 36.120 (0.9030 em) |

We draw the second in both cases. **Not fixed, and the reason is the corpus**: the slides track is
251 `.pptx` and 51 `.ppt` and holds **no ODF presentation at all**, so the change would ship
against the one instrument that has caught every previous mistake in this area and get nothing
back. The exact fix is named at the site.

Open sites **42 → 40**, marked **15** (13 verified, 2 wrong). One of the two removed was a
re-check; the other was **round 54's own marker prose** naming `24.2.7.2` and so keeping a cleared
site open — the file's trap, sprung inside the file that documents it. A rule written down is not
a rule applied.

## Tests

Three new files, **14 new tests**.

| test | mutation | outcome |
|---|---|---|
| `PdfSyntheticObliqueTests` (3) | `bool oblique = run.Font.SyntheticOblique` → `false` | **DETECTED**, 2 of 3 |
| `SyntheticObliqueResolutionTests` (6) | `SyntheticOblique = request.IsItalic && !face.IsItalic` → `false` | **DETECTED**, 3 of 6 |
| `SlideVerticalTextTests` (5) | `VerticalText.Clockwise => 90 * units` → `0` | **DETECTED**, 2 of 5 |
| `SlideVerticalTextTests` (5) | the vert inset rotation → the identity quadruple | **DETECTED**, 1 of 5 |

The two inert cases in each set are the controls by design — the upright arm of each pair, which is
what says the behaviour comes from the flag rather than from anything else the code does.

`SlideVerticalTextTests` is stated as **relations between the three origins**, not as three
absolute points, so nothing in it depends on the face's ascent — the one quantity in the arithmetic
that belongs to the machine rather than to the file. The reference's own numbers are in the
remarks for the record.

**Untested: the Skia sink's `SkewX`.** No corpus metric exercises it — `pdf-image-diff` rasterises
our *PDF* — and it is there so the two backends draw the same page. Said plainly rather than
covered by a test that would only assert the line was written.

Ten non-Fidelity projects, one at a time: Core 337, Containers 109, Text 617, Vector 295,
Rendering 153 (+1 skipped, the same `PdfFontTests` case as at baseline), Markup 259,
OpenDocument 125, WordProcessing 1155, Spreadsheets 925, **Presentations 819** — **4794 passed,
0 failed, 1 skipped**.

**The briefed base of 4790 does not reconcile and should be re-derived.** 4794 less this round's
14 new tests is **4780**, not 4790, and `git diff 89798814dda..HEAD -- dotnet/tests` shows only
this round's three files. Either the briefed figure is stale or ten tests were counted
differently. Flagged rather than assumed away: *a figure quoted rather than re-derived decays* is
this project's own rule.

`cd dotnet && dotnet build -v q -nologo` → **0 warnings, 0 errors**.

## Shared layers — measured, not argued

The diff touches `XRay.Office.Core` (`FontReference`), `XRay.Office.Text` (`SystemFontResolver`) and
`XRay.Office.Rendering` (both sinks). All three tracks were swept.

| track | passing | sheared glyphs before → after (reference) | pages we drew none, before → after |
|---|---|---|---|
| slides | **199 of 302**, 0 manifest disagreements | 0 → **16 740** (16 008) | 157 → **3** |
| **words** | **319 of 337**, 0 manifest disagreements | 0 → **158 673** (154 501) | 759 → **162** |
| sheets | 272 `match` of 307 | 0 → **15 509** (15 497) | 106 → **11** |

**Sheets is the cleanest of the three** — 15 509 sheared glyphs against 15 497, only 11 pages of
106 still disagreeing, and **all ten affected documents verdict `match`**. Its sweep is reported
with two caveats that are the harness and not the change: two `.xlsm` were dropped under load
(both render fine on their own, checked) and `STC_WebList.xlsx` — 4372 pages — timed out in the
image diff. Sheets has no baseline at *this* commit to difference against, so the passing figure
is offered as a level, not a delta; the shear agreement and the ten verdicts are the evidence.

**Words moves no verdict.** Of the 845 words pages with a shear on either side, we drew none on
759 before and on **162** after; 323 are now within 2% of the reference's glyph count and 360 are
further off. That residue is the words track's own font-resolution divergence — the same class as
the three slides documents above — and it is now visible for the first time. It is the words
track's to work, and this is the census that names it.

## Left open, in the order the next round should take them

1. **`.ppt` vertical text.** The Escher `txflTextFlow` property (0x88) is unread: 31 non-zero
   values in 4 documents — `ws_prod…Approval-of-Flight-Conditions` 22, `introduction_to_bea_tuxedo`
   4, `Thailand17` 4, `hofman` 1 — and the turn census still shows those documents short.
   `PptSlideLayout` has no `Turned` equivalent; giving `SlideTextBody.Rotation` the same quarter
   turn from Escher would reuse everything this round built.
2. **Over-rotation, which is now the whole of the turn gap — and it is a chart defect, diagnosed.**
   `Demick_JetBlue.pptx` is the track's **third-largest document at 26.10 `abs_ink` with 6 of its
   10 pages major**, and its 76 turned blocks are **68 at 45° plus 8 at 90°**. The eight are the
   reference's eight, matched exactly. The **sixty-eight are a chart's category axis**: on page 4
   we write 52 `BT` to the reference's 31 and `pdftotext` reads **163 words to its 79** —
   the reference **drops those labels** and we draw them, diagonally, one fragment per block
   (`8 / 20 / 12 / - / 7 / 20 …`). So this is `SlideChart`'s axis-label placement and not a text
   body's rotation at all, and it is worth a whole document's ink. `8_P-Pavese_AIRBUS` (21 to 1)
   and `171128IPAP` (45 to 70, the mirror) are the next two to classify the same way.
   `Sylva`'s 34-to-0 went to **0 to 0** this round, as a side effect of `@vert` and `@rot`
   cancelling on the same body.
3. **The font-resolution divergence the shear exposed.** `2014BSA_Sunday_Killion.pptx` (948
   sheared glyphs to the reference's 0), `16 - UTM - (NASA)`, `Sean Monogue` on slides, and 360
   words pages. Both sides embed the same face *lists*; the divergence is per-run. It has been
   invisible until now because a wrong face and a right face both drew upright.
4. **The fitted bullet's vertical placement** — 1.9 pt too high, `ALIGN_BOTTOM` /
   `aBulletArea.Bottom()`, `outliner.cxx:909-919`. Untouched this round.
5. **`2015-Civil-Rights-Website-training.ppt`**, 30.32 → 29.64 and still the track's second
   largest; `baseline-agreement` 1.4915 over 1228 pairs is unmoved by a shear.
6. **The audit**: `PptxSlideLayout.cs` 2 left, `SlideDrawing.cs` 2, `PptxTextStyles.cs` 1,
   `SlideAutofit.cs` 4 (all four are narrative references to the superseded search, not claims).
7. `wordArtVert` — we draw it horizontally and the reference stacks it. Zero corpus reach.
8. The `pitchFamily` family nibble — still a decision for the user. Unchanged since r50.
