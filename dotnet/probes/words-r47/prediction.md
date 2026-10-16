# Words round 47 — prediction, committed before the post-change sweep

Base `c87cf8952`. Baseline reproduced **exactly**: 158/200 match, |page error| 70, 168
exact pages, |word error| 6666 (`baseline.tsv`, 200 rows, no duplicate path).

## The change

`InlineObject` gains `RaisesTextHeight`, false for every real as-character object and
**true for the phantom object a list label enters measurement as**. `MeasureLine` folds a
flagged object into the height proportional line spacing takes its percentage of.

One line of behaviour: a numbering label taller than its item now raises the
line-spacing base of the line it sits on, where before it raised only the line.

## Why, and on what evidence

Round 46 recorded, from a citation, that a list label "is the same rule" as an
as-character picture — raises the line, takes no share. Four authored probes against the
installed 24.2.7.2 refute that and fix the alternative:

| probe | rows | wrong before | wrong after |
|---|---:|---:|---:|
| `list-label-line-height.py` (L = 12/14/20/28 pt x 100/150/200% + unlabelled control) | 15 | 8 | **0** |
| `label-and-picture.py` (28 pt label + 100 pt picture, one line) | 4 | 2 | **0** |
| `label-wrapped-paragraph.py` (1-line and 3-line items, per-baseline pitches) | 6 | 2 | **0** |
| `first-line-or-last-line.py` (28 pt word first / last / nowhere, no label) | 6 | 0 | **0** |

The mechanism, cited but not relied on: `SwTextFrame::CalcHeightOfLastLine`
(`txtfrm.cxx`:3952-3957) takes `MaxAscentDescent(…, bNoFlyCnt = true)` — *"i#47162 —
suppress consideration of fly content portions"* — and `GetLineSpace` takes
`(prop − 100)%` of it. A fly is suppressed, a number portion is not. The **dev tree in
this checkout would give the other answer** (`Height(nPosHeight, false)` on its number
branch), which is exactly the case where the binary wins.

`first-line-or-last-line.py` is the control the change is *not* allowed to move, and it
did not: it makes the first and last line of a paragraph differ in height with no label
anywhere, and we already reproduced all six rows before the change and still do.

## The census, and what it cannot see

`label-spacing-census.py` over `words/batch-*`, 200 documents, **134 DOCX read and 66
binary unread**:

| band | count |
|---|---:|
| outer — proportional spacing > 100% anywhere in the chain **and** a numbered paragraph | **93 of 134** |
| inner — and a numbering level stating its own `w:sz` | 55 of 134 |

**Both are ceilings, over the DOCX half only.** Stated before the sweep, the things
neither band can see:

- **The 66 `.doc`.** Their levels live in the WW8 `LSTF`/`LVLF` structures, which no
  zip-level census reads. `Ww8DocumentReader` builds the same `PageLabel` as the OOXML
  reader and the fix is in `XRay.Office.Text`, so the binary half is **reachable and
  entirely uncounted**. Round 45 had to go through LibreOffice's own flat-ODF export to
  see that half at all, and this round did not.
- **A label taller through its *face* rather than its size.** A level set in Symbol,
  Wingdings or OpenSymbol beside a Latin item has a different line box at the *same*
  point size. The inner band is blind to all of those and the outer band cannot tell them
  from a level that changes nothing — which is why the two bands are 38 apart.
- **Whether the label is taller at all.** That needs the level's face and the item's
  resolved face and size, each through its own style chain, and then two line boxes.
- **Whether a taller first line moves a break.** The change adds
  `(prop − 100)% x (label box − text box)` to one line per numbered paragraph — often 1 to
  4 pt. A page moves only when it was within that of full, which is the discount that has
  cost every previous round's estimate.
- **Numbering reached through `w:numStyleLink`/`w:styleLink`**, which the census does not
  follow.
- **The line-spacing shrink branch.** Below 100% Writer scales the whole line and
  `ParagraphFormat.Apply` already takes the other branch, so no paragraph under 100% can
  move. The census counts only `> 240`, so this one it does see.

