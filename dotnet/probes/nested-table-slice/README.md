# A nested table taller than the page makes its outer row unsliceable

Measured 2026-08-15 against LibreOffice 26.2.4.2 with the full font set, on
`words/missing-001/docx/May 25 bulletin focus on carers in the workplace.docx`
(4 pages against 4, 435 words against 521 by the gate's count — it fails on
words alone).

## What was traced

`TableLayouter.SliceRow` was given a temporary `XRAY_OFFICE_TRACE_SLICE=1` stderr
trace printing, per call: the room left, the nested-table spans a cut may not
fall inside, the surviving candidate cuts, and the cut chosen. The instrument was
removed afterwards and the tree restored with `git checkout --` plus `touch`,
`obj/` and `bin/` deleted, and the rebuild verified by re-rendering the document
and confirming the output is byte-identical across two runs.

Every one of the six calls on this document reads the same:

```
SLICE room=697.9 rowTop=0.0 drawn=0.0 cells=1 nestedSpans=1 candidates=1
    nested span 0.0..1126.4
    candidates: 1126.4
    -> chosen=none height=0.0

SLICE room=297.8 ...  nested span 0.0..658.0    candidates: 658.0    -> chosen=none
SLICE room=26.4  ...  nested span 0.0..1126.4   candidates: 1126.4   -> chosen=none
SLICE room=697.9 ...  nested span 0.0..1145.6   candidates: 1145.6   -> chosen=none
```

## What it means

The outer row has **one cell holding one nested table that spans the whole
cell**. `SliceRow` forbids a cut inside a nested table and offers only its
bottom, so the candidate list has exactly one entry — and that entry is
**1126.4 pt on a body 697.9 pt tall**. The nested table is taller than any page,
so its bottom is unreachable at every room the paginator ever offers, and
`SliceRow` returns null on every call.

**The row therefore can never be split, at any page, by any amount of room.**
What does not fit is neither placed nor carried, and 86 words fall off the end.
`room=697.9` is a completely empty page, which is what makes this decisive: this
is not a tight fit or an off-by-one, it is a cut that does not exist.

## What it is not

Three plausible causes were tested and refuted before the trace was written, and
none should be re-derived:

- **The VML horizontal rules.** All four `<w:pict>` hold a *self-closing*
  `<v:rect o:hr="t" style="width:468pt;height:1.5pt"/>`. Stripping all four and
  re-rendering leaves page 1's break exactly where it was. (An earlier container
  test reported the content as being "inside `<v:rect>`" — an artefact of
  counting self-closing tags as unclosed.)
- **The banner image being mis-measured.** Deleting the "Monthly focus" picture
  paragraph outright does not bring the following heading onto page 1.
- **`keepNext`.** This document's `Heading2` carries none, and the reference
  itself puts the heading on page 1 with the following table on page 2.

Nor is it `FlowLayouter.MaxNesting`: the document reaches table depth 6 against a
cap of 16. An early count said 54 and was wrong — `<w:tbl` also matches
`w:tblPr`, `w:tblGrid` and `w:tblW`, so the pattern has to be `<w:tbl[ >]`.

## What the reference does, and why it does not need this

Writer refuses to slice such a row too. `bTableLayoutTooComplex`
(`sw/source/core/layout/tabfrm.cxx`:586-594) is set when a cell's *first* lower
is a row frame, and the split at `:611-613` is gated on it:

```cpp
if ( nTmpCut > nCurrentHeight ||
     ( pTmpLastLineRow->IsRowSplitAllowed() &&
       !bTableLayoutTooComplex && nMinHeight < nTmpCut ) )
```

So our null return matches Writer. The difference is what happens next: **a
nested table in Writer is a frame in its own right and splits across the page
through its own follow**, so the outer row never has to be cut. Ours is a
rectangle inside a cell's flow, placed whole or not at all.

## The fix that follows

`SliceRow` must be allowed to choose a cut inside a nested table, and `Sliced`
must then produce a *partial* `PlacedTable` — the rows above the cut plus a
recursively sliced straddling row — and carry the remainder to the next part.
Today `Sliced` assigns each nested table wholly to one part and says so in its
own comment ("`SliceRow` never chooses a cut that crosses one").

This is the most cascade-prone path in the project: it changes how every table
crossing a page boundary is placed. The whole `words/done-*` sweep — 161 of 161
— is the deliverable, and this document is the cheapest oracle for the fix,
since the reference's page 3 ends mid-paragraph ("...all about reducing the")
and its page 4 opens with the continuation ("stigma around this emotion.").

## Reproducing the trace

The instrument is deliberately not left in the tree. To take it again, print
`room`, `whole`, `candidates` and `chosen` in `SliceRow` immediately before
`candidates.Sort()` and immediately before `if (chosen is not { } cut)`, gated on
an environment variable, and render the document above.

Two things cost time and are worth knowing:

- `TreatWarningsAsErrors` promotes `CA1305` on a bare `double.ToString("F1")`,
  so a throwaway trace still needs `CultureInfo.InvariantCulture`.
- The gate's word count and `pdftotext | wc -w` are different numbers — 435 and
  448 here. Comparing one against the other reads as a change that never
  happened.
