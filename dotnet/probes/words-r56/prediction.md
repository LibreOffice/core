# words-r56 — prediction, committed before anything is changed or re-rendered

Baseline reproduced first: `batch-check.sh … 'words/*' … 8` → `TOTAL 355 MATCH 336 MISMATCH 19`,
scored against `MANIFEST.tsv`'s own 337-path list → **319 match, 18 open, zero disagreements with
the manifest's status column, document for document**. The brief's figure exactly.

## The change this covers

**Synthetic oblique does not survive the uniform-paragraph shortcut.** All four word-processing
readers build a paragraph's `PageRun` list only when its formatting *varies*, and each of the four
sites writes the list of properties that count out longhand — face, size, colour, language,
escapement, case map, highlight, underline, strike-through, kerning, tracking. Slant is not on it,
and for most families it does not need to be: an italic run of `Arial` resolves to
`LiberationSans-Italic`, a **different `OpenTypeFace`**, so `face != paragraphFace` fires. The
families with **no italic installed at all** are exactly the fallback faces — DejaVu Sans and
DejaVu Serif have Book and Bold and nothing else — so an italic run that falls back resolves to the
*same* face as its upright neighbour, every other test passes, and the paragraph is folded into one
upright run. The lean is lost at the fold, and it is lost *silently*, because the shortcut's whole
correctness argument is that it only drops properties that do not change a measurement.

Seated by a discriminating pair rather than by argument, `oblique-uniform.py`, ten authored
packages of one paragraph and two runs:

| case | reference leans | we lean |
|---|---:|---:|
| `nonesuch/i` — run 2 states `w:i` and nothing else | 23 | **0** |
| `nonesuch/i+sz` — the same run, plus a `w:sz` the shortcut already tests | 23 | **22** |
| `nonesuch/style-i` — the whole paragraph italic, no run differs | 47 | 46 |
| `arial/i`, `courier/i`, `nonesuch/sz-only`, `nonesuch/b`, `nonesuch/iCs` | 0 | 0 |

The only difference between the first two rows is a property already on the `varies` list. No
hypothesis about how we read `w:i` predicts that; the shortcut does.

The fix is one clause at each of the four sites: `DocxLayoutSource.RunsOf`,
`OdtLayoutSource.RunsOf`, `RtfReader.RunsOf`, `DocReader.RunsOf`. It is the same sentence the
existing clauses carry — *a property that decides only what a mark looks like cannot decide where it
lands, so it has to survive the fold* — applied to the one drawing-only property that was missed.

## What I predict

Instrument named alongside every figure, because round 55 slides missed its own prediction three
times by not doing that.

| quantity | instrument | baseline | predicted after |
|---|---|---:|---|
| sheared glyphs, ours | `shear-chars.py` | 158 673 | **164 000 – 166 500** |
| sheared glyphs, reference | — | 154 501 | 154 501 (fixed) |
| documents the reference shears more of | `shear-split.py` | 38 | **6 – 14** |
| glyphs in that direction | `shear-split.py` | 6 819 | **300 – 1 400** |
| documents where we shear **none** and it shears some | `shear-split.py` | 15 | **0 – 3** |
| pages where the reference shears and we draw none | `shear-split.py` | 162 | **0 – 25** |
| documents we shear more of | `shear-split.py` | 8 | **8 – 12** |
| glyphs in *that* direction | `shear-split.py` | 10 991 | **10 991 – 11 600** |
| **verdict movement** | `batch-check.sh` vs `MANIFEST.tsv` | 319 of 337 | **0, i.e. 319** |
| downside risk | same | — | **−1 to −3** |
| page counts changed | `parity.tsv` | — | **0 to 4** |

