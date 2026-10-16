# Round 154 — O78 seat B: a covered cell's rules are charged to the row's height

**Written incrementally.** Each section was written as its measurement finished.

## 0. What this round is

Round 152 named two seats for **O78** and round 153 shipped the first. This is the second:
`TableLayouter.OwnTopRule` answers wrongly because `DocxLayoutSource.Tables.Resolved` drops every
`w:vMerge` **continuation** cell before its stated top border can reach `PageTableRow.Cells`.

The seat's own scope note was *"scope it to `OwnTopRule` only — fixture 7 holds the upper row's
bottom at a 1 pt `single` in every arm, so it says nothing about whether a continuation cell's
borders should also feed `TopBand` or `BottomBand`. Widening the change to those two is unmeasured."*
**This round measured them, and the answer is that all three are charged** — plus a fourth, the band
at the boundary *below* a continuation cell.

### Environment

| | |
|---|---|
| tree | `/home/user/libreoffice-core`, base `791c5d874` *(round 153, O78 seat A)* |
| reference | `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2 `0229ac93fcf0d7cbc6376066c6f35021cef002dc`** |
| C++ read | this checkout, `configure.ac` **27.2.0.0.alpha0+** — *not* the reference binary's source |
| date | 2026-09-18 |

---

## 1. [bin] The seat reproduces at HEAD, and three more fixtures widen it

Four fixtures, each a set of one-attribute arms. Fixture 7 is round 152's, rebuilt and re-measured;
8, 9 and 10 are this round's and each isolates one place the charge could land by stating `nil`
everywhere else, so that **the covered cell's own statement is the only one at that boundary**.

| fixture | what it isolates | arms | differed before | after |
|---|---|--:|--:|--:|
| 7 `make-fixture7.py` (r152) | a `w:trHeight` floor — `OwnTopRule` | 18 | **2** (`VBN`, `VBA`) | **0** |
| 8 `make-band.py` | the band above the row — `TopBand`'s own term | 8 | **2** (`VB`, `VBH`) | **0** |
| 9 `make-bottom.py` | the band below it — `TopBand`'s *above* term | 4 | **1** (`WB`) | **0** |
| 10 `make-outer.py` | the band below the table's last row — `BottomBand` | 4 | **1** (`ZB`) | **0** |

**34 of 34 arms agree with 26.2.4.2 after the change**, against 28 before.

### And the top edge is charged and NOT drawn

Fixture 8's `VB` is the sharpest arm in the round. Every edge at that boundary states `nil` except
the covered cell's own 2 pt top. 26.2.4.2 pushes the lower row's baseline down by the whole 2 pt —
gap **11.20** against `VN`'s 9.20 — and **draws no rule there at all**. Its `PB` control, the same
table without the merge, is 11.20 *and* draws the rule.

Fixture 9's `WB` is the mirror and behaves the other way: the covered cell's stated **bottom** is
charged at the boundary below it **and is drawn**, because that edge is the merge's own outer one
rather than an interior one. Both renderers already drew it; only the charge was missing.

So this is the one place in the table engine where the height and the ink are deliberately
different sets of statements, and the fix touches only the height.

## 2. [src] And that is what `lcl_GetTopSpace` does

`lcl_GetTopSpace` (`sw/source/core/layout/tabfrm.cxx`:5175-5194) walks **every** lower of the row
frame and takes the maximum of each one's own `SvxBoxItem::CalcLineSpace(SvxBoxItemLine::TOP, true)`:

```cpp
for ( const SwCellFrame* pCurrLower = static_cast<const SwCellFrame*>(rRow.Lower()); pCurrLower;
      pCurrLower = static_cast<const SwCellFrame*>(pCurrLower->GetNext()) )
