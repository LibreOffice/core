# The line EditEngine *appends* is measured by a different function, and that is the 0.32 pt

Round 85, on the slides track. One rule, two seats, and two refutations of the brief that sent
me — one of them the brief's own first suspect.

The defect: a `.ppt` outline placeholder is drawn at 32 pt where 26.2.4.2 draws 30. Round 84
instrumented it and left a measured seat — *row 0 of the fit table fits the box by 113 units of
1/100 mm, 0.875%, where the reference overflows and takes row 1.* That 113 is now accounted for
exactly, and it is not in `SlideAutofit` at all.

## Environment

| | |
|---|---|
| base commit | `6a6f18d9a`, worktree `/home/user/wt-pptfit2`, branch `agent/pptfit2` |
| reference | `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2** `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| `/usr/bin/soffice` | 24.2.7.2 — **not used for anything in this round** |
| corpus | `/home/user/sample-files`, the original 947; `/home/user/corpus-odf`, the converted 1285 |
| reference banks | `/home/user/gate-orig-r83/ref` (947 PDFs, rendered 2026-09-08 13:46–14:08) and `/home/user/gate-odf-r80/ref` (1272, `.odp` column rendered 2026-09-08 03:44) — both **after** the 2026-09-07 font move, so both are valid here |
| C++ tree | `/home/user/libreoffice-core`, read only |
| fonts | all five tarball confounds aside, as they have been since 2026-09-07 |
| workers | 2 for every sweep, 3 for the reference-bank census (which renders nothing) |

**Every C++ citation below was opened and read in this round**, in this checkout, and every line
number is this checkout's.

**No build overlapped a sweep.** Base and head binaries were built by `cp`-ing the source aside,
`git checkout --`, `touch`, and `rm -rf obj bin` on `XRay.Office.Presentations` and
`XRay.Office.Cli` before each build — never `git stash`, which is repository-global and this clone
has many worktrees. The instrumentation is **proved out**: it lives in
`autofit-trace.patch` and not in the tree, and the final binary re-renders all **51 of 51**
`.ppt` byte-identically to the head sweep taken before the doc-comments landed
(`final-ppt` against `head-ppt`, `SOURCE_DATE_EPOCH` pinned on both).

**Disk.** `/` ran from 7.5 GB free to 4.3. Every sweep here renders one document at a time,
takes its columns and its digest, and unlinks the PDF.

---

# 1. The rule

## An appended line is not an ordinary line

`ImpEditEngine::CreateAndInsertEmptyLine` (`editeng/source/editeng/impedit3.cxx`:1851-1996)
builds **one extra line**, and it is reached twice:

- from `createLinesForEmptyParagraph` (`:514-524`), which `CreateLines` short-circuits to for a
  paragraph of no characters (`:668-670`);
- from the end of `CreateLines` (`:1843-1844`) when the paragraph's last line ended on an
  `EE_FEATURE_LINEBR` portion (`bLineBreak = true`, `:1088-1094`).

That function carries **its own copy** of the line-spacing arms, and the copy is not the same
arithmetic as the ordinary one at `:1528-1602`:

| | ordinary line (`:1528-1602`) | appended line (`:1912-1972`) |
|---|---|---|
| `SvxLineSpaceRule::Min` | `fround(scaleYSpacingValue(GetLineHeight()))` | `rLSItem.GetLineHeight()` — **raw** |
| `SvxLineSpaceRule::Fix` | same, scaled | raw |
| `SvxInterLineSpaceRule::Prop` | `fround(GetHeight() × prop × fSpacingY)` | `nH = nTxtHeight × prop / 100` — integer, **truncating**, and **no `fSpacingY`** |
| `SvxInterLineSpaceRule::Off` | `fround(GetHeight() × fSpacingY)`, `:1583-1600` | **the arm does not exist** |
| guard | none | `Prop` only, `if (nPara \|\| pTmpLine->GetStartPortion())` — "Not the very first line", `:1953` |

