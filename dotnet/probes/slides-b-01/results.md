# Slides-B round 01 — shape geometry and image placement

Five subjects: the arrow-shape cluster, `Wildlife for REDAC September 11.pptx` pages 3 and 13,
`Thailand17.ppt` image scaling, `OnTrac…`'s background page number, and whether Escher picture
cropping exists on the slide path.

**Headline.** Two of the five briefed items are refuted outright, and both refutations have a real
defect underneath them. The arrow cluster was fixed two rounds ago and the handover has been
carrying it as open since. The `OnTrac` page number *is* the user's defect exactly, but its
mechanism is not a placeholder colour lookup — the reference draws it **black at 10% alpha**, and
we draw it opaque because a run's colour is only read from `a:solidFill`. Three items are confirmed
and diagnosed to a line: `Thailand17` is Escher crop, unimplemented on the `.ppt` path while the
whole downstream half of it already exists and is proven on the `pptx` path; `Wildlife` page 3 is
a `a:scene3d` camera roll we never read; `Wildlife` pages 3/13/32/33 are a corner-focus circle
gradient we draw as a diagonal linear ramp where the reference draws a radial one.

## 0. Conditions, and one instrument correction taken mid-round

This round began with no package feed and no CLI, and the firewall was opened part-way through. So
items 1–5 were each diagnosed by source reading *first* and then measured. Where the two disagree,
the measurement is stated and wins; where only a read exists it is labelled.

