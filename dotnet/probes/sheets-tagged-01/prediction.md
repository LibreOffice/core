# sheets-tagged-01 — prediction, written before any measurement

Round of 2026-08-14 on branch `wt-tagged-audit`. Follows `sheets-overflow-01`, which established
that `soffice --convert-to pdf` exports a **tagged** PDF by default and that Calc's
`ScOutputData::LayoutStrings` paints differently when it does — one site, `output2.cxx:1542`.

`grep -n bTaggedPDF sc/source/ui/view/output2.cxx` lists eleven hits. This round audits all of
them, plus the sites the same grep finds elsewhere in `sc/`.

The tree read here is **27.2-alpha**; the binary measured is **26.2.4.2**. Per the house rule the
tree gives the mechanism and only the binary gives the behaviour. Eight predictions in this
project have burned on that gap, one of them mine-by-inheritance in the previous round
(`DrawEdit`'s ungated loop). So the predictions below are split into *source* predictions, which
I expect to be right about mechanism, and *behaviour* predictions, which are what actually get
scored.

## Source predictions (what the tree says)

**S1.** Of the eleven `output2.cxx` hits, **five are not branches at all**: 1498 and 4506 are the
two definitions, 1599 is a function parameter declaration, 1573 is that parameter being passed.
They cannot be classified as paint-or-tag because they are not conditionals.

**S2.** Of the six remaining conditionals, **five are pure structure-tag emission** — 1499/1579
open and close the `Table` element around the whole of `LayoutStrings`; 1951 emits `TR`/`TD` for a
cell handed off to `DrawEdit`; 2182/2269 open and close `TR`/`TD`/`P` around the `DrawTextArray`
call; 4551 reopens a `TD` whose only effect is the matching `EndStructureElement` at 4695. None of
them touches a position, a string, a clip rectangle, or a decision to draw.

**S3.** Site **1542 is the only one that affects paint**, and it is already fixed.

**S4.** The previous round recorded `DrawEdit`'s `for (SCCOL nX=0; nX<=mnX2; nX++)` loop as
"ungated, so the source predicted rich-text cells would still repeat, and the binary refuted it".
I predict the refutation was not a version-gap surprise at all but a second-order consequence of
1542: `bEditEngine` is set at `output2.cxx:1947` *inside* `LayoutStringsImpl`, so a column the
tagged `LayoutStrings` loop never visits is never flagged, and `DrawEdit`'s ungated loop then finds
nothing to draw there. If so the two measurements agree and nothing was refuted.

**S5.** `DrawBackground` (`output.cxx:1069`) has a `bTaggedPDF` and it wraps a
`NonStructElement` only. Its column loop, `for (SCCOL nX=mnX1; nX + nMergedCols <= mnX2 + 1; nX
+= nOldMerged)`, starts at `mnX1` unconditionally and its inner merge walk breaks at
`nCol > mnX2+2`. So the brief's separate merged-cell-background clip finding is **bounded by the
column block in Calc too, tagged or not** — it does not share a seat with anything in this audit.

## Behaviour predictions (what the binary will do — these are the scored ones)

**B1.** Rendering the whole sheets track through the same installed 26.2.4.2 twice, changing only
`UseTaggedPDF`, and diffing the **rasterised ink** rather than the text layer, every page that
differs will be explained by the leftward-overflow rule of site 1542 — a run anchored left of the
column block, painted untagged and absent tagged. **No other class of ink difference will
appear.** This single experiment measures all eleven sites at once on real documents, which is
why it is worth more than eleven readings of the tree.

**B2.** **0 of 171 page counts** will differ between tagged and untagged. Pagination is upstream
of painting and the previous round already showed our own fix moved no page count.

**B3.** Ink will differ on **30–60 of 171** documents. The previous round moved 31 word counts;
ink is the more sensitive instrument, so I expect at least that many and allow for documents where
the lead-in run was clipped to a sliver too narrow to change a word count.

**B4.** Every ink difference will be **untagged having strictly more ink than tagged**, never
less, and confined to the **left edge** of the printed block. The tagged branch only ever *removes*
a column from the loop.

**B5.** No site other than 1542 will need a change to our code, so **this round will fix nothing**
in `dotnet/src/`. I am recording that as a prediction rather than a hope precisely because the
temptation to manufacture a fix is what would make the round worthless.

## Where I expect to be wrong

B3's range is the weakest number here — the previous round's equivalent prediction was 8–20 and
the answer was 31, so my ranges on this quantity have been too narrow once already. B1 is the
prediction that matters and the one I would most like to be wrong, because a second paint-affecting
site is the finding this round exists to look for.

## Fidelity baseline

Established before any change: **30 failed of 550**, per the brief. Recorded in `results.md`.
