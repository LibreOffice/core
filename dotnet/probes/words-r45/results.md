# Words round 45 — measured results

Predictions in `prediction.md`, committed at `352329901` before anything was rendered post-change.

## Baseline

Reproduced **exactly** at `45fea26c2`, over `words/batch-*`, 200 rows, no duplicate paths
(`baseline.tsv`):

| | brief | measured |
|---|---:|---:|
| documents matching | 155 | **155** |
| absolute page error | 77 | **77** |
| exactly-correct page counts | 165 | **165** |
| absolute word error | 6605 | **6605** |

## The scoreboard after

`after.tsv`, same harness, same corpus, 200 rows, no duplicates.

| | baseline | after | predicted |
|---|---:|---:|---|
| documents matching | 155 | **157** | 156–157 |
| absolute page error | 77 | **75** | 75–77 |
| exactly-correct page counts | 165 | **167** | 166–167 |
| absolute word error | 6605 | **6602** | 6590–6610 |
| renderings changed | — | **11** | 2–4 — **missed, threefold low** |

Four verdicts moved, three of them the right way:

| document | before | after |
|---|---|---|
| `1257259179492_2007_TPPT_102_Supporting_Doc_2.doc` | `pages` 10/9 | **`match`** 9/9 |
| `1228841571067_2009_TPPT_13__2007_TPPT_102__R.doc` | `pages` 13/12 | **`match`** 12/12 |
| `DOA_Template_Form_Type_Certification_Programme.docx` | `pages` 21/20 | **`match`** 20/20 |
| `gpp-pr-top-7-office-markets-4q-2023.docx` | `match` 4/4 | `pages` **3/4** |

## The rule

> Proportional line spacing extends a line by **(prop − 100)% of the line's *text* height**. It does
> not scale the line. So an as-character picture makes the line taller and takes no share of the
> percentage at all.

`SwTextFormatter::CalcRealHeight` (`sw/source/core/text/itrform2.cxx`:2441-2453) says it in a
comment — *"extend line height by (nPropLineSpace - 100) percent of the font height"* — and takes
the percentage of `GetLineSpacingBaseHeight()`. `SwLineLayout::Height(nNew, bText)`
(`porlay.cxx`:110) records that base only when the portion that raised the line
`IsUsedToCalcLineSpacingHeight`, which in the legacy mode every one of these documents is in means
`PortionType::Text` and nothing else (`porlin.cxx`:324, `porlin.hxx`:144). A fly-in-content is not
one. Where a line has no text portion at all, `porlay.cxx`:645 falls back to the paragraph's own
font.

**The citation is a hypothesis; the probes are the evidence.** Eight authored DOCX, 12 pt Liberation
Serif, measured against the installed 24.2.7.2 as the gap between the paragraph above and the
paragraph below:

| spacing | picture | text on its line | LibreOffice | scaling the line | this rule |
|---|---:|---|---:|---:|---:|
| 100% | 150 pt | — | 163.8 | 163.8 | 163.8 |
| 150% | 10 pt | — | 41.4 | 41.4 | 41.4 |
| 150% | 150 pt | — | **177.6** | 252.6 | **177.6** |
| 200% | 150 pt | — | **191.4** | 341.4 | **191.4** |
| 75% | 150 pt | — | **122.85** | **122.85** | 146.55 |
| 150% | 50 pt | — | 77.6 | 90.9 | 77.6 |
| 150% | 150 pt | 12 pt | **180.2** | 255.9 | **180.2** |
| 150% | 150 pt | 36 pt | **199.15** | 240.9 | **199.15** |

Two things the table settles that one document could not:

- **The slope.** 150% adds half the text height and 200% adds it whole, so "add half" cannot stand
  in for "(prop − 100)%".
- **The base is the line's text, not the paragraph's.** 36 pt of text beside the picture adds
  20.7 pt where 12 pt of text adds 6.9.

And the row that decides the *shape* of the fix rather than its constant: **below 100% the whole
line, picture included, really is scaled.** Writer takes the other branch there —
`SvxLineSpaceRule::Auto` under `PROP_LINE_SPACING_SHRINKS_FIRST_LINE`, which multiplies
`nLineHeight` — and 150 × 0.75 = 112.5 is what comes back, not 150 − 25% of the text. A fix that
used the text height at every percentage would have been wrong in a way no corpus document happens
to exercise.

