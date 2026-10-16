# L4-doc-legacy — findings

34 documents, all legacy `.doc`. Grouped by root cause. Repo read-only throughout; nothing was
built, tested or modified in `/home/user/libreoffice-core`. All patches were generated from copies
and verified with `git apply --check` against `582c8c671`.

**Instruments.** A WW8 dumper written for this lane (CFB directory walk that respects the root
storage's children — an `ObjectPool` holds streams called `1Table` and `Data` too, and taking the
first match by name reads an embedded object's table stream instead of the document's; FIB at
`0x009A` for `FibRgFcLcb97`; piece table; `PlcfBteChpx`/`PlcfBtePapx` FKPs; `PlcSpaMom`/`PlcSpaHdr`;
`PlfLst`, whose declared `lcb` covers only the `LSTF` array and whose `LVL`s follow it). The
`fc → cp` mapping was calibrated against `sprmCFSpec`, which lands on exactly the field and
placeholder characters — 790 of 794 aligned at offset 0, which is how the compressed (cp1252) piece
was caught. Round trips through `soffice --headless --convert-to fodt` gave LibreOffice's own
reading of each document.

---

## R1 — A nested field's separator ends the *enclosing* field's instruction

### What the pages show

`/data/bench/pairs-view/004.jpg`, page 4. The reference sets each heading as
`307.⇥⇥OBJECT FREE AREA307.`; we set `307.⇥⇥OBJECT FREE AREA.⇥RUNWAY OBJECT FREE AREA".` The same
substitution repeats on 308, 309 and 310 on that page and on every numbered heading in the document.
`#005` and `#007` are the same advisory circular at other revisions and show it identically. The
headings are two-column, so each one that grows by a line moves the column break, which is why 4 of
5 compared pages diverge on a document whose content is otherwise right.

### What the document actually contains

`150_5300_13_chg8.doc`, cp 14691–14856 (one uncompressed dump of the piece table):

```
OBJECT FREE AREA
U+0013 tc  \l 2 "
        U+0013 seq level0 \r307 \*arabic U+0014 307 U+0015
        .
        U+0013 seq level1 \h \r0 U+0015   (and level2, level3, level4)
        ⇥RUNWAY OBJECT FREE AREA"
U+0015
.
U+0013 xe "Runway Object Free Area:standards" U+0015
U+0013 xe "Object Free Area (OFA):standards" U+0015
```

The `TC` field has **no separator of its own**; five `SEQ` fields nested inside its instruction each
have one. `sprmCFFldVanish` (`0x0802`) = `0x81` is set over cp 14707–14855 and again over both `XE`
fields, and is *off* at cp 14856 — so the file itself marks exactly the range LibreOffice hides.
That range is what leaks.

LibreOffice's flat-ODF export of the same paragraph:

```xml
<text:span T13>OBJECT FREE AREA</text:span>
<text:toc-mark text:string-value="seq level0 \r307 \*arabic307" text:outline-level="2"/>
<text:span T15><text:sequence text:name="level0" style:num-format="1">307</text:sequence></text:span>
… four empty sequences …
<text:span T16>.</text:span>
<text:alphabetical-index-mark text:string-value="standards" text:key1="Runway Object Free Area"/>
```

i.e. the instruction's plain text is gone and the nested `SEQ`'s **cached result is kept**. That
second `307` is not an inference: it is in the export and it is on the reference page.

The mechanism is `SwWW8ImplReader::Read_FieldVanish` (`ww8par5.cxx:3750`), driven by that
`sprmCFFldVanish`, setting `m_bIgnoreText`; `ReadChars` skips text outright while it is set
(`ww8par.cxx:3395`) — but a *field* is inserted by `Read_Field`/`ImportExtSprm` and not by
`ReadChars`, so a nested field's value survives the suppression. "The innermost open field decides"
reproduces that exactly.

### Where it lives in the source

* `dotnet/src/XRay.Office.WordProcessing/Ww8/Ww8DocumentReader.Layout.cs:630` —
  `int instruction = 0;`, incremented once per `U+0013` and decremented at **both** `U+0014` (`:767`)
  and `U+0015` (`:795`). A nested field with a separator therefore decrements it twice for one
  increment, and the outer field's suppression is cancelled by the inner field's end. Guard at `:679`.
* `dotnet/src/XRay.Office.WordProcessing/Ww8/Ww8DocumentReader.Content.cs:131,139,155` — the extraction
  walk's `bool InFieldInstruction`, set true at every begin and false at every separator *and* every
  end. Same defect in a second shape.

