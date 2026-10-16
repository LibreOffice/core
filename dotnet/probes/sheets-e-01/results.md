# sheets-e-01 — `###`, the accounting `$`, and the grid

Three text-visible divergences the gate round exposed. One is implemented and swept, one is
diagnosed to a seat and deliberately not implemented, one has its rule measured in full and its
implementation handed over. Taken in the briefed order.

Reference: `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/sheets/` (171 PDFs, per-format identity
`stem__ext.pdf`). `check-env.sh` before anything else:

> **LibreOffice 26.2.4.2 620(Build:2)** · Calibri→Carlito, Cambria→Caladea, Arial→Liberation
> Sans, Times New Roman→Liberation Serif, Courier New→Liberation Mono, DejaVu Sans→DejaVu Sans ·
> pdftoppm 26.01.0 · pdftotext 26.01.0 · **"Environment is good."** · `df -h /`: 8.8 GB free.

Ours: worktree `/c/sandbox/workdir/wt-sheets-e`, branch `wt-sheets-e`, based on `735e08c525f`;
`XRAY_OFFICE_CLI` set explicitly on every render; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

**Measured** means read out of a PDF or produced by an authored probe against 26.2.4.2.
**Inferred** means read out of the C++, which is **27.2.0.0.alpha0+ and not the reference
binary**; nothing is claimed on it alone.

---

## 0. The prediction, and the first thing it got right

`prediction.md` beside this file, committed as **`4933bb8f462`** before a single PDF was opened,
before a probe was rendered and before a line of source was changed. Scored in §7.

Its first item is the one that matters most:

> **P0.1 — the brief's direction on `###` is inverted.** The brief says *"we draw 1101 where the
> reference draws 2."* `gate-01` says the opposite in three places. Measurement will show
> reference ≈1101, ours ≈2.

**Measured, on `ODs-February-2022-Airbus-Commercial-Aircraft.xlsx`:**

| | `###` tokens |
|---|---:|
| reference (26.2.4.2) | **1101** |
| ours, before this round | **2** |
| ours, after | **1101** |

The brief is inverted. **We were under-producing `###`, not over-producing it**, and every
downstream direction in this round is therefore the reverse of what the brief implies. Read the
brief's sentence again — *"We are producing it at scale where LibreOffice is not"* — and then the
`gate-01` line it was built from: *"the reference emits `###` 1101 times … and we emit it twice."*
This is the round's required refutation of its own brief, and it was committed as a prediction
rather than discovered and back-filled.

---

## 1. `###` — the rule, measured

### 1.1 The census that named the seat before any rule was established

`ODs-February-2022-Airbus-Commercial-Aircraft.xlsx`, walked as records rather than regexed:
every `<c r="A…">` carrying a numeric `<v>` on all thirteen sheets, joined to `xl/styles.xml`'s
`cellXfs`.

| | count |
|---|---:|
| numeric column-A cells, all thirteen sheets | **1101** |
| of which `numFmtId="0"` — **`General`** | **1099** |
| of which `numFmtId="1"` — a stated `0` | **2** |
| reference `###` tokens in the PDF | **1101** |
| ours before | **2** |
| ours, `1E+00`/`1E+04`/`1E+11`-shaped tokens before | **1099** |

Column A is `width="0.42578125"` — narrower than one digit. **The two counts that agree are the
two non-`General` cells**, and they agree because our `Hash` already implemented the
non-`General` branch. The 1099 we got wrong are exactly the `General` ones, and we drew `1E+00`
for each.

### 1.2 The authored probe

`tests/corpus/features/sheet-hash.fods` (`probes/sheets-e-01/mkhash.py`), rendered by 26.2.4.2
itself. Fourteen rows, **one variable each**, across **twenty column widths** from 0.10 cm to
4.00 cm — so every variant is swept across its boundary rather than sampled on one side of it,
which is more than the brief's "at least two points" asked for and is what made the boundary
comparison below possible. Read back cell by cell by `probes/sheets-e-01/cells.py`, which
assigns a drawn word to a column by its **right** edge: a `###` that does not fit overhangs to
the *left* of its own cell, so a census keyed on the centre or the left edge mis-files exactly
the cells this round is about.

