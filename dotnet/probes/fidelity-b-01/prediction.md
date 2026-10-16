# fidelity-b-01 — prediction, committed before the first measurement

Round: Fidelity-B, an *implementation* round. Worktree `/c/sandbox/workdir/wt-fid-b`, branch
`wt-fid-b`, base `63d5290aacf`. Reference binary **26.2.4.2** (`check-env.sh` clean, quoted in
`results.md`).

The brief hands me twelve genuine X-Ray.Office defects out of the 40 Fidelity failures, already
classified by `dotnet/probes/fidelity-01/results.md`. This file is what I expect **before** I run
anything. It is written from reading — the X-Ray.Office source, and the C++ tree as *explanation
only* — and every number in it is a prediction, not a measurement.

---

## P0 — the baseline reproduces

Fidelity is **510 passed / 40 failed / 550 total, 0 skipped**, and the failing set is the 40 named
in `fidelity-01/fails-head.txt`. The ten non-Fidelity projects are **3523 total, 0 failed**
(Rendering 121 with 1 skipped).

*Risk:* if this does not reproduce I stop and say so, exactly as the standing rule requires.

## P1 — the separator condition is DOCX and DOC, and **not** RTF

The brief asks me to establish rather than assume that `CONTINUOUS_ENDNOTES` "is set unconditionally
by both Word filters" and to check the negative case. I predict from source, ahead of the probe:

- **DOCX** — `sw/source/writerfilter/filter/WriterFilter.cxx:338` sets `ContinuousEndnotes` true in
  `setTargetDocument`, unconditionally, under a comment reading *"compatibility options that are
  valid for the DOCX format"*. **Yes.**
- **DOC** — `sw/source/filter/ww8/ww8par.cxx:2050` sets `CONTINUOUS_ENDNOTES` true unconditionally.
  **Yes.**
- **RTF** — `sw/source/writerfilter/filter/RtfFilter.cxx` is a *different* filter class and sets
  only `UndocumentedWriterfilterHack`. A grep of the whole tree for `ContinuousEndnotes` finds
  exactly one filter-side setter (WriterFilter) plus the ww8 one. **No.**
- **ODF** — neither ODF filter sets it. **No.**

**This is the prediction most likely to be wrong in the direction that costs, and it is the reason
the negative case matters.** X-Ray.Office routes DOCX, DOC *and RTF* through `PaginationOptions.Word`.
If I apply the new rule to the `Word` preset wholesale I break RTF, where LibreOffice keeps Writer's
25 % — and `NoteSeparatorComparisonTests.TheRtfSeparatorGapIsExactlyTheShorterNoteLines` is
**currently green** and asserts our RTF separator top against LibreOffice's to 0.1 pt. So the
condition is a *new* flag, not the existing preset.

I will prove all four by authored probe against the installed 26.2.4.2, in both Word formats and
for the ODF negative case, before writing the conditional.

## P2 — the three rules the flag switches, and the numbers I expect

From `paintfrm.cxx:5845-5868` and `ftnfrm.cxx:57-77,257-272`, as *explanation* of the measurements
`fidelity-01` already took (144.000 pt, invariant under halving the text width; 2.214 pt vertical):

| | Writer (today, ODF) | Word (predicted, DOCX/DOC) |
|---|---|---|
| container top border ("reservation") | `TopDist+BottomDist+LineWidth` = 57+57+10 twips = **6.20 pt** | the **default paragraph style's font line height** |
| separator Y inside the container | container top + `TopDist` = **+2.85 pt** | container top + **0.6 ×** the reservation |
| separator width | **25 %** of the print width | **2 in = 144.000 pt**, clamped to the print width |

X-Ray.Office's present numbers are `NoteSeparatorHeight` 5.669 pt, `NoteSeparatorSpacing` 2.835 pt,
`NoteSeparatorWidth` 0.25 — i.e. Writer's, format-blind, at `Paginator.cs:160-183`.

**Predicted arithmetic for the vertical, and I expect to be about 10 % out.** Writer puts the rule
at `notesTop − 3.35 pt`; Word puts it at `notesTop − 0.4 × reservation`. For the 2.2 pt measured
gap the reservation must be **13.9 pt**, which is a plausible line height for 11 pt Carlito
(ascent+descent ≈ 1.10 em ≈ 12.2 pt) only if the default style is larger than the body's — so I
expect to *measure* the reservation rather than compute it, and I expect my first computed value to
miss by 1–2 pt. Recorded here so that a later agreement is not read as a prediction that held.

## P3 — how many of the nine turn green

The nine are one root cause but **four distinct behaviours**, and they are not equally cheap:

| behaviour | tests | my expectation |
|---|---:|---|
| separator width (2 in) + separator Y (60 %) | `FootnoteComparisonTests.TheRuleAboveTheNotesGoesWhereLibreOfficeDrawsIt` ×2, `PdfOutputComparisonTests.EveryShadeAndRuleIsFilledWhereLibreOfficeFillsIt` ×2 | **4 green** — this is the pair `fidelity-01` §9.4 says must ship *together*, because a width-only fix leaves Y 2.214 pt out |
| the taller reservation evicting a body line | `FootnoteComparisonTests.EveryNoteSitsAtTheFootOfItsOwnPage` ×2, `NoteRestartComparisonTests` ×2 | **4 green, less confidently** — these are pagination, so the reservation has to be right to better than a line, not merely larger |
| endnotes laid out inline at the end of the body | `EndnoteComparisonTests` ×1 (`endnotes.docx`) | **probably not green.** This is a placement rule, not a metric, and `fidelity-01` §5.1 records that `endnotes.doc` already passes because that file states section-end placement explicitly. Implementing a compat *default* for a document that states nothing is a different piece of work from the two metrics above |

**Predicted: 8 of the 9.** If the endnote-placement case turns out to be a one-line default I will
take it, but I am not predicting it.

## P4 — the slide table cell pitch: 2 green, and it is one line

`PptxSlideLayout.cs:704` sets `FontIndependentLineSpacing = false` on a table cell's text body,
under a comment that says outright *"it is the opposite of what the current C++ says"* and cites a
**24.2.7.2** measurement. The rule the brief says we already implement is `SlideTextLayout.cs:704`'s
`1.2 em` box with a `1.0 em` ascent, reached only when that flag is **true** — which it is by
default and which every non-table PPTX body gets.

So the wiring gap is not a missing rule and not a missing call: it is **an override that was correct
against the old binary and is now wrong**, one property initialiser wide. LibreOffice
`a47776a938c` (2025-03-27, tdf#165521, *"pptx layout: don't use font's leading for cells too"*) is
what moved. Deleting the override should turn both `SlideTableComparisonTests` cases green.

**Predicted: 2 green.** The ODP table path is separate (`OdpSlideLayout.cs:331`) and must not move.

## P5 — the twelfth: `SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt`

`fidelity-01` §6.1: reference 94.904 pt wide, ours 95.074 pt; height 46.658 against 46.800.
`sc/source/filter/oox/drawingbase.cxx:267-300` clamps each anchor point to the *next* cell's origin
less one twip — *"Excel seems to limit the offsets to the bottom/left edge of the cell… reduce
cell's right edge by a full twip"*.

**Predicted: small, and 1 green.** The clamp itself is `min(start+offset, nextStart − 1 twip)`. The
cost is that X-Ray.Office resolves a `SheetCellPoint` against the grid at *layout* time in more than
one place (`SheetPageGraphics`, `SheetDrawingArea`, `SheetEmptyPages`), where LibreOffice clamps
once at import — so this is "one rule, three or four call sites", and I predict I will factor a
single resolver rather than write the `min` four times.

*Stated risk:* this touches `XRay.Office.Spreadsheets`, whose 643 tests must not move, and it could
move the sheets corpus. I will measure both rather than argue.

## P6 — the totals

| | predicted |
|---|---|
| Fidelity after | **519 / 550**, 31 failed (8 + 2 + 1 = 11 turned) |
| range I would accept without alarm | 517–521 |
| the ten other projects | **3523 total, 0 failed**, unmoved |

## P7 — what this census cannot see, stated in advance

1. **Whether a currently-green Fidelity test regresses.** My count only ever names the *failing*
   set, so a fix that turns 11 green and 3 red reads as "8 green" unless I diff the failing sets
   both ways. I will name both sets in full, not summarise them.
2. **The corpus.** Tasks 1 and 2 change rendering for every DOCX/DOC with notes and every PPTX with
   a table — that is a large fraction of 534 documents, and no Fidelity count can see it. Task 3
   likewise for XLSX drawings. **None of the three touches `Core`, `Containers`, `Text`, `Vector`,
   `Rendering` or `Markup`**, so the 534-rendering sweep the brief mandates for those is not owed —
   but I intend to sweep anyway, because "how many renderings moved" is the only honest answer to
   "did this cascade", and a verdict *improvement* would be a bonus I should not assume.
3. **Whether the reservation rule is right for a document whose default paragraph style differs
   from its body.** All the fixtures are Carlito at 11 pt. A rule fitted on one font size is a rule
   fitted on one point, and I will say so if I cannot vary it.
4. **Which of my tests are reintroduction-verified.** `verify-test.sh` proves a test fails when the
   fix is removed; a test that cannot be made to fail that way is a drift guard and will be labelled
   one.

---

*Nothing in this file has been measured. Written and committed first, so that agreement counts and
disagreement is visible.*
