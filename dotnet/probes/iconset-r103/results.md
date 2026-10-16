# O29 — `iconSet`, and the seven glyphs the corpus actually reaches

Round 103, seated by `probes/cond-format-r97/results.md` §6 and by register entry **O29**. Base
`e58567aea`, worktree `/home/user/wt-iconset`, branch `agent/iconset`, reference
`/opt/libreoffice26.2/program/soffice` (**LibreOffice 26.2.4.2**, build `0229ac93fcf0…`).

**The seat was parked as an artwork decision rather than a format question, and that framing was
half right.** The decision is in §2 and it took an afternoon; what took the round was that the
census which made it decidable — *which glyph does 26.2.4.2 actually paint, on which document* —
had never been taken, and taking it also turned up a **row-height defect in a reader nobody was
looking at** (§6).

**Everything below is confirmed twice**, as every round on this track has been: once against
26.2.4.2's own rendering of a real corpus document, and once against an authored fixture whose
expectation is read back out of 26.2.4.2's own PDF. The C++ tree is 27.2 alpha and not the
binary's source (confound C8), so every `file:line` here is cited **as this tree** and the
behavioural claim beside it rests on the binary.

**There is no `Task`/subagent tool in this container**, so no page was read by anyone
uncontaminated. One reading is mine and is labelled as such in §3; nothing rests on it — the
glyph identification it suggested was then settled by pixel comparison against the reference's own
installed assets.

**O29 reaches a terminal state: fixed, with a measured reach of 7 renderings and a named
narrowing.** §9 says exactly what is not drawn and why that is recorded rather than guessed.

---

## 1. The census, in both spellings, unioned

`census-iconset.py`, run 2026-09-11 over every file under `/home/user/sample-files` that opens as
an OPC spreadsheet — by content, not by extension, which is round 96's own lesson about
`Special-Procedures_2025-07-10.xls`.

| | rules | documents |
|---|---:|---:|
| main-namespace `cfRule type="iconSet"` | 2 | 2 |
| `x14` extensions **of** one of those | 0 | — |
| `x14:cfRule type="iconSet"` claimed by nobody | 18 | 8 |
| **union** | **20** | **10** |

`244` documents open as OPC spreadsheets. **The two populations are disjoint** — intersection 0 —
which is exactly where an earlier pass lost a document and wrote 9. The `x14`-only rules carry
their range as an `<xm:sqref>` on the enclosing `x14:conditionalFormatting`, *not* on the rule, and
a first cut of this round's own census read it off the rule and printed ten empty `sqref`s before
that was noticed.

The reason the eighteen exist at all is `ExtLstLocalContext`, whose comment says it outright —
*"an ext entry does not need to have an existing corresponding entry"*,
`sc/source/filter/oox/extlstcontext.cxx`:165-194, this tree.

Set names stated, by rule: `3Flags` ×9, `3Stars` ×6, `3Symbols` ×2, `3TrafficLights1` ×2,
`3Symbols2` ×1. **Five of the reference's twenty-two, and that is not the list of glyphs** — see
§3, because fourteen of the twenty rules are `custom="1"` and most of their buckets are
`NoIcons`.

---

## 2. The artwork decision, and the reasoning

**Decided: author the glyphs as vector paths, and author only the seven the corpus reaches.**
`SheetIconArtwork`, 300 lines, no assets, no new dependency, no raster path in the sheet renderer.

The three options the brief named, against what the measurement says:

**(a) Vector paths we author — taken.** Three facts settle it, and only the first was known when
the seat was written:

1. **The corpus reaches seven distinct assets, not twenty-two sets.** Every one of the sixty icons
   26.2.4.2 draws across the ten documents is one of seven images (§3). Three flags, a diamond and
   three discs is less new surface than a raster pipeline plus an asset set, by a wide margin.
2. **The artwork is already vector upstream.** The `colibre` icon set is authored as SVG
   (`icon-themes/colibre_svg/sc/res/icon-set-*.svg`) and the PNGs the application loads are
   rasterisations of it. So the paths are not an approximation of the reference's output — they
   are transcriptions of the source the reference's own bitmap was made from. Every coordinate in
   `SheetIconArtwork` is that SVG's, converted from its relative spelling to absolute, and every
   colour is its own hex. The three flags share one geometry and three palettes, which is how the
   assets are written; the diamond is two filled quadrilaterals rather than a stroke, because that
   is how the asset is written.
