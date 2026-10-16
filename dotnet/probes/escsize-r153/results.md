# Round 153 — O78 seat A: an escaped size is truncated to a twip, not rounded

**Written incrementally.** Each section was written as its measurement finished.

## 0. What this round is

Round 152 (`probes/faa-r152/results.md` §7) named two seats for **O78** and said seat A must ship
first: `Escapement.SizeOf` quantises a superscript's size differently from 26.2.4.2, and on
`FAA 2025-26 Holdover Tables.docx` that one tenth of a point is the whole of a 7.05 pt wrap, a
5.00 pt displacement at the foot of page 82 and a blank page 83.

It also named the rule to put in its place — *round to the nearest tenth of a point* — and this
round **refutes that** and ships truncation to a twip instead. The two agree at the one base size
round 152 could discriminate at, which is why its conclusion survived its own controls.

### Environment

| | |
|---|---|
| tree | `/home/user/libreoffice-core`, base `a3159ce0d` *(A split table row is ruled once at the cut (O83))* |
| ours | `dotnet/tools/…/XRay.Office.Cli`, rebuilt 2026-09-18 12:18:43 UTC for the `head` leg |
| reference | `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2 `0229ac93fcf0d7cbc6376066c6f35021cef002dc`** |
| C++ read | this checkout, `configure.ac` **27.2.0.0.alpha0+** — *not* the reference binary's source |
| date | 2026-09-18 |

---

## 1. [bin] The instrument round 152 used cannot see this rule, and the reason is one line of the PDF writer

`esc-score.py` in that round reads `span['size']` out of the rendered PDF, which is the `Tf`
operator. Rendering `data/adv.docx` — one superscript run at each of 33 base sizes — and reading
both channels off the *same* PDF:

| base | what the `Tf` says | what the ADVANCE says |
|---|--:|--:|
| 5.5 pt | 3.2 pt = **64 tw** | **62.977 tw** |
| 6.0 pt | 3.5 pt = **70 tw** | **69.001 tw** |
| 8.0 pt | 4.6 pt = **92 tw** | **91.971 tw** |
| 11.0 pt | 6.4 pt = **128 tw** | **126.989 tw** |
| 18.0 pt | 10.4 pt = **208 tw** | **207.992 tw** |

**Every one of the 33 `Tf` values is a whole tenth of a point**, and at 21 of the 33 it is not the
size the layout used. So "26.2.4.2 rounds an escaped size to a tenth of a point" is a statement
about `pdfwriter_impl`'s number formatting, and the tenth-of-a-point rule was fitted to it.
`data/adv-ref.txt`.

## 2. [bin] The rule, measured through an advance: truncate to a whole twip, 81 of 81

The channel is CLAUDE.md's own prescription for an advance — difference two **right-aligned** lines
so that every fixed term cancels. Each arm is a right-aligned line holding a label and then N
copies of one digit in a superscript run; two arms per cell differ only in N (2 and 42), so the
difference of the digit span's left edge is exactly 40 advances of one glyph at whatever size the
layout used, with the label, the margin and the side bearing all cancelled. The glyph's `hmtx`
advance is read from the face itself, so the size follows with no free parameter.

| fixture | what varies | arms | `trunc` | `round` to a twip | `round` to a tenth |
|---|---|--:|--:|--:|--:|
| `make-adv.py` (`.docx`, `w:vertAlign`, proportion 58) | 33 base sizes, 4.0–20.0 pt | 33 | **33** | 21 | 18 |
| — *of those, where the three rules disagree* | | 21 | **21** | 9 | 6 |
| `make-prop2.py` (`.fodt`, `style:text-position`) | proportions 25–99 at 11 and 13 pt | 48 | **48** | 24 | 22 |
| — *of those, where the three rules disagree* | | 38 | **38** | 14 | 12 |

**Truncation is right at 81 of 81 and at 59 of 59 discriminating arms; no other candidate is.**
`data/adv-ref.txt`, `data/prop2-ref.txt`.

Two notes on the second fixture. The 40 pt arms are dropped — 42 digits at that size overrun the
measure — and a strongly raised small run lands on a PyMuPDF *line* of its own, so `prop-score2.py`
pairs a label with its digits geometrically (the digits begin where the label ends, on the same
row) rather than by being on one line. The first cut of that pairing matched on x alone and
produced a table of plausible nonsense; requiring the y overlap as well is what fixed it.

## 3. [src] And that is what the C++ does, in two places

`SwSubFont::SetSize` (`sw/source/core/inc/swfont.hxx`:772-783) sets the shrunk font to

```cpp
Font::SetFontSize( Size( m_aSize.Width()  * GetPropr() / 100,
                         m_aSize.Height() * GetPropr() / 100 ) );
