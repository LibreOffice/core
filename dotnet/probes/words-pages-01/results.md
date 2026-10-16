# words-pages-01 — two one-page shortfalls, two unrelated causes, one of them shared

Round brief: `words/batch-004/doc/1447.doc` (3 pages against 4) and `words/batch-006/doc/003.doc`
(4 against 5), both `.doc`, both word-exact, both nominated as the cleanest instances of the
standing **under-paginate** class — "a vertical-budget error (line height, text-area height, or the
line-fits rule)".

**The brief's framing is wrong for both documents**, and that is the round's first result. Neither
is a vertical-budget error. One is a font substitution the gate's font column cannot see; the other
is an empty paragraph measured at the wrong size by the WW8 reader. They have nothing in common
beyond the symptom.

`prediction.md` was committed at `bacadc1f517` before any fix was written or any sweep run.

---

## 1. `003.doc` — an empty paragraph takes the CHPX that ends at its mark

### What was measured

Page 2 onwards is identical in pitch, line for line — 13.80, 27.60, 47.40, 19.80 pt in both
renderings — and differs only in where the page starts. The whole error is on page 1 and it is
**32.20 pt**, which is exactly three empty paragraphs measured at 12 pt where LibreOffice measures
them at 14, 36 and 14:

| empty paragraph | reference | ours | deficit |
|---|---|---|---|
| after "Tisková a informační služba" | 16.10 pt (14 pt) | 13.80 pt (12 pt) | 2.30 |
| after "INFOSERVIS" | 41.40 pt (36 pt) | 13.80 pt (12 pt) | 27.60 |
| after the empty Heading 2 | 16.10 pt (14 pt) | 13.80 pt (12 pt) | 2.30 |

32.20 pt is two 16 pt lines, which is what let two extra empty paragraphs fit at the foot of our
page 1; every page after it then carried two lines more than the reference's, and the seven trailing
empty paragraphs that give the reference a fifth (blank) page fitted on our fourth.

Two instruments made this attributable rather than inferred, and both are committed here:

* **`FlowProbe`** dumps every placed line with its box height and the paragraph it came from. An
  empty paragraph draws no ink, so `pdftotext -bbox` cannot see it on either side and the
  arithmetic above cannot be done from the PDFs alone.
* **`chpx.py`** reads the `.doc` itself — piece table, `PlcfBteChpx`, CHPX FKPs — and prints the
  grpprl in force at each paragraph mark.

### The rule

`chpx.py` says all three of those marks carry **no CHPX exception at all** and `istd` 0 (Normal), so
the paragraph style gives 12 pt, which is what we used. Each of them is the first paragraph after a
CHPX run that *did* carry a size. The FKP reads:

```
run 3: fc 1094..1150  CHps=1c00 (14pt)   ends at cp 63 = an empty paragraph's own mark
run 4: fc 1150..1170  (no exception)     cp 63..72, ten empty paragraphs
run 5: fc 1170..1192  CHps=4800 (36pt)   ends at cp 84
run 6: fc 1192..1194  (no exception)     cp 84
run 7: fc 1194..1196  CHps=1c00 (14pt)   cp 85
run 8: fc 1196......  (no exception)     cp 86 onwards
```

LibreOffice draws the empty paragraphs at cp 63, 84 and 86 at 14, 36 and 14 pt, and the ones at
cp 64-72 and cp 87 at the style's 12 pt. The discriminator is whether a CHPX exception **ends at the
mark**: LibreOffice's reader closes such an attribute at offset 0 of the node that mark has already
opened, and a zero-length hint on an empty node covers the whole node.

> **An empty paragraph whose own mark carries no CHPX exception takes the one in force at the
> position before it.** The paragraph style stays its own — what crosses the mark is a character
> attribute, not a style — and it never crosses a story boundary.

All seven points on the document agree, **including the four that must not inherit**. Our own ODF
path lays the same document out correctly (`soffice --convert-to fodt` then `FlowProbe`), so the
layout engine was never implicated: `Ww8DocumentReader.Describe` was.