| variant | 26.2.4.2 draws | boundary |
|---|---|---|
| `General`, `1` | `###` ×4, then `1` ×16 | 0.25 → 0.30 cm |
| `General`, `12345` | `###` ×12, then `12345` ×8 | 1.00 → 1.20 cm |
| `General`, `123456789012` | `###` ×12, then `1E+11`, `1E+11`, `1.23E+11`, `1.235E+11`, `1.2346E+11`, `123456789012` ×3 | 1.00 → 1.20 cm |
| `General`, `1.5` | `###` ×4, then **`2` ×3**, then `1.5` ×13 | decimals dropped first |
| `General`, `-1` | `###` ×5, then `-1` ×15 | the sign costs one column |
| `0.00`, `1` | `###` ×9, then `1.00` ×11 | **never shortened** |
| `0`, `1` | `###` ×4, then `1` ×16 | control on the non-`General` branch |
| percent, `0.5` | `###` ×13, then `50.00%` ×7 | |
| date | `###` ×16, then `28/02/2022` ×4 | a date hashes; it is not text |
| string `XX` | never `###` at any width; shortened to `X` at one | |
| shrink-to-fit, `12345` | `12345` at **all twenty widths** | shrink suppresses `###` outright |
| wrap, `General` `12345` | identical to the unwrapped row | wrap does not save a plain number |
| wrap, date | `###` on **one line**, row keeps its single-line height | `###` is never broken |
| left-aligned `General` | same boundary as right-aligned | alignment does not move it |

Second sheet, the value-versus-string asymmetry at one width (0.30 cm) with a controlled
neighbour: `###` · `ABCDEFGH` · `A`+`Z` · `###`. **A value never borrows an empty neighbour's
width and a string always does**, so the same column hashes a number and shows a string whole.

**The rule, stated.** A cell draws `###` iff it holds a **value** (or a formula returning one),
its output area is **clipped** — decided against its own column width minus margins, never
widened into a neighbour — and then:

1. a **formula in error**, or "show formulas", hashes; else
2. a format other than **`General`** hashes **outright**, with no attempt to shorten; else
3. the value is re-rendered through `General` with as many characters as the column has
   max-digit widths, dropping decimals and falling back to scientific notation — **and if that
   re-rendered text is still wider than the column, the cell hashes after all.**

**Shrink-to-fit removes the clip before the gate is reached, so it hashes nothing at any width.**

*Inferred, and matching*: `ScDrawStringsVars::SetTextToWidthOrHash`
(`sc/source/ui/view/output2.cxx:610-716`); the gate at `:1974`; `bCellIsValue` suppressing the
spill loop at `:1330`. Step 3 is `:704-710`, three lines long, comment "Even after the decimal
adjustment the text doesn't fit. Give up."

### 1.3 What we had, and what was missing

`SheetTextLayout.Hash` implemented steps 1 and 2 and **stopped**. It rendered the shortened
`General` form and returned it unconditionally. There was no measurement of the result and no
fallback, so a column narrower than one digit produced `Render(value, 0)` → `1E+00`, drew it, and
that was that. The docstring on `SheetGeneralWidth` even states the *reason* the rule is not
"hash anything that does not fit" — and the missing half is the reason it is not "never hash a
`General` cell" either.

## 2. The implementation

`dotnet/src/XRay.Office.Spreadsheets/Layout/SheetTextLayout.cs`, one file, one project.

* **`Hash` (`:892-914`)** now shapes the shortened `General` text and compares its width against
  the available width, falling back to `###` when it still does not fit. It returns a third
  value, `Hashed`, because two things downstream need to know.
* **The wrap suppression (`:474-483`)**: `lines = breaks && !hashed ? Wrap(…) : [run]`. Measured
  on the fixture's `wrapdate` row — 26.2.4.2 draws `###` on one line in a wrapping cell and keeps
  the row's single-line height; we drew **three lines of one `#`**, which moves every row under it.
* Re-shape and re-measure rather than counting characters: the budget is in digit widths and the
  answer is a shaped run, so only the run can decide. Counting characters is what produced the
  original defect.

Nothing else in `src/` changed. The whole diff is inside `XRay.Office.Spreadsheets`, which
`XRay.Office.WordProcessing` and `XRay.Office.Presentations` are siblings of and cannot reach — so
P4.4's static cross-track argument holds and no 534-document sweep is owed (the `sheets-d-01`
distinction: a shared-layer move owes a measurement, a leaf change owes a proof that it is a leaf).

### 2.1 The probe, re-read after the fix

Every one of the fourteen sweep rows now reproduces 26.2.4.2 cell for cell, with **one residual**,
which is pre-existing and untouched:

| width | reference | ours |
|---|---|---|
| 1.60 cm | `1.23E+11` | `1.2E+11` |
| 1.80 cm | `1.235E+11` | `1.23E+11` |
| 2.00 cm | `1.2346E+11` | `1.235E+11` |

