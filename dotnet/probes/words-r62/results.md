# words-r62 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
corpus `/c/sandbox/workdir/sample-files`; worktree `wt-words-r50` on branch `wt-words-r62`, base
`337bc9fe17c`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; every sweep's `TMPDIR` on the host mount at
`/c/sandbox/workdir/scratch-r62-words/tmp`, and `/` never rose above 71 %. One prediction file,
`prediction.md`, committed at `0bed989686d` **before** the first behavioural commit `71e03336520`.

## Baseline, reproduced exactly

`batch-check.sh … 'words/*' … 8` → `TOTAL 355 MATCH 338 MISMATCH 17 REF-CANNOT-RENDER 0`, scored
against `MANIFEST.tsv`'s own 337-path list rather than that total — the extra 18 rows are the
case-insensitive mount's alias entries. **321 of 337, zero disagreements with the manifest's status
column, document for document.** The scorer refuses to print unless every manifest path found a row.

## Result

**321 → 323 of 337. `012_Project_Timeline_Template_Black_and_Brown_Theme` and
`015_Project_Timeline_Template_Colored_Background` both close. Zero regressions.**

| | base | after change 1 | after change 2 |
|---|---:|---:|---:|
| words verdicts | 321 | 321 | **323** |
| our renderings whose bytes changed (cumulative) | — | 4 | **2** |
| verdicts gained | — | 2 | **2** |
| verdicts **lost** | — | **2** | **0** |
| extractable words changed | — | 0 | **0** |
| font lists changed | — | 0 | **0** |
| reference halves differing between sweeps | — | — | **0 of 337** |

The last row is a control worth keeping: over the round's three full sweeps of 355 paths the
reference's page counts, word counts and font lists are **identical every time**. The reference PDFs
do differ *byte for byte* between sweeps and that is not a signal — the whole difference on
`097_Business_Case_Template_Elegant_Layout` is 98 bytes in two XMP `dc:date` elements, with the file
length unchanged. Compare page and word counts, not bytes, on the reference half.

Per document, before → after, and there is nothing to net:

| document | pages before | pages after | reference | words |
|---|---:|---:|---:|---|
| `012_Project_Timeline_Template_Black_and_Brown_Theme_35c76550.docx` | 1 | **2** | 2 | 49/49, unchanged |
| `015_Project_Timeline_Template_Colored_Background_6434b0e8.docx` | 1 | **2** | 2 | 50/50, unchanged |

Exactly **two** of the 337 renderings changed at all, and no word count anywhere moved. The
prediction's own falsification test — *"if any word count moves, the change did something other than
split an empty row off the bottom of a page"* — passes.

## 1. A fly-held table splits, and the deadline is not the body's bottom

26.2.4.2 marks **every** DOCX floating table's frame splittable without exception —
`DomainMapperTableHandler.cxx`:1765, *"A text frame created for floating tables is always allowed to
split"* — so the question is never whether it splits but **where the deadline is**.
`PlaceFloatedTable` placed such a table whole or not at all, and its own remarks said so.

The reference's geometry on `012`, read out of its content stream rather than inferred: eight of the
nine rows on page 1, the first at `y = 128.10` from the sheet's top — the 72 pt margin plus the
56.1 pt its `w:tblpY="1122"` states — and the ninth as `12.40 489.65 99.95 50.35 re f*` on page 2, a
top edge at **72.00**, the top margin exactly. So the continuation's offset from the next page's
text area is nought and `w:tblpY` is applied once.

### The first cut was right and cost two verdicts, which the sweep caught

Splitting at the body's bottom closed `012` and `015` **and broke
`080_Printable_Graph_Paper_Template_Black_Theme` and
`089_Printable_Graph_Paper_Template_Simlpe_Format`**, both 2 pages against the reference's 1. Net
movement zero. The prediction had said *"Regressions predicted: 0"* and that is **falsified**; it had
also written down in advance the blind spot that produced them — *"a row that sizes to its content …
is under-counted. This is the largest blind spot"* — and both documents are exactly that: the census
computed 27.00 pt and 16.20 pt of **slack** from their declared `w:trHeight` values, and their
laid-out rows overflow the body by 10.5 pt.

### `GetFlyAnchorBottom`, and the two conditions that must both hold

