# words-r61 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
corpus `/c/sandbox/workdir/sample-files`; worktree `wt-words-r50` on branch `wt-words-r61`, base
`3f079cea621`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`. One prediction file, `prediction.md` at
`c79aa554809`, committed before the first behavioural commit `2011ae6aacf` and covering both changes.

## Baseline, reproduced exactly

`batch-check.sh … 'words/*' … 8` → `TOTAL 355 MATCH 336 MISMATCH 19`, scored against
`MANIFEST.tsv`'s own 337-path list rather than that total — the extra 18 rows are the
case-insensitive mount's alias entries. **319 match, 18 open, zero disagreements with the manifest's
status column, document for document.** The scorer refuses to print unless every manifest path found
a row.

## Result

**319 → 321 of 337. `097_Business_Case_Template_Elegant_Layout` and
`EHEST-SMS-Safety-Management-Manual-V2` both close. Zero regressions.**

| | base | after change 1 | after change 2 |
|---|---:|---:|---:|
| words verdicts | 319 | **320** | **321** |
| our renderings whose bytes changed (cumulative) | — | 80 | **95** |
| page counts changed | — | 1 | **2** |
| extractable words changed | — | 1 document | **1 document** |
| font lists changed | — | 0 | **0** |
| reference halves differing between sweeps | — | 0 | **0** |

The last row is a control worth keeping: over three full sweeps of 355 paths the reference's own
page counts and word counts are **identical every time**. The non-determinism round 60 found on
`ans_mappings_of_eccairs_terms.xlsx` has no counterpart on this track.

Per document, before → after, and there is nothing to net:

| document | pages before | pages after | reference |
|---|---:|---:|---:|
| `097_Business_Case_Template_Elegant_Layout_3ba9cbf2.docx` | 1 | **2** | 2 |
| `EHEST-SMS-Safety-Management-Manual-V2.docx` | 80 | **82** | 82 |

`EHEST`'s extractable words go 19 236 → 19 270 against the reference's 19 222 — 0.25 %, inside the
gate's 2 % band, and it is the only word count in the corpus that moved. `097` reads 54 words on
both sides and its page 2 is empty on both sides.

## 1. The empty-paragraph item is refuted, and the refutation is what fixed it

Round 59 left this as *"the line-height deficit is 11.50 against 12.65 per empty paragraph, worth
1.15 pt on every empty paragraph in the corpus"*, direction confirmed by a second instrument at
−2.06 pt and the monotonic shape refuted.

**Refutation 1 — it is not a per-empty-paragraph line height.** On `097` the whole 3.36 pt by which
the reference's last table rule sits below ours is spent at **four body paragraphs that sit above a
table**, and nowhere else:

| boundary | reference | ours (base) | ours − ref |
|---|---:|---:|---:|
| body p #0 — an inline image | 35.80 | 36.45 | **+0.65** |
| body p #1 — empty | 22.10 | 21.15 | −0.95 |
| body p #2 — a `<w:br/>`, **two lines** | 35.80 | 34.80 | −1.00 |
| body p #3 — empty | 22.20 | 21.15 | −1.05 |
| body p #4 — a `<w:br/>`, **two lines** | 35.80 | 34.80 | −1.00 |

Sum −3.35, and the document's twenty-five table rules confirm it: every row height agrees, and the
running residual steps only at those five gaps. **The two-line paragraphs cost exactly what the
empty ones cost.** A per-line deficit would be twice as large on them. It is a per-boundary
constant, and the boundary needs a table on one side.

`emptypara.py` — 44 authored packages, eleven families, `k` paragraphs between two single-row
tables — reproduces the corpus figure to the digit (`k=1`: reference 22.10, ours 21.15) and
separates the three quantities a corpus measurement cannot:

* the **marginal line** and the **per-paragraph cost** are exact: the slope in `k` is 21.65 pt on
  both sides for every family, at every `k ≥ 1`;
* the **`par-…` families** — a paragraph in front instead of a table — agree in absolute position at
  every `k`, to 0.01 pt;
* only the **first transition into a table** was wrong, and only under proportional line spacing.
  Under `w:line="240"` (100 %) the same families agreed exactly before this round.

## 2. The law: `CalcUpperSpace` adds `nPrevLineSpacing` before it looks at what follows

`SwFlowFrame::CalcUpperSpace` adds `nPrevLineSpacing` to `nUpper` in **all four** of its branches;
`pOwn->IsTextFrame()` guards only the frame's *own* leading
(`sw/source/core/layout/flowfrm.cxx`:1655-1739). So a `SwTabFrame` is handed the paragraph above's
proportional line spacing exactly as a text frame is. We handed it only paragraph to paragraph.

`tableleading.py`, twelve authored packages, measured on the following table's own top rule and on
the paragraph's own baseline so that "the paragraph grew" and "the gap grew" are never confused:

| arm | reference, against the 100 % case | the law |
|---|---:|---|
| proportion 107.9 / 120 / 150 / 200 % | +1.00, +2.50, +6.30, **+12.65** | `floor(H·p/100) − H` in twips, `H` = 253 = the natural line, `p` an integer per cent |
| the paragraph's own size 11 pt → 22 pt at 150 % | +6.30 → **+12.65** | scales with `H` |
| a two-line paragraph, the big line **last** vs **first** | 12.65 vs 6.30 | `H` is `GetHeightOfLastLine()` |
| `atLeast 400`, `exact 400` | **0** handed down | not `SvxInterLineSpaceRule::Prop` |
| control: a 100 % paragraph between the 150 % one and the table | **0.00 divergence** | the leading stops at the paragraph |

And the half we already had is confirmed on the same page: at 150 % the reference's *own* first
baseline is 81.90 and ours 81.89 — `if( !IsParaLine() )` at `itrform2.cxx`:2425.

`tbl-text-097` localises the extra: the middle paragraph's baseline is 104.55 on the reference and
104.54 here, and the table below it starts 1.01 pt lower on the reference. **The extra is below the
paragraph, not above it**, which is what makes it the *table's* upper space and not the paragraph's
height.

### Reach, and the miss

`tableleading-census.py` resolves `w:spacing/@w:line` and `@w:lineRule` through the paragraph's own
`w:pPr`, then its `w:pStyle` chain following `w:basedOn`, then `w:docDefaults/w:pPrDefault`:

```
paragraph-then-table boundaries        :  1478 in 147 documents
  ... of them proportional over 100%   :   275 in  85 documents