3. **An embedded raster would be a snapshot with no provenance.** The images extracted from the
   reference's PDFs do not byte-match any entry of any installed `images_*.zip` — the PDF writer
   re-encodes them — so a copied PNG could not even be checked against the theme it came from, and
   it would bind this tree to one icon theme of one version. It is also a licensed binary asset in
   a source tree, which is the decision r97 declined to take in a round and was right to.

**(b) Embed a small set of raster assets — rejected**, for (3) above and because it buys nothing:
at the sizes involved the glyph is 6 to 14 device pixels square (§4), where the ink is decided by
colour and coverage rather than by detail.

**(c) Something narrower — taken as well, and this is the second half of the decision.** Only the
glyphs the corpus reaches are drawn. That is not a shortcut but the standing rule on this track:
a set no corpus document uses is nil reach and is recorded rather than drawn speculatively. The
line is checkable — **every glyph drawn here is one the reference actually painted on a corpus
document**, and §9 is the table of what the other eighteen sets would need.

**What the other sets would need**, concretely, because the question will come back. All 56
distinct assets are in the same three-colour ramp and a small shape vocabulary; the geometry for
arrows, traffic lights, triangles, stars, pies, bars and boxes is no harder than the flag's, and
each is a dozen lines. What is *not* cheap is the confirmation: each would need an authored
fixture rendered at 26.2.4.2 and its images identified, which is the only evidence this project
accepts, and none of them would move a corpus rendering.

---

## 3. Which glyph, established rather than assumed

`reference-glyphs.py` → `reference-glyphs.txt`. Every image at most 32 × 32 in the banked
26.2.4.2 PDFs of the ten documents, extracted **with its soft mask applied** — the base image is
RGB with the alpha in a separate `/SMask`, and a first cut of this that skipped the compositing
read a transparent background as black and matched nothing at all.

| document | icons | assets |
|---|---:|---|
| `066_Agile_Gantt_chart` | 27 | `shapes-diamond` ×18, `flags-yellow` ×9 |
| `077_Inventory_list_with_highlighting` | 12 | `flags-red` ×12 |
| `078_Modern_inventory_list` | 12 | `flags-red` ×12 |
| `075_Idea_planner_tasks` | 4 | `symbols1-cross` ×2, `symbols1-exclamation-mark`, `symbols1-check` |
| `069_Blue_modern_balance_sheet` | 2 | `flags-green`, `flags-red` |
| `076_Inventory_list_accessibility_guide` | 2 | `flags-red` ×2 |
| `088_To-do_list_with_progress_tracker` | 1 | `symbols1-check` |
| `041_Business_budget`, `042_Business_monthly_budget`, `sistem-rekod-markah-srm` | 0 | — |
| **total** | **60** | **7 distinct** |

**Which theme, measured.** Each of the seven was compared against every `icon-set-*` entry of all
twenty `images_*.zip` the reference installs, by mean absolute channel difference over the
composited RGBA. Every one's nearest match is `images_colibre.zip`, at **0.007 to 0.064 out of
255**, and **the best any other theme manages on the same image is 19.7 to 45.1** — three hundred
to six thousand times worse. So the reference runs `colibre` headless and these are its shapes.

*The reading of those seven at 6× magnification is mine and was not blind.* It is what told me
the flag has a rounded pole, the diamond is outlined by a second diamond and the three discs carry
a white mark — and none of that is load-bearing, because the SVG sources say the same thing
exactly and the identification above is a pixel comparison rather than a description.

**Why seven and not twenty-two: `custom="1"`.** Fourteen of the twenty rules override at least one
bucket, and most overrides are `NoIcons`. `077` and `078` name `3TrafficLights1` and never draw a
traffic light — all three of their buckets are overridden, two to `NoIcons` and one to `3Flags` 0.
`066` names `3Stars` and never draws a star. **A census of set names is not a census of glyphs**,
and reading the first as the second is what would have made this a twenty-two-set round.

---

## 4. The mechanism

Read out of this tree and confirmed against 26.2.4.2 at every arm (§5).

