# Sheets round thirty-one — probe data

Base commit `150a3dac1`, verified with `git log --oneline -1` before anything was measured.
Reference renderer LibreOffice 24.2.7.2. Both whole-track sweeps ran against a checksummed copy of
the CLI, and the copy's `Paperless.Spreadsheets.dll` hash was diffed against the tree's — and
against the *other* snapshot's — before each run started.

| file | what it is |
|---|---|
| `base-whole-track.tsv` | whole-track sweep at `150a3dac1`, before any change |
| `final-whole-track.tsv` | the same on the final tree |
| `reach.tsv` | which documents' rendered bytes the round moved, clock pinned |
| `score.py` | turns a `rows.tsv` into matches, page error, exact counts, word error |
| `reach.sh` | renders a track with two CLIs under a pinned clock and diffs the bytes |
| `mutate.sh` | the reintroduced defects the round's tests were verified against |

## The baseline reproduces the brief exactly

```
147/171 matches   abs page error 90   exact page counts 154   abs word error 42322
001 10/10  002 10/10  003 10/10  004 10/10  005 10/10  006 10/10  007 10/10  008 10/10
009 9/9    010 8/10   011 6/10   012 8/10   013 8/10   014 9/10  015 5/9   016 5/9
017 6/10   018 3/4
```

Every figure in the brief, to the digit, including the per-batch line. 171 rows, no duplicate
path, no `ref-failed`.

## A collapsed outline group hides its detail rows

SpreadsheetML states an `outlineLevel` on each row and a `collapsed` flag on the summary row
*beside* a group, and leaves the consequence to be derived. Excel normally also writes
`hidden="1"` on every detail row, which is why the derivation is invisible on almost everything —
and a file whose writer omitted it paginates entirely differently without it.

`Application_Compliance_Checklist_5_Apr_2021.xlsx` (batch 011) is that file. Its
`App. Compliance Checklist` sheet states **no `hidden` attribute anywhere** and **329 of its 1033
rows** are hidden by the outline rule alone. It printed 18 pages against LibreOffice's 14 and drew
26353 extractable words against 17718.

The rule is `WorksheetGlobals::convertOutlines`
(`sc/source/filter/oox/worksheethelper.cxx:1307-1334`): a stack of the first index at each open
level; a higher level pushes the current index once per new level; a lower one pops every level
above it and groups each popped level over `[first, current-1]`; only the *innermost* popped group
is hidden, and only when the index that closed it carries the flag. A gap between two stated rows
takes the sheet's default model — level zero, no flag — so it closes every open group without
hiding anything.

**The algorithm was prototyped in Python and checked against LibreOffice's own answer before any
C# was written.** LibreOffice's flat-ODF export of that workbook marks 329 rows
`table:visibility="collapse"`; the prototype produced the same 329, set-equal, nothing on either
side alone.

BIFF deliberately does **not** use this. `XclImpOutlineBuffer` feeds `ScOutlineArray::Insert` a
collapse flag that records the group's state for the user interface
(`sc/source/filter/excel/exctools.cxx:84-129`) and never hides a row; a BIFF `ROW` record carries
`fHidden` itself. XLSB does use it, through the same `WorksheetGlobals`.

### Reach, as a ceiling and as a measurement

A census of `collapsed="1"` over `xl/worksheets/*.xml` finds it in **one** of the track's
documents. **That census reads 118 of the track's 171 documents** — the rest are OLE2 and
carry no such part — but the BIFF half cannot be reached by this rule at all, so for once the
census's blind spot does not matter: the ceiling is one document and the measured reach is one
document.

## The header and footer band was drawn in four wrong ways at once

All four are visible in a single measurement on `fm-provider-service-measures.xlsx` (batch 015),
whose page count already matched at 38/38 and whose word count was 20944 against 21458.

| | ours, before | LibreOffice |
|---|---|---|
| first header line, glyph top, page 34 | 30.212 pt | **21.609 pt** |
| last footer line, glyph bottom, page 34 | 761.571 pt | **770.312 pt** |
| header glyph height (`&14`, page 34) | 11.17 pt | 8.17 pt |
| second and third header lines (page 12) | not drawn | drawn |

