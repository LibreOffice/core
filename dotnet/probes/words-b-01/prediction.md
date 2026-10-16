# Words-B round 1 — prediction, written before a single render

Subject: **list labels and the line-spacing base height**. Written against the *installed*
`soffice`, which is **LibreOffice 26.2.4.2 620(Build:2)**. Every stored figure on this
track, and the whole of round 47, was measured against **24.2.7.2**. There is no
`XRay.Office.Cli` in this container (nuget.org is firewalled, package cache empty), so
**nothing below can carry an "ours" column measured from a running build**; our side is
read out of the source and computed by hand, and that limit is restated in the results.

## What round 47 established, which I am not re-deriving

Against 24.2.7.2, over 31 authored rows:

- A list label taller than its item **does** raise the height that proportional line
  spacing takes its percentage of: `gap(p) − gap(100) = (p−100)% × box(L)` where `box(L)`
  is the *label's* line box, at L = 14, 20, 28 pt over 12 pt text.
- The base is the tallest non-fly portion, not the whole line: a 100 pt as-character
  picture beside a 28 pt label still gave `+32.20`, not `+114`.
- It shipped `InlineObject.RaisesTextHeight` and measured the reach at **1 of 200**
  renderings, 0 verdicts moved.

## The prediction

### 1. Round 47's central measurement will not reproduce on 26.2.4.2 — the label will be *excluded*

I expect the labelled rows to collapse onto the unlabelled control: at p = 200% the
extension will be `+13.8 pt` (= `box(12 pt Liberation Serif)`) for **every** level size
L ∈ {14, 20, 28}, where round 47 measured `+16.10 / +23.00 / +32.20`.

Why I expect it, stated as reading rather than measurement — the C++ tree in this
checkout is 27.2.0.0.alpha0+ and is reference material only, but two things in it are new
since 24.2 and both point the same way:

- `SwLinePortion::IsUsedToCalcLineSpacingHeight` (`sw/source/core/text/porlin.cxx`:324)
  now gates the base height, and it excludes `InNumberGrp()` — *"bullets and numbering,
  FootnoteNum, GrfNum"* — in **both** of its branches. Legacy (`!LINE_SPACING_AS_GAP_BELOW`)
  returns false for anything that is not a text portion; the new branch excludes
  `InNumberGrp()` by name.
- `SwLineLayout::CalcLine` (`porlay.cxx`:454-478) now passes that predicate at both the
  `Height(nPosHeight, …)` call and the `SetLineSpacingBaseHeight` guard, where round 47
  recorded the number branch as an unconditional `Height(nPosHeight, false)`.

If 26.2.4.2 already carries this, round 47's law is a 24.2 law and the fix it shipped is
now wrong in the direction it was right in before. **I am predicting the refutation of my
own brief's standing hypothesis**, and it is the whole reason this round measures rather
than reads.

Confidence: 0.7 that the labelled rows collapse to the control; 0.2 that 26.2.4.2 still
behaves as 24.2 did and round 47 reproduces to the digit; 0.1 that it is neither, i.e. a
third value I have not modelled.

### 2. The gap will have moved from above the line to below it

`SwTextFormatter` (`itrform2.cxx`:2443-2448) now takes the base height from
`GetPrev()` rather than from the current line when `LINE_SPACING_AS_GAP_BELOW` is set —
*"Like Microsoft Word, apply the line spacing gap after the line"* — and
`ww8par.cxx`:2055 says the WW8 filter relies on that setting defaulting to **true**.

Concretely, in a five-line paragraph at 200% with a 28 pt word on line 3 and 12 pt
elsewhere: the gap-above model enlarges the **L2→L3** pitch by `(p−100)% × box(28)`, and
the gap-below model enlarges **L3→L4** instead. Those are 32 pt apart and cannot be
confused.

Confidence: 0.6 that the gap is below on 26.2.4.2 for a DOCX.

This one is **not confined to labels**, and I flag the overlap up front: it is round 45's
law, and if it has moved it has moved for every paragraph with proportional spacing, not
just numbered ones. I will measure it because the label's contribution is only observable
*through* it, and report it as a general finding rather than annexing it.

### 3. Where our model is structurally different whatever 26.2 says

