# orphan3-r163 — "the 3" at the top of our page 15

Document: `/home/user/sample-files/words/metrics-001/docx/02_mcar_part-2_and_IS_v2.10.docx`
Reference: `/opt/libreoffice26.2/program/soffice` 26.2.4.2 (PDF Producer confirms `LibreOffice 26.2.4.2 (X86_64)`).
Ours: `dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/XRay.Office.Cli` (as built).
Page counts: reference 312, ours 314.

## 0. Provenance and staleness  (read first)

- **Reference side — sound.** All `[bin]` numbers for LibreOffice come from
  `/opt/libreoffice26.2/program/soffice`, 26.2.4.2 (PDF `Producer` confirms it). `/usr/bin/soffice`
  (24.2.7.2) was never used. Nothing below about the reference needs re-measuring.
- **Our side — Sep-17 build, NOT HEAD.** Every `[bin]` number attributed to XRay.Office was measured
  with `dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/XRay.Office.Cli`, mtime
  **2026-09-17 06:33**. HEAD is `bccfeb64b` (2026-09-20). **Three commits touched
  `Layout/TableLayouter.cs` after that binary was built**, all of them in this exact area:
  - `f2207fde9` (Sep 17 22:38) "The row below a table boundary pays the whole band (O84)" — replaced
    the half-top/half-bottom border model with `TopBand`/`BottomBand`.
  - `a3159ce0d` (Sep 18 11:25) "A split table row is ruled once at the cut, by its bottom rule (O83)"
    — added the `head` term to `SliceRow`'s fit test, made `border = BottomBand(row)`, and added
    `if (complete) height -= border`.
  - `ffbdb45ac` (Sep 18 23:36) "A cell a vertical merge covers is charged to the row's height".
  So **our measured geometry is pre-`a3159ce0d`** and must be re-run. §7 lists exactly what.
- **`8af1cbb21` (list counters keyed on the abstract definition) is not in play.** "The 3" is not a
  list marker — see §1. No `w:numPr` appears in the row, in `FAATableText`, or in `Normal`.
- **Source readings are at HEAD.** `git status` reports `TableLayouter.cs` clean, and the two
  passages quoted in §3 were re-read out of `git show HEAD:` after the binary's staleness came to
  light. The mechanism is present at HEAD.

## 1. What "the 3" is  [bin, reference side]

It is the **second line of the Date cell of the change-log table row
`2.11.1.1 | 11/2013 | Added notes`**. The Date column is 1109 dxa wide, so `11/2013` wraps
to `11/201` + `3`; the stranded `3` is that second line.

**It is not a list marker, a clause number or an autonumber.** [src] `word/document.xml`, the
`w:tr` with `w14:paraId="457B8AA3"`: three cells, three `w:p`, each `<w:pStyle w:val="FAATableText"/>`
plus `<w:spacing w:line="233" w:lineRule="auto"/>` and nothing else. Literal run text is
`2.11.1.1`, `11/2013`, `Added notes`. No `w:numPr` in the row; none in `FAATableText`; none in
`Normal`, which it is based on. So `8af1cbb21` (counting a list by its abstract definition) cannot
reach this object, and the numbering half of the brief's worry does not apply.

| | reference | ours |
|---|---|---|
| baseline of `2.11.1.1 … Added notes` | p14 y=84.35 | p14 y=66.55 |
| baseline of the `3` | **p14 y=71.90** | **p15 y=~709** (top of the page) |
| row bottom rule | p14 y=67.15 (whole row, 29.30 tall) | p14 y=61.80 (row split) |

## 2. Drift or local: BOTH exist and they are separable — the decisive one is local  [bin]

*Reference columns: 26.2.4.2, sound. Our columns: Sep-17 build, re-run per §7. The reference
columns alone settle the question, because the reference's own master part is 14.90 pt and our
rejection of a part that size does not depend on which of the two builds measured it.*

### 2a. Page 14 is not where the break happens

Both sides put **46 lines on page 14** (45 body lines + the repeated header row). Our page 14
simply *starts one line earlier in the document*: our first body line is
`(8) | 1 | Communication to (b)(9)`, which the reference had already placed as the last line
of its page 13. The one-line offset is inherited, not created, on page 14.

Matching the two sides line-for-line over pages 3–16 (header/footer lines removed):

```
 pg  ref_start ours_start  offset
  3         0          0        0
  4        45         44       -1
  5        90         89       -1
 ...       ...        ...      -1
 16       586        585       -1
```

