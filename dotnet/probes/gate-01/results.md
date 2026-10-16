# Gate 01 — the measuring instrument, corrected

Subject: `batch-check.sh`'s check 2, the extractable-word check, across all three tracks.
Reference: `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/` (534 PDFs, LibreOffice 26.2.4.2
620(Build:2), `fonts-dejavu-core` present). Ours rendered here from
`dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli` at `c13049cc002`, with
`XRAY_OFFICE_CLI` set explicitly, `SOURCE_DATE_EPOCH=1700000000` and `TZ=UTC` on every render.
Branch `wt-gate`, worktree `/c/sandbox/workdir/wt-gate`.

---

## 0. Read this before quoting any number below

> ### These scoreboards are not comparable to any figure recorded before 2026-08-13.
>
> Check 2 counted `pdftotext … | wc -w`. It now counts **tokens carrying at least one Unicode
> letter or digit**. Every `match` count, every `words` verdict and every "absolute word error"
> in `TODO.batches.md`, in `dotnet/probes/*/results.md` and in every stored `*.tsv` in this tree
> was produced by the old count. **A figure quoted without saying which metric produced it is
> now ambiguous**, in the same way that a figure quoted without naming the LibreOffice version
> or the font set already was. This is the project's third undeclared gate input in two weeks;
> `MISSING_PACKAGES.md` and `dotnet/CLAUDE.md`'s container section are the other two.
>
> **The conversion exists and is exact.** `batch-check.sh` and `ref-baseline.sh` now emit the
> old `wc -w` figure as a final `rawwords` / `refrawwords` column, after the verdict, so `$7`
> is still the verdict for every existing reader. Feed `rawwords` to the same unchanged verdict
> block and you get the old verdict back, document for document — verified end to end on
> `slides/batch-002` (10 of 10) and offline on all 534.
>
> The three conversions for this commit, ours at `c13049cc002` against the canonical reference:
>
> | track | old metric | new metric |
> |---|---:|---:|
> | words | 154 / 200 | **154 / 200** |
> | slides | 132 / 163 | **144 / 163** |
> | sheets | 142 / 171 | **144 / 171** |
> | **total** | **428 / 534** | **442 / 534** |
>
> `ref-baseline-all.tsv` beside the canonical reference PDFs **still holds the old metric** in
> its `refwords` column. It was not rewritten — five agents are reading it. The corrected
> reference column for all 534 is in `census.tsv` (see §9).

---

## 1. The prediction

`prediction.md` in this directory, committed as `1bf901bf623` before the first PDF was read
(sha256 `f73387304efd48c035baccc769228eb0d46d6b542e41cb3fa5e1e4acfefefbef`). Scored in §8. Nine
of twenty-one items refuted, and the two most instructive refutations are both mine: the size of
the term, and what it is made of.

---

## 2. Task 1a — the refutation check: **we emit them too, and corpus-wide the term very nearly
cancels**

This is the measurement that could have killed the round, so it was run before anything else.
Both columns are read by the **same** `pdftotext` 26.01.0 in the same pass, so nothing below can
be an artefact of the extractor.

| track | docs | ref non-alphanumeric tokens | ours | docs carrying ≥1 (ref / ours) | Σ per-document \|Δ\| |
|---|---:|---:|---:|---:|---:|
| words | 200 | 33 483 | 33 502 | 196 / 195 | **561** |
| slides | 163 | 15 873 | 16 257 | 162 / 162 | **1 658** |
| sheets | 171 | 240 452 | 242 071 | 162 / 162 | **5 835** |
| **total** | **534** | **289 808** | **291 830** | **520 / 519** | **8 054** |

**We emit them at very nearly the same rate.** 291 830 against 289 808 is 0.7% apart, and the
per-document disagreement is 8 054 — **2.8% of the term**. So the honest headline is the one my
prediction got wrong in the other direction: the raw "15 873 non-alphanumeric tokens on slides"
figure the brief was built on is **one-sided**, and the quantity that can move a verdict is the
1 658, not the 15 873.

**The finding survives anyway, because the residue is concentrated.** On slides it is 10.4% of
the term rather than 2.8%, and it sits on a handful of `.ppt` decks in amounts large enough to
cross the band: `Aerospace_Journey…ppt` 271 against our 68, `PtF-background…ppt` 23 against our
116, `M.017-(French)-France.ppt` 76 against our 256. That is what buys the twelve verdicts in §6.

