# slides-r108 — O15's fourth input is which portion a percentage paragraph space is measured against, and the remaining eleven pages are three distinct causes

Round 108, slides track, one seat: **O15**, the per-page dominant drawn text size over the
51-document binary PowerPoint column.

Two results.

1. **A fourth input to the autofit block height, found, measured and landed.** A `.ppt`
   paragraph whose space above is a *percentage* resolves that percentage against the size of the
   paragraph's **last** portion. This tree read it off the **first**. On
   `gillikin_online_user_mtg_2010.ppt` page 2 the difference is 63 units of a hundredth of a
   millimetre on a block that misses its `constScaleLevels` row by 55, and the whole slide is
   drawn 31 pt where 26.2.4.2 draws 34. **12 → 11 pages, 10 → 9 documents, 58.08 → 55.10 pt of
   summed error, 1 fixed and 0 newly wrong**, with exactly one page of 1534 moving at all.

2. **The remaining eleven are three causes, separated by measurement rather than by argument.**
   Every one of the twelve was put through the same four-leg experiment — the reference reading
   the `.ppt`, the reference reading its own flat ODP of that `.ppt`, this tree reading the
   `.ppt`, and this tree reading that same flat ODP. The twelve split **6 / 3 / 3** — an import
   cause, a layout cause, and a round trip that is not faithful on the page and therefore says
   nothing either way. One of the six imports is the page this round fixed, so the eleven that
   remain are **5 / 3 / 3**. `legs.txt` is that table.

## Environment

| | |
|---|---|
| worktree / branch | `/home/user/wt-slidesize`, `agent/slidesize`, base **`3a926f63f`** |
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**. `/usr/bin/soffice` (24.2.7.2) is used for nothing here |
| reference bank | `/home/user/gate-orig-r83/ref`, the 51 `.ppt` among its 948 PDFs |
| corpus | `/home/user/sample-files`; the slides binary column is **51 `.ppt`** (`ppt.list`) |
| working directory | `/home/user/r108-work` |
| C++ tree | `/home/user/libreoffice-core`, read only, never built. It declares **`27.2.0.0.alpha0+`** and is **not** the reference binary's source |

**Both legs, as this track requires.** The source leg below is a later version's explanation. The
measurement leg is 26.2.4.2's own `--convert-to fodp` of the corpus documents and its own rendered
pages, and it stands on its own — for the mechanism in §2 the flat ODP *states the resolved number
in centimetres*, so the source leg is a corroboration of a figure that was already read off the
reference.

---

# 1. The instrument: four legs, and what each disagreement can and cannot be

Round 103 established two nulls that this round did not re-test and did not need to: the autofit
block arithmetic is exact (84 of 84 bisected block heights agree) and so is the line breaking
(164 of 164 wrap thresholds). What was left had to be an *input* to the block height. Rounds 100
and 107 each found one — a hard line break sizing the line it ends, and an escaped portion
measured twice.

Finding the next one needs an instrument that says **where** a disagreement lives, and round 103's
half-instrument does not: "render the reference's own flat ODP back through the reference" tells
you whether the ODF round trip is faithful, and nothing else. Adding the fourth leg — **render
that same flat ODP through this tree** — makes it decide:

| | |
|---|---|
| `REF.ppt` ≠ `REF.fodp` | the round trip is not faithful there. The page carries **no evidence** either way. |
| `OUR.fodp` = `REF.ppt` | handed the reference's own model, we agree. The disagreement is in **what we read out of the `.ppt`**. |
| `OUR.fodp` ≠ `REF.ppt` | we disagree on that model too. The disagreement is in **how we lay it out**. |

Run over all twelve (`legs.txt`, produced by `rt.sh` and `leg4.sh`):

