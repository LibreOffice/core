# Round 146 — O83 / O84 / O85: one rule, three faces

**Reference binary:** `/opt/libreoffice26.2/program/soffice` — LibreOffice **26.2.4.2**
(the tarball under `/opt`, not `/usr/bin/soffice`, which is 24.2.7.2 and is not the target).
**Source tree read:** `/home/user/libreoffice-core`, `configure.ac` declares **27.2.0.0.alpha0+**,
a single bulk import — *not* the reference binary's source. Every source reading below is a
hypothesis marked **[src]**; every measurement against 26.2.4.2's own output is marked **[bin]**.

Probe directory: `dotnet/probes/tablerow-r146/`.

**Status: complete.** Written section by section as each was established, not at the end.

---

## 0. The question

Three seats, all about where a table row's horizontal border sits and what it costs in height:

* **O83** — a row split across a page has both parts half a border too high; the grid line at the
  split is drawn twice, 0.25 pt apart.
* **O84** — a row's height charges the border its cells *state*; the reference charges the
  *resolved* one.
* **O85** — a row whose cells state borders of different widths has its bands aligned at their
  **top** edge by the reference and centred on the grid line by this tree.

## 1. They are ONE defect, and O85 is the accurate statement of it

**One rule accounts for all three seats.** It has two halves, and both halves are the *same*
statement about where a horizontal table border lives:

> A horizontal border between two table rows occupies a band whose **top edge sits on the row
> boundary** and which hangs **downwards into the row below**. Its width is the **resolved**
> border — the wider of the two edges facing across that boundary, after the cell's own
> `w:tcBorders` has replaced whatever the table's `w:tblBorders` stated. The row **below** pays
> for the whole of that band in its height; the row **above** pays nothing for it.

* **O85** is that rule seen in the *ink* — hence "aligned at their TOP edge". It is the accurate
  one of the three statements.
* **O84** is the same rule seen in the *height*, and the seat understates it. The reference does
  not merely charge the resolved border rather than the stated one; it charges the **whole** of
  the resolved border to the **lower** row and **nothing** to the upper one. "A row whose cells
  all nil an edge is half a border short" is a description of a mid-point, not of the rule.
* **O83** is the same rule at a page cut, where this tree centres the band as it does everywhere
  else, and the reference top-aligns it as it does everywhere else. The 0.25 pt of the seat is
  half of `review-welsh-…`'s 0.5 pt border — the same half-width the centred/top-aligned
  difference produces at every other boundary in the document.

The three were seated separately because each was found through a different instrument (a gap
census, a row-height pairing and a bridge census). Measured directly they are one thing.

---

## 2. The rule in the C++ tree [src]

Read out of `/home/user/libreoffice-core` (27.2.0.0.alpha0+, **not** the reference binary's
source). Four citations, and they are the whole of it.

**(a) Horizontal table borders are drawn `RefMode::Begin`; vertical ones are drawn centred.**

`sw/source/core/layout/paintfrm.cxx`:3061-3064, in `SwTabFramePainter::Insert`:

```cpp
aL.SetRefMode( svx::frame::RefMode::Centered );
aR.SetRefMode( svx::frame::RefMode::Centered );
aT.SetRefMode( !bVert ? svx::frame::RefMode::Begin : svx::frame::RefMode::End );
aB.SetRefMode( !bVert ? svx::frame::RefMode::Begin : svx::frame::RefMode::End );
```

`RefMode::Begin` is documented at `include/svx/framelink.hxx`:40-45 — *"The reference points
specify the begin of the frame border width. The result is that horizontal lines are drawn
below … the reference points."* It is implemented at
`svx/source/sdr/primitive2d/sdrframeborderprimitive2d.cxx`:110-123, which sets
`mfRefModeOffset = +fStyleWidth * 0.5` for `Begin` and `0.0` for `Centered`, and
:603 adds that offset along the perpendicular. So a band of width `W` on a boundary at `y`
covers `[y, y+W]` and a single rule inside it is stroked at `y + W/2`.

This is why **O85 is not a special case of a mixed-width row.** Every horizontal table border in
Writer is top-aligned; a row whose cells all state the *same* width simply cannot show it,
because one uniform band drawn top-aligned and the same band drawn centred differ only by a
constant that the eye reads as the whole table sitting half a border lower.

**(b) The whole band is charged to the row BELOW, as that row's top print margin.**

`sw/source/core/layout/tabfrm.cxx`:5338-5430, `SwRowFrame::Format`, under
`pTabFrame->IsCollapsingBorders()`:

