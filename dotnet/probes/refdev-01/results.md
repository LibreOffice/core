# refdev-01 — Calc's and Impress's reference devices

Round `refdev-01`, 2026-08-15, worktree `wt-refdev`. Reference LibreOffice **26.2.4.2**, Carlito /
Caladea / Liberation / DejaVu / OpenSymbol / IPAGothic / WenQuanYi all resolving. References reused
from `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/`, never re-rendered; `SOURCE_DATE_EPOCH=1700000000`
on every render that is diffed.

Prediction written and committed before any reach or verdict was measured: `prediction.md`,
commit `404a1d46803`.

**Both devices were found and both fit exactly.** Impress is **600 dpi in 1/100 mm** and reproduces
**507 of 507** measured (face, size) pairs, ascent and line height both. Calc is **720 dpi in
1/100 mm** — *not* the 8640 the C++ names — and reproduces **468 of 468** line heights and **663 of
663** ascents. The tree before this round reproduced 82 of 507 and 94 of 468.

Reach: **171 of 171 sheets renderings changed, 152 of 163 slides, 0 of 200 words.** Verdicts:
**0 won and 0 lost, on all three tracks.** No `done-*` document lost its verdict anywhere. Fidelity
is 30 of 550, the briefed baseline, before and after.

That "0 won" is the expected answer and it is stated first rather than buried: the brief said most
of what remains on both tracks is a documented ceiling, and it is.

---

## 1. The three devices

| application | device | map unit | pixels per logical unit | line-height rule |
|---|---|---|---:|---|
| Writer | `RefDevMode::MSO1`, 8640 dpi | twip | 6, exact | `round(a+d)` + `round(g)`, leading in the ascent |
| Impress / Draw | `RefDevMode::Dpi600`, **600 dpi** | 1/100 mm | 0.2362 | `max(round(a)+round(d), round(a+d))`, **no leading** |
| Calc | `RefDevMode::PDF1`, **720 dpi** | 1/100 mm | 0.2835 | the same |

```
H  = round( size_in_logical_units * dpi / units_per_inch )      the em, in whole device pixels
a  = round( ascent  * H / upem )                                whole device pixels, separately
d  = round( descent * H / upem )

ascent      = round( a * units_per_inch / dpi )
lineHeight  = max( ascent + round(d * k), round((a + d) * k) )   where k = units_per_inch / dpi
```

`round` is half **away from zero** throughout, as `lineheight-01` established for Writer.

### Why the line height is a maximum of two roundings

This is the part that could not be guessed. EditEngine measures a line twice and keeps the larger:

| step | citation |
|---|---|
| the text portion's height is `GetTextHeight`, converting the **summed** pixel ascent and descent once | `vcl/source/outdev/text.cxx`:640-651 |
| the formatter metric is `GetFontMetric().GetAscent() + GetDescent()`, each converted **separately** | `vcl/source/outdev/font.cxx`:351-352 |
| `FormatterFontMetric::GetHeight()` is `nMaxAscent + nMaxDescent` | `editeng/source/editeng/impedit.hxx`:184-187 |
| the line keeps whichever is taller | `editeng/source/editeng/impedit3.cxx`:1516-1518 |
| the leading is added only when `IsAddExtLeading()`, which is off outside Writer | `impedit3.cxx`:3133-3135 |

Neither grouping fits alone, and they disagree in *both* directions:

| grouping | Impress, 312 pairs | Calc, 273 pairs |
|---|---:|---:|
| each converted separately | 274 | 238 |
| the sum converted once | 270 | — |
| **the taller of the two** | **312** | **273** |

Liberation Serif on Impress makes the point in four rows: at 6.0 pt the split is 238 and the sum
237; at 10.5 pt the split is 410 and the sum 411. One rule cannot produce both.

### Calc is 720 dpi, and reading the tree gives 8640

