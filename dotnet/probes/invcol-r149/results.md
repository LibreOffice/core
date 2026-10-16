# Round 149 — O100: both bands are one cause, and it is `style:table-centering`

**Measured against LibreOffice 26.2.4.2, build `0229ac93fcf0d7cbc6376066c6f35021cef002dc`**
(`/opt/libreoffice26.2/program/soffice`), with the tarball's duplicate, Latin Noto, Narrow and
Condensed faces moved aside (`.duplicates-aside`, `.noto-aside`, `.condensed-aside` all present at
the time of measurement). Our half is
`dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli`. **The parent session
rebuilt it three times during this round** — mtimes `21:12:06`, `21:41:14`, `21:58:14`, `22:04:30`,
all at 78256 bytes — so every comparison here spans a rebuild and every one was re-rendered and
checked byte for byte rather than assumed: the witness is md5-identical across its rebuild and the
162-document sweep is **162 of 162 identical** across the other three. §10 has the stamps and the
method; `render/mtime.log`, `sweep-all/cli-mtime.txt`, `recheck.txt`. Document:
`/home/user/corpus-odf/ods/b7fde7cdac59-084_Service_invoice_Use_this_template_c92b43dc.ods`,
26.2.4.2's own conversion of `sheets/chartset-010/xlsx/084_Service_invoice_…xlsx`.

