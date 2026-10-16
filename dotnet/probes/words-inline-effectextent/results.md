# `wp:effectExtent` on an inline drawing

*Measured 2026-09-04 in the container described at the top of `dotnet/CLAUDE.md`. **Both** installed
references were used throughout and they agree on every figure below: the distro's **24.2.7.2** at
`/usr/bin/soffice`, and the TDF tarball's **26.2.4.2** at `/opt/libreoffice26.2` with its 33 duplicate
metric-compatible fonts moved aside. `fc-match "DejaVu Sans"` resolves to DejaVu and `fc-match Calibri`
to Carlito. X-Ray.Office at `claude/renderer-comparison-artifact-m1g0wy`.*

## Why this exists

`words/drawingset-001/docx/WordArt_Shapes_Arrows_Catalog1.docx` is a 52-page catalogue of 340 inline
shapes. It rendered to **45 pages against both references' 52**.

Per `probes/words-version-screen/results.md`, the first question is whether a divergence is the gate
binary rather than a defect. **It is not, and this document is unusually clean evidence of that**:
24.2.7.2 and 26.2.4.2 paginate it *identically* — 52 pages each, the same shapes on every page — and
they agree to the twip on all seven fixtures below. Where the two references agree with each other and
we differ, there is no version question to settle.

## The cause

Every one of the 340 drawings is a `wp:inline` with no rotation, and every one carries a
`wp:effectExtent`. Three distinct values, censused off `word/document.xml`:

| `wp:effectExtent` (all four edges) | shapes |
|---|---:|
| `27432` (2.16 pt) | 182 |
| `137160` (10.8 pt) | 99 |
| `91440` (7.2 pt) | 59 |

LibreOffice folds all four edges straight into the object's own margins for this case —
`sw/source/writerfilter/dmapper/GraphicImport.cxx`:1036-1055, guarded by `IMPORT_AS_DETECTED_INLINE`
and `nOOXAngle == 0`, and commented there:

> EffectExtent contains all needed additional space, including fat stroke and shadow. Simple add it to
> the margins.

Those margins are then part of the portion Writer hangs on the line. `SwFlyCntPortion::SetBase`
(`sw/source/core/text/porfly.cxx`:401) sizes the portion from
`SwAsCharAnchoredObjectPosition::GetObjBoundRectInclSpacing()`, and `CalcPosition`
(`sw/source/core/objectpositioning/ascharanchoredobjectposition.cxx`) is where the object rectangle is
enlarged by its spacing:

```cpp
aObjBoundRect.AddTop( - nULSpaceUpper );
aObjBoundRect.AddHeight( nULSpaceLower );
```

So on an unrotated inline drawing the effect extent is simply **room on the line**, and X-Ray.Office read
none of it.

## The measurement

`make-fixture.py` builds minimal DOCX fixtures: a 12 pt `TOPLINE`, a paragraph holding one 144 × 50.4 pt
black inline shape, a 12 pt `BOTLINE`. `measure.py` renders each through both references and through
X-Ray.Office and reports the gap between the two text lines, plus the drawn box's own band.

Gap between `TOPLINE` and `BOTLINE`, in points:

| fixture | 24.2.7.2 | 26.2.4.2 | Δ vs control | ours, before | ours, after |
|---|---:|---:|---:|---:|---:|
| `ee0` — control | 64.25 | 64.25 | — | 64.20 | 64.20 |
| `ee27432` | 68.55 | 68.55 | **+4.30** | 64.20 | 68.50 |
| `ee91440` | 78.65 | 78.65 | **+14.40** | 64.20 | 78.60 |
| `ee137160` | 85.85 | 85.85 | **+21.60** | 64.20 | 85.80 |
| `ee-t-only` | 75.05 | 75.05 | **+10.80** | 64.20 | 75.00 |
| `ee-b-only` | 75.05 | 75.05 | **+10.80** | 64.20 | 75.00 |
| `dist-only` | 64.25 | 64.25 | **+0.00** | 64.20 | 64.20 |

Four things fall out of it:

1. **Top and bottom are independent and additive.** Each alone adds its own 10.8; together they add
   21.6.
