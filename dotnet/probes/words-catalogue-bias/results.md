# The catalogue's residual is not noise: an inline shape's line is one twip short

Measured 2026-09-06, worktree `wt-frames` at `a605d98df`, against
`/opt/libreoffice26.2/program/soffice` (**26.2.4.2**, TDF tarball with its Latin font duplicates moved
aside). Document `words/drawingset-001/docx/WordArt_Shapes_Arrows_Catalog1.docx`, 52 pages, 340
inline `wps:wsp` shapes.

## The claim under test

After `probes/words-inline-shape-ink/` closed the effect-extent half, the catalogue still read
`displaced-vertical` on 43 of its 52 pages while `pdf-ops` found **no one-sided record at all** —
every fill and stroke present on both sides, within 0.73 pt, mean 0.38. The round that measured that
called it *sub-point placement noise on the twip grid* and stopped.

**It is not noise.** Pairing every fill and stroke ours draws against the reference's — same kind,
same size to within a point, nearest anchor within 3 pt — over the whole document:

| | n | mean | mean abs | + / − / 0 |
|---|---:|---:|---:|---|
| horizontal offset | 466 | +0.0009 pt | 0.0071 | 54 / 31 / 381 |
| **vertical offset** | 466 | **+0.2059 pt** | 0.2076 | **418 / 11 / 37** |

The horizontal offset is 1.1 standard errors from zero, which is what noise looks like. The vertical
one is **28.8** standard errors from zero and one-signed 418 to 11. The "0.73 pt worst, mean 0.38" of
the earlier reading is not scatter either: it is a *cumulative* drift. Down page 48, row by row, our
records run 0.0, 0.6, 1.2, 1.6, 2.2, 2.6, 3.0, 3.6, 4.0, 4.6, 5.0, 5.6, 6.0, 7.0, 7.2, 7.6 … 14.6
twips above the reference's — one twip per row, fifteen rows, 14.6 twips by the bottom.

Reading the row pitches off page 48 says the same thing outright. The reference's are whole twips and
ours are not, and ours are each one twip short:

```
26.2.4.2  855  859  861  859  856  864  855  859  861  859  856  864  855  859
ours      853.8 858.0 860.2 858.0 855.0 863.0 853.8 858.0 860.2 858.0 855.0 863.0 853.8 858.0
```

## The rule, and the control that scopes it

Two fixtures, ten identical paragraphs each holding one inline object, nothing else on the page, so
the pitch between the objects *is* the line height. They differ in one thing: what the
`a:graphicData` holds.

| stated height `cy`, twips | 560 | 561 | 562 | 563 | 564 | 565 | 566 | 567 | 568 | 569 | 570 | 571 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `wps:wsp` shape, 26.2.4.2 − ours | +1 | +1 | +1 | +1 | +1 | +1 | +1 | +1 | +1 | +1 | +1 | +1 |
| `pic:pic` picture, 26.2.4.2 − ours | 0 | 0 | 0 | 0 | 0 | 0 | — | — | — | — | — | — |

**A shape's line is one twip taller than the shape; a picture's is exactly the picture.** The picture
is the control that says the twip belongs to the draw layer and not to every inline object: a picture
is a fly holding a `SwNoTextFrame` and its size is its size, while a shape is an `SdrObject` whose
rectangle is inclusive — `SwRect::Bottom() == Top() + Height() - 1`.

**Below the baseline, not above it.** In the same fixture the *first* shape on the page is drawn at
785.19 pt by the reference and 785.20 by us, so the extra twip is behind the object rather than in
front of it. The fix therefore resolves the ascent to the unpadded height and lets the padding fall
into the descent; leaving `InlineObject`'s default would take the padded height as the ascent and move
the drawing down with it.

**A form check box is excluded, and that is the mechanism rather than an exception.** Writer strokes
one in `SwTextPaintInfo::DrawCheckBox` while painting the line; it never becomes an `SdrObject`, so it
never inherits the inclusive rectangle. It is a `PageFrame` here only because that is the vehicle this
model has for something that takes room on a line and draws geometry, and `PageFrame.IsTextPortion`
now says so. `FormCheckBoxTests.TheBoxReservesItsWholeSquareOnTheLine` is what holds that: it asserts
the box is square, and it failed the moment the twip reached it.

## After

The catalogue, same pairing:

| | before | after |
|---|---:|---:|
| mean vertical offset | +0.2059 pt | **+0.0067 pt** |
| mean absolute vertical offset | 0.2076 pt | **0.0147 pt** |
| sign split, + / − / 0 | 418 / 11 / 37 | **148 / 51 / 267** |
| mean horizontal offset (control, unchanged) | +0.0009 pt | +0.0009 pt |

The vertical residual is now the same order as the horizontal one, which is where a genuine sub-twip
noise floor should sit.

## Reach

Twenty-one corpus documents carry an inline shape — `reach-set.txt` is the census, every `words`
document with a `wp:inline` holding a `wps:wsp` or a VML shape, plus the ODF as-char custom shapes.
Rendered before and after and paired against 26.2.4.2:

| | |
|---|---:|
| documents whose page count moved | **0 of 21** |
| documents whose line breaks moved | **0 of 21** |
| documents whose paired vertical offset moved | **4 of 21** — 3 better, 1 worse by 0.0005 pt |
| weighted mean absolute vertical offset, 4728 paired records | **0.2628 → 0.2427 pt** |

```
WordArt_Shapes_Arrows_Catalog1.docx                  0.2076 -> 0.0147
DOA_Template_Form_Type_Certification_Programme.docx  0.9169 -> 0.9011
PI-doc.-no.-2E-Technical-Review-Report.docx          0.0744 -> 0.0728
docs-quality-MA.IMS.00001-…                          0.2548 -> 0.2553
```

Most of the twenty-one are unchanged because one inline shape in a document has nothing to accumulate
into; the catalogue moves because its rows *are* inline shapes, fifteen to a page.

## Scripts

* `bias.py` — pairs every fill and stroke and reports the signed offsets, per document and per page.
* `pitch-fixture.py` — ten paragraphs of one inline `wps:wsp`, swept by the shape's stated height.
* `pitch-picture.py` — the same sweep with a `pic:pic`, which is the control.
