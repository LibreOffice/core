# words-r53 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
`fc-match Calibri` → `Carlito-Regular.ttf`, corpus `/c/sandbox/workdir/sample-files`,
worktree `wt-words-r50` on branch `wt-words-r53`, base `41445736a8c`,
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

`prediction.md` beside this was committed in two parts, each **before** the change it predicts:
`664e3c53d9b` before the first, `f82c0129001` before the second.

## Scoreboard

| | words |
|---|---|
| baseline (`MANIFEST.tsv` status column, reproduced) | **318 / 337** |
| after both changes | **318 / 337** |
| gains | 0 |
| regressions | 0 |

### Baseline reproduction

`batch-check.sh … 'words/*' … 8` → `TOTAL 355  MATCH 335  MISMATCH 20`. Scored against
`MANIFEST.tsv`'s 337-path list rather than that total — the extra 18 rows are the
case-insensitive mount's alias entries — **318 match, 19 open, and 0 disagreements with the
manifest's status column, document for document.** The briefed baseline reproduces exactly.

## Prediction against measurement

| | predicted | measured |
|---|---|---|
| change 1, the break-only paragraph | **+1** (`097`), regression risk named on `096` | **+1 `097`, −1 `096`** — the named risk fired |
| change 2, the frame anchor's size | **+2** (`097` and `096` both to match) | **`096` recovered, `097` lost again** |
| net verdicts | +2, to 320 | **0, still 318** |
| regressions after both | 0 | **0** |
| cross-track slides verdicts | 0 | **0** |
| cross-track slides ink | "ink, not verdicts" | **0 of 55 renderings changed** — over-reached |

**The verdict prediction was wrong and the mechanism predictions were right.** Both changes are
correct against the reference on every authored row; the gate simply cannot see either of them
once they are both in, because they close different halves of the same document's error and `097`
turns out to fail on a **1.7 pt** margin at a page boundary that neither change reaches.

### What did move: 41 of 337 renderings, and 27 of them towards the reference

Byte-comparison of our own renderings before and after, with `/CreationDate` normalised out and
`SOURCE_DATE_EPOCH` pinned on both runs:

| | renderings changed |
|---|---:|
| change 1 alone | 7 |
| change 2 alone | 38 |
| both together | **41 of 337** |

Direction, measured rather than assumed — mean absolute vertical error of every word against the
reference's own word at the same index on the same page, before against after:

**27 closer to the reference, 6 further, 8 unchanged in this metric.**

| document | before | after | Δ |
|---|---:|---:|---:|
| `644730BRI0mna000BOX361539B00public0.doc` | 59.25 | **6.04** | −53.22 |
| `096_Business_Case_Template_Editable_Layout` | 39.09 | **0.38** | −38.71 |
| `097_Business_Case_Template_Elegant_Layout` | 29.15 | **1.61** | −27.54 |
| `090_Business_Case_Template_Blue_Theme` | 19.45 | **1.21** | −18.24 |
| `037_Venn_Diagram_Template_Four_Circle` | 10.71 | **3.75** | −6.96 |
| `template---tpr-technical-progress-report…` | 5.12 | **0.87** | −4.25 |
| `33004` | 2.76 | **0.19** | −2.57 |
| `095_Business_Case_Template_Easy_Format` | 3.60 | **1.36** | −2.25 |
| `091_Business_Case_Template_Complete_Guide` | 7.78 | **5.63** | −2.15 |
| `fleetfastfacts16nov2023` | 5.18 | **3.24** | −1.95 |
| `UG.CAO.00133 … Language` | 12.48 | **10.54** | −1.94 |
| `ABCD-FE-01-00 Flight Envelope` | 5.13 | **3.26** | −1.88 |

**Named rather than netted — the six that moved away**, all small:
`098_Business_Case_Template_Fillable_Layout` 0.95 → 2.29, `B11. TE.CAO.00129 Experience logbook`
1.59 → 1.84, `JEMIT_Template` 8.40 → 8.59, `gpp-pr-top-7-office-markets-4q-2023` 2.13 → 2.25,
`approvals-and-standardisation-…-part-145-ann` 1.75 → 1.86,
`100_Business_Case_Template_Modern_Format` 7.25 → 7.30.