### Two halves, and the second is where the corpus lives

The first commit took the text height from the measured runs. It fixed the DOCX probes and moved
**nothing** on the document the rule was derived from, which is a `.doc`: LibreOffice's own DOCX
round-trip of that file lands `Introduction` at 479.36 against the reference's 476.91, and the
`.doc` still landed it at 577.11. A `.doc` states a picture-only paragraph with **no run at all**,
so the line measured no text height and fell back to the line's own. The second commit supplies the
paragraph's font there, as `porlay.cxx`:645 does.

Worth stating because the two look identical from the outside: a fix verified on an authored probe
and on a round-tripped copy of the failing document **had not reached the failing document**.

## Reach, and the census that was three times too low

**11 renderings changed** — measured by rendering the track twice and comparing bytes with
`/CreationDate` normalised out. (Here the raw and normalised counts agree at 11, because
`SOURCE_DATE_EPOCH` was pinned on both sweeps. Round 44's warning still stands for anyone who does
not pin it.)

The prediction said 2–4, from a ceiling of 4. It was low, and the reason is worth more than the
number:

| census | reads | over | said | of the 11 it names |
|---|---|---:|---:|---:|
| `inline-object-spacing-census.py`, as predicted with | direct `w:spacing` on the `w:p` | 134 DOCX | 1 | 1 |
| the same, resolving `w:pStyle` and `w:basedOn` | + `word/styles.xml` | 134 DOCX | 4 | 3 |
| the same, resolving the default style and `w:docDefaults` | + the top of the chain | 134 DOCX | **17** | **5** |
| `inline-object-spacing-census-doc.py` | LibreOffice's flat-ODF export | 66 `.doc` | **3** | 3 |

**The standing warning on this project is that a grep over what a file declares overstates what it
draws. This is the same mistake pointing the other way, and it is the worse of the two.** An
overstated ceiling is labelled as a ceiling and read as one; an understated one produces a *low*
prediction, and a low prediction that comes true reads as a good prediction. Mine did not come true,
which is the only reason the shortfall was found at all.

The repaired census reaches 17 of 134 and still names only **5 of the 8 DOCX that moved**. The other
three change through a population no census of pictures can see: **a list label taller than the item's
text enters measurement as a phantom inline object**, so a numbered paragraph at more than 100%
spacing is the same rule with no picture in it. `template---tpr-technical-progress-report-with-guidance.docx`
carries **zero** inline objects and a numbering level at 14 pt, and it moved. That is consistent with
Writer, where `SwNumberPortion` is `PortionType::Number` and so is not one of the portions that raise
the base height either.

## The verdict that went the other way, and why it is not a reason to revert

`gpp-pr-top-7-office-markets-4q-2023.docx` matched at 4/4 and now paginates 3 against 4. Its page 1
carries a 178 pt inline picture in a style stating `w:line="288"` — 120%.

The line positions say the change moved it **sharply towards** the reference. First body line after
the picture:

| | ours before | ours after | reference |
|---|---:|---:|---:|
| `02 January 2024, Hamburg…` | 418.84 | **385.24** | 382.24 |

36.60 pt out becomes 3.00 pt out, and every one of the twenty-three lines below it moves with it.
What the change then exposed is a different defect underneath: the reference stops page 1 after
`even more.` at 704.05 and starts `2023 IN BRIEF` on page 2, where we now continue to 749.24 —
about 46 pt of body below where the reference is willing to set any. That is a text-area or
keep-with-next question on that page, and the old line-spacing error had been padding page 1 by
just enough to hide it.

This is the cancelling-fixes case the skill describes, and the aggregate is the check on it: page
error 77 → 75 and exactly-correct page counts 165 → 167 over the same change that cost this
document its verdict. The change is right on its own evidence and the document is now wrong for a
reason that was always there.

## What the round refuted

Both refutations are about the ±1 page cluster as a *cluster*, and both were measured with the
control the skill asks for — over the documents that already match.

### 1. The ±1 cluster is not a page-capacity defect

