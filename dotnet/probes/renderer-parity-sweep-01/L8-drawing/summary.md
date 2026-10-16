# L8-drawing — summary

14 documents → **9 root causes, 2 non-defects.** Two root causes are in this lane's files and
carry patches; seven are recorded as cross-lane dependencies with the markup, the measurement
and the `file:line`.

**RC-1 · DOCX shapes are drawn as their bounding rectangle, and only `a:solidFill` is read.**
`XRay.Office.WordProcessing/Ooxml/DocxFrames.cs:860,798-818` + `Layout/PageFrame.cs:333` +
`Layout/PageDrawing.cs:230-310`. #062 #148 #152 #167 #169 (5 docs). Three separable limbs:
geometry ignored (circles → squares), every fill but `a:solidFill` ignored (gradient, picture,
`grpFill` and the theme `a:fillRef` → nothing drawn, which is what makes white text invisible),
and an `a:ln` without `@w` giving width 0. A probe proves the shared evaluator is correct, so
nothing is needed in this lane. **Cross-lane (L2+L3). Confidence high.**

**RC-2 · A tiled bitmap fill stops after 8192 tiles.** `XRay.Office.Rendering/Fills/Tiles.cs:26,81`.
#076 (1 doc, reach probably wider). Measured: reference 37 550 tile draws, ours **exactly 8192**,
covering the top 22% of the slide. PDF backend only; the Skia backend uses a repeat shader.
**Patch `patches/tiled-fill-truncated.diff`. Mine. Confidence high.**

**RC-3 · `wp:anchor/@relativeHeight` is not read; DOCX frames paint in document order.**
`git grep relativeHeight -- dotnet/src` returns nothing. #148 today (its blue shape is the
second-lowest of 23 anchors and is drawn 22nd, over the captions and icons); #062 latently (its
page background is the **lowest** of 44 and is drawn 9th). Same defect L2 found on #024.
**Cross-lane (L2/L3). SEQUENCING HAZARD: must land before RC-1's fill limb, or #062 turns from
"no background" into "background over everything". Confidence high.**

**RC-4 · A preset sub-path's `fill`/`stroke` flags are parsed off the table line and dropped.**
`XRay.Office.Ooxml/DrawingML/PresetShapeGeometry.cs:110-118`, `CustomShapeGeometry.cs:117-120,216-233,764`.
96 of the table's 320 sub-paths say `fill="none"` (every connector) and 84 say `stroke="false"`
(every pseudo-3D shading face). Probe: `bentConnector3` draws as a filled triangle.
**Patch `patches/preset-subpath-fill-stroke.diff` — enabling; needs a two-line consumer change in
L5. Mine. Confidence high.**

**RC-5 · A picture's `a:scene3d/a:camera/a:rot/@rev` and its `a:ln` border are not read.** #100's
poster. The file states **no `a:xfrm/@rot`** — the tilt is `rev="360000"` (6°), which LibreOffice
converts to shape rotation (`oox/source/drawingml/shape.cxx:1054-1064`). `scene3d` appears
nowhere in `dotnet/src`. **Cross-lane (L5). Confidence high.**

**RC-6 · VML WordArt (`v:textpath`) is not read, so a Word watermark is absent.**
`XRay.Office.WordProcessing/Ooxml/DocxVmlFrames.cs`; `git grep textpath -- dotnet/src` returns
nothing. #142 #190 (2 docs). **Cross-lane (L3). Confidence high.**

**RC-7 · An inline picture taller than the text area is moved to the next page instead of
overflowing.** #176 — the *whole* cover, which the case note read as five missing graphics, is
one 2550×3300 JPEG. Both PDFs draw it at `612.5 0 0 792.65 0 −11 cm`; the reference on page 1,
ours on page 2. Not missing, not mis-scaled, not clipped. **Cross-lane (L2). Confidence high;
re-check against 26.2.4.2 before fixing — it is a position fault.**

**RC-8 · Anchored frames land at the wrong end of the page.** #051. All 16 anchors are
`positionV relativeFrom="paragraph"` with small offsets and nothing unusual stated.
**Cross-lane (L2). Confidence low-medium; position fault, so 26.2.4.2 re-check first.**

**RC-9 · `.ppt` connector arrowheads (Escher 464/465) are never read**, though `SlideLineEnds`
already draws them for PPTX. #157. **Cross-lane (L5). Confidence high.**

**Not a defect — RC-10 · the corner-focus circle gradient.** #100's background. Ours is right for
**26.2.4.2** and wrong for the sweep's **24.2.7.2** reference. The corner→linear branch existed
at `PptxSlideLayout.cs:1411-1418`, was re-checked in round 59 and deliberately removed
(`TODO.24-2-7-audit.md`, `PptxSlideLayout.cs:1591`, worth −54.26 `abs_ink` on this very
document). Round 59 could not test 24.2.7.2; **this lane did**, on a six-arm probe, and the old
branch's rule reproduces exactly. **Do not patch.** The divergence is confirmed, not inferred.

**Not a defect — RC-11 · a CMYK JPEG carrying an ICC profile.** #176's cover art is 4-component
YCCK with a 490 kB embedded profile; Skia colour-manages it and LibreOffice does not, so our
green is (84,166,69) against the reference's (73,226,0). Ours is the more faithful conversion.
Same shape as `TODO.raster-ceiling.md`. **No patch; a policy call above this lane.**

**Correction:** the dispatch said two of these 14 are tagged `lo-broken`. **None is** —
`pl-cases.json`'s six `lo-broken` documents are all in other lanes.

**Not this lane's:** #049 (taller rows, not a hatch bug — the 14 `a:pattFill` cells match; plus
an EMF logo, `XRay.Office.Vector`), #178 (advance divergence, L1), #190's TOC hyperlink styling (L3).
