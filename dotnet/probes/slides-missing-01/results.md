# slides-missing-01 — an autofit that shrinks a body to an em of zero

Round subject: `slides/missing-001`, one document —
`NWD-GLA-Community-Outreach-Day-Oct-2025.pptx`. Page-exact 13/13, **529 words against 638**.

**Environment.** `check-env.sh` green, quoted: `LibreOffice 26.2.4.2 620(Build:2)`; a document
converts; Calibri→Carlito, Cambria→Caladea, Arial→Liberation Sans, Times New Roman→Liberation
Serif, Courier New→Liberation Mono, DejaVu Sans→DejaVu Sans; `pdftoppm 26.01.0`;
`pdftotext 26.01.0`. Reference renderings are the banked
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/slides/` (163). `SOURCE_DATE_EPOCH=1700000000` on
every render that was diffed.

**Baseline, established on the unfixed tree before anything was touched:**
`XRay.Office.Fidelity.Tests` **Failed 30, Passed 520, Skipped 0, Total 550** — the briefed figure,
reproduced exactly.

**Verdict moves.** 529/638 → **644/638, `match`.** One document, and one only.

---

## 1. The seat

`dotnet/src/XRay.Office.Presentations/Layout/SlideAutofit.cs`, `SlideTextLayout.Solve`.

The autofit search is a bisection over the font scale on the interval `[0, 1]` — a faithful port
of LibreOffice **24.2.7.2**, which is not the binary that writes the reference here. It has no
lower bound. A body that overflows its box by a factor of twenty drives it into the thousandths,
and `Scaling.Scaled` then rounds the scaled em **to a whole point** before converting it back to
hundredths of a millimetre:

```
77 pt × 0.003897 = 0.3 pt  →  Rounded(0.3) = 0  →  Length.FromMm100(0)
```

Every run in the body is laid out, shaped and placed at an em of **zero**, and the page receives
no text-showing operator for it at all. This is not "too small to read"; it is silent loss, and
nothing downstream of the fit can tell the two apart.

Traced through a temporary `Console.Error.WriteLine` in `Solve` (kept as
`SlideAutofit.traced.cs` in scratch, deliberately not committed — it is a probe, not code):

| slide | box | grid font height | `bestFont` the search answered | scaled em |
|---|---|---:|---:|---:|
| 5 | 1152128 EMU (90.7 pt) | 60.009 pt | 0.006666 | **0** |
| 6 | 1152128 EMU | 87.987 pt | 0.003410 | **0** |
| 12 | 1152128 EMU | 76.989 pt | 0.003897 | **0** |
| 4 | 1152128 EMU | 43.994 pt | 0.886 | 1376 |

Slide 4 renders because its overflow is small, not because it states `fontScale="90000"`.

### The brief's steer was wrong, and measurably so

> *"a stated scale is making us discard the body entirely"*

**Refuted.** `@fontScale` is never read on these shapes. `Scaling.Stated` is reached only when
`body.AutoFit` is false, and `a:normAutofit` sets it true — so the stated 25000 goes nowhere near
the answer. What makes 25000 the marker of the three failing slides is a correlation, not a
mechanism: a deck whose author had to scale to a quarter is a deck whose overflow is large enough
to push *our* bisection off the bottom of its own range. The earlier round's finding that
26.2.4.2 ignores the stated scale entirely (`probes/slides-text-01/results.md`) survives intact
and is not contradicted by anything here.

## 2. What the reference does: it stops at a quarter and overflows

Read off the banked 26.2.4.2 PDF rather than inferred:

| slide | runs state | reference draws | ratio |
|---|---|---|---|
| 12 | 77 pt | `/F1 18.992 Tf` | 19 / 77 = **0.2468** |
| 5 | 60, 52 pt | glyph boxes 18.3, 15.9 → 15, 13 pt | **0.250** |
| 6 | 88, 72, 60, 28 pt | 26.8, 22.0, 18.3 → 22, 18, 15 pt | **0.250** |

Every one is stated × 0.250 put through `roundToNearestPt`. That is `constScaleLevels`' **last
row**, `{0.250, 0.800}`, and its defining property: the walk stops there whether or not the text
fits, so the placeholder is drawn at a quarter and allowed to run past its own bottom edge. Both
blind reviewers of the fixed pages independently reported the reference's last line being cut off
at the page boundary — that overflow is the rule working, not a defect.

## 3. The fix

Two constants and one clamp in `SlideAutofit.cs`:

```csharp
private const double FitFloor        = 0.250;
private const double FitFloorSpacing = 0.800;
…
if (bestFont < FitFloor) return new Scaling(FitFloor, FitFloorSpacing, true);
```

The search is left free to walk below the floor while it brackets — a degenerate measurement down
there is precisely what tells it to climb back up — but it may not *answer* with a scale the
reference would never have reached, and it may not answer with `Scaling.None` either: an
overflowing body still draws, at the floor, overflowing.

**Why the floor and not the whole twelve-level table.** The table is the correct rule and it is
landing separately on `wt-slides-text`, whose round established it against this same binary. Two
independent ports of one method would collide for no gain. The floor is the one row of that table
this defect turns on, it is independently reference-grounded above, and a correct port of the full
walk **subsumes** it rather than contradicting it — 0.250 is the table's last row. The file's
class remarks, which still claimed 24.2.7.2 was "the installed `soffice`", are corrected to say
what the container actually holds.

**What was deliberately *not* added:** a guard against a zero em as such. At the floor a run
smaller than 2 pt still rounds to zero — and so does the reference's, by the same
`roundToNearestPt`. Reproducing that is faithful; special-casing it would not be.

### It reproduces the reference exactly

| slide | ours after | reference |
|---|---|---|
| 5 | `13.011`, `14.9953` | 13, 15 pt |
| 6 | `14.9953`, `18.0`, `21.9969` | 15, 18, 22 pt |
| 12 | `18.9921` | `18.992` |

## 4. Reach, measured from what resolves

All 163 slides rendered twice — before and after, same tree, `SOURCE_DATE_EPOCH` set — and diffed
byte for byte. **6 of 163 decks change ink. 1 moves verdict.**

| deck | pages | words before / after / ref | verdict |
|---|---|---|---|
| `NWD-GLA-Community-Outreach-Day-Oct-2025` | 13/13 | 529 / **644** / 638 | **words → match** |
| `5b_upasana_dasgupta_-_liability_and_registration` | 15/15 | 965 / 954 / 956 | match, closer |
| `manufacturing_process_simulation_working_group_overview_2023` | 7/7 | 728 / 726 / 714 | match, closer |
| `Liturgical-Commission-2025-Convention-Presentation` | 9/9 | 344 / 344 / 344 | match, sizes moved |
| `Sector_Skills_Insights_Advanced_Manufacturing_summary_slide_pack` | 24/24 | 4587 / 4587 / 4666 | match |
| `ghgp-supply-chain-initiative_20100323_wri` | 52/52 | 3895 / 3895 / 3896 | match |

The five that do not move verdict still move **towards** the reference, and that is checkable
rather than asserted. Comparing the set of `/Tf` values each deck draws against the reference's:

| deck | sizes we stopped drawing | were they in the reference? | sizes we started drawing | in the reference? |
|---|---|---|---|---|
| `5b_upasana` | 15.9874 | no | 21.9969 | **yes** |
| `Liturgical-Commission` | 20.0126, 32.0031 | no, yes | 22.989 | **yes** |
| `manufacturing_process…` | 3.0047, 7.9937, 10.9984 | no, no, no | 3.9969, 14.9953 | **yes, yes** |
| `ghgp-supply-chain` | 10.0063 | no | — | — |

**Every size the fix introduces is one the reference draws.** Six of the seven it removes are
sizes the reference draws nowhere. That is the shape of a fix, not of a trade.

## 5. Regression

`batch-check.sh … 'slides/done-*' … 4` on the fixed tree:

```
BATCHES slides/done-001 … slides/done-015
TOTAL 144  MATCH 144  MISMATCH 0  REF-CANNOT-RENDER 0
```

**144 of 144.** Not optional and not skipped.

*(One methodological scar, recorded because it nearly cost the run: the first attempt at this
sweep was started before the tests that build an "unfixed" binary, and rebuilding the CLI
mid-sweep silently mixed two binaries into one result. It was killed and re-run from scratch on a
stable tree. A sweep and a rebuild must not overlap.)*

## 6. Looked at, blind, by four readers who had not seen the document

`page-vision`, pages 5 and 12, before and after, one fresh subagent each, forbidden to read the
repository or run a command, given the image and no numbers.

**The compositor's own warning was read first.** At 150 dpi it reported *"each half is 86% of the
size it was rendered at"* — a receipt for pixels not delivered — so both pairs were rebuilt at
**129 dpi**, where it reports 100%. At 129 dpi the reference's 19 pt body is 34 px/em, four times
the 8 px/em legibility floor, so nothing below turns on resolution.

**Before, page 12:** *"the entire remainder of the page — more than three-quarters of its height —
is blank white space. No other text, links, or content is visible anywhere else on this half."*
**Before, page 5:** *"the text content present in REFERENCE is absent in OURS."*

Among the candidate causes it was asked to name but not choose between, the page-12 reviewer
produced, unprompted and without access to the source: *"Autofit/scaling collapse: a text box with
autofit-shrink behavior could have its content scaled or clipped to zero/near-zero size in OURS,
making it invisible rather than literally absent from the document model."* An independent reading
landing on the mechanism is worth more than the trace that found it.

**After, both pages:** the body is present, section headings, names, dates and hyperlinks all
match, and the reviewers report only wrapping and vertical-position differences.

**Absence was confirmed in the operators, not in the raster.** Page 12's content stream before the
fix holds the background `re f`, one `/Im10 Do`, and a single `BT … /F2 43.0016 Tf … ET` for the
title. There is no second `BT`.

## 7. A residual this round found and did not fix

Reviewer C, on the fixed page 12: *"In OURS the two email hyperlinks each stay on a single line…
In REFERENCE both of those same hyperlinks wrap onto a second line."* Measured from the ink:

| | widest line drawn on page 12 |
|---|---|
| ours | x = 40.37 … **456.49** |
| reference | x = 40.37 … **641.53** |

The placeholder is 7848872 EMU wide — 618.0 pt, 603.6 inside its insets — and the reference fills
it to 601 pt. We never exceed 416 pt of line. The email address is one unbreakable token 346 pt
wide; **the reference breaks inside it to fill the line and we break at the space before it.** So
this is an emergency mid-token break we do not perform, not a font metric and not the ~0.1% advance
divergence, which cannot move a break by 185 pt. It is why our 644 sits above the reference's 638
rather than below. Nameable, separable, and left for a round that owns line breaking.

## 8. The stale ceiling row, and an audit of the other 36

`dotnet/raster-ceiling-pages.tsv` line 81 recorded this document's page 5 as **ours 15 / ref 5** —
the over-drawing signature that defines the class. Measured on the unfixed tree it is **ours 5 /
ref 72**. The sign was inverted: a *missing-content* defect had been filed in the list of pages the
word gate cannot win, which is the most expensive possible place to put one, because that list
exists to tell the next reader not to look.

The likeliest account is that the row was measured against **24.2.7.2**, whose autofit was the same
unbounded search we had ported — so the reference drew almost nothing there either and 15/5 was
true of a pair of renderings neither of which still exists. That cannot be checked here; the 24.2
binary is not installable in this container. It is offered as the best explanation, not as a
finding.

**All 37 rows were re-measured**, per page, with the script's own metric
(`pdftotext -f N -l N` then `split()`, 1-based pages) against the banked references.

- **No row has flipped sign.** All 36 survivors still have ours ≥ ref. **P7 is refuted** — the
  prediction was that at least two others would have inverted.
- The NWD row is **removed**: post-fix it reads ours 84 / ref 72, and the document matches the
  gate outright, so it is not a ceiling page under any reading. `TODO.raster-ceiling.md`'s counts
  go 37 → 36 and 28 slides → 27.
- **Four rows no longer clear the flagging threshold** (≥8 extra words *and* ≥25% of the
  reference's count) and are recorded in the TODO for whoever owns them, but left in place —
  below-threshold is a weaker claim than not-a-ceiling, and none of the four was validated end to
  end here:

  | document | page | as filed | measured today |
  |---|---:|---|---|
  | `f2_registro_de_aprovacao_com_pbcs_EN.docx` | 1 | 230 / 181 | 197 / 181 |
  | `EHEST-SMS-Safety-Management-Manual-V2.docx` | 43 | 396 / 229 | 231 / 229 |
  | `introduction_to_bea_tuxedo.ppt` | 26 | 30 / 21 | 23 / 21 |
  | `introduction_to_bea_tuxedo.ppt` | 38 | 84 / 63 | 72 / 63 |

**The general lesson is that this table has a shelf life.** Its inputs are our renderer *and* the
reference binary, and both have moved since it was written.

## 9. Tests

`dotnet/tests/XRay.Office.Presentations.Tests/SlideAutofitTests.cs`, two new tests, six cases:

- **`AnOverflowingBodyStopsAtAQuarterAndOverflows`** — five rows, 52/60/72/77/88 pt in a 90.7 pt
  box, the document's own geometry. Every expected value is read off the banked 26.2.4.2 PDF, not
  derived from our output.
- **`NoBodyIsEverScaledToNothing`** — the invariant, swept over 300 geometries (1–60 lines × five
  sizes): the drawn em is never zero and never below 0.24 of the stated size.

**Verified to fail against the unfixed tree**, by restoring `SlideAutofit.cs` from HEAD and
re-running: `Failed: 6, Passed: 36, Total: 42` — all six new cases fail, all 36 pre-existing ones
still pass, so they are not merely re-baselined.

Every project, run individually on the fixed tree:

| project | passed | failed | skipped |
|---|---:|---:|---:|
| Containers | 109 | 0 | 0 |
| Core | 332 | 0 | 0 |
| Markup | 259 | 0 | 0 |
| OpenDocument | 125 | 0 | 0 |
| **Presentations** | **685** | 0 | 0 |
| Rendering | 150 | 0 | 1 |
| Spreadsheets | 770 | 0 | 0 |
| Text | 349 | 0 | 0 |
| Vector | 295 | 0 | 0 |
| WordProcessing | 827 | 0 | 0 |
| **Fidelity** | **520** | **30** | **0** |
| **total** | **4421** | **30** | **1** |

Fidelity is **30 of 550, identical to the baseline** taken on the unfixed tree — same count, and
`Skipped: 0`, so the project covered everything. Rendering's one skip is
`PdfFontTests.ACffFlavouredFaceIsNotClaimedToBeTrueType`, pre-existing and unrelated.

## 10. The prediction, scored

`prediction.md`, committed at `53511f4` before any of §4 onwards was measured.

| | prediction | outcome |
|---|---|---|
| **P1** | the fix is a floor of 0.250 on the font scale — not a change to how `@fontScale` is read, not a zero-em guard | **confirmed** |
| **P2** | the drawn em then matches the reference exactly on all three slides | **confirmed**, 13/15, 15/18/22, 19 |
| **P3** | words move into the band, 620–660 | **confirmed**, 644 against 638 |
| **P4** | 3–8 decks change a byte; 1–3 move verdict | **confirmed**, 6 and 1 |
| **P5** | no regression in `slides/done-*` | **confirmed**, 144/144 |
| **P6** | fidelity unchanged at baseline | **confirmed**, 30/550 both sides |
| **P7** | at least two *other* ceiling rows have flipped sign | **refuted** — none has; four are merely below threshold |
| **P8** | subsumed by, not in conflict with, the twelve-level table | **argued, not tested** — the merge has not been attempted here, and the claim rests on 0.250 being that table's last row |

P7 is the one that mattered and it was wrong in the useful direction: the inversion is **rare**, so
this document's row was not a symptom of a systematically rotten table but a single, specific
casualty of the reference binary changing under it. Volunteering "several rows are probably stale"
would have been the comfortable answer and it is not what the measurement says.