`sw/source/core/layout/fly.cxx`:114. The ordinary answer is the body's print bottom — Writer's own
comment, *"Word >= 2013 style: the fly has to stay inside the body frame"*. The other one is
*"Word <= 2010 style: the fly can overlap with the bottom margin / footer area in case the fly height
fits the body height and the fly bottom fits the page"*, and `isLegacyBehavior` (:104) chooses
between them from **two** conditions ANDed together: the document's `TAB_OVER_MARGIN` flag, which is
`compatibilityMode` 14 or less, **and** a fly positioned against the page frame.

`012` and `015` state `compatibilityMode` **15**; `080` states **14** and anchors
`w:vertAnchor="page"`. Six renderings against 26.2.4.2, one variable each:

| arm | change from the file as found | pages |
|---|---|---:|
| `080` as found — mode 14, `vertAnchor="page"`, 691 pt table 17.3 pt below a 697.9 pt body | — | **1** |
| `080` pushed to `w:tblpY="2886"` | reaches y = 835 on an 841.9 pt sheet, 6.5 pt from the paper's edge | **1** |
| `080` raised to `compatibilityMode` **15** | mode only | **2** |
| `080` anchor changed to `text` at the same position | anchor only | **2** |
| `012` as found — mode 15, `text` anchor | — | **2** |
| `012` dropped to `compatibilityMode` **14** | mode only | **2** |
| `012` given **both** mode 14 and `vertAnchor="page"` | both | **1** |
| `080` with every row height doubled — 1382 pt against a 697.9 pt print area | the height term | **2** |

**Each condition alone is insufficient and the pair is sufficient**, in both directions, on both
documents. The height term is the third: `nFlyHeight <= nPageHeight` failing is what brings the split
back to a Word 2010 file.

Nine renderings of `080` at rising `w:tblpY` and twelve of `012` at falling `w:tblpY` fix the
non-legacy threshold exactly where the body's bottom is: `012` fits at `w:tblpY="222"` (11.1 pt) and
splits at `"322"` (16.1 pt), against a 454.1 pt table in a 468 pt body, so the boundary is
`top + height ≤ 468`.

### Refuted on the way

**`w:vertAnchor` alone is not the discriminator.** The first hypothesis was that a page-anchored fly
is simply not bounded by the body. Four renderings of `012` — as found, `vertAnchor="page"` at the
same absolute position, `vertAnchor="margin"`, and an explicit `vertAnchor="text"` — are
**byte-identical in geometry**, all four two pages, the table's first fill at `12.40 433.05 …` in
every one. Refuted by one experiment before any code was written.

**Nor is it the row-height rule, and nor is it the flow.** `012` with every `w:hRule="exact"`
stripped: still two pages. `080` with `w:hRule="exact"` added to all 48 rows: still one. `012` with
its whole body flow after the table replaced by a single empty paragraph: still two.

### After

`012`'s and `015`'s page 2 now exist; `080`, `089`, `ESPN-R - MCF - RA - Ed1.docx` (58 pages) and
`part-147_approval list_20230119.docx` (2) are all back at the reference's own page counts.
**`015`'s page 2 is exact**: five white rules at
`12.05 539.75 → 191.75`, `12.05 510.95 → 191.75`, `12.30 510.70 → 540.00`, `129.20 511.20 → 539.50`
and `191.50 511.20 → 539.50` — the same five coordinates the reference draws, to 0.00 pt.

**`012`'s page 2 draws nothing at all on our side**, and that is honest rather than hidden: the row
is placed, and everything the reference puts on that page is a table-style conditional fill and a
conditional border, neither of which this reader draws — see §3. On `012` the page count moved and
**not one drawn operator did**; page 1 holds the same 61 records before and after.

### What was deliberately not changed

`PlaceFloatedTable`'s other guard — *a table taller than a whole column stays in the flow* — is
kept. It is not what Writer does, and Writer's own height term is measured against the **page's**
print area rather than the body's, so even the threshold differs whenever a running head takes room.
Two corpus documents are in that class and **both pass the gate today**:
`words/pagination-001/docx/ESPN-R - MCF - RA - Ed1.docx` (123 rows, 1476 pt of declared height
against a 481.90 pt body) and `words/done-005/docx/part-147_approval list_20230119.docx` (782 pt
against 714.30). Trading them blind for nothing was refused; the divergence is pinned by a test that
says in its own remarks that it pins a known-wrong.

## 2. The census, and what it did and did not see

`floattable-census.py`, over the manifest's 271 `.docx`:

```
documents holding a positioned table :   40 of 271 .docx
positioned tables                    :   46
vertAnchor                           : {'text': 29, 'page': 17}
tables whose declared rows overflow  :    4 in 4 documents
```

35 of the 40 match at the base. Predicted **2 to 6** changed renderings and **+2** verdicts;
measured **2** and **+2**. The band was for exactly the blind spot that later fired — content-sized
rows — and the two documents it hid were regressions rather than gains, which the band's wording did
not distinguish. **A changed-rendering band should be stated per direction, not as a magnitude.**

## 3. `012`'s missing fills: the rule is in the file, and it is not `w:shd`

The vision reading and the operator dump agree and the source names it. `012` page 1: the reference
draws **75 fill operations and 10 strokes**; we draw **19 and 2**. The document contains **twelve**
`w:shd` elements in total, all `fill="000000"` on the header row — which is exactly what we draw.
Everything else comes from `w:tblStyle w:val="PlainTable5"` under
`w:tblLook … firstRow="1" firstColumn="1" noHBand="0" noVBand="1"`:

| the reference's page-1 ink | source |
|---|---|
| 8 × `#FFFFFF` in column one | `<w:tblStylePr w:type="firstCol"><w:tcPr><w:shd … w:fill="FFFFFF"/>` |
| 48 × `#F2F2F2` on table rows 2, 4, 6 and 8 | `<w:tblStylePr w:type="band1Horz">`, `w:tblStyleRowBandSize="1"` |
| 12 × `#000000` on row 1 | the document's own `w:shd`, which we already draw |
| 7 bar fills | shapes, which we already draw |
| 1 × `#7F7F7F` rule under row 1 | `firstRow`'s `w:tcBorders/w:bottom` |
| 7 bar outlines | the shapes' `a:ln`, which we draw for none of them |

`WordTableStyleConditions` already resolves `w:tblLook` and hands the layer names back most-specific
first. Its own remarks say why the rest is missing and both reasons are now answerable: *"the band
layers are deliberately absent … Implementing the bands here would be reach that cannot be measured,
so they are left for a round that has a document to measure them on"*, and *"Only the run half of a
layer is applied … A `w:tblStylePr` may also carry `w:pPr`, `w:tcPr` and `w:tblPr`."* **`012` is that
document**, and the missing half is the `w:tcPr` half. Not implemented this round: it moves no gate
column, it needs its own census, and the round's verdicts were already in hand.

## 4. The `COL_AUTO` rule is established, and it reconciles both witnesses — but not round 59's

`autocolour.py`, four arms, two variables, each moved in both directions, on the corpus document
itself with one substitution per arm. Colour read with `textcolour.py`, which reports the fill colour
in force at every text-showing operator.

| arm | the anchor cell | the box's own fill | the title at y = 561.70 | white / black shows |
|---|---|---|---|---|
| as found | black | `<a:noFill/>` | **#FFFFFF** | 23 / 12 |
| `p` | **white** | `<a:noFill/>` | **#000000** | 3 / 32 |
| `s` | black | **white** | **#000000** | 20 / 15 |
| `t` | **white** | **black** | **#FFFFFF** | 6 / 29 |

**The shape's own fill wins when it has one; when it has none the walk continues to the anchor's
background.** `012`'s title box is a `wps` shape with `<a:noFill/>` holding a run that states no
`w:color`, and it is anchored **inside a table cell** — as are all twelve of the document's text
boxes — and that cell carries `w:fill="000000"`. That is `SwFrame::GetBackgroundBrush`
(`sw/source/core/layout/paintfrm.cxx`:8059) reached from `SwFntObj::SetDevFont`'s `bChgFntColor`
branch (`sw/source/core/txtnode/fntcache.cxx`:2369-2437) with `bConsiderTextBox=true`: the fly's
paired shape's fill attributes are used when `isUsed()`, and the walk continues otherwise.

**A first cut of this probe was non-discriminating and would have read as a confirmation.** Arm `s`
was originally filled `#00B050`, which is *dark* by `Color::IsDark`'s WCAG rule — luminance 79.9
against a threshold of 87 — so "the anchor decides" and "the shape decides" predict the same answer
and the arm separates nothing. It came back white and looked like evidence. `#FFFFFF` and `#000000`
are what make the quadruple a discriminator.

