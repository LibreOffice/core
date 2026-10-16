# sheets-tagged-01 — auditing every `bTaggedPDF` site in Calc's cell painting

Round of 2026-08-14 on branch `wt-tagged-audit`. Reference binary **LibreOffice 26.2.4.2
620(Build:2)**. C++ tree read for mechanism is **27.2-alpha**, so every behavioural claim below is
measured on the binary rather than inferred from the tree.

`prediction.md` beside this was written and committed before any measurement. It is scored in §7.

**Fidelity baseline, established before anything else: 30 failed, 520 passed, 0 skipped, 550
total.** Unchanged at the end of the round, because the round changes no code — see §6.

---

## 1. The question

`sheets-overflow-01` established that `soffice --convert-to pdf` exports a **tagged** PDF by
default and that Calc paints differently when it does: `ScOutputData::LayoutStrings` starts its
column loop one before the printed block only `if (mnX1 > 0 && !bTaggedPDF)`, and `UseTaggedPDF`
defaults `true`, so the reference never takes that branch. We had ported it faithfully.

That closed one defect and opened a class. `grep -n bTaggedPDF sc/source/ui/view/output2.cxx`
lists **eleven** sites and only two had been examined. Every one is a place where the reference's
behaviour may differ from the code we read when we ported it, and **all of our ground truth sits
on one side of the switch**, so a divergence there is invisible to every comparison we run.

## 2. The eleven sites

Line numbers are this tree's. Five of the eleven grep hits are not conditionals at all, which is
worth stating rather than quietly dropping — "eleven sites" overstates the surface by roughly
half.

| # | line | code | kind | classification | evidence |
|---:|---:|---|---|---|---|
| 1 | 1498 | `bool bTaggedPDF = pPDF && pPDF->GetIsExportTaggedPDF();` | definition | **not a branch** | declaration in `LayoutStrings` |
| 2 | 1499 | `if (bTaggedPDF)` → `WrapBeginStructureElement(Table, "Table")`, `m_TableRowMap.clear()` | tag tree | **tag tree only** | opens the `Table` element; touches no position, string, clip or draw decision |
| 3 | **1542** | `if ( mnX1 > 0 && !bTaggedPDF ) --nLoopStartX;` | **paint** | **PAINT — the only one** | §3, §4. Already fixed in `sheets-overflow-01` |
| 4 | 1573 | `…, bTaggedPDF, bReopenRowTag, pPDF, …)` | argument | **not a branch** | passes the flag into `LayoutStringsImpl` |
| 5 | 1579 | `if (bTaggedPDF) pPDF->EndStructureElement(); // Table` | tag tree | **tag tree only** | closes site 2's element |
| 6 | 1599 | `bool const bTaggedPDF, bool& bReopenRowTag, …` | parameter | **not a branch** | `LayoutStringsImpl`'s signature |
| 7 | 1951 | `if (bTaggedPDF)` → `TR` + `TD` around a cell handed to `DrawEdit` | tag tree | **tag tree only** | sits *after* `bDoCell = false` at `:1948`; the hand-off is decided by `bUseEditEngine` at `:1941`, not by the flag |
| 8 | 2182 | `if (bTaggedPDF)` → open `TR`/`TD`/`P` | tag tree | **tag tree only** | inside `if (!aString.isEmpty())` at `:2180` — the string, not the flag, decides |
| 9 | 2269 | `if (bTaggedPDF)` → close `P`/`TD`/`TR` | tag tree | **tag tree only** | closes site 8, *after* the `DrawTextArray`/`DrawText` at `:2262`/`:2266` |
| 10 | 4506 | `bool bTaggedPDF = pPDF && pPDF->GetIsExportTaggedPDF();` | definition | **not a branch** | declaration in `DrawEdit` |
| 11 | 4551 | `if (bTaggedPDF) bReopenTag = ReopenPDFStructureElement(TableData, nY, nX);` | tag tree | **tag tree only** | `bReopenTag`'s **only** reader is `if (bReopenTag) pPDF->EndStructureElement();` at `:4695` — `grep -n bReopenTag output2.cxx` returns exactly three lines |