**Every figure below was re-taken after `fonts-dejavu-core` was installed.** My first reference
renders (15:41) predate it; re-rendering the same five decks changed the fallback face on four of
them (`WenQuanYiZenHei` → `DejaVuSans`/`-Bold`), and our own renders changed on four of five as
well. `pdffonts` on my five re-renders is identical to
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/slides/` for all five, so my references and the
corrected corpus agree. `Thailand17` was byte-identical on our side across the font change, which
is a useful control: the item that turns entirely on image placement did not move.

Renders: `SOURCE_DATE_EPOCH=1700000000 TZ=UTC`, one `-env:UserInstallation` profile per concurrent
`soffice`. Reference is **LibreOffice 26.2.4.2**.

### An instrument caveat that nearly cost this round a wrong finding

**`pdf-ops.py` does not report the PDF `sh` shading operator.** We emit a gradient as a real
shading (`/Sh3 sh`); LibreOffice emits gradients as stacked solid polygons. So on `Wildlife`
page 3 the operator dump shows a page-covering fill in the reference and *nothing at all* in ours,
which reads as "we draw no background" and is false — we draw the wrong background. The raster
diff and `pdftoppm` settled it. Anyone using `pdf-ops.py` to compare a gradient against
LibreOffice will see this asymmetry and should not read it as absence.

## 1. Prediction, committed before any measurement

The full text is in `prediction.md` in this directory, written after reading only
`PptShapeGeometry.cs` and the handover's slides section, before any deck was opened.

| # | Predicted | Outcome |
|---|---|---|
| Gate | **zero verdict movement** from every item; the track stays 151/163 | **holds** — nothing here touches slide count, extractable words or embedding |
| 1 | the preset entry is **not** missing; the brief's framing is what to refute | **right, and further** — not only present, the whole item was fixed in round 21 |
| 1a | most likely cause: the arrows are `mso_sptNotPrimitive` freeforms | **wrong** — 86 of 87 type-0 shapes in `Fundamentals` are group containers |
| 2 p3 | rotation stated somewhere we do not read for a picture | **right in shape, wrong in place** — it is `a:scene3d/a:camera/a:rot/@rev`, not an `a:xfrm` |
| 2 p13 | the two blocks are empty inherited placeholders the reference suppresses | **wrong** — they are the two pictures, and the difference is the 35 pt `a:ln` frame |
| 3 | the reference is right, we are wrong, and it is Escher crop | **right, both halves, measured to 0.03 pt** |
| 4 | the colour is inherited from a master/layout list style, not stated on the run | **right about the route, wrong about the value** — it is a `a:gradFill` carrying `a:alpha`, and the reference renders it as alpha, not as a grey |
| 5 | slides has no Escher crop either, so it is not a route for words | **right that `.ppt` lacks it; wrong that it is no route** — the hard half exists |
| reach | crop 10–25 of the `.ppt` decks; camera/blocks wider | crop **16 of 51**, in band; camera rev **1 of 163**, far under |

I predicted at least one of the three reach estimates would come in far under. The corner-focus
gradient did: census says 2 decks / 5 instances, measured visible defect is **1 deck / 4 pages**.

## 2. Item 1 — arrows as rectangles: refuted, and the item is stale

**The shapes reach `PresetOf`, the entries exist, and they resolve.** Three independent checks:

- **Census of the two decks' own Escher records** (`ppt-crop-census.py` machinery). Every arrow in
  both decks carries a built-in `MSO_SPT` that `PptShapeGeometry.PresetOf` maps:
  `Fundamentals_Module_1_basics.ppt` — 7 × type 69 (`leftRightArrow`), 2 × type 104
  (`curvedUpArrow`), 1 × type 13 (`rightArrow`); `W3_Case_Study…` — 3 × type 103
  (`curvedLeftArrow`). None of them carries `pVertices` (325) or `pSegmentInfo` (326), so
  `PptCustomGeometry.Has` is false and the preset branch is taken —
  `src/XRay.Office.Presentations/MsBinary/PptSlideLayout.cs:925` and `:939-943`.
- **All 148 names in the table resolve.** Cross-checked every `N => "name"` in
  `src/XRay.Office.Presentations/MsBinary/PptShapeGeometry.cs:132-283` against the `s <name>` records
  in `src/XRay.Office.Ooxml/DrawingML/PresetShapeGeometry.txt`: 148 entries, 142 distinct names, **0
  missing**, out of 187 presets the table holds. The class comment's claim "a name here always
  resolves" is true.
- **Rendered.** `W3` page 8 and `Fundamentals` page 17 both draw proper arrows in our output —
  the tsunami travel-time curved arrow and the "3 R's" double-headed arrows between the blocks.
  Whole-document pixel diff against the reference: `Fundamentals` 1 major page of 26 (page 24, an
  unrelated chart), `W3` 3 of 20, and the largest `|ink|%` on any page of either deck is **0.89**.
  There is no arrow-shaped defect left to find.

**This was fixed in round 21 and the handover never retired the item.** `TODO.batches.md:9927`
records it in that round's own words: *"The two documents a human review flagged as 'blocks joined
by arrows come out as plain rectangles' both roughly halve: `Fundamentals_Module_1_basics.ppt`
7.18 → 3.37 and `W3_Case_Study…` 7.61 → 3.78"*, against a census ceiling of 37 of the 51 `.ppt`
decks and a measured reach of exactly 37 renderings. `HANDOVER.md:461` still lists it under "Still
open on the track", and `probes/slides-rebase-01/prediction.md:50` predicts the opposite of what is
true — that the shapes *do not reach* the table at all.

**Residual.** What is left on `Fundamentals` is the raster ceiling — page 6, a 529×355 image with a
soft mask, already on `TODO.raster-ceiling.md:117`. Nothing arrow-shaped.

**Reach of anything still missing from the table:** the deliberate absentees are types 0, 24–31,
75, 136–175, 201, 202, each justified in the class comment and each correct. Type 14
(`mso_sptThickArrow`) and type 100 (`mso_sptNotchedCircularArrow`) have no DrawingML preset and are
genuinely unreachable; neither appears in either deck.

**Action: strike the item from the handover.** Do not spend a round on it.

## 3. Item 3 + Item 5 — Escher picture crop, and the route it opens for words

### The measurement

`Thailand17.ppt` page 22 carries a near-full-slide photograph whose Escher property table states
`cropFromBottom = 6554` (10.00%), `cropFromLeft = 5243` (8.00%) and `cropFromRight = 1748`
(2.667%), on the shape whose `msofbtClientAnchor` is `(86, 96, 5331, 4322)` master units =
655.63 × 528.25 pt at 8 units/pt.

| | destination the picture is drawn into |
|---|---|
| ours | `(10.75, −0.25)–(666.38, 528.00)` — 655.63 × 528.25, i.e. exactly the anchor |
| reference | `(−47.96, −58.93)–(685.96, 528.04)` — 733.92 × 586.97 |

The reference's rectangle is the anchor divided by the surviving fractions, offset by the cropped
edges, to three decimal places:

```
width   655.625 / (1 − 0.0800 − 0.02667) = 733.92    measured 733.92
height  528.250 / (1 − 0      − 0.10   ) = 586.94    measured 586.97
left    10.75 − 0.0800 × 733.92          = −47.96    measured −47.96
```

That is `SlideImages.Uncropped` evaluated by hand. **The reference is right and we are wrong**, and
the visible consequence is exactly the user's report: the surviving 90% of the image is stretched
to fill the frame, so every feature in it is **1.111× taller** and 1.119× wider in the reference
than in ours. Page 22 is the deck's second-worst page in the whole-document diff — 18.92% of pixels
differ, `|ink|% = 4.55`, signed `ink% = 4.49`.

The deck's second cropped picture (page 8, `cropFromBottom = 2335` = 3.56%, `cropFromRight = 82` =
0.125%) is exported the other way — LibreOffice bakes the crop into the bitmap (`845 × 572` from a
source whose full height is `572 / (1 − 0.0356) = 593`) and draws it into the unexpanded anchor.
Same visible result, different PDF shape. Worth knowing before someone compares destination
rectangles across a corpus and finds only half the cases.

### Where it is, and why item 5's answer is "no, but"

- **The `.ppt` path does not apply the crop, and says so.**
  `src/XRay.Office.Presentations/MsBinary/PptSlideLayout.cs:1033-1036`, in `Picture`'s remarks:
  *"The destination is the shape's placed rectangle, with no crop applied. `cropFromTop` and its
  three siblings are recorded in the TODO rather than approximated"*. The method body is
  `:1039-1051` and passes `bounds` straight through.
- **The arithmetic already exists and is exercised.**
  `src/XRay.Office.Presentations/Layout/SlideImages.cs:76-92`, `SlideImages.Uncropped`, is the exact
  computation measured above, and `src/XRay.Office.Presentations/Ooxml/PptxSlideLayout.cs:1290-1292`
  already calls it for `a:srcRect` on the `pptx` path. The clip half is
  `src/XRay.Office.Presentations/Layout/SlideDrawing.cs:428-455`.
- **So the fix on the slide path is a four-property read and one call**, not a feature. The
  property ids are `cropFromTop = 256`, `cropFromBottom = 257`, `cropFromLeft = 258`,
  `cropFromRight = 259` — 16.16 fixed-point fractions, `include/svx/msdffdef.hxx:131` states the
  unit in its own comment, and `filter/source/msfilter/msdffimp.cxx:3781-3833`
  (`lcl_ApplyCropping`) divides each by 65536.0. **Note the `+ 1`**: the reference computes
  `(height + 1) × factor + 0.5` in pixels, not `height × factor`.
  `EscherPropertyIds` (`src/XRay.Office.MsBinary/Escher/EscherRecordTypes.cs:137-283`) does not yet
  name these four; it holds `Picture = 260` and `PictureName = 261` and stops.

**Item 5's answer, precisely.** The slide path does *not* have Escher cropping — so it is not a
ready-made route for words in the sense the handover means. But the part that is actually hard —
turning a crop into a larger destination plus a clip, in a drawing IR that has clipping and no
crop — **is** implemented, proven against the reference on the `pptx` path, and format-neutral. The
obstacle to reusing it for `.doc` is layering, not logic: `SlideImages` lives in
`XRay.Office.Presentations`, a sibling of `XRay.Office.WordProcessing`, so a word reader cannot reach
it. `Uncropped` and `Inset` depend on nothing but `DocRect` and `Length`, both in
`XRay.Office.Core.Geometry`/`Units`, so they pass the project's own stated test for what belongs in
Core (`dotnet/CLAUDE.md`, the `Core/Numbers` rule: *"a thing belongs in Core when it depends on
nothing above Core, whatever it was written for"*). **Moving those two methods down, then reading
the four Escher properties once in `XRay.Office.MsBinary`, serves `.ppt`, `.doc` and `.xls` at
once** — which is the stated reason the Escher library is shared in the first place.

### Reach

`census.py` walks every `msofbtOPT` record in the `PowerPoint Document` stream of all 51 `.ppt`
decks and counts shapes stating any non-zero crop property.

| | decks | shapes |
|---|---:|---:|
| any non-zero crop | **16 of 51** | 100 |
| …and also stating `pib` (260), so a picture is actually drawn | **16** | 100 |
| largest crop ≥ 2% (visible at 150 dpi) | 16 | 98 |
| largest crop ≥ 10% (unmistakable) | **14** | 82 |

Decks at ≥10%: `8.16_AOD_FINAL_Provider_Training_Presentation_9_2009.ppt`,
`Airport Planning 09112013.ppt`, `Architecture.ppt`, `BUS-Chapter 05.ppt`, `EG1_dsrc tech.ppt`,
`Fundamentals_Module_1_basics.ppt`, `JesuitAssocOfStudentPersonnel.ppt`, `Lepore.ppt`,
`Thailand17.ppt`, `architecture6.ppt`, `berlin.ppt`, `introduction_to_bea_tuxedo.ppt`,
`outlook_of_nigerian_pension_sector.ppt`, `pres_ioc_phuket.ppt`.

**This is the widest-reaching finding of the round** — 31% of the binary decks, and unlike a
gradient census these are shapes with a `pib`, so every one of them resolves to a drawn picture.

**What this census cannot see.** It counts shapes, not *pages*: several of the 100 are on masters
or notes pages and some decks crop the same picture repeatedly. It does not check whether the shape
is behind something else. And it says nothing about the `.doc`/`.xls` halves of the corpus, which
share the record format and were not walked.

## 4. Item 2 — `Wildlife for REDAC September 11.pptx`

Whole-document diff, ours vs the corrected reference: 41 pages, **7 major**, and the four worst are
pages **3 (65.4% of pixels, |ink| 15.74), 13 (59.2%, 21.32), 32 (60.2%, 16.93), 33 (84.6%, 3.13)**.
Those four are exactly the four slides in the deck that state a background gradient with
`<a:path path="circle">`. Two separate defects sit on page 3 and two on page 13.

### 2a. Page 3, the picture the reference rotates — confirmed, 6.000°

The `p:pic` on `ppt/slides/slide3.xml` states `<a:off x="5257800" y="1600200"/>`
`<a:ext cx="2638032" cy="3962400"/>` = 207.72 × 312.00 pt and **no `rot` on its `a:xfrm`**. We draw
it axis-aligned at `(414.00, 102.00)–(621.72, 414.00)`. The reference draws it inside an
axis-aligned box of `239.16 × 332.00`, which is the rotated rectangle's bounding box:

```
w·|cos θ| + h·|sin θ| = 239.19,  w·|sin θ| + h·|cos θ| = 332.00   at θ = 6.000°
measured                239.16                          332.00
```

The rotation is stated on the picture's `<a:scene3d><a:camera prst="orthographicFront">`
`<a:rot lat="0" lon="0" rev="360000"/>`. `rev` is 1/60000 deg, so **360000 = 6°** — the measured
angle to three decimals. A second, independent instance of the same number: the reference's 15 pt
white `a:ln` around that picture is stroked along a parallelogram whose long side runs
`(390.019, 408.954) → (611.49, 432.255)`, an angle of `atan2(23.30, 221.47) = 6.005°`.

- **Reference side, read:** `oox/source/drawingml/scene3dcontext.cxx:179` parses
  `a:camera/a:rot/@rev` into `Shape3DProperties::maCameraRotation.mnRevolution`;
  `oox/source/drawingml/scene3dhelper.cxx:215-270` (`getAPIAnglesFrom3DProperties`) turns it into
  the shape's z-rotation, combining it with the shape's own rotation
  (`nRevolution -= rnMSOShapeRotation`, `:263`) and zeroing it for the legacy-perspective camera
  presets (`:230-234`).
- **Our side, read:** `src/XRay.Office.Presentations/Ooxml/PptxSlideLayout.cs:1741-1744`,
  `Rotation(XElement? transform)`, reads `@rot` off an `a:xfrm` and nothing else. `scene3d` and
  `camera` appear nowhere in the file. Call sites at `:453`, `:622`, `:658`, `:766`.

### 2b. Page 13, "two blocks we draw and the reference does not" — it is the picture frame

Not empty placeholders, and not a missing picture: both pictures are drawn in both, at the same
place to 0.02 pt (`(445.79, 340.50)–(650.01, 495.87)` ours; `(445.81, 340.55)–(649.99, 495.89)`
reference). The difference is the frame around them. Each `p:pic` states
`<a:ln w="444500" cap="sq">` — a **35 pt** black outline. Both sides use the right width; they put
it in different places:

```
ours       35.0079 w   445.792 340.5019 204.2175 155.3631 re  S     ← stroked on the shape rect
reference  35.00731 w  428.315 513.354 m … 667.502 323.008 …  S     ← rect grown 17.49 pt each side
```

So the reference's black band lies entirely *outside* the picture (17.5 pt of visible frame all
round), while ours straddles the edge and the picture covers its inner half — leaving a frame
half as thick and 17.5 pt too small on every side. That is what the pixel diff attributes as two
block-shaped regions. The same defect is on page 3, where the picture's `a:ln w="190500"` is a
15 pt white frame: the reference's stroked parallelogram sides are `207.72 + 15` and `312 + 15`
long, the shape's own extent plus the full line width, in both axes.

**Read on our side** — `Line`/`Pen` at `src/XRay.Office.Presentations/Ooxml/PptxSlideLayout.cs:1646-1689`
resolve the width correctly (`Length.FromMm100((Emu(line, "w") + 180) / 360)`, and 444500 EMU →
1235 mm100 → 35.0079 pt is exact). **Inferred, not read:** the rule that the reference grows the
outline rectangle by half the line width before stroking it. I measured it on three shapes across
two pages and it holds in both axes each time; I did not find the C++ line that does it, and one
counter-case exists in the same PDF (a rounded-rectangle picture frame at `7.00146 w` whose stroke
path equals its clip path, i.e. no growth), so the rule is probably specific to a rectangular
`p:pic` and should be confirmed before it is implemented.

### 2c. Pages 3, 13, 32, 33 — the corner-focus circle gradient

Both sides draw a background gradient; they draw different gradients. `p:bg/p:bgPr` on slide 3:

```xml
<a:gradFill><a:gsLst><a:gs pos="0"><a:schemeClr val="bg1"/></a:gs>
<a:gs pos="100000"><a:srgbClr val="172977"/></a:gs></a:gsLst>
<a:path path="circle"><a:fillToRect l="100000" t="100000"/></a:path></a:gradFill>
```

`fillToRect l=100000 t=100000` puts the focus on the slide's bottom-right corner. The reference
draws **concentric circles about that corner** — its banding polygons are squares centred at
`(720, 0)` with radii falling from 446.12, i.e. a radial ramp. We draw a **diagonal linear ramp**
corner to corner, so the white reaches the middle of the slide instead of staying a glow at the
corner; at 40 dpi the two pages are unmistakably different.

The branch is explicit and its comment states the refuted claim:

```
src/XRay.Office.Presentations/Ooxml/PptxSlideLayout.cs:1411-1418
    if (gradient.Path == "circle" && (cx is 0 or 100) && (cy is 0 or 100))
    {
        // The focus is a corner: the reference draws the diagonal linear ramp instead,
        // stop 0 at that corner. …
        return SlideGradients.Linear(box, cx == 0 ? 1 : -1, cy == 0 ? 1 : -1, stops) …
    }