sites in documents the gate calls open :    67 in   4 documents
```

Predicted 85–150 changed renderings; **measured 80**. An honest small miss in the safe direction,
and the cause is in the law the census did not apply: the per cent is an integer, so a level stating
`w:line="241"` resolves to 100 % and adds nothing. The census counts sites; it does not resolve the
extra to a number.

### `097`, after

The four text-paragraph boundaries go to **0.00–0.05 pt** and the residual over the document's
remaining twenty-three rules is **flat at −1.64 to −1.69**, i.e. a single displacement and no drift.
That was the round's own stated falsification test — *"if `097` closes but the four boundary deltas
do not go to zero, the 1.00 pt was fitted rather than derived"* — and it passes.

What is left is the one boundary that got worse: the body paragraph holding an inline image, where
we were already 0.65 pt too tall and now add 1.00 more. It is a **pre-existing** error of 1.65 pt in
that paragraph's own height, unmasked rather than caused; `i#47162` says
`MaxAscentDescent(…, bNoFlyCnt=true)` suppresses fly portions from the height the leading is taken
against, and `ParagraphFormat.Apply`'s `baseHeight` already excludes them, so the leading itself is
right and the *line* is not. Open, named, unfixed.

## 3. A second defect the same probe found: an `atLeast` line loses its raise

`tableleading.py` arm 4 was written as a control and came back a finding.
`SwTextFormatter::CalcRealHeight` runs two switches and only the second is guarded by
`if( !IsParaLine() )`. `SvxLineSpaceRule::Min` — OOXML's `atLeast` — is in the **first**
(`itrform2.cxx`:2397 against :2425), so it raises **every** line including a paragraph's first; and
`SwTextFrame::GetLineSpace` answers only for `Prop` and `Fix` (`txtfrm.cxx`:3996), so it hands
**none** of that raise on. We stored the raise where proportional leading is stored, so
`ParagraphLeading.AsDrawn` stripped it from a paragraph's first line and a frame's first line, and
`ParagraphLeading.Below` handed it down.

`w:line="400" w:lineRule="atLeast"` on 11 pt Cambria, offsets from the body's top edge:

| | reference | ours before | ours after |
|---|---:|---:|---:|
| the paragraph's baseline | **17.25** | 9.89 | **17.24** |
| the following table's top | 20.00 | 20.00 | 20.00 |
| `exact 400`, the control | 16.00 | 15.99 | 15.99 |

