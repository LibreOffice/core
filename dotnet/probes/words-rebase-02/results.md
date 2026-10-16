# Words — rebase against the corrected reference (26.2.4.2 **with** DejaVu)

Reference: `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`, 200 PDFs, LibreOffice
26.2.4.2 620(Build:2), `fonts-dejavu-core` + `fonts-dejavu-mono` installed.

CLI measured: `dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli` at
commit **`4cbaeb41c3b`**, verified as that tree's build (zero `.cs` under `dotnet/src` or
`dotnet/tools` newer than the binary). `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC` on every
render. No source file was modified.

**Two facts about that commit that turned out to carry the round.** `git log --oneline
83c0acda971..4cbaeb41c3b -- dotnet/src dotnet/tools` returns **nothing**: the code I measured is
byte-identical to the final state of round 47. And the main checkout moved to `eab6499c860`
mid-round, taking the built CLI with it — everything below was banked before that, and nothing
below needs re-rendering.

**Metric caveat, per the coordinator.** Everything here uses the **current** gate metric
(`pdftotext | wc -w` in a 2%+3 band). A corrected word metric is in flight and this scoreboard
should be expected to be superseded. I have not invented a metric of my own. Section 4 is
direct input to that work.

Harness: the predecessor's `ours-gate.sh`, run **unmodified**; its verdict rule re-validated
here rather than inherited.

---

## 1. The headline

> ## Words scores **154 / 200** against the corrected reference.
>
> The right comparator is **159**, not the briefed 158 — 158 is round 47's *baseline* (pre-fix)
> code, and the code I measured is r47's *final* code. Measured with the **same source code at
> both ends**, the move is **159 → 154**.
>
> The predecessor's **129** is a correct measurement of a font-starved machine.

| | matching | abs page error | exact page counts | abs word error |
|---|---:|---:|---:|---:|
| same code × ref@24.2.7.2 (stored, `words-r47/after-unnamed.tsv`) | **159** | — | — | — |
| same code × ref@26.2.4.2 **−DejaVu** (predecessor) | **129** | 567 | 137 | 13 944 |
| same code × ref@26.2.4.2 **+DejaVu** (measured here) | **154** | **117** | **163** | **7 023** |

Failures: 29 `pages`, 9 `words`, 8 `pages,words`, **0 `unembedded`**, 0 render failures either
side. 100 of 134 `.docx` and 54 of 66 `.doc` match.

---

## 2. The prediction, committed before measuring

`prediction.md` in this directory, written before the first render and before any per-document
reference figure was opened.

| # | prediction | outcome |
|---|---|---|
| P-1 | **ours is not font-independent** (`FontconfigPreferences` reads `/etc/fonts`), so stored ours columns are font-dependent, the predecessor's 108 row is an incoherent mixture, and ours must be re-rendered | **held** — and it is the pivot of the round |
| P0 | this round moves zero verdicts | **held** — no source touched |
| P1 | scoreboard **133**, band 120–145 | **refuted** — 154, outside the band, in our favour |
| P2a | 25–60 of our page counts move with the font set | **refuted** — 19 |
| P2b | our movers go **down**, with the reference | **refuted** — 19 of 19 go **up**; the reference goes down on 41 of 42. They converge from opposite sides |
| P2c | our word counts move on fewer than 40 documents | **refuted** — 169 of 200, and for a reason I had not considered |
| P3 | binary alone costs 23–43 verdicts, point estimate 34 | **refuted** — it costs a **net 5** |
| P4a | `A_320.doc` reference 118 ± 4 | **held** — 118, no font effect at all |
| P4b | `AC-…-10G` reference drops below 714 | **held** — 724 → **696** |
| P4c | `150-5370-10H` also drops, by less than 10G | **refuted** — it does not move (727 → 727) |
| P5 | the 720 dpi twip law holds unchanged | **held** — 170/170, drawn sizes *identical* to the font-starved render |
| P6/P7 | census limits; controls | both controls ran and passed |

