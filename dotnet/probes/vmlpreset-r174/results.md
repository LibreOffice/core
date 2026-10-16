# r174 — which VML shapes get any ink at all

Run while chasing `067`'s missing arrowheads (`../vmlarrow-r173`), and kept because it bounds the
whole area. `DocxVmlFrames.PaintOf` answers `VmlPaint.None` unless the element is a `v:rect`, a
`v:roundrect`, a `v:line` or a straight connector — so anything else is drawn with no fill and no
outline whatever it states.

Censused over the corpus's **reachable** VML — excluding the `mc:Fallback` halves that neither
renderer reads, because the `mc:Choice` beside them carries the same shape as DrawingML:

```
   154  v:rect       painted
   150  v:shape      NOT painted unless it is a straight connector
     6  v:line       painted
     4  v:roundrect  painted

v:shape presets:
    87  _x0000_t32    straight connector, in 5 documents   -- painted
    37  _x0000_t75    picture frame, in 13 documents       -- wants no paint
    15  _x0000_t136   Fontwork, in 5 documents             -- DocxVmlFontwork
     8  _x0000_t202   text box, in 4 documents             -- NOT painted
     3  _x0000_t15    in 1 document                        -- NOT painted
```

**So the gap is eleven shapes in five documents**, and the interesting eight are `_x0000_t202`: a
text box written as a `v:shape` naming the rectangle shapetype rather than as a `v:rect`. `PaintOf`
tests the element's own local name (`shape.Name.LocalName is "rect" or "roundrect"`) and not the
shapetype it names, so such a box's `fillcolor` and `strokecolor` are read and then thrown away.
The two are the same object to Word and to the reference.

**Not taken**, because eleven shapes is a small prize and the fix wants the shapetype's own
geometry rather than a widened name test — `ShapeTypeOf` already resolves the type for Fontwork's
benefit, so the raw material is there. Recorded so the next round does not have to census it again.

*`061_Nursing_Concept_Map_Template`'s four curved connectors (`_x0000_t38`) do not appear here:
they are inside an `mc:Fallback`, so the miss on that document is in the DrawingML path, not this
one.*
