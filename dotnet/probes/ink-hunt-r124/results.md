# The passing set, ranked on ink — and the two things it was hiding are both underlines

Round 124, `/home/user/wt-inkhunt`, branch `agent/inkhunt`, base `d98c8be24`, 2026-09-14.
The register was closed when this round opened: 5 established 26.2 defects, 55 fixed here, 11
confounds, 38 nil-reach and one row another round was closing. So the question was not *what is
on the list* but **what is wrong that nothing has ever put on the list**, and the instruction was
to hunt in the documents that PASS.

| | |
|---|---|
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**, `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| reference renderings | the **banked** leg of gate r122, `/home/user/gate-r122/ref`, 947 PDFs, drawn at `0fa883d0c` |
| our renderings | `dotnet/tools/XRay.Office.Cli` built in this worktree at `d98c8be24`, `SOURCE_DATE_EPOCH=1757462400`, one output directory per *document* |
| population | the **915** rows gate r122 calls `match` — words 329, slides 293, sheets 293 |
| C++ tree | `/home/user/libreoffice-core`, read only (C8: it is `27.2.0.0.alpha0+`, not 26.2's source, so every arm below is confirmed a second time against 26.2.4.2's own output) |
| `/usr/bin/soffice` | 24.2.7.2 — **not used for anything in this round** |

**Reusing the r122 reference bank is sound and is the point.** `d98c8be24` adds probe files only
and touches no source, so our side at this base is the side that bank was paired with; and
comparing against one *fixed* draw of the reference sidesteps the half of C11 that is run-to-run
noise. Where a single document's number carries a finding, that document's reference has been
rendered again — twice — and the quantity the finding rests on compared. §7.

**Seat numbers.** The brief named O69 as the next free seat. Round 123 landed while this round's
readers were running and took O69 and O70, so **every row proposed here was renumbered from O71
up at the coordinator's instruction**. There is no lost seat in the gap.

---

## 0. The instrument, and the two ways it had to be repaired first

`score-ink.py` sums `pdf-image-diff.py`'s per-page `|ink|%` at 512 px on the long edge — the same
column `probes/words-ink-r67`, `probes/ink-pass-r92`, `probes/slides-ink-r94` and
`probes/sheet-ink-r94` ranked on, so this ranking is comparable with theirs. Two repairs:

**Long documents are scored in 25-page chunks.** `pdf-image-diff.py` rasterises a whole document
into one temporary directory before it diffs anything; the corpus's largest workbook is **4372
pages** and that costs several gigabytes at once, against single-digit gigabytes free. Its `|ink|%`
is computed per page from that page's own two rasters and nothing crosses a page boundary, so
splitting the pair at the same page numbers and summing is the identical answer.
`chunk-equivalence.py` checks it rather than assuming it: on `171128IPAP.pptx`, **40 of 40 pages
agree exactly and the two sums are both 9.7700**. (The first, unchunked run of the scorer took
`/` from 6.9 GB to 3.0 GB free before it was killed; that is why this exists.)

**A page whose two renderings differ in SIZE is silently absent from the table.** The tool prints
`page size differs` for such a page and counts it major, and that line does not match the numeric
row parser — so it is dropped from any ink sum built by parsing the tool's output, this round's
included. That is not a defect of the tool but it is a blind spot in the ranking, and closing it
produced a finding of its own: §5.

`rank.py` joins the per-document table to a census of every corpus stem's mentions across all 154
probe write-ups, so a document three rounds have already opened can be told from one nobody has.
**509 of the 915 passing documents have been named somewhere before; 406 never have.**

---

## 1. The ranking

915 of 915 passing documents scored, none unscoreable, **3043.28** summed `|ink|%` in all.
`ranking.tsv` is the whole table; `per-page.tsv` is all 27 408 pages.

### By summed `|ink|%`

| # | document | track | pages | Σ\|ink\|% | per page | worst page | MAJOR | prior mentions |
|--:|---|---|--:|--:|--:|--:|--:|--:|
| 1 | `FAA 2025-26 Holdover Tables.docx` | words | 167 | **341.86** | 2.1775 | 13.12 (p127) | 33 | 45 |
| 2 | `24-25_FAA_Holdover_Tables.docx` | words | 155 | **334.19** | 2.3048 | 13.98 (p109) | 32 | 42 |
| 3 | `NAS-Infrastructure-Roadmaps-v16.0.pptx` | slides | 137 | 151.74 | 1.1076 | 5.53 (p8) | 49 | 48 |
| 4 | `FY2021-AIP-grants.xlsx` | sheets | 58 | 127.05 | 2.1905 | 4.61 (p25) | 30 | 4 |
| 5 | `AC-150-5370-10G-updated-201604.docx` | words | 696 | 113.16 | 0.1626 | 3.02 (p5) | 178 | 28 |
| 6 | `150-5370-10H.docx` | words | 727 | 109.22 | 0.1502 | 3.21 (p7) | **726** | 54 |
| 7 | `grants-2005.xls` | sheets | 201 | 96.11 | 0.4782 | 2.06 (p158) | 45 | 39 |
| 8 | `TK-Syllabus-Comparison-Document-v2.xlsx` | sheets | 1235 | 92.62 | 0.0750 | 6.71 (p20) | 28 | 32 |
| 9 | `CSJU List of Recipients of funds 2013-2020.xlsx` | sheets | 97 | 64.47 | 0.6646 | 8.95 (p41) | 9 | 13 |
| 10 | `atspp_pay_tables.xlsx` | sheets | 101 | 47.46 | 0.4699 | 3.22 (p66) | 15 | 6 |
| 11 | `tk-syllabus-comparison-document-v5.xlsx` | sheets | 855 | 46.53 | 0.0544 | 2.43 (p448) | 37 | 31 |
| 12 | `Reporting_responsibilities_matrix.pptx` | slides | 268 | 34.70 | 0.1295 | 1.28 (p138) | 2 | 25 |

**The head is one document twice.** Ranks 1 and 2 are two winters of the same FAA advisory
circular, and ranks 5 and 6 are two revisions of another, and ranks 8 and 11 two revisions of a
third. That is the shape the brief warned about, and it is why the round did not stop at the head.

### By `|ink|%` per page (≥ 3 pages), which a long document otherwise buries

| # | document | ext | pages | per page | worst page | prior |
|--:|---|---|--:|--:|--:|--:|
| 1 | `24-25_FAA_Holdover_Tables` | docx | 145 | **2.3048** | 13.98 | 42 |
| 2 | `FY2021-AIP-grants` | xlsx | 58 | 2.1905 | 4.61 | 4 |
| 3 | `FAA 2025-26 Holdover Tables` | docx | 157 | 2.1775 | 13.12 | 45 |
| 4 | `HC-Bulletin-template` | docx | 5 | 2.0960 | 7.02 | 13 |
| 5 | `RMI_…GettingOffOil` | **doc** | 6 | 1.8083 | 7.58 | 2 |
| 6 | `May 25 bulletin focus on carers in the workplace` | docx | 4 | 1.7000 | 3.87 | 8 |
| 7 | `100_Lime_and_Lemon_Data-Driven_Chart_for_PowerPoint` | pptx | 5 | 1.6480 | 2.79 | **0** |
| 8 | `programs contact list as of 07-01-10` | **xls** | 3 | 1.5500 | 2.07 | 3 |
| 9 | `part-145-approval list (1)` | docx | 8 | 1.4012 | 2.93 | 5 |
| 10 | `PK_FlugzeugeStricken` | **doc** | 7 | 1.2543 | 6.52 | 1 |

### By worst single page

`24-25_FAA_Holdover` p109 **13.98**, `FAA 2025-26 Holdover` p127 **13.12**, `CSJU` p41 **8.95**,
`fm-provider-service-measures` p5 8.88, `RMI_…GettingOffOil` p2 7.58, `HC-Bulletin-template` p2
7.02, `TK-Syllabus-v2` p20 6.71, `PK_FlugzeugeStricken` p2 6.52.

### The sample actually sent to readers

Ten pages, two per format, drawn from all three rankings and weighted towards documents earlier
rounds have not opened:

| document | page | ext | track |
|---|--:|---|---|
| `FAA 2025-26 Holdover Tables` | 127 | docx | words |
| `150-5370-10H` | 7 | docx | words |
| `PK_FlugzeugeStricken` | 2 | doc | words |
| `RMI_…GettingOffOil` | 2 | doc | words |
| `FY2021-AIP-grants` | 25 | xlsx | sheets |
| `CSJU List of Recipients of funds 2013-2020` | 41 | xlsx | sheets |
| `orbus_togaf_tool_csq` | 7 | xls | sheets |
| `grants-2005` | 158 | xls | sheets |
| `NAS-Infrastructure-Roadmaps-v16.0` | 8 | pptx | slides |
| `ws_prod-…-European-Safety-Strategy-Initiative` | 8 | ppt | slides |

---

## 2. Who read the pages, and what that is worth

**This container has no `Task`/subagent tool — but this round did not have to work around that.**
The coordinating session can spawn blind readers, and it ran **ten, one page each, in parallel,
before I had opened any of them**. They were given the composed labelled pair and nothing else: no
document name in the brief, no numbers, no statement of what was suspected, and an instruction not
to read any project file or run any command. Their reports were relayed verbatim.

So for once the readings in this write-up are **not** contaminated by their author. What *is* mine
and contaminated is everything in §3 onward that I looked at after their reports arrived; each of
those is corroborated by arithmetic that does not depend on any picture — content-stream
operators, 26.2.4.2's own `--convert-to fodt`, Escher records read out of the file, and censuses
over the whole passing set.

Three readers volunteered the single most useful thing in the set: their own resolution limit.
Reader 2 went further and supplied a **control**: it argued that the reference half of its page
*does* render a one-pixel-class stroke — the header rule, plainly visible — so the absence of the
TOC underlines in that half is a real absence and not a thin-line dropout. That is the reasoning
`page-vision` asks for and it is why §3 was worked first.

Two readers disagreed with each other about which side's text is *darker* (reader 6 said ours,
reader 7 said the reference), on different documents, and three independently named the compositor
as a candidate. **That is a refutation, not a finding**, and it is recorded in §6 as one.

---

## 3. SEAT O71 — an underline does not span a tab, and it costs 1437 pages

Reader 2, on `150-5370-10H.docx` page 7, unprompted: *"the reference draws one continuous
horizontal rule spanning the full header width between `12/21/2018` and `AC 150/5370-10H`; ours
draws two short separate underlines, one under each, with the gap blank."*

### What the operators say

Page 100 of the same document, and pages 300 and 500 are character for character the same:

| | rules drawn in the header band |
|---|---|
| this tree | `(72.00, 117.56)` and `(465.28, 540.00)` — two fills, **120.28 pt** |
| 26.2.4.2 | `(72.00, 117.50)`, `(117.55, 305.95)`, `(306.00, 465.30)`, `(465.30, 540.00)` — four abutting strokes, **468.00 pt**, contiguous from 72 to 540 |

**347.72 pt per page** of underline that the reference draws and we do not, and the two spans we
miss are exactly the document's two header tabs. 26.2.4.2's own view of the file says so:
`--convert-to fodt` gives the header as

```xml
<text:h text:style-name="P1" text:outline-level="4">12/21/2018<text:tab/>
  <text:span text:style-name="T1"><text:tab/></text:span>AC 150/5370-10H</text:h>