**Seven of twelve refuted.** I also had to refute one of my own mid-round conclusions
(section 4), which is recorded rather than quietly dropped.

---

## 3. Controls

1. **Known-answer control on the gate.** `ours-gate.sh` run with the corrected reference PDFs
   symlinked in as the *ours* column — reference against reference, same code path, same id
   join: `TOTAL 200  MATCH 200  MISMATCH 0`. This is the control that catches a mis-aligned
   join; it also proves all 200 ids resolve.
2. **Verdict-rule replay, re-run not inherited.** `verdict.py` replayed over **2200 stored rows
   from eleven TSVs across five rounds** (r38, r42, r45, r46, r47); the predecessor replayed
   1000. **Zero mismatches.**
3. **Three known answers reproduced before any new number was read off the harness**: 158
   (stored baseline), 108 (predecessor's computed row), 129 (predecessor's measured row) — all
   to the digit.
4. **Ref cross-check**: figures my gate read out of the PDFs disagree with the banked
   `ref-baseline-all.tsv` on **0 of 200**; same for the font-starved pair.
5. **Shape check**: 154, 117, 163, 46, 14, 76, 5 — none coincides with a corpus constant.
6. Every classifier ran over **all 200**, matching documents included.
7. **The control that decided section 4** — see there; the first instrument I built for it was
   uninformative and I caught it only by asking what each hypothesis actually predicted.

---

## 4. The word column moved because the **measuring tool** changed

This is the round's largest correction and it cuts across all three tracks, so it is stated
before the decomposition that depends on it.

The predecessor reported the reference's word count moving on **174 of 200**, called page count
"the *minor* channel", and attributed 29.4% of the movement to PUA bullet tokens becoming
extractable "in 26.2.4.2 in a way they were not in 24.2.7.2" — a statement about LibreOffice.

Measured over all 200:

- **86 documents where our word count and the reference's moved by the exact same nonzero
  amount** between containers; 84 of them with page counts unchanged on both sides.
  `GLACIERBG.ETT.doc` 409 → 411 on both; `LENTOBUSSIAIKATAULU….doc` 119 → 128 on both;
  `FAA-High-Level-Org-Chart.docx` **693 → 812 on both**.
- Those are **pinned across five stored rounds** at 409/409, 119/119, 693/693.
- Our word counts moved on 169 of 200 and **none moved down**.

### The chain that closes it

| what varies | our word count on the three pinned documents |
|---|---|
| old container, r47-final code | 409 / 119 / 693 |
| this container, same code, **without** DejaVu | **411 / 128 / 812** |
| this container, same code, **with** DejaVu | **411 / 128 / 812** |

Our source code is provably identical (`git log 83c0acda971..4cbaeb41c3b -- dotnet/src
dotnet/tools` is empty). The font set is excluded because both font environments here give the
same numbers. And independently, **our page counts are identical to the stored r47-final column
on 200 of 200**, so the renderer is reproducing across containers exactly. Nothing on our side
changed, and our number moved by precisely the amount the reference's did.

The remaining variable is the instrument: **`pdftotext` 26.01.0** counts differently from
whatever the old container carried. The old poppler version is nowhere recorded — the same
undeclared-input gap as the font set, in a second dimension.

### The mid-round conclusion I had to refute — my own

I first tried to arbitrate with `pdfminer.six` as an independent extractor: it agrees with
poppler 26 on 18 of 18 movers and returns the stored value on 14 of 14 non-movers, which looks
like proof the *content* changed. **It proves nothing.** Both hypotheses predict that reading,
because both extractors read the same *current* PDF; only the old PDF or the old poppler could
discriminate, and neither exists. The same-code/both-font-sets chain above is what actually
discriminates. I record the dead instrument because a low-effort reading of it would have
produced a confident wrong answer, and because it is the same failure the brief warns about —
an instrument whose first big result is not sanity-checked against what it can distinguish.

