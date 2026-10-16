# r172 — a VML shape inside a `v:group` loses nearly all of its paragraph formatting

Task 14 was *"place a VML group's shapes where the reference places them"*, on the strength of a
mean `|dx|` of 55 to 264 pt between this tree and 26.2.4.2 over the five
`06x_Work_Breakdown_Structure_Template` documents.

**That figure was the instrument's, not the tree's, and the placement is exact.** Those templates
repeat `SUBTASK` thirty times, so pairing spans by string pairs a label against an unrelated one.
Every shape of all five carries a fill, so the honest instrument is the ink: matching *filled
rectangles* instead (`../vmlgroup-r172/rects.py`) gives **29/29, 21/21, 41/41 and 22/22 boxes
agreeing with 26.2.4.2 to 0.1 pt**. The 067 count reads 26 against 51 only because the reference
draws thirty extra 5.95 pt square markers there, which is a separate, known class.

What is left once placement is exact is where the text sits *inside* each box, and that turned out
to be one rule.

## The rule

**A VML shape at the top level of a `w:pict` becomes a Writer text frame and its `w:txbxContent`
goes through the whole paragraph machinery. The same shape inside a `v:group` cannot be a text
frame, so it becomes a drawing object whose text is EditEngine text — and almost none of the
paragraph formatting survives that.**

Measured on 26.2.4.2 over thirteen fixtures differing in one property, with a `w:docDefaults`
stating `w:spacing w:after="160" w:line="259" w:lineRule="auto"` — which is what
`069_Work_Breakdown_Structure_Template_Professional_Format` carries:

| property | top level | inside a `v:group` |
|---|---|---|
| `w:docDefaults` spacing | pitch **22.90** | pitch **13.80** — the bare face metric, no spacing at all |
| `w:spacing` stated on the paragraph | 22.90 | — |
| `w:ind w:left="1440"` / `w:firstLine="720"` | 79.3 / 43.3 | **7.2 / 7.2** — dropped |
| `w:numPr` | `1. Alpha` at 25.3 | `Alpha` at 7.2 — **the number is not drawn** |
| `w:jc w:val="center"` | the stating paragraph only | **and the paragraph after it** |
| `w:sz w:val="36"` | 18 pt | 18 pt |
| `w:b` | bold | bold |
| `w:rFonts` Liberation Mono | Liberation Mono | Liberation Mono |
| `w:i` | italic | **upright — dropped** |

Every top-level arm is reproduced by this tree exactly; every grouped arm is not, because this
tree runs the same machinery either way.

### The source predicted the leak, which is why it is worth trusting

`oox::vml::TextBoxContext` reads exactly two things out of a `w:pPr` — `w:jc` into
`moParaAdjust` and `w:pStyle` into `moParaStyleName`
(`oox/source/vml/vmltextboxcontext.cxx`:265-275) — and `TextBox::convert`
(`oox/source/vml/vmltextbox.cxx`:78-160) appends the portions with at most six character
properties and `ParaAdjust`. A paragraph break is itself a portion, appended as `"\n"` carrying
that paragraph's model (`onEndElement`, `:277-284`), so **the adjust lands on the paragraph the
break opens** and the next paragraph inherits it unless it states its own. That is exactly what
the fixture shows: `Alpha` centred, `Bravo` stating nothing and centred with it, `Charlie`
stating `left` and left, `Delta` left.

The character half of that source reading is *wrong* for Writer and the measurement says so —
`XML_b`/`XML_i` are the VML elements, not `w:b`/`w:i`, yet `w:sz`, `w:b` and `w:rFonts` all
survive while `w:i` does not. So writerfilter feeds the character properties in by another route
and the italic falls between the two. **The arms are the authority here, not the file:line.**

## A grouped `v:rect` is the same as a grouped `v:shape`

The obvious next suspect was vertical alignment — a custom shape's `SDRTEXTVERTADJUST` defaults to
centre where a text box's is top, which would explain `065`'s one box sitting 15.2 pt out.
It does not: `grouped-rect-spacing` draws its first line at 4.11 exactly as
`grouped-default-spacing` does. Both are top-aligned and both have the 13.80 pitch.

## Reach, and why nothing changed

**113 of the corpus's 166 reachable VML text boxes are inside a `v:group`, and all 113 are in the
five Work Breakdown templates.** (The other 53 are top-level, where the two engines agree.) The
1638-box figure a markup census gives is 1472 `mc:Fallback` halves that neither renderer reads —
see `../vmlinset-r171/results.md`.

All five are one page, all five are `match` on the gate after r171, and their summed unsigned ink
against 26.2.4.2 is 0.02 to 0.07. So **reproducing the reference here would move no gate row and
very little ink**, and it would mean dropping an author's numbering, indents, paragraph spacing
and italic on purpose. That is `TODO.word-parity.md`'s case exactly, and the entry is recorded
there: Word honours all of it, this tree honours all of it, and the difference is measured rather
than chased again.

The residual it accounts for, from `../vmlgroup-r172/inbox.py`, which pairs each span against the
reference's *within its own box*:

| document | spans | mean \|dx\| | mean \|dy\| | max \|dy\| |
|---|---:|---:|---:|---:|
| `068` | 26 | 0.10 | 0.60 | 0.80 |
| `066` | 24 | 0.04 | 1.34 | 1.60 |
| `067` | 17 | 0.04 | 1.48 | 1.60 |
| `065` | 10 | 1.62 | 4.20 | 15.20 |
| `069` | 66 | 24.93 | 27.14 | 63.70 |

`069` is the pitch: 23.10 against 14.00, which is `1.079 × 13.97 + 8` against the bare 13.97.

## One thing in `069` that is neither

Its `SUBTASK` is drawn 59.80 pt wide by the reference and 55.67 by us, in the same face
(DejaVu Sans, read out of both PDFs) at the same size, while `1.` is 11.40 against 11.45. That is
the **seventh confound** — 26.2.4.2 measuring on the draw layer in the class-ful family and
drawing in the class-less one — reaching a Writer document, which `CLAUDE.md` says body text
cannot show *because `SwTextPainter` never becomes a drawinglayer primitive*. A grouped VML shape
does. It is a LibreOffice defect and is not to be reproduced.