**Nothing is implemented on it, because it disagrees with round 59's counter-witness.** That witness
is a shape filled `#0070C0` — WCAG luminance 15.2, dark by the same rule — whose text the reference
draws **black**, where the rule above predicts white. Either those shapes are not Writer text boxes
at all (a DrawingML shape's text is drawn by editeng, where `SdrObject::getBackgroundFillSet` walks
shape → page → master page and never sees an anchor) or there is a further term. **The next round
re-measures `docs-quality-MA.IMS.00001-Integrated-Management-System-manual.docx` page 9 and
`069_Work_Breakdown_Structure_Template_Professional_Format` before either direction ships.** Round
59's removal stands until then; its own evidence was and is correct.

## 5. The vision reading

Three blind readings, each handed one composed image and nothing else, each forbidden from reading
any other file or running any command, each asked to describe the halves separately before comparing
and to give the direction.

### `080_Printable_Graph_Paper_Template_Black_Theme` page 1, **after** both changes — the control

Chosen because it is a page the round's *second* change is predicted to restore to exactly what the
base drew, having been broken by its first. Not `--worst`.

The reader: *"I found **no content, layout, or geometry difference** between the two halves … Grid
top edge, bottom edge, left inset and right inset are the same in both … Plainly: I see no defect on
our side in this pair."* It reported the grid bounding box as 912 px on our side against 915 on the
reference and explicitly declined to defend a 3 px reading off a downsampled render.

**Second instrument, confirmed and sharper than the reading.** Both sides draw **86 strokes**; the
table's top rule is at `y = 752.10` on ours against `752.59` on the reference and its bottom at
`61.75` against `61.49`, a 0.26–0.49 pt difference which at 150 dpi is under a pixel — inside what
the reader said it could not resolve. The control did its job: the restoration is complete.

### `015_Project_Timeline_Template_Colored_Background` page 2 — the page the change creates

Chosen because it is the round's own item: a page that did not exist before this round.

The reader reported **both halves blank and identical** and refused to invent a difference: *"apart
from the labelling chrome, the two halves look the same to me — both empty … I am not going to
manufacture one."*

**That is correct, and it is a lesson about the instrument rather than about the page.** The row the
reference puts there is drawn as **five white rules on white paper**. The paired composite cannot see
it at any resolution, and neither can `pdftotext`: both text layers are empty. The operator dump can,
and it reports the five strokes at coordinates identical to the reference's to 0.00 pt. **A blind
reading returning "identical" on a page that is genuinely invisible is a true negative, not a failed
reading** — and it is the reason the falsification test for this change was written against the
reference's own `re` and `S` operators rather than against a picture.

The reader's one geometric note — that our half's banner is ~19 px wider — is composition chrome; it
said so itself and it is right.

### `012_Project_Timeline_Template_Black_and_Brown_Theme` page 1 — the second reader on round 61's page

Chosen because round 61's reader found three things there, of which one was checked by a second
instrument and two were not. This is a **fresh** reader with no knowledge of that reading.

It reproduced round 61's first finding independently and in the same direction: *"The reference does
not render the title 'Project Timeline Template'; ours does … Note the direction here is unusual:
**ours draws more text than the reference**, not less."* Confirmed by `textcolour.py`: the reference
issues 23 white shows to 12 black and we issue 25 black to 14 white, and the title's two shows sit at
`y = 561.70` and `505.80` on both sides — **white on the reference, black on ours**, same coordinates.
§4 above then explains why.

It also reported, unprompted, the two of round 61's findings that had **no second instrument**:

* *"The reference draws alternating grey row bands on odd task rows; ours draws none."* — confirmed:
  48 `#F2F2F2` fills on the reference's table rows 2, 4, 6, 8 against none on ours.
* *"The reference outlines bars and rounds their corners; ours does not."* — confirmed: the reference
  strokes seven black rectangles at exactly the seven bar coordinates and we stroke none of them.
* *"The reference left-aligns in-bar labels where ours centres them (~90–95 px shift on 6 of 7
  rows)"* — confirmed at operator level: bar one runs `122.85 → 362.30` and its label starts at
  `145.80` on the reference against `205.31` on ours, 59.5 pt to the right.

Three claims, three second instruments, three confirmations. The one thing the reader could not
settle — whether ours has a hairline bar stroke lost to downscaling — is settled by the dump: we draw
**two** strokes on that page and both are elsewhere.

## 6. The 24.2.7.2 audit — one site, VERIFIED

