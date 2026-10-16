# Words round 46 — measured results

Branch `worktree-words-r46` on `ee87a6d0e`. Predictions in `prediction.md`, each committed before
the sweep it predicts (`d5f75ad3b` and `086c42cfb`).

## Baseline

Reproduced **exactly**, over `words/batch-*`, 200 rows, no duplicate paths (`baseline.tsv`):

| | brief | measured |
|---|---:|---:|
| documents matching | 157 | **157** |
| absolute page error | 75 | **75** |
| exactly-correct page counts | 167 | **167** |
| absolute word error | 6602 | **6602** |

## The round, end to end

| | baseline `ee87a6d0e` | after widow/orphan | after picture descent |
|---|---:|---:|---:|
| documents matching | 157 | **158** | **158** |
| absolute page error | 75 | **67** | **70** |
| exactly-correct page counts | 167 | **168** | **168** |
| absolute word error | 6602 | 6695 | **6666** |
| renderings changed, cumulative | — | 55 | **80 of 200** |

One verdict gained, no verdict lost. Page error −5 over the round, exact page counts +1, word error
+64 — of which +84 is one 697-page document (below).

---

## 1. A DOCX gets widow and orphan control from `w:pPrDefault`'s presence

`WordParagraphFormats` read an absent `w:widowControl` as **off**, so no ordinary DOCX had widow or
orphan control at all. `Ww8LayoutFormat` has read `HasWidowControl ?? true` for the whole corpus's
binary half all along, and that asymmetry between the two readers is what raised the question.

### The rule

> A DOCX paragraph gets widow and orphan control of two lines when `word/styles.xml` carries a
> `w:docDefaults/w:pPrDefault` element — **empty or not** — unless that element's own `w:pPr`, the
> paragraph's style chain or the paragraph itself turns `w:widowControl` off.

The trigger is the element's *presence*, not its content, and Word writes it empty.
`StyleSheetTable::applyDefaults` puts `ParaWidows` and `ParaOrphans` at 2 on the built-in style
every other style inherits from, and it is reached only from the `w:pPrDefault` arm of
`StyleSheetTable::sprm` (`sw/source/writerfilter/dmapper/StyleSheetTable.cxx`:653-670, 2115-2160) —
*"WARNING: these defaults only take effect IF there is a DocDefaults style section. Normally there
is, but not always."*

**The citation is the hypothesis and the probes are the evidence.**
`widow-orphan-default.py` authors nine variants at five straddle positions of a four-line paragraph
whose lines are one unbreakable 28-character token each, and measures the installed 24.2.7.2. The
`para-off` variant states `w:widowControl w:val="0"` on the straddling paragraph and so **measures
the room** at the foot of the page; a variant that puts fewer lines there than `para-off` does at
the same filler count is one with the control on.

| variant | control on at fillers | behaves like |
|---|---|---|
| `no-docDefaults` | — | off |
| `no-pPrDefault` | — | off |
| `empty-pPrDefault` | 14, 16 | **on**, identical to `para-on` |
| `pPrDefault-with-pPr` | 14, 16 | **on** |
| `pPrDefault-widow-off` | — | off |
| `pPrDefault-para-off` | — | off |
| `settings-on` (`w:settings/w:widowControl`) | — | **off** |
| `para-on` | 14, 16 | on |

### Refuted: the document-level `w:widowControl` does nothing

`WordCompatibility`'s remarks say the document-level flag is deliberately unread because *"adding
the document-level flag moved nothing in LibreOffice's output"*. That measurement was right and the
sentence attached to it was wrong: the flag is **inert in 24.2.7.2** — the `settings-on` variant is
indistinguishable from the control at all five straddle positions — and the reason nothing moved is
that the file already had a `w:pPrDefault` and so already had the control. The dev tree does carry
a `SettingsTable.cxx` path that would honour it; the binary that made the references does not.
Pinned by a test.

### Our own widow/orphan model, checked against Writer at fifteen points

The change puts `Paginator.Allowed` live on 130 documents that were never running it, so the probe
takes the target paragraph's line count and every (lines, room) pair was measured:

| paragraph | room 1 | room 2 | room 3 | room 4 |
|---|---:|---:|---:|---:|
| 2 lines | 0 | — | — | — |
| 3 lines | 0 | **0** | — | — |
| 4 lines | 0 | 2 | **2** | 4 |
| 5 lines | 0 | 2 | 3 | **3** |

`Allowed` reproduces every one of them. The bolded cells are the ones a naive "keep what fits"
model gets wrong.

### The numbers

| | baseline | after | predicted |
|---|---:|---:|---|
| documents matching | 157 | **158** | 154–161 |
| absolute page error | 75 | **67** | 62–82 |
| exactly-correct page counts | 167 | **168** | 162–173 |
| absolute word error | 6602 | 6695 | 6450–6750 |
| renderings changed | — | **55** | 55–100 |

Every band held; the reach came in at the bottom of its band.

