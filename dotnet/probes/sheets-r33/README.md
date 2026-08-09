# Sheets round thirty-three — probe data

Base commit `9f44b2943`, checked with `git log --oneline -1` in the worktree before anything was
measured. Reference renderer LibreOffice 24.2.7.2. Both whole-track sweeps ran against a
checksummed copy of the CLI, and the copy's `Paperless.Spreadsheets.dll` hash was diffed against
the tree's before each run started; each copy was additionally proved to hold the change by
rendering the one document whose page count it moves.

| file | what it is |
|---|---|
| `base-whole-track.tsv` | whole-track sweep at `9f44b2943`, before any change |
| `final-whole-track.tsv` | the same on the final tree |
| `reach.log` | the byte-level reach of the band-height half, clock pinned, over all 171 documents — the summary rather than the per-row TSV, which was cleaned up with the sweep's PDFs before it was copied |
| `reach2.log` | the same for the round's two changes together |
| `splice-rerun.tsv` | the two documents the final sweep returned `ref-failed` for, re-run alone |
| `interim-band-only.tsv` | the whole-track sweep with the band height fixed and the drawing half not yet — kept because it is what found the second half |
| `reach.sh` | renders the track with two CLIs and diffs the bytes; unchanged from r31 |
| `score.py` | turns a `rows.tsv` into matches, page error, exact counts, word error |
| `mutate.sh` | the reintroduced defects the round's tests were verified against |
| `rowbands.py` | replays `ScTable::UpdatePageBreaks` down the rows for a stated page height |
| `colwidth.py` | our SpreadsheetML column-width arithmetic against the flat-ODF answer |
| `biff-band-census.py` | which `.xls` sheets have a band Calc pins rather than makes dynamic |
| `GridProbe/` | dumps the geometry a sheet is paginated from, and the bands it produced |
| `sheet-pinned-band.fods` | the source the round's `.xls` fixture was converted from |

## The baseline reproduces the brief exactly

```
149/171 matches   abs page error 86   exact page counts 155   abs word error 32729
001 10/10  002 10/10  003 10/10  004 10/10  005 10/10  006 10/10  007 10/10  008 10/10
009 9/9    010 8/10   011 7/10   012 8/10   013 8/10   014 9/10  015 6/9   016 5/9
017 6/10   018 3/4
```

Every figure in the brief, to the digit, including the per-batch line. 171 rows, no duplicate
path, no `ref-failed` and no `ours-failed`.

## The lead as handed over, and what it actually was

The brief's headline was that five of batches 011 and 016's six failures are **one quantity with
both signs — how many columns fit in a column band** — and that a term erring in both directions
is a rounding or a threshold. The measurement was right, the reading was not, and the two had to
be checked separately.

