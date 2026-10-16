# sheets-rest-01 — results

The five `sheets/*` failures that were not documented ceilings. **Three closed, one closed as a
decision, one advanced but not closed.**

Everything here is measured against the installed **LibreOffice 26.2.4.2**, reusing the banked
reference PDFs at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/sheets/`, with
`SOURCE_DATE_EPOCH=1700000000`. `check-env.sh` reports OK on all five checks, including
`DejaVu Sans -> DejaVu Sans` and both metric-compatible pairs. The 27.2-alpha C++ tree was read for
mechanism and **is contradicted twice below** — once decisively.

## Scoreboard

| | before | after |
|---|---:|---:|
| `sheets/*` (all 171) | 160 match / 11 mismatch | **163 match / 8 mismatch** |
| `sheets/done-*` (156) | 156 / 156 | **156 / 156 — no regression** |

| document | before | after | |
|---|---|---|---|
| `table-001/…/FAA-2019-0995-0002_attachment_2.xlsx` | 32/33 pages | **33/33, words 9995/9994 — match** | fixed |
| `metrics-001/…/ans_mappings_of_eccairs_terms.xlsx` | 190/191 pages | **191/191, words 27894/27896 — match** | fixed |
| `pagination-001/…/CIS_Debian_Linux_8_Benchmark_v1.0.0.xls` | words 7541/8981 | **9021/8981 — match** | fixed |
| `missing-001/…/orbus_togaf_tool_csq.xls` | 33/75 pages | 33/75 pages | **declined, see §2** |
| `pagination-001/…/SIL_TDB648.xlsx` | 89/90 pages | 89/90 pages | **advanced, not closed** |

**The baseline sweep reproduced the brief exactly** — 32/33, 190/191, 7541/8981, 33/75 and 89/90 —
which is what validates the harness before any of this is trusted.

**One number in the brief does not reproduce and the reason is known.** It says 161 of 171; the
baseline sweep here says 160. The difference is `fse_identification_form.xlsx`, the recorded
unstable document, which read `words 440/427` on this run — a 13-word gap against a 8.5-word band.
It is unstable in exactly the recorded way and was not touched. **`done-*` is 156 documents, not
the 161 the brief states**; 161 is `done-*` plus the five ceilings.

## 1. `FAA-2019-0995-0002_attachment_2.xlsx` — a run of blanks wider than the line

**Seat:** `LineFiller`, new option `BreaksOverflowingBlanks`, set by all four sheet layouters.

**The brief's diagnosis was right about the document and wrong about the seat, and the wrong part
is worth correcting in `render-comparison`.** It lists this as the `SheetGrid.IsOptimalSize` entry
in the "read but never used" table. That property is *thoroughly* used —
`SheetOptimalRowHeights.Apply` consults it three times and the whole class exists to honour it. The
table row is stale and should come out; keeping it sends the next round looking for a consumer that
has been there for several rounds.

What is actually wrong is one column of the Accessory List sheet. Column S holds 100-character
strings that are mostly trailing spaces (`"inspected in situ"` and 83 blanks, or 100 blanks), in a
71.25 pt wrapping column. A word processor lets a line's trailing blanks hang past the margin —
that is Writer, and it is what `TextMeasurer.TrimTrailingSpaces` is for. **EditEngine does not.**
`ImpEditEngine::ImpBreakLine` walks a character-position array holding every advance, blanks
included, and when the character it stops on is a blank it breaks one past it and compresses that
blank away (`editeng/source/editeng/impedit3.cxx:2016-2035`).

Measured on `mkspaceprobe.py` — eighteen wrapped cells differing only in their trailing whitespace,
read out of the installed binary's own flat-ODF row heights:

| blanks | 10 | 20 | 30 | 40 | 60 | 80 | 100 | 120 | 160 | 200 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| reference lines | 1 | 1 | 1 | 2 | 2 | 3 | 4 | 4 | 5 | 7 |
| ours, before | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
| ours, after | 1 | 1 | 1 | 2 | 2 | 3 | 4 | 4 | 5 | 7 |

**17 of 17 measurable probe rows exact**, including the two mixed cases (`"inspected in situ"` plus
83 blanks, 40 `M` plus 60 blanks) and the control that has no trailing blanks at all.

On the document: the reference recomputes the Accessory List's rows to 19.39 / 28.35 / 37.30 pt —
two, three and four EditEngine lines — and every 37.30 pt row is one holding a column-S string,
with the correlation exact over all 95 recomputed rows. We gave those rows two or three lines,
fitted 25 body rows on page 28 where the reference fits 18, and lost a page. **The row boundaries
on page 28 now agree rule for rule**: 19 consecutive pitches, every one within 0.01 pt.

The rule is deliberately narrow at both ends. It fires only when the line's *own trailing blanks*
overflow it, and it breaks where EditEngine breaks — one past the blank, so the blank stays on the
line and is compressed to nothing. A line whose single trailing blank merely straddles the edge is
untouched, which is pinned as its own case: that is what stops this from moving every justified
line in the corpus.

**Reach: 31 of 171 renderings changed, 7 of them on the gate, all toward the reference.** One
(`MajCh-Digital-Certificate-Publication-Report.xlsx`) went from 66897 to the reference's exact
66901. Nothing regressed.

**It also removed a workaround.** `SheetCentredWrapBlankTests` asserted that a centred line kept
all 44 of its trailing blanks and that a placement guard then pulled it back inside the cell — and
its own remark said "EditEngine keeps only the blanks that fit". It now does: measured against
26.2.4.2's own PDF of that fixture, the reference draws 29 glyphs, then 23 blanks, then 8, and we
now draw 29, 23 and 8 where we used to draw 52 and then 8. The test was rewritten to assert the
reference's line structure rather than the clamp.

## 2. `orbus_togaf_tool_csq.xls` — the DPCache sheet. **Declined, and here is why.**

The reference prints 75 pages; pages 34-75 are headed `DPCache`. Our 33 pages reproduce its first
33: **32155 reference words on pages 1-33 against our 32183 for the whole document**, a 28-word
gap inside a 643-word band. The entire 14 625-word difference is the DPCache sheet.

The workbook has six `BOUNDSHEET` records and we extract exactly those six.
`XclImpPivotCache::ReadPivotCacheStream` manufactures a seventh from the `PTCACHE` storage stream
when a pivot table's source sheet is external or deleted
(`sc/source/filter/excel/xipivot.cxx:716-734`). All of that is as the brief describes.

**What the brief does not say, and what decides it: LibreOffice has already deleted this
behaviour.** Upstream commit `6bc8bae7047d9ce13d4f287866491eb862bf4816`, *"sc: hide helper sheet
for external pivot sources in XLS"*, dated **2026-05-09**, adds one line to that very block:

```cpp
rDoc.RenameTab( nScTab, aName );
rDoc.SetVisible( nScTab, false );          // <- added 2026-05-09
```

That is after 26.2.4.2 branched, and the installed binary behaves accordingly — its flat-ODF export
of this workbook gives the DPCache table `table:display="true"`, so 26.2.4.2 creates it visible and
prints it while 27.2 will create it hidden and will not.

So matching the banked reference here would mean **writing a PTCACHE record reader in order to
reproduce 42 pages of a synthesised helper sheet that no user authored, that Excel never shows, and
that the reference implementation itself removed from its output three months ago.** The 42 pages
would then have to be deleted again the next time the container's LibreOffice moves.

**Decision: decline.** `orbus_togaf_tool_csq.xls` stays a permanent `pages,words` mismatch against
the 26.2.4.2 bank, and it should be reclassified out of `missing-001` — it is not a missing-content
defect, it is a reference-side defect with an upstream fix already landed. The honest statement of
our position is that our 33 pages are the document, and they match the reference's 33 page for page
and word for word.

The counter-argument, stated fairly: the gate measures agreement with the installed reference, and
this document will now never pass it. If the project's rule were "match the banked reference
whatever it does", this is where that rule costs something real. It is being overridden here on the
grounds that the reference is measurably wrong *by its own maintainers' judgement*, which is a
stronger warrant than our own opinion of what a user wants to see.

## 3. `ans_mappings_of_eccairs_terms.xlsx` — a `<font>` that names no face is Cambria 11

**Seat:** `XlsxCellFormats.ReadFont`, `UnnamedFontFamily` and `UnnamedFontPoints`.

Five of this workbook's seventeen fonts name no face — `<font/>`, `<font><b/></font>`,
`<font><color …/></font>` and so on. Every `<font>` the OOXML filter builds begins as a copy of the
theme buffer's default model, and that model is a hard-coded `Cambria` at 11.0 pt
(`ThemeBuffer::ThemeBuffer`, `sc/source/filter/oox/themebuffer.cxx:31-33`, marked "TODO: locale
dependent font name" and never made one). Nothing in the theme part overrides it.

`XlsxCellFormatTable.Apply`'s remark already named that constant and deliberately used the
workbook's `fonts[0]` instead, on the grounds that a file omitting `rFont` or `sz` is one "no
producer writes". **This document writes it**, and so the argument is refuted for the `<font>` path.
The `rPr` path is left alone — see §6.

Measured on `mkfontprobe.py`, whose `fonts[0]` is Arial 10 so the two candidate answers are
distinguishable, reading the face out of the installed binary's own PDF:

| `<font>` | reference | ours before | ours after |
|---|---|---|---|
| `<font/>` | Caladea-Regular 11.00 | sans 10 | **Caladea-Regular 11.00** |
| `<font><b/></font>` | Caladea-Bold 11.00 | sans-bold 10 | **Caladea-Bold 11.00** |
| `<font><sz val="20"/></font>` | Caladea-Regular 20.01 | sans 20 | **Caladea-Regular 20.01** |
| `<font><name val="Arial"/></font>` | LiberationSans **11.00** | LiberationSans 10 | **LiberationSans 11.00** |
| `<font><color …/></font>` | Caladea-Regular 11.00 | sans 10 | **Caladea-Regular 11.00** |
| `<font><u/><color …/></font>` | Caladea-Regular 11.00 | sans 10 | **Caladea-Regular 11.00** |

7 of 7 exact, control included. The size half is the wider rule and was wrong on its own: a
`<font>` that names a face and omits `sz` was a point small.

The document goes to 191/191 pages, 27894 words against 27896, and now embeds the reference's seven
faces exactly — `Caladea-Regular` and `Caladea-Bold` included, where it embedded five before.

**Reach: 1 of 171 renderings changed.** Only this document.

## 4a. `CIS_Debian_Linux_8_Benchmark_v1.0.0.xls` — an edit cell is not shortened

**Seat:** `SheetTextLayout.IsEditCell`, consulted beside `HasEditCharacters` before `Shorten`.

`ScOutputData::LayoutStrings` reaches for the EditEngine before it looks at a single character:
`else if (aCell.getType() == CELLTYPE_EDIT) bUseEditEngine = true`
(`sc/source/ui/view/output2.cxx:1710-1712`). The two paths disagree about a string that does not
fit: `DrawStrings` shortens it before showing it, `DrawEdit` keeps every character behind a clip.
Only the second leaves the hidden tail in the PDF's text layer — which is the half `pdftotext`
scores. Our `Shorten` was guarded by `HasEditCharacters` and `IsField`, which are two of the ways a
cell reaches that path and not the main one.

Measured on `mkclipprobe.py`, five rows in a column too narrow for their text with the neighbour
occupied so nothing may spill:

| row | reference characters in the text layer | ours before | ours after |
|---|---:|---:|---:|
| plain | 23 of 130 | 24 | **23** |
| rich | **130** | 24 | **130** |
| hard break, cell not wrapping | **130** | 23 | **130** |
| both | **130** | 23 | **130** |

The corpus case is exactly that shape: the CIS remediation and audit columns hold paragraphs with
blank lines between their parts, so every one of them is an edit cell, and the 1440 words the
document was short were their hidden tails. It is now **9021 against 8981**, a 40-word gap inside a
179-word band, at 88/88 pages.

**Reach: 5 of 171 renderings changed, all improving.** `Capability_List…unsorted.xlsx` went to the
reference's exact 29796, `activespecs.xls` from 46566 to 46582 against 46583, `capa-liste-nse-1.xls`
from 20199 to 20202 against 20205.

## 4b. `SIL_TDB648.xlsx` — advanced, **not closed**

**Seat found and fixed:** `SheetOptimalRowHeights.StandingEditLine`. An edit cell is measured
through `GetNeededSize`'s EditEngine branch even when it does not wrap (`bStdOnly` is cleared for
one, `column2.cxx:930-935`), and one EditEngine line is not the arithmetic height. Measured on the
auto-height half of the same probe, Calibri 11: a plain cell takes **276** twips —
`trunc(220 × 1.18) + 40 − 23`, the arithmetic answer exactly — and a rich, hard-broken or
rich-and-broken one takes **298**.

That is precisely this workbook's shape: its "TerrDB Verification" sheet is 285 twips a row for its
plain rows and 298 for its rich footnote rows, and 13 twips a row is enough to fit two extra rows
on a page. **The result is real and measurable: the first page whose content disagrees with the
reference moved from 13 to 17, and the number of pages differing under 1:1 alignment fell from 65
to 61.**

**It is still 89 pages against 90, and the residual is not a row height.** The reference's page 17
carries no text at all and exactly one operator: an image. We emit **2 images for this workbook
against the reference's 308**, because every picture in it is inside an `xdr:grpSp` and
`XlsxDrawings.ReadAnchor` counts a group's *anchor* for the print area but never descends into it to
read the pictures. So the page a drawing alone keeps has nothing to keep it.

That is a new front — group-shape traversal with its `chOff`/`chExt` transform — and it would add
306 pictures to one workbook's ink at the very end of a round with no time to sweep it. **Not
attempted, deliberately.** It is the whole of what is left on this document.

### Two probes that measured nothing, both worth recording

The first cut of the clip probe was **Arial 9**, whose line is shorter than
`ScGlobal::nStdRowHeight`; every row came back at the 256-twip floor and plain and rich were
indistinguishable. *A probe for a height needs a font tall enough that the floor does not bind.*

The second stated `fontId="0"` with no `applyFont` and no `<cellStyles>`, so LibreOffice drew the
plain rows in its own application default while the rich rows took the face their `rPr` named. That
version said a hard break did **not** reach the EditEngine height — the opposite of the truth. *Two
rows differing in one property must differ in only that one.*

## Reach, measured byte for byte

Not from a grep, and not from the gate columns, which see far less than the ink does. Each sweep
was rendered with `SOURCE_DATE_EPOCH` set and compared to the previous one file by file.

| change | renderings changed | on the gate |
|---|---:|---:|
| 1 — overflowing blanks | **31** of 171 | 7 |
| 3 — Cambria 11 | **1** | 1 |
| 4a — edit cell not shortened | **5** | 5 |
| 4b — rich row height | **4** | 0 |
| 4b widened to hard breaks | **0** | 0 |
| **the round** | **40** of 171 | 3 verdicts |

Two of those rows are worth reading properly. **4b changed four renderings and moved no gate
column at all** — it is a correctness fix with no scoreboard behind it, kept because it is measured
against the reference's own row heights and because it is what moved SIL_TDB648's page blocks into
alignment. And **widening 4b from "rich" to "rich or hard break" moved nothing in the corpus**: no
corpus document has a hard-broken non-wrapping cell with an automatic row height. It is kept anyway,
because the narrow version would have been a rule fitted to what this corpus happens to contain
rather than to what the probe measures.

**No document outside the sheets track can move.** Change 1 lives in `XRay.Office.Text`, and
`breaksOverflowingBlanks: true` is passed at exactly four call sites, all in
`SheetTextLayout`; the default is Writer's behaviour. The other three changes are in
`XRay.Office.Spreadsheets`. Corroborated by the fidelity suite, which covers all three families and is
unchanged at 30 failed of 550.

## Visual verification

Four labelled pairs were handed to **fresh subagents, blind** — no page counts, no expectations, no
access to the repository, one page each. The compositor first warned that each half was being shown
at 78-80% of its rendered size; the pairs were re-rendered at 117-120 dpi until it reported
**100%**, and only those were sent.

The two readings that came back are worth more than the verdicts they confirmed.

**`CIS` page 10 — the fix confirmed from the outside.** The reviewer reported the two halves as
geometrically and typographically the same render, with column 1 "hard-clipped mid-word at the
column border, no ellipsis, at the same x" in *both* halves, and singled out a row that "terminates
at the same character in both halves" as evidence the metrics agree. That is exactly right and is
exactly the point: the fix changed the **text layer** and left the **ink** alone. A reader of ink
should see no difference, and saw none.

**`FAA` page 28 — one confirmed finding, one refuted reading.**

*Refuted:* the reviewer reported our rows as taller than the reference's and our table as "~10%
taller". Checked in the operators rather than in the raster: the 19 horizontal rules on that page
now agree pitch for pitch, every one within 0.01 pt. What is real is a **constant 1.40 pt downward
offset of the whole block** — our table runs 520.61 → 77.09 where the reference runs 522.01 →
78.49. This is the skill's own warning landing: an image is not reliable for "whether two nearly
equal lengths are equal", and a stacked composite makes a constant offset look like accumulating
drift. The 1.40 pt is logged below.

*Confirmed:* the reviewer reported that the reference shades the six `MISSING` rows grey and that we
do not. Confirmed in the operators, not in the image: the reference paints **three `#C0C0C0` fills**
on that page and we paint **none**. Traced to its cause — sheet 10 carries
`<conditionalFormatting sqref="C5:F99">` and `"G5:R99"` with `<cfRule type="expression" dxfId="1">`
over a `dxf` whose fill is `bgColor indexed="22"`, which is silver. **We do not evaluate conditional
formatting fills.** Page count and word count both match on this document, so **no gate column can
see it**; only the blind read found it. Logged, not fixed — it is a feature, not one of the four
seats.

## Left for a later round

* **`SIL_TDB648.xlsx`'s remaining page.** `xdr:grpSp` contents are never read: 2 images against 308.
* **Conditional-formatting fills** (`cfRule` + `dxf`), found by the blind read on `FAA` page 28.
* **A constant 1.40 pt vertical offset** of the printed block on `FAA` page 28. Not a row height —
  the pitches agree exactly — so it is the page's top edge or the header band.
* **Blank-run gap widths inside a clipped string.** The `CIS` reviewer found, unprompted, that the
  reference's multi-space field separators are wider than ours in about eight rows and collapse to
  nothing in about four (`"root user:# grep"`, `".confNo results"`). Neither side's clip point moves.
  This is the compressed-blank half of §1 on the *drawing* path and is not reproduced.
* **The `rPr` fallback.** §3 fixed the `<font>` path only. The same `Font::Font(rHelper, bDxf=false)`
  seeds a formatting run from Cambria 11, and `XlsxCellFormatTable.Apply` still falls back to
  `fonts[0]`. Unmeasured, and left alone rather than changed by analogy — the argument that refuted
  it for `<font>` does not automatically transfer.
* **`render-comparison`'s "read but never used" table** lists `SheetGrid.IsOptimalSize`, which has
  been consumed by `SheetOptimalRowHeights` for several rounds. The row is stale.

## Prediction scorecard

Scored against `prediction.md`, committed before the final sweep and before any test was run.

| # | prediction | outcome |
|---|---|---|
| 1 | `sheets/*` ends at 163 or 164, floor 163 | **hit at the floor** — 163. The gain I hoped for did not exist |
| 2 | `done-*` stays clean | **hit** — 156/156, and it is 156 not the briefed 161 |
| 3 | SIL_TDB648 stays 89/90 | **hit** |
| 4 | `orbus` stays 33/75 | **hit**, deliberately |
| 5 | the five ceilings do not move | **hit** — all five unchanged to the word |
| 6 | `fse_identification_form` is a coin toss | **hit** — `words` on both baseline and final, unchanged |
| 7 | 4b moves 5 to 15 documents | **miss, and the useful kind** — it moved **4** in ink and **0** on the gate. I over-estimated by a factor of three and had guessed the wrong order of magnitude for the gate entirely |
| 8 | nothing outside the sheets track moves | **hit** — fidelity unchanged, all nine other projects unchanged |
| 9 | fidelity is exactly 30 of 550, 0 skipped | **hit exactly** |
| 10 | the new tests fail on the unfixed tree | **hit** — 12 fail; and the `XRay.Office.Text` project does not compile there at all, which is weaker evidence and is said as such |
| 11 | I will decline the DPCache sheet, and the strongest argument will be upstream's own | **hit** |
| 12 | the flat-ODF export shows `table:display="true"` | **hit** |

**Eleven of twelve.** The miss is #7 and it is the one I flagged as the least evidenced thing in the
file, which is the right place for a prediction to fail. Two predictions I did not make and should
have: that the round would find a defect no gate column can see (it found two), and that a probe
would have to be rebuilt twice before it measured anything (it did).

## Test counts

Every project run individually on the final tree. Build is **0 warnings, 0 errors**.

| project | result | note |
|---|---|---|
| `XRay.Office.Containers.Tests` | 109 passed, 0 failed, 0 skipped | |
| `XRay.Office.Core.Tests` | 337 passed, 0 failed, 0 skipped | |
| `XRay.Office.Markup.Tests` | 259 passed, 0 failed, 0 skipped | |
| `XRay.Office.OpenDocument.Tests` | 125 passed, 0 failed, 0 skipped | |
| `XRay.Office.Presentations.Tests` | 694 passed, 0 failed, 0 skipped | |
| `XRay.Office.Rendering.Tests` | 150 passed, 0 failed, 1 skipped | as before |
| `XRay.Office.Spreadsheets.Tests` | **847** passed, 0 failed, 0 skipped | was 832; +15 |
| `XRay.Office.Text.Tests` | **359** passed, 0 failed, 0 skipped | was 349; +10 |
| `XRay.Office.Vector.Tests` | 295 passed, 0 failed, 0 skipped | no phantom failure this round |
| `XRay.Office.WordProcessing.Tests` | 850 passed, 0 failed, 0 skipped | |
| `XRay.Office.Fidelity.Tests` | 520 passed, **30 failed**, 0 skipped, 550 total | the briefed baseline, exactly |

**25 new tests**, in three classes plus two rewritten cases:

* `XRay.Office.Text.Tests/OverflowingBlankBreakTests` — 10, including the Writer control and the
  single-straddling-blank bound.
* `XRay.Office.Spreadsheets.Tests/SheetUnnamedFontTests` — 8, including the `fonts[0]` control.
* `XRay.Office.Spreadsheets.Tests/SheetEditCellTests` — 6, including both controls.
* `SheetCentredWrapBlankTests` — one case rewritten and one added, against re-measured reference
  values.

**Verified to fail on the unfixed tree**, by copying the five changed source files aside,
`git checkout --`-ing them, rebuilding and re-running — never `git stash`, which is repository-global
here. Result: **12 failed of 847** in `XRay.Office.Spreadsheets.Tests`, covering all four changes
(6 font, 3 clipping, 1 height, 2 centred-wrap). `XRay.Office.Text.Tests` does not compile against the
unfixed tree, because the constructor option it exercises does not exist there — that is weaker
evidence than a failure and is not dressed up as one; the two `SheetCentredWrapBlankTests` failures
cover the same behaviour end to end and do fail properly.

## Reproducing this

```sh
export XRAY_OFFICE_CLI=…/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
export SOURCE_DATE_EPOCH=1700000000
bash dotnet/probes/sheets-rest-01/sweep.sh /c/sandbox/workdir/sample-files 'sheets/*' /abs/out 6 > sweep.log 2>&1
grep '^TOTAL' sweep.log      # TOTAL 171  MATCH 163  MISMATCH 8  REF-MISSING 0
```

The three probe generators are `mkspaceprobe.py`, `mkfontprobe.py` and `mkclipprobe.py`; each
prints its cases and writes a workbook, and each is read by `soffice --convert-to fods` for row
heights or `--convert-to pdf` for faces and text layers. The three corpus fixtures under
`dotnet/tests/corpus/features/` are the output of the last two.
