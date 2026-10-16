# An `a:hlinkClick` is a field too — and so is a `.ppt`'s and an `.xlsx` cell's, and a `.docx`'s is not

Round 82, on the original corpus's `.pptx` column. The ODF round established that a hyperlink in
Impress text is an EditEngine *field* and closed it for `text:a`; this round establishes the same
for DrawingML, wires it, and settles the three adjacent formats the brief named — two of them
positively and one of them not at all.

## Environment

| | |
|---|---|
| base commit | `e6be864e4`, worktree `/home/user/wt-pptxfield`, branch `agent/pptxfield` |
| reference | `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2** `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| `/usr/bin/soffice` | 24.2.7.2 — **not used for anything in this round** |
| corpus | `/home/user/sample-files`, the original 947; the `.pptx` column is 251 documents |
| base binary | `/home/user/libreoffice-core/…/XRay.Office.Cli`, the main checkout at this round's own base commit, built 05:35 and **not rebuilt** — a whole-corpus gate was running from it throughout |
| fonts | all five tarball confounds aside, as they have been since 2026-09-07 |
| workers | 2 for every sweep, as briefed — with two exceptions recorded below |

**No build in this worktree overlapped a sweep in it**, checked the way the rulebook requires —
the newest render's mtime against the binary's, not `pgrep` and not a clock. The first reach sweep
ran against a CLI and its dependency assemblies all stamped **06:02** or earlier and finished
before the rebuild at **06:29**; the second ran entirely after it, and the only source edits since
are doc comments.

**Two departures from "2 workers", both stated rather than elided.** The fourteen `g0`…`g13`
reference renders were launched in parallel in one command, which was careless and took about a
minute; and the thirty reference renders of the mover sample ran at one worker beside the
second sweep's two. Neither can bias a measurement — a contended reference render either produces
the same PDF or times out visibly — but the box was carrying the parent's whole-corpus gate and
another agent's sweep throughout, and that is why the second sweep took an hour.

## The rule, verified at the seat

Every citation below was opened and read in this round rather than carried from the brief.

**A DrawingML hyperlink run is a field.** `TextRun::insertAt` branches on
`if( maTextCharacterProperties.maHyperlinkPropertyMap.empty() )`
(`oox/source/drawingml/textrun.cxx`:88). With the map empty the run's text is inserted as text;
with anything in it the run becomes
`xFactory->createInstance("com.sun.star.text.TextField.URL")` (`:149`) with the run's own text as
its `Representation` (`:157`), and the same branch imposes the theme's `hlink` colour unless the
map already carries one (`:162-166`) and an underline unless the run states one (`:167-168`).
**The brief's seat, `textrun.cxx:149`, is right**; what it does not say is that the *test* is on
line 88 and is not the element's presence — see below.

**A field is one portion, broken at cells.** `ImpEditEngine::CreateLines`' `EE_FEATURE_FIELD`
branch (`editeng/source/editeng/impedit3.cxx`:1101-1206) measures the field's own text, and when
it is wider than the room left walks it with
`xBreakIterator->nextCharacters(…, CharacterIteratorMode::SKIPCELL, …)`, pushing an entry onto
`ExtraPortionInfo::lineBreaksList` wherever the next cell would overflow. A cell is a grapheme
cluster, so on Latin text that is an opportunity at every character. **The list always begins with
0** — *"always add 1st line break (safe, we already know we are larger than nXWidth)"* — and the
one case that moves the whole field down is `bFieldStartNextLine`, when even the first cell does
not fit **and the line already has content** (`:1148-1149` for the entry and `:1173-1180` for the
branch, the test being `nLineStartX < 0` where `nLineStartX = -nTmpWidth`).

**The spill is a paint-time rule.** `aTmpPos += MoveToNextLine(aStartPos, nMaxAscent, nColumn)`
(`:3793`), and `MoveToNextLine` takes `aStartPos` by reference (`:3273-3279`), so each spill line
advances the running pen by the *ascent* and everything after it moves down with it — while the
formatter never sees a spill line at all, because the whole field is one portion of one `EditLine`.

