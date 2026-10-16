# L2-docx-layout — findings

28 documents, grouped into **six root causes plus three hand-offs**. Every measurement below is
against the sweep's own reference PDFs at `/data/bench/lo/`, or against the `soffice` installed on
this container, which is the same binary that produced them.

## The thing to settle before anything else: which LibreOffice is ground truth

    $ soffice --version
    LibreOffice 24.2.7.2 420(Build:2)
    $ python3 -c "import fitz; print(fitz.open('/data/bench/lo/…/out.pdf').metadata['producer'])"
    LibreOffice 24.2

`dotnet/CLAUDE.md` records that the project's container moved to **26.2.4.2** and that ground truth
genuinely moved with it (47 of 200 words reference page counts). This container is back on 24.2.7.2,
and the corpus half of the sweep was rendered by it. Two of the layout rules in the tree were
deliberately calibrated against 26.2.4.2 and are *wrong for 24.2.7.2* — both are recorded in the
source as such, one of them explicitly ("that was measured against 24.2.7.2", `Paginator.cs:1394`).

They are §A and §B. They are the two largest faults in this lane by reach, and each is a revert of a
change the project made on purpose. I have measured both on this container and written the patches,
but the decision to apply them is a decision about which reference the gate tracks, and belongs to
whoever owns that. Everything from §C down is version-independent.

---

## A · The `w:trHeight` floor is raised by the row's border and cell margins; LibreOffice 24.2 raises it by neither

**Documents.** All 23 of the lane's 28 that contain a table (`w:trHeight` row counts in brackets):
009 (48/48), 015 (1516/1518), 018 (1357/1359), 020 (29), 026 (30/30), 037 (182), 041 (28), 044 (6),
057 (56/56), 058 (48/48), 061 (17/17), 063 (12), 069 (48/48), 093 (144/144), 094 (21), 117 (95),
141 (28/28), 151 (25), 154 (19), 160 (22), 185 (155), 191 (48/48). It is the sole cause on 141, 057,
058, 069, 160, 117, 151, 061, 093 and the larger half of the cause on 009, 026, 041, 044.

### 1 · What the pages show

`141` is the clean case: 20 columns × 28 rows of empty graph paper, identical column pitch on both
sides, no content anywhere near the row heights. Read out of the two PDFs' vector paths:

| | LO 24.2 | X-Ray.Office | declared |
|---|---:|---:|---:|
| grid width | 526.10 pt | 526.10 pt | — |
| grid height | 675.20 pt | 689.20 pt | 675.20 pt |
| row pitch, rows 1-14 | 24.5 23.2 24.5 23.1 … | 25.0 23.65 25.0 23.65 … | 24.5 23.15 … |

Every row is **exactly +0.50 pt**, which is the `TableGrid` style's `w:sz="4"` border. Three more of
the same shape, taking the whole grid's first-rule-to-last-rule span:

| doc | rows | declared sum | reference | ours | per row | border |
|---|---:|---:|---:|---:|---:|---|
| 141 | 28 | 675.20 pt | **675.20** | 689.20 | +0.500 | `w:sz 4` from `TableGrid` |
| 057 | 56 | 666.65 | **666.60** | 694.65 | +0.501 | `w:sz 4` `tblBorders` |
| 058 | 48 | 653.60 | **653.60** | 689.60 | +0.750 | `w:sz 4` from `TableGrid` |
| 069 | 48 | 629.60 | **629.60** | 661.09 | +0.656 | `w:sz 2` on the cells |
| 009 | 48 | 423.00 | **423.00** | 693.60 | (see §C) | — |
| 026 | 30 | 265.50 | **265.50** | 417.25 | (see §C) | — |

The reference sits on the declared sum to 0.05 pt in all six. Row by row on 009 and 026 the
reference's pitch reproduces each `w:trHeight` individually (9.75, 9.0, 9.0, 7.5, 9.0 … against
declared 9.75, 9.0, 9.0, 7.5, 9.0 …), so this is the rule and not a coincidence of totals.

Everywhere else in the lane it shows as "rows a shade taller" — 117's deliverables table, 151's form
tables, 061's fleet tables, 093's compliance table, 160's field grid, 041's approvals table (one
organisation pushed to page 2), 044's Q&A row clipped at the page bottom.

### 2 · What the documents contain