`098` is the worst of them and **a blind reviewer given its page pair could not see it**: the only
differences reported were the `Note:` box drawn solid black by us against a red dashed border in
the reference, and faint grey dotted lines under the reference's solid row rules. On vertical
position the reviewer said *"the vertical position of every element … lines up the same way from
top to bottom on both halves. I do not see any row sitting higher or lower relative to its
counterpart."* A 1.3 pt mean shift is below what a page reading resolves; it is recorded because
it is a real, measured worsening, not because it is visible.

## The two changes

### 1. A paragraph whose whole text is format controls had no height at all

**The brief's observable holds exactly; both its stated reach and its named witnesses do not.**

`TextItemiser.AddWithoutControls` cuts every `IsFormatControl` character out of the item list, and
U+2028 — what all four word-processing readers and both DrawingML readers emit for a manual line
break — is in that set. `MeasuredParagraph.Measure` intersects each `FormattedRun` with those
items, so **a paragraph made of nothing else arrives with `_runs.Length == 0`**, and
`MeasureLine`'s three fallbacks all miss: the fold finds no run, the blanks-refold finds no run,
and the last resort — the empty-paragraph rule — is guarded on `_runs.Length > 0`. Every line came
out **0 pt**.

Measured directly by laying r52's own probe documents out through `DocxLayoutSource` +
`ParagraphLayouter` and reading the boxes:

| document | text | measured runs | per-run path | single-face path |
|---|---|---:|---|---|
| `a-br` | `U+2028` | **0** | 2 lines, **0.00 pt** | 2 lines, 23.10 pt |
| `i-spacebr` | `SPACE U+2028` | 1 | 2 lines, 23.00 pt | 2 lines, 23.10 pt |

That is the whole of "one space is enough": a space is not a format control, so it leaves a run
behind and the fallback fires. **`TextMeasurer` is not the seat**, exactly as r52 said — and
`TrailingLineBreakTests.AParagraphOfNothingButABreakIsTwoLines` has asserted the correct line
*count* since round 45. Only the height was nought. The comment on `IsFormatControl` states the
defect in its own words — *"a paragraph that is nothing but control characters then has no run and
no line"* — and moved the C0 range out to avoid it without moving U+2028.

**Fixed** by keeping one zero-length `MeasuredRun` from the first formatted run when every run has
been itemised away. It is invisible to `Fold` (`touches` and `contains` are both false for an
empty range), invisible to `RunsBetween`, and adds nothing to the prefix table, so only that
fallback can see it.

r52's nine authored variants, re-run:

| case | reference | before | after |
|---|---:|---:|---:|
| one empty paragraph | 12.65 | 11.50 | 11.50 |
| **one paragraph of one `w:br`** | **25.30** | **0.00** | **23.00** |
| two empty paragraphs | 25.30 | 23.00 | 23.00 |
| **one paragraph of two `w:br`** | **37.95** | **0.00** | **34.50** |
| `X<br/>Y`, `<br/>Y`, `Y<br/>`, `<space><br/>` | 25.30 | 23.00 | 23.00 |

Every case is now on the same footing as an ordinary empty paragraph. The residual — 11.50 against
12.65 per line — is the standing line-height deficit that every row of that table carries equally,
including the rows that were already right.

### 2. A run holding nothing but a frame anchor raised the line to its own size

Found by taking `096`'s regression seriously rather than reverting it. With change 1 in,
**`096`'s row pitches became the reference's exactly** — 59.2, 59.2, 59.3, 59.2, 47.6, 47.6, 59.3,
the reference's figures to the tenth — and the whole block was pushed down by one 15 pt excess
between `Email Address` and `Phone Number`. `097` likewise collapsed to a single uniform +34 pt,
in the gap r52 had already localised.

Cutting `097`'s block-1 paragraph five ways and rendering each both ways:

| variant | reference | ours | Δ |
|---|---:|---:|---:|
| as is (anchored drawing + `w:br`) | 136.4 | 173.6 | **+37.2** |
| the `w:br` removed | 122.7 | 141.3 | +18.6 |
| the drawing removed | 136.4 | 137.8 | +1.4 |
| **only the run's `w:sz w:val="52"` removed** | 136.4 | **137.8** | **+1.4** |

Then pinned properly on ten variants — anchored against as-character, 10 pt against 26 pt, alone
against with text beside it — reading the height the paragraph adds over an empty one:

| case | reference | before | after |
|---|---:|---:|---:|
| a run of text at 26 pt | 20.60 | 19.10 | 19.10 |
| anchored drawing, run at 10 pt | 0.00 | −1.10 | 0.00 |
| **anchored drawing, run at 26 pt** | **0.00** | **17.25** | **0.00** |
| anchored drawing at 10 pt, text beside it | 0.00 | 0.00 | 0.00 |
| **anchored drawing at 26 pt, text beside it** | **0.00** | **17.25** | **0.00** |
| as-character drawing, run at 10 pt | 7.00 | 6.95 | 6.95 |
| **as-character drawing, run at 26 pt** | **7.00** | **17.25** | **6.95** |
| as-character at 10 pt, text beside it | 9.70 | 9.70 | 9.70 |
| **as-character at 26 pt, text beside it** | **9.70** | **17.25** | **9.70** |

**The reference's answer does not depend on the run's size on any row.** Ours was the run's size on
every row where that size was large, and already right on every row where it happened to match the
paragraph's — which is why this never read as a systematic error and is worth 34 pt on one document
and nothing on most. It is Writer's line model: a fly is a `SwFlyCntPortion` of its own, an
at-character fly is not in the line at all, a comment mark is a mark, and a run with no text makes
no text portion, so its font never reaches `SwLineLayout::Height`.

**Two halves, and the probe forced the second.** `PageParagraph.Measure` gives such a run the
paragraph's own face and size — right when the anchor is alone on its line. With only that, the
two rows that were already exact came back 0.80 and 9.90 against 0.00 and 9.70, because the
paragraph's body font is not the font of the text beside the anchor. `MeasuredParagraph.Fold` now
also passes over such a run while anything else is on the line, and the alone case falls through
the existing refold to meet it again at the paragraph's size. The anchor arm is **not** gated on
`blanksAreTransparentToHeight`: #i3952 is a Word compatibility setting the RTF and ODF filters
leave off, while "a run with no text is not a text portion" is the line model itself.

## Refutations

### 1. The `w:br`-only paragraph's reach is 22 paragraphs in 13 documents, not 469 in 66

The brief and `words-r52/results.md` say **469 such paragraphs in 66 of 271 documents**, name
`FAA 2025-26 Holdover Tables` (66), `24-25_FAA_Holdover_Tables` (58), `OM template …` (37) and
`EHEST-SMS` (35), and call that *"the risk, and it is why this needs a whole-track sweep"*. **No
census was committed with r52 and none I can construct reproduces those numbers.** Over
`MANIFEST.tsv`'s own 337-path list:

| reading | paragraphs | documents |
|---|---:|---:|
| `w:p` holding any non-page `w:br` | 3936 | 76 |
| …and no non-empty `w:t` | 23 | 14 |
| …and no tab, symbol, field, note or **as-character** drawing either — the defect | **22** | **13** |

The named witnesses hold **1, 1, 1 and 0** respectively. `OM template`'s 37 is its count of `w:br`
*elements*, which is the likeliest source of the figure. Every one of the 22 carries a single
break, so each is worth two lines rather than more. The one figure of r52's that does reproduce is
`097` holding **3**.

Two of the seven renderings change 1 actually moved were **outside** the census, exactly as the
prediction's blind-spot list said they could be: `644730BRI0mna000BOX361539B00public0.doc` is a
binary `.doc`, which the census does not read at all, and it is the largest single improvement in
the whole round (59.25 → 6.04 pt). So the census was a floor, and it said so in advance.

### 2. On `068` we stroke *fewer* paths than the reference, not more

Item 4 of the brief, and it was one measurement. r52 says *"we stroke 53 paths where the reference
strokes 36, with fills exact"*. Counting paint operators out of both PDFs' own decompressed
content streams, with parenthesised strings removed first so a letter `S` inside a text object
cannot be counted as a stroke:

| | stroked paths | box outlines | single-line strokes | fills |
|---|---:|---:|---:|---:|
| reference | **71 `S` + 2 `B`** | 35 (`m` + 5 × `l`) | 36 (`m` + `l`) | 41 |
| ours | **53 `S`** | 41 (`re`) | 12 (`m` + `l`) | 41 |

**The 53 is right and the 36 is the reference's single-line count, not its total.** Compared like
with like: we draw **6 more box outlines and 24 fewer connector strokes**, with the 41 fills exact
on both sides. That is the shape the next round should work — it is a missing-connector problem
with a small over-stroking of boxes beside it, not an over-stroking problem.

### 3. The audit's own per-project table counts files, not sites, for three of its rows

`git grep -c` per project gives Presentations **17**, Spreadsheets **10**, Text **6** — the table
said 15, 9 and 4, which are the file counts. The totals still sum to 48. **The shared layer is ten
sites, not eight**, and the brief's list of eight is short by the two extra `XRay.Office.Text` ones.
Corrected in `TODO.24-2-7-audit.md` with the one-liner that regenerates it.

