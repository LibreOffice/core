# Slides-C round 01 — Escher picture crop, and a run's colour as a fill

An implementation round, not a diagnosis one. Two defects were diagnosed to the line by
`slides-b-01`; both are fixed, both are confirmed against the reference, and the round's own
measurements are below.

**Headline.** Both fixes land and both reproduce the reference: `Thailand17.ppt` page 22 goes from
18.92% of pixels differing to 0.63%, and `OnTrac` from 11 major pages to 2. Measured reach is
**16 of 163 slide renderings**, and **0 of 371** words and sheets renderings — the `XRay.Office.Core`
move is behaviour-free, as it had to be. **Zero verdicts move**, on all three tracks, which is what
was predicted and what the gate's definition requires.

**The refutation is about an instrument, and it is the part worth carrying forward.** The brief's
"run-level `a:gradFill` with alpha: reach 16 decks" is wrong. It is 1 deck. The predecessor's
census reproduces to the digit — 16 decks, 40 instances — because its regex does not require an
element's closing tag to match its opening one, so every self-closing `<a:rPr .../>` in the corpus
opens a span that runs forward until the next unrelated `</a:endParaRPr>` and swallows whatever
`a:gradFill` lies between. Corrected, the census says 1 deck / 2 instances, and the measurement
says 1 deck. The same defect inflates that round's `run_alpha` figure from 21 decks to 51.

## 1. The prediction, and how it did

`prediction.md` in this directory was committed as `7b4a5228c17` before any post-change
measurement and is unedited.

| # | Predicted | Outcome |
|---|---|---|
| verdicts | **zero movement, all three tracks** | **right** — 0 of 16 changed renderings moved, and all three gate columns are identical *to the digit* on both legs |
| crop reach | 12–16 of 51 `.ppt` decks | **right, at the top of the band** — **15 of 51** |
| gradient reach | 5–16 of 112 `pptx` decks | **wrong, and far under** — **1 of 112** |
| slides total | 17–32 of 163 | **wrong, just under** — **16 of 163** |
| words | **0 of 200** | **right** |
| sheets | **0 of 171** | **right** |
| refutation 1 | the `+ 1` in `lcl_ApplyCropping` is in the wrong coordinate space and must not be ported | **holds** — plain fractions reconcile the reference to 0.02 pt |
| refutation 2 | at least one censused crop deck will not change at all | **right, and it is exactly one** — `outlook_of_nigerian_pension_sector.ppt` |
| refutation 3 | the stop rule is LibreOffice's, not the chart heuristic | **holds**, and is now pinned by a test |

I predicted the gradient would land at the bottom of its band. It landed *below* the band, and the
reason is not that the fix under-reaches — it is that the number the band was built on was never
real. That is the one thing in this round I got wrong in a way that mattered, and it was wrong
because I took a predecessor's census figure as an input instead of auditing it first.

## 2. Instruments, checked against known answers before use

Three controls, all run before any headline number was believed:

- **Sweep determinism.** `slides/batch-004` rendered twice with the *same* binary: 10 of 10
  byte-identical, raw and after `/CreationDate` normalisation. With `SOURCE_DATE_EPOCH=1700000000`
  and `TZ=UTC` the two forms agree everywhere in this round, which is itself the check that the
  dates were actually pinned.
- **The two binaries are the two binaries.** `Thailand17.ppt` page 22 image destination:
  base `(10.75, −0.25)–(666.38, 528.00)`, branch `(−47.96, −58.95)–(685.95, 528.00)`, reference
  `(−47.96, −58.93)–(685.96, 528.04)`. The base leg reproduces the predecessor's measured wrong
  rectangle exactly and the branch leg reproduces the reference to 0.03 pt.
- **The pixel differ reproduces a known answer.** `slides-b-01` measured `Thailand17` page 22 at
  18.92% of pixels, `ink% 4.49`, `|ink|% 4.55`, 15 regions. My base leg: 18.92 / 4.49 / 4.55 / 15.
  Identical, so the 0.63% on the branch leg is a real move and not a change of instrument.

**One sweep was thrown away rather than reported.** The first base sweep printed
`rendered 163 of 163` and left 158 files on disk; the container had been killed between the write
and the flush. A sweep that counts from its own in-flight bookkeeping cannot see that, so
`sweep.sh` now re-counts from the directory at the end, re-renders anything missing, and exits
non-zero if the count still does not match its input count. Every figure below is from a sweep
that passed that check.

