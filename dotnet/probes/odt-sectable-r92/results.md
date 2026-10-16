# `odt-sectable-r92` — a section inside a section is not inside its columns, and the table never was the defect

Measured 2026-09-10 in `/home/user/wt-odtsec2` (branch `agent/odtsec2`, base `1184c6316`).
Reference **`/opt/libreoffice26.2/program/soffice` — LibreOffice 26.2.4.2 0229ac93**, with the five
tarball font confounds moved aside (`ls .../truetype | grep -icE
'carlito|caladea|liberation|dejavu|^Noto(Sans|Serif)-'` answers 1, and that one is
`DejaVuMathTeXGyre.ttf`, which duplicates nothing installed).

Our half of the `.odt` track was rendered at HEAD and at the round's base against the banked
reference of `/home/user/gate-odf-r80`, which is what `probes/odt-startx-r88` scored against too.

---

## 0. Both of the brief's halves were wrong about which object is at fault

The brief carried two seats from `odt-startx-r88`. **The first does not exist and the second is not
what it says**, and one measurement refutes each.

### 0.1 A table inside a columned section is already laid out against its column — in this tree

r88 recorded *"a table inside a columned `text:section` is laid out against the page rather than
against its column"*, at 3 of the 33 documents stating a columned section. Reproduced the census
(`odt-startx-r88/census-tables-in-sections.py`, unchanged): the three are
`absrc-pac-01-info-note-en`, `150_5300_13_chg10` and `150_5300_13_chg12`, one table each.

All three were measured on both sides, before this round's change:

| document | the table's own cells, 26.2.4.2 | this tree at the base | the column it is in |
|---|---|---|---|
| `absrc-pac-01-info-note-en` | 88.70, 82.90, 79.55 | 88.75, 82.74, 79.40 | the **first**, 72…216 |
| `150_5300_13_chg10` | `Group #` 357.65, `Tail Height (ft)` 417.60…560.35 | 357.62, 417.94…560.19 | the **second**, 341.8…578.6 |
| `150_5300_13_chg12` | `Group #` 357.95, `Tail Height (ft)` 417.90…560.65 | 357.92, 418.24…560.49 | the **second**, 341.9…578.9 |

**Every cell is within 0.35 pt of the reference's on all three, and the round's change does not
move any of them.** (The two `150_5300_13` revisions put that page one page later than 26.2.4.2
does and fail the gate on pages for reasons of their own; the column the table is laid out in is
not one of them.) The paginator has placed a table against `body.ColumnArea(column)` since before
r88 (`Paginator.cs`:1430-1436) and `Laid` re-breaks the block at `ruler.WidthAt(column)` when the
columns are unequal, so there was nothing to fix.

Confirmed at the reference on purpose-built probes as well (`nested.py`, variants `table` and
`widetable`, on `features/odt-section-columns.odt` whose columns are 216 pt at x 72 and 324):

* a 2 in table inside the section is drawn at **324.10** by 26.2.4.2 and at 326.75 here — inside the
  column, both;
* a **6 in** table, wider than the column, is drawn at **324.10…612.64** by 26.2.4.2 and at
  326.75…613.06 here — so a table keeps its declared width and overflows its column to the right,
  off the page, in both. It is neither shrunk to the column nor moved to the page.

The 2.65 pt that separates the two is the table's own cell inset and is the same in the
single-column control; it is not this mechanism and is left.

**Where r88's reading came from.** Its own sentence names the evidence — *"`INFORMATION HIGHLIGHTS`
at x = 311.27 in a 144 pt first column"* — and 311.27 is where 26.2.4.2 draws that paragraph too
(311.00). That paragraph is in the **second** column and the table is in the first; reading it as a
table that had escaped its column inverted the picture.

### 0.2 26.2.4.2 does not draw `absrc-pac`'s section in one column

The lost verdict's premise is *"26.2.4.2 draws that document's section in ONE column although the
file states two"*. It draws it in two, and the arithmetic is exactly this tree's:

| | 26.2.4.2 |
|---|---|
| the file as it stands | table in column one (79…208), `INFORMATION HIGHLIGHTS` **centred in column two at 311.00…467.63**, above the table's first row |
| `style:columns` removed | `INFORMATION HIGHLIGHTS` at 221.00…377.63 on the **next page**, below the table |
| the columns made even with a `fo:column-gap` | 342.50…499.13, same page, same baseline |

