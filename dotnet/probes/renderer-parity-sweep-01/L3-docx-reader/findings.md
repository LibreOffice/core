# L3-docx-reader — findings

Thirty-seven documents, eight root causes. Every claim below was checked against the
document's own markup before the mechanism was named, and against the reference PDF's
own text layer where the page could answer the question directly.

Ownership: `dotnet/src/XRay.Office.WordProcessing/Ooxml/**` and `.../Model/**`. Two of the
eight root causes need a file outside that and are recorded under **Cross-lane
dependencies** rather than patched.

**What is *not* here.** Most of the case notes also say some version of "the table is set
a little wider and the rows a shade taller, so the page holds one row fewer". That is the
advance-width divergence and the table-sizing cascade already recorded in
`dotnet/CLAUDE.md` (§3, and the `AWR OPS-AOC 044` worked example). It is not re-derived,
not re-attributed, and not patched here. The documents whose *only* divergence is that —
#019, #028, #034, #038, #046, #050, #075, #085, #087, #090, #112, #116, #123, #150, #179 —
are listed once, at the end.

---

## RC1 · A table-of-contents entry is drawn in the `Hyperlink` character style; the reference drops it

### What the pages show

Every contents entry blue and underlined where the reference prints plain black, the page
numbers and leader dots otherwise identical. Reported on six documents:

| case | document | note |
|---|---|---|
| #006 | `UG.CAO.00006 … User Guide` | "Every table-of-contents entry is drawn as a blue underlined hyperlink"; ink ×1.17 |
| #036 | `AC-150-5370-10G-updated-201604` | underlined where the reference is plain; ink ×1.23 |
| #048 | `ESPN-R - MCF - Manual - Ed1.0` | same, differently-styled manual; ink ×1.19 |
| #068 | `OM template for non-complex NCC operators` | same; ink ×1.17 |
| #089 | `33004` | "The 1.1 Management Commitment and Responsibility heading is printed blue" — this is the **TOC entry**, see below |
| #181 | `Agile_Arc_SysDes` | same; ink ×1.16 |

The four ink ratios are the tell that these are one fault: 1.16–1.23 on a page that is
otherwise word-for-word identical is an underline under every line plus a colour change.

#089's case note calls its defect a *heading*. It is not: `Management Commitment and
Responsibility` occurs **exactly once** in `word/document.xml`, in a `w:pStyle="TOC2"`
paragraph inside the TOC field's result. The body heading of that name does not exist in
the file. #089 therefore folds into this root cause.

### What the document actually contains

`Agile_Arc_SysDes.docx`, `word/document.xml`, the TOC field's result:

```xml
<w:p><w:pPr><w:pStyle w:val="TOC1"/>…</w:pPr>
  <w:r><w:fldChar w:fldCharType="begin"/></w:r>
  <w:r><w:instrText xml:space="preserve"> TOC \o "1-3" \h \z \u </w:instrText></w:r>
  <w:r><w:fldChar w:fldCharType="separate"/></w:r>
  <w:hyperlink w:anchor="_Toc450730285" w:history="1">
    <w:r><w:rPr><w:rStyle w:val="Hyperlink"/><w:rFonts w:cs="Arial"/></w:rPr>
      <w:t>Document History</w:t></w:r>
    …<w:instrText xml:space="preserve"> PAGEREF _Toc450730285 \h </w:instrText>…
  </w:hyperlink>
</w:p>
```

and `word/styles.xml`:

```xml
<w:style w:type="character" w:styleId="Hyperlink"><w:name w:val="Hyperlink"/>
  <w:rPr><w:rFonts w:ascii="Arial" w:hAnsi="Arial"/><w:color w:val="0000FF"/>
         <w:sz w:val="20"/><w:u w:val="single"/></w:rPr></w:style>
```

So the file really does say blue-and-underlined, and we are reading it correctly. The
reference disagrees **on purpose**:

`sw/source/writerfilter/dmapper/DomainMapper.cxx:3030-3048`

```cpp
case NS_ooxml::LN_EG_RPrBase_rStyle:
    …
    // Add the property if the style exists, but do not add it elements in TOC:
    // they will receive later another style references from TOC
    if ( bExists && m_pImpl->GetTopContext() && !m_pImpl->IsInTOC())
        m_pImpl->GetTopContext()->Insert( PROP_CHAR_STYLE_NAME, uno::Any( sConvertedName ) );