---

## 3. Task 1b — producer or extractor? Both questions, answered separately

The coordinator's mid-round input is right that the brief's framing is incomplete, and
`words-rebase-02` §4 is not contested here. But **two different questions were being run
together**, and only one of them is the gate's.

### (a) Why did stored word counts move between containers? — the extractor. Not re-litigated.

`words-rebase-02` established it with a chain this round cannot improve on: identical source
code, both font environments, our own word counts moving on 169 of 200 and **86 documents where
ours and the reference moved by the exact same nonzero amount**. Nothing this round contradicts
that, and I did not reach for a second parser: `arbitrate.py`, the dead instrument in
`words-rebase-02/`, already shows why agreement between two extractors reading one current PDF
discriminates nothing.

### (b) Does check 2 mis-decide **today**? — producer-side, and answerable with no historical artefacts at all

This is what the gate consumes, and it needs no old poppler and no old LibreOffice. **In one
sweep, one `pdftotext` binary reads both columns.** An extractor cannot manufacture a difference
between two columns it reads identically. What can, and demonstrably does:

1. **Both producers write the glyph into the text layer as real text.** `layer.py` reads the PDF
   bytes directly, no extractor involved, on `1-secretariat.ppt`:

   | | ToUnicode CMap → U+2022 | text-showing operators using it |
   |---|---|---:|
   | reference | obj 664, code `0x01` | 93 |
   | ours | obj 46, code `0x15` | 74 |

   A `Tj` operator plus a `ToUnicode` entry is everything any conformant extractor of any
   vintage needs. poppler is *reporting* these, not inventing them. (What this does **not**
   settle: whether an older poppler *joined* those glyphs into tokens differently. That remains
   unanswerable here and is stated as such.)

2. **The two producers use different private-use code points for the same glyph.** Measured over
   the whole slides track:

   | rank | reference | ours |
   |---|---|---|
   | 4 | U+F06E ×815 | U+E439 ×783 |
   | 6 | U+F06C ×355 | U+E5CD ×431 |
   | 7 | U+F0D8 ×351 | U+E46F ×332 |
   | 8 | U+F0A7 ×337 | U+E437 ×267 |

   LibreOffice maps Symbol/Wingdings bullets into the `U+F0xx` range; we map them into the
   OpenSymbol `U+E4xx`/`U+E5xx` range. This is measured, not inferred, and it decides §5's
   rejected alternative (a) on its own.

3. **The reference writes rendering markers we do not.**
   `ODs-February-2022-Airbus-Commercial-Aircraft.xlsx`: the reference emits **`###` 1101 times**
   — the column-too-narrow marker — and we emit it twice. Those 1101 tokens are not text the
   document contains; they are a statement about a column width.

**So: the historical question is the extractor's and the gate's question is the producer's, and
they do not have to be settled together.** The repair is the same either way, which is the
useful part: a count defined over letters and digits is insensitive to whether an extractor
surfaces a bullet, so it removes the producer-side asymmetry *and* immunises check 2 against the
extractor change `words-rebase-02` found — and against the next one.

### (c) "The term is poppler's, so own the extractor and leave the definition alone" — I was
offered this conclusion and it is half right

It was put to me as an available and legitimate result, so it is answered rather than skirted,
and on one track it is simply correct.

**Where it is right: words.** Zero verdicts move (§7). Every words failure's correction is 1–5%
of its delta and the two sides' non-alphanumeric counts are near-identical document by document.
On `.doc` and `.docx` the term is exactly what a shared extractor shift looks like — it lands on
both columns and cancels — and no change to the definition can or should recover anything there.
**That is the honest, publishable negative result for a third of the corpus, and it is reported
as the headline of the words row rather than buried.**

**Where it is not: the residue is not shared.** An extractor cannot make `fy2011-aip-grants.xls`
show 11 538 `$`/`-` tokens on our side and 9 020 on the reference's; it cannot make one workbook
carry 1101 `###` and the other 2; it cannot map the same Wingdings bullet to `U+F0A7` in one file
and `U+E437` in the other. Those are bytes each producer wrote, and they are what buys the
fourteen verdicts.

