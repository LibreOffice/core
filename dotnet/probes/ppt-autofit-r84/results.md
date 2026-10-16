# A `.ppt` hyperlink is a field — and the outline autofit is `SlideAutofit`, by 0.32 pt

Round 84, on the original corpus's `.ppt` column. Two items: the legacy twin of round 82's
DrawingML field rule, ported and wired; and the instrumentation the outline-placeholder autofit
item was waiting for. The first is closed with a measured residual; the second is diagnosed and
left, because the diagnosis says the defect is somewhere neither of the two seats the brief named
is.

## Environment

| | |
|---|---|
| base commit | `6d19fd594`, worktree `/home/user/wt-pptautofit`, branch `agent/pptautofit` |
| reference | `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2** `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| `/usr/bin/soffice` | 24.2.7.2 — **not used for anything in this round** |
| corpus | `/home/user/sample-files`, the original 947; the `.ppt` column is 51 documents |
| C++ tree | `/home/user/libreoffice-core`, read only |
| fonts | all five tarball confounds aside, as they have been since 2026-09-07 |
| workers | 2 for every sweep |

**No build overlapped a sweep**, checked the way the rulebook requires — the newest render's mtime
against the binary's, never `pgrep`. The three builds this round needed (base, head, and a
temporarily instrumented head) were each run to completion with nothing rendering, and the
instrumented one is *proved* out of the tree rather than argued out: after reverting it, all
**51 of 51** `.ppt` re-render byte-identically to the head sweep taken before it
(`hash-sweep.sh`, `SOURCE_DATE_EPOCH` fixed on every leg).

**Disk.** `/` ran to 73 MB free part-way through, so every sweep here renders one document at a
time, takes its digest or its span census, and deletes the PDF. That is why `hash-sweep.sh` and
`size-sweep.sh` bank digests and sizes rather than renderings.

---

# 1. The `.ppt` text-range hyperlink

## The rule, verified at the seat

Every citation below was opened and read in this round.

**A `.ppt` hyperlink is an EditEngine field, exactly as a DrawingML one is.** `PPTTextObj`'s
`PPT_PST_InteractiveInfo` case builds a `SvxFieldItem(SvxURLField(…), EE_FEATURE_FIELD)` for the
range (`filter/source/msfilter/svdfppt.cxx`:6936); the second seat splits it across the
`PPTCharPropSet`s the range covers, giving each its own field whose representation is that
portion's own characters (`:7069` for a portion the range swallows whole, `:7090` for the one it
ends inside), imposes `PPT_COLSCHEME_A_UND_HYPERLINK` on each (`:7060`, `:7094`; the constant at
`:145`) and forces the underline attribute on (`:7054-7056`).

**Three conditions decide whether the record becomes a field at all**, and only the first is in the
record:

1. the `TxInteractiveInfoAtom` must be the `InteractiveInfo`'s **very next sibling** —
   `:6911-6921` seeks to the end of the container, reads the next header, and puts it back when it
   is not a `TxInteractiveInfoAtom`;
2. its end must be non-zero (`:6926`);
3. **the atom's `exHyperlinkId` must match an entry of `SdrPowerPointImport::m_aHyperList`**
   (`:6907-6909`). Everything that makes the run a field happens inside that loop.

**The third is the legacy twin of DrawingML's non-empty hyperlink property map.** `m_aHyperList`
is built by `ImplSdPPTImport::Import` from the `_PID_HLINKS` blob of the user-defined section of
`\005DocumentSummaryInformation`, six properties per link
(`sd/source/filter/ppt/pptin.cxx`:353-518), and each entry then takes the *k*th
`ExHyperlinkAtom` of the `ExObjList` (`:531-547`); when the blob is absent, one entry per
`ExHyperlink` is built instead (`:551-575`). Either way **the set of indices that can match is the
set of `ExHyperlinkAtom` values**, which is what this tree reads.

**A `.ppt` hyperlink's emphasis is *replaced*, not added to.** `:7054-7056` sets the underline bit
in `mnAttrSet` and then **assigns** `mnFlags = 1 << PPT_CharAttr_Underline` rather than or-ing it,
so every emphasis the portion stated for itself is turned off *and stated as off*, while the ones
it left to the master still come from there. A bold linked run is drawn light.

