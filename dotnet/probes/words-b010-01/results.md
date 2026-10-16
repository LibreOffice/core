# words-b010-01 — a positioned table in a running head is a frame, and the head's height says so

Round brief: `words/batch-010/docx/5709.16 ch.40_mgfinal.docx`, 31 pages against 32, words 9790/9821
(inside the 2%+3 band), verdict `pages`. One page short.

`prediction.md` was committed at `80f6e0fddec`, before any code was changed and before any sweep.

**It is a real pagination defect**, not either of the two ways the brief warned the measurement might
lie, and the whole of it is 11.00 pt of running-head height. The document renders 32 of 32 and
`match` now, `words/batch-010` is 9 of 9, and `words/batch-001`…`010` is 98 of 99 — the one
remaining being `1447.doc` at 3/4, which is the line-height residue and not this round's.

---

## 1. Which kind of round this was

**Not a non-deterministic reference.** The same installed `soffice` 26.2.4.2 converted the document
five times in the same configuration in the same session:

```
run 1: 32 pages, 10034 words   run 2: 32/10034   run 3: 32/10034   run 4: 32/10034   run 5: 32/10034
md5(pdftotext) = e2e27c575116 on all five
```

The banked reference agrees at 32. So the `fse_identification_form.xlsx` shape is ruled out by
measurement rather than by assumption, which is what `TODO.raster-ceiling.md` asks for.

**Not the `1447.doc` line-height residue.** That residue is a per-line deficit that accumulates
until one line stops fitting. This one does not accumulate at all. Every body line on every page sat
**exactly 11.00 pt** higher in ours than in the reference — 123.81 against 134.81 on pages 3 and 4,
123.69 against 134.69 on page 2, and so on to page 31 — while the line *pitch* inside the body was
identical on both sides to 0.00 pt. One step, at the boundary between the running head and the body,
repeated unchanged on all 31 pages.

Finding it took the technique the brief names: **look for the first vertical divergence, not for the
page that differs.** Pages 1 and 2 diverge in position from line 7 onward and nowhere before it, and
line 6 — the last line of the running head — is at 94.20 against 94.25, which is the whole document's
constant 0.05 pt offset and not a difference at all.

## 2. What the header is, and what LibreOffice does with it

`word/header2.xml` is a **positioned (floating) table** —

```xml
<w:tblpPr w:leftFromText="187" w:rightFromText="187" w:bottomFromText="403"
          w:vertAnchor="text" w:tblpXSpec="center" w:tblpY="1"/>
```

— followed by one empty `FSMHeader` paragraph, which resolves through `FSM Header Right` to `Header`,
Arial 8 pt, a 9.20 pt line.

`soffice --headless --convert-to fodt` says what LibreOffice made of it, which is faster and more
reliable than inferring it: the whole header is a single `text:p` holding a `draw:frame` around a
`draw:text-box` around the table, and the frame's `fr1` style carries

```xml
fo:margin-bottom="0.2799in"   <!-- 403 twips: w:bottomFromText -->
fo:margin-top="0in" style:vertical-pos="from-top" style:vertical-rel="paragraph"
```

### The law, measured on the binary

Seven perturbations of that flat XML, each re-rendered by the installed 26.2.4.2. The figure is the
`yMin` of "Table of Contents", the first body line of page 2, and the page count is the whole
document's:

| variant | body top | pages |
|---|---:|---:|
| unchanged | 134.69 | 32 |
| frame `fo:margin-bottom` 0.2799in → **0** | **114.54** | **31** |
| frame `fo:margin-bottom` 0.2799in → 1in | 186.54 | 35 |
| frame `fo:margin-top` 0 → 1in | 134.69 | 32 |
| anchor paragraph 8 pt → 20 pt, still empty | 134.69 | 32 |
| anchor paragraph given text, 8 pt | 134.69 | 32 |
| anchor paragraph given text at 60 pt (wraps to two lines, 138 pt) | 174.39 | 34 / 35 |
| table row min height 0.691in → 1.5in (frame grows 47.46) | 182.09 | 35 |
| header `fo:min-height` 0.5in → 1.5in | 144.39 | 33 |

Read off:

* the body top moves **one for one** with the frame's lower spacing — 0 → 114.54, 403 twips →
  134.69, 1 in → 186.54, i.e. exactly +20.15 and +72.00;
