# fidelity-01 — the 40 Fidelity failures, enumerated and classified

Round: Fidelity. Worktree `/c/sandbox/workdir/wt-fidelity`, branch `wt-fidelity`.
The prediction is in `prediction.md`, committed at `2420db95528` **before** the first measurement.

Throughout: **measured** means I ran it and read the number; **inferred** means I reasoned to it
and could not close the loop. They are labelled separately everywhere.

---

## 0. Environment, quoted

`.claude/skills/libreoffice-reference/scripts/check-env.sh`, run first (measured):

```
== 1. soffice binary ==
  OK    LibreOffice 26.2.4.2 620(Build:2)
== 3. metric-compatible fonts ==
  OK    Calibri -> Carlito          OK    Cambria -> Caladea
  OK    Arial -> Liberation Sans    OK    Times New Roman -> Liberation Serif
  OK    Courier New -> Liberation Mono
  OK    DejaVu Sans -> DejaVu Sans
== 4. PDF rasteriser ==   OK  pdftoppm 26.01.0
== 5. PDF extractor ==    OK  pdftotext 26.01.0
Environment is good.
```

`df -h /` → **13 GB free, 34 % used** (measured). The full-disk signature the brief warns about —
every test in a class dying at <1 ms — **does not apply here**: the 40 failures take 1–11 s each
and carry real assertion values. This was checked before any mass-failure pattern was believed.

---

## 1. The reconciliation of 550

| | total | passed | failed | skipped |
|---|---:|---:|---:|---:|
| Handover, at `0fb6d41e0`, on LibreOffice **24.2.7.2** | 550 | 550 | 0 | 0 |
| Today, at `HEAD`, on LibreOffice **26.2.4.2** | 550 | 510 | 40 | 0 |

**The total did not change; only the pass/fail split did.** Measured two ways:

- `dotnet test --list-tests` discovers **550** tests today.
- The run reports `Failed: 40, Passed: 510, Skipped: 0, Total: 550`.

550 = 510 + 40 exactly, and 0 skipped means no part of the suite silently covered nothing.
**Prediction P1 confirmed.**

### Two corrections to the brief's expected non-Fidelity state

The brief states the ten non-Fidelity projects must hold at **"3465 total, 0 failed"**. Measured
today, project by project:

```
Core 284  Containers 109  Text 287  Vector 295  Rendering 121  Markup 259
OpenDocument 125  WordProcessing 761  Spreadsheets 621  Presentations 592
                                                     TOTAL 3454, FAILED 0
```

**Every project matches its handover figure exactly. The total is 3454, not 3465** — and the
handover's own table sums to 3454 as well. The brief's 3465 is an arithmetic slip of 11, not a
regression and not a change. Nothing moved; the expected figure was wrong. **0 failed confirmed.**

**But one thing did move, and nothing had flagged it.** The handover says "0 skipped and 0 warnings
throughout". Today `XRay.Office.Rendering.Tests` reports **120 passed, 1 skipped, 121 total**:

```
Skipped XRay.Office.Rendering.Tests.PdfFontTests.ACffFlavouredFaceIsNotClaimedToBeTrueType
```

It skips on `Assert.SkipUnless(TestCffFace.IsAvailable, "no CFF-flavoured face on this machine")`.
`TestCffFace` scans the machine for any sfnt beginning `OTTO` (`DrawnPage.cs:146-170`). Measured:
`find /usr/share/fonts -name '*.otf'` returns **zero files** on this container.

This matters more than a skip count suggests. The test's own remarks record what it guards: poppler
reporting *"Mismatch between font type and embedded font file"* and then *"No font in show"* **161
times**, leaving 161 glyph runs blank on a document that passed every other check. That guard is
currently not running at all.

**This is an undeclared environment variable of its own: the container has no CFF-flavoured font.**
It belongs in `MISSING_PACKAGES.md` beside `fonts-dejavu-core`, and it presents exactly the way that
file warns about — as nothing at all except a skip count nobody was totalling. I have **not**
installed a font to fix it: three other agents are measuring against this machine's fontconfig
chain right now, and adding a face mid-session would silently move their reference renderings. It
is a recommendation, not a change.

**21 names, 40 cases — confirmed, not assumed.** The 40 failing cases resolve to exactly 21
distinct test names; the 19 extra failures are additional `[InlineData]` rows on 9 of those names
(the largest being `ExtractionComparisonTests` ×5, and four names at ×4/×3). There is no second
mechanism hiding in the 19. **Prediction P2 confirmed.** Full lists: `fails-head.txt`.

---

## 2. The decisive experiment: it is not our code

The brief asked for a bisect where it is cheap. It was cheap, and it is conclusive.

The handover records Fidelity green at **`0fb6d41e0`**, and records the installed binary at that
time as **24.2.7.2**. The container swap landed today in `4cbaeb41c3b`. So I checked out
`0fb6d41e0` — the last commit known green — and ran Fidelity **in today's environment**:

```
0fb6d41e0  ->  Failed: 40, Passed: 510, Skipped: 0, Total: 550
HEAD       ->  Failed: 40, Passed: 510, Skipped: 0, Total: 550
diff fails-head.txt fails-0fb6d41e0.txt  ->  IDENTICAL SET
```

The failing set is **byte-identical** at the two commits (`fails-0fb6d41e0.txt`). `git merge-base
--is-ancestor 0fb6d41e0 HEAD` passes, and only four commits touched `dotnet/src` in between.

**Therefore: no XRay.Office code change caused any of the 40.** The environment did.

**But that is where the easy answer stops, and I want to be explicit about it.** "Our code did not
change" is *not* the same as "our code is right". A reference binary that gets **more** correct
will newly disagree with a defect we have always had. So every one of the 40 was pushed through a
second question: **which side is now wrong?** That question is what the rest of this document
answers, and it splits the 40 four ways — including **twelve** cases where the answer is *we are*.

---

## 3. Which environment variable can physically reach these tests

Two of the three suspects in the brief are **excluded by measurement** for the families I took,
and this matters because the brief's leading hypothesis names all three.

**The font set is excluded for the 32 word-processing cases here, and for the 8 sheets/slides
cases in §6.0 — that is all 40.** `MISSING_PACKAGES.md` is right that DejaVu moves
53 of 534 corpus reference page counts — but that is the *corpus*, not this suite. The Fidelity
fixtures are Latin text in Carlito. Measured, by listing the fonts LibreOffice actually embedded
in its own reference PDF for each failing document:

| document | fonts embedded in the reference PDF |
|---|---|
| `paginated.docx` | `Carlito-Regular`, `Carlito-Bold` — nothing else |
| `list-label-overrun.odt` | `Carlito-Regular` only |
| `table-exact-row.docx` | `Carlito-Regular` only |
| `table-autofit.fodt` / `.docx` | `Carlito-Regular` only |

