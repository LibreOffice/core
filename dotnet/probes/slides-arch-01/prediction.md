# slides-arch-01 — prediction

**Subject:** `slides/batch-007/ppt/architecture6.ppt`, briefed as page-exact at 31/31 with
**1926 words against 2544** — a 618-word deficit, "about a quarter of the document".

## Honesty note on when this was written

This file is **not** wholly pre-measurement, and saying otherwise would be the exact dishonesty
the convention exists to prevent. By the time it was written the round had already established,
in order: the character-stream verdict (§P1), the defect (§P2) and its **reach** (§P4). What
remains genuinely unmeasured, and is therefore a real prediction, is §P5 (regression sweep), §P6
(test counts) and §P7 (the gate column). Each section says which it is.

---

## P1 — what kind of round this is *(measured before writing; recorded for the record)*

**P1.1 — The deficit is not missing content.** Whitespace-stripped character streams:
**ours 11048 characters, reference 11038**. We draw *ten more* characters than the reference.
Ratio 0.9868, and every opcode of the difference is one of four things: a bullet's PUA code
point (`U+E47A`/`U+E46F` ours, `U+F0B2`/`U+F0A7` reference), table reading order, three
hyphens `pdftotext` de-hyphenates out of the reference's own line breaks, and two words
(`each layer.`) the **reference** loses off the bottom of page 13.

**P1.2 — The reference is splitting its own words**, shape 3 of `TODO.raster-ceiling.md`, at a
scale an order of magnitude past the recorded case. Confined to five pages — 10, 14, 21, 24, 27,
the pattern-table slides — which hold the entire 618. Page 10's description cell is
**"65 glyphs in 64 show(s)"** in the reference against **"74 glyphs in 11 show(s)"** in ours.

**P1.3 — The gate cannot be won on this document** and the round is therefore a ceiling entry,
not a fix. To reach 2544 we would have to inflate our own tokenisation by 32% by adopting
per-glyph positioning, which is strictly worse output.

## P2 — the real defect the round found *(measured before writing)*

**P2.1 — Every bullet we draw on a binary `.ppt` is hard black.** All 80 OpenSymbol glyphs in
`architecture6.ppt` were `0 0 0 rg`; the reference draws 74 in the paragraph's own
`#46424D` and 6 in `#FF0000`, matching the first character run of each paragraph.

**P2.2 — Cause.** `PptTextBody.Marker` resolved `bulletColor` unconditionally. PowerPoint writes
that word whether or not the bullet has a colour of its own and gates it behind
`PPT_ParaAttr_BuHardColor`, bit 2 of the bullet-flags word; with the flag clear the colour is the
**first portion's** font colour (`svdfppt.cxx:5891-5916` and `:6019-6055`). Our reader discarded
bits 1-3 of that word at `PptTextReader.cs:544`, keeping only bit 0.

## P3 — what I predict is *not* the cause, so it is not chased

**P3.1** The five table pages' 15% width inflation is **not** a metric bug of ours. The reference
positions those glyphs with per-glyph `TJ` corrections of −12 to −164 thousandths; fitting the
implied advances gives **DejaVu Sans Bold** (830/678/595/493/343/478/687 against our Liberation
Sans Bold 722/556/556/389/278/333/611), so LibreOffice measured with one face and drew with
another. The 24 pt title is **157.39 pt ours against 157.28 pt reference** on the same page —
0.07% — so nothing global is wrong with our pen. **Not fixable from our side and not attempted.**

**P3.2** `fonts 5/5` is real here, not coincidence: both sides embed the same five faces and
`/Widths` on the reference's `LiberationSans-Bold` are Liberation's own.

## P4 — reach *(measured before writing)*

**P4.1** 26 of 163 slides renderings change, all binary `.ppt`; 137 byte-identical under
`SOURCE_DATE_EPOCH`.

**P4.2** 935 bullet glyphs take exactly the reference's colour where they previously did not,
**0 move away**. By colour-multiset distance, 21 of 26 documents move closer to the reference, 0
further, 5 unchanged in aggregate.

---

# Genuinely unmeasured from here down

## P5 — regression sweep

**P5.1** `slides/batch-001`…`007` will stay at **66 match of 68**, with the same two failures —
`solog_orientation_august_2019.pptx` (documented ceiling) and `architecture6.ppt` (this round's
ceiling). I predict **no verdict moves in either direction**: the fix changes only colour, and
the gate scores pages, words and font embedding, none of which colour touches.

**P5.2** Specifically I predict `architecture6.ppt` stays `31/31, words, 5/5, 0` with its word
count unchanged at **1926/2544**. If its word count moves at all, something other than colour
changed and the fix is wrong.

**P5.3** Risk I am carrying: 26 documents changed, and a bullet that now resolves to a run colour
could in principle resolve to the *background* colour on a document where the first run is
white-on-dark, making a bullet vanish. Nothing in the sweep would see that — it is invisible to
all three gate checks. I predict it does not happen, on the grounds that LibreOffice does the
same thing and the references agree, but the sweep is not evidence either way.

## P6 — tests

**P6.1** Fidelity baseline before the change measured **30 failed of 550, 0 skipped**. I predict
it stays **30 of 550**. The one test that could plausibly move is
`OutlineMarkerComparisonTests`, and it compares X, Y and font size, not colour.

**P6.2** The other seven projects will stay green with counts unchanged, and I predict the total
across all eight is unchanged apart from the tests this round adds.

**P6.3** No test in the tree asserts a bullet colour today. The tests this round adds are the
first, so a regression here has been undetectable.

## P7 — the gate column

**P7.1** `architecture6.ppt` will still fail on `words` and will still be the only failure in
batch-007. It joins `TODO.raster-ceiling.md` as the largest recorded instance of shape 3.

**P7.2** I predict the visible page improved and the gate column did not move — the same sentence
that ceiling file already had to write once.
