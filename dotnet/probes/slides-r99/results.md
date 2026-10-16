# A line's height counts the blank that ends it, and the bullet is the only thing on a slide that reads a line's height

Round 99, slides track. Three seats — **O15's witness closes on a measured mechanism, its census
moves 71.10 → 66.06 pt with nothing newly wrong, and the seat stays open**, **O28 half closes as
below the noise floor and half stays open, re-aimed at O15**, **O13 stays open with the job sized
and one of its two halves shown not to be a binary-reader job at all**.

**The one figure that goes the wrong way is the ink**, 252.21 → 252.82 summed over the 51 `.ppt`,
and it is two documents. §1.4 gives both, names what moves in each, and says why the change was
kept anyway.

Every figure below was read out of a file that exists in this directory or in the run logs beside
it. Where a census is quoted, the base rate is quoted beside it — that is confound C9, and it
exists because of these seats.

## Environment

| | |
|---|---|
| worktree / branch | `/home/user/wt-slides2`, `agent/slides2`, base `1acd84bea` |
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**. `/usr/bin/soffice` (24.2.7.2) is used for nothing here |
| reference bank | `/home/user/gate-orig-r83/ref`, 948 PDFs |
| reference flat-ODP | `/home/user/r97-slides/fodp-all`, all 51 `.ppt` converted by 26.2.4.2 |
| corpus | `/home/user/sample-files`; the slides binary column is **51 `.ppt`** (`git ls-files slides \| grep -i '\.ppt$'`) |
| C++ tree | `/home/user/libreoffice-core`, read only, never built. **It declares `27.2.0.0.alpha0+`** and is not the reference binary's source — every mechanism below is confirmed a second time against 26.2.4.2's own output |

---

# 0. The instrument

`tfy.py` reads a PDF's content stream and prints, for every text show, the font, the size and the
**baseline**, taken from the stream's own `Tf`/`Td`/`TD`/`Tm`/`T*`/`TL` operators.

It exists because both of the other instruments have already produced a retracted finding on this
seat: `pdftotext -bbox` reports an ink box derived from the font descriptor — a Caladea heading
once showed a flat 2.1 pt offset that was exactly `usWinAscent − sTypoAscent` — and PyMuPDF
`rawdict` reports a character *cell*, which a round once compared and called "tracking out 18 %"
when the cause was a family-class font split.

**Validated against an independent reader before it was used for anything**: on
`2015-Civil-Rights-Website-training.ppt` page 2 it agrees with
`.claude/skills/render-comparison/scripts/pdf-ops.py` on all twelve shows, to the last digit
(`ops-check.txt`).

---

# 1. O15 — the block height is short because a line's trailing blank is not measured

**The witness closes and two of the census's nineteen pages close with it. The seat stays open**,
because seventeen do not, and because this round did not measure the `.pptx` half at either leg.

## 1.1 What the reference does, read and then measured

EditEngine sizes a line by walking **every** portion on it — `pLine->GetStartPortion()` to
`pLine->GetEndPortion()` inclusive, skipping only a `PortionKind::LINEBREAK` — and taking the
largest ascent and the largest descent it finds
(`editeng/source/editeng/impedit3.cxx`:1496-1519, read in this tree; with
`IsFixedCellHeight()` those are the em and `fround(1.2 × em) − em`, `:3138-3141`, and the line
height is `nMaxAscent + nMaxDescent`). A portion that happens to hold nothing but a space is not
exempt. Trailing blanks are left out of a line's *width* — that is the whole point of
`TextLine.VisibleEnd`, and it is still what the line is drawn and aligned by — but nothing leaves
them out of its *height*.

**Confirmed twice against 26.2.4.2 itself, which is the leg that survives the tree being a
different version.** `pres_ioc_phuket.ppt` page 26's white box (`gr78`) ends its wrapped paragraph
with a single space set in **28 pt** Arial Black after 19 pt italic text:

- **26.2.4.2 draws that space.** Its page-26 content stream holds a one-glyph **28.01 pt** show at
  the end of the last line, which ours did not, and its wrapped line carries **63** glyphs where
  ours carried 62 — the break blank, on the line before the break, exactly as `GetEndPortion()`
  says.
- **26.2.4.2's own line pitch says which line the blank sized.** From the second baseline to the
  third it is **31.81 pt** where ours was 22.79. 31.81 pt is 1122 hundredths of a millimetre,
  which is `(fround(670 × 1.2) − 670) + 988` — the 19 pt line's descent plus the **28 pt** line's
  ascent.