No DejaVu, no WenQuanYi, no fallback of any kind. **A font that is never resolved cannot move a
measurement.** §6.0 goes further and removes DejaVu from fontconfig entirely: the reference content
streams come back **byte-identical**. Prediction P4 (2–6 font failures) measures **zero**.

**Poppler is excluded as a *geometric* variable.** I checked whether `pdftotext -bbox` faithfully
reports what the PDF says, rather than computing its own boxes. For `paginated.pdf` line 1, I
reconstructed the pen position directly from the PDF's own content stream — the `Td` origin plus
the `/Widths` array minus the `TJ` kerning adjustments:

```
pen start (Td)                    70.950
advance total from /Widths + TJ  461.461
pen end                          532.411
minus the trailing space (2.486) 529.925
poppler's reported xMax          529.903     <- agrees to 0.022 pt
```

Poppler reports the PDF's own geometry to **0.022 pt**, and correctly excludes the trailing space
from the word box. It is a faithful conduit, not a variable, for every position comparison.
(Poppler remains a *plausible* variable for tests that compare word **counts** or **text**, since
word segmentation is its own decision — that is a different surface, covered in §4.4.)

By elimination, for the position-comparing families the variable is the **LibreOffice version**.

---

## 4. The 40, classified

### The tally, first

| class | cases | where |
|---|---:|---|
| **Genuine defect / gap in XRay.Office** | **12** | §4.4 + §5.1 (9, one root cause), §6.4 (2), §6.1 (1) |
| Environment (LibreOffice) — reference regressed or merely differs; **we are right** | 18 | §4.1, §4.2, §4.3, §5.2, §5.3, §6.2, §6.3, §6.5, §6.6 |
| Environment (LibreOffice) — **exposes a real divergence in our shaping** | 8 | §4.5 |
| **Unexplained** | 2 | §5.4 |
| Environment (**font set**) | **0** | excluded by measurement, §3 |
| Environment (**poppler**) | **0** | excluded by measurement, §3 and §6.0 |
| **Total** | **40** | |

**Every one of the 40 is environment-*triggered*** — §2 proves that. But **12 of them are ours**, and
that is the finding the brief hoped was there. The reference binary did not simply drift: in twelve
cases it moved deliberately **toward the application that owns the format** — Word, Excel,
PowerPoint — and we did not follow.

**Nine of those twelve are a single root cause**, `DocumentSettingId::CONTINUOUS_ENDNOTES`, which
LibreOffice's Word filters now set unconditionally. It is treated as one item across §4.4 and §5.1.

| # | class | cases | verdict |
|---|---|---:|---|
| 4.1 | environment (LibreOffice) — **reference regressed, we are right** | 5 | LO's text filter triples a paragraph |
| 4.2 | environment (LibreOffice) — **reference regressed, we are right** | 3 | LO adds a cell margin to an exact row |
| 4.3 | environment (LibreOffice) — **reference artifact, we are right** | 3 | LO emits a trailing space as its own portion |
| 4.4 | **genuine gap in XRay.Office** (with §5.1) | 4 | Word's note separator, not Writer's |
| 4.5 | environment (LibreOffice) — **exposes a real divergence** | 8 | a kerning gap crossing two tolerances |
| 5.1 | **genuine gap in XRay.Office** (with §4.4) | 5 | Word's continuous-endnote layout |
| 5.2-5.4 | environment / unexplained | 4 | justification rework, a self-referential test, a boundary tie |
| 6.1-6.6 | see §6 | 8 | sheets and slides |

### 4.1 `ExtractionComparisonTests` × 5 — the reference oracle is wrong

Cases: `NothingTheReferenceFindsIsMissingFromTheFeatureDocument` for `tables.doc`, `.docx`,
`.fodt`, `.odt`, `.rtf`. All five report the identical shortfall:

> content the reference filter found is absent from the XRay.Office extraction:
> `'the' x2, 'tables.' x2, 'After' x2`

The reference here is **LibreOffice's `Text (encoded)` filter**, not poppler — `ExtractionComparisonTests.cs:46-51`
calls `_libreOffice.ExtractText`. So **neither poppler nor the font set can reach this test at all.**

Measured, three ways:

1. **The file says it once.** `tables.odt`'s `content.xml` contains exactly one
   `<text:p>After the tables.</text:p>`, after the `Spans` table.
2. **LibreOffice's text filter says it three times** (measured, `grep -c` → 3):
   ```
   7  Wide cell | Tall cell
   8  Plain a   | Plain b
   9            | After the tables. | After the tables.      <- phantom row
   10 After the tables.                                       <- the real paragraph
   ```
   The filter also mis-flattens the nested table: it merges the inner table's rows into the outer
   table's grid (line 3 reads `Outer body left | Inner one`), and then absorbs the following body
   paragraph into a row that does not exist, duplicating it across the remaining columns.
3. **LibreOffice's own PDF rendering of the same file says it once** (measured, `pdftotext | grep -c`
   → 1), and renders the nested table correctly.

So LibreOffice **contradicts itself between its own two exports**, and its rendering agrees with
the file. XRay.Office's extraction is exactly right — it reproduces the nesting faithfully and emits
the paragraph once (measured with the CLI).

**Verdict: environment (LibreOffice version). The reference oracle regressed; XRay.Office is correct.**

*Measured vs inferred:* everything above is measured. I could **not** reduce it to a minimal
trigger — a hand-built FODT with a covered cell, with a row span, and an exact replica of the
`Spans` table geometry all failed to reproduce the phantom row. **The precise trigger is
unexplained**; the misbehaviour itself is not.

### 4.2 `TableComparisonTests` × 3 — LibreOffice adds a margin it should not

Cases: `EveryCellHoldsItsTextWhereLibreOfficeDoes` for `table-exact-row.doc`, `.docx`, `.rtf`.
All three: word 20 ("Golf") sits **58.150 pt** below the first word for us, **61.000 pt** for
LibreOffice. Δ = 2.850 pt against a 0.1 pt tolerance.

**`.odt` and `.fodt` are in the same `[Theory]` and pass.** That polarity is the whole finding.

Measured, from the reference PDFs' word boxes:

| | row 2 top | row 3 top ("Golf") | exact row height |
|---|---:|---:|---:|
| LibreOffice, `.odt` | 92.179 | 114.879 | **22.700 pt** |
| LibreOffice, `.docx` | 92.179 | 117.729 | **25.550 pt** |

The document declares `<w:trHeight w:val="454" w:hRule="exact"/>`. **454 twips = 22.700 pt** — which
is exactly what LibreOffice produces for the ODF twin, and exactly what XRay.Office produces for all
five formats.

The 2.850 pt excess is not a mystery: the same file declares
`<w:tblCellMar><w:top w:w="57" w:type="dxa"/>…`, and **57 twips = 2.850 pt exactly**.

