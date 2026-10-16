# sheets-overflow-01 — overflow text painted on pages LibreOffice leaves blank

Round of 2026-08-14 on branch `wt-sheet-overflow`. Reference binary **LibreOffice 26.2.4.2
620(Build:2)**; banked references at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/sheets/`, reused
rather than re-rendered. `SOURCE_DATE_EPOCH=1700000000` for every rendering compared here.

The prediction written before any measurement is in `prediction.md` beside this. It is scored at
the end.

---

## 1. The RCO page-index caveat, settled first

The brief flagged that `RCO_VOR_Master_List_082824.xlsx` page 73 might be a page-*index*
difference rather than the same defect. It is not.

| | ours | reference |
|---|---:|---:|
| total pages | **80** | **80** |

Per-page words either side of the disputed range:

| page | 68 | 69 | 70 | 71 | 72 | **73** | **74** | **75** | **76** | **77** | 78 | 79 | 80 |
|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| ours | 172 | 157 | 157 | 154 | 12 | **323** | **260** | **1** | **34** | **4** | 59 | 174 | 155 |
| ref | 172 | 157 | 157 | 154 | 12 | **0** | **0** | **0** | **0** | **0** | 64 | 174 | 155 |

The page counts are equal and the pages on *both* sides of the gap agree to the word, so page 73
is page 73 in both documents. The five blank reference pages carry 622 of our 617-word total
surplus (15519 vs 14902). RCO is in the class.

## 2. LibreOffice's actual rule, and how it was measured

### What the source says

`ScOutputData::LayoutStrings` (`sc/source/ui/view/output2.cxx:1541-1543`):

```cpp
SCCOL nLoopStartX = mnX1;
if ( mnX1 > 0  && !bTaggedPDF )
    --nLoopStartX;          // start before mnX1 for rest of long text to the left
