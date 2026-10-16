# `PES-Technical-Report-Template_Jan_2019.docx` — the cover art one page late, and the two things beside it

Measured 2026-09-07 in `/home/user/wt-wordsgap`, branch `agent/wordsgap`, base `f4b8c3825`
(`2e39d5fe8` plus one docs commit).

| | |
|---|---|
| ref26 | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2 0229ac93fcf0d7cb**, its eight Latin duplicates, its Latin Noto and its four `LiberationSansNarrow` all in `.duplicates-aside/` |
| ours | `XRay.Office.Cli` built from this worktree, snapshotted to a fixed directory per leg so no rebuild could swap it mid-measurement |
| gate | `/home/user/gate-2f47/` at `2f4709c08`, reference half reused; scored with `words-ink-r67/score.py`, which replays `batch-check.sh`'s rule as of 2026-09-05 |
| corpus | `/home/user/sample-files`, words track, **338 documents** |

`gen.py` writes the probe documents, `measure.sh` renders a directory both ways and reports page
counts, `render-ours.sh` renders the words track with one CLI.

---

## The finding

The brief named two candidates and **neither is it**.

- *An extra section-1 page from `w:titlePg` with no first-page header.* Refuted as the cause of the
  extra page — though `w:titlePg` turns out to be a real and separate defect, below.
- *A frame at `posOffset` 733.5 pt on a zero-margin 792 pt page forcing a page.* Refuted twice over.
  That item is `558 × 40.5 pt` at 733.5, so it ends at 774 pt and fits; and it is `<wp:wrapNone/>`,
  so it is no obstacle at all. More generally **our overflow rule already matches Writer's**: four
  authored cases, page counts and image placement identical to 26.2.4.2 in all four —

  | probe | ours | ref26 | image |
  |---|---|---|---|
  | `pic-first-fits` (612.5 × 700 pt, body 792) | 1 | 1 | p1 at 92.00 / 92.05 |
  | `pic-first-over` (612.5 × 792.65) | 2 | 2 | p1, hanging to −0.65 / −0.60 |
  | `pic-first-huge` (612.5 × 1200) | 2 | 2 | p1, hanging to −408.00 / −407.95 |
  | `pic-second-over` (a paragraph before it) | 3 | 3 | **p2** in both |

  That is `SwFlowFrame::IsFwdMoveAllowed`, which is `GetIndPrev() != nullptr`
  (`sw/source/core/inc/flowfrm.hxx`:243-246) and makes `MoveFwd` refuse for the body's first frame
  (`sw/source/core/layout/flowfrm.cxx`:2116-2145); and `WidowsAndOrphans::FindBreak`, which returns
  false when the break lands on line 1 because `PrevLine()` fails (`widorp.cxx`:423-441).

**It is the anchor character.** Section 1's body is two paragraphs: one carrying four `wp:anchor`
runs and then one `wp:inline` picture of `7778812 × 10066699` EMU = **612.5 × 792.65 pt**, and one
empty paragraph holding the `w:sectPr`. `DocxLayoutSource`'s walk emitted one `AnchorCharacter`
(`U+0001`) per `w:drawing`, floating or inline. A control character is zero-width
(`ShapingControls`), so what the four cost was a **break opportunity**: an inline object widens every
prefix *past* the boundary it sits at, so a picture wider than the measure is placed anyway on a line
it starts and moves to the next line when anything precedes it. With the anchors emitted, the picture
took line 2 and therefore page 2; every page from 2 to 13 then ran one behind the reference's.

Writer inserts that character for `FLY_AS_CHAR` alone: `SwFormatFlyCnt` — and the
`CH_TXTATR_BREAKWORD` `GetCharOfTextAttr` gives `RES_TXTATR_FLYCNT`
(`sw/source/core/txtnode/thints.cxx`:3633-3652) — is put in by `SwDoc::SetFlyFrameAnchor`
(`sw/source/core/doc/docfly.cxx`:337-348) only on that branch. An `FLY_AT_CHAR` fly carries a
`SwPosition` on its `SwFormatAnchor` and nothing in the node's text.

