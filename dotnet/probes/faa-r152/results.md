# Round 152 — O78 re-measured at HEAD, after the row-band fix of round 149

**Written incrementally.** Each section was written as its measurement finished.

## 0. What this round is and how it is instrumented

Seat **O78** (`probes/OPEN-ISSUES.md`) says `FAA 2025-26 Holdover Tables.docx` is displaced by
1.5 pt at the foot of page 82, that the 1.5 pt is **two faults that nearly cancel**, and that
*"fixing O84 alone makes this worse, which is why it is not shipped."* **O84 was fixed and shipped
in round 149.** This round re-measures the document at HEAD and re-does round 131's page-82 row
pairing rather than trusting its numbers.

### Environment

| | |
|---|---|
| tree | `/home/user/libreoffice-core`, HEAD `b5d46342a` *(A break-only line takes the break run's face, not the neighbour's (O91))* |
| ours | `dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli`, **mtime 2026-09-18 09:52:33 UTC** |
| reference | `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2 `0229ac93fcf0d7cbc6376066c6f35021cef002dc`** |
| C++ read | this checkout, `configure.ac` **27.2.0.0.alpha0+** — *not* the reference binary's source |
| document | `/home/user/sample-files/words/pagination-001/docx/FAA 2025-26 Holdover Tables.docx` |
| date | 2026-09-18 |

**Nothing was built by this round.** The parent session rebuilt the CLI at 09:52:33 while this
round was reading the contract; every figure below was taken against **that one binary**, stated
mtime `2026-09-18 09:52:33.170232595 +0000`, and the mtime was re-`stat`ed before and after each
render. No figure spans a rebuild.

**Instrument.** `rules.py` is `probes/wordstable-r131/show-rules.py` copied verbatim, so the
horizontal-rule reading is round 131's own. `pair-rows.py` re-implements that round's §4 pairing
from the description in its `results.md` — drop the page's 2.25 pt header/footer rules, merge
rules whose `y` agree within 0.4 pt into one grid line (both sides draw two of the grid lines
doubled), pair the two lists positionally, difference consecutive gaps. **The instrument is
validated by the reference half**: all 34 of 26.2.4.2's page-82 grid lines and all 33 of its row
heights reproduce round 131's banked `faa-p82-rows.tsv` **exactly**, to the hundredth, including
its last line at 544.70. So where the new numbers differ from the banked ones, the difference is
ours.

---

## 1. [bin] The document today: same page count, same ten flipped pages, blank page 83 still there

`data/pagecensus.tsv`, `pagecensus.py`.

| | 26.2.4.2 | ours |
|---|--:|--:|
| pages | **167** | **167** |
| pages whose orientation differs | — | **10**: 84, 93, 94, 95, 97, 98, 121, 122, 125, 126 |

**The page *count* matches and always has** — the seat's headline *"prints ten pages in the wrong
paper orientation"* is not a page-count defect, it is a one-page displacement that later closes
again. Aligning the two page streams on alphanumeric character counts (±25):

| pages | state |
|---|---|
| 1 – 82 | aligned |
| **83** | **ours is blank (74 alphanumerics against the reference's 1751)** |
| 84 – 127 | ours runs **one page behind** |
| 128 – 167 | aligned again |

So the seat's *"our page 83 comes out blank"* is reproduced at HEAD, and the ten orientation
mismatches are the landscape/portrait alternations inside the displaced window 84–127.

**The divergence begins exactly where the seat says.** The table on page 82 is the body-level
table at **body index 1225** — 33 rows, all of which fall on page 82 — and body index **1226** is
the paragraph holding `<w:br w:type="page"/>`:

```xml
<w:p><w:pPr><w:jc w:val="left"/></w:pPr><w:r><w:br w:type="page"/></w:r></w:p>
```

Its empty half is what has to fit under the table, and does not.

---

## 2. [bin] The row pairing re-done — term (1) is *mostly* gone, term (2) is bigger, and the net has TRIPLED

`data/faa-p82-rows-r152.tsv` against the banked `probes/wordstable-r131/faa-p82-rows.tsv`.

| | round 131 (banked) | **round 152 (HEAD)** |
|---|--:|--:|
| rows we make **shorter** than the reference | 11 rows, **−5.25 pt** | **2 rows, −1.95 pt** |
| rows we make **taller** | 2 rows, **+6.90 pt** | **1 row, +7.10 pt** |
| rounding residue (±0.05 rows) | −0.10 pt | −0.10 pt |
| **net** | **+1.55 pt** | **+5.05 pt** |
| last grid line, ours | 546.20 | **549.70** |
| last grid line, the reference's | 544.70 | **544.70** |
| **displacement at the foot of page 82** | **+1.50 pt** | **+5.00 pt** |

**Direction unchanged — we are still the taller side — and the magnitude is 3.3× the seat's.**

### Term (1): eight of the eleven closed, two got worse

Round 149's model change is visible row by row. The eleven rows the seat calls short are rows
2, 3, 5, 6, 8, 9, 10, 19, 20, 21 and 32 of `faa-p82-rows.tsv`. Their markup (`data/faa-t1225-rows.tsv`,
`dump-rows.py`) splits them into two kinds:

* **Eight rows — 5, 6, 8, 9, 10, 19, 20, 21 — whose cells all state a `single/sz=8` top and no
  bottom at all.** These were −0.45/−0.55 and are now **+0.05/−0.05**: each gained exactly the
  0.50 pt that the old *half-of-each-stated-rule* model withheld. **These eight are what O84 was
  and they are closed.**
* **Rows 2 and 3, the two mixed header rows**, whose cells state `nil` on some columns and
  `single/sz=8` on others. They were −0.50 and −0.45; they are now **−1.00 and −0.95** — each has
  moved a further 0.50 pt *away* from the reference.
* Row 32 (−0.30) and its partner row 31 (+0.30) were a 0.25 pt merge artefact of the doubled rule
  at the foot; both are now ±0.05.

So term (1) is **−1.95 pt over two rows**, not −5.25 over eleven, and what is left of it is a
different shape from what the seat describes: it is not *"a row whose cells all state
`w:bottom w:val="nil"`"*, it is a row whose cells **disagree** across one boundary.

### Term (2): the wrap, and it is unchanged in substance

Row 7's cell 8 holds `Not Available` + a superscript `19`. The reference sets it on one line, we
break it onto two.

| | 26.2.4.2 | ours |
|---|--:|--:|
| row 7 height | 12.30 | **19.40** (was 18.90 at r131) |
| a *non*-wrapping sibling row (row 6) | 12.40 | **12.35** (was 11.85) |
| **cost of the wrap alone** | — | **+7.05 pt** |
| row 7 delta against the reference | — | **+7.10** (was +6.60) |

**The wrap itself did not change** — it cost 7.05 pt of row height before round 149 and costs
7.05 pt now. What moved is the baseline it is measured from: our one-line rows gained their
missing half band, so the same wrap now reads as +7.10 against the reference instead of +6.60.
The seat's *"two rows tall by 6.90"* was the wrap (+6.60) plus that merge artefact (+0.30); there
is only **one** tall row now.

### Room at the foot

The page's text area ends at ~573.3 pt (the footer rule is at 574.55).

| | 26.2.4.2 | ours, r131 | **ours, HEAD** |
|---|--:|--:|--:|
| last grid line | 544.70 | 546.20 | **549.70** |
| table's outer bottom | 545.20 | — | **550.20** |
| room left under the table | **28.1 pt** | 26.6 pt | **23.1 pt** |

---

## 3. The seat's conclusion was right and round 149's prediction was wrong

O78 says **"Fixing O84 alone makes this worse, which is why it is not shipped."** O84 was shipped
in round 149, and its own seat text says **"only the distribution within the table moves, and O78
is untouched."**

**O78 is not untouched. It is 3.3× worse**: 1.50 pt of displacement at the foot of page 82 before,
**5.00 pt** after. The prediction failed because its premise — *"for a table whose horizontal
rules all have one width the two models sum to the same table height"* — does not hold for this
table: **nine of its 33 rows (5–10, 19–21) state no bottom border at all** while the row beneath
states a 1 pt top, so there is no half band for the row above to give back. The sum is only conserved when every
interior boundary is stated from both sides.

Round 149 was not wrong to ship: it measured *page counts 171 of 179 exact before and after* over
the words track and no document's distance from 26.2.4.2 changing. That measurement is still true
— **this document's page count did not move either, because it was already displaced.** What it
could not see is that a document already on a knife edge went further over it, and O78 is the one
document on that knife edge.

---

## 4. [bin] Term (2): the tenth of a point is the SUPERSCRIPT'S REDUCED SIZE, and nothing else

The seat says the fit *"is not the padding, the column width or the metric"* and leaves it there.
It is none of the four things the brief lists either. It is a fifth: **the size `19` is set at.**

### What the real cell actually states

Body table 1225, row 7, cell 8 (`dump-rows.py`, `data/faa-t1225-rows.tsv`):

```xml
<w:tc><w:tcPr><w:tcW w:w="466" w:type="pct"/><w:tcBorders>…</w:tcBorders>
  <w:noWrap/><w:vAlign w:val="center"/></w:tcPr>
  <w:p><w:pPr><w:jc w:val="center"/>…</w:pPr>
    <w:r><w:rPr>… <w:sz w:val="16"/><w:szCs w:val="16"/> …</w:rPr><w:t>Not Available</w:t></w:r>
    <w:r><w:rPr>… <w:sz w:val="16"/><w:szCs w:val="16"/>
        <w:vertAlign w:val="superscript"/> …</w:rPr><w:t>19</w:t></w:r></w:p></w:tc>
