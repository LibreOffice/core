# fidelity-b-01 — three of the twelve, shipped and measured

Round: Fidelity-B, an *implementation* round on the twelve genuine XRay.Office defects
`dotnet/probes/fidelity-01/results.md` classified out of the 40 Fidelity failures. Worktree
`/c/sandbox/workdir/wt-fid-b`, branch `wt-fid-b`, base `63d5290aacf`.

**The prediction is `prediction.md` in this directory, committed at `036865cbfb8` before the suite
was run once.** It is scored in §7 without editing.

Throughout: **measured** means I ran it and read the number; **inferred** means I reasoned to it and
could not close the loop.

---

## 0. Environment, quoted

`.claude/skills/libreoffice-reference/scripts/check-env.sh`, run first:

```
== 1. soffice binary ==   OK  LibreOffice 26.2.4.2 620(Build:2)
== 2. application modules ==   OK  a document actually converts (writer module present)
== 3. metric-compatible fonts ==
  OK  Calibri -> Carlito        OK  Cambria -> Caladea
  OK  Arial -> Liberation Sans  OK  Times New Roman -> Liberation Serif
  OK  Courier New -> Liberation Mono   OK  DejaVu Sans -> DejaVu Sans
== 4. PDF rasteriser ==   OK  pdftoppm 26.01.0
== 5. PDF extractor ==    OK  pdftotext 26.01.0
Environment is good.
```

`df -h /` → **9.5 GB free, 50 % used** at the start and **9.4 GB** at the end. The full-disk
signature does not apply: no test in this round died in under a millisecond.

---

## 1. The counts, before and after, with both failing sets named

| | total | passed | failed | skipped |
|---|---:|---:|---:|---:|
| `XRay.Office.Fidelity.Tests` at `63d5290aacf` (baseline, re-measured here) | 550 | **510** | **40** | 0 |
| at `dbdae4be370` (this round) | 550 | **519** | **31** | 0 |

**Nine turned green. Nothing turned red.** The two failing sets were diffed in both directions, not
compared by count — `comm -13` over the sorted sets is empty, which is the check `prediction.md`
§P7.1 said a count alone cannot make.

### The nine that turned green

```
FootnoteComparisonTests.TheRuleAboveTheNotesGoesWhereLibreOfficeDrawsIt(footnotes.doc)
FootnoteComparisonTests.TheRuleAboveTheNotesGoesWhereLibreOfficeDrawsIt(footnotes.docx)
FootnoteComparisonTests.EveryNoteSitsAtTheFootOfItsOwnPage(footnote-pages.doc)
FootnoteComparisonTests.EveryNoteSitsAtTheFootOfItsOwnPage(footnote-pages.docx)
PdfOutputComparisonTests.EveryShadeAndRuleIsFilledWhereLibreOfficeFillsIt(footnotes.doc)
PdfOutputComparisonTests.EveryShadeAndRuleIsFilledWhereLibreOfficeFillsIt(footnotes.docx)
NoteRestartComparisonTests.TheNumbersAreTheOnesLibreOfficeDraws(note-restart.doc)
SlideTableComparisonTests.EveryCellsTextIsDrawnWhereLibreOfficeDrawsIt
SlideTableComparisonTests.EveryGridLineIsTheStrokeLibreOfficeDraws
```

### The 31 that remain

