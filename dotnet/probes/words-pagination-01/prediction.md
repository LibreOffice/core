# words/pagination-001 — prediction, written before measuring

Committed before any sweep of the fixed binary. Scored honestly in `results.md`.

## Baseline, measured on the unfixed tree

`batch-check.sh … 'words/pagination-001' … 4` — TOTAL 10, MATCH 0, MISMATCH 10,
REF-CANNOT-RENDER 0. Every one of the ten fails on **pages alone**: words are inside the
max(2%, 3) band on all ten and `unemb` is 0 on all ten.

| document | ours/ref | Δ |
|---|---:|---:|
| `doc/1447.doc` | 3/4 | −1 |
| `doc/A_320.doc` | 141/118 | **+23** |
| `doc/absrc-pac-01-info-note-en.doc` | 6/7 | −1 |
| `docx/24-25_FAA_Holdover_Tables.docx` | 142/155 | −13 |
| `docx/ESPN-R - MCF - Manual - Ed1.0 - For Publication.docx` | 34/35 | −1 |
| `docx/ESPN-R - MCF - RA - Ed1.docx` | 59/58 | +1 |
| `docx/FAA 2025-26 Holdover Tables.docx` | 154/167 | −13 |
| `docx/FO.FCTOA.00010 Application for a Part-ORA ATO Approval.docx` | 15/16 | −1 |
| `docx/report-template.docx` | 19/20 | −1 |
| `docx/template---tpr-technical-progress-report-with-guidance.docx` | 8/7 | +1 |

## The brief does not describe this group

Stated plainly because it changes what "the two pairs" means. The brief names
`150-5370-10H.docx`, `AC-150-5370-10G-updated-201604.docx`,
`EHEST-SMS-Safety-Management-Manual-V2.docx`, `150_5300_13_chg12.doc` and
`docs-quality-MA.IMS.00001-…` as members. **All five are in `words/pagination-002`**, not here,
on disk and in `MANIFEST.tsv` alike. Conversely the brief says `1447.doc` is in
`words/ceiling-001` and is not my group's business; it is in `words/pagination-001`, and it is.

So of the two "pairs the brief promises, only the Holdover pair is in this group. The second
pair — the FAA AC template — is another agent's. `A_320.doc` at **+23** is the group's largest
gap and the brief does not mention it at all.

## The one fix in hand

`WordStyles.CompleteOneSidedSpacing` fills the unstated half of a `w:spacing` from Writer's pool
row for the **parent** style's `w:name`, and gives nought when Writer has no style of that name.
Measured on the installed 26.2.4.2 (`one-sided-spacing-source.py`, sixteen variants plus a
fifteen-name sweep, reading `fo:margin-bottom` out of `--convert-to fodt`), that last clause is
wrong: when the *style itself* is one of Writer's headings it is found in the pool rather than
created, and reads its `Heading` base's 12 pt above / 6 pt below.

The change is **additive** — it fires only where the old reading gave nought. Twelve
observations fit it and three of them refute the old one; "mirror the stated value" is refuted
by a 480 twip control that never appears in an answer.

A census of all 134 corpus `.docx` says exactly **six** documents reach this code path, and the
new clause changes **four** of them.

## Predictions

**P1 — the Holdover pair goes page-exact, and it is one bug for two documents.**
Both have `Heading4` (`w:name="heading 4"`) based on a custom `Notes/Cautions Heading` declared
182 styles later, stating only `w:before="120"`; both use it 214 times, for NOTES *and* for
CAUTIONS. LibreOffice resolves 6 pt below where we resolve nought, so we gain 12 pt per table
page. Predict 142 → 155 and 154 → 167, i.e. **both match**.
*Confidence: moderate.* The mechanism and its size are measured, but 12 pt per page only pays
out as a page where the reference's trailing CAUTIONS bullet was already at the edge. Landing
within ±2 pages without matching is the likely near miss.

**P2 — no `done-*` regression from this change.**
Three passing documents reach the code path. Two do not move at all, because their parent is
`Body Text`, which Writer *does* have a pool row for, so the new clause never fires:
`Press release_EUREKA labels ITEA 3 Cluster.docx` (done-007) and the `heading 2` of
`03_Technical_Report_(progress)_template.docx` (done-015). The two that do move:
- `03_Technical_Report`'s `heading 1` over a `Body Text 2` — 7 uses, 6 pt below each, +42 pt.
  LibreOffice's own import of that file resolves it to 120 twips, so this is a **correction**.
- `PES-Technical-Report-Template_Jan_2019.docx` (done-016) `Heading9` over a `List Paragraph` —
  **27 uses, 12 pt above each, +324 pt**, about four pages of shift on a document that passes.
*Confidence: low on the second.* I could not read that style back out of LibreOffice's own
import — it carries `w:aliases="Bullet List"` and no `Heading 9` appears in the flat XML at all,
so the one check that would have settled it did not resolve. **This is the prediction most
likely to be wrong, and if `done-016` regresses this is why.**

**P3 — the other eight in this group do not move.** None of them reaches this code path.
So the group goes 0/10 → **2/10**, and the honest reading of the round is two seats, not ten.

**P4 — reach across the 200 `words` documents is 4 changed, 2 improved.**
Estimated from what the census RESOLVES — six documents whose styles.xml actually takes the
branch, minus the two whose parent has a pool row. `EHEST-SMS-Safety-Management-Manual-V2.docx`
is the sixth; it is in `pagination-002` and I predict it improves without predicting by how much.
Nothing else in `words` can change, because nothing else reaches the code.

**P5 — two defects the blind reviewer found stay unfixed, and neither is a page-count defect.**
A reviewer that had seen no numbers reported (a) our NOTES list numbering runs away — `1` then
`12–21` on page 20, `1` then `196–205` on page 40, against the reference's `1`–`11`, confirmed
in the text layer rather than the raster; and (b) our content overflows the footer, an orphaned
table header block crossing the footer rule and striking through "Page 73 of 87". Neither is
touched by this change. (a) does not move the gate because the labels are the same *count* of
tokens; I predict both survive the fix, visibly.

**P6 — the fidelity baseline of 30 failed of 550 does not get worse.**
`Sample_SQMS_Program.docx` and `airbus-pdf-information-package_v1-4.docx` in `done-015` are
another agent's and are expected to fail; they are excluded from this claim.

## What I am not attempting, and saying so now

`A_320.doc` (+23), `1447.doc`, `absrc-pac-01-info-note-en.doc`, `report-template.docx`,
`FO.FCTOA.00010`, `template---tpr-…` — six of the ten. `1447.doc` in particular the brief
identifies as the line-height law, which is a specification for a future round and not a
constant to fit.