`scaleYSpacingValue` is `impedit.hxx`:797-801 and is the only route the shrink-to-fit's
`fSpacingY` takes into a height. **It appears nowhere in `CreateAndInsertEmptyLine`.** So an
empty paragraph's line, and the line a trailing hard break appends, keep their full height while
every other line in the same body is tightened.

The `nPara` guard reduces to the paragraph index: a freshly constructed `EditLine` has
`mnStartPortion = 0` (`editeng/inc/EditLine.hxx`:39) and `SetStartPortion` is called at the
*end* of the function (`:1991`), so `pTmpLine->GetStartPortion()` is zero at the test in both
cases. **The body's first paragraph gets no proportional scaling on its appended line at all.**

## And the paragraph's own space truncates, twice

`ImpEditEngine::CalcHeight` (`editeng/source/editeng/impedit2.cxx`:4763-4860) is where a
paragraph's `SvxULSpaceItem` reaches the block height, and it drops fractions in both places:

```cpp
sal_uInt16 nUpper = scaleYSpacingValue(rULItem.GetUpper());   // :4794 — double -> integer
rPortion.mnHeight += nUpper;
...
rPortion.mnHeight += scaleYSpacingValue(rULItem.GetLower());  // :4801
```

The unscaled value is an integer already, because a `SvxULSpaceItem` holds it in the model's own
map unit — a hundredth of a millimetre for a draw object. So the space is **quantised before the
scale and truncated after it**, where this tree kept the exact EMU and rounded.

The same function is where *"the outer two spacings do not count"* comes from (`:4792` and
`:4799`), which this tree already had.

## The witness, and how the two rules were established rather than read

`slides/done-013/ppt/2015-Civil-Rights-Website-training.ppt` page 2. Its outline placeholder is
13124 units tall with 127-unit insets — 12870, and the fit compares against 12871 because
`tools::Rectangle::GetSize()` counts both edges. `soffice --convert-to fodp` gives the model
directly: `fo:line-height="90%"`, `fo:margin-top="0.282cm"` (282 units), 32 pt runs,
`style:shrink-to-fit="true"`, `style:font-independent-line-spacing="true"`. Eight paragraphs —
four with text, three empty, one ending on a hard break.

26.2.4.2 draws it at **29.991 pt** with bullet baselines at 175.663 / 251.575 / 327.487 /
403.398 / 472.139. Three of those gaps are **75.912 pt** and the last is **68.741** — the last
crosses one fewer paragraph space. Two equations in two unknowns:

    2·(line + space)      = 75.912        line + space = 1339 units
    (text + appended) + space = 68.741    space = 253, text + appended = 2172

and the first baseline pins the ascent: text top is 5630 units = 159.581 pt, the first text
baseline is 182.920, so the ascent is 823 units — which is exactly
`fround(1270 × 0.9 × 0.9 × 0.8)`, the ordinary `Prop` arm's `nNewAscent` at row 1 of
`constScaleLevels`. That fixes the natural line at 1270, the paragraph's proportion at 0.9 and
`fSpacingY` at 0.9 with no freedom left, and the split of 2172 then follows:

| | units | arithmetic |
|---|---:|---|
| text line | **1029** | `fround(1270 × 0.9 × 0.9)` |
| appended line | **1143** | `1270 × 90 / 100`, no `fSpacingY` |
| paragraph space | **253** | `trunc(282 × 0.9)`, where `fround` gives 254 |

`1029 + 253 + 1143 + 253 = 2678` units = 75.912 pt, and `1029 + 1143 + 253 = 2425` = 68.741.
**Both to three decimal places, with no free parameter.**

The independent check is the *absolute* position of the last baseline, which none of the above
was fitted to: stacking those numbers from the text top puts it at 479.385 against the
reference's **479.395** — 0.010 pt, which is the export's own uniform offset.

## What that does to the fit — the 113 units, accounted for

`ScaleContentToFitWindow` (`impedit3.cxx`:303-333) formats unscaled, and while the height
overflows walks `constScaleLevels` (`:286-300`) taking the first row that fits. Traced on the
witness (`autofit-trace.patch`):

