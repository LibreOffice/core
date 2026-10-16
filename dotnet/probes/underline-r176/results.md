# r176 — a `w:u` stating no `w:val` is not an underline

`wordsink-r174`'s ink ranking put `f445896eb008d14c1746fc37d412dc22.docx` among the passing
documents at **1.951 per page with 14 of its 15 pages MAJOR** — the worst per-page figure on the
words track, on a document the gate calls `match`.

## What the page shows, and what it does not

Page and text are identical: **79 spans on both sides, the same three faces, the same sizes, the
same line breaks, the same eight strokes.** The only difference is that we draw **108 filled
paths per page and the reference draws none**, every one a black rectangle 1 pt tall and 2–48 pt
wide. Those are underlines.

A blind reading of the page (`page-vision`, no access to the repository) supplied the control that
makes it unambiguous, and it is a control the counts could not: *"the left column … timestamps and
bulleted shot descriptions ARE underlined — same as the left half"*, while the right column is
underlined only in ours. So it is not that we underline everything; it is that we underline one
class of run too many.

## The rule

`CT_Underline`'s `val` is **optional with no default** —
`sw/source/writerfilter/ooxml/model.xml`:17021-17027 declares five attributes and gives none a
default. An element carrying only a `w:color` therefore emits no `LN_CT_Underline_val` sprm at
all: `DomainMapper::lcl_sprm`'s case for it (`dmapper/DomainMapper.cxx`:364) never fires, and the
sibling at `:368` takes the colour and nothing else.

So such an element is **transparent** — a third answer beside "underline" and "no underline". A
run under it keeps whatever an outer layer stated, and a run under nothing gets no underline.
`WordCharacterFormat.UnderlineOf`'s own remarks said the opposite: *"an unstated `w:val` is one
too: the attribute defaults to `single`"*. That is withdrawn.

This document states `<w:u w:color="000000"/>` on four of its styles and `<w:u w:color="666666"/>`
on a run, and no `w:val` anywhere but the two runs the left column uses.

Reading it correctly means the resolution has to see **every** layer rather than the innermost
`w:u`, which is why `UnderlineOf` now takes `WordStyles.RunPropertyLayers`' list.

## Reach and cost

**1070 valueless `<w:u>` in 13 of the 272 corpus DOCX**, against 2665 that state a value.

Rendering all 337 words documents at the round's base and again after: **8 move, 329 are
byte-identical**, and no gate verdict changes either way — an underline adds no character and no
page, which is why this sat at the top of an ink ranking and nowhere on the scoreboard.

| document | \|ink\| before | after |
|---|---:|---:|
| `f445896eb008d14c1746fc37d412dc22.docx` | **29.26** | **0.24** |
| `ESPN-R - MCF - Manual - Ed1.0` | 9.12 | **1.71** |
| `t_TEMPforInvProgs.docx` | 5.01 | 4.67 |
| `33004.docx` | 1.07 | 0.72 |
| `b053-19.docx` | 0.49 | **0.02** |
| `b050-19.docx` | 0.22 | 0.09 |
| `OM template …` | 22.41 | 22.43 |
| `150-5370-10H.docx` | 45.63 | 45.65 |

Summed over the movers **113.21 → 75.52**; the two that rise do so by 0.02, which is a rule now
correctly absent from a page whose other differences dominate it.

The other two tracks cannot move: `WordCharacterFormat` and `WordParagraphFormats` are the DOCX
readers, and the WW8, RTF, ODF, slide and sheet paths each have their own underline reading.

## What is left on those documents

`t_TEMPforInvProgs` (4.67) and `ESPN-R` (1.71) still carry a residual and both hold hundreds of
these elements, so whatever remains there is not this rule — check the drawn rule *positions*
rather than their presence before assuming otherwise.
