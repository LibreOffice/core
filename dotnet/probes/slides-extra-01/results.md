# slides/extra-001 — WordArt is a picture of words, and we were reading it as words

One document, `FAAAIandtheArtandScienceofV&Vfinal.pptx`, page-exact at 30/30 and failing the
word band at **1189 against 1133**. It now **matches** at 1135/1133.

Prediction, written and committed before any of this was measured:
`dotnet/probes/slides-extra-01/prediction.md` (`571f833a38e`).

## The seat

`a:bodyPr/a:prstTxWarp` with `@prst` anything other than `textNoShape`.

* `oox/source/drawingml/textbodypropertiescontext.cxx:215-226` opens a `PresetTextShapeContext`
  for exactly that condition.
* `oox/source/drawingml/shape.cxx:2202-2211` then calls
  `FontworkHelpers::putCustomShapeIntoTextPathMode`.
* `svx/source/customshapes/EnhancedCustomShapeFontWork.cxx` converts the characters to
  `tools::PolyPolygon` outlines. **What reaches the PDF is filled paths with no glyph and no
  `ToUnicode`.**

X-Ray.Office read `a:prstTxWarp` nowhere at all — `grep -rn warp dotnet/src` was three unrelated
hits — so a warped body went through the ordinary text path and its glyphs landed in the text
layer.

The fix is three edits and one gate:

| file | change |
|---|---|
| `src/XRay.Office.Presentations/Ooxml/PptxTextBody.cs` | read `a:prstTxWarp/@prst` off the body chain, normalising `textNoShape` to null |
| `src/XRay.Office.Presentations/Layout/SlideText.cs` | `SlideTextBody.WarpPreset` / `IsTextPath`, and the reasoning |
| `src/XRay.Office.Presentations/Ooxml/PptxSlideLayout.cs` | `if (body.IsTextPath) return null;` in `Text(...)`, the single place a `p:sp`'s text is laid out |

## The brief was wrong about the mechanism, and the correction matters

The brief said we draw the five gauge labels **twice** — "once arc-warped at the reference's
exact bounding boxes, and again unwarped" — and that the warped copy was ours and correct.

Measured, there is no duplicated emission. Slides 13 and 14 hold **two different sets of
shapes**, both legitimately in the file:

* **Set A** — three plain rotated text boxes (ids 2 `Analysis`, 8 `Assistance`,
  9 `Augmentation`), `a:xfrm/@rot` only, no warp. `pdftotext -bbox` puts our fragments within
  **0.3 pt** of the reference's, fragment for fragment. This is what the brief called the
  arc-warped copy; it is rotated, not warped, and it was already right. The per-glyph
  fragmentation (`A na ly si s`) is what rotation does to `pdftotext`, not what an arc does.
* **Set B** — four `a:prstTxWarp` boxes (ids 42–45) holding
  `Assistance`, `Analysis`, `Automation`, `Autonomy`, `Augmentation`, 48 characters. Ours drew
  them as text; the reference draws them as outlines.

`Automation` and `Autonomy` exist **only** in set B, and neither word appears anywhere in the
reference's text layer — which is what settles it. The reference's page-13 content stream holds
**597 `c` curve operators** where ours held 4.

**Believing the brief would have produced the wrong fix.** "Drop the duplicate copy" implies a
second emission to suppress; there is none, and a reader looking for one finds the code has
exactly one `p:sp` → glyph-run path and concludes there is no defect.

## The reference, measured on the installed 26.2.4.2 rather than read from the tree

An authored probe rather than an inference. `make-fixture.py` in this directory builds
`text-warp-deck.pptx`: one slide, three identical text boxes at round positions in Liberation
Sans 18 pt, differing only in their `prstTxWarp` — absent, `textNoShape`, `textArchUp`.

| | reference 26.2.4.2 | ours before | ours after |
|---|---|---|---|
| `Fontwork keeps three` in the text layer | **2** | 3 | **2** |
| answer to the third box | 3 fills over 187 curves | 1 more text block | nothing |

`textNoShape` is drawn as ordinary text and the arch is not. The same holds for `textPlain`,
which curves nothing: `redac-sas-201403-ppt-portfolio-rev-sim.pptx`'s WordArt
`Fractographic Examinations` occurs **0** times in the reference's PDF. So the test really is
`!= textNoShape`, matching the C++ exactly.

## What was fixed, and what was deliberately not

**Not fixed: the arch geometry.** The reference does not merely draw those labels, it *warps*
them, and we now draw nothing where it draws outlines. That is a deliberate partial, and the
choice was measured rather than assumed.

The reference's four Fontwork outlines on page 13, taken from its own content stream, against
where our unwarped runs landed:

| label | reference outline bbox (pt) | our unwarped text bbox (pt) | offset |
|---|---|---|---|
| `Assistance` | 507.9–578.6 × 325.0–391.4 | 482.3–552.6 × 288.9–366.2 | (+25.8, +30.6) |
| `Analysis` | 514.7–569.4 × 217.0–275.2 | 502.0–565.9 × 230.5–284.7 | (+8.1, −11.5) |
| `Automation`/`Autonomy` | 393.4–474.2 × 209.4–290.0 | 416.4–514.3 × 226.7–303.6 | (−31.6, −15.5) |
| `Augmentation` | 390.6–478.0 × 314.1–395.2 | 420.4–496.6 × 281.1–364.5 | (−24.2, −31.9) |

Every offset points outward along the box's own local **up** for `textArchUp` and **down** for
`textArchDown` — the arch's radial displacement, 14 to 40 pt. Reproducing it needs per-glyph
outline warping and a per-run transform the drawing IR does not carry; `IDrawingSink` has
`FillPath` but nothing converts a glyph to a path, and the layout layer cannot reach SkiaSharp.

So the choice was between leaving unwarped ink 14–40 pt from where it belongs and drawing
nothing. **P5 predicted that drawing nothing is nearer, because misplaced ink counts twice in a
comparison and absent ink counts once, and said the fix was wrong if the measurement disagreed.
It agreed**, at 300 dpi against the banked reference:

| page | ref vs ours BEFORE | ref vs ours AFTER | change |
|---|---:|---:|---:|
| 13 | 209 480 px | 178 971 px | **−30 509 (−14.6%)** |
| 14 | 169 457 px | 138 948 px | **−30 509 (−18.0%)** |

Mean absolute difference fell 3.5098 → 3.0061 and 2.8367 → 2.3329. Identical absolute change on
both pages, which is what a single repeated cause looks like.

The residual on page 13 is where it should be: the wheel's ring and tick band differs by
**0.57%** of its pixels — antialiasing — while the four quadrants differ by **6.23%**. That is
the missing Fontwork and nothing else.

## The remaining +2 is `pdftotext`, not ink

1135 against 1133, not the 1133 P1 predicted. The residual is +1 on each of the two pages, and
`pdftotext -raw` output for both pages is now **byte-identical to the reference's**. The
difference is only in default mode, and only in where the tokeniser splits set A's rotated
`Assistance`: the reference reads `As si s ta n c e`, we read `A s si s ta n c e`. Same glyphs,
same positions to 0.3 pt, one more token. It was there before the fix too — set B contributed
exactly 27 of the 28 per page, and this boundary artefact the other 1.

## Verification

**Operators, not a downscaled raster.** Our page-13 text blocks went **49 → 42**; the reference
has **42**. Seven blocks removed, and the count now agrees exactly.

**Two blind reviewers**, fresh subagents given one composed image each and no numbers,
forbidden to read the repo. The compositor's own warning was honoured: the first pair reported
"shown at 86% of composed", so it was re-rendered at 129 dpi until it read 100%.

Both, independently, reported **exactly one difference**: the reference draws four curved
labels inside the wheel quadrants which we do not draw at all. Both listed everything else —
ring, ticks, hub, arrow, the four corner text blocks, the three thumbnails, colours, wrap
points — as identical. Both named "a preset text warp / text-on-a-path (WordArt/Fontwork-style)
that the renderer drops rather than laying out straight" among the candidate causes, from the
image alone.

The load-bearing part is what they did **not** see: *"there is no faint, clipped, mis-coloured
or mis-placed remnant of that text anywhere in the top half — not inside the dial, not spilling
outside it, not stacked at the centre."* Before the fix that remnant was the defect.

One reviewer flagged, at low confidence, a possible small vertical offset of the dial. It is not
real: the ring-and-tick band differs by 0.57%, and the wheel picture's own coordinates are
identical. It is the two composed halves differing by 10 px in height.

## Reach — measured from what resolves

**A grep would have said 67 of 163.** Sixty-seven slides documents carry a `prstTxWarp`
element. **Sixty-five carry only `textNoShape`**, the value that means no warp.

Rendered all 163 slides documents twice — once with the fix, once with the gate disabled and
rebuilt, `SOURCE_DATE_EPOCH=1700000000` both times — and compared by SHA-256:

```
163 documents; byte-identical 161; changed 2
  CHANGED done-009_pptx_redac-sas-201403-ppt-portfolio-rev-sim.pptx.pdf
  CHANGED extra-001_pptx_FAAAIandtheArtandScienceofV&Vfinal.pptx.pdf
```

**2 of 163**, exactly as P3 predicted. The other 161 are byte-identical.

