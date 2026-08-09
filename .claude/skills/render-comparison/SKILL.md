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

## Naming the element that differs: `pdf-ops.py`

The image diff finds the *pages* worth looking at. This finds **what is on them**.

```sh
.claude/skills/render-comparison/scripts/pdf-ops.py dump ours.pdf --page 6
.claude/skills/render-comparison/scripts/pdf-ops.py diff ours.pdf ref.pdf --page 6
.claude/skills/render-comparison/scripts/pdf-ops.py diff ours.pdf ref.pdf --only fill
```

It walks the content stream and emits typed, positioned records — text shows with position,
effective size, face, glyph count and how many operators carry them; fills and strokes with
bounding box and colour; images with box and name. `diff` pairs them by nearest neighbour and
reports three lists: **only in ours**, **only in the reference**, and **drawn by both,
differently**.

**Build this into the loop instead of reading content streams by hand.** Seven consecutive
rounds each shipped a throwaway script doing part of it — `pdfops.py`, `sl14-compare.py`,
`compare-fit.py`, `read-autofit.py`, `sl14-pagediff.py`, `fold-check.py`, `affcmp.py` — and the
forensics, not the fixing, is where those rounds went.

### What it found on its first real run, and how the first reading of it was wrong

A workbook that **passes every existing check** — 34 pages against 34, 1828 words against 1828,
two fonts and both embedded. The diff reports 0 records one-sided and 13 drawn differently, every
one of them `face Carlito vs Carlito-Bold`, with identical glyph counts and positions.
`pdffonts` confirms it: ours embeds `Carlito` twice, the reference embeds `Carlito-Bold` and
`Carlito-Regular`. Nothing in the gate could see any of that, which is the point of the tool.

The sentence written next to it — *"we do not draw bold on that document at all"* — was wrong,
and it is worth keeping the wrong version above so the shape of the error stays visible. The
same signature then turned up on a **words** DOCX in an early batch, which already refutes
"a defect in the BIFF font path". Extracting the embedded font programs out of our own PDF and
reading their TrueType `name` tables settled it:

```
BaseFont AAAAAA+LiberationSans   ->  name6 'LiberationSans-Bold', name4 'Liberation Sans Bold'
BaseFont BAAAAA+LiberationSans   ->  name6 'LiberationSans',      name4 'Liberation Sans'
```

**We embed the right bold program and draw the right bold glyphs.** Our PDF writer was naming
`/BaseFont` from the font's *family* name instead of its *PostScript* name, so a bold face was
announced under the regular family's name. A real defect, and a metadata one: it reaches every
document we render in any non-regular face and moves **zero pixels** on any raster comparison.

Two lessons, both cheap to state and expensive to learn again:

- **A face-name difference is a claim about the PDF's metadata, not about its ink.** Confirm
  which by reading the embedded program's `name` table before sizing the fix. `pdffonts` reports
  the same `/BaseFont` string the diff does, so it corroborates nothing.
- **The instrument's first finding is the one most likely to be mis-read**, because there is no
  prior run to calibrate against. Budget a decisive check for it.

Fixing the writer rather than adding a tolerance to `pdf-ops.py` is deliberate: the false
positive then disappears at its source instead of being suppressed everywhere it might also be
telling the truth.

### Two design points worth knowing before you trust it

- **Matching is nearest-neighbour inside three points, not an exact key.** The first version
  keyed on position rounded to a tenth of a point and matched *nothing*: every show sat about a
  point from its counterpart (51.39 against 52.38), so every record reported as one-sided. If you
  see that shape, the window is too tight, not the document too different.

  **But the window is a matching device, not a verdict.** The comment beside it said for four
  rounds that the offset was "the two renderers putting their page origin in slightly different
  places", and that was wrong on the very workbook it quoted: an `.xls` cell's margin is 40 twips
  where every other format's is 20, and we had one shared constant. The tell is the *sign* — a
  page origin moves every run the same way, a margin moves left-aligned text right and
  right-aligned text left, and the reference did the second. Nobody looked, because the tolerance
  came with a reassuring story attached. **A sub-window offset is not evidence of anything
  benign**; a whole page sitting uniformly displaced is worth measuring, and a displacement that
  reverses with alignment is a margin.
