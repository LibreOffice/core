# The `.ods` header band: what Calc's rule turned out to be, and what the brief had wrong

## Environment

    ours   = XRay.Office.Cli at agent/odshdr, base 0e54dba0a
    ref    = /opt/libreoffice26.2/program/soffice, LibreOffice 26.2.4.2 -- the build that wrote
             /home/user/corpus-odf, so both halves of every comparison are one build's artefacts
    fonts  = system fontconfig; the tarball's four confound families are aside in
             .duplicates-aside/, LiberationSansNarrow included
    rule   = batch-check.sh of 2026-09-05: pages exact, alphanumeric characters within max(2%, 15)
    load   = a whole-ODF-corpus sweep was running in the main checkout throughout, at three
             workers, against /usr/bin/soffice (24.2.7.2). Ours used two. Load average 9-16 on
             four cores, which matters for §3 and for nothing else.
    date   = 2026-09-07

## 1. The rule, established over 73 authored probes rather than fitted to one number

Calc's band is `max(nManHeight, nMaxHeight + nDistance)`:

    rParam.nHeight = nMaxHeight + rParam.nDistance;      sc/source/ui/view/printfun.cxx:838
    if (rParam.nHeight < rParam.nManHeight)                                          :848
        rParam.nHeight = rParam.nManHeight;                                          :849

with, all verified in this checkout rather than copied from the brief:

- `nManHeight` is `ATTR_PAGE_SIZE`'s height — `rParam.nHeight = pHFSet->Get(ATTR_PAGE_SIZE)...`
  at `lcl_FillHFParam`, `printfun.cxx`:666, kept as `rParam.nManHeight = rParam.nHeight` at :683;
- `nDistance` is `ATTR_ULSPACE`, the header's **lower** and the footer's **upper**
  (`printfun.cxx`:898 and :913) — which is ODF's `fo:margin-bottom` on a `style:header-style`
  and `fo:margin-top` on a `style:footer-style`;
- `nMaxHeight` is the EditEngine's laid-out height of the band's text, maximised over the three
  areas of each of the three page variants (`printfun.cxx`:817-836);
- and none of it runs at all unless the band is *dynamic*: `if (!(rParam.bEnable &&
  rParam.bDynamic)) return;` (`printfun.cxx`:793), with `bDynamic` from `ATTR_PAGE_DYNAMIC`
  (:663), whose pool default is **true** (`sc/source/core/data/docpool.cxx`:166).

ODF states every one of those terms, and the `bDynamic` half is the one nothing in this tree
knew about. `fo:min-height` and `svg:height` are **two special items of one property**:
`XMLPageMasterPropSetMapper`'s `finished` emplaces `HeaderIsDynamicHeight` false beside a
`svg:height` and true beside a `fo:min-height`
(`xmloff/source/style/PageMasterImportPropMapper.cxx`:324-330); that property is
`ATTR_PAGE_DYNAMIC` (`sc/source/ui/unoobj/styleuno.cxx`:341-343). So a band written with
`svg:height` prints at exactly its declared height and one written with `fo:min-height` grows.

`gen.py` authors 51 probes and `gen2.py` 22 more — one flat `.ods` each, identical but for the
band — and `measure.py`/`measure2.py` render them through 26.2.4.2 and read the band as the
**shift of the first printed cell row** against an otherwise identical probe with no band at
all. That shift is the one quantity that needs no assumption about where a row's text sits
inside its row, which is why it is the instrument: reading the band off the header's own ink
instead is wrong by a point, because the reference puts the first row's *text* 0.97 pt above
the body's top edge and this tree puts it 0.02 pt below (see §5).

| family | probes | what it settles |
|---|---:|---|
| `gap_*` | 6 | the band follows the gap point for point once `text + gap` passes the declared height, and not at all before it |
| `min_*` | 7 | and follows the declared height point for point while that is the larger |
| `fix_*` | 3 | `svg:height` does not grow: 0.2 cm stays 0.2 cm against a 0.25 cm gap and an 11.16 pt line, where `fo:min-height` would make it 18.2 |
| `font_*` | 30 | five faces × six sizes: the text term is the band's own line height at every size above the floor |
| `lines_*` | 4 | and the sum of its lines |
| `d*h*`, `bare_*`, `mixed` | 22 | the floor is one line of the **default cell font**, and it is the band's floor and not each line's |

