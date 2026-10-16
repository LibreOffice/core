# Round 52 — sheets — prediction addendum: the slicer choice that wins and draws nothing

Committed **before** the second change is written and before anything is rendered with it.
The first half of the round (`prediction.md`, `6812e8ff019`) is measured and closed:
**268 → 269 of 307**, exactly as predicted, zero regressions.

## What two blind readings found, and what the markup says

Two fresh reviewers, on **unrelated documents and unrelated pages**, each given one image and
nothing else, independently reported the **same object** with the same direction and the same
location: *"the reference draws green-outlined text boxes reading `This shape represents a
slicer. Slicers are supported in Excel 2010 or later.` and ours draws nothing there."*

- `049_Expenses_calculator` page 1 — reviewer put the boxes in "the lower band of the content
  card, three in a row", "the single largest affected region, 35–40% of the card".
- `037_Personal_money_tracker` page 3 — reviewer put them "in the right margin beside the chart,
  two stacked boxes", third of nine differences by area.

Per `HANDOVER.md` § 7 this is checked before being treated as corroboration: the two reports are
about **the same object** (a green-bordered rectangle carrying the same sentence), not merely the
same description, and neither page was chosen by `--worst` — both were chosen because that page
carries the document's whole word deficit. `pdftotext` then confirms it independently:
**the reference draws the advisory 3 / 2 / 1 times on `049` / `037` / `DynamicBubbleChart` and we
draw it 0 times.**

## The mechanism, and a test that passes for the wrong reason

`OoxmlXml.ResolveAlternateContent` takes an `mc:Choice` when every prefix its `Requires` names
resolves to a namespace in `OoxmlNamespaces.UnderstoodExtensions`. All three documents write

```xml
<mc:AlternateContent xmlns:a14="http://schemas.microsoft.com/office/drawing/2010/main">
  <mc:Choice Requires="a14">
    <xdr:graphicFrame>…<a:graphicData uri="…/office/drawing/2010/slicer">…
  </mc:Choice>
  <mc:Fallback xmlns=""><xdr:sp>…This shape represents a slicer…</xdr:sp></mc:Fallback>
