# A `.ppt` shape resizes itself around its own text, and the seat that hid it is a dropped paragraph

Round 97, slides track. Three seats — **O14 closes**, **O15 is characterised and stays open with a
correction to its own witness**, **O13 stays open with a reference-side census it did not have**.

> **This round was killed by a container restart and everything below was re-verified afterwards,
> adversarially, against a fresh clean build.** §6 lists what re-verified, and the six claims that
> did not survive it — two of them mine, and one of them a number this file asserted without ever
> having run the measurement. Read §6 before trusting a figure here; every figure that stands has
> the re-measurement named beside it.

## Environment

| | |
|---|---|
| worktree / branch | `/home/user/wt-slidefont`, `agent/slidefont`, base `61bc19e03` |
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**. `/usr/bin/soffice` (24.2.7.2) is used for nothing here |
| reference bank | `/home/user/gate-orig-r83/ref`, 947 PDFs, rendered 2026-09-08 — after the 2026-09-07 font move, so valid |
| corpus | `/home/user/sample-files`; the slides binary column is **51 `.ppt`** (`git ls-files slides \| grep -i '\.ppt$'`) |
| C++ tree | `/home/user/libreoffice-core`, read only, never built |
| fonts | the five tarball confounds aside, untouched during the round |

Every C++ citation below was opened in this checkout, and every one was opened a second time on
the re-verification pass. No build overlapped a sweep:
the base binary was made by copying the three changed sources aside, `git checkout --`,
`touch`, `rm -rf obj bin` on `XRay.Office.Presentations` and `XRay.Office.Cli`, and the head binary the
same way afterwards; both are kept whole under `/home/user/r97-slides/cli-{base,head}`.

---

# 1. O14 — the banner is 103.38 pt because the shape grows to its text, and closes

## 1.1 What the reference does

`pres_ioc_phuket.ppt` page 26's dark blue banner states a client anchor of **519 master units —
64.88 pt** — and 26.2.4.2 draws it **103.38 pt** tall. `--convert-to fodp` gives the reason
outright: the shape comes out as `svg:height="3.647cm"` under a graphic style stating
`draw:auto-grow-height="true"`, and it holds **two empty paragraphs** of 40 pt Arial Black.

The mechanism, read rather than inferred:

- **Escher states it as bit 1 of `DFF_Prop_FitTextToShape` (191)** —
  `include/svx/msdffdef.hxx`:119, the boolean group at 188-191. That shape's property 191 is
  `0x00020002`: the bit and its own "used" flag.
- **`svdfppt.cxx`:1049-1110 turns it into `SdrTextAutoGrowHeightItem` on every horizontal text
  object it builds.** The `SdrObjCustomShape` branch at `:1053-1055`
  (`bAutoGrowHeight = bFitShapeToText`) and the `SdrRectObj` branch at `:1090` both; a *vertical*
  text object gets auto-grow-**width** instead and no height growth at all (`:1077-1080`).
- **The arithmetic is `SdrTextObj::AdjustTextFrameWidthAndHeight`**
  (`svx/source/svdraw/svdotxat.cxx`:44-236) and its custom-shape twin
  (`svx/source/svdraw/svdoashp.cxx`:2249-2421), which agree line for line on the height. The text
  is broken to the frame's **open** width less the two horizontal distances, floored at two units
  (`svdotxat.cxx`:76-77, :110-116); the height is the outliner's **plus one unit of tolerance**;
  that is clamped to the minimum frame height; and only then are the two vertical distances added.
  The growth moves the edge the vertical anchor does not hold — bottom for a top-anchored body,
  top for a bottom-anchored one, half each way for a centred one (`:210-226`).
- **The minimum is where the two branches differ, and it decides whether a shape may *shrink*.**
  `makeSdrTextMinFrameHeightItem` is set only on the `SdrRectObj` branch
  (`svdfppt.cxx`:1116-1119), to `rTextRect.GetHeight() - (nTextTop + nTextBottom)` (`:982`) — so a
  real placeholder can only grow. A custom shape gets no such item, its minimum falls through to
  1, and a box with less text than room shrinks to fit. The branch is taken on one condition
  (`:1041-1055`): the text object's kind is rewritten to Rectangle, and the custom shape therefore
  keeps its own text, **unless** the shape names a placeholder *and* its text kind is not
  `TextInShape`.