`pdf-ops.py`'s `sh`-operator blindness, which the previous round flagged, did not bite: neither
fix is a gradient, and the alpha was read from the PDF's ExtGState dictionaries directly rather
than from an operator dump.

## 3. Fix 1 — Escher picture cropping on the `.ppt` path

### What changed

`PptSlideLayout.Picture` passed the shape's placed rectangle straight through and its own remarks
admitted it. It now grows that rectangle by whatever the four crop properties throw away; the clip
that makes it *look* cropped is the shape outline `SlideDrawing.DrawPicture` was already applying
to every picture.

- `XRay.Office.Core.Geometry.PictureCrop` — `Uncropped` and `Inset`, moved down from
  `XRay.Office.Presentations.Layout.SlideImages`, unchanged.
- `XRay.Office.MsBinary.Escher.EscherPicture.Cropped` — the four-property read, in the layer DOC,
  XLS and PPT already share.
- `EscherPropertyIds.CropFromTop`/`Bottom`/`Left`/`Right` = 256–259, read **signed**.

### Measured reach

**15 of 51 `.ppt` decks changed their rendering.** The census ceiling was 16, and the one deck
that did not move is `outlook_of_nigerian_pension_sector.ppt`, whose only cropped shape is on a
**master**, is 121 × 109 master units (15.1 × 13.6 pt), and is anchored at negative *x* — off the
left edge of the slide. Which is precisely the blind spot the predecessor's census declared for
itself: *"it counts shapes, not pages: several of the 100 are on masters or notes pages."* The
census was a ceiling, it said so, and it overshot by exactly one.

Two decks changed that are **not** in the ≥10% "unmistakable" list — `Employment-Based_I-485.ppt`
and `W3_Case_Study_of_a_Tsunami_Warning_Simulation_Exercise_Ed.ppt` — both in the 16-deck ≥2%
band. So the ≥10% list of 14 predicted 13 movers, and the ≥2% list of 16 predicted 15.

### Fidelity

| | before | after |
|---|---:|---:|
| `Thailand17` page 22, pixels differing | 18.92% | **0.63%** |
| …`\|ink\|%` | 4.55 | **0.06** |
| …verdict | MAJOR | **ok** |
| `Thailand17` whole document, major pages | 15 of 54 | 14 of 54 |
| `architecture6.ppt`, major pages | 6 | **1** |
| `Architecture.ppt` | 5 | 4 |
| `Lepore.ppt` | 2 | 1 |

### The one instruction in the brief I did not follow

The brief says *"Note the `+ 1`: the reference computes `(height + 1) × factor + 0.5` in pixels,
not `height × factor`."* It is really there, at `msdffimp.cxx:3805-3826`, and it must **not** be
ported. It runs in the pixel space of a bitmap LibreOffice is about to trim, not in the shape's
placed rectangle; carrying it across would be a half-pixel rounding rule scaled up by the ratio of
the shape to the bitmap. Plain fractions land on the reference's own destination to 0.03 pt in
three independent coordinates on `Thailand17`, and to 0.03 pt on all four edges of the round-trip
fixture. The reason to record this is that the citation is correct and the instruction drawn from
it is not — the line says what the brief says it says, in a function that is doing something else.

## 4. Fix 2 — a run's colour from `a:gradFill`, with its alpha

### What changed

`PptxTextBody.SolidColour` read `a:solidFill` and nothing else, so a run whose colour was stated
as any other fill resolved to nothing and inherited black. It is now `RunColour` and reduces an
`a:gradFill` to one colour by LibreOffice's own rule: stops ordered **by position**, take the
first, or the second when there are more than two (`fillproperties.cxx:410-418`).

### Measured reach: 1 deck, and the brief's 16 is an artefact

**1 of 112 `pptx` decks changed** — `OnTrac_StarCertificationProgram-3Day.pptx`.

The brief's figure of 16 decks comes from `slides-b-01/census.py:74`:

```python
re.finditer(r'<a:(?:rPr|defRPr|endParaRPr)\b.*?</a:(?:rPr|defRPr|endParaRPr)>', d, re.S)
```