`EmptyParagraphCharacterLayout`, `src/XRay.Office.WordProcessing/Ww8/Ww8DocumentReader.Layout.cs`.

### Reach

Four of the corpus's 66 `.doc` carry the pattern at all (`003`, `07-04`, `A320SimNotes`,
`150_5335_5a`); three renderings moved. `003.doc` goes 4 → **5 pages, `match`**. `A320SimNotes.doc`
goes 40 → 41 against 42, and `P200904290238_0238_51880.doc` re-flows without changing its count.

**No committable fixture exists for this.** Every `.doc` in `tests/corpus/features` is a LibreOffice
export, and LibreOffice's own DOC writer emits an explicit CHPX at every paragraph mark — scanned,
0 of 46. The corpus rules forbid committing a document from the web. The rule is therefore pinned by
this file and by `chpx.py`, which reproduces the FKP table above from the bytes.

---

## 2. `1447.doc` — the family class the document declares decides the substitute

### What was measured

The gate reported `fonts 5/5` and it is not the same five. We drew the body in Liberation Serif and
the reference in **DejaVu Serif**. The line advances name the faces outright: ours 13.80 pt, which
is Liberation Serif's `(1825 + 443 + 87) / 2048 x 12 pt`, and the reference's 14.00 pt, which is
DejaVu Serif's. Both wrap to the same 432.0 pt measure and the reference fits about **11% less text
on each line** — its first paragraph takes nine lines where ours took seven.

The body names the family **`Times`**. Ten authored one-paragraph documents rendered by the
installed 26.2.4.2 give the rule, and it is not about `Times`:

| declared | class | LibreOffice draws | `fc-match <name>` |
|---|---|---|---|
| Times | *(none)* | Liberation Serif | Liberation Serif |
| Times | roman | **DejaVu Serif** | Liberation Serif |
| Times | swiss | **DejaVu Sans** | Liberation Serif |
| Times | modern / script / decorative | Liberation Serif | Liberation Serif |
| Helvetica | *(none)* / swiss | Liberation Sans / **DejaVu Sans** | Liberation Sans |
| Albany | *(none)* / swiss | Liberation Sans / **DejaVu Sans** | Liberation Sans |
| Thorndale | *(none)* / roman | Liberation Serif / **DejaVu Serif** | Liberation Serif |
| Times New Roman / Arial / Calibri / Cambria / Courier New | roman / swiss / swiss / roman / modern | Liberation Serif / Liberation Sans / Carlito / Caladea / Liberation Mono | same |
| Symbol | *(none)* / roman | OpenSymbol / OpenSymbol | OpenSymbol |

`fc-match "Times,serif"` answers DejaVu Serif, and that is the mechanism exactly.
`FontConfigManager::Substitute` (`vcl/unx/generic/font/fontconfig.cxx`:1076-1086) adds the requested
name as `FC_FAMILY` and then **appends a second `FC_FAMILY`** — `"serif"` for `FAMILY_ROMAN`,
`"sans"` for `FAMILY_SWISS`, and nothing at all for any other family type, which is why `modern`
does not mean monospace however much it looks as though it should. It is the *pre-match*
substitution, so it runs **before** LibreOffice consults `VCL.xcu` — and that ordering is what our
resolver had backwards, its comment saying the table is consulted first and fontconfig only when the
chain names nothing installed.

> **When a document declares a Roman or Swiss family class for a font that is not installed, the
> generic family it implies decides the substitute, ahead of LibreOffice's own chain.** Two
> exceptions, both measured: a *strong* metric alias survives it — the test is whether an installed
> face declares itself the equivalent of the very name asked for, which is true of Liberation Sans
> for Arial and false of Liberation Sans for Helvetica — and a pi face is exempt, because every Word
> document declares `Symbol` roman and there is no roman equivalent of a font of arrows.