`311.00` is the centre of the second column of the unequal ruler — `252 + (288 − 156.63)/2`, less
the paragraph's own 6.7 pt of right indent — and `342.50` is that same centre moved 31.5 pt right
when the columns are made even, which is `(315 + 540)/2 − (252 + 540)/2` exactly. **The columns are
live in that file and the ruler this tree computes is the one the reference draws.** So r88's
*"six one-attribute variants leave the rendering identical"* and *"the columns are inert in that
file"* are both withdrawn.

The instrument that produced them is named in `odt-sectable-r89/xhist.py`'s own docstring, which is
the strongest thing that round left: **a histogram of line starts taken with every span of one
baseline merged reads a two-column page as one column**, because a baseline crosses the gap. That
is what "one column" was.

---

## 1. The rule: a section inside a section is a sibling of it, not a child

Writer builds a frame per section, and a nested section's frame is **not** inside its parent's.
Three seats, and the third is why the parent's columns and the parent's indents behave differently:

* **It is inserted behind the parent, into the parent's own upper.**
  `pFrame->InsertBehind(pTmp->GetUpper(), pTmp)` where `pTmp` is the parent's section frame —
  `sw/source/core/layout/frmtool.cxx`:1795-1803, under the comment *"Insert behind the Upper, the
  'Follow' of the Upper will be generated at the EndNode"*. The parent's `SwColumnFrame`s live
  inside the parent's frame, so a sibling never sees them.
* **The parent is split at the nested section's end.** At the section's end node,
  `pFrame = pOuterSectionFrame->SplitSect(pActualSection->GetLastPos(), pPrv)` — *"Splitting moves
  the trailing content to the next frame"*, `frmtool.cxx`:1954-1960 — so what follows the nested
  section is a **second frame of the parent's format**, with its own columns and its own balance.
* **It inherits the parent's indents and not the parent's columns.** `SwSectionFrame::Init` takes
  its width from `GetUpper()->getFramePrintArea()` and insets the print area by
  `GetFormat()->GetLRSpace()` (`sectfrm.cxx`:129-166, `#109700# LRSpace for sections`). A nested
  section's format **is** derived from the enclosing section's —
  `pFormat->SetDerivedFrom(pSectNd ? pSectNd->GetSection().GetFormat() : rDoc.GetDfltFrameFormat())`,
  `sw/source/core/docnode/ndsect.cxx`:1345 — so the `SvxLRSpaceItem` is inherited where the child
  states none. `GetCol()` never is, because **`SwSectionFormat`'s constructor puts the pool's
  default one-column item on every section format outright**:
  `LockModify(); SetFormatAttr(*GetDfltAttr(RES_COL)); UnlockModify();`,
  `sw/source/core/docnode/section.cxx`:608-614.

**And an ODF index is a section.** `text:table-of-content` and its six siblings import as a
`SwSectionNode` like any other, which is why a census counting `text:section` alone answers *nothing
in this corpus is nested* — and why `odt-sectable-r89` was right to name its scripts for nesting.

### Measured at the reference, eight variants of one fixture

`nested.py` puts one element inside the two-column section of `features/odt-section-columns.odt`
after PARA11 and reads 26.2.4.2's PDF. The long marker line is the instrument: laid out in a column
it wraps at 216 pt, laid out against the section's measure at 468.

| variant | 26.2.4.2 draws the marker | and PARA12, the content after it |
|---|---|---|
| `plain` (a paragraph — the control) | 324.10, in the second column | 324.10, still in the second column |
| `toc` (a `text:table-of-content`) | **72.10…519.03**, whole measure | **72.10** — the first column of a second frame |
| `nested` (a `text:section`, one column) | **72.10…523.54** | 72.10 |
| `nested-bare` (a section naming **no style at all**) | **72.10…521.93** | 72.10 |
| `nested-nocols` (a style stating no `style:columns`) | **72.10…530.49** | 72.10 |
| `nested-two` (a nested section stating **two** columns) | 72.10…242.78 — its own columns | 72.10 |
| `table` (a 2 in table) | 324.10 — **inside the column** | 324.10 |
| `widetable` (a 6 in table) | 324.10…612.64 — overflows the column and the page | 324.10 |

and the indents, inside a parent indented 0.5 in and 0.75 in (measure 108…486):

