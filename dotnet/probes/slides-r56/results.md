# slides-r56 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
base `e64f743dbff`, branch `wt-slides-r56`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.
`prediction.md` beside this file was committed as `6f4fff934cf`, before anything was built or
rendered post-change.

## Baseline, and why the briefed `abs_ink` is 4.75 out

| | briefed | measured |
|---|---|---|
| passing over `MANIFEST.tsv` | 200 of 302 | **200 of 302, 0 disagreements** |
| `abs_ink` | 1136.53 | **1131.78** |
| major pages | 395 | **390** |

`ink.tsv` is byte-identical to round 55's final sweep on **306 of 311 rows**. The five that differ
all improved and one changed verdict — which is the parent's own 199 → 200: round 55 *sheets*'
`mc:AlternateContent` change arriving through the merge. `WiGr_2021W_1_Angebot-Nachfrage…pptx`
8.12 → 5.08 `words` → `match`, `Structural Testing` 6.56 → 5.37, `RPA P4` 5.68 → 5.20,
`dhs-293364` 2.51 → 2.47, `Ensemble…AIRBUS` unchanged to the hundredth. The briefed 1136.53 is
the **pre-merge** figure. The baseline reproduces.

`tf-agreement`'s briefed 0.77061 is not what the script prints at this commit — it prints a
per-document mean of **0.85188** over 168 documents and 1678 of 4184 exact pages. Used here as a
control on its own reading rather than reconciled.

## The whole round

| | base | after §1+§2 | **final** |
|---|---:|---:|---:|
| passing over `MANIFEST.tsv` | **200 of 302** | 200 | **199 of 302** |
| page counts changed | | 0 | **0 of 302** |
| `abs_ink` | 1131.78 | 1120.44 | **1106.97 (−24.81)** |
| signed ink | 827.54 | 816.40 | 802.45 |
| major pages | 390 | 390 | **385** |
| **differing pixels over 4530 pages** | 19735.47 | 19752.37 | **19702.17 (−33.30)** |
| turned blocks (reference 1307) | 1374 | | 1381 |
| pages the reference turns and we do not | 7 | | **2** |
| sheared glyphs (reference 16008) | 16740 | | **15792** |
| `tf-agreement` | 0.85188 | | 0.85173 |
| exact `/Tf` pages | 1678 of 4184 | | 1678 of 4184 |

**Eight documents moved. Named, and the regression is not netted.**

| Δ ink | document | before → after |
|---:|---|---|
| **−13.25** | `Demick_JetBlue.pptx` | 26.10 → **12.85**, major 6 → 5 |
| **−12.50** | `2014BSA_Sunday_Killion.pptx` | 19.39 → **6.89**, major **6 → 1** |
| −0.83 | `010605Vul.ppt` | 1.90 → 1.07 — **and this is the round's one verdict regression** |
| −0.18 | `Thailand17.ppt` | 17.87 → 17.69 |
| −0.14 | `Intersil_Italy_CAN_Bus…pptx` | 17.33 → 17.19 |
| −0.03 | `hofman.ppt` | 0.78 → 0.75 |
| +0.01 | `concepts-surrounding-cloud-computing…ppt` | 8.37 → 8.38 |
| **+2.11** | `N2_E_Maestroni_Swarm_COP.pptx` | 1.82 → **3.93**, major 0 → 1 |

**And the gate's own column says two documents became exact.** `introduction_to_bea_tuxedo.ppt`
goes 1785 extractable words to the reference's 1767 → **1767/1767**, and
`2014BSA_Sunday_Killion.pptx` goes 3619/3559 → **3559/3559**. Neither was a passing/failing
boundary; they are simply right now.

### The one verdict regression, and what it costs

`010605Vul.ppt` **`match` → `words`**. Its extractable words go 960 → 963 against the reference's
944, and the gate's band is 2%: 944 × 1.02 = **962.88**. It crossed by **0.12 of a word**.

Everything else about that page moved the right way: its unsigned ink 1.90 → **1.07**, and its
embedded-font count went **7/6 → 6/6** — our font list is now the reference's exactly, which is
the whole point of the change that caused it. Reported as a regression because it is one by the
gate's rule, and named rather than netted against the −25.6 of ink the same change bought.

## 1. `.ppt` `txflTextFlow` — the brief's item 1, and the answer is *not* the DrawingML one