- `ImpCalculateTextFrame` (`svdoashp.cxx`:2424-2450) scales the text frame's movement back onto
  the shape's own rectangle by `shapeHeight / textRectHeight`, which is the identity whenever the
  preset's text area is the whole shape.

`PptSlideLayout.Fitted` and `MinimumFrameHeight` are those rules. Nothing in
`XRay.Office.Presentations` had any answer for auto-grow-height at all before this — `git grep
AutoGrow` over `dotnet/src` found two prose mentions and no code.

## 1.2 The half that made the first cut draw 55.38 pt instead of 103.38

With the growth wired and nothing else, the banner came out **shorter** than the anchor states,
because our reader was measuring one line where the reference measures two. Two further rules,
both read out of the same function and both confirmed against 26.2.4.2's own resolved view:

- **A trailing return leaves an empty paragraph behind and it is a line.**
  `PPTStyleTextPropReader::Init` reads the string under `while (nCharReadCnt < nStringLen)` and
  then appends one more portion, in one more paragraph, whenever the last portion it made belongs
  to the paragraph before the counter — `svdfppt.cxx`:5403-5409, which is exactly the case where
  the text ended on a newline marker. `PptTextBody.Build` had dropped it since the reader was
  written, under the comment *"an artefact of the terminator rather than a paragraph the author
  wrote"*, and that comment was never measured. The banner's whole text is a single `\r` and the
  reference's fodp gives it **two** `<text:p>`.
- **That paragraph is measured in the run BEFORE it.** A `.ppt` states one more character run than
  it has characters, and PowerPoint writes something different in it: the banner's
  `StyleTextPropAtom` states two one-character runs, **40 pt then 8 pt**. The reference never reads
  the second — its loop stops at `nCharReadCnt < nStringLen` and the portion it appends is a copy
  of `aCharPropList.back()` — so both of the banner's lines are 40 pt. **Read straight out of the
  reference rather than reconstructed:** the fodp gives both `<text:p>` the text style `T62`,
  which is `fo:font-size="40pt"` Arial Black, and the shape's graphic style `gr74` carries
  `draw:auto-grow-height="true"` with `fo:padding-top`/`-bottom` of 0.13 cm. One line of 40 and
  one of 8 would be about 2.29 cm, which is the anchor and not what the reference draws.
  *(An earlier draft of this section decomposed 3.647 cm as `2 x fround(1411 x 1.2) + 2 x 127`
  hundredths of a millimetre. **That is 3640 and the number is 3647**, so the 1.2 gloss is wrong:
  the measured line is 1696 units, a ratio of 1.2020. The decomposition is withdrawn; the two
  40 pt paragraphs are read out of `T62` and need no arithmetic.)*
- **A `PageTitle`'s returns are line breaks, not paragraph ends**, and the two go together: keeping
  the trailing paragraph without this would give every title ending in a return a spurious empty
  one. `Init` rewrites every `0x0d` to `0x0b` when the text header names instance 0 and records a
  `PPT_SPEC_NEWLINE` marker for every other instance (`svdfppt.cxx`:5241-5246 for the Unicode
  record, :5261-5266 for the byte one); only the markers split paragraphs. `TSS_Type::PageTitle`
  is 0 and `TSS_Type::Title` — the title of a *title slide* — is 6
  (`include/filter/msfilter/svdfppt.hxx`:157-169), so following the name rather than the number
  would apply it to the wrong one. `PptTextReader.Broken`.

## 1.3 The witness, after

| | anchor | 26.2.4.2 | base | head |
|---|---:|---:|---:|---:|
| `pres_ioc_phuket.ppt` p26 banner, height | 64.88 pt | **103.38** | 64.88 | **103.37** |

Read out of the two PDFs' own content streams: the reference's fill is
`(-18.03, 436.65)-(719.97, 540.03)` and ours is now `(-18.00, 436.63)-(720.00, 540.00)` — heights
103.38 and 103.37, positions agreeing to 0.03 pt. **Re-measured after the restart from a fresh
clean build and a fresh read of the banked reference: the same two rectangles.**

**And the same page carries a shape the fit makes worse.** Its white box is
`(83.96, 77.98)-(654.01, 170.62)`, 92.64 pt, at the reference; the base drew 92.38 and the head
draws **81.77**. The reference's own style for it (`gr78`) states `draw:auto-grow-height="true"`
and `fo:min-height="0cm"`, so 26.2.4.2 *would* let it shrink and does not — because its outliner
measures a taller block than ours does. That is O15's "our block height is short of the
reference's" showing up in geometry instead of in a font size, and it is the price of the shrink
arm: the arm turns a hidden height-measurement error into a visible one. The page still improves,
4.28 → 2.04 `|ink|%`.

