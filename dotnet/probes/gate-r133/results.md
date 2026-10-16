# Gate r133 — the headline went UP by one, and that is not our work either

Full-corpus gate at HEAD `a185ec79a`, both legs fresh, reference `/opt/libreoffice26.2/program/soffice`
— **26.2.4.2**, `0229ac93fcf0d7cbc6376066c6f35021cef002dc`.

```
TOTAL 947  MATCH 916  MISMATCH 31  REF-CANNOT-RENDER 0
```

Every previous gate in this series read **915**. This one reads **916**, and the extra match is
**not** attributable to anything this tree did.

## What was between this gate and the last

Three source-changing merges, the widest spread of drawn-ink change since the underline work:

| round | change | its own measured reach |
|---|---|---|
| 130 | WW8 TOC style suppression keyed on one `sti`; the walk's dropped anchor character; a line of anchors taking no ascent | 6 of 66 `.doc`, 3 of 272 `.docx`, **0 of 338 `.odt`, 0 of 338 `.rtf`** |
| 131 | a shared cell edge is one border, narrower wins, resolved per column | 31 of 338 words renderings |
| 132 | the zoom belongs inside the `llround`, not after it | 31 of 947, **every one xlsx or xls** |

## The diff against r129

| | r129 -> r133 |
|---|---|
| verdict changes, comparable | **0** |
| page changes, comparable | **0** |
| character changes, comparable | **0** |
| rows that moved at all | **10, every one a C13 volatile-date row** |

Our own column is identical on all ten. The two gates ran on 2026-09-14 and 2026-09-15, so the
pair straddles a UTC date boundary and C13 applies by construction.

**None of the three rounds' measured reach shows up here, and that is the expected result**: the
gate scores page count, alphanumeric characters and unembedded fonts, and all three changes are
rule geometry, border resolution and anchor placement. Round 132's confinement claim in
particular was already proved byte-for-byte over all 947 documents, so a mover outside xlsx/xls
would have contradicted it. There is none.

## C13 is symmetric, and this is the dangerous half

The single verdict change is `066_Agile_Gantt_chart_08f9de45.xlsx`, **`words` -> `match`**, with
our column at **3021 before and after** and the reference moving 3083 -> 3080. It is on
`gate-r114/volatile-xlsx.txt` and it holds 21 `TODAY()`.

C13's row was written from this same document flipping the *other* way, and records: *"the
headline 916 -> 915 reads as a regression to anyone looking only at the totals"*. This gate is the
mirror image — **915 -> 916, reading as an improvement** — and it is equally not ours.

**The flattering direction is the more dangerous one.** A regression gets investigated; an
improvement gets reported. Had this gate been read off its totals it would have credited three
rounds with a match they did not earn, and the credit would have been durable: the next gate to
read 915 would then look like a regression from a number that was never real. The rule stands in
both directions — **a verdict change is not attributable until our own column is shown to have
moved** — and on this row it has not moved across r119, r122, r125, r129 and r133.

## Banked

`rows.tsv`, all 947 rows. Diff by `gate-r119/gatediff.py`.