Our scientific mantissa carries **one digit fewer** than the reference's at three widths. That is
`SheetGeneralWidth.Scientific`'s budget arithmetic, not the hash rule, and it is *safe* in the one
direction that matters here: our form is **shorter**, so it fits wherever the reference's does and
cannot manufacture a spurious `###`. Recorded rather than fixed — changing the budget is a
separate change with its own sweep.

## 3. Reach, direction and verdict movement

Both banks rendered with `SOURCE_DATE_EPOCH=1700000000` and `TZ=UTC`, all 171, **re-counted from
disk** (171 and 171) after the sweeping process exited, not when the file count reached its
target. Both binaries **validated before use**: the pre-fix build draws `1E+00` on the fixture's
first row and the post-fix build draws `###`; the post-fix bank's `ODs-February` reproduces
**byte for byte** when re-rendered individually.

### 3.1 Reach — 2 of 171, and my prediction was an order of magnitude high

**Two renderings change**, byte for byte, out of 171. P4.1 predicted 8–35 with a point estimate
of 18. **Refuted.** The reasoning behind the estimate was "most workbooks size their columns to
their content", which was right about the direction and wrong about the magnitude: a column
narrower than a single digit is not a common accident, it is a deliberate spacer column, and two
workbooks in the corpus use one to hold a number.

| document | `###` before | after | reference |
|---|---:|---:|---:|
| `ODs-February-2022-Airbus-Commercial-Aircraft.xlsx` | 2 | **1101** | **1101** |
| `18-02RD301_ILS_components_Master_9-13-18.xls` | 1 | **97** | **723** |
| total, the two changed documents | **3** | **1198** | **1824** |

**Direction: 2 closer, 0 unchanged, 0 further.** P4.2 holds — no document's `###` count
overshoots the reference's, which is the check that would have caught over-application.

`ODs-February` is **exact**: 1101 against 1101, which is also the number of numeric column-A
cells the record census counted (§1.1). Three independent numbers agreeing on 1101 is what makes
this a rule rather than a fit.

Over the **whole track**, all 171 documents on each of the three banks:

| | `###` tokens, 171 documents |
|---|---:|
| ours, before | **2640** |
| ours, after | **3835** |
| reference | **4424** |

The change closes **1195 of the 1784-token gap, 67%**, and the +1195 reconciles exactly with the
two changed documents' 3 → 1198. **The aggregate hides offsetting errors and should be read with
that stated**: §3.2's single document is 626 tokens short on its own, which is *more* than the
589 the track is short overall — so somewhere in the other 169 documents we draw about 37 `###`
more than the reference does. Those documents are byte-identical before and after, so that
surplus is pre-existing and untouched, but a reader who took 589 as "626 minus progress" would be
reading a difference of two errors as one. (`sheets-d-01`'s rule, in a second dress: a count that
sums across cases measures something other than the case.)

### 3.2 The residue, named rather than rounded away

`18-02RD301_ILS_components_Master_9-13-18.xls` goes 1 → 97 against a reference of 723. **626
cells still fail to hash and this round does not explain them.** They are visible in the
reference's text as `… 200 ### FT 200 3 HIRL PAPI …` — a numeric column beside a unit column —
where ours draws the number. The document is a `.xls`, so its column widths and formats come
through the BIFF path rather than the SpreadsheetML one, and its page count already agrees
(166 = 166). It is the first thing a successor should take, and it is the reason §3.1's headline
is "2 documents" rather than "`###` is fixed".

### 3.3 Verdict movement — zero, exactly as predicted, and predicted plainly

The verdict rule is `words-rebase-02/verdict.py`, unchanged; this round moves its *input* only.
Every one of the 171 has **0 unembedded fonts** on our side (`unembedded.tsv`, asserted rather
than assumed), so the verdict is decided by pages and words alone.

| | match | words | pages,words | pages |
|---|---:|---:|---:|---:|
| before | **146** | 16 | 5 | 4 |
| after | **146** | 16 | 4 | 5 |

**Sheets is 146 of 171 before and after.** That reproduces `sheets-d-01`'s scoreboard to the
digit — 146 / 16 / 5 / 4 — computed here from `xray-office analyze` rather than from `pdftotext`,
which is a second instrument returning the same four numbers.

**One document's verdict string changes and the scoreboard does not move**, which is exactly
P4.3:

| | words ours | reference | Δ | band (2% + 3) | verdict |
|---|---:|---:|---:|---:|---|
| `ODs-February`, before | 16 635 | 15 740 | **+895** | 317 | `pages,words` |
| `ODs-February`, after | 15 536 | 15 740 | **−204** | 317 | `pages` |