## 1.4 Reach

**On the file side**, over the 51 `.ppt`'s live persist directories: **1691 slide shapes in 38
documents state `fFitShapeToText`**, of which **1348 hold a `ClientTextbox`**
(`fitshape-census.txt`, `growth.py`'s `shapes.py`).

**On the reference side**, comparing each shape's stated Escher anchor against the height 26.2.4.2
resolves for it in its own flat-ODP export, matched on (x, y, width) — the three coordinates a
top-anchored grow leaves alone — **101 shapes in 22 documents differ by more than a millimetre, 83
taller and 18 shorter**, 30 of them by more than a centimetre (`growth.py`, `growth.txt`). The
largest are `Fundamentals_Module_1_basics` −6.978 cm, `undp_presentation_revised_17_may` +3.437 and
`RRM-training-syllabus…` +3.072.

*What that census cannot see, stated properly.* Of the **1667** grow-flagged shapes in the
flat-ODP exports it only matched **878**; **789 went unmatched** and are outside the figure
entirely. So the census sees 53 % of the population, not all of it. One named reason is rotation —
it matches on `svg:x`/`svg:y`, which a rotated shape's export does not state — and the rotation
compensation at `svdotxat.cxx`:228-236 is left unmodelled, unmeasured rather than measured as nil.
The other 789 are not all rotated and nothing here says what they are.

*A second gap, in the code rather than in the census.* `MinimumFrameHeight` discriminates on
`placeholder && kind != Other`. The reference discriminates on which branch built the text object:
`svdfppt.cxx`:1053 takes the custom-shape branch only when `pRet` **is** an `SdrObjCustomShape`
*and* `eTextKind == Rectangle`, and every other case builds an `SdrRectObj`, which gets the
min-height item at `:1118-1119`. A text-bearing shape that is not a custom shape — a picture, a
line — with `fFitShapeToText` and no placeholder therefore may shrink here and may not at the
reference. **Unmeasured**: no census here separates custom shapes from the rest.

## 1.5 What moved

**The gate — no verdict in either direction.** Our half of the `.ppt` column re-rendered at both
binaries and scored against the banked 26.2.4.2 reference with `batch-check.sh`'s own rule (column
9, `glyphs`, within max(2 %, 15)):

| | base | head |
|---|---:|---:|
| documents | 51 | 51 |
| `match` | **49** | **49** |
| page counts moved | — | **0** |
| alphanumeric counts moved | — | **0** |
| sum \|glyph distance\| | 1320 | **1320** |
| renderings whose bytes changed | — | **20** |

`gate-base.tsv`, `gate-head.tsv`.

**Ink, on the 20 that moved**, `|ink|%` summed over the pages of each against the same bank
(`ink.sh`, `ink-base.tsv`, `ink-head.tsv`):

| | base | head |
|---|---:|---:|
| sum \|ink\|% over the 20 | **118.08** | **103.08** |
| MAJOR pages | 37 | **29** |
| improved / worsened / level | — | **11 / 2 / 7** |

The two that worsen are `ws_prod-g-doc-Events-industrymeeting18112004-European-Safety-Strategy-Initiative`
(+0.36) and `0335fab9-79f0-4944-b92c-f223837ca2d8` (+0.01). The largest gains are
`undp_presentation_revised_17_may` 19.76 → 10.70, `pres_ioc_phuket` 7.78 → 4.77,
`pods05` 8.47 → 7.20 and `8.16_AOD_FINAL…` 21.69 → 20.42.

**Reach is the whole `.ppt` population, not a sample.** `git ls-files` over the corpus finds
**exactly 51 `.ppt` in 963 tracked files and none outside `slides/`**, and no `.pps` or `.pot`; so
the 51 rendered at both binaries above are every document the change can reach through that
reader.

**Confinement.** The diff is three files that only the binary presentation path compiles;
`git grep` over `dotnet/src` finds `PptTextBody`, `PptTextReader` and `PptSlideLayout` named
outside `MsBinary/` three times and all three are doc-comment prose. Measured: every 79th non-`.ppt`
corpus document, 12 of 12 byte-identical — and then **widened on the re-verification pass to every
tenth non-`.ppt` document, 90 of them (30 `.docx`, 29 `.xlsx`, 25 `.pptx`, 4 `.doc`, 2 `.xls`):
90 of 90 renderings byte-identical** between base and head with `SOURCE_DATE_EPOCH` pinned, none
failing (`confine2.sh`, `confine2-list.txt`, `confine2-base.tsv`, `confine2-head.tsv`).