**Where the two genuinely interact, and I checked rather than assumed.** On `Thailand17.ppt` our
text layer emits `•MORE` and `•LONG` as single tokens where the reference emits `•` and `MORE`
separately — poppler's word-joining heuristic reacting to *our* glyph spacing. That is an
extractor behaviour driven by a producer difference, and it is precisely the class the raw count
cannot survive: `•MORE` scores 1 and `• MORE` scores 2 under `wc -w`, but both score 1 under the
corrected count. The letter-or-digit definition is stable across the join, which is a property I
did not design for and should be recorded as a reason to keep it.

**Owning the extractor does not substitute for the definition.** Pinning poppler is unavailable
here and would only freeze the problem; replacing it with PdfPig moves it. Whatever reads the
PDF still has to answer "is a bullet a word?", and it will answer it whether or not anyone
writes the answer down. §5 is that answer written down, and **`xray-office analyze` should
implement it, not re-decide it** — raw token count, letters-or-digits count, and the excluded
classes broken out. If a reimplementation changes the count for any reason other than a bug,
that is a third incomparable scoreboard and has to be published as one.

---

## 4. Task 1c — what the term actually is, corpus-wide and per track

It is **not** mostly bullets. That is the brief's framing and it is right for slides and wrong
for the corpus.

**Corpus-wide, all 534 documents, reference side** (top-10 = 88.3% of 289 808):

| count | share | code point | |
|---:|---:|---|---|
| 99 650 | 34.5% | U+002D | HYPHEN-MINUS |
| 79 123 | 27.4% | U+0024 | DOLLAR SIGN |
| 39 490 | 13.7% | U+002F | SOLIDUS |
| 8 780 | 3.0% | U+0026 | AMPERSAND |
| 6 880 | 2.4% | U+2013 | EN DASH |
| 6 803 | 2.4% | U+2022 | BULLET |
| 4 565 | 1.6% | U+F0B7 | private use (Symbol/Wingdings bullet) |
| 3 482 | 1.2% | U+20AC | EURO SIGN |
| 3 474 | 1.2% | U+2264 | LESS-THAN OR EQUAL TO |
| 2 958 | 1.0% | U+005B | LEFT SQUARE BRACKET |

**Bullet-class code points are ~15 300 of 289 808 — 5.3% of the term.** The bulk is spreadsheet
cell content: the accounting number format renders a zero as `$` and `-`, and a date as `/`.

**Slides alone**, where the brief's framing does hold (top-14 = 79.8% of 15 873): U+2022 ×6038
(38.0%), U+2013 ×2055, U+002D ×848, U+F06E ×815, U+0026 ×565, U+F06C ×355, U+F0D8 ×351,
U+F0A7 ×337. **Bullet-class = 9 682 of 15 873, 61%.**

**Is it confined to list bullets?** No, on three independent readings, and the strongest is not
about lists at all:

- **Spreadsheet cell values**, §3 item 3 and `fy2011-aip-grants.xls` below — the largest share
  of the corpus-wide term by far, and no list is involved.
- **Autoshape and placeholder text**: 25.3% of the reference's slide-track non-alphanumeric
  tokens are *not* the first token on their extracted line. Weakest of the three — `pdftotext`
  reconstructs lines across side-by-side text boxes on a slide and can put a second box's
  leading bullet mid-line — so read it as consistent-with, not as proof.
- **Provenance is not visible in a PDF text layer at all**, exactly as the prediction said it
  would not be. I can see that a token is `•`; I cannot see whether LibreOffice drew it as a
  numbering label, as a character the author typed, or as content.

---

## 5. Task 2 — the metric, and the four things it is not

> **A token counts as a word if and only if it carries at least one Unicode letter or digit** —
> categories `L*` or `N*`, which is exactly `str.isalnum()`. Tokenisation is unchanged:
> whitespace-delimited, `str.split()`, which reproduces `wc -w` on **1068 of 1068** corpus PDFs.
> The 2% + 3-word band is unchanged. The pages and unembedded checks are untouched.

