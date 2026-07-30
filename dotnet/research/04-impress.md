# Impress / Draw File Formats, Document Model & Rendering — Architecture Reference

Scope: this document maps the LibreOffice C++ architecture that a pure C# reimplementation
needs to replicate for (a) content extraction from PPTX / legacy PPT / ODP, and (b) headless
slide rendering to raster images / PDF. All paths are relative to the repo root
(`/home/user/libreoffice-core`). Line numbers are approximate anchors, not guarantees against
future refactors — search for the symbol if a line has drifted.

---

## Table of contents

- A. Shape/draw document model (`svx/`, `sd/`)
- B. PPTX / OOXML PresentationML import (`oox/`)
- C. PPT binary import (`sd/source/filter/ppt`, `filter/source/msfilter`)
- D. ODP import (`xmloff/source/draw`)
- E. Slide rendering path (`drawinglayer/`, `svx/source/sdr/`, headless export)
- F. Top ~20 fidelity-dominating features, ranked
- G. C# reimplementation notes and phased plan

---

## A. Shape / draw document model

### A.1 The core class hierarchy

Everything drawable in Impress/Draw/Calc/Writer shares one model, defined in `svx/`:

```
SdrModel                     (include/svx/svdmodel.hxx)
 └─ SdrPage[]                (include/svx/svdpage.hxx)      -- one per slide/master/notes
     └─ SdrObjList            (base of SdrPage and SdrObjGroup)
         └─ SdrObject[]        (include/svx/svdobj.hxx)      -- abstract shape base
             ├─ SdrRectObj             (svx/source/svdraw/svdorect.cxx)   -- rect/rounded rect
             ├─ SdrCircObj             (svdocirc.cxx)                     -- circle/ellipse/arc/pie
             ├─ SdrPathObj             (svdopath.cxx)                     -- poly/bezier/line
             ├─ SdrTextObj             (svdotext.cxx)         -- ABSTRACT base adding text
             │    ├─ SdrRectObj / SdrCircObj / SdrPathObj all derive from SdrTextObj too
             │    │    (in LO, *all* 2D shapes are SdrTextObj subclasses — text capability
             │    │    is baked into the geometry classes, not bolted on separately)
             │    ├─ SdrObjCustomShape  (svdoashp.cxx)        -- OOXML/legacy preset + custom geom
             │    ├─ SdrCaptionObj      (svdocapt.cxx)        -- callouts
             │    ├─ SdrEdgeObj         (svdoedge.cxx)        -- connectors
             │    ├─ SdrMeasureObj      (svdomeas.cxx)        -- dimension lines
             │    └─ SdrTableObj        (svx/source/table/svdotable.cxx) -- tables
             ├─ SdrGrafObj             (svdograf.cxx)         -- bitmap/vector image (also SdrTextObj-derived)
             ├─ SdrOle2Obj             (svdoole2.cxx)         -- OLE objects, charts, embedded docs
             ├─ SdrObjGroup            (svdogrp.cxx)          -- shape group (is itself an SdrObjList)
             ├─ SdrMediaObj            (svdomedia.cxx)        -- video/audio
             ├─ SdrPageObj             (svdopage.cxx)         -- "page as object" (used by presentation preview)
             ├─ SdrUnoObj              (svdouno.cxx)          -- form controls
             └─ E3dObject/E3dScene     (svx/source/engine3d/) -- 3D scenes (charts, some shapes)
```

Type identity is `SdrObjKind` (`include/svx/svdobjkind.hxx:24`) — an enum (`Group`, `Rectangle`,
`CircleOrEllipse`, `PathLine`, `PathFill`, `Text`, `Graphic`, `OLE2`, `Edge`, `Caption`,
`CustomShape=33`, `Media=34`, `Table=35`, `Annotation=36`, plus 3D and form-control kinds) combined
with an `SdrInventor` (which subsystem owns the object: `Default`=SVDr, `E3d`, `FmForm`, etc. —
`include/svx/svdobj.hxx:85`). A C# port should keep an analogous `(Inventor, ObjKind)` pair plus a
polymorphic shape class hierarchy — do not conflate ObjKind with a single C# `enum ShapeType`
because e.g. rectangle/ellipse/text are all fundamentally "attributed geometry + optional text",
and grouping/paths/connectors have distinct behavioral needs.

`SdrPage` (`include/svx/svdpage.hxx`) holds:
- The `SdrObjList` of shapes (ordered — z-order is list order).
- Page size/border (`tools::Rectangle`, in 1/100 mm).
- A master-page link: `mpMasterPageDescriptor` (`svdpage.hxx:446`), queried via
  `TRG_HasMasterPage()/TRG_GetMasterPage()` (`svdpage.hxx:509-513`).
- `SdrPageProperties` — an `SfxItemSet` holding page-level attributes (background fill etc).
- Layers (`SdrLayerIDSet` visible/printable layer bitsets), used to hide background/layout shapes.

`SdrModel` (`include/svx/svdmodel.hxx`) owns the `SdrItemPool` (the shared attribute-item pool —
see A.3), the page list, default units (1/100 mm, see E.4), and style-sheet pools.

### A.2 Impress specialization (`sd/`)

`sd/inc/drawdoc.hxx` — `SdDrawDocument` extends `SdrModel` (via `FmFormModel`) and adds:
- Multiple `SdPage` lists: `maPages` (standard+notes interleaved: "after a standard page the
  corresponding notes page follows" — see `sd/source/ui/unoidl/unomodel.cxx` comment and
  `svx/source/svdraw/svdpage.cxx:1852` `notesPage = GetPageNum() % 2 == 0`), plus a separate
  master-page list `maMaSdPages`.
- `SdStyleSheetPool` (`sd/source/core/stlpool.cxx`) — the "outline level" paragraph styles
  (`sd/source/core/stlfamily.cxx`, `stlsheet.cxx`) used for placeholder text formatting
  inheritance (title style, outline-1..outline-9 styles, per master page).
- Slide-show/animation data (`sd/source/core/drawdoc_animations.cxx`,
  `sd/source/core/sdpage_animations.cxx`).

`sd/inc/sdpage.hxx` — `SdPage final : public FmFormPage /* : SdrPage */, public SdrObjUserCall`.
Key members:
- `mePageKind` : `PageKind::{Standard, Notes, Handout}` (`sd/inc/pres.hxx:44`).
- `meAutoLayout` : `AutoLayout` enum (`xmloff/inc` / `include/xmloff/autolayout.hxx`) — the classic
  "Title, Content" / "Title only" / "Two content" layout presets used by legacy `.ppt`/`.odp`
  and by Impress's own layout picker (distinct from, but related to, OOXML's `<p:sldLayout>`).
- `maPresentationShapeList` (`sd::ShapeList`) — the subset of page shapes that are "presentation
  objects" (placeholders), each tagged with a `PresObjKind`.
- Per-slide transition/timing: `mePresChange`, `mfTime`, `mbSoundOn`, plus **new-style** transition
  fields added for OOXML fidelity: `mnTransitionType`, `mnTransitionSubtype`,
  `mbTransitionDirection`, `mnTransitionFadeColor`, `mfTransitionDuration`
  (`sd/inc/sdpage.hxx:143-147`) — these map directly to `<p:transition>` `spectrum`/type/subtype
  attributes and are the ones to preserve verbatim in a C# extraction model rather than trying to
  re-derive Impress's older enum-based transition model.
- `HeaderFooterSettings` (`sd/inc/sdpage.hxx:63`) — header/footer/date/slide-number visibility +
  fixed-vs-auto date text, mirroring `<p:hf>`.

`PresObjKind` (`sd/inc/pres.hxx:21`): `NONE, Title, Outline, Text, Graphic, Object, Chart,
OrgChart, Table, Page, Handout, Notes, PagePreview, Header, Footer, DateTime, SlideNumber, Calc,
Media`. Every placeholder shape on a slide/layout/master is registered in
`maPresentationShapeList` under one of these kinds; `SdPage::GetPresObj(kind)` /
`IsPresObj()`/`InsertPresObj()` (`sd/inc/sdpage.hxx:172-183`) are the lookup API. This is the
internal analogue of OOXML's `<p:ph type="..." idx="n"/>` — see B.3 for how OOXML placeholder
type/idx is translated into a `PresObjKind` + index during import.

**Master pages / layouts / inheritance chain (internal model):** Impress's internal model
(inherited from ODF) is two-level: `SdPage` (content) → master `SdPage` (`TRG_GetMasterPage()`).
There is no separate "layout" object at the `SdrPage` level — OOXML's 3-level
slide→layout→master chain is *flattened into 2 levels* during import: the layout's placeholder
geometry/formatting is baked directly onto the slide's placeholder shapes (or the layout is
imported as a distinct hidden master-like page, depending on filter version — see B.3). This is an
important asymmetry a C# importer must decide on explicitly: either (a) mimic LO and flatten
layout→slide at import time, discarding the layout as a separate addressable object, or (b) keep a
true 3-level model in the C# object graph and resolve inheritance lazily at render/extract time
(recommended — see G).

**Background fill inheritance** (`SdrPage::GetPageBackgroundColor`,
`svx/source/svdraw/svdpage.cxx:1827-1877`): if the page's own `SfxItemSet` has
`XATTR_FILLSTYLE == FillStyle_NONE`, fall back to `TRG_GetMasterPage()`'s item set (with a special
case for notes pages under LOKit, which look up the *standard* page's master rather than their
own, because notes masters may have no style sheet set). This exact "NONE→inherit, else use own"
rule is what a C# port must replicate for slide/master/layout background resolution — see B.3 for
the 3-level OOXML equivalent (`<p:bg>` present → use it; absent → walk up layout → master → theme
default `lt1`/`bg1`).

### A.3 Attribute model — `SfxItemSet` / `SDRATTR_*` / `XATTR_*`

Every `SdrObject` carries an `SfxItemSet` (from `svl/`) keyed by "which-ids" — stable small
integers, each mapped by `TypedWhichId<T>` to a concrete `SfxPoolItem` subclass. This is
LibreOffice's generic "attribute bag with typed slots + inheritance via item pool defaults"
mechanism, used everywhere (Writer, Calc, Draw). For shapes, the relevant ranges are defined in
two headers:

- `include/svx/xdef.hxx:88-147` — **`XATTR_*`** (line, fill, and legacy fontwork/"form text"
  attributes; historically shared with the old `svx` "XOut" line/fill engine):
  - Line: `XATTR_LINESTYLE, LINEDASH, LINEWIDTH, LINECOLOR, LINESTART/END(WIDTH/CENTER)` (arrowheads),
    `LINETRANSPARENCE, LINEJOINT, LINECAP` (ids 1000-1012).
  - Fill: `XATTR_FILLSTYLE, FILLCOLOR, FILLGRADIENT, FILLHATCH, FILLBITMAP, FILLTRANSPARENCE,
    GRADIENTSTEPCOUNT, FILLBMP_TILE/POS/SIZEX/SIZEY, FILLFLOATTRANSPARENCE (gradient transparency),
    SECONDARYFILLCOLOR, FILLBMP_TILEOFFSETX/Y, FILLBMP_STRETCH, FILLBACKGROUND,
    FILLUSESLIDEBACKGROUND` (ids 1014-1034).
  - Text/Fontwork (legacy StarDraw "Fontwork", *not* the same as OOXML WordArt/`prstTxWarp`
    fontwork emulation in customshapes, though conceptually related): `XATTR_FORMTXTSTYLE...
    FORMTXTSHDWTRANSP` (1036-1047).
- `include/svx/svddef.hxx` — **`SDRATTR_*`**, contiguous with the XATTR range:
  - Shadow (1048-1058): `SHADOW` (on/off), `SHADOWCOLOR`, `SHADOWXDIST/YDIST`,
    `SHADOWTRANSPARENCE`, `SHADOW3D`, `SHADOWPERSP`, `SHADOWSIZEX/Y`, `SHADOWBLUR`,
    `SHADOWALIGNMENT` — note `SHADOWBLUR`/`SHADOWSIZEX/Y`/`ALIGNMENT` were added later for
    OOXML/CSS-style soft shadows (perspective/blur), beyond the classic hard offset shadow.
  - Caption (1059-1068): callout geometry (`CAPTIONTYPE, FIXEDANGLE, ANGLE, GAP, ESCDIR,
    ESCISREL, ESCREL, ESCABS, LINELEN, FITLINELEN`).
  - Misc/text-frame (1069-1095): `CORNER_RADIUS`, `TEXT_MINFRAMEHEIGHT/WIDTH`,
    `TEXT_AUTOGROWHEIGHT/WIDTH`, `TEXT_FITTOSIZE` (autofit mode, see A.4),
    `TEXT_LEFTDIST/RIGHTDIST/UPPERDIST/LOWERDIST` (inset margins — maps to OOXML `lIns/tIns/
    rIns/bIns`), `TEXT_VERTADJUST/HORZADJUST` (anchor), legacy marquee-text animation items
    (`TEXT_ANIKIND/ANIDIRECTION/...` — StarOffice-era scrolling text, rarely relevant to OOXML),
    `TEXT_CONTOURFRAME` (text follows shape contour), `TEXT_USEFIXEDCELLHEIGHT`,
    `TEXT_WORDWRAP`, `TEXT_CHAINNEXTNAME` (Writer-style text chaining — not used by Impress),
    `TEXT_CLIPVERTOVERFLOW`.
  - Edge/connector (1096-1106): routing distances, glue-point offsets, line-delta counts (for
    orthogonal/curved connector routing).
  - Measure (1107-1128): dimension-line specifics.
  - Circle (1128-1130): `CIRCKIND` (full/section/cut/arc), start/end angle.
  - "NotPersist" transient drag-transform items (1131-1166): rotate/shear/move/resize *deltas*
    used only during interactive editing — **not relevant to a file-format importer/renderer**,
    skip them entirely in a C# port.
  - Graphic color adjustments (1167-1176): `GRAFRED/GREEN/BLUE/LUMINANCE/CONTRAST/GAMMA/
    TRANSPARENCE/INVERT/MODE/CROP` — maps to OOXML `<a:blip>` `<a:duotone>`/`<a:lum>`/
    `<a:alphaModFix>` and `<a:srcRect>` (crop) on picture fills.
  - 3D object/scene (1177-1232): extrusion depth, bevel, lighting, material — relevant for
    `sp3d`/`scene3d` (see A.4) and legacy 3D charts.
  - **Custom shape** (1233-1235): `CUSTOMSHAPE_ENGINE` (service name, usually
    `"com.sun.star.drawing.EnhancedCustomShapeEngine"`), `CUSTOMSHAPE_DATA` (unused string),
    **`CUSTOMSHAPE_GEOMETRY`** (`SdrCustomShapeGeometryItem` — the entire preset/path/handle/
    adjustment/equation payload, see A.5). This single item is the most important one for pptx
    shape fidelity.
  - Table (1236-1241): cell border items (`SvxBoxItem`/`SvxBoxInfoItem`, shared with Writer table
    border model), text rotation, `TABLE_CELL_GRABBAG` (round-trip bag for unmapped OOXML cell
    properties).
  - Glow / soft-edge / text-glow (1242-1248): `GLOW_RADIUS/COLOR/TRANSPARENCY`,
    `SOFTEDGE_RADIUS`, `GLOW_TEXT_*` — direct analogues of OOXML `<a:effectLst><a:glow>`,
    `<a:softEdge>`.
  - Text columns (1249-1250): `TEXTCOLUMNS_NUMBER/SPACING` — OOXML `<a:bodyPr numCol="n"
    spcCol="...">`.
  - `WRITINGMODE2` (1251) — `SvxFrameDirectionItem`, horizontal/vertical/RTL text direction for
    the shape.
  - `EDGEOOXMLCURVE` (1252) — a compatibility flag controlling whether curved connectors are
    routed the OOXML way vs. the ODF way.

**Practical implication for C#:** don't model this as a flat property bag with 250 magic numbers;
model it as strongly-typed sub-records (`FillProperties`, `LineProperties`, `ShadowProperties`,
`TextFrameProperties`, `GlowProperties`, `SoftEdgeProperties`, `CustomShapeGeometry`,
`TableCellProperties`, ...) that mirror these logical groups, since that's how both the OOXML
schema and the ODF schema are actually organized, and it's how you'll want to structure inheritance
resolution (per-group fallback chains, not one giant fallback for the whole item set — though LO's
`SfxItemSet`/pool-default mechanism *does* do a single per-item fallback to the pool's static
defaults, which is effectively "value = own item if set, else the type's global default").

### A.4 Text model (`SdrTextObj`, `editeng/`)

`SdrTextObj` (`include/svx/svdotext.hxx`) is the base adding a text capability to geometry
objects. Text content itself is **not** stored as plain runs on the shape — it's an
`OutlinerParaObject` (built via `editeng/`'s `Outliner`/`EditEngine`), which is the same rich-text
engine used by Writer for text frames. This gives Impress full paragraph/character formatting,
bullets/numbering, tabs, fields (date, slide number, page count, URL fields), and multi-column
flow, at the cost of complexity: a C# port needs *some* rich text engine (paragraphs → runs →
character properties), but does not need the interactive editing machinery (`Outliner` selection,
undo, spell-check) — only decomposition (paragraph/run iteration with resolved character
properties) and metric (line-breaking/layout) capability.

Key text-frame behaviors, all driven by `SDRATTR_TEXT_*` items (A.3):
- **Autofit / "shrink text on overflow"**: `SDRATTR_TEXT_FITTOSIZE` holds an
  `SdrTextFitToSizeType` (`NONE`, `PROPORTIONAL`, `ALLLINES`, `AUTOFIT` — the last being the OOXML
  `normAutofit` shrink-to-fit). Resolution happens in `SdrTextObj::setupAutoFitText()`
  (`include/svx/svdotext.hxx:275-276`, impl in `svdotext.cxx`), which iteratively lays out text at
  decreasing font-scale/line-spacing-scale until it fits the shape's text rect, OR — for OOXML
  import — directly consumes the pre-computed `fontScale`/`lnSpcReduction` from `<a:normAutofit>`
  (see B.5) rather than recomputing them (PowerPoint bakes the fitted scale into the file; LO
  trusts it on import and only recomputes on edit). **This is important**: a C# extractor should
  read `fontScale`/`lnSpcReduction` directly from the XML and apply them as simple linear
  multipliers to font size / line spacing — do not attempt to reimplement PowerPoint's shrink
  algorithm from scratch; replicate what LO does (trust the stored scale).
- **AutoGrowHeight/Width** (`TEXT_AUTOGROWHEIGHT/WIDTH`) — shape resizes to fit text (OOXML
  `spAutoFit`).
- **Insets** (`TEXT_LEFTDIST/RIGHTDIST/UPPERDIST/LOWERDIST`) = OOXML `lIns/rIns/tIns/bIns` on
  `<a:bodyPr>` (EMU in OOXML, 1/100mm internally).
- **Vertical anchor** (`TEXT_VERTADJUST`: Top/Center/Bottom/Block) and **horizontal anchor**
  (`TEXT_HORZADJUST`) = `anchor`/`anchorCtr` attributes on `<a:bodyPr>`.
  Note OOXML anchor is per-bodyPr (whole text box); ODF/LO vertical adjust is likewise per-shape,
  not per-paragraph, so this maps cleanly.
- **Vertical/rotated text**: `SvxWritingModeItem` (`WRITINGMODE2`) plus a per-shape rotation angle
  (`SDRATTR_ROTATEANGLE` in the "not persist" transient range is for interactive drag; the
  *persisted* rotation is stored as part of the object's transform matrix — `SdrObject::GetGeoData()`
  / the object's `struct SdrObjGeoData` snapshot, or for the "60000ths of a degree" convention seen
  in `include/oox/drawingml/drawingmltypes.hxx:170` `calcRotationValue()` which converts OOXML's
  60,000ths-of-a-degree clockwise rotation into LO's 100ths-of-a-degree counter-clockwise
  convention: `rot = -(emuRot) * 600`, wrapped to ±180°). OOXML `vert="vert"/"vert270"/"eaVert"/
  "mongolianVert"` on `<a:bodyPr>` maps to stacked/rotated glyph layouts, handled specially (see
  `spAutoFit` guard in `textbodypropertiescontext.cxx:246-251`, which disables auto-grow-height for
  vertical text modes).