*What is not a confinement instrument, tried and discarded:* comparing the two legs' assembly
bytes. Two full rebuilds of the identical tree produce fourteen differing first-party DLLs while
rendering byte-identical output, so the build is not reproducible at the assembly level and a DLL
comparison says nothing.

## 1.6 The three tests that fail at the base

Measured, not asserted: the three source files reverted to `61bc19e03` with the head test files
left in place, rebuilt, and `XRay.Office.Presentations.Tests` run — **3 failed / 1042 passed of
1045**, and 1045 of 1045 with the head sources back. The three are named below. *(This section
said "two" and then listed three; the third is real and the heading was wrong.)*

- `PptBlankParagraphTests.ATrailingEmptyParagraphIsALine` — the inverted assertion, with the
  citation and the fodp measurement in its remarks. Its sibling
  `TheTrailingEmptyParagraphTakesThePrecedingRunsSizeAndNotTheOneAfterIt` is new and pins the 40/8
  pair.
- `PptOdfPlacementTests.TheSameDeckPlacesItsShapesIdenticallyThroughTheBinaryAndOdfPaths` — **its
  premise has one measured exception now.** `--convert-to fodp` on the two fixtures gives the
  deck's ellipse `svg:height="2.067cm"` out of the `.ppt` and `"2.012cm"` out of the `.odp`, and
  the reason is in the graphic styles the reference writes beside them: the binary shape's carries
  `draw:auto-grow-height="true"` and the ODF shape's does not — **both re-converted with 26.2.4.2
  on the re-verification pass and both confirmed, along with which shape it is** (page 2, the
  `draw:type="ellipse"` at `svg:width` 7.996 cm / 8 cm and `svg:y` 6.002 cm / 6 cm). So the two
  vocabularies do *not*
  describe the same rectangle there, and pinning the binary side to the ODF side pinned it 1.55 pt
  short of what the reference draws. The height of that one shape is now asserted against
  26.2.4.2's own 2.067 cm; every other coordinate of every other shape is unchanged.

---

# 2. O15 — the register's witness is the bullet, and the census is 19 pages whose mechanism is still a reading

**Not closed. The census is measured and stable; its *mechanism* is not, and the two censuses
this section originally offered for it were withdrawn on re-verification (§6.2).**

## 2.1 The two numbers in the register are two different things

O15 reads *"41 `.ppt`/`.pptx` pages still differ on the dominant drawn size across 33 documents;
the witness bullet is 17.773 pt at y 175.011 against 17.802 at 175.663"*. Those are not the same
defect and the second is not a witness for the first:

- **17.773 / 17.802 is one hundredth of a millimetre.** 17.802 pt is 628 units and 17.773 is 627;
  the model's own quantisation step is one unit. It is the *bullet* of
  `2015-Civil-Rights-Website-training.ppt` page 2, whose text baselines round 85 already agrees
  with 26.2.4.2 on to 0.001 pt, and its seat is `Outliner::ImpCalcBulletFont`
  (`editeng/source/outliner/outliner.cxx`:851-855) — one multiplication and one
  `basegfx::fround`, already transcribed in `SlideAutofit.ScaledMarker`. **Calling it "a 0.16 %
  size difference across a third of the deck corpus" conflates it with the census.** The 0.65 pt
  is that bullet's own vertical placement and is likewise a bullet question.
- The census is not 0.16 % of anything: its differences are **whole points**, 0.99 to 24.92 pt.

**The cheapest proof that they are not the same defect, added on the re-verification pass**
(`bullet-p2.txt`). The census page for that deck is **page 22** — ours 17.01 against the
reference's 14.00. The register's bullet is on **page 2**, whose dominant drawn size is **29.99 on
both sides**: page 2 agrees, is not in the 19-page census, and could not be. They are different
pages of the same document.

And on page 2 the two numbers are themselves separable, read out of the content stream's own
`Td`/`Tm` operators rather than out of any bounding box:

| | size | baseline, from the page top |
|---|---:|---:|
| bullet, ours | 17.773 pt | 175.011 |
| bullet, 26.2.4.2 | 17.802 pt | 175.663 |
| body text beside it, ours | 29.99 pt | 182.920 |
| body text beside it, 26.2.4.2 | 29.99 pt | **182.920** |