`ScDocument::GetVirtualDevice_100th_mm` really is `RefDevMode::MSO1` — 8640 dpi —
(`sc/source/core/data/documen8.cxx`:182-193), and it is **not what draws a printed cell**.
`ScOutputData` formats against the *output* device, which on a PDF export is the PDF writer's own
reference device, `RefDevMode::PDF1` = 720 dpi (`vcl/source/gdi/virdev.cxx`:410).

| candidate for Calc | ascent | line height |
|---|---:|---:|
| 8640 dpi, 1/100 mm | 92/273 | 105/273 |
| 600 dpi, 1/100 mm | 66/273 | 37/273 |
| 96 dpi, 1/100 mm | 20/273 | 20/273 |
| exact scaling (the tree) | 94/273 | 96/273 |
| **720 dpi, 1/100 mm** | **273/273** | **273/273** |

The brief said to measure the installed binary rather than read the 27.2-alpha tree, and this is the
case it was warning about. 720 dpi in 1/100 mm is also the device
`SheetDeviceUnits.ReferenceDpi` already carried for the font-size round trip, fitted independently
by an earlier round on 178 sizes — the two agreeing is a check on both.

## 2. The instruments

`pdftext.py` reads every show operator's baseline at **full stream precision**. `pdf-ops.py dump`
rounds to two decimals, which is 0.7 of a hundredth of a millimetre and cannot settle a one-unit
question on a device whose logical unit *is* the hundredth of a millimetre; LibreOffice writes three
decimals, so the numbers were there all along.

`probe-impress.py` authors one slide per (face, size) holding a six-line text box at an exactly
authored frame top, so the first baseline's distance below it *is* the ascent and the five equal
gaps below it *are* the line height. It asks for `style:font-independent-line-spacing="false"`,
which is ODF's default and the only branch that reads a face metric at all.

`probe-calc.py` authors one printed page per pair: a row carrying a manual page break, top-aligned,
zero padding, fixed height, holding a two-paragraph cell. `PROBE_MODE=plain` measures the
single-line `ScOutputData::LayoutStrings` path instead, which gives the ascent on its own — and it
agrees, 195 of 195, so the two Calc paths are one device.

Both score six candidate devices side by side rather than assuming one, and both take
`XRAY_OFFICE_CLI` to score the tree end to end through the real renderer.

| set | pairs | model | tree, end to end |
|---|---:|---|---|
| Impress core, 5 faces | 195 | 195/195 asc, 195/195 height | **195/195, 195/195** |
| Impress extra, 8 faces | 312 | 312/312, 312/312 | **312/312, 312/312** |
| Calc core, 5 faces | 195 | 195/195, 195/195 | ascent constant-offset (§6), **195/195** height |
| Calc extra, 8 faces | 273 | 273/273, 273/273 | **273/273** height |
| Calc single-line, 5 faces | 195 | 195/195 ascent | — |

The eight `extra` faces are ones no prior round measured — other cuts, other ems, a symbol face and
a CJK face whose `hhea` and Windows metrics differ by 7.6% of the em. **IPAGothic fits 39 of 39 on
both devices**, so the CJK 127% scale `lineheight-01` §7(a) found is Writer's alone
(`MS_WORD_COMP_GRID_METRICS`) and does not exist on the EditEngine path.

## 3. What was changed

Five source files.

| file | change |
|---|---|
| `src/XRay.Office.Text/Fonts/LineSpacing.cs` | a `MetricUnit` enum; `MetricGrid` carries it; `Presentation` and `Spreadsheet` grids; `EditHeightOn`, the `max` rule; `ScaledLineHeight` branches on the engine |
| `src/XRay.Office.Presentations/Layout/SlideTextLayout.cs` | `FaceHeight` and `EmitMarker` resolve on `MetricGrid.Presentation`; a single-spacing rule no longer round-trips the height through whole twips |
| `src/XRay.Office.Spreadsheets/Layout/SheetFonts.cs` | both `SheetFace` constructions resolve on `MetricGrid.Spreadsheet` |
| `src/XRay.Office.Spreadsheets/Layout/SheetBandText.cs` | the furniture face too — and the *chart* heights explicitly drop the grid again |