* it does **not** move with the frame's *upper* spacing, because the frame is positioned from the
  paragraph's top at a fixed `svg:y`;
* it does not move with the anchor paragraph at 20 pt, and does move once the paragraph is two 69 pt
  lines — so the two are a `max`, not a sum. The empty paragraph has to be *given text* to reach
  that: raising an empty one's size only ever buys it one taller line, which is why the 20 pt row
  moves nothing. That row's page count came out 34 on one run and 35 on another with the body top
  identical at 174.39 both times, on a deliberately grotesque synthetic variant; the body top is
  what carries the law and the page count is downstream of it;
* text put into that anchor paragraph draws at `yMin` **36.26**, the very top of a header whose own
  top is 36.06 — **overlapping** the frame, not pushed below it. In a header `SwTextFly` does not
  wrap the anchor's text around the fly.

> **A positioned table in a running head is a frame, and its anchor paragraph does not move out of
> its way. The head's height is `max(in-flow content height, frame bottom + the frame's lower
> spacing)`.**

The row that matters most is the second: **setting the frame's lower spacing to nought makes
LibreOffice's own rendering 31 pages** — our page count, produced by our defect, reproduced in the
reference by removing the one property we were not reading.

### What we did

We stacked them, and `w:bottomFromText` was read nowhere in the tree — `grep bottomFromText src/`
was empty, and `DocxLayoutSource.Tables.cs` said outright that only the horizontal half of
`w:tblpPr` was honoured. So our head was

```
table 78.10 + empty paragraph 9.20 = 87.30      against Writer's max(9.20, 78.10 + 20.15) = 98.25
```

10.95 pt short, which is the 11.00 pt step on the page.

## 3. The change

Four files, and the boundary is deliberate.

* `PageTable.IsPositioned` and `PageTable.LowerSpacing` — the flag is carried separately from
  `HorizontalPosition`, which is null for a positioned table stating no `w:tblpXSpec`; two of the
  corpus's four say nothing but `w:tblpX`.
* `DocxLayoutSource.Tables.cs` reads both from `w:tblpPr`.
* `FlowLayouter.LayOut` gains `floatsPositionedTables`. When it is on, a positioned table is placed
  where the flow has reached and leaves it there: it adds nothing to `top`, contributes
  `top + height + LowerSpacing` to a separate reach, and does not count as content the flow has
  passed — so the paragraph beside it is still the frame's first and still drops its leading. The
  flow's `Advance`, and a bottom-aligned flow's shift, take `max` of the two.
* `PageFurnitureSet.Resolve` turns it on, which is the only place it is on.

**The body is untouched on purpose.** There `bWrapAllowed` is true — `sw/source/core/text/txtfly.cxx`,
the `!IsInFootnote() && !bFooterHeader` arm — so the anchor's text goes *below* the fly, which is
what in-flow stacking already approximates, and no measurement was taken there. The corpus census
that decided the boundary: **21 documents hold a positioned table in the body, 4 in a header or
foot.** The larger half is the one left alone.

## 4. Reach

`sweep.sh` from `words-pages-01` — the gate's three checks against the **banked** 26.2.4.2
references, rendering only our half, with `SOURCE_DATE_EPOCH` set. Whole words track, 200 documents,
before and after:

| | baseline | after |
|---|---:|---:|
| match | 158 | **159** |
| page-exact | 166 | **167** |
| total absolute page error | 113 | **110** |
| renderings changed | — | **4** |

The four renderings that changed, byte for byte, are **exactly** the four documents with a positioned
table in a header or a footer. The other 196 are byte-identical.

| document | before | after |
|---|---|---|
| `batch-010/…/5709.16 ch.40_mgfinal.docx` | 31/32 `pages` | **32/32 `match`** |
| `batch-019/…/CRIF - Spécification technique….docx` | 33/29 `pages,words` | 27/29 `pages,words` |
| `batch-004/…/PAT-047 - Architecture and Detailed Design Assessment.docx` | 4/4 `match` | 4/4 `match` |
| `batch-018/…/HC-Bulletin-template.docx` | 5/5 `match` | 5/5 `match` |

`CRIF` halves its page error, 4 → 2, and stays a mismatch; its reference head is about 68 pt taller
than ours for a reason this does not reach.

`PAT-047` is the honest cost. Its page count and verdict are unchanged, but its page-1 body moves
from 5.15 pt below the reference's to 6.35 pt above it: its header table is `vertAnchor="page"` with
no `w:bottomFromText`, so the head loses the empty paragraph's height and gains nothing, and the
`svg:y="0.5047in"` that LibreOffice's importer gives that frame is the vertical half we still do not
read. Marginally further on one document, out of four moved.

### Batch validation, in the order the project requires

`batch-check.sh`, which re-renders the reference, not the banked half:

```
batch-check.sh … 'words/batch-010'       TOTAL 9    MATCH 9   MISMATCH 0
batch-check.sh … 'words/batch-0[01][0-9]' TOTAL 188  MATCH 158 MISMATCH 30
    of which batches 001-010:            TOTAL 99   MATCH 98