- **26.2.4.2's own flat-ODP export says what that costs the shape.** `gr78` states
  `draw:auto-grow-height="true"` with `fo:min-height="0cm"`, so the shape's height *is* the block's:
  the export gives it `svg:height="3.267cm"`, which is its three lines `1016 + 804 + 1186` plus
  0.13 cm of padding at each end — **3266** against the stated 3267, the one unit being
  `tools::Rectangle`'s own both-edges convention. Measuring that last line at 19 pt instead gives
  `1016 + 804 + 804 + 260 = 2884`, **2.884 cm**, and that is the 81.77 pt we drew.

## 1.2 The change

`SlideTextLayout` measured a line's em over `box.Line.Start .. box.Line.VisibleEnd`. It now
measures over `box.Line.Start .. box.Line.End`, in both branches — the font-independent one
(`LargestSize`) and the face-metric one (`FaceHeight`). Nothing else moves: the line is still
drawn, aligned and width-measured by `VisibleEnd`, so no glyph is added to any page by this change
directly.

`SlideTrailingBlankLineHeightTests` pins it with the arithmetic above in miniature and fails at the
base.

## 1.3 The witness, after

All three of page 26's white boxes, read out of the two PDFs' own content streams:

| `pres_ioc_phuket.ppt` p26, white box height | 26.2.4.2 | base | head |
|---|---:|---:|---:|
| `gr78`, the O15 witness | **92.64** | 81.77 | **92.60** |
| the right-hand box | **121.44** | 110.58 | **121.41** |
| the left-hand box | **110.61** | 110.59 | **110.59** |

Two of the page's three white boxes were **10.87 and 10.86 pt short** and are now within
**0.04 pt** of 26.2.4.2; the third was already right and does not move. The base figure 81.77
is the register's own, re-measured here at this round's base rather than taken from it.

## 1.4 What it costs, measured over the whole `.ppt` column

**The gate — no verdict moves, and the verdict statistic gets worse.** Our half of the `.ppt`
column re-rendered at both binaries and scored against the banked 26.2.4.2 reference with
`batch-check.sh`'s own rule (column 9, `glyphs`, alphanumeric characters, within max(2 %, 15)):

| | base | head |
|---|---:|---:|
| documents | 51 | 51 |
| `match` | **49** | **49** |
| page counts moved | — | **0** |
| alphanumeric counts moved | — | **2** |
| sum \|glyph distance\| | **1320** | **1430** |
| renderings whose bytes changed | — | **19** |

**The 110 is two documents and I could not defend either as an improvement.** The change adds no
glyph to any page directly — the line is still drawn to `VisibleEnd`, and the witness's own glyph
counts are 62 and 15 before and after — but a taller block changes what fits inside a shape, and
these two are where that shows:

- `Inducement-to-Insurance-Business.ppt` **12892 → 12812** against the reference's 12899. Page 14
  loses one line, *"refers business to its affiliate title company "B.""*, off the top of a block
  that grew. That page is already badly wrong on both legs — the reference draws eight 28 pt lines
  from y 555.19 and we draw three from 458.39 — so what moved was an existing wrap error, not this
  rule.
- `2015-Civil-Rights-Website-training.ppt` **34818 → 34848** against the reference's 34818, which it
  had been matching exactly. Page 32 *gains* a line, *"Filing a discrimination complaint"*, that
  26.2.4.2 does not draw at all.

One loses a line and the other gains one, which is the signature of a block-height change meeting
our own overflow behaviour rather than of a systematic direction. **Both are recorded as this
round's cost and neither is explained further here.**

**Ink, over all 51, summed `|ink|%` per document against the same bank** (`ink.sh`,
`ink-base.tsv`, `ink-head.tsv`, `ink-summary.txt`):

| | base | head |
|---|---:|---:|
| sum \|ink\|% over 51 | **252.21** | **252.82** |
| MAJOR pages | 66 | 66 |
| documents whose ink moved | — | **13** |
| improved / worsened | — | **9 / 4** |

**On the metric this project ranks by, this round is net 0.61 worse over the `.ppt` column, and
the whole of it is those same two documents.** `Inducement-to-Insurance-Business` +1.77 and
`2015-Civil-Rights-Website-training` +0.85 against −2.01 summed over the nine that improve, the
largest of which are `FAA_Form_337` **8.49 → 7.49** and `ws_prod-g-doc-Events-r-6.-ESM`
5.31 → 4.66. **The witness itself moves only 4.77 → 4.70**, because a shape's height is a thin
region of ink however wrong it is — which is exactly why r97 reached for it as a geometry witness
rather than an ink one, and why the 0.04 pt in §1.3 is the measurement that matters here and the
0.07 of ink is not.