```
 1  EndnoteComparisonTests.EndnotesCollectWhereTheirDocumentPutsThem(endnotes.docx)
 2  ExtractionComparisonTests.NothingTheReferenceFindsIsMissingFromTheFeatureDocument(tables.doc)
 3  ExtractionComparisonTests.NothingTheReferenceFindsIsMissingFromTheFeatureDocument(tables.docx)
 4  ExtractionComparisonTests.NothingTheReferenceFindsIsMissingFromTheFeatureDocument(tables.fodt)
 5  ExtractionComparisonTests.NothingTheReferenceFindsIsMissingFromTheFeatureDocument(tables.odt)
 6  ExtractionComparisonTests.NothingTheReferenceFindsIsMissingFromTheFeatureDocument(tables.rtf)
 7  FrameComparisonTests.TextFillsBothSidesOfAFrameThatTouchesNeitherMargin(frame-parallel.fodt)
 8  FrameComparisonTests.TextFillsBothSidesOfAFrameThatTouchesNeitherMargin(frame-parallel.odt)
 9  JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt(justify-shrink-2013.docx)
10  JustificationShrinkComparisonTests.TheReferenceItselfSetsTheModeFifteenDocumentInFewerLines
11  NoteRestartComparisonTests.TheNumbersAreTheOnesLibreOfficeDraws(note-restart.docx)
12  PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt(paginated.doc)
13  PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt(paginated.docx)
14  PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt(paginated.fodt)
15  PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt(paginated.rtf)
16  SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt(sheet-rich-text.xlsx)
17  SheetSpilledTextComparisonTests.AStringSpillingPastAPageBreakIsDrawnOnBothSidesOfIt
18  SheetSpilledTextComparisonTests.EveryPageShowsAsManyWordsAsLibreOfficeShows(xls-features.xls)
19  SheetTextComparisonTests.EveryCellIsDrawnWhereLibreOfficeDrawsIt(sheet-cell-text.xlsx)
20  SlideAutofitParagraphSpaceComparisonTests.TheFitsSpacingScaleReachesAParagraphsOwnSpace
21  SlideChartFaceComparisonTests.TheThemesFaceDecidesTheValueLabelsAdvances
22  TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop(list-label-overrun.doc)
23  TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop(list-label-overrun.docx)
24  TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop(list-label-overrun.fodt)
25  TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop(list-label-overrun.odt)
26  TableAutoLayoutComparisonTests.EveryCellStartsWhereLibreOfficeStartsIt(table-autofit-full.fodt)
27  TableAutoLayoutComparisonTests.EveryCellStartsWhereLibreOfficeStartsIt(table-autofit-mixed.fodt)
28  TableAutoLayoutComparisonTests.EveryCellStartsWhereLibreOfficeStartsIt(table-autofit.fodt)
29  TableComparisonTests.EveryCellHoldsItsTextWhereLibreOfficeDoes(table-exact-row.doc)
30  TableComparisonTests.EveryCellHoldsItsTextWhereLibreOfficeDoes(table-exact-row.docx)
31  TableComparisonTests.EveryCellHoldsItsTextWhereLibreOfficeDoes(table-exact-row.rtf)
```

26 of those 31 are `fidelity-01`'s "the reference's own behaviour" and its two unexplained — 2-8,
9-10, 12-15, 17-19, 20-21, 22-25, 26-31 — and were deliberately not touched. The remaining three
are 1, 11 and 16, and §4-§6 account for each.

---

## 2. The separator rule, confirmed by authored probe in both Word formats and for ODF

`probes/fidelity-b-01/separator-probe.py`, run against the **installed 26.2.4.2**. One newly
authored minimal FODT — one body paragraph, one footnote, stated margins — converted by `soffice`
to ODT, DOCX, DOC and RTF, each rendered, and the rule read out of the PDF's own path. Nothing is
copied from the corpus or from any real document.

**The first run of this probe found nothing at all, and that is worth recording**: LibreOffice does
not write the rule as `x y w h re f`, which is what I had written the reader for. It writes an
explicit closed polygon, `x y m x y l x y l x y l x y l h B*`. The parser now takes the bounding box
of a `m`/`l` subpath ended by a painting operator.

### The length: two inches, and absolutely rather than proportionally

```
=== column 481.890 pt (margins 2cm), default paragraph style 12pt ===
  fodt  width  120.450 pt ( 25.0% of column, 1.6729 in)
  odt   width  120.450 pt ( 25.0% of column, 1.6729 in)
  docx  width  144.000 pt ( 29.9% of column, 2.0000 in)
  doc   width  144.000 pt ( 29.9% of column, 2.0000 in)
  rtf   width  120.450 pt ( 25.0% of column, 1.6729 in)

=== column 255.118 pt (margins 6cm), default paragraph style 12pt ===
  fodt  width   63.750 pt ( 25.0% of column, 0.8854 in)
  odt   width   63.750 pt ( 25.0% of column, 0.8854 in)
  docx  width  144.000 pt ( 56.4% of column, 2.0000 in)
  doc   width  144.000 pt ( 56.4% of column, 2.0000 in)
  rtf   width   63.750 pt ( 25.0% of column, 0.8854 in)
```

