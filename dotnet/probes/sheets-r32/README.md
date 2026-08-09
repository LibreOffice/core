# Sheets round thirty-two — probe data

Base commit `12cf74f08`, checked with `git log --oneline -1` in the worktree before anything was
measured. Reference renderer LibreOffice 24.2.7.2. Both whole-track sweeps ran against a
checksummed copy of the CLI, and the copy's `Paperless.Spreadsheets.dll` hash was diffed against
the tree's before each run started; the fixed copy was additionally proved to hold the change by
rendering one document whose word count it moves.

| file | what it is |
|---|---|
| `base-whole-track.tsv` | whole-track sweep at `12cf74f08`, before any change |
| `final-whole-track.tsv` | the same on the final tree |
| `reach.tsv` | the byte-level reach, clock pinned — **the ten VML-carrying workbooks, not the whole track**; see below for why that is the complete answer here |
| `score.py` | turns a `rows.tsv` into matches, page error, exact counts, word error |
| `reach.sh` | the round thirty-one whole-track version, kept because it is the general tool; **this round did not run it** |
| `mutate.sh` | the reintroduced defects the round's tests were verified against |

## The baseline reproduces the brief exactly

```
148/171 matches   abs page error 86   exact page counts 155   abs word error 33174
001 10/10  002 10/10  003 10/10  004 10/10  005 10/10  006 10/10  007 10/10  008 10/10
009 9/9    010 8/10   011 6/10   012 8/10   013 8/10   014 9/10  015 6/9   016 5/9
017 6/10   018 3/4
```

Every figure in the brief, to the digit, including the per-batch line. 171 rows, no duplicate
path, no `ref-failed` and no `ours-failed`.

**The first attempt at that sweep was garbage and the reason is worth writing down.** An earlier
run started with `nohup … &` inside a backgrounded tool call was orphaned rather than killed when
the call returned, so a second run appended to the *same* `rows.tsv` while six workers competed
for four cores: duplicate paths, and 30-odd documents reported `ours-failed`/`ref-failed` purely
from the 240-second timeouts. Both symptoms look exactly like a code regression. `ps -eo ppid` on
the sweep script is the check — an orphan has `ppid` 1 — and the two sanity checks the skill
prescribes (`wc -l`, `cut -f1 | sort | uniq -d`) both caught it.

## Visible cell comments

`Application_Compliance_Checklist_5_Apr_2021.xlsx` matched on pages and missed the word gate by
483 against a 354 band. The residue is its **shown cell comments**, which LibreOffice draws inline
and we drew not at all.

The token multiset says so before any code is written: under-draw 508, over-draw 25, and the
under-draw is the comment text with each token repeated six times — `the` 42, `please` 18,
`Does` 6. Ten of those 508 are the printed date, which differs because our leg had
`SOURCE_DATE_EPOCH` pinned and the reference's did not.

Four decisions, each measured against LibreOffice's own answer rather than derived:

1. **Visibility is the VML shape's CSS, not the `x:Visible` element beside it.**
   `Comment::finalizeImport` reads `pVmlNoteShape->getTypeModel().mbVisible`
   (`sc/source/filter/oox/commentsbuffer.cxx:257`). Excel writes `<x:Visible/>` on shapes it also
   marks `visibility:hidden`; five of the nine note shapes on this workbook's two annotated sheets
   do exactly that, and LibreOffice's flat-ODF export carries an `office:annotation` for none of
   them.

2. **The rectangle comes from `x:Anchor`, and only from the shape's CSS when there is none.**
   `ShapeBase::calcShapeRectangle` tries the client anchor first
   (`oox/source/vml/vmlshape.cxx:509-517`). On this workbook the two disagree by inches — the CSS
   puts the first caption at 1013.25 pt and the anchor at 1067.6 pt — and the export reports the
   anchor's answer.

3. **The anchor's offsets are screen pixels at 96 per inch**, not EMUs:
   `ShapeAnchor::importVmlAnchor` sets `CellAnchorType::Pixel` (`drawingbase.cxx:152-155`).

4. **A caption is placed against the *commented* cell, not against the cell its anchor names.**
   `ScNoteUtil::CreateNoteData` throws the absolute rectangle away and keeps
   `maCaptionOffset`/`maCaptionSize` relative to the cell (`postit.cxx:966-973`), and
   `CreateCaptionFromInitData` puts it back at `cellRect.Right() + offset` (`:1046-1053`). This is
   the decision that matters most here and it is invisible on a sheet without print titles: all
   four shown comments on this workbook's printed sheet belong to cells in row 1, the sheet
   repeats row 1 on every page, so LibreOffice draws all four on each of its six pages. Anchoring
   them where their VML says drew them on the first page alone — measured, +220 words instead of
   +445.