```
document                                                    pg  REF.ppt  REF.fodp  OUR.base  OUR.head  OUR.fodp  verdict
JesuitAssocOfStudentPersonnel                               24    13.01     13.01     15.99     15.99     15.99  LAYOUT
RRM-training-syllabus-…-Dec-2009                            16    18.99     18.99     20.01     20.01     20.01  LAYOUT
gfopportunitiesforlinkagespres_2010_en                      27    25.99     28.01     28.01     28.01     28.01  ODP-NOT-FAITHFUL
gillikin_online_user_mtg_2010                                2    33.99     33.99     31.01     33.99     33.99  IMPORT  (fixed this round)
ws_prod-…-M.017-(French)-France                             14    18.99     18.99     20.01     20.01     20.01  LAYOUT
ws_prod-…-M.017-(French)-France                             16    14.99     14.99     17.01     17.01      15.0  IMPORT
ws_prod-g-doc-Events-Part-M-presentation                    21    17.01      18.0      18.0      18.0      18.0  ODP-NOT-FAITHFUL
2015-Civil-Rights-Website-training                          22     14.0     14.99     17.01     17.01     17.01  ODP-NOT-FAITHFUL
Fundamentals_Module_1_basics                                 6     32.0      32.0      7.08      7.08      32.0  IMPORT
Thailand17                                                   8    24.01     24.01     16.68     16.68     24.01  IMPORT
Thailand17                                                  11    11.99     11.99     11.0      11.0      11.99  IMPORT
W3_Case_Study_…_Ed                                          10    24.01     24.01     16.68     16.68     24.01  IMPORT
```

**The brief's split was the wrong one, and this is the useful one.** Round 103 divided the
residual by whether the alphanumeric counts match. That division does not survive: of the **six**
same-alphanumeric pages left at head, three are LAYOUT, one is IMPORT and two are
ODP-NOT-FAITHFUL; of the **five** that differ in alphanumerics, four are IMPORT and one is
ODP-NOT-FAITHFUL. Alphanumeric agreement is a property of the *text* and the causes divide by
where in the pipeline the answer is decided — which is why the brief's "four differ in
alphanumeric count, work the identical-text group" put the two halves of one cause on either side
of the line.

**Two cautions this table carries.**

* The `OUR.fodp` column is rendered by this tree's ODF reader, so it is only a control for a
  `.ppt`-only change; a change in the shared layout would move both columns and the instrument
  would go blind. The change in §2 is in `XRay.Office.Presentations/MsBinary` and cannot reach it.
* A page count that differs between the flat ODP and the rendered PDF makes every page index a
  comparison of two different slides. It happens on **6 of the 51** documents (`lnspc.txt` lists
  them); on those six the leg table's own page indices are the *rendered* ones on all four legs,
  which is why the table renders the flat ODP rather than reading it.

---

# 2. O15's fourth input — a percentage paragraph space is measured against the paragraph's last portion

**Fixed in this tree.**

## 2.1 The witness, and what it costs

`gillikin_online_user_mtg_2010.ppt` page 2 is an outline placeholder of five bullets — four set
in 34 pt, the third running 34 pt into a 36 pt phrase and the fifth running 34 pt into a 24 pt
URL — plus one trailing empty paragraph. Its text area is **11 176** units of a hundredth of a
millimetre, so the fit compares against 11 177.

26.2.4.2 draws it at **34 pt with nine-tenths paragraph spacing**, which is `constScaleLevels`
row `{1.000, 0.900}` — a `.ppt` paragraph states its line spacing, so the row's spacing factor
reaches the paragraph gaps and not the line height (`PptStatedLineSpacingTests` is that rule).
This tree drew **31 pt**, one row further down.

Its own drawn geometry says why. Converting the reference's baselines on that page:

| | measured | decomposition |
|---|---:|---|
| in-paragraph pitch (34 pt → 36 pt line) | 42.81 pt | `1.2 × 33.99 + (36.00 − 33.99)` = 42.79 |
| paragraph 1 → 2 | 48.41 pt | `1.2 × 33.99 + 7.63` |
| paragraph 2 → 3 | 48.87 pt | `1.2 × 33.99 + 8.09` |
| paragraph 4 → 5 | 46.15 pt | `1.2 × 33.99 + 5.37` |

The line height is `1.2 em` throughout, as expected. **The three paragraph gaps are not equal**,
and they are not proportional to the paragraph's first run, which is 34 pt in all three. At the
row's 0.9 they are `0.9 × 8.50 = 7.65`, `0.9 × 9.01 = 8.11` and `0.9 × 6.01 = 5.41` — and 8.50,
9.01 and 6.01 pt are 20/80 of **34 pt, 36 pt and 24 pt**, the size of the *last* portion of each
paragraph. (The residual against the measured 7.63 / 8.09 / 5.37 is the reference's own integer
master-unit chain: the three gaps are held as 270, 286 and 191 units of a hundredth of a
millimetre, which are 7.65, 8.11 and 5.41 pt, and what is read back off the page is those numbers
through the PDF writer.)