**The floor is the finding the brief did not have, and it is what makes the rule usable.** A
6 pt header in a 20 pt workbook takes the 20 pt line; in a 14 pt workbook the 14 pt line; in a
6 pt workbook its own. Two 6 pt lines in a 20 pt workbook still take **one** 20 pt line, which
is what separates "each line has a floor" from "the band has one". The mechanism is in the same
function: `UpdateHFHeight` measures all nine areas and `TextHeight` returns zero only for a
**null** `EditTextObject` (`printfun.cxx`:777-785), while an ODF band's unused left and right
areas are empty objects of one empty paragraph each — set in the workbook's default cell font,
because `MakeEditEngine` fills the band's EditEngine defaults from `getDefaultCellAttribute`
(`printfun.cxx`:1765-1772). So a header shorter than a plain cell of its own workbook is sized
by the two empty areas beside it.

**And the text is centred in `nHeight - nDistance` whichever term won.** `PrintHF` computes
`nDif = paperHeight - textHeight` and applies half of it when positive
(`printfun.cxx`:1876-1912); the seven `min_*` probes put the reference's header ink at
`top + (declared - gap - text) / 2` to 0.01 pt. That is why `SheetPrintSetup.HeaderIsDynamic`
is deliberately left false for ODF: what that flag does in this tree is clamp the text
rectangle to the text, which is a shortcut for "a dynamic band has `nDif == 0`" — true only
while the text term wins. With the band computed properly, `HeaderHeight - HeaderGap` already
*is* `PrintHF`'s paper height.

### Agreement after the change

51 of 51 in the first family and 22 of 22 in the second, worst error **0.227 pt** — a five-line
band, so 0.045 pt a line, which is the whole-twip rounding Calc does and we do not. Before the
change the same probes were out by up to **18.11 pt** (`font_Carlito_24`) and the whole
`font_*` family answered one number for all thirty cells.

## 2. What the brief got wrong

Five things, in ascending order of how much they cost.

**(0) The reach figure does not reproduce, and it is larger.** The brief's census — *"58 of 307
`.ods` declare a band smaller than one line plus its gap"* — comes out at **79** here with the
same 11.5 pt line, over header *and* footer styles (`hfcensus.py`). The assumption-free half of
it is the useful one: **13** of the 307 declare a band whose *gap alone* exceeds its declared
height, and such a band must grow whatever its text is.

**(a) The seat is the reader, not the layout.** The brief's reason — *"it wants the band's own
text height, which is `SheetPageDecoration.TextHeight`'s question and not the reader's"* —
is refuted by the tree it describes: `SheetBandHeight` lives in `Layout/`, is called from
`XlsxPrintSetup`, `XlsPrintSetup` and `XlsbPrintSetup`, and has called
`SheetBandText.LineHeightAt` since round 56. The three Excel readers have measured their bands'
text for eight rounds; the ODF reader simply never asked. The band is now computed in
`OdsPrintSetup` through `SheetBandHeight.Dynamic`, and the four measuring helpers the drawing
path already had (`PartHeight`, `LineHeight`, `SizeOf`, `FaceOf`) moved from
`SheetPageDecoration` into `SheetBandHeight` so that the height and the drawing answer from one
seat rather than two.

**(b) "The number to fit against is one measurement" was the reason to leave it, and it was
avoidable.** The brief had `activespecs`' 10.70 pt and nothing else. Authoring probes cost
about forty minutes and produced 73 numbers across five faces, six sizes, seven declared
heights, six gaps and four line counts — and the 10.70 was *itself* wrong by a point, because
it was derived from the first row's drawn text rather than from the body's top edge. The band
on `activespecs` is **37.66 pt**, not 36.69, and its text term is **11.67 pt**, not 10.70 —
which is DejaVu Sans at 10 pt, the face `MS Sans Serif` resolves to. Fitting the code to the
briefed number would have been a point short on every page.

**(c) The band's own text was not readable at all, and no amount of layout work would have
fixed that.** `OdsCellDecoration.ReadBand` carried *"a text:span carries formatting and nothing
else here, so its children are taken and it is not"* — so every ODF band was sized **and drawn**
in one size and one face, the workbook's default, whatever the file stated. The thirty `font_*`
probes are that defect: the reference's band grows from 39.49 pt at 6 pt to 57.66 pt at
Carlito 24 and ours answered 39.54 at all six sizes of all five faces. A round that had put the
rule in the layout and stopped would have measured the same wrong text.

**(d) `SheetPrintSetup.BandFont` was null for every ODF spreadsheet**, so a band naming no face
was drawn in a fixed ten-point Liberation Sans rather than in the workbook's own default cell
font — the exact defect round 56 fixed for the other three readers, still open here.
`OdsCellFormats.DefaultFont` resolves the `Default` cell style through the same reader a cell
goes through, so the two spellings of a face are settled by *level* before they are settled by
spelling.

## 4. `SIL_TDB648.ods` is not a cell-overflow document, and the census says so in one line