**Why this is a defensible definition of "the same text is present"**, which is the only thing
check 2 exists to decide: a bullet, a column-overflow marker and an accounting dash are glyphs
the **renderer chose**; a letter or a digit is text the **document contains**. Two renderings
hold the same text when they hold the same letters and digits. Nothing about the rule refers to
bullets, so it does not need updating when the next renderer picks a different marker.

**Rejected, with reasons, and the first two were rejected on measurement rather than taste:**

| alternative | rejected because |
|---|---|
| **(a) strip an enumerated set of code points** (U+2022, U+E000–U+F8FF, …) | It is a list fitted to the documents in hand — the failure mode the brief names. And it *does not work*: §3 item 2 measured that the reference writes its Wingdings bullets at U+F06E/F06C/F0D8/F0A7 while we write the same glyphs at U+E439/E5CD/E46F/E437, so a list built from one side strips nothing on the other and makes the asymmetry **worse**. |
| **(b) compare a normalised full-text extraction** (token multisets, edit distance) | It answers a different question. Check 2 is the cheap second of three checks and is meant to separate "the same text" from "text is missing"; a multiset diff also fires on ordering and reading-order differences, which check 2 was never asked to decide and which the render comparison already covers. Also ~500× the cost on a 727-page document. |
| **(c) count characters instead of words** | *More* sensitive to this term, not less — a bullet is one character where the surrounding word is six — and additionally sensitive to hyphenation, which the band exists to tolerate. |
| **(d) drop short tokens** (length ≤ 2, say) | Would silently delete numbering labels, initials, units and CJK words. The probe in §6.2 is the discriminator: a numbered list must read **76 under both metrics**, and it does. |
| **(e) widen the 2% band** | Forbidden by the brief and wrong on its own terms. The band separates hyphenation drift from missing text; widening it to absorb a systematic term hides the term and blinds the check to real losses of the same size. Fix the term. |

**Implementation note, and it is not a style preference.** `python3`, not `grep` or `awk`. This
image carries only the `C` and `C.utf8` locales. Measured on a file of one token per script:

| | Привет | δοκιμή | 日本語 | café | 1. |
|---|:-:|:-:|:-:|:-:|:-:|
| `grep -c '[[:alnum:]]'` under `C.utf8` | ✅ | ✅ | **❌** | ✅ | ✅ |
| `mawk` (the default `awk` here) `/[[:alnum:]]/` | **❌** | **❌** | **❌** | ✅ | ✅ |
| `str.isalnum()` | ✅ | ✅ | ✅ | ✅ | ✅ |

Either shell tool would silently drop wholly-CJK or wholly-Cyrillic tokens while looking
perfectly correct on the English majority of the corpus. That is the `fc-match` trap in a second
dimension — a tool that never fails and always returns *something* — and it is why the
implementation is Unicode-aware by construction rather than by environment.

---

## 6. Task 3 — validating the instrument, before believing anything it says

### 6.1 The control over already-matching documents — **427 of 428**

Run first, as the standing rule requires. Benchmark to beat or explain: the previous round's
131 of 132 on slides.

| track | already matching | still matching | flipped to failing |
|---|---:|---:|---:|
| words | 154 | **154** | 0 |
| slides | 132 | **131** | **1** |
| sheets | 142 | **142** | 0 |
| **total** | **428** | **427** | **1** |

Slides reproduces the benchmark exactly, and the single flip is **explained, and is an
improvement**. `slides/batch-014/ppt/Thailand17.ppt`:

| | ours | ref | Δ | band |
|---|---:|---:|---:|---:|
| raw `wc -w` | 2850 | 2826 | +24 | 56.5 → passes |
| letter-or-digit | 2736 | 2625 | **+111** | 52.5 → fails |

The reference emits **92 more standalone `•`** than we do. Under the old metric that −92 was
cancelling a +111 surplus of real words in our output, and the document passed check 2 **by the
arithmetic of two errors of opposite sign**. The surplus is real: our text layer emits `Tsunami`
7 times more than the reference's, and emits bare single letters (`S` ×6, `E` ×6, `L` ×4) that
look like a tracked or letter-spaced run being fragmented. The corrected metric is exposing a
difference the old one was hiding. **A control loss that is a true positive is not a control
failure**, but it is the kind of claim that must be shown rather than asserted, so the numbers
are above.

### 6.2 A case whose answer I already know — **exact, five ways**

