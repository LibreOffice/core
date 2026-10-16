# The overflow was never the defect, the line break was — and a fitted bullet is centred in the box it would have had unscaled

Round 100, slides track. Three seats. **O34 closes as a fixed defect and its premise is
refuted**: 26.2.4.2 does not clip, shrink or drop an overflowing text block, it draws it and lets
the page clip — the cost round 99 recorded was a hard line break sizing the line it ends.
**O28's baseline half closes on a measured mechanism**: 315 of 1239 bullets out by more than
0.10 pt on the cleanest pages the corpus has, and 0 of 1243 after. **O15 stays open, does not
move, and is sharpened**: the two line-height causes this round fixes are not in it, and what is
left is measured at 0.20 pt of block height on the register's own next witness.

**One instrument is withdrawn and replaced, and it is the load-bearing correction of the round.**
`slides-r99/tfy.py` miscounts pages on **45 of the 51 banked reference PDFs** and on **13 of 13**
of ours, so every per-page pairing round 99 made was between pages that were not necessarily the
same page. Round 99's O28 census reports 42 pages in 5 documents; the same filter on a correct
enumeration finds **310 in 16**.

Every figure below was read out of a file in this directory or out of the run logs beside it,
and each such file is named where the figure is quoted.

## Environment

| | |
|---|---|
| worktree / branch | `/home/user/wt-slidelast`, `agent/slidelast`, base **`18f9da973`** |
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**. `/usr/bin/soffice` (24.2.7.2) is used for nothing here |
| reference bank | `/home/user/gate-orig-r83/ref`, the 51 `.ppt` among its 948 PDFs |
| corpus | `/home/user/sample-files`; the slides binary column is **51 `.ppt`** (`ppt.list`) |
| C++ tree | `/home/user/libreoffice-core`, read only, never built. **It declares `27.2.0.0.alpha0+`** and is not the reference binary's source — §2.4 is a case where the two genuinely disagree, and it is the measurement that decides |

---

# 0. The instrument, and why round 99's census has to be re-derived rather than continued

`tfy.py` finds a PDF's pages by regex-scanning every `N 0 obj` and testing a **700-byte window**
for `/Type /Page`. That window runs past `endobj` into the next object whenever objects are packed
tightly, so a dictionary that is not a page is read as one and takes the *following* object's
`/Contents`. It is not confined to our writer: `tfy-page-count.txt` counts

| | miscounted |
|---|---|
| the 51 banked 26.2.4.2 reference PDFs | **45 of 51**, by 1 to 3 pages each |
| every rendering of ours still on disk | **13 of 13**, by up to 18 |

so both legs of a pairing drift, and they do not drift together. `Inducement-to-Insurance-Business`
is 25 against a real 22 on the reference side and 28 against 22 on ours.