The sequences are otherwise identical. **One event, at the bottom of page 3, offsets everything
through page 16; pages 17+ re-sync.**

### 2b. Baseline table for the drift question

Baselines read from the content streams (`Td`/`TJ` via `render-comparison/scripts/pdf-ops.py`),
not `pdftotext -bbox`. Page 3, every line, both sides:

```
 i   refY    ourY   cum.diff  refGap ourGap
  0  723.60  723.55   0.05
  1  704.05  703.95   0.10     19.55  19.60   <- row boundary
  2  691.60  691.50   0.10     12.45  12.45   <- inside a cell
  3  679.15  679.05   0.10     12.45  12.45
  4  658.25  658.10   0.15     20.90  20.95   <- row boundary
  5  641.35  641.15   0.20     16.90  16.95   <- row boundary
  6  628.90  628.70   0.20     12.45  12.45
 ...
 43   88.15   87.05   1.10     16.90  16.95   <- row boundary
 44   75.70   74.60   1.10     12.45  12.45
 45   58.80   (none)                          <- the line we drop
```

The gap **grows monotonically, in steps of exactly 0.05 pt (1 twip), and only at table-row
boundaries**. Intra-paragraph line pitch is 12.45 pt on both sides at every one of ~20 samples.
Accumulated drift over page 3: **1.10 pt**.

Fitting row height against line count over pages 4–12 (rule-to-rule, from the stroke operators):

| lines in row | n | ref mean | ours mean |
|---|---|---|---|
| 2 | 183 / 187 | 29.3503 | 29.3947 |
| 4 | 1 | 54.20 | 54.30 |
| 5 | 1 | 66.70 | 66.75 |

Solving `h = a·lines + b`: per-line **12.4499 (ref) vs 12.4518 (ours)** — equal;
row overhead **4.4505 (ref) vs 4.4911 (ours)** — ours is ~0.04 pt/row too big.
So the drift is real, it lives entirely in the row overhead (spacing + boundary rule), and it is
**~0.04–0.05 pt per row**. It is *not* what loses the line: see below.

### 2c. The local event

At the bottom of page 3 the reference **splits** the row
`2.1.1.2 | 05/2010 | Added definition: Safety Management System`, keeping line 1 of all three
cells on page 3 and carrying only the Date cell's `0` to page 4. We move the whole row.

Measured from the rules:

| | reference | ours |
|---|---|---|
| last complete row's bottom rule (p3) | 70.95 | 69.85 |
| body bottom (`w:pgMar w:bottom="1080"`, 792 pt page) | 54.00 | 54.00 |
| room left | 16.95 | 15.85 |
| height the split part was charged | **14.90** (placed) | **16.95** (refused) — Sep-17 build |

Ours needs 2.05 pt more for the same part. Even with the 1.10 pt of drift, the reference's
14.90 pt part would still have fitted in our 15.85 pt (0.90 pt to spare). **Removing the drift
alone would not reliably fix it** (16.95 pt of room against a 16.95 pt demand is a coin flip);
removing the over-charge does.

## 3. Root cause  [bin] + [src]

The reference's master (first) part of a split row **prefers** to include the trailing
`w:spacing w:after` of the cells whose content finished above the cut, **but drops it entirely
when the part would not otherwise fit, and takes the split anyway.** We treat that trailing
spacing as mandatory, so the part is 2 pt too tall and we decline the split.

### Fixture (`probe-mastersplit.py`, `probe-room.py`, `mkdocx2.py`)

`mkdocx2.py` writes **`word/settings.xml`** (the sibling builder in
`probes/words-regress-01/mkdocx.py` does not — the trap the brief names). Fixture: three-row,
two-column bordered table; every cell paragraph `w:spacing w:before="40" w:line="233"
w:lineRule="auto"`; `w:after` swept; last row's cell A holds one line (finishes), cell B holds
`05/2010` in a 1109 dxa column (wraps to two lines, so the row splits). Bottom margin swept in
10-twip steps to vary the room continuously.

`after=240` (12 pt), filler=47, reference only — master part height against bottom margin:

```
 mb(tw) bodyBot  prevRule lastRule masterH  p1 lines
   1100   55.00     82.35    55.45   26.90      50     <- before + line + after + border
   1110   55.50     82.35    67.45   14.90      50     <- before + line + border
   ...                                14.90              (unchanged down to room 15.85)
```

**All-or-nothing**: nowhere in the sweep does an intermediate value appear, so the spacing is
dropped whole, not clipped to the room. Same shape at `after=40`: 16.90 until the room falls
below it, then 14.90.

Our side on the same fixture, `after=240`:

```
   1100   55.00   ours  26.70   50 lines on p1, 1 on p2     (agrees)
   1120   56.00   ours  26.95   49 lines on p1, 2 on p2     (whole row moved — the defect)
```

### The fixture predicts the corpus exactly

Reference fit threshold, read off the sweep: the part's bottom rule must sit at
`bodyBottom + 0.25` or below-bounded (half the 0.5 pt rule). Corpus page 3: a part charged the
trailing spacing is 16.90 tall, putting its bottom rule at `70.95 − 16.90 = 54.05`, which is
under `54.00 + 0.25`. So the reference drops the spacing and uses 14.90 →
`70.95 − 14.90 = 56.05`, **which is exactly where the reference draws it**.

### Our source  [src], read at HEAD `bccfeb64b`

`dotnet/src/XRay.Office.WordProcessing/Layout/TableLayouter.cs`, `HeightAt`, line 703:

```csharp
else if (!tableBelow
         && flow.Lines.Count > 0
         && flow.Area.Y + flow.Lines[^1].Top + flow.Lines[^1].Box.Height <= cut)
{
    // Nothing of this cell is left over, so its part is as tall as the cell — the trailing
    // spacing included, which is what `LayOut` charged the row for.
    bottom = Length.Max(bottom, flow.Area.Y + flow.Advance);
}
```

`PlacedFlow.Advance` carries the last paragraph's space-after. `SliceRow`'s fit loop (line 470)
tests only this one height:

```csharp
Length needed = head + HeightAt(cells, rowTop, above, candidate, border, keepsSpacingAtPages);
if (needed > room) break;
```

There is no second, tighter measure to fall back on, so a part that overflows only by its
trailing spacing is rejected and the caller places the row whole on the next page. **Both passages
are present verbatim at HEAD** (`git show HEAD:…` lines 694-710 and 468-482), so the mechanism is
not one of the three post-binary commits' casualties.

`a3159ce0d` in fact makes the master part *dearer*, not cheaper: before it, the fit test was
`HeightAt(...)` alone with `border = BorderHeight(row)` (half the top rule plus half the bottom);
at HEAD it is `head + HeightAt(...)` with `head = bandAbove` and `border = BottomBand(row)`, i.e.
one whole band at the head and another at the cut. On this uniformly 0.5 pt-ruled table the
reference's entire master part spans 14.90 pt from boundary rule to cut rule, so HEAD is expected
to demand ~0.5 pt *more* than the Sep-17 build did, not less. That is a prediction, not a
measurement — see §7.

The existing doc-comment on `UpperSpaceAbove` records a 12-probe sweep that measured the
**follow** part as `before + remaining lines + after + border` — which is right, and which this
finding does not disturb. The **master** part's trailing spacing was never swept.

## 4. Proposed patch (NOT applied)

File: `dotnet/src/XRay.Office.WordProcessing/Layout/TableLayouter.cs`

1. `HeightAt` (line 612): add a `bool chargeTrailing` parameter. When false, skip the
   `else if` at line 703 so `bottom` stays at the last line's bottom (`Advance` not consulted).
   Leave everything else, including the `following is { } next` branch, untouched.

2. `SliceRow`: move the `last` computation (currently line 536, after the search) to before the
   candidate loop — it does not depend on the chosen cut.

3. Replace the fit test in both candidate loops (lines 470-480 and 519-529) with:

```csharp
bool completesRow = candidate >= last;
Length needed = head + HeightAt(cells, rowTop, above, candidate, border,
                                keepsSpacingAtPages, chargeTrailing: true);
if (needed > room)
{
    // A part that CUTS the row may drop the trailing space-after of the cells that
    // finished above the cut rather than decline the split.  [bin] probes/orphan3-r163:
    // 26.2.4.2 charges it when it fits and drops the whole of it when it does not, with
    // no intermediate value anywhere in a 0.5 pt-step sweep of the room.
    if (completesRow) break;
    needed = head + HeightAt(cells, rowTop, above, candidate, border,
                             keepsSpacingAtPages, chargeTrailing: false);
    if (needed > room) break;
}
chosen = candidate;
height = needed;
```

`height` is what `Sliced` builds the rectangles to and what the caller advances by, so the
reduced value propagates to the bottom rule — which is what puts it at 56.05 instead of 54.05.

Both measures are non-decreasing in the cut, so `break` on the tight measure still terminates the
deepest-cut search correctly.