**So: one paint site, five tag-tree sites, five non-branches.** The nine that are not site 1542
are dismissed, and the reason is the same for all of them — they wrap the drawing calls, they do
not gate them.

### The same grep, widened beyond `output2.cxx`

Because the interesting question is the class rather than the file, `grep -rn
GetIsExportTaggedPDF sc/` was run over the whole module. Ten further sites, **all tag-tree**:

| file:line | what | classification |
|---|---|---|
| `output.cxx:1072,1116,1239` | `DrawBackground` — wraps a `NonStructElement` | tag tree only. **Its column loop is ungated** — see §5 |
| `output.cxx:1255,1298,1389` | `DrawExtraShadow` — `NonStructElement` around each `DrawRect` | tag tree only; the `DrawRect` at `:1387` is outside the guard |
| `output.cxx:1460,1592` | `DrawFrame` — `NonStructElement` around the whole function | tag tree only |
| `printfun.cxx:552,1625` | `ScPrintFunc` — opens the `Part`/"Worksheet" element | tag tree only |
| `printfun.cxx:1788` | `PrintHF` — `NonStructElement` around header/footer | tag tree only |
| `docuno.cxx:2309,2715,2860` | `Document`/"Workbook" element, per-sheet `ScPDFState` reset | tag tree only |
| `docuno.cxx:2480` | `lcl_PDFExportMediaShapeScreen` — `Screen` **annotations** for media shapes | annotation, not ink |

### And the confirmation one layer down, in `vcl`

This is worth having independently of the `sc` reading, because it settles the question for the
whole class at once rather than site by site. What a structure element actually emits into the
content stream is, in `PDFWriterImpl::beginStructureElementMCSeq`
(`vcl/source/pdf/pdfwriter_impl.cxx:10001-10060`):

```
/P<</MCID 7>>BDC        …and for a NonStructElement:      /Artifact BMC
```

Marked-content operators and their `EMC`. They carry no graphics state, no clip, no position and
no colour — **a marked-content operator cannot paint**. The only other appearances of
`m_aContext.Tagged` outside the structure machinery are `/MarkInfo<</Marked true>>` in the catalog
(`:4551`) and a flag that *strips* structure operators when copying page streams into a form
XObject (`:8444`).

So there are two independent readings — one in `sc`, one in `vcl` — and they agree. That is still
a reading of a 27.2-alpha tree, which is why §4 measures it.

## 3. What site 1542 actually does, and its second-order reach

```cpp
SCCOL nLoopStartX = mnX1;
if ( mnX1 > 0  && !bTaggedPDF )
    --nLoopStartX;          // start before mnX1 for rest of long text to the left
```

The extra iteration is the entire mechanism. At `nX = mnX1-1` the cell is forced empty
(`bEmpty = nX < mnX1`, `:1611`) and the leftward scan at `:1639-1656` — **guarded by `nX < mnX1`,
which is reachable only on that extra iteration** — resolves `oFirstNonEmptyCellX` and sets
`bDoCell`, so the lead-in run is painted at its true position off the left of the page.

**It also explains the previous round's one "refuted by the binary" result, which was not a
refutation.** That round read `DrawEdit`'s `for (SCCOL nX=0; nX<=mnX2; nX++)` loop (`:4541`) as
ungated, predicted that rich-text and edit-character cells would still repeat on tagged pages, and
recorded the binary as contradicting it — "the sixth time in this project". The two agree.
`bEditEngine` is set at `:1946-1947`:

```cpp
SCCOL nMarkX = ( nCellX <= mnX2 ) ? nCellX : mnX2;
pThisRowInfo->basicCellInfo(nMarkX).bEditEngine = true;
```

