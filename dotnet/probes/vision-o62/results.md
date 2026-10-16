# O62 confirmed, and the blind reading got it wrong first — at a measurable resolution

Round 117 seated **O62**: 26.2.4.2 strokes a Calc cell's hyperlink underline and this tree draws
nothing. It named `084_Service_invoice_Use_this_template` page 1 as the page deserving an
independent reader. The parent ran one, twice, and the pair is a clean calibration of the method.

## The measurement, which settles the seat

`underline.tsv`, read out of both PDFs:

| leg | | x0 | y0 | x1 | y1 | |
|---|---|---|---|---|---|---|
| ours | text | 83.8 | 234.1 | 193.5 | 245.7 | 10 pt, `#000080` |
| ours | **stroke** | — | — | — | — | **nothing in the band** |
| ref | text | 83.8 | 234.1 | 193.3 | 245.7 | 10 pt, `#000080` |
| ref | **stroke** | 83.8 | 243.8 | 193.3 | 243.8 | **0.51 pt, `#000080`** |

Same text, same place, same size, same colour. The reference adds an underline; we draw none.
**O62 stands.**

## Reading one: the full page, at 117 dpi — wrong, and confidently so

The pair was composed side by side (the higher-resolution arrangement for portrait) at the largest
dpi the 2000 px budget allows, **117**. The reviewer went block by block, said it had looked
specifically for marks present in one half and absent in the other, and reported:

> The blue underline under "jordan@example.com" is present in both.

It is not. That is a **false positive on a mark that is not there.**

To its credit the same reading carried its own warning — *"all hairlines are one pixel at this
scale… if a small missing mark is the thing being looked for, it would need a crop at several
times this resolution to be visible at all"* — and I nearly took the presence claim at face value
anyway.

## Reading two: a 600 dpi crop of the same block — exactly right

Same method, same kind of fresh reviewer, one crop of the address block:

> under the email line, nothing in the top half and one solid underline in the bottom half …
> roughly 4–5 px thick … continuous, with no gaps where the descenders of "j" and "p" cross it

4–5 px is right: 0.51 pt at 600 dpi is **4.25 px**. It also named the candidate causes correctly
and ruled one out on its own evidence — *"a hyperlink character style whose underline component is
not applied while its colour is (the navy is clearly right in both, so colour resolution is
working)"* — and noted that an image cannot separate "absent" from "drawn in white".

## The lesson, with the arithmetic

`page-vision` already says that an **absence** reported by a reviewer may be a fact about the
pipeline rather than the document. The symmetric case is what bit here and was not written down:
**a *presence* claim below the resolution limit is equally unreliable, and it is more dangerous,
because it reads as "no defect here" and closes the investigation.**

The arithmetic is the same as the skill's px-per-em rule, applied to a rule instead of a glyph. A
hairline needs roughly **3 px** to be judged present or absent, so it needs

    dpi ≈ 216 / width_in_points

and the viewer's 2000 px budget caps a full page at 117–170 dpi. **So any rule thinner than about
1.3 pt cannot be judged from a full-page pair at all** — at 117 dpi this 0.51 pt underline is
0.83 px. Crop it, or do not ask the question.