**Said plainly: the mechanism is measured twice against 26.2.4.2, the register's own O15 statistic
improves, and the ink figure is slightly worse.** The three reasons for keeping it are the
dominant-size census below (71.10 → 66.06 pt, 2 fixed, **0 newly wrong**), the 0.04 pt in §1.3,
and that what the two ink regressions move is our own overflow behaviour on pages already wrong on
both legs — page 14 of `Inducement` draws three of the reference's eight lines before and after.
**A round that disagrees with that judgement has every number it needs here.**

**The register's own O15 statistic moves, and this is the first round in which it has.**
Per-page dominant drawn text size — the size carrying the most alphanumeric characters, round 84's
statistic — for all 1534 pages of the 51 `.ppt`, our two binaries against the banked reference
(`sizes-base.tsv`, `sizes-head.tsv`, `size-summary.txt`; `sizes-ref.tsv` is r97's, off the same
bank):

| | base | head |
|---|---:|---:|
| pages whose dominant size differs by more than 0.15 pt | **19** | **17** |
| documents holding one | 15 | **13** |
| total \|size error\| over 1534 pages | **71.10 pt** | **66.06 pt** |
| of the differing pages, same alphanumeric count | 13 | 12 |
| … of those, we draw the larger size | 12 | 11 |
| **fixed / newly wrong** | — | **2 / 0** |

**The base reproduces r97's census exactly** — 19 pages, 15 documents, 71.10 pt, 13 same-alnum,
12 of 13 larger — which is worth stating because it is an independent re-derivation of that figure
at a different commit with a differently written scorer, and because it is what makes the head
column comparable to the register.

Three pages move and all three move towards the reference:

| | reference | base | head |
|---|---:|---:|---:|
| `ws_prod-g-doc-Events-r-6.-ESM.ppt` p21 | 18.99 | 20.01 | **18.99** |
| `FAA_Form_337.ppt` p66 | 32.99 | 36.00 | **33.00** |
| `JesuitAssocOfStudentPersonnel.ppt` p24 | 13.01 | 17.01 | **15.99** |

The first two leave the census; the third improves by a point and stays in it. **So a trailing
blank measured at its own size is one of the causes of "our block height is short of the
reference's"** — not all of it, two pages of nineteen, but the first named one.

**Reach beyond the `.ppt`.** `SlideTextLayout` is the shared slide layouter, so this change can
reach `.pptx` and `.odp` as well. Sampled base against head, byte for byte, one document at a time
(`reach.sh`, `reach-pptx.tsv`, `reach-odp.sh`, `reach-odp.tsv`):

| | documents | byte-identical | moved |
|---|---:|---:|---:|
| every 5th corpus `.pptx` | 50 | **47** | 3 |
| every 10th converted `.odp` | 31 | **30** | 1 |

and the three `.pptx` that move are 1.66 → 1.66, 0.61 → 0.61 and 4.38 → 4.36 of summed `|ink|%`
against the bank. So the rule fires almost entirely on the binary column, which is where a
trailing blank at a different size comes from.

## 1.5 What is left on O15

**Not closed.** The witness closes and two of the census's nineteen pages close with it; the
seat is the other seventeen.

1. **The sign is unchanged and is still the load-bearing fact.** 12 of the 13 same-alphanumeric
   pages at the base had us drawing the larger size, and 11 of 12 do at the head. Our measured
   block height is still short of the reference's on the pages that are left.
2. **The bullet is a denser instrument for what is left than the dominant size is, and §2.2 is
   the argument.** `nFirstLineHeight − nFirstLineTextHeight/2` is the one quantity a slide exposes
   that only the bullet reads; 217 bullets on 42 pages measure it against 26.2.4.2 where 19 pages
   measure a font size. And §2.2's census is **unchanged at this round's head**, so whatever the
   remaining error is, it is not this one.
3. **`RESPA_-_Section_8_Webinar.ppt` p18 is still the register's named next instrument** — 314
   characters on both sides, ours 20.01 against 18.99 — and it does not move here.