The same 0.652 pt on all five of the page's bullets, and the body baseline identical to the last
digit. **0.652 pt is twenty-two times the 0.029 pt size difference**, so the one-unit rounding
does not produce the offset and the offset is not evidence for the rounding. Three separable
facts, not one.

## 2.2 The `.ppt` half of the census, re-measured at this round's head

Per-page dominant drawn text size — the size carrying the most alphanumeric characters, round 84's
statistic — for all 1534 pages of the 51 `.ppt`, our two binaries against the banked reference
(`size-sweep.sh`, `sizes-ref.tsv`, `sizes-base.tsv`, `sizes-head.tsv`):

| | base | head |
|---|---:|---:|
| pages whose dominant size differs by more than 0.15 pt | **19** | **19** |
| documents holding one | 15 | 15 |
| total \|size error\| over 1534 pages | 71.10 pt | 71.10 pt |
| fixed / newly wrong | — | **0 / 0** |

**O14's fix moves none of them**, which is worth stating: the bodies on these pages do not end in a
return, so neither the kept paragraph nor the grown shape reaches them.

## 2.3 What those 19 pages are

Two facts split them, and neither is fitted.

**Character counts.** 13 of the 19 draw the *same* number of alphanumeric characters on both sides,
so the statistic is comparing the same text at two sizes. The other 6 do not, so on those it is
comparing different text and the "size difference" is partly or wholly a content difference —
`W3_Case_Study` p10 and `Thailand17` p8 are 500 characters against 92.

**`style:shrink-to-fit`, and why it turns out to establish almost nothing.** Taken out of
26.2.4.2's own flat-ODP export of each document, asking only whether a style named on that page
states it (`shrink-census.txt`): **16 of the 19 pages carry a shrink-to-fit shape, and the three
that do not are three of the six character-count mismatches** — `W3_Case_Study` p10, `Thailand17`
p8 and `Thailand17` p11. **All 13 same-character pages carry one.** Both figures reproduce exactly
on the re-verification pass.

**But the base rate was never taken, and it is 79.3 %** (`shrink-baserate.py`,
`shrink-baserate.txt`). Of the 1515 scored pages whose dominant size *agrees*, **1201 carry a
shrink-to-fit shape**; restricted to pages with at least 30 alphanumeric characters, 1168 of 1427,
**81.9 %**. So:

| | carrying a shrink-to-fit shape |
|---|---:|
| every scored page (1534) | 79.3 % |
| pages whose dominant size **agrees** (1515) | 79.3 % |
| … of those, ≥ 30 alnum characters (1427) | 81.9 % |
| the 19 that differ | 84.2 % |
| the 13 whose alnum counts also agree | 100 % |

**84.2 % against 79.3 % is nothing.** 13 of 13 against 81.9 % is p ≈ 0.07 — suggestive, not
established. And the census is page-level: it cannot say that the shape carrying the *dominant
text* is the shape stating shrink-to-fit, only that some shape somewhere on the page does.

**So the sentence this section used to end on — "the same-character group is an autofit group,
established from the reference's own model rather than from a fit" — is withdrawn.** The census is
*consistent with* the autofit reading and does not distinguish it from any other reading. What
distinguishes the same-character group is the character counts themselves, which are a fact about
the two renderings and not about a model.

**The row, and it is weaker than "weak".** `autofit-rows.txt` searches, for each page, for an
integer stated size and two rows of `constScaleLevels`
(`editeng/source/editeng/impedit3.cxx`:286-300 — the array is there, verbatim, 12 rows starting
1.000, 0.925, 0.850) that give our drawn size and the reference's. **18 of 19 rows admit one**, and
the one that does not is `Fundamentals_Module_1_basics` p6, which round 85 already established is
an embedded chart whose axis labels the reference does not draw as text at all.

**The null was never taken either, and it is 71 %** (`rowfit-null.py`, `rowfit-null.txt`). Eleven
distinct scale levels and a free integer stated size are enough freedom that, over integer sizes
8…48, **87.9 % of arbitrary unequal pairs admit such a triple**, and **71.4 % of pairs within 4 pt**
— which is where all nineteen of these sit. 18 of 19 is 95 % against a 71 % null on 19 samples.
**The fit is close to vacuous and `autofit-rows.txt` should be read as a description of the rows,
not as a measurement.**