- **Text columns**: `SDRATTR_TEXTCOLUMNS_NUMBER/SPACING` = `<a:bodyPr numCol spcCol>`.
- **Word wrap**: `TEXT_WORDWRAP`.

### A.5 Custom shapes — the OOXML fidelity linchpin

`SdrObjCustomShape` (`svx/source/svdraw/svdoashp.cxx`, `include/svx/svdoashp.hxx`) is an
`SdrTextObj` subclass whose *geometry* is entirely data-driven: instead of hardcoded shape logic,
it stores an `SdrCustomShapeGeometryItem` (`SDRATTR_CUSTOMSHAPE_GEOMETRY`) which is a
`PropertyValue` bag holding:
  - `"Type"` — preset name (e.g. `"round-rectangle"`, `"ooxml-wedgeRoundRectangleCallout"`, or, for
    fully custom geometry, an internal marker).
  - `"ViewBox"` — the shape's internal coordinate space (OOXML preset shapes and legacy `mso_spt*`
    shapes both use a nominal 21600×21600 unit square by convention, though `custGeom` can specify
    any `<a:pathLst><a:path w="..." h="...">`).
  - `"AdjustmentValues"` — `Sequence<EnhancedCustomShapeAdjustmentValue>`: the *live* adjustment
    handle values (OOXML `<a:avLst><a:gd name="adj" fmla="val N"/></a:avLst>`, or legacy
    `DFF_Prop_adjustValue`/`adjust2Value`/etc.).
  - `"Equations"` — a string array of formula expressions (`"?f0 = 1/2*width"`-style; OOXML
    `<a:gdLst><a:gd name="..." fmla="..."/></a:gdLst>` guides), each referencing adjustment values,
    other equations, and predefined geometry references (`width`, `height`, `logwidth`, ...) by
    index/name.
  - `"Path"` — the actual `PolyPolygon`/segment-command description (`Coordinates`, `Segments`,
    `GluePoints`, `TextFrames`, `StretchX/Y`) built from evaluating the equations, OR (for OOXML
    `custGeom`) parsed directly from `<a:pathLst>` `moveTo/lnTo/cubicBezTo/arcTo/close` commands.
  - `"Handles"` — interactive adjustment-handle definitions (position formulas + range/polar
    constraints) — needed for *editing*, not rendering, so lower priority for a render/extract-only
    port, but useful if you want to expose "this shape has an adjustable corner-radius" semantics.
  - `"MirroredX"/"MirroredY"`, `"TextPath"` (fontwork-on-a-path flag + scale/same-letter-heights).

**Two independent geometry description systems feed this item:**

1. **Legacy MSO preset shapes** (used for binary `.ppt` import and for the small residual set of
   OOXML `prst` values that map 1:1 onto pre-existing MSO Office-Art autoshapes):
   `svx/source/customshapes/EnhancedCustomShapeGeometry.cxx` (8583 lines!) is a giant static table
   of `mso_sptXXXVert`/`mso_sptXXXSegm`/`mso_sptXXXCalc`/`mso_sptXXXHandle`/`mso_sptXXXTextRect`
   arrays — one set per legacy Office-Art shape type (`mso_sptArc`, `mso_sptCallout1`, ...,
   ~200 shapes), each a hand-transcribed copy of the binary MS-ODRAW shape definitions. Vertices
   use a compact encoded form (`SvxMSDffVertPair`, with `MSO_I` markers meaning "substitute
   calculated value N"); calculations (`SvxMSDffCalculationData`) are opcodes+operands in a tiny
   stack-VM-like format (`0x2000`+ opcodes = add/sub/mul/div/etc. referencing `DFF_Prop_adjustValue`
   and friends, `include/svx/msdffdef.hxx:151` `DFF_Prop_adjustValue = 327`). This whole table is
   evaluated at runtime by `EnhancedCustomShape2d` (`svx/source/customshapes/
   EnhancedCustomShape2d.cxx`, 3111 lines) which resolves adjustment values, evaluates the
   calculation opcodes, and produces the final `PolyPolygon`.
2. **OOXML preset shapes** (`oox/source/drawingml/customshapepresetdata.cxx`, ~1055 lines): a data
   table, **mechanically generated** by `oox/source/drawingml/customshapes/generatePresetsData.pl`
   from Microsoft's normative `presetShapeDefinitions.xml` (the ECMA-376/MS-ODRAWXML reference
   geometry for every `<a:prstGeom prst="...">` value: `rect`, `roundRect`, `ellipse`, `triangle`,
   `chevron`, `star5`, ..., `actionButtonHome`, ~180 shapes total). This is baked at compile time
   into C++ literal `PropertyValue` sequences (parsed back from a serialized dump format at
   `customshapepresetdata.cxx:29` `lcl_parseAdjustmentValue`), so at OOXML import time,
   `CustomShapeProperties` (`oox/source/drawingml/customshapeproperties.cxx`) just looks up the
   `prst` name and clones the corresponding pre-built geometry sequence, then overlays the
   document's actual `<a:avLst>` adjustment values on top.

   For fully custom geometry (`<a:custGeom>`), `oox/source/drawingml/customshapegeometry.cxx`
   parses `<a:avLst>`, `<a:gdLst>` (guides — reusing the *same* formula-command mini-language:
   `*/`, `+-`, `+/`, `ifelse`, `?:`, `abs`, `at2`, `cat2`, `cos`, `max`, `min`, `mod`, `pin`, `sat2`,
   `sin`, `sqrt`, `tan`, `val` — see `customshapegeometry.cxx:45-64` `FormulaCommand` enum),
   `<a:rect>` (text rect), and `<a:pathLst><a:path>` (moveTo/lnTo/arcTo/quadBezTo/cubicBezTo/close,
   each coordinate itself possibly a guide reference) directly into the same
   `EnhancedCustomShapeParameter`/segment-command representation used by the legacy engine, so both
   pipelines converge on one evaluator (`EnhancedCustomShape2d`) and one renderer decomposition
   (`svx/source/sdr/primitive2d/sdrcustomshapeprimitive2d.cxx`).

`EnhancedCustomShapeFunctionParser.cxx` (1162 lines) is a separate, more general infix-expression
parser (`svx/source/customshapes/EnhancedCustomShapeFunctionParser.cxx`) used specifically for ODF
`draw:enhanced-geometry` `draw:formula` strings (which use named-function syntax like
`sum(...)`, `if(...)`, `min(...)`), as opposed to OOXML's positional-opcode `fmla="val N"` /
`"*/ a b c"` style — both ultimately populate the same equation-list representation consumed by
`EnhancedCustomShape2d`.

`oox/source/drawingml/presetgeometrynames.cxx` provides the special-case mapping from OOXML preset
*text-warp* names (`prstTxWarp`, e.g. `textArchUp`, `textWave1`) to LO's Fontwork type strings
(`fontwork-arch-up-curve`, `fontwork-wave`, ...) or to raw `mso-sptNNN` fallbacks where no native
Fontwork equivalent exists — relevant for WordArt-style shapes.

**For a C# port:** the pragmatic path is *not* to reimplement ~200 preset shapes from scratch, but
to build (once) a data table equivalent to `customshapepresetdata.cxx`/`EnhancedCustomShapeGeometry.cxx`
— either by parsing the same normative `presetShapeDefinitions.xml` (Microsoft publishes it; also
derivable from the ECMA-376 spec) directly at build/runtime, or by porting the two C++ tables
mechanically (they are data, not logic) into a C# resource. Then port `EnhancedCustomShape2d`'s
evaluator (adjustment substitution + the small opcode VM) and `custGeom` path parser — this is a
bounded, well-specified task (~1-2k lines of logic) even though the *data* is huge.

### A.6 Tables, connectors, groups