Simulating both walkers over all 34 lane documents: 402 leaked characters and 42 lost on `#004`,
854/65 on `#005`, 278/13 on `#007`, and **zero on the other 31**. The lost characters are the
section numbers themselves — `300 301 302 … 310` — which is precisely the case note's "a field
inside the heading returns the section's *text* where the reference returns its *number*".

Our own extraction confirms the leak independently:
`307.\t\tOBJECT FREE AREA307.\tRUNWAY OBJECT FREE AREA".  The runway object free area…`
(the extraction path emits the nested `307` and leaks; the layout path suppresses the `307` *and*
leaks, which is what the page shows).

### The proposed change

`patches/field-instruction-nesting.diff`. One entry per open field instead of a counter/flag:
push `true` at a begin, replace the top with `false` at that field's separator, pop at its end;
text is hidden when the innermost open field is still in its instruction. The
`instruction == 0` test that gates the computed-`FILENAME` replacement becomes
`fieldInstructions.Count == 1`, which is the same statement ("this field is not nested").

### The probe that would refute it

A two-field `.doc`: `{ TC "a{ SEQ x \* arabic }b" }` followed by plain text. If the reference draws
`b` or the quotation mark, the suppression rule is wrong; if it draws the `SEQ` result and nothing
else, this is right. Cheaper still and already run: our own render of `#004` after the patch should
contain no `"` character in any heading — `pdftotext | grep '"'` over the headings is a one-line check.

### Confidence, and what was not established

**High.** What was not established: whether `sprmCFFldVanish` should *also* be honoured. It would
be belt-and-braces here (the two rules agree on all three documents), but it would additionally hide
the nested `SEQ` result, which the reference draws — so honouring it naively would be a regression.
The nesting rule is the one that is right in general: a nested field inside a non-vanishing field's
instruction (`{ IF { PAGE } … }`) has no `fFieldVanish` and must still be hidden.

---

## R2 — A shape whose FSPA rectangle has no area is discarded, and a rule has none

### What the pages show

`/data/bench/pairs-view/004.jpg`: the reference draws short vertical revision change-bars in the
left margin beside changed paragraphs (visible beside *d. Precision OFZ.*, and at the top of the
column). We draw none, on all three AC 150/5300-13 revisions. The same guard silently drops rules on
three further lane documents.

### What the document actually contains

`PlcSpaMom` for `150_5300_13_chg8.doc` holds **25 anchors and every one is degenerate**. The one
anchored at cp 14938 (immediately after the heading above):

```
cp 14938  U+0008   spid 1063   rect (10430, -10263) .. (10430, -9903)   flags 0x0074
```

`xaLeft == xaRight`, so `Width == 0`; height 360 twips = 0.25 in. `bx = 2`, `by = 2`, `wr = 3`
(text runs through). The Escher `OfficeArtFSP` for spid 1063 has `recInstance = 20` — `mso_sptLine`.
All 25 shapes in the document are type 20. LibreOffice's export carries them as
`<draw:line svg:x1="7.2429in" svg:y1="-6.8772in" svg:x2="7.2429in" svg:y2="-7.1272in"/>` — the same
coincident x, converted to a real line.

Census over the whole lane (`PlcSpaMom` + `PlcSpaHdr`, degenerate = width ≤ 0 or height ≤ 0):

| doc | anchors | degenerate | kinds |
|---|---:|---:|---|
| #003 `AAC-AD-No-2021-01-…` | 123 | **108** | 46 zero-width, 62 zero-height, all type 20 |
| #004 `150_5300_13_chg8` | 25 | **25** | 24 zero-width, 1 zero in both |
| #005 `150_5300_13_chg10` | 72 | **58** | 56 zero-width, 2 zero-height |
| #007 `150_5300_13_chg12` | 23 | **18** | all zero-width |
| #033 `手机免提系统TSB` | 2 | **2** | zero-height |
| #042 `150_5335_5a` | 23 | **2** | one of each |

211 of the lane's 313 shape anchors, and every single one of them shape type 20.

### Where it lives in the source

`dotnet/src/XRay.Office.WordProcessing/Ww8/Ww8Frames.cs:91`:

```csharp
if (anchor.Width <= 0 || anchor.Height <= 0) return null;
```