```

Removing that special case drops through to `:1420-1429`, which already builds
`GradientKind.Radial` about the computed centre — which is what the reference draws. The claim was
presumably true of LibreOffice 24.2.7.2; it is false of 26.2.4.2, and it is the kind of
binary-calibrated claim `dotnet/CLAUDE.md` says needs one re-check each. **I did not verify it
against 24.2.7.2** — that binary is not installable here — so "the reference changed" is inference;
"the reference at 26.2.4.2 draws a radial ramp" is measurement.

### Reach of 2a–2c

Census over all 163 decks, counting only constructs stated in a **slide, layout or master** part —
never a theme, which is the mistake that turned an 87-deck census into 2 changed renderings.

| construct | decks | instances | measured visible defect |
|---|---:|---:|---|
| `a:camera/a:rot/@rev` not a whole turn | **1 of 163** | 2 | 1 page (`Wildlife` 3) |
| `a:ln` with `w > 0` on a `p:pic` | 36 of 112 pptx | 200 | — |
| …with `w ≥ 5 pt`, so the frame is visible | **7 of 112** | 12 | 2 pages measured |
| `a:path path="circle"` with a **corner** focus | **2 of 163** | 5 | **1 deck, 4 pages** |
| `a:path path="circle"` with any other focus | 4 of 163 | 36 | not this branch |
| `a:path path="rect"` or `"shape"` | **0** | **0** | — |

Two things to take from this table:

- **The handover's "no corpus deck states a `rect`/`shape` path gradient" is confirmed exactly** —
  0 and 0 over all 163 decks and all three part kinds. That instruction was right and this census
  is its second measurement.
- **The corner-focus branch reaches 2 decks and I measured a defect on 1.** The other instance is
  `3492.pptx` slide 5; rendered and diffed against the corrected reference, that page comes out at
  2.39% pixels / `|ink| 0.31` — *not* materially divergent. So the census ceiling is 5 gradients
  and the measured floor is 4 pages in one deck. That gap is the pattern this project keeps
  hitting, and it is worth fixing anyway: those 4 pages are 60–85% of their pixels, the largest
  per-page divergence anywhere in this round's five decks.

The 7 decks with a ≥5 pt picture border are `G-InvoicingKeithJarboe.pptx`,
`Presentation - Identify Components of the Airport (1).pptx`, `Wildlife for REDAC September 11.pptx`,
`redac-sas-201703-hf-research-division.pptx`, `vv_summit_SAIC-PRESENTATION_FAA-V&V-Summit_508c.pptx`,
`vvsummit2022-Research-Roadmap-and-the-UML-3-Operational-Integration-Assessment.pptx`,
`vvsummit2022-SAIC-PRESENTATION.pptx`.

## 5. Item 4 — `OnTrac…`'s background page number: the user is right, the brief's mechanism is not

The user reported *"the very large background page number draws black where the reference draws
grey, and the block is shifted"*. Both halves reproduce. The brief asks what the reference resolves
that **placeholder's colour** from. It is not a placeholder and it is not a colour.

### What draws it

`ppt/slideMasters/slideMaster3.xml`, a shape named "Slide Number Placeholder 5" that carries
`<p:nvPr userDrawn="1"/>` and **no `p:ph`** — a plain text box, not a placeholder at all. Its
`a:lstStyle/a:lvl1pPr/a:defRPr` states `sz="8200"`, `algn="r"`,
`<a:latin typeface="Impact"/>`, and for its colour:

```xml
<a:gradFill><a:gsLst>
  <a:gs pos="0"><a:schemeClr val="tx1"><a:alpha val="10000"/></a:schemeClr></a:gs>
  <a:gs pos="100000"><a:schemeClr val="tx1"><a:alpha val="10000"/></a:schemeClr></a:gs>