`XRay.Office.WordProcessing/Ooxml/WordStyles.cs` `HasDefaultParagraphPropertiesElement`: *"LibreOffice
hangs a document-wide default on exactly that presence … `applyDefaults(true)` runs only from the
`w:pPrDefault` arm of `StyleSheetTable::sprm`"*, measured on 24.2.7.2.

`probes/words-r46/widow-orphan-default.py` re-run **unchanged**, 9 variants × 5 straddle positions =
45 renderings on 26.2.4.2. `para-off` measures the room at the foot of page one, so a variant putting
fewer lines there has the control on:

| variant | control on at fillers |
|---|---|
| `no-pPrDefault` | — |
| `no-docDefaults` | — |
| **`empty-pPrDefault`** | **14, 16** |
| **`pPrDefault-with-pPr`** | **14, 16** |
| `pPrDefault-widow-off` | — |
| `pPrDefault-para-off` | — |
| `para-on` (the positive control) | 14, 16 |
| `para-off` (the room control) | — |
| `settings-on` | — |

**VERIFIED.** The discriminating pair is `no-pPrDefault` against `empty-pPrDefault`: at 14 and 16
fillers the first puts 3 and 1 lines on page one and the second puts 2 and 0. A bare
`<w:pPrDefault/>` with no `w:pPr` inside it turns widow/orphan control on. Three arms say it is a
default and not an override — a `w:pPrDefault` *stating* `w:widowControl w:val="0"` is off, a
paragraph stating it is off, and `w:docDefaults` removed entirely is off. And `settings-on` is
**inert at every filler count**, which reconfirms HANDOVER §7's refutation of the document-level
`w:settings/w:widowControl` on the current binary.

Counters re-derived at both commits with the file's own commands, never quoted:

| | base `337bc9fe17c` | this tree |
|---|---:|---:|
| open sites | 37 | **37** |
| marker lines | 31 | **32** |
| VERIFIED / FIXED / WRONG / UNDECIDED | 26 / 4 / 1 / 0 | **27 / 4 / 1 / 0** |

## Refutations, collected

1. **`w:vertAnchor` does not decide whether a fly splits.** Four one-variable renderings of `012` —
   as found, `page`, `margin` and an explicit `text` at the same absolute position — are
   geometrically identical and all four take two pages.
2. **Nor does the row-height rule.** `012` with every `w:hRule="exact"` stripped still splits; `080`
   with `w:hRule="exact"` added to all 48 rows still does not.
3. **Nor does the flow after the table.** `012` with its whole trailing body replaced by one empty
   paragraph still splits.
4. **A fly's deadline is not the body's bottom in a Word 2010 file**, and each of the two conditions
   alone is insufficient — measured in both directions on both documents, eight renderings.
5. **The prediction's "0 regressions" was wrong** and the intermediate sweep is what said so: the
   first cut gained 2 and lost 2 for a net of nothing. The cause was named in the prediction as the
   census's largest blind spot before the sweep ran.
6. **`012`'s missing fills are not `w:shd` at all.** The document holds twelve `w:shd` elements and
   we draw twelve fills from them; the other 56 come from `w:tblStylePr` conditional layers that this
   reader reads for `w:rPr` and not for `w:tcPr`.
7. **A `COL_AUTO` run in a `noFill` text box resolves against its *anchor's* background, not its
   shape's** — four arms, both variables inverted. And the rule **contradicts round 59's measured
   counter-witness**, so neither direction ships until that document is re-measured.
8. **A discriminating arm that is not discriminating reads exactly like a confirmation.** The first
   `s` arm used `#00B050`, dark by `Color::IsDark`, under which both hypotheses predict white.
9. **The reference's PDFs differ byte for byte between sweeps and it means nothing** — 98 bytes of
   XMP `dc:date`, file length unchanged. Its page counts, word counts and font lists are identical
   across all three of this round's sweeps, for all 337 paths.

## Tests

```
Core 390   Containers 109   Text 624   Vector 302   Rendering 153(1 skipped)   Markup 259
OpenDocument 125   WordProcessing 1231   Spreadsheets 1020   Presentations 872     = 5085
0 failed, 1 skipped
```

Re-derived project by project rather than quoted. The words delta is
`XRay.Office.WordProcessing` **1225 → 1231, +6**, the six `FloatedTableDeadlineTests`; `Core` 376 → 390
and `Presentations` 846 → 872 came in with the round-60/61 merge and are not this round's.
`dotnet build -v q -nologo`: **0 warnings, 0 errors.**

