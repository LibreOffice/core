# docsection-r184 — a continuous DOC section's own top margin, and its furniture

`words/done-014/doc/PK_FlugzeugeStricken.doc` held round 183's worst page in the words track,
6.46 unsigned `|ink|%` on page 2. It turns out to be two separate things, one ours and one the
reference's.

## The file

Its `PlcfSed` holds **two** sections, CP [0, 738) and [738, 12823). Both state
`sprmSBkc` = **0**, so both are continuous. The rest of the two `SEPX`:

| | page | left | right | top | bottom | header | footer |
|---|---|---:|---:|---:|---:|---:|---:|
| section 0 | 11906 × 16838 | 1417 | 1417 | **1417** | 1134 | 720 | 720 |
| section 1 | 11906 × 16838 | 1418 | 1134 | **1701** | 1134 | 720 | 720 |

The text holds five `0x0C` characters: CP 737 is the section mark, and CP 2936, 4970, 7147 and
9220 are hard page breaks. So the page starts are CP 0, 738, 2936, 4970, 7147, 9220 — page 1 is
the cover, page 2 the *Kurzfassung*, and page 3 begins *"Wirtschaftslandesrat Viktor SIGL"*.

## Ours: section 1's vertical margins were thrown away — fixed

`Ww8SectionTable.ResolveContinuousBreaks` copied the previous section's top and bottom margins
onto every compatible continuous section. That is right for a continuous section whose descriptor
reaches no page, and wrong for one that states furniture of its own and holds a hard page break:
`wwSectionManager::InsertSegments`'s *"nightmare scenario"* arm builds a descriptor for such a
section and hangs it on the first hard break inside it, **carrying the section's own margins with
it**. `ContinuousPageDescriptors.Resolve` already declines to inherit in exactly that case — and
could not put back what the table pass had erased.

Measured by patching the file (`patch-sepx.py`):

| variant | 26.2.4.2, page 3 body top | this tree, before | after |
|---|---:|---:|---:|
| as shipped | 85.08 (= 1701 tw) | **70.88** | **85.08** |
| section 1 `sprmSDyaTop` → 2500 | 125.03, and 7 pages → 11 | 70.88, 7 pages — *unmoved* | — |
| section 0 `sprmSDyaTop` → 2500 | 125.03 | 125.03 | — |

The middle row is the whole proof: changing section 1's top margin moves the reference by exactly
the amount stated and repaginates it, and moved nothing here at all — because the squash had
already replaced 1701 with 1417. The bottom row shows what we were using instead.

Pages 3 to 7's unsigned ink goes **0.10, 0.08, 0.21, 0.36, 0.54 → 0.01, 0.01, 0.04, 0.05, 0.03**,
and page 6 from MAJOR to ok. Swept over all 337 words documents: the gate is unmoved at 329
`match` and **two rows differ**, both improving — this document 8.32 → 7.13 with 3 major pages → 2,
and `150_5300_13_chg12.doc` 7.97 → 7.96.

`foca_form_1.doc`, the document the squash was measured on in round 72, is unchanged at 3 pages
and 0.07 / 0.00 / 0.05 — its second section states no furniture of its own, so
`ContinuousPageDescriptors` drops its margins exactly as the squash used to.

## The reference's: page 2's furniture is a page late — not taken

What is left on page 2 is 6.42, and it is the same rule seen from the other side. Section 1 begins
at CP 738, which is page 2, and states its own running head. 26.2.4.2 defers the whole descriptor
to the first hard page break inside the section — CP 2936, **page 3** — so its pages 1 and 2 both
wear section 0's letterhead (a full-page 575 × 813 image in that section's header, plus the grey
*Impressum* sidebar and the *Rückfragen-Kontakt* block) and pages 3 to 7 the
`SIGL / SCHAFFLER / STOLLBERGER / KASTNER` running head. We put the running head on page 2, which
is where section 1 starts.

Measured: the reference draws the 575 × 813 header image on pages 1 **and** 2 and we draw it on
page 1 only; a blind reading of the composed page-2 pair, given no numbers, reported the grey
banner, the emblem, the sidebar and the contact block as present in the reference and absent from
ours, and the running head and conference footer as present in ours and absent from the reference.

Word shows a section's own header on the first page that is entirely that section's, which page 2
is. So this is `TODO.word-parity.md`'s territory rather than a defect, and it is recorded there.
