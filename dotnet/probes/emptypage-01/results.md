# Where an empty paragraph goes at a page boundary

Measured 2026-08-15 in the `/c/sandbox/workdir` container: LibreOffice **26.2.4.2**, DejaVu, Carlito,
Caladea and Liberation installed. X-Ray.Office at `wt-w-emptypage`, forked from `d9fde84d42e`.

## The question

Three documents were briefed as one theme — "where empty content belongs relative to a page
boundary" — with two apparently opposite signs:

| document | ours/ref | claim |
|---|---|---|
| `words/pagination-002/doc/150_5300_13_chg12.doc` | 30/31 | the reference emits a page holding only a running head and a page number; we merge it away |
| `words/table-001/doc/150_5300_13_chg8.doc` | 19/18 | its page 15 is the same class |
| `words/extra-001/doc/info-bulletin-601.doc` | 7/6 | the mirror: our pages 5 and 6 open with empty paragraphs the reference keeps at the foot of the page before |

## The probe

`build2.py` writes 48 DOCX files: *n* filler lines (`n` = 50…61), then *e* empty paragraphs
(`e` = 0…3), then a paragraph carrying `w:pageBreakBefore`. Everything that decides the arithmetic
is nailed down so that the two stacks cannot disagree about the fill: Liberation Serif 12 pt in
`w:rPrDefault`, `w:spacing w:line="240" w:lineRule="exact"` in `w:pPrDefault`, A4, 1 inch margins,
no header or footer. Body height is 13958 twips, so 58 exact 12 pt lines fit.

(`build.py` is the first cut and is kept only as the record of it. It states no styles at all, so
LibreOffice sets it at 12 pt and X-Ray.Office at 10 pt; the fill differs by 1.95 pt per line and the
comparison measures that instead. It still reproduced the class once, at `n49-e3`.)

## Result: 48 of 48 agree, page count and last-line position alike

The transition, which is where the rule lives:

| | e=0 | e=1 | e=2 | e=3 |
|---|---|---|---|---|
| n=55 | 2/2 | 2/2 | 2/2 | 2/2 |
| n=56 | 2/2 | 2/2 | 2/2 | **3/3** |
| n=57 | 2/2 | 2/2 | **3/3** | 3/3 |
| n=58 | 2/2 | **3/3** | 3/3 | 3/3 |

`ours/ref`. Each empty paragraph costs exactly one line of room in both stacks, and the page the
overflowing empty paragraph lands on is the same one in both. The last filler line sits at the same
`yMin` on both sides for every one of the 48.

**So there is no divergent rule to implement.** X-Ray.Office already places a run of empty paragraphs
across a page boundary the way Writer does, including the "near-blank" page that results when the
last of them overflows.

## What the three documents actually are

All three are the *same* construction, and it is not a hard break — it is an automatic one falling
inside a run of trailing empty paragraphs. LibreOffice's own `--convert-to fodt` shows it directly,
as a `<text:soft-page-break/>` inside one of the empty paragraphs:

* `150_5300_13_chg12.doc` — after `Figure 2-3. Runway protection zone`:
  `<text:p P120/><text:p P80/><text:p P80><text:soft-page-break/></text:p>`, then `Table 3-1`
  starts a new master page. The reference's blank page 9 is the third empty paragraph.
* `150_5300_13_chg8.doc` — `<text:section name="Section10"><text:p P151><text:soft-page-break/></text:p></text:section>`,
  then `Table A16-1A` starts a new master page. The reference's near-blank page 15 is that one
  empty paragraph.
* `info-bulletin-601.doc` — **24** consecutive empty paragraphs after the disclaimer, with the
  soft break in the 24th, then `Target Dates for Implementation of the Transport Standards`.

A soft break inside such a run lands wherever the preceding page's fill leaves it. So the page it
falls on is a *readout of the fill*, not a rule of its own — which is why the two signs in the brief
are not two rules and not even one rule. They are the same measurement disagreeing in both
directions depending on whether our page above was fuller or emptier than the reference's.

## `info-bulletin-601`: the fill, traced to its cause

Measured with `pdftotext -bbox`:

| | ours before | ref | ours after |
|---|---:|---:|---:|
| page 5, `Useful Web Links` | 124.60 | 99.30 | 99.30 |
| page 6, `Target Dates…` | 141.10 | 113.50 | 113.50 |
| pages | 7 | 6 | 6 |
| words (letter-or-digit) | — | 1320 | 1320 |