```

`IsInTOC()` is `m_bStartTOC`, set by `handleToc` / `handleIndex` / `handleBibliography`
(`DomainMapper_Impl.cxx:7643`, `:7871`, `:7899`) and cleared only when *that* field's
context is popped (`:9269`, guarded by `pContext->GetTOC().is()`), so the `PAGEREF` fields
nested inside each entry do not end the scope. `handleToc` is reached from
`CloseFieldCommand`, which `DomainMapper.cxx:4357` calls at `cFieldSep` — the
`w:fldChar w:fldCharType="separate"`. So the suppression covers exactly the field's
**result**, and nothing else.

Census over the 37 (`word/document.xml`, counting `w:pStyle="TOC N"` paragraphs and
`w:rStyle="Hyperlink"` runs): 18 carry a `TOC` field and 16 carry both — #006, #008, #028,
#029, #035, #036, #048, #068, #073, #075, #089, #090, #123, #179, #181, #192. The six
above are the ones whose compared page happened to be a contents page.

### Where it lives in the source

- `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs:1098-1101` —
  `RunsOf` calls `WordParagraphFormats.ResolveRun` with the run's `w:rPr` and no notion of
  where in the paragraph it sits.
- `dotnet/src/XRay.Office.WordProcessing/Ooxml/WordParagraphFormats.cs:462` —
  `string? characterStyleId = Word.Attribute(Word.Child(runProperties, "rStyle"), "val");`
  unconditionally.
- The comment at `WordParagraphFormats.cs:430-434` states the (correct, general) rule that
  makes this a bug rather than an oversight: *"A hyperlink wraps runs rather than
  formatting them, and the blue underline comes from the `Hyperlink` character style that
  each of those runs names itself."* True everywhere except inside an index field.
- `DocxLayoutSource.cs:1544-1620` already has the field stack (`_fields`, `OpenField`) this
  needs — the field's identity is known at `separate`, which is the right moment. This is
  not "read but never consumed": the instruction *is* consumed, by
  `FieldInstructions.PageFieldOf` and `StyleReferenceName`; `KindOf`'s
  `"TOC" or "INDEX" => WritingFieldKind.TableOfContents` is the mapping with no consumer.

### Proposed change

`patches/toc-entry-character-style.diff`. `RunWalker` counts open index-field *results*,
`StyledRange` gains an `InIndexField` flag (which also joins the range-merge predicate so
two adjacent runs on opposite sides of the boundary do not fuse), and
`ResolveRun` gains `bool ignoreCharacterStyle` which nulls `characterStyleId` while leaving
the run's own `w:rPr` alone — the same split LibreOffice makes. The compact `w:fldSimple`
form is handled too.

### The probe that would refute me

One paragraph in `TOC1`, inside a `TOC \o "1-3" \h` field's result, holding two runs: one
naming `w:rStyle="Hyperlink"` and one stating `<w:u w:val="single"/><w:color w:val="0000FF"/>`
directly. If the reference draws the second blue-and-underlined and the first plain, the
rule is "the character *style* is dropped, direct run properties are kept" and the patch is
right. If it draws both plain, LibreOffice is regenerating the whole entry and the patch is
too narrow. If it draws both blue, this diagnosis is wrong.

### Confidence

**High** for the mechanism (the C++ comment names the behaviour outright and the markup
matches on every document). What I did not establish: whether LibreOffice also overrides
the *paragraph* indents and tab stops of a regenerated entry from its own `TOC N` style —
the case notes say leaders and indent levels already match, so if it does, it agrees with us.

---

## RC2 · A list counter advances on paragraphs inside `w:vMerge` continuation cells

### What the pages show

| case | document | reference | ours |
|---|---|---|---|
| #029 | `B11. TE.CAO.00129 Experience logbook` | ID column 1–13 | 1–7, 9, 12, 13, 14, 15, 16 |
| #035 | `…annex-B-B11.-TE.CAO.00129…logbook` | 1–13 | 1–7, 9, 12–16 |
| #115 | `FO.FCTOA_.000129 … FSTD` | 3.1, 3.2, 3.3, 3.4, 3.5 | 3.1, 3.3, 3.4, 3.6, 3.12 |
| #133 | `A1. EASA Form 2` | no generated numbers | stray `1.1.1`, `1.6.1` in the margin |
| #092 | `FO.FCTOA.00010 … ATO Approval` | — | 34 affected paragraphs, none visible on the compared page |

Both patterns are "the sequence skips, and the skipped numbers are exactly as many as the
invisible paragraphs".

### What the document actually contains

`B11. TE.CAO.00129 Experience logbook.docx`, table 2 (the ID/Option/Description table).
Sixteen paragraphs in the ID column carry `<w:numPr><w:ilvl w:val="0"/><w:numId w:val="16"/></w:numPr>`.
Three of them hold **no `w:r` at all**:

```xml
<w:p><w:pPr><w:pStyle w:val="Default"/>
  <w:numPr><w:ilvl w:val="0"/><w:numId w:val="16"/></w:numPr>
  <w:ind w:left="426"/><w:jc w:val="both"/><w:rPr>…</w:rPr></w:pPr></w:p>