Round 45's comparable figure was a 20-document ceiling and a measured reach of 11
renderings; round 46's was 130 and 55.

## The bands

| | baseline `c87cf8952` | predicted after |
|---|---:|---|
| documents matching | 158 | **155–162** |
| absolute page error | 70 | **58–80** |
| exactly-correct page counts | 168 | **163–174** |
| absolute word error | 6666 | **6450–6900** |
| renderings changed | — | **10–45 of 200** |

The direction is one-sided: a labelled first line can only get *taller*, so pages can only
gain content-pushing height. The ±1 cluster is 13 documents at −1 (we under-paginate) and
7 at +1, so the same change should help the larger group and hurt the smaller one.

## The cross-track measurement owed

`XRay.Office.Text` is a shared layer and `XRay.Office.Presentations` calls
`ParagraphLayouter`. `RaisesTextHeight` defaults **false** and the only caller that sets
it is `PageParagraph.MeasurementObjects` in `XRay.Office.WordProcessing`, so no slide and no
sheet can move. That is an argument, and the routine asks for a measurement: slides and
sheets will be rendered whole at base and at this tree with `SOURCE_DATE_EPOCH` pinned and
`/CreationDate` normalised, and the result reported whichever way it comes out.

---

# Second change, predicted before its own sweep: a `w:style` with no `w:name` is dropped

`WordStyles.Add` now ignores any `w:style` whose `w:name` is absent or empty, so nothing
can reference it — not `w:pStyle`, `w:rStyle`, `w:tblStyle` or `w:basedOn`.

Evidence: `unnamed-style.py`, six authored variants over three style families, each pair
varying only whether `<w:name>` is present. LibreOffice draws the style's 10 pt with the
name and the document default's 12 pt without it, in **all three families**; we drew 10 pt
either way for paragraph and character styles. Plus three causal mutations of
`template---tpr-technical-progress-report-with-guidance.docx`, whose four table styles are
among the sixteen of its fifty-three that have no `w:name`: changing that style's own
`w:spacing` moves nothing in the reference, changing `w:docDefaults` moves everything, and
*adding* a `w:name` changes the drawn size. Mechanism, as the hypothesis:
`StyleSheetTable.cxx`:774.

## The census, and what it cannot see

Over `words/batch-*`: **2 of 134 DOCX** carry a `w:style` with a `w:styleId` and no
`w:name` that is referenced anywhere — 16 unnamed of 53 in the `tpr` template with 4 of
them referenced, and 13 unnamed of 40 with 12 referenced in
`1603642410-MoM-CASCOM-06-2020-draft04.docx`. Both are exporter artefacts (Google Docs and
a localised Word), which is why the population is small and lumpy rather than spread.

This census is **not** a ceiling in the usual way — it reads exactly the condition the code
tests, over the whole of the format that has the concept. What it cannot see:

- **`w:basedOn` chains through a dropped style.** A named style based on an unnamed one now
  loses that parent's properties too, so a document can move without referencing an unnamed
  style directly. Neither of the two named documents needs this to move, but a third might.
- **The 66 `.doc`**, which have no such concept: a WW8 `STD` carries its name in the same
  record, so nothing there can be nameless. This change cannot reach them, and that is a
  fact rather than a blind spot.
- **Whether the drop moves a page.** `1603642410-MoM-CASCOM-06-2020-draft04.docx` currently
  **matches at 9/9**, so it is a regression risk rather than a gain, and it holds twelve
  referenced unnamed styles against the `tpr` template's four.

## The bands

| | before this change | predicted after |
|---|---:|---|
| documents matching | 158 | **157–160** |
| absolute page error | 70 | **68–72** |
| exactly-correct page counts | 168 | **167–170** |
| absolute word error | 6666 | **6500–6800** |
| renderings changed | — | **2–4 of 200** |

Narrow bands on purpose: the census reads the code's own condition over the only format
that can express it, so this is one of the few cases where a prediction should be tight.