### The generalisation

`MetricGrid` was `(int Dpi, bool QuantisesAdvances)` and did all its arithmetic in twips. It is now
`(int Dpi, bool QuantisesAdvances, MetricUnit Unit)`, with `ToPixels`, `ToLength`, `ToAdvance` and
`ToEmSize` going through the unit. The twip path is arithmetically identical to what it was, which
is what makes `words` byte-identical.

**A resolution alone does not describe a device**, and this is why two earlier rounds swept 72 to
6000 dpi and found nothing: 8640 dpi in twips rounds essentially nothing and 8640 dpi in 1/100 mm
rounds a great deal. `ApplicationGridTests.TheLogicalUnitIsPartOfTheDeviceAndNotAnAfterthought`
pins it — 10 pt is 200 twips and 353 hundredths of a millimetre, which are 1200 and 1201 pixels at
the same resolution.

### The engine, not the device, decides the grouping

`LineMetrics.LeadingAboveText` already existed and was documented as the Writer/EditEngine
discriminator. It now decides the *grouping* as well as where the leading sits, because those are
the same distinction. That is right — Writer's own drawing objects format through EditEngine on
Writer's device, so the two axes really are independent — but it made an implicit coupling load-
bearing, and four existing tests were relying on `ScaledLineHeight` ignoring the flag. All four
state Writer's numbers and now say so; none is a behaviour change.

### The twip round trip that had to go

`LineSpacingRule.Apply` works in whole twips. An Impress line height is a whole hundredth of a
millimetre, which is 0.567 of a twip, so single spacing — what nearly every paragraph asks for —
rounded the device's own answer off its own grid. With the round trip, 113 of 195 line heights were
exact and the other 82 were out by one unit in both directions; without it, 195 of 195. The guard is
one line and only fires when the rule changes nothing, so a paragraph that states 150% still gets
the twip arithmetic an earlier round measured on `slides-features.odp`.

## 4. Reach, all 534 documents, both directions

Rendered twice with `SOURCE_DATE_EPOCH` set, so the two runs are byte-comparable with nothing
masked. Verdicts against the banked references, `batch-check.sh`'s three checks column for column.

| track | renderings changed | before | after | won | lost | Σ\|page error\| | Σ\|word error\| |
|---|---:|---:|---:|---:|---:|---|---|
| words | **0 of 200** | 171 | 171 | 0 | 0 | 94 → 94 | 4691 → 4691 |
| slides | **152 of 163** | 147 | 147 | 0 | 0 | 0 → 0 | 3880 → 3880 |
| sheets | **171 of 171** | 163 | 163 | 0 | 0 | 43 → 43 | 21033 → **21115** |

**Not one page count moved, on any document, on any track.** On slides not one word count moved
either. On sheets fourteen documents' word counts moved, seven down and seven up, and the net is
+82 — the largest single mover is `State-Medicaid-Payment-Policies…xlsx` at 9 → 135 against a band
of **808**, and the closest any document comes to its band is `RegChangeReport.xlsx` at 10 of 62.
Nothing is near a boundary in either direction.

### The three `done-*` tracks

| track | documents | before | after | renderings changed |
|---|---:|---:|---:|---:|
| `words/done-*` | 159 | 158 | 158 | 0 |
| `slides/done-*` | 144 | 144 | 144 | 133 |
| `sheets/done-*` | 156 | 156 | 156 | 156 |

**No `done-*` document lost its verdict, in any track.** The one standing `words/done-*` mismatch is
`airbus-pdf-information-package_v1-4.docx`, unchanged and unrelated.

### Where the slides reach actually comes from