The probe `words-rebase-01` said was needed and did not run: `mkbullets.py` authors five
flat-ODT documents with **identical body text** — 64 alphanumeric words — differing only in how
their twelve list items are labelled. Rendered by the reference binary.

| variant | raw `wc -w` | corrected | non-alphanumeric | label seen |
|---|---:|---:|---:|---|
| `none` (no list at all) | 64 | **64** | 0 | — |
| `bullet` | 76 | **64** | 12 | U+2022 |
| `pua` | 76 | **64** | 12 | U+F0A7 |
| `dash` | 76 | **64** | 12 | U+2013 |
| `numbered` | 76 | **76** | 0 | `1.` … `12.` |

Every cell was predicted before the probe was run and every cell is exact. Three things it
settles at once:

- the corrected count **returns the known answer, 64, on all four bullet variants** — |Δ| = 0,
  not "near zero";
- `wc -w` swings **19% on a document whose text does not change by one character**, which
  isolates the label as the cause. This is the probe the confound in `words-rebase-01` §"What is
  inflating the extracted word counts" needed;
- the `numbered` row is the control on the *definition*: its labels carry a digit, so they are
  real words under both metrics, and any rule of the form "drop short tokens" fails here.

### 6.3 Replaying stored rows — **9552, zero mismatches**

Reused rather than rewritten: `dotnet/probes/words-rebase-02/verdict.py`, the transcription of
the shell verdict block, run over **every `*.tsv` under `dotnet/probes/`** — 102 files, eleven
rounds. The predecessors replayed 1000 and 2200 rows.

```
replayed 9552 rows, 0 mismatches
```

What this validates is precise and worth stating precisely: **the rule is untouched and only its
input moved.** The band, the string comparison on pages, the `rw == 0` fallback and the
unembedded check are all bit-identical; fed the raw counts, the block returns the stored verdict
on all 9552 rows.

### 6.4 Three further controls

- **Tokenisation control.** My token count equals `pdftotext … | wc -w` on **1068 of 1068** PDFs
  (534 documents × 2 sides). So the only thing that changed between the two metrics is the
  filter — not the splitting, not the extraction, not the tool.
- **Join control.** The census's raw counts equal the independent sweep's `wc -w` column on
  **534 of 534** documents, asserted rather than eyeballed (`score.py` raises otherwise). All
  534 ids resolve; there is no mis-aligned join.
- **End-to-end control.** The modified `batch-check.sh` was run for real on `slides/batch-002`,
  re-rendering the reference with `soffice` from scratch: its verdicts agree with the offline
  computation on **10 of 10**, its `rawwords` column reproduces the independent sweep on 10 of
  10, and the batch goes 7/10 → 10/10.

---

## 7. Task 5 — the three tracks re-measured, old metric against new

Same PDF bytes on both sides of every comparison, so this is a comparison of **metrics**, not of
runs.

| track | docs | old | **new** | Δ | \|Δwords\| old | \|Δwords\| new |
|---|---:|---:|---:|---:|---:|---:|
| words | 200 | 154 | **154** | **0** | 7 023 | 6 786 |
| slides | 163 | 132 | **144** | **+12** | 5 362 | **4 078** |
| sheets | 171 | 142 | **144** | **+2** | 39 992 | 37 457 |
| **total** | **534** | **428** | **442** | **+14** | 52 377 | 48 321 |

The old column reproduces `slides-rebase-01`'s 132 and `words-rebase-02`'s 154 to the digit,
independently re-rendered here. (The brief's 129 for words is `words-rebase-01`'s figure against
a font-starved reference and is superseded; see `words-rebase-02` §1.)

**The sheets pair, 142 → 144, is mine alone and has no published old figure to reproduce
against.** A sheets round was being taken concurrently while this was measured, so if its
scoreboard differs from 142 the difference is a *code* difference — a commit that landed after
`c13049cc002` — and not a metric difference. **The +2 is the metric's effect and is safe to
carry across; the 142 is not.** Re-derive the sheets old column from that round's own sweep
before quoting it; the two named gains (`2012-GA-Survey…xls`, `fy2011-aip-grants.xls`) are
documents, not counts, and transfer regardless.

### Every document whose verdict changes — 17 rows, 15 of which move the scoreboard