P4.3 predicted **−206**; the measured figure is **−204**. The document still fails check 1 on
pages (154 against 175), so it does not enter the scoreboard — and the words verdict it now
carries is true where the one it carried was false. This is the round the gate *could* have
moved, and it did not, for the reason predicted before it was measured.

### 3.4 What the byte comparison also settles

169 of 171 renderings are **byte-identical** before and after. So the wrap-suppression half of
the change (§2) reaches nothing in this corpus beyond the two documents above, and the row-height
concern it raised is out of its reach entirely — recorded in §10 rather than claimed as fixed.

## 4. The accounting `$`/`-` — refuted as a rendering defect, and then seated anyway

`fy2011-aip-grants.xls`. The brief: *"a workbook writes 11 538 `$`/`-` tokens against the
reference's 9020 … we are emitting more of them than LibreOffice does."*

### 4.1 We are not emitting more of them. We are emitting exactly as many.

Read with **`xray-office analyze`**, one binary over both PDFs in one pass:

| | `$` characters | `-` characters | raw tokens | letter-or-digit | non-alphanumeric |
|---|---:|---:|---:|---:|---:|
| reference | **8242** | **5516** | **56 732** | **43 208** | **13 524** |
| ours | **8242** | **5516** | **56 732** | **43 208** | **13 524** |

**Identical in every column.** Not "within a few percent", which is what P2.1 predicted —
identical to the digit.

The briefed 11 538 / 9020 reproduces exactly, and only, under **poppler**:

| | `$` chars | raw | letter-or-digit | non-alphanumeric | top tokens |
|---|---:|---:|---:|---:|---|
| reference, `pdftotext` | 8242 | 52 221 | 43 201 | **9020** | `$`×6241, `-`×1881, `$-`×379, `$$-`×132, `$$$-`×75 |
| ours, `pdftotext` | 8242 | 54 739 | 43 201 | **11 538** | `$`×8236, `-`×3142 |

**Both PDFs hold the same 8242 `$` glyphs.** The 2518-token gap is entirely poppler joining `$`
to the `-` beside it on one side and not the other. It is not even stable inside poppler:
`pdftotext -bbox` produces **no** joined `$-` token on either side, so the same binary in its
other mode disagrees with itself about this document.

**So the brief's framing of subject 2 is refuted.** Nothing is drawn more often on our side.

### 4.2 What *is* different, measured, and it is 0.6 em

The joining is a spacing difference, so the spacing was measured. Over eight pages, every `$` and
`-` matched positionally between the two renders:

| glyph | our x − reference x | on |
|---|---:|---:|
| `-` | **+6.27 to +6.29 pt** | 350 of 474 |
| `$` | **−0.52 to −0.53 pt** | 350 of 804 |
| both | within 0.21 pt | the remainder |

On one cell: the reference puts `$` at 613.38 and `-` at 620.71, a **2.26 pt** gap that poppler
joins; we put them at 612.86 and 626.98, a **9.05 pt** gap that it does not.

### 4.3 The seat

The workbook's own `FORMAT` records, read by walking BIFF `0x041E` records:

```
41  _(* #,##0_);_(* \(#,##0\);_(* "-"_);_(@_)
44  _("$"* #,##0.00_);_("$"* \(#,##0.00\);_("$"* "-"??_);_(@_)
165 _("$"* #,##0_);_("$"* \(#,##0\);_("$"* "-"??_);_(@_)
```

The zero section is `_("$"* "-"??_)`. Two blanks decide the `-`'s position, and **we render both
as a plain space**:

* `?` → `output.Append(' ')` — `XRay.Office.Core/Numbers/NumberFormatter.cs:559` and `:580`.
  LibreOffice reserves a **digit-width** blank.
* `_x` → `FormatToken.Literal(" ")` — `XRay.Office.Core/Numbers/NumberFormatSection.cs:155-157`,
  whose own comment says *"There is no column to align to during extraction, so a single space
  stands in"*. LibreOffice reserves the width of `x`.

Arithmetic, in ems of Liberation Sans (digit 0.556, space 0.278, `(`/`)` 0.333):

| | LibreOffice | ours | difference | measured at a 10 pt face |
|---|---:|---:|---:|---:|
| leading `_(` | 0.333 | 0.278 | **0.055 em** | **0.052–0.053 em** (the `$`, −0.52 pt) |
| trailing `??` + `_)` | 1.445 | 0.834 | **0.611 em** | **0.627–0.629 em** (the `-`, +6.28 pt) |

The leading blank matches the prediction to 0.003 em and the trailing to 0.017 em, and the
**signs and the ratio are right on both**, which is what makes the attribution safe. The exact
blank widths LibreOffice uses are not pinned to the last digit and that is stated rather than
rounded away.