LibreOffice 26.2.4.2's Word-import path is adding the cell's declared **top margin on top of** an
`hRule="exact"` row height. In OOXML an exact row height is the row's total height; the cell
margins live inside it. Word does not add them, LibreOffice's own ODF path does not add them, and
at 24.2.7.2 LibreOffice's Word path did not add them either (the test passed).

**Verdict: environment (LibreOffice version). The reference regressed; XRay.Office is correct.**
The delta is *exactly* the declared `w:tblCellMar/w:top`, which is as strong as attribution gets.

### 4.3 `TableAutoLayoutComparisonTests` × 3 — a trailing space counted as a run

Cases: `EveryCellStartsWhereLibreOfficeStartsIt` for `table-autofit.fodt`, `table-autofit-full.fodt`,
`table-autofit-mixed.fodt` — "line 3 run count". The `.docx` and `.rtf` rows of the same `[Theory]`
pass.

The assertion (`TableAutoLayoutComparisonTests.cs:120-121`) compares how many text **runs** each
side put on a line. Measured: on the failing line LibreOffice writes **four** portions and we draw
**three**. The fourth, dumped from the PDF content stream, is:

```
Td 509.4 739.239:   /F1 11 Tf <08> Tj
```

A single glyph. Decoding the font's `ToUnicode` CMap gives **`0x08` → `' '` (space)**. LibreOffice's
ODF path emits the line-ending space as a text portion of its own; `pdftotext` correctly reports no
word there, and XRay.Office correctly draws nothing.

**Verdict: environment (LibreOffice version). A reference-side portion-splitting artifact.**
Drawing a trailing space or not is invisible in the output; a run count is a producer's internal
choice, not a fidelity property.

### 4.4 The footnote separator × 4 — **a genuine defect in XRay.Office**

Cases:
- `FootnoteComparisonTests.TheRuleAboveTheNotesGoesWhereLibreOfficeDrawsIt` — `footnotes.doc`, `footnotes.docx`
- `PdfOutputComparisonTests.EveryShadeAndRuleIsFilledWhereLibreOfficeFillsIt` — `footnotes.doc`, `footnotes.docx`

Again the ODF twins (`footnotes.odt`, `footnotes.fodt`) are in the same `[Theory]` and pass.

Measured, straight out of the reference PDFs' content streams:

| | separator width | separator top |
|---|---:|---:|
| LibreOffice, `.odt` | 120.450 pt | 757.451 pt |
| LibreOffice, `.docx` | **144.000 pt** | **755.251 pt** |
| XRay.Office, every format | 120.475 pt | 757.465 pt |

The test's reported figures reproduce to the digit: it says `144.000 pt rendered` and
`755.251 pt rendered`, which is what I get from the PDF arithmetic independently.

**144.000 pt is exactly 2 inches** — Word's footnote separator. I confirmed it is *absolute*, not a
percentage, by editing the DOCX's `w:pgMar` from 1134 to 3000 twips a side (text width 481.9 pt →
295.3 pt) and re-rendering: **the rule stayed exactly 144.000 pt**. A percentage would have shrunk
with the column; it did not.

XRay.Office's side is `XRay.Office.WordProcessing/Layout/Paginator.cs:170`:

```csharp
public double NoteSeparatorWidth { get; init; } = 0.25;
```

consumed at `Paginator.cs:1888`:

```csharp
Length width = page.TextWidth * _options.NoteSeparatorWidth;
```

There is **one global fraction and no format distinction**, and nothing anywhere in `src/`
overrides it. The doc comment on line 166-168 records the measurement it came from — "LibreOffice's
PDF export draws the rule from 56.7 to 177.15 pt… 120.45 of 481.89 is exactly 25%" — and that
measurement is *still exactly right today*, **for ODF**. It was generalised to every format.

**Verdict: genuine defect in XRay.Office.** LibreOffice 26.2.4.2 corrected its Word import to Word's
2-inch separator; we still apply Writer's 25 %-of-text-width rule to Word documents. The environment
change is what made the defect *visible*; the defect is ours and it is real.

*Confidence:* **high on both numbers**, and this was upgraded from "medium on the vertical" once the
source was found. `sw/source/core/layout/paintfrm.cxx:5845-5868` predicts **both** measurements
exactly, behind the Word-only `CONTINUOUS_ENDNOTES` flag of §5.1:

```cpp
if (rIDSA.get(DocumentSettingId::CONTINUOUS_ENDNOTES))
{
    // Word style: instead of fixed value, upper spacing is 60% of all space.
    aPoint.setY(getFrameArea().Pos().Y() + nPrintAreaTop * 0.6);
    ...
    // Length is 2 inches, but don't paint outside the container frame.
    nWidth = o3tl::convert(2, o3tl::Length::in, o3tl::Length::twip);
```

"Length is 2 inches" is the 144.000 pt I measured; "upper spacing is 60% of all space **instead of
fixed value**" is the 2.214 pt vertical shift. Both are commented *"Word style"*.

The brief rightly warns that this checkout is 27.2.0.0.alpha0+ and is **not** the reference binary.
That warning is respected here rather than ignored: the source is not the evidence, it is the
*explanation*. The evidence is the measurement against the installed 26.2.4.2 — 144.000 pt,
invariant under halving the text width — and the source predicts that measurement exactly. Source
and binary agreeing is the strong case; source alone would not have been.

### 4.5 `PageDrawingComparisonTests` × 4 + `TabStopComparisonTests` × 4 — one phenomenon, two tolerances

These eight look like two unrelated families. They are **one**, and this is the finding I would
most want carried forward.

**`PageDrawingComparisonTests`** (`paginated.doc/.docx/.fodt/.rtf`): line 1's right edge,
530.423 pt drawn against 529.903 pt rendered — over a 0.5 pt tolerance by **0.020 pt**.

A Shouldly assertion stops at the first failure, so the reported case is only the first. I
instrumented `Close()` to record every comparison instead of throwing, ran it, and restored the
file (`git status` clean; the instrumented run is not part of any commit). Full distribution over
all four documents — `pagedrawing-deltas.tsv`, **732 comparisons per side**:

| | n | median | p95 | max | over 0.5 pt |
|---|---:|---:|---:|---:|---:|
| left | 732 | 0.1000 | 0.1000 | 0.1000 | 0 |
| right | 732 | 0.4501 | 0.5180 | 0.5570 | **88 (12.0 %)** |

The left edge is a **constant 0.1000 pt** on all 732 — a fixed export offset, not an error. The
right edge is a systematic positive drift whose whole distribution hugs the tolerance: the 0.5 pt
bound cuts through the middle of it. This assertion was never bounding an error; it was bounding a
systematic offset that happened to sit under 0.5.