Escher property 136, unread. Six values, three answers, and the fixture was built to
**discriminate**: `patch-textflow.py` rewrites the four value bytes of this one property in a real
corpus `.ppt` that already carries it and leaves every other byte alone, so no arm states the
reference's own default and the six differ in nothing else. On
`concepts-surrounding-cloud-computing…ppt` page 11, 26.2.4.2 draws:

```
0 HorzN  upright        1 TtoBA  0 -1 1 0      2 BtoT  0 1 -1 0
4 HorzA  upright        3 TtoBN  0 -1 1 0      5 VertN 0 -1 1 0
```

24 blocks apiece, identical pens across 1/3/5. **6 of 6** for *"1, 3 and 5 are vertical; 2 is the
other quarter"*; refutes *"any non-zero turns the same way"* (arm 2 is the opposite quarter) and
*"only `TtoBA`"* (arms 3 and 5).

### The half a reused answer got wrong, and a fixture caught it

Round 55's rule — *cite the C++ for intent, measure `soffice` for truth* — earned its place twice
in one round. `oox`'s `TextBodyProperties::pushTextDistances` shifts the four insets cyclically
before they become `SdrText*DistItem`s; `svdfppt.cxx:857-880` hands `dxTextLeft`, `dyTextTop`,
`dxTextRight` and `dyTextBottom` straight to `makeSdrTextLeftDistItem` and its three siblings with
**no shift at all**. Reusing round 55's rotation put every turned `.ppt` label **3.4 pt out on both
axes** — exactly the difference between this format's 0.25 cm across and 0.13 cm down.

`make-ppt-inset-probe.py` settles it by **isolating one slot at a time**: five files, all four
insets zero and then 40 pt on exactly one edge, each differenced against the zero arm — which
cancels the first line's ascent instead of having to know it.

```
                 TtoBA  (0 -1 1 0)        BtoT  (0 1 -1 0)
  lIns 40 pt     dx   0.00  dy -40.00     dx   0.00  dy +39.99
  tIns 40 pt     dx -39.99  dy   0.00     dx +40.00  dy   0.00
  rIns 40 pt     dx   0.00  dy   0.00     dx   0.00  dy   0.00
  bIns 40 pt     dx   0.00  dy   0.00     dx   0.00  dy   0.00
```

The identity, in both directions. **After the correction, 18 of 18 pen origins land on the
reference's to 0.05 pt** — three anchors × two directions × three boxes.

**And the two arms are not one construction.** A vertical flow transposes the frame; `BtoT` adds
90° to the text object's angle and leaves it horizontal, so its lines break at the shape's *width*
(`svdfppt.cxx:819-821`, `:1174-1188`). No corpus `.ppt` uses it — **all 33 non-zero flows are
`TtoBA`** — and it is implemented anyway because getting it wrong silently is worse than not
having it.

**Reach, measured rather than extrapolated.** Five documents carry a non-zero flow; four moved.
The fifth, `ws_prod…Approval-of-Flight-Conditions.ppt`, carries **22 of the 33** and did not move
at all — its shapes state no `TextId`. A census of the property over-counts reach by 50:1 in the
other direction too: `concepts-surrounding-cloud-computing…ppt` states the property on **106**
shapes and 104 of them state `mso_txflHorzN`.

Pages the reference turns and we do not: **7 → 2**.

## 2. The brief's item 2 is refuted, and what page 4 was actually missing is a gridline mesh

The brief: *`Demick_JetBlue.pptx`'s 76 turned blocks are 68 at 45° + 8 at 90°; the sixty-eight are
a chart category axis the reference does not draw at all.*

**The reference draws all 21 of them, at the same 45°, as glyph outlines.** Page 4's reference
content stream is **1.9 MB with 1502 curve operators**, and 200 of its 430 subpath moves fall in
the 40 pt band under the axis where the labels are. Ours has 124 subpaths in total. That is the
case `ChartAxisLabels`' own remarks already record for `tdf106217.pptx`: an unequally scaled chart
plus a 45° turn is a shear the PDF text state cannot carry, so LibreOffice writes outlines and
`pdftotext` reads nothing. So *52 `BT` to 31* and *163 words to 79* are a **representation**
difference, and the block-counting turn census makes the same granularity mistake round 55 nearly
shipped.