2. **The growth is the stated EMUs rounded to the twip.** 2.16 pt is 43.2 twips, lands at 43, and
   doubles to 4.30 rather than to 4.32. Our reader's shared `Emu` helper already rounds that way, so
   the figure comes out right without a special case.
3. **`dist*` on a `wp:inline` is inert.** A fixture stating `distT="137160" distB="137160"` moves the
   line by **0.00**. That is not the attribute being ignored by accident — `GraphicImport.cxx`:1387-1398
   is four cases of `case NS_ooxml::LN_CT_Inline_distT: m_nTopMargin = 0;`, which never reads
   `nIntValue`. The attribute's *presence* zeroes the margin, discarding its value. A reader that added
   `dist*` to the extent would be 21.6 pt out per drawing on exactly this document, which states both.
4. **The residual 0.05 pt is pre-existing.** It is one twip, it is on the zero-extent control as well,
   and it did not move.

## Where the shape itself lands, and the one thing not reproduced

The box's own band, at 288 dpi, `ee137160` against `ee0` — identical on both references:

| | box top | box height | box left |
|---|---:|---:|---:|
| `ee0` | 85.75 | 50.25 | 72.00 |
| `ee137160` | 96.75 | 50.00 | 82.75 |

So the reference paints the shape at the **outer top plus the top extent**, and at the outer left plus
the left extent — the drawing sits inside the enlarged rectangle rather than filling it.

But a shape carrying a `wps:txbx` splits in two in LibreOffice, and the halves disagree. `tb-ee0` and
`tb-ee137160` are the same fixture with an `INSIDE` run in a centred text box:

| | box top | `INSIDE` y |
|---|---:|---:|
| `tb-ee0` | 85.75 | 104.66 |
| `tb-ee137160` | 96.75 | **104.66** |

**The fill moves by the extent and the text does not.** The text stays centred in the box's *unshifted*
rectangle. That is LibreOffice's draw-shape and TextBox halves failing to sync
(`SwTextBoxHelper::synchronizeGroupTextBoxProperty` is called from `SetBase` and does not carry this
offset), not a rule — and it is visible in the corpus document, where the reference's `WORDART` runs sit
at the same y as ours while the next label sits 21.6 pt lower.

A `PageFrame` is one object and cannot be in two places. It is placed where the reference puts the
**text**, i.e. at the outer top with no extent offset, because that is where the ink the shapes actually
carry ends up. The alternative — offsetting the frame by the extent — would match the reference's fill
rectangle and put every one of the catalogue's 340 text runs 2.16 to 10.8 pt below the reference's.
Recorded here so the next round does not read the un-offset placement as an oversight.

## Result on the corpus document

| | before | after | reference (both) |
|---|---:|---:|---:|
| pages | 45 | **52** | 52 |
| pages whose shape span differs from the reference's | 51 of 52 | **0 of 52** | — |
| words (`pdftotext`) | — | 2492 | 2468 |

All 340 of its drawings are `wps:wsp` shapes, so the picture rule below leaves the document exactly
where this put it.

Word count is inside the gate's `max(2%, 3)` band, which is 49.4 here against a delta of 24.

## Only for a drawing that stays a shape — a plain picture takes none of it

**The first cut of this change applied the extent to every unrotated inline drawing, and that was
wrong for pictures.** It is recorded here rather than quietly corrected, because the fixtures that
missed it were all built from `wps:wsp` shapes and the gate could not see the result.

The whole block that folds the extent into the margins sits inside `if (m_xShape.is())`
(`GraphicImport.cxx`:879-883), and twenty lines above it:

```cpp
if ( nRotation == 0 && !bContainsEffects )
    m_xGraphicObject = createGraphicObject( xGraphic, xShapeProps );
bUseShape = !m_xGraphicObject.is( );
```