`text-band-census.py` reads every word box out of both renderings and reports how high on the page a
mark ever starts and how low one ever ends — the band of ink, which is what a document fitting a
different amount of text between the same margins would have to differ in.

Over the 23 documents at a page delta of ±1: **14 have a band difference under 1 pt**, and 17 under
3 pt. And the control kills it outright — over the 155 documents that **match**, 36 differ by 3 pt
or more and **17 by 10 pt or more**:

| \|band height difference\| | matching | failing |
|---|---:|---:|
| < 1 pt | 98 | 27 |
| 1–3 pt | 21 | 4 |
| 3–10 pt | 19 | 4 |
| ≥ 10 pt | **17** | 10 |

The bottom edge of the band alone says the same: 105 matching documents within 1 pt, 14 past 10 pt.

The observable is commoner on the documents with nothing wrong with them than on the ones a page out.

### 2. The ±1 divergence pages do not concentrate

`page-boundary-drift.py` aligns the two token streams whole with `difflib` and reports, for each of
our pages, where its end lands in the reference's stream relative to the reference's own page end.
Positive drift means we put more on the page. The first page whose drift passes a floor is the break
that moved; everything after it is consequence.

Over the 23 ±1 documents, the first break with a drift of 30 tokens or more:

| where | documents |
|---|---:|
| page 1–3 | **9** |
| elsewhere in the first third | 6 |
| middle third | 5 |
| last third | 1 |
| no break moves that far | 2 |

They are spread from page 1 to page 91, and `line-anatomy.py` on each of those pages returns
`text` on 22 of 23 — the generic verdict, which on a document that paginates differently is a
restatement of the page delta rather than a finding. **There is no single defect under the ±1
cluster**, and the sub-group worth working is the nine that diverge in the first three pages: a
capacity difference on page 1 is a measurement, and one on page 91 is a cascade. Both documents this
round fixed are in those nine.

As a magnitude, the drift is a fair discriminator and a poor diagnosis:

| max \|drift\|, reference tokens | matching | failing |
|---|---:|---:|
| < 10 | 97 | 5 |
| 10–29 | 20 | 2 |
| 30–99 | 29 | 4 |
| ≥ 100 | **9** | **32** |

Measured over 198 of 200 — the two 700-page documents are too large for a quadratic alignment and are
named as skipped rather than approximated.

### 3. An empty page is not it either

Counting pages with no extractable text on each side: **3 of the 45 failing documents differ from the
reference in that count, against 2 of the 155 matching**. Only one is in the ±1 cluster
(`150_5300_13_chg12.doc`, where the reference draws a genuinely empty page 28 — zero content
operators — and we draw none), and that document's breaks have already diverged by page 5, so the
empty page is not the whole of its difference. Recorded so the next round does not re-derive it: the
`IsSkipEmptyPages` question is not what the ±1 cluster is about.

## Two instruments, and how the first one lied

`page-boundary-drift.py` went through two designs that each produced a clean, wrong answer, and the
file keeps both because the failure is not obvious:

- **A local anchor** — the last eight tokens of our page, searched for near the expected position —
  cannot resolve a page whose last tokens are its **footer**, because the footer holds the page
  number and that number differs on every page after the pagination diverges. Unresolved pages were
  then reported as *aligned*. Measured on `33004.docx`: 47 pages compared, five unresolved, the one
  extra page sitting inside them, and the document scored "every break agrees" while being a page
  long. Ten of the 23 ±1 documents were scored that way.
- **Retrying the anchor further back** fixes that document and invents its own artefact: an anchor
  landing on the wrong instance of a repeated running head reports ±80 tokens of drift on a page
  whose neighbours both read zero.

A whole-stream alignment has neither failure mode and costs 18 s over the track, 83 s on the largest
document it will take. The lesson is the ordinary one stated precisely: **a probe that reports
"no difference" is making a claim, and it needs the same scepticism as one that reports a
difference.**

## Tests

Six in `InlineObjectLineSpacingTests`, verified by reintroduction with `verify-test.sh`:

- reverting the base height to the line's height fails **three** of them and nothing else in
  `Paperless.Text` — `ProportionalSpacingAddsAShareOfTheTextHeightRatherThanScalingTheLine`,
  `TwiceSpacedAddsTheWholeTextHeight`, `TheLayouterGivesSuchALineTheTextHeightsShareAndNotTheLines`;