**The table position could not see it, and `verify-test.sh` is what said so.** Stripping the raise
shortens the paragraph by 7.35 pt and handing the same raise on as leading lengthens the gap below
it by 7.35 — they cancel exactly, and the mutation came back `NOT DETECTED` against three
table-position assertions that all passed over a paragraph drawn 7.35 pt too high. A baseline
assertion was added and the same mutation is now detected by that assertion alone. That is the
cheapest instance of "the observable you chose cannot see the defect" this project has recorded, and
it cost one `verify-test.sh` run to find.

`atleast-census.py`: **1 569 paragraphs in 29 documents** resolve to `atLeast`; **675 in 16
documents** state more than 253 twips, the natural line of 11 pt Cambria — a crude threshold and an
over-count for any smaller face. Measured reach: **20 renderings**, consistent with 16 documents and
the alias pair. `EHEST-SMS-Safety-Management-Manual-V2.docx` is 295 of the 675 and is the document
that closed.

## 4. `012` and `015` are **not** in the same class as `097`, and this is a break with the brief

The brief put all three in one class on the strength of "all three of the reference's second pages
are empty". They are empty of *words*. They are not empty of ink, and what is on them is not a
trailing paragraph:

| | reference page 2 holds |
|---|---|
| `097` | nothing at all — 60 bytes of content stream, a clip artefact |
| `012` | **one filled rectangle and one grey rule** — `12.4 489.65 99.95 50.35 re f*` at the top of the body |
| `015` | **five white rules** forming one table row, 28.9 pt tall |

Both `012` and `015` carry a **positioned** table: `w:tblpPr horzAnchor="margin" tblpXSpec="center"
tblpY="1122"`, 15 345 twips wide on a 648 pt text area, so it starts 59.6 pt left of the margin —
which is exactly where the reference's page-2 rectangle starts, at x = 12.4. `012`'s table is nine
rows of 50.4 pt beginning 56.1 pt below the top margin; eight fit above the bottom margin and the
ninth does not. **The reference breaks a fly-held table across the page; `PlaceFloatedTable` cannot,
and says so in its own remarks.** The ninth row is empty, which is why the word counts match and
only the page count differs.

Predicted **unchanged**, and both are unchanged. That was the round's one deliberate disagreement
with its brief and it held.

**A second, larger divergence on `012` that no gate column can see**, found by the vision reading
and confirmed by the content stream: the reference draws **75 fill operations on page 1 and we draw
19**. The reviewer named it independently — *"the reference paints alternating light-gray row bands
behind the task rows; we paint no row shading at all"*.

## 5. The vision reading

Two blind readings, each handed one composed image and nothing else, each forbidden from reading any
other file or running any command, each asked to describe the halves separately before comparing and
to give the direction.

### `097_Business_Case_Template` page 1, **after** the change — chosen because it is the round's item

Not `--worst`: the page the brief names.

Round 59's reader on this page, before the fix, ranked first *"the reference creeps down the page
relative to ours, and the gap grows monotonically from top to bottom"*. **This round's reader, on the
fixed page, reports the two as "extremely close" and says the offset is now constant**: *"this offset
is roughly constant, not cumulative — the gap is ~4–6 px at the first table and still ~4 px at the
bottom of the last table — so it is a single early displacement, not drifting row spacing."*

**Second instrument, confirmed exactly.** The per-rule residual over `097`'s twenty-five table rules
is `+0.01, +0.06,` then **−1.64 … −1.69 flat for all twenty-three remaining rules**. At 150 dpi that
is 3.4–3.5 px, inside the reviewer's 4–6 px, and it is flat where round 59's reader saw a ramp. The
reader also localises it correctly by implication: the displacement is early, and the one boundary
that did not go to zero is body paragraph #0, the second block on the page.

The reader further reports our bold serif headings running 4–8 % wider than the reference's, with the
banner boxes the same width — that is the advance divergence `CLAUDE.md` documents at length, it is
pre-existing, and it is the fourth independent reader to report it.

Two things the reader could **not** settle and said so: whether the constant offset is real or
composition jitter (it is real; measured above), and whether the heading width is substitution,
size or tracking (it is base advances, settled in `probes/advance-divergence/`).

### `012_Project_Timeline_Template` page 1 — chosen because it is a target the round predicted would **not** move

Deliberately: a reading of a page a change is predicted not to touch is the cheapest control there
is, and this one paid for itself twice.

The reader's first-ranked finding: **"The reference is missing the entire title block text.** Ours
draws 'Project Timeline Template' as a huge two-line heading; the reference draws nothing there —
just white space." Direction: content visible on ours, absent on the reference — the *opposite* of
the direction every previous reading on this project has reported.