```cpp
sal_uInt16 nTopPrtMargin = nTopSpace;                       // lcl_GetTopSpace(*this)
if ( pPreviousRow )
{
    const sal_uInt16 nTmpPrtMargin = pPreviousRow->GetBottomLineSize() + nTopLineDist;
    if ( nTmpPrtMargin > nTopPrtMargin )
        nTopPrtMargin = nTmpPrtMargin;
}
…
SetBottomMarginForLowers( nBottomLineDist );    //  3.
SetBottomLineSize( nBottomLineSize );           //  4.
SetTopMarginForLowers( nTopPrtMargin );         //  5.
```

`nTopSpace` is `lcl_GetTopSpace` (:5175-5194) = the maximum over the row's cells of
`SvxBoxItem::CalcLineSpace(TOP, true)` — this row's own stated top line *plus its padding*.
`GetBottomLineSize()` is `lcl_GetBottomLineSize` (:5219-5240) = the maximum over the previous
row's cells of `CalcLineSpace(BOTTOM, true) − GetDistance(BOTTOM)` — the previous row's stated
bottom **line only, padding excluded**. So `nTopPrtMargin` is `max(my top line, the row above's
bottom line) + padding`: the **resolved** width, charged once.

The bottom side is the other half of the claim: `SetBottomMarginForLowers(nBottomLineDist)` —
`lcl_GetBottomLineDist`, the bottom **padding** and *not* the bottom line size. Those two values
are what a cell frame's height is built from:
`lcl_CalcTopAndBottomMargin` (:4976-5004) returns `GetTopMarginForLowers() +
GetBottomMarginForLowers()` for every cell of a collapsing-borders table, and
`lcl_CalcMinCellHeight` (:5047-5052) adds it to the cell's content height.

So **a row never pays for its own bottom border line.** The row below pays, and it pays the
larger of the two facing widths. That is O84's mechanism, and it says why the seat's
"the reference charges the RESOLVED one" is only half the story.

**(c) The facing edges are merged in the painter, not in the model.**

`SwTabFramePainter::Insert(SwLineEntry&, bool)` (`paintfrm.cxx`:3102-…) keys horizontal entries
on the frame edge's y; the upper cell's bottom entry and the lower cell's top entry land on the
same key and are merged with `const svx::frame::Style& rCmpAttr = std::max(rNewAttr, rOldAttr);`
— the wider wins, per overlapping x span, which is round 131's per-column result restated.

**(d) A page cut is the same rule with two substitutions, not a rule of its own.**

* `lcl_IsFirstRowInFollowTableWithoutRepeatedHeadlines` (`paintfrm.cxx`:2815-2833) makes the
  **first row of a follow table** draw its *top* line from the cell's **bottom** border style
  (`bBottomAsTop`, used at :3089), so a continuation page opens with the line the row would have
  closed with.
* `SwTabFramePainter::InsertFollowTopBorder` (:2858-2927) and `InsertMasterBottomBorder`
  (:2932-3005) cover the case where the cell states *neither* top nor bottom: they copy the
  matching column's border from the other end of the table. This is round 131's `PBNN` arm.

Neither substitution changes the geometry: both build an ordinary `SwLineEntry` on the frame
edge, and both go through the same `RefMode::Begin`.

*One thing I could not settle in source, and it does not matter to the answer.* In this tree
`InsertFollowTopBorder`:2924-2926 and `InsertMasterBottomBorder`:2999-3001 call
`SetRefMode(RefMode::Begin)` on the `Style` **after** the `SwLineEntry` has already copied it, so
the copied cut border would be drawn `Centered`. Whether 26.2.4.2 has that ordering I cannot
check (no 26.2 branch exists here). §4 measures it: 26.2.4.2 **does** draw a copied cut border
centred, so the reading holds against the reference, and it is the one exception to the rule
rather than a contradiction of it.

---

## 3. What 26.2.4.2 answers [bin]

