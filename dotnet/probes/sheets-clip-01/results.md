# sheets-clip-01 — the paint clip: a cell's ink is cut at the edge of the page's columns

Round of 2026-08-14 on branch `wt-sheet-clip`. Reference binary **LibreOffice 26.2.4.2
620(Build:2)**; banked references at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/sheets/`, reused
rather than re-rendered. `SOURCE_DATE_EPOCH=1700000000` for every rendering compared here.
`fc-match "DejaVu Sans"` answers DejaVu; `fc-match Calibri` answers Carlito.

`prediction.md` beside this was written and committed before any reach was measured; it is scored
in §8, and it says honestly which of its claims were already established when it was written.

---

## 1. The rule, and how it was measured

### What the source says

`ScOutputData::AdjustAreaParamClipRect` (`sc/source/ui/view/output2.cxx:2928-2954`) is named like a
clamp and is not one:

```cpp
if( rAreaParam.maClipRect.Left() < mnScrX )
{
    rAreaParam.maClipRect.SetLeft( mnScrX );
    rAreaParam.mbLeftClip = true;          // <- sets the flag
}
if( rAreaParam.maClipRect.Right() > mnScrX + mnScrW )
{
    rAreaParam.maClipRect.SetRight( mnScrX + mnScrW );
    rAreaParam.mbRightClip = true;         // <- and this one
}
```

`LayoutStrings` computes `bHClip` from those two flags **after** calling it (`:2038-2039`), and
`DrawEditStandard` ors them into its own `bClip` the same way (`:3239`). So the trim does not
merely narrow a clip that was going to be set anyway — **it turns one on for a cell whose text fits
the room it was given perfectly well**. Then `:2114-2123` widens whichever axis needed no clipping
to the whole block, which is why an emitted rectangle is never narrower than the block in that
direction.

`mnScrX` and `mnScrX + mnScrW` are the printed **column block**: the page's own columns, or its
repeated columns, each a separate `ScPrintFunc::PrintArea` call.

### What the binary does

The tree here is 27.2-alpha and the reference is 26.2.4.2, so the source is a lead. Two independent
measurements settle it, both read out of the reference's **own PDF content stream** rather than
from a raster.

**(a) An authored probe**, committed as `dotnet/tests/corpus/features/sheet-clip-block.fods`.
Eleven 3 cm columns on A4 with 2 cm margins, so five columns fit a page and the block on page 1
runs **56.693 to 481.890 pt**. Five cases, one token each:

| row | case | reference's clip rectangle |
|---|---|---|
| ZAAA | long left-aligned string in A, every neighbour free | `56.693..481.890` |
| ZBBB | long left-aligned string in E, the page's last column, F free | `396.850..481.889` |
| ZCCC | centred merge C5:H5 straddling the break | `226.772..481.890` p1, `56.693..311.754` p2 |
| ZDDD | long string in C with D occupied | `226.772..311.755` |
| ZEEE | a string that fits | **no clip at all** |

ZBBB is the decisive one. Calc widened the area into the free column F, whose right edge is
566.93, and then trimmed it back to 481.889 — the *block's* edge — while keeping column E's own
left edge. A rule that simply substituted the block would have moved the left edge too; a rule that
did not trim would have cut at F.

ZDDD is the control: the ordinary blocked overflow still gets the cell's own edge, so the rule is a
trim and not a replacement.

**(b) The two corpus documents**, likewise from the operators:

- `fse_identification_form.xlsx` p1: the reference wraps the merged description cell in
  `q 199.389 249.109 734.315 61.483 re W* n` — right edge **933.70**, which is exactly where the
  page's fills and rules end (`50.343 … 883.332` wide). We emitted the same rectangle 48.13 pt
  wider — one column further — because our cell box is the whole merge.
- `Infotabelle_WLAN im Flugzeug.xlsx` p2: seven `kein WLAN` runs at `308.75` in **both** renderings,
  agreeing to 0.01 pt. The reference wraps them in `q 50.4 188.05 285.08 597.146 re W* n`, which is
  the block in both axes; we emitted no clip at all. They are `D:E` merges (`<mergeCell ref="D8:E8"/>`
  …) and page 2 prints column D alone, so the merge's align rectangle sticks out on the right.

### The rule

> **A cell's painted text is cut to its output area intersected with the printed column block.** The
> intersection is what turns the clip on: a cell whose area sticks out of the block is clipped even
> when its text fitted the room it borrowed. The axis that needed no clipping is widened to the
> whole block. A cell whose area lies inside the block and whose text fits it is not clipped at all.

Two riders:

- **The text layer is not cut.** A clip removes ink and leaves the glyphs in the PDF. That is the
  whole reason this defect survived a word-count gate on both documents that show it, and it is a
  fact about the reference, not only about us: on
  `Data-Architecture-Tool-Fit-Assessment-Template.xlsx` p1 the reference's ink ends at 241.8 pt
  while `pdftotext` reads all 2230 of its words.
- **A cell that is not clipped at all may still paint past the block, and past the paper.** ZAAA's
  neighbours are free; had they absorbed its width without crossing 481.89 there would have been no
  clip and the run would have been painted in full.

## 2. This contradicts the previous round, and the previous round was measuring the wrong layer

`sheets-overflow-01` §2 records, as a measured rider, that "**it is not a clipping rule** — on the
anchor's own page the run is not clipped to the paper: the rightmost text on probe page 1 reaches
`xMax = 617.63 pt` on a 612 pt page, *identically* in the tagged and untagged renderings".

That measurement is correct and the conclusion drawn from it is not. 617.63 pt is a **text-layer**
figure — `pdftotext`, or a bbox over it — and a clip region never touches the text layer. The same
confusion is visible in that round's own §6(b): it reports fse's "rightmost text now sits at
974.6 pt against the reference's 973.5 pt, so the extents agree". The *ink* extents did not agree:
measured on the raster, ours reached 973.9 pt and the reference stopped at **933.6 pt**, a 40 pt
overhang, and that is the defect this round fixes.

The lesson is narrower than "measure the binary" and worth keeping separately: **when the question
is about paint, measure ink; `pdftotext` is a different instrument pointed at a different half of
the file.**

## 3. What changed

`dotnet/src/XRay.Office.Spreadsheets/Layout/SheetTextLayout.cs`

- **`ClipTo` added.** The port of `AdjustAreaParamClipRect`'s horizontal half: trim the placement's
  left and right to `[BlockLeft, BlockRight]`, and or the fact that it had to trim into the
  clipped flag. It is asked *after* `IsOutside` and never before, which is Calc's order —
  `bOutside` is decided against the untrimmed area (`:2036`) and would answer "inside" for every
  cell once the area had been folded into the block.
- `Draw` now clips to that rectangle instead of to the raw output area.

`dotnet/src/XRay.Office.Core/Graphics/IDrawingSink.cs` and
`dotnet/src/XRay.Office.Rendering/Pdf/PdfContentSink.cs`

- **`ClipPathKeepingText` added**, a default interface method delegating to `ClipPath`, overridden
  in the PDF sink to emit the clip without joining the rectangle that `Hidden` consults. So the ink
  is cut and the glyphs stay in the text layer — which is what LibreOffice's own PDF does.

  This was not optional and it was not foreseen. `PdfContentSink.Hidden` drops a glyph run that the
  active clip excludes entirely, deliberately, so that our PDF and our PNG agree. With the new clip
  in force that rule newly swallowed whole wrapped lines: a first sweep moved **23 word counts, 19
  of them further from the reference**, cost 124 words on `Data-Architecture-…` alone and flipped
  it from `match` to `words`. The reference keeps those glyphs, so we must. Nothing else in the
  tree calls the new method, so no other family's output moves.

**Only the horizontal half of the rule is reproduced.** Calc trims the vertical to
`[mnScrY, mnScrY + mnScrH]` by the same code; nothing measured in the corpus turns on it, our
vertical extent is deliberately the union of the cell and its text rather than a cut, and `RowBand`
carries no bottom edge to trim against. Recorded as open rather than done.

`DrawRotated` is also left alone. Calc clips rotated text at the block outright — "only clip
rotated output text at the page border" (`:5191-5196`) — so the same rule applies there by a
different route, and reproducing it is a separate change with a separate measurement.

## 4. Measured reach — the sheets track, 171 documents

Two full sweeps, before and after, scored against the banked references with `batch-check.sh`'s own
three checks and thresholds (page count; letter-or-digit words in a 2%+3 band; unembedded fonts).

| | match | pages | pages,words | words |
|---|---:|---:|---:|---:|
| before | **154** | 6 | 3 | 8 |
| after | **154** | 6 | 3 | 8 |

- **Page counts moved: 0 of 171.** A clip is downstream of pagination.
- **Word counts moved: 0 of 171.** `Σ|ours − ref|` is **27147 before and 27147 after**, unchanged
  to the word. That is the point of `ClipPathKeepingText`: the reference does not lose those words
  either.
- **Gate verdicts moved: 0.**
- **Fidelity: 30 failed of 550 before and after**, 0 skipped — the branch's inherited baseline,
  unmoved.

**And 71 of 171 documents render differently.** That is the reach, and the gate is blind to every
bit of it. Stated plainly rather than dressed up: *this round moves no scoreboard column at all.*

What did move is ink. Rasterising the first three pages of each of those 71 documents at 100 dpi
and comparing the rightmost inked column against the reference's:

| | pages |
|---|---:|
| closer to the reference | **40** |
| further | 6 |
| unchanged | 162 |

29 documents have at least one page closer, 5 at least one further. The two documents the brief
named are now exact:

| document, page | ours before | ours after | reference |
|---|---:|---:|---:|
| `fse_identification_form.xlsx` p1 | 2029 px | **1945** | 1945 |
| `Infotabelle_WLAN im Flugzeug.xlsx` p2 | 753 px | **698** | 698 |
| `Data-Architecture-Tool-Fit-Assessment-Template.xlsx` p1 | 1240 px | **504** | 504 |

### The six pages that moved further, and why they are not this change's fault

All five documents (`TOGAF9-Tool-ConfReqts-CSQ.xls`, `orbus_togaf_tool_csq.xls`,
`RegChangeReport.xlsx`, `DOE-C2M2-V1.1-to-DOE-C2M2-V2.1.xlsx`, `fm-provider-service-measures.xlsx`)
sit on pages where **our printed column block is narrower than LibreOffice's**, which is a
pre-existing pagination difference the backgrounds show independently of any text. On
`TOGAF9-Tool-ConfReqts-CSQ.xls` p1 the reference's row backgrounds run `50.34 → 632.21` and ours
run `50.40 → 72.54` — one column against many, and the fills are untouched by this round. Before
the fix we painted those pages' spilled text unclipped, which happened to cover more of the page
and so scored closer by accident. The clip is faithful to the block it is given; the block is
wrong, and that is the next thing to fix on those documents.

## 5. Regression

`sheets/batch-005` and `batch-006` first, then `batch-001` through `006` together, both against the
banked references:

| range | documents | match | words |
|---|---:|---:|---:|
| `sheets/batch-00[56]` | 20 | 16 | 4 |
| `sheets/batch-00[1-6]` | 60 | 56 | 4 |

The four are `2017-04-27-Lease-Transition-Records-Checklist-FINAL-1`,
`2020-01-29-Lease-Transition-Records-Checklist-FINAL-1`, `Published_Issuances_2024` and
`fse_identification_form`, and every one of them fails the same check with **the same two numbers
before and after** (2323/2498, 2323/2498, 457/479, 440/427). Zero regressions.

Every test project run individually:

| project | passed | failed | skipped |
|---|---:|---:|---:|
| Containers | 109 | 0 | 0 |
| Core | 313 | 0 | 0 |
| Markup | 259 | 0 | 0 |
| OpenDocument | 125 | 0 | 0 |
| Presentations | 631 | 0 | 0 |
| Rendering | 150 | 0 | 1 |
| Spreadsheets | 731 | 0 | 0 |
| Text | 297 | 0 | 0 |
| Vector | 295 | 0 | 0 |
| WordProcessing | 818 | 0 | 0 |
| **Fidelity** | **520** | **30** | **0** |

The Fidelity 30 is the branch's baseline, established before anything was changed and unmoved by
it. Spreadsheets is 731 against 727 before, and Rendering 150 against 148: the six new tests.

## 6. Verification by looking

Three page pairs at 150 dpi, each handed to a separate fresh subagent that had never seen the
document and was forbidden to read the repository or run any command.

- **`Infotabelle_WLAN im Flugzeug.xlsx` p2** — "Yes, in **BOTH** halves, identically. Seven rows
  contain the right-aligned string `kein WLAN` which is wider than its cell. In both halves the
  render shows `kein` plus a vertical slice through the **middle of the W** — the glyph is cut
  mid-letter exactly at the border line … the cut lands at the same fraction of the W in both
  halves. This is *not* a difference between the halves." The brief's fourth case, closed by a
  reader who was not told what to look for.
- **`Data-Architecture-Tool-Fit-Assessment-Template.xlsx` p1** — "the same horizontal position,
  within ~1–3 px … In both halves the cut coincides exactly with the right border of the table
  below … letters are cut through the middle in both halves. This is a rectangular clip … this is
  the *same* behaviour on both sides, so mid-glyph clipping here is correct-by-reference."
- **`fse_identification_form.xlsx` p1** — "**Both halves cut abruptly, at the same place, and both
  cut through the middle of letters.** The rightmost ink is at x = 1483 in ours and x = 1483 in the
  reference — pixel-identical. Both show `throug`**`h`** with the h's stem sliced vertically … This
  is a hard raster clip, not word-wrap and not an ellipsis." That reader went further than asked,
  cropping and measuring the image, and ruled the class out explicitly: "the horizontal clipping
  behaviour — the thing the right-edge question probes — is *not* a difference here."

### Three new defects the readers found, none of them this one

All confirmed **in the PDF's own operators** before being written down, per the compositor warning
in the `page-vision` skill — a reviewer's "it is absent" is a claim about their image until it is.

1. **A merged cell's background is *not* clipped to the block, and ours is.** Two independent
   readers found it on two documents and the fill operators confirm both:

   | | reference | ours |
   |---|---|---|
   | Infotabelle p2, five zebra rows | `(50.34, 392.09)-(621.16, 407.85)` | `(50.40, 392.10)-(335.48, 407.83)` |
   | fse p1, the grey header band | `(50.34, 487.50)-(981.81, 558.03)` | six per-column fills ending at `933.68` |

   Both reference figures are the full width of the merge — `D:E` and the header's own — while the
   page's block ends at 335.48 and 933.68 respectively. So `ScOutputData::DrawBackground` carries
   no equivalent of `AdjustAreaParamClipRect`: **a merge's text is trimmed to the block and its
   background is not.** That asymmetry would have been dismissed as implausible had it been
   reasoned rather than read off two files.

   Ours is also the wrong shape twice over — one fill per column where the reference emits one per
   merge — which is why the difference shows as a missing 48 pt of grey rather than as a seam.

2. **A justified multi-line cell's last line is placed differently.** Our `Internet/Streaming: 12 €`
   sits "about **115–130 px to the LEFT**" of the reference's, which has it flush against the
   border under the two lines above. Confined to that one three-line justified cell on Infotabelle
   p2; the other multi-line blocks on the page agree to a few pixels.

3. **We draw a description the reference leaves blank**, and it is the whole of fse's residual word
   gap. The reader, with no access to the numbers: "the reference leaves that Description cell
   **completely empty** … the rows above and below are populated normally, so the gap is a single
   missing line". It is the `Serial number` row's description, 13 words — exactly the 440 − 427 the
   gate reports, before and after this round. The row heights are identical in the two renderings,
   which the reader established and which rules out the obvious cause; their remaining candidates
   are a line-height threshold in Calc's vertical fit, a merge the two resolve differently, and our
   inventing the string. Worth its own round.

`SIL_TDB605.xls` p6, the brief's third and self-described weaker case, is **not** in this class.
Its page-1-to-p6 divergence is a whole region below y≈505 pt where we draw paragraphs the reference
does not, which is a drawing-layer or text-frame difference; the table above it agrees. The brief
was right to flag it as "check, not established", and the check says no.

## 7. Scoring the prediction

| # | predicted | outcome |
|---|---|---|
| 2.1 | an authored probe will show the reference clipping a straddling merge's ink at the block edge while its text layer keeps every character | **correct**, all five probe cases |
| 2.2 | the previous round's "not a clipping rule" rider was a text-layer measurement and is wrong about ink | **correct** — 933.6 pt ink against 973.5 pt text on the same page |
| 2.3 | a plain overflow through *empty* neighbours is still clipped at the block edge | **correct** — ZAAA and ZBBB |
| 4 | 0 page counts, 0 word counts, 0 verdicts moved | **correct as shipped, and wrong on the way there.** The first build moved 23 word counts and one verdict; the prediction held only after `ClipPathKeepingText`, which I had not foreseen and which the prediction's own phrasing — "if a word count moves at all I have changed something I did not mean to" — is what caught |
| 4 | Fidelity 30, possibly 29 | **30** |
| 4 | 15–40 of 171 documents' page-1 ink moves | **under-predicted** — 71 of 171 render differently, 29 of them with a moved ink extent in the first three pages |
| 4 | 0 regressions in batches 001–006 | **correct** |

## 8. A process note that cost real time

**`git stash` is repository-global and this repository has several worktrees with an agent in
each.** Stashing one file to build a "before" binary, and popping it forty minutes later, popped
*another branch's* stash into this worktree and left mine to be found by them. Both entries were
recovered (`git stash store` re-creates a dropped one from its commit id, which is still reachable)
and no work was lost, but the two sweeps either side of it had to be re-checked for confounding
before they could be believed — they were clean, because the other branch's changes never reached
this worktree until the pop.

The habit that avoids it: to measure a "before", copy the file aside and restore it by hand, or
build from a detached checkout. Never `git stash` in a shared clone.

## 9. Files

- `prediction.md` — written and committed before the reach was measured.
- `dotnet/tests/corpus/features/sheet-clip-block.fods` — the authored probe, with every reference
  figure in its header.
- `dotnet/tests/XRay.Office.Spreadsheets.Tests/SheetBlockClipTests.cs` — the four clip rectangles and
  the text-keeping variant.
- `dotnet/tests/XRay.Office.Rendering.Tests/PdfInvisibleTextTests.cs` — two tests for what
  `ClipPathKeepingText` does differently in a file, beside the eight that pin the plain clip.