**Second instrument, confirmed, and it is not "missing".** Both text layers hold the title:
`pdftotext -bbox` finds `Project` at `yMin=9.03` on the reference too. Rendered at 100 dpi, the top
125 rows hold **433 dark pixels on the reference and 13 669 on ours**. The page's own content stream
says why: over page 1 the reference issues **23 white text shows against 12 black**, and we issue
**25 black against 14 white**. The title run states **no `w:color` at all** and sits in a `wps` text
box with `<a:noFill/>` — so this is `COL_AUTO` resolved by the **drawing layer**, and 26.2.4.2
resolves it to **white** where we resolve it to black.

That is a witness for exactly the arm round 59 measured, refuted and removed — *"a floating frame's
fill is not the background"* — pointing the other way. Round 59's removal was right on its own
evidence (383 glyphs wrongly white) and this document says the rule it was reaching for exists and
is not the one it tried. It is **not** implemented here: one witness is a lead, and the two
directions have to be separated by a probe before either is shipped.

The reader's third finding — the missing row shading — is confirmed by the fill count above, 75
against 19. Its item 5, in-bar captions sitting left of centre on the reference, and item 4, bar
outlines and rounded corners we do not draw, are **not yet checked by a second instrument**.

## 6. The 24.2.7.2 audit — two sites, both VERIFIED, and a harness trap worth more than either

`OdtLayoutSource.AddsParagraphSpacing` and `OdtLayoutSource.KeepsParagraphSpacingAtPages`, each
carrying a measurement taken on 24.2.7.2. Round 59 verified the **DOCX** twin of the first; this is
the ODF path, which reaches the flag through `SwXMLImport::SetConfigurationSettings` rather than
through `DomainMapper_Impl::ApplySettingsTable`, so the two are separate claims.

`audit_odfspacing.py`, six arms, reference and ours, first baseline and boundary pitch:

| arm | reference | ours |
|---|---|---|
| as shipped: `AddParaTableSpacing=false`, `AtStart=true` | 93.60 / **24.00** | 93.59 / 24.00 |
| `AddParaTableSpacing=true` | 93.60 / **32.00** | 93.59 / 32.00 |
| `AddParaTableSpacing` **removed** | 93.60 / **32.00** | 93.59 / 32.00 |
| `AtStart=false` | **81.60** / 24.00 | 81.59 / 24.00 |
| `AtStart` **removed** | **93.60** / 24.00 | 93.59 / 24.00 |
| both off — the independence control | **81.60 / 24.00** | 81.59 / 24.00 |

**Six of six, VERIFIED.** The two `removed` arms are the discriminators: a stated value implies
nothing about an unstated one, and unstated is what every real document carries.

**The probe's first cut authored a minimal flat ODF and it was ignored by 26.2.4.2.** Correct
namespaces, `ooo:configuration-settings`, the item spelled exactly as the fixture spells it — *our*
reader honoured it (24.00 with the flag false) and the reference answered **32.00 in all six arms,
including the two stating `false`**. Stopping there would have reported both sites WRONG. The arms
are now one string substitution each into round 53's own `paragraph-spacing-collapsed.fodt`, a file
the reference demonstrably reads. What the authored file lacked was not chased, and the point of
that rule is that it does not have to be. Recorded at the site.

Counters re-derived at the base and at this tree with the file's own commands — never quoted:

| | base `3f079cea621` | this tree |
|---|---:|---:|
| open sites | 37 | **37** |
| marker lines | 26 | **28** |
| VERIFIED / FIXED / WRONG | 21 / 4 / 1 | **23 / 4 / 1** |

The open count does not fall on a VERIFIED, per the file's convention. Round 59's note records "38
open, 24 markers" at its own tree; round 60 marked more after it, and the numbers above are what the
commands give at the two commits named.

## Refutations, collected

1. **The empty-paragraph deficit is not 1.15 pt per empty paragraph.** It is 1.00 pt per
   *paragraph-then-table boundary*, and a two-line paragraph costs exactly what an empty one costs —
   measured on the corpus document and again over 44 authored packages.
2. **`097`'s deficit was never a line height at all.** Our per-line and per-paragraph costs were
   already exact to 0.01 pt at every `k ≥ 1`; only the transition into a table was wrong.
3. **`012` and `015` are not in `097`'s class.** Their reference page 2 carries a positioned table's
   ninth row — a filled cell and a rule on `012`, five white rules on `015` — not a trailing empty
   paragraph. Predicted unchanged and unchanged.
4. **`ParagraphLeading` conflated two different raises.** An `atLeast` line's raise is not the
   paragraph above's to give: `SvxLineSpaceRule::Min` sits outside `CalcRealHeight`'s `IsParaLine`
   guard and `GetLineSpace` does not answer for it.