**And a soft bullet reaches past the link for its colour.** `PPTParagraphObj::GetAttrib`'s
`PPT_ParaAttr_BulletColor` case keeps `mnHylinkOrigColor` for exactly this
(`:6037-6042`), falling back to the master character level's own colour when the portion stated
none — so a bulleted paragraph opening on a hyperlink keeps the bullet it had before the link
recoloured the text.

## The census — and the instrument that got it wrong first

`census.py` walks only the objects the **persist directory** names. That matters more than it
sounds: a `.ppt` stream keeps every superseded version of every object, and a top-to-bottom scan
counts orphans as live.

| instrument | documents with a text range | atoms |
|---|---:|---:|
| a stream scan (`stream-scan-census.py`, kept beside the right one) | 23 | **235** |
| the live tree (`census.py`) | 23 | **171** |

The stream scan is not merely inflated, it is *wrong about the answer*: on
`080214-Intl-pol-frameworks…ppt` it finds a dead `ExObjList` declaring **one** link where the live
one declares **two**, which would have left that deck's second URL unlinked. `census.py` ports
`PptPersistDirectory.Read` to Python (`persist.py`) so the two halves agree.

Over the live tree of the corpus's 51 `.ppt`:

| | |
|---|---:|
| documents stating a live text range | **23** |
| such ranges | **171** |
| documents whose ranges resolve to a declared hyperlink | **20** |
| ranges that resolve | **111** |
| documents stating ranges and declaring **no** hyperlink at all | **3** |
| ranges in those three | **60** (57 of them in `BUS-Chapter 05.ppt`) |

**The three are the control the rule needs, and 26.2.4.2 confirms it.** `BUS-Chapter 05.ppt`,
`WC_Update-Aug03.ppt` and `JesuitAssocOfStudentPersonnel.ppt` carry no `ExObjList`, no
`ExHyperlink`, and no `_PID_HLINKS`; the reference draws every one of their 60 ranges in the
body's own colour, word-broken and not underlined — page 50 of `BUS-Chapter 05.pdf` draws all
five of its ranges at `#001932` like the rest of the slide. **All three are byte-identical before
and after this round.**

**`_PID_HLINKS` need not be read**, and that is measured rather than assumed: over the 27 `.ppt`
that state either, the blob's link count equals the live `ExHyperlinkAtom` count on **27 of 27**,
with no disagreement (`hyperlist.py`, which ports `PropItem::Read`'s alignment and NUL rules from
`sd/source/filter/ppt/propread.cxx`:73-186). So the reader takes the live `ExObjList`'s ids and
does not open the property set.

## The change

Five files, all under `XRay.Office.Presentations/MsBinary`, which only the `.ppt`/`.pot`/`.pps`
reader uses.

- **`PptHyperlinks.cs`** (new) — the `ExHyperlinkAtom` values of the live `Document`'s `ExObjList`.
- **`PptRecordTypes.cs`** — the six record types the walk needs.
- **`PptTextReader.cs`** — `PptHyperlinkRange`, `PptTextRun.Hyperlinks`/`IsLinked`, the
  next-sibling walk, and the shift a running field's substitution applies to a range.
- **`PptTextBody.cs`** — a character run is split at the link boundaries and each covered piece is
  built with `IsField`, the scheme's hyperlink slot, the forced underline and the emphasis
  replacement; and `MarkerColour` takes the pre-link colour when the paragraph opens on a link.
- **`PptSlideLayout.cs`** — reads the set once per deck and passes it into both text paths (the
  shape's own client textbox and the slide list's outline text).

Everything else — `CellBrokenSpan`, `CellBreaks`, `SlideTextLayout`'s `Fields` /
`ContinuesField` / `PlacedLine.ContinuesField` and the block-height rule — is round 80's and round
82's and is untouched. **The third importer cost one expression**, exactly as round 82 said the
second did.

## What moved — confinement is exact

`hash-sweep.sh` renders each document with the base binary and with this one,
`SOURCE_DATE_EPOCH` fixed on both legs, and compares digests.