| level | before | after | box |
|---|---:|---:|---:|
| unscaled | 14176 | 14174 | 12871 |
| row 0 `{1.000, 0.900}` | **12758 — fits by 113** | **13239 — overflows** | 12871 |
| row 1 `{0.925, 0.900}` | 11039 | **11488 — fits** | 12871 |

Round 84's 113 units is the four appended lines being tightened: `4 × (1220 − 1098) = 488`, less
the seven spaces gaining a unit each. The tree now takes row 1, draws 30 pt, and **every text
baseline on that page agrees with 26.2.4.2 to 0.001 pt** — 182.920, 258.832 against 258.831,
334.743, 410.655, 479.395.

The bullets do not: we draw 17.773 pt at y 175.011 where the reference draws 17.802 at 175.663.
That is `Outliner::ImpCalcBulletFont`'s own size rule and the bullet's own vertical placement,
one unit and 0.65 pt respectively. **Left with its seat**, `editeng/source/outliner/outliner.cxx`:851-855.

## The change

Three seats, all in `XRay.Office.Presentations`:

- **`SlideTextLayout.Appended`** (new) — the four arms above, with the paragraph-index guard.
- **`SlideTextLayout.EndSize`** (new) — the em an appended line after a break is measured at is
  the paragraph's *last* run's, because `SeekCursor(pNode, bLineBreak ? Len() : 0, aTmpFont)`
  (`:1896`) seeks to the paragraph's end for that case and to zero for an empty paragraph.
- **`SlideTextLayout.ScaledSpace`** — quantise, then truncate.

`Measure` marks a line as appended when it is the paragraph's last **and** its character range is
empty, which is exactly the two routes above: a wrapped continuation always has characters, and a
paragraph ending on a break gets a zero-length final line.

The bullet area's ability to raise such a line (`:1974-1985`, halving the difference into the
ascent) is deliberately **not** modelled — no measured case needs it and the witness is
reproduced without it.

---

# 2. What moved

## The gate — no verdict in either direction

`sweep-ours.sh` re-renders our half and scores it against the banked 26.2.4.2 reference with
`batch-check.sh`'s own rule, which is sound because the diff is confined to `dotnet/src` and
cannot reach `soffice`. **The verdict is column 9, `glyphs` — the alphanumeric character count —
and not column 4, `words`**; I read `batch-check.sh`:279-295 to confirm it rather than taking the
brief's word, and `score.py` carries the same note.

| column | rows | match, base | match, head | renderings differing | page counts moved |
|---|---:|---:|---:|---:|---:|
| `.ppt` | 51 | 49 | **49** | 51 | 0 |
| `.pptx` | 251 | 244 | **244** | 103 | 0 |
| slides, both | 302 | 293 | **293** | 154 | 0 |
| converted `.odp` | 302 | 295 | **295** | 100 | 0 |

The base sweep reproduces `probes/orig-gate-r83/rows.tsv`'s slides verdicts on **302 of 302**,
which is what validates the instrument. Seven glyph counts move across the two columns and none
crosses the band; five of the seven move *towards* the reference, two of them to exact
(`manufacturing_process_simulation…` 4793 → 4733 of 4733, `NWD-GLA-…pptx` 3998 → 3957 of 3957).

## The tracks not targeted do not move

The diff is one file that only `XRay.Office.Presentations` compiles, and neither
`XRay.Office.WordProcessing` nor `XRay.Office.Spreadsheets` sits below it — `git grep SlideTextLayout`
over `dotnet/src` finds three mentions outside that library and all three are doc-comment prose.
Measured anyway: every tenth words and sheets document, **64 of 64 renderings byte-identical**
between base and head with `SOURCE_DATE_EPOCH` pinned (`sample-base.tsv`, `sample-head.tsv`).

## The thing the gate cannot see: the drawn sizes

An autofit is stated nowhere a reader can see, so the measurement is the size actually drawn.
`size-sweep.sh` takes each page's dominant text size — the size carrying the most alphanumeric
characters, round 84's own statistic — for both our binaries and for the banked reference.

**4530 pages of 302 documents, none failing on either side:**

