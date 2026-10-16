# Words — rebase against LibreOffice 26.2.4.2

Reference binary in this container: **LibreOffice 26.2.4.2 620(Build:2)**. Every stored words
figure in `TODO.batches.md` and `dotnet/probes/words-r*/` was taken against **24.2.7.2**, which
this image does not offer. Nothing measured here is a regression against those figures; they are
figures against a superseded binary.

CLI measured, explicitly, in every sweep:
`/c/sandbox/workdir/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli`
(the coordinator's build of `HEAD`). `SOURCE_DATE_EPOCH=1700000000` and `TZ=UTC` pinned on every
render. No source file was modified this round, so no worktree was taken.

---

## The headline

| | ref 24.2.7.2 | ref 26.2.4.2 |
|---|---:|---:|
| documents matching, ours at r47 | **158** (stored) | **108** (computed) |
| documents matching, ours at HEAD | — (not measurable) | **129** (measured) |
| absolute page error, ours at HEAD | — | **567** |
| exactly-correct page counts, ours at HEAD | — | **137** |
| absolute word error, ours at HEAD | — | **13 944** |

**Words scores 129/200 against the current reference binary.** The stored 158/200 is not a
comparison to it.

The two effects can be separated, because ours is pure C# and does not depend on `soffice` at all,
so a stored ours column stays valid when the reference moves:

| | matching |
|---|---:|
| ours@r47 vs ref@24.2.7.2 — stored, reproduced exactly from `words-r47/baseline.tsv` | 158 |
| ours@r47 vs ref@26.2.4.2 — **computed**: stored ours column, newly measured reference column | 108 |
| ours@HEAD vs ref@26.2.4.2 — **measured** end to end | 129 |

So the binary change alone costs **50 verdicts** with no code change at all, and the work between
r47 and HEAD gives back **21** of them against the new reference. Anyone comparing 129 to 158 as
though it were a regression is reading a binary change as a code change.

Caveat stated plainly: the middle row is *computed*, not measured — it pairs a stored ours column
with a measured reference column. It is sound only because ours does not read `soffice`. The
verdict rule used to compute it is `batch-check.sh`'s, transcribed and then **validated by replaying
1000 stored rows from five previous rounds' TSVs (r38, r42, r45, r46, r47) with zero mismatches**,
so the arithmetic is not the weak link. The 158 in the first row is likewise not taken on trust —
it recomputes from the stored file to the digit.

---

## The prediction, committed before measuring

Written to
`/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/words/PREDICTION.md`
before `ref-baseline.sh` was started, before any probe was authored, and before any stored
per-document figure was opened. Abridged, with outcomes:

| # | prediction | outcome |
|---|---|---|
| P0 | **this round moves zero verdicts** | **held** — no source file was touched |
| P1 | the reference renders all 200, or misses at most 2 | **held** — 200/200, zero `ref-failed` |
| P2a | 30–70 reference page counts moved, centred near 47 | **held** — 47 |
| P2b | the top 3 movers carry **more than half** the total \|Δ\| | **refuted** — top 3 carry 170 of 453, 37.5%; it takes five to pass half |
| P2c | prior reference page counts recoverable for far fewer than 200 | **refuted** — recoverable for all 200, from five independent stored rounds that agree with each other perfectly |
| P3.1 | `w:settings/w:widowControl` still inert — **holds** | **held** |
| P3.2 | the 720 dpi round trip **holds**; risk that a 1/10 pt law is indistinguishable | **split** — see below; the risk was real and the two laws turned out to be separable |
| P3.3 | the table-only-header rule has **changed** (55/45) | **refuted** — it reproduces exactly |
| P4a | `A_320.doc` reference is 118 ±2 | **held** — 118 exactly |
| P4b | `AC-…-10G` moves **down** by more than 5 | **refuted** — it moves **up** 27 |
| P4c | the two revisions of the FAA circular move in the **same direction** | **held** — both up |

Five predictions refuted out of eleven, which is the correct order of magnitude for this project.

I predicted P0 — that this round would move no verdict — and it did not. What I did not predict is
that the *reference* would move fifty of them.

---

## Task 1 — the new reference baseline

`ref-baseline.sh /c/sandbox/workdir/sample-files 'words/batch-0*' … 6`, 200 documents across 21
batches, one soffice profile per worker, per-format identity, absolute outdir.

**The reference renders all 200. `REF-CANNOT-RENDER 0`.** Zero documents at NF-4 report an
unembedded font.

Distribution of reference page counts, 26.2.4.2 (5257 pages total, median 6, min 1, max 727):

| pages | 1 | 2 | 3–5 | 6–10 | 11–25 | 26–50 | 51–100 | 101–200 | 200+ |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| documents | 13 | 24 | 56 | 43 | 32 | 14 | 8 | 5 | 5 |

66 `.doc` (815 pages) and 134 `.docx` (4442 pages).

### The movement table vs 24.2.7.2

The stored reference column is recoverable for **all 200** documents, and — this is the control
that makes the delta mean anything — the five stored rounds that carry it (`words-r38-baseline.tsv`,
`words-r42/`, `words-r45/`, `words-r46/`, `words-r47/baseline.tsv`) agree with one another on
**200 of 200**. The old reference column is reproducible; only the binary under it changed.

> **The prior session's coarse figure is confirmed to the digit: 47 of 200 reference page counts
> moved, 453 total |Δ| pages.**

And the sentence that goes with it needs correcting in two places.

**First, the movement is almost entirely in one direction.** 43 documents up (+417 pages), 4 down
(−36). The reference under 26.2.4.2 paginates *longer*. `A_320.doc` at −32 is the largest of the
four downward movers and is the one the brief flagged.

| \|Δ\| | 1 | 2 | 3–5 | 6–20 | 21+ |
|---|---:|---:|---:|---:|---:|
| documents | 17 | 8 | 6 | 9 | 7 |

Largest movers (`oldref → newref`):

| Δ | old | new | document |
|---:|---:|---:|---|
| +68 | 266 | 334 | `SPA-02_mcar_part-2_and_IS_v2.9.docx` |
| +53 | 154 | 207 | `FAA 2025-26 Holdover Tables.docx` |
| +49 | 141 | 190 | `24-25_FAA_Holdover_Tables.docx` |
| +41 | 312 | 353 | `02_mcar_part-2_and_IS_v2.10.docx` |
| **−32** | 150 | **118** | `A_320.doc` |
| +27 | 697 | 724 | `AC-150-5370-10G-updated-201604.docx` |
| +23 | 85 | 108 | `SPA-06_mcar_part-6_and_IS_v2.9.docx` |

**Second, and more consequentially: page count is the *minor* channel.** The page figure is right
and is not the disturbance.

| reference column | documents moved, of 200 |
|---|---:|
| page count | 47 |
| **extractable word count** | **174** |
| font count | 57 |

Total |Δwords| is **17 549**, again almost all upward — 164 documents up, 10 down — and **48
documents move beyond `batch-check.sh`'s 2%+3 word band**, i.e. far enough to change a verdict on
their own. Holding ours fixed at r47, word drift alone destroys **27** verdicts against page drift
alone's **15** (10 more lose both). A reader who took "47 of 200, 453 pages" as the size of the
disturbance would be understating it by roughly a factor of two, and would be watching the wrong
column.

### What is inflating the extracted word counts — partly identified

Ran over **all 200**, not over the movers only, per the standing control. Counting tokens in the
private-use range U+E000–U+F8FF (the Symbol/Wingdings codepoints a Word list bullet resolves to):

| | documents | total PUA tokens | documents with any |
|---|---:|---:|---:|
| reference word count did **not** move | 26 | **1** | 1 |
| reference word count **did** move | 174 | 4925 | 98 |

That is a clean separation, and it says list bullet labels are extractable text in 26.2.4.2 in a
way they were not in 24.2.7.2. But it is only part of the story and the honest accounting says so:
**PUA tokens are 4926 of the 16 739 total word increase — 29.4%** — and on only 32 of the 174
movers does the PUA count match the delta within 10%. The remaining ~70% is unexplained here.

The confound is stated rather than hidden: documents with bullet lists are also the longer, more
structured documents, so "non-movers have no PUA tokens" is partly a statement about which
documents are simple. This is a **measured association with a plausible mechanism, not an isolated
cause**. Isolating it would need an authored probe varying only the presence of a bulleted list,
which was not run.

---

## Task 2 — the three claims re-checked against the installed binary

### 1. `w:settings/w:widowControl` is inert — **HOLDS**

Instrument: `words-r46/widow-orphan-default.py`, re-run unmodified. Nine variants × five straddle
positions of a four-line paragraph whose lines are one unbreakable 28-character token each; the
`para-off` variant measures the room at the foot of page 1 **in the same run**, so the control is
not a separate assumption. 45 conversions.

**The r46 table reproduces cell for cell on 26.2.4.2.**

| variant | control on at fillers | behaves like |
|---|---|---|
| `no-docDefaults` | — | off |
| `no-pPrDefault` | — | off |
| `empty-pPrDefault` | 14, 16 | **on** |
| `pPrDefault-with-pPr` | 14, 16 | **on** |
| `pPrDefault-widow-off` | — | off |
| `pPrDefault-para-off` | — | off |
| **`settings-on`** (`w:settings/w:widowControl`) | **—** | **off** |
| `para-on` | 14, 16 | on |

`settings-on` is indistinguishable from the control at all five straddle positions. The
document-level flag is inert in 26.2.4.2 exactly as it was in 24.2.7.2, and `WordCompatibility`'s
decision not to read it is still correctly calibrated. The `w:pPrDefault`-presence law r46
established survives the binary change intact as a bonus.

### 2. The drawn font size follows a 720 dpi device round trip — **the number holds, the formula does not transfer to words**

This is the refutation of the round.

Two things have to be separated, because the brief runs them together. The `178 of 178` figure
(actually 194 observations in `sheets-r23/ref-font-sizes.tsv`) is a **sheets** result, measured on
eight flat-ODS probes. It is not a words measurement and had never been checked on the Writer path.

**Re-run of the sheets instrument on 26.2.4.2** — `sheets-r23/mkfods.py` regenerated, all eight
probes re-rendered, every `Tf` operand read back out of the page content streams:

- **194 of 194 emitted sizes are identical to the stored 24.2.7.2 values.** Not close — identical.
- Fitting the stored formula over every resolution from 30 to 4000 dpi on the *new* binary's own
  numbers: **720 dpi reproduces all 194, and no other resolution in that range reproduces more
  than 140.**

So on sheets the claim holds completely and the resolution is uniquely determined, not assumed.

**Then the same law on the Writer path, authored here** (`mkfodt.py`): flat-ODT documents, one
paragraph per size, Liberation Sans. Writer has no print-scale equivalent of a sheet's
`style:scale-to`, so every observation is at scale 1 — which is *not* degenerate, because at scale 1
the round trip is the identity for whole point sizes but not for fractional ones. Five sweeps, 170
observations: 16 whole sizes 6–48 pt; 8.00→10.00 and 11.00→13.00 in 0.05 pt steps; 6→16 pt in
third-point steps; 9.00→9.40 in 0.01 pt steps.

Control first, before believing anything: the 16 whole point sizes come back as the identity,
16 of 16 within 0.02 pt, matching the sheets probe at 100%.

| model | reproduces, of 170 |
|---|---:|
| **A** — the sheets law: snap to 1/100 mm, then 720 dpi round trip | **2** |
| **D** — straight to 720 dpi units, no intermediate snap | 148 |
| **C** — snap to **twips**, then 720 dpi round trip | **170** |

```
C:   tw   = round(pt * 20)              # Writer's own map unit is the twip
     size = round(tw * 720 / (72*20)) * 72 / 720
```

every `round` half-away-from-zero. Sweeping model C over 30–4000 dpi: **720 dpi reproduces all 170
and no other resolution reproduces more than 100.**

**Verdict: the "720 dpi device" half of the claim holds, and holds on both tracks, uniquely fitted
on each. The round-trip *formula* as written is a sheets law and reproduces 2 of 170 on words** —
because Calc's map unit is 1/100 mm and Writer's is the twip, so the intermediate snap is a
different one. This is the project's own dominant pattern once more: the measurement reproduces to
the digit and the sentence generalising it is wrong.

The discriminating evidence is the 0.01 pt sweep, where the bucket boundary sits at x.x3 rather
than at x.x5 — 9.02 pt draws at 9.0 and 9.03 pt draws at 9.1. A 1/10 pt quantisation alone (model
D) cannot produce that; only a twip snap ahead of it can. My prediction flagged the risk that a
1/10 pt law would be indistinguishable from a 720 dpi law. It is not indistinguishable, but only
because a sweep finer than the twip was run — a 0.05 pt sweep alone would have left model D
looking correct on 148 of 170 and the twip snap invisible. **A DOCX cannot settle this at all:
`w:sz` is in half-points, so every OOXML size lands on a whole twip and models C and D agree
everywhere.** The ODT was necessary.

### 3. A header copies forward only when it holds at least one top-level `w:p` — **HOLDS**

Both r43 instruments re-run unmodified on 26.2.4.2.

`header-inherit-content-shape.py`, which holds the section markup fixed and varies only the first
section's header content — page 2 belongs to the second section, which names no header of its own:

| header content | running head on page 2 |
|---|---|
| `text` (the r42 control) | inherited |
| `table` | inherited |
| `nested-table` | inherited |
| `image` | inherited |
| `table-image` | inherited |
| `text-then-table` | inherited |
| **`table-no-trailing-p`** | **none** |
| `table-trailing-p` | inherited |
| `text-then-table-no-trailing-p` | inherited |

`header-inherit-bisect.py` agrees from the other end: `hdr-trailing-paragraph`,
`hdr-leading-paragraph` and `hdr-no-tables` all put the head on page 2; every variant whose header
is tables only puts it on page 1 alone. The census reproduces too — **3 of the 134 DOCX**, the same
three documents, with `.doc` invisible to it as before.

**But the costing has changed, and that is worth flagging.** The defect was "costed at exactly one
verdict" under 24.2.7.2. Under 26.2.4.2 all three affected documents fail on **page count** for
reasons that have nothing to do with the running head:

| document | 24.2.7.2 ours/ref | 26.2.4.2 ours/ref |
|---|---|---|
| `UG.CAO.00133 … Language.docx` | 18/18, verdict `words` | 18/**20**, verdict `pages,words` |
| `UG.CAO.00006 … User Guide.docx` | 30/29 | 29/**35** |
| `docs-quality-MA.IMS.00001 … manual.docx` | 43/44 | 42/**62** |

The rule is unchanged; matching it now buys nothing, because none of the three is one page-count
fix away from a verdict any more. Reproducing the defect faithfully is no longer worth a verdict
either way.

---

## Task 3 — the large page outliers, both sides

Measured, both columns, at 26.2.4.2:

| document | 24.2.7.2 ours/ref | **26.2.4.2 ours/ref** | gap then → now |
|---|---|---|---|
| `A_320.doc` | 141 / 150 | **141 / 118** | −9 → **+23** |
| `AC-150-5370-10G-updated-201604.docx` | 687 / 697 | **662 / 724** | −10 → **−62** |
| `150-5370-10H.docx` | 714 / 721 | **714 / 727** | −7 → **−13** |

The prior session's `A_320.doc` figure of **118** is confirmed exactly. Its gap has changed sign:
we were nine pages short of the old reference and are twenty-three pages long against the new one,
with our own output unchanged at 141. Nothing about our `.doc` reader moved; the reference dropped
32 pages under it.

The two revisions of the FAA circular moved the reference in the **same** direction (+27 and +6),
which is what a global layout change looks like rather than a content trigger. The 10G gap widened
much further than the reference alone accounts for, because **ours also moved**, 687 → 662, i.e.
*away* from the reference. That is a genuine, currently-owned 62-page deficit on one document and
the largest single page error in the corpus after `SPA-02`.

Where the 567 pages of absolute error now sit: the top 8 documents carry **387 of 567, 68%**.

| \|Δ\| | ours/ref now | was | document |
|---:|---|---|---|
| 91 | 243/334 | 267/266 | `SPA-02_mcar_part-2_and_IS_v2.9.docx` |
| 62 | 662/724 | 687/697 | `AC-150-5370-10G-updated-201604.docx` |
| 55 | 298/353 | 315/312 | `02_mcar_part-2_and_IS_v2.10.docx` |
| 53 | 154/207 | 154/154 | `FAA 2025-26 Holdover Tables.docx` |
| 48 | 142/190 | 142/141 | `24-25_FAA_Holdover_Tables.docx` |
| 30 | 78/108 | 85/85 | `SPA-06_mcar_part-6_and_IS_v2.9.docx` |
| 25 | 71/96 | 77/76 | `FRE-03_mcar_part-3_and_IS_v2.9.docx` |
| 23 | 141/118 | 141/150 | `A_320.doc` |

Four of these eight were **exact page matches** under 24.2.7.2 and are tens of pages out now
purely because the reference moved. The right next question for this track is what 26.2.4.2 does
differently on that family of long, table-heavy aviation manuals — not why our page counts
"regressed", because on `FAA 2025-26 Holdover Tables.docx` and `24-25_FAA_Holdover_Tables.docx`
ours did not change at all.

---

## Measured vs inferred, stated explicitly

**Measured** — rendered and read this round:

- the reference column for all 200 words documents at 26.2.4.2 (pages, words, fonts, unembedded);
- ours at HEAD for all 200, and the resulting **129/200**, page error 567, 137 exact page counts,
  word error 13 944;
- all three claim re-checks (45 + 8 + 14 + 170 authored conversions);
- the 194-observation sheets font-size reproduction and the 170-observation Writer refit;
- the PUA token census over all 200 reference PDFs;
- the three outliers, both columns.

**Inferred** — computed from stored numbers, sound but not rendered here:

- **108** matching for ours@r47 against ref@26.2.4.2, and therefore the −50/+21 decomposition. It
  pairs r47's stored ours column with this round's measured reference column. Valid because ours
  does not depend on `soffice`; the verdict rule was validated by replaying 1000 stored rows with
  zero mismatches.
- every "moved by N" against 24.2.7.2. These are stored-figure minus measured-figure. The base
  image offers no 24.2.7.2, so **no delta here is a measured delta.** The five stored rounds
  agreeing on 200/200 is what makes them trustworthy; it is not the same as re-rendering.
- the attribution of the 31 lost verdicts. Only **3** of the 31 have an unchanged ours column, so
  for the other 28 the binary change and the r47→HEAD code change are confounded and this round
  cannot separate them per-document. The aggregate decomposition above is the honest form.

**Not established:**

- the ~70% of the reference word-count inflation that PUA bullet tokens do not account for;
- whether the PUA association is causal — it needs a probe varying only the presence of a bulleted
  list, which was not run;
- the mechanism behind the reference's +417-page drift on long aviation manuals;
- anything about the `.doc` half of any XML census — 66 of the 200 are binary `.doc` and go through
  the WW8 reader, where no zip-level census can look. The 3-of-134 header census is a **ceiling on
  DOCX only**, as its own docstring says.

## Artefacts

Not committed. Under
`/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/words/`:
`PREDICTION.md`, `refbase/ref-baseline.tsv` (200 rows), `gate/gate.tsv` (200 rows, the 129/200),
`movement.tsv` (the 47 movers), `pua.tsv`, `tfsize.py` (a `Tf`-operand reader — there is no
qpdf/mutool/pikepdf in this image), `mkfodt.py` (the Writer font-size probe), `ours-gate.sh`
(the gate against an already-rendered reference, so `ref-baseline.sh`'s 200 conversions are not
paid for twice), and the rendered probe sets `widow/`, `hdr-shape/`, `hdr-bisect/`, `fs/`, `fw/`.