Bisected on the document itself, by deleting runs from paragraph 1 (`pesvar`):

| variant | ours | ref26 |
|---|---|---|
| all five runs | image p2 | image p1 |
| the picture run alone | image p1 | image p1 |
| picture + the 733.5 pt box | image p2 | image p1 |
| picture + the 234 pt box | image p2 | image p1 |
| the picture run moved to the head of the paragraph | image p1 | image p1 |
| picture narrowed to 600 pt (fits the measure) | image p1 | image p1 |
| picture shortened to 700 pt (fits the body) | image p1 | image p1 |

## The second defect on the same document, which the first exposed

With the picture on page 1 the document rendered **13 pages against 14** — the old 14 was two
offsetting errors. The second is `Heading9`, the file's bullet style: it is declared tenth,
`w:basedOn` `ListParagraph` declared 114th, and `ListParagraph` carries `<w:contextualSpacing/>`.
26.2.4.2 does **not** inherit that flag; we did, and suppressed the style's own `w:after="240"`
between every pair of bullets — 12.00 pt each, measured as a baseline pitch of 13.80 pt against the
reference's 25.80.

Not a contextual-spacing bug in general: over four authored cases (states it, inherits it, states
nothing, inherits it under a built-in `w:name`) we agree with 26.2.4.2 exactly. It is
**declaration order**, and it is the read-modify-write `OneSidedStyleSpacingTests` already models
from the margin side — both margins and the flag are one `SvxULSpaceItem`, so a style that sets a
margin replaces the whole item with one resolved at that moment, and a parent declared later has
contributed nothing yet. Isolated with one document rendered twice, differing only in where the
parent style sits (`ctx-lpfirst.docx`, `ctx-lplast.docx`; three paragraphs per style at 10 pt, so a
suppressed gap is a pitch of 11.50 and an honoured `w:after="240"` is 23.50):

| child style | parent first | parent last |
|---|---:|---:|
| sets `w:ind` only | 11.50 | 11.50 |
| sets `w:spacing w:before="0"` | 11.50 | 11.50 |
| sets `w:spacing w:after="240"` | 11.50 | **23.50** |
| the parent itself | 11.50 | 11.50 |

The `w:ind` row is the control: the child takes the parent's 1440-twip indent under both orders, so
`w:basedOn` is intact and this is one item's flag.

**Reach: 1 of 272 corpus DOCX** — PES alone (`census-ctx.py`'s shape: a paragraph style that sets a
`w:spacing` margin, does not state `w:contextualSpacing`, and has an ancestor that does and is
declared later).

## A third defect, measured and left

`w:titlePg` with a header (or footer) declared and **no `first` one** leaves the section's first page
an *empty* header — drawn as nothing, and still taking the height of one empty paragraph in the
document's `header`- (`footer`-) named paragraph style. writerfilter chooses that over turning the
header off: `SectionPropertyMap::CloseSectionGroup` sets `PROP_HEADER_NO_FIRST` /
`PROP_FOOTER_NO_FIRST` rather than `HeaderIsOn = false` when some other slot is on
(`sw/source/writerfilter/dmapper/PropertyMap.cxx`:596-618). We give the first page the whole body.

Measured on authored documents with every margin nought, `BODYTOP`'s baseline on page 1:

| probe | ours | ref26 |
|---|---:|---:|
| `hdr-none` (no header at all) | 781.50 | 781.50 |
| `hdr-default` (a header, no `titlePg`) | 768.05 | 768.05 |
| `hdr-titlepg` (a header, `titlePg`, no first) | **781.50** | **768.05** |
| `hdr3-titlepg` (a *three-line* header, same) | **781.50** | **768.05** |
| `hdr-titlepg-first` (both named) | 781.50 | 781.50 |