### 4.4 Not implemented, deliberately

Three reasons, in order:

1. **It moves no verdict and cannot.** The corrected gate already scores this document
   `match` — 43 201 letter-or-digit words on both sides. The gap is invisible to the metric by
   construction, because a `$` carries no letter and no digit.
2. **The fix is in `XRay.Office.Core`**, not in `XRay.Office.Spreadsheets`. `Core/Numbers` is reached
   by word processing, presentations and every chart axis, so it owes the 534-document sweep this
   round did not budget for — the `sheets-c-01` rule, applied to myself.
3. The change needs a *width* where the formatter currently has only a *string*: `_x` and `?` are
   blanks whose width depends on the face, and `FormatToken` carries neither. The comment at
   `NumberFormatSection.cs:155` already says why the space is there, and it is right for
   extraction; a rendering path needs a second channel rather than a different literal.

Handed over as a `XRay.Office.Core` item with the measurement attached, which is worth more than a
rushed change to a shared layer.

## 5. The grid

`6f9e605c-fded-11e3-bd0e-00144feab7de.xls` page 1: the reference draws **107** vertical 0.1 pt
rules where we draw **17**.

### 5.1 We do draw the grid, and the 107 is not 107 rules

The reference's 107 verticals sit at **eighteen** distinct *x* positions:

| x | segments |
|---|---:|
| 20.778 | 1 — the block's frame, not the grid |
| **170.561** | **30** |
| **187.143** | **31** |
| **215.631** | **31** |
| the other fourteen | 1 each |

So the reference draws seventeen grid rules and one frame edge; **we draw seventeen grid rules**
at the same positions. **P3.1 holds: we draw the grid on that page, and the rule *set* already
agrees.** The 107-against-17 is three rules split into one segment per row, with a ~0.113 pt gap
at each row boundary — and **no holes**: the split rules run continuously from 77.583 to 525.287.
**P3.4 is refuted** — I predicted the reference's grid would have missing rules where text spills,
and on this page it has none.

### 5.2 The rule, measured on an authored fixture

`tests/corpus/features/sheet-grid.fods` (`probes/sheets-e-01/mkgrid.py`) — four sheets, one
variable each, grid printing on. Rendered by 26.2.4.2, censused with `sheets-d-01/strokes.py`:

| sheet | what varies | reference draws | ours draws |
|---|---|---|---|
| `control` | nothing, five 2 cm columns | 5 whole-height rules | **5, identical** |
| `hidden-C` | column C hidden | column B's rule **split into 6 per-row segments**, no hole; C's rule absent | 4 whole-height rules, **not split** |
| `merge-CD` | C:D merged on one row | column C's rule split per row **with a hole at the merged row** | 2 runs with the hole, **not split** |
| `overflow-B` | a long string in B spilling across C, one row | column B's rule split per row **with a hole at that row** | **one whole-height rule, no hole** |

**The rule, stated.** A column's vertical grid rule is emitted **one segment per row** whenever
*any* row of the page would suppress it — because the next column is hidden, because a merge
covers the cell to its right, or because a string in that column overflowed across the boundary.
Within that per-row emission the segment is **omitted** on exactly the rows where the merge or
the overflow holds; a hidden neighbour splits without omitting anything. Otherwise the rule is one
stroke down the whole block.

*Inferred, and matching*: `ScOutputData::DrawGrid`'s `bSingle`
(`sc/source/ui/view/output.cxx:456-513`); `bHideGrid` set by `GetOutputArea` at
`output2.cxx:1338` and `:1345` on every column a string extended across.

### 5.3 Our gap, split into what is ink and what is not

* **Segmentation** — 107 against 17 — is not ink. Abutting hairlines with a 0.113 pt gap in a
  0.1 pt rule; the *set* of rules already agrees, position for position.
* **The hole on overflow is ink, and we do not draw it.** `overflow-B` above: we run a grid rule
  straight through a cell a label has spilled across; LibreOffice leaves it out. This is the
  actionable half.
* **The hole on a merge we already draw**, from `SheetMerges.IsOverlappedLeft` — `merge-CD` shows
  our hole in the right place. So the machinery for suppressing a rule exists; what is missing is
  the overflow record feeding it.

### 5.4 Reach, so the handover is sized rather than guessed

Page 1 of all 171, both banks (`probes/sheets-e-01/gridreach.py`):

| | documents |
|---|---:|
| printing a grid at all on page 1 (≥3 hairline verticals) | **13 of 171** |
| whose reference splits at least one rule | **10 of 13** |
| where the reference's rule *count* already equals ours | **11 of 13** |

