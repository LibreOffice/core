# The 24.2.7.2 calibration audit

`CLAUDE.md` records that the reference binary moved from **24.2.7.2** to **26.2.4.2**, and that
"individual claims calibrated to 24.2.7.2 behaviour … are now claims about a superseded binary and
each needs one re-check before it is relied on". It named three. There are **50 such sites in 30
files**, and this is the list.

It exists because round 52 (slides) stopped treating that sentence as advice. `SlideAutofit.cs`
carried four of these sites; its remarks *said* it was a port of 24.2.7.2's bisection and *said* to
check the reference version first. 25.2 had replaced the search with a twelve-row
`constScaleLevels` table. Re-checking that one site was worth **−155.40 `abs_ink`, −11.1% of the
whole slides track** — where the two preceding rounds moved −10.34 and −15.33.

**So the prior on these is not "probably still fine".** **Two of the first five re-checked were
wrong**, and both announced themselves in their own comments.

## Counting them: use `git grep`, not `grep -r` — and count the marker, not the string

```sh
git grep -n '24\.2\.7' -- 'dotnet/src/**/*.cs' | grep -v '24\.2\.7-audit'          # open sites
git grep -c '24\.2\.7-audit' -- 'dotnet/src/**/*.cs' | awk -F: '{s+=$2} END{print s}' # done
grep  -rn '24\.2\.7' dotnet/src --include=*.cs                                       # DOUBLE — never use
```

`grep -r` returns exactly twice as many hits and files as `git grep`, because every project under
`dotnet/src` has a lower-case alias directory entry on this case-insensitive mount (same inode,
link count 1, untracked, invisible to `git ls-tree` and `git status`). See `CLAUDE.md`
§ "This container". **Never `rm` one — it unlinks the source tree.**

**And the string alone is not the metric, because it self-corrupts.** The first rounds to work this
list annotated their sites with sentences like *"…was calibrated to 24.2.7.2 and has been
re-checked"*, which **adds** matches for the very string being counted: the list appeared to grow
from 48 to 50 while it was being worked. So a re-checked site carries an explicit marker, and
progress counts the marker:

```
[24.2.7-audit: VERIFIED  <date>, <round> — …]     the claim still holds on 26.2.4.2
[24.2.7-audit: FIXED     <date>, <round> — …]     it did not hold, and the code now matches
[24.2.7-audit: WRONG     <date>, <round> — …]     it does not hold and the code still does not
[24.2.7-audit: UNDECIDED <date>, <round> — …]     the probe could not separate it; say why
```

**A marker is a claim like any other and can be wrong in either direction.** One site was marked
`WRONG` and later verified correct; another was `UNDECIDED` and later settled. Re-marking is
normal and the date plus round is what makes it followable.


**The running score is one wrong in eleven** (see the outcomes table below), and the useful reading
is not "these are fine" but *"a site cannot be trusted either way until a probe has been pointed at
it"*. A verified site is worth as much as a
broken one, because it is the only thing that stops the next round paying for the same probe.