```

— integer division, in the layout's own unit, which for Writer is the twip.
`SvxFont::SetPhysFont` (`editeng/source/items/svxfont.cxx`:348-364) does the same for an EditEngine
text. Neither rounds, and neither knows about a tenth of a point.

This is the [src] half; §2 is the [bin] half and stands on its own.

## 4. The change

`dotnet/src/XRay.Office.WordProcessing/Layout/Escapement.cs`:

```csharp
-    => Proportion is 0 or 100 ? emSize : Twips(emSize.Twips * Proportion / 100.0);   // Math.Round
+    => Proportion is 0 or 100 ? emSize : Length.FromTwips(emSize.Twips * Proportion / 100);
```

`SizeOf` is the single seat all four word readers use — `DocxLayoutSource.cs`, `OdtLayoutSource.cs`,
`RtfReader.cs`, `DocReader.cs` — so this reaches DOCX, ODT, RTF and WW8 at once. The two rules part
company at **29 of the 57 half-point sizes** between two and thirty point at proportion 58, which is
why the round is swept whole-corpus rather than censused.

The file's own remarks carried the refuted claim — *"58% of eleven point is 127.6 twips, and
LibreOffice draws the citation at 128 — 6.4 pt, not 6.38"* — which is §1's `Tf` read of exactly this
document. It is replaced by the measurement and by a warning not to re-measure it that way.

## 5. The fixture, the tests and the pin

`dotnet/tests/corpus/features/words-escapement-size.fodt` — seven paragraphs, all drawing the same
word at eleven point, differing in one attribute: no escapement, then proportions 58, 49, 53 (as a
subscript), 25 and 100, then a paragraph holding an unescaped run and an escaped span. A width
ratio is therefore a size ratio exactly.

Confirmed at the reference before it was asserted: 26.2.4.2 draws the 58, 49 and 25 per cent arms
at **127.12, 107.12 and 55.03** twips of the unescaped arm's width against this tree's 127, 107 and
55, where the rounded answers would be 128, 108 and 55. `data/fixture-ratios.txt`.

`EscapementSizeTests` is 24 assertions: the arithmetic at eight base sizes where the rules differ,
at seven proportions, the two no-op proportions, the five drawn arms and the span. **Mutation-pinned**
(`mutate.sh`) against four rival rules, each built and run in turn, with the harness grepping the
build output for compiler errors so that a non-compiling arm cannot read as a passing one:

| arm | result |
|---|---|
| round to a twip | **18 of 24 failed** |
| round to a tenth of a point | **14 of 24 failed** |
| no quantisation at all | **18 of 24 failed** |
| truncate to a tenth of a point | **14 of 24 failed** |
| base (truncate to a twip) | 24 passed |

## 6. Reach

`census.py`, `data/census.txt` — the denominator, not the reach:

| | states an escapement |
|---|---|
| corpus `.docx`/`.docm` (`w:vertAlign` super/subscript) | **57 of 271** |
| converted `.odt` (`style:text-position` with a proportion below 100) | **84 of 337** |
| converted `.rtf` (`\super`/`\sub`) | **84 of 337** |
| corpus `.doc` | not censusable statically |

**Every proportion the corpus states is 58** — 417 `super 58%` and 60 `sub 58%` over the 96 `.odt`
that state a text position at all, and nothing else below 100 — so the other proportions in §2 are
authored, and `style:text-position`'s rise is written by LibreOffice as the keyword `super`/`sub`
rather than as a percentage. A census matching only the numeric spelling finds **none** of them;
the first cut of `census.py` did exactly that and reported 0 of 337.

## 7. [bin] O78's witness: the document lands where round 152 said it would, to the hundredth

`FAA 2025-26 Holdover Tables.docx`, this leg's own render against the banked 26.2.4.2 rendering in
`/home/user/refpdfs-words-26.2.4.2/1/` (rendered 2026-09-17, after the Narrow-font move, so it is
not one of the suspect ones).

| | 26.2.4.2 | base (round 152's measurement) | **head** |
|---|--:|--:|--:|
| pages | 167 | 167 | **166** |
| last grid line on page 82 | 544.70 | 549.70 | **542.65** |
| displacement at the foot of page 82 | — | **+5.00** | **−2.05** |
| Σ of the 33 row deltas | — | +5.05 | **−2.00** |
| page 83 | 1751 alphanumerics | **74 — blank** | **correct** |
| pages whose orientation differs | — | 10 | **2** |

Round 152 simulated this fix by re-spelling the document's superscript as an explicit `w:sz` and
measured **−2.05**, Σ **−2.00**, 166 pages, page 83 correct, orientation mismatches 2. The shipped
fix reproduces its simulation **to the hundredth of a point on both figures**, which is as clean a
confirmation as that method offers: the simulation changed the document and this changed the
renderer, and they agree.

**Seat B is still open and is what the remaining 2.05 pt is** — a `w:vMerge` continuation cell's
stated top rule, which `DocxLayoutSource.Tables.Resolved` drops before `OwnTopRule` can see it.
Round 152 measured both together at **−0.05**.

**And the two pages that still differ are the defect round 152 predicted this would expose.** From
page 127 our stream runs one page *ahead*: 26.2.4.2 emits a near-empty page 127 — 160 alphanumerics,
its header and footer and nothing else — and we do not. That page was invisible while the
document's two faults cancelled in the page stream; it is now the whole of the remaining gap and it
is not this seat.

## 8. [bin] Confinement and reach: 118 of 1620 renderings move, all of them word-processing

Our half of the whole corpus, of the 337 converted `.odt` and of the 337 converted `.rtf`, rendered
once at the round's base and once with the fix under `SOURCE_DATE_EPOCH=0`, one output directory per
document. `sweep.sh`, `diff-legs.py`, `confinement.txt`.

| family | ext | moved | of |
|---|---|--:|--:|
| words | docx | **29** | 271 |
| words | doc | **11** | 66 |
| odt column | odt | **39** | 337 |
| rtf column | rtf | **39** | 337 |
| slides | ppt / pptx | **0** | 302 |
| sheets | xls / xlsm / xlsx | **0** | 307 |

**118 of 1620 moved and 1502 are byte-identical**, and the 609 slides and sheets renderings are
untouched — the layering holding, measured rather than inferred, because `XRay.Office.Presentations`
has an escapement of its own and a different unit.

Beside §6's denominator this is the *"census what a rule paints"* rule again: **57 `.docx` state an
escapement and 29 of them draw a different page for it**, because the two rules agree at 28 of the
57 half-point sizes and a document whose superscripts are all at 10 pt cannot move.

## 9. [bin] The instrument that scores it, and the two that cannot

**A document-level mean of |Δx| cannot see this change.** An escaped run's size moves the spans that
follow it on its own line and nothing else, so on a page of a hundred spans the mean is diluted by
two orders of magnitude: `score.py` reports 116 of 118 documents *level*, which is true and useless.
Per span (`score2.py`), of the **5808 spans that moved between the two legs, 3372 are closer to
26.2.4.2 and 2406 further** — a near coin flip, because a 0.05 pt shift lands either side of a
position we already differ from the reference on for unrelated reasons.

**What answers the question is the size itself, through §1's own quantisation.** The reference's
`Tf` is its size rounded to a tenth of a point, so rounding *ours* to a tenth must reproduce it —
and the two candidate rules differ by one twip, which straddles a tenth boundary in exactly the
discriminating cases (92 tw → 4.6, 93 tw → 4.65 → 4.7). `score3.py`, over the 2947 spans in 108
documents whose drawn size moved:

| | matches 26.2.4.2's `Tf` |
|---|--:|
| base (round to a twip) | **1695 of 2947 — 57.5 %** |
| head (truncate) | **2936 of 2947 — 99.6 %** |

**17 documents better, 0 worse, 91 level** — level being the documents where the channel is blind,
`150-5370-10H.docx`'s 352 eleven-point superscripts among them, since 127 and 128 twips both print
as 6.4.

**The 11 residual spans are not counter-evidence and each is a different question.** Eight are
single characters where the reference's `Tf` is 5.8 — 58 % of *ten* point — and ours is 58 % of
eight or eleven, so it is our **base** size that differs there and the escapement is not in it. The
other three carry `Tf` values of 4.9744, 7.1751 and 7.2000, which are not tenths of a point at all,
so that text is drawn under a text matrix and the operator is not comparable.

*And the first cut of `score3.py` scored the head leg **0 of 352** on `150-5370-10H.docx` and
reported 81 documents worse.* It rounded to a tenth in floating point, where 6.35 pt reads back as
6.34999 and rounds **down**. Rounding through the twip count — which is what the writer has —
inverts that verdict. A scorer whose arithmetic is a tenth of the quantity it is scoring has to be
exact.

## 10. [bin] One gate verdict moves, and it moves the wrong way on the document this fixes

Of all 1620 renderings, **exactly one changes a page count or an alphanumeric count**: the witness.

| `FAA 2025-26 Holdover Tables.docx` | 26.2.4.2 | base | head |
|---|--:|--:|--:|
| pages | 167 | 167 | **166** |
| alphanumeric characters | 335 603 | 336 754 | **336 680** |

So its gate verdict goes from `match` to a **page mismatch**, while the rendering is unambiguously
better: page 83 is no longer blank, the displacement at the foot of page 82 goes +5.00 → −2.05 pt,
and ten pages in the wrong orientation become two. The page count agreed **because the document's
two faults cancelled** — our surplus blank page 83 and the near-empty page 127 that 26.2.4.2 emits
and we do not — and closing one of them exposes the other, exactly as round 152 predicted it would.

This is the `w:pgBorders` argument in reverse: the gate's columns are not a measure of whether a
page is right, and a round that optimised them would have kept the blank page. Recorded here, and in
`OPEN-ISSUES.md`, so that the row is not read later as a regression.

## 11. Five fidelity tests read the reference's `Tf` as if it were exact, and now round it the writer's way

The suite is green at its known baseline except for five tests, and all five are C17 in the test
kit rather than a defect: they compare a font size against the number 26.2.4.2's PDF *printed*.

| test | expected | drawn |
|---|--:|--:|
| `FootnoteComparisonTests.EveryCitationIsSetAndRaisedAsLibreOfficeSetsIt` (`.docx`, `.doc`, `.odt`, `.fodt`) | `6.40@4.40` | **`6.35@4.40`** |
| `PdfOutputComparisonTests.EveryLineIsShownWhereLibreOfficeShowsIt` (`footnotes.docx`) | 6.4 ± 0.001 | **6.35** |

Eleven point is 220 twips, 58 per cent of it is 127.6, the reference's layout sets **127** — 6.35 pt
— and its writer prints **6.4**. The old expectation was met by a size that was wrong by a twip, so
these tests were passing *because* of the defect.

`XRay.Office.TestKit.LibreOffice.PdfFontSizes.AsLibreOfficePrintsIt` is the correction: both sides'
sizes are rounded to a tenth of a point through their twip count before they are compared, which is
a no-op on the reference's half. The alternative — loosening the tolerance to 0.05 pt — would accept
a size half a tenth out in either direction, and the other alternative, printing a coarser size in
our own PDF, is making our output worse to make a comparison exact. The rise, the pen and the glyph
counts keep their own resolution, and the five citation sizes this test distinguishes (11.00, 6.40,
5.80, 10.00) are a tenth apart or more, so nothing it was built to catch is given up.

## 12. The suite

Every project run individually and totalled, on the binary this round ships:

| project | result |
|---|---|
| `XRay.Office.Core` | 591 passed |
| `XRay.Office.Text` | 750 passed |
| `XRay.Office.Vector` | 309 passed |
| `XRay.Office.Containers` | 109 passed |
| `XRay.Office.WordProcessing` | **2072** passed (2048 at the round's base, plus this round's 24) |
| `XRay.Office.Spreadsheets` | 1484 passed |
| `XRay.Office.Presentations` | 1205 passed |
| `XRay.Office.Markup` | 259 passed |
| `XRay.Office.Fidelity` | **10 failed of 552**, 0 skipped — the round's base figure exactly |

The ten are the known set: `TabStopComparisonTests` on `list-label-overrun` in four formats,
`JustificationShrinkComparisonTests`, `PageDrawingComparisonTests` on `paginated` in four formats
and `SheetDrawingComparisonTests` — the families `dotnet/CLAUDE.md` records as left failing on
purpose. The five that §11 corrected are green and the suite is back on its baseline, not below it.

Build: 0 warnings, 0 errors.