Nothing exotic: `<w:trPr><w:trHeight w:val="490"/></w:trPr>` with no `w:hRule`, which is `atLeast`
(`MeasureHandler.cxx:71` sets `FIX` for the string `exact` and nothing else). 141 states no
`tblBorders` at all and takes `w:sz="4"` from `w:tblStyle w:val="TableGrid"`; 069 states `w:sz="18"`
on the table and `w:sz="2"` on its interior cells. Cell margins are Word's default 108/0/108/0 except
where a `w:tcMar` says otherwise. I checked: **there is no `w:hRule="exact"` on any of these rows**,
so "we are reading the rule wrong" is refuted before it is proposed.

### 3 · Where it lives

`dotnet/src/XRay.Office.WordProcessing/Layout/TableLayouter.cs:199-206`

```csharp
Length topInset = table.MinHeightIncludesInsets ? TopInset(table.Rows[row]) : Length.Zero;
Length bottomInset = table.MinHeightIncludesInsets ? BottomInset(table.Rows[row]) : Length.Zero;

heights[row] = table.Rows[row].HasExactHeight
    ? Length.Max(Length.Zero, table.Rows[row].MinHeight + bottomInset)
    : Length.Max(heights[row], table.Rows[row].MinHeight + topInset + bottomInset)
      + BorderHeight(table.Rows[row]);
```

This is a faithful port of `lcl_CalcMinRowHeight` and `lcl_GetFixedRowHeight`
(`sw/source/core/layout/tabfrm.cxx`:5058, 5070-5097) under `DocumentSettingId::MIN_ROW_HEIGHT_INCL_BORDER`.
The port is correct. **The behaviour is not in 24.2.7.2.**

### 4 · The measurement

The project's own probe, unmodified, re-run against the installed 24.2.7.2 — six rows of one 10 pt
line under a 24.00 pt floor, sweeping the grid's `w:sz`:

    $ python3 dotnet/probes/words-pagination-01/row-min-height-border.py <scratch> <XRay.Office.Cli>
      w:sz  border pt      rule  LibreOffice     ours    diff
         0       0.00   atLeast        24.00    24.00   +0.00
         4       0.50   atLeast        24.00    24.50   -0.50
         8       1.00   atLeast        24.00    25.00   -1.00
        16       2.00   atLeast        24.00    26.00   -2.00
        24       3.00   atLeast        24.00    27.00   -3.00
        16       2.00     exact        24.00    24.00   +0.00

`dotnet/probes/words-pagination-01/results.md` records the same script reading
24.00 / 24.50 / 25.00 / 26.00 / 27.00 out of the *reference* on 26.2.4.2. Same script, same
document, different `soffice`: **the reference moved, the code did not.**

A second probe of my own adds the margin half (`w:tcMar` top and bottom of 100 twips) and the FAA
shape the 26.2 reading was calibrated on:

    case                     LO     ours     diff
    sz0 mar0              24.00    24.00    +0.00
    sz8 mar0              24.00    25.00    -1.00
    sz0 tcMar100          24.00    34.00   -10.00
    sz8 tcMar100          24.00    35.00   -11.00
    sz8 tcTop23 (trH 397) 19.85    22.00    -2.15      <- 19.85 pt is the bare 397 twips
    sz0 tcTop23 (trH 397) 19.85    21.00    -1.15
    exact sz8 mar100      24.00    29.00    -5.00

`TableLayouter.cs:191` says of that FAA row "397 + (20 + 23) + 0 = 440 twips = 22.00 pt, which is the
reference's row pitch to the hundredth". On 24.2.7.2 the reference's pitch is **19.85**, and 22.00 is
ours.

### 5 · The proposed change

`patches/row-height-floor.diff`. The floor becomes the whole row height, borders and margins
included, and the border is charged against the *content* instead:

```csharp
heights[row] = table.Rows[row].HasExactHeight
    ? Length.Max(Length.Zero, table.Rows[row].MinHeight)
    : Length.Max(heights[row] + BorderHeight(table.Rows[row]), table.Rows[row].MinHeight);
```

It reproduces all thirteen probe rows above and leaves content-bound rows untouched (probe
`hm0_x`: LO 105.20 pt over eight rows, ours 105.20, both before and after). `TopInset` and
`BottomInset` are deleted with it — they had no other caller and `EnforceCodeStyleInBuild` is on.

### 6 · The probe that would refute me