A picture with no rotation and no DrawingML effects becomes a Writer graphic object, its shape is
disposed, and **none of the margin code runs at all**. The conversion is refused — so the drawing
stays a shape and does get the extent — when the picture is rotated (fdo#70457) or when
`EffectProperties`, `3DEffectProperties` or `ArtisticEffectProperties` reach its grab bag.

`make-picture-fixture.py` is the shape fixture with the `wps:wsp` replaced by a `pic:pic`. Both
references identical on every row:

| picture fixture | 24.2.7.2 | 26.2.4.2 | Δ vs control |
|---|---:|---:|---:|
| `pic-ee0` — control | 64.20 | 64.20 | — |
| `pic-ee137160` — all four edges | 64.20 | 64.20 | **+0.00** |
| `pic-gpp` — `gpp-pr`'s own `l=19050 t=19050 r=21590 b=23495` | 64.20 | 64.20 | **+0.00** |
| `pic-ln-ee137160` — plus a 2.25 pt `a:ln` border | 64.20 | 64.20 | **+0.00** |
| `pic-ee137160-shadow` — plus an `a:outerShdw` | 85.85 | 85.85 | **+21.65** |
| `pic-shadow-ee0` — the shadow alone | 64.25 | 64.25 | +0.00 |
| `pic-scene3d` — plus `a:scene3d` and `a:sp3d` | 85.85 | 85.85 | **+21.60** |
| `pic-ee137160-rot` — rotated 20 degrees | 110.45 | 110.45 | +46.25 |
| `pic-rot-ee0` — rotated, **no extent at all** | 110.45 | 110.45 | **+46.25** |

Three things it settles:

1. **A plain picture takes nothing**, at any extent, symmetric or not.
2. **A picture carrying effects takes the full amount**, and the effect itself contributes nothing —
   `pic-shadow-ee0` is the control. So it really is the extent, gated on the conversion being refused.
3. **A border is not an effect.** `bContainsEffects` is only the three grab-bag entries; a picture
   carrying a 2.25 pt `a:ln` still converts and still takes nothing. That matters because the corpus
   pictures that carry an extent nearly all carry a border too, and "the extent is there to cover the
   fat stroke" is a tempting reason to expect otherwise.
4. **The rotated rows are not about the extent.** They are identical with and without one, so the
   +46.25 is the *rotated bounding box* — LibreOffice sizes a rotated inline drawing by its snap
   rectangle, and the rotated branch's margins clamp to zero at this angle. That is a separate open
   defect: we size a rotated inline drawing by its unrotated extent and are 46.25 pt short on this
   fixture, before and after this change alike. Applying the plain extent there would have been
   curve-fitting towards a number produced by something else, so a rotation now skips it.

   **Half of that is right and the conclusion "so a rotation skips the extent" is wrong**, and the
   fixtures here could not have shown it: they vary the *gap*, and downwards the turned height is
   the larger of the two rectangles and swallows the extent whole. Varying the width instead, the
   same 20-degree fixture's line advance goes **149.87 to 171.47 pt** when a `137160` extent is added,
   on both references, with the gap unchanged to the hundredth. The rule is in
   `probes/words-inline-rotated-bbox/`, which supersedes this row: a turned drawing takes Word's
   swapped-and-expanded box across and the larger of that and its turned box down.

### What that cost, and the correction it forces

| document | page-1 one-sided ink vs 26.2, before | with the extent on pictures | with the rule |
|---|---:|---:|---:|
| `done-013/.../gpp-pr-top-7-office-markets-4q-2023.docx` | 8.30 | **9.86** | **8.30** |
| `done-015/.../PI-doc.-no.-2E-Technical-Review-Report.docx` | 0.15 | — | 0.15 |

`gpp-pr`'s chart is one unrotated `pic:pic` with `l=1.5 t=1.5 r=1.7 b=1.85` pt. Its picture is drawn
at **identical x and y before and after** — measured off the PDF's own image placement, `(65.20,
476.80)-(369.50, 654.95)` in both — so the horizontal growth moved nothing. What moved was everything
*below* it: `02 January 2024, Hamburg.` sat at 385.74 pt, went to 389.09 with the extent applied, and
is back at 385.74 with the rule. The reference puts it at 383.64, and that 2.10 pt residual is
pre-existing and untouched by any of this.

**And this overturns what the previous section of this file used to say about
`TE.CAO.00125 … OJT Logbook.docx`.** That document went `match` 15/15 to `pages` 16/15, and it was
recorded here as the change being right — a header logo whose 0.75 pt bottom extent legitimately
grew the header, on the strength of a header fixture that showed both references growing by exactly
that. **The header fixture was a `wps:wsp`; the logbook's logo is a `pic:pic`**, unrotated and
effect-free, so it takes nothing. The document is back to 15 pages and back to `match`.

The header measurement itself stands and is worth keeping, because it establishes the other half:

| header fixture (a `wps:wsp`) | 24.2.7.2 | 26.2.4.2 | Δ | ours |
|---|---:|---:|---:|---:|
| `hdr-ee0` — control | 78.71 | 78.71 | — | 78.66 |
| `hdr-ee9525-rb` | 79.46 | 79.46 | **+0.75** | 79.41 |
| `hdr-ee137160-b` | 89.51 | 89.51 | **+10.80** | 89.46 |
| `hdr-ee137160-all` | 100.31 | 100.31 | **+21.60** | 100.26 |

**A header grows exactly as the body does — for a shape.** Both halves of the rule are needed, and
having only one of them is what produced the wrong conclusion.

### The whole track, measured by displacement rather than by ink

A words sweep either side of the picture rule, with `/CreationDate` masked so a byte comparison means
something: **38 of 338 of our renderings change and 300 are identical.** Gate verdicts go
`MATCH 310 MISMATCH 28` to **`MATCH 311 MISMATCH 27`** — the catalogue matches, and
`TE.CAO.00125 … OJT Logbook.docx` returns to `match` at 15/15.

Scoring those 38 needs care, and the first instrument used on them was the wrong one. Page-1
**one-sided ink** — the share of the page where exactly one side has ink — reported 9 improved, 22
unchanged and **6 worse**. It is displacement-sensitive by construction: a glyph that moves an eighth
of a point can cross a pixel boundary and flip every pixel of its own outline. `FO.FCTOA.00010` is the
worked example — its ink went **+0.76 "worse"** while its first body line moved from 47.16 pt to
47.08 against the reference's 45.76, which is 0.08 pt **closer**, with the page count unchanged at 16.

Mean |Δy| over page-1 words paired by text and order does not have that failure mode, and it is the
column to read:

| | documents | worst |
|---|---:|---|
| closer to 26.2 | 5 | `gpp-pr` **12.175 → 8.970 pt** (−3.205), `PI-doc` 0.749 → **0.049** (−0.700) |
| unchanged | 25 | — |
| further from 26.2 | 7 | `b050-19` 23.188 → 23.938 (+0.750) |
| unmeasurable | 1 | `xx_SETIS_PWS_template` (too few paired words on page 1) |

Net over the 38: **−2.47 pt** of mean displacement.

**The seven that go further are worth naming rather than burying.** Four of them —
`b050-19` (23.2 pt), `UG.CAO.00006` (12.0), `UG.CAO.00133` (10.8) and `May 25 bulletin` (4.5) — have
page-1 deviations an order of magnitude larger than the movement, so a fraction of a point either way
is riding on a different and much bigger defect. The other three move by 0.04 to 0.14 pt. In every
case the direction is "we no longer add something the reference does not add", and the fixtures above
measure that directly on an authored file where nothing else varies — plain, bordered, shadowed,
3-D and rotated pictures, on both binaries. **Where a mechanism is measured in isolation and a corpus
row disagrees, the corpus row is confounded**; these are documents where being wrong happened to
cancel part of an unrelated error.

The general lesson, which is the reason this section is written out rather than folded away: **a
fixture set built from one element type measures that element type.** Eleven fixtures agreed with
both references and all eleven were `wps:wsp`; the corpus documents that broke were pictures, and the
gate scored one of them `match` while it moved 1.5 points of first-page ink. Vary the *kind* of thing
under test, not only its numbers.

## Horizontally — and this is where the section above was wrong by omission

*Measured 2026-09-06, same container, same two references, X-Ray.Office at `agent/draw-inline`.*

Every fixture above varies `t` and `b` only. `make-x-fixture.py` varies `l` and `r`, on the same
144 x 50.4 pt shape laid across a line as `LEFT` + drawing + `RIGHT` on a landscape page, and
`measure-x.py` reads three things off it: the shape's own ink **columns** at 288 dpi, the x of the
words either side of it, and — for the `tbx-*` fixtures, whose shape carries a `wps:txbx` — the box
of the `INSIDE` run.

In PDF points, both references, before this round's change:

| fixture | who | inkL | inkR | adv | INSIDE x | INSIDE y |
|---|---|---:|---:|---:|---:|---:|
| `x-ee0` | 24.2 / 26.2 | 103.50 / 103.25 | 247.50 | 149.87 / 149.84 | — | — |
| | ours | 103.75 | 247.75 | 150.00 | — | — |
| `x-l-only` — `l=137160` | 24.2 / 26.2 | **114.25** | 258.25 | 160.67 / 160.64 | — | — |
| | ours | **103.75** | 247.75 | 160.80 | — | — |
| `x-r-only` — `r=137160` | 24.2 / 26.2 | 103.50 / 103.25 | 247.50 | 160.67 / 160.64 | — | — |
| | ours | 103.75 | 247.75 | 160.80 | — | — |
| `tbx-ee0` | 24.2 / 26.2 | 103.50 | 247.50 / 247.25 | 149.87 | 155.95 / 155.90 | 90.86 |
| | ours | 103.75 | 247.75 | 150.00 | 156.00 | 90.81 |
| `tbx-l-only` | 24.2 / 26.2 | **114.25** | 258.25 | 160.67 | **166.75** / 166.70 | 90.86 |
| | ours | **103.75** | 247.75 | 160.80 | **156.00** | 90.81 |
| `tbx-t-only` | 24.2 / 26.2 | 103.50 | 247.50 / 247.25 | 149.87 | **155.95** / 155.90 | **90.86** |
| | ours | 103.75 | 247.75 | 150.00 | 156.00 | 90.81 |

Four things it settles, and the third is the one the earlier round could not see:

1. **The left extent moves the drawing; the right one does not.** `l` alone moves the ink band by
   10.75 pt (10.8 rounded to the twip) and leaves the advance's own growth to `l+r` together, which
   is what the earlier fixtures already showed vertically.
2. **The advance was already right.** `adv` grows by `l+r` on both references and on ours, before
   and after — the line-box half of the rule landed in the previous round and is untouched here.
3. **Horizontally the draw shape and the TextBox agree, where vertically they do not.** `l` moves
   the fill band *and* the `INSIDE` run by the same 10.8 pt. `t` moves **neither** — the run stays at
   155.95 across and 90.86 down. So the asymmetry recorded above is specifically vertical, and
   placing the frame at the outer corner on *both* axes was right on one and wrong on the other.
4. **It is one line of LibreOffice.** `SwAsCharAnchoredObjectPosition::CalcPosition`
   (`sw/source/core/objectpositioning/ascharanchoredobjectposition.cxx`:129-133) adjusts the anchor
   point by `nLRSpaceLeft` and then by `nULSpaceUpper`; both moves happen, and only the vertical one
   is lost again when `SwTextBoxHelper` fails to carry it to the TextBox fly.

After moving the frame by `EffectExtent.Left` in `FrameLayout.HangInline`, every row above agrees
with both references — `x-l-only` and `tbx-l-only` read `inkL` **114.50** against their 114.25, and
`tbx-l-only`'s `INSIDE` reads **166.80** against 166.75, which is the same 0.25 pt raster quantum and
0.05 pt text offset the zero-extent control already carried.

### On the corpus document

`verdict.py`, 150 dpi, against **both** references, on `WordArt_Shapes_Arrows_Catalog1.docx`:

| | before | after |
|---|---|---|
| page 7 | `displaced-horizontal`, **dx −23 px** (11.04 pt), dy 0, ink −0.0, worst tile 0.2649 | **`match`**, dx 0 |
| page 3 | `displaced-vertical`, worst tile **0.3289** | `displaced-vertical`, worst tile **0.0749** |
| pages with a non-zero `dx` | several | **0 of 52** |
| pages / words | 52 / 2468 | 52 / 2468 (references 52 / 2468) |

The `dx` column is zero on every one of the 52 pages afterwards. What is left on the document is
the *vertical* half — pages 23 onwards read `dy` of 1 to 16 px — and that is the draw-shape/TextBox
disagreement above, not this.

> **Closed 2026-09-06 in `probes/words-inline-shape-ink/`.** *"A frame here is one object and cannot
> be in two places"* was the obstacle and the answer was two rectangles: `PlacedFrame.Area` is the
> text's and `PlacedFrame.Ink` the drawing's, the second offset below the first by the top extent.
> The catalogue goes from 27 pages reading a non-zero `dy`, 164 px in total, to **one page reading
> one pixel**, with `dx` still zero on all 52 and 52 / 1972 pages and words unchanged. That probe
> also supersedes this file's claim that the placement is *"where the reference puts the text and the
> ink it holds"* — it is where the reference puts the text, and the ink is elsewhere.

### The warped-body offset is now vertical only

`DocxFontwork.Inset` moved a warped body's curves by `(effects.Left, effects.Top)`, because a warped
body has no TextBox left and follows the draw shape. Its horizontal half is now in the frame's own
position, so `Inset` shifts by `Top` alone; leaving both would have put a warped body 10.8 pt to the
right of its own shape. Page 18, the warped page, is `match` against both references afterwards.

## Reproducing

Three fixture generators, all read by the same `measure.py`:

```sh
export XRAY_OFFICE_CLI=.../tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
python3 make-fixture.py         /abs/scratch/fx   # a wps:wsp shape in the body
python3 make-picture-fixture.py /abs/scratch/px   # a pic:pic, plain / with effects / rotated
python3 make-header-fixture.py  /abs/scratch/hx   # a shape in a header
python3 measure.py /abs/scratch/fx /abs/scratch/out