This round builds nothing. Every claim is marked **[src]** (a reading of the
27.2.0.0.alpha0+ C++ checkout, which is *not* the reference binary's source) or **[bin]** (a
measurement of 26.2.4.2's own output).

## 0. Headline

**The `-38` band and the `-213` band are the same defect, and neither is a column.** They are the
two printed pages of one sheet, each displaced by the amount its own page would be displaced by
**horizontal print centring** — `style:table-centering="horizontal"` on the page layout that sheet's
master page names. The third page, the only one of the three whose page layout does *not* state
centring, is exact. Predicted from the file with **no free parameter**:

| pdf page | columns | width | predicted dx | measured dx | spans |
|---|---|---:|---:|---|---:|
| 1 | B C D E | 464.710 pt | **−37.645** | −37.73 .. −37.61 | 34 |
| 2 | F | 114.660 pt | **−212.670** | −212.68 .. −212.55 | 25 |
| 3 | *About sheet, `Mpm3`, no centring* | — | **0** | 0.00 .. 0.00 | 12 |

`centring-arith.py` reads the page width, the margins and the column widths out of the `.ods`,
splits the print range into printed pages, and prints the predicted offset beside the measured
band. The residual is 0.027 and 0.008 pt on the medians (§7).

**The seat is one misspelt word.** `OdsPrintSetup.cs`:106 asks the document for
`style:table-centring` — the project's British house spelling — where ODF's attribute is
`style:table-centering`. So `CentresHorizontally` and `CentresVertically` have been false for every
`.ods` ever read, and `SpreadsheetPages.BodyOrigin`, which implements the rule correctly and has
since round 69, has never once been told to apply it.

**Confirmed twice at 26.2.4.2 and reached over the corpus.** Removing the attribute makes the
reference draw the block where we draw it, to 0.03 and 0.05 pt; the same document read through the
OOXML reader, which spells it correctly, is exact on all three pages. **81 of the 307 converted
`.ods` state the attribute**, and of the 75 scoreable, **69 are displaced against a control-group
base rate of 10 of 71 — 59 against 1 at a 2 pt threshold** — with every sign accounted for and the
six non-movers predicted by the rule itself.

## 1. The instrument was sound

`probes/odfpad-r142/invoice-bands.py` reproduces exactly today — `[(-213, 25), (-38, 34), (0, 12)]`
— and, unlike round 146's `difflib` pairing and round 148's mixed-boundary census, it has **no
pairing artefact and no coverage bias**:

* **71 spans on each side and all 71 pair.** No key is unmatched in either direction, so the
  histogram is over the *whole* document, not a subset (`spans-ours.tsv`, `spans-ref.tsv`).
* **No key is a duplicate**, so the `zip` over each key's list never pairs two different spans —
  the failure mode that made round 146's ranking meaningless cannot occur here.
* The key includes `round(y0, 1)`, so all 71 pairing means **the vertical agrees to 0.1 pt on every
  span of the document**. That is itself a result: whatever this is, it is horizontal only, which
  is exactly what `table-centering="horizontal"` predicts.

What the histogram could not show, because it bands a scalar, is that each band is a **page**.
`dump-spans.py` emits page, y, x, width, font, size and text with nothing merged, and the band
structure falls out as page structure the moment you look at the column.

## 2. What the document is

```
style:page-layout "Mpm4"  fo:page-width 8.5in  fo:margin-left/right 0.5in
                          style:table-centering="horizontal"
  <- style:master-page "PageStyle_Invoice" <- table style "ta1" <- <table:table name="Invoice"
                                                        print-ranges="Invoice.B2:Invoice.F38">
style:page-layout "Mpm3"  fo:margin-left/right 0.7in      (no table-centering)
  <- "PageStyle_About" <- "ta2" <- <table:table name="About">
```

Printable width is `612 − 36 − 36 = 540`. The print range B..F is `579.370` pt wide, so it does not
fit and the printer breaks it after E: page 1 carries B C D E (464.710), page 2 carries F alone
(114.660). Centred, those sit `(540 − 464.710)/2 = 37.645` and `(540 − 114.660)/2 = 212.670` right
of the left margin. **That is the whole of both bands.** The `About` sheet is its own page style
with no centring and no overflow, and it is the page that matches exactly — a control inside the
same document, the same renderer and the same run.

So the register's *"the middle band is a whole-page constant of 37.7 pt this document has carried
throughout"* and *"the `-213` band is one column in the wrong place"* are one sentence: **neither
is a constant of the document and neither is a column.** The 37.7 pt is not a margin, an inset or a
text origin — it is half the slack left over on page 1, and the 213 pt is half the slack left over
on page 2. They differ only because the two pages hold different amounts of the sheet.

## 3. The cause, confirmed twice

### [bin] — 26.2.4.2 reproduces *our* output when the attribute is removed

`mutate.py` writes one-attribute variants of the `.ods`, touching `style:table-centering` on
`Mpm4` and nothing else; `variant-bands.py` reports the top-left corner of the drawn block on each
printed page. Rendered through 26.2.4.2 (`variants/ref/`):

| variant | page 1 x | page 2 x | page 3 x | page 1 y |
|---|---:|---:|---:|---:|
| `horizontal` (as found) | 83.79 | 250.55 | 51.39 | 39.58 |
| **attribute removed** | **46.15** | **37.93** | 51.39 | 39.58 |
| `vertical` | 46.15 | 37.93 | 51.39 | **47.07** |
| `both` | 83.79 | 250.55 | 51.39 | 47.07 |
| **ours, file as found** | **46.12** | **37.88** | 51.39 | 39.58 |

**With the attribute removed, 26.2.4.2 draws the block where we draw it — 46.15 against our 46.12
and 37.93 against our 37.88**, a residual of 0.03 and 0.05 pt, which is the advance-width noise.
So our renderer is behaving exactly as if the attribute were absent, on both pages, and the
attribute is the only thing that was changed. The `vertical` and `both` arms show the vertical half
is real, separate and worth 7.48 pt here, and that it moves `y` without touching `x`. The `About`
page never moves under any variant: it is a different page style and states no centring.

### [bin] — the same document read through the OOXML reader is exact

The corpus original, `sheets/chartset-010/xlsx/084_Service_invoice_Use_this_template_c92b43dc.xlsx`,
states `<printOptions horizontalCentered="1"/>` on the Invoice sheet and nothing on the About
sheet — the same two facts the converted `.ods` states as `style:table-centering="horizontal"` on
`Mpm4` and nothing on `Mpm3`. Rendered through the same binary, the same layout code and the same
page split (34 / 25 / 12 spans a page on both sides, as in the `.ods`):

| reader | page 1 dx | page 2 dx | page 3 dx |
|---|---:|---:|---:|
| **OOXML** (`XlsxPrintSetup`) | **0.00** | **0.01** | **0.00** |
| ODF (`OdsPrintSetup`) | −37.67 | −212.68 | 0.00 |

`invoice-bands.py` on the `.xlsx` pair reports **one band: `[(0, 70)]`**. So this is not a layout
defect, not a table-apportionment question and not a column at all: **the layout engine already
does print centring correctly, and only the ODF reader fails to tell it to.**

*(70 rather than 71 is the sixth confound in `dotnet/CLAUDE.md`, not a defect: the `TODAY()` cell
reads `1/22/2025` from the `.xlsx`'s cache and `9/17/2026` from the reference's recalculation. The
`.ods` matches 71 of 71 only because `/home/user/corpus-odf` was converted at 07:06 **today**, so
its cached value happens to be today's date. **This measurement will read 70 of 71 tomorrow**, and
the band structure is unaffected either way.)*

### [src] — the rule in the C++ tree

Read in the 27.2.0.0.alpha0+ checkout, which is not the reference binary's source; the `[bin]`
legs above stand on their own.

* `xmloff/source/style/PageMasterStyleMap.cxx`:97-98 maps `style:table-centering` to **two**
  properties, `CenterHorizontally` and `CenterVertically`, both `MID_FLAG_MERGE_ATTRIBUTE`.
* `xmloff/source/style/PageMasterPropHdl.cxx`:320-336 and :360-376 are the handlers: horizontal is
  true for `both` **or** `horizontal`, vertical for `both` **or** `vertical`. The attribute is a
  single token carrying two booleans, which is why `both` exists at all.
* `sc/source/ui/view/printfun.cxx`:873-874 reads them into `bCenterHor` / `bCenterVer`, and
  :2144-2191 applies them:
  `nLeftSpace += ( aPageRect.GetWidth() - nDataWidth ) / 2`, where `nDataWidth` is
  `GetColWidth` summed over **this printed page's own columns `nX1..nX2`**, plus the repeated
  columns, plus `PRINT_HEADER_WIDTH` if headings print, plus the border and shadow distances.
  Per printed page, which is precisely why the two pages of one sheet get two different offsets.
  The header and footer are printed **before** this, under the comment *"head and foot lines
  (without centering)"* (:2131), so furniture is not centred with the block.

## 4. The rule was implemented in round 69 and one of the three readers never delivered the flag

`probes/overflow-r69/results.md` is where print centring was worked: it found the unclamped
halving, cited `printfun.cxx`:2165 and :2188, fixed `SpreadsheetPages.BodyOrigin`'s positive-only
guard on `048_Expense_trends_budget`'s 167.3 pt, and censused **"73 of the corpus's 243 xlsx-family
workbooks state a centring flag; 170 sheets horizontal, 11 vertical"** (`results.md`:90, :335).

**Today's `census.py`, written without reference to that file, reproduces all three of those
numbers exactly** — 73 documents, 170 horizontal sheets, 11 vertical sheets. That is an independent
instrument agreeing with a stored figure from a different round against a different corpus
snapshot, which is worth recording in a project whose standing rule is that stored figures decay.

What round 69 could not see is that it was fixing the *consumer*. The OOXML, BIFF and XLSB readers
all set `CentresHorizontally`; the ODF reader asks the document for an attribute that does not
exist, so it has been sending `false` the whole time and the fix it was given has never once run
on a `.ods`. **A rule can be correct, cited, measured and reachable from three of four readers**,
and the fourth fails silently because a reader that finds no attribute is indistinguishable from a
document that states none. That is the same shape as the five ODF-namespace instances
`dotnet/CLAUDE.md` records, in a new disguise: **the namespace was right and the local name was
misspelt in the project's own house style**, so grepping the corpus for both namespaces — the
remedy that file prescribes — could not have found it. The check that does find it is grepping the
*reader* for the specification's spelling and getting nothing.

## 5. The C# seat, and the change

**`dotnet/src/XRay.Office.Spreadsheets/OpenDocument/OdsPrintSetup.cs`, line 106**, in the method that
folds a page layout into a `SheetPrintSetup`:

```csharp
string? centring = Get(page, OdfNamespaces.Style, "table-centring");
```

The ODF attribute is **`style:table-centering`**. `table-centring` is the project's British house
spelling — which is right for `CentresHorizontally`, `SheetHorizontalAlignment.Centre` and the
local `centring` — leaked into an attribute *name*, where the spelling is the specification's.
`Get` returns null for every document, so `CentresHorizontally` and `CentresVertically`
(`:151-152`) are false on every `.ods` ever read, and the two lines below them have never once
been true.

**The change is one word**: `"table-centring"` → `"table-centering"`. Nothing else is needed —
`SpreadsheetPages.BodyOrigin` (`src/XRay.Office.Spreadsheets/Layout/SpreadsheetPages.cs`:539-549)
already consumes both flags, already halves the remainder without clamping it, and already carries
`printfun.cxx`'s reasoning in its own remarks. **Not made; the parent makes the edit.**

`grep -rn 'table-centring\|table-centering' dotnet/src` returns exactly that one line: the correct
spelling appears nowhere in the tree, so there is no second reader to keep in step. Elsewhere the
ODF readers spell the *specification's* tokens correctly — `OdsCellFormats.cs`:698 and
`OdsShapeText.cs`:191,203 all match on `"center"` — so this is an isolated slip, not a convention.

## 6. Reach, censused — with the base rate beside every count

`census.py`. The unit is **a document whose rendered sheet names a page layout that states the
attribute**, not a document containing the string: LibreOffice writes page layouts for master
pages no table uses (`Default` and `Report` are in every converted `.ods`), so *stated anywhere*
and *reached by a table* are two numbers. Here they happen to agree, which is itself worth
recording — it means no converted `.ods` states this as boilerplate.

| column | base rate | states it | a table reaches it | values |
|---|---:|---:|---:|---|
| `/home/user/corpus-odf/ods` | **307** | **81** | **81** (236 tables) | `horizontal` 224, `both` 12 |
| `/home/user/corpus-odf/odt` | **337** | **0** | **0** | — |
| `/home/user/corpus-odf/odp` | **directory absent** | 0 | 0 | *0 occurrences **and** 0 base-rate documents — the tell* |
| `/home/user/corpus-odf/rtf` | **337** | *not a zip; instrument does not apply* | | |

So **81 of 307** converted `.ods`, a quarter of the column, and **12 of the 81 state `both`**, which
is the vertical half as well. The `.odt` zero is a real zero, not an absent-column zero: the base
rate is 337 documents and `style:table-centering` is a Calc page property that Writer has no
analogue for. **No claim is made about `.odp`** — there is no such column here.

*Three corrections to the brief's own framing of the corpus, each checked today.* `MANIFEST.tsv`
holds **946 rows, words 337 / slides 302 / sheets 307**, not 947 / 338 — one words row has gone
since `dotnet/CLAUDE.md` was written, so 947 is a decayed figure and 946 is the authority.
`/home/user/corpus-odf` **does** have an `rtf` column, 337 files; only `odp` is absent. And the
`ods` column is 307 files in 307 distinct inodes, so the case-alias inflation does not reach it.

For the format the reader already gets right, as the comparison: of the **243** `xlsx`/`xlsm` rows
in `MANIFEST.tsv`, **73 documents (170 sheets)** state `printOptions/@horizontalCentered` and **9
(11 sheets)** state `verticalCentered`. The `.ods` figure is larger because that column also carries
26.2.4.2's conversion of the 64 `.xls`.

## 7. The numbers, to three places

| | predicted from the file | measured median | n | within-page sd |
|---|---:|---:|---:|---:|
| page 1 (B C D E) | −37.645 | **−37.672** | 34 | 0.021 |
| page 2 (F) | −212.670 | **−212.678** | 25 | 0.031 |
| page 3 (no centring) | 0 | **+0.000** | 12 | 0.000 |

Page 3 is the control on the whole instrument: its spans are **0.492 pt wider on average than
26.2.4.2's and up to 0.699 pt wider**, and its `dx` is nevertheless 0.000 with a standard deviation
of 0.000 — so on this document a span's left edge does not move with its width, and the residuals
on pages 1 and 2 are **not** advance-width noise. They are 0.027 and 0.008 pt. A plausible
source is that Calc halves in whole **twips** (`nLeftSpace += (aPageRect.GetWidth() − nDataWidth)/2`
is integer arithmetic on `tools::Long` document twips, so the offset is truncated to 0.05 pt) —
**not established**, and at 0.03 pt it is below anything this project acts on.

## 8. What the rule *paints*, measured, with a control group

A census counts documents that state the attribute; it does not say what the attribute moves.
`reach-sweep.sh` renders each document both ways and `reach-measure.py` reports, per printed page,
the **median** of the paired spans' `x` difference, calling a page *displaced* above a threshold.

Two groups of 81. **States**: all 81 converted `.ods` whose rendered sheet reaches a
`style:table-centering`. **Control**: 81 sampled with `random.seed(149)` from the 226 that state
none — same size, same column, same two binaries, same run. The control is what makes the
treatment figure mean anything: this corpus has plenty of other reasons for a left edge to be
wrong, and the base rate is how many.

| | states it | control |
|---|---:|---:|
| listed | 81 | 81 |
| scored (spans paired) | 75 | 71 |
| **at least one page displaced > 0.5 pt** | **69 (92%)** | **10 (14%)** |
| **> 2 pt** | **59 (79%)** | **1 (1.4%)** |
| **> 10 pt** | **25 (33%)** | **1 (1.4%)** |
| median of each document's worst \|dx\| | **5.53 pt** | 0.99 pt |
| max | **311.90 pt** | 11.02 pt |

Nine of the control's ten movers sit between 0.82 and 1.64 pt — just over the 0.5 pt threshold —
so **at any threshold this project would act on, the base rate is one document in 71.**

### The three residual groups are each explained, and two of them confirm the rule

**Six of the 75 states-group documents are not displaced at all**, and their worst \|dx\| is
0.00, 0.00, 0.02, 0.03, 0.08 and 0.12 pt — exact, not merely small. They are
`057_Simple_balance_sheet`, `073_Graph_paper`, `034_Personal_net_worth_calculator`,
`016_Free_Organizational_Chart_Template`, `090_Weekly_time_sheet` and `Hazard Analysis Template`.
**That is the rule predicting its own absence**: centring moves nothing when the printed block
fills the printable width, and a sheet of graph paper is the limiting case. A rule that fired on
all 75 would be the suspicious result.

**Six are `unpaired`** — our rendering and the reference share no `(page, y, text)` key at all, so
the instrument declines to score them rather than scoring them wrongly. They are
`058_Social_media_engagement_data`, `036_Simple_to-do_list`, `040_Blood_pressure_tracker`,
`045_Check_register_with_chart`, `063_Sales_pipeline` and
`2012-GA-Survey-Chapter-6-Tables-16Dec2013-V2`. **No claim is made about them in either
direction**; five of the six are chart-bearing templates, where a paginated or turned-label
difference would break the key. Settling them needs a matcher that does not key on exact `y`.

**Four are displaced in BOTH directions across their pages, and that is the unclamped halving.**
`printfun.cxx`:2165 halves the remainder whatever its sign, so a block **wider** than the page is
centred off both edges and the reference draws it to the *left* of where a left-flush renderer
puts it — the opposite sign. `048_Expense_trends_budget` is the witness and it is decisive:

```
page  1: median dx  +167.02   <- the `tips` sheet, ONE column wider than the page
page  2: median dx    -1.58
page  3..14:          +0.02
```

**Round 69 measured that exact page of that exact workbook at −167.06 to −167.42 pt** — *"every
line of page 1 sits 167.3 pt left of ours in 26.2.4.2"*, `probes/overflow-r69/results.md`:32-38 —
diagnosed it as the missing clamp, and fixed it. It measured the `.xlsx`. **This is the same
workbook's converted `.ods`, still showing the same magnitude with the sign inverted by the
direction of the subtraction, because the ODF reader never delivers the flag.** An expected value
written down by a different round, a month ago, from a different file format, reproduced to
0.3 pt. The other three mixed-sign documents — `NPIAS_App_A` (235.20), `TK-Syllabus-Comparison-Document-v2` (171.81) and `fy20-may20-sep20` (12.30) — are the same shape: several sheets, some
narrower than the page and some wider.

So of the 75 scored: **69 displaced, 6 correctly not displaced, and every one of the 69 has its
sign accounted for** — 65 leftward throughout, 4 mixed because they contain both the under-wide
and the over-wide case.

`TK-Syllabus-Comparison-Document-v2` is worth flagging separately: it is the head of round 94's
ink ranking (`probes/sheet-ink-r94/`), carried there at 205.57 % summed unsigned ink after the
conditional-format work, with the residual described as *"a row-height question"*. It states
`style:table-centering` and displaces by up to 171.81 pt. **Part of that residual is this**, and a
round sent after its row heights should re-measure after the one-word change.

## 9. Looking at the page — and it is my own reading, twice contaminated

**There is no uncontaminated reader in this container and I checked rather than guessed.** This
agent has no `Task`/subagent tool; `mcp__Claude_Code_Remote__create_session` spawns a sibling in
its own container which cannot open `/home/user/...` and has no channel back. `page-vision`'s own
warning says six rounds have now reached that section and reported it missing; this is the seventh
and the answer has not changed. So the readings below are mine, and they are contaminated **twice**
— once because no blind reader saw the page first, and once because **I did the arithmetic before
I looked**, which is the opposite of the order the skill asks for. Nothing in §0-§9 depends on
them; every claim there is arithmetic over span coordinates and file attributes.

`pairs/invoice-p1.png` and `pairs/invoice-p2.png`, composed at 150 dpi and shown at 117 dpi
(2000 px budget, `layout=side`). What they show, and what they do not:

* **Both pages are otherwise identical.** Same content, same rows, same table rules, same wrap,
  same colours, same blank rows — the block moves as one rigid object and nothing inside it moves
  relative to anything else. That is direction and kind, and it is what rules out a merge, a
  column width, a table apportionment and a per-cell error: all four would deform the block.
* **The two pages look like completely different defects and that is the whole trap.** On page 1
  the offset is 37.6 pt on a 612 pt page — a 6% inset that reads as *very slightly narrow margins*
  and would never start an investigation. On page 2 the same rule is 212.7 pt and reads as *a
  column dumped on the left edge*. One cause, two appearances, and they were seated as two
  questions.
* **What the image cannot decide** and the arithmetic had to: whether the reference is inset or we
  are outset, whether the amount is a margin, a table origin or a centring remainder, and whether
  the two pages share a cause. The image cannot see the page-2 offset and the page-1 offset as the
  same number, because they are not the same number.

Anyone with a reader should hand `pairs/invoice-p2.png` to it cold. The prediction this round would
make is *"the right-hand rendering places the whole block further right; nothing within the block
differs"* — and a blind reading that says anything else is worth more than this section.

## 10. Instrument notes — three, and the first cost this round an hour

**A relative `-env:UserInstallation` does not fail, it HANGS.** `dotnet/CLAUDE.md` and
`batch-check.sh`:56 both say *absolute, always*, and the reason they give is not the reason that
matters. Measured here: with `-env:UserInstallation="file://sweep-all/prof0"` two workers sat
**167 seconds on their first document each, wrote nothing, printed nothing and never timed out** —
the `timeout -k 30 600` had not fired, so the run looked merely slow. With the same script and
`file://$(cd "$2" && pwd)/prof0` the same two workers did **44 documents in 90 seconds**. A sweep
that produces no rows and no error is this before it is anything else.

**Three rebuilds landed in the middle of this round and every one was measured rather than
assumed.** The parent session was building and running its suite in this same tree throughout, and
`XRay.Office.Cli` changed mtime at **21:41:14**, **21:58:14** and **22:04:30**, always at 78256
bytes. `dotnet/CLAUDE.md`'s rule is that a run spanning a rebuild is void — and it is void *unless
the rebuild is shown not to have moved these renderings*, which is one cheap measurement rather
than a judgement.

* The witness pair in §0-§7 spans `21:12:06 → 21:41:14`. Re-rendered afterwards, the invoice's
  `.ods` is **md5-identical**, `6e01f20316185dfa11a9f1a257270548` both times.
* The 162-document sweep spans `21:41:14 → 21:58:14`. `recheck-ours.sh` re-rendered **our whole
  half** with the binary as it then stood, itself finishing after `22:04:30`, so the comparison
  straddles all three rebuilds: **162 of 162 byte-identical, 0 differ** (`recheck.txt`).

So no build in this window moved a `.ods` rendering, and the sweep stands. This is *not* a general
licence — it is a statement about these 162 documents against these four binaries. **Record both
mtimes and re-render; do not reason about it.**

**`odfpad-r142/invoice-bands.py` is sound and this round found no artefact in it.** Round 146's
`difflib` pairing and round 148's mixed-boundary census were both instrument failures; this is not
one. What it *is* is a scalar histogram of something that turns out to have page structure, so it
showed three bands where the useful statement is three **pages** — the fix is not a different
pairing, it is printing the page column, which `dump-spans.py` does.

## 11. What I could NOT establish

1. **Whether every one of the 69 displacements is *this* rule, as opposed to this rule plus
   others.** §8 separates the groups at 79% against 1.4%, accounts for every sign, and predicts its
   own six non-movers; two documents' magnitudes are pinned exactly, the witness from its own file
   (§0) and `048_Expense_trends_budget` against a figure round 69 banked from the `.xlsx` (§8). What
   is *not* done is predicting each of the other 67 magnitudes from its own file, which needs a
   faithful model of Calc's page splitting under `style:scale-to`, `style:scale-to-pages`, repeat
   columns and multi-sheet print ranges — the layout engine, not a probe. **The measurement that
   would settle it** is the project's own idiom (`probes/odt-split-r82/unsplit.py`): render each of
   the 81 through 26.2.4.2 **twice**, as found and with `style:table-centering` stripped, and check
   that the reference's own two renderings differ by exactly the offset our rendering sits at. It
   is entirely on the reference side, needs no build, and costs 81 further reference renders — which
   this round did not have the CPU for beside the parent's suite.

   *And 6 of the 75 could not be scored at all* (§8, `unpaired`). A matcher that does not key on an
   exact `y` would score them; this round deliberately did not write one, because changing the
   pairing rule mid-round is how round 146's ranking happened.

2. **Whether the vertical half is right once the flag arrives.** No document in this round's
   measurement exercises it as the *whole* of a difference — the witness states `horizontal`, and
   the 12 documents stating `both` were not separated out. 26.2.4.2 moves the witness down by
   **7.48 pt** under a synthetic `vertical` (§4), so the arm is real; whether
   `SpreadsheetPages.BodyOrigin`'s `Rows(y)` sum reproduces
   `printfun.cxx`:2174-2191's `nDataHeight` — which adds `PRINT_HEADER_HEIGHT`, the border
   distances and the shadow space — is **untested**. **The measurement**: the same four-variant
   mutation of §4 against this tree after the fix, which is four renders.

3. **The 0.027 pt residual** between the prediction and the measured median on page 1. Twip
   truncation in `nLeftSpace` is a candidate and is not established.

4. **Whether anything regresses.** I did not build and could not run the suite, so I cannot say
   the one-word change is free. Two things reduce the risk and neither is proof: `grep` finds no
   test anywhere in `dotnet/tests` naming `table-centring`, `table-centering` or an ODS centring
   assertion, so nothing pins the current behaviour; and `Extent(Columns(x).Select(c => c.Width))`
   does not depend on `x` — `Columns(left)` takes its column range from `placement`, not from
   `left` — so there is no fixed point to solve and the shift cannot change which columns are on
   the page. **The measurement**: build, run `XRay.Office.Spreadsheets.Tests` and
   `XRay.Office.Fidelity.Tests`, and re-run the `.ods` column of the gate.

5. **Whether O100's `-213` was ever "one column".** It was not, on today's rendering. Whether it
   read that way when the register entry was written — before round 142 or after — I cannot check:
   `probes/odfpad-r142/` banks md5 fingerprints of the renderings, not the renderings, so there is
   nothing to re-measure the old state against. The band counts reproduce exactly
   (`-213×25, -38×34, 0×12`), so the *data* has not decayed; only the description was never checked
   against the page column.

## 12. What the parent should change

Neither edit is made here.

**`dotnet/src/XRay.Office.Spreadsheets/OpenDocument/OdsPrintSetup.cs`:106** —
`"table-centring"` → `"table-centering"`. One word. Worth a comment saying why the spelling breaks
the file's own convention, because the next reader will "fix" it back.

**`dotnet/probes/OPEN-ISSUES.md`, the `| O100 |` row** — the seat as stated is wrong in both
halves and should be replaced rather than amended. Suggested text:

> **O100 — `084_Service_invoice`'s two bands are one defect and it is `style:table-centering`.**
> Not a column, not a merge, not a width, and not a whole-page constant: they are the two printed
> pages of one sheet, each displaced by the amount horizontal print centring would move it.
> Predicted from the file with no free parameter — page 1 carries columns B C D E (464.710 pt) in
> a 540 pt printable width, so −37.645 against a measured −37.672; page 2 carries F alone
> (114.660 pt), so −212.670 against −212.678; page 3 is the `About` sheet, whose page layout states
> no centring, and is exact. Confirmed twice at 26.2.4.2: removing the attribute makes the
> reference draw the block where we draw it (46.15/37.93 against our 46.12/37.88), and the same
> document read through the OOXML reader, which spells the attribute correctly, is 0.00/0.01/0.00.
> **Seat: `OdsPrintSetup.cs`:106 asks for `style:table-centring`** — the project's British house
> spelling leaked into an ODF attribute name — so `CentresHorizontally` and `CentresVertically`
> have never been true for any `.ods`. `SpreadsheetPages.BodyOrigin` already implements
> `printfun.cxx`:2144-2191 including the unclamped halving and the heading strip.
> `probes/invcol-r149/results.md`.

**`dotnet/probes/PROVENANCE.tsv`** — a row for `probes/invcol-r149/`, by hand
(`provenance-index.py` must not be run in this container). Era `current`, LibreOffice 26.2.4.2
`0229ac93`, tarball Latin duplicates / Noto / Condensed moved aside.

**`dotnet/CLAUDE.md`** — two paragraphs are worth carrying, and both are general rather than about
this document. The namespace rule in the ODF section has five instances of *"LibreOffice writes an
attribute in a namespace the specification does not"*; this is the sixth disguise and a different
one — **the namespace was right and the local name was misspelt in the reader's own house style**,
which no amount of grepping the corpus for both namespaces can find. And the hang: *a relative
`-env:UserInstallation` does not fail, it hangs past its own `timeout`*, measured at 167 s with
nothing written.

## 13. Files

| file | what it is |
|---|---|
| `render/` | the witness pair, `render/mtime.log` (six CLI stamps), and `render/ours-recheck` |
| `dump-spans.py`, `spans-ours.tsv`, `spans-ref.tsv` | every span, nothing merged or paired |
| `centring-arith.py` | the prediction from the `.ods`, beside the measured band |
| `mutate.py`, `variants/` | the four one-attribute variants and 26.2.4.2's renderings of them |
| `variant-bands.py` | the corner of the drawn block per variant, per page |
| `xlsx/` | the same document through the OOXML reader, both ways |
| `census.py`, `census-ods-hits.tsv` | the reach census, with the base rate beside every count |
| `reach-sweep.sh`, `list-states.txt`, `list-control.txt`, `list-all.txt` | the two-group sweep |
| `reach-measure.py`, `reach.tsv` | per-document displaced-page counts, both groups |
| `recheck-ours.sh`, `recheck.txt` | our half re-rendered after the rebuilds; 162 of 162 identical |
| `sweep-all/fingerprints-*.txt` | md5 of all 486 sweep renderings, 162 rows each |
| `sweep-all/cli-mtime.txt`, `recheck-mtime.txt`, `ref-version.txt` | the binaries each leg ran |
| `reach-summary.txt` | `reach-measure.py`'s output, as printed |
| `pairs/invoice-p1.png`, `pairs/invoice-p2.png` | the composed pairs, 117 dpi effective |

**Provenance**, because no TSV here records it and the project's census says 3 of 256 do:
measured 2026-09-17 in `/home/user/libreoffice-core`, reference
`/opt/libreoffice26.2/program/soffice` = LibreOffice 26.2.4.2 `0229ac93fcf0d7cbc6376066c6f35021cef002dc`,
with `.duplicates-aside`, `.noto-aside` and `.condensed-aside` all present in the tarball's font
directory; our half `XRay.Office.Cli` at four mtimes — `21:12:06`, `21:41:14`, `21:58:14`, `22:04:30`, all
78256 bytes — recorded in `render/mtime.log`, `sweep-all/cli-mtime.txt` and `recheck-mtime.txt`,
with `recheck.txt` showing the renderings unmoved across them; corpus `/home/user/sample-files` at `MANIFEST.tsv` = 946
rows; converted ODF `/home/user/corpus-odf` (`ods` 307, `odt` 337, `rtf` 337, no `odp`).

## 14. What is banked here and what is not

The 486 PDFs this round rendered are **not** kept — 232 MB is not data beside a write-up. What is
kept is the md5 of every one of them (`sweep-all/fingerprints-{ours,ref,ours-recheck}.txt`, 162
rows each), the per-document scores (`reach.tsv`), the two binaries' stamps
(`sweep-all/cli-mtime.txt`, `recheck-mtime.txt`, `ref-version.txt`) and the scripts, so the sweep
is reproducible as `./reach-sweep.sh list-all.txt <dir>` and checkable against the fingerprints.
The witness renderings are small and **are** kept, in `render/`, `variants/` and `xlsx/`.

Per `dotnet/CLAUDE.md`'s own census — 3 of 256 stored TSVs record the environment they were
measured in — the provenance is in §13 and in this file rather than only in the prose, and
`PROVENANCE.tsv` needs a hand-added row (§12).