**This is a 13-document item, not a track-wide one**, and that is the most useful thing this
section produces: it is worth doing after anything with a wider reach.

### 5.5 Not implemented, and why — plus a bigger defect found on the way

Implementing the overflow hole needs a per-(row, boundary) record produced where the text is laid
out (`SheetTextLayout`, which is the only place that knows a string spilled) and consumed by
`SheetPageDecoration.DrawGrid`. The paint order already suits it — `SpreadsheetPages.Draw` draws
cells and *then* the grid — but `SheetTextLayout.Draw` returns `void`, so the change is a new
channel through three files plus its own 171-document sweep. With subject 1 shipped and swept, the
brief's "a precise diagnosis of one beats a vague gesture at three" says to stop here.

**And the census found something larger than the item it was sizing.** `apron-area.xls` page 1:

| class | reference | ours |
|---|---:|---:|
| V 0.1 pt (the grid) | **70** | **0** |
| H 0.1 pt (the grid) | **56** | **0** |
| H 0.51 / 0.737 / 1.5023 pt | 10 / 34 / 9 | **0 / 0 / 0** |
| H 1.75 pt | 45 | 45 |

We draw **no grid at all** on that page and are missing three border width classes, while drawing
the 1.75 pt class exactly. The document nonetheless **matches the gate** — 3 pages and 417 words
on both sides — which is the cleanest demonstration in this report of what check 2 cannot see. Not
diagnosed; recorded as the largest single lead this round produced.

## 6. Two instrument findings, both of which cost time

**A batched instrument and a serial one are not the same instrument in practice.** The first
`gate.py` called `xray-office analyze` once per file per bank — 342 process launches. Twenty
minutes in it had reached the letter C. `analyze` takes many files in one invocation and emits a
row each; the batched form is the identical measurement and finishes. Recorded because the serial
version *looked* correct and would have been abandoned as "too slow to measure" rather than
rewritten.

**And it read the wrong column.** `analyze`'s TSV is
`file pages words wordsRaw wordsAlnum bullets symbols punct fonts unembedded subset …` — the
unembedded count is field **9** and field 10 is `subset`, which is nonzero on nearly every PDF
here. Taking 10 for 9 scored **all 171 documents as failing the unembedded check** and produced
"gate matches: before 0, after 0" — a number so obviously wrong that it was caught, which is the
only reason it is a footnote and not a published figure. The comparable trap in `sheets-c-01` was
`zipfile.is_zipfile`; this one is a column index, and it is worse in one way: it produced a *plausible
shape* (before and after equal) with a wrong magnitude.

## 7. The prediction, scored

| # | predicted | outcome |
|---|---|---|
| P0.1 | the brief's `###` direction is inverted; reference ≈1101, ours ≈2 | **right** — 1101 and 2, exactly |
| P0.2 | the fix is *adding* `###` | **right** |
| P1.1 | a non-`General` format hashes outright, no shortening | **right** — `0.00` gives `###`×9 then `1.00`, never `1.0` |
| P1.2 | `General` shortens, then goes scientific, and only then hashes | **right** — `123456789012` gives `###`, then `1E+11`, then the full number |
| P1.3 | a text cell never hashes | **right**, at all twenty widths |
| P1.4 | a formula in error hashes | **not tested** — the fixture has no formula; stated as untested rather than claimed |
| P1.5 | a value never borrows an empty neighbour; a string does | **right**, and it is the fixture's second sheet |
| P1.6 | shrink-to-fit suppresses `###` — *"weakest of the six"* | **right**, and not weak: `12345` at all twenty widths |
| P1.7 | wrap does not save a plain number | **right**; and the *date* half of the variant was **void — the fixture reused the non-wrapping style**, corrected and re-run |
| P1.8 | a merged cell hashes on the merged width | **not tested**; the fixture has no merge |
| P1.9 | the corpus gap is the **clip decision**, priors (i) 30% / (ii) 30% / (iii) 15% / (iv) 15% / (v) 10% | **refuted, all five** — the clip decision is *identical* on both sides at every width in the sweep. The gap is the hash rule after all: the missing third branch. My whole candidate list was built on "our `Hash` already implements the rule", which was 2/3 true |
| P2.1 | glyph counts agree within a few percent | **right, and far stronger** — `$` 8242 = 8242, and every `analyze` column identical |
| P2.2 | the reference emits `$` and `-` in one run, ours in two | **not shown as an operator claim**; measured instead as a 6.28 pt position difference, which is the same finding at the level the PDF actually exposes |
| P2.3 | verdict movement from subject 2: zero | **right**, and structural — the metric cannot see a `$` |
| P2.4 | if 2.1 fails, look at the repeat-fill count | **not reached** |
| P3.1 | we draw the grid; the 17 is one rule per column | **right** — 17 rules at the reference's 17 positions |
| P3.2 | the 107 is `bSingle`'s per-row branch | **right**, and all three of its triggers reproduce |
| P3.3 | `bHideGrid` from overflow is the cause on that page, 70% | **refuted** — the split rules have no holes, so the trigger there is the **hidden neighbour**, my 20% branch |
| P3.4 | the reference's grid has holes where text spills; that is the visible defect | **half right, and the half that matters is right** — on the briefed page there are none, but the authored fixture shows the hole is real and that **we do not draw it** |
| P3.5 | implementing holes alone moves the page's count to 14–17 | **not reached** |
| P4.1 | 8–35 renderings change, point estimate 18 | **refuted** — **2** |
| P4.2 | every changed document moves toward the reference, none overshoots | **right** — 2 closer, 0 further |
| P4.3 | `ODs-February` words Δ +895 → **−206**, inside the band; verdict string flips; scoreboard does not move | **right**, and the arithmetic to within 2 words (**−204**) |
| P4.4 | cross-track reach zero by a static argument | **right** — the whole `src/` diff is one file in `XRay.Office.Spreadsheets` |
| P4.5 | subject 1 fully, subject 2 to a diagnosis, subject 3 to a diagnosis | **right** |