Reference: `/opt/libreoffice26.2/program/soffice`, LibreOffice **26.2.4.2**
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`. Every render is `timeout -k 30 900` into its own
`-env:UserInstallation`, and `render.sh` refuses to report success unless the PDF exists and is
non-empty. Every number below is read out of the PDF's own path operators by `read-rules.py`
(`page.get_drawings()`, `re` and `l` items) — nothing is read off a raster and nothing is
attributed by a y-band guess. Each arm is one table on its own page, so the tables' first grid
lines coincide and the arms are directly comparable.

**The compatibility control first, because `CLAUDE.md` says a hand-built DOCX without a
`word/settings.xml` answers a different question.** `height.docx` and
`height-nosettings.docx` are the same ten arms, the second with the settings part and its
relationship removed. **Their drawn geometry is identical, line for line.** So for *this* family
of questions the settings part changes nothing — which is worth recording, because round 131's
`borderprobe.docx` has no settings part either and its arms therefore stand. The part is present
in all of this round's fixtures regardless.

### 3.1 The band's top edge sits on the row boundary — `align.docx`

Three columns, three different stated widths across **one** boundary, nothing else varied.

| arm | who states the edge | column 0 | column 1 | column 2 |
|---|---|---|---|---|
| `A1` | the upper row's `w:bottom` | 0.5 pt, band **95.001**..95.501 | 3.0 pt, band **95.001**..98.001 | 1.5 pt, band **95.001**..96.501 |
| `A2` | the lower row's `w:top` | 0.5 pt, band **95.001**..95.501 | 3.0 pt, band **95.001**..98.001 | 1.5 pt, band **95.001**..96.501 |

**All three bands share a top edge at 95.001 and nothing else.** Their stroked centres are
95.251, 96.501 and 95.751 — `boundary + W/2` in each case, which is `RefMode::Begin` exactly.
`A2` is `A1` with the one attribute moved to the other side of the boundary and its output is
identical in every figure, so the side that states the edge is not a variable.

The double-border arms say the same thing about a band with internal structure. `A3`/`A4` put a
`w:val="double" w:sz="12"` beside two 0.5 pt singles: 26.2.4.2 reads the double as
0.5/0.5/0.5 scaled to **1.5/1.5/1.5** and draws its two rules at bands 95.001..96.501 and
98.001..99.501, i.e. a 4.5 pt band whose **top edge is again 95.001**, with the singles' bands at
95.001..95.501 beside it. This is O85's `150_5335_5a.doc` shape reproduced from nothing.

*The table's own outer top border obeys the same rule* — `A1`'s is a 1 pt band 82.401..83.401 on a
table whose frame top is 82.401 — and the outer bottom does too (band 109.501..110.501 below a
last row ending at 109.501). **There is no separate rule for an outer line.**

### 3.2 The row BELOW pays the whole resolved band, and the row above pays nothing — `height.docx`

Three rows; only the pair of attributes facing across boundary 1 changes; boundary 2 is nil/nil
throughout as the internal control. Table frame top 82.401 in every arm.

| arm | upper `w:bottom` | lower `w:top` | drawn band | boundary 1 | table frame bottom | table height |
|---|---|---|---|---|--:|--:|
| `H0`  | nil | nil | none | — | 118.101 | 35.700 |
| `HB`  | **3.0** | nil | 95.001..98.001 | 95.001 | 121.101 | 38.700 |
| `HT`  | nil | **3.0** | 95.001..98.001 | 95.001 | 121.101 | 38.700 |
| `HBT` | 3.0 | 3.0 | 95.001..98.001 | 95.001 | 121.101 | 38.700 |
| `HWB` | **3.0** | **0.5** | 95.001..98.001 | 95.001 | 121.101 | 38.700 |
| `HWT` | **0.5** | **3.0** | 95.001..98.001 | 95.001 | 121.101 | 38.700 |
| `HS`  | 0.5 | 0.5 | 95.001..95.501 | 95.001 | 118.601 | 36.200 |

Four results, none of which needs a free parameter:

* **The width is the maximum of the two facing stated widths.** `HB`, `HT`, `HBT`, `HWB` and
  `HWT` are indistinguishable in every figure — 3.0 pt drawn and 3.000 pt of height.
* **The table grows by exactly the band**: `H0` → `HS` is +0.500 for a 0.5 pt band, `H0` → `HB` is
  +3.000 for a 3 pt one.
* **Boundary 1 does not move.** It is at 95.001 in every arm that draws anything there, and
  `H0`'s rows 1 and 2 place it there too. So **the row above pays nothing** — not half, nothing.
  A 3 pt edge below row 0 leaves row 0 exactly as tall as an edge of nil does.
* **The whole of the band is therefore inside the row below**, which is the same statement as
  §3.1 read through the height instead of through the ink.

*This is the sharpening O84 needs.* "A row whose cells all nil an edge is half a border short" is
what a model that charges `(stated top + stated bottom)/2` sees when it meets a rule that charges
`max(top, bottom)` to one side. The rule is not "resolve then halve"; it is **resolve, then give
all of it to the lower row**.

### 3.3 A table-level border the cell replaces — `height.docx` arms TB*

`w:tblBorders/w:insideH` of 3 pt, with the cell's own statement varied:

| arm | cell states | drawn at boundary 1 | table height |
|---|---|---|--:|
| `TB0` | no `w:tcBorders` at all | 3.0 pt, band 95.001..98.001 (and the same at boundary 2) | 41.700 |
| `TBN` | `w:top`/`w:bottom` = `nil` | **nothing** | **35.700** — `H0` exactly |
| `TBS` | `w:top`/`w:bottom` = 0.5 pt | 0.5 pt, band 95.001..95.501 | **36.200** — `HS` exactly |

So the cell's own statement replaces the table's, and **the height follows the replacement, not
the statement**: `TBS` states 0.5 where the table states 3.0 and is 2.5 pt shorter than `TB0`;
`TBN` states `nil` against the table's 3.0 and is the bare table. A model that charged what the
table states, or that could not let a cell `nil` beat an `insideH`, would be 3 pt a row out here.

### 3.4 A `w:trHeight` floor charges the STATED top line, not the resolved one — `trheight.docx`

This is what round 131 could not fit, and the reason is that **the two quantities are different
and both are in play**. Row 1 carries `w:trHeight w:val="400" w:hRule="atLeast"` (20 pt); only
the pair facing across boundary 1 changes.

| arm | upper `w:bottom` | row 1's own `w:top` | drawn band | table frame bottom | vs `R0` |
|---|---|---|---|--:|--:|
| `R0` | nil | nil | none | 126.501 | — |
| `RA` | **3.0** | nil | 3.0 pt, 95.001..98.001 | **126.501** | **+0.000** |
| `RB` | nil | **3.0** | 3.0 pt, 95.001..98.001 | 129.501 | **+3.000** |
| `RC` | 3.0 | 3.0 | 3.0 pt, 95.001..98.001 | 129.501 | +3.000 |
| `RD` | **3.0** | **0.5** | 3.0 pt, 95.001..98.001 | 127.001 | **+0.500** |

`RA` and `RB` draw **the same 3 pt line at the same y** and differ by **3 pt of row height**;
`RD` draws a 3 pt line and grows by 0.5. The growth is the row's **own stated** top line, every
time, and the drawn band is the **resolved** one, every time.

That is `lcl_CalcMinRowHeight` (`tabfrm.cxx`:5087-5097) against `SwRowFrame::Format` (:5399-5404)
[src]: the floor adds `lcl_GetTopSpace(*_pRow)` — this row's own `CalcLineSpace(TOP, true)` —
while the cell's print margin is `max(that, the row above's bottom line size + this row's top
distance)`. The row's height is then the larger of the floor and the content-plus-margins, which
in `RA` is `max(20.0, 3.0 + 11.55)` = 20.0 — the band eats into the floor rather than adding to
it.

**So there is no linear function of the two facing widths, and round 131 was right that none
fits.** The closed form is

```
  charge(row) = max( trHeightFloor + ownStatedTopLineSpace ,  content + resolvedTopBand )
              + ownBottomLineDist                      // the bottom PADDING, never the line