```

and the `w:vMerge` map of that table's first column is decisive:

```
row 0-6   (no vMerge)
row 7     vMerge=restart      "Task type"          <- numbered, drawn, counts
row 8     vMerge=continue     (empty, numbered)    <- counted by us, not by the reference
rows 9-15 vMerge=continue     (empty, unnumbered)
row 16    vMerge=restart      " Type of activity"  <- numbered, drawn, counts
rows 17,18 vMerge=continue    (empty, numbered)    <- counted by us, not by the reference
rows 19,20 vMerge=continue    (empty, unnumbered)
rows 21-25 (no vMerge)                             <- numbered, drawn, count
```

Thirteen numbered paragraphs are in cells that are drawn; three are in cells that are
covered. 16 − 3 = 13, which is exactly the reference's range, and the three numbers our
output skips (8, 10, 11) are exactly those three rows.

`FO.FCTOA_.000129` says the same thing through style-inherited numbering: `Heading2`
carries `<w:numPr><w:ilvl w:val="1"/><w:numId w:val="1"/></w:numPr>` and section 3's blocks
are, in document order, `restart, continue, (none), restart, continue, restart,
continue×5, (none)`. Counting the six continuations turns 3.1, 3.2, 3.3, 3.4, 3.5 into
3.1, 3.3, 3.4, 3.6, 3.12 — the observed output, digit for digit.

**A story I checked and dropped before believing it.** "The counter should not advance on
an *empty* paragraph" fits #029 and is wrong: `FO.FCTOA_.000129`'s covered `Heading2`
paragraphs are empty too, but so is nothing about its restart cells, and the reference
numbers a heading in a `w:vMerge w:val="restart"` cell perfectly happily — its own
reference PDF prints `2.1.1 Name and Address` and `3.1 Audit of Management …`, both from
restart cells. Emptiness is a coincidence of these templates; being *covered* is the rule.

LibreOffice states the rule from the other end:

`sw/source/core/unocore/unotbl.cxx:978-990`

```cpp
else if (rPropertyName == "VerticalMerge")
{
    //Hack to allow clearing of numbering from the paragraphs in the merged cells.
    SwNodeIndex aIdx(*GetStartNode(), 1);
    …
        if (pNd) pNd->SetCountedInList(false);
}
```

Census of numbered paragraphs sitting in `w:vMerge` continuation cells, over the 37:
#029 (3), #035 (3), #092 (34), #115 (44), #133 (39). Nothing else in the lane has any.

### Where it lives in the source

- `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.Tables.cs:480-493` —
  `Blocks = ReadCell(child)` runs for **every** cell, before `Merge(cellProperties)` is even
  evaluated.
- `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.Tables.cs:1169` —
  `Resolved()` then drops the continuation cells: `if (cell.Merge == VerticalMerge.Continue) continue;`.
  So the cell's *blocks* never reach the page and its *counter advance* always does. The
  counter is the one thing that escapes a cell that is not drawn.
- `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.Lists.cs:56-58` —
  `string? drawn = _numbering.Advance(numId, level);`, with the comment
  *"Advanced whether or not the level draws anything"*, which is right for a `none`-formatted
  level and wrong for a paragraph that is not on the page at all.

### Proposed change

`patches/covered-cell-list-counter.diff`. `Row()` evaluates `Merge(cellProperties)` before
reading the cell and sets `_inCoveredCell` around the read; `ListFormatting` leaves `drawn`
null when it is set, which routes through the existing no-label branch and neither advances
the counter nor draws a number. Restart cells are untouched, because the reference numbers
them.

### The probe that would refute me

A two-column, three-row table. Column 1: `restart`, `continue`, `continue`; every one of
the three paragraphs numbered from the same `w:numId`. Column 2: three plain numbered
paragraphs from the same list. If the reference numbers them 1, 2, 3, 4 (one for the
restart, three for column 2) the rule is "continuations do not count" and the patch is
right. If it numbers them 1, 2, 3, 4, 5, 6 the diagnosis is wrong. If it numbers only
column 2, LibreOffice's `unotbl` hack really does apply to restart cells too and the patch
is too narrow — but `FO.FCTOA_.000129`'s own reference already rules that out.

### Confidence

**High** for #029, #035, #092 and #115: the arithmetic reproduces the reference's
sequences exactly on two independent templates.

**Partial for #133.** Its stray `1.1.1` and `1.6.1` *are* computed by counting covered
cells (`1.6.1` needs the five covered `Heading2` paragraphs between them), so the patch
changes them — to `1.1.1` and `1.1.2`. It does not remove them, and the reference emits
**none**. I could not establish why: #133's `abstractNum 0` and its `Heading1-3` styles
(`outlineLvl` 0/1/2, `numId` 1) are byte-for-byte the same shape as `FO.FCTOA_.000129`'s,
which the reference *does* number; the only structural difference I found is that in #133
every level-0 and level-1 heading carries `<w:numPr><w:numId w:val="0"/></w:numPr>` and
types its number as literal text, so the two visible numbers would be the first counted
items in their list. That is a second, unidentified cause and I am flagging it rather than
guessing.

---

## RC3 · `<w:color w:val="auto"/>` is read as "this layer states nothing"

### What the pages show

- #043 `AFS-050-004-F2_0i` — "The title and effective-date runs in the header table come out
  blue where the reference prints them black."
- #145 `form_1123_application_form_rvsm_spa` — "Belgian Civil Aviation Authority is printed
  blue where the reference prints it black."
- #089 `33004` — 77 body runs in the same shape (the case note's blue heading is RC1, but
  the document carries this too).
- #181 `Agile_Arc_SysDes` — 28 caption runs.

### What the document actually contains

`form_1123_application_form_rvsm_spa.docx`, `word/header1.xml`:

```xml
<w:p><w:pPr><w:pStyle w:val="Heading5"/><w:spacing w:after="120"/>
     <w:rPr><w:i w:val="0"/><w:color w:val="auto"/></w:rPr></w:pPr>
  <w:r><w:rPr><w:i w:val="0"/><w:color w:val="auto"/></w:rPr>
    <w:t>Belgian Civil Aviation Authority</w:t></w:r></w:p>
