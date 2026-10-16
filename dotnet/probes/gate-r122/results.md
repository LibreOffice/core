# Gate r122 — the underline change moves one gate column, and that column is the reference's

Full-corpus gate at HEAD `0fa883d0c`, both legs fresh, reference
`/opt/libreoffice26.2/program/soffice` (**26.2.4.2**, `0229ac93fcf0d7cbc6376066c6f35021cef002dc`).

```
TOTAL 947  MATCH 915  MISMATCH 32  REF-CANNOT-RENDER 0
```

Unchanged from r114, r116 and r119.

## The diff against r119

r119 and r122 ran on the **same UTC day**, so C13 does not apply to this pair — and, as a
control on C13 itself, **not one of the 32 volatile-date rows moved**, where the r116 -> r119 pair
that did straddle a boundary moved seven of them and nothing else.

| | r119 -> r122 |
|---|---|
| verdict changes | **0** |
| page changes | **0** |
| character changes | **1** |
| C13 volatile rows moved | **0** |

The one row is `sheets/done-013/xlsx/alle einzeln.xlsx`, **278869/278866 -> 278869/278868**: our
column is identical and the **reference** moved by 2.

## That row is C11, confirmed by re-rendering rather than assumed

C11's rule is that a per-document delta is trustworthy only once the reference has been rendered
twice for that document. Rendered twice, same binary, same file, same UTC day
(`alle-einzeln-ref-run-a.txt`, `-run-b.txt`):

| | run a | run b |
|---|---:|---:|
| alphanumeric characters (UTF-8) | 278868 | **278866** |
| bytes of `pdftotext` output | 371698 | 371698 |

The two runs differ by exactly the 2 that the gate saw, and the gate's r119 and r122 values are
*these two values*. C11 already names this document and this mechanism — *"`alle einzeln` (+2 —
it wrote `Janßen,` in one run and `Janssen,` in the other, twice, a font resolving differently,
the same class as C1)"*. Here the pair is `Meß,` / `Mess,`, twice. **Nothing in this row is ours.**

## What is new: the same document also oscillates in CASE, and no character count can see it

Diffing the two runs token by token gives **two** differences, not one:

| how many | run a | run b | effect on the character count |
|---:|---|---|---|
| 2 | `Mess,` | `Meß,` | **+2** — this is what the gate sees |
| 8 | `HYP` | `Hyp` | **none** — same length |

The eight `HYP`/`Hyp` are a *case* oscillation, and they are **length-preserving**, so a
reproducibility check done by comparing character counts scores this document as having moved by
2 when it has actually moved in 10 places. Both are the same class as C1 — a face resolving
differently between runs, here changing whether the small-capital run reaches the text layer
uppercased.

**The consequence for method**: C11's "render the reference twice and compare the count" is a
*lower bound* on the reference's instability, not a test for it. A document can be
character-count-stable and still not reproducible. Where a round needs the reference to be stable
in what it *says*, diff the extracted text, not its length.

## Superseded: the two-state claim below is wrong

**Gate r129 refuted this.** This file says the document has two states and that two fresh renders
"named both". It has at least **three** — 278866, 278868 and **278870** — because there are two
*independent* ß/ss sites, `Janßen`/`Janssen` and the `Meß`/`Mess` found here, and this write-up
treated them as one. Two renders that agree sample the same corner twice and bound nothing. See
`probes/gate-r129/results.md`; the numbers below are correct as observations and wrong as a bound.

## An instrument error of mine, recorded so it is not re-derived

My first count of the two runs gave **277247 and 277243**, a difference of 4, and I nearly wrote
that the movement was 4 rather than 2. The count was `tr -cd '[:alnum:]' | wc -c` under the **C
locale**, which does not treat `ß` as alphanumeric and so discards both its UTF-8 bytes: each
`Meß`/`Mess` swap therefore reads as 2 rather than 1. Counted as characters under a UTF-8 locale
the two runs are 278868 and 278866, which matches the gate exactly. **The 4 was the locale, not
the document.**

## Banked

`rows.tsv`; the two reference extractions, so the oscillation can be re-diffed without re-rendering.
