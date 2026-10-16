# Gate r119 — the round-117 and round-118 merges move no gate column at all

Full-corpus gate at HEAD `90b5b2207`, both legs fresh. Reference
`/opt/libreoffice26.2/program/soffice` — LibreOffice **26.2.4.2**, hash
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`, confirmed from the run's own banner rather than
assumed. Measured leg published to `/home/user/gate-r119-cli`.

```
XRAY_OFFICE_CLI=/home/user/gate-r119-cli/XRay.Office.Cli REF_SOFFICE=/opt/libreoffice26.2/program/soffice \
RENDER_TIMEOUT=300 batch-check.sh /home/user/sample-files "*" /home/user/gate-r119 3
```

```
TOTAL 947  MATCH 915  MISMATCH 32  REF-CANNOT-RENDER 0
```

Unchanged from r114 and r116, which were also 947 / 915.

## The diff against r116 is empty on every comparable row

`gatediff.py` compares pages (col 3), verdict (col 7) and alphanumeric characters (col 9) row by
row, and flags any row whose basename is in `gate-r114/volatile-xlsx.txt` as **not comparable**
under C13.

| | r116 -> r119 |
|---|---|
| rows | 947 -> 947, no row added or dropped |
| verdict changes, comparable | **0** |
| page changes, comparable | **0** |
| character changes, comparable | **0** |
| rows that moved at all | **7, every one of them a C13 volatile-date row** |

**On all seven, our own column is byte-identical and only the reference moved:**

| document | ours | reference r116 -> r119 |
|---|---:|---|
| `047_Date_tracker_Gantt_chart` | 3058 | 3441 -> **3367** |
| `066_Agile_Gantt_chart` | 3021 | 3086 -> 3083 |
| `085_Simple_Gantt_chart` | 3043 | 3061 -> 3063 |
| `082_Project_to_do_list` | 1128 | 1132 -> 1137 |
| `083_Project_tracker` | 935 | 929 -> 925 |
| `036_Simple_to-do_list` | 354 | 359 -> 354 |
| `088_To-do_list_with_progress_tracker` | 324 | 316 -> 315 |

The r116 gate ran on 2026-09-13 and this one on 2026-09-14, so the pair straddles a UTC date
boundary and C13 applies by construction. C13's second rule — *a verdict change is not a
regression until our own column is shown to have moved* — is satisfied trivially here: no verdict
changed, and our column moved on nothing.

`047_Date_tracker` returning **3441 -> 3367**, its r114 value, is C11's two-state oscillator
completing a cycle; it is also a volatile row, so it is doubly explained and neither explanation
needs the other.

## What this establishes about rounds 117 and 118

Between r116 and this gate the tree took the `chartslide` and `underline` merges — the width-0
chart rules, the `.ppt` table's inner measure, and the Calc hyperlink underline and colour. **None
of it moves a single gate column on any of 947 documents**, which is the expected result and is
worth stating rather than assuming: every change in those two rounds is sub-point stroke geometry
or colour, and the gate scores page count, alphanumeric characters and unembedded fonts. The gate
is not evidence that those changes are right. It is evidence that they broke nothing it can see.

## Banked

- `rows.tsv` — all 947 rows.
- The diff script is `gatediff.py`, validated before use on the known-good r114 -> r116 pair, where
  it reproduces exactly the two documented C11 oscillators (`SIL_TDB648` 30899 -> 30896 and
  `047_Date_tracker` 3367 -> 3441), both moving only in the reference column, and correctly flags
  `047` as volatile and `SIL_TDB648` as not.