**Halving the column is the measurement that separates the two rules**, and it separates them
cleanly: DOCX and DOC hold at exactly 2.0000 in through a 47 % change of column, ODF and RTF track
the column at exactly 25.0 %.

### The negative case, which was the decision this round could most easily have got wrong

**RTF keeps Writer's rule.** The brief's "unconditional by both Word filters" is exactly right, and
RTF is not one of them: `sw/source/writerfilter/filter/WriterFilter.cxx:338` sets
`ContinuousEndnotes` under a comment reading *"options that are valid for the DOCX format"*,
`sw/source/filter/ww8/ww8par.cxx:2050` sets `CONTINUOUS_ENDNOTES` for DOC, and a grep of the whole
tree finds no third setter — `RtfFilter::setTargetDocument` sets only
`UndocumentedWriterfilterHack`. **XRay.Office routes DOCX, DOC and RTF through the same
`PaginationOptions.Word` preset**, so folding this into that preset — which is the obvious place —
would have given RTF a separator LibreOffice does not draw for it and broken
`NoteSeparatorComparisonTests.TheRtfSeparatorGapIsExactlyTheShorterNoteLines`, which was green and
asserts our RTF rule against LibreOffice's to 0.1 pt. Hence a flag of its own,
`PaginationOptions.UsesWordNoteSeparator`.

### The vertical rule, which needed a second axis to pin

A single font size cannot tell a proportion from a constant either, so the probe also varies the
**default paragraph style's** size at one column width. `rule-to-note` below is the rule's top less
the first note line's baseline:

| default paragraph style | fodt / odt / rtf | docx / doc | difference |
|---|---:|---:|---:|
| 8 pt | 12.700 | 13.050 | **0.350** |
| 12 pt | 12.700 | 14.900 | **2.200** |
| 24 pt | 12.700 | 20.400 | **7.700** |

Writer's is a constant; Word's is not, and it scales with the **default paragraph style** rather
than with the body or the note. That is `sw::FootnoteSeparatorHeight` taking the branch whose helper
is documented as *"the height of the line that hosts the separator line (the top margin of the
container), based on the default paragraph style"* (`ftnfrm.cxx:57-77, 257-272`), with the rule then
placed at 60 % of it (`paintfrm.cxx:5850-5852`).

The 12 pt row's **2.200 pt** is the same figure `fidelity-01` measured on the corpus document, from
a different document and a different route.

**One detail the arithmetic only closes with**: the 60 % is computed in `double` and assigned
through `Point::setY`, which takes a `tools::Long` of twips and therefore *truncates*. Solving the
three rows above for the reservation gives 185, 277 and 552 twips under truncation, and non-integers
under rounding. `Paginator.RaisedAboveNotes` truncates for that reason.

*Predicted and wrong, as `prediction.md` §P2 said it expected to be:* I predicted the reservation
would be ≈13.9 pt for the 12 pt case. It is **13.85 pt (277 twips)**. The prediction also said "I
expect my first computed value to miss by 1-2 pt"; it missed by 0.025.

---

## 3. What shipped for the separator

| file | what |
|---|---|
| `Layout/Paginator.cs` | `PaginationOptions.UsesWordNoteSeparator`, `WordNoteSeparatorLength` (2 in), the two branches in `Separator`, and `RaisedAboveNotes` |
| `Layout/PageContent.cs` | `LaidOutPage.NoteSeparator`'s remarks, which stated Writer's numbers as though they were the only ones |
| `Ooxml/DocxLayoutSource.cs` | `DefaultParagraphLineHeight` — the default paragraph style's font resolved through the same chain and the same device grid every run goes through |
| `Ooxml/DocxReader.cs`, `Ww8/DocReader.cs` | set the flag and the reservation; fall back to Writer's reservation when no face can be read |
| `Ww8/Ww8DocumentReader.Layout.cs` | `DefaultStyleFont`, style nought's character chain, resolved by the same call `BlankFurniture` already used |

**Both halves ship together.** `fidelity-01` §9.4 declined the width-only version because
`FootnoteComparisonTests.cs:227` also asserts the rule's `Y`; that is exactly right, and it is why
this is one flag switching two things rather than a constant.

---

## 4. `note-restart.docx` — pagination fixed, one `pdftotext -layout` column left