1. **Only the first line of each part was drawn.** Every format can write a line break into one
   part; Calc holds each area as an `EditTextObject` of several paragraphs and draws the whole
   object (`ScPrintFunc::PrintHF`, `sc/source/ui/view/printfun.cxx:1874-1912`).
2. **The `&n` size code was parsed and discarded.** The parser consumed it so that the digits
   would not print, and the band was then drawn at a fixed ten point.
3. **The page's own zoom was not applied.** `PrintHF` switches the device to `aTwipMode`, whose
   scale fraction *is* the zoom (`InitModes`, `:2645`), so a header on a sheet printed at a third
   is drawn at a third.
4. **The text was centred in the band where Calc anchors it.**
   `XclImpPageSettings::Finalize` marks every Excel band dynamic and, where the text does not fit
   between the two margins, shrinks the band instead (`sc/source/filter/excel/xipage.cxx:316-331`)
   — so `PrintHF`'s `nDif` is at or below zero either way and the text sits against the edge the
   margin fixes: a header's top, a footer's bottom.

A fifth and a sixth came out of the fixture and are the reason the fixture exists:

5. **The three parts share one band height** — the tallest of the three — and each is *centred*
   in it. `PrintHF` gives the EditEngine one `aPaperSize` and computes `nDif` per area against it;
   `UpdateHFHeight` has already set that height to the greatest of the three (`:820-834`).
   Measured on the fixture, whose footer holds two 8 pt lines on the left and one 10 pt line on
   the right: LibreOffice puts the right line's glyph box 3.35 pt below the band's top edge —
   exactly half the 6.7 pt difference — where anchoring each part on its own puts it hard against
   the footer margin.
6. **A section switch resets the font to the workbook's own default.** `SetNewPortion` calls
   `ResetFontData` (`sc/source/filter/excel/xihelper.cxx:534-548`) and the OOXML parser does
   `maFontModel = getStyles().getDefaultFontModel()` (`sc/source/filter/oox/pagesettings.cxx:868`).
   `&L&8…&RFoot right` draws the right part at ten point, not eight. `SheetBandHeight` — which
   computes the band's *height* and predates this round — carried the same defect, so the two now
   agree.

After the change, on the same two pages of `fm-provider-service-measures.xlsx`: header top
**21.601** against 21.618 and 21.602 against 21.609; footer bottom **590.400** against 590.343 and
770.400 against 770.312. Within a twentieth of a point on both edges, at two different zooms.

**One thing is measured and not explained.** Our drawn glyph heights are consistently about 8%
smaller than LibreOffice's — 3.72 pt against 4.018 at `&9`, 7.506 against 8.170 at `&14`, on two
sheets with different zooms. The positions agree and the size does not, so the residue is in the
zoom the band is drawn at rather than in the placement. It moves no gate and it is not chased
here.

### Reach, as a ceiling and as a measurement

A census of a newline inside `<oddHeader>`/`<oddFooter>` and their `even`/`first` variants finds
**7 documents**, over the **118 of 171** the census can read; the other 53 are OLE2 and state the
same string in a BIFF `HEADER`/`FOOTER` record no zip-level census sees. The other three defects
— the size code, the zoom and the anchoring — reach **every document with a header or footer at
all**, which is most of the track, and the byte-level reach run below is the measurement.

## Tests

Two classes on one authored fixture, `tests/corpus/features/sheet-outline-collapse.xlsx`. It is
built to separate the decisions rather than copied from a corpus workbook: a level-1 group closed
by a row carrying `collapsed`, an identical group closed by one that does not, and a nested pair
where only the inner one collapses. Its header is two 14 pt lines and its footer has two 8 pt
lines on the left against one line stating no size on the right. Every asserted coordinate is
LibreOffice 24.2.7.2's own, read off its flat-ODF export and its PDF of that file.

LibreOffice hides rows 4–8 and 19–21 of its 23, and we now hide the same eight.

Verified by reintroducing each defect (`mutate.sh`, run through
`.claude/skills/corpus-batches/scripts/verify-test.sh`):