```

with `Heading5` resolving to `w:color w:val="333399"`. `AFS-050-004-F2_0i.docx`,
`word/header1.xml`:

```xml
<w:p><w:pPr><w:pStyle w:val="Title3"/>…</w:pPr>
  <w:r><w:rPr><w:color w:val="auto"/></w:rPr>
    <w:t>Title: International Aviation Safety Assessment Assessor's Checklist …</w:t></w:r></w:p>
```

with `Title3` → `w:color w:val="0000FF"`. Neither carries a `w:themeColor`.

Census over all six parts of each of the 37 (runs stating `w:color w:val="auto"`, with no
`w:rStyle`, in a paragraph whose style chain resolves to a colour that is neither `auto`
nor `000000`): **#043 5 · #089 77 · #145 1 · #181 28**, styles `Title3` (0000FF),
`Heading2` (4F81BD), `Heading5` (333399), `Caption` (44546A).

LibreOffice inserts the value with no branch for `auto` — the tokenizer hands it across as
`COL_AUTO` and `DomainMapper.cxx:2676` does
`GetTopContext()->Insert(PROP_CHAR_COLOR, uno::Any(pThemeColorHandler->mnColor))`, so the
run's automatic colour overrides the paragraph style's, and Writer paints automatic as
black on a light ground.

### Where it lives in the source

- `dotnet/src/XRay.Office.WordProcessing/Ooxml/WordThemeColour.cs:158-172` — `Literal` returns
  null for `auto`, deliberately and with a correct reason (`w:val="auto" w:themeColor="accent1"`
  must resolve through the theme).
- `dotnet/src/XRay.Office.WordProcessing/Ooxml/WordThemeColour.cs:68-69` — the two-argument
  `Read` then returns that null straight out when there is no theme reference beside it.
- `dotnet/src/XRay.Office.WordProcessing/Ooxml/WordParagraphFormats.cs:503` and
  `Ooxml/WordCharacterFormat.cs:124` are the only two callers of that overload, and both
  read `null` as "inherit", which is how the style's blue survives.

The layering above is right: `WordStyles.ResolveRunProperty("color", …)` correctly returns
the run's own `w:color` element as the winning layer. The element is then thrown away.

### Proposed change

`patches/automatic-run-colour.diff`. The two-argument overload — the `w:color` one — falls
back to `Colour.Black` when nothing resolved *and* the element states `w:val="auto"`. The
six-argument overload is deliberately left alone: `w:shd`'s `w:fill="auto"` means *no*
fill, and answering black there would paint every automatically-shaded cell.

### The probe that would refute me

One paragraph in a style setting `w:color w:val="FF0000"`, holding two runs: one bare and
one stating `<w:color w:val="auto"/>`. Reference draws the first red and the second black
⇒ patch right. Both red ⇒ diagnosis wrong.

### Confidence

**High.** What I did not establish: Writer flips automatic to *white* over a dark
background (`SwViewOption`/`SvxFont` contrast logic). Every run in these four documents is
over white, so the patch answering black is right for them; a run stating `auto` over a
black `w:shd` fill would still come out black under this patch and white in the reference.
That is a narrower defect than the one being fixed, and it is not in this lane's corpus.

---

## RC4 · `w:ptab` — the positional tab — is dropped

### What the pages show

#139 `PI-doc.-no.-2E-Technical-Review-Report`: "The footer's tab stops are not honoured:
the reference spaces `Page 5 of 12`, `Version 1` and `Last saved…` across the page,
XRay.Office runs them together as `Page 5 of 12Version 1Last saved…`."

### What the document actually contains

`word/footer3.xml`, one paragraph:

```xml
<w:r><w:t xml:space="preserve">Page </w:t></w:r>… PAGE …  of … NUMPAGES …
<w:r><w:ptab w:relativeTo="margin" w:alignment="center" w:leader="none"/></w:r>
<w:r><w:t>Version 1</w:t></w:r>
<w:r><w:ptab w:relativeTo="margin" w:alignment="right" w:leader="none"/></w:r>
<w:r><w:t xml:space="preserve">Last saved </w:t></w:r>… SAVEDATE …
```

There are no `w:tabs` in the paragraph or its `Noga` style — the stops *are* the `w:ptab`
elements, which is why "the footer's tab stops are not honoured" is the wrong shape of the
problem: there are none to honour.

LibreOffice emits a plain tab character for every `w:ptab`. Its tokenizer model says so
directly, `sw/source/writerfilter/ooxml/model.xml:18204-18208`:

```xml
<resource name="CT_PTab" resource="Properties">
  <attribute name="alignment" tokenid="ooxml:CT_PTab_alignment"/>
  <attribute name="relativeTo" tokenid="ooxml:CT_PTab_relativeTo"/>
  <attribute name="leader" tokenid="ooxml:CT_PTab_leader"/>
  <action name="end" action="tab"/>