```

under `Header_20_Char`, which carries `style:text-underline-style="solid"`.

### The rule, in the reference's own source

`SwTabPortion::Paint` (`sw/source/core/text/txttab.cxx`:626-641) — the comment is the project's:

```cpp
// Tabs should be underlined at once
if( rInf.GetFont()->IsPaintBlank() )
{
    const SwTwips nCharWidth = rInf.GetTextSize(OUString(' ')).Width();
    if( nCharWidth )
    {
        sal_Int32 nChar = Width() / nCharWidth;
        rInf.DrawText(RepeatedUChar(' ', nChar), *this, 0, nChar, true);
    }
}
```

A tab is painted as a run of literal spaces in the current font, so whatever line the font carries
is drawn across it. `SwGluePortion::Paint` (`porglue.cxx`:68-75) does the identical thing for
justification glue and `porrst.cxx`:298 for the hole portion. And the switch is not "underline":

```cpp
m_bPaintBlank = ( LINESTYLE_NONE != m_aFont.GetUnderline()
             || LINESTYLE_NONE != m_aFont.GetOverline()
             || STRIKEOUT_NONE != m_aFont.GetStrikeout() )
             && !m_aFont.IsWordLineMode();
```

`SwFntObj`'s constructor, `sw/source/core/txtnode/fntcache.cxx`:106-109. **So it is underline,
overline and strikethrough alike, and `IsWordLineMode` — Word's `w:u w:val="words"` and RTF's
`\ulw` — is exactly the attribute that turns it off.** A reader that implements "underline the
runs" rather than "underline the line" gets the common case right and this one wrong.

### Reach, measured on what is drawn, with the mirror direction as its base rate

`gap-census.py` looks for one specific shape and nothing else: two of our rules on one baseline
with clear air between them, and a reference rule bridging at least 80 % of that air. It counts
fills and strokes on **both** sides, because C16 says a census that looks for one shape scores the
other side's rules as absent, and it reports horizontal **cover** rather than object count,
because C16's second half says one hyperlink is one stroke there and as many fills as it has rich
segments here.

Over the 329 passing words documents:

| | documents | pages | length |
|---|--:|--:|--:|
| **the reference bridges a gap of ours** | **10** | **1437** | **501 248 pt** |
| we bridge a gap of the reference | 7 | 16 | 1 584 pt |

The mirror direction is the base rate and it is **300× smaller in pages and in length**. The two
700-page FAA advisory circulars carry 1420 of the 1437 pages at 347 pt each; the other eight are
`exhibit-06---technical-architecture-template.docx`, `review-welsh-government-communications-…`,
`150_5335_5a.doc`, `RobertQ_Service.doc`, `1_tpr_template__from_fy14_.docx`, `195584360.docx`,
`TE.CAO.00125 …` and `template---tpr-technical-progress-report-…`.

`exhibit-06` page 4 is the second clean witness and a different document family:

| | |
|---|---|
| ours | `(54.00, 59.58)`, `(70.60, 157.82)`, `(551.92, 557.50)` |
| 26.2.4.2 | `(54.10, 59.60)`, `(70.60, 157.80)`, **`(157.80, 551.90)`**, `(551.95, 557.45)` |

394.10 pt of tab, underlined there and blank here.

**Not O64 and not O70.** Both of those are about a rule's *thickness* and *offset*; this is about
whether a rule exists over a span at all, and the spans in question carry no glyph. Round 123's
offset work does not touch it.

---

## 4. SEAT O72 — a contents entry keeps the file's `Hyperlink` style, and the reference throws it away

Reader 2, same page, first finding: *"OURS draws an underline under effectively every one of ~43
TOC lines, at both indent levels; the REFERENCE draws none."*

### The operators

`150-5370-10H.docx` page 7: **58 fills in ours, 4 strokes in the reference** — and those four are
§3's header rule. Every one of our fills tracks the *entry text* of a contents line and stops
before the leader dots; the reference has nothing on any of those baselines.

### What the file says, and what the reference does with it

Every entry is `<w:hyperlink w:anchor="_Toc…"><w:r><w:rPr><w:rStyle w:val="Hyperlink"/></w:rPr>…`,
and the document's own `Hyperlink` character style states `<w:color w:val="auto"/>` **and
`<w:u w:val="single"/>`**. Word underlines it. We underline it. 26.2.4.2 does not, because it does
not render the field's stored result at all — it imports the `TOC` field as a live Writer index and
regenerates the entries. Its own `--convert-to fodt` shows both halves:

```xml
<text:table-of-content-entry-template text:outline-level="1" text:style-name="Contents_20_1">
  <text:index-entry-link-start text:style-name="Index_20_Link"/>
  …
