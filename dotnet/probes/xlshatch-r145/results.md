# Round 145 — the BIFF half of round 144's hatch mix (seat O102)

Reference binary: `/opt/libreoffice26.2/program/soffice` → **LibreOffice 26.2.4.2**,
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`. The C++ tree read here is
`/home/user/libreoffice-core` at **27.2.0.0.alpha0+**, which is *not* that binary's source, so
claims are marked **[src]** for a reading of this tree and **[bin]** for a measurement against
26.2.4.2's own output.

Witness: `sheets/done-010/xls/PC1000.xls`.

## 0. The seat, and the half of it that was wrong

O102 was seated at the end of round 144 and said two things. The first is right:

> an `.xls` cell's hatch is not mixed

The second is **wrong and is withdrawn**:

> …and a `CF` record's area is not swapped

`XlsConditionalFormats.ReadAreaBlock` has had the swap since it was written — its own remark
names `FillFromCF8`'s two corrections and implements both. What it did *not* have is the mix,
which is the same defect as the first half rather than a second one.

That seat was written from reading `XlsCellDecoration` and assuming its neighbour matched. It is
the O98 lesson again, one round later: **a seat written from reading and not from running can
name a defect that is already fixed.** Reading the second file took ninety seconds and would have
seated it correctly.

## 1. The rule

**[src]** Calc has no hatched cell background, in BIFF exactly as in SpreadsheetML.
`XclImpCellArea::FillToItemSet` (`sc/source/filter/excel/xistyle.cxx`:1094-1114) substitutes the
system window text colour for an unstated foreground and the window background for an unstated
background, then hands both to `XclTools::GetPatternColor` (`xltools.cxx`:347-358), which indexes
a **nineteen-entry ratio table** by the pattern number and calls `ScfTools::GetMixedColor`
(`ftools.cxx`:118-130) — `(background − pattern) × ratio / 0x80 + pattern` per component, in
integer arithmetic.

**Two things make this not a transcription of round 144's table.**

- **The ratio runs the other way.** Here `0x00` is the full pattern colour and `0x80` the full
  background, so `solid` is index 1 with ratio `0x00`; in SpreadsheetML `alpha` is how much of the
  pattern colour shows and `solid` is `0x80`.
- **The two tables are not each other's complement.** BIFF's 12.5 % grey is `0x70`;
  SpreadsheetML's `gray125` is `0x10`. Those are the same statement — 12.5 % of the pattern colour
  — but a reader that derived one table from the other by subtraction would get every *other*
  entry wrong, because the two formats number their patterns differently as well.

**[bin]** `PC1000.xls` states pattern `0x11` on 82 cells, black over white. 26.2.4.2's own
`--convert-to fods` gives their cell style `fo:background-color="#dfdfdf"`, used 47 times, and
its PDF paints `#DFDFDF`. `(255 − 0) × 0x70 / 0x80` is 223, which is `0xDF` exactly. Before this
round we painted the background whole — white — and drew a white panel where the reference draws
a grey one; after it we draw `#DFDFDF` on 13 pages of 13, the same page count as the reference.

## 2. Reach, censused from the record stream

A zip walk cannot see any of this: a BIFF fill pattern is a six-bit field inside an `XF` record.
`census-xf.py` walks the `Workbook` stream and answers the question that matters — not how many
`XF` records state a hatch, but **how many a cell can reach**, which needs the `XF` index each
cell record states.

```
BIFF workbooks scanned: 63          (+1 mislabelled: Special-Procedures_2025-07-10.xls is a zip)
XF records stating a hatch:  2 in 2 documents   {14: 1, 17: 1}
cells reaching one:         82 in 1 document    {17: 82}
```

**One document.** The `XF` stating pattern 14 in the other is never named by a cell.

`census-cf.py` does the same for the conditional-format path, walking past whichever of the
number-format, font, alignment and border blocks each `CF` record's flags declare:

```
CF records: 187,  of which 160 state an area block
  pattern 0 (none):  157 records in 5 documents
  pattern 1 (solid):   3 records in 1 document
```

**Nil reach.** No BIFF conditional format in the corpus states a hatch at all. The mix is applied
there anyway, because it is the same helper and the arm is now correct rather than approximately
correct, but it is measured as painting nothing today and the write-up says so rather than
counting it towards the round.

## 3. What changed

