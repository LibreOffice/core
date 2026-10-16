# L1 — summary

**Reference caveat first.** All measurements are against PDFs produced by **LibreOffice
24.2.7.2** (`Producer: LibreOffice 24.2`; `soffice --version` here). The tree is developed
against **26.2.4.2** (`dotnet/CLAUDE.md`:546, `dotnet/TODO.24-2-7-audit.md`). Two of the
causes below are that version gap, not defects. **No patches are proposed by this lane, and
that is the finding, not an omission** — reasons per cause.

## The headline

The advance divergence explains **17 of the 107 reflow documents (16%)** as the only
measurable difference, and **12 (11%)** quantitatively. The project's implicit assumption
that `reflow` ≈ the advance divergence is wrong by a factor of six. Measured over **296,847
aligned glyphs**: corpus-wide ours/ref = **1.00019**; Carlito **1.00108**, DejaVu Sans
**1.00069**, Liberation Sans **1.00009**, Liberation Serif **0.99958** — confirming and
sharpening `dotnet/CLAUDE.md`'s per-face seat, and showing it survives the version move.

## Root causes, by document count

**A · Advance-width divergence — 17 of 107 (16%) · confidence high / medium-high**
Same face, same size, same measure; only the advances differ, by 0.004–0.4%. Predicted
per-line break-move probability 1.23% against 2.22% observed line-start disagreement, a 1.6×
paragraph-restart multiplier — the model holds for 12 of the 17. **No patch:** reproducing
FreeType's hinted advance at LibreOffice's ppem is architectural, per
`dotnet/CLAUDE.md`'s refuted kerning and grid probes. Prize if closed: 17 documents, ~12
expected to resolve.

**B · Rendered em size differs — 26 (24%) · confidence high (effect) / medium-high (cause)**
Same text, different em size, almost always by one point. Restricted to same-size words the
advance ratio is 1.0000–1.0018, so the size is the whole effect. 17 are autofit decks;
`XRay.Office.Presentations/Layout/SlideAutofit.cs`:31-38,87 says it implements **26.2.4.2's
`constScaleLevels` ladder**, replacing the 24.2.7.2 bisection this sweep's reference used.
**Version divergence — do not re-tune** (L5/L8). Six non-slide cases are real, incl. #106.

**C · Different face resolved — 8 (7%) · confidence high (measurement) / mixed (cause)**
2–27% width differences, entirely the substitution. #101 and #031 are the ordering rule at
`XRay.Office.Text/Fonts/SystemFontResolver.cs`:490-499, whose own comment says it was
**measured against 26.2.4.2** and names `Helvetica` and `Times` as the cases that separate
the versions. **Version divergence in my own file — no patch.** The four
`DejaVu Sans → DejaVu Serif` cases (`Aptos`, `Segoe UI`) are unexplained and need one
authored probe per name at both binaries.

**D · Column width provably different — 5 (5%) · confidence high**
Disjoint line-break brackets: #091 −65.1 pt, #117 −58.5, #129 −28.9, #046 +20.9, #108 −8.9 —
20 to 250× the advance effect. #091 is mine: a `continuous` section break where we lay the
text out at section 2's width from section 1's left origin (`612 − 36 − 72 = 504.0`, inside
our measured limit). **Cross-lane: `XRay.Office.WordProcessing` (L2/L3).**

**E · Line breaks identical — 40 (37%) · confidence high**
Every shared line breaks at the same word, over 6,469 line starts. These documents have **no
text-metrics fault**; the divergence is vertical (page filling, row height, paragraph space).
Several case notes blame "a wider text measure" that measurement shows is not there — do not
act on those readings. #017 is the clean instance: identical breaks, identical 13.8/27.6 pt
pitch, identical text band, and two extra lines per page from page 2 — a blank-paragraph
rule at the page boundary, L2's paginator.

**F · Leading differs — 4 (4%); no measurable text (graph-paper grids) — 5 (5%); advances
6× the band (#005 #007) — 2 (2%).** Reported in `findings.md`; none of them mine.

## Cross-lane dependency this lane found

**`w:w` character scaling (ECMA-376 §17.3.2.43) is read by nobody.** `#106` sets
`<w:w w:val="103"/>` on `Normal`, so the reference sets every glyph 3% wide and we do not —
2.6% narrow over 4,301 glyphs, 3 of 74 line starts agreeing. Needs
`WordParagraphFormats.cs` (read it), `MeasuredParagraph.cs` `FormattedRun` (a `Scale` beside
`Tracking` — mine) and the painter, together. **Deliberately not shipped as a
`XRay.Office.Text`-only diff**: an unconsumed field is the pattern that has already been the
cause four times here. 11 of 271 corpus docx carry a non-100 `w:w`; one carries it
document-wide.

## Not a defect

The constant **−0.100 pt** line-start offset present on 58 of 107 cases is the text origin,
it is the reference that is off the declared margin, and 0.1 pt cannot move a break.
