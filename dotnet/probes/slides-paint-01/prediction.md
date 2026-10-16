# slides-paint-01 — prediction, committed before measuring

Written after reading code only (LibreOffice C++ at 27.2.0.0.alpha0+ for intent, XRay.Office C# for
what we do) and after locating the six decks on disk. **No corpus census, no render, no `soffice`
run, no PDF read has happened yet.** Environment checked: `check-env.sh` green, LibreOffice
**26.2.4.2** 620(Build:2), Carlito/Caladea/Liberation/DejaVu all resolving.

The six decks, with formats — already established, so not predicted:

| deck | format | user's words |
|---|---|---|
| `1-secretariat` | `.ppt` | image rendering without transparent background |
| `pres_ioc_phuket` | `.ppt` | missing transparent color handling for many images |
| `Thailand17` | `.ppt` | header missing shadow? |
| `Aerospace_Journey_…BCB1637572DA6` | `.ppt` | title missing shadow |
| `16 - UTM - (NASA)` | `.pptx` | title missing underline |
| `Stakeholders-v08052017 - v5` | `.pptx` | link missing underline |

So transparency and shadow are wholly binary-path questions and underline is wholly a DrawingML
question. That split is a fact, not a prediction.

---

## Cluster 1 — image transparency

**P1.1 — The user's diagnosis is right and it is a missing feature, not a wrong one.**
`grep -rn "clrChange\|ClrChange\|TransparentColour\|TransparentColor\|pictureTransparent"` over all
of `dotnet/src` and `dotnet/tests` returns **zero lines**. So we do not read Escher property 263 and
we do not read `a:clrChange`; we draw every picture with the alpha it was stored with. I predict the
observation reproduces and that the mechanism is absence.

**P1.2** — Escher property **263** (`DFF_Prop_pictureTransparent`, `include/svx/msdffdef.hxx:138`)
is present in both `.ppt` decks, and at more sites in `pres_ioc_phuket` than in `1-secretariat`,
because the user wrote "many images" for one and singular for the other.

**P1.3 — reach.** Property 263 appears in **fewer than 15 of the 163 decks**, and `a:clrChange`
in **fewer than 5** of the `.pptx` half. This is the number I am least sure of and the one a
regex would get wrong; the census walks records.

**P1.4 — direction.** We draw **more** ink than the reference on these pages: an opaque rectangle
of background colour where the reference has knocked it out. Signed `|ink|` difference positive.
It is an over-draw defect, not an under-draw one, which is the opposite direction from clusters 2
and 3.

**P1.5 — what LibreOffice actually does**, from `msdffimp.cxx:3894-3903` →
`Bitmap::CombineMaskOr` (`vcl/source/bitmap/bitmap.cxx:2517`) → `Bitmap::CreateAlphaMask`
(`vcl/source/bitmap/bitmappaint.cxx:684`): an **independent per-channel box** of ±**9** around the
stated colour — `nMinR ≤ R ≤ nMaxR && nMinG ≤ G ≤ nMaxG && nMinB ≤ B ≤ nMaxB` — producing **binary**
alpha, not a graded one, OR-combined with any alpha the picture already had. It is applied
**only when `aGraf.GetType() == GraphicType::Bitmap`**, so a WMF/EMF picture carrying property 263
gets nothing. I predict 26.2.4.2 matches this; the tree is not the reference binary and this needs
measuring.

**What this census cannot see, named in advance.** (a) Whether the pictures carrying 263 are
bitmaps or metafiles — only the bitmaps change. (b) Whether **any pixel** in a given picture falls
inside the ±9 box; a file may state a transparent colour that the artwork never uses, in which case
the property is present and inert. (c) Whether the affected region is visible at all — it may sit
under another shape. So the count of decks holding the property is an **upper bound** on reach, and
I expect the visible reach to be strictly smaller.

## Cluster 2 — missing shadow