**`XRay.Office.Vector` reported 21 failures of 302 in the batch run and 0 of 302 alone**, on an
unmodified project this diff cannot reach. That is the phantom `CLAUDE.md` documents; re-running
alone is what identifies it, and it is recorded rather than quietly dropped.

Through `verify-test.sh`, tree clean before each and restored after — **five mutations, all five
detected, and the attribution is the point**:

| mutation | detected by |
|---|---|
| the legacy branch removed — every fly stops at the body | `AWordTwentyTenPageAnchoredFlyHangsIntoTheBottomMargin` **alone** |
| the compatibility-mode half of `isLegacyBehavior` ignored | the three mode-15 tests; **the anchor control correctly passes** |
| the page-anchor half of `isLegacyBehavior` ignored | `TheAnchorAloneDoesNotGrantTheOverlap` **alone** |
| the continuation placed at `w:tblpY` again instead of at the top | `TheContinuationStartsAtTheTopOfTheNextPagesTextArea` **alone** |
| the split switched off entirely | four of the six |

**The fourth is the one worth reading.** Placing the continuation at `w:tblpY` a second time produces
the *same page count*, so every page-count assertion passes over a row drawn 100 pt too low. Only the
baseline assertion sees it. That is round 61's lesson — *assert the baseline, not only the
consequence* — applied before the fact rather than after, and the run confirms it fires alone.

The binary was re-rendered after the last `verify-test.sh` and `012` came back **byte-identical** to
the sweep's own rendering, 25 847 bytes, 0 differing — so the double rebuild left the shipped state.

## Shared layers

**None.** `git diff 337bc9fe17c..HEAD --name-only` over `dotnet/src` is three files, all under
`XRay.Office.WordProcessing`: `Layout/Paginator.cs`, `Ooxml/DocxReader.cs` and `Ooxml/WordStyles.cs`
(a comment only). Nothing in `Core`, `Containers`, `Text`, `Vector`, `Rendering`, `Markup` or
`Ooxml` was touched, and the same command over those six trees prints nothing. Slides and sheets
cannot move **by construction**; that is a falsifiable claim for the parent's sweep.

## Files

- `prediction.md` — committed at `0bed989686d`, before `71e03336520`.
- `floattable-census.py` — positioned tables resolved through `w:tblpPr` and `w:vertAnchor`, with its
  five blind spots in the docstring.
- `textcolour.py` — every text-showing operator with the fill colour in force, which `pdf-ops.py`
  deliberately does not report. `q`/`Q` save and restore it and the text matrix is reset at `BT`; the
  first cut did neither and reported y values in the thousands on a 612 pt page.
- `autocolour.py` — the four-arm `COL_AUTO` quadruple, with the non-discriminating first cut recorded
  in its docstring.
- `dotnet/tests/XRay.Office.WordProcessing.Tests/FloatedTableDeadlineTests.cs` — six tests: the split,
  the continuation's own position, the legacy overlap, its two single-condition controls, and the
  guard that was kept, which says in its remarks that it pins a known divergence.

## What the next round does first

1. **`012`'s 56 missing fills and 8 missing strokes** — `w:tblStylePr`'s `w:tcPr` half and the band
   layers. The rule is fully read out in §3, `WordTableStyleConditions` already resolves the layer
   names and the look, and its own remark asks for exactly the document this round found. It needs a
   census over the corpus's `w:tblStylePr` styles and it moves no gate column.
2. **Round 59's counter-witness, re-measured** — `docs-quality-MA.IMS.00001-…docx` page 9 and
   `069_Work_Breakdown_Structure_Template_Professional_Format`. §4 establishes the rule and predicts
   white where round 59 measured black; find out whether those shapes are Writer text boxes at all
   before either direction ships.
3. **The tall-table guard**, with the two documents it protects named and passing:
   `ESPN-R - MCF - RA - Ed1.docx` and `part-147_approval list_20230119.docx`. Writer floats and
   splits those too, and its height term is the **page's** print area, not the body's.
4. **`097`'s remaining 1.65 pt**, in the height of a body paragraph holding an inline image —
   untouched this round. The leading taken against it is right; the line is not.
5. Then the `.doc` label slant at `Ww8DocumentReader.Describe` — still 80 of the 81 remaining
   OpenSymbol glyphs — and the Carlito-versus-serif class.
