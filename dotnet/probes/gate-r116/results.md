# Gate r116 — the border change moved 120 renderings and not one gate row

`TOTAL 947  MATCH 915  MISMATCH 32  REF-CANNOT-RENDER 0`, against r114's identical 915/32.
**0 verdicts changed.** Both gates ran on 2026-09-13, so C13 does not apply to this diff.

Round 115's border work changed **120 of 307** sheets renderings. The gate saw none of it, which
is exactly what that round predicted and stated: a stroke weight moves no page count and no
alphanumeric count, so it cannot reach a gate column. This is the confirmation.

## Every row that moved is the reference's own

Four rows differ, and **our column is unchanged on all four**:

| document | ours | reference r114 → r116 |
|---|---|---|
| `047_Date_tracker_Gantt_chart` | 3058 | 3367 → **3441** |
| `SIL_TDB648` | 30893 | 30899 → **30896** |
| `PBN Matrix NAAs (V01)` | 28329 | 28331 (token columns moved) |
| `fm-provider-service-measures` | 134559 | 134559 (token columns moved) |

## This sharpens C11: `047` is an oscillator, not a date victim

When `047` moved by 74 characters across the r112→r114 pair, that pair crossed a UTC date boundary
and the workbook holds 27 `TODAY()`, so the volatile-date confound was the obvious suspect — and
was ruled out at the time only because both of those runs were the same day. It has now moved
**back**, 3441 → 3367 → 3441, on a pair that is unambiguously the same day.

So `047` flips between exactly two values, like `SIL_TDB648` at 30896 ↔ 30899. Its 27 `TODAY()`
calls are real and would matter across a date boundary, but they are **not** what makes it move
here. Two of the five known C11 documents are now established as two-state oscillators rather than
drifters, which is a more useful thing to know: a single re-render of the reference settles which
state you are in.