**`TabStopComparisonTests`** (`list-label-overrun` × 4): word 4 at 117.571 drawn / 117.442 rendered,
Δ 0.129 against a 0.1 pt tolerance. Instrumented the same way (`overrun-deltas.tsv`), the whole
line is diagnostic:

```
  w  1 A          delta=+0.0000   <- margin
  w  2 label      delta=+0.0186
  w  3 wider      delta=+0.0737
  w  4 than       delta=+0.1287   <== OVER
  w  5 its        delta=+0.1598   <== OVER
  w  6 stop.      delta=+0.1889   <== OVER
  w  7 Paragraph  delta=+0.0000   <- margin, drift resets
  w  9 Overrun    delta=+0.0000   <- tab stop, exact
  w 19 1.         delta=+0.0000   <- tab stop, exact
  w 20 Within     delta=+0.0000   <- tab stop, exact
```

**Every tab stop and every margin is exact to 0.0000 pt.** The test is named for tab stops and its
tolerance comment is about tab stops — and the tab stops are perfect. What fails is the drift that
accumulates *between* them, along a run of text, and resets at every hard position.

So both families are measuring the same quantity: **XRay.Office's horizontal advances run
systematically ~0.1–0.2 % wider than LibreOffice's**, accumulating along a line (~0.03–0.055 pt per
word) and resetting at any absolutely-positioned point.

**Verdict: environment (LibreOffice version) as the trigger — but it exposes a real, pre-existing,
sub-point divergence in our text measurement.** This is not a new defect and it is not noise; it is
a genuine ~0.1 % disagreement that both tolerances were absorbing until LibreOffice's advances
moved slightly.

It also **contradicts a stated project assumption.** `dotnet/CLAUDE.md` says: "HarfBuzz is what
LibreOffice shapes with, so advance widths agree by construction." Measured, they do not — not by
much, but systematically and cumulatively. That sentence should not be relied on again without a
number beside it.

**Where the divergence actually lives — measured, and it is not rounding.** Both sides shape the
same string in the same face, so the *unkerned* advance sum is identical by construction. Computed
from Carlito's `hmtx` with fontTools for `paginated`'s line 1 (100 characters, 11 pt):

```
exact unkerned advance sum      462.747 pt
XRay.Office's line width          459.573 pt   -> we remove 3.174 pt
LibreOffice's, from its own PDF  458.975 pt   -> it removes 3.772 pt
```

**LibreOffice applies ~19 % more kerning than we do on this line** (3.772 vs 3.174 pt). The
divergence is in the *shaping*, not in a rounding step — which is a much better lead for whoever
picks this up than the one I started with.

*I checked the obvious alternative before believing this.* If the paragraph were **justified**, both
sides would stretch to their own measure and a 0.6 pt gap would mean our text area was 0.6 pt wide,
not that our kerning differed. Measured: `paginated.docx` declares `<w:jc w:val="start"/>` — left
aligned — and the reference's line right-edges are ragged (529.903, 530.596, 531.212, 532.697,
533.643, …), not flush. Justification is ruled out.

That check also **settles the left edge in our favour**, which is worth stating because §4.5's other
column depends on it. The document declares `w:left="1417"` twips = **70.850 pt**, and XRay.Office
puts the pen at exactly **70.850**. LibreOffice's PDF puts it at 70.950. So the constant 0.1000 pt
seen on all 732 left comparisons is **LibreOffice's export offset, not our error** — which is
precisely what `TabStopComparisonTests.cs:37-42` documents `PdfPenOffsetPoints` to be, now confirmed
from the source document's own margin rather than from a fitted constant.

*Inferred and explicitly not established:* I first suspected quantisation, because the tab-stop
drift steps sit near whole multiples of 1/100 mm (0.02835 pt) — 1.94, 1.94, 1.10, 1.03 units on
consecutive words. Other steps (0.66, 2.50, 3.47, 4.20) do **not** fit, and the kerning measurement
above is a cleaner account of the same 0.6 pt. I record the quantisation idea as **refuted-ish
rather than supported**, and the kerning gap as the measured fact.

I did **not** determine which side's kerning is right. That needs a per-pair comparison against
Carlito's `kern`/`GPOS` tables, which is a round of its own — and it is worth one, because
`dotnet/CLAUDE.md`'s "advance widths agree by construction" is the assumption it would test.

---

## 5. The notes, pagination and frame families — 9 cases

### 5.1 One LibreOffice compatibility flag accounts for 5 of them, and for §4.4 as well

`EndnoteComparisonTests` ×1 (`endnotes.docx`),
`FootnoteComparisonTests.EveryNoteSitsAtTheFootOfItsOwnPage` ×2 (`footnote-pages.doc/.docx`),
`NoteRestartComparisonTests` ×2 (`note-restart.doc/.docx`).

All five are DOC/DOCX; **every ODF twin in the same `[Theory]` passes**. That polarity, which is now
the round's recurring signature, traces to a single setting: `DocumentSettingId::CONTINUOUS_ENDNOTES`,
set **unconditionally** by both Word filters — `sw/source/writerfilter/filter/WriterFilter.cxx:338`
(DOCX) and `sw/source/filter/ww8/ww8par.cxx:2050` (DOC) — and by neither ODF filter. It switches
three things at once:

- **The note container's top border** becomes the default paragraph's line height (≈13.4 pt here)
  instead of Writer's `TopDist+BottomDist+LineWidth` = 57+57+10 twips = 6.2 pt
  (`sw/source/core/layout/ftnfrm.cxx:257-272`). Measured separator positions confirm it: the ODF
  rule sits at y 108.339/157.139, DOC and DOCX at 110.539/159.339. That ≈7 pt is what evicts one
  body line from page 1 — which is exactly what cases 2–5 report.
- **The separator's length.** `sw/source/core/layout/paintfrm.cxx:5845-5868` carries the comment
  *"Length is 2 inches"*. **This is §4.4's 144.000 pt, found independently in the source.**
- **Endnote placement**: laid out inline at the end of the body rather than on their own page,
  which is case 1. Measured: `endnotes.docx` page-1 text bottom 740.53 pt (1 page); `endnotes.odt`
  702.30 pt (2 pages). `endnotes.doc` passes because that file states section-end placement
  explicitly and we honour it; the DOCX states nothing and LibreOffice now applies a compat
  *default* we do not implement.

The enabling commits are the `tdf#160984` "sw continuous endnotes" series — `d74fb6b5713`
(2024-05-16, DOC) and `1ae5ea3f78c` (2024-05-21, DOCX) — both after the 24.2 branch point, so
present in 26.2.4.2 and absent from 24.2.7.2.

**Verdict: environment (LibreOffice version) for all five.** Cases 2–5 are pagination, not text and
not numbering — verified word-level: `note-restart` cites 1,2,3,4 on each page identically on both
sides, and a `SequenceMatcher` over `footnote-pages` page 1 gives exactly one non-equal opcode, the
same eleven words, in both DOC and DOCX. We simply keep one more body line on page 1.