### 4. `090`'s missing navy banners look exactly like item 3 and are not item 3

A blind reviewer given `090`'s page pair — a document that passes the gate and whose vertical
alignment this round moved from 19.45 pt to 1.21 pt — reported: *"In the left half, each heading is
plain bold black text sitting directly on the white background with no decoration. In the right
half, each corresponding heading is white bold text set inside a solid dark navy blue banner shaped
like a rightward-pointing arrow/chevron … This is the most significant visible difference,
appearing at three separate locations down the page."* Everything else the reviewer called
identical, including the vertical positions.

That is word for word the shape of item 3 — a shape the reference fills and we do not. **It is not
item 3.** `090` holds **zero** `wps:wsp`, zero `a:fillRef` and zero `a:lnRef`; its banners are
**VML**, three `v:shape type="#_x0000_t15"` with `fillcolor="#002060"` stated literally, and
`_x0000_t15` is exactly what r52's `DocxVmlFrames` note says it deliberately leaves undrawn. Its
`v:shapetype` carries its own path — `m@0,l,,,21600@0,21600,21600,10800xe` with `adj="16200"` — so
drawing it needs no theme and no style matrix, only the shapetype's `@path` and `@adj`.

**Two readers agreeing on a description is not agreement on a mechanism, and one reader matching a
brief's description is not corroboration of that brief's mechanism either.** This is the same trap
`HANDOVER.md` §7 records, met from a new direction.

## Item 3 verified rather than assumed, and one figure corrected

The brief asked for the mechanism to be checked before anything is built on it. It holds, exactly:

- `DocxFrameContext` is `(DrawingTheme? Theme, bool InHeaderFooter, int CompatibilityMode)` — the
  colour scheme and nothing else. `DrawingStyleMatrix` is reached by `PptxSlideLayout`,
  `DrawingChartPlot`, `DrawingTableStyle` and `DrawingEffects`, and by **nothing** in
  `XRay.Office.WordProcessing`. Seventh instance of "a route, not a rule", confirmed.
- `DocxFrames.Appearance` returns `(fill, null, 0)` the moment `a:ln` is absent, and its own
  remarks name the style matrix as the half it does not do.
- `056`: **39** zero-extent `wps:wsp` members, **34** with no `a:ln`, split **34 `prstGeom
  prst="line"` and 5 `straightConnector1`**. 66 of its 93 `wsp` carry `wps:style/a:lnRef`.
  Every figure r52 gave reproduces.

**One correction.** r52 says the fix "will also want arrow ends, which the blind reviewer saw and
no census counts". Counted: `056` holds **5** `a:headEnd`/`a:tailEnd` elements in total, and they
are on the 5 `straightConnector1`s. So arrow ends are a five-shape job, not a bulk one, and the
"bracket spines" the reviewer described are more likely the 34 zero-extent `prst="line"` shapes.

## Cross-track — measured, not argued

`XRay.Office.Text/Layout/MeasuredParagraph.cs` is shared and is reached by `SlideTextLayout` and
`SheetTextLayout` as well as `PageContent`.

**The anchor half has no reach outside word processing at all**, and that is checkable rather than
argued: U+0001 is emitted by `DocxLayoutSource`, `OdtLayoutSource`, `Ww8DocumentReader` and the RTF
reader, and by **no** presentation or spreadsheet reader (`git grep 'U+0001'` over
`XRay.Office.Presentations` and `XRay.Office.Spreadsheets` returns only an XLS pivot-cache byte
constant). A slide or sheet would have to hold a literal U+0001 in its own text.

**The break half was swept both ways.** The census found 17 break-only `a:p` in 7 decks (slides)
and **0** in sheets. The six batches holding those decks — `done-004`, `done-007`, `ceiling-002`,
`done-012`, `done-010`, `done-011`, **55 documents** — were swept at this branch and again with
`MeasuredParagraph.cs` alone checked out at `41445736a8c` by `git show` + `cp` + `touch` (never
`git checkout`), with `git diff --cached` asserted empty afterwards and the file asserted to
contain `IsAllAnchors` zero times on the before-leg and three times after.

**Of 55 documents: 0 gate rows changed and 0 renderings changed, byte for byte.** Not a page, not a
word, not a font, not a pixel. So the slides census over-reached and the prediction's "ink, not
verdicts" is refuted in the safer direction — there is no ink either. The parent owes no
cross-track sweep for this change beyond what is recorded here; sheets has no witness to sweep.