`note-restart.doc` turned green; `note-restart.docx` did not, and it is worth saying precisely what
is left because "still red" would be misleading.

**The pagination and the content are now exactly right.** Rendering both sides and diffing the
extracted text with runs of spaces folded and line breaks *kept* gives **no difference at all**,
form feeds included — so the page split, the note placement and the renumbering all agree.

The residual is that the test extracts with `pdftotext -layout` and `TextComparer.Normalise` trims
only *trailing* whitespace, so the comparison is sensitive to the column grid poppler infers. Ours:

```
body text            ours 56.7000   theirs 56.8000   (56.700 pt = the declared w:left of 1134 twips,
                                                      plus LibreOffice's documented 0.1 pt pen offset)
first note line      ours 59.6397   theirs 59.7000   (0.0603 apart — 0.0397 beyond that offset)
```

A 0.040 pt difference in where the note's first line starts, which tips `-layout` into one extra
leading space on each of the eight note lines. That 0.040 pt is the ~0.1 % advance divergence the
brief rules out of scope for this round, arriving through a text comparison rather than a position
one. **Not chased.**

---

## 5. `endnotes.docx` — not attempted, and why

`prediction.md` §P3 predicted this one would not turn green, and it did not. It is a *placement*
rule, not a metric: LibreOffice now lays a DOCX's endnotes inline at the end of the body where the
document states no placement, so it makes one page where we make two (`drawn.Count should be 1 but
was 2`). `endnotes.doc` passes already because that file states section-end placement explicitly and
we honour it. Implementing a compatibility **default** for a document that states nothing is a
different piece of work from the two metrics above and belongs with whatever else
`CONTINUOUS_ENDNOTES` implies for endnote flow.

---

## 6. The twelfth — `SheetDrawingComparisonTests`, declined, with its size stated

`fidelity-01` §6.1 identified `ShapeAnchor::calcCellAnchorEmu`'s clamp
(`sc/source/filter/oox/drawingbase.cxx:267-300`), *"Excel seems to limit the offsets to the
bottom/left edge of the cell… reduce cell's right edge by a full twip"*, as the fix for the
reference's 94.904 pt against our 95.074 pt.

**I implemented it, measured it, and reverted it.** The implementation is in this branch's history
at `59aa76fbc66` and is not in the final tree.

What was measured, with the clamp in:

| | width | height |
|---|---:|---:|
| ours before | 95.074 | 46.800 |
| ours with the clamp | **95.0173** | **46.7433** |
| reference 26.2.4.2 | 94.904 | 46.658 |

The clamp fires and moves both axes, and it moves them by exactly the right amount: worked through
in the reference's own unit, `calcCellAnchorEmu` applied to this file's anchor (`from` row 1
offset 45720 EMU, `to` row 1 offset 640080 EMU) over Calc's row heights (403 and 1008 twips, summed
and converted by `GetMMRect`) predicts a height of **1649.24 → 1649 hmm**, and 1649 hmm is
**46.7433 pt — our number with the clamp, to the digit.** The reference draws 1646 hmm.

**So the clamp is fully and faithfully implemented, and the reference's number does not come from
it.** The remaining 3-4 hundredths of a millimetre on each axis arrive from a later stage — Calc
re-anchors an imported drawing on its own drawing layer after the filter has run — which is a
second mechanism and its own investigation.

**Size, stated as the brief asks.** The clamp itself is one rule and about 60 lines: a
`SheetDrawing.ClampsOffsetsToCell` flag (SpreadsheetML only — LibreOffice does not clamp on the BIFF
or ODF paths, and BIFF states its offsets in 1024ths of a cell so it could only ever overrun by a
twip), a shared `SheetCellPoint.OffsetWithin`, and four call sites in `SheetPageGraphics`,
`SheetDrawingArea` and `SheetEmptyPages` that resolve a cell point against the grid. **It turns no
test green**, and it changes the rectangle of every XLSX drawing whose offset overruns its cell
across the corpus. That is the same trade `fidelity-01` §9.4 declined for the width-only separator
fix, and declining it here is the same judgement: a behaviour change with no measurement that can
say it is an improvement, on the one document where it can be checked and is still wrong afterwards.

---

