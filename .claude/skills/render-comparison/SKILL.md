---
name: render-comparison
description: Compare a Paperless rendering of a document against the LibreOffice reference rendering, and diagnose what the differences mean. Use when checking rendering fidelity, investigating why a rendered page looks wrong, reviewing a Paperless rendering change for regressions, or establishing/updating golden reference images. Covers choosing a comparison metric, reading diff images, and telling a layout bug apart from a drawing bug.
---

# Comparing a Paperless rendering against LibreOffice

Generate reference output with the **libreoffice-reference** skill first; this skill is
about comparing against it and interpreting the result.

## The one thing to internalise first

**Rendering errors cascade.** Almost every visual difference has one of two shapes, and
telling them apart is most of the diagnostic work:

- **Local difference** — a wrong colour, a missing shape, a wrong border. Confined to one
  area. The rest of the page still matches.
- **Reflow cascade** — one wrong measurement early on (a font metric, a margin, a line
  break) shifts everything after it. Half the page mismatches, subsequent pages mismatch
  wholesale, and the *reported* difference is enormous while the *actual* bug is one
  number in one place.

A raw "83% of pixels differ" tells you nothing about which you have. So never stop at a
single percentage. Look at *where* the differences are, and whether content moved or
changed.

Corollary: **fix cascades before chasing anything else.** A single wrong font metric can
manufacture hundreds of unrelated-looking failures across a corpus, and fixing it makes
them all disappear at once.

## Running a comparison

```bash
# 1. Reference output
.claude/skills/libreoffice-reference/scripts/lo-convert.sh --pdf --png --dpi 150 \
    --outdir /tmp/ref  document.docx

# 2. Paperless output at the SAME dpi -- see the pitfall below
dotnet run --project dotnet/tools/Paperless.Cli -- render --dpi 150 \
    --outdir /tmp/actual document.docx

# 3. Compare
.claude/skills/render-comparison/scripts/compare-images.py \
    --expected /tmp/ref/document.docx --actual /tmp/actual \
    --report /tmp/report.md --diff-dir /tmp/diffs
```

The comparison tool is pure Python with no third-party dependencies — it decodes PNG via
`zlib` directly, so it works in a bare container.

### Pitfall: both sides must be the same size

A DPI mismatch, or a page-size misread, makes every metric meaningless. The tool reports
`DIMENSION MISMATCH` and stops rather than producing a number that looks like a fidelity
score but is not. When you see it, fix the geometry first: it means Paperless read the
page dimensions wrongly, which is its own bug and a more fundamental one.

## The metrics, and which one to trust

`compare-images.py` reports several deliberately, because no single number distinguishes
the failure modes.

| Metric | What it is | What it is good for |
|---|---|---|
| `differing_fraction` | Fraction of pixels off by more than a tolerance | Trend over time. Noisy: antialiasing alone moves it |
| `mean_abs_error` | Mean per-channel difference across the page | A single "how close overall" number. Diluted by whitespace |
| `max_tile_error` | Worst mean error in any 32x32 tile | Catching a badly wrong *small* area a page average would hide |
| `ink_delta` | Difference in non-white pixel fraction | **Strongly negative ⇒ Paperless drew too little.** Something was skipped outright: an unsupported shape, a vector image that failed to decode |
| `shifted_tiles` | Tiles matching the reference well, but at an offset | **The cascade detector.** Non-zero ⇒ content *moved*, so suspect layout — font metrics, margins, line breaking — not drawing |
| `row_profile_shift` | Vertical offset best aligning the two ink-per-row profiles | Confirms a whole-page vertical shift and quantifies it |

### Reading them together

| Pattern | Diagnosis |
|---|---|
| Low everything | Match. Small `differing_fraction` with near-zero `mean_abs_error` is just antialiasing — expected and fine |
| `ink_delta` strongly negative, `shifted_tiles` ≈ 0 | Content missing. Look for an unimplemented feature |
| `ink_delta` ≈ 0, `shifted_tiles` high | Reflow cascade. Everything is drawn but positioned wrongly. Find the *first* divergence down the page — the rest is downstream of it |
| `max_tile_error` high, `mean_abs_error` low | One small region badly wrong. Usually a colour, a border or one shape |
| `ink_delta` strongly positive | Paperless drew something extra: a debug artefact, a wrongly-visible element, or a fill that should have been transparent |
| Everything huge, page 1 included | Not a detail bug. Wrong page size, wrong background, or a failed load |