**What page 4 actually differs by is `c:minorGridlines`, and that is the `cmp` report's own
words**: *"a solid area drawn differently (31.18% of page, x 0.10–0.87, y 0.28–0.74)"* — the plot
area. The reference draws 28 horizontal and 21 vertical minor lines over it; we drew none. The
element is read nowhere in `Core/Charts`.

Two page reviewers, given the composed pair and nothing else, **both** said our render had no
data-point markers and no vertical gridlines. Both were wrong: a 200 dpi crop of the same PDF
shows squares, diamonds and triangles at every vertex and a full major grid. **A composed pair
downscaled to fit cannot be trusted below about ten points**, and two independent observers made
the identical error, so this is not one reviewer's slip. The finding they *did* deliver — the
plot-area geometry and the mesh — is the one that mattered.

### The sub-interval count is not chart2's default, and the reference's own page proves it

`ScaleAutomatism`'s default is 2. `AxisConverter::convertFromModel`
(`oox/source/drawingml/chart/axisconverter.cxx:389-409`) overrides it for a value axis:
`round(majorUnit / minorUnit)` when both are stated, **5** when `c:minorUnit` is absent — its own
comment is `tdf#114168 … as MS Excel do` — and 9 for a logarithmic axis stating one. Its
`CATEGORY` branch sets nothing, so 2 stands there. Measured on the reference's page 4: 8 major
gridlines 25.97 pt apart and 28 minor ones 5.19 pt apart, **25.97 / 5.19 = 5.00**, and one
category minor per interval at the midpoint.

### A minor gridline states its width and its dash, and both are visible

Reading only the colour drew `N2_E_Maestroni_Swarm_COP.pptx`'s 110 minor lines solid and hairline
where the reference draws them dashed at half a point — **0.66 of that document's unsigned ink for
two attributes**. `ChartLine` already carried a width and a dash; only the reader was missing.

### It costs differing pixels on two documents, and that is a *different* defect made visible

`Demick_JetBlue.pptx` ink **26.10 → 12.85** and major 6 → 5, but its differing pixels rise
114.22 → 124.33; `N2_E_Maestroni` ink 1.82 → 3.93 and differing pixels 140.81 → 149.74. **Both
have the same cause and it is not the mesh.** Our plot rectangles are displaced from the
reference's — JetBlue's plot floor sits 5.5 pt low against a top edge that agrees to 1 pt, and
N2's whole plot is 15.6 pt right of the reference's — so a mesh that is right in count, pitch,
dash and width lands beside the reference's and is counted twice. The two instruments disagree in
sign and both are telling the truth: **coverage improved and placement did not**. The plot
rectangle is the next round's.

**The gridline colour is a second, older gap, measured and left open.** `ObjectFormatter`'s
automatic table (`objectformatter.cxx:223-235`) paints a major gridline as the theme's `tx1` at
`tint 75000` and a minor one at `tint 50000`; on this page 26.2.4.2 draws `0x666666` and
`0x8B8B8B` where this reader draws `0xB3B3B3` for **both** — which it was already doing for the
major grid before this round. The minor grid deliberately takes the same default rather than a
guessed lighter one.

## 3. The font-resolution divergence is an EMF reader bug, it is one line, and it is the largest thing in the round

The brief: *`2014BSA_Sunday_Killion` shears 948 glyphs where the reference shears none — same face
lists, per-run divergence.* The face lists are **not** the same: we embed
`DejaVuSans` roman and the reference embeds none. Following that:

* all 469 uses of that face are the text of an **EMF chart**, `ppt/media/image10.emf`;
* every one of that metafile's 28 `EMR_EXTCREATEFONTINDIRECTW` records names **`Times New Roman`**;
* the 64-byte `lfFaceName` field reads
  `54 00 69 00 … 6e 00 00 00 7f 13 65 43 18 ee a8 08 …` — the name, its terminator, and then
  **twelve code units of stack rubbish**;
* `EmfReader.CreateFont` read the field as *32 code units with the NULs skipped*, so it asked for
  `"Times New Roman፿䍥…"`, which no substitution table recognises and which fell through
  to the generic sans;
* and `WmfReader.CreateFont` has always read the same structure correctly
  (`name.IndexOf((byte)0)`). **Two readers of one field disagreed, and the wrong one was the one
  with the corpus behind it.**

`if (c == 0) continue;` → `if (c == 0) break;`.