## `097` and `096`, in full, because the pair is the round's real result

`097` page 1, our y against the reference's, after both changes:

| row | reference | ours before | ours after |
|---|---:|---:|---:|
| `BUSINESS CASE` | 20.4 | 21.0 | 21.0 |
| `Document Control` | 69.5 | 72.5 | 72.5 |
| `Document Information` | 136.4 | 173.6 | **139.4** |
| the five table rows | 182.6 … 309.1 | 219.1 … 344.8 | **184.8 … 310.6** |
| `Document History` | 350.7 | 386.9 | **352.8** |
| `Versions Issue Date Charges` | 431.2 | 440.4 | **431.5** |
| `Document Approvals` | 618.2 | 627.1 | **618.2** |
| `Role Name Signature Date` | 697.8 | 679.8 | **696.1** |

Every row is now within 3 pt and two of them are exact. **The reference's page 2 holds no text at
all** — it is the trailing empty paragraph and the section mark, spilled. Our last table ends
1.7 pt higher than the reference's, so that paragraph still fits and we stay on one page. `097` is
now a **1.7 pt boundary case** rooted in the standing line-height deficit (our empty paragraph is
11.50 pt against the reference's 12.65), not a 34 pt layout error.

`096` is `1/1` against `1/1` again and its mean vertical error is **0.38 pt**, from 39.09.

## Tests

Two new files, **28 tests**, all passing:

| file | tests | verified by reintroduction |
|---|---:|---|
| `XRay.Office.Text.Tests/ControlOnlyParagraphHeightTests.cs` | 15 | **11** detect it; 4 are the reach controls |
| `XRay.Office.WordProcessing.Tests/AnchorRunHeightTests.cs` | 13 | both halves detected, by separate mutations |

Mutations run through `verify-test.sh`, tree clean before each and index asserted empty after:

| mutation | detected by |
|---|---|
| never keep a run when every one is itemised away | 11 of `ControlOnlyParagraphHeightTests` |
| `Coalesce(Runs)` instead of the anchor rewrite | `AnAnchorAloneIsAsTallAsTheParagraphWhateverTheRunSays` |
| `IsAllAnchors` never true in `Fold` | `TheTextsOwnSizeDecidesEvenWhenItIsNotTheParagraphs` |

**The third mutation went undetected on the first attempt and the test was added for it**, which is
the point of running the tool: with the paragraph's size and the text's size equal, the two halves
of the rule are indistinguishable, so the discriminating test states them unequal in both
directions.

The four undetected tests in the first file are deliberate reach controls —
`AParagraphHoldingAnythingElseKeepsItsOwnRuns` (×3) and `AParagraphWithNoRunsAtAllIsUnchanged`.
They are labelled as such rather than reported as detectors.

Ten non-Fidelity projects, run one at a time:

```
Core 337   Containers 109   Text 611   Vector 295   Rendering 150(1 skipped)   Markup 259
OpenDocument 125   WordProcessing 1096   Spreadsheets 895   Presentations 780     = 4657
0 failed
```

**4629 → 4657, delta +28** — 15 in `Text` and 13 in `WordProcessing`, the new tests and nothing
else. `dotnet build -v q -nologo`: **0 warnings, 0 errors.**

## The 24.2.7.2 audit — four shared-layer sites re-checked, one wrong

Full log in `TODO.24-2-7-audit.md`. **Not fixed**, per the brief: a change on the wrong one owes a
measured sweep of all three tracks and that is the parent's.

### `SystemFontResolver.cs:629` — WRONG

The site says an unrecognised family resolves to **DejaVu Sans**, *"because this path is what
LibreOffice reaches by asking fontconfig, and fontconfig's reply for a name it does not recognise
is its default family"*, and names Aptos, Segoe UI, Roboto, Lato, Montserrat, Myriad Pro, Futura,
Optima and Univers as having been probed.

`font-fallback-recheck.py` re-probes exactly those, one authored DOCX each, converted by the
installed `soffice`, with the drawn face read out of the PDF. **Four controls agree** — Liberation
Serif answers itself, Calibri answers Carlito, Cambria answers Caladea, Arial answers Liberation
Sans — and then **all ten unrecognised families answer DejaVu *Serif***, together with two authored
nonsense names, one carrying a serif hint and one carrying none. Both answer Serif, so the shape of
the name does not decide it either.

**The stated mechanism is falsified independently of the answer**: `fc-match Aptos` on this machine
returns `DejaVuSans.ttf`, as does `fc-match ""`. Whatever 26.2.4.2 is doing, it is not taking
fontconfig's default — the second time this project has caught that assumption.

**Cost, measured rather than assumed.** Comparing the embedded font list of all 337 words
renderings against the reference's: **86 disagree, and 73 of those carry `DejaVuSans` on our side**,
most as the plain pair `ours=DejaVuSans, ref=DejaVuSerif` — `033_Venn_Diagram_Template_Colored_Theme`,
`035_Venn_Diagram_Template_Editable_Format`, the whole `00x_Free_Genogram` family, and so on. The
two faces have different advances, so each is a line-breaking difference as well as a visible one.

### The other three

| site | outcome |
|---|---|
| `SystemFontResolver.cs:406` — "DejaVu first, never Liberation" | **verified still correct.** Ten unrecognised families, all land on DejaVu, none on Liberation. Only *which* DejaVu is wrong. |
| `SystemFontResolver.cs:435` — "no family → Liberation Serif" | **undecided, and recorded as such.** The probe's no-family DOCX carries no `styles.xml`, so LibreOffice applies *Word's* default and the case came back Carlito without ever reaching `DefaultFonts`. Needs a fixture that does. Still says 24.2.7.2. |
| `MeasuredParagraph.cs:741` — the picture-alone descent | **verified unchanged.** `words-r46/picture-alone-descent.py` re-run: 8 of 8 DOCX rows and 4 of 4 `fodt alone` rows exact, and the reference's own figures identical to round 46's 24.2.7.2 readings to the tenth at 20, 50 and 150 pt. The four `fodt with-text` rows still come back 27.60 at every picture height — round 46's own measured-and-deliberately-unfixed ODF defect, not a movement in the reference. It doubles as the regression control for change 2, since it is the whole DOCX picture family. |

Every verified site's comment now names **26.2.4.2** and the date. `:435` deliberately still says
24.2.7.2, because by that file's convention an undecided site is an unverified one.

## Left open, in the order the next round should take it

1. **`097` is now a 1.7 pt boundary case.** Our last table ends 1.7 pt above the reference's, so
   the trailing empty paragraph fits where the reference's spills to a textless page 2. That is the
   standing line-height deficit — our empty paragraph is 11.50 pt against 12.65 — and it is now the
   *only* thing between `097` and a verdict. It is also r52's open item 9 and it is worth more than
   one document: it is 1.15 pt on every empty paragraph in the corpus.
2. **`SystemFontResolver.cs:629` is wrong and 73 words renderings already carry the wrong DejaVu.**
   Reported, not fixed. The change is one line; the sweep it owes is three tracks. **This is the
   highest-value item the round found and it is deliberately left for the parent to schedule.**
3. **`#_x0000_t15` VML, three shapes on `090` alone.** The blind reading gives it a page-level cost
   on a document that otherwise matches, and the shapetype states its own path and `adj`, so it
   needs neither theme nor style matrix. `#_x0000_t136` WordArt (15 shapes) is the same shape of
   job and is harder.
4. **`DrawingStyleMatrix` still does not reach `DocxFrames`** — 458 shapes in 40 documents, verified
   above. Its arrow-end tail is **5** shapes on `056`, not a bulk job.
5. **`068`: 24 missing connector strokes and 6 surplus box outlines**, fills exact. Measured above;
   the direction stated in r52 was backwards.
6. **`012` and `015`** still share nothing with `097` — 0 break-only paragraphs each — and remain
   unexplained. `015`'s rendering moved this round (28.80 pt mean error, unchanged by the metric)
   and is a good candidate for a blind reading.
7. **The six documents that moved away from the reference**, led by `098` at +1.34 pt. Below what a
   page reading resolves; worth one measurement before it is assumed benign.
8. `028` at 317/327 is still the largest words gap; `024` is SmartArt and has never been looked at;
   `SheetChart` and `SlideChart` still run a multi-line label together.

## Files

- `prediction.md` — two parts, each committed before the change it predicts.
- `control-only-paragraph-census.py` — every paragraph in a corpus whose whole laid-out text is
  format controls, over all three families, with its blind spots in its own header.
- `anchor-run-size-census.py` — anchor-only runs stating a size of their own, per document.
- `font-fallback-recheck.py` — the `SystemFontResolver` re-check, with its four known-answer
  controls.