```

There is **no row-span test in it**, and that is the point: eight other places in the same file test
`getRowSpan() < 1` to skip exactly this cell (`:351`, `:510`, `:1437`, `:1747`, `:1935`, `:5510`,
and `findfrm.cxx`:1849, :1867), so a covered cell does have a frame and this function does see it.
`lcl_CalcMinRowHeight` adds it at `:5093`.

## 3. The change

`PageTableRow` gains `CoveredTopRule` and `CoveredBottomRule` — the widest rules stated by the cells
the row *drops*. `TableLayouter` seeds `TopBand`, `BottomBand` and `OwnTopRule` with them instead of
with zero, which is the whole of the layout change; the drawing is untouched, and the top edge is
therefore still drawn nowhere.

Three readers fill them, at the three places that already dropped the cell:
`DocxLayoutSource.Tables.Resolved`, `RtfDocumentReader`'s layout-row build and
`Ww8DocumentReader.LayoutTables`'s. The WW8 one resolves the covered cell's borders through the same
`ResolveBorders` as a kept cell, because the reference sees the box format after the importer's
defaults.

## 4. [bin] Reach beyond DOCX, measured by converting the fixtures with 26.2.4.2 itself

Each fixture converted to `.rtf`, `.odt` and `.doc` by the reference, then rendered by both sides.
Arms that differ, before and after:

| | fixture 7 | 8 | 9 | 10 | after |
|---|--:|--:|--:|--:|---|
| `.rtf` | 2 of 18 | — | — | — | **0 of 34** |
| `.doc` | 6 of 18 | — | — | — | **5 of 34** |
| `.odt` | 10 of 18 | — | — | — | **13 of 34 unchanged by this round** |

**RTF is the same defect and the same fix closes it.** WW8 closes `VBN` and `VBA`; its other four
arms are a *different* defect — the reference charges 1 pt on arms where the round-tripped file
states no top rule at all (`VNN`, `VNA`, `VAN`, `VAA`), which means the covered cell's borders in a
`.doc` are not what the `.docx` stated, and reading them is a WW8 question rather than a layout one.

**ODT has two defects of its own and this round implements neither.** Ten of fixture 7's eighteen
arms fail there, and five of the ten hold **no merge at all** — so the first is not this seat: a
converted `.odt` states `MinRowHeightInclBorder = true` in its `settings.xml` and
`OdtLayoutSource` never reads it, so `PageTable.MinHeightIncludesInsets` is false for every `.odt`
and no row height charges its insets or its own top rule. All 337 converted `.odt` state it. The
second is this seat's ODF twin: three arms of fixtures 8 and 9 fail on `.odt` for the same reason
they failed on `.docx`, and ODF spells the covered cell `table:covered-table-cell`. Both are in
`OPEN-ISSUES.md`.

## 5. [bin] O78's witness

`FAA 2025-26 Holdover Tables.docx`, this tree against the banked 26.2.4.2 rendering:

| | 26.2.4.2 | r152 (base) | r153 (seat A) | **r154 (both seats)** |
|---|--:|--:|--:|--:|
| last grid line on page 82 | 544.70 | 549.70 | 542.65 | **544.65** |
| displacement at the foot of page 82 | — | +5.00 | −2.05 | **−0.05** |
| Σ of the 33 row deltas | — | +5.05 | −2.00 | **0.00** |
| pages | 167 | 167 | 166 | **166** |
| page 83 | 1751 characters | blank | correct | correct |
| pages whose orientation differs | — | 10 | 2 | **2** |

**The 33 row deltas sum to exactly zero and the table's foot is 0.05 pt from the reference's**,
which is the two writers' own text-origin constant — the same 0.05 that separates every rule in this
round's fixtures. Round 152 simulated both seats together and predicted −0.05; the two shipped fixes
reproduce its simulation exactly.

## 6. [bin] Confinement and reach: 44 of 1620 renderings move

Our half of the whole corpus, of the 337 converted `.odt` and of the 337 converted `.rtf`, rendered
once at the round's base and once with the fix under `SOURCE_DATE_EPOCH=0`, one output directory per
document. `sweep.sh`, `diff-legs.py`, `confinement.txt`.

| family | ext | moved | of |
|---|---|--:|--:|
| words | docx | **17** | 271 |
| words | doc | **4** | 66 |
| rtf column | rtf | **23** | 337 |
| odt column | odt | **0** | 337 |
| slides + sheets | — | **0** | 609 |

**44 moved, 1576 byte-identical.** The `.odt` column is zero by construction — its reader is the one
of the four that does not fill the carrier — and that is the confinement statement: a change made in
three readers and one layouter reaches nothing else.

***A container restart killed the head leg at 1319 of 1620 and it was finished rather than
restarted.*** CLAUDE.md's rule — restart into a fresh directory — is about two *live* writers in one
directory; a run the container killed is not one. What its death can leave is a truncated file, so
`resume.py` opens every PDF the leg already held and deletes any that does not parse: **1318 good, 1
empty**, which was the render in flight when the machine went down. The remaining 302 were rendered
against the same binary, stated mtime unchanged.

## 7. [bin] Scoring, and the format the corpus says least about

Per span, pairing the reference's spans with each leg's by `(page, text)` in draw order and counting
only the spans that moved between the two legs (`score2.py`):

| | spans moved | closer to 26.2.4.2 | further | documents better / worse |
|---|--:|--:|--:|---|
| `.docx` | 13 156 | **12 884** | 270 | **15 / 1** |
| `.rtf` | 3 678 | **3 246** | 429 | 12 / 7 |
| `.doc` | 593 | **433** | 160 | 1 / 3 |
| all | 17 427 | **16 563** | 859 | 28 / 11 |

Summed mean |Δy| over the 44 movers goes **1105.45 → 1089.63 pt** — `.docx` −12.16, `.rtf` −3.68,
`.doc` +0.12 — and |Δx| is flat at +0.1 %.

**The witness improves on the measure as well as on its grid**: `FAA 2025-26 Holdover Tables.docx`
mean |Δy| **1.0460 → 0.7540** and `24-25_FAA_Holdover_Tables.docx` **2.0940 → 1.6631**, with no page
or alphanumeric count moving on either.

**Two documents' counts move and both are the same template family**; a third moves *towards* the
reference. `047_Visual_Product_Roadmap_Template` draws one alphanumeric more than before in both its
`.docx` and its `.rtf` — 182 and 224 against the reference's 178 and 218 — while its mean |Δy| goes
0.1256 → 0.0190, and `24-25_FAA_Holdover_Tables.rtf` goes **221 → 222 pages against the reference's
223** and 320 241 → 320 315 characters against 321 294, closer on both.

**`.doc` is the format this round can say least about, and its numbers are noise rather than a
verdict.** Its four movers' mean |Δy| changes by −0.077, +0.178, +0.013 and +0.005 pt on documents
already 7 to 45 pt from the reference. The fixture explains why: in a `.doc` round trip the covered
cell's borders are **not** what the `.docx` stated — four of fixture 7's arms have 26.2.4.2 charging a
rule that the file states nowhere — so the carrier is being fed borders that are themselves wrong.
The change is kept because the rule is the same rule and it closes two measured arms, and the WW8
border question is recorded as a seat of its own.

**The `.rtf` split is a threshold, not a direction.** Within one template family three improve —
`049` to **0.0000**, an exact match, and `044` by 3.92 — and two worsen by 3.24 and 2.22, which is
what a table whose height now crosses a boundary differently looks like.

## 8. The seats this round opened

Three, each measured here and none implemented, all in `OPEN-ISSUES.md`:

| | |
|---|---|
| **O103** | An `.odt` never charges a row's insets or its own top rule to a declared height, because `MinRowHeightInclBorder` is a *document setting* that `OdtLayoutSource` does not read — and **all 337 converted `.odt` state it**. Ten of fixture 7's eighteen arms fail on `.odt` and five of the ten hold no merge at all, so this is the larger half of that column's row-height error. |
| **O104** | The ODF twin of this seat: a `table:covered-table-cell`'s stated rules are not charged. 3 arms. |
| **O105** | A `.doc`'s covered cell carries borders the file does not state, so reading them is not enough: 4 arms still differ after the WW8 carrier, and on all four the reference charges a rule nobody states. |

## 9. The suite

| project | result |
|---|---|
| `XRay.Office.Core` | 591 passed |
| `XRay.Office.Text` | 750 passed |
| `XRay.Office.Vector` | 309 passed |
| `XRay.Office.Containers` | 109 passed |
| `XRay.Office.WordProcessing` | **2076** passed — 2072 at the round's base plus this round's 4 |
| `XRay.Office.Spreadsheets` | 1484 passed |
| `XRay.Office.Presentations` | 1205 passed |
| `XRay.Office.Markup` | 259 passed |
| `XRay.Office.Fidelity` | **10 failed of 552**, 0 skipped — the round's base figure exactly, and the known set: `TabStopComparisonTests` and `PageDrawingComparisonTests` in four formats each, `JustificationShrinkComparisonTests` and `SheetDrawingComparisonTests` |

Build: 0 warnings, 0 errors.
