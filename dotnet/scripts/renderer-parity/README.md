# Cross-renderer parity sweep

Measures XRay.Office *and* a second independent implementation against the same
LibreOffice reference over a whole corpus, so the question "how close are we"
gets an answer with something to compare it to. The second engine here is the
Rust/WASM `office-open-xml-viewer`, driven in headless Chromium through
Playwright; a third engine only needs a new `render_*` step that writes PDFs or
per-page PNGs into `/data/bench/<engine>/<id>/`.

This is a corpus instrument, not a gate. `render-comparison` in
`.claude/skills/` is still the right tool for a single document.

## Pipeline

Everything is resumable and keyed on a stable document id, so a run that dies
half way is restarted by re-invoking the same command.

```bash
python3 manifest.py              # corpus -> /data/bench/manifest.tsv, one id per document
python3 render_lo.py             # reference PDFs   (4 soffice workers, private profile each)
python3 render_pl.py             # XRay.Office PDFs   (xray-office render --format pdf)
node    render_wv.mjs            # viewer PNGs      (Vite + Chromium, 3 contexts)
python3 score.py --shard N --shards 4   # per-page metrics -> /data/bench/scores/<id>.json
python3 aggregate.py             # -> summary.json, documents.json
python3 report_data.py           # -> report.json
python3 gallery.py 3             # -> gallery.json (base64 side-by-side JPEGs)
python3 build_page.py            # -> a single self-contained HTML report
```

`triptych.py <id> <page> <out.png>` builds a three-engine image for one page when
you want to look at something the metrics flagged.

To produce the single-engine catalogue -- every document that does not match, worst
first, each with the divergent page side by side:

```bash
python3 pl_cases.py     # the non-matching set -> pairs-view/*.jpg to read from,
                        # plus an embeddable WebP per case, in pl-cases.json
# read pairs-view/NNN.jpg and write one reading per case into analysis/NNN.json
python3 tag.py          # tag each reading with the defect classes it describes
python3 corrections.py  # fold the source investigation's corrections over the readings
python3 audit.py        # page alignment, word presence and resource counts, per case
python3 claims.py       # each reading's own <em> quotes, sought in both PDFs
python3 zorder.py       # text we draw and then paint over
python3 corrections2.py # fold the version-independent findings over the readings
python3 build_pl_page.py
```

`lanes.py` splits the non-matching set for parallel investigation, partitioned by
**source-file ownership** rather than by document count -- two agents editing one
`.cs` file is what produces a merge conflict, so each lane owns a disjoint set of
directories. `note.py` records one reading per rank as its own file, so the
reading pass is resumable.

`pl_cases.py` writes two images per document on purpose: a high-resolution JPEG to
read the defect from, and a small WebP to embed. Diagnosing from the embed size
means diagnosing from an image the defect may not survive.

`corrections.py` exists because **a reading is a hypothesis and some of them are
wrong.** Of the first 192 readings, eleven were refuted by measurement, three
documents turned out to be the reference at fault, and twenty-nine were neither
engine's error -- the tree is calibrated to LibreOffice 26.2.4.2 and this sweep's
reference is 24.2.7.2. It keeps those three kinds apart, preserves the original
reading beneath each correction rather than overwriting it, and gives the
catalogue a `not a defect` band so a version divergence cannot be counted as a
defect. A catalogue that silently rewrote itself would lose the record of what it
had claimed.

## What the numbers mean, and what they do not

`metrics.py` reports several figures per page rather than one, for the reason the
`render-comparison` skill gives: no single number separates "a shape is missing"
from "everything moved down three pixels".

**SSIM is the headline and it misranks on its own.** It collapses on
high-frequency content — a mottled slide background, a graph-paper rule — where a
half-pixel resampling difference flips every pixel while the page is visually
identical. The lowest-scoring XRay.Office slide of a 946-document run scored 0.49
and is indistinguishable from the reference by eye; its mean absolute error is
0.035. So `aggregate.py` calls a page defective only when a low SSIM is
corroborated by a second, independent metric (`DEFECT_*` at the top of the file),
and worst-case galleries are ranked on that, never on SSIM alone.

