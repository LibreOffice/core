# The `.rtf` column, and four gaps between the RTF reader and the three that were already right

## Environment

    ours   = XRay.Office.Cli @ 0c645ceaf (base) and @ 0c645ceaf + this round, Release
    ref    = /opt/libreoffice26.2/program/soffice, LibreOffice 26.2.4.2 (TDF tarball).
             NOT /usr/bin/soffice, which is 24.2.7.2 and cannot reproduce the bank.
    fonts  = system fontconfig, all four tarball confounds aside
    rule   = batch-check.sh of 2026-09-05: page count, then alphanumeric characters within
             max(2%, 15), then unembedded fonts
    corpus = /home/user/corpus-odf (the .rtf column, 338 documents, 328 of them banked)
             and /home/user/sample-files/words (the original words track, 338 documents)

The reference half was **not** re-rendered: `score.py` joins the columns banked in
`gate-odf-rows.tsv`, which is sound because the whole of this round is confined to
`dotnet/src/XRay.Office.WordProcessing/Rtf/` and cannot change what `soffice` made. Our half at
the base commit was rendered fresh and **reproduces the banked verdict on 328 of 328**, so the
instrument is the scoreboard's.

## The result

| | match | of | |
|---|---:|---:|---|
| `.rtf`, at the base commit | 198 | 328 | 60.4% |
| `.rtf`, after this round | **215** | 328 | **65.5%** |
| the original words track | 338 | 338 | **0 rows moved** |

The original track cannot move by construction — it holds no `.rtf` at all, and every file
this round touches is under `Rtf/` — and that was measured as well as argued.

## The classification, and why the brief's first instruction did not carry here

The brief said to copy the `.odp` round: rank the deficits by `ref − ours` glyphs and look for
**repeated values**, using the same document's verdict in its **original spelling** as the
discriminator. The discriminator worked and the ranking did not.

**The discriminator.** Of the 123 banked non-matching rows, **106 pass as `.doc`/`.docx` and
fail as `.rtf`** — those are ours. 17 fail both and are not RTF defects.

**The ranking.** Unlike `.odp`, where six deficits covered 93 documents, the `.rtf` deficits
have almost no repetition: the largest repeat is a value shared by two documents, and nine of
the 106 share the value 0. So there is no fixed block of undrawn text here; the column is many
causes, and the page counts run **in both directions** — 46 rows have more pages than the
reference, 27 fewer, and 33 agree while failing on text.

**What replaced it** was ranking by *construct* rather than by deficit — asking which control
words each failing document actually contains:

| construct present | of the 106 |
|---|---:|
| `{\listtext…\pard…}` | **61** |
| a positioned table (`\tposy`) | 8 |
| a turned cell (`\cltxbtlr` / `\cltxtbrl`) | 3 |
| none of the three | 34 |

## What was closed

### 1. A list label's `\pard` took its paragraph out of its cell — the largest of the four

RTF has no list counters, so Word writes the rendered `1.` of every numbered item into a
`{\listtext\pard\plain \tab}` group. That `\pard` belongs to the label and is undone when the
group closes; table membership is the exception, because it is held on the flow rather than on
the group stack, so nothing restored it. Every cell of such a row was then read as body text
and the row never formed.

LibreOffice states the rule in its own source (`rtfdispatchflag.cxx`:588): *"\pard is allowed
between \cell and \row, but in that case it should not reset the fact that we're inside a
table."* Its renderer agrees on a two-line synthetic file.

**Found by extraction, not by pixels.** `150_5300_13_chg8` is 19 pages in both renderers and
7461 characters short. `pdftotext` of the reference draws Table A16-1A in full; ours drew the
bare list numbers `1.` to `12.` and no cell text whatever.

**21 709 such labels in 179 of the 338 documents.** 203 → 215.

### 2. A row definition can put its table on the page instead of in the flow

`\tpvpg`, `\tposy` and their family are RTF's Positioned Wrapped Tables, which LibreOffice
turns into `w:tblpPr`. A fly is not in the flow, so the paragraphs after it start where it
started; read as an ordinary table it consumes the flow and pushes them off the sheet.

**The set of words is LibreOffice's dispatch table, not the specification's prose.**
`rtftokenizer.cxx`:1619-1633 recognises `\tposxl`, `\tposxi`, `\tposxo`, `\tposyt`,
`\tposyil`, `\tposyin`, `\tposyout`, `\tposnegx` and `\tposnegy`, and **no dispatcher handles
any of them** — they reach no `tblpPr`, so none makes a table a fly and none states an
alignment the reference applies. Reading `\tposxl` as "left" because that is what it means
would left-align a table the reference leaves at its indent.

1135 of the 22 720 row definitions state a position, in 46 documents. Not one of the nine
dropped words occurs at all.

### 3. A cell states which way its text runs

`\cltxbtlr` and its siblings are `w:textDirection`; the reader did not read them. The layout
law was already there and already tested — this is that law never reached.

**It does not share the seat `dotnet/CLAUDE.md` records.** Those three documents ("rotated cell
text drawn upright a glyph per line") are all `.docx` and all pass; the DOCX reader has always
fed `PageTableCell.TextDirection`. This was a reading gap on the other spelling of the same
law. 186 `\cltxbtlr` in 11 documents, all eleven failing; six move a page closer.

### 4. A `PAGE` field prints the page it is on

`Layout.PageFields` has rewritten `PAGE` and `NUMPAGES` since round 45 and the DOCX, ODF and
WW8 readers have all fed it since; the RTF reader laid out the producer's cached `\fldrslt`
verbatim, so every page printed whichever number the writer last saved. 122 of the 338 carry
one, 924 `PAGE` and 94 `NUMPAGES` — and almost none of it is visible to the gate, a page
number being one or two characters, which is why the first RTF gate ran past it.

## The two defects the brief handed over

**A — `047_Visual_Product_Roadmap`**, ours 3 pages against 1, its rotated labels drawn a glyph
at a time as `S t r a t e D ge sy i g n`. The interleaving is closed by (3): 175 glyphs → 182
against the reference's 183, and 3 pages → 2. **It is still one page over**, and that residual
is not the direction — the same document at 1/1 as `.docx` says the remaining page belongs to
something else in the RTF path.

**B — `1_tpr_template__from_fy14_`**, ours 12 pages against 10 with the page-number field
reading 2 where the reference reads 1. The field is closed by (4). The page count was **not**
`\pgnstart`: it closed with (1), the document's tables having been read as body text. It is
now 10/10 and fails on text alone.

## What is left, and the honest cost

Four rows left `match`: `20110608_psp` 3 → 4 pages, `EHEST-SMS-Safety-Management-Manual`
81 → 82, `4400-91_Proposal_To_Lease_Space` 7 → 6, and `03_Technical_Report_(progress)_template`
on text. All four are the price of forming a row that used to be a run of body paragraphs, and
the reading is what the oracle does; the residual is table pagination.

The same trade shows at the top of the column. `SPA-06_mcar_part-6` went from 66 pages and
37 034 characters short to 88 pages and nearly all of the text present; `FRE-03_mcar_part-3`
from 54/67 to 76/67; `150_5300_13_chg10` from 77/77 with 25 132 characters missing to 57/77.
**The text is now read and the tables holding it do not paginate like Writer's.** That is the
next round's seat, and it is a layout question rather than a reading one — which is a better
place to be standing than where this round started.

The 34 rows carrying none of the four constructs are untouched and unclassified.