5. **A table's position cannot see the `atLeast` defect** — stripping the raise and handing it on
   cancel exactly. Three passing assertions over a paragraph drawn 7.35 pt too high, caught by
   `verify-test.sh` reporting `NOT DETECTED`.
6. **An authored flat ODF's `office:settings` was ignored by 26.2.4.2** while our reader honoured
   it. A probe that had trusted its own fixture would have reported two correct sites WRONG.
7. **The reference draws `012`'s title white and we draw it black**, from a `COL_AUTO` run in a
   `noFill` `wps` text box — a witness in the *opposite* direction to the frame-fill arm round 59
   measured and removed. 433 dark pixels against 13 669 in the top 125 rows at 100 dpi.

## Tests

```
Core 376   Containers 109   Text 624   Vector 302   Rendering 153(1 skipped)   Markup 259
OpenDocument 125   WordProcessing 1225   Spreadsheets 1020   Presentations 846     = 5039
0 failed, 1 skipped
```

**5 034 → 5 039, delta +5**, re-derived project by project rather than quoted: the whole delta is
`XRay.Office.WordProcessing` 1 220 → 1 225, the five `TableParagraphLeadingTests`.
`dotnet build -v q -nologo`: **0 warnings, 0 errors.**

Through `verify-test.sh`, tree clean before each and restored after — **three mutations, all three
detected, and the third only after the test was strengthened**:

| mutation | detected by |
|---|---|
| the table arm takes no leading (`? Length.Zero`) | all 3 table assertions |
| the table arm takes a **constant** 6.30 pt leading | **only** `AParagraphAtAHundredPerCentHandsNothingToTheTableBelowIt` — the control does its job and the other two correctly pass |
| `RaisesEveryLine` returns `false`, i.e. the `atLeast` defect back | **`TheAtLeastRaiseSitsAboveTheTextAndNotBelowIt` alone.** `NOT DETECTED` before that assertion existed |

The second mutation is the one worth reading: it is the *wrong rule* rather than *no rule*, and only
the control separates them.

## Shared layers

**None.** The diff is confined to `XRay.Office.WordProcessing/Layout/{Paginator,FlowLayouter,
ParagraphLeading}.cs`, `XRay.Office.WordProcessing/OpenDocument/OdtLayoutSource.cs` (comments only) and
the tests. `git grep` finds no consumer of `ParagraphLeading` outside `XRay.Office.WordProcessing`, and
nothing in `Core`, `Containers`, `Text`, `Vector`, `Rendering` or `Markup` was touched. Slides and
sheets cannot move **by construction**; that is a falsifiable claim for the parent's sweep.

## Files

- `prediction.md` — committed at `c79aa554809`, before `2011ae6aacf`.
- `emptypara.py` — 44 packages, eleven families, the slope-in-`k` decomposition.
- `tableleading.py` — 12 packages: five proportions, two sizes, both orders of a two-line paragraph,
  `atLeast`, `exact`, and a control.
- `tableleading-census.py` — paragraph-then-table boundaries, resolved through the style chain.
- `atleast-census.py` — `atLeast` paragraphs, resolved the same way, printed per paragraph and per
  document and never summed into one figure.
- `audit_odfspacing.py` — six variants of round 53's own fixture; the authored-fixture trap is in its
  docstring.
- `dotnet/tests/corpus/features/table-paragraph-leading.docx` — three paragraph/table pairs, twelve
  measured positions, every one of them a reference figure.

## What the next round does first

1. **`012` and `015` — split a fly-held table across a page.** Two verdicts, fully characterised
   above: the table is `w:tblpPr`-positioned, 767.25 pt wide on a 648 pt text area, and its last row
   overflows the body by 41.7 pt on `012`. Writer's fly-held table does split; `PlaceFloatedTable`
   refuses and its remarks say why. The overflowing row is **empty**, so the word count already
   matches and only the page count is at stake.
2. **`012`'s missing fills — 75 fill operations against our 19 on page 1**, and its white
   `COL_AUTO` title. Both were found by one blind reading and both are confirmed by a second
   instrument. Neither moves a verdict; both are large on the page.
3. **`097`'s remaining 1.65 pt**, in the height of a body paragraph holding an inline image. The
   leading taken against it is right; the line is not.
4. Then the `.doc` label slant at `Ww8DocumentReader.Describe` — untouched this round, still 80 of
   the 81 remaining OpenSymbol glyphs — and the Carlito-versus-serif class, of which
   `AAC-AD-…-MAX.doc` is 46 637 glyphs.