4. **The `.pptx` half of the census was not measured**, at the base or the head. r85's 41 pages
   over both columns still stands only as of its own base, and the `.ppt` half of it is 19 at this
   round's base and 17 at its head.

---

# 2. O28 — the size closes as unobservable, the baseline does not and is re-aimed

The seat holds three facts. The third — that the body text beside the bullet is at an **identical**
baseline on both sides — is agreement and has nothing to attribute. The other two are separable and
they end in different places.

## 2.1 The size is a one-unit rounding with no direction, and nothing that scores this corpus can see it

Every `.ppt` rendered at this round's base and every OpenSymbol show paired against the banked
26.2.4.2 reference's, on pages where both draw the same number of bullets and no bullet's size
differs by more than 0.35 pt — **292 bullets in 11 documents**, out of 1878 pages scanned
(`bullet-census.txt`, `bulletcensus.py`, `sweep.sh`, `tfy.py`):

| reference − ours, in hundredths of a millimetre | bullets |
|---|---:|
| −2 | 25 |
| **−1** | **69** |
| **0** | **128** |
| **+1** | **67** |
| +2 | 3 |

264 of 292 are exact or one unit out, and the one unit is **symmetric — 69 low against 67 high**.
A wrong *rule* has a sign. This has none: what is left is which side of a half a single product
falls on. `Outliner::ImpCalcBulletFont` is
`fround(aStdFont.GetFontSize().Height() × GetBulletRelSize()/100 × fFontY)`
(`editeng/source/outliner/outliner.cxx`:851-855, opened in this checkout) and
`SlideAutofit.ScaledMarker` is the same product in the same unit.

**One hundredth of a millimetre is below every threshold this project measures.**
`pdf-image-diff.py` rasterises a page's long edge to **512 pixels** (`--long-edge`, its default),
which on a 720 pt slide is 0.711 pixels per point — so 0.028 pt is **0.02 of a pixel** and moves no
ink; and the gate's verdict column counts alphanumeric characters, which a font size does not
change. On the register's own witness it is 17.773 against 17.802 — 0.16 % of one glyph.

**This half of O28 closes as *nil observable reach*, which is the register's second terminal state
read literally** — not as a LibreOffice defect, and not as "I could not find a witness". There are
292 witnesses; 164 of them differ; and the difference is smaller than the resolution of every
instrument this project scores with. The census above is what establishes that, and it is here so
that a later round can disagree with the arithmetic rather than re-run the search.

## 2.2 The baseline is real, one-directional, and it is not the bullet's font

**The base rate first, because that is what C9 asks for.** Restrict to pages where the two
renderings draw the *same sequence of (family, size) shows* — so the page's layout agrees — and
that carry at least one bullet: **42 pages in 5 documents**, 217 bullets and 425 body shows on the
same 42 pages (`bullet-census-agreeing.txt`, `bulletcensus2.py`).

| on those 42 pages | within 0.01 pt | within 0.10 pt | **beyond 0.10 pt** |
|---|---:|---:|---:|
| the **body**'s 425 shows | 191 | 411 | **14 — 3.3 %** |
| the **bullets**' 217 | 11 | 115 | **102 — 47 %** |

The bullet is misplaced at **fourteen times the body's rate on the same pages**, and it runs one
way: the residual's minimum is −2.268 pt and its maximum +0.085. The register's 0.652 pt is one
value of it, not a constant.

**It is not the bullet font's metrics.** If it were, the offset would be a fixed fraction of the
bullet's em. Per size on those pages it is not: +0.014 pt at 22.11 pt, +0.043 at 14.00, −1.843 at
14.29, −2.268 at 17.09.

**Where it does live, and this is the useful part.** Read out of this tree, the reference's
symbol-bullet baseline is

```
rStartPos.Y + aBulletArea.Bottom() − FontMetric::GetDescent()
aBulletArea.Top    = nFirstLineHeight − nFirstLineTextHeight + nFirstLineTextHeight/2 − bulletHeight/2
aBulletArea.Bottom = Top + bulletHeight − 1
bulletHeight       = OutputDevice::GetTextHeight() = the bullet font's ascent + descent
```

(`editeng/source/outliner/outliner.cxx`:1461-1467 for the area's vertical, :892, :906-909 and :919
for the placement, :951-956 for the descent step and :1334-1340 for the size;
`include/tools/gen.hxx`:597 for `Bottom = Top + Height − 1`; `vcl/source/outdev/text.cxx`:178 and
`vcl/source/outdev/font.cxx`:910 for what `GetTextHeight` returns) — while the body's first
baseline is `rStartPos.Y + nFirstLineMaxAscent` (`outliner.cxx`:919, the same statement's other
arm).