- removing the *at-or-above-100%* guard, so the base height is used at every percentage, fails
  **one** — `BelowFullSpacingTheWholeLineIncludingTheObjectIsScaled` — which is the refuted rule
  pinned in code.

The other two are preconditions rather than pinned rules and were **not** verified by
reintroduction: `APlainLineOfTextMeasuresTheSameHeightEitherWay` asserts the change is inert on
ordinary text, and `AnInlinePictureRaisesTheLineAndLeavesTheTextHeightAlone` asserts the measurement
reports the two heights apart.

## The cross-track measurement a shared-layer change owes

`Paperless.Text` is below all three families, and `Paperless.Presentations` and
`Paperless.Spreadsheets` both reference `ParagraphLayouter` and `MeasuredParagraph`, so "no gate
input can move" would have been an argument rather than a measurement.

Both other tracks were rendered whole with our own CLI — no `soffice` — at `45fea26c2` and at this
branch, `SOURCE_DATE_EPOCH` pinned, and compared with `/CreationDate` normalised out:

| | documents | rendered both sides | byte-identical | differing |
|---|---:|---:|---:|---:|
| slides + sheets | 334 | **334** | **334** | **0** |

Nothing moved. The single-argument `Apply(natural)` both other layouts call now forwards to
`Apply(natural, natural)`, which is the old arithmetic exactly; only `ParagraphLayouter`'s per-line
path passes a base height apart from the line height, and only a word-processing paragraph reaches it
with an inline object on the line.

## A trap worth carrying forward: a clean `git status` over a reverted `dotnet/src`

The cross-track measurement a `Paperless.Text` change owes needs the *old* code to render with, so
`dotnet/src` was checked out at the base commit to render the slides and sheets tracks. The two
commits that followed used `git add -A`, and each of them committed that revert; the branch then held
the round's tests, probes, results and scoreboard with **none of its code**.

Every ordinary check passed. `git status` was clean — the tree really did match its own HEAD. The
build succeeded, because the reverted source is the source that shipped last round. The tests would
have failed, but they had already been run.

The skill warns that `git add -A` while a mutation is applied commits the defect. This is the same
shape with the sign reversed: `git add -A` while the *fix* is un-applied commits its absence. The
check that catches it is one line, and it is worth running before any commit that says a round
shipped code:

    git diff <base>..HEAD --stat -- dotnet/src      # must be non-empty

Nothing measured is affected — the sweep, the reach comparison and every test count were produced by
a tree built from the fixed source, before the checkout, and the restored tree reproduces the three
gained documents at 9, 12 and 20 pages. What was wrong was only what the branch would have handed on,
which is the part nobody re-measures.

## Left open

- **A picture alone on its line keeps 2.6 pt of the paragraph font's descent that LibreOffice
  drops.** Every probe with text beside the picture matches to 0.05 pt; the three with the picture
  alone are 2.6 pt tall, which is exactly Liberation Serif's 12 pt descent. Not touched, because
  `MeasuredParagraph` carries a measurement from an **ODF** fixture where LibreOffice *does* add that
  descent (`picture-anchor.fodt`, a 1 cm picture giving a 31.46 pt line where 28.35 + 3.11 is the
  arithmetic), and these probes cannot separate a format difference from a text-on-the-line one. The
  document that motivated the round lands within 0.15 pt regardless.
- **`gpp-pr-top-7-office-markets-4q-2023.docx` now sets about 46 pt of body below where the
  reference stops.** A verdict is available there and it is not this rule.
- **The ±1 cluster is 21 documents and no shared cause.** The nine diverging in the first three
  pages are the tractable sub-group.
- **The list-label population is unmeasured.** Three of the eleven renderings that changed carry no
  inline object at all; a numbering level taller than its item is the same rule, and how many
  documents it reaches was not counted.

## Test counts on the final tree

Core 284, Containers 109, Text **277** (271 + six `InlineObjectLineSpacingTests`), Vector 293,
Rendering 121, Markup 259, OpenDocument 125, WordProcessing 746, Spreadsheets 621, Presentations 576,
Fidelity 550 — **0 skipped, 0 warnings**, each project run on its own and its count compared against
the known-good one.