**P2.1 — Both decks are `.ppt`, so this is the legacy character shadow bit and not `a:outerShdw`.**
`PptCharacterStyle.ToEmphasis` (`src/XRay.Office.Presentations/MsBinary/PptStyleSheet.cs:528-542`)
reads `0x0001` bold, `0x0002` italic, `0x0004` underline, `0x0100` strikethrough — and **not
`0x0010`, shadow**, which the file's own comment at `PptStyleSheet.cs:11` names. `RunEmphasis`
(`src/XRay.Office.Core/Extraction/Content.cs:177-199`) has no `Shadow` member for it to be stored in.
So: **we draw nothing at all**, we do not draw it wrongly. That is the distinction the review
cannot make and I am committing to the first half of it.

**P2.2 — It is not an outer shadow and has no blur.** LibreOffice routes the character bit to
`SvxShadowedItem`, which VCL draws in its special-text path as a **second, hard-edged copy of the
glyphs** at a small offset derived from the font's line height, in black — or in light grey when
the text itself is black. Offset is roughly `1 + (lineHeight − 24)/24` device pixels. So the right
model is a duplicated glyph run, not a blurred rectangle.

**P2.3** — The `SlideShadow` we already have (`SlideShapes.cs:181`, read on the binary path at
`PptSlideLayout.cs:980-1013` from the Escher `Shadowed`/`ShadowOffsetX/Y`/`ShadowColour`
properties) is a **shape** shadow and is a different feature from this. The two decks will show the
character bit set and the shape property absent. If instead the shape property is present and we
are dropping it, P2.1 is wrong and I will say so.

**P2.4 — reach.** The `0x10` bit is a common PowerPoint-97 title style, so I predict it is set in
**more than 10** of the roughly 80 `.ppt` decks — noticeably wider than the two the user named.

**Cannot see:** whether the bit is set on a run that is actually *drawn* (a master's default
inherited by a placeholder with no text changes nothing), and whether the run's colour makes the
shadow visible against the background.

## Cluster 3 — missing underline

**P3.1 — This one is not a missing feature, unlike the other two, and the two decks do not share
a cause.** Underline is implemented on both readers and drawn:
`PptxTextBody.cs:628-654` reads `a:rPr/@u` against `ST_TextUnderlineType`,
`PptStyleSheet.cs:534` reads the binary `0x0004`, `SlideTextLayout.cs:1253-1271` builds the rule
from the face's own `UnderlinePosition`/`UnderlineThickness`. So a bare "we don't do underline"
is refuted before I start.

**P3.2 — `16 - UTM - (NASA)` (title).** The `u` is stated somewhere other than the run — a
`defRPr` in the layout's or master's `lstStyle`, or on the placeholder's `lvl1pPr` — and our
`First(...)` inheritance chain does not reach it.

**P3.3 — `Stakeholders-v08052017 - v5` (link).** The underline comes from `a:hlinkClick` and not
from `@u` at all: LibreOffice underlines and recolours hyperlink text from the link, so the run's
own `rPr` may legitimately state nothing. **Two decks, two different causes**, and I am predicting
that explicitly rather than assuming the shared wording means a shared fix.

**Cannot see:** whether the reference's underline on `Stakeholders` is the theme's `hlink` colour
too, which would make it a colour defect as well as a decoration one.

## Verdict movement

**Zero.** Slides is 163/163 page-exact; not one of these six observations is visible to any gate
column, and a knocked-out background, a duplicated glyph run and a 1 pt rule move neither a page
count nor a word count. I predict the gate is byte-for-byte unchanged in its verdict columns on all
163 decks whatever I find, and I am saying so before measuring rather than after.

## Standing risks I am carrying

- The C++ tree read above is **27.2.0.0.alpha0+**. Every claim about what LibreOffice *draws*
  (P1.5, P2.2) is intent read from the wrong binary and must be measured against **26.2.4.2**.
- `pdf-image-diff.py` rasterises at 512 px. A 1 pt underline and a 1 px shadow offset are both
  below that; if I report "unchanged" from it I am reporting the raster, not the rendering. Read
  the operators.