**slides — 13 in, 1 out**

| old → new | raw ours/ref | corrected ours/ref | non-alnum ours/ref | deck |
|---|---:|---:|---:|---|
| `words` → **`match`** | 838/871 | **815/815** | 23/56 | `batch-002/ppt/080214-Intl-pol-frameworks…ppt` |
| `words` → **`match`** | 1025/932 | **909/909** | 116/23 | `batch-002/ppt/ws_prod…PtF-background-+-principles.ppt` |
| `words` → **`match`** | 1925/1878 | 1844/1845 | 81/33 | `batch-002/ppt/ws_prod…Part-M-presentation.ppt` |
| `words` → **`match`** | 842/818 | **799/799** | 43/19 | `batch-003/ppt/ws_prod…2007-Privileges.ppt` |
| `words` → **`match`** | 2855/2712 | **2662/2662** | 193/50 | `batch-003/ppt/ws_prod…MDM.032-(ENGLISH)-CZ.ppt` |
| `words` → **`match`** | 1882/1957 | 1814/1836 | 68/121 | `batch-004/ppt/undp_presentation_revised_17_may.ppt` |
| `words` → **`match`** | 6471/6291 | **6215/6215** | 256/76 | `batch-004/ppt/ws_prod…M.017-(French)-France.ppt` |
| `words` → **`match`** | 389/343 | **332/332** | 57/11 | `batch-004/ppt/ws_prod…European-Safety-Strategy-Initiative.ppt` |
| `words` → **`match`** | 210/216 | **150/150** | 60/66 | `batch-007/ppt/1-secretariat.ppt` |
| `words` → **`match`** | 1984/1831 | 1785/1767 | 199/64 | `batch-007/ppt/introduction_to_bea_tuxedo.ppt` |
| `words` → **`match`** | 2635/2571 | 2458/2463 | 177/108 | `batch-008/ppt/concepts-surrounding-cloud-computing…ppt` |
| `words` → **`match`** | 4919/4811 | 4688/4670 | 231/141 | `batch-011/pptx/171128IPAP.pptx` |
| **`match` → `words`** | 2850/2826 | 2736/2625 | 114/201 | `batch-014/ppt/Thailand17.ppt` — §6.1 |
| `words` → **`match`** | 1953/2156 | **1885/1885** | 68/271 | `batch-015/ppt/Aerospace_Journey_of_Flight…ppt` |

**Eight agree on real words to the digit**, and twelve of the thirteen gains are `.ppt` — the
`slides-rebase-01` prediction of "13 in, 1 out" reproduces to the document. `171128IPAP.pptx` is
the one `.pptx` in the list.

**sheets — 2 in, 1 verdict string changed without moving the scoreboard**

| old → new | raw ours/ref | corrected ours/ref | non-alnum ours/ref | document |
|---|---:|---:|---:|---|
| `words` → **`match`** | 643/627 | 636/624 | 7/3 | `batch-002/xls/2012-GA-Survey-Chapter-6-Tables-16Dec2013-V2.xls` |
| `words` → **`match`** | 54739/52221 | **43201/43201** | 11538/9020 | `batch-014/xls/fy2011-aip-grants.xls` |
| `pages` → `pages,words` | 16758/16970 | 16610/15715 | 148/1255 | `batch-017/xlsx/ODs-February-2022-Airbus-Commercial-Aircraft.xlsx` |

`fy2011-aip-grants.xls` is the round's clearest single case: the reference writes an accounting
zero as `$ - `, sometimes running the padding together (`$-` ×379, `$$-` ×132, `$$$-` ×75,
`$$$$-` ×52), we write it as separate `$` and `-`. Raw counts differ by 2518 and fail;
letter-or-digit counts are **43201 and 43201, exact**.

`ODs-February-2022…xlsx` moves in the other direction and is the case that shows the correction
is not a lenience: the reference's **1101 `###` column-overflow markers** were cancelling a real
895-word surplus in our output. It already fails on pages, so the scoreboard does not move, but
the words verdict it now carries is true and the one it used to carry was false.

**words — nothing moves, and that is the result**