- **Subset prefixes are stripped.** `AAAAAA+Carlito` and `BAAAAA+Carlito` are the same face and
  the prefix is assigned per file; `Carlito` against `Carlito-Bold` is what remains, and that is
  a finding.
- **A displaced element appears in *both* one-sided lists, and reading only one of them says
  "missing".** The match window is three points. Move something further than that and its two
  records find no partner, so it is reported once under *only in ours* and once under *only in
  the reference* — which is exactly what a genuinely absent element looks like from one side.

  This is not hypothetical: a brief of mine claimed a document's page 1 was "missing the
  reference's entire departmental title block", diagnosed from the reference-only half of a
  diff. We draw all of it, 14.15 pt too high. The round that inherited the claim had to spend
  part of itself disproving it, and the real defect on that page — a two-column section filling
  only its first column — is a different bug entirely.

  **Read both one-sided lists together and look for the same content in each** before concluding
  anything is absent. A pair that appears in both, with the same glyph count and text and
  different positions, is one displaced element and its offset is the measurement you want.

Text is decoded by joining `pdftotext -bbox` on position rather than by reading the subset's
ToUnicode CMap here — poppler already has that decoder, and reimplementing it is a large surface
to get quietly wrong. A record therefore always carries a glyph count, which never lies, and
carries words when poppler could read them.

## Attributing a mark to the source that made it: `trace-text.py`

`pdf-ops.py` says a record moved and quotes the text it holds. On a real document that word
appears thirty times, so the quote identifies nothing, and the next step is reading markup and
guessing which paragraph it was.

```sh
S=.claude/skills/render-comparison/scripts
$S/trace-text.py rewrite in.docx /abs/scratch/in.tok.docx --map /abs/scratch/map.tsv
$S/pdf-ops.py diff ours.pdf ref.pdf | $S/trace-text.py resolve /abs/scratch/map.tsv
$S/trace-text.py locate /abs/scratch/map.tsv 4XXXXXXXXXX
```

`rewrite` replaces every ASCII word in a zip-container document with a token unique across the
whole file. `resolve` reads a diff on stdin and annotates each line with the part, element,
ordinal and the original sentence that produced it:

```
text p1 (70.85, 747.10) 9.00pt AAAAAA+LiberationSans  1 glyphs in 1 show(s) "3 4XXXXXXXXXX"
    ↳ 4XXXXXXXXXX  word/document.xml  t[3].0  'Information'  ‹Information›
```

Supported: `docx`/`pptx`/`xlsx` and their variants, and ODF. **Binary `.doc`/`.xls`/`.ppt`
cannot be rewritten**, and converting one first would change the layout under study, which
defeats the purpose — use a sibling document in the same shape, or skip it.

### What it preserves, and the one thing it does not

- **The document's word count exactly.** Only maximal runs of `[A-Za-z0-9]` are replaced;
  whitespace, punctuation and every non-ASCII script are left alone. Measured with
  `paperless extract`, which reads the model rather than the page: 1142 words against 1142 on a
  twelve-slide deck, 604 against 604 on a workbook.

  **The `pdftotext` count can still drift a little, and that is not the document changing.**
  Extraction from a PDF re-infers word boundaries from geometry, and a token's glyphs have
  different advances from the word they replaced, so a gap that read as intra-word in the
  original can read as a space in the rewrite. Measured: exact on a DOCX (368/368) and on an
  XLSX (604/604), and +13 on 1142 — about 1% — on a deck full of small text frames.

  This does **not** weaken the comparison you actually run, because both renderers are given the
  same tokenised file and the drift applies to both. What it rules out is comparing a rewritten
  file's word count against the *original's*.
- **Character count wherever the counter fits.** A token is base-36 of its index padded to the
  original length with `X`, which is outside the base-36 alphabet and so cannot collide with an
  encoded digit. The run prints how many tokens came out *longer* than the word they replaced —
  that is exactly the population whose line breaking may have shifted.