</mc:AlternateContent>
```

`a14` **is** `DrawingML2010`, which **is** in `UnderstoodExtensions`. So the choice wins, the
frame it holds is a slicer we have no reader for, and the anchor draws nothing at all — no ink,
no words. The fallback is never reached.

Round 50 wrote the rule that was meant to prevent exactly this and left a comment saying so:
the chartex exception "deliberately does not generalise to the identically-shaped slicer
placeholder, **because the reference draws that one**". Its test
`OoxmlAlternateContentTests.ASlicerChoiceStillLosesToItsFallback` asserts precisely the right
thing — and **passes for the wrong reason**. Its helper binds the `a14` prefix to the synthetic
namespace `urn:the-extension`, which is not understood, so the fallback wins by the *general*
rule and the test never exercises the case the corpus actually contains. It is the project's
dominant pattern — the predecessor claim reproduces and the sentence attached to it is wrong —
found in the project's own test rather than in a brief.

## The change

Narrowest possible, and the exact mirror of round 50's chartex constant: **a choice whose
`a:graphicData/@uri` is the 2010 slicer URI loses to a sibling `mc:Fallback`**, whatever its
`Requires` says. No fallback, no change. `XRay.Office.Ooxml/OoxmlXml.cs` and
`OoxmlNamespaces.cs`, plus the round-50 test rewritten to bind the real namespace.

## The census, and the cross-track reach

Every corpus document parsed, every `mc:Choice` whose `Requires` resolves **entirely** to
namespaces in `UnderstoodExtensions`, every `a:graphicData/@uri` inside it, with whether a
sibling `mc:Fallback` exists:

| uri inside an understood `mc:Choice` | documents | families |
|---|---:|---|
| `…/word/2010/wordprocessingShape` | 108 | words |
| `…/word/2010/wordprocessingGroup` | 51 | words |
| `…/word/2010/wordprocessingCanvas` | 4 | words |
| **`…/drawing/2010/slicer`** | **3** | **sheets** |
| `…/drawingml/2006/picture` | 1 | words |

Keyed on the last-but-one row and nothing else, so the blast radius is **3 documents, all sheets,
all `open`, 0 words and 0 slides**. This is a **shared-layer diff** (`XRay.Office.Ooxml`) and the
parent still owes the cross-track sweep; the census says nothing outside sheets can move, and the
words rows above are the documents to look at if anything does, since they share the code path
but not the key.

The 2010 slicer URI occurs in **7** sheets documents. The other four —
`Part_129_Operators`, `Part_375_Operators`, `TDA_Smoke-Detectors` (all `done`) and
`070_Equipment_inventory_list` (`open`) — write it under `Requires="sle15"`
(`…/drawing/2012/slicer`), which is **not** understood, so their fallback is taken already and
this change cannot reach them. `070` already draws its three "table slicer" advisories, and both
sides draw three: its 12-word gap is our advisory text **wrapping at different points**
(`Excel.If`, `TableThis`, `ofversion` are joined tokens in ours), which is a tokenisation
difference inside a shape we already draw and is not this defect.

## The prediction

**Three documents change, one verdict moves, sheets 269 → 270 of 307.**

| document | before | predicted after | verdict |
|---|---|---|---|
| `049_Expenses_calculator_c351f3d0.xlsx` | `words` 5/5 **213/332** | **~333**/332 | `words` → **`match`** |
| `037_Personal_money_tracker…XLSX` | `words` 5/5 442/505 | ~522/505 | **stays `words`** |
| `DynamicBubbleChart.xlsx` | `words` 5/5 309/341 | ~349/341 | **stays `words`** |

Derived, not observed: the advisory is 40 word-gate tokens (13 + 27), and the reference draws it
3, 2 and 1 times. `049`'s page 1 is ours 25 words against the reference's 146 — a 121-word gap of
which 120 is the three advisories and 1 is the pivot caption `Category`, so 213 + 120 = 333
against a band of 6.64. `037` and `DynamicBubbleChart` **overshoot** — they end up 17 and 8 words
*above* the reference against bands of 10.1 and 6.82 — so both stay failing while moving much
closer. Predicting a miss by 1.2 words on `DynamicBubbleChart` is deliberate: if it lands, the
40-token estimate was low.

**Zero page-count changes**, **zero movement on the other four slicer documents**, and zero
movement anywhere else.

## What this census cannot see

1. **Whether the fallback shape actually draws once selected.** The census establishes which
   branch is chosen; it says nothing about whether `XlsxDrawings` renders an `xdr:sp` that arrives
   inside `<mc:Fallback xmlns="">` — a fallback that resets the default namespace to empty. If it
   does not, all three documents are unchanged and the round's second half is a refutation.
2. **Page counts.** Three shapes that were drawing nothing start drawing. If a shape's anchor
   extends the used area, the print range and therefore the page count can move. All three
   documents' anchors sit inside columns the sheet already prints, so this is not expected — but
   it is the failure mode to look for and it is why 0 page-count movement is part of the
   prediction rather than an assumption.
3. **`.xls` and `.ods`.** Neither format states MCE at all, so neither can be reached; the census
   only opened zips.
4. **Nested `mc:AlternateContent`.** `ResolveAlternateContent` re-queries after each replacement,
   so a fallback containing another `AlternateContent` resolves in turn. The four `sle15`
   documents are that shape and the census asserts they do not change; if one does, this is why.
5. **Word tokenisation of the advisory.** 40 is counted from the sentence. `pdftotext` joins
   tokens across a line break in a narrow box — measured on `070`, where ours produces `Excel.If`
   and `TableThis` — so a narrow box could make our count differ from 40 in either direction.
6. **Ink.** Nothing here measures whether our green-bordered rectangle is the right size, colour
   or position; only that the words appear. The reviewers' descriptions are the only evidence
   about the box itself.