**This also strengthens §4.4 rather than replacing it.** The 2-inch rule is not an accident of the
new binary: it is LibreOffice deliberately implementing Word's behaviour behind a Word-only compat
flag, with a source comment saying so. XRay.Office applying Writer's 25 % rule to Word documents is a
real defect, and it now has a source-level citation as well as a measurement.

### 5.2 `JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt` ×1

`justify-shrink-2013.docx`: we set it in 4 lines, LibreOffice in 5. The companion
`justify-shrink-2007.docx` matches line for line on both sides, which **rules out any font-metric
explanation independently** — identical metrics, different shrink decision. The compatibilityMode-15
flag is still imported; the algorithm behind it was rewritten toward MSO's. Best candidate:
`529755f0919` (2025-06-02), *"tdf#166113 sw smart justify: adjust algorithm for interoperability"*.

**Verdict: environment (LibreOffice version).** Which side is *right* is undetermined — the change
was made in the name of matching Word, which suggests the reference is now closer, but that was not
proven and 24.2.7.2 could not be installed to A/B it.

### 5.3 `JustificationShrinkComparisonTests.TheReferenceItselfSetsTheModeFifteenDocumentInFewerLines` ×1 — **the test does not test XRay.Office**

This is the distinct category the brief asked to have flagged, and it is real
(`JustificationShrinkComparisonTests.cs:96-109`):

```csharp
List<double> newer = Rendered(Corpus.Require("justify-shrink-2013.docx"));
List<double> older = Rendered(Corpus.Require("justify-shrink-2007.docx"));
newer.Count.ShouldBeLessThan(older.Count);
```

`Rendered()` (`:111-114`) is *only* `PdfWords.Read(_libreOffice.ConvertToPdf(...))`. There is **no
`Drawn()`, no `WordProcessingReader`, no `Layout()` — XRay.Office never executes.** The test asserts
that the *installed binary* sets one fixture in fewer lines than another. Its own comment says so:
"Guards the fixture rather than the engine."

Measured: both documents now come out in **5 lines**, so `5 < 5` fails.

**Verdict: environment (LibreOffice version), and the only case in the 40 that is a candidate for
deletion rather than repair.** It can never be a statement about XRay.Office's correctness.

**But it should not simply be deleted, and this is the nuance worth keeping.** The flag still *has*
an effect — LibreOffice breaks line 1 of the 2013 document after "across" and the 2007 document
after "dam". The fixture is not dead; only its chosen proxy ("fewer lines") has expired. Restating
it as *"the two documents break differently"* keeps the guard the test was written to provide.

### 5.4 `FrameComparisonTests` ×2 — **unexplained, and deliberately left so**

`frame-parallel.odt` and `.fodt`: "line 4 resumes at 354.40 pt, where LibreOffice drew nothing."

Both sides place the frame **identically** — the frame's own 9 pt word "Both." is at (240.95, 110.48)
in ours and (240.95, 110.48) in the reference. The frame occupies x 240.95–354.35 with its top at
y 110.5; line 4's box is 97.0–110.4. **It touches the frame's top edge and does not cross it.** We
treat touching as overlapping and divide the line around the frame; 26.2.4.2 treats touching as
non-overlapping and runs the line straight through.

Confirmed as a pure boundary tie by nudging the frame in copies of the fixture and re-rendering:

| `svg:y` | LibreOffice 26.2.4.2, line 4 |
|---|---|
| −0.05 cm | divided — "pagination" at 354.5 (= XRay.Office) |
| −0.02 cm | divided — 354.5 |
| **0 cm (as shipped)** | **not divided** — "pagination" at 242.7 |
| +0.02 cm | not divided |
| +0.05 cm | not divided |

**Verdict: environment (LibreOffice version) as the trigger; which side is right is UNEXPLAINED.**
This is a one-twip tie-break on an exact-touch boundary, not a defect on either side. Geometrically
the reference's current answer is the more defensible one, but that is an opinion about a `>` versus
a `>=`, and it should be decided deliberately rather than re-baselined into existence. The test's
own remark (`FrameComparisonTests.cs:353-358`) records that the ODF forms *did* narrow that line
while the DOCX did not — so the old behaviour was already format-inconsistent.

**Caveat carried from this whole section:** 24.2.7.2 could not be installed for a true A/B — the
LibreOffice download hosts are firewalled. Every "reference moved" verdict here rests on the
measurements above, on LibreOffice commits dated after the 24.2 branch whose code paths were read
in this tree and whose effects were reproduced, and on §2's proof that these tests were green at
`0fb6d41e0` on 24.2.7.2.

---

## 6. The sheets and slides families — 8 cases

### 6.0 Two exclusions, both established by measurement rather than by argument

**Poppler is structurally unable to reach 6 of the 8.** `PdfTextRuns`, `PdfStrokes` and `PdfPaints`
inflate the PDF content streams and read the operators directly
(`PdfTextRuns.cs:66-83`, `PdfStrokes.cs:81,258`, `PdfPaints.cs:308`); only `PdfWords` shells out to
`pdftotext` (`PdfWords.cs:52,64-81`). Cases 1, 2, 4, 5, 7 and 8 never touch poppler at all. For the
two that do, it was exonerated by direct comparison against the content stream (§6.6).

**The font set is excluded for all 8, and this time by an actual A/B.** Every reference was
re-rendered with a fontconfig that rejects `/usr/share/fonts/truetype/dejavu/*` (`fc-list | grep -c
dejavu` → 0) into a fresh LibreOffice profile. The content streams are **byte-identical with and
without DejaVu** for all six documents. No render embeds a DejaVu face: these themes name DejaVu
only as the `ea`/`cs` face and all the text is Latin. This is the strongest form of the §3 result —
the variable was actually removed and nothing moved.

### 6.1 `SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt` ×1 — **an XRay.Office gap**

`sheet-rich-text.xlsx`, picture 1 on page 3: reference 94.904 pt wide, ours 95.074 pt.

`sc/source/filter/oox/drawingbase.cxx:267-300` (`calcCellAnchorEmu`) now clamps the `to` offset —
*"Excel seems to limit the offsets to the bottom/left edge of the cell… reduce cell's right edge by
a full twip"* — in commits `50421467b80` / `71dc2b20cc7` (Nov 2025). Height moved the same way
(46.800 → 46.658).

The test's own docstring records the 24.2.7.2 figure as **1.3201 in = 95.05 pt**
(`SheetDrawingComparisonTests.cs:19`), which is the raw anchor arithmetic and matches **ours**.
So we match the file and the old reference; LibreOffice deliberately changed to match Excel.