**So the bullet is the only thing on a slide that reads `nFirstLineHeight` and
`nFirstLineTextHeight`. The body reads only `nFirstLineMaxAscent`.** Differencing the two removes
`rStartPos` entirely and leaves
`nFirstLineHeight − ceil(nFirstLineTextHeight/2) + ceil(bulletHeight/2) − 1 − descent − nFirstLineMaxAscent`;
ours is the same expression up to that `− 1`.

On the register's own witness, instrumented (a temporary dump in `EmitMarker`, since removed):
our line gives **H = 1029, TH = 1270, A = 823**, and OpenSymbol's typographic metrics — which is
what VCL takes for it, its `fsSelection` bit 7 is set and `FontMetricData::ImplCalcLineSpacing`
prefers Typo whenever it is (`vcl/source/font/fontmetric.cxx`:497-534) — give
`ceil(bulletHeight/2) − descent = 150`. The body baseline agreeing to the last digit forces the
reference's ascent to 823 as well, and the bullet then requires that term to be **174**. The face
gives 150 from hhea and typo (which are the same numbers in this font, 1420/442 on a 2048 em) and
192 from usWin. Neither is 174. Solving the same equation over the corpus's other clean bullets —
*on the assumption that the reference's H, TH and A are ours* — hands back four different values of
that term per em, 0.238, 0.26-0.276, 0.316 and 0.371, where one face has one metric. **That is the
refutation of the assumption, not a fifth metric:** the residual is in H and TH, the two numbers
only the bullet reads, and not in the bullet at all.

**And §1's fix is not that error.** The same census re-run at this round's head — the five clean
documents re-rendered, `bulletcensus2h.py` — is **identical in every figure**: 217 bullets, 115
within 0.10 pt, 102 beyond it, mean −0.4765, and the same per-size table. So the trailing-blank
rule and the bullet's residual are two different line-height questions, which is worth knowing
before a round assumes closing one closes the other.

**Left seated, re-aimed.** O28's baseline half is an instrument for a line-height error rather
than a bullet seat, and the line-height error is O15's. A round taking it should instrument
`nFirstLineHeight` and `nFirstLineTextHeight` against 26.2.4.2 directly, on the pages
`bullet-census-agreeing.txt` names, rather than chase the bullet — 217 bullets on 42 pages is a
far denser sample of it than 19 pages of dominant drawn size.

---

# 3. O13 — the reach is two shapes, and half the job is not a binary reader at all

**Not closed and not implemented.** What this round adds is the size of the job and a correction
to what the witness actually is.

The reference-side census re-run over all 51 `.ppt`'s flat-ODP exports, now reading each text-path
shape's **fill** as well as its preset (`textpath-census.txt`):

```
8.16_AOD_FINAL_Provider_Training_Presentation_9_2009   gr123  fontwork-arch-up-curve  fill=solid
pres_ioc_phuket                                        gr75   fontwork-plain-text     fill=bitmap
                                                                                      image=Bitmap_1 repeat=stretch
2 text-path shapes, in 2 of the 51 .ppt
```

**A correction to the register.** The phuket title is *not* "a gradient clipped to the glyph
outlines". 26.2.4.2's own export states `draw:fill="bitmap"` with `style:repeat="stretch"` on
`gr75` — a stretched **bitmap**. It matters because it decides which half of the job the shape
belongs to.

**The job is two halves and only one of them is the Escher reader.**

1. **The reader.** `PptSlideLayout` would set `SlideTextBody.WarpFontworkType` and the adjustment
   values the way `OdpSlideLayout` already does
   (`XRay.Office.Presentations/OpenDocument/OdpSlideLayout.cs`:646-662) — so the engine really is not
   the obstacle, and a second format already reaches it by the ODF-named route. The pieces are the
   ones the register names: `gtextUNICODE` (192), `gtextAlign` (194), `gtextSize` (195),
   `gtextFont` (197) and the boolean group `gtextFStrikethrough` (255), whose bit `0x4000` is
   `TextPath`, read the way `filter/source/msfilter/msdffimp.cxx`:2517-2571 does, plus the
   shape-type → preset-name table at
   `svx/source/customshapes/EnhancedCustomShapeTypeNames.cxx`:171 and :179 — opened in this
   checkout and confirmed to be the `fontwork-*` name list, which is r97's own correction and
   stands.
