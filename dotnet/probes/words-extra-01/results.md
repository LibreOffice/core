# words/extra-001 — results

Seven documents, briefed as two clusters of "we draw content the reference does not". One
cluster is ours and is fixed; the other is a reference import defect that was closed two rounds
ago; two of the seven documents are not `extra` failures at all. Two further defects were found
on the way, one of them in the measuring apparatus rather than in XRay.Office.

Environment: LibreOffice **26.2.4.2**, Carlito/Caladea/Liberation/DejaVu all resolving,
`pdftotext` 26.01.0, `SOURCE_DATE_EPOCH=1700000000` on every render. References reused from
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`, never re-rendered.

## Baseline, established before anything changed

`XRay.Office.Fidelity.Tests` — **Failed 30, Passed 520, Skipped 0, Total 550**. The briefed
baseline exactly, so the tree was the one the brief described.

## The brief was wrong about cluster A, and the resolver was innocent

The brief attributes cluster A to `mc:AlternateContent` being drawn from both branches. It is
not. `OoxmlXml.ResolveAlternateContent` is correct and demonstrably runs on header parts —
`DocxFile.LoadHeaderOrFooter` loads through `OoxmlXml.TryLoad`, which calls `Normalise`. Page 1
of `ABCD-WB-08-00` is +2 words against the reference, not doubled; drawing both branches would
have doubled every page.

**The real cause is a fixed-height text box whose content is taller than the box.**
`word/header1.xml`'s first box is `wp:extent cy="190500"` — 15.00 pt — with `a:noAutofit` and
four paragraphs of 8 pt text in it. Writer formats the lines that fit and never lays the rest
out; we laid all of them out.

The surplus predicted from that arithmetic alone was 60 tokens on `ABCD-WB-08-00`: 2 per page on
all 12 pages from two 21.00 pt footer boxes each holding a label and a value, plus 6 per page on
the 6 even pages from the header box. The measured movement was **+117 → +57, exactly 60.**

The document supplies its own control, which is what makes this conclusive rather than plausible.
`footer2.xml`'s box 3 (`Revision` / `0`) is 21.00 pt with 1.70 pt insets — 17.6 pt of room — and
the reference draws one paragraph. Its box 5 (`Page 1` / `of 12`) is 25.00 pt with 3.60 pt insets
— 17.8 pt of room — and the reference draws **two**. Two hundredths of an inch apart, opposite
outcomes, and the reason is in the markup: box 5's first paragraph is `pStyle="Footer"` and has
no space-after, while box 3's takes the document default `w:after="200"`, which pushes its second
paragraph's top from ~9.3 pt to ~19.3 pt and over the edge.

## The measured rule, from 60 authored boxes

`probe-textbox-overflow.py` and `probe-textbox-sweep.py` render boxes of stated height 1–100 pt
at three inset sizes through the installed 26.2.4.2, each holding six paragraphs of 8 pt text.

> **A line is formatted iff its top offset is strictly less than the box's content height**
> (the stated height less `tIns` and `bIns`), **and the first line is always formatted** however
> short the box.

Everything else about it was measured rather than assumed, and three of the four answers are not
the obvious ones:

| variant | LibreOffice 26.2.4.2 |
|---|---|
| `a:noAutofit` | truncates |
| `a:normAutofit` | **truncates** — it does not shrink the text |
| `a:spAutoFit` | grows the box; nothing is dropped |
| `bodyPr/@vertOverflow` = `overflow` or `clip` | **no effect**; both truncate identically |
| VML `v:textbox` | truncates identically to `wps:txbx` |
| VML `mso-fit-shape-to-text:t` | grows, like `spAutoFit` |
| a table inside a box | truncates **by row**, on the same rule |
| `anchor` = `t` / `ctr` / `b` | same line count |

The competing rule — keep a line only when it fits entirely — is **refuted**, not merely
unsupported: a 10 pt box with zero insets draws two lines of a face taller than 5 pt. The
zero-inset sweep pins the face's line height to [9.5, 9.8) pt and every one of the 60 boxes falls
out of the single rule above.

## Cluster B is not our defect, and A and B are two bugs, not one

**Established, then re-established.** Cluster B is LibreOffice's own table-only-header import
defect: it copies a section's header into a following section only when the source header holds
at least one top-level `w:p`, and `UG.CAO.00133`'s `header1.xml` and `header6.xml` both have
`['tbl']` as their only top-level child. It was diagnosed in round 43, recorded in
`TODO.batches.md` §2, and **pinned by `SectionInheritedHeaderTests`**, whose class comment says
in terms that following the reference here requires deleting a test that explains why not to.
The cost was costed at the time: one verdict.

That prior work was calibrated against 24.2.7.2, and CLAUDE.md requires such claims to be
re-checked once against 26.2.4.2. **It reproduces.** Two independent measurements:

- `probe-header-inheritance.py` and `probe-header-geometry.py` — twelve authored two-section
  shapes varying the reference set (`even` only, `first` only, both, neither, an explicit
  `default`, with and without `w:evenAndOddHeaders`) and the page geometry (same margins,
  different margins, landscape, three sections). **LibreOffice inherits the default header in
  every one of them**, so §17.10.1 inheritance is not the variable and our implementation of it
  is right.
- Stripping every `even` and `first` header reference out of the real `UG.CAO.00133` — the shape
  §17.10.1 calls link-to-previous outright — leaves the reference still heading pages 1 and 14–17
  and no others. Removing the references changes nothing.

So the answer to the brief's question: **A and B are genuinely different bugs, and only A is
ours.** A is a shape-level text overflow that fires wherever a text box appears — header, footer
or body — and has nothing to do with sections. B is a section-level inheritance failure inside
LibreOffice's importer that we deliberately do not reproduce. They are not one
header-inheritance defect in different clothes; the fix for A moved neither `UG.CAO` document by
a single byte.

## Two of the seven documents are misfiled

`762.doc` (22 pages against 23, **−2** words) and `info-bulletin-601.doc` (5 against 6, **−4**)
are one page short with word deltas inside the noise. They are pagination failures, not `extra`
ones — we draw *less* than the reference on both — and the brief does not mention them at all.
Neither moved.

## The fix

Four files, one behaviour.

| File | Change |
|---|---|
| `src/XRay.Office.WordProcessing/Layout/PageFrame.cs` | `HasFixedHeight`, with the measurement on it |
| `src/XRay.Office.WordProcessing/Ooxml/DocxFrames.cs` | reads `wps:bodyPr` insets into `Padding` and `a:spAutoFit` into `HasFixedHeight`, in both the plain and the group-member paths |
| `src/XRay.Office.WordProcessing/Layout/FlowLayouter.cs` | `Truncated(flow, height)` — the rule |
| `src/XRay.Office.WordProcessing/Layout/FrameLayout.cs` | calls it from `Content`, and only for a fixed-height frame |

`HasFixedHeight` defaults to **false**, so the ODF, WW8 and RTF frame readers keep the behaviour
they had. The rule was measured on the DOCX importer and nowhere else; a round that wants it for
the others should measure them rather than assume it transfers.

The `wps:bodyPr` insets had to be read as well, and were not predicted. They are not decoration
here — they are the operand of the rule. A 15.00 pt box with the ECMA defaults holds 7.80 pt of
text, not 15.00, and reading them as zero keeps one line more than Writer draws on exactly the
boxes this is about. The WW8 reader already read the equivalent Escher properties into the same
property, so this made the DOCX path consistent with it rather than introducing a concept.

**Deliberately not done:** a table straddling a box's lower edge is kept or dropped whole, where
LibreOffice truncates it by row. Recorded in the remarks on `Truncated`. No document in this
group has one, and doing it properly means giving `TableLayouter` a height it does not currently
take.

## Reach, over all 200 words documents

Rendered twice — once with the fixed binary, once with the reverted one built from files copied
aside (never `git stash`; this repository has forty-odd worktrees and one stash stack) — and
diffed against the banked references with `SOURCE_DATE_EPOCH` set. Estimated from what
**resolves**, not from a grep: `mc:AlternateContent` and `wps:txbx` are everywhere in OOXML and
counting them would have wildly overstated it.

- **26 of 200 documents changed**; 174 byte-identical.
- **No page count moved on any of the 200.**
- **1 verdict to pass, 0 to fail.** Track total **159 → 160**.
- Summed |word error| over the 26 that changed: **2363 → 1895**, an improvement of 468.

| document | before | after |
|---|---|---|
| `ABCD-SDE-23-00 … Avionic System Description` | WORDS, +391 | **pass**, +57 |
| `ABCD-WB-08-00 … Weight and Balance` | WORDS, +117 | WORDS, +57 |
| `ABCD-FE-01-00 … Flight Envelope` | PAGES, +220 | PAGES, +150 |
| 23 others | unchanged verdict, 22 of them already passing | |

**One document's word error got worse and the fix is still right on it.**
`airbus-pdf-information-package_v1-4` went from −22 to −27 against the reference. Its page 1 had
**+5 surplus tokens before the fix — "Document available in English (only)" — and matches the
reference exactly after it.** The total moved the wrong way because the document is separately 59
words short on page 9, so removing correct surplus uncovered an unrelated shortfall. This is the
one case that could have looked like the truncation over-firing, and it is the opposite.

`ABCD-WB-08-00` misses by 2 words against a 55.2-word band. Its residual +57 is entirely other
defects, none of them text boxes: heading numbers off by one or two (`1.` where the reference has
`0.`, `6.1` where it has `7.1`), subscript tokenisation (`WMEW` as one token against the
reference's `MEW` + `W`), and a page-10 diagram whose text fragments differently.

## Regression

The 174 byte-identical documents include every `done-*` document that did not change, so their
verdicts cannot have moved; the 26 that changed were re-verdicted individually. Per group:

```
done-001 … done-015   10 -> 10 passes each
done-016              9  ->  9    (the group holds 9 documents, not 10)
extra-001             0  ->  1
every failure group   unchanged
```

**All 159 `done-*` documents pass, before and after.** No regression.

## Test counts, every project run individually

| Project | Failed | Passed | Skipped | Total |
|---|---:|---:|---:|---:|
| Containers | 0 | 109 | 0 | 109 |
| Core | 0 | 332 | 0 | 332 |
| Markup | 0 | 259 | 0 | 259 |
| OpenDocument | 0 | 125 | 0 | 125 |
| Presentations | 0 | 679 | 0 | 679 |
| Rendering | 0 | 150 | 1 | 151 |
| Spreadsheets | 0 | 770 | 0 | 770 |
| Text | 0 | 349 | 0 | 349 |
| Vector | 0 | 295 | 0 | 295 |
| WordProcessing | 0 | 834 | 0 | 834 |
| **Fidelity** | **30** | **520** | **0** | **550** |

Fidelity is at the baseline count as well as the baseline colour, so it is not a truncated run.
WordProcessing is 827 + the 7 new tests.

### The new tests fail against the unfixed tree

`TextBoxOverflowTests`, over `tests/corpus/features/textbox-overflow.docx` — three boxes holding
the same six paragraphs and differing only in height and autofit. LibreOffice's own PDF of it
extracts `BOXA0 BOXA1 BOXA2 BOXB0 BOXC0…BOXC5`, which is the 3 / 1 / 6 the tests assert.

Run against the reverted tree, the three behavioural tests give **Failed 2, Passed 1** — the
truncation cases fail and the `spAutoFit` control passes, which is what it must do, since a tree
that truncates nothing draws all six. The four API-level tests cannot be compiled against the
unfixed tree at all, by construction. Against the fixed tree all seven pass.

## Prediction scored

7 of 9. Both misses are on the same axis: I underestimated how much of the ABCD documents'
surplus was this one defect.

| # | Claim | Conf | Outcome |
|---|---|---:|---|
| P1 | `WB-08` flips to pass | 45% | **refuted** — +57 against a 55.2 band, short by 2 |
| P2 | `SDE-23` does **not** flip | 60% | **refuted** — it did; the dot-leaders were a smaller share than I thought |
| P3 | `FE-01` does not flip | 85% | correct |
| P4 | Neither `UG.CAO` moves | 90% | correct — both byte-identical |
| P5 | The two `.doc` files are misclassified | 80% | correct |
| P6 | 10–30 change ink, 1–3 verdicts, net non-negative | 50% | correct — 26, 1, +1 |
| P7 | No `done-*` regression | 70% | correct |
| P8 | Fidelity stays 30 / 550 | 75% | correct |
| P9 | New `PageFrame` property + one truncation site, no `OoxmlXml` change | 80% | half — `OoxmlXml` untouched as predicted, but the `bodyPr` insets were needed and not foreseen |

The refutation criterion written into `prediction.md` was "if more than a couple of documents lose
words the truncation is amplifying a metric error". One did, and it turned out to be correct on
inspection. The criterion did not fire.

## Two findings outside the brief

### 1. We draw no fill and no outline on a DrawingML text box

Found by two blind reviewers reading crops of `ABCD-WB-08-00` page 2 and `ABCD-SDE-23-00` page 4
independently, neither told anything but "describe these two halves". Both reported the same
thing: the reference draws a grey header panel, a bordered box round the "Company logo"
placeholder and a solid grey bar behind `Document reference:`, and **we draw none of them** — so
that bar's white text lands on white paper and is invisible in our rendering.

Confirmed in the PDF's operators rather than in the raster, per the skill's own warning. On page
2 the reference has **9 transparency-group XObjects and 6 strokes**; we have **0 and 4**. The
XObjects are the alpha fills — `header1.xml`'s box 0 carries
`<a:solidFill><a:srgbClr val="000000"><a:alpha val="50000"/>`, which LibreOffice's PDF export
writes as a transparency group rather than as a plain `re f`, which is why a fill-only grep of
the content stream finds nothing. `PageFrame` has `Fill`, `BorderColour` and `BorderWidth`;
`DocxFrames` sets none of them.

This is a **missing** defect sitting inside an **extra** group, on the same documents and in the
same header. It costs no gate column — both renderings agree on the words — which is precisely
the blindness CLAUDE.md warns the gate has, and it took two people looking at a page to see it.

### 2. `libreoffice-math` is not installed, so every reference draws nothing for OMML

The brief says `ABCD-FE-01-00` "draws ~40 OMML equation tokens on page 7 where the reference
draws no ink at all". The ink claim is right and the page claim is not — the reference draws 200
words on that page. But the cause is not an XRay.Office defect and not a LibreOffice one:

```
dpkg -l | grep libreoffice   →  writer, calc, impress, draw, base-core, core … and no math
/usr/lib/libreoffice/program/  →  no smlo, no libsmlo.so
/usr/lib/libreoffice/share/registry/  →  no math.xcd
```

`probe-omml-reference.py` authors a document holding one `m:oMathPara` between two ordinary
paragraphs. The reference's text layer is `BEFOREEQUATION AFTEREQUATION` and its ink is two bands
— the two paragraphs. The equation's paragraph **reserves its vertical space and draws nothing**,
which is what an embedded object that cannot be instantiated looks like.

`ABCD-FE-01-00` holds 33 `m:oMathPara`, and a row-by-row ink scan of page 7 finds four bands
where we draw and the reference does not. That is the bulk of its residual +150.

**This is the same class of gap as the missing `fonts-dejavu-core` that CLAUDE.md documents at
length: an undeclared input to the gate.** The installed module set decides the reference as
surely as the font set does, and the install line in CLAUDE.md names
`libreoffice-writer libreoffice-calc libreoffice-impress` and no `math`. Every banked reference
for an equation-bearing document is measuring a LibreOffice that cannot render equations, and any
round that "fixes" our equation rendering to match will be fitting to that.

Deliberately **not installed here.** Installing it would change the reference for every
equation-bearing document in the corpus while other agents are mid-measurement against the banked
set, and re-banking is a decision for whoever owns that artefact. It should be installed, and the
references re-banked, the same way DejaVu was.

## Files

```
src/XRay.Office.WordProcessing/Layout/PageFrame.cs          HasFixedHeight
src/XRay.Office.WordProcessing/Layout/FlowLayouter.cs       Truncated
src/XRay.Office.WordProcessing/Layout/FrameLayout.cs        the call site
src/XRay.Office.WordProcessing/Ooxml/DocxFrames.cs          bodyPr insets and spAutoFit
tests/XRay.Office.WordProcessing.Tests/TextBoxOverflowTests.cs
tests/corpus/features/textbox-overflow.docx               authored by make-fixture.py
probes/words-extra-01/                                    every probe above, runnable
```