python3 make-x-fixture.py /abs/scratch/xfx        # the same shape with `l`/`r` varied
python3 measure-x.py /abs/scratch/xfx /abs/scratch/xout
```

`measure-x.py` is the horizontal reader: same three renderers, but it reports the ink **columns**
rather than the rows, plus the x of `LEFT`, `RIGHT` and a text box's `INSIDE`.

`make-picture-fixture.py` writes its own 8x8 PNG with `struct` and `zlib` rather than depending on an
image library, so the fixture half runs in a bare container.

`score.py` is the scorer the corpus table above uses, and it carries both metrics on purpose:

```sh
python3 score.py before.pdf after.pdf reference.pdf
# ink  before 10.014  after 8.418  delta -1.596
# dev  before 12.175  after 8.970  delta -3.205
```

Read `dev`. `ink` is there so that a later round comparing against a stored first-page-ink figure can
reproduce it, and so that the two can be seen disagreeing.

`measure.py` needs Pillow for the box band and `pdftotext` for the text lines. It renders each fixture
through `/usr/bin/soffice`, `/opt/libreoffice26.2/program/soffice` and `$XRAY_OFFICE_CLI`, so it re-checks
the version question every time rather than trusting this file's claim that there is none.

## A caution about the document's shape census

The round that opened this document was briefed that it holds *"291 VML `v:shape`, 96 carrying a
WordArt `textpath`, 204 with a gradient fill, 48 with a `scene3d`"*. Counted off the part itself, split
by which half of the `mc:AlternateContent` the count falls in:

| | `mc:Choice` (DrawingML — what both renderers use) | `mc:Fallback` (VML — what neither uses) |
|---|---:|---:|
| shapes | 340 `wps:wsp` | 148 `v:shape`, 88 `v:rect`, 57 `v:line` |
| WordArt | 123 `a:prstTxWarp`, 24 of them a real warp | 48 `v:textpath` |
| gradient shape fill | **0** `a:gradFill` | **0** `type="gradient"` |
| 3-D | **0** `a:scene3d` | **0** `o:extrusion` |

**There is no gradient shape fill and no `scene3d` anywhere in the file, in either branch.** The 208
gradients that do exist are `w14:textFill` on runs, which LibreOffice's DOCX import draws none of. Two of
the briefed figures are close to twice the fallback's real counts (96 = 2 x 48, 291 ~ 2 x 148) and two
have no counterpart at all.

Nothing was lost to it here, but it is `render-comparison`'s rule 6 arriving again: *ask what the
document actually contains before believing a theory about it.* A round that had gone looking for the
3-D extrusion path would have found no caller, and one that had gone looking for a shape-gradient defect
would have been reading a text-fill property the reference discards.
