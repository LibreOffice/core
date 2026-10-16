# Gate r125 — 368 renderings changed their rule geometry and no gate column moved

Full-corpus gate at HEAD `e484d5db3`, both legs fresh, reference
`/opt/libreoffice26.2/program/soffice` — **26.2.4.2**, `0229ac93fcf0d7cbc6376066c6f35021cef002dc`.

```
TOTAL 947  MATCH 915  MISMATCH 32  REF-CANNOT-RENDER 0
```

Unchanged from r114, r116, r119 and r122.

## What was between this gate and the last one

The round-123 merge, which is the largest change to drawn ink in many rounds:

- the double underline through four word-processing spellings;
- **O64's offset half** — every underline and strikethrough moved to where the reference puts
  it, across all three layout models;
- the Calc double-line position, which O64 had scored "exact" on thickness alone;
- a bold underline.

Round 123 measured that as **368 of 947 renderings changed**, the same 368 O64 moved, track for
track. Round 124 changed no source at all.

## The diff against r122

| | r122 -> r125 |
|---|---|
| verdict changes | **0** |
| page changes | **0** |
| character changes | **1** |
| C13 volatile rows moved | **0** |

The one row is `sheets/done-013/xlsx/alle einzeln.xlsx`, **278869/278868 -> 278869/278866** — our
column identical, the reference moving by 2.

**That row needs no investigation this time, and the reason is banked.** Gate r122's write-up
recorded this document as a C11 oscillator and named both of its states from two fresh reference
renders on the same UTC day: **278868 and 278866**. Those are exactly the two values here, and the
mechanism is recorded there too — `Mess,` / `Meß,` twice, a face resolving differently between
runs, the same class as C1. The oscillator has simply returned its other value. **Corrected at r129: "its other value" is wrong — there is no pair. The document reaches at least three totals from two independent ß/ss sites, and r129 observed 278870.** Our column has now
held 278869 across r119, r122 and r125.

## What this establishes, stated narrowly

**Every underline and strikethrough in 368 of 947 renderings is drawn at a different offset than
it was two gates ago, and not one page count, character count or verdict moved.** That is the
expected result and it is the whole point of stating it: the gate scores page count, alphanumeric
characters and unembedded fonts, so a rule's geometry is invisible to it by construction. This
gate is evidence that a large change to drawn ink broke nothing the gate can see. It is **not**
evidence that the new geometry is right — that evidence is round 123's 252 of 252 on the authored
position probe and 15 062 of 15 103 against the reference's own corpus rules.

## Banked

`rows.tsv`, all 947 rows. Diff produced by `gate-r119/gatediff.py`.