The brief: *"our long cell strings are broken and clipped at their own column where the
reference runs them across the empty cells beside them"*, 61 pages against 88.

**Page 1 is the same document on both sides.** Every one of its fourteen text runs agrees to a
tenth of a point, and the long prose cells run to x = 598 pt in *both* renderings on a sheet
whose first column ends far short of that — so this tree already overflows a string into the
empty cells beside it, and the reference already does the same. `firstdiff.py` puts the first
page whose character count differs at **page 6**, where the reference has **no text at all**
and 1399 non-white pixels at 40 dpi, and we have 158 characters.

Counted over the whole document: the reference has **15 text-free pages of 88** and we have
**5 of 60**, and the reference's are picture pages.

The cause is one attribute. `SIL_TDB648.ods` holds **74 `draw:frame`**, of which **72 state
`draw:transform` and no `svg:x`** — the shape LibreOffice writes for a turned or skewed
picture. `OdsDrawings` reads no transform at all, so 72 of its 74 pictures do not exist for us,
and that costs three things at once, which is exactly why it reads as a text defect:

- the pictures' own ink;
- the print area, because `ScDocument::GetPrintArea` unions the drawing layer's bounding box
  into the cells' extent (`sc/source/core/data/documen2.cxx`:644-666) — `SheetDrawingArea`
  already implements that union and is being handed 2 drawings instead of 74;
- and the empty-page test, because a page whose only content is a picture must not be dropped —
  `SheetEmptyPages` already implements that overlap test and sees the same 2.

**Reach is five documents and 81 frames of 504** (`framecensus.py` over the 307 converted
`.ods`), and 72 of the 81 are in this one file. So it is a narrow defect with one large witness
rather than the class the brief took it for — which is also why it should not be fixed on the
same afternoon as the band, without its own measurement: the four other documents each hold
one to four such frames, and moving them moves ink on documents that pass today.

**Left, with the seat named.** `XRay.Office.WordProcessing/OpenDocument/OdfFrames.cs`:211-243
already takes the `translate(...)` out of a `draw:transform` for a Writer frame, and
`OdpSlideLayout`:747-843 parses the whole matrix; the sheet reader has neither.

## 5. Two smaller things this measurement turned up, both left

**A cell's first printed row sits a point lower in our rendering than in the reference's.** On
the band probes the reference puts the first row's *text* 0.97 pt **above** the body's top edge
and this tree puts it 0.02 pt below — measured as the difference between `band-none`'s first-row
text top and the top margin, which is 56.693 pt of margin against a 58.693 pt text top on the
reference and against our own. It is constant across every probe in both families, it cancels
out of the band measurement (which is why this round could use the shift and ignore it), and it
is worth about a point of body height per page. It is also the reason the brief's `activespecs`
figure was a point out.

**A `.ods` band's colour and decoration are still dropped.** `ReadBand` now keeps a
`text:span`'s size, family, weight and slope, because those are the four
`SheetHeaderSegment` models and the three the band's *height* turns on. A span's colour and
underline are still discarded — as they are for all three formats.
## 3. `ours-failed` is a wall-clock class, not a crash class, and the 22 does not reproduce

The brief: *"the 22 `.ods` rows that fail as `ours-failed` — we produce no output at all for
them … a crash or a timeout is often one bug across many documents and is the cheapest win on
the board."*

**Measured at this round's base, a full 307-document `.ods` sweep gives one** — the same figure
after the change. The 22 come from `/home/user/gate-odf-rows.tsv`, a whole-ODF-corpus gate
taken 2026-09-06 22:49 at three workers, and the previous round had already re-run exactly
those 22 one at a time (`/home/user/odf-round-work/rerun-before.tsv`, 00:04 the same night):
**15 of the 22 rendered successfully**, 4 reported *"Out of memory."*, 2 timed out at 300 s and
2 were killed. Its own rescore of all 22 (`ods-after.tsv` in the same directory) produced output
for **all twenty-two**.

Re-measured here at `0e54dba0a`:

| document | prior round, alone | this base, alone |
|---|---|---|
| `STC_WebList.ods` | rc −6, *Out of memory.* | **rc 0**, 200.6 s, peak RSS 687 MB, 4372 pages against the reference's 4372 |
| `034_Personal_net_worth_calculator…ods` | rc 0, 93 s | **2.5 s** |

So there is no crash to find. What there is is a **long tail of slow documents against
`batch-check.sh`'s 240-second `timeout`**, and how many of them cross it is a property of the
machine rather than of the tree: this round's sweep ran two workers beside another session's
three, at a load average of 9–16 on four cores. The one row that fails here is
`Global_Market_Forecast_2016-2035_Airbus_Data_Set.ods` at **240.2 s**, and the next four are
213.5, 159.5, 129.6 and 113.0 (the `seconds` column of `ods-after.tsv`). Nothing separates the
first from the second except the cut-off.