```

with `bTaggedPDF = pPDF && pPDF->GetIsExportTaggedPDF()` at `:1498`. At that one extra column the
cell is forced empty (`bEmpty = nX < mnX1`, `:1611`) and a scan runs *leftward from `mnX1`* over
empty cells until it finds `oFirstNonEmptyCellX` (`:1640-1656`) — **unbounded**, not one column.

The mirror-image lookahead at the block's right-hand end (`:1660-1678`) finds the first cell with
content *past* `mnX2`, so that a right-aligned run anchored beyond the paper spills back onto it.
**It carries no `bTaggedPDF` guard.**

`officecfg/registry/schema/org/openoffice/Office/Common.xcs:4318-4323` gives `UseTaggedPDF` a
default of **`true`**.

### What the binary does

The tree here is 27.2-alpha and the reference is 26.2.4.2, so the source is a lead and not an
answer. Two measurements settle it.

**(a) The natural experiment.** `essd-16-3433-2024-t02.xlsx` rendered twice through the *same*
26.2.4.2, changing nothing but one filter option
(`pdf:calc_pdf_Export:{"UseTaggedPDF":{"type":"boolean","value":"false"}}`):

| page | tagged (= the banked reference) | untagged | ours, before this round |
|---:|---:|---:|---:|
| 1 | 439 | 439 | 439 |
| 2 | **0** | 315 | 315 |
| 3 | **0** | 152 | 153 |
| 4 | **0** | 49 | 49 |

`pdfinfo` reports `Tagged: yes` on every banked reference. **Our port was faithful — to a branch
the reference never takes.** That is the whole defect: we reproduced LibreOffice's *screen and
untagged* behaviour, and the ground truth is tagged PDF export.

**(b) An authored probe**, `overflow-probe2.fods` — 21 columns of one inch on 8.5×11 paper with
0.7 in margins, so seven columns per page and three pages of columns. Row 1 a long plain string in
A, row 3 one containing U+00A0 (the `HasEditCharacters` path), row 5 a rich-text cell (two spans,
so `CELLTYPE_EDIT`), row 7 a plain string anchored in D, row 21 a **right-aligned** long string
anchored in the last column, and a marker in U so the used range is 21 wide. Tokens are unique per
row, so `pdftotext` per page says exactly which run landed where.

| page | tagged reference | untagged |
|---:|---|---|
| 1 | ZDD 9, ZNB 14, ZPL 15, ZRA 14, **ZRT 15** | same |
| 2 | **ZRT 16 and nothing else** | ZDD 15, ZNB 15, ZPL 16, ZRA 8, ZRB 7 |
| 3 | ZEDGE 1, **ZRT 14** | ZDD 15, ZNB 5, ZPL 16, ZRB 16, ZEDGE 1 |

### The rule

> **A run that overflows *rightwards* is painted on the page holding its anchor cell and on no
> other, however far its text reaches. A run that overflows *leftwards* is painted on every page
> its width covers.**

Two riders, both measured:

- **It is not a clipping rule.** On the anchor's own page the run is not clipped to the paper: the
  rightmost text on probe page 1 reaches `xMax = 617.63 pt` on a 612 pt page (printable right edge
  561.6 pt), *identically* in the tagged and untagged renderings. The brief's essd figures
  (617.0 ours / 617.7 reference) are the same observation.
- **It is not path-dependent.** Reading `DrawEdit`'s loop (`:4543`, `for (SCCOL nX=0; nX<=mnX2;
  nX++)`, ungated) predicts that rich-text and edit-character cells *would* still be redrawn. The
  binary says otherwise — ZNB and ZRA/ZRB are absent from tagged page 2 exactly as ZPL is. **This
  prediction of mine burned on the version gap, which is the sixth time in this project.** The
  measurement wins; the fix is unconditional.

## 3. What changed

`dotnet/src/XRay.Office.Spreadsheets/Layout/SpreadsheetPages.cs`

- **`DrawLeadIn` deleted.** It was a faithful port of the `!bTaggedPDF` branch — walk left from the
  band's first column to the nearest cell with text, draw it at its true position off the page, let
  the ordinary overflow rules carry its tail onto the paper. Nothing about it was wrong except that
  the reference never runs it.
- **`DrawTrailIn` added**, the mirror at `mnX2` that *is* ungated: walk right from the band's last
  column over free cells, bounded by `UsedRange.LastColumn` (Calc's `nLastContentCol`), stop at a
  merge, draw the cell found at its true position off the right of the page. It needs no alignment
  condition, exactly as Calc needs none — a left-aligned cell found this way spills away from the
  block, so its output area never reaches it and `bOutside` drops it.
- `ColumnBand.First` → `ColumnBand.Last`, since it is now the band's right-hand edge that decides
  whose spill reaches back into it. `First` had no other reader.

`dotnet/src/XRay.Office.Spreadsheets/Layout/SheetTextLayout.cs`

- `IsOutside`'s remarks corrected. They asserted that "Calc's string loop starts one column
  *before* the block so that a long string reaching in from the left is drawn", stated as the
  reason a page does not carry every neighbour. **That is the untagged behaviour and it was the
  brief's nominated seat.** `IsOutside` itself is correct and unchanged: a run anchored several
  columns left genuinely *does* overlap the next block, so `bOutside` answers "inside" for it in
  Calc too, and Calc still does not draw it. The seat was one level up, in which cells the page
  offers at all.

No change to `SheetTextOverflow.ExtendedLastColumn` or `SheetEmptyPages`. Which pages *exist* is
`IsPrintEmpty`'s question, answered from the print area's overflow extension, and is genuinely
independent of tagging — which is why a page bought by a spill that is never painted is still
printed, and printed blank. The brief was right that the print-area extension is not the bug.

## 4. Measured reach — the sheets track, 171 documents

Three sweeps against the banked references, same three checks and thresholds as `batch-check.sh`
(page count; letter-or-digit words in a 2%+3 band; unembedded fonts). Sweep A has the trail-in
disabled so the two halves are attributable.

| | match | pages | pages,words | words |
|---|---:|---:|---:|---:|
| baseline | **146** | 4 | 5 | 16 |
| A — lead-in removed | **154** | 6 | 3 | 8 |
| B — trail-in added too | **154** | 6 | 3 | 8 |

- **Page counts: 0 of 171 moved**, in either sweep. Pagination is upstream of painting, as the
  brief said.
- **Word counts: 31 of 171 moved.** 28 moved closer to the reference, 3 further. Total
  `Σ|ours − ref|` over the track falls from **36 545 to 27 151**, a 26% reduction.
- **Gate verdicts: 10 moved, all improvements, 0 regressions.** Net +8 `match` (two of the ten went
  `pages,words` → `pages`, so they fix their word count and still fail an unrelated page count).
- **8 documents became word-exact** that were not.
- **Fidelity: one test fixed, none broken** —
  `SheetSpilledTextComparisonTests.EveryPageShowsAsManyWordsAsLibreOfficeShows`.

Landmarks:

| document | before | after | reference |
|---|---:|---:|---:|
| `essd-16-3433-2024-t02.xlsx` | 945 | **431** | 431 |
| `RCO_VOR_Master_List_082824.xlsx` | 15516 | **14896** | 14901 |
| `Aircraft_Database.xlsx` | 16505 | **16356** | 16356 |
| `Infotabelle_WLAN im Flugzeug.xlsx` | 394 | **362** | 363 |
| `7-memento-2015-transports-aeriens-b.xls` | 28496 | **27537** | 27524 |
| `grants-2005.xls` | 34897 | **34032** | 34036 |

The trail-in's own contribution (A → B) is **3 documents**, all small: `Company_Seniority_Date_
Calculator` 2903→2909 (ref 2918) and `environment-edb-docs-edb-emissions-databank` 63950→63953
(ref 63955) both improve; `tk-syllabus-comparison-document-v5` 234736→234742 (ref 234666) drifts by
six words on a 234 000-word document. No verdict moves either way. It is kept because it is
measured behaviour of the reference and it is what makes the authored fixture reproduce word for
word, not because the gate asked for it.

### The three that moved further from the reference

| document | before | after | reference |
|---|---:|---:|---:|
| `EASA-IFP-145Scope(WEB)…` | 32300 | 32260 | 34602 |
| `fm-provider-service-measures.xlsx` | 21225 | 21219 | 21348 |
| `tk-syllabus-comparison-document-v5.xlsx` | 234736 | 234742 | 234666 |

The first two were already *under* the reference by 2302 and 123 words respectively — they have a
separate defect losing text, and removing 40 and 6 more words makes that arithmetically worse
without being related. None changes a verdict.

`CIS_Debian_Linux_8_Benchmark_v1.0.0.xls` deserves a note: 15386 → 7541 against a reference of
8981, from a large surplus to a smaller deficit. **Its figure is confounded and cannot be scored
here** — it paginates 109 pages against the reference's 88, so its column bands are not the
reference's bands and no per-band paint rule can be assessed against it. It failed `pages,words`
before and after.

## 5. Verification by looking, not only by counting

Per the standing instruction, three page pairs were composed at 150 dpi and handed to a subagent
that had never seen the documents and was forbidden to read the repository.

- `essd-16-3433-2024-t02.xlsx` **page 2** — "both halves completely empty… every pixel below the
  label bar is exactly 255 in both". Was 315 words of another column's prose, every line beginning
  mid-word.
- `RCO_VOR_Master_List_082824.xlsx` **page 73** — "identical, both fully blank, zero non-white
  pixels". Was 40 rows of red note text.
- `Infotabelle_WLAN im Flugzeug.xlsx` **page 2** — table geometry identical, 477 px wide on both,
  text lines agreeing to 1–4 px. **But see §6.**

## 6. Three things that contradict the brief

**(a) The nominated seat was wrong, and usefully so.** The brief put the defect at
`SheetTextLayout`'s `OutsideBlock`/`IsOutside` test, lines 238-262, on the strength of its own
remarks. Those remarks were the defect — they recorded the untagged rule as fact — but the test
they describe is correct and needed no change. The reasoning in the brief ("for a run anchored many
columns to the left, that test says *overlaps* on every subsequent strip") is exactly right about
what the test answers and wrong about the consequence: Calc's test answers the same way and Calc
still paints nothing, because the anchor column is never visited. Fixing `IsOutside` would have
required inventing a rule Calc does not have.

**(b) `fse_identification_form.xlsx` is not in this class.** Its word count did not move at all —
440 vs the reference's 427, before and after — and its divergence is confined to page 1 (pages 2
and 3 are 19/19 and 29/29, exact). Its rightmost text now sits at 974.6 pt against the reference's
973.5 pt on a 1008 pt page, so the extents agree. The brief's own description of it — "wrap points
are identical in both, so it is the paint clip that differs" — is right, and a paint clip is a
different defect from a paint *repeat*.

**(c) `Infotabelle_WLAN im Flugzeug.xlsx` passes the gate now but is still visibly wrong, and for
the reason the brief gave.** The blind reviewer, unprompted, found seven `kein WLAN` rows where our
text runs **44 px past a 477 px-wide table border** while the reference cuts it at the border
mid-glyph. That is the brief's fourth case, it is the same clip defect as (b), and it survives this
fix untouched — the word gate cannot see it because both renderers put the same characters in the
text layer and only one of them clips the ink.

**So the brief's four corroborating cases are two classes, not one.** essd and RCO are the
paint-repeat defect and are fixed. fse and Infotabelle are a paint-*clip* defect, still open, with
a reproducible case in each of `bHClip`'s two directions. That is the natural next round and it has
two documents and a reviewer's independent description waiting for it.

## 7. Scoring the prediction

| # | predicted | outcome |
|---|---|---|
| 1 | tagged export is the default, so left reach is **0 columns** | **correct** — `Tagged: yes` on every banked reference; the filter-option experiment is decisive |
| 2 | draw only on the page holding the anchor cell | **correct**, for the rightward direction |
| 3 | not clipped to the page on the anchor's own page | **correct** — 617.63 pt on a 612 pt page, identical tagged and untagged |
| 4 | 8–20 of 171 word counts move, 0 page counts | **half right** — 0 page counts, but **31** word counts moved, above my range |
| 5 | RCO 80 pages either way ⇒ same defect | **correct** — 80 = 80, neighbours word-exact |
| — | *(unwritten, and wrong)* `DrawEdit` is ungated so rich and edit-character cells still repeat | **refuted by the binary**; the source predicted it and the measurement did not |

Two lessons worth keeping. The first is the house rule doing its job for the sixth time: I read the
mechanism correctly out of a 27.2-alpha tree and got the *consequence* wrong twice — once on the
reach being one column when it is unbounded-but-gated, once on `DrawEdit`. The second is narrower
and new: **`bTaggedPDF` is a live switch on Calc's paint, and everything this project measures is
on one side of it.** Anything else in `output2.cxx` behind that flag is, by the same argument,
being ported to the wrong branch — `grep -n bTaggedPDF sc/source/ui/view/output2.cxx` lists eleven
sites and only two have been examined.

## Files

- `prediction.md` — written and committed before measuring.
- Probe workbooks: the two authored `.fods` are reproduced as corpus fixtures rather than kept
  here — `dotnet/tests/corpus/features/sheet-trail-in.fods` (new) and the corrected header of
  `sheet-lead-in.fods`.
