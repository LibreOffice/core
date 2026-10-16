# r173 — a VML outline's arrowheads

`LineEnds.Apply` has turned a stroked path into a stroke plus a filled marker since the round that
read `a:headEnd`, and `DocxVmlFrames` set neither `PageFrame.HeadEnd` nor `PageFrame.TailEnd`, so
a VML connector was drawn as a bare line with no head.

Found while measuring the Work Breakdown templates (`../vmlgroup-r172`): on
`067_Work_Breakdown_Structure_Template_Gray_Theme` 26.2.4.2 drew **25 filled paths of about
5.95 pt square** in `#7F8FA9` that this tree drew not at all, beside 25 `_x0000_t32` straight
connectors.

## The rule

VML names five arrow shapes and DrawingML names the same five differently, so the reference
translates rather than reads — `lclGetDmlArrowType` (`oox/source/vml/vmlformatting.cxx`:599-611):

| VML | DrawingML |
|---|---|
| `block` | `triangle` |
| `classic` | `stealth` |
| `diamond` | `diamond` |
| `oval` | `oval` |
| `open` | `arrow` |

and the two size words at `:613-632`: `narrow`/`short` → `sm`, `medium` → `med`,
`wide`/`long` → `lg`. Anything else, `none` included, draws nothing.

It is on the `v:stroke` child and **never on the shape**, which is unlike every other outline
attribute the reader handles — `strokecolor` and `strokeweight` each have a shape-level spelling
as well.

## Reach is one document, and the census that says seventeen counts the wrong thing

177 VML stroke arrows are stated in **17** corpus DOCX. Rendering all 17 at the round's base as
well as after shows that **16 of them already drew the reference's arrowhead count exactly** —
they write the same shape as DrawingML in an `mc:Choice` beside the VML, and the DrawingML reader
has had `a:headEnd` all along.

The one that moves is `067`, whose connectors are inside a `v:group` and therefore have no
DrawingML twin:

| | before | after | 26.2.4.2 |
|---|---:|---:|---:|
| arrowhead-sized fills | **0** | **25** | **25** |
| summed unsigned \|ink\| | 0.068 | **0.035** | — |

**1 of 17 renderings moved and the other 16 are byte-identical.** No gate column can see an
arrowhead, and `067` was `match` before and after.

*This is the second time in three rounds that a markup census overstated a VML reach by an order
of magnitude, and both times for the same reason: a `w:pict` is very often the fallback half of
something the DrawingML reader already handles. Render the base as well as the fix before
quoting a reach.*

## Two residuals that are not this

**`061_Nursing_Concept_Map_Template` draws none of its four on either side of the change**, and
the arrowhead is the smaller half of it: its connectors are `type="#_x0000_t38"`, a **curved**
connector, and `PaintOf` recognises only `rect`, `roundrect`, a straight connector and a line —
so it returns `VmlPaint.None` and the whole line is missing, not just its head. Four shapes in
one document.

**`ABCD-SDE-23-00` draws 39 arrowhead-sized fills against the reference's 14**, before this change
as well as after, so whatever that is predates it.