```

with the first term dropping out for a content-driven row.

---
## 4. A page cut is the same rule — `split.docx`, `splitnil.docx`, and the seat's own document [bin]

`split.docx` is one 46-row table whose interior edges are stated 1 pt on the **upper** side only;
row 30 holds thirty paragraphs, so the row itself straddles the page boundary. Page geometry:
text area top 70.85 pt, bottom 771.04 pt.

| | y | band | width |
|---|--:|---|--:|
| last interior boundary on page 1 (rows 29/30) | 457.901 | 457.901..458.901 | 239.00 (interior) |
| **the cut, foot of page 1** | 759.201 | **759.201..760.201** | 241.00 (outer) |
| **the cut, head of page 2** | 70.901 | **70.901..71.901** | 241.00 (outer) |
| first interior boundary on page 2 | 118.101 | 118.101..119.101 | 239.00 |

The master part's last text line ends at 759.201 and its 1 pt band hangs **below** it; the follow
part's frame top is 70.901 — the text-area top — and its 1 pt band hangs **below** that, with the
first line of continued text starting at 71.901. Both are `RefMode::Begin`, both are top-aligned
on the frame edge, and **the follow part pays a full 1 pt for the line at its head** (4 lines
× 11.55 pt fills 71.901 to 118.101 exactly). The split row therefore pays the border twice, once
per part, and that is the reference's arithmetic rather than a defect.

The line at the head of page 2 comes from `lcl_IsFirstRowInFollowTableWithoutRepeatedHeadlines`
(`paintfrm.cxx`:2815-2833, used at :3089) [src]: the first row of a follow table draws its **top**
line from the cell's **bottom** border style. Nothing in the geometry is special.

**And the seat's own document reproduces to the thousandth.** `review-welsh-…-mandelson.docx`
rendered fresh through 26.2.4.2 here:

| | reference band | reference stroke | what O83 records this tree drawing |
|---|---|--:|--:|
| page 9, the boundary at the head | 120.901..121.401 | **121.151** | 120.900 **and** 121.150 |
| page 10, the boundary at the foot | 721.601..722.101 | **721.851** | 721.600 **and** 721.850 |
| page 10, the cut | 768.501..769.001 | **768.751** | 768.50 |
| page 11, the cut | 72.001..72.501 | **72.251** | 72.00 |

Read that table carefully, because it says something the seat does not. **Of the two lines this
tree draws at a split boundary, one is on the reference's stroke and the other is on the
reference's band TOP.** 121.150 is the reference's 121.151; 120.900 is the reference's 120.901.
Likewise 721.850 against 721.851, and 721.600 against 721.601. The same at both cuts: 768.50 and
72.00 are the reference's band tops, not its strokes.

So this tree is not "half a border too high at a page cut" in the sense of a uniform shift — it is
right on the ordinary side of the cut and **centred instead of top-aligned on the continuation
side**, which is §5.

### The one place 26.2.4.2 itself centres a horizontal band — `splitnil.docx`

`splitnil.docx` is 70 rows in which every interior horizontal edge is `nil` on **both** sides and
only the first row's top and the last row's bottom state anything (3 pt, chosen so that `Begin`
and `Centered` differ by a visible 1.5 pt). This is the only arm that reaches
`InsertFollowTopBorder` / `InsertMasterBottomBorder`, which copy a border from the other end of
the table when the cell states neither side.

| | band | table frame edge | alignment |
|---|---|--:|---|
| page 1, the table's own outer top (a stated border) | 82.401..85.401 | 82.401 | **Begin** |
| page 1, the cut (a **copied** border) | 765.401..768.401 | ≈766.85 | **Centered** |
| page 2, the cut (a **copied** border) | 69.401..72.401 | 70.901 | **Centered** |
| page 2, the last row's own outer bottom (a stated border) | 197.901..200.901 | 197.901 | **Begin** |

**26.2.4.2 draws a copied cut border centred on the table edge and charges it no height at all** —
page 2's first line starts at 70.901, inside the band. This is the one exception to the rule, it
is measured rather than assumed, and it is exactly what the source ordering in this tree predicts
[src]: `InsertFollowTopBorder`:2924-2926 and `InsertMasterBottomBorder`:2999-3001 construct the
`SwLineEntry` from the `Style` **before** calling `SetRefMode(svx::frame::RefMode::Begin)` on it,
so the entry keeps the `Centered` default. A [src] reading of a tree that is not the reference's,
confirmed [bin] against the reference. It reaches only a table whose cells state neither edge at
the cut, which round 131's `PBNN` arm is and `review-welsh-…` is not.

---

## 5. What this tree does, and why the three seats look like three [src, this tree]

Read out of `dotnet/src`, not measured — the round could not run `XRay.Office.Cli`.

* `TableLayouter` starts the table at `BorderHeight(rows[0]) / 2`
  (`Layout/TableLayouter.cs`:84 and :235) and charges every row
  `BorderHeight(row) = (max stated top width + max stated bottom width) / 2`
  (:1105-1117, :206).
* `PageDrawing.DrawBorders` draws each rule **centred** on the grid line:
  `Rule((bands.Outer / 2) - half, bands.Outer)` (`Layout/PageDrawing.cs`:780) is offset zero for
  a single rule.

**Half-of-each plus centred is algebraically identical to all-to-the-lower-row plus top-aligned,
for as long as every horizontal border in the table has the same width.** Put the grid line half
a border below the boundary and centre the band on it, and the band lands exactly where the
reference's top-aligned band lands. That is why the model has survived: the overwhelming majority
of real tables draw one width throughout.

It comes apart in exactly three places, and those are the three seats:

1. **Two facing edges of different widths** → the charge is `(top + bottom)/2` where the reference's
   is `max(top, bottom)`; the error is `|top − bottom| / 2` of row height. Nil on one side is the
   common case of it and is half a border. **That is O84**, and it is O78's 5.25 pt.
2. **Two columns of one boundary at different widths** → one grid line cannot be half of two
   different widths below the boundary, so the narrower column's band is drawn
   `(W_max − W_column)/2` too low. **That is O85**, and it is `150_5335_5a.doc`'s 0.625 pt.
3. **A page cut** → the half-border offset is seeded from the table's first row and is not carried
   onto the continuation, so the continuation's first grid line lands *on* the boundary and its
   centred band is `W/2` too high. **That is O83**, and §4 shows the ordinary side of the same
   boundary landing on the reference to the thousandth while the continuation side lands on the
   reference's band top.

One rule, three symptoms. A fix that moves the model to *the row below pays the resolved band,
and the band is drawn from the boundary downwards* closes all three; a fix that keeps the
half-and-half split and only resolves the width — which is what round 131 tried and measured as
worse — cannot, and §3.4 says why the residue it saw had no linear fit.

---
## 6. Reach — what the rule paints [bin] and what states it [src of the markup]

All 337 words-track documents of `MANIFEST.tsv` (words **337** rows: 271 `docx`, 66 `doc`;
the manifest's own path list, not a `find` total — note that `CLAUDE.md` records the words track
as 338 and 272 `docx`, and today's manifest gives 337 and 271, so the denominator moved by one
somewhere between round 131 and here) rendered fresh through 26.2.4.2, three workers,
one `-env:UserInstallation` each, `timeout -k 30 900`: **337 rendered, 0 failed**
(`sweep.tsv`). The census reads those renderings' path operators; it does not read our own
output, because this round could not run `XRay.Office.Cli`.

Denominator: 26.2.4.2 draws **29 306** distinct horizontal rule y-groups over the 337 documents,
in 296 of them. That counts paragraph rules and underlines as well as table boundaries, so it is
an upper bound on the boundaries at issue.

### O85 — a boundary whose columns carry bands of different widths

`census-pdf.py`, two shapes, both read off the reference's own top-aligned bands:
segments of **more than one thickness** sharing one band top side by side, and a **narrower group
hanging 0–6 pt below a wider one and contained in it** — the second is the multi-line border
present on some columns only, which is `150_5335_5a.doc`'s shape and which the first shape misses
because a double's two rules are two 0.5 pt strokes, not one 1.5 pt one.

| | documents | boundaries |
|---|--:|--:|
| **a boundary whose columns differ — this tree cannot draw it at one y** | **49 of 337** | **770** |
| the base rate: a boundary drawn per column, all columns one width | 141 of 337 | 2 850 |
| all horizontal rule y-groups (upper bound on the denominator) | 296 of 337 | 29 306 |

So **21 % of the per-column boundaries the reference draws carry more than one width**, and the
seat's own witness is in the list: `150_5335_5a.doc` scores 20. The head of it is
`150_5300_13_chg8.doc` 210, `150-5370-10H.docx` 206 and `ESPN-R - MCF - RA - Ed1.docx` 112.

### O83 — a table crossing a page boundary

A proxy, and it is stated as one: the verticals of page *p* end where a horizontal rule sits, the
verticals of page *p+1* begin where a horizontal rule sits at the very top of the body, and at
least two of their x positions agree to a point. A table that ends exactly at the foot of one page
beside a different table opening the next would also match, so this is an **upper bound**.

**42 of 337 documents, 329 cuts.** Every cut is two misplaced rules — the foot of the master and
the head of the continuation — so the class is about **658 rules**, against the 29 306 the
reference draws. `review-welsh-…-mandelson.docx`, the seat's own document, scores **9**; the head
is `OM template for non-complex NCC operators_August 2016.docx` at 112, `ESPN-R - MCF - RA -
Ed1.docx` at 50 and `ESPN-R - MCF - Manual` at 31.

I could **not** separate, statically or from the reference's ink, the cuts at which a *row* splits
mid-content from those that fall between two whole rows. It does not matter to the reach: §5 says
the missing half-border offset is a property of the continuation's first grid line, which every
cut has. The **doubled** line the seat describes needs a row that splits; the two **misplaced**
cut rules need only a cut.

### O84 — a boundary whose two facing widths differ

This one cannot be read from the reference's ink at all, because both readings draw the same line
— the difference is in the height, and measuring it needs our own rendering. Censused from the
markup instead, `census-docx.py`, over the 271 corpus `.docx`: the cell's own `w:tcBorders`, else
`w:tblPrEx`, else the table's `w:tblBorders/w:insideH`, else the table style's walked through
`w:basedOn`; `A` = the upper row's thickest resolved bottom, `B` = the lower row's thickest
resolved top.

| | |
|---|--:|
| `.docx` read | **271 of 271** |
| holding at least one table row boundary | 176 (16 430 boundaries) |
| **with at least one boundary where `A ≠ B`** | **38** (**914** boundaries) |
| the height this tree is out over all of them, `Σ|A − B| / 2` | **257.1 pt** |

The head: `FAA 2025-26 Holdover Tables.docx` 97.00 pt over 356 of 1 394 boundaries,
`24-25_FAA_Holdover_Tables.docx` 74.25, `150-5370-10H.docx` 12.75,
`UG.CAO.00133 … Language.docx` 11.12, `private-hire-operators-licensed.docx` 9.50,
`review-welsh-…-mandelson.docx` 7.00. Round 131's *5.25 pt on FAA page 82* is one page of the
first of those, and the figure scales.

**What this census is not.** It is 271 of the 337 words documents — **the 66 `.doc` are WW8 and
cannot be read statically**, so the class is larger than 38 by an unmeasured amount, and
`150_5335_5a.doc` and `150_5300_13_chg8.doc` are both `.doc`. It resolves neither conditional
table-style formatting (`w:tblLook`, first-row and banding overrides) nor `w:tblCellSpacing`, and
it counts nested tables by their own markup, so it is a **lower bound** on the class. And the
`Σ|A − B|/2` is the height error *at the boundaries*, not the page-count consequence of it: a
table that ends 6 pt lower may cost a page or may cost nothing, and only a rendering can say
which.

---
## 7. Supporting measurement: how 26.2.4.2 scales a `double` border — `double.docx` [bin]

`census-docx.py` needs this, and one size would not have settled it. Seven sizes, one arm each,
the same boundary in every arm:

| `w:sz` | stated | the two drawn rules | total band | top edge |
|--:|--:|---|--:|--:|
| 2 | 0.25 | 0.250 pt at 95.026 and 95.526 | 0.75 | 95.026 |
| 4 | 0.50 | 0.500 at 95.001 and 96.001 | 1.50 | 95.001 |
| 6 | 0.75 | 0.750 at 95.026 and 96.526 | 2.25 | 95.026 |
| 8 | 1.00 | 1.000 at 95.001 and 97.001 | 3.00 | 95.001 |
| 12 | 1.50 | 1.500 at 95.001 and 98.001 | 4.50 | 95.001 |
| 18 | 2.25 | 2.250 at 95.026 and 99.526 | 6.75 | 95.026 |
| 24 | 3.00 | 3.000 at 95.001 and 101.001 | 9.00 | 95.001 |

**Three equal bands of `sz/8` pt — line, gap, line — total `3 × sz/8`, top-aligned on the
boundary at 7 of 7.** The 0.025 pt on the odd sizes is twip rounding. So a `w:val="double"`
edge is three times as tall as its `w:sz` suggests, which is why `150_5335_5a.doc`'s
"1.5 pt double" occupies 1.5 pt of *band* from a `w:sz` of 4, and why a census that reads `sz/8`
for a double understates the class.

---

## 8. What this round could NOT establish, and what it refuted

**Could not establish.**

* **Anything measured about this tree's own output.** The round was forbidden to build or run
  `XRay.Office.Cli`. Every statement about our side is a reading of `dotnet/src` (§5) or a figure
  round 131 banked. The seat numbers for O83 were checked the only way available — by
  re-rendering `review-welsh-…-mandelson.docx` through 26.2.4.2 and confirming the reference half
  of each pair (§4) — and they reproduce to the thousandth.
* **Whether 26.2.4.2's `InsertFollowTopBorder` / `InsertMasterBottomBorder` carry this tree's
  statement ordering.** There is no 26.2 branch in this checkout. What is measured is that their
  *output* is centred (§4), which is what that ordering produces.
* **The reach of O84 on the 66 corpus `.doc`.** WW8 cannot be read statically and the defect is
  invisible in the reference's ink, so 38 of 271 `.docx` is a lower bound on a class whose two
  largest O85 members are both `.doc`.
* **Whether a row that splits mid-content can be told from a table that cuts between two whole
  rows**, from either the markup or the reference's ink. §6 says why it does not change the reach.
* **The page-count cost of the height change.** O78 records that fixing O84 alone makes
  `FAA 2025-26 Holdover Tables.docx` worse, and this round did not re-measure that — it could not
  render our side. §9 says what has to be measured before it ships.
* **`TE.CAO.00125 … OJT Logbook.docx`'s 14 pt.** See below.

**Refuted.**

* **That these are three defects.** One rule, measured three ways (§1, §3, §4).
* **O84 as stated.** *"The reference charges the RESOLVED border"* is true and incomplete: the
  reference charges the whole of the resolved border to the **lower** row and nothing to the
  upper. `HB` and `HS` differ only in their lower border's width and put boundary 1 at the same
  95.001, so the upper row's height does not depend on that width at all (§3.2). A model that
  resolves the width and keeps charging half to each side is still wrong by `W/2` a boundary, and
  that is what round 131 built and measured as worse.
* **That some linear function of the two facing widths fits a `w:trHeight` row's charge.** None
  does, and §3.4 says why: the floor uses the row's **own stated** top line and the print margin
  uses the **resolved** one, and the row takes the larger of the two results. `RA` and `RB` draw
  an identical 3 pt line at an identical y and differ by 3.000 pt of height.
* **That O85 is a property of a mixed-width row.** Every horizontal table band in Writer is
  top-aligned, including a table's own outer top and bottom and including a uniform boundary
  (§3.1). A mixed-width row is only where a *centred* model can no longer imitate it.
* **Round 131's guess that `TE.CAO.00125 … OJT Logbook.docx` is "probably O85's mechanism and
  probably O83's too".** The census puts it in neither class: 0 mixed-width boundaries and 0 page
  cuts, against 14 per-column boundaries of one width. Whatever its 14 pt is, it is not this rule.
* **That a hand-built DOCX without `word/settings.xml` answers a different question *here*.**
  `height.docx` and `height-nosettings.docx` are line-for-line identical in their drawn geometry
  (§3). This is a scoped negative, not a repeal of `CLAUDE.md`'s rule — the part is present in
  every fixture this round shipped, and the rule cost a round in a different family. What it does
  settle is that round 131's settings-less `borderprobe.docx` arms stand.
* **That any of the three is already fixed.** `PageDrawing.DrawBorders` still centres
  (`Layout/PageDrawing.cs`:780) and `BorderHeight` is still `(max top + max bottom) / 2`
  (`Layout/TableLayouter.cs`:1105-1117). O82's fix is read by the drawing alone and deliberately
  leaves the heights on the stated rows, which is O84 by construction.

---

## 9. Per-seat recommendation

**Take all three in one round, and take them in this order.**

| seat | worth a round? | reach [bin unless noted] | risk |
|---|---|---|---|
| **O85** | **yes** — and it is the cheapest of the three | 49 of 337 documents, **770 boundaries** (21 % of the 2 850 per-column boundaries the reference draws) | **none to the layout.** It is where a band is drawn, not how tall a row is; the same narrowing O82 shipped applies |
| **O83** | **yes** | 42 of 337 documents, **329 cuts = ~658 misplaced rules**; a proxy and an upper bound | **none to the layout**, for the same reason — the cut rules are drawn, not measured |
| **O84** | **yes, but last and with a gate** | **38 of 271 `.docx`, 914 boundaries, 257.1 pt** of row height (a *lower* bound: the 66 `.doc` are unmeasured) | **high.** It moves row heights and therefore page counts; O78 is a document on a 1.5 pt knife-edge where the previous attempt made it worse |

None of the three is nil-reach and none is already fixed.

**Why one round.** O85 and O83 are the same line of drawing code and O84 is the height that line
implies. Fixing the drawing alone leaves the geometry inconsistent with the heights the layout
hands it, which is exactly the state round 131 left behind and is why
`review-welsh-…-mandelson.docx` went from 1 207 pt short of the reference to 472 pt long.

**What the fix is**, stated as the measurement and not as a patch:

1. A row boundary has **one y** — the boundary itself — and each column's band is drawn from it
   **downwards**, at the column's own resolved width. Replace `Rule((bands.Outer / 2) − half, …)`
   for a horizontal edge with an offset of `+bands.Outer / 2`, and stop seeding the table's top
   with `BorderHeight(rows[0]) / 2`.
2. A row's height charge is
   `max(trHeightFloor + ownStatedTopLineSpace, content + resolvedTopBand) + ownBottomLineDist`,
   with the resolved band being `max(this row's thickest top, the row above's thickest bottom)`
   and the row above charged **nothing**. `RowsWithSharedEdges` already computes the resolution;
   what changes is that the layout reads it and that the charge stops being halved.
3. A `w:val="double"` edge is **three** bands of `w:sz/8` pt, not one (§7).
4. At a page cut the continuation's first boundary is an ordinary boundary and gets the ordinary
   treatment — with **one exception, measured**: a border *copied* by
   `InsertFollowTopBorder` / `InsertMasterBottomBorder`, which fires only where the cell states
   neither side of the cut, is drawn **centred** on the table edge and charges **no** height
   (§4). Do not unify that one away.

**The gate the round must pass**, because the reach census cannot see it: our own words track
rendered before and after under `SOURCE_DATE_EPOCH`, scored on **page counts and alphanumeric
counts**, plus `FAA 2025-26 Holdover Tables.docx` re-measured page by page against 26.2.4.2 —
that document is O78 and its 1.5 pt is two faults that nearly cancel, only one of which this
change touches. A fix that closes 257 pt of boundary error and moves one page count is not
obviously a win, and the round has to say which.

---

## 10. The scripts and the data

| file | what it is |
|---|---|
| `make-probes.py` | `align.docx`, `height.docx`, `height-nosettings.docx`, `split.docx` |
| `make-splitnil.py` | `splitnil.docx` — the only arm reaching the cut-border copy path |
| `make-trheight.py` | `trheight.docx` — stated versus resolved on a `w:trHeight` floor |
| `make-double.py` | `double.docx` — seven sizes of `w:val="double"` |
| `render.sh` | one document through 26.2.4.2, own profile, `timeout -k 30 900`, refuses to report success on an empty PDF |
| `sweep-ref.sh` | the 337 words documents, three workers, one profile each → `sweep.tsv` (337 ok, 0 failed) |
| `read-rules.py` | every horizontal rule of a PDF out of its path operators, band edges included |
| `census-pdf.py` → `census-pdf.tsv` | mixed-width boundaries and page cuts, per document |
| `census-docx.py` → `census-docx.tsv` | facing-width disagreement and its height, per `.docx` |
| `fixtures/`, `out/`, `refpdf/` | the authored documents, their renderings, and the corpus sweep |
