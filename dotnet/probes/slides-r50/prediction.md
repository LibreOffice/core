# slides-r50 — prediction, committed before any post-change rendering

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
corpus `/c/sandbox/workdir/sample-files`, base commit `ac147b7e5bb`, `SOURCE_DATE_EPOCH=1700000000`,
`TZ=UTC`.

## Baseline reproduced before anything was changed

Whole-track sweep of all 35 slides batches (`track-ink-sweep.sh`, 6 workers, reference PDFs reused
from `refpdfs-26.2.4.2-fonts/slides` where banked): **TOTAL 305 MATCH 200**.

Reconciled document by document against `MANIFEST.tsv`: **302 of 302 agree, 0 disagreements**,
i.e. **198 of 302 passing**, exactly the briefed baseline. The sweep's 305/200 is 305 *files* for
302 *documents* — `ceiling-001/…Sylva…`, `extra-001/…FAAAI…` and `metrics-001/Ramp Up Campaign…`
each exist twice on disk, once `.pptx` and once `.PPTX`, byte-identical. The manifest lists each
once, under the lower-case path, so a case-sensitive join on the `path` column silently drops
those three rows.

Ink over the same sweep: **INK 1045.96, 507 major pages over 305 documents** — the whole-track ink
sweep round 41 never ran.

## What the change is

`a:bodyPr/@wrap="none"` suppresses wrapping **unless the body states `a:spAutoFit` or
`a:normAutofit`**, in which case LibreOffice wraps at the shape width. We currently treat
`wrap="none"` as unconditionally unbounded (`PptxTextBody.cs:172`, `Wraps = Stated(…,"wrap") != "none"`).

Measured on 9 authored one-shape variants against 26.2.4.2, varying two axes independently:

| `wrap` | autofit stated | reference wraps? |
|---|---|---|
| `none` | (absent) | no |
| `none` | `noAutofit` | no |
| `none` | `spAutoFit` | **yes** |
| `none` | `normAutofit` | **yes** |
| `square` | absent / `noAutofit` / `spAutoFit` / `normAutofit` | yes (all four) |

## Documents expected to change

**Renderings expected to move: 30 near-certain, up to ~60.**

The near-certain 30 are the documents where we currently draw text **outside the media box** — a
drawn measurement, not a declaration census (`tools/offpage.py` over the baseline renderings):
ours 30 documents, the reference 9. 17 are `text`, 5 `ceiling`, **8 currently pass**. The
signature repeats across one template family: the last slide's `wrap="none"` + `spAutoFit` line
`Free Templates & Infographics for PowerPoint and Google Slides` overhangs the 720 pt page by
**8.7 pt**, so `Slides` extracts as `Slid` and two characters are lost.

The eight currently-passing ones are the regression surface and are named here in advance:
`171128IPAP__pptx` (3.7 pt), `2015-Civil-Rights-Website-training__ppt` (0.7),
`3492__pptx` (1.5), `Copy of Full deck with references…__pptx` (1.4),
`NAS-Infrastructure-Roadmaps-v16.0__pptx` (5.0), `PRM_training__pptx` (6.2),
`Stakeholders-v08052017 - v5__pptx` (1.6), `Statement of Work presentation__pptx` (3.9).

The declaration census (`tools/wrap-census.py`) finds a `wrap="none"` body that also states
`spAutoFit`/`normAutofit` in **154 of 253** zip-readable slides documents — 28 of 28 `text`,
68 of 69 `ceiling`, **58 of 155 passing**. That is the *ceiling* on reach, not the estimate: most
such bodies hold text that fits, and a body whose text fits wraps identically either way.

## Verdict movement expected: **0**

Stated as a number, and zero is the honest answer. The word-gate failures on this track are
dominated by the reference emitting *more* tokens than we do for the same characters — the
token-splitting ceiling — and recovering one or two truncated words does not close a gap of
20–70 words. `049_Five-Block_Hub_Spoke` is 264 words to the reference's 286; gaining `Slides`
makes it 265. No document is near the max(2%, 3) band from below.