### Do not gate on exact pixel equality

Two correct rasterisers disagree on antialiasing and sub-pixel rounding. Strict equality
fails on visually identical output. Set thresholds per metric, and prefer
`shifted_tiles == 0` plus a small `ink_delta` over any single aggregate.

The exception: LibreOffice's *own* output is byte-deterministic (verified), so a **golden
reference image** can be checksummed. A changed checksum means the reference genuinely
changed — usually a LibreOffice upgrade. Regenerate deliberately, never silently.

## Diff images

`--diff-dir` writes, per page, a side-by-side triptych: reference, Paperless output, and a
difference map with differing pixels highlighted.

**Look at these.** A number says a page is wrong; the image says *which part*, which is
what leads to a fix. When reporting a fidelity problem to a human, include the diff image
— it is the single most useful artefact this workflow produces.

## Isolating a difference

Once a page differs, narrow it down before theorising:

1. **Is it page 1 or only later pages?** Later-only strongly implies a cascade whose
   origin is on an earlier page.
2. **Bisect the document.** Delete content until the difference disappears. The last
   removal is where the problem lives.
3. **Check the fonts actually used.** Ask Paperless which face it resolved each request
   to and compare with what LibreOffice used. A silent substitution difference explains
   most mysterious reflows — run `check-env.sh` from the libreoffice-reference skill.
4. **Compare text before pixels.** If extracted text already differs, the bug is in
   parsing, not rendering, and is far easier to find there. Use the
   **extraction-comparison** skill.
5. **Round-trip the document through LibreOffice and render *that*.** The sharpest form of
   the sibling-format trick, and it takes one command:

   ```bash
   soffice --headless --convert-to fodt --outdir /tmp/rt problem.doc
   paperless render /tmp/rt/problem.fodt --format pdf --outdir /tmp/rt
   ```

   LibreOffice has now done the reading and Paperless only the layout. If the round-tripped
   file renders correctly, **the defect is in our reader** and the layout engine is
   exonerated; if it renders wrong the same way, the reverse. This localised a WW8 bug in a
   single step that had survived two rounds of layout theorising — blank paragraphs being
   dropped from DOC table cells, on a document that pads each of 106 entries with 21 of them
   so each takes a page.

   `.fodt`, `.fods` and `.fodp` are the right targets: flat XML you can also read directly to
   see what LibreOffice thinks the document says.

6. **Ask what the document actually contains before believing a theory about it.** Three
   diagnoses in this project named a mechanism the file does not use:

   - "an `sprmPDyaLine` at-least value is not reaching cell paragraphs" — that document
     contains no `sprmPDyaLine` at all, and LibreOffice's own flat-ODF export of it carries
     no line spacing on any paragraph involved.
   - "prefer EMF+ records over the downlevel EMF fallback" — those blobs contain zero EMF+
     records; there was nothing to prefer.
   - "chart category-axis labels are drawn touching" — the deck has no chart parts; the
     labels are inside an EMF and should be *rotated*.

   Grepping the decompressed part for the record or attribute you are about to blame costs a
   minute and refutes a plausible story before you spend a day on it.

7. **When extraction is right and rendering is wrong, suspect a value only rendering resolves.**
   The two paths read the same file and can resolve the same attribute differently — and a
   defect that lives only in the rendering path is invisible to every text comparison, which
   is exactly how it survives round after round.

   Measured twice in this project, both times the largest defect of their round:
   `<a:latin typeface="+mn-lt"/>` taken literally by the slide layout reader while extraction
   resolved it through the theme (DejaVu Sans against Carlito, 39% wider, so every line broke
   early and the tail fell off the slide); and the placeholder inheritance chain resolved for
   extraction and ignored by rendering.

   The tell is a document whose extracted text is perfect and whose page is wrong. Grep the
   attribute name and check *both* readers handle it — a value resolved in one and taken
   literally in the other is a standing bug waiting for a corpus big enough to show it.

