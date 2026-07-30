# LibreOffice Graphics / Font / Text-Shaping / Rendering / Export Stack

A reference for a pure C# reimplementation that renders documents headlessly to raster
images and PDF with reasonable visual fidelity to LibreOffice. All citations are
`path/file.cxx:line` relative to the repository root
(`/home/user/libreoffice-core`). Line numbers reflect the checked-out commit at the
time of writing and will drift over time, but the surrounding architecture is stable.

---

## Table of contents

- [A. VCL output abstraction](#a-vcl-output-abstraction)
- [B. Fonts and text layout](#b-fonts-and-text-layout)
- [C. Graphics primitives / drawinglayer](#c-graphics-primitives--drawinglayer)
- [D. Bitmap/graphic import](#d-bitmapgraphic-import)
- [E. PDF export](#e-pdf-export)
- [F. Headless conversion entry points](#f-headless-conversion-entry-points)
- [G. Assessment for C#](#g-assessment-for-c)

---

## A. VCL output abstraction

### A.1 The `OutputDevice` abstraction

`OutputDevice` (`include/vcl/outdev.hxx:169`) is the single abstract "device to draw
on" class in LibreOffice. `Window`, `Printer`, `VirtualDevice` and `PDFWriterImpl` are
all subclasses (or contain one), so document-layout code (`sw`, `sc`, `sd`, `svx`,
`drawinglayer`) is almost always written against `OutputDevice`/`VirtualDevice`
directly and never talks to a platform graphics API. This is the most important
architectural fact for a reimplementation: **there is exactly one drawing surface
abstraction used everywhere**, and it is a fairly small, well-defined set of
"immediate mode" drawing calls plus a state stack. A C# port should build the exact
same kind of seam: one `IOutputDevice`/`Canvas` interface implemented once for
raster (SkiaSharp) and once for PDF (a content-stream writer), with all document code
only calling into that interface.

Key call groups actually used by document code (as opposed to internal helpers):

| Group | Representative calls | Header |
|---|---|---|
| Text | `DrawText`, `DrawTextArray`, `DrawStretchText`, `GetTextArray`, `GetTextWidth`, `GetTextBoundRect` | `include/vcl/outdev.hxx:893-1153` |
| Polygon / path | `DrawPolyLine`, `DrawPolygon`, `DrawPolyPolygon` (both `tools::Polygon` and `basegfx::B2DPolygon` overloads) | `include/vcl/outdev.hxx:694-787` |
| Bitmaps | `DrawBitmapEx`, `DrawBitmap`, `DrawImage` | `vcl/source/outdev/bitmap.cxx`, `bitmapex.cxx` |
| Gradient / hatch | `DrawGradient(Rect\|PolyPolygon, Gradient)`, `DrawHatch` | `include/vcl/outdev.hxx:821-863` |
| Transparency | `DrawTransparent(PolyPolygon, sal_uInt16 percent)`, `DrawTransparent(..., Gradient)` (float transparence) | `include/vcl/outdev.hxx:1518-1542` |
| Clipping | `SetClipRegion()`, `SetClipRegion(vcl::Region)`, `IntersectClipRegion` | `include/vcl/outdev.hxx:534-535` |
| Mapping | `SetMapMode(MapMode)`, `LogicToPixel`, `PixelToLogic` (Point/Size/Rect/Polygon/PolyPolygon/Region overloads, all with an optional explicit `MapMode`) | `include/vcl/outdev.hxx:1585-1664` |
| Line/fill state | `SetLineColor`, `SetFillColor`, `SetFont`, `SetTextColor`, `SetTextFillColor`, `SetTextLineColor` | `vcl/source/outdev/*.cxx` |
| State stack | `Push(PushFlags)`, `Pop()` — snapshots/restores line color, fill color, font, text colors, clip region, map mode, ref point, layout mode, text language | `vcl/source/outdev/stack.cxx` |

`Push`/`Pop` combined with the rest of this call surface is precisely what
`GDIMetaFile` records (section A.3) — the metafile is just a recorded trace of these
calls plus the state-mutating ones (`SetLineColor` etc.), which is why the metafile
action set and the `OutputDevice` method set line up almost 1:1.

### A.2 `VirtualDevice` and the headless (cairo) backend

`VirtualDevice` (`include/vcl/virdev.hxx`, impl `vcl/source/gdi/virdev.cxx`) is an
`OutputDevice` that renders into an off-screen pixel buffer instead of a window; it is
what `--headless` mode uses for everything (screenshotting, rendering pages for
export, computing text metrics without ever creating a GUI window).

Each platform provides a `SalVirtualDevice`/`SalGraphics` implementation. The one that
matters for headless/server operation is the **SVP (Salv-Vcl-Plugin) "headless"
backend**:

- `vcl/inc/headless/svpgdi.hxx:40` — `SvpSalGraphics : public SalGraphicsAutoDelegateToImpl`
  wraps a `CairoCommon` member (`m_aCairoCommon`) plus a `CairoTextRender`
  (`vcl/unx/generic/print/... /cairotextrender` interface) for glyph rendering, and a
  `SvpGraphicsBackend`.
- `vcl/headless/CairoCommon.cxx` (2145 lines) is the actual rasterizer: it builds
  `cairo_surface_t`/`cairo_t` contexts, converts `basegfx::B2DPolygon` to Cairo paths
  (`AddPolygonToPath`, `vcl/headless/CairoCommon.cxx:149`), does damage tracking
  (`getFillDamage`, `getClipBox`, etc., lines 74-140), and implements fill/stroke,
  clip, and blit operations against libcairo.
- `vcl/headless/svpvd.hxx:30` — `SvpSalVirtualDevice : public SalVirtualDevice`
  creates a cairo image surface sized to the requested pixel dimensions; there is no
  windowing system dependency at all.
- `vcl/headless/svpinst.hxx:90` / `headlessinst.cxx:17` — `SvpSalInstance` /
  `HeadlessSalInstance` are the "no display server" `SalInstance` used when
  `--headless` is passed (also used by `unittest`/`gtktiledviewer`-less CI runs).

**Conclusion for the port:** LibreOffice's own headless rendering path is already a
CPU rasterizer built on top of libcairo, with no GPU or window-system dependency.
This is architecturally identical to what SkiaSharp (raster) gives a C# port for free
— confirming SkiaSharp as the natural analogue of "VCL + cairo headless backend."

### A.3 `GDIMetaFile` / `MetaAction` — the device-independent display list

`GDIMetaFile` (`include/vcl/gdimtf.hxx`, impl `vcl/source/gdi/gdimtf.cxx`, 2291 lines)
is LibreOffice's recordable/replayable display list: `OutputDevice::EnableOutput(false)`
+ `OutputDevice::SetConnectMetaFile(pMtf)` causes every draw call on that device to be
converted 1:1 into a `MetaAction` subclass instance appended to the metafile instead of
(or in addition to) drawing pixels. The metafile can then be:

- replayed onto any other `OutputDevice` (screen, printer, PDF writer, another VirtualDevice) via `GDIMetaFile::Play` (`vcl/source/gdi/gdimtf.cxx`),
- serialized to/from the native `.svm` binary format (`vcl/source/filter/svm/SvmReader.cxx`, `SvmWriter.cxx`),
- scaled, clipped, and walked action-by-action for analysis (used heavily by the PDF/EMF/print code paths).

This is a genuinely excellent model for a C# rendering IR: a flat, ordered list of
small, serializable, replayable "recorded graphics calls" that is 100% decoupled from
both the document model that produced it and the backend that eventually consumes it.
A C# port should strongly consider an equivalent `MetaAction`-like IR: it gives you
(a) a natural PDF-export target (walk the list, emit PDF operators — exactly what
`vcl/source/pdf/pdfwriter_impl2.cxx` does, see §E), (b) a natural raster target (walk
the list, call SkiaSharp), and (c) trivial "record once, render N times" semantics
for multi-page documents, print preview, thumbnailing, etc.

#### A.3.1 Full `MetaActionType` enumeration

Defined in `include/vcl/metaactiontypes.hxx:23-81`; concrete classes with their data
members in `include/vcl/metaact.hxx`.

| Enum value | Class | Key data | Purpose |
|---|---|---|---|
| `PIXEL` (100) | `MetaPixelAction` (`metaact.hxx:81`) | Point, Color | single pixel |
| `POINT` (101) | `MetaPointAction` (`:107`) | Point | degenerate "point" (line state) |
| `LINE` (102) | `MetaLineAction` (`:131`) | 2 Points, `LineInfo` (width/style/dash) | straight line |
| `RECT` (103) | `MetaRectAction` (`:162`) | Rectangle | filled/stroked rect |
| `ROUNDRECT` (104) | `MetaRoundRectAction` (`:187`) | Rectangle, horiz/vert round radius | rounded rect |
| `ELLIPSE` (105) | `MetaEllipseAction` (`:217`) | Rectangle | ellipse inscribed in rect |
| `ARC` (106) | `MetaArcAction` (`:242`) | Rectangle, start/end Point | elliptical arc |
| `PIE` (107) | `MetaPieAction` (`:272`) | Rectangle, start/end Point | pie slice |
| `CHORD` (108) | `MetaChordAction` (`:302`) | Rectangle, start/end Point | chord |
| `POLYLINE` (109) | `MetaPolyLineAction` (`:332`) | `tools::Polygon`, `LineInfo` | open/closed polyline |
| `POLYGON` (110) | `MetaPolygonAction` (`:360`) | `tools::Polygon` | filled polygon |
| `POLYPOLYGON` (111) | `MetaPolyPolygonAction` (`:385`) | `tools::PolyPolygon` | multi-contour fill (holes) |
| `TEXT` (112) | `MetaTextAction` (`:410`) | Point, string, index/len | simple text at a point |
| `TEXTARRAY` (113) | `MetaTextArrayAction` (`:442`) | Point, string, `KernArray` (DX array), kashida array | text with explicit per-glyph advances |
| `STRETCHTEXT` (114) | `MetaStretchTextAction` (`:490`) | Point, string, target width | text stretched/compressed to fit a width |
| `TEXTRECT` (115) | `MetaTextRectAction` (`:525`) | Rectangle, string, `DrawTextFlags` | text laid out/wrapped/ellipsized in a box |
| `TEXTLINE` (146) | `MetaTextLineAction` (`:555`) | Point, width, strikeout/underline style | underline/strikeout stroke under text |
| `BMP` (116) | `MetaBmpAction` (`:590`) | Point, `Bitmap` | opaque bitmap blit |
| `BMPSCALE` (117) | `MetaBmpScaleAction` (`:617`) | Point, Size, `Bitmap` | scaled blit |
| `BMPSCALEPART` (118) | `MetaBmpScalePartAction` (`:647`) | dest Point/Size, src Point/Size, `Bitmap` | scaled sub-rect blit |
| `BMPEX` (119) | `MetaBmpExAction` (`:682`) | Point, `BitmapEx`-equivalent (bitmap+alpha) | blit with alpha |
| `BMPEXSCALE` (120) | `MetaBmpExScaleAction` (`:710`) | as above + Size | scaled blit with alpha |
| `BMPEXSCALEPART` (121) | `MetaBmpExScalePartAction` (`:741`) | as above + src rect | scaled sub-rect blit with alpha |
| `MASK` (122)/`MASKSCALE` (123)/`MASKSCALEPART` (124) | `MetaMaskAction` family (`:777-877`) | Bitmap + Color | draw a 1-bit mask tinted with a color (legacy) |
| `GRADIENT` (125) | `MetaGradientAction` (`:879`) | Rectangle, `Gradient` | simple 2/3-color gradient fill |
| `GRADIENTEX` (148) | `MetaGradientExAction` (`:906`) | `tools::PolyPolygon`, `Gradient` | gradient clipped to arbitrary polygon, plus pre-rendered fallback decomposition |
| `HATCH` (126) | `MetaHatchAction` (`:933`) | `tools::PolyPolygon`, `Hatch` (style/color/distance/angle) | hatch fill |
| `WALLPAPER` (127) | `MetaWallpaperAction` (`:960`) | Rectangle, `Wallpaper` (color/bitmap/gradient background) | background fill |
| `CLIPREGION` (128) | `MetaClipRegionAction` (`:988`) | `vcl::Region`, bool clipping-enabled | replace clip |
| `ISECTRECTCLIPREGION` (129) | `MetaISectRectClipRegionAction` (`:1015`) | Rectangle | intersect clip with rect |
| `ISECTREGIONCLIPREGION` (130) | `MetaISectRegionClipRegionAction` (`:1040`) | `vcl::Region` | intersect clip with region |
| `MOVECLIPREGION` (131) | `MetaMoveClipRegionAction` (`:1065`) | dx, dy | translate current clip |
| `LINECOLOR` (132) | `MetaLineColorAction` (`:1091`) | Color, bool set/unset | stroke color state |
| `FILLCOLOR` (133) | `MetaFillColorAction` (`:1115`) | Color, bool set/unset | fill color state |
| `TEXTCOLOR` (134) | `MetaTextColorAction` (`:1140`) | Color | glyph fill color |
| `TEXTFILLCOLOR` (135) | `MetaTextFillColorAction` (`:1162`) | Color, bool | text background color |
| `TEXTLINECOLOR` (145) | `MetaTextLineColorAction` (`:1187`) | Color, bool | underline color |
| `OVERLINECOLOR` (151) | `MetaOverlineColorAction` (`:1211`) | Color, bool | overline color |
| `TEXTALIGN` (136) | `MetaTextAlignAction` (`:1235`) | `TextAlign` (baseline/top/bottom) | text vertical anchor |
| `MAPMODE` (137) | `MetaMapModeAction` (`:1257`) | `MapMode` | switch logical coordinate system |
| `FONT` (138) | `MetaFontAction` (`:1285`) | `vcl::Font` (family/size/weight/italic/etc.) | current font state |
| `PUSH` (139) | `MetaPushAction` (`:1315`) | `PushFlags` | save state |
| `POP` (140) | `MetaPopAction` (`:1337`) | — | restore state |
| `RASTEROP` (141) | `MetaRasterOpAction` (`:1353`) | `RasterOp` (XOR/invert/overpaint — legacy GDI) | raster operation |
| `Transparent` (142) | `MetaTransparentAction` (`:1375`) | `tools::PolyPolygon`, uniform transparency % | flat-alpha polygon fill |
| `FLOATTRANSPARENT` (147) | `MetaFloatTransparentAction` (`:1404`) | nested `GDIMetaFile` + gradient-shaped transparency | content masked by a *gradient* alpha (soft edges/shadows) |
| `EPS` (143) | `MetaEPSAction` (`:1445`) | `GfxLink` (raw EPS bytes) + substitute preview `GDIMetaFile` | embedded EPS with a fallback raster/vector preview |
| `REFPOINT` (144) | `MetaRefPointAction` (`:1477`) | Point | sets a reference point some other actions are relative to |
| `COMMENT` (512) | `MetaCommentAction` (`:1501`) | string comment + opaque binary payload | out-of-band metadata (e.g. embedded EMF+ records, group markers used by PDF export for `BeginGroup`/`EndGroup`, native-JPEG passthrough hints) |
| `LAYOUTMODE` (149) | `MetaLayoutModeAction` (`:1532`) | `vcl::text::ComplexTextLayoutFlags` | RTL / bidi-strong / text-layout flags |
| `TEXTLANGUAGE` (150) | `MetaTextLanguageAction` (`:1554`) | `LanguageType` | language tag used for shaping/line-breaking |

Notes for a C# IR design:

- `MetaActionType::COMMENT` (`MetaCommentAction`) is used as an **extensibility escape
  hatch** — group boundaries for OLE objects/transparency groups (`"XGROUP_SEQ_BEGIN"`,
  etc.), EMF+ raw record passthrough, and PDF-specific annotations are all smuggled
  through comment actions with opaque byte payloads read back out by the consuming
  renderer. A C# port's IR should keep an analogous "opaque tagged blob" action for
  the same reason (grouping/undo boundaries without polluting the primitive set).
- Almost every action has `Move(dx,dy)` and `Scale(fx,fy)` virtual methods
  (`vcl/source/gdi/metaact.cxx`) used to reposition/rescale an entire recorded
  metafile in place (e.g. fitting an embedded OLE object's metafile into its frame) —
  worth replicating as an IR-level capability rather than re-recording.
- `GDIMetaFile` additionally records a **preferred size + preferred `MapMode`**
  (`GetPrefSize()`/`GetPrefMapMode()`), which callers use to scale the whole action
  list to a target device size — this is exactly how EMF/WMF/SVM/embedded-object
  preview images are fit into a placeholder frame.

### A.4 Units and mapping: twips, 1/100 mm, points, pixels

`MapUnit` (`include/tools/mapunit.hxx:26-36`):

```
Map100thMM, Map10thMM, MapMM, MapCM,
Map1000thInch, Map100thInch, Map10thInch, MapInch,
MapPoint, MapTwip,
MapPixel,
MapSysFont, MapAppFont,
MapRelative
```

Every `MapUnit` maps onto a shared, more general `o3tl::Length` enum
(`include/o3tl/unit_conversion.hxx:25-49`) that also knows about EMU (1/360000 cm —
needed for OOXML/DrawingML), twips (1/20 pt — needed for RTF/DOC), pica, feet, miles,
and PowerPoint's "master unit" (1/576 in). All conversions between units are done
through a single generic rational-arithmetic routine, `o3tl::convert` /
`o3tl::convertSaturate` / `o3tl::getConversionMulDiv`, parameterized by a pair of
`Length` enum values — i.e., LibreOffice does **not** use ad-hoc floating point
multipliers per unit pair; it computes an exact integer multiply/divide fraction
per pair and applies it with rounding, which avoids compounding rounding error.

`include/tools/UnitConversion.hxx` layers convenience helpers on top:

```cpp
convertTwipToMm100(n)      // o3tl::convert(n, Length::twip, Length::mm100)
sanitiseMm100ToTwip(n)     // o3tl::convertSaturate(n, Length::mm100, Length::twip)
convertPointToMm100(n)
convertMm100ToPoint(n)
convertMasterUnitToMm100(n)  // PPT 1/576" master unit <-> mm/100
convertMm100ToMasterUnit(n)
```

Internally, **1/100 mm (`Map100thMM`) is the dominant "logical unit"** used by
`sw`/`sc`/`sd` document models (Writer/Calc/Impress all store geometry in 1/100 mm,
`MapUnit::Map100thMM`), while RTF/DOC import/export and some legacy Writer layout
code use twips (`MapUnit::MapTwip`, 1/20 pt = 1/1440 in). OOXML (DOCX/XLSX/PPTX) uses
EMU (1/360000 cm = 1/914400 in) at the XML level, which is converted to 1/100 mm on
import via `o3tl::convert(..., Length::emu, Length::mm100)`. PDF/PostScript use points
(1/72 in). A C# port needs the same "one canonical internal unit + one generic
rational converter" design — do not hand-roll separate float constants per unit pair
per module, that is a proven source of cross-format rounding drift.

`MapMode` (`include/vcl/mapmod.hxx:31`) additionally carries an **origin** (`Point`)
and **scale factors** (`fScaleX`,`fScaleY`, as `Fraction`s in the historical
implementation) on top of the base unit, so `LogicToPixel`/`PixelToLogic`
(`include/vcl/outdev.hxx:1632-1664`) is a full affine (translate+scale, no rotation)
transform from "logical units" to "device pixels," parameterized per-`OutputDevice`
by its `MapMode` and its physical DPI (`GetDPIX()`/`GetDPIY()`, needed because a
`VirtualDevice`'s "pixel" is not a fixed physical size — it's set to emulate the
target output DPI, e.g. 96 for screen, 300+ for print/PDF raster fallback).

---

## B. Fonts and text layout

### B.1 Font selection & matching pipeline

- `vcl::font::FontSelectPattern` (`vcl/inc/font/FontSelectPattern.hxx`, impl
  `vcl/source/font/FontSelectPattern.cxx`) is the "wanted font" descriptor: family
  name (with `;`-separated fallback list), height, width, weight, italic, pitch,
  language, orientation, and OpenType feature/variation strings baked into the name
  (`family:feat1=val;feat2=val`, parsed in `GenericSalLayout::ParseFeatures`).
- `vcl::font::PhysicalFontCollection` (`vcl/inc/font/PhysicalFontCollection.hxx`,
  impl `vcl/source/font/PhysicalFontCollection.cxx`, ~1000+ lines) is the process-wide
  registry of fonts actually installed/available. It indexes
  `vcl::font::PhysicalFontFamily` objects (one per family name;
  `vcl/inc/font/PhysicalFontFamily.hxx`) each of which owns 1+
  `vcl::font::PhysicalFontFace` instances (`vcl/inc/font/PhysicalFontFace.hxx`) — one
  per weight/style/variant physically on disk.
- Matching order in `PhysicalFontCollection::FindFontFamily(FontSelectPattern&)`
  (`vcl/source/font/PhysicalFontCollection.cxx:1032`): exact search-name match
  (`ImplFindFontFamilyBySearchName`, `:339`) → per-token fallback list
  (`FindFontFamilyByTokenNames`, `:403`) → configured substitution table lookup
  (`ImplFindFontFamilyBySubstFontAttr`, `:423`, driven by `utl::FontNameAttr`) →
  attribute-based nearest match (`FindFontFamilyByAttributes`, `:475`, matching
  pitch/weight/italic/family-class when no name matches at all) →
  `ImplFontSubstitute`/default-font fallback
  (`ImplFindFontFamilyOfDefaultFont`, `:906`).
- **Direct/explicit substitution**: `vcl::font::DirectFontSubstitution`
  (`vcl/inc/font/DirectFontSubstitution.hxx`, impl
  `vcl/source/font/DirectFontSubstitution.cxx`) implements user/administrator
  overrides (Tools ▸ Options font-replacement table equivalents), applied before
  fontconfig/OS matching.
- **Fontconfig integration (Linux/Unix)**:
  `vcl/unx/generic/font/fontconfig.cxx` (1455 lines). `FontConfigManager::Substitute`
  (`:1059`) drives an `FcPattern`/`FcConfigSubstitute`/`FcFontMatch` fontconfig query
  whenever an exact/table match is not found — i.e. on Linux, "which font actually
  renders" is ultimately delegated to the system's fontconfig substitution rules for
  anything not covered by LO's own tables. **There is no equivalent on Windows/macOS**
  — those platforms use the native font-enumeration API plus LO's own tables only.
  A C# port targeting Linux-parity output should either shell out to/bind
  `libfontconfig`, or ship a curated static substitution table (see next point) and
  accept it won't reproduce arbitrary end-user fontconfig customization.
- **Built-in substitution table**: shipped as configuration data, not code, in
  `officecfg/registry/data/org/openoffice/VCL.xcu`. It has two node groups:
  - `<node oor:name="DefaultFonts">` (`:21` onward) — per-locale default UI/document
    font (e.g. `en`, `ja`, `zh-cn`, `ar`, `he`, `hi`, ... ~35 locales).
  - `<node oor:name="FontSubstitutions">` (`:857` onward) — per-locale map from a
    *requested* font name (e.g. `albany`, `algerian`, lower-cased/normalized) to an
    ordered `SubstFonts` fallback list (e.g. Albany → `albanyamt;albany;
    liberationsans;arial;nimbussansl;helvetica;...`), a `SubstFontsMS` hint (what the
    "real" Microsoft font name is, e.g. `Arial`), and coarse `FontWeight`/`FontWidth`/
    `FontType` classification tags. This table is the single most valuable, portable
    artifact for a C# port: it directly encodes "if a DOCX asks for font X and it's
    not installed, what's the closest free-substitute chain LibreOffice will actually
    render," independent of any platform font API. **Recommendation: parse this XCU
    directly (or a generated copy of it) rather than reimplementing matching logic.**

### B.2 Shaping: `SalLayout`, HarfBuzz, `ImplLayoutArgs`

- `vcl::text::ImplLayoutArgs` (`vcl/inc/ImplLayoutArgs.hxx:31`) is the shaping input:
  the substring range (`mnMinCharPos`/`mnEndCharPos`), `SalLayoutFlags`
  (bidi-strong/RTL/vertical/kashida/etc.), language tag, requested layout width (for
  justification), text orientation, and — crucially — `ImplLayoutRuns maRuns` /
  `maFallbackRuns`: pre-computed bidi/script/font-fallback run boundaries that the
  layout engine iterates with `GetNextRun`/`GetNextPos`.
- `GenericSalLayout` (`vcl/inc/sallayout.hxx:95`) is the concrete single-font-run
  shaper; `MultiSalLayout` (`:47`) composes up to `MAX_FALLBACK` (16,
  `vcl/inc/sallayout.hxx:43`) `GenericSalLayout` instances to realize font
  *fallback* (e.g. Latin run in the requested font, CJK run falls back to a CJK
  font, emoji run falls back to a color-emoji font) as one logical glyph run.
- **HarfBuzz is the shaping engine**, used directly in
  `vcl/source/gdi/CommonSalLayout.cxx` (1149 lines) — see the ports of
  `hb_buffer_create`/`hb_buffer_set_direction`/`hb_buffer_set_script`/
  `hb_buffer_set_language`/`hb_buffer_add_utf16`/`hb_shape_full`
  (`CommonSalLayout.cxx:126-213`, and again `:443-624` for the main text path).
  `hb_shape_full` is called with an explicit `hb_feature_t` array built from parsed
  OpenType feature strings (`GenericSalLayout::ParseFeatures`,
  `vcl/source/gdi/sallayout.cxx`) — i.e. `font:feat=1` name-embedded syntax maps
  straight onto HarfBuzz features (`liga`, `kern`, `smcp`, `frac`, stylistic sets,
  etc; the full catalog is in `vcl/inc/font/OpenTypeFeatureDefinitionList.hxx` /
  `OpenTypeFeatureStrings.hrc`).
- Kashida justification (Arabic elongation), ligatures, and complex-script shaping
  (Indic/Thai/Khmer reordering) are therefore **entirely HarfBuzz's responsibility**
  — LO does not reimplement shaping logic, it only prepares run boundaries
  (script/bidi/font-fallback) and post-processes the resulting glyph stream for
  justification (`GenericSalLayout::Justify`, `ApplyJustificationData`) and Asian
  kerning (`ApplyAsianKerning`).
- **Script runs & bidi**: `i18nutil/source/utility/scriptchangescanner.cxx` drives
  ICU directly: `ubidi_setPara` (`:160`) and `ubidi_getLogicalRun` (`:99`, `:133`) for
  bidi-level runs, and `vcl/inc/scrptrun.h:61`'s `ScriptRun` (`class ... final :
  public icu::UObject`) for Unicode-script run boundaries (Latin/Greek/Han/Arabic/
  etc.) so that HarfBuzz gets correctly-scoped `hb_script_t`/`hb_direction_t` per
  run. **ICU is a hard, direct dependency of the shaping pipeline, not just an
  optional add-on** — a C# port should plan on ICU4N (or an ICU4C P/Invoke binding
  in preference to ICU4N, for byte-for-byte `ubidi`/script-property parity) as the
  bidi + script-run engine feeding HarfBuzzSharp.
- Vertical text (CJK top-to-bottom): handled via HarfBuzz vertical-glyph-alternate
  lookups (`GenericSalLayout::HasVerticalAlternate`,
  `vcl/inc/sallayout.hxx:152`) and per-glyph vertical advances taken from `hmtx`/
  `vmtx`/`VORG`/`vhea` tables through HarfBuzz's `hb_font_get_glyph_v_advance` etc.

### B.3 `GetTextArray`/`KernArray`/`DXArray` semantics — read this before implementing text layout

`KernArray` is simply `std::vector<double>` (`include/vcl/kernarray.hxx:16`). The
**critical semantic point** (easy to get backwards) is that the public
`OutputDevice::GetTextArray`/`GetPartialTextArray` API
(`vcl/source/outdev/text.cxx:760-853`) returns **cumulative absolute end-positions per
character**, not per-character deltas. Internally `SalLayout::FillDXArray` returns
per-*glyph*/per-*character* widths (`GenericSalLayout::GetCharWidths`,
`vcl/source/gdi/CommonSalLayout.cxx:867`), and `OutputDevice::GetPartialTextArray`
explicitly converts them with a running sum right before returning
(`vcl/source/outdev/text.cxx:844-849`):

```cpp
// convert virtual char widths to virtual absolute positions
if( pDXPixelArray )
{
    for (int i = 1; i < nPartLen; ++i)
        (*pDXPixelArray)[i] += (*pDXPixelArray)[i - 1];
}
```

So: `pDXAry[i]` = the x-offset (in logical units, from the text's start point) of the
**end** of character `i`. `MetaTextArrayAction` (§A.3.1) stores exactly this
cumulative array, and it is precisely what a WMF/EMF `ExtTextOutW` DX array or an
OOXML `<w:position>`-adjacent run-width table represents too — reproducing this
exact cumulative convention (not "advance per glyph") is essential for pixel-accurate
reflow/justification parity, especially where documents embed explicit per-character
kerning arrays instead of relying on shaping (common in old DOC/RTF and in any
document round-tripped through Word).

### B.4 Font metrics: ascent/descent/leading — how line height is computed

This is one of the highest-value findings for fidelity, because "line height"
differences are one of the most visible ways two renderers diverge on identical
input. `FontMetricData::ImplCalcLineSpacing`
(`vcl/source/font/fontmetric.cxx:434-562`) is the authoritative algorithm:

1. **Variable fonts**: if the font has an `avar`/`fvar` table (checked via
   `hb_ot_var_has_data`), trust HarfBuzz's already-instanced
   `HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER` / `_DESCENDER` / `_LINE_GAP` directly
   (`:444-459`) — these already reflect the current variation-axis settings.
2. **Static fonts** — a specific, documented precedence chain (`:460-536`), because
   real-world fonts disagree about which of `hhea`/`OS/2` to trust:
   - Read `hhea` ascent/descent/lineGap first (private HarfBuzz OT-metrics tags
     `Hasc`/`Hdsc`/`Hlgp`, since `hhea` is a mandatory table and should always be
     present) — but only accept it if ascent ≥ 0 and descent ≤ 0 (defends against
     malformed fonts, tdf#107605).
   - If `OS/2` is present (mandatory on Windows), it takes priority: read
     `usWinAscent`/`usWinDescent` (`HB_OT_METRICS_TAG_HORIZONTAL_CLIPPING_ASCENT/
     DESCENT`) as the default (because historically Windows' `TEXTMETRIC`/GDI line
     height comes from `usWinAscent/Descent`, and many legacy documents were
     authored assuming that), **unless** the OS/2 `fsSelection` bit 7
     (`USE_TYPO_METRICS`, read via a raw big-endian table parse at
     `vcl::OS2_fsSelection_offset`, `:519-527`) is set, in which case the `sTypoAscender`/
     `sTypoDescender`/`sTypoLineGap` ("typo" metrics) are used instead — this is the
     official OpenType-recommended way for a font to say "please use my real
     typographic metrics, not the (historically bloated) Win metrics."
   - A per-font-name configurable override list exists for both directions:
     `officecfg` keys `Office::Common::Misc::FontsUseWinMetrics` and
     `FontsDontUseUnderlineMetrics` (read via `ShouldUseWinMetrics`,
     `fontmetric.cxx:401`, and `ShouldNotUseUnderlineMetrics`, `:185`) — i.e. LO
     ships a hardcoded exception list for specific fonts known to lie about their
     own metrics.
3. `mnIntLeading` ("internal leading") is *derived*, not read from the font: `mnAscent
   + mnDescent - mnHeight` (`:543`), where `mnHeight` is the requested pixel/logical
   font size. This matches the classic Windows GDI `TEXTMETRIC` definition of
   internal leading and is what Writer/Calc/Impress line-spacing algorithms actually
   consume for single-spacing.
4. `mdEmSize`/`mdHorCJKAdvanceSize`/`mdVertCJKAdvanceSize` (`:546-561`): the CJK
   advance width is measured as the actual advance of U+6C34 (水, "water" — the
   standard "ideographic character" reference glyph), falling back to the font's em
   size if that glyph is absent. This "ic"/"ideographic character" measurement is
   used for CJK-relative indentation (`tdf#36709`) and is *not* simply `unitsPerEm`.
5. Underline/strikeout thickness and offset: preferentially taken from the OpenType
   `post`/`OS/2` `underline*`/`strikeout*` fields via
   `HB_OT_METRICS_TAG_UNDERLINE_SIZE/OFFSET` and `..._STRIKEOUT_...`
   (`ImplInitTextLineSizeHarfBuzz`, `fontmetric.cxx:200-259`); if unavailable, a
   documented fallback heuristic derives them from descent (`ImplInitTextLineSize`,
   `:261-351`, e.g. "single underline thickness ≈ 25% of descent, bold ≈ 50%,
   double-line spacing has its own DPI-scaled minimum gap").
6. A separate `ImplInitBaselines` (`:564-584`) reads the OpenType `BASE` table
   hanging-baseline value via `hb_ot_layout_get_baseline`, used for
   vertical/hanging-baseline text alignment (Indic scripts).

**Recommendation:** replicate this exact precedence (variable-font passthrough → hhea
→ OS/2 win-vs-typo based on `fsSelection` bit 7 → derived internal leading) in the C#
port's metrics layer; HarfBuzzSharp exposes the same `hb_ot_metrics_get_position` API
so the algorithm ports almost verbatim. Do **not** simply use whatever a naive
"get font metrics" call from a higher-level text layout library returns (e.g. Skia's
`SKFontMetrics` or `System.Drawing`'s line spacing) without checking which OS/2 flag
convention it defaults to — mismatches here are a very common, very visible source of
line-height drift between renderers.

### B.5 Font embedding (document-level and PDF)

- ODF/OOXML documents can embed subset or full font files directly (`fontwork`/
  `EmbeddedFontsManager` in `vcl/source/gdi/embeddedfontsmanager.cxx`, plus
  `EOTConverter.cxx` for legacy EOT-wrapped fonts sometimes seen in old `.docx`).
  `vcl/source/gdi/embeddedfontsafdko.cxx` uses Adobe's AFDKO tooling
  (`vcl/source/gdi/afdko.hxx`) for certain embedding/subsetting transforms.
- PDF export subsetting/embedding is covered fully in §E.2 (modern LO subsets via
  HarfBuzz's `hb-subset`, not a hand-rolled TrueType/CFF subsetter).

---

## C. Graphics primitives / drawinglayer

### C.1 `basegfx` — the geometry kernel

- `basegfx::B2DPolygon` (`include/basegfx/polygon/b2dpolygon.hxx`, impl
  `basegfx/source/polygon/b2dpolygon.cxx`, 1554 lines): a single contour as an ordered
  point list where **any point may carry two Bezier control points**
  (`appendBezierSegment(nextControlPoint, prevControlPointOfNext, endPoint)`,
  `b2dpolygon.hxx:100`; `areControlPointsUsed()`, `:110`) — i.e. LO's polygon type is
  natively a mixed line/cubic-Bezier path, not a separate "path" type layered on top
  of a pure polyline type. This matters for a C# port: don't model your path type as
  "polyline OR bezier-path," model it the LO way as "polyline where any vertex can
  optionally have Bezier control handles," because a huge amount of decomposition
  code (rounded rects, ellipses-as-4-bezier-arcs, stroke-to-fill conversion) depends
  on being able to freely mix straight and curved segments in one contour.
- `basegfx::B2DPolyPolygon`: an ordered set of `B2DPolygon`s that together define one
  fill region including holes (even-odd/nonzero winding handled by the fill
  processor, not baked into the polygon itself).
- `basegfx::B2DHomMatrix` (`include/basegfx/matrix/b2dhommatrix.hxx:34`, impl
  `basegfx/source/matrix/b2dhommatrix.cxx`): the standard 2D homogeneous
  (3x3-conceptually, translate+rotate+scale+shear) transform matrix used
  pervasively — every drawinglayer primitive that has geometry carries a
  `B2DHomMatrix` rather than separate position/scale/rotation fields.
- `basegfx::B2DRange`/`B2DRectangle`: axis-aligned bounding boxes, used for both
  layout bounds and clip/damage-region math.

### C.2 `drawinglayer` — the modern immediate-mode primitive tree

`drawinglayer` sits **above** `OutputDevice`/`GDIMetaFile` conceptually (it targets
`OutputDevice` as one of several possible processors) and is what Draw/Impress/Writer
frame/shape rendering, and the SVG/EMF+ importers, actually build as their in-memory
scene graph before rasterizing. There are 73 primitive classes under
`include/drawinglayer/primitive2d/` (matching 73 `.cxx` files in
`drawinglayer/source/primitive2d/`).

`BasePrimitive2D` (`include/drawinglayer/primitive2d/baseprimitive2d.hxx:121`) is the
common base. Its doctring (`:48-120`) spells out the exact decomposition contract a
C# port should copy:

> A method to get a simplified representation is provided by `get2DDecomposition`.
> ... An example is e.g. a fat line, whose decomposition may contain the geometric
> representation of that line using filled polygon primitives. When the renderer
> knows how to handle fat lines, he may process this primitive directly; if not he
> can use the decomposition. ... Current Basic 2D Primitives are: `BitmapPrimitive2D`,
> `PointArrayPrimitive2D`, `PolygonHairlinePrimitive2D`, `PolyPolygonColorPrimitive2D`.

I.e. this is a **recursive decomposition tree, not a flat display list** (contrast
with `GDIMetaFile` in §A.3, which *is* flat): every primitive either (a) is one of a
small set of "basic" primitives a renderer must implement natively, or (b) implements
`get2DDecomposition(Primitive2DDecompositionVisitor&, ViewInformation2D)` to lazily
expand itself into a `Primitive2DContainer` of simpler primitives (visitor pattern,
`include/drawinglayer/primitive2d/Primitive2DVisitor.hxx`), all the way down to the
basic set. `getB2DRange` (bounding box) has a similar default-via-decomposition
behavior (`:141-143`). This means a minimal renderer only needs to implement ~4
primitive kinds plus decomposition-driving, and gets fat lines, gradients, shadows,
3D-embedded-object projections, etc. "for free" by recursing.

#### C.2.1 Primitive2D catalog (grouped)

| Category | Classes (file stem) |
|---|---|
| Basic fills | `polygonprimitive2d` (hairline), `PolyPolygonColorPrimitive2D`, `PolyPolygonHairlinePrimitive2D`, `PolyPolygonRGBAPrimitive2D` |
| Stroke | `PolyPolygonStrokePrimitive2D` (fat/dashed/capped/joined line, decomposes to fill polygons), `borderlineprimitive2d` (table/frame double/triple border styles) |
| Gradient fill | `fillgradientprimitive2d`, `PolyPolygonGradientPrimitive2D`, `PolyPolygonAlphaGradientPrimitive2D`, `svggradientprimitive2d` (linear/radial SVG-spec gradients) |
| Hatch fill | `fillhatchprimitive2d`, `PolyPolygonHatchPrimitive2D` |
| Bitmap/graphic fill | `fillgraphicprimitive2d` (tiled/stretched bitmap or metafile fill), `PolyPolygonGraphicPrimitive2D`, `patternfillprimitive2d` |
| Bitmaps | `bitmapprimitive2d`, `BitmapAlphaPrimitive2D`, `discretebitmapprimitive2d` (device-pixel-snapped) |
| Text | `textprimitive2d` (`TextSimplePortionPrimitive2D`), `textdecoratedprimitive2d` (`TextDecoratedPortionPrimitive2D`: + underline/strikeout/emphasis marks), `textlineprimitive2d`, `textstrikeoutprimitive2d`, `texteffectprimitive2d` (relief/shadow/outline text effects), `textbreakuphelper`, `textlayoutdevice`, `textenumsprimitive2d`, `texthierarchyprimitive2d` (semantic tagging: paragraph/line/bullet/field/footnote markers for a11y+PDF tags) |
| Transparency/compositing | `transparenceprimitive2d` (per-pixel alpha from a child primitive tree as mask), `unifiedtransparenceprimitive2d` (single flat alpha), `maskprimitive2d`, `invertprimitive2d`, `modifiedcolorprimitive2d` (color remap, e.g. grayscale/watermark preview) |
| Shadow/effects | `shadowprimitive2d`, `discreteshadowprimitive2d`, `softedgeprimitive2d`, `glowprimitive2d`, `GlowSoftEgdeShadowTools` |
| Grouping/structure | `groupprimitive2d`, `transformprimitive2d` (push a `B2DHomMatrix`), `hiddengeometryprimitive2d` (contributes to bounds/hit-test but not paint), `structuretagprimitive2d`, `objectinfoprimitive2d`, `pagehierarchyprimitive2d`, `pagepreviewprimitive2d` |
| Embedding | `metafileprimitive2d` (wraps a legacy `GDIMetaFile`, e.g. an OLE object preview or WMF/EMF import result — a bridge back down to §A.3!), `embedded3dprimitive2d` (3D scene projected to 2D), `epsprimitive2d`, `graphicprimitive2d`/`graphicprimitivehelper2d` (a `Graphic` — bitmap or vector — placed with cropping) |
| Misc | `backgroundcolorprimitive2d`, `wallpaperprimitive2d`, `gridprimitive2d`, `helplineprimitive2d`, `markerarrayprimitive2d`, `pointarrayprimitive2d`, `controlprimitive2d` (form controls), `sceneprimitive2d`, `mediaprimitive2d` (video/audio placeholder), `wrongspellprimitive2d`, `animatedprimitive2d`, `cropprimitive2d`, `exclusiveeditviewprimitive2d` |

#### C.2.2 Processors

`drawinglayer/source/processor2d/` implements consumers that walk a primitive tree:

| File | Role |
|---|---|
| `vclprocessor2d.cxx`/`.hxx` | shared base for VCL-backed processors (maps primitives onto `OutputDevice` calls) |
| `vclpixelprocessor2d.cxx` (1130 lines) | switches on `getPrimitive2DID()` (see the ~30 `PRIMITIVE2D_ID_*` cases at `vclpixelprocessor2d.cxx:195-364`) and either handles a primitive directly against an `OutputDevice`/cairo surface, or falls through to `get2DDecomposition` for anything it doesn't special-case — this is the main on-screen/raster renderer |
| `vclmetafileprocessor2d.cxx` (2861 lines) | renders a primitive tree back down into a `GDIMetaFile` (bridges the modern primitive tree back to the legacy flat `MetaAction` IR — used so that shape/Draw rendering can still be captured as a metafile for printing/embedding/EMF export) |
| `cairopixelprocessor2d.cxx` | a more direct Cairo-backed processor (bypassing some `OutputDevice` indirection) |
| `d2dpixelprocessor2d.cxx` | Direct2D-backed processor (Windows) |
| `hittestprocessor2d.cxx` | hit-testing (not rendering) — walks geometry to answer "what's under this point" |
| `contourextractor2d.cxx`, `linegeometryextractor2d.cxx`, `textaspolygonextractor2d.cxx`, `textextractor2d.cxx` | non-rendering extraction processors (get outlines/text runs back out of a primitive tree, used by e.g. "convert text to curves" and text-search/accessibility) |

**Architectural takeaway for the C# port:** you do not need to design a brand new
IR. `drawinglayer`'s primitive tree (recursive, decomposition-based, ~30 "must
implement" cases with everything else falling back to decomposition) is a proven,
production-grade design specifically built to let a *new* renderer support a large
surface area by implementing a small kernel. This is a stronger model to copy than
the flat `MetaAction` list for **shape/drawing content** (Draw/Impress/Writer
frames), while the flat `MetaAction`/`GDIMetaFile` model remains the better fit for
**page-level output** (what actually gets sent to print/PDF, and what WMF/EMF import
naturally produces). In practice LibreOffice uses both, bridged by
`metafileprimitive2d` (primitive→wraps a metafile) and `vclmetafileprocessor2d`
(primitive tree→bakes down to a metafile) — a C# port should plan for the same two-
tier design: a small recursive primitive/scene-graph IR for shape authoring content,
flattened to a simple ordered draw-command list for final page composition/export.

### C.3 Gradient geometry

Two independent gradient math implementations exist and a port needs to pick (or
replicate) the right one depending on which layer it's matching:

- **Legacy/VCL gradient** (`vcl/source/gdi/gradient.cxx`): used by `MetaGradientAction`/
  `MetaGradientExAction` and classic `OutputDevice::DrawGradient`. Step count is
  either explicit (`Gradient::GetSteps()`/`SetSteps()`,
  `gradient.cxx:191-198`) or computed from rectangle size
  (`GetMetafileSteps`, referenced at `:339-342`), then clamped to
  `nMaxColorSteps`/at-least-2 (`:458`, `:551`) — i.e. LO renders a gradient as a
  **discrete stack of N flat-colored bands/polygons**, not a true continuous shader,
  for metafile/print/PDF output (this is directly relevant to pixel-comparable
  output: reproduce the same *banding*, not a smooth GPU-shader gradient, if
  matching legacy rendering exactly is the goal — a smooth gradient will look
  subtly "too clean" next to LO's classic renderer).
- **basegfx/drawinglayer gradient** (`basegfx/source/tools/gradienttools.cxx`, 760
  lines): produces an `ODFGradientInfo` (texture transform matrix + aspect ratio +
  step count) consumed by `FillGradientPrimitive2D`/`SvgGradientPrimitive2D` for the
  modern shape-rendering path; this one can decompose to either a stepped-band
  polygon fill (parity with legacy) or, via the `cairopixelprocessor2d`/GPU
  processors, a true smooth gradient — so "banded vs smooth" is a per-processor
  choice, not an inherent property of the primitive.
- Hatch geometry (angle/distance/style) is computed in
  `vcl/source/gdi/hatch.cxx`/`OutputDevice::DrawHatchLine` (declared
  `include/vcl/outdev.hxx:670`) as literal repeated line segments clipped to the
  fill polygon — straightforward to reproduce exactly (angle, spacing in logical
  units, single/double/triple/diamond-crossed hatch styles).

---

## D. Bitmap/graphic import

### D.1 Raster formats — `GraphicFilter`

`GraphicFilter` (`include/vcl/graphicfilter.hxx:231`) is the top-level façade;
`GraphicFilter::ImportGraphic`/`GraphicFilter::ExportGraphic` dispatch by detected
(or forced) format. Format detection lives in
`vcl/source/filter/GraphicFormatDetector.cxx`, whose `GraphicFileFormat` enum
(`vcl/inc/graphic/GraphicFormatDetector.hxx:34-121`) is the authoritative list of
every format LO can sniff/import:

`BMP, GIF, JPG, PCD, PCX, PNG, APNG, XBM, XPM, PBM, PGM, PPM, RAS, TGA, PSD, EPS, TIF,
DXF, MET, PCT, SVM, WMF, EMF, SVG, WMZ (gzipped WMF), EMZ (gzipped EMF), SVGZ (gzipped
SVG), WEBP, MOV, PDF`.

Import implementations, one directory per format under `vcl/source/filter/`:
`ipng/` (well, `png/`), `ijpg/` (`jpeg/`), `igif/`, `itiff/`, `iwebp/` (`webp/`),
`ibmp/` (`bmp/`), plus `eps/`, `ipdf/`, `imet/`, `ipcd/`, `ipcx/`, `ipict/`, `ipsd/`,
`iras/`, `itga/`, `ixbm/`, `ixpm/`, `idxf/`, and the vector formats `wmf/`, `svm/`
(all listed at `vcl/source/filter/` — 22 format subdirectories in total). Most of
these (PNG/JPEG/GIF/WEBP/TIFF) are thin wrappers over well-known third-party codecs
(libpng, libjpeg-turbo, giflib-equivalent, libwebp, libtiff) rather than custom
decoders — good news for a C# port, since SkiaSharp's bundled codecs
already cover this entire common-raster-format list with no meaningful gaps. The
exotic legacy formats (PCD/PCX/PICT/PSD/RAS/TGA/MET/DXF) are extremely unlikely to
appear in real-world DOCX/XLSX/PPTX/ODF content — safe to deprioritize or skip.

### D.2 `Graphic`, `GraphicObject`, and the `Bitmap`/alpha model

`vcl::graphic::Graphic`/`GraphicObject` (`vcl/source/gdi/graph.cxx`,
`vcl/source/graphic/`) wrap **either** a raster bitmap **or** a vector `GDIMetaFile`
behind one handle — i.e. "a `Graphic`" in LO is polymorphic over raster vs. vector,
which is exactly the right model for a C# port's own image abstraction given how
often WMF/EMF/SVG content is embedded as "just a picture" in a document.

Note the class name history: what older LibreOffice/OOo documentation calls
`BitmapEx` (bitmap + separate alpha/transparency channel) has been **merged directly
into `Bitmap`** in this checkout — there is no `BitmapEx` class anymore
(`include/vcl/bitmap.hxx:125`, `class Bitmap final`). A `Bitmap` can itself carry an
embedded alpha channel; `AlphaMask` (`include/vcl/alpha.hxx:31`) is a thin wrapper
around an 8-bit-per-pixel `Bitmap` used to construct/extract that alpha data
(`Bitmap::CreateAlphaMask()`, `bitmap.hxx:164`; `Bitmap(const Bitmap&, const
AlphaMask&)` combining ctor, `:135`; `SplitIntoColorAndAlpha()`, `:692`). A C# port
targeting current-generation LO fidelity should model images the same way: one
bitmap type with an optional embedded alpha plane, not two parallel types.

### D.3 Vector metafile import — WMF/EMF/EMF+ (`emfio`)

`emfio/source/reader/` has exactly three translation units doing all the work:
`wmfreader.cxx` (legacy 16-bit Windows Metafile), `emfreader.cxx` (32-bit Enhanced
Metafile), and `mtftools.cxx` (3206 lines — shared helper: coordinate/DC-state
tracking, `MtfTools` class that both readers drive to synthesize `GDIMetaFile`
`MetaAction`s from parsed WMF/EMF records). Total ≈ 8188 lines across the three
files — a comparatively compact, single-pass, record-by-record translator from
WMF/EMF binary records straight into `MetaAction` calls (i.e. output is a
`GDIMetaFile`, feeding straight back into §A.3's IR).

**EMF+ is not handled in `emfio` at all** — `mtftools.cxx` only recognizes the
`EMR_COMMENT` record that wraps an EMF+ payload and *passes the raw bytes through*
as an opaque blob (`MtfTools::PassEMFPlusHeaderInfo`/`PassEMFPlus`,
`mtftools.cxx:3166-3206`) tagged as a `MetaCommentAction`. The actual EMF+ record
parser/interpreter lives in **`drawinglayer/source/tools/`**:

| File | Role |
|---|---|
| `emfphelperdata.cxx` (2916 lines) | the EMF+ record-type switch/dispatcher and DC-state machine (`case EmfPlusRecordType...`, ~50+ cases from `EmfPlusRecordTypeHeader` through `EmfPlusRecordTypeDrawDriverString`, `emfphelperdata.cxx:1655` onward) — builds a `drawinglayer` primitive tree per record |
| `emfpbrush.cxx`/`.hxx` | EMF+ brush objects → `FillGradientPrimitive2D`/hatch/solid fill primitives |
| `emfppen.cxx`/`.hxx` | EMF+ pen objects (dash patterns, caps, joins, compound lines) |
| `emfppath.cxx`/`.hxx` (354 lines) | EMF+ `GraphicsPath` object → `B2DPolyPolygon` |
| `emfpcustomlinecap.cxx`/`.hxx` | custom arrow/line-cap shapes |
| `emfpregion.cxx`/`.hxx` | EMF+ region objects (clip) |
| `emfpimage.cxx` (150 lines) | embedded raster images inside EMF+ (`EmfPlusRecordTypeDrawImage`) |
| `wmfemfhelper.cxx` | shared WMF/EMF/EMF+ geometry helpers |

Coverage is broad — the record-name table at `emfphelperdata.cxx:79-126` lists ~50
named EMF+ record types including all the fill/stroke primitive records
(rects/polygons/ellipses/pies/paths/closed-curves/Bezier-curves/lines), `DrawImage`/
`DrawImagePoints` (bitmap placement), `DrawString`/`DrawDriverString` (native
GDI+ text, including per-glyph positioned "driver string" runs used by editors like
PowerPoint for precise placement), world-transform stack ops
(Save/Restore/BeginContainer/EndContainer/*WorldTransform), and clip ops
(SetClipRect/Path/Region/OffsetClip). This is **real, substantial EMF+ support**, not
a stub — expect PowerPoint files with vector clip-art (which is overwhelmingly
EMF+ in modern `.pptx`) to render close to correctly. There is currently **no
mature, actively maintained open-source C# EMF+ parser** with comparable coverage
(see §G) — this is the single largest "no good off-the-shelf C# library" gap
identified in this investigation.

### D.4 SVG import (`svgio`)

`svgio/source/svgreader/` (≈14,861 total lines across ~40 files) is a from-scratch
SVG 1.1-subset parser/renderer built directly on `drawinglayer` primitives (each
`svg*node.cxx` file corresponds to one SVG element: `svgpathnode`, `svgrectnode`,
`svgcirclenode`/`svgellipsenode`, `svggradientnode`/`svggradientstopnode`,
`svgpatternnode`, `svgclippathnode`, `svgmasknode`, `svgmarkernode`,
`svgfe*node.cxx` — Gaussian blur/color-matrix/composite/blend/flood/offset/merge
filter primitives, `svgtextnode`/`svgtextpathnode`/`svgtextposition`,
`svgswitchnode`, `svgsymbolnode`, `svgstylenode`/`svgstyleattributes` for CSS-like
styling, `svgtitledescnode` for accessibility metadata). This directly produces
`drawinglayer::primitive2d` trees (§C.2), so an SVG import in a C# port that also
adopted a `drawinglayer`-style primitive tree could reuse the exact same downstream
rasterizer/PDF-exporter as native shapes — a strong argument for standardizing on
one primitive IR across "native shapes" and "imported vector graphics."

### D.5 `.svm` — LibreOffice's own metafile serialization

`SvmReader`/`SvmWriter` (`include/vcl/filter/SvmReader.hxx`,`SvmWriter.hxx`, impl
`vcl/source/filter/svm/SvmReader.cxx`,`SvmWriter.cxx`) serialize a `GDIMetaFile`
(§A.3) to/from LO's own binary `.svm` format — essentially a direct byte-for-byte
dump of the `MetaAction` list (tag + fields per action, `switch` over
`MetaActionType`). This is only really relevant for round-tripping LO-authored
content (clipart gallery, some ODF-internal graphic caches); a C# port targeting
DOCX/XLSX/PPTX/PDF fidelity does not need to implement `.svm` itself, but the file
*is* a good, minimal worked example of "serialize this exact IR" if a similar binary
form is ever wanted for the C# `MetaAction`-equivalent IR.

---

## E. PDF export

### E.1 Architecture: `PDFWriterImpl` is an `OutputDevice`

The most important fact for this section: `PDFWriterImpl` (`vcl/inc/pdf/
pdfwriter_impl.hxx:712`) is declared `final : public VirtualDevice, public
PDFObjectContainer`. **PDF export therefore does not "translate a metafile to PDF" as
a separate offline pass — it *is* a `VirtualDevice`/`OutputDevice` implementation**
whose `Draw*`/`Set*` method overrides emit PDF content-stream operators (and, where
needed, new indirect PDF objects — fonts, images, ExtGState soft masks, annotations)
instead of touching pixels. All the ordinary document-rendering code that would
normally paint onto a screen or a raster `VirtualDevice` can therefore paint onto a
`PDFWriterImpl` completely unmodified.

The actual page-content bridge is `PDFWriterImpl::playMetafile`
(`vcl/source/pdf/pdfwriter_impl2.cxx:272`), invoked by
`filter/source/pdf/pdfexport.cxx:1333` (`rWriter.PlayMetafile(aMtf, aCtx,
&rPDFExtOutDevData)`). It is a straight `switch` over every `MetaActionType`
(`pdfwriter_impl2.cxx:295-1007+`), one case per action from §A.3.1 — `PIXEL` through
`REFPOINT` are all handled (`RASTEROP`/`REFPOINT` are effectively no-ops for PDF,
`:1006-1007`). `MetaActionType::COMMENT` (`:555`) is special-cased to recover
grouping/transparency-group boundaries and native-JPEG-passthrough hints that were
smuggled through as comment payloads (§A.3.1) — e.g. `FILLCOLOR`-inside-comment
sequences used for gradient/hatch pre-rendering fallback data
(`pdfwriter_impl2.cxx:569-735`).

**End-to-end PDF export pipeline:**

1. Each document module (`sw`/`sc`/`sd`) implements UNO `XRenderable` on its model
   object (`SwXTextDocument::getRendererCount/render`,
   `sw/source/uibase/uno/unotxdoc.cxx:2670,3155`; `ScModelObj::...`,
   `sc/source/ui/unoobj/docuno.cxx:1911,2605`; `SdXImpressDocument::...`,
   `sd/source/ui/unoidl/unomodel.cxx:3088,3497`).
2. `filter/source/pdf/pdfexport.cxx` (`PDFExport::Export`) drives that
   `XRenderable`: for each page, calls `render(nPage, selection, renderOptions)`
   which internally lays the page out onto a `VirtualDevice` connected to a
   `GDIMetaFile` recorder (§A.3) — i.e. each module still produces the flat
   `MetaAction` IR per page, regardless of whether its *shape* rendering internally
   used the recursive `drawinglayer` primitive tree (§C.2) to get there.
3. `PDFExport::ImplExportPage` (`pdfexport.cxx:1296`) takes that per-page
   `GDIMetaFile`, optionally strips/downsamples transparency
   (`RemoveTransparenciesFromMetaFile`, honoring `mbRemoveTransparencies`), sets up a
   `PlayMetafileContext` (max image resolution, JPEG quality, lossless-only flag),
   and calls `rWriter.PlayMetafile(...)` (§E.1) to translate that page's flat action
   list into PDF page content + resources.
4. `vcl::pdf::PDFWriter`/`PDFWriterImpl` (`vcl/source/pdf/pdfwriter.cxx`,
   `pdfwriter_impl.cxx` — 11258 lines, `pdfwriter_impl2.cxx` — 1100 lines) assembles
   the final PDF byte stream: object table, cross-reference table/stream, page tree,
   font objects (subset or full-embed, §E.2), image XObjects, ExtGState soft masks
   for transparency, optional structure tree for tagged/accessible PDF, optional
   encryption (`PDFEncryptor.cxx`, `PDFEncryptorR6.cxx` — AES-128/256, RC4).

### E.2 Font subsetting/embedding for PDF

This checkout uses a **modern HarfBuzz-based subsetting pipeline**, not the legacy
hand-rolled TrueType/Type1/CFF subsetter implied by older LibreOffice documentation
(`vcl/source/fontsubset/cff.cxx`, 3597 lines, still exists and is used for
lower-level CFF table parsing/manipulation, but the actual "build me a subset font
for embedding" entry point is HarfBuzz's `hb-subset` API):

- `PhysicalFontFace::CreateFontSubset` (`vcl/source/font/PhysicalFontFace.cxx:509`)
  builds an `hb_subset_input_t`, adds the exact glyph IDs used on the page to the
  subset glyph set, sets up **old-to-new glyph ID remapping** so each glyph lands at
  the PDF-encoding byte position the content stream expects
  (`hb_subset_input_old_to_new_glyph_mapping`, `:539-544`), restricts kept SFNT
  tables to exactly what PDF embedding needs (`head, hhea, hmtx, loca, maxp, glyf,
  CFF , post, name, OS/2, cvt, fpgm, prep, CFF2`, `:549-554`), and calls
  `hb_subset_or_fail`.
- Variable-font handling: non-CFF2 variable fonts are **instanced** (a specific
  variation-axis position is baked into a static font) via `hb-subset` before
  embedding (`pdfwriter_impl.cxx:5507`); CFF2 variable fonts are downgraded to plain
  CFF (`HB_SUBSET_FLAGS_DOWNGRADE_CFF2`, PDF has no CFF2 support) with HarfBuzz
  ≥13.0.0, and fall back to embedding as **Type 3 (bitmap-per-glyph) fonts** for
  CFF2/color fonts on older HarfBuzz or unsupported cases
  (`pdfwriter_impl.cxx:5504-5568` — see the comment at `:5504-5509`).
- Color fonts (`COLR`/`CPAL`, or embedded-bitmap `sbix`/`CBDT`) are also routed to
  Type 3 embedding (`pFace->IsColorFont()`, `:5514`).
- Legacy Type 1 output path still exists for cases needing it (`"<</Type/Font/
  Subtype/Type1/BaseFont/"`, `pdfwriter_impl.cxx:1392,2192`) but the default/primary
  path for TrueType/OpenType fonts is TrueType or CFF subset embedding via
  `CreateFontSubset` (`:2093`).
- Failure fallback: if `CreateFontSubset` fails, LO emits a warning comment into the
  PDF and falls back to one of PDF's 14 standard base fonts
  (`vcl/source/pdf/pdfbuildin_fonts.cxx`) rather than failing the export.

**Recommendation for C#:** replicating this exact "glyph-subset + old-to-new glyph
remap + table allowlist" recipe is very achievable with HarfBuzzSharp, which exposes
the same `hb-subset` API 1:1 (`hb_subset_input_*`, `hb_subset_or_fail`) — this is
one of the strongest "good C# option exists, just call the same underlying library"
findings in this whole investigation (see §G).

### E.3 Transparency, soft masks, tagged PDF, image passthrough

- **Transparency groups & soft masks**: `MetaActionType::FLOATTRANSPARENT` (gradient-
  shaped alpha — e.g. drop shadows, soft-edged glow) is realized in PDF as an
  `ExtGState` soft mask referencing a separate transparency-group XObject whose
  luminosity encodes the alpha; flat `MetaActionType::Transparent` (uniform %) maps
  to a plain `ExtGState /ca /CA` alpha value. PDF/A export forces
  `mbRemoveTransparencies`-style flattening because PDF/A-1b forbids transparency
  entirely (`include/vcl/pdfwriter.hxx:227,324`: "PDF/A does not allow transparency").
- **Tagged/accessible PDF**: `PDFWriter::BeginStructureElement`/
  `EndStructureElement`/`InitStructureElement` (`include/vcl/pdfwriter.hxx:1116-1170`)
  build a logical structure tree (`StructTreeRoot`, id 0) in parallel with content
  emission, driven by `texthierarchyprimitive2d`/`structuretagprimitive2d` (§C.2.1)
  markers recorded during layout — controlled by the `Tagged` export option
  (`pdfwriter.hxx:667`).
- **JPEG passthrough**: when a document embeds an already-JPEG-compressed image, LO
  does **not** decode+re-encode it for PDF — it detects `GfxLink::GetType() ==
  GfxLinkType::NativeJpg` (`vcl/source/pdf/pdfextoutdevdata.cxx:439,467`) and copies
  the original compressed byte stream straight into a PDF image XObject with
  `/Filter /DCTDecode`, preserving both quality and file size. The same mechanism
  exists for `NativePng`/`NativePdf` sources when "adequate compression" is already
  present (`:445-447`) — worth copying, since re-encoding is both slower and lossier
  than passthrough for the common case.
- **PDF versions**: `SelectPdfVersion` FilterData option selects among PDF 1.4–2.0
  and `PDF_A_1`/`PDF_A_2`/`PDF_A_3`/`PDF_A_4` profiles (`include/vcl/pdfwriter.hxx`
  enum near `:227`); PDF/A forbids transparency, requires embedded fonts and
  color-managed color, and requires XMP metadata (`vcl/source/pdf/XmpMetadata.cxx`).

---

## F. Headless conversion entry points

### F.1 `soffice --headless --convert-to <ext>[:filter[:filterOptions]]` end-to-end

1. **Argument parsing**: `desktop/source/app/cmdlineargs.cxx:556` recognizes
   `--convert-to`; the very next token is stored as
   `"output_file_extension[:output_filter_name]"` (error message text at `:567`
   confirms the exact grammar). `--convert-to` implies headless
   (`cmdlineargs.cxx:563`); `--outdir` (`:600`) must directly follow a
   `--convert-to`/`--print-to-file` spec to redirect output location.
   `--infilter=<name>[:<options>]` (`cmdlineargs.cxx:450-452`) forces the *input*
   filter (bypassing type-sniffing) — necessary for ambiguous extensions (e.g.
   forcing a `.txt` file to be read as CSV with a specific delimiter/encoding token
   string, or forcing a `.doc` to be read by a specific legacy filter).
2. **IPC/dispatch**: whether from the very first `soffice` process or relayed to an
   already-running instance via `officeipcthread.cxx` (`:954,1278` — pulls
   `aRequest.aInFilter` from parsed cmdline args and adds it to the dispatch list),
   the request lands in `DispatchWatcher::executeDispatchRequests`
   (`desktop/source/app/dispatchwatcher.cxx`, 876 lines).
3. **Filter-name / filter-options parsing** (`dispatchwatcher.cxx:648-693`): the
   `OutputFileExtension[:OutputFilterName[:FilterOptions]]` string is split on the
   first `:` — everything before is `aFilterExt` (used to name the output file) and
   `sFilterName` (the export filter's *internal* name, e.g. `writer_pdf_Export`,
   `impress_png_Export`, `calc_pdf_Export`); everything after the second `:` is
   passed through verbatim as `FilterOptions` (a `PropertyValue` on the
   `storeToURL()` call, `:687-688,721`). If **no** `:` is present at all, the bare
   token is passed as `FilterName` directly and the *default* export filter for that
   extension is looked up via `impl_GuessFilter` (`:635`) when even that is empty.
4. **FilterOptions grammar** — two distinct conventions coexist depending on target
   filter:
   - **Comma-separated positional tokens** (legacy convention), e.g. Calc's
     `"Text - txt - csv (StarCalc)"` filter reads `FilterOptions` as a comma-joined
     positional list — token 11 (0-based) controls per-sheet-file splitting
     (`dispatchwatcher.cxx:666-683`); the classic CSV filter string is of the form
     `field-separator,text-delimiter,character-set,...`.
   - **Inline JSON**, recognized purely by the string *starting with `{`*: both
     `filter/source/graphic/GraphicExportFilter.cxx:104-118` (covers all raster
     image export filters — PNG/JPG/GIF/BMP/etc. via the generic graphic exporter
     used for `impress_png_Export` and friends) and
     `filter/source/pdf/pdffilter.cxx:80-92` do, verbatim:
     ```cpp
     if (!aFilterData.hasElements() && aFilterOptions.startsWith("{"))
         aFilterData = comphelper::containerToSequence(
             comphelper::JsonToPropertyValues(aFilterOptions.toUtf8()));
     ```
     i.e. `comphelper::JsonToPropertyValues`
     (`comphelper/source/misc/sequenceashashmap.cxx:306-397`, using
     `boost::property_tree`) parses the JSON blob straight into a `Sequence<
     PropertyValue>` that's indistinguishable from a normal `FilterData` argument
     passed via the API — **this is exactly the `png:impress_png_Export:{...}`
     syntax the task description references**, and it works for *any* filter that
     checks `FilterOptions.startsWith("{")` this way (PDF and all raster graphic
     exports at minimum).
5. **Recognized FilterData keys** (non-exhaustive, gathered from the two filters
   above):
   - Raster export (`GraphicExportFilter.cxx:122-129`): `PixelWidth`, `PixelHeight`
     (target raster size in pixels; if either is 0/absent, falls back to the
     document's native page size in pixels at 96dpi, `filterRenderDocument()`
     `:174-185`). Underlying `GraphicFilter::ExportGraphic` (called at `:194`) also
     honors format-specific `FilterData` keys handled deeper in each codec (PNG:
     compression level/interlacing; JPEG: `Quality`; not enumerated exhaustively
     here — check `vcl/source/filter/png/PngImageWriter.cxx` and
     `vcl/source/filter/jpeg/` for the authoritative key names of the format
     you're targeting).
   - PDF export (`filter/source/pdf/pdfexport.cxx:538-617`): `PageRange` (e.g.
     `"1-4,7"`), `UseLosslessCompression` (bool), `Quality` (JPEG quality 1-100 for
     lossy image compression), `ReduceImageResolution` (bool) +
     `MaxImageResolution` (DPI cap applied when the previous flag is set),
     `IsSkipEmptyPages` (bool), `SelectPdfVersion` (PDF 1.4-2.0 / PDF-A variant
     enum), `ExportNotes` (bool, Impress speaker notes / Writer comments),
     `Watermark` (text overlay string).
6. **Storing**: `xStorable->storeToURL(aOutFile, conversionProperties)`
   (`dispatchwatcher.cxx:721`) is the actual trigger — `conversionProperties` always
   includes `ConversionRequestOrigin="CommandLine"`, `Overwrite=true`, `FilterName`,
   and (when present) `FilterOptions`/`ImageFilter` (the `--convert-images-to`
   companion flag, which forces all embedded raster images to be re-encoded to a
   given format on export, `cmdlinehelp.cxx:155-156`).

### F.2 CLI syntax reference (from `cmdlinehelp.cxx:151-187`, verbatim examples)

```
--convert-to pdf *.odt
--convert-to epub *.doc
--convert-to pdf:writer_pdf_Export --outdir /home/user *.doc
--convert-to "html:XHTML Writer File:UTF8" --convert-images-to "jpg" *.doc
--convert-to "txt:Text (encoded):UTF8" *.doc
--print-to-file [--printer-name printer_name] [--outdir output_dir]
--cat               # dump text content of following files to console (headless implied; not combinable with --convert-to)
--script-cat        # dump embedded script/macro source instead of document text
--infilter=<name>[:<filterOptions>]   # force input filter, bypassing type sniffing
```

`--convert-to txt`/`html`/`csv` route through the same mechanism as PDF/PNG — only
the `FilterName` differs (e.g. Writer's plain-text export filter, Calc's
`"Text - txt - csv (StarCalc)"`), which makes these all useful, low-cost text/content
*extraction* paths (not just "conversion") — a C# port that already has DOCX/XLSX/
PPTX parsers of its own would typically not need to shell out for this, but it's a
useful reference implementation for what "correct" plain-text/HTML extraction looks
like (e.g. how footnotes, headers/footers, and multi-sheet workbooks are
linearized).

---

## G. Assessment for C#

Legend: ✅ good existing option · ⚠️ partial/needs work · ❌ no good off-the-shelf option (plan to hand-roll or heavily adapt).

| Layer | LibreOffice does this with | Recommended C# approach | Verdict |
|---|---|---|---|
| Raster canvas (fill/stroke/clip/blend, headless, no display) | VCL headless backend → libcairo (`vcl/headless/CairoCommon.cxx`) | **SkiaSharp** (`SKSurface`/`SKCanvas`, offscreen `SKBitmap`/raster or PDF-backed `SKDocument`) | ✅ Direct architectural analogue — cairo and Skia are peer technologies (path-fill AA rasterizers with matrix-based CTM, clip stacks, gradients, blend modes). SkiaSharp even ships its own PDF canvas backend, which is tempting but see PDF row below for why it's not a full substitute for LO's PDF writer. |
| Rendering IR / display list | `GDIMetaFile`/`MetaAction` (flat, replayable, §A.3) + `drawinglayer` primitive tree (recursive, decomposable, §C.2) | Hand-roll a two-tier IR modeled directly on these: (1) a small recursive "scene" primitive set with a `Decompose()` contract for shape content, (2) a flat ordered "draw command" list for page-level composition/export, with an explicit bridge between them (mirroring `metafileprimitive2d`/`vclmetafileprocessor2d`) | ✅ (design, not a library) — this is genuinely the best part of LO's architecture to copy wholesale; it's a proven, minimal-surface design for "support lots of rendering features via a small required kernel + decomposition." |
| Font enumeration/matching/substitution | Native OS font APIs + fontconfig (Linux) + LO's own `VCL.xcu` substitution table + `DirectFontSubstitution` | Parse `officecfg/registry/data/org/openoffice/VCL.xcu`'s `FontSubstitutions`/`DefaultFonts` tables directly (it's just XML) as your substitution source of truth; for physical font discovery, `SkiaSharp`'s `SKFontManager` (wraps DirectWrite/CoreText/fontconfig per-OS) is a reasonable enumerator | ✅ for the *table* (just reuse LO's own XCU — no need to reverse-engineer substitution rules), ⚠️ for physical enumeration/fontconfig-parity on Linux (SkiaSharp's fontconfig binding is close but not byte-identical to LO's own `FontConfigManager`) |
| Text shaping (HarfBuzz) | HarfBuzz directly, `vcl/source/gdi/CommonSalLayout.cxx` | **HarfBuzzSharp** (official Google/SkiaSharp-adjacent binding, same native HarfBuzz build) | ✅ Best possible parity — same underlying C library, so shaping *output* (ligatures, kerning, complex scripts, kashida justification) should match near bit-for-bit given the same font + feature string + script/bidi run boundaries. The work is entirely in replicating the *run segmentation* (next row) and the *metrics precedence* (§B.4) correctly around it. |
| Bidi + Unicode script runs | ICU (`ubidi_*`) + ICU `icu::UObject`-based `ScriptRun` (`i18nutil/scriptchangescanner.cxx`, `vcl/inc/scrptrun.h`) | **ICU4N** (managed ICU port) for convenience, or a native ICU4C P/Invoke wrapper for exact `ubidi` parity; either way, replicate LO's specific run-splitting order (bidi levels first, then script boundaries within each level) before feeding runs to HarfBuzzSharp | ✅ ICU4N covers `Bidi`/script property APIs adequately; ⚠️ verify edge-case parity (mirrored characters, paragraph embedding level defaults) against ICU4C if pixel-exact bidi reordering matters |
| Font metrics (ascent/descent/leading, line height) | Custom precedence: variable-font passthrough → hhea → OS/2 (win vs. typo via `fsSelection` bit 7) → derived internal leading (§B.4) | Reimplement `FontMetricData::ImplCalcLineSpacing` verbatim using **HarfBuzzSharp**'s `hb_ot_metrics_get_position` bindings (same API surface as the C++ code) — do **not** trust SkiaSharp's `SKFontMetrics` or `System.Drawing`/GDI+ line-height defaults, they don't follow this exact precedence | ✅ HarfBuzzSharp exposes what's needed; the *algorithm* itself must still be hand-ported (it's ~150 lines, well worth doing exactly) |
| Font subsetting for PDF embedding | HarfBuzz `hb-subset` (glyph-set + old→new glyph remap + table allowlist, §E.2) | **HarfBuzzSharp**'s `hb-subset` bindings, calling the identical API (`hb_subset_input_*`, `hb_subset_or_fail`) LO itself uses | ✅ One of the strongest findings here — this isn't "an equivalent library," it's literally the same subsetting engine, callable from C# today. |
| PDF *writing* with embedded subset fonts, tagged structure, transparency groups/soft masks, JPEG passthrough | Hand-rolled `PDFWriterImpl`/`PDFObjectContainer` (`vcl/source/pdf/pdfwriter_impl.cxx`, 11k+ lines) acting as an `OutputDevice` | **This is the weakest link for off-the-shelf C#.** No mature managed library both (a) writes arbitrary low-level PDF objects/content streams under full programmatic control *and* (b) has first-class embedded/subset OpenType (CFF+TrueType+CFF2-instancing+color-font) support with soft-mask transparency groups. Practical options, roughly ranked: **PdfSharpCore/PDFsharp** (actively maintained forks exist; good low-level object/content-stream control, but font-embedding/subsetting is comparatively weak — you'd likely need to feed it pre-subset font bytes from HarfBuzzSharp yourself and hand-build the `/FontFile2`/`/FontFile3`/CIDFont dictionaries); **QuestPDF** (excellent high-level fluent layout API, but that's the wrong layer here — you need low-level content-stream emission matching a pre-computed layout, not QuestPDF's own layout engine); **PdfPig** is read-only (parsing), not relevant to export at all. | ❌ Plan to hand-roll the PDF object/content-stream/xref writer yourself (it is a well-documented, bounded format — the actual difficulty is entirely in font embedding, which HarfBuzzSharp's hb-subset solves for you) rather than expect a single library to give you PDFWriterImpl-equivalent capability. |
| WMF/EMF import | Hand-rolled binary record reader (`emfio/`, ~8.2k lines) | No actively-maintained, sufficiently-complete open-source C# WMF/EMF *parser* exists (`System.Drawing`'s `Metafile` class can *play* WMF/EMF only on Windows via GDI+, is not cross-platform, and gives no access to a structured record list for re-targeting to your own canvas) | ❌ Hand-roll; the format is well documented (MS-WMF/MS-EMF specs) and `emfio/source/reader/mtftools.cxx` is a good, compact reference implementation to port logic from (record parsing + DC-state tracking, mapped onto your own IR from row 2) |
| EMF+ import | Hand-rolled record interpreter (`drawinglayer/source/tools/emfp*.cxx`, ~3.8k lines, ~50 record types) | Same situation, more severe — EMF+ (MS-EMFPLUS spec) is a second, denser format layered inside EMF comment records, with its own object model (brushes/pens/paths/regions/images/world-transform stack) | ❌ **This is the single biggest gap flagged in this whole investigation.** No usable C# library. Given how common EMF+ vector clip-art is in real-world `.pptx`/`.docx` files, budget real time for porting `emfphelperdata.cxx`'s record dispatcher and the `emfp*` object model — it's substantial but bounded (a few thousand lines), and LO's implementation is a solid, readable reference to translate from. |
| SVG import | Hand-rolled parser → `drawinglayer` primitives (`svgio/`, ~14.9k lines) | **SkiaSharp.Svg**, **Svg.Skia** (resvg-adjacent), or **Svg** (W3C-ish .NET SVG renderer) all exist and are reasonably mature | ✅ Multiple viable options; verify filter-effect coverage (Gaussian blur/color-matrix/composite — LO has explicit `svgfe*` nodes for these) since that's where lighter-weight SVG libraries often cut corners |
| Common raster codecs (PNG/JPEG/GIF/WEBP/TIFF/BMP) | libpng/libjpeg-turbo/libwebp/libtiff via thin VCL wrappers | **SkiaSharp** (broadest format+color-space coverage, native codecs) | ✅ No gap; SkiaSharp comfortably exceeds what's needed here, and reusing the canvas layer's own codecs avoids a second image-decode dependency and any color-management mismatch between two libraries. |
| Gradient/hatch geometry | Custom stepped-band math for metafile/legacy parity (`vcl/source/gdi/gradient.cxx`) vs. true continuous math for modern shape rendering (`basegfx/source/tools/gradienttools.cxx`) | Hand-roll both variants directly from the cited files — this is simple, well-bounded arithmetic (not a "library" problem) | ✅ trivial to port, just don't skip the *banded* legacy variant if matching classic export output is a goal |
| Units/measurement conversion | Single generic rational-arithmetic `o3tl::convert`/`Length` enum covering 19 units incl. EMU/twip/mm100/point/pica/master-unit | Hand-roll a small `enum Unit { Mm100, Twip, Point, Emu, Inch, Pixel, ... }` + one generic `Convert(value, from, to)` using exact rational multipliers (mirror `o3tl/unit_conversion.hxx`'s table) | ✅ trivial, but **do this exactly this way** (one canonical unit + one generic converter) — do not scatter ad hoc `* 20`/`/ 2540` style conversions through the codebase, that's precisely the kind of drift LO's design avoids |

### Summary recommendation

For a from-scratch C# renderer aiming at "reasonable fidelity" headless
raster+PDF output:

1. **Canvas**: SkiaSharp for raster. For PDF, do not rely on SkiaSharp's built-in PDF
   canvas if embedded/subset font fidelity and PDF/A or tagged-PDF requirements
   matter — write your own thin PDF object/content-stream layer (bounded, well-
   specified format) fed by the same draw-command IR used for raster, exactly
   mirroring how `PDFWriterImpl` is "just another `OutputDevice`" in LO.
2. **Text**: HarfBuzzSharp for shaping *and* font subsetting (same native library LO
   uses for both) + ICU4N (or ICU4C P/Invoke) for bidi/script runs, with LO's exact
   font-metrics precedence (§B.4) and cumulative-DX-array convention (§B.3) ported
   line-for-line — these two algorithms are small but are exactly where subtle
   layout mismatches come from.
3. **IR**: design the two-tier flat-list + recursive-decomposition rendering IR
   modeled directly on `GDIMetaFile`/`MetaAction` + `drawinglayer`'s
   `BasePrimitive2D` decomposition contract (§A.3, §C.2) — this is free, proven
   design work to copy rather than reinvent.
4. **Fonts**: reuse LO's own `VCL.xcu` substitution table as data (don't
   reverse-engineer it) for the "requested font not installed" fallback chain.
5. **Images**: SkiaSharp covers the common raster formats fully; budget
   real, non-trivial engineering time specifically for **EMF+ import**, which has no
   existing C# library and is very common in real-world Office documents — this is
   the highest-risk, least-covered area identified in this investigation, followed
   by full-fidelity PDF *writing* with subset-font embedding (font subsetting itself
   is solved by HarfBuzzSharp; the surrounding PDF object graph is not solved by any
   single existing library and should be hand-written against the PDF spec).