| `2014BSA_Sunday_Killion.pptx` | before | after |
|---|---:|---:|
| `abs_ink` | 19.39 | **6.89** |
| differing pixels | 158.26 | **113.12** |
| major pages | 6 | **1** |
| sheared glyphs (reference 0) | 948 | **0** |
| extractable words (reference 3559) | 3619 | **3559** |
| embedded font list | one face more than the reference | **the reference's, exactly** |

**Why it was invisible for thirty rounds**: a wrong face and a right face both drew upright.
Round 55 taught this stack to synthesise an oblique and the wrong face, having no italic, began to
lean where the reference does not — which is the 948, and which is what led here. The brief's item
3 was a real signal read one level too high.

Reach over the whole corpus, `emf-facename-census.py`: **1879 records in 19 documents** — 1440 in
13 slides decks, 367 in 4 words documents, 72 in 2 workbooks — naming Times New Roman, Arial,
Calibri, Calibri Light, System and Verdana.

## Measured against the prediction

| # | predicted | measured |
|---|---|---|
| 1 | verdicts **0**, band −2…+1 | **−1**, 200 → 199. Inside the band, on its risk arm, and for a change that did not exist when the prediction was written |
| 2 | page counts 0 of 302 | **0 of 302** |
| 3 | `.ppt` flow: **turned blocks and differing pixels**, not ink; 1–3 documents move on ink by >0.005 | **3** — `Thailand17`, `hofman`, `concepts`. Pages the reference turns and we do not **7 → 2**. `introduction_to_bea_tuxedo` moved on *words* (1785 → 1767, the reference's figure exactly) and not on ink, which is the column the prediction named |
| 4 | minor grid: Demick 26.10 → **below 20** ✓ (12.85); major 6 → **≤4** ✗ (5); the other two < 0.5 each ✗ (**N2 +2.11**); `abs_ink` −5…−10 ✗ (**−11.34**, past the band on the good side) | two of four |
| 5 | controls: `tf-agreement` ±0.0005 and sheared glyphs **16740 exactly** | `tf-agreement` **0.85188 → 0.85173**, exact-`/Tf` pages 1678 unmoved ✓. Sheared glyphs **16740 → 15792** ✗ — and **the control did its job**: the whole −948 is the EMF fix, a change made after the prediction, and the column named it exactly |
| 6 | documents moved on ink **4–8** | **8** — inside the band for the first time in four rounds, **and the composition is wrong**: the prediction named eight documents, five of them moved, and three more (`2014BSA`, `Intersil`, `010605Vul`) came from a change that did not exist yet. A count landing in its band is not the same as a census being right, and this one is not |
| 7 | cross-track: gridlines reach 4 sheets + 1 words document, 0 verdict movement | **measured, not argued** — see below |

**The documents-moved quantity has now hit its band once in four rounds and the honest reading is
that it did not.** Rounds 53–55 missed by over-extrapolating, by censusing symptoms and by
censusing the right documents in the wrong column. This round's census was right about the five
documents it could see and blind to the three it could not, because the third change was found by
following a *measurement* rather than the brief — and the band absorbed the difference.

## Refutations

1. **The brief's item 2**, on two independent measurements. `Demick_JetBlue` page 4's rotated
   category axis is drawn by the reference, at our angle, as **glyph outlines** — 1502 curve
   operators and 200 subpath moves in the label band, against 124 subpaths on our whole page. The
   `52 BT`/`163 words` figures are a representation difference. The page's real defect is
   `c:minorGridlines`, worth **13.25 of unsigned ink** on that document alone.
2. **The brief's item 3**, one level down. Not a per-run font-resolution divergence in the slide
   text: an EMF reader that treated a NUL-terminated fixed field as NUL-padded. One line, 19
   documents, all three tracks.
3. **Reusing round 55's inset rotation on the `.ppt` path**, refuted by my own probe before it
   shipped. `oox` shifts the four insets cyclically and `svdfppt` does not; the identity is right
   here and the shift was wrong by 3.4 pt on both axes.
4. **The composed page pair as an instrument below ten points.** Two independent reviewers, given
   only the image, both reported that our render had no data-point markers and no vertical
   gridlines. A 200 dpi crop of the same PDF shows both. The pair is a good instrument for
   *structure* and a bad one for *small marks*, and this round nearly took "we draw no markers" as
   a finding.

## The 24.2.7.2 audit

`Layout/SlideDrawing.cs` :341 and :360 — `FillReachesThePage`. **VERIFIED on 26.2.4.2, both
halves.** Open sites held at **40**; marked 15 → **17** (14 verified, 3 wrong). Re-derived with
the file's own commands, not quoted.

The site makes two claims and they need different fixtures.

* *A package entry loses the frame's fill*: `2014BSA_Sunday_Killion.pptx` rendered as found, with
  the frame's `a:solidFill` changed to red, and with it replaced by `a:noFill` gives three
  **byte-identical** page-5 images.
* *An inline metafile keeps it*: **this is the half a single rendering cannot settle**, because one
  file's answer is consistent with "never drawn" and with "always drawn". A **discriminating
  pair** — one authored flat ODP holding a 306 kB EMF as `office:binary-data` under a red frame,
  and the reference's own `--convert-to odp` of that same file, which moves the identical bytes to
  `Pictures/` and changes nothing else — draws **108 304 red pixels** and **none**.

`probes/slides-r56/audit_picturefill.py`. That is the second re-check in three to turn on a
discriminating pair, and in both cases the naive single-fixture reading was available and wrong.

## Tests

Three new files, **15 new tests**, and the total reconciles: **4824 = 4809 + 15**.

| test | mutation | outcome |
|---|---|---|
| `PptTextFlowTests` (4) | `1 or 3 or 5 => Clockwise, 2 => Anticlockwise` → `None` | **DETECTED**, 2 of 4 |
| `PptTextFlowTests` (4) | `DocRect area = transpose ? …` → `false ? …` | **NOT DETECTED** — see below |
| `DrawingChartMinorGridTests` (8) | `ValueMinorGrid = MinorGridOf(…)` → `null` | **DETECTED**, 3 of 8 |
| `DrawingChartMinorGridTests` (8) | `return minor is null ? 5 : 2;` → `return 2;` | **DETECTED**, 2 of 8 |
| `EmfFaceNameTests` (3) | `if (c == 0) break;` → `continue;` | **DETECTED**, 2 of 3 |

**One mutation came back clean and it is reported rather than papered over.** Collapsing the
`BtoT` arm onto the transposing one breaks no test: `PptTextFlowTests` asserts the *matrix* and
the boxes' ordering, and this fixture's text is two glyphs, so no line breaks either way and the
width the frame breaks at never shows. The arm has **zero corpus reach** and its numbers are in
the probe, not in a test. Stated plainly: the second arm of §1 is validated by measurement and is
a drift guard at best in the suite.

Each test class's inert cases are controls by design — the horizontal box that says the turn comes
from the property, the axis that states only a major grid, and the zeroed face-name field.

Ten non-Fidelity projects, one at a time: Core 337, Containers 109, Text 617, Vector 298,
Rendering 153 (+1 skipped, the same `PdfFontTests` case as at baseline), Markup 259,
OpenDocument 125, WordProcessing 1155, Spreadsheets 940, **Presentations 831** — **4824 passed,
0 failed, 1 skipped**. `cd dotnet && dotnet build -v q -nologo` → **0 warnings, 0 errors**.

## Shared layers — measured, not argued

Two of the three changes reach outside this track and the parent must gate the corpus.

**`c:minorGridlines` touches `XRay.Office.Core/Charts` and `XRay.Office.Ooxml`.** Census reach: 4
sheets documents and 1 words document, all named, all rendered before and after:

| document | ink before → after | differing pixels | major |
|---|---|---|---|
| `033_Event_planning_tracker…xlsx` | 12.68 → 12.68 | 19.72 → 19.70 | 1 → 1 |
| `035_Project_plan_for_law_firms…xlsx` | 3.07 → 3.07 | 22.85 → 22.85 | 0 → 0 |
| `038_Baby_growth_chart…xlsx` | 0.68 → 0.72 | 5.41 → 5.41 | 0 → 0 |
| `039_Baby_growth_tracker…xlsx` | 0.78 → 0.92 | 3.56 → 3.56 | 1 → 1 |
| `ABCD-FE-01-00 Flight Envelope…docx` | 0.00 → 0.00 | 0.00 → 0.00 | 0 → 0 |

**`EmfReader` touches `XRay.Office.Vector`.** Census reach outside slides: 4 words documents and 2
workbooks, all named, all rendered before and after, **all six better or level on both
instruments and none crossing a word-count band**:

| document | ink before → after | differing pixels | words vs reference |
|---|---|---|---|
| `bulletin.docx` | 4.37 → **4.14** | 37.51 → **36.61** | 0.37% → **0.00%** (3253 exactly) |
| `docs-quality-MA.IMS.00001…docx` | 24.42 → 24.25 | 510.97 → 510.33 | 0.16% → 0.16% |
| `UG.CAO.00006 Foreign Part 145…docx` | 11.27 → 11.17 | 382.84 → 382.73 | 3.87% → 3.87% (already failing) |
| `EHEST-SMS-Safety-Management-Manual-V2.docx` | 0.00 → 0.00 | 0.00 → 0.00 | 0.04% → 0.07% |
| `012_Contextures_chart_sample…xlsx` | 0.54 → 0.44 | 4.37 → 3.96 | 0.00% → 0.00% |
| `013_Contextures_chart_sample…xlsx` | 0.64 → 0.61 | 5.11 → 4.72 | 0.00% → 0.00% |

No page count moves on any of the eleven. Predicted cross-track verdict movement **0**, and
nothing measured here contradicts it — but the parent's gate is the authority, and the words and
sheets tracks should be swept whole.

## Left open, in the order the next round should take them

1. **The chart plot rectangle.** Both of this round's ink regressions are one defect: our plot
   area is displaced from the reference's — `Demick_JetBlue`'s floor 5.5 pt low against a top edge
   that agrees to 1 pt, `N2_E_Maestroni`'s whole plot 15.6 pt right — so correct gridlines land
   beside the reference's. It is now the thing standing between `Demick_JetBlue` at 12.85 and a
   much smaller number, and the mesh has made it measurable for the first time.
2. **The gridline colour, which is the OOXML automatic-format layer for gridlines.**
   `objectformatter.cxx:223-235` says major = theme `tx1` at `tint 75000` and minor at `tint
   50000`; 26.2.4.2 draws `0x666666` and `0x8B8B8B` and we draw `0xB3B3B3` for both. This predates
   the round and reaches every OOXML chart with a grid, not the eight this round's census names.
3. **The rest of the EMF face-name reach.** 19 documents carry the defect and 8 were measured
   here; the other 11 are 12 slides decks' worth of funnel diagrams and were not individually
   checked. And **`WmfReader` should be read for the mirror-image mistake**: it reads the face name
   correctly, but nothing has checked its *other* fixed fields for the same padded-versus-
   terminated confusion.
4. **`010605Vul.ppt`**, `match` → `words` by 0.12 of a word. Its render improved on ink and its
   font list is now exact. Either the gate's 2% band is the wrong instrument for it or there are
   three extractable words we should not be splitting; the charstream test is the first move.
5. **The fitted bullet's vertical placement** — 1.9 pt too high, `ALIGN_BOTTOM` /
   `aBulletArea.Bottom()`, `outliner.cxx:909-919`. Untouched for three rounds now.
6. **The `.ppt` `cdirFont` (Escher 137)**, unread and deliberately so: it *toggles* the vertical
   flag as well as turning, so a reader of `txflTextFlow` that ignores it is wrong on a shape
   stating both. Two documents and six shapes carry one — `outlook_of_nigerian_pension_sector.ppt`
   ×1 at `cdir 1` and `introduction_to_bea_tuxedo.ppt` ×5 at `cdir 2` — and **none of them also
   states a non-zero flow**, so it is recorded rather than guessed at.
7. **`mso_anchorTopCentered` on a vertical `.ppt` body.** The reference centres the block *along*
   the line and we put it at the start; measured on the probe (an inset moves it by half). **Zero
   corpus reach**: all 33 vertical flows carry `anchorText` 0 or 1.
8. **The automatic marker at series index 2 should point down, not up.** `typegroupconverter.cxx`
   names the cycle *square, diamond, arrow down, arrow up* and `ChartMarker` has one triangle;
   the reference's third series on `Demick_JetBlue` page 4 is ▼ and ours is ▲. No ink in it.
9. `2015-Civil-Rights-Website-training.ppt`, 29.64 and still the track's second largest.
10. `wordArtVert`, and the `pitchFamily` family nibble — both unchanged since round 55.