```

The glob reached batch-019 rather than batch-010 and was left to run rather than narrowed, so
batches 011-019 are re-proven too. Batches 001-010 are **98 of 99**; the one failure is
`words/batch-004/doc/1447.doc` at 3/4, which is the line-height residue.

## 5. Tests

`tests/XRay.Office.WordProcessing.Tests/FurniturePositionedTableTests.cs`, eight tests, and the split
was verified by putting the defect back rather than asserted:

| tree | fails |
|---|---:|
| `FlowLayouter` + `PageFurnitureSet` reverted, reader kept | **3 of 8** |
| those and the reader reverted | **5 of 8** |
| as committed | 0 of 8 |

The three that never fail are the controls — an ordinary table in a head still stacks, the body still
stacks a positioned one, and an ordinary table is still not positioned. Reverting `PageTable`'s two
new properties as well does not compile, which is why the two rows above are the measurement.

Every project run individually, counts read rather than colours:

| project | passed | failed | skipped | total |
|---|---:|---:|---:|---:|
| XRay.Office.Core.Tests | 332 | 0 | 0 | 332 |
| XRay.Office.Containers.Tests | 109 | 0 | 0 | 109 |
| XRay.Office.Markup.Tests | 259 | 0 | 0 | 259 |
| XRay.Office.OpenDocument.Tests | 125 | 0 | 0 | 125 |
| XRay.Office.Presentations.Tests | 679 | 0 | 0 | 679 |
| XRay.Office.Rendering.Tests | 150 | 0 | 1 | 151 |
| XRay.Office.Spreadsheets.Tests | 762 | 0 | 0 | 762 |
| XRay.Office.Text.Tests | 349 | 0 | 0 | 349 |
| XRay.Office.Vector.Tests | 295 | 0 | 0 | 295 |
| XRay.Office.WordProcessing.Tests | 827 | 0 | 0 | 827 |
| XRay.Office.Fidelity.Tests | 520 | **30** | 0 | 550 |

`XRay.Office.Fidelity.Tests` was **30 of 550 before the change and 30 after**, established on the
unmodified tree before anything was edited. `XRay.Office.WordProcessing.Tests` is 819 + the 8 added
here. The build is warning-free.

**One run of `XRay.Office.Vector.Tests` reported 1 failed of 295, and nine subsequent runs reported 0
of 295.** It was seen once, inside a loop running every project back to back, and the test's name was
not captured. Nothing in this branch reaches `XRay.Office.Vector`. Recorded rather than explained.

## 6. Looking at the page — and what the blind reviewer found that the numbers cannot

`pair.sh "5709.16 ch.40_mgfinal__docx" --worst` on the **fixed** tree chose page 30, `|ink| 11.46%`,
and the image went to a fresh subagent with no numbers, no brief, and no access to anything but the
one file.

It confirmed the fix from the other direction and then found something else.

**Confirmed.** "Line breaking within every shared paragraph … wrap at exactly the same words on both
sides … same word count per line, same ragged-right shape", "text column width equal within
measurement error", "header box: same top and bottom y, same width … same two centered bold title
lines with the same line split", "`of 32` matches". The running head — the thing this round changed —
is now pixel-close on both sides, which is what the 0.05 pt residual says from the other side.

**Found, and it is a second defect this round did not fix.** The reviewer reported, unprompted, that
"our page 30 begins *later in the document flow* than the reference's": ours opens at list item 3
where the reference opens at heading `48.21`, roughly eight lines ahead, and it named "different
keep-with-next / widow-orphan handling on the 48.21 heading" as one of six candidate causes and gave
the measurement that would separate it.

That candidate is the right one, and the control is committed:

* The first page whose *content* diverges is **page 8**. Our page 7 ends with the heading
  `40.44 – Aviation Safety Inspectors, Avionics` at `yMin` 675.24; the reference moves that heading
  to page 8 and ends page 7 one paragraph earlier.
* The heading's style is `FSMHeading3`, based on `Heading3`, which carries `<w:keepNext/>` —
  `fo:keep-with-next="always"` in the flat ODF.
* **Delete `fo:keep-with-next` from the flat ODF and LibreOffice's page 7 ends with
  `40.44 – Aviation Safety Inspectors, Avionics`, page 8 begins with `Refer to section 41.2…`, and
  the document is still 32 pages** — character for character what we produce. So the divergence *is*
  keep-with-next and nothing else.

The seat is named and not chased. `Paginator`'s keep-with-next test is
`!FirstLineFits(next, …)` (`Paginator.cs`:1067, and `FirstLineFits` at :1422), which asks whether the
successor's **first** line fits. Here it does — there is room for one more 13.8 pt line — and the
successor moves anyway, because orphan control wants two. The paragraph goes to the next page and the
heading is left behind. Writer's `SwFlowFrame::IsKeep` asks whether the successor can actually *start*
there, which is a stronger question.

It is left for a round of its own because the change touches every document with a `keepNext` heading
— which is most of the corpus — and needs its own prediction and its own reach measurement. It costs
this document nothing on the gate: it is 32/32 and `match` either way.

**One more thing the reviewer saw that is real and is not ours to fix here.** Our head draws
`FSH_5709.16_40_DD_1_0` where the reference draws `5709.16 ch.40_mgfinal.docx`. The header's field is
`FILENAME \* MERGEFORMAT`; we render the cached result Word stored, LibreOffice re-evaluates it
against the file it is converting. Both are defensible and they are not the same string, so the text
layers differ on every page of every document with a `FILENAME` field. Named as a lead.

## 7. Scoring the prediction

| # | prediction | outcome |
|---|---|---|
| P1 | 31 → 32 pages, `pages` → `match` | **right** |
| P2 | exactly 4 renderings change, and which four | **right** — byte-diff over all 200, the named four and nothing else |
| P3 | match 158 → 159 | **right** |
| P4 | page-exact 166 → 167, page error 113 → 112 | **half right** — page-exact 167, error 110 rather than 112; `CRIF` gave back two more than expected |
| P5 | `PAT-047` stays `match` | **right** on the verdict, and it names the cost the verdict hides: its body moved from 5.15 pt below the reference to 6.35 pt above |
| P6 | `HC-Bulletin-template` stays `match` | **right** |
| P7 | `CRIF` stays a mismatch | **right**, and its page error halves |
| P8 | Fidelity 30/550 before and after | **right** |
| P9 | batch-010 9/9, batches 001-010 98/99 with `1447.doc` the remainder | **right** |
| P10 | the blind reading will locate the difference in where the page breaks, not in the running head | **right, and it found more than was predicted** — it also located a keep-with-next divergence this round had not looked for, which §6 then confirmed against the binary |

Nine right, one half right, and the one thing no prediction contained came from the page that was
looked at rather than from any column of the gate.

## 8. Leads

* **Keep-with-next must ask whether the successor can start, not whether its first line fits.**
  §6 above, with the flat-ODF control. `Paginator.cs`:1067/:1422. A round of its own.
* **The vertical half of `w:tblpPr` is still unread**, and now that a positioned table in a head is a
  frame it is the next thing that document class needs: `PAT-047`'s frame has `svg:y="0.5047in"`,
  which is the whole of its remaining 6.35 pt.
* **`FILENAME` fields are not re-evaluated.** We draw Word's cached result; LibreOffice draws the
  name of the file it converted.
* **The dot-leader fill stops one dot short**, leaving a gap that `pdftotext` reads as a word break:
  our TOC line is `…........ 22` and the reference's is `…........22`, which is 19 extra tokens on
  page 2 of this document alone and costs nothing but the word column.
