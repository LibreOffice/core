# Gate r114 — 915 match, and the one verdict that moved is not ours

`TOTAL 947  MATCH 915  MISMATCH 32  REF-CANNOT-RENDER 0`, against r112's 916/31.

## The regression is the volatile-date confound, and our side did not move

`066_Agile_Gantt_chart.xlsx`, `match` → `words`:

| | ours | reference | band `max(2 % ref, 15)` | \|Δ\| | verdict |
|---|---:|---:|---:|---:|---|
| r112 | 3021 | 3062 | 61.24 | 41 | match |
| r114 | **3021** | **3086** | 61.72 | **65** | words |

**Our count is identical.** The reference moved 24 characters, and that alone crossed the band.

The two gates straddled a **UTC date rollover** — r112 at 2026-09-12 21:26, r114 at 2026-09-13
02:54 — and `066` holds **21 `TODAY()`**. That is the sixth confound, whose register entry already
says *"never attribute a delta on such a row to a patch without re-measuring both sides the same
day."* This is the first time it has been observed **flipping a gate verdict** rather than only
moving a count, so the entry is now stronger than it was.

Base rate: **32 of 243** zip spreadsheets in the corpus hold `TODAY()` or `NOW()`
(`volatile-xlsx.txt`); `.xls` is not scanned by that method, so 32 is a floor.

Ten of the twelve movers checked hold one: `083_Project_tracker` 36, `047_Date_tracker` 27,
`066_Agile_Gantt` 21, `065_Weight_loss` 20, `035_Project_plan` 20, `036_Simple_to-do` 18,
`055_Project_timeline` 13, `075_Idea_planner` 5, `085_Simple_Gantt` 3, `062_Run_chart` 3.

## What did move because of round 113

The two documents in that list holding **no** volatile call are exactly the two the chart round
predicted, which is the cleanest confirmation available:

| document | ours r112 → r114 | reference | outcome |
|---|---|---|---|
| `027_Simple_personal_cash_flow` | 7932 → **7949** | 7949 unchanged | **exact** |
| `026_Monthly_cash_flow` | 7864 → **7879** | 7852 unchanged | verdict unchanged |

And `055_Project_timeline` moved as O50 said it would and for the reason O50 gave: ours
**960 → 934** against an unchanged reference 1045, so its shortfall widens from 85 to 111 and it
stays a `words` mismatch. That is the stated cost of a correct change, not a surprise.

## What this says about comparing two gates

A gate diff is only sound for the documents whose inputs did not change under it. Across a date
boundary, **at least 32 of 947 rows are not comparable at all**, and one of them changed verdict
on a reference-only movement. Two consequences, both cheap:

- **Run both legs of a comparison on the same UTC day**, or re-render the reference for any row
  that moved before reading anything into it.
- **A verdict change is not a regression until our own column is shown to have moved.** Here it
  had not, and the headline 916 → 915 would have been reported as a regression by anyone reading
  the totals alone.