```

Two things there are worth naming before the measurement. The cell states **`w:noWrap`**, which is
a tempting explanation and is not one: [src] `w:noWrap` is tokenised by
`sw/source/writerfilter/ooxml/model.xml`:18671 as `ooxml:CT_TcPrBase_noWrap` and **nothing in
`sw/` consumes that token** — `git grep CT_TcPrBase_noWrap -- sw/` returns that one line. [bin]
fixture 1's `NW-1`/`NW-0` arms differ only in whether `<w:noWrap/>` is present and 26.2.4.2 draws
both identically. And the column widths, measured off the two renderings' own vertical rules,
go the **wrong way**: the cell is **63.10 pt wide in ours and 63.00 in the reference's**, so we
have 0.10 pt *more* inner width and still break.

### The fit measured directly, at one twip of resolution

`make-fixture.py` / `make-fixture2.py` build a cell that reproduces the real one — Word's 108-twip
default margins stated explicitly, `w:sz 16`, `w:jc center` — and sweep the stated cell width one
twip at a time. The observable is whether the cell takes one line or two. Three sweeps differ in
**one attribute**:

| sweep | the `19` run | narrowest cell that fits, 26.2.4.2 | ours | difference |
|---|---|--:|--:|--:|
| **S** | `w:vertAlign w:val="superscript"` (the real cell) | **1261 tw** | **1263 tw** | **2 tw = 0.10 pt** |
| **Z** | an explicit `w:sz w:val="9"` (4.5 pt), no escapement | 1259 tw | 1259 tw | **0** |
| **N** | no second run at all | (below the sweep) | (below the sweep) | **0** |

`data/fit2.tsv`. **Take the escapement away and the two renderers agree to the twip.** The real
cell resolves to 1262 twips in ours — one twip short of our own threshold of 1263, and one twip
above the reference's 1261.

### And the base measurement is not merely "close", it is exact on both sides

`make-fixture3.py`: twelve strings whose exact `hmtx` width in twips has a fractional part between
0.08 and 0.42 — the only region in which "compare exactly and need `ceil`" and "round the
measurement to whole twips first and need `round`" give different answers. Sweeping each one twip
at a time:

**Both renderers need exactly `ceil(exact hmtx width) + 216`, 12 of 12 each, 24 of 24 in all, with
no free parameter.** `data/fit3-meta.tsv`, `data/fit3-ref.tsv`, `data/fit3-ours.tsv`. So the text
measurement, the cell margins and the fit comparison are identical, and CLAUDE.md's *"ours agrees
with 26.2.4.2 to a worst case of 0.0077%"* holds here at a resolution of one twip in nine hundred.

### The superscript's size, over 57 base sizes

`make-fixture5.py` sets `Mx` + a superscript `19` at every half-point from 2.0 to 30.0 pt and
`esc-score.py` reads the size each renderer drew the `19` at. Four candidate rules, all applied to
the base size each renderer actually drew, rounding half away from zero:

| rule | matches 26.2.4.2 | matches ours |
|---|--:|--:|
| `trunc(size_twips × 58 / 100)` | 30 | 35 |
| **`round(size_twips × 58 / 100)`** | 28 | **57 of 57** |
| **`round(size_tenths_of_a_point × 58 / 100)`** — the nearest **tenth of a point** | **57 of 57** | 28 |
| round-trip through 1/100 mm | 30 | 49 |

`data/esc-rule.tsv`. Both rules are exact and they are different rules:

* **ours** rounds 58% of the size to the nearest **twip** — `Escapement.SizeOf`, which is
  `Length.FromTwips((long)Math.Round(emSize.Twips * Proportion / 100.0))`;
* **26.2.4.2** rounds it to the nearest **tenth of a point**, i.e. to an even number of twips —
  every one of its 57 answers is an even twip count.

At the size this document uses: 8 pt → 160 twips → 58% is **92.8** → the reference sets
**92 tw = 4.60 pt** and we set **93 tw = 4.65 pt**. That 0.05 pt of size is **0.0556 pt** of advance
on `19`, and it is the whole of the 0.10 pt.

**They differ at 29 of the 57 base sizes**, in runs of five consecutive half-points out of every
ten — 2.0–3.5, 6.5–8.5, 11.5–13.5, 16.5–18.5, 21.5–23.5, 26.5–28.5. It is not a corner case.

### Confirmed a second way, through a channel with no quantisation in it

`make-fixture4.py` puts `i ` at 8 pt in front of **k** copies of a superscript `19` and sweeps the
width for k = 1, 2, 3, 4, 6, 8, 12, 16. Every fixed term — the base run, the margins, the borders —
is identical across arms, so differencing two arms cancels them, which is the technique CLAUDE.md
prescribes for advance comparison. `fit4-score.py`, `data/fit4-score.txt`:

| k | 1 | 2 | 3 | 4 | 6 | 8 | 12 | 16 |
|---|--:|--:|--:|--:|--:|--:|--:|--:|
| predicted at a **4.60 pt** superscript | 399 | 501 | 603 | 706 | 910 | 1115 | 1524 | 1934 |
| **26.2.4.2** | **399** | **501** | **603** | **706** | **910** | 1117 | 1531 | 1945 |
| predicted at a **4.65 pt** superscript | 400 | 503 | 607 | 710 | 917 | 1124 | 1538 | 1952 |
| **ours** | **400** | **503** | **607** | **710** | **917** | **1124** | **1538** | **1952** |

**Ours is the 4.65 pt model at 8 of 8. The reference is the 4.60 pt model at 5 of 8** — at k = 1,
which is the real cell's case, and up to k = 6. The three long arms are in §7.

---

## 5. [bin] What is left of term (1): a `w:vMerge` CONTINUATION cell's stated top rule, which we do not see

With term (2) simulated away (§6), page 82's only remaining error is rows 2 and 3, at **−1.00 and
−0.95 pt**. Both are `w:trHeight` rows on their floor — 450 twips (22.5 pt) and 227 (11.35) — and
in both our height is *exactly* the stated floor while the reference's is the floor plus **1.00 pt**.

Their markup explains why (`data/faa-t1225-rows.tsv` and the `gridSpan`/`vMerge` dump in §2's
script): in each of those rows the **only** cell stating a 1 pt top is column 0, and column 0 is a
`<w:vMerge/>` **continuation** of the cell that started in row 0. Every other cell in the row states
`w:top w:val="nil"` or nothing.

`make-fixture7.py` is the one-attribute test. Two columns; the lower row is on a `w:trHeight` 450
floor; the arms vary which of its two cells states a 1 pt top and which states `nil` or nothing,
and whether the left column is one cell vertically merged over both rows.

| arm | lower-left top | lower-right top | left column | 26.2.4.2 | ours |
|---|---|---|---|--:|--:|
| `MBN` | 1 pt | `nil` | two cells | 23.50 | 23.50 |
| `MBA` | 1 pt | absent | two cells | 23.50 | 23.50 |
| `MNB` | `nil` | 1 pt | two cells | 23.50 | 23.50 |
| `MNN` | `nil` | `nil` | two cells | 22.50 | 22.50 |
| **`VBN`** | **1 pt** | `nil` | **`w:vMerge`** | **23.50** | **22.50** |
| **`VBA`** | **1 pt** | absent | **`w:vMerge`** | **23.50** | **22.50** |
| `VNB` | `nil` | 1 pt | `w:vMerge` | 23.50 | 23.50 |
| `VNN` | `nil` | `nil` | `w:vMerge` | 22.50 | 22.50 |

`data/trmix-ref.tsv`, `data/trmix-ours.tsv`; 16 of 18 arms agree and the two that do not are
`VBN` and `VBA`. **A row's `w:trHeight` floor is raised by the thickest top rule its cells state,
and 26.2.4.2 counts a `w:vMerge` continuation cell among them while we do not.** Those two arms
are the exact shape of the real document's rows 2 and 3.

An earlier arm set (`make-fixture6.py`, `data/trh-ref.tsv`) is the control that says this is *not*
the resolved-versus-stated question O84 answered: with one column and no merge, `BN` (upper row
states a 1 pt bottom, lower row states `nil`) gives **22.50 on both sides** — both charge the row's
own statement, and neither charges the resolved band, in the floor branch.

---

## 6. [bin] Both terms simulated on the real document, one attribute each

Neither fix can be *built* by this round, so each is simulated by re-spelling the document so that
the two renderers cannot disagree — and in both cases **26.2.4.2's own page-82 grid is byte-identical
to its rendering of the untouched file**, which is the control that says the patch measures us.

* **(a) `patch-doc.py`** re-spells the `19` run as an explicit `w:sz w:val="9"` (4.5 pt) with no
  escapement. §4's `Z` arm shows both renderers then need the same 1259 twips, which the real
  cell's 1260/1262 exceeds on both sides. Three runs in the document carry that exact `w:rPr`.
* **(b) `patch-both.py`** does (a) and additionally promotes **one** `nil` top to a 1 pt `single`
  in each of rows 2 and 3 — at a column whose facing edge above is already a 1 pt `single`, so the
  reference's resolved band there does not change. This is what our `OwnTopRule` would see if it
  could see the continuation cell.

| | last grid line, ours | vs the reference's 544.70 | Σ of the 33 row deltas | our pages | our page 83 |
|---|--:|--:|--:|--:|---|
| HEAD, untouched | 549.70 | **+5.00** | +5.05 | 167 | **blank (74 chars)** |
| (a) superscript only | 542.65 | **−2.05** | −2.00 | 166 | **correct (1751 chars)** |
| **(a) + (b)** | **544.65** | **−0.05** | **0.00** | 166 | **correct** |
| round 131's banked figure | 546.20 | +1.50 | +1.55 | — | blank |

`data/sz9-p82-rows.tsv`, `data/both-p82-rows.tsv`, `data/sz9-pagecensus.tsv`,
`data/both-pagecensus.tsv`.

**With both simulated the page-82 table lands within 0.05 pt of 26.2.4.2 and the 33 row deltas sum
to exactly zero**, and the document's page stream is **exact from page 1 to page 126**, including
page 83. The ten flipped-orientation pages go **10 → 2**.

**And the two that remain are a different defect, which this round did not chase.** From page 127
our stream runs one page *ahead*: 26.2.4.2 emits a near-empty page 127 (160 alphanumeric characters,
which is the header and footer and nothing else) and we do not, and our total is 166 against 167.
That page is invisible today because **the untouched document's two faults cancel in the page
stream as well as in the arithmetic** — our surplus page 83 and our missing page 127 are why
`pdfinfo` says 167 on both sides and why pages 128–167 align at HEAD. Closing O78 will expose it.

---

## 7. The seats, named and not made

### SEAT A — `Escapement.SizeOf` rounds 58% to a twip; 26.2.4.2 rounds it to a tenth of a point

`dotnet/src/XRay.Office.WordProcessing/Layout/Escapement.cs`:68

```csharp
public Length SizeOf(Length emSize)
    => Proportion is 0 or 100 ? emSize : Twips(emSize.Twips * Proportion / 100.0);
