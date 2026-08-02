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

7. **Check whether the field is read but never used.** A property parsed by every reader and
   consumed by nothing is invisible to unit tests and produces a quiet, systematic error.
   Two were found this way — `TabStop.Leader` (declared by 51 of 136 corpus DOCX files, so
   every dotted table-of-contents line was blank) and `IsCapitalised`. Grep for the property
   name: if the only hits are the readers and the model, it does nothing.

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