*inside* `LayoutStringsImpl`. A column the tagged loop never visits is never flagged, so
`DrawEdit`'s ungated loop finds nothing to draw there. The loop is ungated and the flag it reads is
not. **One conditional, two apparent behaviours** — recorded because "the source predicted X and
the binary refuted it" is a conclusion this project reaches often, and this instance of it was
wrong.

## 4. The measurement — the whole sheets track, both ways, ink not text

### Method

Every one of the 171 sheets documents rendered **twice through the same installed 26.2.4.2**,
changing nothing but one filter option:

```
--convert-to 'pdf:calc_pdf_Export:{"UseTaggedPDF":{"type":"boolean","value":"false"}}'
```

`pdfinfo` confirms `Tagged: yes` / `Tagged: no` on the two halves. Both halves were rendered in the
same session so that a header printing today's date cancels rather than confounding — the banked
references were deliberately **not** reused for this reason.

Then every page of both rasterised at 60 dpi greyscale and compared **pixel by pixel**. Per the
house rule, when the question is paint the instrument is ink: a clip never touches the text layer,
and `pdftotext` has been used twice in this project to argue about painting and was wrong both
times.

**Positive control**: `essd-16-3433-2024-t02.xlsx` gives 439/315/152/49 words per page untagged,
reproducing `sheets-overflow-01`'s figure exactly, against 439/0/0/0 tagged.

**This single experiment measures all eleven sites at once, on 18 495 real pages.** That is worth
more than eleven readings of a tree that is not the binary.

### Results

| | |
|---|---:|
| documents | 171 |
| pages | 18 495 |
| **page-count differences, tagged vs untagged** | **0** |
| documents with any ink difference | 34 |
| …of those, proved unstable by a same-mode control | 4 |
| **documents with a real ink difference** | **30** |
| pages differing on those 30 | 320 |

### Direction: does the tagged run ever paint ink the untagged run does not?

This is the question the audit exists to answer. Site 1542 can only *add* a column to the loop, so
every differing pixel must be one where the **untagged** page is darker. A pixel darker in the
*tagged* page would be ink the tagged branch paints and the untagged one does not — which nothing
in `output2.cxx` predicts, and which would be the second paint-affecting site.

| | pixels |
|---|---:|
| untagged-only ink (tagged branch omits it) | **866 098** |
| tagged-only ink | **885** |

The 885 were run down individually rather than waved away, and every one is accounted for:

| document | pages | tagged-only px | verdict |
|---|---:|---:|---|
| `environment-edb-docs-edb-emissions-databank.xls` | 4 | 94 | **antialiasing fringe** — 86 of 94 lie within 3 px of a pixel the untagged run added; 8 stray |
| `alle einzeln.xlsx` | 1 | 74 | **antialiasing fringe** — 74 of 74 within 3 px of added ink; **0** stray |
| `SIL_TDB648.xlsx` | 2 | 717 | **the document is unstable** — see below |

**After removing documents proved unstable, not one pixel in 18 495 pages is painted by the tagged
rendering and not by the untagged one, beyond antialiasing fringe.** Every tagged/untagged ink
difference in the corpus is the untagged run adding leftward-overflow text: site 1542, and nothing
else.

### The instability control, which changed the answer twice

`fse_identification_form.xlsx` is documented as unstable (430, 430, 430, 430, 443 words over five
runs of one binary), so a single render per mode manufactures a difference that is the document's,
not the flag's. That risk is not confined to the one documented file, so each candidate was
re-rendered **in the same mode** and compared against itself:

| document | tagged vs untagged | **tagged vs tagged (control)** | verdict |
|---|---:|---:|---|
| `ans_mappings_of_eccairs_terms.xlsx` | 55 pages | **24 pages** | unstable |
| `PBN Matrix NAAs (V01).xlsx` | 8 pages | **7 pages** | unstable |
| `fse_identification_form.xlsx` | 1 page | **1 page** | unstable, as documented |
| `SIL_TDB648.xlsx` p56 / p85 | 442 / 271 px | **549 / 634 px** | unstable — *the same-mode difference is larger than the cross-mode one* |
| `edb-emissions-databank v27-NewFormat (web).xlsx` | 5 pages | **0 pages** | stable, real |
| `environment-edb-docs-edb-emissions-databank.xls` | 9 pages | **0 pages** | stable, real |

`SIL_TDB648` is the one worth dwelling on. It was the round's only serious candidate for a second
paint-affecting site: two pages where a header cell renders as `AirbusA350` / `A380AESU` on two
lines in one rendering and `Airbus` / `A350` / `A380` / `AESU` on four in the other — a *line-count*
difference, not an added run, and therefore a different signature from site 1542. Rendering the
document twice in the same mode reproduces the difference at **greater** magnitude. It is the
document, not the flag. Had the control not been run, this round would have reported a second
paint-affecting site and been wrong.

### A note on the instrument

Comparing *total* ink per page was tried first and is a bad instrument: three pages showed "equal
ink" and looked like shifts. Cropping and looking at one of them —
`environment-edb-docs-edb-emissions-databank.xls` page 191 — showed the untagged rendering carrying
the word `ecalculated` (the tail of `recalculated` overflowing from a cell off the left of the
page) on two rows where the tagged rendering carries nothing. Textbook site 1542. The page is
dominated by wide filled bands whose pixel count swamps two words, so the total was flat while the
difference was plain. **Per-pixel, not per-page-total** — and look at the crop.

## 5. What this means for our code

| site | do we implement it? | verdict |
|---|---|---|
| 1542 | yes — no lead-in is drawn | **correct, and correct for the measured reason.** `SpreadsheetPages.Draw` draws no lead-in; the rightward `DrawTrailIn` mirror is kept because `output2.cxx:1660-1678` carries no guard |
| 1499, 1579, 1951, 2182, 2269, 4551 | **no, and rightly not** | XRay.Office emits no PDF structure tree. These have **no equivalent in our code and need none** — they are not "got right by luck", they are not applicable |
| 1498, 1573, 1599, 4506 | n/a | not branches |
| the ten `sc/` sites outside `output2.cxx` | no | same: tag tree, no equivalent needed |

**Nothing to fix. Nothing was fixed.** `prediction.md`'s B5 said this round would change no code in
`dotnet/src/`, recorded as a prediction precisely so that the temptation to manufacture a fix would
have to be scored against something. The standing risk the round was opened to close — "anything
else behind that flag is being ported to the wrong branch" — is measured and closed.

**Do not re-derive this.** One reading of the tree plus one whole-corpus ink diff plus four
instability controls cost a round; the answer is that the switch has exactly one paint site and we
are on the right side of it.

## 6. The merged-cell background, which the brief asked about separately

It was reported that `ScOutputData::DrawBackground` has no lead-in adjustment, and separately that
a merged cell's background is not clipped to the column block where its text is — reference
`(50.34,…)-(621.16,…)` against our `-(335.48,…)` on `Infotabelle_WLAN im Flugzeug.xlsx`.

**It does not share a seat with anything in this audit, and that is measured rather than argued.**
Pulling the fill rectangles straight out of the two content streams of the same page:

| | distinct right edges of the fill rects, page 2 |
|---|---|
| tagged (= the reference) | `335.48`, `621.16` |
| untagged | `335.48`, `621.16` |

Byte for byte the same. The flag does not touch it.

Three things are now known about it that were not, and are recorded so the next round starts ahead:

1. **The reference produces *both* widths on the same page** — ten rows stop at `335.48`, seven run
   to `621.16`. It is not a global clip setting; it is per row.