**And the reference states those three numbers itself.** Its flat ODP of the same file gives the
five bullets `fo:margin-top` of `0.3cm`, `0.3cm`, **`0.318cm`**, `0.3cm` and **`0.212cm`** — 300,
300, 318, 300 and 212 units, which are exactly 20/80 of 34, 34, 36, 34 and 24 pt. That is a
number read out of the reference, not inferred from a rendering.

## 2.2 The mechanism

`PPTParagraphObj::ApplyTo` converts a positive `PPT_ParaAttr_UpperDist` — a percentage — into
master units before it ever becomes an `SvxULSpaceItem`, and the height it multiplies by comes off
the **back** of the portion list (`filter/source/msfilter/svdfppt.cxx`:6296-6306, this tree):

```cpp
if ( ( nUpperDist > 0 ) || ( nLowerDist > 0 ) )
{
    if (!m_PortionList.empty())
    {
        sal_uInt32 nFontHeight = 0;
        m_PortionList.back()->GetAttrib(
                PPT_CharAttr_FontHeight, nFontHeight, nDestinationInstance);
        if ( static_cast<sal_Int16>(nUpperDist) > 0 )
            nUpperDist = - static_cast<sal_Int16>( ( nFontHeight * nUpperDist * 100 ) / 1000 );
        if ( static_cast<sal_Int16>(nLowerDist) > 0 )
            nLowerDist = - static_cast<sal_Int16>( ( nFontHeight * nLowerDist * 100 ) / 1000 );
    }
    bIsHardAttribute = true;
}
```

and the negative it has just made then goes through `convertMasterUnitToMm100`
(`:6311-6324`, `include/tools/UnitConversion.hxx`:43-47). Eighty master units make a point, so the
whole of it is `size × percentage / 80` — and the size is the **last** portion's.

The percentage arm is also the only arm this reaches: a value already at or below zero is an
absolute distance in master units and no portion's size touches it. That is the control the test
asserts.

## 2.3 The census, with the base rate beside it

`ulpct.py` and `ulpct2.py` ask the flat ODPs of all 51 `.ppt` which candidate — the paragraph's
first portion, its last, or its largest — reproduces the `fo:margin-top` the reference exported.
The percentage is not stated in the ODP, so a candidate is admitted when some whole percentage
reproduces the length exactly.

**`ulpct.py` is the version with the free parameter left loose, and it is reported here because
its base rate is the confound.** Allowing any percentage from 1 to 400 admits the *first* portion
for 64.3 % of paragraphs and the *largest* for 68.6 % — so "the last portion fits 98.6 % of them"
would have been almost worthless on its own.

`ulpct2.py` removes the free parameter using the document itself: every paragraph whose runs are
all one size pins the percentage exactly whenever only one whole value reproduces its margin, and
the multi-size paragraphs are then scored against that document's own pinned set. Over all 51
documents (`ulpct2.txt`):

| candidate | admitted | of 178 multi-size paragraphs |
|---|---:|---:|
| the paragraph's **last** portion | **147** | **82.6 %** |
| its first portion | 44 | 24.7 % |
| its largest portion | 61 | 34.3 % |

**26 of the 31 paragraphs the last-portion rule does not explain are one document**, `EG1_dsrc
tech.ppt`, whose text is set at 3.5, 4 and 4.3 pt and where *no* candidate is admitted — i.e. its
pinned percentage set is incomplete rather than the rule being wrong there. Excluding it the rule
holds for 147 of 152, **96.7 %**. Both figures are stated because the exclusion is post hoc.

## 2.4 The measurement of the fix, at the block

A temporary diagnostic in `SlideAutofit.Solve`, gated on `XRAY_OFFICE_FIT_DUMP` and **removed before
the commit**, printed the walk (`witness-gillikin.txt`):

| row | before | after | box |
|---|---:|---:|---:|
| unscaled | 11352 | 11282 | 11177 |
| `{1.000, 0.900}` | **11232** — overflows by 55 | **11168** — fits by 9 | 11177 |
| `{0.925, 0.900}` | 10355 — taken | not reached | |