- **`SdrTableObj`** (`svx/source/table/svdotable.cxx`, `include/svx/svdotable.hxx`) wraps a private
  `SdrTableObjImpl` holding a `TableModel`/`CellVector` — effectively a mini spreadsheet: rows,
  columns (with individual width/height), per-cell `SdrText` (own `OutlinerParaObject`) and
  per-cell border/fill items (`SDRATTR_TABLE_BORDER*`, and generic fill/line items scoped to the
  cell). Rendering treats each cell as a mini text-shape framed by resolved borders (shared corner
  border resolution logic à la Writer tables — adjacent cells' borders interact, "highest priority
  wins" style). OOXML `<a:tbl>` (`a:tr`, `a:tc`, `a:gridCol`, cell `a:tcPr` with `marL/marR/marT/
  marB`, merge via `gridSpan`/`rowSpan`/`hMerge`/`vMerge`) is imported by `oox/source/drawingml/
  table/` (see B.2 file list) into this model.
- **`SdrEdgeObj`** (`svx/source/svdraw/svdoedge.cxx`, `include/svx/svdoedge.hxx`) = connectors.
  Holds two `SdrObjConnection`s (`svdoedge.hxx:39`) each optionally bound to a "node" `SdrObject` +
  glue-point index (`GetConnectedNode()`, `svdoedge.hxx:221`), so the connector re-routes when
  endpoints move. `SDRATTR_EDGEKIND` selects straight/orthogonal-lines/bezier routing;
  `SDRATTR_EDGEOOXMLCURVE` toggles OOXML-compatible curve routing math vs. ODF's own. A C# port
  doesn't need live re-routing (no interactivity) but does need one-shot routing computation at
  extraction/render time given the current endpoint positions — port the routing algorithm from
  `svdoedge.cxx`, not just the raw geometry, since connectors are frequently *not* given explicit
  point lists in the file and must be computed.
- **`SdrObjGroup`** (`svdogrp.cxx`) — a shape whose children are an embedded `SdrObjList`; group
  transform (position/rotation/scale) applies to all children via a matrix, composed
  hierarchically. OOXML `<p:grpSp>` maps directly; note OOXML groups carry *both* a child
  coordinate space (`<a:chOff>/<a:chExt>`) and an outer placement (`<a:off>/<a:ext>`) — the ratio
  between them is the effective child-to-parent scale, which LO's import folds into the group's
  transform matrix.

---

## B. PPTX / OOXML PresentationML import (`oox/`)

### B.1 Package/fragment structure

`oox/` is a generic OOXML-parsing framework (fragment handlers driven by a fast SAX-like XML
parser keyed by pre-registered integer tokens, `oox/source/token/`, not text comparisons) shared
by Writer/Calc/Impress OOXML import. For presentations:

```
PresentationFragmentHandler        oox/source/ppt/presentationfragmenthandler.cxx
  parses ppt/presentation.xml:
    <p:sldIdLst>  -> list of slide part relationship ids (in display order)
    <p:sldMasterIdLst> -> master parts
    <p:notesMasterIdLst>, <p:handoutMasterIdLst>
    <p:sldSz>, <p:notesSz>  -> EMU slide/notes dimensions
    <p:defaultTextStyle>    -> presentation-wide default TextListStyle
  for each slide id -> creates/queues SlideFragmentHandler
  for each master id -> creates/queues SlideMasterFragmentHandler

SlideMasterFragmentHandler          (extends LayoutFragmentHandler? see below)
  parses ppt/slideMasters/slideMasterN.xml:
    <p:cSld><p:spTree> shapes (placeholders with default geometry)
    <p:clrMap>          -> theme-slot -> actual scheme-color remap for this master
    <p:sldLayoutIdLst>   -> layouts belonging to this master
    <p:txStyles>         -> titleStyle / bodyStyle / otherStyle (per-outline-level TextListStyle)
  loads the master's theme via r:id -> ThemeFragmentHandler (theme1.xml)

LayoutFragmentHandler                oox/source/ppt/layoutfragmenthandler.cxx
  parses ppt/slideLayouts/slideLayoutN.xml (same shape/spTree structure as a slide)
  <p:sldLayout ... matchingName="..." type="title|obj|twoObj|...">
  links back to its SlideMasterFragmentHandler via r:id

SlideFragmentHandler                 oox/source/ppt/slidefragmenthandler.cxx
  parses ppt/slides/slideN.xml:
    <p:cSld><p:spTree>  -> actual shapes (may be placeholders referencing layout by idx/type)
    <p:clrMapOvr>       -> optional per-slide color-map override
    <p:transition>       -> SlideTransition (slidetransition.cxx / slidetransitioncontext.cxx)
    <p:timing>            -> animation TimeNodeContext tree (see below)
  links to its layout via r:id; layout links to master; master links to theme.
```

Each of these ultimately builds a **`SlidePersist`** (`include/oox/ppt/slidepersist.hxx`) — the
in-memory staging object for one slide/layout/master/notes-page before its shapes are materialized
into real UNO `XShape`s (`SlidePersist::createXShapes()`). It holds:
- `mxPage` (target `XDrawPage`), `mpMasterPagePtr` (parent `SlidePersist`, chaining
  slide→layout→master — `setMasterPersist/getMasterPersist`, `slidepersist.hxx:77-78`),
  `mpThemePtr`, `mpClrMapPtr` (the resolved `<p:clrMap>`/`<p:clrMapOvr>` for *this* level),
  `maShapesPtr` (root shape tree), `maShapeMap` (id→`ShapePtr`, for connector/animation target
  lookups), and **five separately-tracked default text styles**:
  `maDefaultTextStylePtr / maTitleTextStylePtr / maBodyTextStylePtr / maNotesTextStylePtr /
  maOtherTextStylePtr` (`slidepersist.hxx:102-106`) — corresponding to `<p:txStyles><p:titleStyle>/
  <p:bodyStyle>/<p:otherStyle>` on the *master*, consulted when a placeholder shape doesn't specify
  its own paragraph/character properties (see B.3).

### B.2 DrawingML shape import (`oox/source/drawingml/`)

`Shape` (`oox/source/drawingml/shape.cxx`, `include/oox/drawingml/shape.hxx`) is the universal
staging shape object (analogous to `SdrObject` but pre-materialization). Key fields/behaviors:
- `moSubTypeIndex` (`shape.hxx:191`) — placeholder `idx` (from `<p:ph idx="n">`); together with
  the placeholder `type` token this is the join key against the layout's/master's matching
  placeholder shape.
- `addShape()` (`shape.hxx:214`) walks: resolve service name for the shape's geometry kind →
  create the real UNO shape → `createAndInsert()` (`shape.hxx:300`) applies transform, then fill/
  line/effect/text properties (merging with inherited placeholder properties, see B.3) →
  `finalizeXShape()` (`shape.hxx:330`) hook for shape-type-specific post-processing (e.g. table
  cell population, OLE object linking, diagram layout).
- Contexts: `ShapeContext`/`ShapeGroupContext` (generic `<p:sp>`/`<p:grpSp>`),
  `GraphicShapeContext` (`<p:pic>`), `ConnectorShapeContext` (`<p:cxnSp>`), plus PPT-specific
  `PPTShape`/`PPTShapeContext` (`oox/source/ppt/pptshape.cxx`, `pptshapecontext.cxx`) which add
  slide-specific concerns (placeholder resolution, OLE/graphic-frame handling for embedded
  charts/tables/SmartArt via `<p:graphicFrame>`).
- **Fill** (`FillProperties`, `fillproperties.cxx`) — solid/gradient/pattern/blip(picture)/group
  fill, resolved against theme fill style matrices (`<a:fillStyleLst>` index referenced by
  `<p:style><a:fillRef idx="n">`).
- **Line** (`LineProperties`, `lineproperties.cxx`) — width, dash (preset or custom `<a:custDash>`),
  cap (`flat/round/sq`), join (`round/bevel/miter` + miter limit), head/tail arrowheads
  (type/width/length enums), compound line (`sng/dbl/thickThin/thinThick/tri`).
- **Effects** (`effectproperties.cxx`, `effectpropertiescontext.cxx`) — outer/inner shadow
  (`<a:outerShdw>`/`<a:innerShdw>`, with blur radius, direction, distance, alpha), glow
  (`<a:glow>`), soft edge (`<a:softEdge>`), reflection (`<a:reflection>` — **not modeled** by
  `SDRATTR_*` beyond a boolean-ish support in some renderers; treat as lower priority, see F),
  presets via `<a:effectLst>` vs. the more complex `<a:effectDag>` (rare; effectDag is largely
  ignored/flattened).
- **Text** (`textbody.cxx`, `textbodyproperties.cxx`, `textparagraph.cxx`,
  `textcharacterproperties.cxx`, `textliststyle.cxx`) — full `<p:txBody>`/`<a:bodyPr>`/`<a:lstStyle>`/
  `<a:p>`/`<a:pPr>`/`<a:r>`/`<a:rPr>` tree, converted into an `oox::drawingml::TextBody`/
  `TextParagraph`/`TextCharacterProperties` intermediate model, later pushed into an
  `OutlinerParaObject` when the real shape is created (`finalizeXShape`/dedicated text-import path
  ties into `editeng`).
- **Table** (`oox/source/drawingml/table/`) — `<a:tbl>` import into `SdrTableObj`.
- **Diagram/SmartArt** (`oox/source/drawingml/diagram/`) — parses `data1.xml` (data model:
  `datamodelcontext.cxx`) + `layout1.xml` (layout algorithm tree: `diagramlayoutatoms.cxx`,
  `layoutnodecontext.cxx`, "layout atoms" = a small declarative layout-algorithm VM: `alg`,
  `shape`, `presOf`, `constr`, `rule` nodes evaluated by `layoutatomvisitors.cxx`) +
  `quickStyle1.xml`/`colors1.xml`, and *executes* the SmartArt layout algorithm to synthesize
  concrete shapes (`diagramhelper_oox.cxx`). **This is one of the most complex subsystems** — a
  full SmartArt layout engine reimplementation is a large sub-project by itself (see F/G: consider
  rendering the pre-rendered fallback image (`<p:graphicFrame>` often also carries a `blipFill`
  fallback via `mc:AlternateContent`) rather than re-executing the layout algorithm, for a v1 port).
- **Charts** (`oox/source/drawingml/chart/`) — imports `<c:chartSpace>` (a *different* XML
  namespace/spec, DrawingML-Chart) into LO's `chart2` model; out of scope for this document's
  shape-model focus but relevant to extraction (chart data/series) and to rendering (chart2 has its
  own view/rendering pipeline, separate from `drawinglayer` shape primitives, though its output is
  still composited into the slide via an `SdrOle2Obj`).

### B.3 Placeholder & style inheritance chain — the #1 fidelity issue

This is the crux of pptx text/fill fidelity. Resolution order, most-specific first:

```
1. Shape's own explicit properties in <p:sp><p:spPr>/<p:txBody>            (highest priority)
2. Matching placeholder shape on the slide LAYOUT (matched by ph type+idx)
3. Matching placeholder shape on the slide MASTER  (matched by ph type, idx often implicit/absent
                                                     for title/body "generic" placeholders)
4. Master's <p:txStyles> level style for the placeholder's *outline level*
   (titleStyle for type="title"/"ctrTitle"; bodyStyle by lvl 0-8 for type="body"/"subTitle"/etc.;
    otherStyle for non-placeholder shapes' default paragraph props on that master)
5. Theme defaults: <p:defaultTextStyle> in presentation.xml (presentation-wide fallback), and
   <a:objectDefaults><a:spDef>/<a:lnDef>/<a:txDef> in theme1.xml (shape/line/text default styles)
6. Hardcoded OOXML schema defaults (e.g. default font, default 100% line spacing)
```

Matching a slide placeholder to its layout/master counterpart is by **`(type, idx)`** pair from
`<p:ph type="..." idx="n"/>`: `type` defaults to `"body"` if omitted, `idx` defaults to `0`;
`SlidePersist`'s shape map + `moSubTypeIndex` (`shape.hxx:191`) is the mechanism, and the actual
placeholder-property-merge happens while building each shape's final `PropertyMap` in
`Shape::addShape`/`finalizeXShape`, pulling geometry/fill/line from the matched layout placeholder
when the slide's own placeholder shape has *no explicit* `<p:spPr>` child override for that
category, and pulling text list-style levels from
`getTitleTextStyle()/getBodyTextStyle()/getOtherTextStyle()` (`slidepersist.hxx:103-106`) on the
`SlidePersist` chain (walking `getMasterPersist()` up to the master level) when a paragraph doesn't
specify its own `<a:pPr>`/`<a:lstStyle>` level entry.

**Per-paragraph-level list-style inheritance (`lvlXpPr`):** `<a:lstStyle>` (found on
`<p:txBody>`, on a layout/master placeholder, on `<p:txStyles><p:bodyStyle>`, or on
`<p:defaultTextStyle>`) is a **9-level array** (`lvl1pPr` .. `lvl9pPr`), each holding paragraph
defaults (bullet/numbering, indent, spacing, alignment) *and* a default run-properties
(`defRPr`) for that level. A paragraph's actual outline level (`<a:pPr lvl="n">`, 0-based, default
0) selects which `lvlNpPr` entry to inherit from at each stage of the chain above — i.e. inheritance
is **per-level**, not "first non-empty lstStyle wins as a whole". A C# implementation must resolve
this level-by-level: for a paragraph at level L, walk shape's own `lstStyle[L]` →
layout-placeholder's `lstStyle[L]` → master-placeholder's `lstStyle[L]` →
`bodyStyle/titleStyle/otherStyle[L]` on master → theme `txDef` → schema default, merging
(not replacing) at each step for properties not yet set (`textliststyle.cxx`,
`textparagraphproperties.cxx` implement this merge via an "optional-if-unset" pattern — every
character/paragraph property is an `std::optional<T>`-like wrapper, and merge = "take mine if set,
else take source's").

**Color inheritance** interacts with the above via the *theme color scheme* + `<p:clrMap>`: a
shape's color reference is often a **scheme slot token** (`bg1, tx1, bg2, tx2, accent1..6, hlink,
folHlink`, or ODF-legacy `dk1/lt1/dk2/lt2`), not a literal RGB. `<p:clrMap>` on the slide master
(and optional `<p:clrMapOvr>` on the slide, which can be `<a:overrideClrMapping>` for a full custom
map, or `<a:masterClrMapping/>` meaning "use master's map unchanged") remaps abstract slots
(`bg1→lt1`, `tx1→dk1`, etc. is the *typical* identity-ish default but master themes can swap them,
e.g. a dark-background master often maps `bg1→dk1`) *before* looking the color up in the theme's 12
scheme colors (`dk1,lt1,dk2,lt2,accent1-6,hlink,folHlink` — `oox/source/drawingml/color.cxx:212-240`
`constSchemeColorNameToIndex`). Then **color transforms** (`lumMod`, `lumOff`, `shade`, `tint`,
`satMod`, `hueMod`, and several others — full opcode set at `color.cxx:445-570`) are applied on top,
each modifying HSL components multiplicatively (`Mod`) or additively (`Off`) with clamping
(`color.cxx:200-210` `lclModValue`/`lclOffValue`) — resolved in `Color::getColor()`
(`color.cxx:723`), which special-cases `Theme` (scheme lookup via `GraphicHelper::getSchemeColor`),
`Palette`, `System`, and `Placeholder` (style-matrix placeholder color `phClr`, used inside
`<a:fillRef>/<a:lnRef>/<a:effectRef>` style-matrix references, where `phClr` is filled in with the
referenced accent/scheme color before any local transforms are applied — i.e. `<a:fillRef idx="2">
<a:schemeClr val="accent1"/></a:fillRef>` means "use fill-style-matrix slot 2, with placeholder
color resolved to accent1"). **`getComplexColor()`/`getLumMod()`/`getLumOff()`/`getTintOrShade()`
(`color.cxx:649-721`) show the exact 1000ths-of-a-percent→100ths-of-a-percent scaling** a C# port
must replicate bit-for-bit to avoid visible color drift on shade/tint-heavy themes (which is nearly
all default Office themes — accent color tint/shade ramps are how PowerPoint derives its palette
row of lighter/darker variants).

**Style-matrix references** (`<p:style><a:lnRef idx="n">/<a:fillRef idx="n">/<a:effectRef idx="n">/
<a:fontRef idx="n">`): index into the theme's `<a:fmtScheme>` (`<a:fillStyleLst>`,
`<a:lnStyleLst>`, `<a:effectStyleLst>`, each with exactly 3 entries — subtle/moderate/intense — plus
`<a:bgFillStyleLst>`) rather than into an arbitrary property; idx 0 conventionally means "no
style"/none, 1000+ can mean background fill list. This is how "shape styles" (the visual gallery
PowerPoint shows for "Shape Styles" ribbon) actually work — resolve the matrix entry, then still
apply the same `phClr`-substitution + transform pipeline as above.

### B.4 Theme (`oox/source/drawingml/theme.cxx`, `themeelementscontext.cxx`)

`<a:theme><a:themeElements>` = `<a:clrScheme>` (12 colors) + `<a:fontScheme>` (`majorFont`/
`minorFont`, each with Latin/East-Asian/Complex-script typeface + per-script overrides) +
`<a:fmtScheme>` (the 3 style-lists above) + `<a:objectDefaults>` (`spDef`/`lnDef`/`txDef` — default
properties applied to any shape/line/text that doesn't otherwise reference a style, essentially the
"zero-th" fallback before hardcoded schema defaults). `ThemeFragmentHandler`
(`oox/source/drawingml/themefragmenthandler.cxx`) parses this into a `Theme`/`ClrScheme` object
attached to each `SlidePersist` (shared instance across all slides/layouts under one master —
important for perf and for correctly reflecting "same theme instance" edits, less important for a
render-once C# port, but still worth caching per master to avoid re-parsing).

### B.5 Autofit / normAutofit — extraction-time application

Already covered in A.4; restated for the OOXML side: `<a:bodyPr><a:normAutofit fontScale="N"
lnSpcReduction="M"/>` — both values are 1000ths-of-a-percent (`textbodypropertiescontext.cxx:240-243`
divides by `100000.0`). Apply `fontScale` as a multiplier on every run's effective font size, and
`(1 - lnSpcReduction)` as a multiplier on line spacing, for that text body. `<a:spAutoFit/>` instead
means "grow the shape to fit text" (maps to `TextAutoGrowHeight = true`, suppressed for vertical
text modes — `textbodypropertiescontext.cxx:246-251`). `<a:noAutofit/>` disables both.

---

## C. PPT (binary) import (`sd/source/filter/ppt`, `filter/source/msfilter`)

Legacy binary `.ppt` is an **OLE2/CFB compound file** containing streams (`PowerPoint Document`,
`Current User`, `Pictures`, etc.), and within the main stream, a tree of **records** — each an
8-byte header (`DFF_COMMON_RECORD_HEADER_SIZE`, `include/svx/msdffdef.hxx:24`: 2-byte
ver/instance, 2-byte record type, 4-byte length) forming either an atom (leaf data) or a container
(nested records), read via `DffRecordHeader` (`include/filter/msfilter/dffrecordheader.hxx`).

Two overlapping record vocabularies exist and can appear in the same file:

- **PPT-specific records** (`PPT_PST_*`, `include/filter/msfilter/svdfppt.hxx:1297+`): e.g.
  `PPT_PST_Document=1000`, `DocumentAtom=1001`, `SlideAtom=1007`, `NotesAtom=1009`,
  `Environment=1010`, `SlidePersistAtom=1011`, `SSSlideInfoAtom=1017` (slide-show/transition info),
  plus (not fully enumerated above, but present in the same block) `TextHeaderAtom`,
  `TextCharsAtom`, `TextBytesAtom`, `StyleTextPropAtom`, `TextSpecInfoAtom`, `MasterPersistAtom`,
  `SlideListWithText`, `UserEditAtom`, `PersistPtrIncrementalBlock` — the "presentation structure"
  layer (slide list, per-slide persist offsets, text runs+styles, headers/footers).
- **Escher / MS-ODRAW records** (`DFF_msofbt*`, `include/svx/msdffdef.hxx:38+`): the *drawing*
  layer shared with Word/Excel — `DggContainer`/`Dgg` (document-wide shape-id/BLIP-store info),
  `BstoreContainer`/`BSE` (image store), `DgContainer`/`Dg` (per-slide drawing), `SpgrContainer`/
  `Spgr`/`SpContainer`/`Sp` (shape group tree / individual shape), `OPT` (the shape's "shape
  option" property bag — this is where fill/line/effect/geometry properties live, keyed by
  `DFF_Prop_*` ids, `include/svx/msdffdef.hxx:151` e.g. `DFF_Prop_adjustValue=327`, plus ~150 more
  covering fill type/color/blip-id, line color/width/dash, shadow, rotation, wrap text, autoshape
  adjustments, text-on-shape flags, etc.).

**Persist directory mechanism**: PPT files are edited incrementally (each save can append new
records rather than rewriting the file), so a **persist-object directory**
(`PersistPtrIncrementalBlock` atoms building a `PptSlidePersistList`,
`include/filter/msfilter/svdfppt.hxx:385`) maps persist-IDs to *byte offsets* in the stream, and a
`UserEditAtom` chain (`PptUserEditAtom`, `svdfppt.hxx:309`) records the sequence of edit sessions —
the importer walks backward from the *last* `UserEditAtom` to find the current persist directory
(older/superseded records are simply orphaned garbage in the stream, not deleted). A C# binary-ppt
reader must replicate this "follow the edit-atom chain to the latest persist map" logic rather than
naively scanning the stream top-to-bottom, or it risks picking up stale/undone content.

**Import driver classes** (`include/filter/msfilter/svdfppt.hxx`):
- `SvxMSDffManager` (`include/filter/msfilter/msdffimp.hxx`, impl `filter/source/msfilter/
  msdffimp.cxx`, 7676 lines) — the shared Escher/ODRAW decoder (also used by Word/Excel binary
  import). Decodes shape trees, resolves `OPT` property sets into `EscherPropertyContainer`
  (`dffpropset.hxx`/`.cxx`) key→value maps, builds `SdrObject`s from shape-type + properties,
  including mapping legacy MSO autoshape types directly onto the `EnhancedCustomShapeGeometry.cxx`
  tables from A.5 (same table, shared with binary AND, transitively, with a few OOXML presets).
- `SdrEscherImport : SvxMSDffManager` (`svdfppt.hxx:453`) — Impress/Draw-flavored subclass.
- `SdrPowerPointImport : SdrEscherImport` (`svdfppt.hxx:551`) — the top-level PPT-specific driver:
  walks `PptSlidePersistEntry` list (`svdfppt.hxx:349`), rebuilds slide/master/notes pages, styles
  (`PPTStyleSheet`, `svdfppt.hxx:853`, built from `PPTCharSheet`/`PPTParaSheet` per outline level,
  themselves from `PPTCharLevel`/`PPTParaLevel` — the binary-format equivalent of OOXML's
  `titleStyle/bodyStyle` level arrays).
- **Text**: `PPTTextObj`/`ImplPPTTextObj` (`svdfppt.hxx:1226/1196`) assembles per-shape text from
  `TextHeaderAtom` (announces kind: title/body/other + encoding) + `TextCharsAtom`(Unicode)/
  `TextBytesAtom` (8-bit, legacy code page) for raw characters, `StyleTextPropAtom` for
  character/paragraph run styling (parsed via `PPTStyleTextPropReader`, `svdfppt.hxx:1047`, into
  `PPTCharPropSet`/`PPTParaPropSet` run lists, `svdfppt.hxx:930/899`), and `TextSpecInfoAtom`
  (language/spell-check-adjacent per-run info, `PPTTextSpecInfoAtomInterpreter`, `svdfppt.hxx:674`).
  `PPTParagraphObj`/`PPTPortionObj` (`svdfppt.hxx:1137/1092`) are the paragraph/run assembly
  classes, ultimately feeding an `Outliner` to build the real `OutlinerParaObject` — architecturally
  parallel to how OOXML's `TextParagraph`/`TextCharacterProperties` feed the same target.
- `ppt97animations.cxx`/`pptinanimations.cxx` (`sd/source/filter/ppt/`) — legacy PPT97-era
  animation-info atoms (simpler than the OOXML `<p:timing>` SMIL-like tree; a distinct, older
  animation model to special-case if binary-ppt animation extraction is in scope).

**Note on the "eppt" export side** (`sd/source/filter/eppt/`) — this is LO's binary-PPT *writer*,
useful only as a cross-reference for record shapes when the *reader*-side comments are terse; not
needed for an import/render-only C# port, but the header `escherex.hxx`/`epptbase.hxx` there
double as a second (writer's-eye-view) enumeration of `PPT_PST_*`/Escher constants.

---

## D. ODP import (`xmloff/source/draw/`)

ODF drawing import reuses the generic `xmloff` SAX-context-per-element framework. Key files:
- `sdxmlimp.cxx`/`sdxmlimp_impl.hxx` — top-level `SdXMLImport` (extends generic `SvXMLImport`),
  dispatches `office:body`/`office:presentation` or `office:drawing`.
- `ximpbody.cxx` — body-level context, iterates `draw:page` (slides) and
  `style:presentation-page-layout` / `draw:layer-set` / master styles.
- `ximppage.cxx` — one `draw:page`: page properties (`draw:style-name` → its own
  `style:style` `style:family="drawing-page"`, which is where **background fill** and
  **presentation:page-layout-name** — the ODF analogue of OOXML's autolayout — live), iterates
  child shape elements, and (via `presentation:placeholder` children or a shape's own
  `presentation:class` attribute — `title/outline/subtitle/notes/...`, matching `sd`'s
  `PresObjKind`) registers presentation/placeholder shapes.
- `ximpshap.cxx` (large — the ODF analogue of oox's `shape.cxx`) — per-draw-element-type context
  factory: `draw:rect`, `draw:ellipse`, `draw:polygon/polyline`, `draw:path`, `draw:frame` (generic
  container for image/OLE/plugin/text-box content — ODF wraps most "rich" content in
  `draw:frame` + a typed child, unlike OOXML's more type-specific top-level elements),
  `draw:custom-shape` (see below), `draw:connector`, `draw:caption`, `draw:g` (group),
  `draw:control` (form controls), 3D (`ximp3dobject.cxx`/`ximp3dscene.cxx`).
- `ximpcustomshape.cxx` — parses `draw:custom-shape` + its child `draw:enhanced-geometry`
  (attributes: `draw:type`, `draw:enhanced-path` (path command string, similar vocabulary to
  OOXML's but ODF's own textual mini-language: `M/L/C/Z/N/X/...` letters with parameter refs like
  `?f0`), `draw:modifiers` (adjustment values), `draw:glue-points`, nested
  `draw:equation` elements (`draw:formula` using the *named-function* syntax parsed by
  `EnhancedCustomShapeFunctionParser.cxx`, contrasting with OOXML's positional-opcode style — see
  A.5) — feeding the *same* `SdrCustomShapeGeometryItem`/`EnhancedCustomShape2d` pipeline as OOXML
  and binary-ppt shapes converge on. This convergence is a strong argument for a C# port to
  likewise normalize all three input formats onto one internal custom-shape representation early.
- `ximpstyl.cxx` — style family import: `style:style` (graphic/paragraph/text/drawing-page
  families), **master pages** (`style:master-page`, referenced from a `draw:page`'s
  `draw:master-page-name` — a *direct single-level* master reference, matching `sd`'s internal
  2-level model much more directly than OOXML's 3-level slide/layout/master, since ODF has no
  separate "layout" concept — layout-equivalent info is folded into
  `style:presentation-page-layout` + `presentation:placeholder`/autolayout on the page itself),
  and outline-level presentation styles (`style:style style:family="presentation"`, one per
  `PresObjKind`/outline-level combination — the direct serialization of `SdStyleSheetPool`,
  A.2). `layerimp.cxx` handles `draw:layer-set`/layer visibility.
- `animationimport.cxx` — `anim:` namespace SMIL-based animation import (ODF's animation model is
  itself SMIL-flavored, arguably closer to OOXML's `<p:timing>` tree than to legacy binary-ppt's
  flatter model — see E below on the shared conceptual root).
- `numithdl.cxx`/`XMLNumberStyles*.cxx` — numbering/bullet style import for outline text.

Because ODF's placeholder/style model is shallower (2-level, no separate layout object) than
OOXML's, an ODP importer is considerably simpler for inheritance resolution — mostly relevant to a
C# port as a *sanity-check* target: if your unified internal model correctly special-cases "no
layout level" for ODP (or synthesizes a pass-through layout), the OOXML resolution code path (B.3)
should degenerate correctly rather than needing separate logic.

---

## E. Slide rendering path

### E.1 The primitive2d pipeline, conceptually

LibreOffice's modern 2D rendering (`drawinglayer/`) is a **declarative scene-graph decomposition**
model, not immediate-mode painting:

```
SdrObject (model)
   └─ sdr::contact::ViewContact           (1 per SdrObject; "view-independent" decomposition)
        ViewContactOfSdrRectObj / OfSdrPathObj / OfSdrObjCustomShape / OfSdrPage / OfGroup / ...
        (svx/source/sdr/contact/viewcontactof*.cxx)
        .createViewIndependentPrimitive2DSequence()   -- turns model state into Primitive2DContainer
             (include/svx/sdr/contact/viewcontact.hxx:83)
   └─ sdr::contact::ViewObjectContact      (1 per (SdrObject, ObjectContact) pair — supports the
        same object appearing differently in different views, e.g. master-page shapes shown
        through multiple slides, or print vs. screen)
   └─ sdr::contact::ObjectContact          (1 per rendering target: ObjectContactOfPageView for
        an on-screen/print SdrPageView, ObjectContactOfObjListPainter for isolated/offscreen
        rendering of a subset of objects — this is the one a headless renderer typically drives)
```

`createViewIndependentPrimitive2DSequence` for a shape decomposes into a tree of
**`Primitive2D`** objects (`drawinglayer/source/primitive2d/`, ~60 primitive kinds), e.g. for a
filled+stroked+shadowed+text rectangle: a `SdrRectanglePrimitive2D`-family wrapper eventually
bottoms out into `PolyPolygonColorPrimitive2D`/`PolyPolygonGradientPrimitive2D`/
`PolyPolygonHatchPrimitive2D`/`FillGraphicPrimitive2D` (fill), `PolyPolygonStrokePrimitive2D`/
`BorderLinePrimitive2D` (line, dash/cap/join resolved into actual stroked polygon geometry at this
stage — *not* left as "draw a dashed line" instructions, the dash pattern is pre-expanded into
polygon segments), `ShadowPrimitive2D` (offset+blur/perspective shadow, wrapping the shape's own
fill/stroke primitives), `GlowPrimitive2D`/`SoftEdgePrimitive2D` (post-effect wrapping — see
`svx/source/sdr/primitive2d/sdrdecompositiontools.cxx:939-971`
`createEmbeddedGlowPrimitive`/`createEmbeddedSoftEdgePrimitive` — these literally wrap the shape's
content primitives in an effect primitive that a renderer processes as "render content to an
offscreen buffer, then blur/glow/composite"), and text via `SdrTextPrimitive2D`/
`SdrAutoFitTextPrimitive2D` (deferring actual glyph layout to `TextBreakupHelper`/EditEngine
integration — `textbreakuphelper.cxx`). Custom shapes have their own top-level
`SdrCustomShapeprimitive2d.cxx` decomposition, gradients `fillgradientprimitive2d.cxx`
(supports linear/axial/radial/elliptical/square/rect — a superset of ODF's own gradient kinds,
covering OOXML's `<a:gradFill>` path/linear types plus tile-rect for `<a:tileRect>`).

Key architectural point for a C# port: **primitives are resolution-independent and metric, not
pixel commands** — a `Primitive2D` tree can be walked by *different* processors (see E.2) to
produce pixel output, a `GDIMetaFile` (vector recording, used for OLE/clipboard/print-preview),
bounding-box-only hit testing, or text/contour extraction, without re-deriving shape geometry each
time. A from-scratch C# renderer doesn't need to copy this exact indirection (you can go
shape→pixels directly), **but the primitive taxonomy itself is a very good checklist of "what visual
features must be supported"** — treat the list of primitive2d source files as your rendering
feature checklist (fills: color/gradient/hatch/bitmap/pattern; strokes: hairline/full with
dash-expansion; effects: shadow/glow/softedge; text: with/without autofit; media: still-frame for
video/animated gif first-frame; 3D: embedded-3D and scene primitives if you support legacy 3D
shapes/charts).

### E.2 Processors — turning primitives into pixels

`drawinglayer/source/processor2d/`:
- `VclPixelProcessor2D` (`vclpixelprocessor2d.cxx`) — the traditional VCL (LO's platform
  abstraction) `OutputDevice`-based rasterizer; walks the primitive tree and calls VCL drawing
  primitives (`DrawPolyPolygon`, `DrawGradient`, `DrawBitmap`, ...), relying on the platform
  backend (X11/Skia/Cairo/etc. under VCL) for actual rasterization.
- `CairoPixelProcessor2D` (`cairopixelprocessor2d.cxx`) — a more direct Cairo-backed path (used
  when VCL's Cairo/Skia backend is selected), for higher-fidelity anti-aliasing/gradients/effects.
- `VclMetafileProcessor2D` (`vclmetafileprocessor2d.cxx`) — renders into a `GDIMetaFile` (vector
  action list) instead of pixels — this is what feeds OLE embedding, clipboard, and (importantly)
  **PDF export**, since `vcl/source/gdi/pdfwriter*` consumes metafile-like action streams to emit
  real PDF vector operators (so text/fills/strokes stay vector in the PDF, not rasterized) rather
  than rendering to a bitmap and re-embedding it.
- `D2DPixelProcessor2D` (`d2dpixelprocessor2d.cxx`) — Windows Direct2D backend equivalent.
- Non-visual processors: `HitTestProcessor2D`, `ContourExtractor2D`, `TextExtractor2D`,
  `ObjectInfoExtractor2D`, `LineGeometryExtractor2D` — these are the ones most relevant to a
  **content-extraction** (as opposed to rendering) C# port: they demonstrate how to walk the exact
  same primitive/shape decomposition to pull out plain text, geometric outlines, or per-run
  metadata, guaranteeing extraction and rendering never disagree about what a shape "is".

For a from-scratch **C# renderer**, the practical target is closer to
`VclMetafileProcessor2D`+`pdfwriter` for PDF output (emit real vector operators: path fill/stroke,
text runs with font/position, image XObjects) and closer to `VclPixelProcessor2D` for raster PNG
(rasterize fills/strokes/gradients/text into a pixel buffer — e.g. via `System.Drawing`/SkiaSharp/
ImageSharp equivalents). Both need the *same* upstream geometry resolution (shape→final polygon,
including custom-shape evaluation, connector routing, text layout with autofit already applied) —
build that shared layer once.

### E.3 Headless rendering entry point

`soffice --headless --convert-to png/pdf` ultimately reaches:
- `SdXImpressDocument` implements `css::view::XRenderable` (`sd/source/ui/unoidl/unomodel.cxx`):
  `getRendererCount()`/`getRenderer()` (page count / per-page render hints, `unomodel.cxx:3087-3117`)
  and **`render(nRenderer, rSelection, rxOptions)`** (`unomodel.cxx:3497+`). This method:
  1. Resolves target page (`nRenderer+1` = 1-based slide index; honors `"ExportNotesPages"` option
     to switch to the notes-page variant of the same slide index — `unomodel.cxx:3517-3522`).
  2. Reads `"RenderDevice"` option → an `awt::XDevice` → unwraps to a real VCL `OutputDevice`
     (`unomodel.cxx:3528-3529`) — for PDF export this is a PDF-writing device
     (`vcl::PDFExtOutDevData`, detected via `dynamic_cast` at `unomodel.cxx:3534`); for PNG export
     it's a virtual-device/bitmap target set up by the generic graphic-export filter machinery.
  3. Builds an offscreen `sd::ClientView` (`::sd::View` subclass) bound to that `OutputDevice`,
     with UI chrome disabled (`SetHlplVisible/SetGridVisible/SetBordVisible/SetPageVisible/
     SetGlueVisible(false)` — `unomodel.cxx:3556-3560`) and clips to the page's visArea in
     1/100mm map mode (`unomodel.cxx:3547,3562-3563`).
  4. Resolves the page background color with `bScreenDisplay=false` when the target is a
     printer/PDF device (`OUTDEV_PRINTER`/`OUTDEV_PDF` check, `unomodel.cxx:3596-3599`) to suppress
     "auto color" (theme-aware UI chrome color) substitution that only makes sense on-screen.
  5. For PDF targets specifically, also emits link/hyperlink annotations and (optionally) slide
     transition metadata and comments as PDF-native annotations (`ImplPDFExportShapeInteraction`,
     `ImplPDFExportComments`, seen inline in the same function) — i.e. **PDF export is not just
     "render to vector graphics", it also carries semantic annotations that a pure rasterizer
     would drop**; relevant if your C# PDF export needs to preserve hyperlinks/notes.
  6. Actual painting happens via the normal `SdrPaintView`/`ObjectContact` → primitive2d pipeline
     (E.1/E.2) triggered by the view's redraw, *not* shown inline in this function (it's invoked
     through the generic drawing-layer view painting further down the call chain into
     `sd::View`/`SdrPaintView::CompleteRedraw` → `ObjectContactOfObjListPainter` or
     `ObjectContactOfPageView` → `VclPixelProcessor2D`/`VclMetafileProcessor2D`).
- The user-facing `--convert-to` flow (outside `sd/`, in `desktop`/`sfx2`/framework filter
  dispatch) ultimately calls this same `XRenderable` interface per-page (for PNG/JPG each page is a
  separate `GraphicExporter` invocation with size derived from the slide's 1/100mm size at the
  requested DPI; for PDF, `filter/source/pdf/pdfexport.cxx`+`pdffilter.cxx` drive one `render()`
  call per page into a single accumulating PDF writer device, then finalize the PDF byte stream).
- `sd/source/ui/view/DocumentRenderer.cxx` is the *UI-facing* Print/Export dialog's use of the same
  renderable interface (handout/multi-slide-per-page layout, print ranges) — a useful reference for
  handout-page layout math (N-slides-per-page grid + optional slide numbers) if the C# port needs
  to reproduce Impress's handout export, not just per-slide export.

### E.4 Units

- **Internal drawing-layer unit**: 1/100 mm ("hundredths of a millimeter", `tools::Rectangle`/
  `Point`/`Size` integer coordinates) — this is ODF's native unit too, so ODP round-trips without
  conversion.
  - Also frequently expressed as `MapUnit::Map100thMM` (VCL `MapMode`).
- **OOXML unit**: EMU — English Metric Units, 914400 per inch, 360000 per centimeter, exactly
  36000 per 1/100mm (`convertHmmToEmu`/`convertEmuToHmm`,
  `include/oox/drawingml/drawingmltypes.hxx:181-190`, both implemented via the generic
  `o3tl::convert`/`o3tl::Length` unit-conversion helpers — `o3tl::Length::mm100` ↔
  `o3tl::Length::emu`). Also `convertEmuToPoints` (`drawingmltypes.hxx:193-196`) for font-size-
  adjacent conversions (though font sizes in OOXML `<a:rPr sz="n">` are already in **100ths of a
  point**, a *third* unit, not EMU — don't conflate).
- **Rotation**: OOXML stores rotation as **60,000ths of a degree, clockwise**, on `<a:xfrm rot="n">`;
  LO stores rotation as **100ths of a degree, counter-clockwise**, with values normalized to a
  canonical range. Conversion (`drawingmltypes.hxx:170-178` `calcRotationValue` — note this
  particular helper is actually used in the *export* direction in that file, but the inverse
  arithmetic is the one to replicate on import: `internal_100th_deg = -(ooxml_60000th_deg / 600)`,
  then normalize to e.g. `[0, 36000)`).
  - Also watch the **flip-then-rotate vs rotate-then-flip** order: OOXML applies `flipH`/`flipV`
    *before* `rot` in its transform composition (`<a:xfrm rot="r" flipH="1">`: mirror first, then
    rotate by r about the shape's own center) — get the matrix composition order wrong and shapes
    with both flip and non-trivial rotation will end up mirrored across the wrong axis.
- **Font size**: OOXML 100ths of a point (`sz="1800"` = 18pt); LO internal character height is
  also point-based (`editeng` `SvxFontHeightItem` in 1/100 pt for OOXML-originated docs, though
  ODF stores font size as plain points/percent in its style XML) — low conversion risk but keep
  the "100ths of a point" scale factor explicit in a C# model rather than assuming "points" bare.
- **Percentages** in OOXML are commonly 1000ths of a percent (`fontScale="90000"` = 90%,
  `lumMod val="60000"` = 60%) — *not* the same scale as adjustment-value percentages elsewhere;
  always check the specific attribute's documented scale (100 000 = 100% is the dominant OOXML
  convention across color transforms and autofit, but legacy/ODF formula percentages are often
  plain 0-100 or 0-1).

---

## F. Top ~20 features that dominate visual fidelity (ranked)

Ranked by (frequency across real-world decks) × (visual impact if wrong). Pointers reference the
sections above.

1. **Placeholder inheritance chain (shape→layout→master→theme, per text level)** — B.3. Gets font,
   size, color, bullet, and fill wrong on nearly every slide if mishandled; the single highest-ROI
   subsystem to get exactly right.
2. **Theme color scheme + clrMap + lumMod/lumOff/tint/shade resolution** — B.3/B.4. Wrong shade
   ramps make entire decks look "off-brand"; very common (accent color variants are everywhere).
3. **Font/text run character properties + run-level color/bold/italic/underline resolution** —
   B.2/B.3, A.4. Basic but must be exactly right (most visible element on every slide).
4. **normAutofit fontScale/lnSpcReduction application** — A.4/B.5. Extremely common on
   title/body placeholders; skipping it makes text overflow shapes it shouldn't.
5. **Custom shape preset geometry (prstGeom) evaluation, incl. adjustment values** — A.5.
   Rectangles/ellipses are trivial, but rounded-rects, callouts, arrows, stars, banners are
   everywhere and each has nontrivial adjustable geometry; getting the *default* adjustment values
   right matters even when the doc doesn't override them.
6. **Solid/gradient/picture fill rendering, incl. theme fillRef style-matrix resolution** —
   A.3/B.2/B.3. Gradients (esp. multi-stop, angle, tile) are common in modern templates.
7. **Line/stroke rendering incl. dash patterns, caps/joins, arrowheads** — A.3/B.2.
8. **Shadow (outer/inner), with correct blur/offset/alpha** — A.3/B.2/E.1. Default theme shape
   styles frequently include a subtle shadow; missing it is subtly "flat"-looking wrong.
9. **Table cell borders/fills + text** — A.6/B.2. Extremely common content type; border-resolution
   edge cases (adjacent-cell precedence) are easy to get subtly wrong.
10. **Group shape transform composition (incl. child-offset/extent scaling)** — A.6. Nested groups
    with non-uniform scale are common from copy-pasted content; a transform bug compounds visibly.
11. **Slide/master/layout background fill inheritance (incl. "use slide background" for masters)**
    — A.2/B.3. Wrong background = wrong contrast for everything on the slide.
12. **Picture crop (`srcRect`)/stretch/tile and image color effects (duotone, grayscale, alpha
    fix)** — A.3 (`SDRATTR_GRAF*`)/B.2. Common on photo-heavy decks.
13. **Connector routing (orthogonal/curved) between shape glue points** — A.6. Diagrams/flowcharts
    rely on this; wrong routing is very visually obvious.
14. **Vertical text / text rotation / writing mode** — A.4. Less common but total failure (garbled
    layout) when present, notably in Asian-market templates.
15. **Text box autofit-to-shape (spAutoFit) and shape-fit-to-text sizing** — A.4/B.5.
16. **Glow / soft edge effects** — A.3/B.2/E.1. Increasingly common in modern Office themes
    (button-like shape styles).
17. **Slide transition + basic entrance/exit animation extraction (even if not "played")** — A.2,
    B.1 (`<p:timing>`)/E — needed if the target includes animation metadata, not just static
    render; getting transition *type* right matters for any downstream player, even a simplified
    one.
18. **SmartArt (diagram) fallback rendering** — B.2. Full layout-algorithm execution is very high
    effort; even just correctly reading pre-rendered fallback images and text content preserves a
    lot of value at much lower cost — see G phasing.
19. **Chart rendering (chart2 data + basic chart types)** — B.2 (chart import), out of core scope
    here but omnipresent in business decks; a v1 that only extracts chart *data*/*category labels*
    without full native chart rendering is a reasonable phase-1 cut (render a placeholder or a
    simplified bar/line/pie).
20. **3D/embedded-OLE objects and legacy WordArt (`prstTxWarp` fontwork)** — A.5 (`fontwork-*`
    mapping), A.6 (E3dObject). Low frequency in modern decks, safe to defer.

---

## G. C# reimplementation notes and phased plan

### G.1 Recommended internal object model

Do **not** mirror `SdrObject`'s "everything is secretly a text object" inheritance quirk (A.1) or
the 250-item flat `SfxItemSet` (A.3) directly. Instead:

```
Presentation
  Theme[]                (dedup'd by identity/hash — many slides/masters share one)
  MasterSlide[]           -> Background, PlaceholderDefaults[(type,idx) -> ShapeStyle], TextStyles[titleStyle,bodyStyle(9 lvls),otherStyle]
    Layout[]              -> same shape as MasterSlide but with a MasterSlide parent ref
       Slide[]            -> Shapes[], Background?, Transition?, Timing/Animations?, NotesSlide?
  Shape (abstract)
    Transform { Offset, Extent, Rotation, FlipH, FlipV }   // resolved to a single 2x3 matrix
    Fill?, Line?, Shadow?, Glow?, SoftEdge?, Reflection?
    Placeholder? { Type, Index }              // null for non-placeholder shapes
    + one of:
      GeometryShape { PresetOrCustomGeometry, TextBody? }   // rect/ellipse/customShape/etc, unified
      TextBox { TextBody }
      Picture { BlipRef, Crop, ColorEffects }
      GraphicFrame { Table | Chart | SmartArtRef | OleObject }
      Connector { StartConnection?, EndConnection?, RoutingKind }
      GroupShape { ChildShapes[], ChildCoordSpace }
TextBody { Paragraph[] }
Paragraph { Level(0-8), ParagraphProps (resolved+raw), Run[] | Field[] }
Run { Text, CharacterProps (resolved+raw) }
```

Keep **both** "raw as specified on this element" and a **lazily-computed "resolved/effective"**
view per shape/paragraph/run, rather than baking inheritance in at parse time — this lets you (a)
re-render cheaply if a master/theme changes, (b) unit-test inheritance resolution in isolation, and
(c) support ODP's shallower 2-level model by simply omitting the Layout level (Slide.Layout =
Slide.MasterSlide) without duplicating resolution logic.

### G.2 Bounded, data-heavy subsystems to port mechanically (not redesign)

- Custom-shape preset geometry table (A.5) — port `customshapepresetdata.cxx` (OOXML presets, ~180
  shapes) as data; port `EnhancedCustomShapeGeometry.cxx`'s legacy `mso_spt*` tables only if binary
  `.ppt` support is in scope (large effort, ~8.5k lines of tables).
- The equation/formula mini-VM (A.5, `EnhancedCustomShape2d`, `customshapegeometry.cxx`
  `FormulaCommand` set, `EnhancedCustomShapeFunctionParser.cxx` for ODF) — small, well-bounded,
  port faithfully (this is pure arithmetic, easy to unit test against known shapes).
- Color transform pipeline (B.3, `color.cxx:723+`) — small, critical, port bit-exact including the
  1000ths/100ths percent scaling and clamping order.
- Rotation/flip/unit conversion helpers (E.4) — trivial but easy to get subtly backwards (sign/
  order); port with unit tests using known-good sample files as oracles.

### G.3 High-effort subsystems to consider descoping or simplifying for v1

- SmartArt/diagram layout algorithm execution (B.2) — very large; v1 could render only extracted
  text/fallback image.
- Full chart2-equivalent chart rendering — separate project-sized effort; v1 could extract series
  data + render a basic bar/line/pie approximation, or embed the fallback image PowerPoint stores.
- Binary `.ppt` Escher decoding (C) — only build this if legacy `.ppt` input is a real requirement;
  it duplicates a large fraction of the OOXML custom-shape/text work with a different, older
  wire-format, for shrinking real-world relevance.
- Interactive-only concerns (drag transforms, glue-point editing UI, undo) — skip entirely; a
  render/extract-only port needs zero interactivity.
- Legacy StarOffice text-animation (`SDRATTR_TEXT_ANI*`, A.3) and old-school 3D lighting model
  (A.3, 3D scene items) — safe to deprioritize.

### G.4 Phased build order (suggested)

1. **OOXML core geometry + fill/line + text, no inheritance**: parse one slide's shapes with fully
   explicit (non-placeholder) properties; render solid-fill/solid-line rectangles/ellipses/text
   boxes to PNG. Validates unit conversion (E.4) and basic primitive rendering end-to-end.
2. **Theme + color transforms + clrMap** (B.3/B.4): scheme color resolution, lumMod/lumOff/tint/
   shade, style-matrix fillRef/lnRef/effectRef. Validates against decks using default Office
   themes (very high real-world coverage jump).
3. **Placeholder inheritance chain** (B.3): slide→layout→master→theme text/fill inheritance,
   per-level lstStyle resolution. This is the biggest fidelity unlock; budget the most time here.
4. **Custom shape preset geometry + adjustment values** (A.5): port the preset data table +
   equation evaluator; cover the ~30 most common presets first (rect variants, callouts, arrows,
   stars, flowchart shapes) before the long tail.
5. **Autofit (normAutofit/spAutoFit)** (A.4/B.5), **tables** (A.6/B.2), **groups/transforms**
   (A.6), **connectors** (A.6) — roughly parallelizable once (1)-(3) are solid.
6. **Effects**: shadow, glow, soft edge (A.3/B.2/E.1).
7. **Pictures**: crop, color effects, tiling (A.3, B.2).
8. **Master/slide background inheritance**, **notes pages**, **handout layout** (A.2/E.3).
9. **PDF export** via a vector-operator writer (E.2/E.3) once raster PNG rendering is trusted (PDF
   text-as-vector correctness is easiest to validate by diffing against the already-validated
   raster path).
10. **Animations/transitions extraction** (metadata only — A.2/B.1) if the target format needs to
    preserve them for a downstream player, even without implementing playback.
11. **SmartArt/charts/ODP/binary-ppt** — add per G.3 priorities based on actual corpus composition;
    profile a representative sample of real decks to decide whether these are worth full
    investment vs. graceful degradation (render fallback image / extract text only).

### G.5 Validation strategy

Since no compilation/execution of this C++ tree is available in this investigation, validate the
C# port against **actual LibreOffice behavior** by: (a) using `soffice --headless --convert-to
png/pdf` on a corpus of representative decks as a golden-output oracle (visual diff against your
C# renderer's output), and (b) for the inheritance/color-math subsystems specifically, write
targeted unit tests using hand-constructed minimal pptx/odp files that isolate one inheritance
level or one color transform at a time — this is far more debuggable than diffing whole-slide
raster output when something is subtly wrong.