Three documents moved their page count and all three towards the reference:

| document | before | after |
|---|---|---|
| `gpp-pr-top-7-office-markets-4q-2023.docx` | `pages` 3/4 | **`match` 4/4** |
| `AC-150-5370-10G-updated-201604.docx` | 686/697 | 690/697 |
| `150-5370-10H.docx` | 711/721 | 714/721 |

`gpp` is the document round 45 gave up a verdict to expose, and the causal test is a mutation
rather than an argument: setting `w:widowControl w:val="0"` on its "In the year 2023 …" paragraph
puts that line back on page 1 **in the reference**, and setting `<w:widowControl/>` leaves it on
page 2. Its two-line paragraph had one line of room and LibreOffice moved the pair.

The census is a ceiling and says so: **130 of 134 DOCX** carry a `w:pPrDefault`, four have one that
turns the flag off, none lack it, and the 66 `.doc` are untouched. 55 renderings actually changed.

### Tests

Seven in `DocxWidowControlTests`, `XRay.Office.WordProcessing` 746 → **753**. All seven verified by
reintroduction with `verify-test.sh`, four mutations:

| mutation | fails |
|---|---:|
| the default never applies | 2 |
| the presence flag is never set | the same 2 |
| a stated `w:widowControl` always reads on | 3 |
| the default applies to every DOCX | 2 |

The last two are the two refuted alternatives — "widow control is simply on for every DOCX" and
"the document-wide default overrides an explicit off" — pinned in code. None of the seven is a
precondition.

Confined to `XRay.Office.WordProcessing`, so no cross-track sweep is owed for this half.

---

## 2. A line holding only an as-character picture keeps no text descent

Round 45's open item, in its words: *"a picture alone on its line keeps 2.6 pt of the paragraph
font's descent LibreOffice drops … `MeasuredParagraph` cites an ODF fixture where LibreOffice does
add that descent, and these probes cannot separate a format difference from a text-on-the-line
one."*

`picture-alone-descent.py` is the pair that separates them: the same two shapes — picture alone,
picture with text beside it — authored in **both** DOCX and flat ODF, at four picture heights,
measured as the gap between the baseline above the picture's paragraph and the baseline below it.

| picture | docx alone | docx with text | fodt alone | fodt with text |
|---:|---:|---:|---:|---:|
| 5 pt | 27.60 | 27.60 | **18.80** | 27.60 |
| 20 pt | 33.80 | 36.40 | 33.80 | 36.40 |
| 50 pt | 63.80 | 66.40 | 63.80 | 66.40 |
| 150 pt | 163.80 | 166.40 | 163.80 | 166.40 |

**LibreOffice's fodt and docx agree to 0.00 pt in all eight pairs, and with-text − alone is +2.60
in all four** — Liberation Serif's 12 pt descent to the tenth. It was never a format difference:
`dotnet/tests/corpus/features/picture-anchor.fodt` reads *"An inline picture follows: ⟨picture⟩ and
that was it"*, so its picture has text on its line and it is the **with-text** row.

The 5 pt rows are what fix the *shape* of the fix rather than its constant. A 5 pt picture alone
gives 13.8 pt of line in DOCX and 5 pt in flat ODF, because DOCX emits an anchor character and so
has a run whose height floors the line, and flat ODF emits nothing and so has no run at all. Only
the **descent** is dropped; the run's height is still accumulated.

Keyed on the line's characters — every one a non-printing control, with an as-character object on
the line — rather than on the run being empty, because the readers disagree about whether a picture
is a character at all. A U+0001 standing for a field result or a note reference is deliberately not
covered: that mark is drawn and its descent is real.

### The numbers

| | before | after | predicted |
|---|---:|---:|---|
| documents matching | 158 | **158** | 156–161 |
| absolute page error | 67 | **70** | 60–72 |
| exactly-correct page counts | 168 | **168** | 165–172 |
| absolute word error | 6695 | **6666** | 6550–6800 |
| renderings changed | — | **49** | 25–70 |

**No verdict moved and page error went the wrong way by three.** All three points are one document,
`AC-150-5370-10G-updated-201604.docx`, 690/697 → 687/697, which carries six picture-only paragraphs
and was already the track's largest page outlier for an unrelated reason. Reported as a fix that
cost three points of page error, not as a win: it is right on its own evidence, at **14 of 16
authored probe rows exact against 11 of 16 before**, and the aggregate word error fell 29.

### The cross-track measurement a shared-layer change owes

`XRay.Office.Text` sits below all three families. Slides and sheets — 334 documents — were rendered
whole with our own CLI at `453ed3081` and at this tree, `SOURCE_DATE_EPOCH` pinned, `/CreationDate`
and `/ID` normalised out: **334 of 334 byte-identical, 0 differing.** The old code was restored with
`git checkout HEAD -- <file>` and `git diff ee87a6d0e..HEAD --stat -- dotnet/src` checked non-empty
before committing, which is round 45's trap.

### Tests