152 of 163 was a surprise, because the corpus holds **no ODP at all** — 112 pptx and 51 ppt — and a
PPTX body sets `FontIndependentLineSpacing`, the branch that never reads a face metric. So the whole
track was rendered a third time with each of the two call sites reverted on its own:

| call site | documents it reaches |
|---|---:|
| `FaceHeight` — the paragraph path, i.e. chart labels here | **14 of 163** |
| `EmitMarker` — a symbol bullet's own baseline | **150 of 163** |

A character bullet is drawn by EditEngine whichever line-spacing rule the paragraph beside it uses,
so it goes through the device on every deck that has one. That is a real correction and it is worth
almost nothing to the gate, which is the shape of this whole round.

### One page, read blind

`RegChangeReport.xlsx` page 2 was handed to a reviewer that had never seen it and was forbidden to
read the repository. Its reading, unprompted:

> Line spacing is **identical** — the leading is 16–17 px in both, and the shared 26 lines land on
> the same y to the pixel. Nothing wraps differently: every shared line breaks after the same word.

Which is the change, seen from outside. The same reading found something else, on a document that
**passes** the gate: LibreOffice clips a page-spanning tall row to the printed band and we do not,
so about 115 px of the cell's text spills above the row's top border and 190 px below it, and nine
lines that belong on page 3 are drawn on page 2. Page count, word count and font embedding are all
correct, so no gate column can see it. Recorded in §6.

## 5. Tests

88 new tests in three files, plus four existing expectations moved.

`tests/XRay.Office.Text.Tests/ApplicationGridTests.cs` — 75. Both devices asserted directly; 30
Impress and 28 Calc (face, size) rows whose every number is a distance LibreOffice drew; four rows
where the two applications draw *different* lines for the same face and size; eight rows pinning the
`max` rule with both losing candidates stated; the engine-versus-device distinction; and the
logical-unit pin. Design-unit metrics are stated rather than read from the installed files, so the
arithmetic is tested without the tests depending on a font being present.

`tests/XRay.Office.Presentations.Tests/SlideReferenceDeviceTests.cs` — 7, through a laid-out body
rather than a constructed metric. Includes the control that a PPTX body reaches the device *not at
all*, so that wiring the branches the wrong way round fails rather than passing quietly.

`tests/XRay.Office.Spreadsheets.Tests/SheetReferenceDeviceTests.cs` — 6, through a package read from
bytes and drawn to a recording sink, so the assertion is on baselines a page actually carries.

### Verified failing against the unfixed behaviour

Two separate reverts, so each half is proved on its own.

| reverted | result |
|---|---|
| the wiring only (three files back to a grid-less `Resolve`) | Presentations **4 failed**, Spreadsheets **5 failed**, Text 0 failed |
| `EditHeightOn`'s `max` back to the split-only rule | Text **13 failed**, Presentations 1, Spreadsheets 1 |

Text passing under the first revert is by design: `ApplicationGridTests` names the grid explicitly
and tests arithmetic, so only the second revert can move it.

One test was rewritten because of what the first revert showed. `SlideReferenceDeviceTests`'
exact-scaling case used 18 pt, where exact scaling gives 710 and the old whole-twip round trip turns
that into the correct 711 — the right answer for the wrong reason. It uses 10 pt now, where the two
differ. **A case a broken tree passes by accident is not a test**, and only running the revert
found it.

### Counts, every project run individually

| project | passed | failed | skipped | baseline |
|---|---:|---:|---:|---|
| Core | 337 | 0 | 0 | 337 |
| Containers | 109 | 0 | 0 | 109 |
| Text | **487** | 0 | 0 | 412 + 75 |
| Vector | 295 | 0 | 0 | 295 |
| Rendering | 150 | 0 | 1 | 150 |
| Markup | 259 | 0 | 0 | 259 |
| OpenDocument | 125 | 0 | 0 | 125 |
| WordProcessing | 896 | 0 | 0 | 896 |
| Spreadsheets | **853** | 0 | 0 | 847 + 6 |
| Presentations | **717** | 0 | 0 | 710 + 7 |
| **Fidelity** | **520** | **30** | **0** | **30 of 550** |
| total | 4748 | 30 | 1 | |