<style:style style:name="Index_20_Link" style:display-name="Index Link" style:family="text"/>
```

`Index Link` is Writer's `STR_POOLCHR_TOXJUMP` pool character style (`sw/inc/strings.hrc`:48,
`sw/source/core/doc/SwStyleNameMapper.cxx`:693) and in this document's own resolved style table it
carries **no `<style:text-properties>` at all**. The regenerated entry is

```xml
<text:a xlink:href="#_Toc533170052" text:style-name="Index_20_Link"
        text:visited-style-name="Index_20_Link">Part 1 – General Contract Provisions<text:span
        text:style-name="T16"><text:tab/>1</text:span></text:a>
```

— no span carrying the file's `Hyperlink` style anywhere in it.

The assignment is explicit in the importer:
`sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx`:9355-9360,

```cpp
if (IsInTOC())
{
    OUString sDisplayName(u"Index Link"_ustr);
    xCrsrProperties->setPropertyValue(u"VisitedCharStyleName"_ustr, uno::Any(sDisplayName));
    xCrsrProperties->setPropertyValue(u"UnvisitedCharStyleName"_ustr, uno::Any(sDisplayName));
}
```

and **it is not an OOXML rule**: `sw/source/filter/ww8/ww8par5.cxx`:2334, 3362-3363 and 3653 give a
WW8 index link the same `Index Link`, so a `.doc` behaves the same way.

### Reach, and the base-rate control that makes it convincing

`toc-page-census.py` identifies a contents page from the *drawn text* — five or more runs of five
or more leader dots — rather than from the markup, and compares total rule cover on those pages.
Over the passing set:

| | |
|---|---|
| documents with at least one contents page | **40** |
| contents pages in all | **60** |
| rule cover on them, ours | **262 704 pt** |
| rule cover on them, 26.2.4.2 | **77 630 pt** |
| ours draws > 200 pt more there | **34 of 40** |
| the reference draws > 200 pt more there | 3 of 40 |
| the reference draws **nothing at all** there while we draw > 200 pt | **14 of 40** |

**And the same test on the same documents' ordinary pages reverses:** ours more in **5 of 40**,
the reference more in **19 of 40**. The excess is a property of contents pages, not of these
documents. (`toc-census.tsv` is the declaration census behind the selection — 43 of the passing
DOCX state both a `TOC` field and a `Hyperlink` `rStyle` — and it is an upper bound, which is why
the drawn measurement above is the one quoted.)

### What this does NOT license

It licenses suppressing the field result's character formatting on a `TOC` entry. It does **not**
license regenerating the index: the reference recomputes entry text and page numbers from the live
document, and where a stored TOC is stale the two will differ in *content*. Nothing here measures
that, and §5 is a reminder that pagination inside these documents is not settled.

---

## 5. Ten pages of two documents are the wrong SHAPE, and no gate column can see it

The scorer's dropped rows (§0) pointed at this and `page-size-census.py` measured it directly out
of the two PDFs' page boxes, with no rendering at all. Over all 915 passing documents:

**Exactly 2 documents, 20 pages, and all 20 are orientation flips.**

| | | |
|---|---|---|
| `FAA 2025-26 Holdover Tables.docx` | 10 of 167 pages | ours 792×612 where the reference is 612×792, and the reverse |
| `24-25_FAA_Holdover_Tables.docx` | 10 of 155 pages | the same |

Both documents pass the gate — 167/167 and 155/155 pages, glyphs 336754/335613 and 300664/299593,
inside the band. The orientation runs are identical in *structure* and displaced by exactly one
page:

```
FAA 2025-26  ours: … L71-84 P85-93 L94 P95 L96-97 P98 L99-121 P122 L123-125 P126 L127-157 P158-167
             ref:  … L71-83 P84-92 L93   P94 L95-96 P97 L98-120 P121 L122-124 P125 L126-157 P158-167