**A `text:a` is the same thing through the other importer**, `xmloff/source/text/txtparai.cxx`
:1352-1374: `XMLImpHyperlinkContext_Impl` when the cursor has a `HyperLinkURL` property and
`XMLUrlFieldImportContext` when it has not.

## Measured at the reference, on one-attribute variants

`make-pptx-variants.py` writes decks that differ in exactly one thing: a 14 × 5 cm box at
(1 cm, 1 cm), zero insets, `a:noAutofit`, 16 pt Liberation Sans, one paragraph. The ordinary pitch
there is 1.2 em = **19.190 pt** and a field's spill is 1.0 em = **15.987**. All figures are points
from the page top, read out of 26.2.4.2's own PDF by `baselines.py`.

| variant | what 26.2.4.2 draws |
|---|---|
| `v1` the URL as plain text | 44.306 / 63.496 / 82.687, every pitch 19.190, breaking after `organisational-` and `planning-` |
| `v2` the same URL as one hyperlink run | 44.306 / **60.293 / 76.280**, the two spills 15.987 apart, breaking inside `development` |
| `v3` the link starting part way along a line | the field stays on the line it starts and spills at 15.987 |
| `v4` `v2` middle-anchored | first baseline **105.591** — one line of 19.190 centred in the box |
| `v5` `v1` middle-anchored | first baseline 86.400 — three lines of 19.190 centred |
| `v6` `v2` bottom-anchored | first baseline 166.847, the two spills below the box's bottom edge |
| `v7` a link short enough to fit | one line at 44.306; nothing changes at all |
| `v8` the same link split across **two** `a:r` | **identical to `v2`, span for span** |
| `v9` plain text after the field | the tail is drawn on the last spill line's own baseline, at x = 106.413 |
| `v10` `<a:hlinkClick r:id=""/>` and nothing else | **black, not underlined, and word-broken exactly as `v1`** |
| `v11` the same plus `tooltip="t"` | blue and cell-broken exactly as `v2` |
| `v12` the field in a centred paragraph | drawn off the left edge at x = −5.961 with characters missing |
| `v13` a paragraph after a field paragraph | `AFTER` at **95.471** = 76.280 + 19.190 |
| `v14` `v13` as plain text | `AFTER` at 101.877 = 82.687 + 19.190 |
| `v15` a **short** link mid-paragraph, the line boundary inside it | line 1 ends `https://exa`, line 2 begins `mple.org/abcdefgh` at 15.987 and then continues in plain text |
| `v16` `v15` as plain text | breaks at the spaces, three lines of 19.190 |
| `g19`…`g26` a link glued behind a `(`, the prefix lengthened one character at a time | at `g23` the `(` fits and the link's first character does not: 26.2.4.2 leaves the `(` on line 1 and puts the **whole** link on line 2 at the ordinary pitch |

`v4` against `v5` is the block-height half on its own: same characters, same box, and the first
baseline differs by 19.19 pt because one paragraph is one `EditLine` and the other is three.

`v8` is the OOXML-specific question and its answer is *no difference*: **one field per `a:r`**,
and two adjacent fields fill exactly as one does. `v15` is the one that explains this round's
reach — a hyperlink run of *any* length moves the break whenever the line's boundary falls
inside it, which is far wider than "a long URL".

`g23` is `bFieldStartNextLine` caught in the act, and it is why `CellBreaks.Merge` now also offers
the stretch's own start: with a `(` immediately in front of the link there is no break opportunity
of the language's own there, and without that entry the line breaks in front of the `(` instead.

## The adjacent formats

Each was decided by an authored probe rendered through 26.2.4.2, not by reading alone. All four
`.docx` and both `.xlsx` are in `make-docx-xlsx-variants.py`; the text column is 14 cm in every
one of them, so they are comparable with the deck variants above.

### `.docx` — **refuted, and in the form the brief thought might survive**

| probe | 26.2.4.2 |
|---|---|
| `w1` the URL as plain text in a paragraph | 71.751 / 90.201 / 108.651, breaking after `organisational-` |
| `w2` the same inside a `w:hyperlink` | **identical, span for span** |
| `w3` the URL as plain text inside a `wps` text box | identical |
| `w4` the same inside a `w:hyperlink` inside that text box | **identical** |