Zero verdict changes. Every words failure's correction is 1–5% of its delta and none crosses the
band; the two sides' non-alphanumeric counts are near-identical document by document
(`TE.CAO.00125…docx` 381 against 384, `xx_SETIS_PWS_template…docx` 57 against 57). The words
track's ceiling was hard anyway — of its 46 failures only 9 are `words`-only — but the reason it
moves nothing is not the ceiling, it is that **the term cancels on `.docx` and `.doc`**. The
`4925 PUA tokens` figure the brief carried from `words-rebase-01` is worth **zero** verdicts,
and `words-rebase-02` had already withdrawn the delta it was fitted against.

---

## 8. The prediction, scored

| # | predicted | measured | |
|---|---|---|---|
| R1 | we emit them: >90% of slides, >60% of words | 162/163 and 195/200 | ✅ |
| R2 | does not cancel; >half of decks differ >20% | per-document \|Δ\| is **2.8%** of the term corpus-wide, 10.4% on slides | ❌ **the term very nearly cancels** |
| R3 | slides Σ\|Δ\| is 25–70% of 15 873 | **1 658, 10.4%** | ❌ far smaller |
| C1 | top 10 code points > 85% | **88.3%** | ✅ |
| C2 | not confined to list bullets | held far harder than predicted — bullets are **5.3%** of the corpus-wide term | ✅✅ |
| C3 | numbered labels counted by both | probe: 76 under both | ✅ |
| M1 | letter-or-digit chosen over four alternatives | chosen, and (a) refuted by measurement | ✅ |
| M2 | a naive `grep` drops CJK/Cyrillic and looks fine | held, and **worse**: no UTF-8 locale here gets Han right, and `mawk` is ASCII-only | ✅✅ |
| V1 | slides ≥130/132, ≤6 total flips | **131/132**, **1** flip corpus-wide | ✅ |
| V2 | known answer exact, \|Δ\|≤1 | **0** on all four bullet variants; numbered reads 76 under both | ✅ |
| V3 | replay, 0 mismatches | **9552 rows, 0** | ✅ |
| S0 | old metric reproduces 132 / 129 / 135 | **132 / 154 / 142** | ⚠ slides ✅; words and sheets baselines were stale in the brief — `words-rebase-02` landed 154 mid-round |
| S1 | slides 132 → **144**, 13 in 1 out | **144**, 13 in 1 out, to the document | ✅ |
| S2 | words 129 → 131, ceiling 8 | **154 → 154**, movement **0** | ❌ on the number; the ceiling logic held |
| S3 | sheets 135 → 139 | **142 → 144** | ⚠ +2 against a predicted +4 |
| S4 | total +14 | **+14** (428 → 442) | ✅ on the delta, ❌ on both baselines |
| N3 | no raster-ceiling document moves; if one does, I have a bug | **5 of 104 moved** | ❌ — and the inference was wrong too, see below |
| N1/N2/N5/N6 | provenance, unmapped glyphs, the residual term, ordering — all still invisible | still invisible | ✅ |

**The two refutations worth carrying forward.**

**R2/R3 — I overestimated the term by roughly a factor of five, in the direction the brief
invited.** The 15 873 figure I was handed is a count of one side, and I predicted the two sides
would disagree badly. They agree to 2.8%. The finding survives only because the residue is
concentrated on thirteen `.ppt` decks, which is a much narrower claim than the one the round
started from. Had I not run the refutation check first, §7's twelve verdicts would have been
written up as "the gate was counting bullets" rather than "our `.ppt` path emits a different
number of bullets than LibreOffice's does, and the gate could not see past it."

**N3 — I predicted no raster-ceiling document would move, and said that if one did I had a bug.
Five moved, and the inference was wrong, not the metric.** `TODO.raster-ceiling.md` lists
documents with *at least one* rasterised page; such a document can perfectly well be failing
check 2 for an unrelated reason on its other forty pages.
`ws_prod…European-Safety-Strategy-Initiative.ppt` and `introduction_to_bea_tuxedo.ppt` are on
that list and now match — so for those two the raster ceiling was **not** what was blocking
them. The list is a caution, not a verdict, exactly as its own preamble says about its
threshold.

---

## 9. Code, artefacts, and what is measured versus inferred

**Code, on `wt-gate`:**