**Which is a real defect, but a different one, and it should be filed as performance.** The
useful figure for whoever takes it is the shape of the tail rather than a verdict count: eight
of 307 `.ods` take over 100 s, and the gate's own timeout is 240.

**And it changes what a stored `ours-failed` count means.** A verdict that depends on how many
workers were running is not comparable between sweeps, which is the `SWEEP_ONLY` lesson in a
second form: a banked gate's `ours-failed` rows are a claim about the machine at that hour.
Before working one, re-run the document alone.
## 6. What moved

| | base `0e54dba0a` | + the band | of |
|---|---:|---:|---:|
| `.ods` | **217** | **228** | 307 |

Twelve verdicts move, **eleven gain a match and none is lost**. Six were `pages`, two
`pages,words`, three `words`, and one goes `pages,words` → `pages`.

| document | before | after | reference |
|---|---:|---:|---:|
| `activespecs.ods` | 254 | **266** | 266 |
| `PA_Delaware.ods` | 118 | **122** | 122 |
| `fy2010-aip-grants.ods` | — | **100** | 100 |
| `fy2011-aip-grants.ods` | — | **93** | 93 |
| `fy20-may20-sep20.ods` | 95 | **96** | 96 |
| `npias_2009_appA.ods` | — | **73** | 73 |
| `certification-type-…-MAdB-Light-Prop-14-28112013.ods` | — | **343** | 343 |
| `6f9e605c-fded-11e3-bd0e-00144feab7de.ods` | — | **16** | 16 |
| `011_Contextures_chart_sample_599b4392.ods` | — | 4 | 4 |
| `4-12-Kings-Attachment-13-…-Security-Requirements.ods` | — | 6 | 6 |
| `cy10_primary_enplanements.ods` | — | 12 | 12 |
| `tk-syllabus-comparison-document-v5.ods` | `pages,words` | `pages` | 855 |

Five of the eight the brief named by sign — `activespecs`, `PA_Delaware`, `fy2010-aip-grants`,
`fy2011-aip-grants`, `fy20-may20-sep20` — are among them.

### The instrument, and the control that is not circular

Both numbers are scored against **one** reference half: the 307 reference PDFs rendered by
`batch-check.sh` at 26.2.4.2 for the base sweep. That reuse is sound because the diff is
confined to `dotnet/src` and cannot reach `soffice`; what makes it non-circular is that
`rescore.py` re-renders **every** row rather than a list, so no row is unchanged by
construction.

The control is a third full pass: **the base binary put back through `rescore.py`** against the
same bank answers **217 match**, exactly what `batch-check.sh` gave it. One row's sub-verdict
differs and it is worth recording, because it is an instrument fault rather than a tree one:
`batch-check.sh` scored `tk-syllabus-comparison-document-v5.ods` as `pages,words` with an
**empty** page count and zero characters, which is what a document that hit the 240 s `timeout`
*after* beginning to write its PDF looks like — the partial file exists, so `[ -f "$o" ]`
passes and the row is not `ours-failed`. **So a banked `ours-failed` count undercounts the
timeouts**, and a row with an empty page-count field is one of them.

### The original `.xlsx`/`.xls` track does not move

`samediff.py` renders all 307 `.xlsx`/`.xls`/`.xlsm` of `/home/user/sample-files/sheets` with
the base binary and with this one, under a fixed `SOURCE_DATE_EPOCH` so that a `&D` header
prints the same date on both legs, and compares the two PDFs byte for byte.

**307 documents, 0 moved, 0 without output** — every one of the 307 renders to the same bytes
before and after. `orig.log` in the probe directory.

That is the check the diff needs, because four of the five changed files are ODF-only and the
fifth — `SheetBandHeight` — is where the four measuring helpers moved to from
`SheetPageDecoration`. A move is behaviour-identical or it is not, and only rendering says
which.

## 7. What was deliberately left

- **`draw:transform` on a sheet's `draw:frame`** (§4). Five documents, 81 frames, one large
  witness. It wants its own measurement pass.
- **The one-point row-text offset** (§5). Constant, cancels out of everything measured here,
  and worth a point of body per page.
- **The performance tail** (§3). Eight of 307 `.ods` take over 100 s to render and the gate's
  timeout is 240. It is a real defect and it is not a correctness one.
- **A band's colour and decoration**, which no format's band models.
- **`SheetPrintSetup.HeaderIsDynamic` for ODF**, deliberately left false. §1 gives the
  measurement: with the band computed properly the flag's clamp is not wanted, and the seven
  `min_*` probes are what say so.