The one already run: `row-min-height-border.py` at `w:sz="24"`. If LibreOffice answers 27.00 there,
the border does belong on the floor and this whole section is wrong. It answers 24.00 on 24.2.7.2 and
27.00 on 26.2.4.2, which is not a refutation of either reading — it is the statement that the two
references disagree and one has to be chosen.

The *second* probe worth running before applying: render 015 and 018 with the patch and count pages.
`dotnet/probes/words-pagination-01/results.md` records this exact change, in the other direction,
moving `FAA 2025-26 Holdover Tables.docx` from 154 to 185 pages and `24-25_FAA_Holdover_Tables.docx`
from 142 to 154. Against the 24.2 references of **154** and **141**, reverting predicts 167 → ~154
(a match) and 155 → ~142 (one over). If it does not land there, the row rule is not what is driving
those two documents and I have over-claimed.

### 7 · Confidence, and what I did not establish

**High** that 24.2.7.2 does not add the border or the margins, and that the patch reproduces it.
**Medium** on whether the project wants this: it is a deliberate revert of round `words-pagination-01`
defect 2, whose own reach measurement was "85 of 200 words renderings change". I did not build or
test — the brief forbids it — so every page-count number here is a prediction.

---

## B · A `nextPage` section break keeps its space-before at every compatibility mode; 24.2 collapses it at mode ≥ 15

**Documents.** Every one with more than one section: 185 (83 `sectPr`), 015 (36), 018 (36), 093 (10),
063 (6), 117 (5), 037 (4), 011 (2), 121 (2). Worth 20 pt of space-before — the exact figure depends
on the document — at the top of each page that opens a section.

### 1 · What the pages show

This is a vertical-only fault and the images show it as "we fit more per page". 070 is the clearest:
identical content, and on pages 2-6 our lowest ink sits at y = 698.8 / 697.0 / 686.2 / 685.6 / 677.2
against the reference's 643.6 / 686.8 / 660.4 / 658.0 / 625.6 — up to 55 pt more on the page, with no
per-line difference anywhere. 015 and 018 are the pagination cases: 167 pages against 154, 155 against
141, drifting apart beyond the five sampled pages.

### 2 · What the documents contain

Ordinary `nextPage` section breaks: `<w:p><w:pPr>…<w:sectPr><w:pgSz …/><w:pgMar …/></w:sectPr></w:pPr></w:p>`.
015 and 018 alternate portrait and landscape across theirs (`pgSz 12240×15840` and `15840×12240`), which
is why the geometry-change case was worth probing separately.

### 3 · Where it lives

`dotnet/src/XRay.Office.WordProcessing/Layout/Paginator.cs:1395-1402`

```csharp
bool keepsSpaceHere =
    column == 0
    && (pages.Count == 0
        || (paragraph.Format.StartsNewPage && !_options.CollapsesUpperAtPageTop)
        || (pageIsSectionFirst
            && geometry.Break is not (SectionBreak.Continuous or SectionBreak.NewColumn)));
```

The third clause is ungated: a section break keeps the space whatever the mode. Its own comment says
where it came from — *"This supersedes the note on `CollapsesUpperAtPageTop` claiming a plain section
break sets no page description and collapses like any other; that was measured against 24.2.7.2."*

### 4 · The measurement

Nine synthetics, 20 pt of space-before, 72 pt top margin, reading the first word of page two, on the
installed 24.2.7.2:

    kind        mode 15   mode 12   no compatSetting
    plain         72.03     92.03     92.03
    landscape     72.03     92.03     92.03
    titlePg       72.03     92.03     92.03
    ours          92.44     92.44     92.44

The landscape variant changes `pgSz` across the break and the `titlePg` variant adds `w:titlePg`;
both are inert. The mode is the whole rule — which is what the other two explicit breaks already do,
and where we already agree. `dotnet/probes/words-r11/mk-topofpage-spacing.py`, re-run unmodified,
gives twelve more rows, eleven of which match and one of which is this one:

    probe-automatic{,-compat12,-compat14,-nocompat}   LO 72.35   ours 72.35
    probe-pagebreakbefore                             LO 72.35   ours 72.35
    probe-pagebreakbefore-compat{12,14},-nocompat     LO 92.35   ours 92.35
    probe-leading-br, -prevafter, probe-firstpage     LO 72.35   ours 72.35
    probe-sectionbreak                                LO 72.35   ours 92.35   <-

### 5 · The proposed change

`patches/section-break-space-before.diff` — gate the section clause on the same flag the
page-break clause already uses:

```csharp
bool keepsSpaceHere =
    column == 0
    && (pages.Count == 0
        || (!_options.CollapsesUpperAtPageTop
            && (paragraph.Format.StartsNewPage
                || (pageIsSectionFirst
                    && geometry.Break is not (SectionBreak.Continuous or SectionBreak.NewColumn)))));
```

This reproduces all 21 probe rows above.

### 6 · The probe that would refute me

`probe-sectionbreak` at mode 12. If 24.2 answered 72.03 there, the rule would be "a section break
never keeps the space" and the clause should be deleted rather than gated. It answers 92.03.

### 7 · Confidence

**High** on the measurement. **Medium** on the version decision, exactly as §A. I did not establish
how much of 015/018/185's page-count gap is this rather than §A; they compound, and only a build
separates them.

---

## C · `w:hideMark` on every cell of a blank row makes `w:trHeight` an exact height

**Documents.** 009 and 026 — the two worst pages in the lane (SSIM 0.438 and 0.512). No other
document in the lane has a single row where every cell carries `w:hideMark` and no cell holds text.

### 1 · What the pages show

The grid is drawn about 1.6× too tall. 026's 30 rows span 265.50 pt in the reference and 417.25 pt
in ours; 009's 48 rows span 423.00 against 693.60. On 009 the oversized table pushes the "Title:" and
"Date:" ruled fields off the sheet entirely, which is the `content-missing` tag.

### 2 · What the documents contain

Rows of 180/195/165/150 twips (9.00/9.75/8.25/7.50 pt) whose every cell is

```xml
<w:tc><w:tcPr><w:tcW w:w="234" w:type="dxa"/><w:tcBorders>…</w:tcBorders>
  <w:shd w:val="clear" w:color="auto" w:fill="auto"/><w:noWrap/>
  <w:vAlign w:val="bottom"/><w:hideMark/></w:tcPr>
  <w:p><w:pPr><w:spacing w:after="0" w:line="240" w:lineRule="auto"/>…</w:pPr>
    <w:r>…<w:t> </w:t></w:r></w:p></w:tc>
```

— a single non-breaking space at the document default of 11 pt, whose line is 13.4 pt, comfortably
taller than the 9 pt row. Both documents carry `w:hideMark` on **1728** and **1800** cells, on every
row. 009's cells have no `w:tcBorders` at all, 026's have them, and both behave identically, so the
border is not what is happening here.

### 3 · Where it lives

`sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx:1157`

```cpp
if (lcl_hideMarks(m_aCellProperties[nRow]) && lcl_emptyRow(m_aTableRanges, nRow))
{
    // We have CellHideMark on all cells, and also all cells are empty:
    // Force the row height to be exactly as specified, and not just as the minimum suggestion.
    rRow->Insert(PROP_SIZE_TYPE, uno::Any(text::SizeType::FIX));
}
```

`lcl_hideMarks` (:1027) requires the property on **every** cell and no `w:vMerge` anywhere in the row.
Nothing in X-Ray.Office reads `w:hideMark`: `grep -r hideMark dotnet/src` returns nothing. The reader
that would set the flag is `Ooxml/DocxLayoutSource.Tables.cs`, which already computes
`PageTableRow.HasExactHeight` from `w:hRule` — **cross-lane**, see the section at the end.

### 4 · The measurement

026 itself, ablated one element at a time and re-rendered through the installed `soffice` (declared
row sum 265.50 pt):

| variant | reference span |
|---|---:|
| baseline | **265.50** |
| every `<w:hideMark/>` deleted | 417.30 |
| `w:hideMark` kept on half the cells | 417.30 |
| the non-breaking space replaced by `X` | 417.30 |
| the whole run deleted | 265.50 |
| the non-breaking space replaced by a preserved ASCII space | 265.50 |
| two non-breaking spaces instead of one | 265.50 |
| `<w:tblpPr>` deleted | 265.50 |
| `<w:noWrap/>` deleted | 265.50 |
| `<w:vAlign w:val="bottom"/>` deleted | 265.50 |

417.30 is what X-Ray.Office produces (417.25). So: `w:hideMark` on **all** cells is necessary and the
"all" is load-bearing; whitespace-only content counts as empty and one visible character does not;
positioning, wrapping and vertical alignment are inert.

### 5 · The proposed change

**No patch in this lane.** The whole change is one expression in the reader:

```csharp
HasExactHeight = hRule is "exact"
    || (AllCellsHideMark(row) && NoCellHasVisibleText(row) && NoCellIsVerticallyMerged(row))
```