- `.claude/skills/corpus-batches/scripts/batch-check.sh` — `words_of()`, the verdict block's
  input, the `rawwords` column, and a non-comparability banner in the header and in the TSV.
- `.claude/skills/corpus-batches/scripts/ref-baseline.sh` — the same `words_of()` and a
  `refrawwords` column. **Changed deliberately and in lockstep**: the two scripts are only
  comparable column for column if they count the same way, and a reference baseline taken with
  the old count against a sweep taken with the new one reads as a corpus-wide word failure.
- `dotnet/probes/gate-01/` — `prediction.md`, this file, and the probes:
  `census.py` (the 534-document, both-sides census with its tokenisation control), `layer.py`
  (the PDF-bytes reader behind §3), `mkbullets.py` (the known-answer probe), `score.py` (the
  old-vs-new scoreboards), `census.tsv` (534 rows: raw and corrected counts, both sides — this
  is the corrected reference column for the whole corpus), `hist.json` (the code-point
  histograms).

**Not changed:** `ref-baseline-all.tsv` beside the canonical reference PDFs. Its `refwords`
column is still the old metric, because five agents are reading it; `census.tsv` is the
corrected column for the same 534 documents.

**Measured here:** ours re-rendered for all 534 at `c13049cc002`; the old-metric scoreboards
428 = 154 + 132 + 142; the both-sides census and its code-point histograms; the ToUnicode/`Tj`
read of two PDFs; the five authored bullet probes; the locale table; the 9552-row replay; the
`slides/batch-002` end-to-end run with a fresh `soffice` reference; every figure in §7.

**Inferred:** nothing in §7 — old and new are computed over the same PDF bytes. The only
inference in this file is §4's "reaches autoshape and placeholder text", which rests on
`pdftotext`'s line reconstruction and is labelled as weak where it appears.

**Not established:** whether an older poppler *joined* these glyphs into tokens differently
(unanswerable here, and it does not affect the gate, which reads both columns with one binary);
the provenance of any individual token; whether `Thailand17.ppt`'s +111 real words are duplicated
text or a fragmented letter-spaced run; and everything `words-rebase-02` §8 lists as open.

## 10. What this leaves open

1. **The `.ppt` bullet-emission difference is still a real defect and is now *only* a defect.**
   With the gate corrected it costs zero verdicts, so it drops out of the scoreboard — but
   twelve decks emit visibly different numbers of bullet characters than LibreOffice does, and
   `Aerospace_Journey…ppt` (68 against 271) is the extreme. It should be re-prioritised as a
   fidelity item, not a gate item.
2. **`Thailand17.ppt`** — the one newly-exposed failure. +111 real words, with bare single
   letters in our text layer suggesting a tracked run being fragmented.
3. **`ODs-February-2022…xlsx`** — we emit 895 more real words than the reference and do not emit
   `###`. Whether we *should* emit `###` is a rendering question this round does not answer.
4. **Rewiring to `xray-office analyze`.** A Tooling agent is replacing `pdfinfo` +
   `pdftotext | wc -w` + `pdffonts` with an in-process PdfPig reader. **`words_of()` in
   `batch-check.sh` is the seam**: it already returns exactly the pair the new verb should emit,
   `<letters-or-digits> <raw>`, so the rewiring is a substitution of that one function body and
   nothing else in the script has to move. Three requirements fall out of this round and are
   handed over rather than assumed:
   - **the definition is §5's and must not be re-decided** — categories `L*`/`N*`, tokenisation
     unchanged from `wc -w`;
   - **the tokenisation control must be re-run**, because PdfPig's text extraction is not
     poppler's. My split reproduces `wc -w` on 1068 of 1068 PDFs; a PdfPig-based count that does
     not reproduce the `rawwords` column on all 534 documents is **a fourth incomparable
     scoreboard**, not a drop-in, and the divergence has to be measured and published before any
     verdict taken with it is quoted;
   - **the excluded classes should be emitted as counts**, not merely dropped, so the next agent
     to find a systematic term can see it without authoring a census.
5. **`ref-baseline-all.tsv` still carries the old `refwords`.** Left alone deliberately (§9).
   Whoever re-bakes the canonical reference should add the corrected column rather than
   overwrite the old one; `census.tsv` holds the corrected values for all 534 in the meantime.
