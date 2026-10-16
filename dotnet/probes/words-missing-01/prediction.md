# words/missing-001 — prediction, written before the measuring sweeps

Written after the two fixes were implemented and spot-checked on their own documents, and
**before** the 200-document reach sweep, the `words/done-*` regression sweep and the test runs.
Scored honestly in `results.md`; where a spot check preceded this, it is said so below rather
than claimed as a prediction.

## What the brief said and what the disk says

The brief named four documents. `words/missing-001` holds **five** — `MANIFEST.tsv` agrees —
and the two lists differ at both ends:

| document | in brief | measured baseline verdict |
|---|---|---|
| `TE.CAO.00125 … OJT Logbook.docx` | yes | `words` 2675/2793 |
| `xx_SETIS_PWS_template_10.19.22.docx` | yes | `words` 4375/4923 |
| `May 25 bulletin focus on carers in the workplace.docx` | yes | `pages,words` 5/4, 435/521 |
| `33004.docx` | yes | **match** — already closed, as the brief suspected |
| `CRIF - Spécification technique - Socle applicatif.docx` | **no** | `pages,words` 27/29, 6260/6618 |

So the group is four failing documents, but not the four named.

## The footnote corpus census — the brief's open question

The brief asked whether footnotes are a widespread feature gap ("may be the largest single item
on the track", "may reach thirty documents"). Measured over all 200 words documents:

* **13 carry OOXML footnotes.** No `.doc`, `.rtf` or `.odt` document in the track carries one.
* **Footnotes are already implemented and already work.** Nine of the thirteen draw their note
  bodies today, four of them in `done-*` batches that pass the gate.
* The failing four all share one property: **the anchor is inside a table cell.** Exactly three
  documents on the track cite a footnote from inside a table — `TE.CAO.00125`,
  `FO.FCTOA.00010 Application for a Part-ORA ATO Approval.docx` and
  `EHEST-SMS-Safety-Management-Manual-V2.docx` — and those are the three whose note text is in
  the reference's PDF and absent from ours.

So the reach is **3 documents, not 30**, and the item is a narrow paginator gap rather than a
feature gap. Line numbering, the second item, is stated by **1** document in 200 (`w:lnNumType`
appears once). I worked footnotes first anyway, on that 3-vs-1 ordering.

## The two seats

1. **`Paginator`'s table arm never collected the notes its cells cite.** Notes were gathered on
   the `PageParagraph` branch of the top-level flow only; `PlaceTablePart` placed cells and the
   notes inside them were read, numbered and dropped. Fixed by `PlacedNotes`, which walks a
   placed table's cells' flows, plus one step of the room feedback loop the paragraph arm already
   runs.
2. **No margin line numbering at all.** New `LineNumbering`, read from the last
   `w:sectPr/w:lnNumType`, applied as a pass over the finished pages and drawn right-aligned
   0.5 cm in from the text edge at the document's default character size.

Spot checks already run (so *not* predictions): `TE.CAO.00125` 15/15 pages and 2793/2793 words,
exact; `xx_SETIS` 15/15 pages and 4920/4923 words, and its page-11 numbers 364…385 at 10 pt
Liberation Serif on the reference's own baselines.

## Predictions

### P1 — `words/missing-001`

**2 of 5 move to `match`, 3 of 5 match in total** (`33004` was already there). `May 25 bulletin`
and `CRIF` are untouched by both fixes and stay failing with the same verdicts. Confidence: high.

### P2 — the whole words track, scored against the banked 26.2.4.2 references

Baseline measured before any change: **167 / 200 match**. Predicted after: **169 / 200**.

I do *not* expect `FO.FCTOA.00010` or `EHEST-SMS` to flip, even though both gain a footnote:
both fail on `pages`, and one footnote's worth of body text is unlikely to be what decides a
714-page or a 59-page count. Confidence: medium-high on 169; if it is 170 or 171 the extra is
one of those two.

### P3 — reach, measured as documents whose PDF changes byte for byte under a fixed
`SOURCE_DATE_EPOCH`

**Between 3 and 10 of 200.** The floor is the three in-table-footnote documents plus `xx_SETIS`
(four); the range above it is the second half of the footnote change, which now reserves the
note area's height before placing a table. That can move a table part on any page carrying both
a footnote and a table, which is a population I have not counted. Confidence: low on the upper
bound, high on the floor.

**I expect no document to get worse.** If the table-room reservation regresses anything, it will
show as a `done-*` document moving from `match` to `pages`, and the honest response is to drop
the reservation and keep only the collection.

### P4 — `words/done-*`

**157 of 159 match**, with the two known failures the brief names — `Sample_SQMS_Program`
(60/61 pages) and `airbus-pdf-information-package_v1-4` (1272/1299 words). Both were measured
failing on the baseline sweep before any change, so they are not mine. Confidence: high on the
two being present; medium on nothing else moving, for the P3 reason.

### P5 — tests

Both new tests fail against the tree with the fixes reverted and pass with them. The fidelity
baseline is **30 failed of 550**; predicted after: **30, or 28** if the two documents the fixes
close are among the 30. Every other project: 0 failed.

## What I am not doing, and saying so now

`May 25 bulletin` and `CRIF` are **not** attempted. Two seats closed well is the brief's own
preference over four touched, and neither of those two is a bounded change: the bulletin is
missing two graphics *and* a whole trailing section (five pages against four), and `CRIF` is not
diagnosed at all — the brief does not mention it, because the brief did not know it was in the
group.
