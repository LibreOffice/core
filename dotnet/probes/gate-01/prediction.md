# Gate 01 — prediction, committed before any measurement

Written before running a single census, sweep or probe of this round. The only inputs are
`eab6499c860`, `dotnet/probes/slides-rebase-01/results.md`, `dotnet/probes/words-rebase-01/results.md`,
the current `batch-check.sh`, and a listing of the reference/`ours` PDF directories. No PDF has
been read.

---

## R — the refutation check (does *our* output emit the same glyphs?)

This is the measurement that could kill the whole task, so it is predicted first and separately.

- **R1. We do emit them.** Slides §3's table already shows an `ours bullets` column that is
  non-zero on all eight rows, so the term certainly does not vanish on our side.
  Predicted: **> 90% of the 163 slide decks** and **> 60% of the 200 words documents** carry at
  least one standalone non-alphanumeric token in *our* PDF.
- **R2. It does not cancel.** Predicted: per-document the two sides disagree badly — on slides,
  **more than half** of the decks differ by more than 20% in their bullet-token count, and the
  sign is not consistent (slides §3 shows both directions: 23 vs 56 one way, 256 vs 76 the
  other). Aggregate totals may be within a factor of two while per-document differences are
  large; the aggregate is the misleading number here.
- **R3. Therefore the finding survives**, but is **smaller than the raw
  "15873 non-alphanumeric tokens" figure suggests**, because that figure is one-sided. The
  quantity that matters is Δ(bullets) = |ours_bullets − ref_bullets|, not ref_bullets.
  Predicted: total Δ(bullets) over slides is **between 25% and 70%** of the 15873.
- **R4. If R1–R3 are wrong** — i.e. we emit them at the same per-document rate — the corrected
  metric moves close to zero verdicts and this round's honest result is "the term cancels; the
  gate was not measuring bullets after all". I record now that I would publish that.

## C — what the reference emits, and where

- **C1. Code points.** The set is dominated by `U+2022 BULLET`, the Symbol/Wingdings
  private-use block `U+F000–U+F0FF` (a subset of `U+E000–U+F8FF`), `U+2013 EN DASH`,
  `U+25CF BLACK CIRCLE`, `U+25AA/U+25A0` squares and `U+00B7 MIDDLE DOT`. Predicted:
  **the top 10 code points account for > 85%** of all standalone non-alphanumeric tokens.