Outside slides: the words track has 2 documents with a real warp
(`extra-001/…/ABCD-SDE-23-00…docx`, `done-014/…/exhibit-06---technical-architecture-template.docx`,
both `textPlain`) and sheets has none. Those go through the word-processing shape reader, which
this change does not touch — see *Follow-ups*.

## Regression

`slides/done-*`, all 15 batches, before and after:

```
before  TOTAL 144  MATCH 144  MISMATCH 0  REF-CANNOT-RENDER 0
after   TOTAL 144  MATCH 144  MISMATCH 0  REF-CANNOT-RENDER 0
```

Exactly one row moved, and it is the predicted one:

```
- redac-sas-201403-ppt-portfolio-rev-sim.pptx  14/14  2017/2014  match
+ redac-sas-201403-ppt-portfolio-rev-sim.pptx  14/14  2006/2014  match
```

−11 words, which is the 11 words in that deck's five WordArt shapes counted from its markup,
and none of the 11 is in the reference either.

**Worth recording rather than glossing: the fix removed a compensating error.** That document's
delta went from **+3 to −8**. It was passing partly because 11 words we should not have drawn
were covering an 8-word shortfall somewhere else on the same deck. The band is ±40.3 so it
still passes comfortably, but there is now a real −8 there that was previously invisible.

## Tests

Every project run individually and totalled by hand, because
`dotnet test XRay.Office.slnx` is the run most likely to truncate and least likely to say so.

| project | passed | failed | skipped |
|---|---:|---:|---:|
| XRay.Office.Core.Tests | 337 | 0 | 0 |
| XRay.Office.Containers.Tests | 109 | 0 | 0 |
| XRay.Office.Text.Tests | 349 | 0 | 0 |
| XRay.Office.Vector.Tests | 295 | 0 | 0 |
| XRay.Office.Rendering.Tests | 150 | 0 | 1 |
| XRay.Office.Markup.Tests | 259 | 0 | 0 |
| XRay.Office.OpenDocument.Tests | 125 | 0 | 0 |
| XRay.Office.Presentations.Tests | **704** | 0 | 0 |
| XRay.Office.Spreadsheets.Tests | 832 | 0 | 0 |
| XRay.Office.WordProcessing.Tests | 850 | 0 | 0 |
| XRay.Office.Fidelity.Tests | 520 | **30** | 0 |
| **total** | **4530** | **30** | **1** |

`XRay.Office.Fidelity.Tests` reports **30 failed of 550** against **550 discovered**
(`--list-tests`) and **0 skipped** — the briefed baseline exactly, and a complete run rather
than a truncated one. All 30 are pre-existing word-processing, spreadsheet, table and tab-stop
failures; none is a slide-text failure. The single Rendering skip is the pre-existing
`ACffFlavouredFaceIsNotClaimedToBeTrueType`. Presentations was 694 before; the 10 new ones are
`SlideTextWarpTests`.

**The new tests fail against the unfixed tree**, verified by copying the two source files aside
(`cp`, never `git stash` — the stash stack is repository-global and this clone has many
worktrees), reverting the behaviour while keeping the model so the test project still compiles,
and rebuilding:

```
Failed!  - Failed: 6, Passed: 4, Total: 10
  AStatedWarpMakesTheBodyATextPath(preset: "textArchUp" | "textArchDown" | "textPlain" | "textCirclePour")
  AWarpOnAPlaceholderBehindTheShapeIsInherited
  OnlyTheWarpedBoxDrawsNoText
```

The four that keep passing are the ones asserting behaviour that must **not** change:
`textNoShape` is not a text path, an unstated body is not a text path, and all three copies of
the string are still extracted.

### A trap worth writing down: `mv` back defeats the up-to-date check

Restoring `file.after` over `file` with `mv` preserves the copy's **older** mtime, so MSBuild
saw the source as older than the output and did not rebuild. The build said "Build succeeded, 0
warnings", the CLI's timestamp updated, and the next sweep measured the *unfixed* binary and
reported the unchanged 1189/1133. It looked exactly like the fix not working. `touch` the
restored files before building, and prove the binary is the one you think it is — here, by
rendering the fixture and counting 2 rather than 3.

## The ceiling table

`TODO.raster-ceiling.md:212` and `raster-ceiling-pages.tsv:68` listed **page 14 of this
document as a raster ceiling**. It never was: the +28 was our own WordArt. Both are corrected —
the row struck through in the table with the file's existing false-positive convention, and the
TSV verdict changed from `ceiling` to `not-a-ceiling-our-wordart` so the record survives while
consumers filtering on `ceiling` stop seeing it.

Re-derived by importing the detector's own functions rather than by assertion:

| | before the fix | after |
|---|---|---|
| page 13 | ours 148 / ref 120, +28, floor 30.0 → **not flagged** | +1 → not flagged |
| page 14 | ours 117 / ref 89, +28, floor 22.2 → **flagged** | +1 → not flagged |

