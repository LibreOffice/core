# Sheets rebase-02 — predictions committed BEFORE measuring

Written 2026-08-13, after reading `sheets-rebase-01/{prediction,results}.md` and before
running anything. Worktree `/c/sandbox/workdir/wt-sheets-a` at `a5d453fae3f` (source-identical
to `4cbaeb41c3b`; the newer commit is documentation only), built clean, 0 warnings.

Inputs I am allowed to reason from and have *not* re-derived: the brief's statement that with
LibreOffice held at 26.2.4.2 and only the font set varying, the font alone moves **11 of 171**
sheets reference page counts totalling **43 pages**, all in the same direction (fewer pages with
DejaVu), and **36** word counts.

## Task 1 — the corrected sheets scoreboard

- **P1.1** The corrected score lands in **140–155 of 171**, point estimate **147**. Reasoning:
  135/171 was measured with our (DejaVu) renders against a non-DejaVu reference — a mismatched
  pair. Correcting the reference half can only recover documents the mismatch broke: at most the
  11 page movers plus whichever of the 36 word movers crossed the 2%+3 band. 135 + 11 = 146 is
  the page-only ceiling; a couple of word recoveries puts it near 147. It will **not** reach the
  stored 155, because the version change is still uncorrected and irreducible.
- **P1.2** At least **one** document that matched against the *pre*-font reference will
  **mismatch** against the corrected one (a coincidental agreement with the WenQuanYi-metric
  reference). I put this at ~55%; if the flip count is 0 the two reference sets differ from our
  column strictly in our favour, which is the tidier and slightly less likely world.
- **P1.3** Of the residual mismatches, **`pages` will dominate `words`** as the leading failure
  token, and the count of `ours-failed` will be **0** (the predecessor rendered 171/171 of ours).
- **P1.4** The harness re-validation on `sheets/batch-001` reproduces the predecessor's **10/10**
  claim exactly. If it does not, the predecessor's `ours/` bank and mine disagree and the whole
  135 figure was measured on stale renders.

## Task 2 — `sectors-defense-and-aerospace.xlsx`, 227 → 449

- **P2.1** **It is a version effect, not a font effect.** Prediction: the corrected reference
  page count for this document is **449, unchanged** from the pre-font reference. Confidence
  ~85%. Reasoning, stated so it can be scored: the brief says the font alone moves 43 pages in
  **total** across the whole sheets track. A document whose font-attributable movement were even
  a large minority of its own 222-page swing would blow that budget on its own. Therefore this
  document is almost certainly **not** among the 11 font movers, and its swing is left entirely
  to the 24.2 → 26.2 bump.
- **P2.2** If P2.1 is wrong and the count did move with the font, I predict it moved **down**
  (DejaVu is narrower; the pagination is width-driven) and by **less than 43** pages, i.e. still
  nowhere near 227, so the conclusion "predominantly a version effect" survives either way.
- **P2.3** The document does not resolve DejaVu at all — I predict its reference PDF's font list
  is **identical** between the pre-font and corrected reference sets. That is the direct test and
  it is independent of the page count.

## Task 3 — the `7-memento` inferred step

- **P3.1** The inferred step **confirms**: clamping the ±1 neighbour lookups in `Edges.Build` to
  the placed band moves page 2's `#003366` pixel count from 847 sharply toward the reference's 1.
  Confidence ~75%. I expect it to reach **0 or 1**, not an intermediate value, because the rule
  removes the *only* source of that colour on the page.
- **P3.2** The `#0066CC` count does **not** recover to 63,765. The predecessor measured 18,061
  against 63,765 — a 3.5× shortfall the colour rule cannot explain, since taking column 1's own
  right style adds at most the segments column 2 was displacing. I predict ours lands **below
  25,000**, leaving a second, unexplained leading-edge effect open. Confidence ~70%.
- **P3.3** **The fix moves ZERO verdicts on the sheets track.** Near-certain (~95%). The gate's
  three predicates are page count, extractable words and unembedded fonts. A border's colour and
  presence are invisible to all three. This is the "most real fixes move zero verdicts" case and
  I am predicting it in advance rather than discovering it. The corpus reach must therefore be
  measured as **byte/ink divergence**, not as scoreboard movement, and a scoreboard that is
  unchanged after the fix is **confirmation the fix is safe**, not evidence it did nothing.
- **P3.4** Corpus reach measured as "renders whose bytes change": I predict **more than 20 and
  fewer than 90** of 171. Every sheet whose printed band ends inside a bordered region is in the
  class, which is common, but most workbooks either have no borders at the band edge or state the
  same style on both sides, where `Resolve` returns the same answer either way.
- **P3.5** Some of the byte-changed documents currently **match** the gate. I predict at least
  **5** matching documents change bytes, and that **none** of them leaves `match`.

## Task 4 — what I expect to refute

The brief says to expect to refute something, possibly in the brief. My candidate, flagged in
advance: the brief describes the `7-memento` defect as "an off-page column paints a border on our
page", which frames it as *extra* ink. The measured numbers say we paint **3.5× too little**
`#0066CC` and only 847 px of wrong-colour `#003366` — so the dominant defect on that page is
**missing** ink, not extra ink, and P3.2 says the fix will not close it. If that holds, the
brief's framing of the defect is right about the mechanism and wrong about its size.

## What this round's census CANNOT see

- **The gate is blind to everything the `7-memento` fix touches.** Colour, stroke position,
  ink coverage and border presence move no predicate. A "zero verdicts moved" result is
  therefore not evidence about the fix's correctness in either direction, and I must not report
  it as such.
- **I cannot render at 24.2.7.2.** Every "version effect" statement in this round compares a
  number I measured at 26.2.4.2 against a number *someone else stored* at 24.2.7.2 in a different
  container. It is not a controlled A/B; the font set of that original container is inferred from
  `SheetColumnDigitsTests`, not observed.
- **I do not re-render the reference bank.** Any nondeterminism in `soffice` output is invisible
  to me, and every reference figure inherits whatever load the bank was rendered under. I can
  only re-run individual documents solo, as the predecessor did for the 449.
- **`pdftotext | wc -w` is not the text.** Two renders can agree on word count and disagree on
  every glyph position, and a render that rasterises text scores 0 words without being wrong.
- **A byte diff over-counts reach.** A single changed stroke and a whole re-paginated document
  both register as "bytes differ". Byte reach is an upper bound on meaningful reach.
- **Font resolution inside our engine is not the fontconfig chain.** When ours and the reference
  disagree I cannot tell from these instruments whether we picked a different face or laid out
  the same face differently.
- **One file's border census says nothing about the corpus.** The `7-memento` measurements are
  page 2 of one workbook; the reach census is a separate instrument with its own error modes.