## 7. The slide table cell pitch — a deleted override, and three tests re-baselined

`PptxSlideLayout.cs:704` forced `FontIndependentLineSpacing = false` on a table cell's text body.
The rule the brief says we already implement is `SlideTextLayout`'s 1.2 em box with a 1.0 em ascent,
reached **only when that flag is true** — which is its default and which every non-table PPTX body
gets. So the call site was reached; an override was turning it off. **The fix is deleting the
override.** Named as the brief asked: `PptxSlideLayout.CellBody`.

The override's own comment recorded that it contradicted LibreOffice's C++ and cited a **24.2.7.2**
measurement, which was honest and correct at the time. `a47776a938c` (2025-03-27, tdf#165521,
*"pptx layout: don't use font's leading for cells too"*) settled it the other way, its message
saying *"Microsoft just ignores the font metrics, and simply adds 20 % to the font height."*

Measured on 26.2.4.2's own PDF of `slide-table-grid.pptx`, before changing the tests:

```
slide 1, first cell baseline   446.4 on a 540 pt slide  ->  93.600 top-down
                               = 72 (row) + 3.6 (inset) + 18.000 = exactly 1.0 em of ascent
slide 2, the wrapping cell     302.4 and 280.8          ->  pitch 21.600 = 1.2 x 18 exactly
slide 2, bottom rule                                    ->  266.428 top-down
```

Three `SlideTablePlacementTests` were calibrated to the old binary and are re-baselined **to those
figures, read out of the reference's PDF rather than adopted from our own output**: 91.928 → 93.6,
20.154 → 21.6 (and the test renamed from `...IsTheFacesAndNotTheEm` to `...IsTheEmAndNotTheFaces`,
because its name asserted the old rule), 263.537 → 266.428. Tolerances are unchanged.

### The one regression, caught and restated rather than tolerated

`OdpTableComparisonTests.TheSameTableThroughEitherFormatDrawsTheSameStrokes` turned red. It compares
**our ODP output against our PPTX output** and never touches LibreOffice, so it cannot say which
side is right.

Measured before touching it — LibreOffice 26.2.4.2 rendering the two files itself:

| | first cell baseline | slide-2 bottom rule |
|---|---:|---:|
| `slide-table-grid.pptx` | 93.600 | **266.428** |
| `odp-table-grid.odp` | 91.928 | **263.537** |

**The reference itself now draws the same table 2.891 pt differently through the two formats**,
because tdf#165521 moved PowerPoint cells and left ODF cells on the face's metrics. Both of our
readers reproduce their own side against the reference — `EveryGridLineIsTheStrokeLibreOfficeDraws`
for the ODP and `SlideTableComparisonTests` for the PPTX both pass.

So the test's *claim* survives and the assumption that every number in it is format-independent has
expired — `fidelity-01` §5.3's category, and its recommendation that restating beats deleting. The
divergence is now asserted as a **stated number over a counted set**: 8 of the 42 stroke
coordinates, all on the slide holding the row that states `h="0"`, all below the last stated row
boundary, each expected to be exactly `GrownRowDrop = 2.892` pt higher in the ODF rendering. Our own
two renderings differ by **2.900** on all six affected strokes and by **0.000** on the other
fifteen — so the exception is bounded, counted, and would fail if it widened, narrowed, or reached
the first slide.

---

## 8. Tests: reintroduction-verified versus drift guards

All by `.claude/skills/corpus-batches/scripts/verify-test.sh`, on a clean tree, rebuilt on both legs.

| test | mutation put back | outcome |
|---|---|---|
| `NoteSeparatorRuleTests.AWordDocumentsRuleIsTwoInchesWhateverItsColumn` (×2) | `UsesWordNoteSeparator = false` in `DocxReader` and `DocReader` | **DETECTED** |
| `NoteSeparatorRuleTests.TheWordRuleSitsTwoPointTwoHigherAboveItsNotesThanWritersDoes` | same | **DETECTED** |
| `NoteSeparatorRuleTests.EveryOtherDocumentsRuleIsAQuarterOfItsColumn` (×3) | `UsesWordNoteSeparator = true` added to **`RtfReader`** | **DETECTED** — the negative case, and the one this round could most easily have got wrong |
| `SlideTablePlacementTests.ACellsLineHeightIsTheEmAndNotTheFaces` | `FontIndependentLineSpacing = false` restored in `PptxSlideLayout.CellBody` | **DETECTED** |
| `SlideTablePlacementTests.ACellsTextStartsAtItsOwnMarginAndNotAtTheBodysInsets` | same | **DETECTED** |
| `SlideTablePlacementTests.ARowWithNoStatedHeightGrowsToItsText` | same | **DETECTED** |

