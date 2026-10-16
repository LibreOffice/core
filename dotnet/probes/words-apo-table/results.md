# A DOC table that belongs in a Word text frame

Measured 2026-09-06 in `/home/user/wt-aac`, branch `agent/aacpage`, base `2f4709c08`.
Reference `/opt/libreoffice26.2/program/soffice` **26.2.4.2**, its Latin metric duplicates and
its Latin `NotoSans`/`NotoSerif` moved aside; `/usr/bin/soffice` is 24.2.7.2 and is what the
gate's own reference half is rendered with. System fonts `/usr/share/fonts`,
`fc-match "DejaVu Sans"` answers DejaVu.

## What regressed, and where it actually was

`words/done-011/doc/AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc` went `match` → `pages`
across the eight merges `531e9a1f3..2f4709c08`, 20 pages to 21 against a reference that gives
20 under **both** installed binaries.

Bisected on the first-parent chain, one build and one render per commit:

| commit | pages | page 2 words | faces |
|---|---:|---:|---:|
| `531e9a1f3` (base) | 20 | 420 | 5 |
| `8d9dae86d` merge(text) | 20 | 420 | 5 |
| `90085378d` merge(words) | 20 | 420 | 5 |
| `260611dae` merge(fonts) | **21** | **362** | **6** |

and inside that merge:

| commit | pages | page 2 words | faces |
|---|---:|---:|---:|
| `1f2489ec2` fix(fonts): glyph fallback asks fontconfig first | 20 | 420 | 5 |
| `e5241e0c1` fix(words): a font code after sprmCSymbol | 20 | 420 | 5 |
| `91ef6cf5e` fix(words): a WW8 style that names no font takes `Stshi.ftcAsci` | **21** | **362** | **6** |

`91ef6cf5e` is right and stays. What it did was put the body in Carlito where it had been in
Liberation Serif, which is what the reference does — and Carlito's line is *taller*, so the
document stopped being able to absorb a pre-existing 72 pt hole on page 1.

**The brief's own reading — "page 2 is where to look first" — is wrong, and page 2 is entirely
innocent.** Ours and the reference draw page 2's paragraphs line for line identically. Page 2
is 68 words short because it inherits two paragraphs from page 1 and therefore loses five lines
at its foot. Page 1 is the seat:

| | ours at `2f4709c08` | 26.2.4.2 |
|---|---:|---:|
| first body baseline (`Airworthiness Directive`, 20 pt) | 587.00 | **659.34** |
| logo, an as-character picture | (546.00, 625.50)-(691.60, 694.10) | **(44.90, 697.94)-(190.40, 766.44)** |
| `Airworthiness Directive` → `Manufacturer(s):` | 559.35 | 558.30 |

The body's *own* extent agrees to 1.05 pt over the whole page. Everything on page 1 simply
starts **72.34 pt** too low, and the picture is 72.44 pt too low with it — one number, not two.

## The mechanism

The header block is not a header. `PlcSpaHdr` is empty and the document's `style:header` is
empty; LibreOffice's own flat-ODF export puts the masthead in the **body**, as a
`draw:frame`/`draw:text-box` at `svg:x="2.7693in"` holding a three-row `table:table`, followed
by a paragraph whose only content is the logo `as-char`. That frame is a Word **APO** — a run
of ordinary body paragraphs carrying `sprmPPc`, `sprmPDxaAbs`, `sprmPDyaAbs` and `sprmPWr` —
and its paragraphs are the table's own.

`Ww8DocumentReader` read those sprms for every paragraph *except* one in a table:

```csharp
TextFrame = paragraph.IsInTable ? Ww8TextFramePosition.None : ResolveTextFrame(markPosition);
```