private static Length Twips(double value) => Length.FromTwips((long)Math.Round(value));
```

The measured rule is **the nearest tenth of a point**, 57 of 57 over base sizes 2.0–30.0 pt:

```csharp
// 26.2.4.2 quantises a proportional size to a whole tenth of a point, not to a twip: 58 % of
// eight point is 92.8 twips and it sets 92 (4.60 pt) where rounding to a twip sets 93 (4.65).
// Measured on probes/faa-r152/data/esc-rule.tsv, 57 of 57 base sizes, half away from zero.
=> Proportion is 0 or 100
     ? emSize
     : Length.FromTwips(2 * (long)Math.Floor(emSize.Twips * Proportion / 200.0 + 0.5));
```

The constant is **2 twips** — a tenth of a point — and the arithmetic is
`round(twips × proportion / 100 / 2) × 2` with half away from zero, which reproduces all 57.

**Read this before making the change.** `SizeOf` is the single seat all four words readers use
(`DocxLayoutSource.cs`:1238, `OdtLayoutSource.cs`:1259, `RtfReader.cs`:686, `DocReader.cs`:1221),
so the change reaches DOCX, ODT, RTF and WW8 at once. **57 of the words track's 271 `.docx` state a
`w:vertAlign` super/subscript**, the 66 `.doc` cannot be censused statically, and the two rules
differ at **29 of 57** half-point sizes, so this is a broad change and needs a whole-track sweep
either side of it. It is also *not* obviously the right shape for the ODF and RTF spellings, whose
proportions are arbitrary percentages rather than 58 — this round measured 58 only.

**And the sign matters for the order of work.** Seat A alone takes this document from **+5.00 pt to
−2.05** and closes its blank page 83; seat B alone takes it from +5.00 to **+7.00** and makes it
worse. If only one ships, it must be A.

### SEAT B — a `w:vMerge` continuation cell's stated top rule is dropped before `OwnTopRule` can see it

`dotnet/src/XRay.Office.WordProcessing/Layout/TableLayouter.cs`:1181 at HEAD is what answers wrongly:

```csharp
private static Length OwnTopRule(PageTableRow row)
{
    Length top = Length.Zero;
    foreach (PageTableCell cell in row.Cells) top = Length.Max(top, cell.Borders.Top.Width);
    return top;
}
```

but the cause is upstream, at
`dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.Tables.cs`:1391, in `Resolved`:

```csharp
if (cell.Merge == VerticalMerge.Continue) continue;   // the cell never reaches PageTableRow.Cells
```

Dropping the continuation cell is right for everything that draws it and wrong for this one
number: the cell states a `w:tcBorders/w:top` that 26.2.4.2 charges to the row's `w:trHeight`
floor. The change is to carry the widest top rule stated by the continuation cells a row drops —
a `CoveredTopRule` on `PageTableRow`, set in `Resolved` — and to take `Length.Max` with it in
`OwnTopRule`. The measured constant is the fixture's: `VBN` and `VBA` must answer **23.50**, a
**1.00 pt** charge on a 22.50 pt floor, where we answer 22.50.

**Scope it to `OwnTopRule` only.** Fixture 7 holds the upper row's bottom at a 1 pt `single` in
every arm, so `TopBand` is 1.00 throughout and the fixture says **nothing** about whether a
continuation cell's borders should also feed `TopBand` or `BottomBand`. Widening the change to
those two is unmeasured.

Neither change was made. `OPEN-ISSUES.md` and `PROVENANCE.tsv` were not touched.

---

## 8. Verdict on the seat, and what this round could not establish

### O78 is still open, its arithmetic is stale, and its conclusion was right

| the seat says | measured at HEAD, 2026-09-18 |
|---|---|
| displacement at the foot of page 82 is **1.5 pt** | **5.00 pt** |
| eleven rows short by **−5.25** | **two rows short by −1.95**, and they are a different mechanism |
| two rows tall by **+6.90** | **one row tall by +7.10**; the wrap itself is unchanged at 7.05 pt of row |
| the short rows are *"a row whose cells all state `w:bottom w:val="nil"`"* — **O84** | eight of those eleven are closed by round 149; what is left is a `w:vMerge` continuation's top rule |
| *"the fit turns on a tenth of a point in fifty-two … not the padding, the column width or the metric"* | true, and the tenth of a point is the **superscript's reduced size**: 4.60 pt against our 4.65 |
| *"both sides measure that string identically at 52.2 pt"* | the **base** string yes, exactly, 24 of 24; the `19` no |
| *"inner width of 52.3 (ours) against 52.2 (the reference's)"* | right, and it is the **wrong way** — we have the wider cell and still break |
| *"fixing O84 alone makes this worse"* | **confirmed**: 1.50 → 5.00 pt |
| round 149's *"O78 is untouched"* | **refuted** |

**The seat should be rewritten rather than closed**, with the two seats of §7 in it, and it is
worth saying in the row that round 149's own prediction about this document was wrong. Round 149's
headline measurement — page counts 171 of 179 exact before and after — was not wrong; it simply
could not see a document that was already displaced going further over its edge.

### What this round could not establish

1. **Why 26.2.4.2 quantises a proportional font size to a tenth of a point.** The rule is measured
   at 57 of 57 and the mechanism is not found. [src] `SwSubFont::SetSize`
   (`sw/source/core/inc/swfont.hxx`:772-784) is `m_aSize.Height() * GetPropr() / 100` in integer
   arithmetic, which in twips is `trunc` and fits only **30 of 57**; a round trip through 1/100 mm
   fits 30. Something between the item and the portion is working in tenths of a point and I did
   not find it. **The measurement that would settle it**: instrument `SwSubFont::SetSize` in a
   debug build, which this project's absolute rules forbid — so the honest alternative is a
   `--convert-to fodt` of fixture 5 and reading the `fo:font-size` the reference itself writes for
   each escaped run, which is a file-format channel rather than a rendering one.
2. **Why the reference's superscript advance stops matching a 4.60 pt size at k ≥ 8.** §4's
   table: at k = 8, 12, 16 the reference needs 1117, 1531 and 1945 twips where a constant 4.60 pt
   superscript predicts 1115, 1524 and 1934, and **no single per-`19` advance fits all eight arms**
   — k = 3 bounds it at ≤ 102.333 twips and k = 8 bounds it at > 102.5. Ours is constant at 8 of 8.
   This does not touch the real cell, which is k = 1, but it means §4's fourth measurement is a
   confirmation over five arms and not eight. **The measurement that would settle it**: the same
   sweep at two more base sizes, to see whether the drift is a function of the run's length in
   glyphs or of its width.
3. **Whether a `w:vMerge` continuation's borders should also feed `TopBand` and `BottomBand`.**
   Fixture 7 cannot see it (§7, seat B).
4. **The near-empty page 127 that 26.2.4.2 emits and we do not** (§6). It is the next defect on
   this document and it is uncharacterised. **The measurement that would settle it**: the same
   page-82 treatment applied to the table above body index ~1226+N that feeds page 127 of the
   `both` variant — pair its grid lines and find the room at its foot.
5. **Reach of seat A beyond DOCX.** The census is static and therefore `.docx`-only: 57 of 271.
   The 66 `.doc`, and every `.rtf`/`.odt`, are counted only as present.

### Provenance

Every figure in this file is against **26.2.4.2 `0229ac93…`** from `/opt/libreoffice26.2`, with an
absolute `-env:UserInstallation` and `timeout -k 30 900`, and against the CLI at HEAD `b5d46342a`.
The parent session rebuilt that CLI once during the round, at **10:00:47** (it had been 09:52:33).
**§1 and §2 were measured against the 09:52:33 binary and everything from §4 on against the
10:00:47 one.** The document was re-rendered against the later binary and `data/faa-p82-rows-r152b.tsv`
is byte-identical to `data/faa-p82-rows-r152.tsv`, so **no comparison in this file spans a change
in the tree.**

**And the binary is not HEAD alone, which has to be said.** The parent session is working in this
same tree on the table-split half of O83, and `Paginator.cs` and `TableLayouter.cs` carry
uncommitted changes (+65/−14) whose current mtimes — 10:04:17 and 10:16:14 — are *after* both
builds. So each binary is HEAD `b5d46342a` plus whatever state those two files were in at the
moment it was built, and that state cannot be recovered now. What can be said is that the two
builds produce **identical** page-82 geometry and an identical page census for this document, so
whatever differs between them does not reach it. **`OwnTopRule` is untouched by that diff and
identical at HEAD**, so seat B's quotation is HEAD's own text; `Escapement.cs` and
`DocxLayoutSource.Tables.cs` are clean.

Nothing outside `dotnet/probes/faa-r152/` was written. `OPEN-ISSUES.md`, `PROVENANCE.tsv`, the
source and the tests were not touched, and nothing was built.

### What is in this directory

| | |
|---|---|
| `ref-render.sh` | the only way the reference is invoked here: `/opt/libreoffice26.2`, absolute `-env:UserInstallation`, a private profile, `timeout -k 30 900` |
| `rules.py` | `probes/wordstable-r131/show-rules.py`, copied verbatim — the horizontal-rule reader |
| `vrules.py` | its vertical counterpart, used for the column widths in §4 |
| `pair-rows.py`, `pagecensus.py` | round 131's §4 pairing, reimplemented; and the per-page size/orientation/character census |
| `dump-rows.py` | a body-level table's per-row `w:trHeight` and stated top/bottom borders |
| `hmtx.py` | exact advance widths straight out of a TTF's `hmtx`, used for every prediction |
| `make-fixture.py` … `make-fixture7.py` | the seven one-attribute fixtures; each writes its own `.docx` into `data/` |
| `read-fit.py`, `read-esc.py`, `read-trh.py`, `fit4-score.py`, `esc-score.py` | their readers and scorers |
| `patch-doc.py`, `patch-both.py` | the two one-attribute variants of the real document |
| `census-vertalign.py` | the `w:vertAlign` census of the words track |
| `data/*.tsv` | every measurement quoted above |
| `out/` | only the small fixture renderings. **The four renderings of the 167-page document (two binaries × untouched/patched, ~8 MB each) are deleted and regenerable** — `ref-render.sh` takes eight seconds and the CLI seven. |

---

## Note on what is banked here

**The two patched copies of the corpus document (≈880 kB each) and the 15 rendered PDFs are not
committed.** Both are derived: the `.docx` are the corpus file re-spelt by this round's own scripts
(§5's simulations), and the renders follow from them. What is banked is everything a later round
would read — the scripts that build and measure, every `.tsv` of measurements, the census and the
logs, and this write-up. The corpus original is at
`words/*/docx/FAA 2025-26 Holdover Tables.docx` and must not be modified in place.
