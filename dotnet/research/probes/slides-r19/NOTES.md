# Slides, round nineteen — working notes

## The base

The worktree opened **481 commits behind** `claude/paperless-odf-phase-1-rnyzcu` — the ninth
agent in a row on a stale base, and the largest gap yet recorded. Fast-forwarded to `09a35cdae`
before measuring anything.

## The baseline reproduced round eighteen's four headline figures to the digit

163 documents, one row each, no path twice, 0 `ref-failed`, against a checksummed CLI snapshot.

| | round eighteen reported | measured here |
|---|---|---|
| word gate | 151 / 163 | 151 / 163 |
| signed `ink%` | 1360.08 | 1360.08 |
| unsigned `\|ink\|%` | 1680.10 | 1680.10 |
| major pages | 438 | 438 |

So the harness and the handover agree, and nothing in this round's numbers rests on a
disputed starting point.

## What was taken, and why

The brief's ordered list was (1) the 325-page size residue and its one-point autofit class,
(2) quantise the shape's text area, (3) `8_P-Pavese` page 16's axis slot width, (4) the raster
list's threshold.

**Item 2 was taken, and item 1's framing is worth correcting.** Round seventeen pinned
`2015-Civil-Rights-Website-training.ppt` page 21's box height to a 16 mm100 window and called
the residue "a height measurement worth about a third of a point over nineteen lines". That
reads the *window's width* as the size of the error, and it is not: the window is where the
true box lies, and our error only has to cross its nearer edge. A quantity that is one or two
units out is exactly the size of thing a 16-unit window catches. That is the argument for
doing item 2 before concluding anything about item 1 — which is also what the brief says, for
a different reason.

A third fix was found while looking at the ink ranking, and it is much larger than either.

## The text rectangle was never on the draw layer's grid

`SlideTextLayout.Place` laid text out against the rectangle the reader produced, at the file's
full EMU precision. The reference cannot: oox builds a shape's matrix in EMUs and scales it
into hundredths of a millimetre at the end (`oox/source/drawingml/shape.cxx`:1226-1230, tag
`libreoffice-24.2.7.2`), `SvxShape` hands the result to `SdrObject::SetSnapRect`, and a
`tools::Rectangle` holds four `sal_Int32` of the model's map unit. The four text distances are
`SdrMetricItem`s of the same unit.

**Both edges round, and the extent does not.** `round(bottom) − round(top)` is not
`round(bottom − top)`, so a box of one extent is a whole unit taller or shorter depending on
where on the slide it sits. An implementation that rounded the width and height would agree
with neither the reference nor the old behaviour.

### The fixture, and what it separates

`tests/corpus/features/slide-text-area-grid.pptx`: three boxes with `lIns`/`tIns`/`rIns`/`bIns`
of zero and one 18 pt run each, whose left and top edges are 2540.497, 5080.503 and 8890.497
units. Two directions of rounding on two axes, so a constant shift cannot pass it.

| | reference | before | after |
|---|---|---|---|
| box one, pen across | 72.000 | 72.0141 | **72.000** |
| box two, pen across | 144.028 | 144.0143 | **144.0283** |
| box three, baseline below box one | 216.000 | 216.0141 | **216.000** |

The baselines are compared as a *difference* because LibreOffice's PDF export puts its whole
page one unit — 0.028 pt — above ours: its background rectangle starts at `y = 0.028` on a
540 pt page and every baseline carries the same offset. That is a uniform offset of the whole
content and not an ascent difference, which is worth writing down because it looks exactly
like one on a single measurement.

## A blip's `a:lum` is a picture recolour, and washout is not the pair it states

Found by ranking the baseline by `|ink|%` and looking at the second document rather than the
first. `N2_E_Maestroni_Swarm_COP.pptx` carries **67.34 of unsigned ink over thirty pages with one
major page**, and page 1 alone is **63.62** — the reference washes its full-bleed satellite photograph almost
white and we drew it at full strength.

The slide states `<a:blip r:embed="rId3"><a:lum bright="70000" contrast="-70000"/></a:blip>`.

### The mechanism, and why reading it is not enough

`GraphicProperties::pushToPropMap` (`oox/source/drawingml/fillproperties.cxx`:797-926, 24.2.7.2)
sorts a blip's brightness and contrast into three cases that use **two different arithmetics**:

1. **70 and −70 exactly** — PowerPoint's "Washout" — is mapped to LibreOffice's own
   `ColorMode_WATERMARK` and the stated pair is discarded (`fillproperties.cxx`:826-831).
   Watermark is a fixed **+50 luminance and −70 contrast** (`WATERMARK_LUM_OFFSET`,
   `WATERMARK_CON_OFFSET`, `vcl/source/graphic/GraphicObject.cxx`:53-54; applied at
   `drawinglayer/source/primitive2d/graphicprimitivehelper2d.cxx`:758-768 through
   `BColorModifier_RGBLuminanceContrast`).
2. **Both non-zero otherwise** is baked into the bitmap with *MSO's* formula, which spends half
   the brightness before the contrast and half after (`Bitmap::Adjust`'s `msoBrightness`
   branch, `vcl/source/bitmap/bitmap.cxx`:1694-1698).
3. **One of the two alone** becomes `PROP_AdjustLuminance` or `PROP_AdjustContrast` and goes
   through the colour modifier, which spends the whole brightness after the contrast.

The checkout is a 27.2 alpha and the binary that made the reference is 24.2.7.2, so the source
is a hypothesis. `check-washout.py` tests it against the reference's own pixels — predicting
the reference's page from ours, mean absolute error per channel over the whole of page one:

```
identity                      163.03 of 255
washout (lum +0.5, con -0.7)    7.15
bright 70% alone               24.94
bright +0.7 then contrast -0.7 30.98
```

The last line is the discriminator: applying the pair the *file states*, through the same
modifier, is four times worse than applying the watermark mapping. The mapping is the binary's
and not merely the source's.

### Reach

`count-bliplum.py` parses rather than greps, because a colour's `<a:lum>` and a blip's share a
tag name. Of the slides track's **112 pptx-family decks, 15 carry a blip `a:lum`**: twelve
blips state the washout pair, one states 20 and 20, and thirty-two state it empty and mean
nothing by it. So all three cases occur and the commonest is the no-op, which must stay a
no-op — it costs the JPEG pass-through otherwise.

### A defect in this round's own wiring, found by rendering rather than by reading

The first build changed nothing at all. `RasterImageDecoder.Ensure` builds a fresh image from
the encoded bytes and copies across only the fields it names — it named `Duotone` and not the
new one, so the recolouring was dropped on **every** picture, since one that is already decoded
never takes that branch. The symptom is a picture that decodes perfectly and draws untouched.
`TheRecolouringSurvivesADecode` asserts it now.

### The fixture

`tests/corpus/features/slide-picture-washout.pptx` draws one generated 0–255 ramp four times:
untouched, washout, brightness alone, and both stated. Mean absolute error per channel against
LibreOffice at 72 dpi:

| band | before | after |
|---|---|---|
| untouched | 2.20 | 2.20 |
| washout, 70 and −70 | 120.02 | **0.24** |
| brightness alone, 40 | 85.87 | **1.52** |
| both stated, 20 and 20 | 47.94 | **2.28** |

The fidelity bound is the *control band's own error plus one level* rather than a constant:
both writers resample a 256-pixel picture to 560 and interpolate differently, which costs about
two levels on a steep ramp whatever the recolouring does. A fixed threshold would be either
loose enough to pass a broken curve or tight enough to fail on resampling.

The unit test's expectations are the reference's own levels, read column by column off that
page, and every one of them reproduces exactly.

## The binary twin, named and not implemented

`.ppt` states the same thing as Escher properties and LibreOffice applies the same rule:
`DFF_Prop_pictureContrast` and `DFF_Prop_pictureBrightness`, with `nContrast == -70 &&
nBrightness == 70` selecting `GraphicDrawMode::Watermark`
(`filter/source/msfilter/msdffimp.cxx`:3906-3960). The contrast's encoding is not linear — it is
`0x10000` for MSO's 50%, units of 1/50th below that and `50/(100−x) × 0x10000` above — and the
brightness is the raw value over 327.

**Unmeasured**: I did not count how many corpus `.ppt` files carry either property. Recording
it because the brief's own `a:prstTxWarp` note is the same shape of trap — scoping a DrawingML
feature to pptx alone misses its binary twin, and half the instances with it.

## The text-rectangle fix, swept whole

`sweep-grid`, 163 rows, no path twice, 0 `ref-failed`, against a checksummed snapshot that was
verified to move the fixture before the run started.

| | baseline | after |
|---|---|---|
| word gate | 151 / 163 | **151 / 163** |
| signed `ink%` | 1360.08 | **1357.76** |
| unsigned `\|ink\|%` | 1680.10 | **1677.66** |
| major pages | 438 | **438** |

**114 documents moved — 68 better, 46 worse, 5.39 won against 2.95 lost.** No verdict changed on
any document and no batch moved. That is the shape the brief predicted: a change that touches
nearly every deck by a fraction of a point each, and wins about twice what it loses.

### And it did not close the one-point band, which refutes my own hypothesis

I took this fix partly on the argument that the autofit search is a *threshold* comparison, so a
box that is one or two units out lands on the wrong side of a 16-unit window. The pinned document
says otherwise. `2015-Civil-Rights-Website-training.ppt` page 21 is

```
ours 20.010 x31   ref 18.990 x31   +5.37%
```

**before and after, unchanged**, and the deck's 30 differing pages stayed 30 — its `|ink|%` went
33.20 → 33.34, slightly the wrong way. So the box is not what decided it.

That is worth more than the fix is. With the em on the grid since round seventeen and the box on
it now — and a fixture proving both edges land where the reference's do — **the only unquantised
quantity left in that comparison is the measured text height**, which is what rounds seventeen and
eighteen both attributed it to. The elimination is now complete rather than argued.

One caveat, specific to this document: `2015-Civil-Rights` is a `.ppt`, and `PptSlideLayout`
scales a grouped shape's rectangle by `Math.Round(width.Emu * placement.A)` before `Place` sees
it. If that group path disagrees with LibreOffice's by more than a unit, the box on *this* deck is
still not the reference's and the elimination does not hold here. I did not measure the group
scale itself.

## Tests

Per project, each run redirected to its own file, on the final tree. **0 failed and 0 skipped in
every one.**

Core 243, Text 237, Containers 109, Vector 291, Rendering **115**, Markup 259, OpenDocument 125,
WordProcessing 608, Spreadsheets 437, Presentations **520**, Fidelity **544**.

The three that moved are this round's own: Rendering +11 (`LuminanceRecolourTests`),
Presentations +3 (`SlideTextAreaGridTests`), Fidelity +2 (`SlideTextAreaGridComparisonTests`,
`SlidePictureRecolourComparisonTests`). Every other count is the base's to the digit — and note
that the base's Spreadsheets is **437** and not the 432 slides round eighteen recorded, because
sheets round twenty landed in the same merge.