</resource>
```

and only afterwards, for a **left**-aligned one that follows text, `HandlePTab`
(`DomainMapper_Impl.cxx:5550-5604`) replaces that tab with a line break — returning
immediately for `center` and `right`. So the reference's three-part footer is three fields
separated by two ordinary tabs, landing on the `Noga` style's default stops.

Three documents in the lane carry a `w:ptab`: #008 (1), #013 (1), #139 (2).

### Where it lives in the source

- `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs:1810-1812` — `RunWalker`'s
  element switch has `"tab"`, `"sym"`, `"noBreakHyphen"`, `"br"` … and no `"ptab"`, so it
  falls to `default: Append(child, depth + 1)` and contributes nothing.
- `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxContentReader.cs:527` — the extraction side
  skips it explicitly: `case "separator" or "continuationSeparator" or "lastRenderedPageBreak" or "ptab": break;`.

### Proposed change

`patches/positional-tab.diff`. Both readers emit `\t` for a `w:ptab`; the layout reader
emits a line separator instead for `w:alignment="left"` following text, which is
`HandlePTab` restated. No new geometry: the character lands on the paragraph's existing
stops, which is what LibreOffice does.

### The probe that would refute me

A footer paragraph with **no** `w:tabs`, holding `A`, a `w:ptab w:alignment="right"`, and
`B`. If the reference puts `B` at the default tab stop, LibreOffice really is emitting a
plain tab and the patch matches it. If `B` sits flush at the right margin, LibreOffice is
honouring the alignment and the patch under-delivers (it would still be strictly better
than dropping the element).

### Confidence

**High** for the character; **medium** for the placement, because the probe above is the
one I could not run. Note the patch changes extracted text as well as the page: three
documents gain two or three tab characters, which can move `wc -w`.

---

## RC5 · A STYLEREF running head is answered from a document-final map

### What the pages show

#073 `231164_SystemDesignDocument`. Running heads, read out of the two PDFs' own text
layers:

| page | reference | ours |
|---|---|---|
| 3 | List of Figures | List of Tables |
| 4 | Introduction | External Interfaces |
| 5 | General Overview and Design Guidelines/Approach | External Interfaces |
| 6 | General Overview and Design Guidelines/Approach | External Interfaces |

Ours is constant. The constant is the document's **last** `Heading 2` (`External
Interfaces`) and its **last** `Front Matter Header` (`List of Tables`).

### What the document actually contains

```
word/header1.xml  <w:fldSimple w:instr=" STYLEREF  &quot;Front Matter Header&quot;  \* MERGEFORMAT ">
word/header2.xml  <w:fldSimple w:instr=" STYLEREF  &quot;Heading 2&quot;  \* MERGEFORMAT ">
word/header3.xml  <w:fldSimple w:instr=" STYLEREF  &quot;Back Matter Heading&quot;  \* MERGEFORMAT ">
```

No part switch, so `FieldInstructions.StyleReferenceName` accepts all three and we
substitute. The body has three sections and sixty headings; the running head changes
several times *inside* section 2.

### Where it lives in the source

- `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs:915` —
  `_styleText[styled] = mapped;` after each paragraph. Correct for a STYLEREF in the body.
- `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs:943-962` —
  `StyleReferenceText` answers from that map, "the most recent one … because the search is
  backwards from the field".
- `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxReader.cs:326` — `Layout()` runs
  `List<PageBlock> blocks = source.Read(body);` and only then `Paginated(source, body)`,
  which is where `Furniture()` reads the header parts. **By the time a header is read the
  body walk has finished**, so "the most recent one" is the document's last.
- The field's own value is per-page, exactly like `PageFieldKind.PageNumber`, and unlike
  that one it has no per-page substitution: `Layout/PageFields.cs` handles two kinds and a
  STYLEREF is not one of them.

Writer resolves it from the field's own node, scanning backwards
(`SwGetRefFieldType::FindAnchorRefStyleOther`), which for a running head means "the heading
in force on this page".

### Proposed change

`patches/styleref-running-head.diff` — a holding change, not the fix. `StyleReferenceText`
returns null while `InHeaderFooter`, so the producer's cached result stands. That is this
method's own stated policy ("a wrong substitution is worse than a stale one") and it is
what Word wrote for the page the field was last laid out on. On #073 it moves page 3 from
`List of Tables` to the reference's `List of Figures` and leaves pages 4–6 unchanged
(header2's cache is `External Interfaces`, the same string we compute).

The real fix is per-page and needs a file this lane does not own — see **Cross-lane
dependencies**.

### The probe that would refute me

Two `Heading 2` paragraphs a page apart, a `STYLEREF "Heading 2"` header, and a stale
cached result naming neither. If the reference's two pages print the two different headings
the per-page reading is right; if both print the same one, LibreOffice is resolving once
per section and the section-snapshot fix (cheap, in-lane) would be the whole answer.

### Confidence

**High** on the diagnosis — the two PDFs' text layers settle it. **Medium** on the holding
patch: it is strictly right on #073, but it trades a computed answer for a cached one
across the whole corpus, and a document whose cache is empty would lose text it currently
draws.

---

## RC6 · A table border's *style* is not modelled at all

### What the pages show

- #067 `195584360` — "The notice boxes are drawn with solid borders where the reference
  draws dotted ones — the border style is not being carried through."
- #145 `form_1123…` — "the dashed separators the reference draws between the numbered
  operating procedures are missing."
- #043 `AFS-050-004-F2_0i` — "the dotted in-cell separators sit differently."

### What the document actually contains

`form_1123_application_form_rvsm_spa.docx`, repeated 64 times:

```xml
<w:tcBorders><w:top w:val="single" w:sz="4" w:space="0" w:color="auto"/>
  <w:left w:val="single" w:sz="4" w:space="0" w:color="auto"/>
  <w:bottom w:val="dashed" w:sz="4" w:space="0" w:color="auto"/>
  <w:right w:val="single" w:sz="4" w:space="0" w:color="auto"/></w:tcBorders>