**Verdict: genuine gap in XRay.Office.** Parity now requires adopting the clamp. Same shape as §4.4:
the reference moved toward the owning application and we did not follow.

### 6.2 `SheetSpilledTextComparisonTests` ×2 — **the reference is dropping visible text**

`AStringSpillingPastAPageBreakIsDrawnOnBothSidesOfIt` and
`EveryPageShowsAsManyWordsAsLibreOfficeShows(xls-features.xls)`. One root cause, and it is decisive.

`sc/source/ui/view/output2.cxx:1542` now reads:

```cpp
if ( mnX1 > 0 && !bTaggedPDF ) --nLoopStartX;
```

The test's own docstring cites that line **without** the `!bTaggedPDF` guard
(`SheetSpilledTextComparisonTests.cs:33-35`). Calc skips the spill lead-in column when exporting
**tagged** PDF, and `officecfg/…/Common.xcs:4318-4324` now defaults `UseTaggedPDF` to `true`.

**Proof, by re-rendering with tagged PDF explicitly off: `[40, 34, 1213, 1011]` and 48 long runs on
page 4 — exactly what XRay.Office produces.** So XRay.Office draws what a reader actually sees, and the
reference silently drops visible text in its new default export mode.

**Verdict: environment (LibreOffice version), and a *fourth* environment variable nobody had
declared — the PDF export's tagging default.** XRay.Office is right.

Two details worth keeping. It is **not** a pagination change: both sides have 4 pages with identical
counts on pages 1–3. And the reported `963` rather than `1011` is a **harness artifact** —
`PdfWords.cs:117-119` matches `xMin="([0-9.]+)"`, which rejects the 48 words whose box starts at a
*negative* x. `1011 − 48 = 963`. The harness cannot see text that spills left of the page origin.

### 6.3 `SheetTextComparisonTests` ×1 — a one-twip rounding flip upstream

`sheet-cell-text.xlsx`, run 12: reference 261.666 pt, ours 261.3827 pt — Δ 0.283 pt = 6 twips, on an
`indent="2"` cell.

This is *exactly* the failure mode XRay.Office's own source comment predicts
(`XlsxCellFormats.cs:485-489`): *"Ten-point Liberation Sans has a 55.57-twip space, and rounding it
to 56 rather than truncating to 55 puts a two-level indent 0.3 pt out."* Reference = 6 × 56 twips =
16.8 pt; ours = 6 × 55 = 16.5 pt. LibreOffice's chain is `scaleValue(3.0 * indent, …)`
(`stylesbuffer.cxx:1263`) over `xFont->getCharWidth(' ')` (`unitconverter.cxx:139-142`).

**Verdict: environment (LibreOffice version).** A rounding-vs-truncation flip upstream, on a case our
own code comment had already anticipated and chosen the other side of.

### 6.4 `SlideTableComparisonTests` ×2 — **the clearest XRay.Office defect in the 40**

`EveryCellsTextIsDrawnWhereLibreOfficeDrawsIt` (baseline: ref 93.600, ours 91.899) and
`EveryGridLineIsTheStrokeLibreOfficeDraws` (grid line 14 `ToY`: ref 266.854, ours 263.925).

Measured: the reference's first baseline is `row_top + 3.6 + 18.000` — an ascent of exactly
**1.0 em** — and its line pitch is **21.600 = 1.2 em**. Ours uses the `hhea` ascent
(16.299 = 1854/2048 × 18) and a pitch of 20.150. Case 2 falls straight out of case 1: the `h="0"`
third row on slide 2 wraps to two lines, so reference = 2 × 21.6 + 7.2 = 50.4 and ours =
2 × 20.15 + 7.2 = 47.5.

The mechanism is `ImplCalculateFontIndependentLineSpacing` (`impedit3.cxx:500-504`, `×12/10`) plus
`impedit3.cxx:3138-3142` (`nAscent = font height`), enabled for table cells by commit
**`a47776a938c` (2025-03-27), tdf#165521, "pptx layout: don't use font's leading for cells too"**,
whose message says: *"Microsoft just ignores the font metrics, and simply adds 20 % to the font
height."*

**Verdict: genuine defect in XRay.Office — and an unusually actionable one. XRay.Office already applies
the 1.2 rule to pptx text boxes** (its autofit box pitch is 14.4 = 1.2 × 12); it simply does not
apply it to **table cells**. The rule is implemented and in the tree; it is not reaching one call
site.

### 6.5 `SlideAutofitParagraphSpaceComparisonTests` ×1 — a second reference-pinning test

`SlideAutofitParagraphSpaceComparisonTests.cs:91-93`:

```csharp
SizeAt(theirs, 10).ShouldBe(SizeAt(ours, 10), TolerancePoints, "150 pt box");
```

It fails because the **reference** now picks 14.003 / 18.992 / 20.013 where 24.2.7.2 picked
11.99 / 17.01 / 18.00. Lines 86-88, `SizeAt(ours, …).ShouldBe(11.99 …)`, **still pass** — so
XRay.Office reproduces the old binary exactly, and the whole test, literals included, is calibrated to
24.2.7.2. Cause: the autofit search was reworked in `f61ea135430` / `db64748f1ee` / `a3daf52dd21`
(Mar–Apr 2024, first shipped in 24.8).

**Verdict: environment (LibreOffice version).** Note the test's *thesis* survives intact — the
reference's paragraph gap is 9.580 against a nominal 12 pt `spcBef`, so the fit's spacing scale
(~0.8) still does reach the paragraph's own space. **Only the numbers moved, not the claim.**

### 6.6 `SlideChartFaceComparisonTests` ×1 — a third reference-pinning test

`SlideChartFaceComparisonTests.cs:125`:

```csharp
theirs.ShouldBe(MonospacedDigitAdvance, 0.1, "the reference's digit advance");
```

A literal assertion about the **reference binary's** output. Poppler is exonerated by direct
measurement: the reference's content stream has `83.703 … Tm … [<02>16<06>16<06>]TJ` for "100" and
`89.542 … Tm` for "80", and `pdftotext -bbox` reports `left=83.7030` / `89.5420` — poppler
reproduces the pen exactly. The 5.839 comes from LibreOffice now emitting a `16`/1000-em tightening
between every glyph (6.003 − 0.16 = 5.84); XRay.Office writes a single `<…>Tj` with no adjustment,
giving the true 6.009.

**Verdict: environment (LibreOffice version). The test's actual claim still holds** — 5.839 is still
far nearer Mono's 6.01 than Sans' 5.556, which is what the test set out to prove. Only the 0.1 pt
tolerance breaks.

*Not determined:* which release first flipped `UseTaggedPDF` for the headless `--convert-to pdf`
path (§6.2). The schema default changed in `544d6d781b3c` (2023) but the `output2.cxx` guard arrived
in `b3c93b16d62` (Jan 2024), and this repo carries no release tags to resolve the branch point. It
does not affect the verdict — the untagged re-render reproduces XRay.Office's numbers exactly, which
isolates the cause regardless of when it shipped.