With §A applied, an exact row is laid out at exactly `w:trHeight`, which is 265.50 and 423.00.

### 6 · The probe that would refute me

The ablation above is the probe, and it left one thing unexplained that a reviewer should know about:
built **from scratch** rather than by editing 026, the same markup does not reproduce. A minimal
package (no `styles.xml`, no `settings.xml`) with `w:hideMark` and a non-breaking space reads 111.6 pt
where the floor is 72.0; the *same document body* dropped into 026's own package reads **72.0**. So
the emptiness test is sensitive to something in the package I did not isolate. Since every corpus
document that matters is a real package, the rule as stated is safe for them, but "a non-breaking
space counts as empty" should be treated as *observed on these documents* rather than as LibreOffice's
general rule — `lcl_emptyRow` compares UNO text ranges and plainly does not read the text.

### 7 · Confidence

**High** for 009 and 026. **Low** on the general emptiness predicate. A conservative implementation
would require the cell to hold no `w:t` at all; that still fixes nothing, because these cells hold one.
Requiring "no non-whitespace character" is what the ablation supports.

---

## D · Where a table's left edge goes: `w:tblpX` is never read, and an absent `w:tblInd` at mode ≤ 14 skips the padding subtraction

**Documents.** 191 (both faults), 154 and 160 (the second only).

### 1 · What the pages show

191's grid sits **35.05 pt** to the right of the reference's: reference left rule at x = 36.20,
ours at 71.25, with the two grids the same width (521.4 pt) to 0.05 pt. Its "Title: / Date:" rule and
its TemplateHub logo go missing and the document runs to two pages against the reference's one.
154's three inner tables sit **5.2 pt** right of the reference's (66.60 → 71.80), same width.

### 2 · What the documents contain

191: `<w:tblpPr w:leftFromText="180" w:rightFromText="180" w:vertAnchor="page" w:horzAnchor="margin"
w:tblpX="-594" …/>`, no `w:tblInd`, `compatibilityMode` 14, left margin 1440 twips, first cell border
`w:sz="12"`. The reference's left rule is then

    72.00 − 29.70 (w:tblpX) − 5.40 (a cell padding) − 0.75 (half the border) = 36.15   vs 36.20 measured

and ours is `72.00 − 0.75 = 71.25`, which is exactly what you get by dropping the first two terms.
154: `compatibilityMode` 12, three tables with `w:tblInd w:w="-743"` (we agree with the reference on
those, 29.15 against 29.20) and three with **no** `w:tblInd` (we are 5.2 pt right).

### 3 · Where it lives

`Ooxml/DocxLayoutSource.Tables.cs`:

- `Table()` reads `tblpY` and `vertAnchor` and **never reads `w:tblpX`** — `grep -rn tblpX dotnet/src`
  returns nothing. `PageTable` has `VerticalOffset` and `VerticalOrigin` and no horizontal pair.
  A positioned table stating a distance rather than a `w:tblpXSpec` therefore falls through
  `HorizontalPositionOf` to `null` and is placed at its indent. Three of 191's siblings state a spec
  and are fine; 191 states a distance.
- `LeftEdge()`, the mode ≤ 14 branch:

  ```csharp
  // Only an indent the document actually states makes Word measure to the text. Without one Word
  // invents an indent of its own, and what it invents behaves like the modern rule.
  Length distance = indent is null
      ? border / 2
      : Length.Max(border / 2, first?.Padding.Left ?? Length.Zero);
  ```

  The carve-out for an absent `w:tblInd` is not LibreOffice's behaviour on 24.2.

Both are **cross-lane**.

### 4 · The measurement

Twelve synthetics on 24.2.7.2 — one two-column table, 72 pt left margin, `w:sz="4"` borders — reading
the leftmost vertical rule:

    mode  tblInd       LO     ours    diff
      12    None     66.60    71.75   -5.15
      12       0     66.60    66.60   +0.00
      12     100     71.60    71.60   +0.00
      14    None     66.60    71.75   -5.15
      14       0     66.60    66.60   +0.00
      14     100     71.60    71.60   +0.00
      15    None     72.30    72.25   +0.05
      15       0     72.30    72.25   +0.05
      15     100     77.30    77.25   +0.05
    None    None     72.30    71.75   +0.55
    None       0     72.30    66.60   +5.70
    None     100     77.30    71.60   +5.70