| mutation | cases that fail |
|---|---|
| `m1` the collapsed group is never applied | 3 |
| `m2` only the first line of each part prints | 3 |
| `m3` the stated size is discarded | 5 |
| `m4` the band is centred rather than anchored | 2 |
| `m5` each part is anchored on its own | 1 |
| `m6` the size carries across a section switch | 2 |

**`m6`'s first version came back undetected and the mutation was wrong, not the test.** It
changed the `&L` case alone, and the case that detects the defect uses `&L&8Left&RRight`, where
the reset that matters is the one on `&R`. Recorded because "undetected" was the honest first
reading and it took a second look to see which side was at fault.

## The whole-track result

Both sweeps: 171 rows, no duplicate path, **zero `ref-failed`**.

| | matches | abs page error | exact page counts | abs word error |
|---|---|---|---|---|
| `base-whole-track.tsv` | 147/171 | 90 | 154 | 42322 |
| `final-whole-track.tsv` | **148/171** | **86** | **155** | **33179** |

Per batch at the end: 001–009 89/89, 010 8/10, 011 6/10, 012 8/10, 013 8/10, 014 9/10,
**015 6/9**, 016 5/9, 017 6/10, 018 3/4. **No batch fell.**

Two verdicts moved, and only one of them is a match:

| document | before | after |
|---|---|---|
| `fm-provider-service-measures.xlsx` | 38/38, 20944/21458 `words` | 38/38, 21364/21458 **`match`** |
| `Application_Compliance_Checklist_5_Apr_2021.xlsx` | 18/14, 26353/17718 `pages,words` | **14/14**, 17235/17718 `words` |

**The checklist is the round's biggest single movement and it still fails the word gate**, by 483
words against a 354 band. What is left is its **visible cell comments**: the workbook's sheets
carry `comments2.xml` and a VML drawing marking several of them visible, LibreOffice draws them
as note shapes inline, and we draw none of them. That is also why our fit-to-width zoom is 2.6%
larger than LibreOffice's on that sheet — the note shapes extend its print area four columns to
the right, to column O against our K. Diagnosed, **not implemented**.

Twelve documents moved their word count by twenty or more, ten of them towards the reference:

| Δ error | document | before → after |
|---:|---|---|
| −8152 | `Application_Compliance_Checklist_5_Apr_2021.xlsx` | 26353 → 17235 / 17718 |
| −420 | `fm-provider-service-measures.xlsx` | 20944 → 21364 / 21458 |
| −314 | `cy01_state.xls` | 20235 → 20559 / 20554 |
| −305 | `npias_2009_appA.xls` | 49492 → 49857 / 49827 |
| −82 | `TICAPCapability_Final.xls` | 4838 → 4940 / 4930 |
| −49 | `cy06_primary_np_comm.xls` | 7037 → 7092 / 7089 |
| −48 | `FAA-2019-0995-0002_attachment_2.xlsx` | 10106 → 10154 / 10245 |
| −26 | `SLSA_Directory_031423.xlsx` | 5874 → 5904 / 5902 |
| −24 | `jobs-bulletin-51-22-december-2025.xlsx` | 1866 → **1890** / 1890 |
| −24 | `hdss-bulletin-index-2019-2022.xlsx` | 3741 → 3765 / 3780 |
| +48 | `RMP 2011-2014 and Inventory.xls` | 18586 → 18634 / 18548 |
| +266 | `FY2023-AIP-grants.xlsx` | 91910 → 92176 / 91849 |

Four of those ten are `.xls`, which no census of an OOXML part could have found and which is
exactly the blind spot the skill warns about — the multi-line footer is stated in a BIFF `FOOTER`
record there. The two that moved *away* both stay on the verdict they had: `RMP` was failing on
its page count before and after, and `FY2023-AIP-grants` is 0.36% over a 2% band.

**Only one page count in the whole track changed**, and the page-error and exact-count movements
are entirely that document: 90 − 4 = 86, 154 + 1 = 155. That is the outcome to be suspicious of
and it has the same explanation the margin round had — a header band's height comes from the two
margins and `SheetBandHeight` already modelled its growth, so drawing the band correctly moves
ink and words and not paper. The outline rule is what moved paper, on the one document that has
one.

## Byte-level reach

`reach.sh`, both CLIs under `SOURCE_DATE_EPOCH=1700000000`.