**Verdict movement of zero is the honest answer and I am predicting it.** The gate reads page count,
extractable word count and font embedding. A synthetic oblique changes none of the three: the
reference passes the same slant to HarfBuzz as `hb_font_set_synthetic_slant`, which moves outlines
and leaves advances alone, so the roman and italic halves of a line carry the same `TJ` array and
the same pen origin. Nothing reflows, no token is added or lost, and the face embedded is the same
face. The fix is worth shipping because it is 6 819 glyphs drawn wrong on 412 pages, not because a
column can see it.

**The named downside.** The fold is not free to undo. Splitting a paragraph into runs breaks the
shaping context, and `PageContent.Coalesce` is what puts it back — it rejoins adjacent
`FormattedRun`s that are equal in every measurement field. If the drawing-only flag I add to the
split predicate were also to reach `FormattedRun`, the rejoin would fail and every affected
paragraph would measure a fraction wide, which on a long document is a page. I therefore expect
0 page-count changes and will treat any as a failure of this reasoning rather than as noise.

## The 38 documents the reference shears more of

26 `.docx` (6 461 glyphs) and 12 `.doc` (358). Largest first:
`EHEST-SMS-Safety-Management-Manual-V2` 5 003 (and it is one of the 18 open documents, 80/82 pages),
`TE.CAO.00125 … OJT Logbook` 668, `SPA-06_mcar_part-6` 248, `SPA-11_mcar_part-11` 235,
`review-welsh-government-communications-mister-peter-mandelson` 105, `手机免提系统TSB` 82,
`A320SimNotes` 75, `1228841571067_2009_TPPT_13` 74, `2024-2nd-quarter-case-summaries` 63, and 29 more.

Fifteen of them draw **no** sheared glyph at all today, which is the cleanest reading of the class:
the whole document's italic falls back to a family with no italic installed.

`review-welsh` is the worked case. Both sides embed the identical six faces; both draw its page-5
table body in DejaVu Sans; the reference leans 105 of those glyphs and we lean none. The four runs
are `<w:rPr><w:rFonts w:eastAsia="Aptos"/><w:i/></w:rPr>` inside paragraphs whose other runs differ
by nothing but the `w:i`. **Same face lists, per-run divergence** — the shape the slides round named.

## What this census cannot see, written down before the sweep

The census is of **the reference's own output**, which round 55 slides established is the most
direct census available and still got the column wrong. Specifically:

1. **It cannot see paragraphs where both sides already lean but on different runs.** 250 pages have
   both sides shearing with ours fewer and 143 with ours more; the fix moves an unknown part of the
   first group and I have not modelled it. The 6 819 figure is the *deficit*, not the number of
   glyphs that will change hands.
2. **It cannot see the over-shear at all, and cannot rule out that this fix makes it worse.** Eight
   documents lean 10 991 glyphs more than the reference — `644730BRI0mna000BOX361539B00public0.doc`
   6 643 to 2 171, `SPA-02_mcar` 58 011 to 54 694, `02_mcar` 49 900 to 46 856. A blind reader on
   `644730BRI` page 2 reports an entire lead paragraph set slanted on our side and upright on the
   reference's, so that is a *different* defect — but a fix that only ever adds lean can only add to
   it, and if any of those documents has an italic run folded into an upright paragraph as well, my
   upper bound is low.
3. **It cannot see ODT or RTF.** The words corpus holds no ODF text document at all, and the RTF
   arm is changed on the same argument with no witness. Those two of the four sites ship
   **unmeasured on the corpus** and are covered only by unit tests. Say so rather than counting them.
4. **It cannot see documents where the shortcut folds an italic run whose face has an italic**, i.e.
   nothing, correctly — the negative control is that `arial/i` and `courier/i` are 0 on both sides
   before the change, and they must still be 0 after it.
5. **A subset-encoded PDF hides which text leaned.** The counts are exact — a `Tj` string is one
   byte per glyph in these encodings — but the *identity* of the leaning text was established on
   `review-welsh` from the package, not from the PDF, and only on that one document.

## Not covered by this prediction

The `.doc` over-italic and the legacy `FORMCHECKBOX` fields are separate items in this round and get
their own predictions if they reach a change.