---

## 7. The prediction, scored against what was measured

Committed at `2420db95528` before the suite was run once. Scoring it honestly matters more than
scoring it well.

| | prediction | outcome |
|---|---|---|
| **P1** | 550 unchanged, only the split moved | **Right.** 550 discovered, 510+40, 0 skipped |
| **P2** | 21 names / 40 cases are parameterised rows, no second mechanism | **Right.** 9 names carry the extra 19 rows |
| **P3** | plurality is LibreOffice-version, ~20-25, concentrated in **hard-coded figures** | **Half right, and the reasoning was wrong.** The class is right and far larger than predicted (~34). The *mechanism* is backwards: the hard-coded figures all **pass**; it is the live comparisons that broke (§8) |
| **P4** | 2-6 font-set failures | **Wrong — zero.** Every failing document embeds Carlito alone. I reasoned from `MISSING_PACKAGES.md`'s corpus-wide result to a suite that shares none of its documents |
| **P5** | 3-8 poppler failures | **Wrong — zero.** Poppler tracks the PDF's own geometry to 0.022 pt (§3) |
| **P6** | ≥1 genuine defect hiding behind the environment story; 1-8 | **Right, 4** — the footnote separator (§4.4), plus a real sub-point advance divergence underneath 8 more (§4.5) |
| **P7** | 1-5 unexplained | **Right, 2** — the frame boundary tie (§5.4), plus the unreduced trigger in §4.1 |

**The two predictions I was most confident about after P1/P2 — the font set and poppler — were both
exactly zero.** I had reached for the environment variables the brief handed me and assumed they
reached this suite because they reach the corpus. They do not: the corpus is 534 mixed real-world
documents, half of which resolve a fallback; the Fidelity fixtures are hand-built Latin probes in
Carlito. **Checking which fonts LibreOffice actually embedded took one `grep` and refuted both.**

That is the same failure the brief warns about, committed by me in writing before measuring, and it
is why the prediction was committed first.

---

## 8. The brief's leading hypothesis, tested rather than accepted

> "If Fidelity pins expectations to the old binary's output, some or all of the 40 are the
> environment moving rather than our code breaking."

**The first clause is largely false, and it changes what repairs are available.**

Fidelity does **not** pin stored figures. `LibreOfficeRunner` shells out to `soffice` and rebuilds
the reference **live, at test time**, on every run. Both sides move together. That is why "the
stored figure is stale, re-baseline it" — the repair the brief anticipated — applies to almost none
of the 40.

The suite *does* contain hard-coded figures read off 24.2.7.2, and they are exactly where you would
expect: `TableAutoLayoutComparisonTests.cs:145-159` pins column widths as literal
`[InlineData("table-autofit.fodt", 160.6, 107.1, 214.1, …)]`. **Every one of those passes.** The
stored numbers are fine; it is the *live* comparisons that broke.

So the environment did not invalidate our recorded measurements. It changed the thing being
measured against — and in three of the five families it changed it **for the worse**.

---

## 9. What I propose to repair, and what I actually changed

**What I changed: nothing.** No expectation was re-baselined, no tolerance loosened, no test
deleted. The two files I instrumented (`PageDrawingComparisonTests.cs`, `TabStopComparisonTests.cs`)
were restored byte-for-byte from copies taken first; `git status` on `dotnet/tests/` is clean and
the only committed artifacts are this document and the measurement `.tsv`s. The suite still reports
40 red, honestly.

That is deliberate. Every one of these repairs is a judgement about *which side is right*, and on a
project whose recorded failure mode is "the number reproduces and the sentence attached to it is
wrong", shipping five of them in one round on one agent's reading is how a suite stops meaning
anything. Below is what I would ship, each justified separately, in the order I would do it.

### 9.1 `ExtractionComparisonTests` × 5 — a documented deviation, but the blunt form is not good enough

The obvious repair is a `KnownDeviations` entry for `tables.*` in the switch at
`ExtractionComparisonTests.cs:85`, in the style of the three quirks already recorded there, naming
the evidence: *LibreOffice 26.2.4.2's `Text (encoded)` filter absorbs the paragraph after a table
into a phantom row and duplicates it across the remaining columns; its own PDF rendering of the
same file emits it once, and so does the file.*

**I did not ship it, and the reason is worth recording rather than the entry.** `KnownDeviations`
suppresses **tokens**, not occurrences — the entry would have to allow `the`, `tables.` and `After`
across the entire document. `the` is one of the commonest words in the corpus; allowing it wholesale
means a genuine future loss of a different `the` from this document would be silently masked. The
existing `revisions.*` entry has the same weakness, which is presumably why nobody noticed.

The repair that is actually right is to make `FindMissingTokens` deviation-aware by **count**
(allow *two* extra `the`, not all `the`), which is a `TestKit.Comparison` change, not a one-line
switch case. Recommended, but as its own small piece of work with a test of its own.

*Confidence in the diagnosis: high — LibreOffice contradicting its own renderer settles which side
is wrong. Confidence that the blunt entry is the right repair: low, hence not shipped.*

### 9.2 `TableComparisonTests` × 3 — hold, and report upstream (recommend: do not change the test)

XRay.Office produces exactly the declared 454 twips; LibreOffice adds exactly the declared
`w:tblCellMar/w:top`. I would leave these three **red** rather than paper over them: they are a
correct test catching a reference regression, and a `KnownDeviations`-style suppression here would
hide it if LibreOffice fixes it back. The right action is an upstream bug report against
LibreOffice's Word-import handling of `hRule="exact"`, and a note in `TODO.md` so the next round
does not re-derive it.

*If* the suite must be green, the honest form is a skip with that reason attached — never a widened
tolerance, because 2.85 pt is not noise and a tolerance that admits it would admit a real defect.

### 9.3 `TableAutoLayoutComparisonTests` × 3 — the test is measuring the wrong thing, but the fix is not one line

The assertion to change is the run count at `TableAutoLayoutComparisonTests.cs:120`: whitespace-only
reference runs should be filtered out before counting. This is **not** a relaxation — a run count is
the producer's internal portion-splitting choice, and whether LibreOffice emits a line-ending space
as its own `Tj` is invisible in the rendered page. Filtering would make the assertion measure what
its own comment says it measures ("a column too narrow wraps a cell that should not wrap").

**I attempted this and stopped, because it is not small.** `PdfTextRun`
(`PdfTextRuns.cs:29-36`) carries `PageIndex, X, Y, FontSize, FontResource, GlyphCount, Colour` —
**it does not carry the text**. There is no way to ask "is this run whitespace" without teaching
the reader to decode the font's `ToUnicode` CMap, and that class states outright that it is
"deliberately not a PDF parser". Filtering on `GlyphCount == 1` instead would be a guess that
discards legitimate single-glyph runs.