There is no backreference between the opening alternation and the closing one, and `.*?` under
`re.S` is not stopped by anything. A **self-closing** `<a:rPr lang="en-US"/>` — by far the
commonest form of the element in a real deck — therefore does not match as itself; it opens a span
that runs forward to the next closing tag of *any* of the three names, dragging whole paragraphs
and `<a:spPr>` blocks along with it, and any `a:gradFill` in that wake is counted as a run colour.

`gradfill-census-audit.py` runs both rules over all 112 decks:

| rule | decks | instances |
|---|---:|---:|
| `census.py:74` as written | **16** | 40 |
| the same with matched tags | **1** | 2 |
| measured, by rendering | **1** | — |

One matched span from `PAL Block Intro 2023.pptx / slide10.xml` is **2391 characters** long, opens
on `<a:rPr lang="en-US" b="1" dirty="0"/>`, crosses four `<a:p>` elements and closes on an
unrelated `</a:endParaRPr>`. That is where 39 of the 40 "instances" live.

The corrected census and the measurement agree exactly — 1 and 1 — which is the strongest form
this project's census-versus-reach comparison has taken. **The same defect is in the next rule
down** (`census.py:79`, `run_alpha`): 51 decks / 1410 instances as written, **21 decks / 942**
with matched tags. And neither reproduces that round's *reported* 16 decks / 60 instances for
`run_alpha` at all, so that table row does not follow from its own script by any reading.

### Fidelity

The reference draws the 82 pt background page number as `0 0 0 rg` inside a transparency group
under `/CA 0.1 /ca 0.1`. We now emit `<</Type/ExtGState/ca 0.102>>` — 10% quantised to a byte,
25.5 → 26, 26/255 = 0.10196 — and it appears in the branch leg only.

`OnTrac` whole document: **11 major pages → 2**, the largest single-deck improvement in the round.

**The user's report was right and the brief's mechanism was wrong**, and the distinction is worth
preserving because it is this project's most repeated pattern. The user said the number "draws
black where the reference draws grey". The brief asked what the reference resolves that
*placeholder's colour* from. There is no placeholder — the shape carries `userDrawn="1"` and no
`p:ph` — there is no grey, and there is no colour lookup. It is black at a tenth opacity, which
looks like grey over white and does not over the photograph it actually sits on.

Not chased, as instructed: the reference wrapping "10" onto two lines on the six two-digit pages.
Position is exact to 0.01 pt on the six single-digit pages and untouched here.

## 5. The cross-track sweep the `XRay.Office.Core` move owes

`XRay.Office.Core` is the zero-dependency layer every other library inherits, so the move is
measured rather than argued. Both legs are full renders of the whole track with the base and
branch binaries, byte-compared with `/CreationDate` and `/ModDate` normalised out.

| track | renderings | differ raw | differ after date normalisation |
|---|---:|---:|---:|
| **words** | 200 | **0** | **0** |
| **sheets** | 171 | **0** | **0** |
| slides | 163 | 16 | 16 |

**0 of 371.** The raw and normalised columns agree everywhere, which confirms the dates were
pinned and that the zeroes are not a masking artefact.

This is a round-45-shaped change, not a round-44-shaped one, and for the stated reason: round 44
changed behaviour in a shared component and every affected deck moved; round 45 moved code in
`XRay.Office.Text` without changing behaviour and swept 334 of 334 identical. Nothing above
`PictureCrop` calls it differently than it called `SlideImages`, and the only new caller is on the
`.ppt` path.

**What the move buys, and what it deliberately does not do yet.** `XlsDrawing.cs:346` and
`Ww8DocumentReader.Drawings.cs:200` each already read the same `pib` property from the same
`EscherPropertyTable`, and each is now **one call** from `EscherPicture.Cropped`. Neither is wired
up in this round, on purpose: doing it in the same commit would have destroyed the very
measurement the commit exists to produce. That is the cheapest remaining item on the two other
tracks and it should be a follow-up, measured the same way.

## 6. Verdicts: zero, and stated plainly

Only a rendering that changed can move a verdict; the other 147 slides, 200 words and 171 sheets
renderings are byte-identical, so their verdicts are identical by construction. The 16 that
changed were scored on both legs with `batch-check.sh`'s own arithmetic — page count exact,
extracted words inside a 2%-and-3-word band, zero unembedded fonts.

**0 of 16 moved.** Every gate column is identical between the two legs *to the digit*: same page
counts, same word counts, same zero unembedded fonts, on all sixteen. Five of the sixteen were
already failing on `words` before this round and still are, for reasons this round did not touch.

