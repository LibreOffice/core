# Round 126 — a decoration crosses a tab, and a contents entry loses its style in every paragraph and not only the first

Round 126, `/home/user/wt-wordsdec`, branch `agent/wordsdec`, base `bdfa0c947`, 2026-09-14.
Two seats, both opened by round 124's blind reading of `150-5370-10H.docx` page 7, both closed here.

| | |
|---|---|
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**, `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| reference renderings | the **banked** leg of gate r125, `/home/user/gate-r125/ref`, 947 PDFs, rendered 2026-09-14 11:30 |
| our base leg | `/home/user/gate-r125-cli/XRay.Office.Cli`, built 11:24 — and `bdfa0c947` touches `dotnet/probes` only, `git show --name-only` over `dotnet/src/` is **0 files**, so that binary *is* this round's base |
| our after leg | `dotnet/tools/XRay.Office.Cli` built in this worktree at the committed source, `SOURCE_DATE_EPOCH=1757462400`, one output directory per **document** |
| population | the **338** words-track documents of `MANIFEST.tsv`; the gap census additionally filters to gate r125's `match` rows, as round 124's did |
| C++ tree | `/home/user/libreoffice-core`, read only. **C8**: it is `27.2.0.0.alpha0+`, not 26.2's source, so every arm below is confirmed a second time against 26.2.4.2's own output |
| `/usr/bin/soffice` | 24.2.7.2 — **not used for anything in this round** |

**Seat numbers.** This round was briefed that the next free number was O76. Round 127 landed while
it was running and took O76 and O77 (both slides-track, and neither touching either seat here), so
**the two rows proposed below were renumbered from O81 up at the coordinator's instruction.** There
is no lost seat in the gap.

**A challenge answered by measurement rather than by argument.** Mid-round the coordinator put the
representation question directly: *"the reference paints a tab as repeated space glyphs in the
current font (`Width() / nCharWidth` of them), not as a rule of the tab's exact width, so a fix that
draws a continuous rule across the tab will match the witness and be wrong wherever the space advance
does not divide the tab width evenly."* That is the right question and it is the one §2 was built to
answer. The `resid` group exists for nothing else: five tab widths whose residue modulo the blank is
80, 20, 100, 100 and 100 twips, where the two readings are **1.00 to 5.00 pt apart**. 26.2.4.2 draws
the tab's **full width** at 5 of 5. The source says why — `bKern = true` routes the call to
`DrawStretchText(aPos, rInf.GetWidth(), …)`, so the blanks are set *onto* the portion's width — and
the count decides one thing only, whether anything is drawn at all, which the `mono` group brackets
to the twip. See §2(A) and §2(B).

**Reading contamination.** No blind reader was available (`CLAUDE.md`, *In this container there is no such reader*), and every page opened in this round was opened knowing what it was being opened for. Nothing below rests on a reading: both seats are scored on drawn operators, and both have an authored fixture measured at the reference with no free parameter.

---

## 0. The headline

| | before | after | reference |
|---|--:|--:|--:|
| **O71** the reference bridges a gap between two of our rules | 10 documents, 1439 pages, **501 248 pt** | 6 documents, 17 pages, **2 460 pt** | — |
| the mirror direction (the base rate) | 7 documents, 24 pages, 2 453 pt | 7, 24, **2 453 pt** | — |
| **O71** authored probe, the two decorations this engine draws | **4 of 111** | **111 of 111** | — |
| **O72** rule cover on the 98 drawn contents pages | 576 547 pt | **110 002 pt** | 82 588 pt |
| **O72** Σ\|ours − reference\| on those pages | 511 497 pt | **48 531 pt** | — |
| **O72** documents within 10 pt of the reference there | 2 of 50 | **39 of 50** | — |
| whole words track, Σ\|ours − reference\| rule cover | 1 482 385 pt | **646 963 pt** | — |
| words renderings that move | — | **49 of 338** (289 byte-identical) | — |
| page counts and alphanumeric counts that move | — | **0 and 0** | — |

Both seats are **fixed in this tree**. Two things are left with a measurement rather than a fix and are seated below: **O81**, the WW8 half of O72, which is 70 % of O72's residue; and **O82**, the residue O71's own census reports, which on its largest document is a table border and not a text rule at all.

---

## 1. O71 — the cheap discriminator first: `IsWordLineMode()` has NIL corpus reach

The brief asked this to be settled before any code, because it decides whether the switch is implemented or only the default. It is settled four ways, one per format, and the answer is nil in all four.

`wordline-census.py`, over the 338 words documents and over 26.2.4.2's own `--convert-to` of them:

| spelling | where | documents | statements |
|---|---|--:|--:|
| `w:u w:val="words"` | 272 corpus DOCX | **0** | **0** |
| `\ulw` (not `\ulwave`) | 338 converted `.rtf` | **0** | **0** |
| `style:text-{underline,overline,line-through}-mode="skip-white-space"` | 338 converted `.odt` | **0** | **0** |
| `sprmCKul` operand 2 | 66 corpus `.doc` | **0** | **0** |

**Base rate, so the nil is informative** (C9): **155 of the 272 DOCX state a non-`none` `w:u`, 2299 statements.** The class is well populated and word-line mode is absent from it — the same shape as O69's double underline on the slides track.

Two instrument notes, both worth keeping:

- **The exporter writes the attribute only where the property is set, and it writes it.** Over all 338 converted `.odt` there are **9** `style:text-*-mode` statements in 2 documents and every one says `continuous` — word mode explicitly **off**. So the zero above is the reference's own answer for every words document and not an attribute nothing exports. (One of the two is `exhibit-06---technical-architecture-template`, which is O71's second witness.)
- **A raw-byte `sprmCKul` scan of a `.doc` is refuted and is kept here so it is not re-derived.** Scanning the `WordDocument` stream for the little-endian opcode `3E 2A` followed by operand `02` reports **3 hits in 2 documents**; neither document appears in the `--convert-to` census, so all three are false positives. A WW8 sprm lives inside a grpprl and a byte pair matches anywhere. The reference's own view of the file is the instrument.

**Consequence, and it is the brief's own: implement the default and not the switch.** The cost of declining is stated in §3 and is 8 authored rows of 171.

**And the third decoration has nothing to draw.** `m_bPaintBlank` is `(underline || overline || strikeout) && !IsWordLineMode()`, so O71 is all three lines — but **no word-processing format in scope can state an overline except ODF**, and over the 338 converted `.odt` there are **0** non-`none` `style:text-overline-style` (9 statements, all `none`). `PageRun` has no overline field and none was added; the probe's 48 overline rows draw nothing here before and after, which is a pre-existing absence this round did not touch and did not create.

---

## 2. O71 — what the reference actually draws across a tab, measured rather than read

Reading `SwTabPortion::Paint` leaves two questions a reader has to answer and the source does not answer on its own.

```cpp
// Tabs should be underlined at once
if( rInf.GetFont()->IsPaintBlank() )
{
    const SwTwips nCharWidth = rInf.GetTextSize(OUString(' ')).Width();
    if( nCharWidth )
    {
        sal_Int32 nChar = Width() / nCharWidth;
        rInf.DrawText(OUString::Concat(RepeatedUChar(' ', nChar)), *this,
                      TextFrameIndex(0), TextFrameIndex(nChar), true);
    }
}
```
`sw/source/core/text/txttab.cxx`:626-641.

**(A) How long is the rule?** *n* blanks at their own advances would stop up to one blank short of the stop. They do not, and the reason is the `true` at the end — `bKern`. `SwTextPaintInfo::DrawText_` turns it into `aDrawInf.SetKern(rPor.Width())` and routes the call to `SwSubFont::DrawStretchText_` (`inftxt.cxx`:798-808), which ends in `rInf.GetOut().DrawStretchText(aPos, rInf.GetWidth(), …)` (`swfont.cxx`:1289-1345). The blanks are **set onto the portion's own width**, so the rule reaches the stop. It is the same `bKern` and the same compression `PageDrawing.Leader` already models for a dot leader.

**(B) What if the tab is narrower than one blank?** `nChar` is then 0, the string is empty, and `DrawText_` returns on `!nLength`. No rule at all.

Both are measured at 26.2.4.2 on an authored `.fodt`, `make-tabprobe.py` → `tabprobe.fodt`, **171 rows**:

| group | rows | what it is |
|---|--:|---|
| `span` | 144 | 4 faces × 3 sizes × 3 decorations × 4 tab stops — does the rule bridge, and how far |
| `skip` | 12 | the same at 10 pt with `style:text-*-mode="skip-white-space"` — the switch §1 says nothing states |
| `mono` | 6 | Liberation Mono at 10 pt, whose blank is **120 twips to the twip**; tab widths 30, 90, 119, 121, 180, 361 |
| `cliff` | 4 | Liberation Sans at 10 pt, whose blank is **55.566 twips**; tab widths 54, 55, 56, 57 — `floor` against `round` |
| `resid` | 5 | Liberation Mono at 10 pt again, tab widths 200, 260, 340, 460, 580 twips — **residues of 80, 20, 100, 100 and 100 twips**, which is the discriminator between the two representations |

**C11 control**: rendered twice into separate user profiles, same UTC day. `ref-tabprobe-a.tsv` and `ref-tabprobe-b.tsv` are **byte-identical**, so nothing below rests on one draw of the reference.

What 26.2.4.2 answers (`ref-tabprobe-a.tsv`):

- **(A) full width, and the `resid` group settles it against the alternative rather than merely agreeing with it.** The two candidate representations are *the blanks are stretched onto the tab's own width* and *`nChar` blanks are set at their own advances, so the rule stops `Width() mod nCharWidth` short of the stop*. They are 1.00 to 5.00 pt apart on the five `resid` rows, against a reader that merges two rules at 0.5 pt, and 26.2.4.2 answers the first at **5 of 5**:

  | tab | blanks (120 twips each) | model A, stretched | model B, at own advance | what 26.2.4.2 draws |
  |--:|--:|--:|--:|--:|
  | 200 twips = 10.00 pt | 1 | 16.00 pt | 12.00 pt | **16.00 pt** |
  | 260 = 13.00 | 2 | 19.00 | 18.00 | **19.00** |
  | 340 = 17.00 | 2 | 23.00 | 18.00 | **23.00** |
  | 460 = 23.00 | 3 | 29.00 | 24.00 | **29.00** |
  | 580 = 29.00 | 4 | 35.00 | 30.00 | **35.00** |

  (Each figure is the whole row's merged cover: the tab plus the 6.00 pt `Z` after it. There is no text before the tab in this group, so the stop position *is* the tab's width and nothing has to be predicted.) All 96 `span` rows carrying a decoration this engine draws come back the same way — **one contiguous rule** from the first glyph to the last, e.g. `L000` `56.70 → 242.00` where the stop is 2.5 in from a text origin of 56.70. Six of them join their neighbour with a **0.20 pt** rounding step, which is why the reader merges at 0.5 pt and both legs go through the same merge.
- **(B) the cliff is exactly one blank.** Mono: 30, 90 and 119 twips draw **two** rules with the tab blank; 121, 180 and 361 draw **one**, contiguous. The step falls between 119 and 121 against a 120-twip blank, with no free parameter.
- **(B′) and the blank is TRUNCATED to the twip, not rounded.** Liberation Sans' 55.566 twips separates the two: a tab of **54** twips draws no rule and one of **55** draws one. Rounding to 56 would refuse the 55. This is the same truncation `Leader`'s remarks record for the fill character, measured a second time on a second call site.
- **(C) the switch is real.** All 12 `skip` rows draw **two** rules with the tab blank, where their `span` twins draw one. So the attribute behaves exactly as `fntcache.cxx`:106-109 says, and §1 says no corpus document states it.

---

## 3. O71 — the fix, and the probe scored against the reference

`PageDrawing.TabRule`, called from `RunsIn` beside `Leader` and before the empty-stretch exit (a tab followed by nothing still paints its blanks). `TabbedSegment` already carried `GapLeft`/`GapWidth`, so the blank the tab advanced over needed no new geometry; the decorated run is `RunAt(segment.Start - 1)`, the font **at the tab**, which is the same position `Leader` reads. The existing `Rules(...)` is reused unchanged, so the tab's rule goes through round 123's `LineSpacing.ResolveRuleWidths` and `MetricGrid.WriterTextLine` chain exactly as the glyphs' own does — the brief's requirement that the rule not be exact everywhere except across a tab.

`compare-tabprobe.py`, ours against 26.2.4.2 row by row, agreeing when the merged cover is within 0.25 pt **and** the bridged/not-bridged verdict matches:

| group | before | after |
|---|--:|--:|
| `span`, underline | 0 of 48 | **48 of 48** |
| `span`, strikethrough | 0 of 48 | **48 of 48** |
| `mono` (the sub-blank cliff) | 3 of 6 | **6 of 6** |
| `cliff` (floor against round) | 1 of 4 | **4 of 4** |
| `resid` (stretched against set) | 0 of 5 | **5 of 5** |
| **implemented decorations** | **4 of 111** | **111 of 111** |
| `span`, overline | 0 of 48 | 0 of 48 — §1, unmodelled here and unmodelled before |
| `skip` (the switch) | 8 of 12 | **0 of 12** — the cost of declining |

**The `skip` rows are the honest half of this table.** Before the change they agreed *by accident*: this tree drew no tab rule at all, which is what word-line mode asks for. Now it draws one, so 8 of the 12 (the four overline rows draw nothing either way) disagree. That is the whole cost of not implementing the switch, and it is 8 authored rows against 0 corpus documents.

### The witnesses

`150-5370-10H.docx`, the header band, pages 100, 300 and 500 alike:

| | rules in the header band | cover |
|---|---|--:|
| before | `(72.00, 117.56)` `(465.28, 540.00)` — two fills | **120.28 pt** |
| **after** | `(72.00, 117.56)` `(117.56, 306.00)` `(306.00, 465.28)` `(465.28, 540.00)` | **468.00 pt** |
| 26.2.4.2 | `(72.00, 117.50)` `(117.55, 305.95)` `(306.00, 465.30)` `(465.30, 540.00)` — four strokes | **468.00 pt** |

Contiguous 72 → 540 on both sides, cover equal to the hundredth of a point, and the two spans recovered are exactly the document's two header tabs. (C16 is answered throughout: this tree fills and the reference strokes, and every figure here counts both shapes and compares horizontal **cover**, never object count.)

`exhibit-06---technical-architecture-template.docx` page 4 is the second family and is also a contents page, so it is scored under §5 with both seats applied.

### Reach

Round 124's own `gap-census.py`, unmodified, run over both of our legs against the same banked
reference — the population is gate r125's 329 `match` words rows, as round 124's was r122's 329:

| | documents | pages | length |
|---|--:|--:|--:|
| **the reference bridges a gap of ours**, before | **10** | **1439** | **501 248 pt** |
| the same, after | **6** | **17** | **2 460 pt** |
| we bridge a gap of the reference (the base rate), before and after | 7 | 24 | 2 453 pt |

The before row reproduces round 124's 10 / 1437 / 501 248 to two pages; the two are a document
whose gate verdict differs between r122's bank and r125's. The mirror direction does not move at
all, which is the check that this adds ink only where the reference has it. The residue is §7.



---

## 4. O72 — round 124's control re-derived first, as the brief asked

`toc-pages.py` is round 124's `toc-page-census.py` re-derived from scratch, with two deliberate changes: the population is the rendered pairs rather than a declaration census (so it cannot inherit that census's `.doc` upper bound, a UTF-16 scan for `" TOC "`), and cover is **merged per y-band** rather than summed (so a rule drawn as several abutting pieces — which is exactly what a bridged tab produces — is not counted several times). A contents page is identified from the **drawn** text: five or more runs of five or more leader dots.

Over the whole words track at the round's base, **50 documents carry a drawn contents page, 98 pages in all** (round 124 found 40 over 60 pages; its population was the 84-row declaration census):

| | on the contents pages |
|---|--:|
| rule cover, ours | **576 547 pt** |
| rule cover, 26.2.4.2 | **82 588 pt** |
| ours more by > 200 pt | **42 of 50** |
| the reference more by > 200 pt | 4 of 50 |
| the reference draws **nothing** while we draw > 200 pt | **16 of 50** |

**And the control reverses, exactly as round 124 reported.** The same test on the *ordinary* pages of the same 50 documents, per-document median:

| | ordinary pages |
|---|--:|
| ours higher | **8 of 50** |
| the reference higher | **18 of 50** |

Round 124 had 5 of 40 against 19 of 40. **The reversal reproduces**, so the seat's argument stands: the excess is a property of contents pages and not of these documents.

---

## 5. O72 — the cause was not the missing rule; it was the paragraph boundary

The suppression round 124's seat asks for **was already in the tree.** `DocxLayoutSource.RunWalker` has carried `_indexResults`, `InIndexField` and `IsIndexField` — with `DomainMapper.cxx`:3037-3047 and `DomainMapper_Impl.cxx`:9355-9360 cited in its own remarks — since before this round. It reached one contents entry per document.

**A `RunWalker` is built per paragraph** (`DocxLayoutSource.cs`, beside `_footnoteNumber` and `_pageBreakPending`), and a `TOC` field's `fldChar begin`, its `w:instrText` and its `separate` all sit in the **first entry's** paragraph while its `end` sits in the **last**. So the counter was reset at every paragraph boundary and suppressed the `Hyperlink` character style on entry one and on nothing else. On `150-5370-10H.docx` that is 1 of 43 contents lines. LibreOffice's own `m_bStartTOC` is on the document-level import state for exactly this reason.

The fix is three pieces, all in `DocxLayoutSource`:

- `_indexFieldDepth`, a document-level counter, seeded into each walker and read back from `RunWalker.IndexFieldDepth` after every paragraph — the same shape as the note counters beside it.
- `_inheritedIndexResults` in the walker, because the field stack *is* per paragraph: the `end` that closes a multi-paragraph field arrives with nothing of that field's on the stack to pop. An unmatched `end` closes the inherited depth instead. The nested `PAGEREF` fields inside each entry open and close within their own paragraph, so an unmatched `end` is the outer field's and no other.
- `Read` clears the depth and `ReadFlow` **saves and restores** it rather than merely clearing it, because a text frame's content comes through `ReadFlow` too and a frame can sit inside a body paragraph.

### After

| | before | after | reference |
|---|--:|--:|--:|
| rule cover on the 98 contents pages | 576 547 pt | **110 002 pt** | 82 588 pt |
| Σ \|ours − reference\| there | 511 497 pt | **48 531 pt** | — |
| documents within 10 pt of the reference there | 2 of 50 | **39 of 50** | — |
| ours more by > 200 pt | 42 of 50 | **3 of 50** | — |
| the reference draws nothing while we draw > 200 pt | 16 of 50 | **1 of 50** | — |

Witnesses, both seats applied:

| page | before | after | 26.2.4.2 |
|---|--:|--:|--:|
| `150-5370-10H.docx` p7 (contents) | 9413.49 pt | **468.00 pt** | **468.00 pt** |
| `exhibit-06---technical-architecture-template.docx` p4 (contents) | 4205.54 pt | **1969.90 pt** | **1969.40 pt** |

`150-5370-10H` p7's 468.00 is O71's header rule and nothing else, which is what round 124 said the reference's four strokes on that page were.

Documents landing on the reference to a point or two include `02_mcar_part-2_and_IS_v2.10` 87 591 → 7488 against 7487, `SPA-02_mcar_part-2_and_IS_v2.9` 120 706 → 9360 against 9359, `OM template for non-complex NCC operators` 54 433 → 8017 against 8017, `AC-150-5370-10G-updated-201604` 17 918 → 1872 against 1872, and thirteen more that go to **0 against 0**.

---

## 6. Reach, confinement and the gate columns

**Our half of the words track rendered twice**, base and after, `SOURCE_DATE_EPOCH` pinned on both, one output directory per document (`sweep-words.py`; 338 of 338 on each leg, 0 failures):

| | |
|---|--:|
| renderings that move | **49 of 338** |
| byte-identical | **289 of 338** |
| page counts that move | **0** |
| alphanumeric counts that move | **0** |

**47 of the 49 are `.docx` and 2 are `.doc`** (`PK_FlugzeugeStricken.doc`, `RobertQ_Service.doc`),
and both `.doc` are O71's: the tab bridging is in `PageDrawing` and reaches all four
word-processing readers, while O72's fix is in `DocxLayoutSource` and reaches one. The five `.doc`
with a drawn contents page are byte-for-byte unchanged in `toc-pages-base.tsv` and
`toc-pages-after.tsv`, which is the control that says so — and is the measurement O81 rests on.

**Slides and sheets cannot move, and that is structural rather than measured.** Both edited files are in `XRay.Office.WordProcessing`, and neither `XRay.Office.Presentations.csproj` nor `XRay.Office.Spreadsheets.csproj` references it — their `ProjectReference` sets are Core, Containers, MsBinary, Ooxml, OpenDocument, Text and Vector. No slides or sheets rendering can differ by a byte, so the other 609 corpus documents were not re-rendered.

**No gate verdict can move**, and the columns confirm it: `gate-columns.py` scored all 49 movers on page count and alphanumeric count against both legs and the reference, and **0 changed either**. A rule is ink; this is the `w:pgBorders` argument again.

**Whole-track rule cover**, ours against the reference over all 338 documents and every page (`rule-cover.py`, merged per band, both shapes):

| | |
|---|--:|
| Σ \|ours − reference\| | **1 482 385 → 646 963 pt** |
| documents better | **41** |
| documents worse | **6** |
| level | **291** |

### The six that worsen are one class, and they were already short

All six were drawing **less** rule than the reference before this round, and removing wrongly-drawn contents underlines made the shortfall visible instead of creating one:

| document | ours − reference, before | after |
|---|--:|--:|
| `33004.docx` | −6 125 | −15 337 |
| `DOA_Template_Form_Type_Certification_Programme.docx` | −16 835 | −22 491 |
| `ABCD-SDE-23-00 - Avionic System Description…docx` | −16 566 | −19 604 |
| `ABCD-WB-08-00 Weight and Balance Report…docx` | −4 913 | −7 642 |
| `system_design__technical_architecture_template.docx` | +1 019 | −2 404 |
| `TE.CAO.00125 Foreign Part 145 approvals - OJT Logbook.docx` | −590 | −1 932 |

Worked through on `33004.docx`, whose whole change is on its one contents page (9680 → 468): 26.2.4.2 draws **two** rules on that page, a header rule at `y = 56.15` running 72 → 540 and a footer rule at `y = 737.85`; this tree draws the footer rule and **no header rule at all**. The missing 468 pt is a third defect that the excess contents underlines had been masking. That is the chart-fit lesson in a different track — *a fix that removes wrong ink makes the ink we never drew visible* — and it is why the statistic is quoted with its sign rather than as a magnitude.

---

## 7. What is left, with the measurement that sizes it

### SEAT O81 — the WW8 half of O72 is unimplemented, and it is 70 % of the residue

`Ww8DocumentReader.Layout` has no index-field state at all, and the rule is not an OOXML one: `sw/source/filter/ww8/ww8par5.cxx`:2334, 3362-3363 and 3653 dress a WW8 index link in the same `Index Link` pool style that `DomainMapper_Impl.cxx`:9358 gives a DOCX one. Of the 5 corpus `.doc` with a drawn contents page, **2 over-draw and neither moved this round**:

| document | contents pages | ours | 26.2.4.2 |
|---|--:|--:|--:|
| `150_5335_5a.doc` | 3 | **29 251 pt** | **0 pt** |
| `361400CSLegislation1RF01PUBLIC1.doc` | 1 | **4 960 pt** | 146 pt |
| *control* `A320SimNotes.doc` | 2 | 1 698 pt | 1 698 pt |
| *control* `absrc-pac-01-info-note-en.doc` | 1 | 1 911 pt | 2 458 pt |
| *control* `150_5300_13_chg10.doc` | 3 | 0 pt | 38 pt |

The two are **34 065 pt of O72's remaining 48 531**, and `150_5335_5a.doc` is the cleanest witness the corpus offers: the reference draws **nothing** on three contents pages where this tree draws 29 251 pt. Base rate: 5 of the 66 `.doc` carry a drawn contents page, and 3 of the 5 already agree.

### SEAT O82 — O71's own residue is dominated by a rule that is not a text rule

After the fix the gap census still reports the reference bridging a gap of ours on **6 documents, 17 pages, 2 460 pt** — against a mirror direction (the base rate) of 7 documents, 24 pages, **2 453 pt**, unchanged by this round. So the residue is at its own null in total. It is not evenly spread: `review-welsh-government-communications-mister-peter-mandelson.docx` is **1 511 pt over 8 pages**, and on its page 10 the "gap" is a **table row's top border**: 26.2.4.2 strokes `60.15 → 535.25` in one piece and this tree draws `60.08 → 129.02` and `296.92 → 535.22`, with the row's two cells (`iShare`, `These are all similar`) either side of the hole. That is `PageDrawing.DrawBorders`/`WithWordJoins`, not a decoration.

**Two things follow.** The seat itself: a table row border drawn in pieces where the reference draws it whole, 1 511 pt over 8 pages of 1 document, one document characterised and the other five (949 pt in all) not. And an **instrument caveat for round 124's own figure**: `gap-census.py` cannot tell a text rule from a table border, so some part of its 501 248 pt was never O71's. The before/after bounds that part at ≤ 2 460 pt, or 0.5 %.

### Named and not seated

- **`t_TEMPforInvProgs.docx`**, 6 683 pt against 2 808 on three contents pages after the fix (12 894 before). The only `.docx` still over-drawing there by more than 200 pt.
- **`SwGluePortion::Paint`** (`porglue.cxx`:68-75) and **`SwHolePortion::Paint`** (`porrst.cxx`:298) paint blanks under the identical `IsPaintBlank()` test — justification glue and the trailing blank that hangs outside a line. Neither is modelled. No witness was isolated: the gap census's residue is §7's border on the one document it characterises.
- **`IsWordLineMode()`**, §1 and §3: nil in all four spellings, 8 authored rows of 166 the cost of declining.
- **Overline**, §1: 0 of 338 in the reference's own view, and three of the four formats cannot state one.

---

## 8. Tests

| file | cases | failing at the base |
|---|--:|--:|
| `UnderlineTests` (4 new cases in 3 methods) | 6 | **4** |
| `DocxIndexFieldTests` (new) | 3 | **1** |

`dotnet test --filter` over the seven at the base — with `TabRule`'s call site disabled and the walker seeded with 0 — reports **5 failed of 9**. The four that pass at the base are the negative arms and the controls, and each is pinned by its own mutation rather than left to look like coverage:

- `ATabNarrowerThanOneBlankCarriesNoRule(119, false)` and `ThePlaceTheDecorationStartsDecidesWhetherTheTabIsBridged(5, 1)` are the *no rule* halves of theories whose other half fails at the base.
- `AFieldThatIsNotAnIndexKeepsItsResultsCharacterStyle` is O72's control: the same three paragraphs under a `HYPERLINK` field keep the style, so the suppression is keyed on the field's **name** and not on a hyperlink.
- `ARunAfterTheFieldsEndKeepsItsCharacterStyle` pins the unmatched-`end` decrement, and **it fails when that branch is removed** — checked by mutation. Its first cut passed the mutation and was wrong: `ShouldAllBe` over an empty `Runs` list passes vacuously, and a paragraph whose formatting has just been stripped collapses to a uniform one with no runs at all. Each entry now carries a second, italic run so the list can never be empty, and the assertion counts underlined runs rather than quantifying over them.

**The full run**, projects individually (`CLAUDE.md`: total them yourself, a drop with zero failures is a truncated run):

| project | passed | failed |
|---|--:|--:|
| `XRay.Office.Core.Tests` | 591 | 0 |
| `XRay.Office.Containers.Tests` | 109 | 0 |
| `XRay.Office.Markup.Tests` | 259 | 0 |
| `XRay.Office.OpenDocument.Tests` | 169 | 0 |
| `XRay.Office.Rendering.Tests` | 164 | 0 |
| `XRay.Office.Text.Tests` | 744 | 0 |
| `XRay.Office.Vector.Tests` | 309 | 0 |
| `XRay.Office.WordProcessing.Tests` | 1974 | 0 |
| `XRay.Office.Presentations.Tests` | 1203 | 0 |
| `XRay.Office.Spreadsheets.Tests` | 1387 | 0 |
| `XRay.Office.Fidelity.Tests` | 542 | **10** |

**One run was discarded, and it was caught by the check rather than by anything looking wrong.**
The first run of `XRay.Office.WordProcessing.Tests` reported `Failed: 0, Passed: 1408, Total: 1408`
against **1974 discovered** — 566 tests silently dropped under a summary line that reads as a clean
pass. That is `CLAUDE.md`'s *A truncated run reports success*, and the figure above is two
subsequent runs of the project alone, both **1974 of 1974**. Every project's count in this table was
checked against its own `dotnet test --list-tests`; the listing collapses some theory rows and is
therefore a lower bound (Markup runs 259 against 249 listed), so a run *below* the listing is the
signal and above it is normal.

**0 skipped in every project**, which is what says none of them covered nothing. The ten fidelity
failures are the standing set O64's own register row records — *"0 fidelity verdicts (10 of 552
failed before and the same ten after)"* — and the names are banked in `fidelity-failures.txt`.
`dotnet build XRay.Office.slnx`: **0 warnings, 0 errors**. The full table is `test-run.txt`.

---

## 9. Refutations of this round's own findings, kept rather than deleted

- **"The fix does not fire."** The first render of `150-5370-10H.docx` after `TabRule` landed was identical to the base, and the first instrumented run printed nothing at all. Neither was true: `dotnet build src/XRay.Office.WordProcessing/…` does not copy the library into `tools/XRay.Office.Cli/bin`, so both measurements were of the **base binary under a new name**. This is `CLAUDE.md`'s stale-binary trap arriving through the build graph rather than through a file timestamp, and the guard is the same — re-render one document and byte-compare it against the run you are claiming to have reproduced. Done at the end of this round: the restored source's rendering of `150-5370-10H.docx` is **md5-identical** to the sweep's own copy.
- **The tab probe's first two readings were the instrument.** A y-tolerance of `0.9 × line height` merges adjacent lines' rules at 10 pt, which shifted the whole Mono group by one row and made the cliff read as 30/90 rather than 119/121. And a `cliff` group marked with a trailing `R` collided with every `span` row's own `R`, because PyMuPDF splits a line at a wide gap and reports the two halves as separate lines. Both were found by an arithmetic contradiction — a reported stop that was the *previous* row's — and not by the numbers looking wrong.
- **"3 of 66 `.doc` state word underline."** §1: a raw-byte sprm scan, refuted by the reference's own view. Kept in `wordline-census.py` with the operand corrected, because the correction (operand **2**, not 4, `ww8par6.cxx` `Read_Underline`) is worth as much as the census.
- **"The six documents that worsen are over-drawing."** §6: all six are *under*-drawing and were before this round. The magnitude `|ours − reference|` says the opposite of the sign, and the sign is the finding.

---

## 10. The scripts and the banked data

| | |
|---|---|
| `wordline-census.py` | §1, the four spellings of word-line mode, with the `w:u` base rate |
| `make-tabprobe.py`, `tabprobe.fodt`, `tabprobe-manifest.tsv` | §2, the 166-row authored probe |
| `read-tabprobe.py` | one rendering → per-row rule cover, both shapes, merged at 0.5 pt |
| `ref-tabprobe-a.tsv`, `ref-tabprobe-b.tsv` | 26.2.4.2 twice, separate profiles — the C11 control, identical |
| `ours-tabprobe-base.tsv`, `ours-tabprobe.tsv` | our two legs of the same probe |
| `compare-tabprobe.py`, `tabprobe-compare-base.tsv`, `tabprobe-compare.tsv` | §3, row by row, 4 of 106 → 106 of 106 |
| `show-rules.py` | every rule on one page, both shapes — the witness instrument |
| `cover.py` | merged rule cover per page |
| `sweep-words.py` | our half of the 338 words documents, one directory per document |
| `movers.py`, `movers.tsv` | §6, which renderings moved, byte for byte |
| `gate-columns.py`, `gate-columns.tsv` | §6, page and alphanumeric counts for every mover |
| `rule-cover.py`, `rule-cover-base.tsv`, `rule-cover-after.tsv` | §6, whole-document rule cover against the reference |
| `toc-pages.py`, `toc-pages-base.tsv`, `toc-pages-after.tsv` | §4 and §5, contents pages **and** the ordinary-page control |
| `gap-base.tsv`, `gap-after.tsv` | §7, round 124's own `gap-census.py` re-run on both legs |
| `test-run.txt` | §8.1 |


> **Renumbered at merge.** This round's two seats were written as O78 and O79 and are
> **O81 and O82** in the register. Round 128 finished first and took O78/O79 while this round
> was still running, and O80 is round 128's own closed row. The numbers here have been updated
> so the register and this file agree; nothing else changed.