So neither corroborating census does the work the section wanted from it. What is left standing
for O15 is the 19-page census itself, its stability under O14, and the sign below — all three
of which are direct measurements of the two renderings.

**The sign.** On 12 of the 13 same-character pages **we draw the larger size**, so our search stops
earlier, so **our measured block height is short of the reference's**. The exception is
`gillikin_online_user_mtg_2010` p2, where ours is 31 against 34.

## 2.4 What is left on O15, sharpened

1. **The height our fit measures is short.** That is one direction and one mechanism family, and it
   is round 85's area (`ImpEditEngine::CreateLines`, the appended-line arms, `CalcHeight`'s
   truncation, `scaleYSpacingValue`). A round taking it should instrument `SlideAutofit`'s walk on
   `RESPA_-_Section_8_Webinar.ppt` page 18 — 314 characters on both sides, ours 20.01 and the
   reference 18.99, outline placeholder `pr23` with `style:shrink-to-fit="true"` and a 9.964 cm
   minimum height — and report the block height at each row against the box, which is the only
   measurement that separates "one line too few" from "every line a fraction short".
   **O14 gives this a second, independent witness that has nothing to do with font size**:
   `pres_ioc_phuket` p26's white box, where the reference's own model allows the shrink and the
   reference does not take it, and we shrink 10.9 pt (§1.3). A shape height is a much more direct
   read of the block height than a dominant drawn size is, and it needs no autofit search at all.
2. **The bullet is a separate, one-unit seat** at `outliner.cxx`:851-855 with its own vertical
   placement question, and it is *not* what the census measures.
3. **The `.pptx` half was not re-measured this round.** Round 85's figure of 41 pages over both
   columns stands only as of its own base; the `.ppt` half of it is 19 here.

---

# 3. O13 — Escher WordArt, with the census the two earlier ones did not have

**Not closed, and not implemented.** What this round adds is the *reference side* of the reach,
which both `probes/escher-wordart-census` and `probes/fontwork-reach` established from the file
alone.

`--convert-to fodp` over all 51 `.ppt` and a grep for `draw:text-path="true"`:

```
2 shapes, in 2 of the 51 .ppt
  pres_ioc_phuket.ppt                                       draw:type="fontwork-plain-text"
  8.16_AOD_FINAL_Provider_Training_Presentation_9_2009.ppt   draw:type="fontwork-arch-up-curve"
```

That is 26.2.4.2 itself saying which shapes it makes Fontwork and **which preset name it gives
each** — and the two names are exactly the ones `XRay.Office.Ooxml/DrawingML/Fontwork*` is keyed by,
which is the strongest statement yet that the engine is not the obstacle and the Escher reader is.
It also reproduces the two earlier censuses' shape types (136 `mso_sptTextPlainText` and 144
`mso_sptTextArchUpCurve`) from the other side.

**The residual it costs.** After O14, `pres_ioc_phuket` is 4.77 summed `|ink|%` over 26 pages, and
its worst page is **page 26 at 2.04**, which is 43 % of the whole document's remaining ink and is
the WordArt: the reference clips a gradient to the glyph outlines and we paint the rectangle.

**Left seated.** Two shapes in two of 51 is a small reach, the engine exists, and the missing
piece is a reader: the `gtextUNICODE` (192), `gtextFont` (197), `gtextSize` (195), `gtextAlign`
(194) and `gtextFStrikethrough` (255) properties — all five IDs checked against
`include/svx/msdffdef.hxx`:121-129 — read into a `TextPath` property set the way
`msdffimp.cxx`:2517-2571 does (**`TextPath` itself is bit 0x4000 of property 255, `TextPathMode`
0x100 and `SameLetterHeights` 0x80**), plus the shape-type-to-Fontwork-name mapping, which is at
`svx/source/customshapes/EnhancedCustomShapeTypeNames.cxx`:171 and :179 — `fontwork-plain-text` →
`mso_sptTextPlainText`, `fontwork-arch-up-curve` → `mso_sptTextArchUpCurve`.
*(An earlier draft cited `msdffimp.cxx`:2516-2600 for that mapping. **That range is the `TextPath`
property sequence and holds no name table**; the misattribution is corrected here.)* `probes/fontwork-reach` has already read all five of the corpus's shapes'
property 255 and established that **none** states `SameLetterHeights` or `ScaleX`, so the two knobs
that would need measuring are unreachable here. What is *not* established, and is what a round
would need, is what the reference does with the shape's **fill** when the text is a path — the
gradient on page 26 is clipped to the glyphs, and nothing in this tree does that yet.