```

Census of non-solid `w:val` on any border element, over `document.xml`, `styles.xml` and
every header/footer of the 37:

```
PAT-047 …                       outset 392
A1. EASA Form 2                 dotted 158
195584360                       dotted 121, double 2
AFS-050-004-F2_0i               dotted  81, double 15
FO.FCTOA_.000129                dotted  82
form_1123_…_rvsm_spa            dashed  64
33004                           dotted   8
FO.FCTOA.00010                  dotted   6
TE.CAO.00125 / B11 / annex-B    thinThickSmallGap 3 / 2+1 thickThinSmallGap / 3
ECSS-E-ST-50-16C, PI-doc-2E     double 2 each
AC-150…, 231164, Agile_Arc      1 each (double / dashed / double)
```

Sixteen of the thirty-seven. All drawn as a solid line of the stated width.

### Where it lives in the source

- `dotnet/src/XRay.Office.WordProcessing/Layout/PageTable.cs:49` —
  `public readonly record struct TableBorder(Length Width, Colour Colour)`. There is no
  style member, so nothing downstream *can* draw a dash.
- `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.Tables.cs:929-949` —
  `Border()` reads `w:val` only to test it: `if (Word.Attribute(stated, "val") is null or "none" or "nil") return default(TableBorder);`
  and then reads `w:sz` and the colour. This is the "read but never consumed" pattern in its
  purest form: the attribute is parsed on every border in every table and discarded.
- `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.ParagraphBorders.cs:109`
  and `DocxLayoutSource.Tables.cs:945` do the same for paragraph and conditional borders.

### Proposed change

**None — cross-lane.** The reader half is mine; the type and the painter are not. See
**Cross-lane dependencies**.

### The probe that would refute me

A one-cell table with `w:bottom w:val="dashed" w:sz="4"`. If the reference's rule is dashed
and ours is solid, the diagnosis is confirmed with no ambiguity at all. (I did not run it;
the markup census plus the missing struct member is already conclusive about *our* side.)

### Confidence

**High.** Unknown: LibreOffice's dash geometry — `SvxBorderLineStyle::DASHED` has a
specific dash/gap ratio (`editeng/source/items/borderline.cxx`) that a reimplementation
would have to match, and `outset`, `thinThickSmallGap` and friends map onto Writer's own
line styles rather than onto a dash pattern. #013's 392 `outset` borders in particular are
a *double* line in Writer, not a dashed one.

---

## RC7 · `w:pgBorders` is not read anywhere

### What the pages show

- #010 `Case-Study-Heathrow-Airport` — "The reference draws a green double-rule page border
  around the whole sheet; XRay.Office draws none."
- #043 `AFS-050-004-F2_0i` — "The reference draws a thin border around the whole page;
  XRay.Office draws none."

### What the document actually contains

`Case-Study-Heathrow-Airport.docx`, in the section's `w:sectPr`:

```xml
<w:pgBorders w:offsetFrom="page">
  <w:top    w:val="single" w:sz="36" w:space="15" w:color="396533" w:shadow="1"/>
  <w:left   w:val="single" w:sz="36" w:space="15" w:color="396533" w:shadow="1"/>
  <w:bottom w:val="single" w:sz="36" w:space="15" w:color="396533" w:shadow="1"/>
  <w:right  w:val="single" w:sz="36" w:space="15" w:color="396533" w:shadow="1"/>