The `hdr3` row is what says it is an empty paragraph and not the default header's height. Which
paragraph it is, from two documents differing only in style sizes: with `Header` at 40 pt over a 20 pt
`Normal` the reserved space is 46.00 pt, and with `Header` at 20 pt over a 40 pt `Normal` it is 23.00
— `size × 1.14990`, Liberation Sans, both times the **`Header` style**. `disamb-hdr` puts the default
header's own paragraph in a third style at 30 pt and the answer is still the `Header` style's 40.
Footers behave identically (`ftr-titlepg` fits `L056` on page 1 where `ftr-none` fits `L057`), and
naming a first-page *footer* does not excuse the header (`firstftr-only`, 736.60 in the reference).

**Reach: 12 of 272 corpus DOCX** for the header case and 12 for the footer case (`census-titlepg.py`).
It only moves a page where `w:header + one empty line > w:top`, which is PES's zero-margin section and
not `hdss-bulletin-issue-285`'s (`w:top` 1418, `w:header` 851). **Left unfixed** — it is worth a round
of its own, and on PES it moves the cover art 12.65 pt rather than a page.

## What moved

Words track, 338 of 338, our half re-rendered with each binary and scored against
`/home/user/gate-2f47/parity.tsv`:

```
before   TOTAL 338  MATCH 314  PAGES 20  PAGES,WORDS 2  WORDS 2
after    TOTAL 338  MATCH 314  PAGES 20  PAGES,WORDS 2  WORDS 2
```

Identical, and the same three rows differ from the bank in both legs (they are the merges between
`2f4709c08` and this base, not this change). **12 of 338 renderings changed**, and their `|ink|%`
against 26.2.4.2, summed per document over `pdf-image-diff.py`'s own column at 512 px:

| document | pages b/a/ref | before | after |
|---|---|---:|---:|
| `PES-Technical-Report-Template_Jan_2019.docx` | 14/14/14 | **35.91** | **2.37** |
| `SPA-06_mcar_part-6_and_IS_v2.9.docx` | 85/85/85 | 11.74 | 11.43 |
| `docs-quality-MA.IMS.00001-…manual.docx` | 44/44/44 | 10.48 | 10.39 |
| `Agile_Arc_SysDes.docx` | 20/20/20 | 2.42 | 2.53 |
| `Form-SM-76A-…-11.docx` | 3/3/3 | 1.50 | 1.50 |
| `097_Business_Case_Template_Elegant_Layout.docx` | 2/2/2 | 0.44 | 0.05 |
| `091_Business_Case_Template_Complete_Guide.docx` | 5/5/5 | 0.30 | 0.25 |
| `Press release_EUREKA labels ITEA 3 Cluster.docx` | 2/2/2 | 0.18 | 0.18 |
| `096_Business_Case_Template_Editable_Layout.docx` | 1/1/1 | 0.16 | 0.18 |
| `template---tpr-…-with-guidance.docx` | 7/7/7 | 0.13 | 0.14 |
| `035_Venn_Diagram_Template_Editable_Format.docx` | 1/1/1 | 0.04 | 0.04 |
| `037_Venn_Diagram_Template_Four_Circle.docx` | 1/1/1 | 0.01 | 0.05 |
| **total** | | **63.31** | **29.11** |

35.91 is `words-ink-r67/rank-after.tsv`'s own figure for PES to the hundredth, so this reproduces
that ranking's instrument before moving it. The four documents that rose did so by 0.11, 0.04, 0.02
and 0.01.

## Reproducing

```sh
python3 gen.py /abs/probe                                   # the authored documents
./measure.sh /abs/probe /abs/out <cli>                      # both engines, page counts
./render-ours.sh <cli> /abs/ours /abs/words-paths.txt 6     # the track
python3 ../words-ink-r67/score.py /abs/ours /home/user/gate-2f47 words/
```