| | base | head |
|---|---:|---:|
| pages whose dominant size differs from 26.2.4.2 by more than 0.15 pt | **56** | **41** |
| documents holding one | 39 | 33 |
| total \|size error\| over all 4530 pages | **222.92 pt** | **159.96 pt** |
| fixed | — | **15** |
| newly wrong | — | **0** |

The 15 are 6 `.ppt` and 9 `.pptx`. On the `.ppt` column alone the census is **25 pages in 18
documents** before, which reproduces round 84's figure exactly, and 19 after.

---

# 3. What this brief got wrong, and what round 84 got wrong

1. **"`SlideTextLayout.Spaced` scales every line's height by `fSpacingY`. Check whether
   LibreOffice does that to every line or only between lines."** Neither. `Spaced` is a faithful
   transcription of the `Off` arm and LibreOffice does apply `fSpacingY` to **every ordinary
   line's** height, `:1583-1600`. What it does not apply it to is the *appended* line, which is
   measured by a different function with no such arm. The asymmetry is real and is not the one the
   brief named.

2. **"`SlideAutofit` is the wrong place to look, by 0.32 pt."** The first half is right and is the
   round's most useful sentence; the second half attributes the error to the fit. `SlideAutofit`
   is correct as it stands — the table, the walk, the comparison, the slack and the device grid
   all reproduce the reference. What was wrong is the *height it was handed*, in
   `SlideTextLayout`.

3. **"`Autofits`' wrap test is separately wrong for an outline placeholder — reach 55 shapes in
   2 documents, and the seat's own comment denies it."** **The seat's comment is right and round
   84's contradiction of it is withdrawn.** The reference derives `bAutoGrowWidth = !bWordWrap`
   only for a shape that is an `SdrObjCustomShape` *and* whose text kind was rewritten to
   Rectangle (`svdfppt.cxx`:1053-1055); every other branch sets it false (`:1084`). The rewrite
   happens on exactly one condition — `!aTextObj.GetOEPlaceHolderAtom() || nPlaceholderId ==
   PptPlaceholder::NONE` (`:1043-1047`). So the two rules can only disagree on a Body-kind text
   that *names itself a placeholder* and states `wrapNone`, and
   **`placeholder-census.py` finds none: all 55 of the `wrapNone` Body shapes carry no
   `OEPlaceholderAtom` at all.** Corroborated at the reference rather than left as a reading — over
   every page of both decks the set of drawn text sizes matches 26.2.4.2 exactly, the sole
   exception being two size classes on page 6 of `Fundamentals_Module_1_basics.ppt`.

4. **And that page 6 is not a wrap case either.** Round 84 called its "7.08 against 32.00" one of
   three ratios that are no `constScaleLevels` step. It is an **embedded chart** whose axis labels
   we draw as text and 26.2.4.2 does not draw at all: every placeholder on that page agrees to
   0.06 pt and 40.00 / 32.00 exactly. A dominant-size census cannot tell a fit apart from a chart,
   and this is the second time in two rounds that statistic has named the wrong document.

5. **"29.99 pt is `round(32 × 0.925)`, a real row of the shrink table."** Confirmed, and the
   stale 31.01 / 0.9691 is confirmed stale. Nothing to correct.