</w:pgBorders>
```

`AFS-050-004-F2_0i.docx`: the same element with `w:sz="4" w:space="24" w:color="auto"`.
`OM template for non-complex NCC operators` (#068) carries one too.

`git grep pgBorders -- dotnet/` returns **nothing**. Not parsed, not modelled, not drawn.

### Where it would live

- `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxPageGeometry.cs` reads the rest of
  `w:sectPr` and is the reader's home.
- `dotnet/src/XRay.Office.WordProcessing/Model/PageGeometry.cs` is where the four sides and
  the `w:offsetFrom` datum would be carried.
- The painter is not in this lane.

### Proposed change

**None — cross-lane.** Adding the reader alone would be dead code, which is the very
pattern this project keeps tripping over.

### Confidence

**High** that it is absent and that it is the whole of the missing rule on #010 and #043.
Note #010 also has two other faults in its case note (the logo overlap and the measure);
this root cause is only the border.

---

## RC8 · Refuted — the EASA footers are not a page-number offset, and ours is the better output

The case notes for #029 and #133 both propose "a page-numbering offset is not being
applied". **There is no `w:pgNumType` element anywhere in either document** — not in any
`w:sectPr`, and #133 has only one section. Grepping the part for the attribute before
blaming it, per the brief, costs a minute and it refutes the story.

What is actually happening, from the reference PDFs' own text layers:

```
B11. TE.CAO.00129 Experience logbook.docx (6 pages)
  reference: Page 3 of 6 · Page 3 of 6 · Page 3 of 6 · Page 3 of 6 · Page 3 of 6 · Page 3 of 6
  ours:      Page 1 of 6 · Page 2 of 6 · Page 3 of 6 · Page 4 of 6 · Page 5 of 6 · Page 6 of 6

A1. EASA Form 2.docx (7 pages)
  reference: Page 6 of 7 on every page