8. **A word count can fault output that is geometrically perfect.** The ceiling case is
   well known — LibreOffice rasterises and we draw real text, so we score worse for being
   better. The inverse exists too and is easier to miss, because it looks like a layout bug.

   Measured on one `.xls`: 34 pages against 34, **2019 words against 1828**, the whole excess
   being `http://www.` appearing as its own token 48 times. The cells state `no-wrap`, so a
   wrap looked like the obvious cause — and `pdftotext -bbox` refuted it outright, reporting
   our `http://www.` as spanning the width of the *entire* URL. The glyphs are in the right
   place; only the text layer splits.

   The cause is operator granularity. A `TJ` array carries kerning inside one show, so a
   string of any length stays one token; a sequence of `Tj` with a `Td` between segments does
   not, and poppler reads each reposition as a word boundary. Ours capped at 28 glyphs per
   operator against the reference's single arrays, so any run longer than that fragments.

   **So when a page-exact document's word count moves, compare bounding boxes before
   diagnosing layout.** If the box of the first token spans the whole string, the layout is
   right and the defect is in how the text was written.

9. **Check whether the field is read but never used.** A property parsed by every reader and
   consumed by nothing is invisible to unit tests and produces a quiet, systematic error.

   This has now been the cause four times, once on each family, which makes it worth *grepping
   for on purpose* rather than waiting to trip over:

   | Property | Reach | Symptom |
   |---|---|---|
   | `TabStop.Leader` | 51 of 136 corpus DOCX | every dotted table-of-contents line blank |
   | `IsCapitalised` (`w:caps`/`w:smallCaps`) | — | wrong advance widths, so wrong line breaks |
   | `a:rPr/@baseline` | 29 of 112 corpus PPTX | superscripts on the baseline at full size |
   | `SheetGrid.IsOptimalSize` (`ht` without `customHeight`) | every XLSX with a height hint | row pitch 15.735 pt against 15.0 |
   | `WritingFieldKind.PageNumber` (`PAGE`) | every DOC/DOCX with a page-number field | footers print the producer's last cached result — `9` on all nine pages |

   Grep for the property name: **if the only hits are the readers and the model, it does
   nothing.** Every reader parsing it is what makes the bug so quiet — the parse looks like
   support, and four separate readers agreeing on a value nobody consumes reads as thoroughness.

   Note the last two entries are also a warning about *why* these survive. Escapement had
   nowhere to be stored (`SlideTextRun` carried a flag, not a magnitude) and the row-height flag
   has nothing behind it to call (`ScColumn::GetNeededSize` is a feature, not a line). A field
   read and never used is often blocked on something real rather than merely forgotten, so
   budget for the feature rather than expecting a wiring change.

## Comparing whole documents as images

`pdf-image-diff.py` renders two PDFs to PNG, diffs them page by page, groups the differing
pixels into regions, and reports where each region is and what kind of difference it looks
like.

```sh
.claude/skills/render-comparison/scripts/pdf-image-diff.py ours.pdf ref.pdf --outdir cmp
```

```
page    diff%   ink%    regions verdict
1       64.30   9.53    1       MAJOR
                                the whole page: ink missing from ours — a graphic, glyphs or a fill (70.30% of page, …)
2       60.56   33.60   3       MAJOR
                                middle-centre: a fill or background shading the reference has and we do not (49.99% of page, …)
```

### `ink%`, not `diff%`, is the column that decides

This is the whole design, and getting it wrong makes the tool useless. **Two renderers that agree about a page still differ on almost every glyph pixel**, because a two-pixel drift down the page lands every line somewhere slightly different. Measured on a plain one-page letter that matches the reference word for word: **8.98% of pixels differ and every paragraph is a region.** Judged on `diff%`, all 23 documents in a level-one sample were "major" — which is the same as saying none of them were.

So a region only counts when **one side has substantially more ink in it than the other**. `ink%` is the share of the page's ink the two sides cannot account for between them. A reflow moves ink and leaves it near nought; a missing fill does not:

| | `diff%` | `ink%` | verdict |
|---|---|---|---|
| letter matching word for word, drifted two pixels | 8.98 | **0.04** | `shifted` |
| deck missing its table-cell fills | 4.70 | **0.54** | `MAJOR` |
| deck drawing white boxes over its background | 60.56 | **33.60** | `MAJOR` |

`shifted` means the same ink in a different place — real, usually a font or spacing difference, and the extraction comparison is a better instrument for it than more pixels.

It writes `cmp/ours/page-NNN.png`, `cmp/ref/page-NNN.png`, and `cmp/diff/page-NNN.png` — the
reference faded to grey with each differing region boxed in red, which is the artefact to
look at when the text report is ambiguous. Exit status is 0 when no page differs majorly, 1
when one does, 2 when the comparison could not be made.

### A page a pixel taller than its twin is still the same page

`pdftoppm -scale-to` pins the longest edge and rounds the other from the aspect ratio, so two
renderings of a 16:9 deck whose page height differs in the second decimal of a point land on
**512×288 and 512×289**. The tool used to call that "page size differs", skip the page — and
count it as *major*, so a deck reported 22 major pages when it has 7. **Nine of the slides
track's 163 documents were unmeasurable for this reason alone**, which made any change to them
invisible to the one instrument that track still has.

It now crops both to the common size when they are within 2 px and 1%, and says so in the
summary. Cropping rather than rescaling is deliberate: both images are anchored top-left and
within a rounding step of the same scale, so the worst drift is one pixel at the far edge —
inside the `DILATE = 3` tolerance the regions already carry. Resampling every pixel to remove a
difference smaller than the thing being measured would add noise, not remove it.

Beyond that slack it is a real paper-size or orientation difference and still refuses, because
there cropping would be hiding the finding rather than enabling it.

### Use it only once page counts and word counts already agree

This answers "the right text is on the right page, but does the page *look* right". It
cannot answer anything else, because if pagination differs then page 3 here is being
compared against a different page 3 there and every region it reports is an artefact of the
misalignment. **The script enforces this** — it exits 2 rather than produce a plausible,
meaningless report.

So the order is: page count, then extractable words, then this. Each earlier check makes the
next one meaningful, and running this one first wastes the effort and misleads you.

### What it is for

The word count is blind to anything that is not a word, and this project has now shipped a
document that passes it perfectly while being obviously wrong to a human. On
`Sylva introduction session.pptx` — **1115 words against 1115, an exact match** — the
reference draws dark teal table cells with white text on them and we draw pale cells, so the
white text is invisible against white. The image diff names it on the first page it appears:
*"a fill or background shading the reference has and we do not"*.

That is the class it exists to catch:

- a missing fill, gradient or background panel
- a missing or misplaced graphic, logo or chart
- a rule, border or table grid that is not drawn
- a whole block in the wrong place, or in the wrong colour
- text that is drawn but unreadable because what is behind it is wrong

### Reading the hints

The hint is coarse on purpose. It exists to tell you which of several very different
investigations to start, not to be right about the cause — a wrong guess that sends you to
the right part of the page still saves the search.

| Hint | Usually means |
|---|---|
| *a fill or background shading the reference has and we do not* | a shape's fill, a table cell background, a slide background |
| *ink missing from ours* | a graphic or a block of glyphs never drawn |
| *ink we draw that the reference does not* | we draw something it suppresses — an overflowing frame, a hidden shape, metafile text it rasterises |
| *a rule or border* | a table grid line, a paragraph border, an underline |
| *the same marks in a different colour* | theme colour resolution, or a fill/stroke mix-up |
| *marks displaced or reshaped* | the commonest and least specific: a font substitution, a wrong indent, a line break |

`marks displaced or reshaped` covering most of a page usually means a reflow, and the useful
next step is the extraction comparison rather than more pixels.

### Why 512, and what it deliberately cannot see

512 pixels on the longest edge is about a sixth of the rendered size. A half-point line-break
difference disappears at that scale and a missing background panel does not — which is the
whole point. It finds the differences that survive being squinted at.

It will therefore **not** find: sub-pixel positioning, hinting and antialiasing differences,
one-word reflows, or a font substituted for a metrically-compatible one. Those are the
fidelity suite's job, which compares PDF operators directly.