The three paragraph gaps that move are `300 → 318` and `300 → 212` twice, net −63 at full scale
and −57 after the row's 0.9 — which is the whole of the 55-unit overflow. The reference's own
drawn baselines decompose to 11 169 at that row, one unit from our 11 168 (`ScaledSpace` rounds
190.8 down where the reference's integer master-unit chain gives 191).

## 2.5 The change

One line of `PptTextBody.Paragraph`, with the citation and the ODP measurement beside it:

```csharp
Length size = runs.Count > 0 ? runs[^1].Size : Length.FromPoints(characters.FontHeight);
```

`Distance` — which already implemented `size × value / 80` for the positive arm and master units
for the negative one — is unchanged. Nothing else changed.

## 2.6 The test

`PptParagraphSpaceSizeTests`, six cases. Four are a theory over `(opening, closing)` run sizes:
34→24 must give 6.0 pt, **24→34 must give 8.5 pt**, 34→36 must give 9.0 and 34→34 must give 8.5.
The asymmetry is the assertion — a rule reading the first portion, the largest portion or the
smallest portion fails one of the four. The fifth is the negative-value control, which must not
move. The sixth asserts the property the corpus moved on: the same two bodies in the same box take
different `constScaleLevels` rows once the gap is the only thing between them.

**Measured at the base rather than asserted**: with the one line reverted and the test file left in
place, **4 of the 6 fail and 2 pass** — the two that pass are the negative-value control and the
34→34 row, both of which are the same under either reading, which is exactly why they are in the
theory.

## 2.7 What it costs the column

`sizesweep.sh` over the 51, scored with `slides-r107/sizescore.py` against the banked reference
table (`size-summary.txt`, `sizes-head.tsv`):

| | base = `3a926f63f` | head |
|---|---:|---:|
| pages differing by more than 0.15 pt | **12** | **11** |
| documents holding one | 10 | **9** |
| total \|size error\| over 1534 pages | **58.08** | **55.10** |
| of the differing pages, same alphanumeric count | 7 | 6 |
| **fixed / newly wrong** | — | **1 / 0** |
| pages whose dominant size moved at all | — | **1 of 1534** |

**The base column is round 107's committed `sizes-head.tsv` and it is validated rather than
assumed**: 1533 of its 1534 rows are reproduced byte for byte by this round's own sweep of the
same tree at head, the one exception being the page the change fixes. A base column that had been
measured on a different tree would not do that.

## 2.8 What it costs the column on ink

`inksweep.sh` renders all 51 at both legs and compares each against the banked reference with
`pdf-image-diff.py`; `rescore.sh` then re-scores every document whose bytes moved over **every**
differing page rather than only the MAJOR ones. Both tables are in `ink.txt`.

**Twenty-one of the 51 renderings move and thirty are byte-identical.** Over the twenty-one:

| | base | head |
|---|---:|---:|
| sum \|ink\|% over every differing page | **114.95** | **113.08** |
| MAJOR pages | 32 | **31** |
| improve / worsen / level | — | **9 / 4 / 8** |
| page counts differing | 0 | 0 |
| alphanumeric counts differing | 0 | 0 |

**No page count and no alphanumeric count moves anywhere in the column**, which is expected and
is the reason the gate cannot see this change: a paragraph gap adds no character and no page. The
four that worsen do so by 0.02, 0.04, 0.06 and 0.08 of a percent.

The largest single movement is not the witness. `gfopportunitiesforlinkagespres_2010_en` goes
**3.92 → 2.91** and loses its only MAJOR page: its page 27 was 1.18 \|ink\|% over eleven regions
and is 0.59 over ten, and the size row for that page does **not** move — it is 28.01 against the
reference's 25.99 at both legs, one of the three ODP-NOT-FAITHFUL pages. So the correct paragraph
gap improved a page whose remaining defect is something else. `gillikin` itself is 6.83 → 6.14.

**One caution about the instrument, which cost this round a wrong table.** `pdf-image-diff.py
--quiet` prints only the pages it calls MAJOR. A sweep that sums its output is therefore summing
over MAJOR pages alone — 52.09 → 50.91 here — which is not the whole-column figure earlier rounds
quote and is not comparable to it. Table 1 of `ink.txt` is that narrower statistic, labelled as
such; table 2 is the one above.


---

# 3. The other eleven, named

## 3.1 IMPORT — five pages, and they are not one thing

`Thailand17` 8 and 11, `W3_Case_Study…` 10, `Fundamentals_Module_1_basics` 6 and
`ws_prod-…-M.017-(French)-France` 16 all agree with the reference once handed its own ODP and
disagree reading the `.ppt`. They are not one cause:

* **`Thailand17` 8 and `W3_Case_Study…` 10 are the same slide in two decks**, and the size row is
  not the defect: the reference draws **92** alphanumerics on that page and we draw **500**. We are
  putting text on the page that the reference does not draw at all, and the dominant size follows
  from that. Reading the flat ODP we draw 46 at the reference's 24.01 — still not 92, so the ODF
  leg does not draw the same characters either. This is a *text-presence* question and does not
  belong to the fit.
* **`Fundamentals_Module_1_basics` 6**: 44 alphanumerics against our 87, and we draw 7.08 pt
  against 32. Same shape of problem, an order of magnitude worse.
* **`Thailand17` 11** is the other way round — 548 alphanumerics against our 358, so here the
  `.ppt` leg draws *less* than the reference and the flat ODP leg draws all 548 at 11.99, the
  reference's size. Not obviously the same cause as page 8 of the same deck, and not established
  either way.
* **`ws_prod-…-M.017-(French)-France` 16 is a bullet the reference does not draw.** Reading the
  `.ppt` we give the first four paragraphs a bullet — an `OpenSymbol` glyph at **25.795 pt** beside
  17.008 pt text — and indent them behind it; 26.2.4.2 draws no bullet on those four and starts its
  text at the margin. The oversized bullet is what inflates the lines that push the body a row
  down. Reading the same flat ODP we draw no bullet there and answer 15.0 against its 14.99.

## 3.2 LAYOUT — three pages, and all three state a proportional line height below 100 %

`JesuitAssocOfStudentPersonnel` 24, `RRM-training-syllabus-…` 16 and
`ws_prod-…-M.017-(French)-France` 14 disagree with the reference **on the reference's own ODF
model**, which is a different seat from everything rounds 100, 103, 107 and this one have fixed —
those were all inputs, this is the arithmetic over them.

What the three have in common, read out of the reference's own flat ODP: every one of them states
`fo:line-height` at 80 % or 90 % on the body that carries the page's dominant text. Neither of the
two ODP-NOT-FAITHFUL pages does, and only one of the four IMPORT pages does.

**With the base rate beside it, as this track requires.** `lnspc.py` counts, over the 45 documents
whose flat ODP and rendered PDF have the same page count, how many of their 1316 pages state a
sub-100 % `fo:line-height` at all: **353, a 26.8 % base rate**. Three of three at a 26.8 % null is
a one-in-fifty coincidence, which is *suggestive and not established* — three pages is a small
sample and this round did not go on to isolate the term. What can be said without hedging is that
the three are a **layout** cause and the other eight are not, because that is what the fourth leg
measures directly.

The direction is also not the same as §2's. On `JesuitAssocOfStudentPersonnel` 24 the body states
20 pt with `fo:line-height="80%"` and seven interior empty paragraphs; the reference takes row
`{0.625, 0.800}` and draws 13.01 pt while we take `{0.775, 0.800}` and draw 15.99 — **our** block
is the short one there, where on gillikin ours was the tall one.

## 3.3 ODP-NOT-FAITHFUL — three pages that carry no evidence

`gfopportunitiesforlinkagespres_2010_en` 27, `ws_prod-g-doc-Events-Part-M-presentation` 21 and
`2015-Civil-Rights-Website-training` 22. On each, 26.2.4.2 reading its own flat ODP of the file
answers something different from 26.2.4.2 reading the file — so the round trip loses whatever
decides the page and the four-leg instrument is blind there.

Worth recording: on all three, **the reference's own ODP answer is ours**
(28.01/28.01, 18.0/18.0, and 14.99 against our 17.01 — two of three exactly). So on two of them
what we read matches what the reference *writes out* and not what it *draws*: the deciding term
survives in the reference's model and does not survive its ODF export. That is a hint about where
to look and it is not a measurement of anything.

---

# 4. The suite

Read out of this run's own output (`tests.log`), not from the briefed number.
`dotnet build XRay.Office.slnx -c Release` at **0 warnings, 0 errors**, then each project alone with
`--no-build` — individually rather than as a solution, because a whole-solution run on this box has
twice lost a project to an out-of-memory kill and reported itself as a pass.

| project | |
|---|---|
| Containers | 109 / 109 |
| Core | 560 / 560 |
| Markup | 259 / 259 |
| OpenDocument | 160 / 160 |
| Rendering | 164 / 164 |
| Spreadsheets | 1315 / 1315 |
| Text | 728 / 728 |
| Vector | 309 / 309 |
| WordProcessing | 1938 / 1938 |
| **Presentations** | **1107 / 1107** |
| **Fidelity** | **542 passed / 10 failed of 552, 0 skipped** |

The ten are exactly the briefed baseline and nothing else — `PageDrawingComparisonTests` ×4
(`paginated.doc/.docx/.fodt/.rtf`), `TabStopComparisonTests` ×4
(`list-label-overrun.doc/.docx/.fodt/.odt`), `SheetDrawingComparisonTests` ×1
(`sheet-rich-text.xlsx`) and `JustificationShrinkComparisonTests` ×1 (`justify-shrink-2013.docx`).
**No eleventh.** Every project reported its full total and none was aborted.

Presentations' 1107 includes the six new `PptParagraphSpaceSizeTests`. The base total was not
re-run this round, so 1101 is arithmetic rather than a measurement and is not quoted as one.

---

# 5. What could not be settled

* **The three LAYOUT pages.** Named, separated from the rest by direct measurement, and correlated
  with a sub-100 % proportional line height at a 26.8 % null on a sample of three. Not isolated.
  The next round has a decisive instrument for them that costs nothing: they reproduce from a flat
  ODP, so a single-variable series on the ODP itself — the same method round 107 used on the
  escapement — will separate the proportional line height from the interior empty paragraphs
  without touching the `.ppt` reader at all.
* **The five IMPORT pages** are at least three separate questions, and three of them
  (`Thailand17` 8, `W3_Case_Study` 10 and `Fundamentals_Module_1_basics` 6) are *not size
  questions*: we draw two to five times the reference's alphanumerics on them. `Thailand17` 11
  draws two thirds of them. All four should leave the O15 statistic and become their own seat;
  only `ws_prod-…-France` 16, the oversized bullet, is a fit question.
* **The 31 paragraphs `ulpct2.py` does not explain**, 26 of them in `EG1_dsrc tech.ppt`, where no
  candidate is admitted. The likeliest reading is that the document's pinned percentage set is
  incomplete — its text is 3.5 to 4.3 pt, where the master-unit truncation makes several
  percentages collide — but that was not checked.
* **The source leg is a later version.** Every `file:line` above is `/home/user/libreoffice-core`
  at `27.2.0.0.alpha0+`. §2's measurement leg is not: the three margins in centimetres are
  26.2.4.2's own export of the witness, and the sweeps are against its banked renderings.

---

# 6. The files

| | |
|---|---|
| `legs.txt` | the four-leg table: each of O15's twelve pages at `REF.ppt`, `REF.fodp`, `OUR.base`, `OUR.head`, `OUR.fodp` |
| `rt.sh`, `leg4.sh` | build those four legs for one document |
| `fodpall.sh` | flatten all 51 `.ppt` to ODP with 26.2.4.2 |
| `spans.py` | every drawn text span of one page — size, baseline, face, text — read out of the content stream |
| `witness-gillikin.txt` | the shrink-to-fit walk before and after, and the reference's own baselines on that page |
| `ulpct.py` | the census with the free percentage left loose — reported for its base rate, which is why it is not the evidence |
| `ulpct2.py`, `ulpct2.txt` | the same census with the percentage pinned by each document's own single-size paragraphs |
| `lnspc.py`, `lnspc.txt` | how many pages state a sub-100 % `fo:line-height`, and which of the eleven do |
| `sizesweep.sh`, `sizes-head.tsv`, `size-summary.txt` | O15's statistic at head, scored against `slides-r107/sizes-ref.tsv` |
| `inksweep.sh`, `ink.txt` | both legs of the 51 against the banked renderings: pages, alphanumerics, summed \|ink\|% and MAJOR pages, and which renderings moved at all |
| `ppt.list` | the 51 documents |