24-25        ours: … L66-78 P79-86 L87   P88 L89-90 P91 L92-111 P112 L113-115 P116 L117-143 P144-155
             ref:  … L66-79 P80-87 L88   P89 L90-91 P92 L93-112 P113 L114-116 P117 L118-143 P144-155
```

Everything agrees to page 70 (respectively 65); then **one section holds one page more for us and
one page fewer in the other document**, and every downstream section boundary is displaced by one
until a long final landscape run absorbs it and the two re-synchronise by the last page.

So the cause is a break position inside one section and the *symptom* is ten pages of the wrong
paper. Reader 4, given page 127 blind, reported exactly this and nothing else: ours prints
`Page A-38` and carries the whole of `TABLE ADJ-19`; the reference prints `Page A-39` and is 75 %
white. **Diagnosed and left** — the break position is not identified here, and a page-by-page
pairing of these two documents is meaningless until it is. This is why the coordinator's advice
to re-pair by content rather than by index is right and is recorded here for the next round.

*One observation of reader 4's is about our half alone and is cheap for someone to check: our
NOTES list on that page numbers `1, 2, 3, 4, 5` and then `516, 517, 518, 519, 520, 521, 522`
where the table's own superscripts imply `6`-`12`.*

---

## 6. SEATS O73 and O74 — a `.ppt` trapezoid, upside down and at the wrong angle

Reader 9, on `ws_prod-…-European-Safety-Strategy-Initiative.ppt` slide 8, blind: *"the trapezoid's
taper runs the opposite way … and it is not a clean mirror: ours tapers less"*, with the frame the
same in both and the gradient following the shape rather than moving independently. It named the
stored adjustment value as the decisive measurement. It was right on all three counts.

### The geometry, read out of the two content streams

26.2.4.2 draws the shape as a clip path and paints ~62 full-width gradient bands through it:

```
284.627 40.791 m  702.539 40.791 l  531.751 403.172 l  455.386 403.172 l  284.627 40.791 l h  W* n
```

We draw a stroked quadrilateral. On the same 417.912 × 362.381 pt rectangle:

| | full-width edge | narrow edge | inset each side |
|---|---|--:|--:|
| 26.2.4.2 | **bottom** | 76.365 pt | **170.774** |
| this tree | **top** | 236.7 pt | 90.6 |

### Both halves resolve exactly, with no free parameter

The file states **`DFF_Prop_adjustValue` (Escher property 327) = 8826**, read out of the
`msofbtOPT` record beside each of the deck's four `msofbtSp` records of shape type 8. LibreOffice's
`msoTrapezoid` (`svx/source/customshapes/EnhancedCustomShapeGeometry.cxx`:345-387) is

```cpp
const SvxMSDffVertPair mso_sptTrapezoidVert[] =      // adjustment1 : 0 - 10800
{   { 0, 0 }, { 21600, 0 }, {0 MSO_I, 21600 }, { 1 MSO_I, 21600 } };
const SvxMSDffCalculationData mso_sptTrapezoidCalc[] =
{   { 0x8000, { 21600, 0, DFF_Prop_adjustValue } },  // 21600 - adj
    { 0x2000, { DFF_Prop_adjustValue, 0, 0 } }, … };
