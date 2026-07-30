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
5. **Try the same content in a sibling format.** If a `.docx` differs but the equivalent
   `.odt` does not, the bug is in the DOCX reader, not in layout.

## Recording results

Keep the reference version with the numbers — `soffice --version`. LibreOffice's layout
changes between releases, so a comparison is only reproducible against a stated version.
When a difference is genuinely LibreOffice's own import bug rather than a Paperless
defect, record it as a known deviation with a note, instead of contorting Paperless to
reproduce it.