`SystemFontResolver.Resolve`, with `DeclaredFontFamily` on `FontRequest`; fed by `FFN.ff` in
`Ww8FontTable` and by `w:family` in `fontTable.xml` for DOCX.

### Reach, and the instrument that measures it properly

The word gate barely sees this. What it is *about* is which face is on the page, so the honest
measure is the symmetric difference between our face set and the reference's, over the 36 renderings
the two fixes changed:

| face-set distance to the reference | documents |
|---|---:|
| closer | **29** |
| unchanged | 5 |
| further | 2 |

`1447.doc` goes from 4 faces wrong to **0**; `Sample_SQMS_Program.docx`, `May 25 bulletin…docx`,
`ECSS-E-ST-50-16C…docx` and eight others go to an exact match. The two that went further are
`ABCD-FE-01-00` and `ABCD-WB-08-00`, from one author: both now draw *an extra* DejaVu Serif the
reference draws in nothing, from a `Times-Roman`/`Cambria Math` entry the reference never puts a
glyph in. Named as a lead, not chased.

### `1447.doc` is still 3 pages against 4, and the residue is 1 twip

The font fix landed and did not finish the document. **Every line break on page 1 now matches the
reference word for word**, which none of them did before — confirmed by a blind reading of the
page pair by a subagent that was shown nothing but the image and reported "the wrapped line-breaks
within shared sentences match word-for-word", "font, font size, justification and margins appear the
same", and located the whole difference in where the page breaks. That is the `page-vision` control
agreeing with the measurement from the other direction.

What is left is that our DejaVu Serif line is **13.95 pt and LibreOffice's is 14.00** — one twip. By
line 35 the accumulated 0.91 pt is exactly the margin by which our 37th line fits and the
reference's does not; with only one line of the next paragraph fitting, its orphan control (2) moves
the whole four-line paragraph to page 2, and the reference's page 1 ends three lines early.

**This was investigated and deliberately not changed.** See §4 — no rule fits.

---

## 3. Numbers

Baseline and both sweeps use `sweep.sh`, committed here: the gate's own three checks against the
**banked** 26.2.4.2 references, rendering only our half, with `SOURCE_DATE_EPOCH` set. It is
validated twice over — its baseline reproduces `words-e-01`'s recorded whole-track figures exactly
(155 match, 165 page-exact, 115 page error), and its verdicts on `batch-004`/`batch-006` reproduce
`batch-check.sh`'s document for document. It costs 3 minutes where `batch-check.sh` costs an hour,
which is what made a before-and-after reach figure affordable.

### The whole words track, 200 documents

**Two bases, because the branch was merged with a second round that overlaps it.** The figures below
the line are the ones that describe what is on the integration branch after this work; the ones above
it are what this branch did on its own and are kept because they are what the diagnosis was made
against, not because they are still current.

*Against the pre-merge base (`b8f26134e07`) — this branch alone:*

| | baseline | after |
|---|---:|---:|
| match | 155 | 157 |
| page-exact | 165 | 166 |
| total absolute page error | 115 | 113 |
| renderings changed | — | 36 |
| face-set distance to the reference | — | 29 closer, 5 unchanged, **2 further** |

*Against `claude/paperless-odf-phase-1-rnyzcu` at `7756cd67565` — what merging this adds:*

| | integration alone | with this branch |
|---|---:|---:|
| match | 156 | **157** |
| page-exact | 166 | 166 |
| total absolute page error | 114 | **113** |
| renderings changed | — | 13 |
| face-set distance to the reference | — | **8 closer, 5 unchanged, 0 further** |
| verdicts lost | — | **0** |

The verdict this branch adds on top of the integration branch is `003.doc` — the WW8 empty-paragraph
rule, which nothing else implements. `150_5335_5a.doc` is gained by *both* rounds independently and
is counted once, in the integration branch's own 156.