---

# 4. The suite

**This section previously stated the Fidelity figure without the run having happened.** The log it
pointed at stops at its `== Fidelity` line with a NuGet restore error (`NU1900`, the package
source unreachable, promoted to an error by `TreatWarningsAsErrors`) and Spreadsheets had failed to
build in the same pass on a half-deleted `obj`. The number was right; it was asserted, not
measured. It is measured now, on a clean restore and a clean `dotnet build XRay.Office.slnx -c
Release` at **0 warnings, 0 errors** — `tests.log`, kept beside this file:

| project | |
|---|---|
| Containers | 109 / 109 |
| Core | 521 / 521 |
| Markup | 259 / 259 |
| OpenDocument | 146 / 146 |
| Presentations | **1045 / 1045** |
| Rendering | 164 / 164 |
| Spreadsheets | 1264 / 1264 |
| Text | 728 / 728 |
| Vector | 309 / 309 |
| WordProcessing | 1885 / 1885 |
| **Fidelity** | **542 passed / 10 failed of 552, 0 skipped** |

The ten are exactly the briefed baseline: `PageDrawing` ×4 (`paginated.docx`/`.fodt`/`.rtf`/`.doc`),
`TabStop` ×4 (`list-label-overrun.docx`/`.doc`/`.odt`/`.fodt`), `SheetDrawing`
(`sheet-rich-text.xlsx`), `JustificationShrink` (`justify-shrink-2013.docx`). No eleventh.

# 5. The scripts

| | |
|---|---|
| `fodp-sweep.sh` | 26.2.4.2's flat-ODP of all 51 `.ppt` — the instrument the whole round runs on |
| `shapes.py`, `persist.py` | the live-record-tree walk: every slide shape's type, anchor and `msofbtOPT` properties |
| `growth.py` | stated Escher anchor against the height the reference resolves, matched on (x, y, width) |
| `fitshape-census.txt` | how many live `.ppt` shapes state `fFitShapeToText`, and how many hold text |
| `run-sweep.sh` | our half of the `.ppt` column, scored against the banked 26.2.4.2 reference |
| `ink.sh` | summed `\|ink\|%` per document against the same bank, rendering and deleting one at a time |
| `autofit-rows.txt` | each dominant-size row fitted to a stated size and two `constScaleLevels` rows |
| `shrink-census.txt` | whether each of those pages carries a `style:shrink-to-fit` shape at the reference |
| `fitshape.py` | the driver behind `fitshape-census.txt`, which the first pass did not keep |
| `shrink-baserate.py`, `.txt` | the base rate `shrink-census.txt` is worth nothing without — **79.3 %** |
| `rowfit-null.py`, `.txt` | how often an arbitrary size pair admits `autofit-rows.txt`'s fit — **71-88 %** |
| `bullet-p2.txt` | O28's two numbers off the content stream's own `Td`/`Tm`, beside O15's census page |
| `gate-head-reverify.tsv` | the gate re-run at a fresh clean build — all 51 rows identical to `gate-head.tsv` |
| `confine2.sh`, `confine2-{list.txt,base.tsv,head.tsv}` | confinement widened to 90 non-`.ppt` documents |
| `tests.log` | the suite, actually run |

---

# 6. The re-verification pass, and what did not survive it

The container was restarted mid-round; the work was committed unbuilt, untested and unreviewed as
`e9729974b` and then re-read as an adversary against a fresh clean build. This is the ledger.

## 6.1 Re-measured from scratch and standing