The 25.3 pt and 27.6 pt were the leading empty paragraphs, and they were there because page 3 had
lost a line: every **first** line of a bulleted item started at x 66.00 against the reference's
51.40. 66.00 − 30.00 = 36 pt is the default tab interval; 51.40 − 30.10 = 21.30 pt is the
paragraph's own declared tab stop, which is also its left indent.

The level states a 36 pt list tab (`nTabPos` = 720 twips, read identically by both stacks — the
level's `grpprlPapx` is `…15C6 05 00 01 D002 06…`). The paragraph declares a stop at 21.30 pt.
Writer merges the level's stop **into** the paragraph's ruler (`SwLineInfo::InitLineInfo`,
`sw/source/core/text/inftxt.cxx`:124-137) and then runs the ordinary
`SwTextFormatter::GetTabStop` over the merged list, so the level's stop wins only where it is the
nearest one ahead of the pen. The pen after a label sits inside the hanging indent, so the search
position is negative relative to the tab origin and every paragraph stop is still ahead of it.

`PageLabel.ListTabAdvance` preferred the level's stop unconditionally. The fix moves the merge into
`ParagraphFormat.NextTabStop(Length, Length?)`.

## The residue on `info-bulletin-601` page 1, and why it is not a second defect

Pages and words are now exact, but page 1 still differs and the difference is worth stating so the
next round does not re-open it. Anchoring on `Transport?` — the second line of heading 1, at ink
top 273.20 on both sides — the reference's heading 2 sits **30.0 pt** lower than ours, and 30.0 pt
is exactly **two** of this document's 15.0 pt body lines (`Text body` carries
`style:line-height-at-least="0.2083in"`).

One of the two is visible: the reference breaks `…minimise the risk of` / `complaints.` where we
keep `risk of complaints.` on one line, with the line ending at x 311.8 in a 569 pt measure, so
width does not force it. The paragraph ends `complaints.` + U+00A0 + space. The other 15.0 pt
carries no ink at all and falls at the same place.

That is the NBSP rule an earlier round already characterised, and it is the *reference* that is a
line longer, not us. A blind reviewer given only the paired image and no numbers reported both
symptoms independently — "the right half breaks paragraph B one line earlier … the left half's
line 4 ends far short of the right margin, so nothing about column width forces that break", and
"heading 2 sits noticeably lower on the right … more than the one extra body line accounts for".

Reproducing it would make our page 1 one line shorter, not longer, and the standing note is that
implementing it takes this document to 8 pages rather than to 6. It is left alone.

## What this does *not* explain

`chg12` and `chg8` are unchanged at 30/31 and 19/18. Both diverge from the reference well before
their near-blank page:

* `chg12` page 4 draws two columns of body text on top of each other (`PrecisionAirplane. / Large
  Approach An Category II airplane (CATofII) more Runway.than A` in `pdftotext -layout`), and its
  pages 4-6 hold 1081/443/212 words against the reference's 785/601/386. Page 8's furniture is the
  even-page set because our page number there is 20 where the reference's is 19 — one page of drift
  already accumulated.
* `chg8` page 14 holds entirely different text from the reference's page 14.

For both, the near-blank page is downstream of that, exactly as the probe predicts: same rule,
different fill.

## Regression sweep

`words/done-001` … `words/done-016`, run as `done-00[1-8]` (80 of 80 match) and
`done-0[01][09123456]` (141 of 141 rows, 139 match), which between them cover all sixteen batches
with 001-006 measured twice and agreeing.

The two non-matching rows are one document under both its casings —
`words/done-015/docx/airbus-pdf-information-package_v1-4.docx`, pages 9/9 and words **1269/1299**,
which fails on `d > b*0.02 && d > 3` by four words. **It is not a regression**, established by
building the pre-fix binary and measuring it: 9 pages and 1269 words there too, with
`info-bulletin-601` at 7 pages on the same binary as the control that the revert took. The
reference draws a repeated table heading — `SupplyOn Quality Gate Mapping / ID / Field / Example /
Mandatory` and the `(to be filled by the supplier)` / `Where can you find this value on your
invoice: (do not change!)` cells — on one continuation page where we draw none. Sorting both word
streams shows the whole 30-word difference is that one block; everything else is the same tokens in
a different reading order.