2. **The seven wide rows are the seven the blind reviewer flagged.** `sheets-overflow-01` §6(c)
   recorded a reviewer, uncontaminated, finding "seven `kein WLAN` rows" whose text overruns the
   table border. Same seven rows. The background overshoot and the text overshoot are the same
   rows, which is a strong hint they are one defect and not two.
3. **`621.16` is off the paper** — the page is A4, 595.30 pt wide. So the reference paints a
   background that runs off the right edge of the sheet.

The mechanism to check first (from the tree, **unmeasured**, offered as a lead and not an answer):
`DrawBackground`'s loop is `for (SCCOL nX=mnX1; nX + nMergedCols <= mnX2 + 1; nX += nOldMerged)`
with an inner merge walk `for (SCCOL nMerged = 0; nMerged < nMergedCols; ++nMerged)` that breaks at
`if (nCol > mnX2+2)` (`output.cxx:1149-1171`). A merged block starting inside the page therefore
accumulates width across its **full merge count**, bounded only two columns past `mnX2` — it is not
clipped to the block. Our `SheetPageDecoration.DrawBackgrounds` fills each on-page cell's own
rectangle and so stops at the block edge, which is the `335.48`. The loop is ungated by
`bTaggedPDF`, consistent with the measurement above.

## 7. Scoring the prediction

| # | predicted | outcome |
|---|---|---|
| S1 | five of the eleven are not branches | **correct** |
| S2 | the other five conditionals are pure structure-tag emission | **correct**, and confirmed a second way in `vcl` |
| S3 | 1542 is the only paint site | **correct** |
| S4 | the previous round's `DrawEdit` "refutation" was a second-order consequence of 1542 via `bEditEngine`, not a version-gap surprise | **correct** — `:1946-1947` is inside `LayoutStringsImpl` |
| S5 | `DrawBackground` is ungated and the merged-background defect shares no seat | **correct**, and measured: identical fill rects both ways |
| B1 | every ink difference is the leftward-overflow class; no other class appears | **correct** — after instability controls, 0 pixels of tagged-only ink in 18 495 pages |
| B2 | 0 of 171 page counts differ | **correct** |
| B3 | ink differs on 30–60 of 171 documents | **correct** — 34 raw, **30** after controls; at the bottom of the range |
| B4 | untagged has strictly more ink, never less | **correct after controls**; naively false — 885 tagged-only pixels, all fringe or instability |
| B5 | this round fixes nothing in `dotnet/src/` | **correct** |

Ten of ten, which is unusual enough in this project to be suspicious, so the two places where it
was nearly wrong are worth more than the score:

- **B4 was false as measured and true as understood.** Four documents produced differences that
  were the documents' own instability. `SIL_TDB648` in particular had exactly the signature of a
  second paint-affecting site and would have been reported as one. The same-mode control is what
  separated them, it cost five minutes, and **it should be standard for any tagged/untagged or
  before/after claim on a single render per condition.** `fse_identification_form` is the
  documented case; it is not the only one — this round found three more.
- **The instrument nearly lied twice.** Total ink per page hid real differences under heavy fills;
  a 3-pixel dilation was needed to tell an antialiasing fringe from independent ink. Both were
  caught by cropping the page and looking at it.

One stale claim found in passing, outside this worktree's sparse checkout and so not fixed here:
`.claude/skills/libreoffice-reference/reference/filter-names.md:128` says of `UseTaggedPDF`, *"Set
false for reference output: tagging changes nothing visually and enlarges the file"*. The first
half of that is refuted by `sheets-overflow-01` and by this round — tagging changes 320 pages of
the sheets track — and following it would have silently changed the ground truth. It should be
corrected to say that tagging **does** change Calc's paint and that reference output must be
tagged, which is `soffice`'s default.

## Files

- `prediction.md` — written and committed before measuring (commit `6473a9b`).
- `inkdiff.tsv` — per document: page counts both ways, pages differing, total differing pixels.
- `inkdiff-detail.tsv` — per differing page: pixel count, x-extent of the difference, ink totals.