| project | site | round | outcome |
|---|---|---|---|
| `XRay.Office.Presentations` | `Layout/SlideAutofit.cs` (4 sites) | 52 | **WRONG** — −155.40 `abs_ink`, 11.1% of the slides track |
| `XRay.Office.Spreadsheets` | `Layout/SheetFonts.cs` (2 sites) | 53 | **verified 26.2.4.2, 2026-08-21** — 30 of 30 authored column widths exact to 0.001 pt |
| `XRay.Office.Spreadsheets` | `Layout/SheetGeneralWidth.cs` | 53 | **verified 26.2.4.2, 2026-08-21** — 27 of 27, every `###` threshold crossing |
| `XRay.Office.Spreadsheets` | `Layout/SheetDeviceUnits.cs` | 53 | **verified 26.2.4.2, 2026-08-21** — 45 of 45 within 0.1% relative |
| `XRay.Office.Spreadsheets` | `Layout/SheetOptimalRowHeights.cs` | 54 | **verified 26.2.4.2, 2026-08-21** — 30 of 30 wrapped row heights within half a twip, control at 300 |
| `XRay.Office.Spreadsheets` | `Layout/SheetPageDecoration.cs` | 55 | **WRONG, half of it** — the zero-band guard holds; "draws at every stated band above zero" does not. Reported, not fixed; a `Math.Min` defect the probe found beside it *was* fixed |
| `XRay.Office.Spreadsheets` | `Layout/SheetPageDecoration.cs` | 56 | **the same site again, and now implemented.** There is no threshold: the band is a *clip rectangle* and the apparent threshold is `ascent - inkAscent`. Round 55's own 8 pt bracket falls out of it with nothing fitted; its 20 pt bracket is **refuted** (4.32 pt draws) |
| `XRay.Office.Spreadsheets` | `Layout/SheetShapeText.cs` (`DefaultSize`) | 56 | **verified 26.2.4.2, 2026-08-21** — 12 pt by two instruments, the flat-ODS export and the rendering, with the 1100 control first and an 1800 box to separate 12 from 18 |
| `XRay.Office.Spreadsheets` | `Layout/SheetNotes.cs` (column-major note order) | 56 | **verified 26.2.4.2, 2026-08-21** — `Hazard Analysis Template.xls` still lists D1 F2 H2 J2 L1 N2 P2 R2, and reading order would put L1 second |
| `XRay.Office.Spreadsheets` | `Ooxml/XlsxNoteCaptions.cs` (a VML anchor's offsets as 96-dpi screen pixels) | 57 | **verified 26.2.4.2, 2026-08-21** — control first (offset 0 lands on the row grid to 0.012 pt), then three steps of 14.998 pt for 20 px, i.e. 96.0 dpi. **And the probe exposed a rule the site does not state**: the offset is *clamped* to the anchored cell's own extent, which we do not do — 5 anchors of 365 in one document of fifteen |
| `XRay.Office.Spreadsheets` | `Layout/SheetText.cs` (`MeasurePixels`, the per-glyph pixel rounding) | 58 | **verified 26.2.4.2, 2026-08-21** — the turned-cell fixture round-tripped through the installed binary gives **216 of 216** row heights unchanged, all 72 quarter-turn heights among them. The discriminator is in the fixture and not in an argument: rounding the *total* instead of per glyph breaks **26 of its 36 cases** through `verify-test.sh`, and the reference moved none of them. **The last unverified `XRay.Office.Spreadsheets` site.** |

| `XRay.Office.Ooxml` | `DrawingML/DrawingFill.cs` :115 (`a:lum` as a whole per cent) | 61 | **verified 26.2.4.2, 2026-08-21** — the division is integer and truncating. Five authored `.docx` fixtures read off the reference alone: `70999/-70999` renders byte-for-byte as `70000/-70000` (mean channel 251.842/248.718/251.841 on both), which only truncation gives, while `69500/-69500` renders differently, which rounding could not. **The two cases fail under the two readings in opposite directions**, so neither is a one-sided test, and two controls ran first (`71000` differs; no `a:lum` differs from everything). `probes/sheets-r61/audit_lumpercent.py` |

| `XRay.Office.WordProcessing` | `Ooxml/DocxLayoutSource.cs` :777 (`w:pPr/w:rPr` is the paragraph *mark's* formatting) | 63 | **verified 26.2.4.2, 2026-08-21** — all three claims, on the reference alone, each with a control: a `Heavy` paragraph whose mark says `<w:b w:val="0"/>` is drawn **Bold** while the same style with the *run* saying it is drawn regular; a mark saying `<w:b/><w:sz w:val="48"/>` leaves the text 10.00 pt upright where the run gives 24.00 pt Bold; and an empty paragraph whose mark states 36 pt has a baseline pitch of 52.90 against a control's 23.00, which is 36 x 1.15 exactly. `probes/words-r63/audit_markstyle.py`. **The last open site under `XRay.Office.WordProcessing` that a corpus document can reach** — see below |

| `XRay.Office.Text` | `Layout/LineBreaker.cs` :473 (a hyphen never opens a number) | 58 | **WRONG, and now FIXED** — the sentence was inverted and the code implemented the inversion. 26.2.4.2 lets a hyphen open a number everywhere *except* after a digit, which is where i#83229 puts the break back. Ten authored packages, two controls, no width tuning: 7 of 10 agreed before, 10 of 10 after |

| `XRay.Office.Presentations` | `Ooxml/PptxSlideLayout.cs` :763 (table cell line spacing) | 54 | **verified 26.2.4.2, 2026-08-21** — 6 of 6 stated sizes put the reference's first baseline one em below the cell's top |
| `XRay.Office.Presentations` | `Layout/SlideDrawing.cs` :341, :360 (a picture frame's own fill) | 56 | **verified 26.2.4.2, 2026-08-21** — three byte-identical renderings for the package-entry arm, and a discriminating pair (108 304 red pixels inline, 0 zipped) for the inline arm |
| `XRay.Office.Presentations` | `Ooxml/PptxSlideLayout.cs` :1591 (an `a:fillToRect`'s focus) | 59 | **two of three rules verified, the third WRONG and now FIXED.** The clamp and the truncation to whole per cent both still hold on 26.2.4.2 — 0.5% lands on 0 and 1% does not — but the branch they fed does not: a `path="circle"` focused on a **corner** is `draw:style="radial"` there, where the superseded binary made it a 45° linear ramp. Re-run of round 39's *own* four-arm fixture through the reference's flat-ODF export, all four arms radial. Worth **−54.26 `abs_ink` and −244.20 differing pixels** on `Wildlife for REDAC September 11.pptx` alone; 67 corner-focus circle paths in 7 documents |
| `XRay.Office.Presentations` | `OpenDocument/OdpSlideLayout.cs` :302 (the ODF half of the same claim) | 55 | **WRONG — reported, not fixed.** The rule is no longer fixed at all: 26.2.4.2 obeys `style:font-independent-line-spacing` as stated, one em when true and the face's own 0.903 em when absent. **Not fixed because the slides corpus holds no ODF presentation** — 251 `.pptx` and 51 `.ppt` — so the change would ship unmeasured |

**A trap the probe harness cost half an hour to find, before anyone else pays for it again.** A
minimal authored `.xlsx` with no `<cellStyles>` element has its `cellXf` font **discarded entirely**
by LibreOffice, so a font-size probe reads a constant 10 pt on the reference at every stated size
and reports 46 of 48 cases wrong. `dotnet/probes/sheets-r53-totalsrow/audit_mkwb.py` is a fixture
generator that is known to be read correctly by 26.2.4.2; start from it. Confirm any new fixture by
`soffice --convert-to fods` and reading `fo:font-size` back before trusting a single measurement.

## The first site on this list that a probe settled without rendering our own side — round 61

`DrawingFill.cs`'s `a:lum` claim is about **what the reference reads out of a file**, not about what
we draw, so the whole discriminator lives on one side. Round 61 authored five minimal `.docx`
packages around a saturated red/blue checkerboard and compared the reference's own mean channel
across them; our renderer never ran. Two of the five cases are **controls whose answers were known
before the probe**, and the two live cases fail under the two candidate readings *in opposite
directions* — `70999` agrees with `70000` only under truncation, `69500` agrees with it only under
rounding. A probe that can only fail one way is worth much less than one that can fail either.

The claim held: the division is integer, the washout branch tests the integer for exactly 70 and
−70, and C#'s truncation toward zero matches C++'s on the negative contrast without a second arm.
**`XRay.Office.Ooxml` is now one of one re-checked.**

## Round 63 (sheets): the counters again, and the site round 62 named had already been done

Re-derived at base `43142b73ccf` with this file's own commands: **37 open hits in 26 files** —
unchanged — and **34 marker lines, 29 `VERIFIED`, 4 `FIXED`, 1 `WRONG`, 0 `UNDECIDED`**, where
round 62 recorded 31 / 26 / 4 / 1. The three extra `VERIFIED` arrived with the slides and words
branches merged beside it. **Sixth time a quoted number here has failed to reproduce.** The open
count is again unchanged, so the drift is in the marker column alone.

**Round 62 (sheets) named `XRay.Office.Core/Graphics/GlyphRun.cs` :347/:369 as the next site, and it
already carries a `VERIFIED` marker of the same date from slides-r62.** The round that named it
and the round that did it were the same round; the sheets write-up could not know, because the
merge had not happened when it was written. **A "next site" is only useful if it is checked against
the file at the moment of writing** — which is what this file already says about counters and now
says about sites too.

What is left there is claims **two and three**, and the marker itself says why they are open: no
corpus document states a lone `a:lum` brightness or a non-washout pair, so they can only be settled
by authored fixtures. `probes/sheets-r61/audit_lumpercent.py` is the harness. Three arms —
brightness alone, contrast alone, and both — separate them outright, because
`BColorModifier_RGBLuminanceContrast` spends the whole brightness *after* the contrast
(`basegfx/source/color/bcolormodifier.cxx`:387-405) and `Bitmap::Adjust`'s `msoBrightness` branch
spends half before and half after (`vcl/source/bitmap/bitmap.cxx`:1694-1698), and the two differ on
every non-zero pair. `XRay.Office.Rendering/Images/RasterImageDecoder.cs`:239 is the same branch from
the decoder's side and belongs in the same run.

**Round 63 ran no re-check** and says so rather than filing a hurried one: its budget went to 328
authored decks, four whole-track sweeps and a cross-track control.

## The next site, named with the command rather than from prose — round 62 (sheets)

Round 62 ran **no** re-check: its budget went to the chart advance-width law and two whole-track
sweeps, and the audit's own rule is that a site cannot be trusted either way until a probe is
pointed at it — so saying "none this round" is the honest entry rather than a hurried one.

What it did do is name the next site *from the file* and not from a write-up, which is the failure
this list records twice already. `git grep -n '24\.2\.7' -- 'dotnet/src/XRay.Office.Core/**/*.cs'
| grep -v '24\.2\.7-audit'` gives exactly two hits, both in
**`XRay.Office.Core/Graphics/GlyphRun.cs`** at :347 and :369, and they are **not** the claim round 61
verified. Round 61 settled `XRay.Office.Ooxml/DrawingML/DrawingFill.cs`'s reading of `a:lum` as a
*whole per cent* — the truncation and the exact ±70 test. `GlyphRun.cs` states three further claims
about what is then *done* with the pair, and only the first overlaps:

1. 70/−70 is thrown away and replaced by `ColorMode_WATERMARK`'s fixed **+50 luminance, −70
   contrast** — round 61's five fixtures bear on this and do not settle it: `70/−70` and `71/−71`
   render at mean channel 251.842/248.718 and 251.117/247.268, which separates them but does not by
   itself show the *fixed* offsets;
2. both non-zero otherwise goes through `Bitmap::Adjust`'s `msoBrightness` branch, half the
   brightness before the contrast and half after;
3. one alone goes through `BColorModifier_RGBLuminanceContrast`, whole brightness after.

Claims 2 and 3 are two different arithmetics on the same inputs, so **one fixture set separates
them outright**: brightness alone, contrast alone, and both together, compared against the two
formulas' predicted mean channel. `probes/sheets-r61/audit_lumpercent.py` already authors the
packages and reads the reference's mean channel, so the re-check is that generator plus two
formulas. `XRay.Office.Rendering/Images/RasterImageDecoder.cs`:239 is the same `Bitmap::Adjust`
branch seen from the decoder's side and should be re-checked in the same run.

## A named next site that was not one — round 60 (sheets)

Round 59 named **`XRay.Office.Core/Graphics/GlyphRun.cs`** as the next site to re-check, "precisely
because [a] vertical-metric divergence is live there". **It is not, and one `git grep` separates
them.** That file's two open hits are at lines 347 and 369 and both belong to `LuminanceRecolour`:
`a:blip/a:lum` brightness and contrast, PowerPoint's washout mapping to `ColorMode_WATERMARK`, and
`Bitmap::Adjust`'s `msoBrightness` branch. There is **no vertical-metric claim in the file at all**.

The seat of the divergence was `SheetBandText.Ungridded` in `XRay.Office.Spreadsheets`, which carries
no marker because it was never a 24.2.7.2 claim — it was a claim about `chart2` made on this
project, and round 60 measured it wrong (a chart's text is quantised onto a **96 dpi** device;
`probes/sheets-r60/`).

**The lesson for this list**: a site named from the *prose* of a round's own write-up is not a site
named from the file. Name it with the command at the top of this file, or the next round pays for
the mismatch. Two rounds carried this one.

It is also worth recording that four of `XRay.Office.Text`'s open hits are prose inside
**already-marked** sites — `MeasuredParagraph.cs` and `SystemFontResolver.cs`, both `VERIFIED` by
the words track — so the open count still over-reads for exactly the reason this file's header
gives.

## Progress

| round | sites re-checked | outcome |
|---|---|---|
| 52 (slides) | `SlideAutofit.cs`, 4 sites | **WRONG.** −155.40 `abs_ink`, −11.1% of the track |
| 53 (slides) | `SlideTextLayout.cs`, **6 sites** | **all six still correct** — and the probes written to check them found a *fifth* branch none of the six described, worth another two fixes' worth of baseline accuracy. See `probes/slides-r53/results.md` § "The 24.2.7.2 audit" |
| 54 (sheets) | `SheetOptimalRowHeights.cs`, the `WrappedHeight` fit | **still correct** — 30 of 30 authored wrapped rows within half a twip on 26.2.4.2, read twice over (marker deltas in both PDFs, and the reference's own `fods` `style:row-height`) with the site's own 300-twip figure as the control. `probes/sheets-r54/audit_rowheight.py` |
| 63 (words) | `Ooxml/DocxLayoutSource.cs` :777, three claims in one site | **all three verified** — and the useful half of this entry is the *classification* beside it. `XRay.Office.WordProcessing`'s eleven open hits are not eleven open sites: five are prose inside an existing marker (`WordCompatibility.cs`, `WriterPoolSpacing.cs` x2, `WordStyles.cs` :316, and the `Paginator.cs` note that *supersedes* a 24.2.7.2 claim), and five more state a 26.2.4.2 measurement in the same comment as the 24.2.7.2 one they replaced — two of those in the ODT and RTF readers, whose formats have **no witness in this corpus** (271 `.docx`, 66 `.doc`, 0 `.rtf`, 0 `.odt`). That left exactly one, and it is now closed. **The header's warning that the string over-reads is quantified here for the first time: 11 hits, 1 reachable open site.** `probes/words-r63/audit_markstyle.py` |
| 58 (words) | `XRay.Office.Text/Layout/LineBreaker.cs`, the number-opening hyphen | **WRONG, and fixed.** The site's own five worked examples were the evidence and three of them are false on 26.2.4.2 — `E-22`, `$-22` and `A -222` do **not** break after the hyphen, and neither do `abc-22` or a hyphen opening the text; only `10-19` and `5-2` do. One rule covers all ten authored cases: **a hyphen opens a number unless a digit precedes it.** The method needed no width tuning — each token is longer than its line in a six-character column, so *where* it breaks separates the rules outright, and two controls with known answers ran with it (`abcd-efghij` breaks after its hyphen on both sides; `(222` does not break after the bracket). `probes/words-r58/audit_hyphenbreak.py`. **The lesson is the one this file already carries and it arrived from the other side**: the old comment's worked examples were what made it checkable, and they were what refuted it. A site that shows its measurements is worth more than one that states a conclusion, even when the measurements are wrong |
| 57 (sheets) | `Ooxml/XlsxNoteCaptions.cs`, the VML anchor's units | **verified** — 96 dpi exactly on 26.2.4.2, the control first: an anchor with a zero row offset lands on the row grid to 0.012 pt, and offsets of 20, 40 and 60 px step the exported annotation by 14.998, 14.998 and 14.990 pt. Seventy-two dpi would have stepped it by 20 and EMUs by nothing. **The probe's first cut read "neither" at every step and was measuring a clamp, not a law**: its rows were 20 pt (26.7 px) and its offsets 48, 96 and 144, so all three saturated at exactly one row. Sixty-point rows separate the rule from the clamp. `probes/sheets-r57/audit_vmlanchor.py`. The clamp itself is a real divergence — we do not clamp — with 5 anchors of 365 to its name (`census-vmlclamp.py`), recorded at the site and not implemented |
| 58 (sheets) | `Layout/SheetText.cs`, `MeasurePixels` | **verified** — 216 of 216 turned-cell row heights reproduce on 26.2.4.2, including every quarter-turn one, where `ScPatternAttr::GetCellOrientation` puts the string's *width* straight into the row height so nothing stands between `GetTextWidth` and the number. Four of the eighteen distinct widths — the twelve-point ones — differ by up to 1.4% between the two readings of the rounding, so the discriminator is built in; and the mutation that rounds the total instead fails 26 of 36 cases. `probes/sheets-r58/audit_rotatedwidth.py`. **`XRay.Office.Spreadsheets` is now ten of ten re-checked, nine correct** |
| 56 (sheets) | `SheetShapeText.cs` `DefaultSize`; `SheetNotes.cs`'s note order; and `SheetPageDecoration.cs` a second time | **two verified, one WRONG and now fixed.** The shape-text default is 12 pt on 26.2.4.2 by two instruments (`probes/sheets-r56/audit_shapetext.py`), the note order is unchanged, and round 55's "text-fit threshold" at `SheetPageDecoration.cs` turns out to be a **clip rectangle** — `PrintHF` sets one of `Rectangle(aStart, nHeight - nDistance)` and `DrawText_ToPosition` emits nothing for an area whose ink misses it. Round 55's 8 pt bracket falls out of `ascent - capHeight` with no free parameter; its 20 pt bracket does not reproduce. **A site re-checked twice in consecutive rounds, and the second pass replaced a fitted law with a mechanism** — the same argument as re-marking an already-`VERIFIED` site |

| 59 (slides) | `PptxSlideLayout.cs` :1591, the `a:fillToRect` focus — **three rules in one site** | **two verified, one wrong and fixed.** The value of re-running a *previous round's own fixture* rather than authoring a new one: the four arms already existed and already separated the three rules, so the re-check cost one render and read the answer off `soffice`'s own `fodp` export. The rule that broke is the one no argument would have singled out — the other two are arithmetic and survived a version bump, and the one that did not is a *branch* whose existence depended on the arithmetic. **A site can be two-thirds right and still be shipping a 54-point defect.** `probes/slides-r59/results.md` § 5 |
| 56 (slides) | `SlideDrawing.cs`, the picture-frame fill, **2 sites** | **still correct, both halves.** The package-entry arm is three byte-identical page images of one corpus deck under three different stated fills; the inline arm needed a **discriminating pair** — the same 306 kB EMF inline and then moved to `Pictures/` by the reference's own exporter — and the two differ by 108 304 red pixels against 0. `probes/slides-r56/audit_picturefill.py` |
| 55 (sheets) | `SheetPageDecoration.cs`, the header/footer band guard | **WRONG in half.** The zero-band claim holds and every negative band too; "the reference draws the footer at every stated band above zero" is false — nothing is drawn at 0.72 or 1.44 pt of 8 pt text, and the threshold *scales with the point size* (1.44–2.16 pt at 8 pt, 4.32–5.76 pt at 20 pt). A text-fit rule, not a constant. Reported and not implemented: four corpus worksheets have a positive band under 6 pt and all four pass today. The probe also found a **real 18 pt body displacement** at a *negative* band, which is fixed. `probes/sheets-r55/audit_pagedecoration.py`, `census-bands.py` |

| 54 (slides) | `PptxSlideLayout.cs` :763, 1 site | **still correct** — and the version bump is *why* it is correct: the site records that 24.2.7.2 drew a cell's first baseline at the face's own 0.907 em, `a47776a938c` (tdf#165521) removed the leading for cells, and 26.2.4.2 draws it at one em on 6 of 6 stated sizes from 10 to 40 pt. A site whose comment says "the running binary disagrees with its own C++" is the highest-prior kind on this list |

**48 → 42 sites, 30 → 29 files.** Two in one: a re-check is worth running even when it comes back
clean, because authoring the probe is what exposes what the site does *not* say. Round 53's
`make-linespace-probe.py` confirmed all four of `SlideTextLayout.cs`'s EditEngine sites and, in the
same rendering, showed that `SvxLineSpaceRule::Fix` and `::Min` — the two arms *before* the ones
those sites describe — were not transcribed at all. That was a 9.58 pt vertical displacement on
every paragraph stating an exact line height, 769 of them in 23 documents.

It also promoted one recorded *divergence* from a judgement to a measurement: `LineSpacingRule`'s
50% clamp is Writer's and not EditEngine's, and at 40% the reference draws `fround(0.40 × natural)`
rather than clamping. 26.2.4.2 has no such clamp either.

## The open count over-states by about four times, and here is the honest one

Found by round 63 (words), which noticed that `XRay.Office.WordProcessing` shows **11 open hits** and
has **one reachable open site**. Measured corpus-wide:

| project | raw open hits | hits in files carrying **no** marker |
|---|---:|---:|
| `XRay.Office.WordProcessing` | 11 | **3** |
| `XRay.Office.Spreadsheets` | 9 | **4** |
| `XRay.Office.Presentations` | 8 | **0** |
| `XRay.Office.Text` | 5 | **1** |
| `XRay.Office.Core` | 2 | **0** |
| `XRay.Office.Rendering` | 1 | **1** |
| `XRay.Office.Ooxml` | 1 | **0** |
| **total** | **37** | **9** |

**A raw hit is not an unverified claim.** Most of the 37 are one of:

- **prose inside or beside a site that has already been marked** — the marker records the outcome and
  the surrounding comment still names the old binary because that is the history being described;
- **historical narrative** about a superseded port, which is not a claim about behaviour at all;
- **a site in a reader with no witness in this corpus** — two of `XRay.Office.WordProcessing`'s sit in
  the ODT and RTF readers, and the words corpus holds **no `.rtf` at all**.

```sh
# hits in files that carry no marker — the honest backlog
for p in dotnet/src/XRay.Office.*; do
  for f in $(comm -23 <(git grep -ln '24\.2\.7' -- "$p/**/*.cs" | sort) \
                      <(git grep -ln '24\.2\.7-audit' -- "$p/**/*.cs" | sort)); do
    git grep -c '24\.2\.7' -- "$f"
  done
done | awk -F: '{s+=$2} END{print s+0}'
```

**This is the fourth time this file's metric has needed correcting** — it conflated hits with files,
it grew while being worked, a marker's own prose put a cleared site back into the count, and now it
turns out to over-state the backlog four-fold. **Every one of those was found by a round, not by the
file.** The lesson has stopped being about this metric: **a count of mentions is never a count of
claims, and the gap grows every time the thing is worked on.**

## The size of the list — **run the command, do not read a number**

This file has carried a hand-maintained count three times and it has been wrong three times: it
conflated hits with files, it grew while the list was being worked, and a marker's own prose put a
cleared site back into the open count. **The count is not maintained here. It is computed.**

Recomputed from the tree with `git grep`, excluding lines that carry a `[24.2.7-audit: …]` marker.
**Hits and files are different numbers and an earlier version of this table conflated them**, which
a round caught.

| project | open hits | reaches |
|---|---:|---|
| `XRay.Office.WordProcessing` | 11 | words |
| `XRay.Office.Spreadsheets` | 9 | sheets |
| `XRay.Office.Presentations` | 9 | slides |
| `XRay.Office.Text` | 6 | **all three tracks** |
| `XRay.Office.Core` | 2 | **all three tracks** |
| `XRay.Office.Rendering` | 1 | **all three tracks** |
| `XRay.Office.Ooxml` | 1 | **all three tracks** |
| **total** | **39** | |

Re-derived at round 57's tree, with the commands below. **The `42` and the `Presentations 11` this
table carried did not reproduce and are corrected — for the fifth time in this file's history.**
At round 57's base commit the same commands give **40** (WordProcessing 11, Spreadsheets 10,
Presentations 9, Text 6, Core 2, Rendering 1, Ooxml 1), so two of the three figures were already
wrong when they were written and only the sheets column moved this round. Run the commands. **The per-file column is gone**: it said `XRay.Office.Text` 8 where
the command gives 6 and the "orientation only" table below already said 6, and a file count that
disagrees with itself two tables apart is worth less than the command. `git grep -l` over the open
sites gives **23** files, not 26.

**The two figures above did not reproduce and are corrected.** This table said `44` open and the
paragraph below said `12` markers. Re-derived at commit `e11ee5ac386` with the commands this file
itself gives: **42 open, 13 marked**. A figure quoted rather than re-derived decays — the same
lesson this file records about round 53's "73 of 337", arriving again in the file that records it.

**Re-derived at round 62's base `337bc9fe17c`** (the merge of slides r60, sheets r61 and words
r61), with this file's own commands: **37 open hits in 26 files**, and **31 marker lines — 26
`VERIFIED`, 4 `FIXED`, 1 `WRONG`, 0 `UNDECIDED`**. Round 61 recorded 26 marker lines (21/4/1) and
that figure **does not reproduce here**: it was derived at that round's own pre-merge base, and the
slides and words branches merged beside it carried five more marker lines in. The open count is
unchanged at 37/26, which is why the discrepancy is in the marker column alone. *This is the fifth
time a number in this file has failed to reproduce, and every one of them was a number that was
quoted rather than computed — including this replacement, which is only worth anything until the
next merge.* Per project, open hits: WordProcessing 11, Spreadsheets 9, Presentations 8, Text 5,
Core 2, Rendering 1, Ooxml 1.

Marked so far, re-derived at round 57's tree with the commands below: **19** marker lines —
**16 `VERIFIED`, 2 `FIXED`, 1 `WRONG`, 0 `UNDECIDED`**. At round 57's base the same commands give
**18** — 15 / 2 / 1 / 0. (This paragraph said `14 — 9 verified, 3 wrong, 1
undecided, 1 half-wrong`, and none of those four figures reproduces; at round 56's base it was
**15 lines, 13 verified, 2 wrong, 0 undecided**. The same failure the file records twice already:
a hand-maintained count decays. Marker *lines* are what the command counts, and a site re-marked
in a later round adds a line without adding a site, which is why this number can exceed the number
of sites re-checked.)

**A bug in this file's own third command.** `git grep -c "audit: X" -- …` prints `path:count` over
the working tree, so `awk -F: '{s+=$2}'` is right — but add a tree-ish (`git grep -c … <commit> --
…`) and the output becomes `commit:path:count`, where `$2` is the *path* and the total silently
comes out **0**. Round 56 read "0 markers at the base commit" from it before noticing. Use `$3`
when comparing against a commit.

**The open count *does* fall when a site is FIXED and its old prose was wrong.** Round 58's
`LineBreaker.cs` site did not merely have a stale figure — its four worked examples were false, so
they were replaced by the ones measured on 26.2.4.2 rather than kept as a record of what the code
was fitted to. The commands give **38 open** at that commit against 39 before it, and
`XRay.Office.Text` 5 against 6. There is nothing to preserve in a sentence that is wrong about the
binary it names.

The **open** count does not fall when a site is verified, and that is deliberate: the sentence that
names 24.2.7.2 stays, because it records what the figure was fitted to. Round 55's marker is the
fourteenth and the open count held at 42. Read the two numbers as "how many sites still carry an
unchecked 24.2.7.2 claim" and "how many have been checked", not as a total and a remainder.

Reproduce both numbers with:

```sh
# open sites — a hit that is not itself a marker line
git grep -n '24\.2\.7' -- 'dotnet/src/**/*.cs' | grep -v '24\.2\.7-audit' | wc -l

# per project
for p in dotnet/src/XRay.Office.*; do
  n=$(git grep -n '24\.2\.7' -- "$p/**/*.cs" | grep -vc '24\.2\.7-audit')
  [ "$n" -gt 0 ] && printf '%4d  %s\n' "$n" "$p"
done | sort -rn

# done, by outcome
for k in VERIFIED FIXED WRONG UNDECIDED; do
  printf '%-10s %s\n' "$k" "$(git grep -c "audit: $k" -- 'dotnet/src/**/*.cs' | awk -F: '{s+=$2} END{print s+0}')"
done
```

**`WRONG` and `FIXED` are different states and the count must separate them.** A `WRONG` marker
whose prose said "fixed" was indistinguishable from one whose prose said "reported, not fixed", so
no command could answer the question that actually matters — *how many claims known to be false are
still shipping in the code*. `FIXED` was added for that, 2026-08-21, and the two already-repaired
sites were re-marked. **`WRONG` now means the defect is still live.**

**A marker's prose must not name `24.2.7.2`.** Round 54 wrote a marker whose second line did, which
pushed the open count *up* at the moment a site was cleared — the file's own trap, sprung by the
file's own convention. Say "the superseded binary", or name **26.2.4.2**, and put the old version
only inside the `[24.2.7-audit: …]` bracket if it is needed at all.

**No snapshot is kept here, deliberately.** One was, and three consecutive rounds had to re-derive
it and found it wrong each time — `42` against `40` against `39`, and a per-project figure wrong
alongside them. A number in this file is a number someone will quote instead of running the command,
and the whole point of the marker convention is that the command is cheap. **Run it.**

If you want a figure for a report, run the command and date it in the report, not here.

| project | open hits |
|---|---:|
| `XRay.Office.WordProcessing` | 11 |
| `XRay.Office.Spreadsheets` | 10 |
| `XRay.Office.Presentations` | 9 |
| `XRay.Office.Text` | 6 |
| `XRay.Office.Core` | 2 |
| `XRay.Office.Rendering` | 1 |
| `XRay.Office.Ooxml` | 1 |

Total open **40**, marked **17** (14 verified, 3 wrong) — computed with the commands above on
2026-08-21 at the end of round 56, and **not maintained**: run them. Round 56 marked
`SlideDrawing.cs` and the open count held at 40, as it is meant to.

**Round 56's marker is a second data point for the discriminating-pair method**, which round 55
introduced here. `SlideDrawing.cs`'s claim has an inline arm and a package-entry arm, and either
arm rendered on its own is consistent with "the fill is never drawn" *and* with "the fill is
always drawn" — only the two together, over bytes the reference itself moved from one storage to
the other, separate them. Two of the last three re-checks on this list turned on that, and in both
cases the naive single-fixture reading was available and wrong.

Two of the two the round removed are worth naming, because only one of them was a re-check.
`OdpSlideLayout.cs:302` was re-checked and marked. The other was **round 54's own marker at
`PptxSlideLayout.cs:763`, whose prose named the superseded version** and so kept a cleared site
in the open count — the trap this file describes, still live in the file it was described in.
Rewriting one clause to say "the superseded binary" cleared it. **A rule written down is not a
rule applied**; the count is the only thing that notices.

## Outcomes so far — **one** site wrong, of every one re-checked

| site | outcome |
|---|---|
| `SlideAutofit.cs` (one claim, 4 hits) | **WRONG**, fixed r52 — 25.2 replaced 24.2's bisection with `constScaleLevels`. −155.40 `abs_ink`, **11.1% of the slides track** |
| `SlideTextLayout.cs` (6 sites) | **VERIFIED** r53 — and authoring the probe exposed a chain arm all six described and none implemented, worth 769 sites' worth of fix |
| `SheetFonts.cs` (2), `SheetGeneralWidth.cs`, `SheetDeviceUnits.cs` | **VERIFIED** r53 — 30/30, 27/27, 45/45 authored cases exact |
| `SystemFontResolver.cs:406` | **VERIFIED** r53 |
| `SystemFontResolver.cs:441` | **VERIFIED** r54 — was UNDECIDED; settled with two fixtures that actually reach `DefaultFonts` |
| `SystemFontResolver.cs:657` (`GenericFallbacks`) | **VERIFIED** r54 — **was recorded WRONG by r53 and is not.** See below. Re-confirmed r55 from a fifth caller: the DOC filter *does* reach it undeclared, and it answers correctly there too |
| `MeasuredParagraph.cs` | **VERIFIED** r53 |

### The one that was recorded WRONG and was not — read this before trusting any entry here

Round 53 probed `GenericFallbacks`, found ten unrecognised families answering DejaVu **Serif** where
the code says Sans, measured that **86 of 337 words renderings disagree with the reference's font
list**, and recorded the site `WRONG`. I read that as the largest single known defect on the project
and dispatched a round to fix it.

**The site is correct. The rule simply does not live there.** Round 54 established it on **126
authored files** through the installed `soffice`, with five known-answer controls:

| filter | an unrecognised family, nothing declared |
|---|---|
| DOCX, DOC, RTF | **DejaVu Serif** — only `w:family="swiss"` moves it, to Sans; RTF's `\fnil`/`\froman`/`\fswiss`/`\fmodern` are all inert |
| ODF text, XLSX, PPTX, flat ODS | **fontconfig's own generic** — `Consolas` → DejaVu Sans *Mono* |

**The answer belongs to the filter, not the resolver.** Round 53's probe was DOCX-only: it held the
*format* fixed without noticing the format was the variable. The discriminator it lacked is that
`45-latin.conf` files 60 families under a generic and none is installed here, so `fc-match`
separates three answers rather than one.

**Had the recommended one-line change been made in `XRay.Office.Text`, it would have reflowed 202
slides and 130 sheets renderings, every one of them currently correct.** The fix belongs in
`XRay.Office.WordProcessing/Layout/WordFallbackClass.cs`, and that is where it went; the diff in
`XRay.Office.Text` is comment-only, verified by diff.

**Three lessons, all of which this project already states somewhere and none of which stopped it:**

1. **A probe that varies one thing must know what it is holding fixed.** "One variable at a time" is
   satisfied by a DOCX-only sweep and is still wrong, because the constant was the answer.
2. **An audit entry is a claim like any other.** `WRONG` earns no more trust than the comment it
   contradicts — this is § 7's "a refutation inherits no privilege from being a refutation",
   arriving for the second time in three rounds.
3. **A figure quoted rather than re-derived decays.** Round 53's "73 of 337 carry DejaVu Sans"
   **does not reproduce at any reading** — the candidates are 70, 40 and 32. Its companion figure,
   86, reproduces exactly. I repeated the 73 in a brief and in a report.
4. **A verified site can still carry a wrong sentence, and round 55 found one here.** Round 54's
   own marker said the word-processing filters "do not reach this undeclared". Three of the four
   filters it tested were not word-processing filters at all, and the DOC arm — the one it could
   not measure, because its probe was a DOCX round trip — *does* reach it. The verdict was right
   and the reason was over-general. **Re-verifying a site that is already VERIFIED found a real
   correction**, which is the argument for re-marking with a date and round rather than treating
   `VERIFIED` as terminal.

## How to work it, and the order

**The twelve shared-layer sites first** — `XRay.Office.Text` 8, `Core` 2, `Rendering` 1, `Ooxml` 1,
across 7 files.
They reach all three tracks, so one wrong calibration there is three tracks' worth of error, and
`SystemFontResolver.cs` sits upstream of the font resolution that decides 267 reference renderings.
A shared-layer re-check owes a **measurement** across the other two tracks, not an argument.

Then per-track, each track taking its own, densest file first. `SlideTextLayout.cs`'s six are the
obvious next after `SlideAutofit` — they are the same subsystem, and inter-paragraph spacing on the
`.ppt` path is already the slides track's named front.

**A re-check is a probe against the installed binary, not a reading of the C++.** The tree in this
checkout is 27.2.0.0.alpha0+, which is *also* not the reference. Authored variants, one thing
varied at a time, at least two points so a slope is fixed. Five rounds have burned predictions on
reading the source instead.

**Record the outcome at the site**, in both directions. A site re-checked and found *still correct*
is as valuable as one found wrong, and is the only thing that stops the next round re-checking it:
change the comment to name **26.2.4.2** and the date it was verified. A site left saying `24.2.7.2`
is, by this file's convention, unverified.


## Re-check log

One row per site actually re-checked, newest first. A site is only "verified" when the comment at
the site itself names **26.2.4.2** and the date — this table is the index, not the record.

| date | site | outcome | how |
|---|---|---|---|
| 2026-08-21 | `XRay.Office.WordProcessing/Ooxml/WordStyles.cs` `HasDefaultParagraphPropertiesElement` (a bare `w:pPrDefault` turns widow/orphan control on) | **VERIFIED** | round 62, `probes/words-r46/widow-orphan-default.py` re-run unchanged on 26.2.4.2 — 9 variants x 5 straddle positions, 45 renderings. The discriminating pair is `no-pPrDefault` against `empty-pPrDefault`: at 14 and 16 fillers the first puts 3 and 1 target lines on page one, the same as the `para-off` room control, and the second puts 2 and 0, the same as `para-on`. Presence with no `w:pPr` inside it is what does it. Three arms say default and not override — a `w:pPrDefault` *stating* `w:widowControl w:val="0"` is off, a paragraph stating it is off, `w:docDefaults` removed entirely is off. And `settings-on` is **inert at every filler count**, reconfirming HANDOVER §7's refutation of the document-level `w:settings/w:widowControl` on the current binary |
| 2026-08-21 | `XRay.Office.Presentations/Layout/SlideDrawing.cs` :341, :360 (a picture frame's own fill) | **VERIFIED, both halves** | round 56, `probes/slides-r56/audit_picturefill.py`. The site makes two claims and they need different fixtures. *A package entry loses the fill*: `2014BSA_Sunday_Killion.pptx` rendered as found, with the frame's `a:solidFill` changed to red, and with it replaced by `a:noFill` gives three **byte-identical** page-5 images on 26.2.4.2 — no fill reaches the page whatever the frame states. *An inline metafile keeps it*: this is the half a single rendering cannot settle, so it was checked as a **discriminating pair** — one authored flat ODP holding a 306 kB EMF as `office:binary-data` under a red frame draws **108 304 red pixels**, and the reference's own `--convert-to odp` of that same file, which moves the identical bytes to `Pictures/` and changes nothing else, draws **none**. The storage is the only variable and it decides the answer, which is exactly what `FillReachesThePage`'s `IsInline` term encodes |
| 2026-08-21 | `XRay.Office.Presentations/OpenDocument/OdpSlideLayout.cs` :302 (ODF drawing-cell first baseline) | **WRONG — reported, not fixed** | round 55, `probes/slides-r55/odp-cell-baseline.py`. **A discriminating pair, and the pair is the whole method here**: `soffice --convert-to odp` writes `style:font-independent-line-spacing="true"` onto every drawing cell it emits, so the round-tripped fixture states the very attribute under test and one rendering of it measures the exporter's habit rather than the rule. Rendering it beside a byte-identical copy with that one attribute deleted separates them on four of six sizes — 1.0013 / 0.9998 / 1.0003 / 1.0001 em with it, 0.9020 / 0.9100 / 0.9074 / 0.9030 em without. The site's claim was a *fixed* 0.907 em "whatever `tablecellcontext.cxx:61` sets"; the behaviour is now attribute-driven, and LibreOffice's own ODP export always writes `true`, so an Impress deck takes the one-em arm and this reader draws it 1.7 pt high in an 18 pt face |
| 2026-08-21 | `XRay.Office.Text/Fonts/SystemFontResolver.cs` `GenericFallbacks` (unrecognised → DejaVu Sans) | **VERIFIED again, from a fifth caller** | round 55. Round 54 verified it from four filters that reach it undeclared and stated that the word-processing filters never do. **The DOC filter does**: `GetFontParams` maps `ff` 0, 6 and 7 onto `FAMILY_DONTKNOW` and `SetNewFontAttr` sets it on the item, so those runs arrive here with no class and this switch answers them — DejaVu Sans, and DejaVu Sans *Mono* for `Consolas`, which is this switch's own column. Nine flat-ODF fixtures exported to Word 97 and back, `probes/words-r55/doc-family-code.py`; `Garamond` is the control, forced `FAMILY_ROMAN` by `GetFontParams`'s name-override list and drawn Serif where the otherwise identical `Aptos` is drawn Sans |
| 2026-08-21 | `XRay.Office.Text/Fonts/SystemFontResolver.cs` `GenericFallbacks` (unrecognised → DejaVu Sans) | **VERIFIED, and round 53's WRONG reversed** | `probes/words-r54/font-fallback-rule.py` (98 authored files, 5 controls) + `cross-format-fallback.py` (28): the branch is right for every filter that reaches it undeclared — ODF text, XLSX, PPTX, flat ODS, all tracking fontconfig, `Consolas` → DejaVu Sans **Mono**. The DOCX/DOC/RTF answer is a **roman default applied by the reader**, now in `WordFallbackClass`. Cross-track evidence: 0 of 302 slides and 0 of 307 sheets renderings show the Sans-for-Serif pair |
| 2026-08-21 | `XRay.Office.Text/Fonts/SystemFontResolver.cs` `DefaultFallbacks` (no family → Liberation Serif) | **VERIFIED** | round 54; two fixtures that reach `DefaultFonts` rather than Word's default — a flat ODF declaring no font anywhere, and a DOCX whose `docDefaults` state an empty `w:rFonts` — **both Liberation Serif** on 26.2.4.2. `w:ascii=""` is a third state and answers DejaVu Serif, because the filter reads it as a named family |

| 2026-08-21 | `XRay.Office.Spreadsheets/Layout/SheetOptimalRowHeights.cs` (the thirty-row `WrappedHeight` fit) | **verified, unchanged** | `probes/sheets-r54/audit_rowheight.py`: six sizes x one-to-five unbreakable words, no `ht`/`customHeight`, marker-delta y off both PDFs cross-checked against the reference's `fods` `style:row-height`. 30 of 30 within 0.05 twips; the twelve-point single-line control reads the 300 the site already claims |
| 2026-08-21 | `XRay.Office.Text/Layout/MeasuredParagraph.cs` (picture-alone descent) | **verified, unchanged** | `probes/words-r46/picture-alone-descent.py` re-run: 8 of 8 DOCX rows and 4 of 4 `fodt alone` rows exact, and the reference's own figures identical to round 46's 24.2.7.2 readings to the tenth at 20, 50 and 150 pt |
| 2026-08-21 | `XRay.Office.Text/Fonts/SystemFontResolver.cs` :406 (DejaVu, never Liberation) | **verified in that respect** | `probes/words-r53/font-fallback-recheck.py`: ten unrecognised families, all land on DejaVu, none on Liberation |
| 2026-08-21 | `XRay.Office.Text/Fonts/SystemFontResolver.cs` :629 (unrecognised → DejaVu **Sans**) | **WRONG — reported, not fixed** | same probe: all ten answer DejaVu **Serif**, with four controls agreeing; and `fc-match` answers Sans, so the stated mechanism ("fontconfig's default") is falsified too. **86 of 337 words renderings already disagree with the reference's font list and 73 carry DejaVu Sans on our side.** A change here owes a measured sweep of all three tracks |
| 2026-08-21 | `XRay.Office.Text/Fonts/SystemFontResolver.cs` :435 (no family → Liberation Serif) | **undecided** | the probe's no-family DOCX carries no `styles.xml`, so LibreOffice applies *Word's* default (Carlito) and the case never reaches `DefaultFonts`. Needs a fixture that does |
| 2026-08-21 | `XRay.Office.Presentations/Ooxml/PptxSlideLayout.cs` :763 (PPTX table cell line spacing) | **verified, and the claim had already been corrected** | `probes/slides-r54/make-cell-baseline-probe.py`: one table, one cell, zero margins, top-anchored, six stated sizes 10–40 pt. The reference's first baseline sits 1.0007 … 1.0002 ems below the cell's top edge on all six — one em, not the 0.907 em of 24.2.7.2 — and our own land on it to 0.000 pt. **The ODF half of the same claim, `OdpSlideLayout.cs:302`, is NOT covered** |
| 2026-08-20 | `XRay.Office.Presentations/Layout/SlideAutofit.cs` ×4 | **WRONG, fixed** | round 52; −155.40 `abs_ink`, −11.1% of the slides track |

**Two of the four shared-layer sites re-checked so far were wrong.** The prior on the remaining
44 is not "probably still fine".

## `XRay.Office.Spreadsheets` is finished — **ten of ten, nine correct**

Round 58 took `Layout/SheetText.cs`'s per-glyph pixel rounding, the last one, and it came back
**VERIFIED**. The ten, in the order they were taken: `SheetFonts.cs` (2 sites),
`SheetGeneralWidth.cs`, `SheetDeviceUnits.cs`, `SheetOptimalRowHeights.cs`,
`SheetPageDecoration.cs` (twice, and the only one wrong), `SheetShapeText.cs`, `SheetNotes.cs`,
`Ooxml/XlsxNoteCaptions.cs`, `Layout/SheetText.cs`.

**The one that was wrong is still the only furniture claim among them, and that is now nine
observations to one and no longer a pattern worth acting on.** Round 56 read it as one, round 57
tested it by taking the other furniture claim first and it came back correct, and round 58's
metric claim came back correct too. What the ten actually say is that **the sheets metric work
held across the binary change almost completely** — every fitted number, from column widths to
row heights to device units to glyph advances, reproduces on 26.2.4.2.

**The next track to finish is whichever takes its own list next.** `XRay.Office.WordProcessing`
carries 11 open sites and `XRay.Office.Presentations` 9, and the twelve shared-layer sites —
`Text` 6, `Core` 2, `Rendering` 1, `Ooxml` 1 — are still the highest-value ones, because two of
the four shared sites re-checked so far were wrong and each reaches all three tracks.

The running score across all rounds is now **three wrong in sixteen**. Two of the three were shared-layer sites; the third is `SheetPageDecoration.cs`. **Ten** `XRay.Office.Spreadsheets` sites have been re-checked and nine were correct.

Round 56 recorded "the only site found wrong is the only furniture claim, which is a pattern rather than a coincidence" and sent round 57 at the other furniture claim on the strength of it. `XlsxNoteCaptions.cs` came back correct and round 58's metric claim came back correct too, so **the pattern was two observations of one event and it never repeated** — which is worth as much as the prior was, and is the reason the re-check was run rather than the claim being assumed either way.
