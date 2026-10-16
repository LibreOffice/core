# slides-b-01 — prediction, committed before any measurement

Written after reading only `PptShapeGeometry.cs` (the whole file) and the HANDOVER slides
section. No probe has been run, no deck has been opened, no PDF has been produced.

## Gate

**I predict zero verdict movement from every item below.** Slides check 1 is a deck's slide
count, which no shape geometry or image placement can change; check 2 is extractable words in a
2% band, and none of these items adds or removes a text run (the OnTrac page number is a
placeholder that already draws its digits, just in the wrong colour and place); check 3 is
unembedded fonts, untouched by any of this. The track is 151/163 and I expect it to stay
151/163 whatever is implemented from this round. Stating this so the round is not scored on
the gate.

## Item 1 — arrows as rectangles (`Fundamentals_Module_1_basics.ppt`, `W3_Case_Study…`)

`PresetOf` already holds `13 => "rightArrow"`, `66-70` the four/six plain arrows, `93/94`
striped and notched right arrow, `89-91` the bent family, `77-83` the arrow callouts. It is
implausible that a 1990s-authored `.ppt` uses an arrow outside that set. So I predict **the
entry is not missing** and the brief's framing ("assume an entry is missing") is the thing to
refute. My ranked guesses for the real cause:

1. **The shapes are `mso_sptNotPrimitive` (type 0)** — drawn as a freeform/`pptCustomGeometry`
   path — and the custom-geometry path either is not reached or fails, falling back to the
   bounding rectangle. `PresetOf(0)` is deliberately null, so a grep of shape types would show
   type 0 and *look* like "no entry", which is exactly the trap named.
2. **A route problem, not a table problem** — the sixth-instance shape this project keeps
   hitting: something reads the type but the layouter is handed only the bounds. i.e. `PresetOf`
   is correct and its caller drops the result for shapes in some position (inside a group, in a
   master/placeholder, behind a `msofbtClientAnchor`).
3. Least likely: the arrow really is a WordArt/absent type.

I predict the two decks differ from each other — the brief warns they may — and specifically
that at least one of them is not an arrow-preset problem at all.

## Item 2 — `Wildlife for REDAC September 11.pptx` p3 rotation, p13 two extra blocks

p3: I predict the rotation is stated somewhere we do not read for a *picture* specifically —
most likely `p:pic/p:spPr/a:xfrm/@rot` being read for `p:sp` but not `p:pic`, or the
rotation living on the group (`a:chOff`/group `@rot`) and not being composed into the child.
p13: I predict the two blocks we draw are **empty placeholders inherited from the layout**
that the reference suppresses because they have no text (LibreOffice drops an empty
non-outline placeholder at import), so we draw the layout's fill/outline for a shape
PowerPoint would show only in edit view.

## Item 3 — `Thailand17.ppt` image scaling

I predict **the reference is right and we are wrong**, and specifically that the discrepancy is
the Escher picture crop fractions (`pib` crop from top/bottom, props 256-263). Cropping an
image and keeping the same frame makes the *drawn* content shorter/taller than the source
aspect. Given item 5 says cropping is unimplemented in words, my prior is it is unimplemented
in slides too and this deck is the visible cost. Alternative: the frame comes from
`msofbtChildAnchor` in a group whose scale we do not apply.

## Item 4 — `OnTrac…` grey page number

I predict the colour is **not stated on the run at all** and the reference resolves it from the
slide-master's `p:txStyles`/`otherStyle` or the layout placeholder's list style rather than from
the theme's `tx1`, and we fall back to black. Position shift: the same placeholder's `a:xfrm`
inherited from layout rather than master.

## Item 5 — Escher picture cropping in the slide path

I predict **slides does not have it either**, so it is not a route for words. If it does, I
predict it is partial (crop applied to the source rect but not composed with `blipFlags` or
`fFlipH`).

## Reach

Before any census: I predict arrow-type-0 shapes reach **fewer than 10** of the 163 decks;
Escher picture crop props reach **10-25** of the 61-odd `.ppt` decks; the Wildlife p13 empty
placeholder pattern is the widest, plausibly **30+**, because empty layout placeholders are
ubiquitous. I expect at least one of these three to come in far under the guess, in the manner
of the round that predicted 35-55 and measured 2.
