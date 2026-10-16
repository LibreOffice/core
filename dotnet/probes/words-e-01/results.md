# Words-E round 1 — the landscape sub-shape is three defects, and one of them is a page break that should have died

Baseline `8a8194c517c`. Worktree `/c/sandbox/workdir/wt-words-e`, branch `wt-words-e`,
`XRAY_OFFICE_CLI` set explicitly to this tree's binary on every sweep. Reference
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`, 200 PDFs from LibreOffice 26.2.4.2
620(Build:2). `check-env.sh` green before anything was measured: Calibri→Carlito,
Cambria→Caladea, Arial→Liberation Sans, DejaVu Sans→DejaVu Sans, pdftoppm and pdftotext
26.01.0. `SOURCE_DATE_EPOCH=1700000000` and `TZ=UTC` on every render, ours and the probes'
alike. `git merge-base --is-ancestor 5c34499a205 HEAD` succeeds, so words-d's sloppy-fit
page geometry fix is in this baseline and page geometry stayed closed.

---

## 1. The headline

> **Words is 154 / 200 before and 155 / 200 after.** Absolute page error 117 → **115**, exact
> page counts 163 → **165**, render failures 0 → 0. **Four of 200 renderings changed, all four
> moved a page count, and two moved a verdict.**
>
> The result is not the +1. It is that **the three-document landscape sub-shape is three
> different defects**, and saying so cost one of them being fixed. Its "exactly one extra
> portrait page immediately before the first landscape run" is, on two of the three, an
> artefact of run-length coding: `33004.docx`'s extra page is at page 39 of a 40-page portrait
> run spanning fifteen sections, and nothing about it is near the landscape boundary.
>
> The one that *was* localised at a section boundary is `1_tpr_template__from_fy14_.docx`, and
> its cause is now measured, fixed and tested: **a page break landing on an empty section mark
> must die with the mark.** LibreOffice's removal test protects a *column* break and names no
> page break at all; we were taking the break at its word and emitting a page holding one word,
> the footer's page number.

Verdict split: 154 `match` / 29 `pages` / 8 `pages,words` / 9 `words` before;
155 / 28 / 7 / 10 after. 0 `unembedded`, 0 render failures at both ends.

---

## 2. The prediction, scored

`prediction.md` in this directory, committed as `45d8c80a8d1` before the first render of the
round and before any per-document figure of this baseline was read. Its blind-spot section is
§A there and is not restated here except where it fired.

| # | prediction | outcome |
|---|---|---|
| P1 | baseline reproduces to the digit: 154, 117, 163, 0 failed | **held** — all four |
| P2 | the 37 check-1 failures are `words-d-01/gate-after.tsv`'s, same page pairs | **held** — `diff` on the page and verdict columns empty over all 200 |
| P3 | the three page-shape sequences reproduce: `A6BA`/`A3BA4B`/`A40B3A5` against `A5BA`/`A2BA4B`/`A39B3A5` | **held** — all six, to the run length |
| P4 | the extra page is overflow on 3 of 3, blank on 0 of 3 | **refuted** — **1 of 3** overflow, **1 of 3** a near-blank inserted page, **1 of 3** a table splitting deep in the portrait run. Three documents, three causes |
| P5 | on `template---tpr…` the extra page and the line-fit are one defect | **held on that document** — its page 2 fits 3 fewer lines and the deficit cascades into a 14-word page 4 — but on a premise (P4) that was refuted, so it generalises to nothing |
| P6 | none of the three uses `evenPage`/`oddPage` at the boundary | **held** — 0 of 3; all breaks are `nextPage`, plus one `continuous` |
| P7 | 26.2.4.2 emits no extra page for `nextPage` into landscape, slope 1 | **held** — five fills (1, 46, 47, 92, 93 paragraphs) give 1, 1, 2, 2, 3 portrait pages, and the landscape section takes exactly one, for every break type |
| P8 | 26.2.4.2 promotes a `continuous` break across a page-setup change | **held in outcome and wrong in mechanism** — it promotes across an **orientation flag** change and across nothing else. A continuous section 720 twips wider, 720 twips taller, one twip wider, or an inch deeper in margin all stay on the page |
| P9 | we do not implement that promotion | **held** — 8 of 72 authored variants |
| P10 | the landscape rule is not the cause of the three documents' extra page | **split.** Right that this rule is unreachable by them — a census of all 134 DOCX finds **0** across an orientation change — and wrong that the cause was P4's overflow. The rule that mattered was a different section rule I had not predicted at all |
| P11 | verdicts moved: point estimate 0, band 0–3 | **inside the band, off the point estimate** — **+1** |
| P12 | the continuous fix reaches 1–15 renderings | **refuted** — **0**. It is a correct fix with zero corpus reach, and the census says so independently of the sweep |
| P13 | the page-1 divergence concentration has no shared cause | **held**, and see §7 — it also refutes the briefed characterisation it was predicting about |
| P14 | a page-1 classifier over the 154 matching documents shows the same distribution shape | **held** |

**Three refuted, two split, and P8's mechanism wrong underneath a correct outcome.** P4 is the
one worth reading twice: it is the prediction the whole round was aimed at, it named a
plausible single cause for a three-document shape, and the shape had three causes.

### Three claims in the brief that did not survive

1. **"Exactly one extra portrait page immediately before the first landscape run" is not a
   localisation.** Under run-length coding, *any* extra portrait page anywhere in a portrait
   prefix presents as one extra page at the end of that prefix. `33004.docx` has fifteen
   consecutive portrait sections before its landscape one, so its whole 40-page prefix is one
   run; its extra page is at page 39–40 (ours 79 extracted lines on page 39 against the
   reference's 49, a table splitting differently), forty pages of content away from any
   section boundary.
2. **`template---tpr…docx`'s "reference fits 37 lines where we fit 33" does not reproduce.**
   Measured by extracted lines: **page 2 is ours 27 lines / 213 words against the reference's
   30 / 251**, and page 3 is 37 lines on *both* sides with different content. The 37 is real
   and it is page 3's, on both sides; the defect is a 3-line deficit on page 2.
3. **"The ±1 cluster's divergences are bunched at page 1" is an instrument artefact.** See §7.

---

## 3. The landscape rule, measured on the installed binary

Everything here was rendered by the installed 26.2.4.2 from DOCX authored by `mkdocx.py` —
one paragraph style, one margin set, one thing varied at a time — and read back from the
PDF's own media boxes, never from a parse of the input. Attribution to C++ came afterwards,
because the tree in this checkout is 27.2.0.0.alpha0+ and made none of the references.

Sequences below are run-length coded page shapes: `P2L1` is two portrait pages then one
landscape. 46 paragraphs at 12 pt single-spaced is exactly one full Letter page, which is what
makes 47 the discriminating fill.

### 3.1 Break type does not matter, and the slope is 1 (25 variants)

| section 1 fill | `nextPage` | `continuous` | `evenPage` | `oddPage` | no `w:type` |
|---|---|---|---|---|---|
| 1 paragraph | `P1L1` | `P1L1` | `P1L1` | `P1L1` | `P1L1` |
| 46 (one full page) | `P1L1` | `P1L1` | `P1L1` | `P1L1` | `P1L1` |
| 47 (a page and a line) | `P2L1` | `P2L1` | `P2L1` | `P2L1` | `P2L1` |
| 92 (two full pages) | `P2L1` | `P2L1` | `P2L1` | `P2L1` | `P2L1` |
| 93 | `P3L1` | `P3L1` | `P3L1` | `P3L1` | `P3L1` |

Portrait pages are `ceil(fill / 46)` exactly, at five points, and the landscape section takes
one page and never two. **No break type inserts a page at a portrait→landscape boundary** —
including `evenPage` and `oddPage`, whose parity filler is laid out but not exported
(`IsSkipEmptyPages`), which our Paginator already models and which the 47-fill no-geometry-change
row below confirms from the other side.

### 3.2 What promotes a `continuous` break — the flag, and only the flag (14 variants)

Section 1 filled to 47 paragraphs throughout, so a promoted break shows as `P3`/`…L1` and a
genuine continuation as `P2`.

| section 1 → continuous section 2 | 26.2.4.2 |
|---|---|
| identical setup | `P2` |
| 720 twips wider (still portrait) | `P2` |
| 720 twips taller | `P2` |
| **one twip** wider | `P2` |
| `w:orient="portrait"` added, same numbers | `P2` |
| top margin +720 twips | `P2` |
| **`w:orient="landscape"`** | **`P2L1`** |
| 15840 × 12240 with **no** `w:orient` (landscape *shaped*) | `P2` — and the whole document stays portrait |
| L(flag) → continuous P (no flag) | `L2P1` |
| L(flag) → continuous P (flag) | `L2P1` |
| L(flag) → continuous L(flag) | `L2` |
| **L *shaped*, no flag → continuous P, no flag** | **`L2`** |
| L shaped, no flag → continuous L(flag) | `L3` |
| P(flag) → continuous P(flag) | `P2` |

The last four rows are the ones that settle it. A physically landscape sheet that does not
state `w:orient` does **not** count as landscape: it sits beside a portrait-shaped continuous
section with no break at all, and stating the flag on that second section breaks the page even
though the sheet stops being landscape. Neither the stated width nor the stated height nor the
margins enter the comparison at any tolerance, down to one twip.

> **The rule, as measured.** A `w:type="continuous"` section break is replaced by a `nextPage`
> break exactly when the section's `w:orient` flag differs from the previous section's, where
> an absent `w:orient` means portrait. Nothing else about the two page setups is compared.

**Where it lives.** `sw/source/writerfilter/dmapper/PropertyMap.cxx`:1661-1678,
`SectionPropertyMap::CloseSectionGroup` — "if page orientation differs from previous section, it
can't be treated as continuous". It reads `PROP_IS_LANDSCAPE` off its own and the previous
section's property map and compares nothing else, and `PROP_IS_LANDSCAPE` is written once, in
`DomainMapper.cxx`:2868, from a `CT_PageSz.orient` that is reset to false at :2859 and set only
at :831 from `w:orient` itself. The 27.2-alpha source describes 26.2.4.2 here exactly, and it is
recorded that way round — measured first, attributed second — because the same tree has
misdescribed the reference binary twice this session.

### 3.3 The rule that actually mattered: an empty section mark and a page break (17 variants)

We already drop the paragraph a `w:sectPr` hangs off when it has no content — Word's section
break *is* that paragraph mark, and `sweep5.py` confirms 26.2.4.2 charges it nothing: 46 text
paragraphs plus an empty mark is one page, plus a mark holding one space is two, and marks
holding an empty run, an empty `w:t` or a bookmark are all still empty. **All eight of those
agreed with us before this round.**

The gap was next door.

| section 1 tail | 26.2.4.2 | ours, before |
|---|---|---|
| empty mark, no page break | `P1L1` | `P1L1` |
| mark carrying text | `P1L1` | `P1L1` |
| **page-break paragraph, then empty mark** | **`P1L1`** | `P2L1` |
| page-break paragraph, then mark carrying text | `P2L1` | `P2L1` |
| page break inside the empty mark | `P1L1` | `P1L1` |
| page-break paragraph, empty paragraph, empty mark | `P2L1` | `P2L1` |
| **two page-break paragraphs, then empty mark** | **`P2L1`** | `P3L1` |
| **`w:pageBreakBefore` on the empty mark itself** | **`P1L1`** | `P2L1` |
| **the same on a section already filling its page** | **`P1L1`** | `P2L1` |
| **page-break paragraph, empty mark, section 2 portrait** | **`P2`** | `P3` |
| mark carrying a column break | `P2L1` | `P1L1` |
| column-break paragraph, then empty mark | `P2L1` | `P1L1` |

> **The rule, as measured.** The empty section mark is removed whether or not a page break has
> landed on it, and the break dies with it — a page break is only ever an instruction to put the
> *next* paragraph on a new page, and after the removal there is no next paragraph to move. A
> *column* break is different and does save the mark.

**Where it lives.** `sw/source/writerfilter/dmapper/DomainMapper.cxx`:4852, the `bRemove`
expression. Its guard list is `!bSingleParagraphAfterRedline && !bIsColumnBreak &&
!GetIsLastSectionGroup() && !GetParaHadField() && !GetIsPreviousParagraphFramed() &&
!HasTopAnchoredObjects() && !IsParaWithInlineObject()`. `bIsColumnBreak` is built twelve lines
above from `BreakType_COLUMN_BEFORE`/`_AFTER`/`_BOTH` and **there is no page-break term
anywhere in it**. The two rows measuring `w:pageBreakBefore` matter because they reach the same
paragraph property by a second route, so a fix that forgave only the deferred break would leave
half the defect standing.

**The last two rows are a gap in the other direction and are not fixed here.** 26.2.4.2 keeps a
mark carrying a column break and gives it a page; we drop it, because the reader models only
`w:br w:type="page"` and has no paragraph-level column break at all. That is a different
subsystem, its corpus reach is unmeasured, and it is recorded rather than guessed at. It is the
only remaining disagreement in the 72 authored variants.

### 3.4 The variant scoreboard

Rendered by both renderers from the same DOCX bytes, `ours.py` reading each side's own PDF into
a separate directory — the first version of that script wrote both to one directory under one
name, and `xray-office render`'s output silently overwrote `soffice`'s, which reads as perfect
agreement. Caught by an unrelated CLI error before any number was believed.

| | agreeing | differing |
|---|---:|---:|
| 72 authored variants, before | 55 | **17** |
| after the section-mark fix | 62 | 10 |
| after both fixes | **70** | **2** (both the column-break gap of §3.3) |

---

## 4. The three documents, taken apart page by page

`align.py` prints, per page and side by side, the shape, the extracted word count, the line
count and the first six words. An *inserted* page shows as a page with almost no words and the
sequence resuming after it; an *overflow* shows as every page's opening words walking backwards.

| document | ours/ref | where | what it is |
|---|---|---|---|
| `1_tpr_template__from_fy14_.docx` | 9/8 | page 3 | **inserted.** Pages 1 and 2 are word for word and line for line the reference's — 541/541 and 494/494 — and our page 3 holds **one word**, the footer's page number, at y 779–793 pt of an 842 pt sheet. Section 1 ends with an explicit `w:br w:type="page"` and then an empty mark |
| `template---tpr-…docx` | 8/7 | page 2 | **overflow.** Page 2 is ours 27 lines / 213 words against 30 / 251; pages 2–4 of ours hold 622 words where the reference's pages 2–3 hold 623. The 3-line deficit cascades into a 14-word one-line page 4 |
| `33004.docx` | 48/47 | pages 39–40 | **a table splitting.** Pages 2–5 realign exactly (301, 376, 222, 326 on both). Page 39 is ours 79 lines / 113 words against the reference's 49 / 95. The first landscape section is number 16 of 19 and every one of the other 18 is `nextPage`, so the whole 40-page prefix is a single run and the divergence is nowhere near its end |

After the fix, `1_tpr_template` is `P2L1P4L1` on both sides and matches. The other two are
unchanged and remain open, as two unrelated flow defects rather than one shape.

---

## 5. The implementation, with file and line

| file | change |
|---|---|
| `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs`:403 `IsSectionMarkOnly` | dropped the `&& !paragraph.Format.StartsNewPage` term. Column breaks need no guard here because only `w:br w:type="page"` ever reaches that flag (`DocxLayoutSource.cs`:1170), so the term could never have meant one |
| `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxReader.cs` `PromoteContinuousAcrossOrientation`, new, called from `ReadSections` | rewrites a `Continuous` break to `NextPage` when `Page.IsLandscape` differs from the previous section's. Stated values, not carried-forward ones, because that is what `GetLastSectionContext()` reads |

`Page.IsLandscape` is already exactly the `w:orient` flag — `DocxPageGeometry.ReadGeometry`
records it and deliberately does not swap the dimensions on its strength — so the promotion is a
comparison of the one thing 26.2.4.2 compares. **The WW8 reader is untouched**: its
`ResolveContinuousBreaks` implements a different importer's rule (`ww8par6.cxx`, a whole-sheet
comparison) and this is not that rule.

### Tests — six, **all verified by reintroduction, none a drift guard**

`verify-test.sh` on a clean tree, one mutation at a time.

| test | mutation that fails it |
|---|---|
| `APageBreakDoesNotSaveAnEmptySectionMark` | restoring `&& !paragraph.Format.StartsNewPage` |
| `PageBreakBeforeOnTheMarkItselfDoesNotSaveItEither` | the same |
| `AContinuousSectionThatTurnsTheSheetStartsAPage` | removing the promotion pass |
| `AContinuousSectionThatKeepsTheOrientationDoesNotStartAPage` | promoting **every** continuous break |
| `AContinuousSectionOfADifferentSizeButTheSameOrientationDoesNotStartAPage` | promoting on any geometry difference; promoting every break |
| `ALandscapeShapedSheetWithoutTheFlagDoesNotCountAsLandscape` | comparing width against height instead of the flag; and both of the above |

Four of the six detect **over**-application, which is the failure mode a rule fitted to eight
variants invites. Every fixture is newly authored and minimal; nothing was copied or excerpted
from the corpus, and the CV is not mentioned anywhere in this round's artefacts.

### Build and suite

`dotnet build XRay.Office.slnx -v q -nologo`: **0 warnings, 0 errors.** Ten non-Fidelity projects,
one at a time in the foreground: Core 284, Containers 109, Text 289, Vector 295, Rendering 121
(1 skipped), Markup 259, OpenDocument 125, **WordProcessing 775**, Spreadsheets 628,
Presentations 592 — **3477 total, 0 failed**, against 3471 before and 769 WordProcessing.
Fidelity not run, per instruction.

No cross-track sweep is owed: both changes are inside `XRay.Office.WordProcessing`, which
`XRay.Office.Spreadsheets` and `XRay.Office.Presentations` do not reference. That is a structural
argument and is labelled as one.

---

## 6. Reach, measured two ways, and one fix that reaches nothing

| | measured |
|---|---:|
| renderings changed byte for byte, of 200 | **4** |
| of those, page count moved | **4** |
| of those, verdict moved | **2** |
| static census: DOCX with an empty mark carrying a page break | **5** |
| static census: DOCX with a `continuous` section at all | 19 |
| static census: DOCX with a `continuous` section **across an orientation change** | **0** |

**So the continuous/orientation fix changes nothing on this corpus, and both instruments say so
independently** — the census before the sweep and the byte comparison after it. It is a correct
rule with eight measured variants, four tests and zero reach, shipped because the next document
that carries the shape should not have to find it again.

The mark census **over-counts by one against the byte comparison**, and the direction is worth
recording. `PES-Technical-Report-Template_Jan_2019.docx` has the shape and its rendering did not
change a byte: dropping that mark moved nothing there. Both numbers are reported rather than the
flattering one, and the byte comparison is the instrument without a threshold.

The four movers:

| ours/ref before → after | verdict before → after | document |
|---|---|---|
| 9/8 → **8/8** | `pages` → **`match`** | `1_tpr_template__from_fy14_.docx` |
| 7/6 → **6/6** | `pages,words` → **`words`** | `B11. TE.CAO.00129 Experience logbook.docx` |
| 315/312 → 314/312 | `pages` → `pages` | `02_mcar_part-2_and_IS_v2.10.docx` |
| 80/82 → 79/82 | `pages` → `pages` | `EHEST-SMS-Safety-Management-Manual-V2.docx` |

**Three of the four move towards the reference and one moves away from it.** `EHEST` was two
pages short and is now three. Its mark is genuinely one 26.2.4.2 also removes — the rule is not
in doubt — so the extra page it was carrying was cancelling a page it loses somewhere else, and
the arithmetic that made it look closer is gone. That is a document whose 79/82 is now honest
rather than a document this round broke, but the page error it contributes went up by one and
that is in the 115.

### What these censuses cannot see, stated rather than discovered later

- **Both censuses read `word/document.xml` and are blind to the 66 `.doc`.** Here that costs
  nothing and it is checkable rather than assumed: both changes are in `DocxReader` and
  `DocxLayoutSource`, which no `.doc` reaches — `Ww8DocumentReader` has its own section table
  and its own continuous rule. The corpus-wide byte comparison covers all 200 regardless and
  found no `.doc` among the four movers.
- **The mark census reads only direct `w:body` siblings**, so a page break deferred from inside
  a `w:sdt` or a tracked insertion, or a `pageBreakBefore` inherited from a paragraph *style*,
  is invisible to it. It can under-count, and the byte comparison is what bounds that.
- **The variant sweeps prove what 26.2.4.2 does with my markup**, not what a real document
  contains. §4 is the bridge and it is one document wide.

---

## 7. The page-1 concentration: refuted, and it refutes the brief's version of it too

`first-divergence.py` answers "where does the ink first differ", which a substituted face or a
hairline moves and which — per its own new header — cannot resolve geometry below ~1.6 pt on A4
and has no kind for "this page is extra". For a page-count question the narrower instrument is
better: **on which page do the two renderings stop holding the same amount of text?** A page's
extracted word count is unmoved by a glyph substitution or a wrong colour and moves the instant
a line lands on a different page.

`flowdiv.py`, over all 200 including the 155 that match, tolerance 2 words per page.

**Known-answer control, run before the result was read**: with the reference PDFs supplied as
the *ours* column, all 200 return `none` at both tolerance 2 and exact equality.

| first page whose word count differs by >2 | match (155) | `pages` (28) | `pages,words` (7) | `words` (10) |
|---|---:|---:|---:|---:|
| none | **80** | **0** | **0** | **0** |
| 1 | 27 | 8 | 4 | 6 |
| 2–4 | 31 | 12 | 3 | 2 |
| 5–20 | 15 | 7 | 0 | 2 |
| 21+ | 2 | 1 | 0 | 0 |

**The standing finding reproduces on a completely different instrument: what separates a failure
from a pass is whether a divergence exists at all.** 80 of 155 matching documents have no flow
divergence anywhere; 0 of 45 failures have none. Conditional on diverging at all, 27 of 75
matching documents break on page 1 (36 %) against 12 of 35 page failures (34 %) — the same
distribution, twice.

**And the briefed characterisation does not survive.** words-d reported the ±1 cluster's
divergences "bunched at page 1". The cluster is 21 documents after this round (23 before it;
`1_tpr_template` and `B11. TE.CAO.00129` left it), and under a flow-only instrument its first
breaks are **1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 11, 16** — six distinct
values spanning pages 1 to 16, with only 8 of 21 on page 1. That is much closer to the round
*before* words-d, which had them spread 1 to 91. The two instruments
disagree because they measure different things: an ink divergence on page 1 is routinely a font
or hairline difference that has nothing to do with a page count. Measured on the 17 page
failures whose *ink* words-d put on page 1: **3 of the 17 have byte-identical extracted text on
page 1**, and their flow breaks are `1×11, 2×3, 3, 7, 15` — so the ink instrument is right about
eleven of them and puts the other six up to fourteen pages early. The correction is smaller than
the ±1 one above, and it is in the same direction.

The largest single case is unchanged and unexplained: `A_320.doc`, 141/118, whose first flow
break is page **21** — twenty pages of identical flow before anything moves at all.

**What this cannot see.** A flow defect that preserves a page's word count within two tokens is
invisible to it; a running head contributes a per-page constant to both sides and so cancels;
and it is `pdftotext` 26.01.0's tokenisation throughout, which `words-rebase-02` §4 established
is itself a measurement input.

---

## 8. Measured, inferred, not established

**Measured**, each from a render:

- The baseline 154 / 117 / 163 / 0 reproducing to the digit, and the after 155 / 115 / 165 / 0.
- The 72 authored variants at 26.2.4.2 and at both our baseline and our fixed code (§3).
- The page-by-page decomposition of all three landscape documents (§4).
- 4 of 200 renderings changed, 4 page counts, 2 verdicts (§6).
- The two static censuses, 5 and 0 of 134 (§6).
- The flow-divergence classification over all 200 with the matching control, and its
  reference-against-reference known-answer control (§7).
- All six tests failing under a reintroduced defect (§5).

**Inferred, and marked as such:**

- That `CloseSectionGroup` and `bRemove` are the *mechanisms*. Both behaviours are measured on
  the installed binary and both match the 27.2-alpha source term for term; the attribution is
  still a reading of a tree that made none of the references.
- That the `w:pageBreakBefore` and deferred-`w:br` routes are one property in LibreOffice as
  they are in us. Measured to behave identically; that they are the same variable is read from
  source.
- That the `.doc` half cannot reach either change. Read from the reader graph and corroborated
  by no `.doc` appearing among the four byte-level movers, not by a separate measurement.

**Not established:**

- **Why `template---tpr…docx`'s page 2 fits 27 lines where the reference fits 30.** It is a
  table metric on a specific page and it is the whole of that document's page-count error.
- **Why `33004.docx`'s page 39 takes 79 extracted lines against the reference's 49.** A table
  splitting differently, forty pages into the document.
- **The column-break arm of the mark rule** (§3.3, last two rows): 26.2.4.2 keeps a mark
  carrying a column break and we drop it, and the reader has no paragraph-level column break to
  guard with. Two authored variants, unmeasured corpus reach.
- **The RTF and ODF reach of either rule** — the words corpus holds neither.
- Everything words-d left open: `A_320.doc`'s 141/118, and *why* the 35 remaining page failures
  fail. Page geometry stays excluded; section-break page emission is now excluded too, for
  every shape in §3 except the column-break one.

---

## Artefacts

In this directory: `prediction.md` (committed first, at `45d8c80a8d1`), `mkdocx.py` (the DOCX
author), `sweep.py`/`sweep2.py`/`sweep3.py`/`sweep4.py`/`sweep5.py`/`sweep6.py` (the six variant
sweeps against `soffice`), `ours.py` (the same variants through our CLI, with the
overwrite trap it was caught by written into it), `align.py` (the page-by-page decomposition),
`flowdiv.py` (the flow-divergence classifier and its control), `gate-base.tsv` and
`gate-after.tsv` (200 rows each).