So the slides track stays at **132 of 163**, and words and sheets stay wherever they were.

This is the correct outcome and not a weak one. The gate asks three questions — how many pages,
how many extractable words, are the fonts embedded — and a crop is none of them and an alpha is
none of them. A round that works out in advance that its fixes are invisible to the gate, says so,
ships them anyway because the reference disagrees with us and the reference is ground truth, and
then measures the thing the gate cannot see, has done the right work. What that unseen measurement
says is:

| | base | branch |
|---|---:|---:|
| major pages across the 16 changed decks | **86** | **69** |

A 20% reduction, and **no deck got worse on any page**.

## 7. Tests

23 new tests. Split by what `verify-test.sh` actually established, not by intent.

### Verified by reintroduction — the mutation was applied, built, and named a failing test

| mutation | detected by |
|---|---|
| `EscherPicture.Cropped(shape.Properties, bounds)` → `bounds` in `PptSlideLayout` | `ACroppedPictureInAPptIsDrawnLargerThanItsFrame` |
| `SignedValue` → `Value` in `EscherPicture.Fraction` | `ANegativeCropIsReadAsNegative` |
| `CropFromRight` read replaced by `0` | `EachOfTheFourPropertiesIsRead`, + 2 others |
| `CropFromLeft = 258` → `250` | `ACroppedPictureInAPptIsDrawnLargerThanItsFrame` |
| `?? destination` → `?? DocRect.Empty` | `ACropThatKeepsNothingFallsBackToTheAnchor` |
| `PictureCrop.Uncropped` call in `PptxSlideLayout` replaced by identity | `TheSameCropOnThePptxPathGivesTheSameRectangle` |
| `width / horizontal` → `width` in `PictureCrop.Uncropped` | `UncroppedMatchesTheReferenceDestination`, + 4 |
| the keeps-nothing guard weakened | `ACropThatKeepsNothingIsRefusedRatherThanDivided` |
| an off-by-one EMU added to `Uncropped`'s left offset | `NoCropIsTheIdentity` |
| `width * (1 - left - right)` → `width` in `Inset` | `InsetLeavesTheStatedFractionsEmpty`, `ANegativeInsetOverhangs` |
| the whole `gradFill` branch made unreachable | `TwoIdenticalStopsAtOneTenthAlphaAreBlackAtOneTenthAlpha`, + 3 |
| `ordered[Count > 2 ? 1 : 0]` → `ordered[0]` | `TheStopIsTheFirstUnlessThereAreMoreThanTwo` |
| `OrderBy(stop => stop.Position)` dropped | `TheStopIsTheFirstUnlessThereAreMoreThanTwo` |
| `"solidFill"` → `"solidFillX"` | `ASolidFillIsUnaffected` |

### Drift guards only — kept deliberately, and labelled

- `EscherPictureCropTests.AShapeStatingNoCropIsPlacedWhereItIs` — guards the early-out for the
  overwhelmingly common case. Every mutation that breaks it also breaks a detector.
- `SlideRunGradientColourTests.ARunWithNoFillIsOpaqueBlack` — pins the state every
  gradient-filled run used to be in, so the fix is visible as a *move off* this answer rather
  than as an absolute. It cannot fail unless the fallback itself changes.

### Fixture provenance

Both new corpus fixtures are **authored for this round**, not collected. `picture-crop.pptx`
(4.8 KB) is written element by element by `make-crop-fixture.py` in this directory: an 8 × 8 PNG
of four quadrants in a minimal package, one `p:pic` at 288 × 216 pt stating
`<a:srcRect l="10000" t="20000" r="30000" b="40000"/>`. Re-running that script reproduces all
**12 members of the committed zip with identical content**; the archives differ only in the
timestamp fields the zip format embeds, which the script does not pin. `picture-crop.ppt`
(605 KB) is that file put through `soffice --convert-to ppt` — which is what turns the
`a:srcRect` into the four Escher properties, and is the only way to get a `.ppt` crop without
hand-assembling a `PowerPoint Document` stream. Its size is LibreOffice's export padding and is
in line with the committed `.ppt` fixtures already there (`ppt-page-fill.ppt` and
`ppt-fill-opacity.ppt` are both 604672 bytes). No third-party content, nothing from the web,
nothing personal, no licence question.