```

so the narrow edge runs from `adj` to `21600 − adj` **across the shape's own width**:
`(21600 − 2×8826)/21600 × 417.912 = 76.38 pt` against the reference's measured **76.365**.

Our own preset table has the DrawingML `trapezoid`
(`src/XRay.Office.Ooxml/DrawingML/PresetShapeGeometry.txt`:7061-7078): `a adj val 25000`,
`x2 = ss·a/100000`, and a path `m l b / l x2 t / l x3 t / l r b`. Two independent consequences,
and they are two separate defects:

**O73 — the stated adjustment never arrives.**
`src/XRay.Office.Presentations/MsBinary/PptShapeGeometry.cs`:334-337 is

```csharp
public static int? Adjustment(ushort shapeType, int value)
    => PresetOf(shapeType) is "roundRect" or "triangle"
        ? (int)((long)value * DrawingMlViewBox / AdjustmentViewBox)
        : null;
```

Every other preset answers `null` and the layouter falls back to the DrawingML default. Here that
is `ss × 25000/100000 = 362.381 × 0.25 = 90.6 pt`, which is the inset we drew, to the tenth.
*And a straight rescale of the number would still be wrong*: `8826 × 100000/21600 = 40861` gives
`ss × 0.40861 = 148.1 pt`, because Escher measures the inset across the **width** and DrawingML
across `ss = min(w, h)`. The conversion is per preset, not a constant.

**O74 — the two preset tables are vertical mirrors and we map them by name.** Escher's
`mso_sptTrapezoid` puts the full-width edge at `y = 0` (the shape's top); DrawingML's `trapezoid`
puts it at `b` (the bottom). We read the file's `fFlipV` correctly — three of this deck's four
trapezoids state it, `PptSlideLayout.cs`:984 applies it — and apply it to the wrong base, so the
drawn shape comes out inverted whether the flag is set or not.

### Reach

`escher-adjust-census.py` walks every OLE2 stream of every `.ppt`, `.doc` and `.xls` in the passing
set for an `msofbtSp`/`msofbtOPT` pair carrying property 327:

**158 shapes in 10 documents state an adjustment this tree discards**, all `.ppt` (10 of the 49
passing `.ppt`), against 12 documents stating one at all:

| MSO type | preset | shapes | documents |
|--:|---|--:|--:|
| 37 | `curvedConnector2` | 50 | 1 |
| 38 | `curvedConnector3` | 45 | 2 |
| 32 | `straightConnector1` | 28 | 2 |
| 63 | `wedgeEllipseCallout` | 10 | 2 |
| 34 | `bentConnector3` | 9 | 1 |
| **8** | **`trapezoid`** | **5** | **2** |
| others (62, 33, 39, 136, 88, 35, 66, 144) | | 11 | 6 |

O74's own reach is narrower and is stated separately: **5 Escher trapezoids in 2 documents**
(`ws_prod-…-European-Safety-Strategy-Initiative.ppt` and
`outlook_of_nigerian_pension_sector.ppt`), **3 of the 5 stating `fFlipV`**. The mirror is a
property of the two tables, so it costs every one of the five whether or not it states an
adjustment.

---

## 7. SEAT O75 — two adjacent inline pictures can never be split across two lines

Reader 1, on `RMI_…GettingOffOil.doc` page 2: *"ours omits an image the reference draws … ours
places body text on this page the reference does not."* Half right, and the half that is wrong is
the more useful half.

**Both sides draw two images.** They are placed differently:

| | upper image | lower image |
|---|---|---|
| 26.2.4.2 | y 433.65-692.40 | y 159.30-453.75 |
| this tree | y 418.05-676.90 | y 418.05-712.60 |

Our two share a bottom edge to the hundredth of a point and overlap almost completely — one
picture on top of the other, which reads exactly like a missing picture. Their **heights are
right** (258.85 against 258.75, 294.55 against 294.45); only the line they sit on is wrong.

26.2.4.2's own `--convert-to fodt` gives the cause: the two are **`text:anchor-type="as-char"`
frames, 6.4945 in and 6.5 in wide, in one `<text:p>`** whose measure is 6.5 in. Together they are
twice the measure; the reference breaks the line between them and we do not.

**The seat is the anchor character's line-break class.** Every word-processing reader here emits
`U+0001` for an inline object (`PageContent.cs`:709, and `Ww8DocumentReader.Layout.cs`:929 for this
format). `LineBreakProperties.Tables.cs` classifies `U+0001` as **`LineBreakClass.CM`**, which is
correct per UAX #14 — and UAX #14's LB9 (*do not break a combining character sequence*) then
forbids a break **before** a CM. Two adjacent inline objects are two adjacent CM characters, so
there is no break opportunity between them at all and the second object can never leave the first
object's line however wide the pair is. Writer has no such rule: an as-char picture is a
`SwFlyCntPortion` and the breaker decides on width.

This is the other half of the rule `CLAUDE.md` already records from the PES cover-art fix — *an
inline object widens every prefix past its boundary, so a picture wider than the measure is placed
anyway on a line it starts and pushed onto the next line when anything precedes it*. That fix gave
the object its anchor character; this says the anchor character is the wrong class to break on.

**Reach: 1 of 329, against a base rate of 0.** `baseline-pair-census.py` counts pairs of drawn
images that share a bottom edge to 0.5 pt and overlap by at least half the smaller — the shape an
unbroken line makes, and one a watermark or a layered logo does not. **Ours: 5 documents. The
reference: 0.** Inspecting all five by hand, only `RMI_…GettingOffOil.doc` is this mechanism; the
other four are §8's separate duplicate-draw class. So the honest figure is one witness with a zero
base rate, and the mechanism is general to DOCX, DOC, ODT and RTF alike because all four emit the
same character.

---

## 8. Characterised, not seated

- **`075_Storyboard_Template_Fillable_Format.docx` and `079_Storyboard_Template_Simple_Format.docx`
  draw a nested group's members ~8.85× too wide and mostly off the paper.** Page 1: the reference
  draws four frames of 175 × 152 pt at x 17-192, stacked at y 127/296/465/635; we draw them
  1548 pt wide from x = −1111 on a 612 pt page. The outer group's `a:ext` 7190740 × 9953415 against
  `a:chExt` 7190806 × 10402785 is near identity, so the factor comes from a **nested** `wpg:wgp`;
  `probes/words-group-transform` modelled the outer fit and its census does not list either
  document. **Both are `prior_mentions = 0` — nothing has ever looked at them.** Their ink is only
  0.71 and 1.67 because the frames are pale, which is precisely why an ink ranking alone would
  never have found this.
- **An image drawn at about a third of its size, and drawn twice.** `A1. EASA Form 2.docx` p7:
  reference one image 442,376-520,426 (78 × 50); ours two identical 421,487-445,512 (24 × 25).
  Same shape on `eTAR_External_Web_tool_Tip_Sheet_mh.docx` p4, `5709.16 ch.40_mgfinal.docx` p1 and
  `FO.FCTOA_.000129 ….docx` p3. **4 documents**, mechanism not identified.
- **`grants-2005.xls` p158: we draw ~27 pale yellow bars on a page the reference leaves blank, and
  neither side draws a single glyph there** (reader 3, who volunteered the glyph-count identity as
  the useful part of its reading). Reader 3 also could not separate ~54 half-height banded rows
  from ~27 full-height fills drawn at half the row height. Not worked.
- **`PK_FlugzeugeStricken.doc` p2: the reference draws a grey masthead, a heraldic corner mark, a
  red rule, a full-height grey right sidebar and an `Impressum` block; we draw none of them** — yet
  our heading sits at the same y, so the space is reserved and only the paint is missing (reader 8).
  Our running head and footer are absent from the reference's half, but its masthead covers exactly
  that band, so occlusion is consistent with the pixels and the claim is unsettled. `prior = 1`.
- **`NAS-Infrastructure-Roadmaps-v16.0.pptx` p8** (reader 10): a paler and shorter background wash,
  one caption wrapping to two lines where the reference sets it on one, and a ~20 px green box where
  the reference draws a ~2 px stroke. Its sibling `-Weather` is O67 (`NOT WORK`); whether this is
  the same seat is not established. Note the reader's own list of things that are **identical**,
  including a broken wrap with a stranded `t` that both sides reproduce.
- **`CSJU List of Recipients of funds 2013-2020.xlsx` p41**: ours 47 data rows, the reference 3, both
  terminating on the same record (reader 5). Rows-per-page accumulation over pages 1-40; pagination,
  like §5, and it wants re-pairing by content first.

## 9. Measured and discarded — recorded so they are not re-derived

- **Off-page ink is NOT a class in which we are worse.** `offpage-census.py` sums the area of every
  drawn item's box falling outside the media box, both sides, over all 915: **ours overhangs by more
  than 10× and 2000 sq pt in 79 documents; the reference does in 73.** That is at chance and the
  classifier is worthless as a filter. Its *magnitude* tail is not — `NAS-Infrastructure-Roadmaps-v16.0`
  11 984 584 against 211 426 sq pt, and the two Storyboards against zero — so read the extremes, never
  the count.
- **"One side's text is darker" is a compositor artefact until a 1:1 crop says otherwise.** Reader 6
  said ours darker on `FY2021-AIP-grants` p25; reader 7 said the reference darker on
  `orbus_togaf_tool_csq` p7; three readers named downsampling as a candidate for it unprompted. Two
  blind readers disagreeing on the *direction* of the same effect on different documents is evidence
  it is a property of the pipeline.
- **`FY2021-AIP-grants.xlsx` — rank 2 by ink per page — is a null on structure.** Reader 6, blind:
  same ~16 columns at the same x fractions, the same two-line header stacking breaking at the same
  words, the same ~90-100 rows in the same order, the same tall anomalous Louisiana row, spot-checked
  dollar figures agreeing. Its 2.19 per page is not a layout difference and is not explained here.
- **`orbus_togaf_tool_csq.xls` p7 is a near-null**, and reader 7 supplied the strongest single
  corroboration in the set for our text metrics: **line breaking inside table cells is identical, the
  same words at the same line-ends in the same cells**, and both sides over-run the same trailing words
  across the same rules without clipping or growing the row. Where we reproduce the reference's own
  oddity, that is a match.
- **`solo-census.py` is kept but should not be quoted.** It counts rules one side draws on a baseline
  the other leaves empty, and it fires on **163 of 232** documents against the reference's 215 — a
  y-band tolerance of 1.2 pt makes a small vertical offset read as two absences. It is C9's lesson
  again: a classifier with no measured false-positive rate is a hypothesis about the corpus. The
  useful instruments are `gap-census.py` and `toc-page-census.py`, both of which carry their own
  reversal as a control.
- **The general "we overlap images" class is not enriched either**: 13 documents against the
  reference's 52. Only the narrower baseline-sharing shape of §7 is (5 against 0).

## 10. The C11 control

For each document a finding rests on, the reference was rendered **twice more**, fresh, same binary,
same UTC day, into separate user profiles, and the quantity the finding rests on was compared —
not the character count, which C11 now records as a lower bound on the reference's instability.

| document | quantity | run a | run b | the r122 bank |
|---|---|---|---|---|
| `150-5370-10H.docx` | rule count and length on pages 7/100/300/500 | 22, **4679.30 pt** | 22, **4679.30 pt** | 22, **4679.30 pt** |
| `150-5370-10H.docx` | `pdftotext` md5 | `92bf795c…` | `92bf795c…` | `92bf795c…` |
| `ws_prod-…-Safety-Strategy-Initiative.ppt` | the trapezoid's clip path, slide 8 | `284.627 40.791 m 702.539 40.791 l 531.751 403.172 l 455.386 403.172 l` | identical | identical |

Both PDFs differ byte for byte between runs (the creation date), and neither of the two quantities
above moves at all.

## 11. Tests and confinement

**Nothing in `dotnet/src` was changed this round.** The brief asked for finding and characterising
rather than fixing, and no fix here was small enough to be both obviously correct and fully
measured. So there is no confinement leg to run and no test to move: the binary that produced every
figure is the one `dotnet build XRay.Office.slnx` gives at `d98c8be24`, **0 warnings, 0 errors**.

## 12. The scripts and the banked data

| | |
|---|---|
| `sweep-ours.py` | our half of all 947, one output directory per *document* |
| `score-ink.py` | per-page and per-document `\|ink\|%` against the banked reference, in 25-page chunks |
| `chunk-equivalence.py`, `chunk-equivalence.txt` | the check that chunking changes no number |
| `rank.py`, `ranking.tsv` | the ranking, with each document's prior-mention count beside it |
| `per-page.tsv`, `per-doc.tsv` | all 27 408 scored pages and all 915 documents |
| `page-size-census.py`, `page-size.tsv` | §5, per-page paper size, no rendering |
| `rule-census.py`, `rule-census-words.tsv` | drawn rule length per document, both shapes counted |
| `gap-census.py`, `gap-words.tsv` | §3, a rule bridging a gap between two of the other side's |
| `toc-census.py`, `toc-census.tsv` | §4's declaration census (upper bound) |
| `toc-page-census.py`, `toc-pages.tsv` | §4's drawn measurement, with the non-contents-page control |
| `escher-adjust-census.py`, `escher-adjust.tsv` | §6, Escher adjustments this tree discards |
| `baseline-pair-census.py`, `baseline-pairs.tsv` | §7, images sharing a baseline |
| `image-overlap-census.py`, `overlap-all.tsv` | §9, the wider overlap class that is at chance |
| `offpage-census.py`, `offpage.tsv` | §9, ink outside the paper, at chance by count |
| `solo-census.py`, `solo-words.tsv` | §9, **kept as a refuted instrument**, not to be quoted |
| `prior-mentions.tsv` | every passing document's mention count across all probe write-ups |
| `toc-page7-pair.png`, `trapezoid-slide8-pair.png`, `aschar-page2-pair.png` | the three composed pairs that produced findings |