This is the whole of it. The rest of the pipeline is already correct and already reads the property:
`Ww8Frames.cs:159` computes `isLine` from the shape type, `PageFrame.IsLine` carries it, and
`dotnet/src/XRay.Office.WordProcessing/Layout/PageDrawing.cs:279` strokes a line frame from
`(area.X, area.Y)` to `(area.Right, area.Bottom)` — its box's diagonal — which for a coincident pair
of edges is exactly the vertical or horizontal rule. **This is the "read but never consumed" pattern
in its sharpest form: the shape type is read, the line geometry is implemented, and the frame is
thrown away one line before either is reached.**

### The proposed change

`patches/degenerate-line-shapes.diff`. Hoist `isLine` above the guard and let a line through when
exactly one axis is degenerate. A negative extent is still rejected (a malformed record), and so is
a rectangle degenerate in both axes (nothing to draw). Non-line shapes are unaffected.

### The probe that would refute it

Render `150_5300_13_chg8.doc` page 4 after the patch and count vertical strokes in the left margin:
the reference has them beside the changed paragraphs and nowhere else. If the patched build draws
them in the wrong place rather than not at all, the defect is the FSPA's origin resolution
(`bx = by = 2`, text-relative) and not the guard — that is the next most likely explanation, and the
one this distinguishes from. A negative control is worth having too: `#003` gains 108 shapes and its
page 3 currently matches for ink, so a large ink change there would say the guard was load-bearing
for something else.

### Confidence

**High** that the guard is why nothing is drawn; **medium** on the visual outcome, because the
placement of a `bx=2, by=2` anchor with a large negative `ya` is not something I could verify without
building.

---

## R3 — HTML auto space-after is never handed back when a list ends at a cell mark

### What the pages show

`/data/bench/pairs-view/080.jpg`. `FlightLaws.doc`'s table is far more compact than the reference's:
the reference leaves a blank line before *After touchdown, ground mode is reactivated*, before *No
rudder pedal feedback*, and before *In the event of a go-around*; we leave none, and the table ends
halfway down the page where the reference's fills it. Every line of text is present.

### The stated mechanism is not in the file

The case note says empty paragraphs inside cells are dropped. **The document contains none.** The
piece table between the two bullets the gap sits between is:

```
…Is active until shortly after liftoff.\rAfter touchdown, ground mode is reactivated…
```

One `U+000D`. What the reference draws there is a **margin**: LibreOffice's own export gives that
paragraph `fo:margin-bottom="0.1945in"` — 280 twips, fourteen points, the HTML auto-spacing value.
Every paragraph in the cell carries `sprmPDyaBefore = 100`, `sprmPDyaAfter = 100`,
`sprmPFDyaBeforeAuto = 1`, `sprmPFDyaAfterAuto = 1`, `sprmPIlfo = 1`, `sprmPIlvl = 0`.

The round-trip test from `render-comparison` §5 localises it in one step: rendering
`soffice --convert-to fodt` output of the document through our own CLI **restores the gaps**
(`work/triptych.png`, reference | ours-from-doc | ours-from-fodt). The layout engine is exonerated;
the defect is in the WW8 reader.

### What the file and the reference actually say

Reading LibreOffice's export cell by cell, the paragraph that carries the 0.1945 in is:

| cell | list | items | which item carries the margin |
|---|---|---:|---|
| Ground Mode | WW8Num8 | 4 | **3rd** (the cell's last-but-one) |
| Flight Mode | WW8Num13 | 10 | **9th** |
| Flare Mode | WW8Num2 | 3 | **2nd** |
| Protections / WW8Num7 | 4 | | **4th** — the run's own last, because a heading follows *inside* the cell |
| Protections / WW8Num10 | 2 | | **2nd** — same |
| Protections / WW8Num9 | 2 | | **2nd** — same |
| Protections / WW8Num11 | 2 | | **1st** — the cell's last-but-one |

Six groups, one rule: the margin goes to the last item of the run **unless** that item is the last
paragraph of the cell, in which case it goes to the one before it. That is not a special case — it
is `WW8TabDesc::SetPamInCell` (`ww8par2.cxx:2908-2934`) forcing the cell's last paragraph's lower
spacing to nought on the way into the next cell, so the restore that
`SwWW8ImplReader::FinalizeTextNode` (`ww8par.cxx:2627-2673`) performs on the *remembered* paragraph
lands on the previous one. The memory is a member of the reader (`m_pPreviousNumPaM`), not of a cell:
it survives the cell boundary and is discharged by the next unnumbered paragraph, which here is the
next row's label cell.

### Where it lives in the source

`dotnet/src/XRay.Office.WordProcessing/Ww8/Ww8DocumentReader.AutoSpacing.cs`. `SuppressWithinList` is
called **once per cell** and once for the flow, and its loop has no post-condition: a run that
reaches the end of its block list leaves `previous` set and never restores it. So a list that ends
at a cell wall gets zero margin at both ends. The cell-edge rule then zeroes the last paragraph
again, which is right, but the last-but-one never gets anything.

### The proposed change

`patches/cell-auto-spacing.diff`. One walk in document order — a small `ListRun` class holding the
same three pieces of state LibreOffice holds — recursing into tables so the memory crosses cell and
row boundaries as the reader's does. A cell's last paragraph takes the zeroing like any other and is
then neither consulted nor remembered. The cell-edge and flow-edge rules are unchanged.

I checked all six assertions in `tests/XRay.Office.WordProcessing.Tests/DocAutoSpacingTests.cs` by
hand against the new walk: none of them puts a list inside a cell, so all six hold unchanged.

The change is strictly additive against today's output: today no item in a cell-terminated run gets
the margin, so nothing can lose one. Census over the lane: `#080` has 9 cell-terminated numbered runs
with auto-after; `#023`, `#032` and `#081` carry auto-spacing but have **zero** such runs, so they
cannot move.

### The probe that would refute it

A two-cell `.doc` whose first cell holds three bullets of one list and whose second cell holds plain
text, with `sprmPFDyaAfterAuto` on all of them. The reference puts 280 twips under the **second**
bullet if this is right and under the third if the restore actually lands on the cell's last
paragraph. That is also the **version discriminator** — see below.

### Confidence, and what was not established

**Medium-high.** The rule is measured on six groups of one document, against 24.2.7.2's export only.
I could not reconcile it line-by-line with LibreOffice's control flow: `SwWW8ImplReader::TabCellEnd`
(`ww8par2.cxx:3469-3478`) *does* call `FinalizeTextNode` for the cell's last paragraph, which on a
naive reading would make that paragraph the remembered one and give it the margin — and the export
says otherwise, six times out of six. Something between `SetPamInCell`'s zeroing and the node moves
that build the table decides it. **I am implementing the measured behaviour, not the derived one**,
and say so.

---

## R4 — A `FORMCHECKBOX`'s square lives inside the field's instruction and is dropped

### What the pages show

`/data/bench/pairs-view/054.jpg`. The reference draws an empty square before *CAT II*, *CAT IIIA*,
*CAT IIIB*, before *Yes* and *No*, and before every item of the *AFM* group; we draw none. The case
note calls this an inconsistency within one document, because squares *do* appear lower down beside
*Type design / FAA STC / STC / Service Bulletin*. `#060` has the same defect with no such contrast.

### What the document actually contains — and the "inconsistency" is not one

Every checkbox that is missing is a field:

```
U+0013 " FORMCHECKBOX " U+0001 U+0015
```

`1528364855.doc` holds **37** of these and `f111.doc` **58**, and **not one of them contains a
`U+0014`**. There is no separator, so the `U+0001` that stands for the box is the last character of
the field's *code*.

The squares that *are* drawn are not fields at all — they are literal `U+25A1 WHITE SQUARE`
characters in the piece table:

```
The AWO type design approval is reflected in:\r□ Type design    □ FAA STC    □ STC    □ Service Bulletin
```

Seven of them in `#054`, none in `#060`. So the document is entirely consistent: every field is
dropped and every literal character is drawn.

LibreOffice reads the placeholder at exactly that offset —
`if (rStr[pF->nLCode-1]==0x01) ImportFormulaControl(aFormula, pF->nSCode+pF->nLCode-1, WW8_CT_CHECKBOX)`
(`ww8par3.cxx:191-192`) — and `Read_Field` even special-cases the sibling case in the same file:
`if (aF.nId == 70) bCodeNest = false; // need to import 0x01 in FORMTEXT` (`ww8par5.cxx:967`).

### Where it lives in the source

`dotnet/src/XRay.Office.WordProcessing/Ww8/Ww8DocumentReader.Layout.cs:679` — the instruction guard
`continue`s on every character that is not a field marker or a paragraph mark, and `U+0001` is one of
them. There is no `FORMCHECKBOX` handling anywhere under `Ww8/`; `Ww8FieldTypes` names `SHAPE`,
`FILENAME`, `PAGE` and `NUMPAGES` and stops there.

The DOCX reader has the whole thing already —
`dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs:1328` `CheckBoxFrame`, with
`CheckBoxInset = 25 tw` and `CheckBoxStroke = 0.1 pt` — and `PageFrame.BorderInset` /
`PageFrame.IsCrossed` and `PageDrawing.cs:267,298` exist to serve it. Only the WW8 half is missing.
The geometry was pinned on 26.2.4.2 in `dotnet/probes/words-r56/prediction-checkbox.md`: the square
is the line's text height less 50 twips at every size and in every face, and the field's own declared
size is inert.

### The proposed change

`patches/form-checkbox.diff`, four files:

* `Ww8FieldTypes.cs` — `FormCheckBox = 71`, `Read_F_FormCheckBox`'s slot in `aWW8FieldTab`.
* `Ww8DocumentReader.Layout.cs` — let a `U+0001` through the instruction guard when the innermost
  open field is a `FORMCHECKBOX`; record its offset and emit an anchor character so it takes room on
  the line (the width matters as much as the box — 95 positions in this lane are currently reserving
  nothing).
* `Ww8DocumentReader.FormFields.cs` (new) — the ticked state, from the `FFData` in the `Data` stream
  at the placeholder's own `sprmCPicLocation`, behind a 68-byte picture header: `iRes` when it is not
  25, else the default that follows the control's name.
* `DocReader.cs` — `WithCheckBoxes`, which sizes the square from the *run's* resolved face exactly as
  `CheckBoxFrame` does, and falls back to the paragraph's.

### The probe that would refute it

`pdftotext` cannot see a square, so compare ink: page 1 of `#054` should gain 37 small strokes and
its lines should each get 11-ish points narrower. If the boxes appear but at the wrong size, the
refutation is that WW8 *does* honour the `FFData`'s `hps` where DOCX ignores `w:checkBox/w:size` —
build a `.doc` with two boxes at declared sizes 5 pt and 40 pt in a 12 pt run and see whether the
reference draws them differently.

### Confidence, and what was not established

**High** on the diagnosis and the geometry, **medium** on the ticked state: I read `iRes` and the
default per `WW8FormulaControl::FormulaRead` (`ww8par3.cxx:2127-2185`) but could not test it, and
every box in both lane documents appears unticked in the reference, so the corpus does not exercise
it. I also did not implement `FORMTEXT` (field 70) or `FORMLISTBOX` (83); neither draws a box, but
`FORMTEXT`'s `U+0001` is skipped by the same guard and is worth a separate look.

---

## Refuted — mechanisms the files do not contain

Recorded because `dotnet/CLAUDE.md` asks for exactly this, and each cost about a minute to check.

1. **"Blank paragraphs dropped from table cells" (`#080`).** `FlightLaws.doc` contains no empty
   paragraph anywhere near the gaps; the piece table has a single `U+000D`. The gap is R3's margin.
   `grep`-equivalent: dump cp 690–960 and count `\r`.
2. **"The ideographic comma after each list number dropped" (`#033`).** `pdftotext -layout` of both
   PDFs reads `1、打开音响…` and `A、检查音响…`. The comma is drawn on our side. The `LVL`'s `xst` is
   `'\x00、'` with `rgbxchNums[0] = 1` and our `FormatLabel` handles it; the extraction path emits it
   too. The document's real divergence is looser line spacing on CJK text — a metrics question, not
   a list-marker one, and not this lane's.
3. **"Checkbox glyphs drawn in some sections of a form and not others" (`#054`).** Refined rather
   than refuted: the drawn ones are literal `U+25A1` characters and the missing ones are all 37
   fields. There is no per-section behaviour to explain.

---

## Not established — recorded, not patched

* **`#059` `07-04.doc`, the underlined `January 1, 2008`.** Not "the effective-date value placed in
  the flow": LibreOffice reads the paragraph too — its export carries
  `<text:p text:style-name="P11">…January 1, 2008 </text:p>` — and then does not draw it on page 1
  (one occurrence of the string in the whole reference PDF, in the body on page 3). The document's
  first three paragraphs are a Word frame (`sprmPDxaAbs`, `sprmPDyaAbs`, `sprmPPc`, `sprmPDxaWidth`,
  `sprmPWHeightAbs`); LibreOffice places it at `svg:y="-0.7492in"` relative to its anchor with
  `style:wrap="parallel"` and 6.47 in of a 7.5 in measure, and the date line — 72 spaces, a tab, 19
  spaces, a tab, then the text, in a paragraph with `fo:margin-left="-0.3752in"` and tab stops from
  −1 in to 7.5 in — ends up out of view. This is a frame-position and wrap question, one document
  deep, and I could not establish which of the three candidates (frame origin, wrap band, tab
  resolution against a negative indent) is ours. **Not patched.**
* **`#005`/`#007`, the footer chapter marker.** Confirmed on the image (the reference's page 4 footer
  is empty; ours prints `2` and `Chap 1`), but our page 4 and the reference's page 4 are not the same
  logical page — the document is offset by R1 and R2. I did not establish an independent cause and
  expect this to resolve with them. **Not patched.** Re-check after R1+R2 land.
* **`#025` `SFSP_2013-02_Bulletin.doc`, the hollow-diamond bullets.** The six *Additional Topics*
  items all carry `sprmPIlfo = 4` and one level; there is no per-item difference in the list tables
  to produce a diamond on some and a round bullet on others, and LibreOffice's export gives all six
  the same `text:list-style` with one `loext:marker-style-name`. The visual claim is not corroborated
  and I did not find a mechanism. **Not patched.**
* **`#159` `DEP2008-1900.doc`, the added bullet.** The *Fleet vehicles* paragraph carries
  `sprmPIlfo = 5` and LibreOffice makes it a `<text:list-item>` too, so the reference is a list item
  as well. It does carry `sprmPNumRM` (`0xC645`, `fNumRM = 1` — the numbering is a revision mark),
  but LibreOffice maps that sprm to `nullptr` (`ww8par6.cxx:6278`), so that is not the reason either.
  **Not patched.**
* **The residual, 23 of 34 documents.** Every remaining case in this lane is tagged `reflow` or
  `pagination` and describes text set a shade tighter or wider with nothing missing or mis-styled.
  That is the advance-width divergence `dotnet/CLAUDE.md` §3 records with a known seat (grid-fitted
  versus unhinted advances) and explicitly forbids re-deriving. Nothing in the WW8 reader accounts
  for it and I propose nothing for it.

---

## Reference-version check

The reference here is **24.2.7.2**; the tree is developed against **26.2.4.2**. Per root cause:

* **R1** — version-independent. No version of Word or LibreOffice draws a `TC` field's own code, and
  the leaked text includes a bare `"` from the field's argument syntax. The mechanism I cite
  (`Read_FieldVanish` → `m_bIgnoreText` → `ReadChars`) is present in the checkout's own C++ tree,
  which is newer than both.
* **R2** — version-independent in the same sense: the file states 25 line shapes and the reference
  draws 25 rules. The patch is guarded to `mso_sptLine`/`StraightConnector` so nothing else can move.
* **R3** — **this is the one measured against 24.2.7.2 alone**, because it is measured from that
  binary's own flat-ODF export and only that binary is installed here. The discriminating probe is
  the two-cell fixture above, rendered under both binaries: if 26.2.4.2 puts the margin under the
  *third* bullet rather than the second, the rule is version-divergent and the patch should be
  re-tuned or held. Today's output gives the margin to neither, so the patch moves toward whichever
  answer is right rather than away from one — but it should be the last of the four to be applied,
  and re-measured first.
* **R4** — version-independent, and its geometry is *already* 26.2-calibrated: it reuses the constants
  and the derivation from `probes/words-r56/prediction-checkbox.md`, measured on 26.2.4.2, whose census
  was `.docx`-only. This patch is the WW8 half of work the tree already committed to.

The C++ checkout's history is a single squashed commit, so `git log -L` archaeology on the WW8
importer is not available here; I say what I could check and what I could not.

---

## Cross-lane dependencies

**None.** All four patches are confined to `dotnet/src/XRay.Office.WordProcessing/Ww8/**`. Two shared
files were read and deliberately not touched because they already do the right thing:
`XRay.Office.WordProcessing/Layout/PageDrawing.cs:279` (strokes a line frame as its diagonal, which is
what makes R2 a one-line change) and `Layout/PageFrame.cs:360,368` (`BorderInset` and `IsCrossed`,
which is what makes R4 need no new drawing primitive).

One thing a serialising pass should know: R1 and R4 both rewrite the same instruction guard in
`Ww8DocumentReader.Layout.cs`, so `form-checkbox.diff` is generated against a tree that already has
`field-instruction-nesting.diff` applied. **Apply R1 before R4.** R2 and R3 are independent of both
and of each other and apply alone.

Verified: each of the four applies cleanly to `582c8c671` on its own (R4 to `582c8c671` + R1), and
all four apply in the order R1, R2, R3, R4 to give exactly the intended tree.