| variant | 26.2.4.2 draws the marker |
|---|---|
| `nested-margins` (the child states neither indent) | **108.10…448.4** — the parent's pair, inherited |
| `nested-ownmargin-outer` (the child states 1.5 in and 0.25 in) | **180.10…508.92** — its own pair, from the page |
| `nested-leftonly` (the child states only `fo:margin-left`) | 180.10…**515.71** — the right side is **nought**, not the parent's 54 pt |

The last row is the item semantics visible: `fo:margin-left` and `fo:margin-right` are one
`SvxLRSpaceItem`, so stating either replaces the pair.

---

## 2. The change

Three files, and the middle one is the half that is not about ODF at all.

* **`OdfSectionGeometry.Nested`** builds the geometry of a section met inside another: one column
  unless it states its own, its own indents where it states either and the enclosing section's
  otherwise. It never answers null — a nested section that changes nothing still has to *leave* the
  enclosing one's columns.
* **`OdtLayoutSource`** intercepts a `text:section` or one of ODF's seven index elements met while a
  section is open (`IsIndex`, `EnterNestedSection`, `LeaveNestedSection`), allocating a section for
  the nested content and a fresh one of the enclosing geometry afterwards — which is `SplitSect`.
* **`Paginator`** leaves the columns when a text section's column geometry changes part way down a
  page: the flow drops past `columnReach`, the deepest of the columns already filled, and starts
  again at the first column. Without it the pen stays in the column it was in, and the new section's
  full-measure lines are drawn at that column's origin.
* **`PageContent.ColumnArea(PlacedLine)`** no longer sends a line that states one column to the
  *page's* column at that line's index. That branch was written for "a line that states nothing",
  and since every body line records the count in force where it was laid out, the only lines it
  could reach were exactly the ones this round creates — a full-measure index on a page the
  paginator emitted while a two-column section was current.

### What it is not

The nested-section arm fires only where the column geometry actually changes, so a paragraph naming
its own master page inside a text section — which allocates another section of the same geometry —
does not restart the columns. A nested section whose columns *equal* its parent's is therefore not
split off; no corpus document does it and the fixture's `nested-two` differs by its gap.

---

## 3. What moved

</content>
`rows-before.tsv` is our half of the `.odt` track rendered by a binary built with the four source
files reverted to `1184c6316`, `rows-after.tsv` by the tree as it stands; both are scored against
the same banked reference bytes (`/home/user/gate-odf-r80/ref`), so only our side moved, and
**no row failed on either side in either run** — 338 of 338 scored.

| | `.odt` track |
|---|---|
| before | **292** of 338 match |
| after | **293** of 338 match |

**One row moves and it is the one round 88 cost.**

| document | before | after |
|---|---|---|
| `absrc-pac-01-info-note-en` | `pages` 6/7, 6873/6874 glyphs | **match** 7/7, 6874/6874 |

**Two renderings change in the whole column** (`odt-startx-r88/movers.py`, the conversion date
masked), and both improve against 26.2.4.2 (`odt-startx-r88/startx.py`):

| matched b/a | mean \|Δx\| before | after | within 0.1 pt | unique | document |
|---:|---:|---:|---:|---:|---|
| 162/166 | 9.110 | **0.335** | 117 → 125 | 156 | `absrc-pac-01-info-note-en` |
| 374/374 | 9.103 | **9.012** | 109 → 110 | 227 | `150_5300_13_chg8` |
| 536/540 | **9.105** | **6.345** | 226 → 235 | | total |

`150_5300_13_chg8` has seven columned sections and four indented ones; what is left on it is the
residual in §5 and not this mechanism.

### The regression that the first cut had, and what it says about the balance search

An intermediate build took `ABCD-SDE-23-00` from 29 pages to 30 against the reference's 29 — the
content *after* its two-column section moved to the next page, where 26.2.4.2 (and this tree
before the change) puts it 48 pt below the section's last line on the same page. The cause is worth
recording because it is not obvious: that section **balances**, and the paginator's balance search
tries the tallest candidate first, so a `columnReach` accumulated across trials holds the *first*
trial's depth — the whole band — rather than the accepted one's. `RestoreBalanceStart` now puts it
back to the section's own top with the rest of the trial state, and `BeginBalance` sets it to the
section's top at every section start. With that, the document is 29 pages again and the only row
that moves in the whole column is `absrc-pac`.

---

## 4. Confinement