| column | rendered both ways | differ |
|---|---:|---:|
| `.ppt`, all of it | **51** | **20** |
| `.pptx` (every 25th) | 10 | 0 |
| `.docx` (every 25th) | 10 | 0 |
| `.xlsx` (every 25th) | 9 | 0 |
| `.doc` (every 25th) | 2 | 0 |
| `.xls` (every 25th) | 2 | 0 |
| `.xlsm` | 1 | 0 |

**The 20 movers are exactly the 20 documents the census predicts**, name for name. The three that
state ranges and declare no hyperlink are byte-identical, the 28 that state no range at all are
byte-identical, and **no page count moved on any of the 84**. `reach.tsv`.

The other tracks are a sample and are stated as one; what makes a sample enough is that the diff
is five files in `XRay.Office.Presentations/MsBinary`, which no other family's reader references.
`PptTextReader` is shared with `PptContentBuilder` (extraction), which passes no declared set, so
extraction cannot move either — and there is a test for that.

## What moved — against the reference

The gate cannot score this: a hyperlink drawn black has exactly the alphanumeric characters it has
drawn blue, so no `glyphs` count and no page count changes. The measurement is the drawn spans.
`spanscore.py` renders each mover through 26.2.4.2 and counts how many of the reference's spans
each of our two binaries reproduces — same page, same text, origin within 0.30 pt — and does it
twice, once ignoring colour and once requiring it.

Over **all 23 documents that state a live text range** (the 20 movers and the three controls):

| | base `6d19fd594` | this round |
|---|---:|---:|
| reference spans placed | **7650** | **7679** |
| reference spans placed *and* coloured | **7592** | **7622** |
| of the reference's | 12079 | 12079 |
| documents better on colour | — | **9** |
| documents worse | — | **5** |
| documents level | — | 9 |

**Net +30 spans; +56 across the nine that improve and −26 across the five that worsen.**
`mover-scores.tsv`. The largest movers are `gillikin_online_user_mtg_2010.ppt` (+21),
`2015-Civil-Rights-Website-training.ppt` (+12) and `0335fab9-…ppt` (+10).

## The residual, and everything established about it