**The refutation worth carrying forward is P1.9.** I predicted the corpus gap was upstream of the
hash rule because our `Hash` "already implements P1.1/P1.2", and listed five candidate causes with
priors. All five are wrong and the sixth possibility — that the rule itself was two-thirds
implemented — was not on the list at all. The lesson is narrow and repeatable: **a docstring that
correctly states two branches of a three-branch rule reads as a complete implementation**, and
`SheetGeneralWidth`'s does, at length, with citations. The way it was caught was measuring the
*boundary* on both sides rather than the *outcome*: the boundaries were identical, which killed
every clip-side candidate in one reading.

## 8. Tests

`tests/XRay.Office.Spreadsheets.Tests/SheetHashOverflowTests.cs`, **13 cases**, every expectation
quoted from 26.2.4.2's own render of an authored fixture
(`tests/corpus/features/sheet-hash.fods`, generated by `probes/sheets-e-01/mkhash.py`).

### Verified by reintroduction — all 13, none a drift guard

`verify-test.sh XRay.Office.Spreadsheets '<mutation>' SheetHashOverflowTests`:

| mutation | what it puts back | detected by |
|---|---|---:|
| `is { } fitted && fitted.Width <= available)` → `is { } fitted)` | the missing third branch — the round's defect, exactly | **7 of 13** |
| `breaks && !hashed` → `breaks` | wrapping the hash text again | **12 of 13** |
| `cell.Value is double value && …HasGeneralFormat` → `false && …` | **over**-application: every clipped value hashes | **2 of 13** |

The first proves the cases detect **under**-hashing; the third proves they detect
**over**-hashing, which one blunt mutation cannot separate — `ShrinkToFitNeverHashes` and the
`General` sweep rows are what fail there, and they are in the file for exactly that reason.

### Deliberate controls that must *not* fail

`ShrinkToFitNeverHashes`, the `string-XX` sweep row and `AValueHashesWhereAStringSpills`'s
middle two assertions pin the cases where LibreOffice does **not** hash. Without them the fix
reads as "a number that does not fit hashes", which is the trap `SheetGeneralWidth`'s own
docstring was written to prevent.

### Not tested, and named

* **A formula in error** (P1.4) and **a merged cell** (P1.8). The fixture has neither, so the
  first branch of `SetTextToWidthOrHash` is unexercised on this side. It was already implemented
  and is unchanged by this round, but it has no case here and that is stated rather than implied.
* **Nothing in `sheet-grid.fods` has a test.** The fixture is committed and its reference answers
  are in §5.2; the implementation it is for was not done, so a test would assert our current
  behaviour, which is the definition of a drift guard for a known-wrong output. Left untested
  deliberately.

## 9. Build and test counts

`dotnet build XRay.Office.slnx -v q -nologo`: **0 warnings, 0 errors**, and the exit status was
checked (`EXIT=0`) rather than the colour.