**The two documents that got worse pre-merge got better in the merge.** `ABCD-FE-01-00` and
`ABCD-WB-08-00` were the only two renderings this branch moved *away* from the reference's face set,
and both were Symbol runs declared roman being routed to DejaVu Serif. The pi-face carve-out —
written for them and measured on the binary — is in the reconciled resolver, and neither document
changes at all now.

One page count still goes backwards and it is the same one as before:
`May 25 bulletin focus on carers in the workplace.docx` was 4/4 and is now 5/4. Its face set went
from 4 wrong to **exactly the reference's** — it had been page-exact with the wrong font, which is
the cancelling-errors shape `TODO.batches.md` warns about. The document fails the gate on words
either way and always did.

### Batch validation, in the order the project requires

```
batch-check.sh … 'words/batch-00[46]'    TOTAL 20  MATCH 19  MISMATCH 1
batch-check.sh … 'words/batch-00[1-6]'   TOTAL 60  MATCH 59  MISMATCH 1
```

18/20 before, 19/20 after; the one remaining is `1447.doc` at 3/4. Batches 001-003 and 005 are
untouched — 58/60 before, 59/60 after.

### Tests

Every project run individually, counts read rather than colours. Figures are the **reconciled** tree.

| project | passed | failed | skipped | total |
|---|---:|---:|---:|---:|
| XRay.Office.Core.Tests | 313 | 0 | 0 | 313 |
| XRay.Office.Containers.Tests | 109 | 0 | 0 | 109 |
| XRay.Office.Markup.Tests | 259 | 0 | 0 | 259 |
| XRay.Office.OpenDocument.Tests | 125 | 0 | 0 | 125 |
| XRay.Office.Presentations.Tests | 631 | 0 | 0 | 631 |
| XRay.Office.Rendering.Tests | 148 | 0 | 1 | 149 |
| XRay.Office.Spreadsheets.Tests | 747 | 0 | 0 | 747 |
| XRay.Office.Text.Tests | 310 | 0 | 0 | 310 |
| XRay.Office.Vector.Tests | 295 | 0 | 0 | 295 |
| XRay.Office.WordProcessing.Tests | 818 | 0 | 0 | 818 |
| XRay.Office.Fidelity.Tests | 520 | **30** | 0 | 550 |

`XRay.Office.Fidelity.Tests` had **31** failures on the pre-merge base and on this branch before the
merge, and has **30** now — one fewer, not one more. **The flip is the integration branch's, not this
reconciliation's**: built and run on its own, `7756cd67565` also reports 30 of 550, and its failing
set is identical name for name to the merged tree's. Nothing here added a failure and nothing here
fixed one. The build is warning-free.

`XRay.Office.Rendering.Tests` has skipped 1 on every run measured this session, including the
pre-merge base — it is not new.

---

## 4. What was measured and deliberately not acted on: the line-height law

Chasing `1447.doc`'s last twip led to a finding worth more than the document, and to a decision not
to act on it.

Line pitch was measured for five installed faces at fifteen sizes by reading the `Td`/`TD`/`Tm`
operators out of LibreOffice's own PDFs (`baselines.py` here; `pdftotext -bbox` reports the *ink*
box, which moves with whichever glyphs a line holds and cannot settle a one-twip question). Our rule
— round `(ascender + descender + lineGap) / upem x size` once, to whole twips — is right on **70 of
75** points. The five it misses are each exactly +1 twip:

| face | size | exact (twips) | ours | LibreOffice |
|---|---:|---:|---:|---:|
| Carlito | 18 | 439.453 | 439 | **440** |
| DejaVu Serif / Sans | 12 | 279.375 | 279 | **280** |
| Liberation Sans | 13 | 298.975 | 299 | **300** |
| Liberation Sans | 16 | 367.969 | 368 | **369** |
| Liberation Serif | 10 | 229.980 | 230 | **231** |

**The rounding rule cannot be "sum then round", and this refutes it outright**: Liberation Serif
(1825 + 443 + 87) and Liberation Sans (1854 + 434 + 67) have the *identical* total of 2355 design
units, so the formula predicts the same pitch for both at every size — and they measurably differ at
10, 13 and 16 pt. The split decides, not the sum.