`XlsPatternFill` is the BIFF twin of round 144's `XlsxPatternFill` — the ratio table and the
mix — and both BIFF readers go through it:

- `XlsDecorationTable.FormatOf` mixes instead of taking the background for anything but `solid`.
  An `XF` marks all three of its area parts used together (`XclImpCellArea::SetUsedFlags`,
  `xistyle.cxx`:1036-1039), so both colours are always stated there and the window-colour
  fallbacks are only reached for an index the palette does not hold.
- `XlsConditionalFormats.ReadAreaBlock` keeps its swap and mixes the hatch arm, substituting the
  window text and window background colours for an unstated half — which in a `CF` record really
  can happen, and is the reference's own substitution rather than a guard against a bad index.

`solid` is the mix at ratio `0x00`, so it is not a special case beside the general one; that is
what makes this a rule rather than a branch.

## 4. Reach, measured — and the census and the sweep agree exactly

The sheets track, 307 documents from `MANIFEST.tsv`, rendered twice under `SOURCE_DATE_EPOCH=0`
with nothing but the binary changing (`probes/cfmix-r144/par-sweep.sh`, whose base leg is round
144's own `after` fingerprints — the same source as this round's `HEAD`):

```
documents scored: 307      failed on either leg: 0
byte-identical:   306      moved: 1
```

**The one document that moves is `PC1000.xls`, which is the one the census named.** That is as
tight as a confinement result gets and it is worth stating as the round's method note: the census
predicted one document from the record stream before anything was built, and the sweep found
exactly that one.

Scored against 26.2.4.2, all three legs rendered the same day:

| | base | after |
| --- | ---: | ---: |
| pages | 13 of 13 | 13 of 13 |
| summed unsigned ink over 13 pages | 1.59 | **1.57** |
| page 10, where the hatch is | 0.27 | **0.25** |
| MAJOR pages | 0 | 0 |

**The page-level ink measure barely registers it, and the fill operators state it plainly.**
Counting filled paths by colour on page 10 (`fills.txt`):

| | `#DFDFDF` | `#FFFFFF` |
| --- | ---: | ---: |
| ours, base | 0 | 21 |
| ours, after | **18** | **3** |
| 26.2.4.2 | **1** | **3** |

The white count now agrees exactly, and the grey is present on both sides — ours as 18 separate
rectangles against the reference's one, which is the rectangle coalescing `dotnet/CLAUDE.md`
already records and not a difference in what is painted. A light grey over white is a small
pixel difference spread over 82 cells, which is why `|ink|%` moves by two hundredths while the
thing being measured goes from absent to exact. **Count the operators when the question is
*which colour*; the raster measure answers *how much area*.**

No gate column can see any of it: a fill adds no alphanumeric character and no page.

## 5. Tests, and what pins them

`XlsPatternFillTests`, 13 assertions: the ratio of each named pattern, the witness' grey against
26.2.4.2's own value, that `solid` is the mix at nought rather than a branch beside it, and that
the truncation is towards zero in **both** directions — `0 → 255` at ratio `0x70` gives 223 and
`255 → 0` gives 32, which a symmetric rounding would not.

Pinned by two mutations (`mutations.txt`):

| mutation | caught by |
| --- | ---: |
| the hatch is not mixed, the background is taken whole | **11** tests |
| SpreadsheetML's ratio table is transcribed in place of BIFF's own | **11**, including the existing `SheetDecorationTests` fixture `sheet-decor-xls.xls` |

The second is the one worth having: it is the mistake an author who had just written
`XlsxPatternFill` is most likely to make, and the existing BIFF decoration fixture catches it
independently of anything this round wrote.

## 6. Suite state

Every project run separately: Containers 109, Core 591, Markup 259, OpenDocument 194,
Presentations 1205, Rendering 164, **Spreadsheets 1479**, Text 750, Vector 309,
WordProcessing 2013 — all green. Fidelity 542 passed, **10 failed of 552, 0 skipped**, the same
ten as round 144 and by name: four `TabStopComparisonTests`, four `PageDrawingComparisonTests` on
`paginated.*`, one `JustificationShrinkComparisonTests`, and `SheetDrawingComparisonTests` on
`sheet-rich-text.xlsx`. None is BIFF; `sheet-rich-text.xlsx` is an `.xlsx` and cannot reach
`MsBinary` at all.
