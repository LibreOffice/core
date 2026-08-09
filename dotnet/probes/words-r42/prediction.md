# Words round 42 — predictions, committed before anything was rendered post-change

Baseline reproduced at `4efef703e`: **154/200**, absolute page error **78**, **164** exactly-correct
page counts, absolute word error **6427** (`probes/words-r42/baseline.tsv`).

Two changes, both found by taking the `box` cluster apart rather than by believing it.

## Fix A — a frame anchored in a furniture table was never placed

`FrameLayout.FlowsOn` yielded the header, the footer and the notes area, and then walked
`page.Tables`. A `PlacedFlow` carries its own `Tables`, and those were not walked — so a picture
anchored in a *cell of a header table* had no rectangle resolved and was drawn nowhere. The
doc comment on the method already claimed "the cells of every table, however deeply the tables
nest", which is what it now does.

## Fix B — a page field inside a text box was never resolved

`PageFields.Carries`/`Resolve` descended into a `PageTable`'s cells and not into a paragraph's
`Frames`. `Carries` is also what decides whether a running head may be cached across pages, so a
footer whose `PAGE` field sits in a `txbxContent` was laid out **once** and every page got page
one's copy. Measured on `UG.CAO.00133 … Language.docx`: eighteen pages all printing
`Page 1 of 18` where the reference prints 1 to 18 — which is the defect the user reported as
*"Libre office shows page 8 and 1. While we show page 1 and 2."*

## Reach

The census is a **ceiling over the formats it can read, which is 134 of the track's 200**; the 66
`.doc` keep their header stories in the WW8 text stream where no zip-level census can see them.
`probes/words-r42/furniture-frame-census.py`:

| population | census |
|---|---:|
| header/footer part with a `w:tbl` holding a `w:drawing` or `w:pict` | 26 |
| header/footer part with a `PAGE`/`NUMPAGES` instruction inside a `txbxContent` | 11 |
| both | 8 |

The table census over-counts on purpose: it accepts a legacy `w:pict`, and the DOCX run walker
makes a frame anchor for `w:drawing` only, so a VML-only header picture is not reached by fix A
at all.

**Predicted measured reach, by rendering the track twice and byte-comparing with
`SOURCE_DATE_EPOCH` pinned: 18–30 of 200 renderings.**

## Verdict movement

**Predicted: zero verdicts move. 154/200, page error 78, exact pages 164.**

Neither change adds or removes a word. A logo is not text; `Page 1 of 18` and `Page 8 of 18` are
four tokens either way. Nor should either change pagination: an as-character frame already
contributes its height at measure time in `FlowLayouter`, so the header was already as tall as it
should be — only the ink was missing.

Word error is predicted **unchanged at 6427**, with the caveat that a page number is one token
whose *glyphs* change, and `pdftotext` can join or split a token differently: a movement of a few
words either way would not falsify this, a movement of tens would.

`UG.CAO.00133 … Language.docx` is predicted to stay a **failure**. It is 3944 words against 3700,
and 244 of that surplus is a third defect these fixes do not touch: we draw the section-0 running
head on all eighteen pages where LibreOffice draws it on five.