**The bucket is the last entry whose comparison holds.** `ScIconSetFormat::GetIconSetInfo`
(`sc/source/core/data/colorscale.cxx`:1186-1253) walks every entry and keeps the highest matching
index — it does not break (`:1205-1216`). Each entry has its own mode, `EqGreater` by default
(`:1207`), moved to `Greater` only by a stated false `gte` (`condformatbuffer.cxx`:117-122).

**Four ways it answers nothing**, and the fourth is the one the corpus is made of: a cell that is
not numeric (`:1189-1190`), a rule with fewer than three entries (`:1195-1196`), a value no entry
accepts (`:1218-1219`), and a custom bucket spelt `NoIcons` — stored as index −1 by `importIcon`
(`condformatbuffer.cxx`:456-467) and answered as null at `:1236-1239`. **A null means the cell
keeps its own text however `showValue` is set**, because `DrawStrings` never sees an icon to
suppress it for (`sc/source/ui/view/output2.cxx`:1691-1698).

**The thresholds.** `GetMinValue`/`GetMaxValue` (`colorscale.cxx`:1312-1334) take the first (last)
entry's own number when it is typed `num` or is a formula and the range's smallest (largest)
otherwise; `CalcValue` (`:1336-1361`) turns each entry into a number, with `PERCENTILE` over a
single value answering that value. `reverse` reflects the index across the entries **after** the
search (`:1226-1230`) and **before** the custom vector is consulted (`:1232-1245`), so the two
compose in that order.

**A formula threshold is a formula whatever the attribute says.** `importFormula`
(`condformatbuffer.cxx`:424-435) keeps an `xm:f` as a number only when the type is `num`,
`percent` or `percentile` *and* the whole string parses; otherwise `ConvertToModel` (`:329-333`)
overrides the stated type with `COLORSCALE_FORMULA`. 26.2.4.2's own `fods` of
`069_Blue_modern_balance_sheet` writes `calcext:value="=[.$C$11]" calcext:type="formula"` for a
`<x14:cfvo type="num">`.

**The geometry.** `drawIconSets` (`sc/source/ui/view/output.cxx`:960-989) is four statements:

- the height is the cell's own font height — the ten points at `:967` is a fallback for a null
  `mnHeight`, and `GetIconSetInfo` always sets that field from `ATTR_FONT_HEIGHT`
  (`colorscale.cxx`:1222-1224), so the branch at `:969-980` always wins;
- the width is that height times the bitmap's aspect ratio (`:982-984`), which is 1 for every
  16 × 16 asset, so the icon is square;
- it sits at `rRect.Left() + 2 * nOneX`, `rRect.Bottom() - 2 * nOneY - aHeight` (`:988`) — the same
  two-device-pixel inset a data bar takes, which is why this shares `BarInset`;
- and it is clipped to the cell (`:987`).

**The height is measured, not derived.** At 100 % print scale on an 11 pt cell 26.2.4.2 draws all
five fixture icons at exactly **11.00 × 11.00 pt** (`fixture-glyphs.txt`). Over the corpus the
per-icon area runs 19.4 pt² on `066` to 103.9 pt² on `069`, which is what a constant ten points
cannot be.

And the icon does **not** displace the cell's text: `showValue="0"` removes the number outright
(`output2.cxx`:1696-1697) and `showValue="1"` draws it where it always was.

---

## 5. The fixtures, and what each one is for

`make-fixtures.py` writes four workbooks into `dotnet/tests/corpus/features/`. All four stems were
checked against `dotnet/tests/corpus`, `/home/user/sample-files` and `/home/user/corpus-odf`:
**zero occurrences each**, so no `--convert-to` output can overwrite another's.

Each was rendered by 26.2.4.2 (`fixture-reference/painted.txt`, produced by round 97's
`fixture-paint.py`, one output directory per fixture) and every image identified by pixel hash
(`fixture-reference/fixture-glyphs.txt`). Every expectation in `XlsxIconSetTests` is read out of
those two files.

| fixture | states | 26.2.4.2 draws | ours |
|---|---|---|---|
| `sheet-cf-icon-set-symbols` | `3Symbols`, three `percent` stops, **and nothing else** | cross, cross, exclamation, tick, tick; all five numbers | same |
| `sheet-cf-icon-set-x14-custom` | an `x14`-only rule, `custom`, `NoIcons`, `showValue="0"` | — , diamond, diamond, flag-red, flag-red; **one** number, the `NoIcons` cell's | same |
| `sheet-cf-icon-set-reverse` | `reverse="1"` and one `gte="0"` | green, green, amber, red, red | same |
| `sheet-cf-icon-set-formula` | an `x14` `num` stop whose `xm:f` is `$A$7` | red, red, amber, green, green | same |