Two findings, not one. The absent-`w:tblInd` carve-out is wrong at modes 12 and 14 — LibreOffice
subtracts the cell padding whether or not the indent is stated. And an **absent `compatibilityMode`
behaves like mode 15**, not like 12; `LeftEdge`'s comment says "mode 14 or less, which is also what an
absent `compatibilityMode` means" and cites `SettingsTable.cxx:637`, and the rendering disagrees.
That second half reaches 063, the one document in the lane with no `compatSetting` at all.

### 5 · The proposed change

**No patch in this lane** — both halves are in `Ooxml/`. In outline:

- delete the `indent is null` carve-out, so the mode ≤ 14 branch always subtracts
  `max(border/2, first cell's left padding)`;
- decide the compatibility default from the rendering rather than from `SettingsTable.cxx`, i.e. treat
  an absent `compatibilityMode` as the modern rule;
- add `w:tblpX` + `w:horzAnchor` to `PageTable` beside the existing vertical pair, and let
  `PageTable.LeftWithin` use it — that half is mine and I will write it once the reader can feed it.

### 6 · The probe that would refute me

The table above is the probe; the row that would break it is `mode 12 / tblInd 0`, where a
"the padding is subtracted only when the indent is negative" reading would predict 71.60. It reads
66.60. For 191 specifically: set `w:tblpX="0"` and re-render — if the grid does not move to
x ≈ 66, `w:tblpX` is not the term I think it is.

### 7 · Confidence

**High** for the measurement and for 154. **Medium** for 191, where I have arithmetic that lands
within 0.05 pt but have not rendered the changed document.

---

## E · A dxa `w:tblW` is ignored whenever every `w:gridCol` states a width

**Documents.** 026 only. I checked all 28: it is the single table in the lane whose `w:tblW` is a
`dxa` that disagrees with its grid sum.

### 1 · What the pages show

026's grid is 708.70 pt wide in ours and 704.10 in the reference, over 60 columns — 4.6 pt, or one
extra sixteenth of a cell.

### 2 · What the document contains

`<w:tblW w:w="14081" w:type="dxa"/>` over a `w:tblGrid` whose 60 `w:gridCol` widths sum to **14174**.
14081 twips is 704.05 pt and 14174 is 708.70. The reference draws 704.10; we draw 708.70. Both sides
land on their number to 0.05 pt, and they are different numbers.

### 3 · Where it lives

`Ooxml/DocxLayoutSource.Tables.cs`, `Fit()`:

```csharp
if (declared.All(width => width is not null)) return null;
```

A table that sizes every column gets no `TableColumnFit`, and `PageTable.WidthsWithin` then returns
`ColumnWidths` untouched — the grid sum. `DomainMapperTableManager::sprm`
(`DomainMapperTableManager.cxx`:180-190) sets `TABLE_WIDTH` from `w:tblW` whenever it is a non-zero
`dxa`, and `endOfRowAction` (:647) falls back to the grid sum **only** when it is not
(`if(!m_nTableWidth && !pTableGrid->empty())`). The grid never reaches Writer as widths at all — it
becomes relative `TableColumnSeparator`s (:700-760), which is already what `TableColumnFit` models.
So the stated width is read and thrown away: the fourth instance in this project of a property that
is parsed and never consumed.

### 4 · The measurement

The two PDFs, above. Independent of §A and §C, which are vertical.

### 5 · The proposed change

**No patch in this lane.** `Fit()` should build a `TableColumnFit` whenever a `dxa` `w:tblW`
disagrees with the grid sum, with `TableWidth` set to the stated width and `IsAuto` all false;
`TableColumnFit.ResolveWord` already restates the grid as separators at a given total and needs
nothing new. Alternatively `PageTable` gains a `DeclaredWidth` and `WidthsWithin` scales to it — that
half is mine, the setter is not.

### 6 · The probe that would refute me

A one-row table with `w:tblW w:w="4000" w:type="dxa"` over a grid of `2500 + 2500`. If LibreOffice
draws it 250 pt wide the stated width wins; if 5000 twips, the grid does. I did not run it — the
026 measurement already separates the two hypotheses by 4.6 pt and lands on 704.05.

### 7 · Confidence

**High**, small reach.

---

## F · Anchored shapes are painted in the reverse of LibreOffice's order

**Documents.** 024. Its page is 52 anchored `wps` shapes and not one table or paragraph border.

### 1 · What the pages show

