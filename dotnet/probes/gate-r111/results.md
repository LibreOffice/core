# Gate r111 — full corpus at the round-110 merged HEAD

`TOTAL 947  MATCH 916  MISMATCH 31  REF-CANNOT-RENDER 0`

Both legs fresh, nothing else running. `XRAY_OFFICE_CLI` a Release publish of the merged tree,
`REF_SOFFICE=/opt/libreoffice26.2/program/soffice` (26.2.4.2,
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`), `RENDER_TIMEOUT=300`, 3 workers.

## Against `gate-r109` (the pre-round-110 baseline)

Identical headline, and row by row: **0 verdicts changed**, in either direction. That is what
the three measuring rounds predicted — `chart-inner-r110` (42 movers of 947),
`slides-size2-r110` (26 of 604) and `chart-collide-r110` (0 of 176) each reported every mover
keeping both its page count and its alphanumeric count.

**Ten rows moved without changing verdict.** Seven are this tree moving toward the reference:

| document | r109 | r111 |
|---|---|---|
| `033_Event_planning_tracker` | fonts **4/5**, words 573/574 | fonts **5/5**, words 574/574 |
| `039_Baby_growth_tracker` | 189/190 | **190/190** |
| `bitesize-writing-a-report` | 656/658 | **658/658** |
| `027_Unit_Circle_Chart_Graphical_Chart` | 379/378 | **378/378** |
| `029_Annual_budget` | 309/312 | 310/312 |
| `046_Cost_analysis_with_Pareto_chart` | 150/157 | 165/157 |
| `pie-chart-result.docx` | 40/40 | 39/40 |

`033`'s font column is the pivot `dxf` work reaching the gate: the fifth face the reference
uses is now used here too. `046` and `pie-chart-result` move only in the token columns, which
decide nothing; every one of the ten is unchanged in column 9 except where noted below.

## Three of the ten moved on the *reference* side, and that is a finding

Same binary, same corpus, same timeout — the reference leg should be identical between the two
gates. On three rows it was not, and the PDFs themselves differ, two of them in byte size.
Censused over all 947: see `ref-reproducibility.tsv` and the section this added to
`.claude/skills/corpus-batches/SKILL.md`.

**Pair 1 (r109→r111): 943 of 947 reproduce their extracted text exactly; 4 do not, all `.xlsx`;
2 differ in the alphanumeric count, by 2 and 3.**

- `alle einzeln` wrote `Janßen,` in one run and `Janssen,` in the other, twice — exactly its
  +2. A font resolving differently between runs, the same class as the five banked tarball
  font confounds.
- `SIL_TDB648` produced a PDF about a kilobyte smaller, with four labels reordered and two
  lines joined. Content-stream ordering, not extraction.
- `PBN Matrix NAAs (V01)` and `ans_mappings_of_eccairs_terms` differ in their text layer with
  **no** change to the character count.

## Corrected after gate r112: the roster is not fixed and the movement is not small

A second census over the next pair (r111→r112, `ref-reproducibility-pair2.tsv`) found **4 of 947
again — and not the same four.** Union over both pairs is **five**. `alle einzeln` was stable in
pair 2; `047_Date_tracker_Gantt_chart`, stable in pair 1, moved by **+74** with the PDF itself
105167 against 105422 bytes. `SIL_TDB648` oscillates between exactly two counts, 30896 ↔ 30899.

**And the claim first made here — that no verdict was at risk — is not general.** `047`'s 74
exceeds its own gate band: the test is `d > ref*0.02 && d > 15`, which on a reference of
3367–3441 characters is 67.3–68.8, so the reference's own run-to-run movement is wider than the
band the verdict is decided in. Nothing changed verdict only because that document already fails
on pages (5/8). It carries 27 `TODAY()`, but both runs were the same UTC day, so the volatile-date
confound does not explain it.

What survives unchanged is the conclusion this does *not* support: at least 942 of 947 reproduce
their character count exactly in any given pair, so a one-character delta on an ordinary document
is real and a corpus-wide noise floor would discard findings. What replaces "check these four" is
a procedure — **a per-document delta is trustworthy only once the reference has been rendered
twice for that document**, and a whole-track confinement is much safer than a single-document
claim because a handful of unstable documents cannot move a track total far.