6. **Every corpus denominator was re-censused** rather than quoted, as instructed: `.ppt` 51,
   `.pptx` 251, slides 302, converted `.odp` 302, and the `.ppt` size census 25 pages in 18
   documents. The `.ppt` census was re-run through the persist directory
   (`persist.py`/`census.py`, round 84's), never through a stream scan.

## And one thing this tree says that is stale, found on the way and left

`SlideAutofit`'s remarks have said for many rounds that *"the stated scale is thrown away —
the reference reads `a:normAutofit/@fontScale` into `TextBodyProperties::mnFontScale` and then
never reads that field again"*. **That is a claim about 24.2 and it is no longer true.**
`oox/source/drawingml/textbodypropertiescontext.cxx`:242-243 sets `PROP_TextFitToSizeFontScale`
and `PROP_TextFitToSizeSpacingScale`; `svx/source/unodraw/unoshape.cxx`:2343-2367 puts them on
the `SdrTextFitToSizeTypeItem`; and `SdrTextObj::setupAutoFitText`
(`svx/source/svdraw/svdotext.cxx`:1238-1247) hands them to the outliner **instead of searching**,
after which `ScaleContentToFitWindow` walks the table only if that first format overflows.

**The gate on it is `@lnSpcReduction`, not `@fontScale`**: the spacing scale is
`1.0 - lnSpcReduction/100000` and the attribute's own default is 100000, so an element stating
`fontScale` alone yields a spacing scale of zero, fails the `fSpacingScale > 0.0` guard, and is
searched from scratch exactly as before. Censused over the corpus: **326 `a:normAutofit` state
`fontScale`, 209 state `lnSpcReduction`, the latter in 40 of the 251 `.pptx`.** A `.ppt` cannot
reach it — `svdfppt.cxx`:1099 builds the item from its type alone and both scales default to zero
(`include/svx/sdtfsitm.hxx`:68-69).

**Read from the source and censused; not measured at the reference.** It is a second change with
its own before/after and this round did not have the budget for it. Whoever takes it should
measure before implementing: honouring a scale PowerPoint computed against PowerPoint's metrics
is exactly the thing the paragraph above warns about, and the fact that 26.2 reads it does not
by itself say the drawn answer follows.

---

## The suite

Every project run individually, at this round's head.

| project | base `6a6f18d9a` | this round |
|---|---:|---:|
| Containers | 109 | 109 |
| Core | 519 | 519 |
| Markup | 259 | 259 |
| OpenDocument | 143 | 143 |
| Presentations | 1039 | **1044** |
| Rendering | 164 | 164 |
| Spreadsheets | 1226 | 1226 |
| Text | 728 | 728 |
| Vector | 302 | 302 |
| WordProcessing | 1787 | 1787 |
| **total** | **6276** | **6281** |

0 failed and 0 skipped everywhere. The five new ones are `SlideAppendedLineTests`. Two existing
expectations moved with the space rule and both are the *wiring* rather than the fit —
`SlideAutofitTests.TheFitsSpacingScaleReachesAParagraphsOwnSpace` 424 → 423 and 381 → 380 (its
third row, 338, is the control that does not move), and
`SlideParagraphSpacingTests.TheFirstSpaceBeforeAndTheLastSpaceAfterAreNotCounted`, which now
asserts in the draw layer's own unit because that is the unit the item is held in. Both carry the
citation and the measurement in their remarks.

`XRay.Office.Fidelity.Tests`: **542 passed / 10 failed of 552, 0 skipped** — the briefed baseline
exactly, and the same ten (PageDrawing ×4, TabStop ×4, SheetDrawing, JustificationShrink).

---

## The scripts

| | |
|---|---|
| `sweep-ours.sh` | our half of a track, scored against a banked 26.2.4.2 reference, digested and unlinked |
| `size-sweep.sh` | per-page dominant drawn text size; with `BANK` set it censuses the banked reference instead of rendering |
| `sizes.py` | the dominant size of every page of one PDF (round 84's) |
| `hash-list.sh` | a list of documents rendered and digested, for a confinement check |
| `score.py` | the two comparisons above, with the column-9 rule spelled out |
| `baselines.py` | every drawn span of a page: baseline, x, size, colour, face, text (round 84's) |
| `wrap-census.py` | non-wrapping outline placeholders (round 84's, reproduced) |
| `placeholder-census.py` | **the missing half of it**: does such a shape carry an `OEPlaceholderAtom`? |
| `persist.py`, `census.py`, `paths.py` | round 84's live-record-tree walk, without which a `.ppt` census counts orphans |
| `autofit-trace.patch` | the instrumentation, kept out of the tree and applied only to trace |

Banked figures: `slides-base.tsv`/`slides-head.tsv` and `odp-base.tsv`/`odp-head.tsv` (the four
sweeps, 302 rows each, digests included), `sizes-ref.tsv`/`sizes-base.tsv`/`sizes-head.tsv` (4550,
4530 and 4530 pages of dominant sizes), `sample-base.tsv`/`sample-head.tsv` (the 64-document
confinement check), `wrap-census.txt` and `placeholder-census.txt`.