- **C2. Not confined to list bullets.** Predicted: the same code points appear in
  autoshape/placeholder text and in table cells, not only in paragraphs carrying a list
  numbering. I cannot prove provenance from the PDF text layer at all (see "what the census
  cannot see"), so this will be argued from position/context, not asserted.
- **C3.** Numbered-list labels (`1.`, `a)`, `iv.`) are **not** part of this term because they
  contain a letter or a digit, so any letter-or-digit rule leaves them counted on both sides.

## M — the metric I expect to choose

- **M1.** I expect to choose **"a token counts as a word iff it contains at least one Unicode
  letter or digit"**, over (a) stripping an enumerated code-point set, (b) comparing normalised
  full text, (c) counting characters, (d) widening the band. Reason predicted in advance: (a) is
  a list fitted to the data I am trying to measure, which is the failure mode the brief names;
  (b) answers a different question than check 2 asks; (c) is *more* bullet-sensitive, not less.
- **M2. The locale trap.** Predicted: a naive `grep -c '[[:alnum:]]'` under `LC_ALL=C` drops
  every wholly-Cyrillic/Greek/CJK token and would under-count those documents catastrophically,
  while looking perfectly correct on the English majority of the corpus. I predict the corpus
  contains at least one document where this changes the count by more than 2%. The
  implementation must be Unicode-aware by construction, not by environment.

## V — instrument validation

- **V1. Control over already-matching documents.** Benchmark to beat or explain: slides
  **131 of 132**. Predicted: slides **≥ 130 of 132** stay matching; words **≥ 126 of 129**;
  sheets **≥ 132 of 135**. Predicted total flips-to-failing across all 534: **≤ 6**.
- **V2. Known answer.** An authored bulleted probe (a document with an exactly known number of
  alphanumeric words and an exactly known number of bullets) rendered by the reference:
  predicted the corrected count is **exact**, |Δ| ≤ 1, while the raw `wc -w` count exceeds it by
  exactly the bullet count.
- **V3. Replay.** `words2/verdict.py` already exists and replayed 1000 stored rows with zero
  mismatches. Predicted: fed the **raw** counts, my re-implementation reproduces every stored
  verdict in those same TSVs — i.e. the change is confined to the words term and does not touch
  pages or unembedded. Predicted mismatches: **0**.

## S — the three scoreboards, old vs new

Baselines as recorded: slides **132/163**, words **129/200**, sheets **135/171** — total
**396/534**. (I have looked at the stored per-track TSVs only far enough to count verdict
strings; one of them disagrees with its own `results.md`, so I predict I will have to re-render
`ours` myself rather than reuse them.)

- **S0.** Predicted: my own re-render of `ours` at `HEAD` reproduces slides **132 ± 1**,
  words **129 ± 2**, sheets **135 ± 2** under the *old* metric. If it does not, everything below
  is void and the round becomes an investigation of why.
- **S1. Slides: 132 → 144.** Slides §3 computed this exact figure with the same split, so this
  is a reproduction, not a prediction; I predict it reproduces to the document (13 in, 1 out).
- **S2. Words: 129 → 131**, range **129–134**. The ceiling is hard and worth stating: of the 71
  words failures, only **8** are `words`-only. The other 19 word-failures also fail `pages`, and
  a corrected word metric cannot flip those. So words gains **at most 8** no matter how large
  the bullet term is, and the words track's "4925 PUA tokens" headline is therefore worth **≤ 8
  verdicts**. I predict most of the 8 are *not* bullet cases.
- **S3. Sheets: 135 → 139**, range **135–145**, ceiling 16 (`words`-only failures). I predict
  sheets moves **least per failure available**, because a spreadsheet's text layer is cell
  values and has far fewer list bullets than a deck or a manual.
- **S4. Total: 396 → 410**, range **400–420**.
- **S5.** Predicted: the corrected metric explains **more** of the words track's word-count
  inflation than the PUA-only census did — **40–70%** against the recorded **29.4%** — because
  it also catches `U+2022` and the dashes, which the `U+E000–U+F8FF` filter missed.

## What this census cannot see, and I say so before it can embarrass me

1. **Provenance.** A PDF text layer has no list structure. I can see that a token is `•`; I
   cannot see whether LibreOffice drew it as a numbering label, as a literal character the
   author typed, or as a dingbat that *is* the content. Any claim of the form "these are list
   bullets" is an inference from the code point and its position, and I will label it as one.
2. **Glyphs with no `ToUnicode` mapping** are invisible to `pdftotext` on either side — they
   contribute zero tokens and cannot be counted. If the reference's bullet were unmapped the
   whole effect would hide, so the effect's *visibility* is itself conditional.
3. **Rasterised text.** `TODO.raster-ceiling.md`'s 37 pages contribute zero reference tokens
   where ours contributes real ones. The corrected metric does nothing for those and I predict
   it moves **none** of them; if a raster-ceiling document flips, I have a bug.
4. **Whether 26.2 newly *emits* bullets or poppler 26.01 newly *extracts* them.** Unanswerable
   in this container — there is no earlier LibreOffice and no earlier poppler. It also does not
   matter for the repair: either way the two sides must be compared on the same footing.
5. **Anything about the ~70% of the words inflation that is not bullets.** If S5 lands low, the
   residue is still unexplained and this round does not explain it.
6. **Ordering and duplication.** A count is a count: a document that renders the right words in
   the wrong order, or twice, passes check 2 under both the old metric and the new one. The
   corrected metric is not a stronger test, only an unbiased one.