Testing both halves of the pair is deliberate: the `.pptx` holds the `XRay.Office.Core` call site
and the `.ppt` holds the new one, so the two paths are held to the same rectangle and the shared
arithmetic cannot drift apart. Reference agreement on the `.ppt`, at 26.2.4.2:
ours `(24.39, 36.01)–(505.13, 574.72)`, reference `(24.38, 36.03)–(505.13, 574.75)`.

## 8. Final state

```
dotnet build -v q -nologo     0 Warning(s)   0 Error(s)
```

| project | before | after |
|---|---:|---:|
| Core | 284 | **294** |
| Containers | 109 | 109 |
| Text | 287 | 287 |
| Vector | 295 | 295 |
| Rendering | 121 (1 skipped) | 121 (1 skipped) |
| Markup | 259 | 259 |
| OpenDocument | 125 | 125 |
| WordProcessing | 761 | 761 |
| Spreadsheets | 621 | 621 |
| Presentations | 592 | **605** |
| **total** | **3454** | **3477**, 0 failed, 1 skipped |

Two small corrections to the brief's stated baseline, neither consequential. Its per-project list
sums to **3454**, not the 3458 quoted alongside it. And it records Text as "287 (14 skipped)";
Text reports 287 total with **0** skipped here, before and after, so nothing of mine changed it.

## 9. Read versus inferred

**Measured or read:**

- Both destination rectangles on `Thailand17` page 22 from the two PDFs, and the reference's;
  the fixture's rectangles on both paths and the reference's; every pixel-diff figure, including
  the base-leg reproduction of the predecessor's 18.92 / 4.49 / 4.55 / 15.
- The ExtGState dictionaries of both `OnTrac` legs, read from the PDF objects.
- All three sweeps, both legs, complete and completeness-checked; all three byte comparisons.
- All 16 changed renderings' three gate columns on both legs against the canonical reference.
- `census.py:74` and `:79` re-run as written, reproducing 16/40 and 51/1410; the matched-tag
  rule giving 1/2 and 21/942; the 2391-character example span.
- `outlook_of_nigerian_pension_sector.ppt`'s single cropped shape, its master, its anchor.
- `msdffimp.cxx:3781-3833`, `msdffdef.hxx:131`, `fillproperties.cxx:402-425`,
  `textcharacterproperties.cxx:115-156`.
- Every reintroduction result in §7, from `verify-test.sh` exit 0 with a named test.

**Inferred, and flagged:**

- That the 158-of-163 shortfall in the discarded first sweep was a killed container rather than a
  script defect. The script could not distinguish them, which is why it now re-counts; I did not
  reproduce the loss deliberately.
- That the two decks changed outside the ≥10% list are the ≥2% band's other two. Their crop
  fractions were not individually re-read.
- That no other consumer of `SlideImages.Uncropped` existed. Established by grep over `src/` and
  `tests/`, which is a complete search of this repository and not of any caller outside it.

## 10. Files

- `prediction.md` — committed before measurement, unedited.
- `sweep.sh` — renders one track into `stem__ext.pdf`, with the completeness check §2 describes.
- `compare.py` — byte comparison of two sweeps, raw and with dates normalised.
- `gate-changed.sh` — `batch-check.sh`'s three columns, on the renderings that changed, both legs.
- `gradfill-reach.py` — run-level gradient fills with their part kind, alpha and chosen stop.
- `gradfill-census-audit.py` — the loose rule against the matched-tag one, with an example span.
- `make-crop-fixture.py` — builds `tests/corpus/features/picture-crop.pptx`; the `.ppt` is that
  file through `soffice --convert-to ppt`.

## 11. What the next round should take from this

1. **Wire `EscherPicture.Cropped` into `XlsDrawing` and `Ww8DocumentReader.Drawings`.** One call
   each, the arithmetic is shipped and tested, and the reach on those two tracks is unmeasured
   because this round deliberately kept its cross-track sweep at zero.
2. **Do not trust `slides-b-01`'s `run_gradfill` or `run_alpha` reach figures**, and check any
   other census on this project that pairs alternating tag names without a backreference.
3. The remaining `slides-b-01` items are untouched and their reach figures come from a
   *different* rule in the same script — the crop census, which used a binary record walker rather
   than a regex, and which this round confirmed to within one deck.