Fidelity is 30 of 550 — measured on the unmodified tree before anything was changed and again at
the end, the same 30. Build is 0 warnings, 0 errors.

**One flaky run, and it is the documented one.** `XRay.Office.Vector.Tests` reported 1 failed of 295
on the whole-suite pass and 0 failed on three subsequent runs alone, with no failing name captured.
Nothing in this round touches Vector.

### The mtime trap, guarded

The tree was built seven times across three revert/restore cycles. Every restore is `cp` followed by
`touch`, with `rm -rf src/<project>/{obj,bin}` before the rebuild — and after each restore a subset
of the corpus was re-rendered and compared **byte for byte** against the run being claimed: 60 of 60
on `slides/done-00[1-3]` and `sheets/done-00[1-3]` after the first two cycles, 50 of 50 on
`slides/done-00[1-5]` after the last.

## 6. Found and deliberately not done

### (a) A page-spanning row is not clipped to its printed band

`RegChangeReport.xlsx` page 2, §4. LibreOffice clips a tall row's text to the band the page shows —
shearing a glyph mid-height at the boundary — and we paint the whole block, so it overflows the row
border at both ends and the tail that belongs on the next page is drawn on this one. Line origin,
metrics, wrapping and borders are all correct; only the clip is missing. The document **passes the
gate** on all three checks. Deciding between "the clip is missing" and "the block is repainted on
every page strip" needs pages 1 and 3, which one image cannot give.

### (b) A top-aligned Calc cell's first baseline is a constant 35 units low

Our renderer puts the first baseline of a top-aligned, zero-padded cell exactly **35 hundredths of a
millimetre** (0.99 pt) below where LibreOffice puts it — on all 195 pairs, every face and every size,
which is what makes it a placement constant rather than a metric. The line *pitch* is exact, so it
does not accumulate. It was invisible before this round because the pitch was wrong too. Calc's
default vertical alignment is bottom, so this probe exercises a path a real workbook reaches less
often than it looks; it wants its own before/after rather than being bolted on here.

### (c) `OpenSymbol` does not produce two baselines in the Calc probe

39 of the 312 `extra` pairs — one whole face — were unmeasured on the Calc probe and measured
cleanly on the Impress one. An instrument artefact, not a rule failure; the same face fits 39 of 39
on Impress.

### (d) Writer's drawing objects still scale exactly

Writer's own text boxes and shapes format through EditEngine on Writer's 8640 dpi device, so they
want `new MetricGrid(8640, false, MetricUnit.Twip)` with the `max` rule and no leading. Nothing in
this round gives them one, because nothing measured them. The type now expresses it.

## 7. Predictions, scored

Nine right, three wrong. Two of the three wrong ones are the same mistake and it is worth naming:
**I reasoned about the paragraph path and forgot the bullet.**