```

The reference prints a **constant**, and the constant is the producer's cached field
result — `<w:t>3</w:t>` and `<w:t>6</w:t>` respectively, sitting in the `w:fldChar
separate…end` span in `word/footer1.xml`. LibreOffice is not applying an offset; it is not
evaluating the field at all.

Why: both footers put their `PAGE`/`NUMPAGES` inside a DrawingML text box
(`mc:AlternateContent → wpg:wgp → wps:wsp → w:txbxContent`) that is **anchored in a
paragraph inside a `w:tbl` in the footer**. Writer's draw-layer outliner has no
page-number field at all — `SwDoc::CalcFieldValueHdl`
(`sw/source/core/doc/docdraw.cxx:562-621`) handles `SvxDateField`, `SvxURLField`,
`SdrMeasureField` and `SvxExtTimeField`, and answers `"?"` for anything else — so text that
does not become a Writer TextBox loses its fields and keeps the cached string.

The control is #075, `UG.CAO.00133 … Language.docx`, same house style, byte-identical shape
of `wpg:wgp / wps:wsp / w:txbxContent` down to the shape ids, cache `1 of 18` — and there
the reference prints `Page 1 of 18 … Page 5 of 18`, live. The one structural difference
between them is that #075's footer is plain paragraphs and #029's and #133's wrap the whole
thing in a `w:tbl`.

Word updates these fields; that is what the template is built for, and #075 proves
LibreOffice does too when it can. **Ours is the correct output on #029 and #133 and the
reference is the one that is stale.** No patch. Do not chase these two footers, and do not
add an offset that the files do not ask for.

### The probe that would refute me

The same footer text box twice: once directly in `w:ftr`, once inside a single-cell
`w:tbl` in `w:ftr`, both with a stale cached `PAGE`. If only the second freezes, the
"text box that fails to become a Writer TextBox" reading is right. If both freeze, #075 is
being drawn by some other path and I have the discriminator wrong — but either way the
conclusion (no offset; ours is right) stands.

---

## Not established

**#008 `hdss-bulletin-issue-285-25-june-2025` — a header drawn on pages that should have
none.** The document has three sections; section 1 names `header default → rId12`
(the decorative band) and `footer default`/`footer first`; section 2 is `w:type="continuous"`,
names `header default → rId15` and carries `<w:titlePg/>` **with no first-page header
reference**; section 3 names no header at all and inherits. We draw section 1's band plus
section 2's `HDSS Bulletin Issue 285` + `PAGE` on page 4; the reference draws no header
there. The `Issue 2854` run-together is faithful to the file (`header2.xml` has no
separator between the text and the `PAGE` field) — the reference simply does not draw that
header. Deciding this needs the page→section mapping, which lives in the paginator, not in
this lane. The missing `OFFICIAL` footer is a second symptom of the same slot resolution
(`footer1.xml` and `footer2.xml` both contain it).

**#089 — a yellow highlight dropped.** The run is a **single trailing space** at the end of
a paragraph: `<w:r><w:rPr>…<w:highlight w:val="yellow"/>…</w:rPr><w:t xml:space="preserve"> </w:t></w:r>`.
We read the highlight (it survives `RunsOf`'s uniform-paragraph shortcut, which tests
`style.Highlight is not null` explicitly) and lose it at line break, where a trailing space
is trimmed. That is line-breaking behaviour in `XRay.Office.Text`, not this lane.

**#139 — `SAVEDATE` 09:27 against the reference's 08:15.** We draw the cached
`04/09/2018 09:27`; LibreOffice re-evaluates from the package's `dcterms:modified` (the
same instant, in UTC). Matching it means treating `SAVEDATE`/`PRINTDATE` the way
`FILENAME` and `TITLE` are already treated — but `FieldInstructions.ConstantFieldOf` lives
at `dotnet/src/XRay.Office.WordProcessing/FieldInstructions.cs`, outside this lane's two
directories, so it is recorded and not patched.

**#111 — blue oval bullets drawn as black dots**, and its full-page pagination gap. Not
investigated; the bullet glyph is a picture bullet or a symbol-font slot and the pagination
gap is much larger than anything above.

## Reflow-only documents

These carry no defect this lane owns; their divergence is the advance-width and
table-sizing cascade recorded in `dotnet/CLAUDE.md`, and every one of their case notes says
so in its own words ("laid out wider", "rows slightly taller", "one extra row reaches this
page", "leading fractionally looser"):

#019, #028, #034, #038, #046, #050, #075, #085, #087, #090, #112, #116, #123, #150, #179,
and the page-count halves of #040, #068 and #192.

---

## Cross-lane dependencies

**1. `XRay.Office.WordProcessing/Layout/PageTable.cs` — `TableBorder` needs a line style.**
RC6, sixteen documents. `TableBorder(Length Width, Colour Colour)` at line 49 needs a third
member (an enum over OOXML's `w:val` mapped onto Writer's `SvxBorderLineStyle`), the
`Border()` readers in `Ooxml/DocxLayoutSource.Tables.cs:929` and
`Ooxml/DocxLayoutSource.ParagraphBorders.cs:109` need to fill it — that half is mine and I
will write it once the type exists — and the table/paragraph border painter needs to honour
it. `dashed`, `dotted` and `double` cover 14 of the 16; `outset` and `thinThickSmallGap`
are Writer double-line styles rather than dash patterns.

**2. `XRay.Office.WordProcessing/Layout/PageFields.cs` — a per-page STYLEREF.**
RC5. `PageFieldKind` has two members and the argument in its own doc comment for why —
"every other field … is a cross-reference the reader already resolved". A STYLEREF in a
running head is the counter-example: its value changes per page exactly as `PAGE`'s does.
It needs a third kind plus, for each page, the last paragraph text per style *at that
point* in the flow. The reader half — recording `(block index, text)` per style instead of
last-wins — is mine and is small; the substitution is not.

**3. `FieldInstructions.cs` (package root) — `SAVEDATE`/`PRINTDATE` re-evaluation.**
#139. `ConstantFieldOf` already re-evaluates `FILENAME` and `TITLE` for exactly this
reason; the modification timestamp is a fourth of the same kind and needs the `\@` picture
applied.

**4. A page-border painter.** RC7, three documents. The `w:pgBorders` reader and its model
belong in my two directories; nothing can draw the result today.