**The first exists because of the `mbNeg` lesson.** An earlier round on this family shipped an
inverted arm and caught it only on re-reading, because every one of its fixtures stated the
optional attribute that most corpus documents omit. `sheet-cf-icon-set-symbols` states `iconSet`
and three `cfvo` and **nothing else** — no `showValue`, no `reverse`, no `custom`, no `cfIcon`, no
`gte`, no extension — so each of those defaults is exercised by its absence.

**The second and fourth are each discriminating rather than merely confirming.**
`sheet-cf-icon-set-x14-custom` has no `conditionalFormatting` element at all, so a reader that
walks only the main namespace draws nothing on it; and `sheet-cf-icon-set-formula`'s thresholds
resolve to 1, 3 and 3 with `$A$7` read and to 1, 0 and 0 without it — under which every value
clears every stop and all five cells would be green rather than red, red, amber, green, green.

**Positions agree to 0.14 pt across and 0.08 down** on the three fixtures whose rows are settled
(§6 is the fourth), which is this tree's own constant text-origin offset from the reference's and
not an icon quantity.

**Each arm has a test that fails without it — re-derived by mutation rather than asserted.** Four
one-line mutations of `XlsxIconSets`, each built and run:

| mutation | tests failing of 4 |
|---|---:|
| take the **first** matching bucket instead of the last | 4 |
| stop resolving a single-cell formula threshold | 1 |
| ignore `reverse` | 1 |
| make a `NoIcons` bucket an icon with no glyph instead of no icon | 1 |
| *(none — the tree as committed)* | **0** |

---

## 6. A second seat found on the way, and it is a row height

**The `x14` extension list was excluded from `ReadConditionalRanges`, and the reason given for it
was false.** That function collects the blocks a conditional format covers — not for what the
condition paints but for the **row height**, because `ScColumn::GetOptimalHeight` clears
`bStdOnly` for a cell whose pattern carries a non-empty `ATTR_CONDITIONAL`
(`sc/source/core/data/column2.cxx`:937-941) and measures it through `GetNeededSize` instead of
taking `lcl_GetAttribHeight`'s arithmetic. The two answers differ by **22 twips at 11 pt**, 298
against 276. Its remarks said an `x14` rule was *"deliberately not read … the plain element is
what every workbook the corpus holds writes"*.

**Six corpus documents state an `x14` range no main-namespace `conditionalFormatting` covers** —
10 ranges, all of them `iconSet` — and every row of those ranges was being measured by the wrong
branch. `sheet-cf-icon-set-formula.xlsx` shows it with no corpus noise at all: its only rule is an
`x14`-only one, 26.2.4.2's baseline pitch on it is **14.88 pt** and this tree's was **13.78**.
After the fix the fixture's five icons sit at 74.52, 89.40, 104.28, 119.17 and 134.05 against the
reference's 74.44, 89.32, 104.20, 119.08 and 133.97 — the same 0.08 pt offset as everywhere else.

The fix is four lines and a shared helper: `ReadConditionalRanges` now also walks
`x14:conditionalFormatting` and reads its `xm:sqref`. An extension a main-namespace rule *does*
claim needs no entry of its own, and adding one anyway costs nothing, so the two are not told
apart.

**What is left beside it, measured and deliberately not chased.** The
`sheet-cf-icon-set-x14-custom` fixture still has a 13.78 pt pitch where the reference has 14.88,
and the cause is a *different* mechanism that predates this round: `XlsxHiddenValues` — a
pre-existing partial `iconSet` reader that neither round 96 nor round 97 mentions — drops a
`showValue="0"` cell's text at **cell-build** time, in `XlsxSheetReader`, so the row is measured as
though it were empty. The reference clears `bDoCell` at **paint** time, after the row's height is
settled. Moving ours to paint time would fix the height and is what `SheetDecoration.HidesValue`
already does correctly for the drawing half; it would also put the number back into
`ContentTableCell`, which reaches text extraction, chart ranges and the print-area scan. That is a
round of its own and it is recorded here rather than taken.