| | claim | conf | outcome |
|---|---|---:|---|
| P1 | fewer than 30 of 163 slides renderings change, because the corpus holds no ODP | 80% | **wrong** — 152. The corpus really holds no ODP and `FaceHeight` really reaches only 14; `EmitMarker` reaches 150 and I did not think of it |
| P2 | more than 120 of 171 sheets renderings change | 75% | **right** — 171 of 171 |
| P3 | words byte-identical, 0 of 200 | 85% | **right** — 0 |
| P4 | net verdicts won ≥ lost | 65% | **right**, trivially — 0 and 0 |
| P5 | no `done-*` document loses its verdict, in any track | 55% | **right** — none, in any of the three |
| P6 | 3 or fewer verdicts won across both tracks | 70% | **right** — 0 |
| P7 | Fidelity no worse than 30 of 550 | 50% | **right** — exactly 30, twice |
| P8 | `MetricGrid` needs a logical unit *and* a second line-height rule | 80% | **right** — and the second rule turned out to be a maximum of two, which I had not guessed |
| P9 | at least three existing slides or sheets tests need their expectations moved | 75% | **wrong** — none did. Four tests moved and all four are Writer's, in two other projects, for a different reason: they built a gridded metric without saying which engine it belonged to |
| P10 | the 39 unmeasured Calc pairs are one whole face | 70% | **right** — all 39 are OpenSymbol |
| P11 | Calc's row heights are untouched | 85% | **right** — not one page count moved on any of the 171 |
| P12 | any slides verdict that moves will be a chart deck | 60% | **wrong** in premise — no slides verdict moved at all, and the renderings that moved are bullets rather than charts |

## 8. Contradicting the brief

- **"Calc is 8640 dpi in 1/100 mm."** It is 720. `lineheight-01` §7(b) read
  `ScDocument::GetVirtualDevice_100th_mm` and that device is real and is not what draws a printed
  cell. 8640 dpi in 1/100 mm scores 105 of 273 line heights against 720 dpi's 273.
- **"Impress … the quantisation is worth up to 28 times what it is in Writer."** Right in principle
  and it does not land where the sentence implies. A device pixel is 2.4 twips against Writer's
  sixth of one, so the *bound* is 14 times per rounding — but the corpus's decks are PPTX and PPT,
  which never read a face metric for their paragraphs at all, so on slides the whole effect arrives
  through symbol bullets and chart labels.
- **"So the same class of defect is very likely open on slides and sheets, and on Impress it should
  be much larger per line than the Writer case."** Per line, yes: Writer's was one twip and
  Impress's is up to two hundredths of a millimetre out of a line-height rounding four times
  coarser. In renderings moved it is comparable — 152 and 171 against Writer's 159. In *verdicts* it
  is nothing, which the brief predicted.
- **"`MetricGrid` would need a logical unit as well as a resolution."** True, and not sufficient.
  It also needed the EditEngine line-height rule, which is neither of the two groupings anyone had
  tried but the maximum of them, and it needed a twip round trip removed from the slides path that
  had been compensating for the old arithmetic.
- **"most of what remains on both is documented ceilings … so this may well be a large fidelity
  improvement that moves few or no gate verdicts."** Exactly right, and the outcome is the "no" end
  of it: 323 renderings changed and no verdict moved in either direction.

## Files

```
src/XRay.Office.Text/Fonts/LineSpacing.cs                    MetricUnit, the two grids, the max rule
src/XRay.Office.Presentations/Layout/SlideTextLayout.cs      Impress's device, and the twip round trip
src/XRay.Office.Spreadsheets/Layout/SheetFonts.cs            Calc's device on every cell face
src/XRay.Office.Spreadsheets/Layout/SheetBandText.cs         and on the furniture; charts keep none
tests/XRay.Office.Text.Tests/ApplicationGridTests.cs                    75
tests/XRay.Office.Presentations.Tests/SlideReferenceDeviceTests.cs       7
tests/XRay.Office.Spreadsheets.Tests/SheetReferenceDeviceTests.cs        6
tests/XRay.Office.Text.Tests/{ReferenceGridTests,MetricGridTests}.cs    Writer's faces say so
tests/XRay.Office.WordProcessing.Tests/{HighlightTests,ListLabelTests}.cs  two harnesses, likewise
probes/refdev-01/pdftext.py                                full-precision baselines out of a PDF
probes/refdev-01/probe_faces.py                            the shared face and size table
probes/refdev-01/probe-impress.py                          507 pairs, six candidate devices
probes/refdev-01/probe-calc.py                             468 pairs, two Calc paths
probes/refdev-01/{impress,calc}-*.txt                      the measured tables as run
```