- **Not width.** Equal character count is not equal advance. A rewritten document does not lay
  out identically to its original and **must never be used as a fidelity reference**. It is an
  instrument for attribution: run it on a document you already know differs, to find out which
  source run owns the difference.

Field codes are skipped by name — `w:instrText`, `w:fldSimple`, `c:f`, the ODF `text:page-number`
family. Rewriting one produces a document that still opens and renders something entirely
different, which is the worst failure mode a diagnostic can have.

### Why the resolver reads only the quoted part of a line

A short word mints a short token, and a short token is indistinguishable from an ordinary number.
The first version resolved `p1`, `17 glyphs` and the x-coordinate `342.78` against whichever
words held indices 1, 17 and 34, burying every real hit. So: when a line carries quoted text —
as every `pdf-ops.py` record does — only the quoted part is searched; when it does not, only
*padded* tokens count, because a padded token contains an `X` and cannot be a number.

One property to expect rather than debug: a record's quoted text comes from the positional
`pdftotext` join, so it can overrun into neighbouring words and `resolve` will name them too.
The first token listed is the one at the record's own position.

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

#### An aggregate `|ink|%` can never be smaller than the signed `ink%` beside it

`page_ink` is printed as `abs(sum(...))`, so summing the unsigned column cannot come out below
the signed one. That is a free correctness check on any table built from these numbers, and this
is the episode that shows why to run it.

**The defect:** a sweep script's `awk` matched the tool's own trailing summary line —
`N pages, M with major differences` — and added one per major page into the signed column. Not a
formula error; a parser eating a line it was never meant to see. Any script that pipes this
tool's output through a line filter is exposed, so the fix belongs there.

**The part worth learning is what happened next.** Round twenty *reported* a signed total of
1264.88. Round twenty-one recomputed from the same script, got 1692.88, and reported its
predecessor's figure as wrong — a correction I then relayed into this file. Round twenty-two
checked the invariant and inverted it back: 1692.88 exceeds that round's `|ink|%` of 1583.00,
which is impossible, and 1692.88 − 1264.88 = 428 is exactly the round's major-page count. **The
original number was right; the correction carried the contamination.**

So check the invariant on aggregates you produce, on aggregates you inherit, and — especially —
on corrections to either. A correction carries more authority than the figure it replaces, so it
gets examined less, which is exactly backwards: it has had one fewer pair of eyes on it.

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

**This has now happened often enough to plan around rather than treat as an exception.** A later
round found the source wrong *twice about the same constant* — the limit at which an axis wraps
its category labels — while the binary was right both times. Three probe boundaries, each
crossed by a different variable, bracketed the real value in `[0.990, 1.056]` of the tick
spacing; the code in the tree says something else, and a fix built on it had been turning an
axis two categories early and costing 20–42 pt of band.

The practical form of the rule: **a citation is a hypothesis with a file and a line number.** It
is more convincing than a bare guess and it is not evidence. Cite the source in the report by
all means, but derive the number from a probe against the installed binary, and say which of the
two each claim rests on. Several of this project's best rounds have cited a mechanism precisely
and then measured a different constant from the one the citation implies.

### An instrument can manufacture a defect out of nothing

Two measurement faults in one round each read as an enormous rendering error:

- A plot-rectangle reader taking the *modal span* of drawn lines returned a tick's 4 pt span
  instead of the plot's, and needed a gridline **47 of 61 chart parts do not draw**.
- The same reader, not restricted to the chart's own frame, reported **2161 pt** of error on a
  deck whose real error is **25.64 pt**.

Both numbers are absurd on their face and both were nearly acted on, because a large number
reads as a large finding. Before believing an instrument's first big result, **run it on a case
whose answer you already know** — a document that matches, or a probe you authored — and check
it returns something near zero. That costs one run and it has now saved two rounds.

## Recording results

Keep the reference version with the numbers — `soffice --version`. LibreOffice's layout
changes between releases, so a comparison is only reproducible against a stated version.
When a difference is genuinely LibreOffice's own import bug rather than a Paperless
defect, record it as a known deviation with a note, instead of contorting Paperless to
reproduce it.