A page diffs in about a tenth of a second, so a fifteen-page deck takes three seconds and a
corpus batch is practical. Raise `--long-edge` when you need to see detail on one document;
do not raise it for a sweep.

### Tuning

- `--threshold` (default 40) — how far a channel must differ for a pixel to count. Two
  renderers that agree about where a glyph is still differ by 20–30 along its antialiased
  edge, so lowering this floods the report with noise.
- `--min-area` (default 0.0004) — the smallest region worth reporting, as a fraction of the
  page. About 100 pixels at 512, which is a character or two.
- `--quiet` — only pages with major differences, for sweeping many documents.

## Turn a measurement into a name

A line advance is not an opaque number. It is `(ascent + descent + gap) / unitsPerEm × size`
for one specific face, so **you can invert it and find out which face is being used.**

Worked example, which is how the largest single defect in this project was found. Our line
advance was 12.65 pt where LibreOffice's was 13.45, at 11 pt:

```
12.65 / 11 × 2048 = 2355   → Liberation Serif's line box
13.45 / 11 × 2048 = 2500   → Carlito's
```

Not a line-height rule at all — the paragraph was being laid out in the wrong font. The
document's runs named `w:asciiTheme="minorHAnsi"`, a reference into the theme's font scheme
whose minor Latin face is Calibri; nothing read `w:asciiTheme`, so they fell back to Times
New Roman. 112 of 136 corpus DOCX files name their fonts that way.

The same arithmetic, run the other way, identified DejaVu Sans as the reference's face on a
whole family of documents: 12.80 pt per 11 pt line and 10.50 per 9 pt is `hhea`
1901/−483/0. Dump `head.unitsPerEm` and `hhea` for the candidates and match.

**So when a metric is wrong by a few percent, suspect the wrong face before suspecting the
rule.** A rule error usually gives a clean ratio (1.15, 1.2, 2.0); a substitution error gives
an arbitrary one that resolves exactly against some installed font's tables.

## Ask LibreOffice what it computed, instead of inferring it

When a difference comes down to a number LibreOffice worked out — a column width, a line
height, a margin, a resolved font — **export the document to flat ODF and read the number
out of the XML**. It is faster than inferring it from pitch, and it is not subject to the
compounding that makes an inferred figure ambiguous.

```sh
soffice --headless --convert-to fods --outdir /tmp/rt book.xlsx    # fodt / fodp likewise
grep -o 'style:column-width="[^"]*"' /tmp/rt/book.fods | head
```

Worked example. A column width in SpreadsheetML is a count of digits of the workbook's
default font, and Paperless had the twips-per-digit figure hardcoded. Measuring the *pitch*
of the two renderings gave 137 twips per digit; the flat-ODF export gave 886.2, 6591.2,
1389.0 and 1182.1 twips against digit counts of 6.664, 49.555, 10.441 and 8.887 — **133.0,
four times over**. The 3% discrepancy was a separate fit-to-page zoom difference, and
chasing the inferred 137 would have produced a fudge factor that fitted one document.

The same export settles rounding questions that no amount of source-reading will: probe
workbooks came back 111.50 → 111 and 121.64 → 121 but 139.97 → 140 and 152.70 → 153, which
says a device's quantisation decides them rather than a rounding rule.

## The C++ in this tree is not the reference binary

The checkout is a development branch; the `soffice` generating your references is a release
(24.2.7.2 here). They disagree, and the source is the more persuasive of the two, which makes
it the more dangerous.

Two diagnoses in this project were inverted by checking the installed binary instead of the
code beside it — most sharply, the source said an unknown font family falls back through
LibreOffice's own generic lists, while the running binary demonstrably takes fontconfig's
answer. Reading the source alone would have produced a confident, wrong, and thoroughly
cited fix.

Read the source to learn *what mechanism exists*. Measure the binary to learn *what it does*.
When they disagree, the binary wins, because the binary made the reference PDFs.

## Recording results

Keep the reference version with the numbers — `soffice --version`. LibreOffice's layout
changes between releases, so a comparison is only reproducible against a stated version.
When a difference is genuinely LibreOffice's own import bug rather than a Paperless
defect, record it as a known deviation with a note, instead of contorting Paperless to
reproduce it.