So: recommended, sized honestly as a `TestKit` change rather than a test tweak. I identified the
offending run by decoding the CMap by hand for this document only (`0x08` → `' '`), which is fine
for a diagnosis and not a basis for a shipped filter.

### 9.4 The footnote separator × 4 — a real defect; diagnose now, fix as its own round (recommend: do not fix here)

Diagnosed to file and line:

- **Where the file's own answer is discarded:** `XRay.Office.WordProcessing/Ooxml/DocxFile.cs:293`
  drops `w:type="separator"` pseudo-notes as "drawing furniture, not content". True for extraction,
  but the layout side then has nothing to draw from.
- **Where the wrong value is substituted:** `XRay.Office.WordProcessing/Layout/Paginator.cs:170`,
  `NoteSeparatorWidth = 0.25`, consumed at `Paginator.cs:1888` as
  `page.TextWidth * _options.NoteSeparatorWidth` — one global fraction, no format distinction,
  overridden nowhere in `src/`.
- **What the file actually says:** `word/footnotes.xml` carries
  `<w:footnote w:type="separator"><w:p><w:pPr><w:rPr><w:sz w:val="12"/></w:rPr></w:pPr><w:r><w:separator/></w:r></w:p></w:footnote>`
  — Word's 2-inch rule, in a 6 pt paragraph. That single element accounts for **both** discrepancies:
  the 144 pt width *and* the 2.214 pt vertical shift (the 6 pt paragraph's own metrics).
- **The mechanism to express it already exists:** `PaginationOptions.Word` (`Paginator.cs:32-37`),
  already selected by `DocxReader`, `RtfReader` and `DocReader` while ODF uses `.Default`.

**Why I did not ship it.** The small version — an absolute 144 pt on the `Word` preset — **turns no
test green**, because `FootnoteComparisonTests.cs:227` also asserts the rule's `Y`, and that is
still 2.214 pt out. So the cheap fix costs a behaviour change for every Word document with
footnotes and buys nothing measurable, which fails the brief's own test for shipping a fix. The
version that *is* correct — read the separator pseudo-note and lay it out with its own paragraph
metrics — is a real piece of work and deserves its own round with its own prediction.

### 9.5 `PageDrawing` × 4 + `TabStop` × 4 — do not touch the tolerances yet (recommend: measure first)

This is where I most expect a future round to do the wrong thing, so I want the reasoning on record.

The tempting repair is 0.5 → 0.75 and 0.1 → 0.2. **Resist it.** Those numbers would be chosen to
clear the observed maxima (0.557 and 0.189) and nothing else — the definition of a silent
re-baseline. And they would blunt genuinely sharp instruments: `TabStop`'s 0.1 pt bound is two
twips, and its comment explains that it exists to catch a default tab interval of 720 twips where
LibreOffice uses 709. Doubling it to catch a 0.13 pt drift would also stop catching that.

What these eight actually show is a **systematic ~0.1–0.2 % advance divergence** that both tests
were absorbing. The right repair is to characterise and fix *that*, after which both tolerances can
stay where they are. Until then I would leave them red: they are correctly reporting a real
divergence, and the divergence is the more valuable object.

The cheap, honest interim option — if red must be cleared — is to split the assertion: keep the
tight bound on **absolutely positioned** points (margins and tab stops, where we are exact to
0.0000 pt, measured on 10 of 24 words) and assert the drift **per unit of text** rather than
absolutely, so the bound stops depending on how long the line happens to be.

### 9.6 The two defects the sheets/slides half turned up

**`SlideTableComparisonTests` ×2 (§6.4) — recommend: fix, and it is small.** This is the one place I
would spend a following round's budget first. XRay.Office already implements the rule (a 1.2 × font
height pitch with a 1.0 em ascent) and already applies it to pptx **text boxes** — its autofit box
pitch is 14.4 = 1.2 × 12. It simply does not apply it to **table cells**. That is a call-site gap,
not a missing feature, and it is measurable in one test class in seconds. I did not ship it only
because I did not find and verify the call site myself — it came from the parallel investigation and
the brief's standard is that I measure what I ship.

**`SheetDrawingComparisonTests` ×1 (§6.1) — recommend: adopt the clamp.** `calcCellAnchorEmu`'s
"reduce the cell's right edge by a full twip" is Excel's behaviour, and the test's own docstring
already records the pre-change figure, so the before/after is documented in the test.

---

## 10. What I would do next, in order

1. **`SlideTableComparisonTests` ×2** — smallest real defect, rule already in the tree, one call site
   (§6.4, §9.6).
2. **`CONTINUOUS_ENDNOTES` ×9** — the largest single root cause in the 40 and the largest single
   parity gap, spanning the note separator's width *and* vertical placement *and* endnote placement
   *and* note-container spacing (§4.4, §5.1). One flag, four behaviours, nine tests.
3. **The kerning gap (§4.5)** — 8 tests, and it tests `dotnet/CLAUDE.md`'s "advance widths agree by
   construction", which is measurably false. Worth a round on its own before anyone widens a
   tolerance to hide it.
4. **`MISSING_PACKAGES.md`** — add the missing CFF-flavoured font (§1) and the PDF export's tagging
   default (§6.2). Both are undeclared environment inputs; both present as something else.
5. **The three reference-pinning tests** (§5.3, §6.5, §6.6) — decide deliberately whether a test that
   asserts the installed binary's own output belongs in a parity suite. In all three cases the
   *claim* survives and only the number broke, so restating beats deleting.

## 11. Honest limits of this round

- **No A/B against 24.2.7.2 was possible** — the LibreOffice download hosts are firewalled. Every
  "the reference moved" verdict rests on measurement against 26.2.4.2, on upstream commits dated
  after the 24.2 branch whose code paths were read in this tree and whose effects were reproduced,
  and on §2's proof that these tests were green at `0fb6d41e0` on 24.2.7.2. That is strong, but it
  is not the direct comparison, and it should not be quoted as one.
- **§4.1's trigger was not reduced.** Three hand-built minimal fixtures failed to reproduce the
  phantom row. The misbehaviour is measured; its trigger is unexplained.
- **§5.4 is unexplained on purpose.** A one-twip exact-touch tie is a `>` versus `>=` decision, not a
  defect, and I declined to pick a side by re-baselining.
- **§4.5's mechanism is open.** The accumulation and the 19 % kerning gap are measured; which side's
  kerning is correct is not established.
- Two of the eight sheets/slides cases and all nine notes/pagination cases were measured by parallel
  investigation rather than by me directly. I verified the load-bearing citation myself
  (`paintfrm.cxx:5845-5868`, quoted in §4.4) and it held exactly; I did not re-verify every other
  line reference.