**It is not the columns.** `colwidth.py` replays our SpreadsheetML column-width arithmetic
against LibreOffice's own answer, read out of the `style:column-width` its flat-ODF export
writes. On `SIL_TDB648.xlsx` — one of the two documents said to take a column too few — our
widths reproduce **all 16384 columns on all eleven sheets exactly**, and the margins agree to
the twip. The digit width the whole conversion hangs on comes out at 127 twips on both sides
(DejaVu Sans at 10 pt, which is what fontconfig makes of the workbook's stated MS Sans Serif).

**On `RMP 2011-2014 and Inventory.xls` it is not even the columns' axis.** That sheet is
`fitToWidth=1`, so it has exactly one column band whatever the widths are. Aligning the two
renderings page by page shows they agree through page 11 and drift from page 12, and both
renderings end with a *notes* page per sheet, which the page-order arithmetic had hidden: we
print 22 row bands where LibreOffice prints 21.

So the quantity is the **printable page height**, and the term that is wrong in it is a
threshold — just not the one named.

## A pinned BIFF header or footer band has no minimum height

`XclImpPageSettings::Finalize` splits on whether the band's text fits the distance the two
margins leave, `fHeaderDist < 0.0` (`sc/source/filter/excel/xipage.cxx:315-331`):

- **It fits.** The band is marked *dynamic* and only the distance to the body is written; no
  `ATTR_PAGE_SIZE` is put on the item set. `nManHeight` therefore stays at whatever a fresh page
  style has — 0.5 cm of text plus a 0.25 cm gap, 425 twips (`stlsheet.cxx:184`) — and
  `UpdateHFHeight` floors the band there.
- **It does not fit** (#i23296, the band would overlay the sheet). The band is marked *not*
  dynamic and `ATTR_PAGE_SIZE` is written explicitly at the margin distance. `UpdateHFHeight`
  then returns on its own first line — `if (!(rParam.bEnable && rParam.bDynamic)) return;`,
  `sc/source/ui/view/printfun.cxx:793` — so it never reaches the `nManHeight` comparison and
  **no floor applies at all**.

We applied the floor in both cases. The change is to withhold it in the second, and
`SheetBandHeight.Printed` now reports which branch the filter took, since it is already the
thing that makes the decision.

### Measured against LibreOffice's own four numbers, not derived

`RMP 2011-2014 and Inventory.xls` exercises both branches on both bands, and its flat-ODF export
states every one of the four heights. `fo:min-height` is `nManHeight` on a dynamic band and
`svg:height` is `nHeight` on a pinned one:

| sheet | band | LibreOffice | ours before | ours after |
|---|---|---|---|---|
| `RMP 2011-2014` | header | dynamic, min-height 0.2953 in = **425** | 425 | **425** |
| `RMP 2011-2014` | footer | pinned, svg:height 0.122 in = **176** | 425 | **176** |
| `Inventory` | header | pinned, svg:height 0.2638 in = **380** | 425 | **380** |
| `Inventory` | footer | pinned, svg:height 0.0783 in = **113** | 425 | **113** |

The exported distances corroborate the branch as well as the height: the dynamic header carries
`fo:margin-bottom="0.0472in"`, which is exactly `topMargin − headerMargin − 12 pt` for a header
whose one line states `&12`, and both pinned bands carry a zero distance, which is what
`lclPutMarginItem(…, EXC_ID_BOTTOMMARGIN, 0.0)` writes.

### The row bands were reproduced in a script before any C# was written

`rowbands.py` replays `ScTable::UpdatePageBreaks` down the rows for a stated page height, over
**LibreOffice's own row heights** taken from the same export — which agree with ours to the twip
on all 210 rows, so the row heights are not in question either.

The page height is `18898 − header − footer` twips at this sheet's 60% scale. Scanning it:

| page height | bands | first divergence from ours |
|---|---|---|
| 18000–18074 | 22 | — (this is ours: floored, 18048) |
| 18075–18179 | 21 | band 19 |
| **18180–18344** | **21** | **band 12, ends at row 120 rather than 119** |
| 18345–18494 | 21 | band 11 |

Our floored answer is 18048 and the pinned one is `18898 − 425 − 176 = 18297`, in the third row.
That row is the reference's: aligning the two renderings by token overlap puts ref page 12 at 96%
of ours and ref page 19 against **our page 20**, which the 18075 split (identical to ours through
page 18) cannot produce and the 18180 split does. The workbook is 39 pages before and **38** after,
which is the reference's count.

### Reach, predicted before the sweep and matched exactly

`biff-band-census.py` counts sheets with a footer or header the filter pins (`nominal > stated`)
whose stated band is under 425 twips — the population the floor was wrong for. **Over all 171
documents rather than a subset**: 61 open as an OLE2 workbook stream and 110 as a zip, 61 + 110
is 171, and only the BIFF reader carries the floor, so the census covers the whole track and the
zip half is empty by construction rather than unknown.

The census names **five** documents. Rendering all 171 with both CLIs under
`SOURCE_DATE_EPOCH=1700000000`: **five differ, 166 are byte-identical**, and they are the same
five by name.

Re-measured on the round's **two** changes together (`reach2.tsv`), it is **seven differ, 164
identical**: the drawing half also moves `programs contact list as of 07-01-10.xls` and
`cy06_primary_np_comm.xls`, whose pinned bands lose their 142-twip gap and therefore move their
footer text without moving a page. Both were matching before and match after. **Every one of the
seven is a `.xls`** — the relaxed drawing guard reaches no XLSX and no ODS on this corpus, which
is the ceiling the census gave.

| document | batch |
|---|---|
| `2012-GA-Survey-Chapter-5-Tables-16Dec2013-V2.xls` | 002 |
| `2012-GA-Survey-Chapter-6-Tables-16Dec2013-V2.xls` | 002 |
| `CSA_CCM_v1.2.xls` | 007 |
| `PC1000.xls` | 010 |
| `RMP 2011-2014 and Inventory.xls` | 011 |

Three of the five state a footer margin *equal* to the page margin, so their pinned band is
nothing at all where we gave it 425 twips.

## The half of the rule the first sweep found

Withholding the floor and stopping there **cost two matches and gained one**: the whole-track
sweep came back 148/171, page error 85, 156 exact page counts, word error 32726
(`interim-band-only.tsv`). Exactly the five documents moved and no others, which is the census
again, but two of them lost their word gate.

The three workbooks whose pinned band is *nothing* stopped drawing their footer. Calc does not:
`PrintHF` clips the text to `tools::Rectangle(aStart, aPaperSize)`, and **a VCL rectangle built
from a zero-height `Size` has no bottom edge at all** — it is unbounded rather than empty
(`printfun.cxx:1870`). So a zero band suppresses the *space* and not the ink. Measured on
`2012-GA-Survey-Chapter-6-Tables-16Dec2013-V2.xls`, whose printed sheet has a 0.5 in bottom
margin and a 0.5 in footer margin: LibreOffice draws `Page 6 - 2` with its top at **575.95 pt**
on a 612 pt page — the bottom margin line to a twentieth of a point — running down into the
margin, and the words it drops are exactly the four pages of `Page 6 - N`.

The gap goes with it, and for the same citation: the branch that pins a band writes the distance
to the body out as nothing (`lclPutMarginItem(…, EXC_ID_BOTTOMMARGIN, 0.0)`, `xipage.cxx:322`),
because the band was already too short for its own text. Without that a pinned 176-twip band
would put its one line in the 34 twips left after the shared 142-twip default. **A dynamic
band's distance is `statedBand − nominal` and is deliberately left at the default**: our drawing
places a dynamic footer against the sheet and a dynamic header against the top margin, so the gap
cancels out of the footer entirely and only shifts a header's centring inside its own band.
Nothing on this corpus measures that. It is a real, small, unmeasured deviation and is labelled
as one in the source.

| document | baseline | band height only | with the drawing half |
|---|---|---|---|
| `2012-GA-Survey-Chapter-5…` | 537/537 `match` | 529/537 `match` | **537/537** |
| `2012-GA-Survey-Chapter-6…` | 643/643 `match` | 627/643 `words` | **643/643** |
| `CSA_CCM_v1.2.xls` | 15958/15852 `match` | unchanged | unchanged |
| `PC1000.xls` | 864/873 `match` | 855/873 `words` | **864/873** |
| `RMP 2011-2014 and Inventory.xls` | 39/38 `pages` | 38/38 `match` | **38/38 `match`** |

## Tests

`tests/corpus/features/sheet-pinned-band-xls.xls`, authored as flat ODF and converted by
LibreOffice so that its BIFF page records say what they are meant to — verified by reading the
records back: bottom margin 0.35 in against a footer margin of 0.25, so the stated band is
**144 twips**, and one centred footer line in the workbook's own **10 pt Liberation Sans**, whose
200-twip nominal does not fit it. The face is stated explicitly because a fixture naming Arial
resolves to the unwired default and would pass whatever the code did.

Its 34 rows total 14450 twips against a body of **14616** pinned and **14335** floored, so the
sheet is one page if the floor is withheld and two if it is applied. LibreOffice renders it on
one; we rendered it on two before and one after.

`tests/corpus/features/sheet-zero-band-xls.xls` is the same document with the footer band taken
to **nothing** — a 0.25 in bottom margin against a 0.25 in footer margin. LibreOffice renders it
on one page with the top of `Zero band footer` at **773.95 pt** on a 792 pt page, which is its
bottom margin line; before this round we rendered two pages and drew the footer 12.6 pt higher,
and after it we draw it at 774.00.

Five cases in `SheetPinnedBandTests`, verified by reintroducing each defect through
`.claude/skills/corpus-batches/scripts/verify-test.sh`:

| mutation | cases that fail |
|---|---|
| `m1` the floor applied to every band, dynamic or not — the defect this round removed | 2 |
| `m2` a band the margins leave no room for treated as dynamic | 1 |
| `m3` a band whose text overflows it still reported dynamic | 3 |
| `m4` a band of no height suppressed rather than drawn | 1 |
| `m5` a pinned band keeping the shared 142-twip gap | 1 |

A fourth mutation is worth recording as a **non-detection rather than a gap**: writing `m3` as
`isDynamic = nominal > statedBand + Length.FromPoints(1000)` changes nothing, because inside that
branch the expression is false either way. `verify-test.sh` reporting it undetected is the right
answer, and it is an equivalent formulation rather than a defect.

## `EASA-IFP-145Scope(WEB)_…xlsx`: the 2350 words are the `/Widths` rounding

The brief's largest measured residue was this workbook's 2350 lost words, attributed to
"one column's width or alignment". **It is neither, and it is not in the spreadsheet code at all.**

The token multiset reproduces exactly: under-draw 4747, over-draw 2397, difference 2350
occurrences of `EASA.UK.1` joined to the next cell's first token. The content streams for those
rows are otherwise identical — same glyph codes, same `Td` positions, same subset of Carlito at
10.998 pt — and `pdf-ops.py diff` reports **0 records one-sided and 0 drawn differently** on a
sample page.

What differs is one array. **LibreOffice truncates its `/Widths` to whole thousandths of an em
and we state them to four decimals**, and our writer holds the pen to layout, so:

| | `EASA.UK.1` advance | its right edge | gap to the next cell |
|---|---|---|---|
| reference | 4273/1000 em = 46.9945 pt | 98.3865 | **1.1375 pt** |
| ours | 4277.8322/1000 em = 47.0493 pt | 98.4414 | **1.0830 pt** |

Poppler's word break sits between them: `minWordBreakSpace` defaults to 0.1 of the font size,
which is 1.0998 pt here. 2350 rows, one token each.

Truncation is LibreOffice's rule and not a coincidence — **it reproduces all 77 of this
document's width entries where rounding misses 39**, and it reproduces Liberation Sans and
Liberation Sans Bold on another of the track's references at every entry where the two rules
disagree (666 against 667, 943 against 944, 610 against 611, 277 against 278).

**It was not shipped, and the reason is worth stating rather than the deviation.** Truncating our
`/Widths` alone would not reproduce the reference's spacing: `PdfContentSink` corrects the pen
whenever it drifts more than 0.0025 pt from where layout put it, and a truncated width drifts
about 0.0066 pt per glyph at this size, so every glyph would take a `TJ` adjustment and land back
at the accurate position — a much larger content stream and the same word count. Reproducing the
reference here means adopting *both* halves of its arithmetic: the truncation **and** the
uncorrected drift it causes. That is a deliberate degradation of our text layer, it is in the
shared PDF writer and therefore reaches all three tracks, and it is a decision for a round that
can sweep all three. The mechanism is now named and measured; what is left is the judgement.

## What is still open on batches 011 and 016

With `RMP` closed, the brief's cluster is four documents and its stated unifying cause is
refuted for at least two of them. What has *not* been re-triaged this round:
`Capability_List_…unsorted.xlsx`, `SIL_TDB648.xlsx`, `tk-syllabus-comparison-document-v5.xlsx`,
`flightstandards-doc-Cross-reference-table_version02.xlsx`,
`7-memento-2015-transports-aeriens-b.xls` and `T0A0D0000090006XLSE.xls`. The only thing this
round establishes about them is negative and worth keeping: **on `SIL_TDB648` the column widths
and the page margins are exactly LibreOffice's**, so whatever costs it its extra page is
downstream of both. `GridProbe` and `rowbands.py` are the instruments for the next attempt, and
on `RMP` they took the question from "a different column band" to a named constant in about an
hour.

## The whole-track result

| | matches | abs page error | exact page counts | abs word error |
|---|---|---|---|---|
| `base-whole-track.tsv` | 149/171 | 86 | 155 | 32729 |
| `interim-band-only.tsv` | 148/171 | 85 | 156 | 32726 |
| `final-whole-track.tsv` | **150/171** | **85** | **156** | **32673** |

Per batch at the end: 001–009 89/89, 010 8/10, **011 8/10**, 012 8/10, 013 8/10, 014 9/10,
015 6/9, 016 5/9, 017 6/10, 018 3/4. **No batch fell.**

**Exactly one row of 171 changes**, and it is the one the round was aimed at.

| document | before | after |
|---|---|---|
| `RMP 2011-2014 and Inventory.xls` | 39/38 pages, 18634/18548 `pages` | **38/38, 18578/18548 `match`** |

The other four documents the change reaches come out with the word counts they had at the
baseline, which is the point: the band height moved on all five and only one of them was
paginating wrongly because of it.

**Two rows of the final sweep came back `ref-failed` and are spliced rather than believed.**
`fy2011-aip-grants.xls` and `6880ac7361ca1b99a9230811_ST Capability List Rev.16 - Web.xlsx` both
convert cleanly when re-run alone under one worker — 93/93 and 217/217, both `match`
(`splice-rerun.tsv`) — which is the `soffice`-wedges-past-its-own-timeout shape the skill
describes, on a machine carrying a load average around 17. Believing the raw sweep would have
reported 148/171 and two batches falling, from an `soffice` that had nothing to do with this
round's change.

## Test counts

Every project run individually, whole output captured, **0 skipped** everywhere.

| Core | Containers | Text | Vector | Rendering | Markup | OpenDocument | WordProcessing | Spreadsheets | Presentations |
|---|---|---|---|---|---|---|---|---|---|
| 275 | 109 | 240 | 291 | 119 | 259 | 125 | 696 | **598** | 542 |

Every count is the briefed known-good except Spreadsheets, which is 593 plus this round's 5.

## What this round did not touch

Every source change is inside `Paperless.Spreadsheets` — `MsBinary/XlsPrintSetup.cs`,
`Layout/SheetBandHeight.cs`, `Layout/SheetPageDecoration.cs` — so the words and slides tracks
cannot be reached by it. The one lead that *would* reach them, the `/Widths` rounding above, is
diagnosed and deliberately unshipped for exactly that reason.

## The twelve-document pagination cluster is not a column-fit predicate

A mid-round review widened the brief's three documents to twelve (plus two just outside a 1%
word band): every document whose page count is wrong while its words are right, deltas
−3 −3 −2 −1 −1 −1 −1 +1 +1 +1 +1 +3. The question asked was for the observation that separates
*one predicate* from *twelve coincidences*. It is cheap, and it is negative.

**Every input to the column-band predicate agrees with LibreOffice, on every document checked.**
`ScTable::UpdatePageBreaks` measures column widths against the printable page width and nothing
else, and our port is faithful, so the split can only differ if one of those numbers does.
Against LibreOffice's own flat-ODF export (`geom-check.py`, `page-check.py`):

| document | sheets | column widths differing | page rectangle differing |
|---|---:|---:|---:|
| `SIL_TDB648.xlsx` | 11 | **0** of 16384 per sheet | **0** |
| `CSJU List of Recipients…xlsx` | 8 | **0** | **0** |
| `tk-syllabus-comparison-document-v5.xlsx` | 17 | **0** | **0** |
| `flightstandards-doc-Cross-reference-table_version02.xlsx` | 6 | **0** | **0** |
| `Capability_List_…unsorted.xlsx` | 1 | **0** | **0** |
| `FAA-2019-0995-0002_attachment_2.xlsx` | 11 | **0** | **0** |
| `seihon_zassi_kikou_20221215.xlsx` | 1 | **0** | **0** |

The page rectangle here is the paper size, all four margins, the scale and the pinned band
heights — every term of `GetDocPageSize`.

**And two of the fourteen have one column band per sheet**, so a column-fit defect cannot reach
them at any value: `RMP 2011-2014 and Inventory.xls` (`fitToWidth=1`, and its cause turned out to
be the page *height*) and `seihon_zassi_kikou_20221215.xlsx`. `axis-triage.sh` prints the band
counts for the whole list.

### What the cluster does look like: a wrapped row one text line out, in both directions

The row heights are the last input, and they differ — always by a whole multiple of one line of
the cell's own text, and in both directions (`row-check.py`, ours against the export):

| document | rows compared | differing | examples (ours vs LibreOffice, twips) |
|---|---:|---:|---|
| `tk-syllabus…v5.xlsx` | 300/sheet | 16 over 17 sheets | 477 vs 701, 701 vs 925, 925 vs 1149, 3611 vs 3388, 1597 vs 1373 |
| `flightstandards…version02.xlsx` | 400 | 12 | 925 vs 1820, 925 vs 1149, 1373 vs 1597, 2716 vs 2940 |
| `seihon_zassi_kikou_20221215.xlsx` | 5159 | **129** | 1104 vs 1373, 1373 vs 1641 |
| `Capability_List_…unsorted.xlsx` | 400 | 1 | 390 vs 567 |

224.1 twips is one 11 pt line on these documents and 268.8 is one 13 pt line; every difference
above is a whole number of them. That is a wrapped cell taking one line more or fewer than
LibreOffice's, which is exactly the shape the brief described — *one quantity with both signs* —
attached to the wrong axis.

**Two of the seven do not fit even that, and saying so is the point.** `CSJU List of Recipients
of funds 2013-2020.xlsx` has **word counts exactly equal** at 55219/55219, +1 page, and **not one
differing column width, row height or page term on any of its eight sheets, over 5500 rows**.
`FAA-2019-0995-0002_attachment_2.xlsx` is the same on every sheet it has. Whatever splits those
two differently is downstream of the geometry entirely — a repeated band, a print area, an empty
page — and it is not the same finding as the row heights. A cluster of one symptom is not a
cluster of one cause, and this one is at least three.

### What to do with it

The instruments are committed and the next round can start from a measurement rather than a
hypothesis: `axis-triage.sh` says which axis can possibly be at fault, `geom-check.py` and
`page-check.py` rule out the column axis in one run each, and `row-check.py` names the rows.
`SheetOptimalRowHeights` is where the row-height work lands, and the population is large —
129 rows on one sheet of one document — so it is worth a prototype against the export's own
answer before any C#, in the shape that closed this round's own lead.