The **338 originals** of the words track (`.docx`/`.doc` from `/home/user/sample-files/words`), the
**338 `.rtf` twins** of the converted corpus and ten `.ods` and ten `.odp` — 696 documents — were
rendered by a binary built with the four source files reverted to `1184c6316` and again by the tree
as it stands, `obj`/`bin` cleared on each leg, the restore done with `cp` and an explicit `touch`,
and `SOURCE_DATE_EPOCH` fixed on both:

```
696 renderings compared with the date masked, 4 differ
```

**Four is the honest answer and it is better than zero would have been.** The nested-section arm is
guarded by `WritingSection.IsTextSection`, which no reader but the ODF one sets, so it reaches none
of them; what does is `ColumnArea(PlacedLine)`, and the four are exactly the documents whose pages
mix column counts — `150_5300_13_chg12.doc`, `150_5300_13_chg8.doc`, `150_5300_13_chg8.rtf` and
`JEMIT_Template.docx`.

**No page count and no glyph count moves on any of the four**, and the one text line that moves is
*corrected*: `150_5300_13_chg8`'s centred `Chapter 3.  RUNWAY DESIGN` goes from x 205.05 to
**231.75** against 26.2.4.2's **231.80** — a full-measure heading that had been centred inside a
column of a page whose last section is columned. The same line moves the same way in the document's
`.doc`, its `.rtf` and its `.odt`. `JEMIT_Template.docx` has 308 text lines and **not one of them
moves**; its byte difference is elsewhere in the content stream. Mean |Δx| over the three with a
banked reference: **11.319 → 11.303 pt**.

The restored binary was then checked against the sweep it is claimed to have produced: one document
re-rendered and compared byte for byte with the date masked.

---

## 5. What is left, measured

**A page carries one text area and it is the last section's, so a `text:section`'s own indents
reach the layout and not the drawing.** `Derived` adds them to the master's margins and the
paginator takes them at the break (`Paginator.cs`:1258-1270), but `LaidOutPage.BodyArea` is
`geometry.TextArea` at the moment the page is *emitted* (`:2649`), and a page whose columned section
ends part way down it is emitted with the section that restores the master. A line carries its own
column count, gap and ruler and therefore its own column *within* that body — but not the body's own
origin and width.

Measured on `nested.py`'s `plain-margins`, whose two-column section is indented 0.5 in and 0.75 in:
26.2.4.2 draws its first column at **108.10** and this tree at **72.00**, with the same wrap and the
same baselines. **Left.** Of the corpus's 10 indented sections in 5 documents
(`odt-startx-r88/census-sections.py`), the one whose indent the gate can see —
`644730BRI0mna000BOX361539B00public0` — is unaffected, because its section runs to the end of the
body: every page of it, the last included, starts at 72 against 26.2.4.2's 72, and its verdict is a
match before and after. The other four are the three `150_5300_13` revisions and `手机免提系统TSB`,
which fail on pages for reasons of their own.

**A nested section whose columns equal its enclosing one's is not split off.** The paginator leaves
the columns only where the count or the gap changes, which is what keeps a paragraph naming its own
master page inside a text section from restarting them. Nothing in the corpus states such a section.

**And the 2.65 pt between our table's first cell and 26.2.4.2's is a cell inset, not a column.** It
is the same inside a column, outside one, and in the single-column control.

---

## 6. Files

| file | what it is |
|---|---|
| `nested.py` | sixteen variants of `features/odt-section-columns.odt`, each putting one element inside its two-column section, rendered through 26.2.4.2 |
| `census-nested.py` | every `text:section` and index in the converted `.odt`, how many are inside another and how many inside a columned one |
| `compare.py` | two sweeps compared, with every row that failed on either side in either run excluded |
| `confine.sh`, `confine.list` | the 696-document no-reach check and its list |
| `rows-before.tsv`, `rows-after.tsv` | the two `.odt` sweeps, both against `/home/user/gate-odf-r80/ref` |
| `movers.txt`, `movers.list`, `startx.txt` | which `.odt` renderings changed, and their start-x against 26.2.4.2 |
| `confinement.txt` | the confinement diff and the three movers' start-x |

Reused rather than rebuilt: `odt-startx-r88/movers.py`, `startx.py`,
`census-tables-in-sections.py`, `census-sections.py` and `sweep-odt-ours.sh`; and
`odt-sectable-r89/xhist.py`, whose docstring is what identified the instrument behind both of the
brief's claims.