### What this changes

| claim | as reported | corrected |
|---|---:|---:|
| reference words beyond the 2%+3 band vs 24.2.7.2 | 34 of 200 raw | **1 of 200** once our identically-measured column is differenced out |
| reference **page** counts moved by the version change | 47 of 200, 453 pages | **14 of 200, 76 pages** |

**The "47 of 200, 453 pages" figure in `dotnet/CLAUDE.md` and in `words-rebase-01/results.md`
— described there as the prior session's coarse figure "confirmed to the digit" — is 33
documents and 377 pages of *missing font*.** The version's own effect on reference pagination
is 14 documents and 76 pages, and it is not one-directional: 8 up, 6 down.

Input for the Gate agent, offered without acting on it: the predecessor's PUA census is a good
fit for a *poppler* change rather than a LibreOffice one — 4925 PUA tokens on the movers against
**1** on the non-movers is what "the extractor now surfaces glyphs it used to drop" looks like.
It is not the whole term: `LENTOBUSSIAIKATAULU….doc` carries **zero** PUA tokens and still moved
+9, so at least one further tokenisation change is in there.

---

## 5. The decomposition, and the leg that does not exist

### What exists

| | ref@24.2.7.2 | ref@26.2.4.2 −DejaVu | ref@26.2.4.2 +DejaVu |
|---|---|---|---|
| available | stored figures only | on disk | on disk |
| re-renderable | **no — permanently** | yes | yes |

**The unavailable leg, named: there is no LibreOffice 24.2.7.2 in this image and no route to
one** — the archives offer no earlier build and the download hosts are firewalled. The cell
"24.2.7.2 with a font set I can vary" cannot be filled, **the font effect at the old binary is
unmeasurable, and no full 2×2 version×font decomposition is possible.** Anyone who writes one
has fabricated a cell. A second axis is equally unavailable: the old container's **poppler**,
which section 4 shows is a live variable.

### The legs

| leg | ours | ref | matching | sound? |
|---|---|---|---:|---|
| A′ | r47-final (stored) | 24.2.7.2 (stored) | **159** | yes — one container, one tool set |
| A | r47-baseline (stored) | 24.2.7.2 (stored) | 158 | yes, but it is pre-fix code |
| B | r47-baseline | 26.2.4.2 −DejaVu | 108 | **no** |
| C′ | r47-final | 26.2.4.2 +DejaVu | 127 | **no** |
| D | measured code −DejaVu | 26.2.4.2 −DejaVu | **129** | yes — one container |
| E | measured code +DejaVu | 26.2.4.2 +DejaVu | **154** | yes — one container |

B and C′ pair a *stored* ours column against a *newly measured* reference column. The
predecessor justified that with "ours does not read `soffice`" — true and insufficient: ours
reads the **font set**, and both columns are read out by a **`pdftotext` whose version also
changed**. B and C′ are cross-container mixtures on two axes and are reported only to be
labelled unsound.

### Result: the binary costs a net **5**, the font **25**

Because the code is identical at both ends, A′ and E are a single-variable comparison. Document
by document: **6 lost, 1 gained, net −5. All seven are `pages` verdicts; our page count is
identical on all 200; so every one of them is the reference's page count moving.** The words
channel contributes **zero** verdict changes between them — which is exactly what section 4
predicts, since the tool shifted both columns together.

| | ours/ref then → now | document |
|---|---|---|
| lost | 3/3 → 3/4 | `1447.doc` |
| lost | 4/4 → 4/5 | `003.doc` |
| lost | 8/8 → 8/7 | `template---tpr-technical-progress-report-with-guidance.docx` |
| lost | 63/63 → 63/64 | `150_5335_5a.doc` |
| lost | 59/59 → 59/58 | `ESPN-R - MCF - RA - Ed1.docx` |
| lost | 154/154 → 154/167 | `FAA 2025-26 Holdover Tables.docx` |
| **gained** | 15/16 → 15/15 | `f445896eb008d14c1746fc37d412dc22.docx` |

