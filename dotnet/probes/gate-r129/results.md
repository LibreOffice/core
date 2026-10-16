# Gate r129 — and `alle einzeln` is not a two-state oscillator, which I had banked wrongly

Full-corpus gate at HEAD `e1b05c875`, both legs fresh, reference `/opt/libreoffice26.2/program/soffice`
— **26.2.4.2**, `0229ac93fcf0d7cbc6376066c6f35021cef002dc`.

```
TOTAL 947  MATCH 915  MISMATCH 32  REF-CANNOT-RENDER 0
```

Unchanged from r114, r116, r119, r122 and r125.

## The diff against r125

| | r125 -> r129 |
|---|---|
| verdict changes | **0** |
| page changes | **0** |
| character changes | **1** |
| C13 volatile rows moved | **0** |

Between the two gates the tree took the round-126, 127 and 128 merges — the tab decoration, the
TOC field's character style, the Escher adjustment and trapezoid mirror, and the shaped-picture
clip. Round 126 measured 49 of 338 words renderings moving with 0 page counts and 0 character
counts; round 128 moved 4 of 35 candidate documents with 31 bit-identical. **No gate column
moved on any of them.**

## The one changed row refutes something this project had banked, including by me

`sheets/done-013/xlsx/alle einzeln.xlsx`: **278869/278866 -> 278869/278870**. Our column is
identical, as it has been across r119, r122, r125 and r129. The reference moved — to a value
**neither previously recorded state**.

Gate r122's write-up called this document a two-state oscillator and said it had "named both of
its states from two fresh reference renders". **That was wrong**, and this gate is what exposed
it. Six fresh renders on one UTC day (`alle-einzeln-ref-run-1/5/6.txt`):

| run | alphanumeric | `Meß` | `Mess` | `HYP` | `Hyp` |
|---|---:|---:|---:|---:|---:|
| 1, 3 | **278868** | 4 | 6 | 37 | 27 |
| 2, 4 | **278870** | 4 | 6 | 29 | 35 |
| 5 | **278866** | 6 | 4 | 37 | 27 |
| 6 | **278870** | 4 | 6 | 37 | 27 |

**Runs 1 and 6 agree on every one of those four counts and still differ by 2 characters**, which
is what proves the earlier account incomplete. Diffing them token by token finds the missing
site: `Janßen,` / `Janssen,`, twice.

So there are **two independent ß/ss sites**, not one — `Janßen`/`Janssen`, which C11's original
text named, and `Meß`/`Mess`, which gate r122 found and silently treated as the same site. Each
site carries two occurrences and is worth 2 characters, so the three reachable totals are
278866 (both ß), 278868 (one site each way) and 278870 (both ss). The `HYP`/`Hyp` case swap is
independent again and length-preserving, so it changes nothing the gate can see.

**The correction that generalises**: this document's instability is not a flip between two
states. It is a **product of independent binary sites**, so the number of reachable values is
`2^n`, not 2 — and "I rendered it twice and saw both states" is not a bound on anything. Two
renders that agree prove only that you sampled the same corner twice. C11's row is updated.

*Not claimed here:* whether `047_Date_tracker_Gantt_chart`, which C11 also calls a two-state
oscillator, is likewise more than two-state. It was not re-measured in this round and the
correction above is asserted only for `alle einzeln`.

## Banked

`rows.tsv`; three of the six reference extractions, chosen to include the pair that agrees on
all four counts and still differs, so the third site can be re-diffed without re-rendering.