**Page 13 was missed for a different reason than the brief supposed.** The brief attributed it
to condition 3 being computed on the net delta — a defect of ours cancelling a ceiling. Nothing
cancels anything on page 13: gross and net are both +28. It is the **25% ratio bar**. The
reference has 120 words on page 13 and 89 on page 14, so the same +28 clears the floor on one
page and misses it on the other. A defect of fixed size is flagged according to how much *other*
text the page happens to carry.

### The general fault, measured

Over the whole slides track — 163 documents, 4199 pages, our renderings against the banked
references:

| | pages | share |
|---|---:|---|
| condition 1 fires (reference raster we do not draw) | **1482** | 35.3% of all pages |
| …of those, our word count **equals** the reference's exactly | **1091** | 73.6% |
| …of those, also clearing condition 3 | 26 | 1.8% |

Condition 1 fires on more than a third of every slide page in the corpus and says nothing on
three-quarters of them. On this document alone it fires on **23 of 30 pages**, 21 with a zero
word delta. So **the flag is condition 3 almost by itself** — and condition 3 is a statement
about our own surplus, not about the reference. Any defect that adds words to a page has a
better-than-one-in-three chance of landing on a condition-1 page and being filed here as
unwinnable, which is the most expensive place to put a fixable defect.

## Scoring the prediction

| | prediction | outcome |
|---|---|---|
| P1 | the +28 is set B; 1189 → **1133** exactly | **partly wrong.** 1135. Set B was 27 of the 28; the last one per page is a `pdftotext` split boundary on set A that predates the fix |
| P2 | `slides/extra-001` becomes `MATCH`, 30/30 | **right.** 30/30, 1135/1133, `TOTAL 1 MATCH 1 MISMATCH 0` |
| P3 | grep says 67, **exactly 2 of 163 renderings change** | **right,** and by SHA-256 over both full renderings, not by inspection |
| P4 | no regression; `redac-sas-201403` still passes with ~11 fewer | **right.** 144/144; −11 exactly; still `match` |
| P5 | suppressing *reduces* the ink diff, and the fix is wrong if it does not | **right.** −30 509 px on each page |
| P6 | fidelity 30 of 550; new tests fail unfixed | **right.** 30/550/0 skipped, 550 discovered; 6 of 10 fail unfixed |
| P7 | page 14 is not a ceiling, page 13 unlisted, and the general fault is measurable | **right on the rows, wrong on the cause of page 13's absence** — the ratio bar, not the net delta. The general fault is measured and is larger than described |

Two of seven were wrong in a way the measurement corrected, both because a plausible
arithmetic identity (56 = 2 × 28 = set B) was assumed to be exact when it was 54 + 2.

## Follow-ups

1. **Implement the arch geometry.** The eight reference bounding boxes on pages 13 and 14 are
   an exact target to validate against, and the residual is now isolated to the four quadrants.
   It needs glyph-to-path — `IDrawingSink.FillPath` exists, a glyph outline source does not
   above `XRay.Office.Rendering` — and a per-run transform on `PlacedText`.
2. **The other Fontwork readers.** `.ppt` (`PptShapeGeometry` already knows the word) and ODF
   (`draw:enhanced-geometry/@draw:text-path`) take different paths and are untouched; so is the
   word-processing DOCX shape reader, where the corpus holds 2 `textPlain` documents.
3. **`CLAUDE.md` still says the table "lists 37 pages".** With this correction the TSV holds 34
   rows whose verdict is `ceiling`. Left alone here to avoid a conflict with the other worktrees.
4. **The −8 now visible on `redac-sas-201403`**, previously masked by the 11 WordArt words.
5. **Condition 3 on the ours-only token count**, as `TODO.raster-ceiling.md` already recommends,
   plus a condition that says something about the reference's ink — condition 1 as it stands
   does not discriminate.

## Reproducing

```sh
export SOURCE_DATE_EPOCH=1700000000
export XRAY_OFFICE_CLI=/c/sandbox/workdir/wt-s-extra2/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli

python3 dotnet/probes/slides-extra-01/make-fixture.py /tmp/text-warp-deck.pptx
soffice --headless --convert-to pdf --outdir /tmp /tmp/text-warp-deck.pptx
pdftotext -raw /tmp/text-warp-deck.pdf -          # two copies, not three

.claude/skills/corpus-batches/scripts/batch-check.sh \
    /c/sandbox/workdir/sample-files 'slides/extra-001' /tmp/out 2 > /tmp/sweep.log 2>&1
grep '^TOTAL' /tmp/sweep.log
```

Never pipe `batch-check.sh` into `head` or `tail`; redirect and read the file.
