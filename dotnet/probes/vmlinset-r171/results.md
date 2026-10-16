# r171 — a VML text box's inset and its fixed height

`DocxVmlFrames` built every VML text box's `PageFrame` with

```csharp
Padding = box is null ? default : default,
```

— a stub whose two branches are the same expression — and never set `HasFixedHeight` at all. So
every VML text box was laid out with its text against its own edge and grew to hold whatever it
was given. The DrawingML reader beside it has had both since it was written.

## The rule, from the source

`TextBoxContext`'s constructor (`oox/source/vml/vmltextboxcontext.cxx`:185-211) reads
`v:textbox/@inset` as a comma list of up to four CSS lengths — left, top, right, bottom —
substituting **that side's** default for an empty or missing one: `0.1in` across and `0.05in`
down. So an absent attribute and a stated `0,0,0,0` are opposite answers, and `inset="4mm"` moves
the left side alone.

The whole block is guarded on `insetmode != "auto"`, so a box asking for the automatic mode leaves
`borderDistanceSet` false, `vmlshape.cxx`:778-784 sets no distance, and the frame keeps Writer's
own `Frame` style padding of 1.5 mm.

`mso-fit-shape-to-text` is VML's `a:spAutoFit` and is read from **two** styles into one flag: the
shape's (`vmlshapecontext.cxx`:551) and the box's own (`vmltextboxcontext.cxx`:221-222). Both set
`mbAutoHeight` on *presence alone*, whatever value follows the colon, and that becomes
`SizeType::MIN` against `FIX` (`vmlshape.cxx`:776-777).

## Confirmed twice, on ten one-attribute fixtures

`build.py` writes the same 200 × 60 pt shape ten times, differing in one attribute.
`soffice --convert-to fodt` prints the reference's own resolved padding:

| arm | left | top | right | bottom | height |
|---|---|---|---|---|---|
| `plain` (no inset) | 0.1in | 0.05in | 0.1in | 0.05in | fixed |
| `inset-zero` `0,0,0,0` | 0 | 0 | 0 | 0 | fixed |
| `inset-partial` `,.6mm,,.6mm` | 0.1in | 0.0236in | 0.1in | 0.0236in | fixed |
| `inset-one` `4mm` | 0.1575in | 0.05in | 0.1in | 0.05in | fixed |
| `inset-four` | 0.1035in | 0.0535in | 0.1035in | 0.0535in | fixed |
| `insetmode-auto` | *(inherits `Frame`'s 0.0591in)* | | | | fixed |
| `insetmode-custom` | 0.1in | 0.05in | 0.1in | 0.05in | fixed |
| `short-boxfit`, `short-shapefit` | 0.1in | 0.05in | 0.1in | 0.05in | **no `svg:height`** |

and rendering the same ten agrees: after the fix, **10 arms of 10** match on the left inset to
0.10 pt — the two writers' constant text-origin offset, which the body paragraph shows too — and
exactly on the top. The `short` arm, 24 pt holding four lines, draws **2 lines on both sides**
where before we drew four.

### A fixture has to declare the shapetype Word writes

The first cut of `build.py` omitted `v:shapetype id="_x0000_t202"`, and then
`type="#_x0000_t202"` resolved to nothing: the reference imported a `draw:custom-shape` carrying
`<draw:enhanced-geometry draw:type="0"/>` rather than a Writer text frame. Its graphic style
*states* every padding correctly and **none of them reaches the page** — all ten arms rendered
byte-identically at 72 + 1.5 mm, which reads exactly like an attribute the reference ignores.
Declaring the shapetype turns the object into `draw:frame`/`draw:text-box` and the ten arms
separate. This is `office-corpus`'s *"a fixture minimal enough to be obviously correct may be
minimal enough to be answering a different question"* in a new disguise: what was missing was not
a settings part but a shape *type*.

## Reach: the census counts markup, the renderer sees a tenth of it

131 of the 272 corpus DOCX state a `v:textbox`, 1638 boxes in all, and 1467 state no `inset`. That
figure is not the reach: **1472 of the 1638 are the `mc:Fallback` half of an `mc:AlternateContent`**
whose `mc:Choice` carries the same shape as DrawingML, so the reader takes the `w:drawing` and
never looks at the VML. `drawn.py` separates them — **166 boxes in 15 documents** are reachable.

Rendering our half of the 131 at the round's base and again after, under `SOURCE_DATE_EPOCH`, one
output directory per document: **14 renderings move and 117 are byte-identical**, and the 14 are
inside the 15. *Census what a rule paints, not how many times it is stated.*

## Cost: one gate verdict, and it lands exactly

| | |
|---|---|
| gate over the 131 | **129 → 130 match** |
| movers better against 26.2.4.2 | 4 |
| movers worse | 2, by 0.01 and 0.03 of ink |
| summed \|ink\| over the movers | 4.14 → 4.04 |

`068_Work_Breakdown_Structure_Template_Green_Theme` drew **493** alphanumeric characters against
the reference's **475** and now draws **475** — `glyphs` to `match`, on the nose. Two more
truncate to the reference's count as well: `065` 185 → 179 against 179, `066` 255 → 246 against
246. That is the fixed height doing the work; the inset is what moved the ink.

**The five Work Breakdown templates carry a much larger placement defect on top of this**, which
this does not touch: paired against the reference their mean `|dx|` is 55 to 264 pt. `065`
improves from 108 to 55 and `067` from 202 to 196; `069` reads 262 → 264, which is inside what
pairing thirty repeated `SUBTASK` labels by string can resolve. Whoever takes that next should
pair by cluster rather than by string — `probes/words-seat-r94/columnx.py` is the instrument.
