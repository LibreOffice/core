# r171 — the OMML formulas in a deck are owed and cost nothing

Round 165 gave `XRay.Office.Ooxml/OfficeMath/OfficeMathBox` the vertical half of `SmNode::Arrange`
and wired it into `DocxLayoutSource`; `PptxTextBody` does not call it, and that was recorded as
owed from the round.

**Measured, it is owed and worth nothing.** Three of the 251 corpus decks carry an `m:oMath` — 67
formulas in `WiGr_2021W_1_Angebot-Nachfrage-Elastizität`, 3 in `RPA P4 - Advanced Material`, 1 in
`Structural Testing` — and all three are already at the reference:

| deck | pages | glyphs | spans | mean \|dx\| | mean \|dy\| |
|---|---|---|---|---:|---:|
| `WiGr_2021W_1_…Elastizität` | 51 of 51 | 13011 / 13007 | 936 / 936 | 0.52 | 0.94 |
| `Structural Testing` | 89 of 89 | 20363 / 20363 | 1850 / 1853 | 3.11 | 0.71 |
| `RPA P4 - Advanced Material` | 20 of 20 | 8749 / 8744 | 577 / 576 | 4.67 | 1.73 |

The formula text is drawn, it is in the reference's own text layer as often as in ours, and every
paired span sits within a couple of points of where the reference puts it.

**Why the height cannot matter here is structural, not lucky.** `OfficeMathBox` exists because a
formula's height decides where the *next line* falls and therefore where the page breaks. A slide
has no pagination: its page count is its slide count, so the only thing a formula's height could
move is an autofit's shrink factor or a box's vertical anchor — and on these three it moves
neither far enough to displace a span by more than 4.67 pt of `|dx|`, which is a wrap difference
rather than a height one.

So the item is closed by measurement. Should a deck ever turn up whose formula overflows its
placeholder, `SlideAutofit` is where the height would have to reach, not `SlideTextBody`.