Five in `PictureAloneLineHeightTests`, `XRay.Office.Text` 277 → **282**. Three verified by
reintroduction:

| mutation | fails |
|---|---|
| the rule dropped | `APictureAloneOnAnAnchorCharacterLineIsExactlyAsTallAsThePicture` |
| the rule fires on a line that does hold text | `TextBesideThePictureKeepsItsDescent`, plus `InlineObjectLineSpacingTests.AnInlinePictureRaisesTheLineAndLeavesTheTextHeightAlone` and `ItemisationTests.AnInlineObjectRaisesTheLinesAscentAndKeepsItsDescent` — an existing control that says the new test is not the only thing holding that half |
| the run's height dropped beside its descent | `AShortPictureAloneStillGetsTheParagraphFontsLineAsAFloor` |

The other two are **drift guards and are labelled as such in the file**.
`APictureOnAnEmptyLineIsExactlyAsTallAsThePicture` asserts behaviour this change did not alter. And
no mutation of the guard can fail `AControlCharacterWithNoObjectIsUntouched`: with nothing raising
the ascent, the run's height dominates the maximum whether its descent is counted or not, so
widening the guard to fire on an object-free line is an **equivalent formulation rather than an
undetected defect**. That is a different finding from an untested rule and is recorded as one.

---

## Measured and deliberately not fixed

**An as-character object at the very end of a flat-ODF paragraph contributes no height.** The
`fodt with-text` rows come back 27.60 against LibreOffice's 36.40 at every picture height — the
picture is measured as though it were not there — and putting one more word *after* the picture
makes them exact (36.40 against 36.40, verified separately). The cause is
`one.Offset < end` in `MeasureLine`'s object walk: the object's offset equals the paragraph's text
length, so it falls outside every line's half-open range.

Not fixed, because deciding which line an object at the paragraph's end belongs to needs the line
list, which `MeasureLine` does not have; the obvious widening (`o <= end`) attaches it to *every*
line of a wrapped paragraph. `FrameLayout` already solves the same problem for *placement* with an
`EndsParagraph` flag on the line, which is where a fix should start. **No ODF document is in the
words track**, so this is a measured defect with a reproducing probe rather than a guess.

## The list-label population, measured

Round 45 left this unmeasured: three of the eleven renderings its rule changed carry no inline
object at all, because a numbering level taller than its item enters the line the same way a
picture does (`SwNumberPortion` is `PortionType::Number`, which does not raise the line-spacing base
height either).

`list-label-population.py` over `words/batch-*`: **51 of 134 DOCX** both resolve some paragraph to a
proportional line spacing above 100% *and* carry a numbering level that states its own `w:sz`; 64
more have the spacing and no sized level, 7 have a sized level and no spacing above 100%.

It is a **ceiling and a partial one**: it cannot see the 66 `.doc`, whose levels live in the WW8
`LSTF`/`LVLF` structures; it cannot see a label taller for a reason other than `w:sz`, which is
every level set in Symbol or Wingdings beside a Latin item; and it cannot see whether the taller
label ever moves a break. Round 45's measured reach on the whole rule was 11 renderings against a
20-document ceiling, so a comparable discount applies here.

## Left open

- **`AC-150-5370-10G-updated-201604.docx` 687/697 and `150-5370-10H.docx` 714/721.** Both moved
  towards the reference this round and neither is close. On the token multiset both improved where
  it matters: under-draw 363 → 339 and 389 → 366, and total mismatch 934 → 899 on the second. The
  scalar word error rose on the first because its over-draw rose more than its under-draw fell.
- **The ODF end-of-paragraph inline object**, above.
- **The ±1 page cluster is 13 documents at −1 and 7 at +1**, down from 14 and 7. Round 45's
  refutation stands: no shared cause, and the sub-group worth working is the ones that diverge in
  the first three pages.
- `template---tpr-technical-progress-report-with-guidance.docx` 7/8 was examined and not fixed. Its
  page 2 holds 37 lines against the reference's 33 while ending *lower* on the page (85.40 against
  74.35), so the reference's lines there are taller — its header table's rows are ~2 pt taller per
  row over eight rows, and after the table it stops the page while we continue. That is a
  table-metric question on a document with a 14 pt numbering level, not a flow one.
- Untouched, as handed over: the 249 `FORMCHECKBOX` fields across 16 documents; `A_320.doc`
  141/150; the `.doc` reader-split clusters; both round-38 leads; `手机免提系统TSB.doc`; the
  standing table-only-header import-defect decision; and Escher picture cropping.

## Test counts on the final tree

Core 284, Containers 109, Text **282** (277 + five `PictureAloneLineHeightTests`), Vector 293,
Rendering 121, Markup 259, OpenDocument 125, WordProcessing **753** (746 + seven
`DocxWidowControlTests`), Spreadsheets 621, Presentations 576, Fidelity 550 — **0 skipped, 0
warnings**, each project run on its own with its output kept and its count compared against the
known-good one.