Current behaviour: a split row's first part always pays the finished cells' `w:spacing w:after`;
if that does not fit, the whole row moves.
Corrected behaviour: it pays it when it fits, and drops it entirely when it does not, splitting
anyway.

## 5. Ruled out

- **Accumulated measurement drift as *the* cause.** It exists (0.04–0.05 pt per table row,
  1.10 pt over page 3) but is an order of magnitude smaller than the 2.05 pt discrepancy and
  leaves 0.90 pt of headroom at the decisive point. Reported separately below.
- **Line-height / font-metric error.** Intra-paragraph pitch is 12.45 pt on both sides at every
  sample on pages 3 and 14. The row-height fit gives 12.4499 (ref) vs 12.4518 (ours) per line.
- **`w:widowControl` / orphan-widow control.** The object is a table row, not a body paragraph;
  the divergence is reproduced by a three-row fixture with no widow/orphan settings at all.
- **`w:keepNext` / `w:keepLines`.** Absent from the row, its neighbours, and the `FAATableText`
  style; the fixture reproduces without them.
- **Space-before collapse at the top of a page.** The follow part's `before` is charged
  identically on both sides (ref 16.90 = 2 + 12.45 + 2 + 0.45 on p4), which is the existing
  `UpperSpaceAbove` behaviour and is correct.
- **`w:cantSplit` / `tblHeader`.** The row carries no `trPr`; the fixture has no repeated header
  and behaves the same.

## 6. Open / not settled

- **The 0.04–0.05 pt (1 twip) per-row overhead.** Ours 4.4911 pt vs reference 4.4505 pt of
  row overhead, with `w:spacing before=40 after=40` (2 pt + 2 pt) and a `w:sz="4"` (0.5 pt)
  `insideH` rule. `2 + 2 + 0.5 = 4.50` is exactly our figure, so the reference is charging
  ~0.45 pt rather than 0.50 pt for the boundary rule — but the reference's own row heights are
  not on the twip grid (2-line rows alternate 29.30/29.40 around a mean of 29.3503), so the
  residual is sub-twip and its origin was not established here. A separate seat.
- **Doubled boundary rule at our split rows.** Our pages 13/14/15 each draw two horizontal rules
  0.25 pt apart at the head of a split part (e.g. p14 `78.50` and `78.75`); the reference draws
  one. Cosmetic, not implicated in the page break. A separate seat.
- Whether the reference also drops a cell's **bottom padding** along with the space-after when it
  compresses: this table's `tblCellMar` bottom is 0, so the two cannot be told apart here.

## 7. What must be re-measured against a HEAD build

Everything in this file about the **reference** stands as measured. The following are Sep-17-build
numbers and need re-running once `dotnet/tools/XRay.Office.Cli` is rebuilt at `bccfeb64b` or later.
All the harness is in this directory and takes about a minute.

| what | how | what to check |
|---|---|---|
| The corpus symptom still exists | `XRay.Office.Cli render` the document, `pdftotext -f 14 -l 15` | Does our page 15 still open with `3`? Is the page count still 314 against 312? |
| The one-line offset across pages 3-16 | the inline script in §2a (recorded as `pagesum.py` / the offset table) | Still `-1` from page 4 through page 16? |
| Our page-3 geometry | `python3 ybase.py ours/…pdf 3` and the stroke dump | Last complete row's bottom rule (was 69.85), room against the 54.00 body bottom (was 15.85) |
| Our split-part height | stroke dump of our pages 14 and 15 | Was 16.95 rule-to-rule. `a3159ce0d` predicts ~17.45 at HEAD (one band at the head plus one at the cut) |
| The 0.04-0.05 pt/row overhead drift | `python3 rowfit.py` | `f2207fde9` rewrote the band model; the per-row overhead (ours 4.4911 vs ref 4.4505) may have moved |
| The doubled rule at a split | stroke dump, pages 13-15 | **Probably already fixed** by `a3159ce0d` ("ruled once at the cut"). Withdraw that open seat if the two strokes 0.25 pt apart are gone |
| The fixture crossover | `python3 probe-room.py 240 47 1090 1200` and `python3 probe-room.py 40 49 1180 1220` | The reference columns are fixed. Re-read only the `ours` rows: does HEAD still move the whole row where 26.2.4.2 splits it? |

The reference-side conclusions that do **not** need re-running: what "the 3" is; that the reference
splits the row at the foot of page 3; that its master part is 14.90 pt there and 56.05 pt is where
it rules it; the whole `probe-room.py` reference sweep including the all-or-nothing collapse of the
trailing spacing; and the arithmetic that shows the fixture's rule predicts the corpus exactly.