The **font**, with code and binary held constant and both columns re-rendered here (D vs E):
**installing DejaVu is worth 25 verdicts**, 129 → 154. Traversing the same corner in the two
orders gives +16 then +9, or +4 then +21 — non-additive, so the two sides interact, but both
routes total 25.

**So of the predecessor's "the binary change alone costs 50 verdicts": the binary is 5, the
missing font is 25, and the remainder is the artefact of pairing columns across containers.**
Its companion sentence — "the work between r47 and HEAD gives back 21" — describes work that
does not exist: there is no source change between r47's final commit and the commit measured.

### The old container's fonts, measured rather than inferred

The brief's argument that DejaVu was present rests on `SheetColumnDigitsTests`. Direct
measurement:

| our page counts vs the stored r47-final column | agreement |
|---|---:|
| ours re-rendered **with** DejaVu | **200 / 200** |
| ours re-rendered **without** DejaVu | 180 / 200 |
| restricted to the 19 documents the font set moves | **19 / 19 with, 0 / 19 without** |

200 of 200, and 19–0 on the discriminating subset. The old container's effective font set equals
this one's for every face our layout engine touches. That is now measured.

---

## 6. The three large outliers, re-taken

| document | ours/ref @24.2.7.2 | ours/ref −DejaVu | **ours/ref +DejaVu (now)** | verdict |
|---|---|---|---|---|
| `A_320.doc` | 141 / 150 | 141 / 118 | **141 / 118** | `pages`, gap +23 |
| `AC-150-5370-10G-updated-201604.docx` | 687 / 697 | 662 / 724 | **687 / 696** | `pages`, gap **−9** |
| `150-5370-10H.docx` | 714 / 721 | 714 / 727 | **714 / 727** | `pages`, gap −13 |

- **`A_320.doc` has no font effect at all** — 118 with and without DejaVu. Its −32 pages is
  entirely the version change and is the largest single version effect in the corpus. Our 141
  has not moved.
- **`AC-…-10G` is the confound probe and it resolves cleanly.** It moves on both axes: the
  reference goes 697 → 724 on the version and 724 → **696** on the font, while *ours* goes
  687 → 662 on the font and back to 687 with DejaVu. The predecessor's "a genuine,
  currently-owned 62-page deficit… the largest page error in the corpus after SPA-02" is
  **entirely an artefact of the missing font**. The real gap is 9 pages, against 10 at
  24.2.7.2. Nothing regressed.
- **`150-5370-10H` does not move on the font axis at all.** The two revisions of the circular
  move together on the *version* axis (+6 and +27) and **not** on the font axis (0 and −28).
  "The font effect is all one direction" is true of its sign and false of its reach.

Where the 117 pages of absolute page error now sit:

| \|Δ\| | ours/ref now | was (−DejaVu) | document |
|---:|---|---|---|
| 23 | 141/118 | 141/118 | `A_320.doc` |
| 13 | 154/167 | 154/207 | `FAA 2025-26 Holdover Tables.docx` |
| 13 | 714/727 | 714/727 | `150-5370-10H.docx` |
| 13 | 142/155 | 142/190 | `24-25_FAA_Holdover_Tables.docx` |
| 9 | 687/696 | 662/724 | `AC-150-5370-10G-updated-201604.docx` |

`SPA-02_mcar_part-2_and_IS_v2.9.docx`, the predecessor's 91-page top error, **matches exactly
now** (267/267), as do `02_mcar_part-2_and_IS_v2.10.docx` and `SPA-06`.

---

## 7. The carried-forward claims

### The 720 dpi twip law — holds, and is untouched by the font set

The five authored flat-ODT sweeps re-rendered under the corrected font set (170 observations:
16 whole sizes 6–48 pt; 8.00→10.00 and 11.00→13.00 in 0.05 pt steps; 6→16 pt in thirds;
9.00→9.40 in 0.01 pt steps).