</a:gsLst><a:lin ang="5400000" scaled="0"/></a:gradFill>
```

Two identical stops: `tx1` at **10% alpha**. The deck's own `sldNum` placeholders — layout 8's and
master 3's — are a different, 16 pt, bottom-right thing that neither side draws large.

### What the reference does with it, measured

The reference emits the digits inside a transparency-group form XObject drawn under an ExtGState:

```
q 842.939 390.53 116.759 149.47 re W* n          ← clipped to the shape box
q /EGS823 gs /Tr822 Do Q                          ← 823: << /CA 0.1 /ca 0.1 >>
  Tr822 content:  0 0 0 rg BT 848.239 473.698 Td /F1 82.006 Tf <0E>Tj ET
```

So **"grey" is black at `ca 0.1`**, not a grey colour — and 82.006 pt is `sz="8200"` exactly. The
clip rectangle `x = 842.939` is the shape's `a:off x="10705769"` = 843.0 pt, and its width 116.759
is `a:ext cx="1483056"` = 116.78 pt. Both sides agree on the size.

Our emission on the same page:

```
q 0 0 0 rg BT 900.3794 473.6693 Td /F1 82.0063 Tf <23>Tj ET Q
```

Opaque black, no `gs`, and no clip.

### Position — right on 6 pages of 12, and the 6 that are wrong are a line break

Reading the `Td` of every 82 pt run in both PDFs:

| pages | reference | ours |
|---|---|---|
| 4–9 (one digit) | `900.369 473.698`, six times | `900.3794 473.6693` — **agreeing to 0.01 pt** |
| 10–15 (two digits) | two runs: `848.239 473.698` and `848.239 391.691` | one run, both glyphs, `861.34 473.67` |

The reference **wraps "10" onto two lines** inside that 116.8 pt box; we fit it on one. Because the
paragraph is `algn="r"`, one line of two glyphs right-aligns 13.1 pt to the right of one line of
one, and our "1" then sits above the top of the page and is clipped away — which is the "block is
shifted" the user saw. I did not establish why the reference wraps: the box's usable width is
116.78 − 2 × 7.2 pt inset = 102.4 pt and two digit advances measure 78.1 pt from our own
right-alignment, which fits. That is the one loose end on this item.

### Why we lose the colour — two independent holes, both read

1. **`src/XRay.Office.Presentations/Ooxml/PptxTextBody.cs:744-755`**, `SolidColour`, is the only
   colour route for a run: `XElement? solid = Drawing.Child(properties, "solidFill"); if (solid is
   null) return null;`. The colour here is an `a:gradFill`, so nothing resolves and the run falls
   back to black. `gradFill` does not appear anywhere in `PptxTextBody.cs`.
2. Even resolved, the alpha would survive only through the run's colour. The transform machinery
   *does* support it — `src/XRay.Office.Ooxml/DrawingML/DrawingColourTransforms.cs:142-144` maps
   `alpha`/`alphaMod`/`alphaOff`, and `:358-366` applies them — so this hole is upstream of a
   working mechanism, not a missing one.

**The reference's route, read:** `oox/source/drawingml/textcharacterpropertiescontext.cxx:126-129`
parses a run's `a:gradFill` into `maFillProperties`;
`oox/source/drawingml/textcharacterproperties.cxx:115-117` reduces any fill, gradient included, to
one colour via `getBestSolidColor()`; `:139` sets `PROP_CharColor` and `:153-156` sets
`PROP_CharTransparence` from that colour's alpha. That last line is the `/ca 0.1`.

### Reach

| construct (slide/layout/master parts only) | decks | instances |
|---|---:|---:|
| a run or list level whose colour is an `a:gradFill` | **16 of 163** | 40 |
| a run's `a:solidFill` carrying an `a:alpha` | **16 of 163** | 60 |

Both are real and both are invisible to every gate. The second is the cheaper fix of the two and
already has its machinery. Largest holders of run-level alpha:
`section_1_our_rights_presentation.pptx` (16), `REDAC Briefing_SSIT_CARA_08132014.ppt.pptx` (8),
`NAS-Infrastructure-Roadmaps-v16.0.pptx` (7) — the last of which is a reviewed false positive and
must not be worked.

**What this census cannot see.** It counts declarations in a part, not runs that resolve against
them: a `defRPr` in a master `lstStyle` may be inherited by many runs or by none, and a `rPr` on a
run that is later overridden counts anyway. So 40 and 60 are ceilings, and the `OnTrac` case —
one master declaration reaching 12 of 15 pages — shows the multiplier can go either way.

## 6. Read versus inferred

**Read (source, both trees) or measured (renders and PDFs):**

- Every arrow's `MSO_SPT`, its absence of `pVertices`, the 148/148 name resolution, and both decks
  rendered with correct arrows.
- Round 21's own measurement of the arrow fix, quoted from `TODO.batches.md:9927`.
- `Thailand17`'s crop fractions from the Escher property table; both destination rectangles from
  the two PDFs; the arithmetic reconciling them to 0.03 pt; `msdffimp.cxx:3781-3833` for the unit
  and the `+1`; `PptSlideLayout.cs:1033-1036` admitting the gap; `SlideImages.Uncropped` and its
  `pptx` call site.
- `rev="360000"`, the 6.000° bounding box, the 6.005° stroke angle, `scene3dcontext.cxx:179`,
  `scene3dhelper.cxx:215-270`, and `PptxSlideLayout.cs:1741` reading only `@rot`.
- The 35 pt and 15 pt `a:ln` widths, both sides' stroke paths and widths from the raw content
  streams.
- The circle-path gradient markup, the reference's concentric-circle banding, our diagonal ramp,
  and `PptxSlideLayout.cs:1411-1418`.
- `slideMaster3.xml`'s `userDrawn` text box, its `sz`/`algn`/`gradFill`/`alpha`; `/CA 0.1 /ca 0.1`
  in `EGS823`; every `Td` of every 82 pt run in both PDFs; `PptxTextBody.cs:744`;
  `DrawingColourTransforms.cs:142-144`; `textcharacterproperties.cxx:115-156`.
- All census counts, from the scripts in this directory.

**Inferred, and flagged as such:**

- That the reference grows a rectangular `p:pic`'s outline rect by half the line width before
  stroking it. Measured three times, never read, and one counter-case exists (a rounded frame with
  no growth). Confirm before implementing.
- That LibreOffice 24.2.7.2 drew a corner-focus circle gradient as a diagonal ramp and 26.2.4.2
  changed. The 26.2.4.2 behaviour is measured; the change is inference from the code comment.
- Why the reference wraps `"10"` in a box that arithmetic says it fits.
- That the round-21 fix, rather than anything since, is what closed the arrow item. The evidence is
  that round's own before/after numbers plus the current renders; I did not bisect.

## 7. What did not move, as predicted

No item here can move a verdict. Slides check 1 is the slide count — `Fundamentals` 26/26,
`W3` 20/20, `OnTrac` 15/15, `Thailand17` 54/54, `Wildlife` 41/41, all exact before and after. Check
2 counts words, and no item adds or removes a run — the `OnTrac` page number already draws its
digits, in the wrong colour and, on 6 pages of 12, the wrong place. Check 3 is embedding, untouched.
**The correct expectation for implementing all of this is 151/163 → 151/163.** It is still worth
doing: between them these items account for the four worst pages in `Wildlife` (60–85% of pixels
each), the second-worst page in `Thailand17`, and 11 of 15 pages in `OnTrac`.

## 8. Suggested order, by measured reach per unit of work

1. **Escher picture crop on the `.ppt` path** — 16 of 51 decks, 100 shapes, 14 decks unmistakable.
   Four property reads plus one existing call. Move `SlideImages.Uncropped`/`Inset` to
   `XRay.Office.Core` in the same change and the word and sheet paths inherit the hard half.
2. **The corner-focus circle gradient** — 1 deck, 4 pages, but the largest per-page divergence in
   the round, and the fix is deleting a special case that already has a correct fall-through.
3. **A run's colour from `a:gradFill`, and its alpha** — 16 decks each. The alpha machinery exists.
4. **`a:scene3d/a:camera/a:rot/@rev` as a z-rotation** — 1 deck, 2 shapes. Cheap, narrow, and it is
   a user-reported defect.
5. **The picture-border growth rule** — 7 decks at a visible width. Confirm the rule first.

## 9. Files

- `prediction.md` — committed before any measurement, unedited since.
- `census.py` — the whole-corpus census: Escher crop over the `.ppt` half, and `p:pic` line widths,
  camera revolutions, run gradient fills and path gradients over the `pptx` half, restricted to
  slide/layout/master parts.
- `ppt-crop-census.py` — per-deck Escher walker: every cropped shape with its slide, anchor and
  crop fractions.