2. **The fill, which is not a `.ppt` problem.** `SlideFontwork.Read` ends
   `return … new Drawing(outline, Paint.Solid(stated.Colour))`
   (`XRay.Office.Presentations/Ooxml/SlideFontwork.cs`:106), and `PptxSlideLayout` says so in as many
   words — *"the first run's colour becomes the fill, the shape's own fill, pen and shadow go"*
   (`PptxSlideLayout.cs`:933-935). **No path in this tree paints a Fontwork with anything but a
   solid run colour**, so a `.pptx` or an `.odp` stating a bitmap- or gradient-filled WordArt is
   wrong in exactly the same way and through exactly the same code.

**Left seated, sharpened, and the brief's own escape taken.** Wiring the reader without the fill
would draw the phuket title as a solid yellow bar where the reference draws a stretched bitmap
clipped to the glyphs — *something different again*, which is what the brief asked not to land for
a reach this small. The seat is therefore two seats: an Escher `TextPath` reader whose reach is
**2 shapes in 2 of 51 `.ppt`**, and a non-solid Fontwork fill in `SlideFontwork` whose reach is
every text-path shape in every format that states one — a number this round did not measure and
which is *not* two.

---

# 4. The suite

Read out of this run's own output rather than from the briefed number — `dotnet build
XRay.Office.slnx -c Release` at **0 warnings, 0 errors**, then `dotnet test XRay.Office.slnx -c Release
--no-build`, every one of the eleven projects reached (`tests.log`, kept beside this file):

| project | |
|---|---|
| Containers | 109 / 109 |
| Core | 521 / 521 |
| Markup | 259 / 259 |
| OpenDocument | 146 / 146 |
| **Presentations** | **1047 / 1047** |
| Rendering | 164 / 164 |
| Spreadsheets | 1280 / 1280 |
| Text | 728 / 728 |
| Vector | 309 / 309 |
| WordProcessing | 1924 / 1924 |
| **Fidelity** | **542 passed / 10 failed of 552, 0 skipped** |

The ten are exactly the briefed baseline and nothing else: `PageDrawing` ×4, `TabStop` ×4,
`SheetDrawing` ×1, `JustificationShrink` ×1. **No eleventh.**

Presentations is **1047** against r97's 1045; the two are `SlideTrailingBlankLineHeightTests`.
**Measured rather than asserted:** with `SlideTextLayout.cs` reverted to `1acd84bea` and the test
file left in place, `ATrailingBlankIsMeasuredAtItsOwnSize` **fails** and its control
`ATrailingBlankAtTheTextsOwnSizeChangesNothing` passes — 1 failed, 1 passed of 2 — and both pass
with the source back.

# 5. The files

| | |
|---|---|
| `tfy.py` | the instrument: font, size and **baseline** of every text show, from the content stream's own `Tf`/`Td`/`Tm`/`T*` operators. No bounding box anywhere in it |
| `ops-check.txt` | `tfy.py` against `pdf-ops.py` on one page, twelve shows, to the last digit |
| `sweep.sh` | our half of the `.ppt` column rendered and reduced to a show table, one document at a time, deleted as it goes |
| `bulletcensus.py`, `bullet-census.txt` | every paired OpenSymbol bullet in the corpus — the size distribution of §2.1 |
| `bulletcensus2.py`, `bullet-census-agreeing.txt` | the same restricted to pages whose whole show sequence agrees, **with the body's rate on the same pages beside it** — §2.2 |
| `textpath-census.txt` | every shape 26.2.4.2 makes a Fontwork of, over all 51 `.ppt`, **with the fill it gives each** |
| `gate-base.tsv`, `gate-head.tsv` | the gate over the 51 `.ppt` at both binaries, against the banked 26.2.4.2 reference |
| `ink.sh`, `ink-base.tsv`, `ink-head.tsv` | summed `\|ink\|%` per document against the same bank |
| `sizes-base.tsv`, `sizes-head.tsv` | the per-page dominant drawn size at both binaries (`sizes-ref.tsv` is r97's, from the same bank) |
| `reach.sh`, `reach-pptx.tsv` | the change reaches every presentation format, so a `.pptx` sample measured base against head |
| `witness-p26.txt` | the three white boxes of `pres_ioc_phuket.ppt` p26 at both binaries and at the reference, and the text shows beside them |
| `tests.log` | the suite |