| claim | how it was re-checked |
|---|---|
| the build | `dotnet restore` + `dotnet build XRay.Office.slnx -c Release`: **0 warnings, 0 errors** |
| the suite | all eleven projects run; Fidelity **542 / 10** with exactly the ten known names (§4) |
| **witness 103.37 against 103.38** | fresh head build re-rendered `pres_ioc_phuket.ppt`; both fills re-read out of the two content streams |
| the banner is two 40 pt paragraphs | reference fodp re-read: two `<text:p>`, both `T62` = `fo:font-size="40pt"` Arial Black, on a `gr74` stating `draw:auto-grow-height="true"` |
| **every O14 C++ citation** | each opened again by line: `msdffdef.hxx`:119; `svdfppt.cxx`:1051, 1053-1055, 1078-1081, 1090, 1116-1119, 982, 5405-5410, 5241-5246, 5263-5268; `svdotxat.cxx`:44, 76-77, 78-84, 110-118, 210-226, 228-236; `svdoashp.cxx`:2249, 2424. All correct |
| `fitshape-census.txt` | re-derived with `fitshape.py`: **12044 / 1691 / 38 / 1348**, output identical |
| `growth.txt` | `growth.py` re-run: **byte-identical**; 101 shapes in 22 documents over 1 mm, 83 taller / 18 shorter, 30 over 1 cm |
| `gate-head.tsv` | the whole 51-document gate re-run at the fresh build: **all 51 rows identical including the md5 column**. `match` 49, 0 page counts and 0 alphanumeric counts moved, sum \|glyph distance\| 1320 |
| `gate-base.tsv` | `cli-base` re-rendered the witness: banner **64.88 pt** and md5 matching the banked base row, so that binary is the base |
| the ink table | re-aggregated: **118.08 → 103.08**, MAJOR 37 → 29, 11 / 2 / 7; the witness re-diffed live at 7.78 → 4.77 and p26 4.28 → 2.04 |
| O13's reference-side census | fodp grep re-run over all 51: **2 shapes in 2 documents**, `fontwork-plain-text` and `fontwork-arch-up-curve`; all five `gtext*` property IDs checked |
| O15's 19-page census | re-derived from the three size tables: **19 pages, 15 documents, 71.10 pt, 0 fixed / 0 newly wrong, 13 same-alnum, 12 of 13 we draw larger** |
| O28's two numbers | measured off `Td`/`Tm`, not off a bounding box (§2.1, `bullet-p2.txt`) |
| the dominant-size statistic is not an artefact | the runner-up size on all 13 same-character pages carries at most half the winner's characters; no near-ties, so the statistic is stable |
| **reach** | exactly **51 `.ppt` in 963 tracked corpus files**, none outside `slides/`, no `.pps`/`.pot` — the sweep is the population |
| confinement | widened to **90 of 90** non-`.ppt` renderings byte-identical |
| three tests fail at the base | sources reverted to `61bc19e03`, head tests kept, rebuilt: **3 failed / 1042 passed**; 1045 / 1045 restored |

## 6.2 Withdrawn

1. **§4's suite figure was asserted, not run.** The right number, reached the wrong way. §4.
2. **The `style:shrink-to-fit` census does not establish the autofit reading.** Base rate 79.3 %
   among agreeing pages, 81.9 % among text-bearing ones; the census's 84.2 % is at it. §2.3.
3. **The `(stated size, row, row)` fit is close to vacuous** — 71 % of arbitrary near pairs admit
   one. §2.3.
4. **`3.647 cm = 2 × fround(1411 × 1.2) + 2 × 127`** is 3640, not 3647. The two 40 pt paragraphs
   are read out of the style, not reconstructed. §1.2.
5. **O13's `msdffimp.cxx`:2516-2600 does not hold the shape-type-to-name mapping.** That is
   `EnhancedCustomShapeTypeNames.cxx`:171, :179; :2517-2571 is the `TextPath` property set. §3.
6. **"The two tests that fail at the base" is three**, and one of them was a passing test that had
   to be redirected rather than a failing one. §1.6.

## 6.3 Newly recorded, not previously stated

- **The growth census sees 53 % of its population** — 878 matched of 1667 grow-flagged shapes,
  789 unmatched. §1.4.
- **`MinimumFrameHeight` discriminates on the placeholder; the reference discriminates on which
  branch built the text object.** A text-bearing non-custom-shape with `fFitShapeToText` and no
  placeholder diverges. Unmeasured. §1.4.
- **The shrink arm makes one shape on the witness page worse** — `pres_ioc_phuket` p26's white box,
  92.38 → 81.77 against 92.64 — because our block height is short where the reference's model would
  also allow the shrink. This is O15's open question in geometry rather than in font size, and it
  is a better instrument for it than a dominant drawn size. §1.3, §2.4.
- **Page 2 and page 22.** O28's witness bullet and O15's census entry for the same deck are
  different pages, and page 2's dominant size agrees on both sides. §2.1.
- **An assembly-byte comparison is not a confinement instrument here**: two full rebuilds of one
  tree give fourteen differing first-party DLLs and identical renderings. §1.5.
- A dangling reference in `PptOdfPlacementTests.cs` to a test named
  `TheBinarysGrownShapeIsWhatTwentySixTwoResolvesForIt`, which exists nowhere. Fixed.