so the masthead was laid out in the flow and spent its whole height — 72.3 pt — on the first
page. Word's own rule is not "never in a table" but "at a row's head only", and it moves the
row rather than the paragraph (`SwWW8ImplReader::TestApo`, `sw/source/filter/ww8/ww8par2.cxx`
:440, guard `GetCurrentCol() == 0 && InFirstParaInCell()`, with the comment *"if it is the
first cell of a row then the whole table row jumps into the new frame"*).

The fix reads the position at every cell paragraph, keeps the one standing at each row's head,
requires the rows to agree, and hangs the whole table on the frame; `LiftTextFrames` now lifts
a table block as readily as a paragraph one.

## What it moved, and in which direction

Words track re-swept whole, 338 of 338 against `/home/user/gate-2f47/parity.tsv`:
**match 312 → 313, `pages` 22 → 21**, and exactly two rows move at all — the AAC document
(`pages` → `match`, 21/20 → 20/20, 7454/7453 → 7453/7453 words) and `CP-ETSO template_v2020.DOC`
(one word, 1111 → 1110, verdict `match` throughout).

Five `.doc` renderings change pixel for pixel, and every one of them moves **towards** 26.2.4.2
— `movers.tsv`, ink is the mean absolute grey difference at 30 dpi over the shared pages:

| document | pages before/after/ref | ink before | ink after |
|---|---|---:|---:|
| `AAC-AD-No-2021-01…doc` | 21 / 20 / 20 | 22.93 | **10.51** |
| `手机免提系统TSB.doc` | 3 / 3 / 3 | 14.27 | **9.48** |
| `CP-ETSO template_v2020.DOC` | 6 / 6 / 6 | 14.30 | **11.02** |
| `237287_…geolink2neu.doc` | 8 / 8 / 4 | 23.72 | **21.82** |
| `2013_11.doc` | 8 / 8 / 8 | 15.41 | **14.94** |

`CP-ETSO`'s one lost gate word comes with a 23% fall in ink, so the gate's word column is
reading a re-flow, not a loss. `geolink2neu` paginates 8 against 26.2's 4 both before and
after: that is the version gap — the gate's 24.2.7.2 reference gives 8 — and is untouched here.

## What the AAC document still gets wrong

The logo's **vertical** placement is now exact — (546.00, **697.85**)-(691.60, **766.45**)
against the reference's (44.90, **697.94**)-(190.40, **766.44**) — and its **horizontal** is
still wrong by 501 pt: we draw it half off the right edge of the paper.

Its paragraph is right-aligned with a 0.389 in right margin, so its right edge is 546.0, and we
put the *left* edge of a 145.6 pt picture there. Two things are missing and each is worth one
line of arithmetic: the picture's own width is not in the line the alignment measures (546.0
rather than 546.0 − 145.6 = 400.4), and the line is not narrowed to the band left of the frame
(the frame's wrap zone starts at 190.4, and 190.4 − 145.6 = **44.8**, which is the reference's
44.90 to within the picture's 0.0008 in padding). `FrameObstacles.SpaceFor` already computes
that band correctly for text; an as-character frame's line does not reach it. Pre-existing —
the picture sat at x = 546.00 at `90085378d` too — and it costs no page.

The other residual is on page 1's `Foreign AD:` row and is the **justification shrink** the
fidelity suite already fails on: the reference fits `…dated 20 November 2020,` on one line by
drawing the whole line at `0.9875 0 0 1 148.7 397.389 Tm` — a 98.75 % horizontal scale, which
is `SwTextGuess::Guess`'s *"tdf#168251 minimum glyph scaling allows more text in the line"* plus
`SwTextPortion::Format`'s `SetScaleWidth`. We do not shrink, so `2020,` wraps. Glyph advances
agree: word for word on that line ours/theirs is 1.0135–1.0145, against the 1.01266 the scale
alone predicts and the ~0.05 % the reference's truncated PDF widths add. Whole document:
`0.9875` on six records of one line and `0.9863636364` on four of another, and nothing else in
twenty pages — so it is a per-line shrink, not a character scale the document states.