**The geometry was reproduced in a script before any C# was written.** Working from LibreOffice's
own flat-ODF export of the workbook and of an authored fixture, the arithmetic above reproduces
every one of the eight `svg:x/y/width/height` values to within two hundredths of a millimetre.
One of those checks refuted a wrong first reading: three captions came out 0.244 in shorter than
the export, which looked like the caption's auto-grow height — and it was the *row heights*, since
Calc freezes the caption's size at import and the optimal-height pass afterwards shrank three rows
by exactly that much. Using the file's own `ht` values instead made all four agree.

### What is knowingly approximate

- **The frozen size.** The rectangle here is resolved against the grid the page is drawn on rather
  than against the file's stated row heights, so a caption spanning rows the optimal-height pass
  changes ends up as tall as those rows become. Separating the two grids costs a second resolution
  of every sheet's geometry and moves no measurement on this corpus.
- **The caption is drawn as a rectangle**, not as `SdrCaptionObj`'s rounded box with a tail to the
  cell and a shadow. The fill (`#ffffe1`, or the VML's own `fillcolor`) and the hairline border are
  drawn.
- **A comment with no VML note shape at all is shown by LibreOffice and is not shown here.**
  `Comment::finalizeImport` initialises `bVisible` to *true* and only overwrites it from a shape
  it finds (`commentsbuffer.cxx:222-258`), so such a comment gets a caption at Calc's own default
  position and size through `AutoPlaceCaption`. That branch is not implemented, and the census
  says why it could not be measured either way: of the track's **8** workbooks carrying a comments
  part, **0** lack a VML part.
- **LibreOffice drops the character formatting of a comment's *first* run** when it builds the
  caption — measured on both the corpus workbook and the authored fixture, where the reference
  draws the first run at the Note style's 10 pt Liberation Sans and every later run at the stated
  face and size. It looks like the property write on the replaced text range rather than a rule,
  it is not reproduced, and the tests assert positions rather than faces because of it.

### Reach, as a ceiling and as a measurement

**For once the census covers the whole track.** 110 of the 171 documents open as a zip and 61 as
an OLE2 workbook stream, and 110 + 61 is 171. Of the 110, **one** carries a VML note shape marked
`visibility:visible`. Of the 61, **none** carries a BIFF `NOTE` record with `fShow` set — so the
binary half, which no OOXML census can see, is empty here rather than unknown, and the `.xls`
reader is deliberately left as it was (`XlsDrawing` already drops note objects and says why).

Ten documents carry a VML part at all, which is the structural precondition for the new code path
to do anything. Rendering all ten with both CLIs under `SOURCE_DATE_EPOCH=1700000000`:
**one differs, nine are byte-identical** — the same answer the census gives.

| document | bytes |
|---|---|
| `Application_Compliance_Checklist_5_Apr_2021.xlsx` | **differ** |
| the other nine VML-carrying workbooks | identical |

## Tests

`tests/corpus/features/sheet-cell-comment-shown.xlsx`, authored rather than copied: one sheet
repeating row 1 as a print title, a shown comment on `B1` whose CSS position disagrees with its
anchor by 500 pt, a comment on `B3` carrying `<x:Visible/>` *and* `visibility:hidden`, and a shown
comment on `E1` reaching four columns past the last cell. LibreOffice's export of it carries
exactly two annotations, both `office:display="true"`, and its PDF is three pages with both
captions drawn on pages 1 and 2 at the same place.

Five cases in `SheetShownCommentTests`, verified by reintroducing each defect through
`.claude/skills/corpus-batches/scripts/verify-test.sh`:

| mutation | cases that fail |
|---|---|
| `m1` the VML relationship is never resolved, so no caption is read | 4 |
| `m2` visibility taken from `x:Visible` rather than from the style | 1 |
| `m3` the anchor's pixel offsets read as points | 1 |
| `m4` the caption placed against the cell its anchor names | 1 |
| `m5` the text inset left at an ordinary drawing object's rather than the Note style's | 1 |

`m1`'s first version returned early from the reader and **failed to build** — `TreatWarningsAsErrors`
turns the unreachable code into an error, and `verify-test.sh` correctly refuses to call that a
detection. Rewritten as a relationship-name change it detects four of the five.

## The whole-track result

Both sweeps: 171 rows, no duplicate path, **zero `ref-failed`, zero `ours-failed`**.

| | matches | abs page error | exact page counts | abs word error |
|---|---|---|---|---|
| `base-whole-track.tsv` | 148/171 | 86 | 155 | 33174 |
| `final-whole-track.tsv` | **149/171** | 86 | 155 | **32729** |

Per batch at the end: 001–009 89/89, 010 8/10, **011 7/10**, 012 8/10, 013 8/10, 014 9/10,
015 6/9, 016 5/9, 017 6/10, 018 3/4. **No batch fell.**

**Exactly one row of 171 changes, and it is the one the round was aimed at.**

| document | before | after |
|---|---|---|
| `Application_Compliance_Checklist_5_Apr_2021.xlsx` | 14/14, 17235/17718 `words` | 14/14, **17680/17718 `match`** |

The word error moves by 445, which is the whole of the track's 33174 → 32729; no page count
anywhere moves, which is right, because a caption is an object on the sheet and the print area it
widens was already wide enough to keep this workbook at fourteen pages. That narrowness is the
honest headline: the census and the byte-level reach agreed on one document before the sweep ran,
and the sweep found one. The change is worth having because the mechanism is general — any
workbook with a pinned-open comment — and the corpus happens to hold one instance of it.

## What batches 011 and 016 are, triaged and not fixed

Both batches' residue is **pagination with the words already right**, and the diagnosis below is
the first divergent page of each document, read off the baseline sweep's own renderings rather
than re-rendered.

| document | batch | pages | words | first divergence |
|---|---|---|---|---|
| `Capability_List_…unsorted.xlsx` | 011 | 150/147 | 29917/29920 | page 12, a different column band |
| `RMP 2011-2014 and Inventory.xls` | 011 | 39/38 | 18634/18548 | page 1 ends a column early |
| `SIL_TDB648.xlsx` | 016 | 89/88 | 7680/7679 | page 5 starts a column late |
| `tk-syllabus-comparison-document-v5.xlsx` | 016 | 852/855 | 235372/235154 | page 5 starts a column late |
| `flightstandards-doc-Cross-reference-table_version02.xlsx` | 016 | 461/464 | 68159/68113 | page 2, a cell wraps one token later |
| `7-memento-2015-transports-aeriens-b.xls` | 016 | 190/191 | 28848/28846 | page 7, a word missing from a wrap |

**Five of the six are the same quantity with both signs: how many columns fit in one column
band.** On `SIL_TDB648` and `RMP` our band takes one column *fewer* than LibreOffice's, so we
print an extra band and an extra page; on `tk-syllabus` our band takes one *more*, so we print
three fewer. `Capability_List`'s page 12 holds a band ours has already moved past. That is the
shape the skill warns about — opposite signs of one quantity are the same code with the same
bug — and it says the lead is the predicate that decides whether the next column still fits the
printable width, not six separate documents.

The remaining two are cell wrapping rather than paging: on `flightstandards` the header
`IR/AMC/GM` breaks after the first solidus in the reference and after the second in ours, which
is a width question rather than a break-opportunity one, since both renderers break after a
solidus and ours simply fits more before it.

`T0A0D0000090006XLSE.xls` (011, pages exact, **+2098 words**) is not in that cluster: we
over-draw. It was not chased.

**One residue outside these batches is much larger than any of them and is cheap to state.**
`EASA-IFP-145Scope(WEB)_…xlsx` (018, 114/114 pages, 32485 against 34835 words) loses exactly
2350 words to one boundary: the reference draws `EASA.UK.1` and the next cell's text as two
tokens and we draw them touching, so `pdftotext` reads `EASA.UK.1AEM` as one. The token multiset
is decisive — under-draw 4747, over-draw 2397, and the difference is exactly the 2350 occurrences
of `EASA.UK.1`. Both renderers join the *following* pair (`LIMITUNITED`), so this is one column's
width or alignment rather than the sink's operator granularity.

## An unexplained vertical offset on the checklist's own sheet

Worth recording because it was found while measuring something else and is not the comments.
On `Application_Compliance_Checklist_5_Apr_2021.xlsx` our whole printed block on the
`App. Compliance Checklist` sheet sits **31.2 pt lower** than LibreOffice's — the repeated title
row's own text is at 85.96 pt where the reference draws it at 54.75, and every horizontal
coordinate agrees to a fifth of a point. The captions are placed correctly *relative to our own
block*, which is why they land 31.4 pt below the reference's. It moves no gate, it is not the
comments, and it was not chased.

## Test counts

Every project run individually, whole output captured, **0 skipped** everywhere.

| Core | Containers | Text | Vector | Rendering | Markup | OpenDocument | WordProcessing | Spreadsheets | Presentations |
|---|---|---|---|---|---|---|---|---|---|
| 275 | 109 | 240 | 291 | 119 | 259 | 125 | 696 | **593** | 542 |

Every count is the briefed known-good except Spreadsheets, which is 588 plus this round's 5.

`Paperless.Fidelity.Tests` on the final tree: **550 of 550, 0 skipped**, the briefed known-good
count exactly, in 12 minutes under a load average of about 20.