The tag on this case says the page comes out empty. It does not: **every shape is drawn and every one
is in the right place**, and the order is inverted, so the last thing painted — a full-page dark
rectangle — covers the rest. Page 1's eight vector fills, in paint order:

    LibreOffice                                     X-Ray.Office
    0  dark grey  [0.0, 0.0, 843.0, 594.0]          0  red      [558.2, 122.2, 778.0, 166.5]
    1  white      [37.8, 36.0, 805.1, 555.1]        1  red      [311.5, 122.2, 531.2, 166.5]
    2  grey       [63.7, 122.2, 283.5, 528.8]       2  red      [ 63.0, 122.2, 282.8, 166.5]
    3  grey       [311.1, 122.2, 531.0, 526.5]      3  grey     [559.5, 122.2, 779.2, 525.0]
    4  grey       [559.4, 122.2, 779.2, 525.0]      4  grey     [311.2, 122.2, 531.0, 526.5]
    5  red        [ 63.0, 122.2, 282.8, 166.5]      5  grey     [ 63.8, 122.2, 283.5, 528.8]
    6  red        [311.4, 122.2, 531.2, 166.5]      6  white    [ 38.7, 36.0, 805.9, 555.0]
    7  red        [558.2, 122.2, 778.0, 166.5]      7  dark grey[  0.0,  0.0, 843.0, 594.0]

Exact reverses, rectangle for rectangle, to a tenth of a point. Both sides also carry 29 words of
text, which is why the ink metric reads 1.32× rather than 0 — the page is not empty, it is buried.

### 2 · What the document contains

52 `<mc:AlternateContent><mc:Choice Requires="wps">` blocks, every anchor `behindDoc="0"`,
`positionV relativeFrom="paragraph"`, `positionH` from `margin`, `column` and `page`, and a
`relativeHeight` on each: `251668480, 251704320, 251702272, 251701248, 251700224, 251699200,
251697152, 251695104, …`. The three red bars are document anchors 5, 6, 7 and LibreOffice paints them
**last**; we paint them first.

### 3 · Where it lives

`grep -rn "relativeHeight" dotnet/src` returns **nothing** — the z-order key is not read anywhere in
the tree. Paint order is `page.Frames`' list order, consumed by `Layout/PageDrawing.cs:73-88` (which
correctly splits `BehindText` first) and built by `Layout/FrameLayout.cs:199-260` in
`paragraph.Frames` order, which `Ooxml/DocxLayoutSource.FramesOf` builds in document order.

### 4 · What I could not establish

Why the emitted order is reversed. Document order would give reds *after* greys, which is
LibreOffice's answer, not ours; and sorting by `relativeHeight` ascending or descending gives neither.
Separating those needs an instrumented run, which means a build, which the brief forbids.

### 5 · The proposed change

**No patch.** The Layout half — a `PageFrame.ZOrder` and an ordering in `PageDrawing.Draw` — would be
inert until `Ooxml/DocxFrames.ReadAll` reads `wp:anchor/@relativeHeight`, and shipping an inert half
of a fix I cannot test would be worse than describing it. Recorded as a cross-lane dependency.

### 6 · The probe that would refute me

Three overlapping anchored rectangles in one paragraph — red, then green, then blue in document
order, with `relativeHeight` in the opposite order — rendered both ways. It says in one page whether
LibreOffice's key is the document order or the attribute, and whether ours is reversed or unordered.

### 7 · Confidence

**High** that the two orders are exact reverses and that all the ink is present. **Low** on the seat.

---

## G · Two defects I did not seat

### 011 — content sits 14.7 pt high at the top of every page after the first

The lane brief's reading of this document — *"the boxed Sample IEP Goal table and the body text below
it both sit further right and run wider"* — **is refuted by the geometry.** There is no table in the
document at all (`w:tbl` count: 0); the box is a `w:pict` text box, and its rectangle measures

    reference  x 58.55 … 547.20   (width 488.65)   y  51.70 … 321.00   (height 269.30)
    ours       x 57.60 … 546.25   (width 488.65)   y  36.00 … 305.35   (height 269.35)

— the same width to 0.00 pt and the same height to 0.05, sitting 0.95 pt left and **15.70 pt high**.
Word counts and line breaks on the compared page are identical (541 both). The pattern repeats:
page 3 opens at y 50.70 against our 36.00, page 5 at 54.80 against 40.10, page 6 at 65.40 against
36.00, and 36.00 is exactly the `w:pgMar w:top="720"`. So the reference leaves space above the first
block of a page and we leave none. It is not §B — I probed the four break kinds at four modes and we
agree on eleven of twelve — and the first block here is an anchored text box rather than a paragraph,
which is the part I ran out of budget to chase. **This is a real defect in this lane and it is open.**