Six new tests in `NoteSeparatorRuleTests`; all six are detectors, in both directions. No new test in
this round is a drift guard.

`OdpTableComparisonTests.TheSameTableThroughEitherFormatDrawsTheSameStrokes` is not listed because it
is not new. It is a **cross-reader consistency guard** rather than a detector of either change: it
compares two of our own outputs, so no single-sided mutation is the thing it exists to catch. Its
`inside.ShouldBe(8)` is what keeps it from degenerating into one.

---

## 9. Corpus reach and verdict movement

**None of the three changes touches `Core`, `Containers`, `Text`, `Vector`, `Rendering` or
`Markup`**, so the 534-rendering sweep the brief mandates for those is not owed. It was run anyway,
because a Fidelity count cannot see a cascade and "no other project moved" is not the same claim.

**Method.** Every corpus document rendered by `xray-office render` with `SOURCE_DATE_EPOCH` set — so
`/CreationDate` is normalised out by construction rather than masked afterwards — and the rendered
PDF bytes hashed. Run twice over `words` and `slides`: once at `dbdae4be370` and once with
`dotnet/src/XRay.Office.WordProcessing` and `dotnet/src/XRay.Office.Presentations` checked out at the
base `63d5290aacf` and the CLI rebuilt. 363 documents each side, **0 render failures on either**.
The tree was restored and rebuilt afterwards and `git status -- dotnet` is clean.

### Reach

| track | documents | renderings changed |
|---|---:|---:|
| slides | 163 | **48 (29.4 %)** |
| words | 200 | **17 (8.5 %)** |
| sheets | 171 | **0**, by construction |
| **total** | **534** | **65** |

That is the honest size of these two changes: a table-cell line pitch reaches nearly a third of the
deck corpus, and a note separator reaches every Word document that has notes.

### Verdict movement

The 65 that moved were then re-rendered on both builds and scored against
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/ref-baseline-all.tsv` on the gate's own two checks —
page count exact, word count within 2 %:

| | count |
|---|---:|
| renderings changed | 65 |
| of which **page count** changed | **0** |
| of which word count changed | 6 |
| **gate verdicts changed** | **0** — none gained, none lost |

**Nothing cascaded.** The change that most invited one is the separator reservation, which nearly
triples (5.669 pt → ≈13.85 pt for an 11 pt default style) and therefore takes that much more room
out of the body on every page carrying a footnote — and it moved no page count on the corpus at all.
The six word-count moves are 2 to 33 words on documents of 2,000 to 58,000, all far inside the 2 %
band on both sides:

```
slides/batch-008/pptx/8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx                    2331 -> 2308
slides/batch-011/pptx/section_1_our_rights_presentation.pptx                      4257 -> 4245
slides/batch-012/pptx/NAS-Infrastructure-Roadmaps-v16.0.pptx                     19370 -> 19337
slides/batch-014/pptx/Intersil_Italy_CAN_Bus_Transceiver_Presentation_Final.pptx   3827 -> 3825
slides/batch-016/pptx/Reporting_responsibilities_matrix.pptx                      57904 -> 57918
words/batch-015/doc/644730BRI0mna000BOX361539B00public0.doc                        2336 -> 2338
```

A taller table cell fits fewer words in a fixed cell, which is where five of the six come from.

*What this does not say:* verdicts held, but "held" includes documents that were failing the gate
before and after. This is a **no-regression** measurement, not an improvement one — the brief's
"if a fix moves a corpus verdict, that is a bonus" did not pay out, and the honest reading is that
these two defects were never what the corpus scoreboard was losing on.

**`XRay.Office.Spreadsheets` is untouched in the final tree** — `git diff` against the base over
`dotnet/src/XRay.Office.Spreadsheets/` is empty — so the sheets track cannot have moved, and it is
reported above as a control rather than as a result.

---

## 10. The other ten projects

Run individually and totalled by hand, as `dotnet/CLAUDE.md` requires:

```
Core 298   Containers 109   Text 289   Vector 295   Rendering 121 (1 skipped)
Markup 259   OpenDocument 125   WordProcessing 781   Spreadsheets 643   Presentations 609
                                                     TOTAL 3529, FAILED 0