`MeasuredParagraph.MeasureLine` (`dotnet/src/XRay.Office.Text/Layout/MeasuredParagraph.cs`:661)
folds the label in as `textHeight = Max(textHeight, one.AboveBaseline + one.BelowBaseline)`
— the label's **own whole box**. For runs the same method builds `Max(height, ascent +
descent)` from *maxima across portions*. The two disagree exactly when the label supplies
the ascent and something else on the line supplies a deeper descent. I predict LibreOffice
takes the tallest single portion's height (`GetLineSpacingBaseHeight` is a max over
`nPosHeight`), i.e. **our composition is right and round 47's prose calling it "the label's
own line box" is right too** — but I have never seen it separated, and round 47 could not
separate it because every one of its rows scaled one font family, where the two models are
identical by construction.

Confidence: 0.55 that tallest-portion wins over max-ascent-plus-max-descent. This is the
one prediction I hold weakly on purpose.

### 4. Reach

Round 47's own refinement — levels stating a `w:sz` larger than the document default run
size, and proportional spacing above 100% — gave **17 of 134 DOCX**, and the one document
that actually moved was **not among the 17**. My census will resolve rather than declare
(see below), so I expect it to land near but not on that figure.

| band | predicted |
|---|---|
| DOCX resolving to a label taller than its item, anywhere | **45–75 of 134** |
| …and that item at proportional spacing > 100% | **12–25 of 134** |
| renderings whose page count would change if the label were excluded from the base | **0–3 of 200** |
| verdicts moved (pages, ±2% words, unembedded fonts) | **0** |

I am predicting **no verdict movement** outright. A spacing law of 1–4 pt on one line per
numbered paragraph is invisible to all three gates except through a page that was already
within a line of full, and round 47 measured that discount at 1 rendering in 200 on a
change of the same size. If this round is right and moves nothing, that is the result, and
I will say so rather than dress it up.

## What my census cannot see — named before it is run

The census resolves each `w:p` to a label size and an item size through
`w:docDefaults` → `w:style` chain via `w:basedOn` → `w:pPr/w:rPr` (the paragraph mark,
which is what a label actually inherits) → the level's `w:lvl/w:rPr/w:sz`, and the
numbering through `w:numPr`, the paragraph style's `w:numPr`, and `w:numStyleLink`. It is
still a **ceiling**, and these are the holes:

- **The 66 `.doc` are wholly invisible.** Their levels live in WW8 `LSTF`/`LVLF`
  and no zip-level census reads them. The law is in `XRay.Office.Text`, which both readers
  share, so that half is reachable and entirely uncounted. Round 45 had to go through
  LibreOffice's own flat-ODF export to see it at all; I am not doing that, so the true
  reach could be up to half again as large than anything I report.
- **The ODF reader.** `OdtLayoutSource.Lists.cs` builds the same `PageLabel`. There are no
  `.odt` in `words/`, so this is zero *here* and non-zero for the product.
- **A label taller through its face rather than its size.** A level in Wingdings or Symbol
  beside a Latin item has a different line box at the *same* point size — OpenSymbol's
  ascent and descent are not Liberation Serif's. This is precisely the case round 47's one
  moving document turned out to be, and a size-comparing census scores it as *no label
  effect at all*. I will count it separately as a face band and I will not be able to say
  which way each one goes without resolving two font files per paragraph.
- **`w:lvlOverride`/`w:startOverride` and `w:numStyleLink`/`w:styleLink` chains.** I follow
  `w:numStyleLink` one hop and `w:lvlOverride`'s `w:lvl`; a two-hop chain, or a level
  reached only through `w:pStyle` → style-linked list, may resolve to the wrong level and
  therefore the wrong size.
- **`w:rStyle` inside `w:lvl/w:rPr`.** Neither our `LabelSize` nor my census follows a
  character style named by the level, so a level whose size arrives that way scores as
  "inherits the item's size" in both, and the two agree for the wrong reason.
- **Theme fonts and `w:szCs`.** A complex-script size or a `w:asciiTheme` indirection is
  read for the family but not for the size.
- **Whether a taller first line moves a break at all.** The census counts paragraphs, and a
  page moves only when it was within a few points of full. That discount is what has cost
  every previous round's estimate on this track, and I have no way to compute it without a
  CLI, so the "renderings changed" band above is an *argument*, not a census.
- **The whole "ours" column.** With no build I cannot render one page of our output. Every
  statement about what we currently draw is read from source, and where I give a number
  for our side it is arithmetic on font tables in the same shape the source does it, not a
  measurement. Any such row can be wrong in a way only a build would show.

## Addendum, written after the firewall opened and still before the first render

The coordinator lifted the nuget block partway through writing this file. `dotnet restore`
and `dotnet build` now succeed and `tools/XRay.Office.Cli/…/XRay.Office.Cli` exists and runs;
durable 26.2.4.2 reference renderings for all 200 words documents are at
`/c/sandbox/workdir/refpdfs-26.2.4.2/words/`. **Nothing above has been measured yet** —
this addendum is appended before the first `soffice` or CLI invocation of the round, and
nothing above it is altered.

What that changes, and what it does not:

- Every probe below now gets an **ours** column measured from a real render rather than
  computed from font tables. The paragraph in the preamble about having no "ours" column
  no longer applies to the probes; it still applies to nothing else, because I never got
  as far as using it.
- The reach band stops being an argument and becomes a **measurement**: ours rendered
  against the stored 26.2.4.2 references over all 200, before and after any change. The
  standing rule still binds — reach is what a paragraph *resolves* to, not what a part
  declares — so the resolving census is still run, and now it is checkable against the
  rendered answer instead of standing in for it.
- The bands themselves I leave exactly as predicted: 45–75 DOCX with a taller label,
  12–25 of those at proportional spacing above 100%, **0–3 of 200 renderings changed**,
  **0 verdicts moved**. Raising them now that they are checkable would be the one move
  that makes a prediction worthless.
- New, and predicted here rather than after the fact: I expect our current tree to be
  **wrong** on the labelled rows in the direction round 47 put it — 26.2.4.2 excludes the
  label and r47 taught us to include it — so the probe should show ours too *tall* on
  labelled paragraphs at p > 100%, by `(p−100)% × (box(L) − box(item))`.
- `SOURCE_DATE_EPOCH=1700000000` and `TZ=UTC` on every render, per the updated brief.

## The controls this round is not allowed to fail

- **An unlabelled paragraph must reproduce round 45's law on 26.2.4.2** before any labelled
  row is believed. If the control has itself moved, the instrument is measuring the engine
  change and not the label, and every labelled row is uninterpretable.
- **At p = 100% every row must come back equal**, labelled or not, up to the label's own
  effect on the raw line height. A difference at 100% means the probe is measuring
  something other than the spacing base.
- **The five-line probe must give a flat pitch when nothing is tall**, so that a pitch
  difference is attributable.