### 037 — third-level list indent, and a footnote set as a hanging block

`(i) (ii) (iii)` items sit further right than the reference's, and the footnote marker is separated
from its text by a tab where the reference runs the superscript into the text. `Layout/ListLabel.cs`
and `Layout/NoteNumbering.cs` are both mine; I did not measure either. Open.

---

## Hand-offs

### To L1 — 097, 140, 121

All three have **identical page geometry** on every compared page and differ only by words crossing a
line or page boundary, which is the advance divergence `dotnet/CLAUDE.md` seats in grid-fitted vs.
unhinted advances:

- **097**: 325 words against 324 on page 1, every page's ink from y 72.0 to 695.2 / 705.0 / 498.3 on
  *both* sides to 0.1 pt. Page 2 opens "of the system's…" against "the system's…" — one word.
- **140**: identical word counts on all five pages; our lowest ink is 0.6 to 1.2 pt below the
  reference's, with no structural difference anywhere.
- **121**: identical word counts on all three pages; page 3 sits 4.0 pt low as a single step.

None of these is a table, an indent or a break. Not mine, and no patch proposed.

### Confirmed `lo-broken`

- **094** — confirmed by eye against the pair: LibreOffice loses the timeline's "Assigned To" column,
  the Week #1-#3 header bands and the first `Person #1 / Research` row, leaving a grey strip; we draw
  the whole table. **Ours is the better output.** Its `w:tblW="15586"` matches its grid sum exactly and
  its 21 rows all state a `w:trHeight`, so §A still moves it a little — file it, do not chase it.
- **154** — confirmed: LibreOffice overprints the "BUSINESS CASE" title onto the Document Control band.
  The width difference the tag mentions is §D above.

### Not geometry

- **020** — the colour half (a style's blue inherited where the reference resolves to automatic) is
  not this lane's. The geometry half is §A: 29 of its 30 rows state a `w:trHeight` and its table
  x-range matches the reference's to 0.03 pt, so "the form table is set wider" is not what the PDFs say.
- **063** — the journal title missing from the masthead and the page number moving side are content
  and field faults, not geometry. Its tables are `w:tblW w:type="pct"` and positioned; it is also the
  lane's only document with no `compatibilityMode`, which §D's second half touches.

---

## Cross-lane dependencies

All four are in `dotnet/src/XRay.Office.WordProcessing/Ooxml/`, which this lane does not own. Each is
measured above; none has a patch here.

1. **`Ooxml/DocxLayoutSource.Tables.cs`** — set `PageTableRow.HasExactHeight` when every cell of a row
   carries `w:hideMark`, no cell holds a non-whitespace character and no cell is vertically merged
   (§C). Needs §A applied to have its intended effect. Fixes 009 and 026.
2. **`Ooxml/DocxLayoutSource.Tables.cs`, `LeftEdge`** — drop the `indent is null` carve-out in the
   mode ≤ 14 branch, and treat an absent `compatibilityMode` as the modern rule (§D). Fixes 154,
   part of 191, touches 063.
3. **`Ooxml/DocxLayoutSource.Tables.cs`, `Table`/`Fit`** — read `w:tblpX` and `w:horzAnchor` into a new
   horizontal pair on `PageTable` (§D), and honour a `dxa` `w:tblW` that disagrees with the grid sum
   (§E). I will write the `PageTable`/`LeftWithin`/`WidthsWithin` half in a follow-up once the reader
   can feed it. Fixes 191 and 026's width.
4. **`Ooxml/DocxFrames.cs`** — read `wp:anchor/@relativeHeight` onto `PageFrame` so that
   `Layout/PageDrawing.cs` can order by it (§F). Fixes 024, whose page is currently unreadable.

## Patches

| file | root cause | files touched | applies to HEAD |
|---|---|---|---|
| `patches/row-height-floor.diff` | §A | `Layout/TableLayouter.cs` | `git apply --check` clean |
| `patches/section-break-space-before.diff` | §B | `Layout/Paginator.cs` | `git apply --check` clean |

Both were verified with `git apply --check` against the checkout and **nothing was applied, built or
tested** — the checkout is untouched.