I therefore expect **198 → 198**, and I am equally prepared for 198 → 197 or lower if wrapping
newly introduced on a passing document costs it a page or a word.

## What this census cannot see

- **The 49 `.ppt` binaries.** `wrap-census.py` reads zips; the binary format carries the same
  concept in a different structure and is not scanned at all. `2015-Civil-Rights-Website-training__ppt`
  is in the off-page list, so the class does reach `.ppt` and the census understates it there by
  an unknown amount.
- **Inheritance.** The census tests one `a:bodyPr` element for both `wrap` and the autofit. A body
  inheriting `wrap="none"` from a layout or master while stating its own `spAutoFit` — or the
  reverse — is invisible, **in both directions**, so this is neither a floor nor a ceiling. The
  runtime chain (`PptxTextBody` walks `bodyChain`) does resolve inheritance, so the implemented
  behaviour will reach documents the census does not list.
- **Declared is not drawn.** A `wrap="none"` body whose text fits its shape produces no change at
  all. This is why the 154 is quoted as a ceiling and the 30 as the estimate.
- **Overflow that stays on the page.** `offpage.py` only sees text crossing the *media box*. Text
  that overflows its *shape* but stays on the page will change when this lands and is counted in
  neither number. This is the main reason the estimate has a range rather than a point.
- **The other formats.** ODP's `fo:wrap-option` is a different attribute on a different reader and
  is untouched and unmeasured here.

## Refutations this round already carries, before the change

Recorded here because they are results in their own right and were established by measurement:

1. **The brief's stated mechanism for the 75 `ceiling` documents — `spc="150"` letter spacing — is
   wrong.** On `049_Five-Block_Hub_Spoke` the `spc="150"` in the master is **not inside
   `p:titleStyle`** and does not apply to the split text. Re-zipping the deck with one attribute
   changed at a time isolates the cause to **`pitchFamily`'s family nibble** on `<a:latin>`:
   with it the reference draws the title 622.40 pt wide in **11** `pdftotext` tokens, without it
   541.55 pt in **6**. `panose`, `charset` and `kern` are each inert (6 variants). Confirmed in
   isolation on authored decks for `Helvetica`, `Times`, `Albany` and `Thorndale`, and
   **inert on all five installed families tested** (`Arial`, `Calibri`, `Liberation Sans`,
   `Carlito`, `DejaVu Sans`), so it fires only where LibreOffice must substitute.
2. **Font substitution is not the cause of the wider title.** Both sides embed the same subset,
   `DAAAAA+LiberationSans-Bold`, at the same `36 Tf`. We emit one `Tj`; the reference emits a `TJ`
   array with a negative adjustment after every glyph (+2.84 pt/gap on line 1, +3.32 pt/gap on
   line 2 — not uniform, so not letter spacing either).
3. **The 28 `text` documents are not 28 distinct defects.** Charstream over all 302: 9 of the 28
   have an *identical* character multiset (jaccard 1.0000), and 15 more differ from the reference
   by **exactly two characters**, always the same two — the `es` of `Slides`. Two documents
   (`038_Competitive_Advantage_Card`, `035_Chemistry_Column_PowerPoint_Chart`) differ by 48 and 20
   characters in chart labels and are the only genuinely separate members.

## Prediction of the ceiling-sample verification

The brief asks for ~10 of the 75 `ceiling` documents to be checked. The charstream test is cheap
enough to run on all 302 from the sweep's own PDFs, so all 75 were checked rather than a sample.
Predicted before the numbers were read out by kind: most correctly filed, with a handful of
known-open documents (`Demick_JetBlue`, `OnTrac…`, `8_P-Pavese_AIRBUS…`, `16 - UTM - (NASA)`)
showing genuinely different characters because they carry separately-recorded real defects.