`tfz.py` replaces it. It enumerates through the real page tree (PyMuPDF's) and **tracks `cm` as
well as the text matrix**, which `tfy.py` does not — 26.2.4.2's own PDFs place an underline with
`q 1 0 0 1 x y cm … Q`, and a writer that positioned text that way would have been reported at
y = 0. Nothing else about it changes: it prints the font, the size, the x and the **baseline** of
every text show, out of the content stream's own operators, with no bounding box anywhere in it.

**What that costs and what it buys.** Round 99's §2.2 census (42 pages, 5 documents, 217 bullets)
is withdrawn as a *sample*; its direction is not. The same filter on `tfz.py`'s tables finds
**310 pages in 16 documents, 1462 bullets and 4769 body shows**, and the residual is still there
and still one-directional. Round 99's §1.3 witness table and its `ops-check.txt` validation
survive, because both are single-page readings on documents where the drift had not yet started.

---

# 1. O34 — the reference overflows, and the cost round 99 recorded was a hard line break

**Closed. Fixed in this tree, with the seat's own premise refuted.**

## 1.1 What 26.2.4.2 does when a text block is taller than its shape

**It draws it.** Not clipped to the shape, not shrunk, not truncated: the block grows past the
shape's edge and past the page's, and the page is what removes the ink.

The witness is round 99's own regression, `Inducement-to-Insurance-Business.ppt` page 14
(`witness-p14.txt`). 26.2.4.2's own flat-ODP export gives its title frame
`svg:height="7.509cm"` at `svg:y="3.117cm"` with `draw:textarea-vertical-align="bottom"`,
`draw:auto-grow-height="false"` and 0.127 cm of padding at each end, on a 33.87 × 19.05 cm page —
so the text area runs from PDF y **448.05 down to 242.49**, 205.6 pt tall. The paragraph is nine
wrapped lines of 28 pt at `fo:line-height="90%"` plus a trailing empty line whose span states
54 pt. **The reference's topmost baseline is 555.194** — 107.1 pt above the text area's top edge
and **15.2 pt above the top of the page** — and its lowest is **−12.416**, below the bottom of it.

Censused rather than left as one document (`offpage.py`, `offpage.txt`): over all 33 549 text
shows the reference draws in the 51 `.ppt`,

| | shows | baseline above the page | below it | | documents |
|---|---:|---:|---:|---:|---:|
| 26.2.4.2 | 33 549 | 10 | 81 | **91 — 0.27 %** | 10 of 51 |
| ours, at this round's head | 32 183 | 10 | 35 | **45 — 0.14 %** | 7 of 51 |

**A renderer that clipped an overflowing block to its shape could not produce one of the 91.** So
"what we do when a text block overflows its shape" is not a defect: both sides overflow, and the
seat's premise is withdrawn.

## 1.2 What the cost actually was

**A hard line break does not size the line it ends, and every other portion of that line does.**
EditEngine's line-metrics loop walks `GetStartPortion()` to `GetEndPortion()` and skips exactly
one kind — `if ( rTP.GetKind() != PortionKind::LINEBREAK )`, with the comment beside it naming
this very case, *"problem with hard font height attribute, when everything but the line break has
this attribute"* (`editeng/source/editeng/impedit3.cxx`:1498-1516). The kind is set by
`EE_FEATURE_LINEBR` and by nothing else (`:1088-1099`), and the break is the line's **last**
character: `pLine->SetEnd(nPortionStart + 1)` (`:1441-1449`). A line holding nothing but a break
measures zero — `EditLine::CalcTextSize` adds nothing for that kind
(`editeng/source/editeng/EditLine.cxx`:69-71) — and falls back to the seek above the loop,
`SeekCursor(pNode, pLine->GetStart()+1, aTmpFont)` and, under `IsFixedCellHeight()`,
`ImplCalculateFontIndependentLineSpacing(aTmpFont.GetFontHeight())` (`:1478-1491`): the height of
the break's **own** character. All read in this checkout, which declares `27.2.0.0.alpha0+`.

Round 99's fix moved `SlideTextLayout`'s line measurement from `TextLine.VisibleEnd` to
`TextLine.End`. That is right, and it brought this in with it: a line separator is trailing
whitespace, so `VisibleEnd` had been excluding it **by accident**
(`TextMeasurer.TrimTrailingSpaces`, which trims every `EndsLine` character).

## 1.3 Measured at 26.2.4.2, on a deck built for it

`make-break-probe.py` (`break-probe.txt`). Five boxes per slide, 18 pt text, one construct whose
size is swept 18, 24, 28, 36, 54, 72 pt; `a:noAutofit`, `anchor="t"`, no insets. The deck is
converted to `.ppt` by 26.2.4.2 itself so the binary reader is exercised, and rendered by
26.2.4.2 and by this tree at both legs.

| box | what it holds | 26.2.4.2 |
|---|---|---|
| `BREAK` | `"AAA"`, a break at the swept size, `"BBB"` | **462.019 and 440.419 on all six slides** — the break's size changes nothing |
| `BLANK` | a trailing space at the swept size | moves on every slide, 462.019 → 408.019 |
| `RUN` | a visible glyph at the swept size | **identical to `BLANK` in every cell** |
| `TWICE` | two breaks in a row, so the middle line is nothing but one | that line's height *is* the swept size's `fround(1.2 × em)`: first-to-third baseline distance `127 + fround(1.2 × em) + 635` hundredths of a millimetre on all six, 1524 at 18 pt through 3810 at 72 |
| `TAIL` | a break that ends the paragraph | the appended empty line takes the break's size too |

`BLANK` agreeing with `RUN` cell for cell is round 99's own rule re-measured at the binary on a
deck built for it, which is worth having beside the correction to it.

**Cells beyond 0.10 pt of 26.2.4.2: base 15 of 30, head 0 of 30.**

## 1.4 The change, and the witness after it

`SlideTextLayout.MeasuredEnd` drops one character off `TextLine.End` when that character is a
line separator (U+2028, U+000B, U+000C, U+0085 — the set `TextMeasurer` fills lines by). When the
line is nothing but the break the range collapses to `start == end`, which is precisely the case
`LargestSize` and `FaceHeight` already answer from the run covering `start` — the break's own —
so EditEngine's zero-height fallback needs no special case.

`Inducement-to-Insurance-Business.ppt` page 14, all 29 shows (`witness-p14.txt`):

| | 26.2.4.2 | base | head |
|---|---:|---:|---:|
| first title baseline | 555.194 | 583.228 | **555.165** |
| last title baseline | 313.228 | 318.813 | **313.200** |
| last body baseline | −12.416 | −12.444 | −12.444 |
| shows agreeing within 0.03 pt | — | 0 of 29 | **29 of 29** |

The base drew every 28 pt line 28.03 pt higher — one whole line further off the top of a
bottom-anchored block — because it measured the paragraph's last line at the 54 pt of the break
that ends it. Its extracted alphanumeric count goes **12 812 → 12 892** against the reference's
12 899, and `2015-Civil-Rights-Website-training` **34 846 → 34 816**, which is the reference's
count exactly.

## 1.5 What it costs over the column

`sweep.sh`, `base.tsv`, `head.tsv`, `gate-summary.txt`:

| | base (= round 99 head) | + O34 |
|---|---:|---:|
| documents | 51 | 51 |
| `match` (page count equal, alphanumerics within max(2 %, 15)) | **49** | **49** |
| page counts differing | 0 | 0 |
| sum \|glyph distance\| | 1417 | **1307** |
| sum \|ink\|% | 252.82 | **250.10** |
| MAJOR pages | 66 | **65** |
| renderings whose bytes changed | — | 9 |

Four improve, two worsen by 0.04 and 0.02, three do not move measurably. **This pays round 99's
recorded debt in full**: its own base was 252.21 and its head 252.82 — reproduced here as this
round's base, 252.82 — and this leg is 2.11 below the figure the debt was measured from.

## 1.6 The test

`SlideLineBreakLineHeightTests`, five cases: the break at both spellings (U+000B and U+2028), a
control at the text's own size, the two-break case whose middle line *is* sized by the break, and
its control. Measured rather than asserted: with `SlideTextLayout.cs` reverted to `18f9da973` and
the test file left in place, **3 fail and the 2 controls pass**; all five pass with the source
back.

---

# 2. O28 — the baseline half closes, and the mechanism is a stale cache in 26.2.4.2

**Closed. Fixed in this tree.**

## 2.1 The census, re-derived and with its base rate

`bulletcensus.py`, `bullet-census-base.txt` / `-o34.txt` / `-head.txt`. Round 99's filter — pages
where the two renderings draw the same sequence of (family, size) shows and at least one is a
bullet — plus a **stricter one this round adds**: pages where *every body show* also sits within
0.10 pt of the reference's baseline, so the page's layout provably agrees and what is left on a
bullet is the bullet's own placement.

| | base | + O34 | + O28 |
|---|---:|---:|---:|
| pages scanned | 1532 | 1532 | 1532 |
| agreeing pages carrying a bullet | 310 in 16 documents | 310 | 310 |
| bullets beyond 0.10 pt | 426 of 1462 (29.1 %) | 423 (28.9 %) | **73 (5.0 %)** |
| **body** shows beyond 0.10 pt, same pages | 401 of 4769 (**8.4 %**) | 388 (8.1 %) | 388 (8.1 %) |
| pages where every body show is within 0.10 pt | 262 | 263 | 263 |
| bullets beyond 0.10 pt on those | **315 of 1239 (25.4 %)** | 315 of 1243 | **0 of 1243 (0.0 %)** |
| their mean / min / max | −0.2117 / −4.067 / +0.114 | — | **+0.0071 / −0.057 / +0.086** |

The last row is the one that matters: on pages where the body is exact by construction, a quarter
of the bullets were out and none is now, and the residual range is inside the 0.028 pt the two
writers differ by everywhere.

**§1's fix does not move it** — 426 → 423, which is confound C9's own warning arriving: two
line-height questions that look like one.

## 2.2 The stated size and the stated line spacing are not where it lives

`make-bullet-probe.py`, `bullet-probe.txt`. Four stated sizes × six stated line-spacing
percentages (none, 70, 80, 90, 100, 120), `a:noAutofit`, as `.pptx` and as 26.2.4.2's own `.ppt`
of it. **0 of 24 cells beyond 0.10 pt on either format at either leg**, worst 0.057.

That is a null and it is what re-aimed the round: the model of `nFirstLineHeight`,
`nFirstLineTextHeight` and `nFirstLineMaxAscent` that this tree already had is exactly
EditEngine's over that whole grid, including the `Prop` branch's own `f80Percent` on the ascent
(`impedit3.cxx`:1553-1600).

## 2.3 The variable that was missing is the fit

`make-bullet-fit-probe.py`, `bullet-fit-probe.txt`. One 24 pt bulleted body per slide,
`a:normAutofit`, the box swept 60…420 pt so the search settles on a different row of
`constScaleLevels` each time.

| box height | 26.2.4.2 | base | head |
|---:|---:|---:|---:|
| 60 | −2.806 | 3.727 | **−2.835** |
| 90 | 1.361 | 5.414 | **1.389** |
| 120 | 5.528 | 7.143 | **5.584** |
| 160 … 420 (not scaled) | 3.912 | 3.855 | 3.883 |

(bullet baseline less the first body baseline, in points.) **Cells beyond 0.10 pt: base 6 of 16,
head 0 of 16.** The drawn bullet size, the drawn text size and the body's baseline pitch agree
with 26.2.4.2 on all sixteen at both legs, so nothing but the bullet's own vertical is in
question.

## 2.4 The rule, and the one place the checkout and the binary disagree

`Outliner::ImpCalcBulletArea` centres a `SVX_NUM_CHAR_SPECIAL` bullet in a box

```
Top    = nFirstLineHeight − nFirstLineTextHeight + nFirstLineTextHeight/2 − bulletHeight/2
Bottom = Top + bulletHeight − 1
```

and `Outliner::StripBullet` draws it from `Bottom` less the bullet font's descent
(`editeng/source/outliner/outliner.cxx`:1461-1467, :892, :906-919, :951-956;
`include/tools/gen.hxx`:597 for the `− 1`). `bulletHeight` is `ImplGetBulletSize`, which sets
`ImpCalcBulletFont`'s font on the reference device, reads `GetTextHeight()` — and **caches the
answer on the paragraph** (`:1315-1355`).

**The residual is exactly `(1 − fontScale) × unscaledBulletHeight / 2`, over five distinct font
scales.** Solving each of the six scaled probe cells for the box height the reference must have
used, against our own `H`, `TH` and metrics:

| font scale | residual, hundredths of a mm | `(1 − s) × 385` |
|---:|---:|---:|
| 0.400 | 231 | 231.0 |
| 0.475 | 203 | 202.1 |
| 0.625 | 144 | 144.4 |
| 0.700 | 115 | 115.5 |
| 0.850 | 58, 58 | 57.8 |

and 385 is `bulletHeight/2` at the paragraph's **unscaled** 24 pt — OpenSymbol's typographic
1420/442 on a 2048 em, at the reference device's 200 whole pixels, 770 units. So the box is the
size the bullet would have had if the fit had not run, while the bullet inside it is drawn at the
size the fit chose, because `StripBullet` asks `ImpCalcBulletFont` again for the font it paints
with (`:851-855`, which does multiply by `getScalingParameters().fFontY`). **The autofit search
formats the outliner once unscaled before it walks the levels** (`impedit3.cxx`:303-333), so the
call that fills the cache is the unscaled one.

**This checkout would not do that and 26.2.4.2 does.** Here `IsBulletInvalid` compares a stored
`ScalingParameters` (`include/editeng/outliner.hxx`:154-172), so the cache is correctly
invalidated when the fit's scale changes; the checkout declares `27.2.0.0.alpha0+`. The source leg
is therefore a later version's explanation, and it is `bullet-fit-probe.txt` that establishes the
behaviour of the binary this tree is calibrated to. Said plainly: **the arm was measured first and
the source read afterwards, and where they disagree the measurement is what is implemented.**

## 2.5 The change, its reach and its cost

`SlideTextLayout.BulletBoxHeight` is the box, at `ScaledMarker(Scaling.None, …)`; `EmitMarker`
now computes the baseline as the four integer steps above, in hundredths of a millimetre, rather
than as `Height − TextHeight/2 + (ascent − descent)/2` in EMU. The `− 1` and `TH/2`'s truncation
come with it, which is the 0.028 pt the unscaled probe rows also improve by.

`head2.tsv`, `gate-summary.txt`:

| | + O34 | + O28 |
|---|---:|---:|
| `match` | 49 | **49** |
| page counts differing | 0 | 0 |
| sum \|glyph distance\| | 1307 | **1307** |
| sum \|ink\|% | 250.10 | **250.28** |
| MAJOR pages | 65 | **65** |
| renderings whose bytes changed | — | **50 of 51** |

**50 renderings move and the ink figure moves by 0.18**, all of it in two documents (+0.13, +0.08)
against 44 that move by less than 0.005 — because a bullet is a small mark however wrongly it is
placed. No gate column can see a bullet's baseline at all. **The instrument that can is §2.1's
census, and 315 → 0 is what this seat is scored on.** A round that disagrees with keeping a change
worth +0.18 of ink has both numbers here.

## 2.6 The test

`SlideFittedBulletBoxTests`: a fitted 24 pt bulleted body whose bullet ends up **below** its
text's baseline, and the unfitted control that must not move. Measured: at `18f9da973` the fitted
case fails and the control passes.

## 2.7 What is left of O28

Nothing this round can name. The size half closed at round 99 as nil observable reach; the
baseline half is 0 of 1243 on the clean pages and 73 of 1462 (5.0 %) on the wider sample, against
the body's 388 of 4769 (8.1 %) on the same pages — **the bullet is now placed better than the body
text beside it**, so the residual there is the page's layout and not the bullet.

---

# 3. O15 — unchanged, and now sized

**Not closed, and not moved by either of this round's fixes.**

`sizes.py`, `sizes-head.tsv`, `size-summary.txt`. The register's own statistic — the per-page
dominant drawn text size, the size carrying the most alphanumeric characters — over all 1534 pages
of the 51 `.ppt`.

**The scorer is validated before it is used**: `sizes.py` reproduces
`probes/slides-r97/sizes-ref.tsv` **1534 of 1534 rows** off the same bank, tie-break included, so
it is that scorer and its head column is comparable with `probes/slides-r99/sizes-head.tsv`, which
is this round's base.

| | base | head |
|---|---:|---:|
| pages differing by more than 0.15 pt | **17** | **17** |
| documents holding one | 13 | 13 |
| total \|size error\| over 1534 pages | **66.06 pt** | **66.06 pt** |
| of the differing pages, same alphanumeric count | 12 | 12 |
| … of those, we draw the larger size | 11 | 11 |
| **fixed / newly wrong** | — | **0 / 0** |
| pages whose dominant size moved | — | **0** |

So the two line-height causes closed above are **not** what is left of O15, and the seat's
"our block height is short of the reference's" is now a claim about a third cause. `size-summary.txt`
lists all seventeen pages, which the register did not.

**What is left is sized, on the register's own next witness.** A temporary dump in
`SlideAutofit.Solve` (since removed) printed, for each of `RESPA_-_Section_8_Webinar.ppt`'s 32
autofitted shapes, the box height it fits against and the block height it measures at every one of
`constScaleLevels`' twelve rows (`fit-margin.txt`). Nineteen of the 32 are scaled at all, and
**the tightest wins its row by 7 hundredths of a millimetre — 0.20 pt on a block of 9957, 0.07 %.**
One row of the table is one point of drawn size, which is exactly the 20.01 against 18.99 that
page 18 of that document shows.

**So the seat is no longer "a line is short" but "the block is short by a fifth of a point over
thirteen lines", and the instrument for it is a per-line height comparison rather than a per-page
dominant size.** Nine of the twelve same-alphanumeric pages are one `constScaleLevels` row apart,
which is the same shape. What this round did not do is instrument that margin over the whole
column; `fit-margin.txt` is one document.

---

# 4. The suite

Read out of this run's own output (`tests.log`), not from the briefed number.
`dotnet build XRay.Office.slnx -c Release` at **0 warnings, 0 errors**, then
`dotnet test XRay.Office.slnx -c Release --no-build`, all eleven projects reached:

| project | |
|---|---|
| Containers | 109 / 109 |
| Core | 528 / 528 |
| Markup | 259 / 259 |
| OpenDocument | 146 / 146 |
| Rendering | 164 / 164 |
| Spreadsheets | 1292 / 1292 |
| Text | 728 / 728 |
| Vector | 309 / 309 |
| **Presentations** | **1054 / 1054** |
| WordProcessing | 1938 / 1938 |
| **Fidelity** | **542 passed / 10 failed of 552, 0 skipped** |

The ten are exactly the briefed baseline and nothing else — `PageDrawing` ×4, `TabStop` ×4,
`SheetDrawing` ×1, `JustificationShrink` ×1. **No eleventh.**

Presentations is **1054** against round 99's 1047: five `SlideLineBreakLineHeightTests` and two
`SlideFittedBulletBoxTests`. With `SlideTextLayout.cs` reverted to `18f9da973` and both files left
in place, **4 of the 7 fail and the 3 controls pass**.

# 5. The files

| | |
|---|---|
| `tfz.py` | the instrument: font, size, x and **baseline** of every text show, through the real page tree and with the CTM tracked. Replaces `slides-r99/tfy.py` |
| `tfy-page-count.txt` | why: `tfy.py` against the true page count, on the 51 banked reference PDFs and on every rendering of ours still on disk |
| `offpage.py`, `offpage.txt` | every text show whose baseline is off the page, both sides — the census that answers O34's question |
| `witness-p14.txt` | `Inducement-to-Insurance-Business.ppt` p14: the frame 26.2.4.2 resolves for it, and all 29 shows at the three legs |
| `make-break-probe.py`, `break-probe.txt` | the line-break rule at 26.2.4.2, five boxes × six sizes, all three legs |
| `make-bullet-probe.py`, `bullet-probe.txt` | the bullet's offset over the stated size and the stated line spacing — the **null** that re-aimed §2 |
| `make-bullet-fit-probe.py`, `bullet-fit-probe.txt` | the same with the box swept so the autofit search fires; base 6 of 16 out, head 0 |
| `_parts.py` | the package scaffolding the three probe generators share, lifted from `slides-r52/make-fit-probe.py` |
| `bulletcensus.py`, `bullet-census-{base,o34,head}.txt` | every paired bullet on an agreeing page, with the body's rate beside it and the clean-page subset |
| `sweep.sh`, `base.tsv`, `head.tsv`, `head2.tsv`, `gate-summary.txt` | the 51 `.ppt` at three legs: pages, alphanumerics, summed \|ink\|%, MAJOR, and the md5 that says which renderings moved |
| `shows.sh` | the same column reduced to `tfz.py` tables, which is what the censuses read |
| `sizes.py`, `sizesweep.sh`, `sizes-head.tsv`, `sizes-ref-mine.tsv`, `size-summary.txt` | O15's dominant-size census, with the scorer's 1534-of-1534 validation against `slides-r97/sizes-ref.tsv` |
| `fit-margin.txt` | how close `RESPA`'s autofit search is to the next row down: 7 hundredths of a millimetre at the tightest |
| `ppt.list` | the 51 documents |
| `tests.log` | the suite |