**The five that worsen are one class: 26.2.4.2 declines to make a field of a range whose id *does*
resolve, and I cannot name the rule.** It is document-level — `RESPA_-_Section_8_Webinar.ppt`,
`gillikin_online_user_mtg_2010.ppt`, `iep-amount-frequency-for-webinar.ppt`,
`joint_user_outcomes_michael_fullerton_29.06.12.ppt` and most of
`gfopportunitiesforlinkagespres_2010_en.ppt` make **no** text field, and the other fifteen make all
of theirs. On `RESPA` page 11 the reference draws the URL at `#333e48` (the body's colour, not the
scheme's `#1e6d37`), breaks it at the solidus after `compliance-corner/`, spaces the two lines
24.038 pt apart — the ordinary 1.2 em — and draws **no underline at all**; this tree draws
`#1e6d37`, cell-breaks at `…corner/2` and spills 20.041 pt, which is 1.0 em. Every one of those
four observations says *not a field*.

**It is the identifier, established by a one-attribute variant series and not by reading.**
`patch-id.py` rewrites one `InteractiveInfoAtom`'s `exHyperlinkId` in a copy of `RESPA` and
changes nothing else; each copy was rendered through 26.2.4.2:

| id written | in the live `ExObjList`? | 26.2.4.2 draws |
|---:|---|---|
| 13 | yes, atom 0 | **`#1e6d37`, cell-broken — a field** |
| 15 | yes, atom 1 | **a field** |
| 17, 20, 22, 24 (the file's own), 27, 35 | yes, atoms 2, 3, 4, 5, 6, 9 | `#333e48`, word-broken — not a field |
| 0, 14, 16 | no | not a field |

So the reference behaves as though `m_aHyperList` held **two** entries where the file declares ten.
`patch-range.py` moving the range to `(20, 40)` changes nothing, so it is not the range.

**Four candidate mechanisms are refuted, each measured:**

- *The `_PID_HLINKS` count bounds it.* No — the blob declares 10 links and parses cleanly to 10
  under `PropItem::Read`'s own rules, alignment and terminating NUL included (`hyperlist.py`).
- *The blob's declared size truncates it.* No — `cbSize` is **exactly** the bytes the entries
  consume, on all ten documents checked.
- *`Section::GetDictionary` never finds `_PID_HLINKS`.* No — it is found in all five failing
  documents under LibreOffice's own no-padding, stop-at-empty-name reading
  (`dictionary.py`, `propread.cxx`:262-309).
- *The `ExObjList` is shaped differently.* No — `RESPA` and `Lepore` (which works) have identical
  `ExObjList` structures: an `ExObjListAtom` and then one `ExHyperlink` per link, each holding an
  `ExHyperlinkAtom` and two `CString`s.

**Left with its seat**: `sd/source/filter/ppt/pptin.cxx`:353-547 and
`filter/source/msfilter/svdfppt.cxx`:6907-6909, with `patch-id.py` as the instrument. Whoever takes
it should start by proving the entry count rather than the index mapping — the variant series
already shows the *indices* are right for the first two entries.

**The change is shipped anyway**, and the reason is the rulebook's own: it implements what the
source says, it is a net gain of 30 reference spans with nine documents better and five worse, and
the alternative is a heuristic gate invented to match a reference behaviour nobody has explained.
Reporting the residual is worth more than hiding it behind a guess.

---

# 2. The outline-placeholder autofit — instrumented

The brief asked for one thing first: *print our own scale for that shape beside the reference's
rather than theorise about either.* That is what this section is.

## The reach is not 303 pages over 91 documents

`size-sweep.sh` renders every one of the 51 `.ppt` both ways and records, per page, the text size
carrying the most alphanumeric characters — the same statistic the TODO's census used. Measured on
**1534 pages of 51 documents, none failing on either side**:

| | |
|---|---:|
| pages whose dominant size differs by more than 0.15 pt | **25** |
| documents holding one | **18** |
| the reference is **smaller** (we under-shrink) | 19 pages, 14 documents |
| the reference is **larger** (we over-shrink) | 6 pages, 5 documents |
| of the 25, one `constScaleLevels` step apart | **22** |

`size-census.tsv`. **The TODO's "303 pages over 91 documents" is a whole-slides-track figure over
all 302 slide documents, not the `.ppt` column, and it is stale in any case.** Its two named worst
documents are in this census at **2 pages each**, not 17 and 16.

## The worked case has changed under the record, and the new number is a table row

The TODO says page 2 of `2015-Civil-Rights-Website-training.ppt` is drawn by the reference at
**31.01 pt with an 18.59 pt bullet**, "one scale of 0.9691". Re-measured this round against
26.2.4.2:

| | size | bullet | first four bullet baselines |
|---|---:|---:|---|
| 26.2.4.2 | **29.99** | 17.80 | 182.920 / 258.831 / 334.743 / 410.655 |
| this tree | **32.00** | 19.19 | 184.507 / 261.156 / 368.929 / 445.578 |

`29.99 / 32.00` is not 0.9691, it is `round(32 × 0.925)` — **row 1 of `constScaleLevels` exactly**,
and the stated size really is 32 pt: LibreOffice's own `--convert-to odp` of the deck writes
`Default-outline1`'s first level as `fo:font-size="32pt"` with `style:shrink-to-fit="true"` on the
frame. So the 0.9691 in the record predates the round that replaced 24.2.7.2's bisection with the
table (`slides-r52`) and should not be worked from; **0.9691 is not producible by any row of
`constScaleLevels` under `setRoundFontSizeToPt(true)`** (`svx/source/svdraw/svdotext.cxx`:1231),
which is what should have flagged it as stale.

## Which of the two seats is wrong: `SlideAutofit`, and by 0.32 pt

`SlideTextLayout.Solve` was instrumented to print, per autofitted body, its largest run size, its
unscaled formatted height, the box it is fitted against and the row it answers with. The
instrumentation was built, run over the 18 differing documents, and then reverted — and the revert
is proved by the 51-of-51 byte-identical re-render recorded in *Environment*.

For page 2's body it prints:

```
AUTOFIT  size=32.00  unscaled=14176  avail=12871  row=0  font=1
```

Three things follow, and they answer the brief's question:

- **`PptSlideLayout.Autofits` is right here.** It returns true, the fit runs, and the unscaled
  height (14176 units of 1/100 mm) does overflow the box (12871). We are not failing to shrink
  because we declined to try.
- **`SlideAutofit` is wrong.** It takes row 0 — `{font 1.000, spacing 0.900}`, full size with
  tighter leading — where 26.2.4.2 takes row 1. Row 0's height is `14176 × 0.9 = 12758` against
  `12871` available: **it fits by 113 units, which is 0.32 pt, or 0.875% of the box.**
- **So the defect is in the height measured at row 0, not in the table and not in the decision to
  fit.** It is a sub-one-percent error, which is why it moves so few pages.

The trace over all 18 documents (`autofit-trace.tsv`, 899 autofitted bodies) puts a size on that:

| the fit answers | bodies |
|---|---:|
| no shrink needed | 596 |
| row 0 — full size, spacing 0.900 | **124** |
| row 1 — 0.925 | 137 |
| row 2 — 0.850 | 32 |
| rows 3-9 | 10 |

and **only 6 of the 124 row-0 answers fit by less than 2% of the box height**. The two tightest are
the two documents the size census names most often. So this is not a systematic offset applied
everywhere: it is a small height error that decides the answer on a handful of nearly-full boxes,
and the next round should measure *what row 0's line height should be* — `fSpacingY` in EditEngine
scales the paragraph's upper and lower space and a *stated* proportional line spacing
(`impedit3.cxx`:1555-1600, `impedit2.cxx`:4781-4850), and `SlideTextLayout.Spaced` scales **every
line's height** by it, which is the first thing to check.

## And `Autofits`' wrap test is wrong, with reach the seat's own comment denies

`Autofits` returns `!growsToText && Wraps(shape)`. The C++ derives
`bAutoGrowWidth = !bWordWrap` **only** for a custom shape whose text kind resolved to Rectangle
(`svdfppt.cxx`:1053-1055); every other branch — which is every real Body, HalfBody or QuarterBody
placeholder — sets `bAutoGrowWidth = false` at `:1084` whatever the wrap says, and only
`bAutoGrowHeight = bFitShapeToText` can suppress the fit. So a non-wrapping outline placeholder is
autofitted by the reference and left alone by this tree.

The seat's doc-comment says *"No deck in the slides corpus holds that combination"*. **It does.**
`wrap-census.py` over the live tree of the 51 `.ppt`: of **1401** Body/HalfBody/QuarterBody shapes,
**55 state `wrapNone`, in 2 documents** — `Architecture.ppt` and
`Fundamentals_Module_1_basics.ppt` — and **25** of those also leave `fFitShapeToText` clear, so
they would newly autofit. That is an upper bound: it does not yet test "is a custom shape with no
placeholder atom", which is the one case where the wrap really does decide.

`Fundamentals_Module_1_basics.ppt` is in the size census, at page 6, and it is one of the three
pages there whose ratio is no `constScaleLevels` step at all (7.08 against 32.00). **Left, with
its seat and its census** — it is a second change to the same subsystem and it wants its own
before/after, which this round did not have the budget for.

---

## The suite

Every project run individually, at this round's head.

| project | base `6d19fd594` | this round |
|---|---:|---:|
| Containers | 109 | 109 |
| Core | 519 | 519 |
| Markup | 259 | 259 |
| OpenDocument | 143 | 143 |
| Presentations | 1027 | **1039** |
| Rendering | 164 | 164 |
| Spreadsheets | 1207 | 1207 |
| Text | 728 | 728 |
| Vector | 302 | 302 |
| WordProcessing | 1787 | 1787 |
| **total** | **6245** | **6257** |

0 failed and 0 skipped everywhere; the twelve new ones are `PptHyperlinkFieldTests`.

`XRay.Office.Fidelity.Tests`: **542 passed / 10 failed of 552, 0 skipped** — the briefed baseline
exactly, unchanged.

---

## What this brief got wrong

1. **"17 of the corpus's 51 `.ppt`, 91 atoms" is neither the reach nor an upper bound of it.** It
   was a byte-pattern scan for record type 4063 and it is *low*: the live record tree holds **171**
   such ranges in **23** documents. What it over-counts is documents that make a field —
   **20**, with **111** ranges — because three documents state 60 ranges and declare no hyperlink.
   The brief called its figure an upper bound; it is an upper bound on the wrong quantity.
2. **"23 documents carry any `InteractiveInfo` at all" understates it too.** 23 is the number
   carrying a *text-range* one; shape-level click actions are commoner and are irrelevant to this.
3. **The `.ppt` equivalent of DrawingML's empty property map is not a property of the record.** It
   is whether the deck declares the hyperlink the record names, and that lives in a different
   stream and a different container. A reader that tests the record alone makes 60 fields the
   reference does not.
4. **"31.01 pt … one scale of 0.9691" is stale and cannot be right against 26.2.4.2.** The
   reference draws 29.99 today, which is `round(32 × 0.925)` — `constScaleLevels` row 1 — and
   0.9691 is not producible by any row of that table under `setRoundFontSizeToPt(true)`. The figure
   predates `slides-r52`.
5. **"303 pages over 91 documents" is not a `.ppt` figure.** On the `.ppt` column it is **25 pages
   over 18 documents**, and the brief's two named worst documents contribute two pages each.
6. **"`PptSlideLayout.Autofits` decides whether and `SlideAutofit` decides how much; which is
   wrong is unmeasured."** Measured now: on the worked case `Autofits` is right and `SlideAutofit`
   is wrong, by 0.32 pt on a 12871-unit box. But `Autofits` is *separately* wrong about the wrap,
   with 55 shapes in 2 documents of reach — so the answer is "both, for different reasons, and the
   one the worked case shows is `SlideAutofit`".

## The scripts

| | |
|---|---|
| `persist.py` | `PptPersistDirectory.Read` in Python — the live record tree, without which every census counts orphans |
| `census.py` | text-range hyperlinks over the live tree, with the declared identifiers beside them |
| `stream-scan-census.py` | the **wrong** instrument, kept: a stream scan, which finds 235 ranges and a dead `ExObjList` |
| `ranges.py` | the characters each range covers, with the shape's text around them |
| `hyperlist.py` | `m_aHyperList`, simulating `PropItem::Read`'s alignment and terminator rules |
| `dictionary.py` | whether `Section::GetDictionary` finds `_PID_HLINKS`, by LibreOffice's own reading |
| `pid-hlinks.py` | the first, cruder `_PID_HLINKS` reader, kept beside it |
| `probe-unresolved.py` | what the three field-less documents actually state |
| `patch-id.py` | one `InteractiveInfoAtom`'s `exHyperlinkId`, rewritten in a copy — the variant series |
| `patch-range.py` | one `TxInteractiveInfoAtom`'s character range, likewise |
| `ref-render.sh`, `ref-all.sh` | one document, or a list, through 26.2.4.2 — hex-digest profile and `timeout -k` |
| `baselines.py` | every drawn span of a PDF: baseline, x, size, colour, face, text |
| `spanscore.py` | how many of the reference's spans two renderings reproduce, with and without colour |
| `linkcolour.py`, `linkverdict.py` | per-link: does the reference draw it as a field? |
| `hash-sweep.sh` | a list rendered and digested one document at a time, deleting as it goes |
| `sizes.py`, `size-sweep.sh` | per-page dominant text size, both ways |
| `wrap-census.py` | non-wrapping outline placeholders |
| `paths.py` | corpus paths for a list of names |

And the banked figures: `census.tsv`/`.summary` and `stream-scan-census.tsv` (the two censuses),
`reach.tsv` (84 documents rendered both ways), `mover-scores.tsv` (the 23 scored against
26.2.4.2), `linkverdict.tsv`/`linkcolour.tsv` (the per-link verdicts), `sizes.tsv` and
`size-census.tsv` (1534 pages of dominant sizes), `autofit-trace.tsv` (899 traced fits) and
`wrap-census.txt`.