- **All 170 emitted `Tf` operands are identical to the font-starved render.** Zero differ. This
  is also a control on my own P-1 reasoning: installing a font moves face selection and not the
  export pipeline, so the round's other figures are not an environmental artefact.
- Refit: **model C (snap to twips, then the 720 dpi round trip) reproduces 170/170**; swept
  30–4000 dpi, **720 dpi reproduces all 170 and no other resolution reproduces more than 100**.
- One arithmetic correction: transcribed verbatim from `sheets-r23/README.md`, the sheets law
  reproduces **33 of 170** at a ±0.005 pt tolerance, not the reported 2 — the 2 looks like exact
  float equality. The ranking, the discriminating 0.01 pt sweep and the conclusion are
  unaffected; the sheets law cannot return a whole point size at all (it draws 6 pt as 6.0094).

### The table-only-header rule — holds, and **its costing is alive again**

Not re-run; its *costing* was re-taken, because "the costing is dead" was measured against the
font-starved reference:

| document | @24.2.7.2 | −DejaVu | **+DejaVu (now)** |
|---|---|---|---|
| `UG.CAO.00133 … Language.docx` | 18/18, `words` | 18/20, `pages,words` | **18/18, `words`** |
| `UG.CAO.00006 … User Guide.docx` | 30/29, `pages,words` | 29/35 | **30/29, `pages,words`** |
| `docs-quality-MA.IMS.00001 … manual.docx` | 43/44, `pages` | 42/62 | **43/44, `pages`** |

All three return exactly to their 24.2.7.2 figures. **"Under 26.2.4.2 all three fail on page
count for reasons that have nothing to do with the running head" was an artefact of the missing
font**, and the original one-verdict costing on `UG.CAO.00133` — page-exact, failing only on
words — is restored. Consistent with, not proof of: we emit 4007 words against the reference's
3762, ~14 per page over 18 pages, the right order for a running head the reference drops. That
verdict is also the kind the corrected word metric may move, so it should be re-costed after the
Gate agent lands.

### `w:widowControl` inert — carried forward unchanged, not re-run.

---

## 8. Measured vs inferred

**Measured here**: ours for all 200 with the corrected font set and the **154/200**, page error
117, 163 exact page counts, word error 7023; both controls (200/200 ref-vs-ref; 2200-row
replay); the font effect on our own renderer by re-rendering; the 170-observation flat-ODT sweep
re-rendered and refitted; the three-way same-code word-count chain in section 4.

**Computed** (stored column × measured column): legs A′ = 159 and A = 158 recomputed from the
stored TSVs; B = 108 and C′ = 127, reported only to be labelled unsound; every "moved by N"
against 24.2.7.2, including the corrected 14-of-200 / 76-page figure. The image has no
24.2.7.2, so **no delta against it is a measured delta.**

**Not established**: the old container's poppler version (unrecorded — the tool change is
identified by its signature, not a version diff); the residual tokenisation term beyond bullet
surfacing; the mechanism of the reference's 14-document page movement (`A_320.doc`'s −32 is
unexplained and is 23 of the current 117 pages of error); whether the predecessor's PUA
association survives — it was fitted against a delta now known to be instrument, so it should be
treated as **withdrawn** rather than standing; anything about the `.doc` half of any XML census
(66 of 200 go through the WW8 reader).

---

## Artefacts

In this directory, uncommitted: `prediction.md`, `gate.tsv` (200 rows, the 154/200),
`verdict.py` (the verdict rule plus the replay control), `decompose.py` (the leg table),
`detail*.py` (the movement and outlier tables), `final.py` / `final2.py` (the same-code
comparison), `bullets.py`, `arbitrate.py` (**the dead instrument — kept deliberately**), and
`fitfw.py` (the font-size refit). Under
`/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/words2/`:
the rendered `gate/ours/` PDFs, the `control/` known-answer run, and the re-rendered `fw/`
font-size probes.