```

Every figure matches the brief's exactly **except `WordProcessing`, which is 781 against 775** —
the six new `NoteSeparatorRuleTests` cases. So the mandated total moves from **3523 to 3529**, by
addition and not by drift; nothing changed its verdict. `Rendering`'s single skip is the
CFF-flavoured-font gap `fidelity-01` §1 recorded and is unchanged.

Build: `dotnet build XRay.Office.slnx` → **0 warnings, 0 errors**.

---

## 11. The prediction, scored against what was measured

| | prediction | outcome |
|---|---|---|
| **P0** | baseline reproduces at 510/40/550, 0 skipped | **Right**, and the failing set matches `fidelity-01`'s |
| **P1** | the flag is DOCX + DOC and **not** RTF; ODF negative | **Right, and load-bearing.** Measured on five spellings of one authored document at two column widths. Had this gone the other way the fix would have broken a green test |
| **P2** | reservation ≈ 13.9 pt at 12 pt; expected to miss by 1-2 pt | **Right at 13.85 pt (277 twips)**, and the stated expectation of missing was itself wrong — it missed by 0.025. The *truncation* in the 60 % was not predicted and is what makes the three sizes come out exactly |
| **P3** | 8 of the 9 separator cases; not the endnote one | **Wrong, 7 of 9.** The endnote case did not turn, as predicted; `note-restart.docx` also did not, for a reason §4 states and which I had not anticipated — a `pdftotext -layout` column, not a layout error |
| **P4** | 2 green from the slide table, one line | **Right**, both, and it was one property initialiser — but it cost **three unit-test re-baselines and one restated Fidelity test** that the prediction did not mention. "One line" was true of the source and false of the round |
| **P5** | the sheet clamp is small, 1 green | **Wrong.** The clamp is real, cited and faithfully implemented — it reproduces `calcCellAnchorEmu` to the digit — and the reference's number does not come from it. 0 green, reverted, §6 |
| **P6** | Fidelity 519/550, 31 failed; range 517-521 | **Right on the number and wrong on its parts**: 519/31, from 7 + 2 + 0 rather than 8 + 2 + 1 |
| **P7.1** | a count cannot see a regression; diff both sets | **Vindicated.** One test did turn red (§7) and the count alone would have read the round as "8 green" |

**The prediction that mattered was P1**, and it mattered because it was the one that named a way to
be wrong rather than a number to hit. **The prediction that was most wrong was P5**, and its error
is the one this project keeps making: `fidelity-01` located the mechanism correctly, quoted the
right source comment, and the sentence "adopting the clamp gives parity" did not survive doing it.
The clamp does exactly what its comment says; it just is not what the 0.170 pt was.

---

## 12. Honest limits

- **`note-restart.docx` is called fixed on the evidence of a text diff, not of the test.** The test
  is still red. The claim in §4 is that the pagination and the drawn characters agree exactly and
  the residue is a poppler column; that is measured, but the assertion that would prove it is the
  one still failing.
- **The reservation rule is fitted on Latin faces at four sizes.** 8, 11, 12 and 24 pt, in
  Liberation Serif and Carlito. A default paragraph style in a face whose metrics resolve
  differently — or a document using printer metrics, where `MetricGrid` is in play — was not
  measured, and `DefaultParagraphLineHeight` passes the grid through on the assumption that it
  should.
- **The 24.2.7.2 A/B is still impossible** — the LibreOffice download hosts remain firewalled. Every
  "the reference moved" statement here rests on 26.2.4.2 measurements plus upstream commits dated
  after the 24.2 branch, exactly as `fidelity-01` §11 records.
- **§6's second mechanism is named but not established.** "Calc re-anchors an imported drawing on
  its own drawing layer" is where the remaining 3 hmm must come from by elimination; I did not
  reproduce it. What *is* established is that `calcCellAnchorEmu` alone predicts 1649 hmm and the
  reference draws 1646.