**One consequence for honesty about this round's own arms.** Because `XlsxHiddenValues` already
hides the same cells, `SheetIcon.ShowValue` reaching `HidesValue` changes **nothing on the
corpus** — the two agree everywhere, on 066 and 077 (the only `showValue="0"` corpus rules) and on
all four fixtures. It is the mechanism-correct place for it and it is covered by
`AnExtensionOnlyRuleIsARuleAndItsNoIconsBucketKeepsItsText`, but its measured corpus reach is
**nil**, and saying otherwise would be claiming an arm the corpus cannot see.

---

## 7. Reach, and it is 7 renderings

**N11 applies and the honest headline is the second number.** 20 rules in 10 documents is a markup
count; **7 renderings change**, and they are exactly the seven documents 26.2.4.2 draws an icon on.

Rendered **all 307 sheets documents twice**, at the round's base (`e58567aea`) and with the change,
under `SOURCE_DATE_EPOCH`, one output directory per *document*, from two frozen CLI copies outside
the tree — so no rebuild could reach either leg while it ran, and the tree was rebuilt several
times during them without touching the measurement. Both legs completed **307 of 307 with `ok` on
every row and no failure on either side** (`sweep-base-hashes.tsv`, `sweep-after-hashes.tsv`; the
script is round 96's `sweep-ours.py`, unchanged).

**300 of 307 are byte-identical. Seven move:**

| document | pages | base | **after** | MAJOR |
|---|---:|---:|---:|---|
| `066_Agile_Gantt_chart` | 5 | 3.50 | **3.50** | 2 → 2 |
| `076_Inventory_list_accessibility_guide` | 9 | 0.99 | **0.99** | 0 → 0 |
| `075_Idea_planner_tasks` | 1 | 0.30 | **0.29** | 1 → 1 |
| `088_To-do_list_with_progress_tracker` | 1 | 0.04 | **0.03** | 0 → 0 |
| `069_Blue_modern_balance_sheet` | 4 | 0.04 | **0.03** | 0 → 0 |
| `078_Modern_inventory_list` | 1 | 0.02 | **0.00** | 0 → 0 |
| `077_Inventory_list_with_highlighting` | 1 | 0.02 | **0.01** | 0 → 0 |
| **sum** | | **4.91** | **4.85** | **3 → 3** |

**Five improve, two hold, none worsens**, and no page count moves anywhere. `pdf-image-diff.py`
reports to a hundredth of a percent, so 3.50 → 3.50 and 0.99 → 0.99 mean *below that resolution*
rather than *identical* — the bytes moved on both. The three documents that state a rule and draw
no icon — `041`, `042`, `sistem-rekod-markah-srm` — are byte-identical on both legs, which is the
control that matters most: the reader does not invent an icon where the reference draws none.

**The ink numbers are small and the icon census is the sharper instrument.** `icon-census.py`
identifies every icon on both sides — the reference's by pixel hash, ours by the palette colours
of each cluster of paths — and **9 of the 10 documents agree glyph for glyph and count for
count**:

```
= 088   1 tick-green x1                                1 tick-green x1
= 041   0 (none)                                       0 (none)
= 075   4 cross-red x2, excl-amber x1, tick-green x1   4 cross-red x2, excl-amber x1, tick-green x1
= 069   2 flag-green x1, flag-red x1                   2 flag-green x1, flag-red x1
= 078  12 flag-red x12                                12 flag-red x12
= 042   0 (none)                                       0 (none)
! 066  27 diamond x18, flag-amber x9                  18 diamond x9, flag-amber x9
= 077  12 flag-red x12                                12 flag-red x12
= 076   2 flag-red x2                                  2 flag-red x2
= sistem 0 (none)                                      0 (none)
```

**And the tenth is the volatile-date confound, not an icon defect.** `066_Agile_Gantt_chart`'s
`F12` is `<f ca="1">TODAY()</f>` with a cached `44957`, and its `I12:BL12` are
`IF(AND($C12="Goal",I$7>=$F12,I$7<=$F12+$G12-1),2,…)` cached as the empty string. LibreOffice
recalculates on load, so the bar moves and those cells evaluate to 2 — a diamond — while we render
the cached blank. The same recalculation is visible in plain text on the same page: the reference
draws `9/8/2026` where we draw `1/31/2023`, and its whole Gantt grid sits 38.6 pt to the right of
ours. This is `CLAUDE.md`'s sixth confound arriving on this seat; that document was already MAJOR
on the same pages at the base and is unchanged by this round.

**Geometry against the reference, on the two documents where a single icon can be measured
cleanly:**

| | 26.2.4.2 | ours |
|---|---|---|
| `088` H3 | 600.93–608.63 × 160.72–168.42, 7.70 pt square | 601.04–608.74 × 160.78–168.48, 7.70 pt square |
| `075` F7–F10 | 330.56–337.49, 6.93 pt square | 330.64–337.57, 6.93 pt square |

**Sizes agree exactly and positions to 0.11 pt**, which is the same constant text-origin offset
the data-bar round measured. (`069`'s flags read 8.92 pt wide against the reference's 10.19
because the reference's box is the whole 16 × 16 bitmap including its transparent left margin and
ours is the inked part; the pole starts at x = 2 of 16, and 14/16 × 10.19 = 8.92.)

---

## 8. Confinement and the suite

**The diff is confined to `dotnet/src/XRay.Office.Spreadsheets`** — eight files, three of them new —
plus one test file and four fixtures. Nothing under `dotnet/src` outside it changes, so by the
dependency layering no words document and no deck can move. §7 is the measurement of the same
thing for the family that *can*: 300 of 307 sheets renderings byte-identical.

The converted-ODF column cannot move either, and for a narrower reason than the layering:
`SheetFormatting.IconAt` has exactly one writer, `XlsxIconSets`, so an `.ods` sheet has no icons
to draw, and `ReadConditionalRanges` is the SpreadsheetML reader's — `OdsConditionalFormats.ReadRanges`
is untouched.

Clean build of the whole solution at `-c Release`: **0 warnings, 0 errors**, with
`TreatWarningsAsErrors` on.

| project | passed | discovered | |
|---|---:|---:|---|
| `XRay.Office.Core.Tests` | 528 | — | ✓ |
| `XRay.Office.Markup.Tests` | 259 | — | ✓ |
| `XRay.Office.Text.Tests` | 728 | — | ✓ |
| `XRay.Office.Containers.Tests` | 109 | — | ✓ |
| `XRay.Office.Vector.Tests` | 309 | — | ✓ |
| `XRay.Office.Rendering.Tests` | 164 | — | ✓ |
| `XRay.Office.OpenDocument.Tests` | 146 | — | ✓ |
| `XRay.Office.WordProcessing.Tests` | 1938 | **1938** | ✓ |
| `XRay.Office.Spreadsheets.Tests` | 1299 | **1299** | ✓ (4 of them this round's) |
| `XRay.Office.Presentations.Tests` | 1054 | **1054** | ✓ |
| **ten projects** | **6534** | | **0 failed** |

The three largest were checked against `dotnet test --list-tests` in the same run rather than
against a remembered figure, which is `dotnet/CLAUDE.md`'s rule for catching a truncated run from
the side where the count is taken first: **passed equals discovered in all three**.

`XRay.Office.Fidelity.Tests`: **542 passed, 10 failed of 552**, and the ten are exactly the known
names — `PageDrawingComparisonTests` ×4 (`paginated.` `docx`/`fodt`/`rtf`/`doc`),
`TabStopComparisonTests` ×4 (`list-label-overrun.` `docx`/`doc`/`odt`/`fodt`),
`SheetDrawingComparisonTests` (`sheet-rich-text.xlsx`) and `JustificationShrinkComparisonTests`
(`justify-shrink-2013.docx`). **No eleventh failure.**

---

## 9. What is not drawn, and why that is recorded rather than guessed

`SheetIconArtwork` draws **7 of the reference's 56 distinct icon-set assets**. The table is
`aBitmapMap` and the `a3Flags`-style tables beside it (`colorscale.cxx`:1399-1521), read
programmatically rather than transcribed:

| OOXML `iconSet` | n | assets | drawn here |
|---|---:|---|---|
| `3Arrows` | 3 | colorarrows-down, -same, -up | 0 of 3 |
| `3ArrowsGray` | 3 | grayarrows-down, -same, -up | 0 of 3 |
| **`3Flags`** | 3 | flags-red, flags-yellow, flags-green | **3 of 3** |
| `3Signs` | 3 | shapes-diamond, shapes-triangle, shapes-circle | **1 of 3** |
| **`3Symbols`** | 3 | symbols1-cross, -exclamation-mark, -check | **3 of 3** |
| **`3Symbols2`** | 3 | *(the same three)* | **3 of 3** |
| `3TrafficLights1` | 3 | circles1-red, -yellow, -green | 0 of 3 |
| `3TrafficLights2` | 3 | trafficlights-red, -yellow, -green | 0 of 3 |
| `3Smilies` | 3 | negative-yellow-smilie, neutral-yellow-smilie, positive-yellow-smilie | 0 of 3 |
| `3ColorSmilies` | 3 | negative-red-smilie, neutral-yellow-smilie, positive-green-smilie | 0 of 3 |
| `3Triangles` | 3 | triangles-down, -same, -up | 0 of 3 |
| `3Stars` | 3 | stars-empty, stars-half, stars-full | 0 of 3 |
| `4Arrows` | 4 | colorarrows ×4 | 0 of 4 |
| `4ArrowsGray` | 4 | grayarrows ×4 | 0 of 4 |
| `4Rating` | 4 | bars-one-quarter … bars-full | 0 of 4 |
| `4RedToBlack` | 4 | circles2-dark-gray … circles2-dark-red | 0 of 4 |
| `4TrafficLights` | 4 | circles1-gray, -red, -yellow, -green | 0 of 4 |
| `5Arrows` | 5 | colorarrows ×5 | 0 of 5 |
| `5ArrowsGray` | 5 | grayarrows ×5 | 0 of 5 |
| `5Quarters` | 5 | pies-empty … pies-full | 0 of 5 |
| `5Rating` | 5 | bars-empty … bars-full | 0 of 5 |
| `5Boxes` | 5 | squares-empty … squares-full | 0 of 5 |

**Every row with a zero is nil reach on this corpus**, in the strong sense: no corpus rule selects
any of those assets, established by the census in §1 and the glyph identification in §3 together —
the two rules that *name* `3TrafficLights1` and the six that name `3Stars` override every one of
their own buckets, so the sets are named and their glyphs are never reached.

A cell whose bucket resolves to one of them takes `SheetIconGlyph.Unpainted`: the reference has an
`ScIconSetInfo` for such a cell, so `showValue="0"` still removes its number, and nothing is
painted rather than something guessed. **No corpus document reaches that member.**

---

## 10. What is left

**O29 — fixed.** Mechanism cited by `file:line` in this tree and confirmed at 26.2.4.2 on every
arm; four fixtures whose expectations are the reference's own PDF; four tests, each shown to fail
under a one-line mutation of the rule it pins; a measured reach of **7 of 947 renderings**, five
improving and none worsening, with 9 of the 10 stating documents agreeing with the reference glyph
for glyph.

**Two things found on the way and left, each with its seat:**

- **`XlsxHiddenValues` hides a `showValue="0"` cell's text one stage too early**, at cell-build
  rather than at paint, which shortens the row by 22 twips at 11 pt. Measured on
  `sheet-cf-icon-set-x14-custom.xlsx`: 13.78 pt pitch against 26.2.4.2's 14.88. The fix is to let
  `SheetDecoration.HidesValue` carry it alone, which reaches text extraction, chart ranges and the
  print-area scan — a round of its own. Corpus reach: 2 documents (`066`, `077`).
- **`066_Agile_Gantt_chart`'s nine missing diamonds are `TODAY()`**, not the icon reader. Its
  `IF(AND(…))` cells cache the empty string and recalculate to 2 in LibreOffice. Nothing to fix
  here; it belongs to the volatile-formula class `CLAUDE.md` records as the sixth confound.

**And one instrument note.** A 16 × 16 icon in a LibreOffice PDF is an RGB image with its alpha in
a separate `/SMask`. Extracting the base image alone gives a picture whose most common colour is
black — the transparent background — which matches no theme asset and reads as *the reference is
drawing something we have never seen*. `pymupdf.Pixmap(pix, pymupdf.Pixmap(doc, smask))` is the
whole of the correction, and without it this round's central identification returns nothing at all
while looking like it worked.