No candidate fits. Rounding ascent, descent and leading separately is worse (19 misses of 75);
ceiling is worse (13); `max` of the two is wrong on Liberation Serif at 10 pt; ascent-plus-leading
against descent is wrong on Liberation Serif at 12 pt. Every miss is +1 and never −1, so the true
value is `roundA` or `roundA + 1` and something not yet named picks between them.

Left alone on purpose. It is a change to the metric every line of every document in all three tracks
is measured with, on evidence that names no mechanism — which is the fudge-factor trap wearing a
different hat. The table above is the specification for whoever takes it: 75 points, an exact
instrument, and a refutation to start from.

## 5. Leads

* **The line-height law, above.** Worth a round of its own. Every document short by a line at the
  foot of a page is a candidate.
* **RTF and ODF declare the same family class and neither reader records it.** RTF's `\froman`
  family is not parsed at all; ODF's `style:font-family-generic` is present in every file
  LibreOffice writes and is not read. Both are reader work, not resolver work — the resolver takes
  `DeclaredFontFamily` from anyone.
* **Presentations and spreadsheets are untouched and should not stay that way.** OOXML carries the
  same datum as `pitchFamily` on `<a:latin>`. Nothing in this round changes a slide or a sheet,
  because no reader outside word processing sets `DeclaredFamily`.
* **`ABCD-FE-01-00` and `ABCD-WB-08-00`** now embed one face more than the reference. A
  `Times-Roman` or `Cambria Math` entry the reference puts no glyph in.

---

## 6. Scoring `prediction.md`

| # | claim | conf | outcome |
|---|---|---:|---|
| P1 | The WW8 rule makes `003.doc` 5 pages and a `match`. | 0.85 | **right** |
| P2 | The family-class rule makes `1447.doc` 4 pages and a `match`. | 0.75 | **wrong** — the faces match the reference exactly and every line break on page 1 does too, and the document is still 3 pages. The one-twip line height finished it, and I had no reason to suspect a second cause behind the first. |
| P3 | The WW8 rule changes 3-12 renderings, no more than 2 verdicts backwards. | 0.55 | **right** — 3 renderings, 0 backwards. At the very bottom of the interval. |
| P4 | The family-class rule changes 10-45 renderings. | 0.60 | **right** — 33 (13 `.doc`, 20 `.docx`). The stated failure mode was that it might reach nothing; it did not. |
| P5 | Net verdicts improve by at least 2 and the page error falls. | 0.60 | **right** — 155 → 157, 115 → 113. Exactly at the boundary, so this is a weak pass. |
| P6 | Neither rule touches `batch-001`..`batch-003`. | 0.50 | **wrong** — one `batch-003` rendering changed. It kept its verdict. |
| P7 | 1 to 4 currently-matching documents break. | 0.65 | **wrong, and wrong in the good direction** — none did. One *page-exact* document lost its page-exactness, but it was already failing the gate on words and it gained the reference's exact face set doing it. |

Four of seven. The two misses that matter are P2 and P7, and they miss in opposite directions: the
font fix did less than predicted to the page count of the document it was derived from, and more
than predicted to the rest of the track without costing anything. Both are the same underlying
mistake — **treating a font substitution as a pagination fix**. It is a *rendering* fix that
sometimes moves a page count, which is why the face-set table in §2 is the honest measure of it and
the gate's two verdicts are not.

---

## 7. Reconciling with the parallel round

A second agent implemented "carry a document's declared font shape into the resolver" independently
and landed first, on `claude/paperless-odf-phase-1-rnyzcu`. Merging produced 13 conflicts across
seven files. Which implementation survived, and why:

| piece | kept | why |
|---|---|---|
| `FontRequest.DeclaredClass` typed as `FontFamilyClass`, plus `DeclaredFontShape(Class, Pitch)` | **theirs** | It reuses a type that exists and carries the pitch beside the class. My `DeclaredFontFamily` recorded `modern`/`script`/`decorative` faithfully, which is tidier as a record and worth nothing as behaviour: all three are inert and their readers collapse them at the point of reading, which is where the measurement that they are inert belongs. |
| `Ww8FontTable.ShapeOf`/`ShapeIn`, `WordFontTable.ShapeOf`, `LayoutFonts.DeclaredShapes`, the DOCX and DOC wiring | **theirs** | Same shape as mine and wider: theirs also wires SpreadsheetML `<family val>`, rich-text `rPr`, the BIFF `FONT` family byte, the XLSB `BrtFont` byte and ODF's spreadsheet path. Mine wired DOC and DOCX only. |
| **Where in the order the declaration is consulted** | **mine** | Theirs reads it inside `GenericFallbacks`, which runs only after the substitution chain has come up empty. Mine runs it *before* the chain. This is the only behavioural disagreement and it is decisive. |
| The strong-metric-alias exception | **mine** | Required by the ordering above and by nothing else: with the chain first, Arial and Times New Roman are protected by the chain itself; with the declaration first they need an explicit exception or every Word document reflows. |
| The pi-face exemption | **mine** | Same: only reachable once the declaration outranks the chain. |
| A declared fixed pitch beating a declared family | **theirs** | Measured by them, absent from mine, and folded into the shared helper. |

### Why the ordering is mine

Theirs is not wrong on any case they measured — `Garamond`, `Georgia`, `Futura`, `Tahoma` and
`TimesNewRomanPSMT` have no installed chain entry, so chain-first and chain-second give the same
answer for all five. The two orderings can only be told apart by a name whose chain entry *is*
installed, and there are four of those in the measurements: `Times` (chain names `liberationserif`),
`Helvetica` and `Albany` (`liberationsans`), `Thorndale` (`liberationserif`). Under chain-first every
one of them answers Liberation. Measured against the installed 26.2.4.2 with one authored document
each, every one of them answers **DejaVu**.

The source says the same thing and says it first: `FontConfigManager::Substitute` is registered as
the *pre-match* substitution, so it runs before `VCL.xcu` is consulted at all rather than after the
chain fails. Their remark on
`ADeclaredShapeCannotDisplaceAFamilyThatIsInstalledOrSubstitutable` — "the declaration is consulted
only once the chain has come up empty" — was the reasoning, not a measurement, and it has been
replaced by the mechanism that actually holds those two assertions up.

Concretely: without the ordering, `1447.doc` still renders its body in Liberation Serif, because
`times` names `liberationserif` and the chain finds it. That is the document the whole font half of
this round came from.

### Tests

`Ww8FontTableTests` (theirs) is a superset of my `DocFontFamilyClassTests` — every `ff` value, the
masking trap, the pitch bits, duplicate names and a malformed length. Mine was deleted and the one
thing it held that theirs did not, the `1447.doc` measurement that says why the field is worth
reading, moved into their class remark. In `FontResolutionTests` the two duplicated tests were
collapsed into one theory covering both sets of names, with the remark rewritten to name the
mechanism; the other four are kept as they were, and my two new ones — the weak-alias ordering and
the pi face — are added beside them.

### A check the reconciliation happened to make available

The merged tree's 200 gate rows are **byte-identical** to the ones this branch produced on its own,
and so is the `batch-001`–`006` TSV. That is not a coincidence and it is worth stating as evidence:
the other round's font reach on the words track is a strict subset of this one's, because the
pre-match ordering fires everywhere the chain-last ordering does and on four more names besides. It
also says nothing of theirs was lost in the merge and nothing was applied twice — either would have
moved a row.

Where the two rounds are *not* redundant is everything outside the words track: their spreadsheet
wiring (SpreadsheetML `<family val>`, rich-text `rPr`, the BIFF and XLSB font family bytes, ODF's
spreadsheet path) has no counterpart here and is untouched by this reconciliation.