So a Writer hyperlink is a character property and not a field, exactly as `txtparai.cxx`:1359
predicts — and **the boundary is not "a shape's text is EditEngine"**, which was the brief's
reason for expecting the text-box row to differ. A `wps:txbx` holds `w:txbxContent`, which
writerfilter imports as a Writer *fly*'s text; the cursor over it has `HyperLinkURL` like any
other Writer cursor. The DrawingML path a `.docx` *does* reach is a chart's or a diagram's text,
and the corpus states **zero** `a:r/a:rPr/a:hlinkClick` in any `word/` part of any of its 272
`.docx`.

### `.xlsx` — **a field, and this tree already models it**

| probe | 26.2.4.2 |
|---|---|
| `x1` a wrapping cell holding the URL | 80.930 / 92.127, pitch 11.197, breaking at the solidus |
| `x2` the same cell with a `<hyperlink ref="A1"/>` | 80.930 / **90.029**, pitch **9.099**, breaking inside `leadership`, and drawn `#000080` |

`WorksheetGlobals::insertHyperlink` (`sc/source/filter/oox/worksheethelper.cxx`:1062-1080)
replaces a `CELLTYPE_STRING`/`CELLTYPE_EDIT` cell's content with one `SvxURLField` whose
representation is the whole cell string, so the cell is an `EditTextObject` holding a single
field and the same `CreateLines` branch applies — cell-broken, and spilled at the ascent.
`SheetLayout.HyperlinkRanges`/`HoldsField` and `SheetTextLayout`'s `#000080` are already that
rule; what they carry is the *painted* consequence — Calc turns clipping on for a wrapping cell
that holds a field (`readCellContent`, `sc/source/ui/view/output2.cxx`:2551-2567, *"Fields aren't
wrapped, so clipping is enabled"*) and measures the row from one line — rather than the cell
break. **Left as it stands**: the two models differ only in which character straddles the clip
boundary, and the sheets track is not this round's.

### `.ppt` — **a field, and this tree does not read the hyperlink at all**

`SdrPowerPointImport` builds a real `SvxFieldItem(SvxURLField(…), EE_FEATURE_FIELD)` for a
`PPT_PST_TxInteractiveInfoAtom`'s text range (`filter/source/msfilter/svdfppt.cxx`:6936), splits
it across the `PPTCharPropSet`s the range covers (`:7069`, `:7090`) and forces the underline bit
on each (`:7054-7056`) — so the legacy path is the same rule as DrawingML's, reached through
`m_aHyperList` instead of a relationship. **X-Ray.Office's `.ppt` reader has no
`InteractiveInfo`/`ExHyperlink` handling of any kind** — `git grep` finds none of those record
names under `dotnet/src` — so a `.ppt` hyperlink is currently neither a field, nor blue, nor
underlined. Reach, counted by scanning every `.ppt` for a record header of type 4063 — a byte-pattern
scan rather than a record walk, so it is an upper bound and is stated as one:
**17 of the corpus's 51 `.ppt`, 91 atoms**, against 23 documents carrying any `InteractiveInfo`
at all. Left with its seat; it is a reader feature and not a
line, which is why it is not in this round.

## The change

Three files. The first two are read only by the deck reader; the third is shared with the ODF
path, which is why its confinement is measured rather than argued.

- **`XRay.Office.Ooxml/DrawingML/DrawingHyperlink.cs`** (new) — `MakesField` answers the question
  `textrun.cxx`:88 asks, from the element's own attributes, and `StatesItsOwnColour` the one
  `:162` asks. A stated `r:id` is *taken* to resolve, because the layout reader carries no
  relationship table; the approximation is exact on the corpus, where every non-empty `r:id` on a
  text run resolves and the only element that leaves the map empty states `r:id=""` and nothing
  else.
- **`XRay.Office.Presentations/Ooxml/PptxTextBody.cs`** — the run is built with
  `IsField: DrawingHyperlink.MakesField(hyperlink)`, and the hyperlink colour and the automatic
  underline are gated on the same answer instead of on the element's presence.
- **`XRay.Office.Text/Layout/CellBreaks.cs`** — `Merge` also offers the stretch's own start, which
  is `lineBreaksList`'s unconditional first entry and is what carries a whole field down when even
  its first cell does not fit. This is shared with the ODF path, which is why its confinement is
  measured below rather than argued.

Everything else — `CellBrokenSpan`, `ParagraphLayouter`, `TextMeasurer`, `SlideTextLayout`'s
`Fields`/`ContinuesField`/`PlacedLine.ContinuesField` and the block-height rule — is the ODF
round's and is untouched. That is the whole point of the flag being on `SlideTextRun`: the second
importer costs one expression.

**`a:fld` was deliberately not made a field, and the reason is a count.** `TextField::insertAt`
does create a real field for a recognised type (`oox/source/drawingml/textfield.cxx`:63-148,
:177-200), so the same rules apply to a slide number and a date. On the corpus's slides there are
**1266 `slidenum` in 47 decks and 56 `datetime1` in 3**, and the longest cached text of any of them
is **10 characters** — nothing that can overflow a placeholder, so the flag would move nothing and
could only introduce risk.

## What moved — the probes

**26 of the 28 variants agree with 26.2.4.2 span for span** after the change, on text, colour and
origin, at a tolerance of 0.7 pt (`variant-agreement.txt`, produced by `compare.py`). Most agree to
**0.029 pt**, which is the constant 1/100 mm the two stacks differ by everywhere; the four that
reach 0.4–0.7 pt do so on the *x* of a span deep inside one text object, which is the PDF's own
truncated-width channel and not a layout difference.

The two that do not agree are both known and neither is the field rule:

- **`v12`, an over-long field in a centred paragraph.** The `EditLine`'s width is the whole
  field's, so `ImpAdjustLine` starts the line left of the box and the reference draws two fragments
  at x = −5.96 and −4.04 with characters missing altogether. This tree aligns each spill line on
  its own and produces a readable paragraph. **Deliberately not reproduced**, exactly as the ODF
  round left it.
- **`g25`/`h25`, a line-measure edge at 1.4 pt.** 26.2.4.2 fits a 52-character line that this tree
  breaks one word earlier. `h25` holds **no hyperlink at all** and differs identically, so it is
  pre-existing and outside this round; it is recorded because it sits in the middle of the `g`
  series and would otherwise read as a field defect.

**The committed fixture agrees with 26.2.4.2 on all 17 of its spans**, worst origin difference
0.029 pt: `dotnet/tests/corpus/features/slide-hyperlink-field.pptx`, six slides, rendered by both
and compared by `compare.py`.

## What moved — the corpus

`reach.sh` renders one document with the base binary and with this one, `SOURCE_DATE_EPOCH` fixed
on both legs, and compares the two PDFs byte for byte. The base binary is the main checkout's, at
this round's own base commit, and was not rebuilt at any point.

### Confinement is exact

Over the whole `.pptx` column — **251 documents, every one rendered both ways** —

| | documents |
|---|---:|
| byte-identical | 134 |
| differ | **117** |

and the split is exactly the census's: **every one of the 117 movers carries a text-run
`a:hlinkClick`**, all 113 decks that carry none are byte-identical, and 21 of the 138 decks that do
carry one are byte-identical as well — a link that fits and that no line boundary falls inside
changes nothing, which is the control the rule needs. `reach-stage1.tsv`.

### A byte difference is not always a difference on the page

`001_1-2-3_Horizontal_Hierarchy_39795b4d.pptx` is a mover whose *visible* rendering does not move
at all: `pdftotext` gives identical text for both binaries and every span PyMuPDF reports is at the
same origin. What changed is an 11 pt run in the page's content stream that both readers clip away
— a template's instruction box parked outside the slide, whose link is re-broken exactly as the
rule says and whose ink never reaches paper. `visible.py` is the discriminator, and the split
between the byte reach and the visible one is reported below rather than elided.

This matters for reading the reach figure, because **89 of the 117 movers are `chartset-*`
template decks** and all of them carry the same `presentationgo.com` link on their masters.

### The movers move towards the reference, measured against it

The gate cannot score this — a re-broken URL has the same alphanumeric characters — so the
measurement is the drawn spans. `score.py` renders the document through 26.2.4.2 and counts how
many of the reference's spans each of our two binaries reproduces: same page, same text, and an
origin within 0.30 pt, which is above the constant 0.028 the two stacks differ by everywhere and
well below a line.

A sample of **30 of the 117 movers**, drawn with a fixed seed and stratified 12 template decks to
18 real ones so that the `chartset-*` majority does not swamp it:

| | base `e6be864e4` | this round |
|---|---:|---:|
| reference spans reproduced | **8216** | **8379** |
| of the reference's | 10380 | 10380 |
| documents better | — | **21** |
| documents worse | — | **0** |
| documents level | — | 9 |

**Twenty-one better, none worse.** The largest movers are
`section_1_our_rights_presentation.pptx` (+35 spans), `BHCA Part II webinar #2 - 10.2020.pptx`
(+24), `schematicplay.pptx` (+20), `WiGr_2021W_1_…` and `solog_orientation_august_2019.pptx`
(+12 each). **Eight of the nine level ones do not change on the page at all** — they are the
`chartset-*` templates whose only field is in an off-slide instruction box — and the ninth,
`NCW-2024-Guide-.pptx`, changes on the page without changing its distance from the reference.
`mover-scores.tsv`.

### The stretch-start entry changes no corpus document

The reach sweep was run twice: once with the field flag alone, and once with the whole change. The
second run over the **138 decks that carry a text-run `a:hlinkClick`** — the only population either
half can reach — reproduces the first **row for row**: the same 117 movers and the same 21
byte-identical decks. So `CellBreaks.Merge`'s new entry is correctness with **no corpus witness**;
`g23` is its only measured case, and it is authored. Recorded because a change with no witness is
exactly the kind that is later removed as dead.

## The tracks this round did not target do not move

The diff reaches three files. Two are read only by the deck reader
(`DrawingHyperlink`, `PptxTextBody`); the third, `CellBreaks`, is shared — but `cellBroken` is
passed by exactly one caller in the tree, `SlideTextLayout`, and it is non-empty only for a
paragraph holding a run with `SlideTextRun.IsField`. Two readers set that flag: `OdfTextBody` and,
now, `PptxTextBody`. So the populations that can move are the `.pptx` column and the `.odp` one,
and nothing else can — which is argued here and measured below rather than left as an argument.

| column | rendered both ways | differ |
|---|---:|---:|
| `.pptx`, all of it | 251 | 117 |
| `.pptx` carrying no text-run `a:hlinkClick` | 113 | **0** |
| **`.odp` of the converted corpus, every document holding a `text:a`** | **156** | **0** |
| `.ppt` | 11 | **0** |
| `.docx` | 11 | **0** |
| `.doc` | 5 | **0** |
| `.xlsx` | 10 | **0** |
| `.xls` | 5 | **0** |

**The `.odp` column is the important row and it is a complete population, not a sample**: all 156
of the converted corpus's `.odp` that hold a `text:a` — the only ones `CellBreaks` can reach —
render byte-identically with the base binary and with this one. So the shared change is inert on
the track it is shared with, which is what the ODF gate's 295 of 302 needed to survive untouched.

The five other original tracks are a **sample** and are stated as one: every fifth `.ppt`, every
twenty-seventh `.docx`, every twenty-fourth `.xlsx`, every thirteenth `.doc` and `.xls` — 42
documents, **42 byte-identical**. What makes a sample enough there is that it is a sample of a
population the diff cannot reach at all: no reader outside `OdfTextBody` and `PptxTextBody` sets
`SlideTextRun.IsField`, and `cellBroken` has one caller.
`reach-stage2.tsv`, `reach-other-tracks.tsv`.

## The suite

Every project run individually, at this round's head.

| project | base `e6be864e4` | this round |
|---|---:|---:|
| Containers | 109 | 109 |
| Core | 516 | 516 |
| Markup | 259 | 259 |
| OpenDocument | 143 | 143 |
| Presentations | 1005 | **1027** |
| Rendering | 164 | 164 |
| Spreadsheets | 1198 | 1198 |
| Text | 727 | **728** |
| Vector | 302 | 302 |
| WordProcessing | 1771 | 1771 |
| **total** | **6194** | **6217** |

0 failed and 0 skipped everywhere; the twenty-three new ones are `PptxHyperlinkFieldTests` (7),
`DrawingHyperlinkTests` (15) and `CellBreakTests.TheStretchesOwnStartIsAnOpportunity` (1).

### `XRay.Office.Fidelity.Tests`

**542 passed / 10 failed of 552, 0 skipped** — the briefed baseline exactly. The run was captured
as its last thirty lines, so of the ten only the last two names are in the transcript,
`TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop` (`list-label-overrun.odt`, 0.1287
against a 0.1 pt tolerance) and
`SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt` (`sheet-rich-text.xlsx`,
95.074 against 94.904); both are in the briefed set of ten and the count is the briefed count.


## What this brief got wrong

1. **"2325 `hlinkClick` occurrences in 95 of the 251 decks" — and round 80's "2293 on the
   slides of 95" — is not the reach.** It counts every
   `a:hlinkClick` element in the package, and **1922 of the 2537 in those decks sit on a
   `p:cNvPr`** — a click action on the *shape*, which is not text and cannot be a field. Counted
   by the parent element over all 251 `.pptx`: `cNvPr` 1922 in 64 documents, `rPr` **608** in
   **138**, `endParaRPr` 7 in 5 (an `endParaRPr` carries no text, so no run and no field). Of the
   608, **416 are on a slide proper in 89 documents**, 177 on a slideMaster in 89, 14 on a
   notesSlide in 4 and 1 on a slideLayout. `census.py` and `census2.py`.
2. **"It is one line and a measurement" is two lines, and the second is not the field flag.**
   `textrun.cxx`:88 tests whether the run's hyperlink property *map* is empty, not whether the
   element is there, and an `a:hlinkClick` that states nothing leaves it empty — no field, no
   colour, **no underline**. This tree underlined and coloured such a run. Corpus reach is one
   occurrence (an `.xlsx` drawing), so it is a correctness fix and not a mover; the pair `v10`
   / `v11` is what settles it at the reference.
3. **"An over-long URL" understates what moves.** A hyperlink run of *any* length changes where
   the line breaks whenever the line's boundary falls inside it, because the cell opportunities
   are added to the paragraph's — `v15`'s link is 28 characters and the reference breaks it after
   `https://exa`. 375 of the 416 slide hyperlink runs are under 40 characters, and **117 of the
   251 decks still move**.
4. **"A shape's text in a `.docx` goes through EditEngine, so the boundary matters."** It does
   not go through EditEngine: a `wps:txbx` is a Writer fly's text and its cursor has
   `HyperLinkURL` like any other. `w3`/`w4` are span-identical to `w1`/`w2`.
5. **The `.xlsx` question was already answered in the tree.** `SheetLayout.HyperlinkRanges` and
   `SheetTextLayout`'s `#000080` have modelled the Calc field since round 69, with the
   `worksheethelper.cxx` seat cited; what this round adds is that the *cell break* fires there
   too, which that model approximates rather than reproduces.
6. **`style:font-charset="x-symbol"` did not prove cheap.** It needs a `Charset` on
   `OdfFontFace`, a way to carry it on `SlideMarker` and through the ODF run reader, and — because
   the change would land in the ODF path this round already touches through shared code — an
   `.odp` sweep of its own to prove it. Its own measured effect is that we draw a glyph where
   26.2.4.2 draws a *blank* on 4 of 302 documents. Left with its seat, unchanged.

## An instrument note, because it cost this round a sweep's tail

**Editing a shell script while it is running makes `bash` re-execute part of it.** `bash` reads a
script incrementally and remembers a byte offset, so rewriting `reach.sh` in place — to let it take
a list file as well as a glob — moved the offset under the live interpreter and it resumed at the
`xargs` line and swept the whole column a second time. The row file went past its own denominator:
**301 rows for 251 documents**, with the duplicates beginning exactly where the edit landed.

It is the third shape of the rule `dotnet/CLAUDE.md` already carries twice — two workers of one run
sharing a directory, and two runs sharing a directory. **The second shape then happened here too,
and by my own hand**: a confinement sweep was started with a list built by an unquoted `$(ls …)`,
which split five paths on their spaces; killing the driver with `pkill` left its `xargs` children
alive, and they went on writing into the directory the replacement run had just recreated. Five
`base-failed` rows named fragments of one `.xls` filename, which is the tell. **Killing a sweep
means killing its children, and a restart goes into a fresh directory.** This one is *one* run, one directory, and
the script itself as the shared resource, and the tell is the same: a row file that keeps growing
after its denominator is reached has a second writer. The first 251 rows are 251 distinct
documents and are the sweep; they are kept as `reach-stage1.tsv` and everything after them is
discarded. **Copy a script aside before editing it if anything is running it**, exactly as
`CLAUDE.md` says to copy a source file aside rather than `git stash` it.

## What is left, with its seat

- **A `.ppt` hyperlink is a field and this tree does not read the record.** 17 of 51 corpus `.ppt`,
  91 `PPT_PST_TxInteractiveInfoAtom`. It needs the `ExObjList`/`ExHyperlink` container and the
  per-shape `PPT_PST_InteractiveInfo`, which is a reader feature rather than a line.
  `filter/source/msfilter/svdfppt.cxx`:6900-6941 and :7020-7100.
- **An `.xlsx` hyperlink cell is cell-broken and spilled at the ascent, and this tree draws it as
  one clipped line.** `x2` shows 26.2.4.2 breaking inside `leadership` and drawing the remainder
  9.099 pt below. The two agree on everything the row shows at the row heights Calc computes,
  because the row is measured from one line and the rest is clipped; they differ on the character
  that straddles the clip. `SheetLayout.HoldsField`, `SheetTextLayout.Place`.
- **A centred or right-aligned paragraph holding an over-long field.** `v12`, and the ODF round's
  `v11`/`v12` before it: the reference draws it off the box's left edge with characters lost. Not
  reproduced on purpose.
- **A 1.4 pt line-measure edge on a 52-character line of plain text** (`h25`), which has nothing to
  do with fields and is visible in the `g`/`h` series here because it sits in the middle of it.
- **`style:font-charset="x-symbol"`**, unchanged and not cheap — see *What this brief got wrong*.
- **`a:fld` is a field too and would move nothing**, measured: 1322 of them on the corpus's slides,
  longest cached text 10 characters.

## The scripts

| | |
|---|---|
| `census.py` | every `a:hlinkClick` in every OOXML corpus document, classified by whether the property map survives |
| `census2.py` | the same restricted to `a:r/a:rPr`, by part kind, with the run text |
| `precede-census.py` | what precedes a slide hyperlink run: whitespace, the paragraph's start, or glued text |
| `ppt-hyperlink-census.py` | `.ppt` carrying a `PPT_PST_TxInteractiveInfoAtom` text range |
| `fld-census.py` | `a:fld` on the corpus's slides, by type, with the cached text length |
| `make-pptx-variants.py` | the field variants and the glued-`(` series |
| `make-docx-xlsx-variants.py` | the four `.docx` and two `.xlsx` adjacent-format probes |
| `make-fixture.py` | writes `dotnet/tests/corpus/features/slide-hyperlink-field.pptx` |
| `ref-render.sh` | one document through 26.2.4.2, hex-digest profile and `timeout -k` |
| `baselines.py` | every drawn span of a PDF: baseline, x, size, colour, face, text |
| `compare.py` | two PDFs' spans, matched on text and colour with a tolerance on the origin |
| `score.py` | how many of the reference's spans each of two renderings reproduces |
| `visible.py` | whether two renderings that differ byte for byte differ *on the page* |
| `reach.sh` | one column or list rendered with two binaries, byte-compared, movers kept |
| `score-movers.sh` | the reference for each named document, then `score.py` over base and head |

And the banked figures: `hlink-census.tsv`/`.summary` and `longest-slide-links.tsv` (the census),
`reach-stage1.tsv` (the whole `.pptx` column, field flag alone), `reach-stage2.tsv` (the 138
link-bearing decks and the 156 link-bearing `.odp`, whole change), `reach-other-tracks.tsv` (42
documents of the five other original tracks), `mover-scores.tsv` (the 30-document reference
scoring) and `variant-agreement.txt`.