Three figures are kept apart on purpose and should not be collapsed:

| Figure | What it answers |
|---|---|
| `fidelity_rendered` | how close the pages are, on documents the engine opened |
| `fidelity_corpus` | the same, with every document it could not open scored as zero |
| `page_exact_rate` | whether it paginates the way the reference does |

Reporting only the first flatters an engine that declines half the corpus;
reporting only the second hides how good the half it does render is.

## Two traps this harness exists to avoid

**Comparing artefacts of different kinds.** A spreadsheet viewer with no print
pagination draws a scrollable grid at screen scale with row and column headers;
LibreOffice fits the same sheet to paper, scaled and broken across pages. Those
are not the same object and no alignment makes the pixel comparison mean what it
means for a document or a slide. `aggregate.py` marks such an engine/family pair
as not paginated and withholds its page-count figure; the similarity figure it
still produces is a grid-region indication, not a fidelity score.

**Letting the harness's own failures read as the engine's.** A browser-driven
engine fails in ways that have nothing to do with rendering: a dev server that
does not decode `%2C` in a path, a canvas taller than the viewport so the
screenshot silently captures only its top, a renderer spinning in a layout that
no Playwright timeout interrupts, a browser that dies and takes the rest of the
sweep with it. Each of those produced a plausible-looking wrong number here
before it was fixed. `render_wv.mjs` now stages the corpus under sanitised ids,
sizes the viewport to the canvas, enforces a hard per-document deadline that
force-closes the target, and relaunches a dead browser rather than aborting.
Classify a failure as the engine's only when the engine said so.

## Environment

Run `.claude/skills/libreoffice-reference/scripts/check-env.sh` first. Metric-
compatible substitute fonts (Carlito, Caladea, Liberation) must be installed: a
silent font substitution invalidates every number produced after it. Record the
LibreOffice version alongside any result — it is part of the measurement.

Python needs `numpy`, `Pillow` and `PyMuPDF`. Only the first `MAX_PAGES` (5)
pages of each document are compared; page counts are compared in full.

## Checking a reading without the reference's version

`corrections.py` folds in findings that are true of *this* reference binary. Many
of them are not true of another one -- 29 of its 43 changes were version
divergences. The three scripts beside it were written to ask only questions whose
answers are statements about **our own output**, which no reference-version move
can invalidate:

| script | asks |
|---|---|
| `audit.py` | is the page pair even the same page? are the reference's words anywhere in ours? do both sides draw the same number of images and shape the same faces? |
| `claims.py` | a reading that calls a phrase missing names it, in `<em>`. Is it in our text layer -- and if so, is it *visible*? |
| `zorder.py` | is text drawn and then covered by a fill emitted later in the same content stream? |

Two things this pass taught, both worth keeping:

**A hit is not a finding.** Of 25 quotes that `claims.py` reported present-and-visible
in text a reading called defective, **all 25** were false positives: the phrase was
context (*"the rule beneath SIGL / SCHAFFLER"*), not the thing said to be missing. An
absence verb anywhere in a reading does not attach to every quote in it. Nothing from
that class was published.

**Measure visibility in both directions.** `claims.py`'s first cut scored a patch by
ground colour minus its *darkest* pixel, which reports white text on navy as invisible
-- the exact opposite of the truth. It now takes the pixel farthest from the ground in
either direction. Three "invisible" findings dissolved when that was fixed.

`zorder.py` carries a live limitation, stated in its own docstring: it reads text-block
anchors straight out of the content stream and **does not apply the `cm` transforms
above them**, so a block inside a transformed chart resolves to the wrong patch of
paper. That produced one plausible false positive, caught only by cropping the page and
looking at it. Every finding it reports was confirmed against the reference by eye
before being written down, and two candidates were withdrawn.