| project | briefed | now | Δ |
|---|---:|---:|---:|
| Core | 305 | 305 | |
| Containers | 109 | 109 | |
| Text | 289 | 289 | |
| Vector | 295 | 295 | |
| Rendering | 149 (1 skipped) | 149 (1 skipped) | |
| Markup | 259 | 259 | |
| OpenDocument | 125 | 125 | |
| WordProcessing | 789 | 789 | |
| Spreadsheets | 663 | **676** | **+13** |
| Presentations | 613 | 613 | |
| **total** | **3596** | **3609** | **+13** |

**0 failed**, projects run individually and totalled by hand. `XRay.Office.Fidelity.Tests` was not
run, as instructed.

## 10. What this round could not see

* **The gate cannot see subjects 2 or 3 at all.** `$` carries no letter or digit, and no glyph is
  emitted by `DrawGrid`. `apron-area` matching the gate with its whole grid missing is the
  demonstration.
* **`###` is the exception, and it is a *negative* signal.** Hashing a cell removes a real word
  and adds an invisible token, so the metric moves — but only on a document that was already
  wrong, and only in the direction of fewer words.
* **poppler's word-joining threshold is not observable from a PDF.** §4 measures the gap the two
  sides emit (2.26 pt against 9.05 pt); the threshold between them is inferred, and the fact that
  `pdftotext -bbox` joins neither says the threshold is mode-dependent as well.
* **`bHideGrid` has no PDF trace.** §5's attribution of a missing rule to overflow rather than to
  a merge rests entirely on the authored fixture separating them; on a corpus page the two are
  indistinguishable in the output.
* **The exact blank widths LibreOffice reserves for `?` and `_x`** are inferred from em ratios,
  not read out of the binary (§4.3).
* **Row heights are not asserted anywhere**, and the fixture exposed a defect in them that this
  round does not touch. The fixture's last three rows span, baseline to baseline, **217.38 →
  244.76 pt** on the reference and **196.11 → 324.11 pt** on ours — 27 pt against 128 — because
  our row-height pass wraps a `General` number in a wrapping cell where Calc disables automatic
  line breaks for a plain number format outright. It is **pre-existing and unchanged in either
  direction by this round**: the same 196.11 → 324.11 span is measured on the pre-fix binary, and
  the fix moves only *where inside that span* the hashed cell's single baseline sits. Named in §11.
* **The 27.2 tree is not 26.2.4.2.** Every `output.cxx` / `output2.cxx` citation is a statement
  about a binary that was not measured.

## 11. Leads this produced

1. **The wrapping plain-number row height.** Our row-height pass wraps a `General` number in a
   wrapping cell; Calc disables automatic line breaks for a plain number format entirely
   (i#111387). On the authored fixture the row comes out ~6× too tall. `SheetTextLayout.Breaks`
   already encodes the rule for *drawing*; the height pass does not consult it.
2. **`apron-area.xls` page 1** — no grid and three missing border classes, on a document that
   matches the gate (§5.5). The largest single lead here.
3. **The grid's per-row segmentation and the overflow hole** (§5), sized at 13 documents.
4. **`?` and `_x` as blanks of a stated width** (§4), a `XRay.Office.Core/Numbers` item owing a
   534-document sweep.
5. **The scientific mantissa is one digit short at three widths** (§2.1) — a
   `SheetGeneralWidth.Scientific` budget question, safe in the direction that matters and
   unresolved.
6. **`18-02RD301_ILS_components_Master_9-13-18.xls`: 626 cells that should hash and do not**
   (§3.2), and **~37 across the rest of the track that hash and should not** (§3.1). The first is
   a `.xls`, so the BIFF column-width and format path is where to look.

---

## Appendix — artefacts

Everything in `dotnet/probes/sheets-e-01/`:

| file | what it is |
|---|---|
| `prediction.md` | committed as `4933bb8f462` before any measurement |
| `mkhash.py` | authors `sheet-hash.fods` — fourteen variants × twenty widths |
| `mkgrid.py` | authors `sheet-grid.fods` — the three `bSingle` triggers, one per sheet |
| `cells.py` | reads a rendered PDF back as a (row, column) grid, keyed on the **right** edge |
| `table.py` | the two-sided variant table |
| `sweep.sh` | renders the whole track into one directory with the clock pinned |
| `gate.py` | reach, direction and verdict over three banks, batched |
| `gridreach.py` | how often `bSingle` fires, page 1 of all 171 |
| `gate.tsv` | 171 rows: pages, words and `###` on all three banks, both verdicts |
| `gridreach.tsv` | 171 rows: grid rules and segments, reference against ours |
| `unembedded.tsv` | the unembedded-font column for all 171, all zero |

Fixtures, committed under `tests/corpus/features/`: `sheet-hash.fods`, `sheet-grid.fods`. Both
are **authored**, not collected.
