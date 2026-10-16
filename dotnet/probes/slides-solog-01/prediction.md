# slides-solog-01 — prediction, committed before the remaining measurements

Round: `slides/batch-004/pptx/solog_orientation_august_2019.pptx`, the one `words` verdict in
slides batches 001–006 (57 of 58 match). Briefed as `pages 15/15  words 670/685  fonts 5/6`.

## Honesty note on ordering

Item 0 below was **measured before this file was written**, so it is recorded as a result and
not scored as a prediction. Everything from item 1 down is written blind and is scored in
`results.md`.

## 0. Already measured (not a prediction)

The 15-word deficit is **not missing text**. Extracting both PDFs with `pdftotext` and comparing
the per-page *character* multiset with whitespace removed, all 15 pages agree exactly except for
two `-` characters that `pdftotext` itself eats when it de-hyphenates one of our soft line breaks.
Ref 4758 non-space characters, ours 4756. The 15 tokens are the reference's, produced by
`pdftotext` splitting the reference's own output:

| where | ref tokens | our tokens | Δ |
|---|---|---|--:|
| `MIAMI` in the logo, pages 1, 2, 3, 15 | `M` `IAM` `I` | `MIAMI` | 8 |
| `dtpoole@miami.edu`, page 3 | `dtpoole@m` `iami.edu` | one | 1 |
| `http://bulletin.miami.edu/`, page 4 | `h` `ttp://…` | one | 1 |
| two wrapped URLs, page 5 | 2 + 3 | 1 + 1 | 3 |
| wrapped URL, page 8 | 2 | 1 | 1 |
| wrapped URL, page 9 | 2 | 1 | 1 |
| | | | **15** |

## 1. Predictions about the two font findings

1.1 **The `DejaVuSans-Bold` / `LiberationSans-Bold` split is a real substitution of a real
drawn run, not a `/BaseFont` naming artefact.** Confidence 0.6. The counter-case the brief
cites (`/BaseFont` named from the family rather than the PostScript name) would put the *same*
face on both sides under two names; here the two names denote two genuinely different families
that are both installed, so a naming bug cannot produce it. What I am unsure of is whether the
run that takes it is drawn at all.

1.2 **The requested family is not installed and both sides are substituting — differently.**
Confidence 0.55. Predicted requested family: an MS-only face — `Arial`, `Tahoma`,
`Trebuchet MS`, `Verdana` or `Times New Roman` — asked for in bold. If it is `Arial` we should
be *right* (`Liberation Sans` is the metric-compatible substitute) and the reference is the one
substituting oddly, which would make this a reference quirk rather than our defect.

1.3 **We already have the symbol recode on the slides path.** Confidence 0.9 — it is visible in
`XRay.Office.Presentations/Layout/SlideTextLayout.cs` and `Ooxml/PptxTextBody.cs`, so the brief's
worry that "that work may have been done on the words path only" is predicted **refuted**. The
missing `OpenSymbol` here is therefore predicted to be a *narrower* failure of that wiring on
one particular bullet declaration, not an absent feature.

1.4 **The bullet that should reach OpenSymbol is declared in a slide layout or master rather
than on the slide's own paragraphs.** Confidence 0.4. This is the shape most likely to slip a
resolver that only reads the slide.

## 2. Predictions about reach

2.1 If a code change ships at all, its measured reach over the 163-document slides track is
**2–20 renderings changed**. Confidence 0.55 in the band. The prior the project has earned is
that reach estimated from anything but a resolver comes out fortyfold high.

2.2 **Zero verdicts move on the slides track**, in either direction. Confidence 0.7. The gate
cannot see a bullet glyph (it is excluded from the word count as of 2026-08-13) and cannot see
a face substitution at all.

2.3 `solog_orientation_august_2019.pptx` **stays a `words` failure whatever I fix**, because
item 0 says the 15 tokens are the reference's tokenisation and no change to our renderer can
manufacture them. Confidence 0.85. The honest outcome for the gate row is a documented
ceiling, not a pass.

## 3. Prediction about the regression run

3.1 `slides/batch-001` … `batch-006` re-run together stays at **57 of 58 match**, with the same
single row failing. Confidence 0.75.

## 4. Prediction about the blind reading

4.1 The fresh reviewer will report a **bullet or glyph difference** on the worst page before it
reports anything about a bold face. Confidence 0.5.

4.2 The reviewer will name at least one defect that appears nowhere in this brief. Confidence
0.65 — this has been the outcome every previous time the project has run one.
