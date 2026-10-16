# X-Ray.Office — working notes

X-Ray.Office is a **pure C# / .NET** library set for content extraction and headless
rendering of the file formats LibreOffice's Writer, Calc and Impress support.

It lives in the `dotnet/` subdirectory of a LibreOffice source checkout. The surrounding
C++ tree is **reference material, not a build dependency** — we read it to learn how the
formats behave, and we run an installed `soffice` to generate ground truth.

**It is X-Ray.Office, part of the [X-Ray](https://github.com/curiosity-ai/x-ray) family, and it was
called Paperless until it joined.** Package ids are hyphenated (`X-Ray.Office`,
`X-Ray.Office.Core`, ...); projects, assemblies and namespaces are not (`XRay.Office.Core`). In
prose the product is "X-Ray.Office"; `XRay.Office` is a code identifier. Environment variables are
`XRAY_OFFICE_*` and the CLI is `xray-office`. Older probe write-ups, branch names and fixture text
still say Paperless: the fixtures' text is what the tests assert, so leave it.

**Packaging.** Every library under `src/` is packable; `Directory.Build.props` derives its
`PackageId`, packs `assets/README.package.md` and `assets/logo.png`, and sets the licence
(MPL-2.0; `XRay.Office.Text` widens it for the bundled fonts). The facade `X-Ray.Office` references
every other library, so installing it brings the set. Tests, the test kit, the CLI and probes are
not packable. `.devops/build-x-ray-office-nuget.yml` publishes all thirteen in one run at one CalVer
version on a change to `dotnet/` on `master`; a library added under `src/` must be added to its
lists, and to the facade's references.

## Scope

**In scope.** Word processing (`docx docm dotx dotm doc dot rtf odt ott fodt`),
spreadsheets (`xlsx xlsm xltx xltm xlsb xls xlt ods ots fods csv`), presentations
(`pptx pptm potx potm ppsx ppsm ppt pot pps odp otp fodp`), plus the legacy
OpenOffice.org 1.x forms (`sxw sxc sxi`).

**Out of scope.** Draw, Math and Base. Do not add them. Also: writing/export of any
format (X-Ray.Office reads), macro execution (never — X-Ray.Office only reports that macros are
*present*), and editing.

## Absolute rules

1. **Never build the C++ tree.** It takes hours and is never needed. Use an installed
   `soffice` for reference output — see the `libreoffice-reference` skill.

   **And the tree is not the reference binary's source.** `configure.ac`:21 declares
   `27.2.0.0.alpha0+`; the checkout is a single bulk import dated 2026-07-29; the binary at
   `/opt/libreoffice26.2/program/soffice` is **26.2.4.2**. Do not write *"read out of 26.2's
   source"* — write which tree you read, and treat a hunk as *probably* also 26.2's unless it
   matters, in which case say you could not check. Per-file `git log` cannot help: every file
   carries the import date. There is no 26.2 branch or tag in this checkout.

   This is why every arm gets confirmed **twice** — once in source, once against 26.2.4.2's
   own output via `--convert-to fods/fodt/fodp` or a rendered fixture. The second leg is a
   measurement of the actual reference and stands on its own; the first is now known to be a
   different version's explanation of it.
2. **Never execute macros.** Macro-enabled formats are read as data. `CanCarryMacros` on
   `FormatInfo` exists so callers can surface the risk; nothing executes.
3. **Rasterise with SkiaSharp, shape with HarfBuzzSharp.** HarfBuzz is what LibreOffice
   shapes with, which is why it was chosen. Font metrics come from a hand-rolled OpenType
   reader in `XRay.Office.Text` — matching LibreOffice's line heights needs raw `hhea`/`OS/2`
   access and our own precedence rules. Before adding any graphics dependency, read the note
   at the top of `Directory.Packages.props`.

   **Advance widths do agree, and the "~0.1% advance divergence" that stood here for four
   rounds does not exist.** It was an artefact of the instrument, and every figure that was
   ever quoted for it — "ours is exactly `hmtx × size / upem` and the reference's is not",
   "differs per glyph by up to 0.3%", "no quantisation grid fits", "the reference must be
   grid-fitting the outline at LibreOffice's ppem" — came from reading the reference's
   advances out of a LibreOffice PDF's *glyph positioning*, whether through
   `pdftotext -bbox` or from the `TJ` integers. **That channel is quantised to whole
   thousandths of an em, and one thousandth of an em is 0.36% of a Liberation Serif `i` and
   0.17% of a Liberation Mono digit.** The instrument's resolution was several times the
   defect it was used to measure.

   Two facts settle it, and `probes/advance-ppem/` holds both:

   - **LibreOffice shapes unhinted, exactly as we do.**
     `LogicalFontInstance::InitHbFont` (`vcl/source/font/LogicalFontInstance.cxx`:94-103)
     builds its HarfBuzz font with `hb_font_set_scale(font, upem, upem)` and
     `hb_ot_font_set_funcs` — HarfBuzz's own OpenType functions, reading `hmtx`. There is no
     FreeType font-funcs object in the advance path and no hinting anywhere in it.
   - **Its PDF writer truncates every declared width.** `registerGlyph` records
     `XUnits(upem, width)`, and `XUnits` is `(n * 1000) / nUPEM`
     (`vcl/inc/fontsubset.hxx`:29) — integer division, so every declared width is
     `floor(hmtx × 1000 / upem)` by construction and up to a thousandth of an em *short*;
     `drawHorizontalGlyphs` (`vcl/source/pdf/pdfwriter_impl.cxx`:5814) corrects a gap only
     when `trunc(declared − actual·1000/ppem + 0.5)` is non-zero, which a systematic
     sub-unit deficit never makes it. Measured over every glyph of every subset in three
     corpus documents, **every declared width is `floor(hmtx × 1000 / upem)` in both
     binaries, mean deficit 0.48–0.65 thousandths of an em per glyph.** A pen reconstructed
     inside one text object therefore falls behind the pen the layout intended by about half
     a thousandth of an em per glyph and resets at every `Td`. That is the whole of "0.1%,
     and it accumulates *between* the stops".

   Measured through a channel that is **not** quantised — differencing two right-aligned
   lines, so the `Td` the writer states is `margin − width(line)` and every fixed term
   cancels — over **5 faces × 6 units × up to 11 sizes = 314 cases**: ours agrees with
   26.2.4.2 to a **worst case of 0.0077%** and with 24.2.7.2 to **0.0107%**, which is the
   instrument's own floor. The units include `Hamburgefonstiv`, a six-kern-pair phrase, a prose sentence and
   ` o`, so shaping, kerning, ligatures and the space glyph are all inside that agreement.

   **The control is in the suite already.** `TabStopComparisonTests` runs one assertion at one
   tolerance over two documents: `tabbed.docx` passes and `list-label-overrun.docx` fails.
   In the first, every stretch after a tab is its own text object with its own `Td`, so the
   reference *states* each position and all three renderings agree at every word. In the
   second the whole line is one text object and every position after the first has to be
   reconstructed. Same test, same tolerance — what changes is whether the number was stated
   or reconstructed.

   **26.2.4.2 is not "further from the design metric" than 24.2.7.2.** On the failing
   documents the `Td` origins of every portion are *identical* between the two binaries: the
   layout did not move. What moved is the `TJ` arrays — 24.2.7.2 emits an adjustment at a
   handful of positions and 26.2.4.2 at nearly every one, because 26.2.4.2 encodes the
   kerning the layout applied and 24.2.7.2 dropped some of it, which made its lines wider and
   cancelled part of the truncation deficit.

   So there is nothing to reproduce in `XRay.Office.Text`, and **the fidelity failures in this
   family are left failing on purpose**: eight of them compare a position N glyphs deep
   inside one reference text object, where the channel's resolution is `N × 0.5/1000 em ×
   size` — 0.55 pt on `paginated.docx` line 1, against a 0.5 pt tolerance. Only writing our
   own PDF with LibreOffice's truncated integer widths would close them, and that is making
   our output worse to make a test greener. Each of those tests now carries the correction in
   its own remarks.

   **`SheetTextComparisonTests` was never this family and is now closed.** Its failure is a
   Calc *indent*: an OOXML `indent` level is three spaces of the workbook's default font
   (`sc/source/filter/oox/stylesbuffer.cxx`:1263) and one space is
   `xFont->getCharWidth(' ')` (`unitconverter.cxx`:139), which is
   `OutputDevice::GetTextWidth` cast to `sal_Int16` — a **whole number of twips**. It rounds;
   we truncated. Measured over the six default font sizes at which Liberation Sans' 5.5566
   twips per point separate the two rules, 26.2.4.2 rounds at six of six and 24.2.7.2 at four
   of six. `SheetDrawingComparisonTests` is not this family either — its own remark
   classifies it as 26.2.4.2 clamping a full-cell anchor offset, on 34 probe renderings.

   **`SlideChartFaceComparisonTests`' 5.839 pt digit was a separate defect, and it was ours.** A
   chart's text is not laid out by Writer, Calc or Impress: `chart2`'s view builds it as plain text
   shapes on the `VirtualDevice` that `DrawModelWrapper` creates from
   `Application::GetDefaultDevice()` with `MapUnit::Map100thMM`
   (`chart2/source/view/main/DrawModelWrapper.cxx`:88-99), and **that device is 96 dpi**
   (`SvpSalGraphics::GetResolution`, `vcl/headless/svpgdi.cxx`:44). An `OutputDevice` instantiates a
   font at a whole number of device pixels, so a 10 pt label is laid out at **13** rather than 13.34
   and every advance in it is **2.5% narrow**; at 11 pt the device sets 15 for 14.67 and they are
   2.3% *wide*. The scale is `round(size × 96/72) / (size × 96/72)` and it is
   `MetricGrid.Chart.PixelEmScale`, which `SheetBandText.ChartShape` has applied to a workbook's
   charts since round 62. `SlideChart` and `FrameChart` did not, and now do.

   Measured over **twelve sizes × two binaries**, with no free parameter: the drawn advance follows
   `round(px96)/px96` on the sign at 12 of 12 for **both** 24.2.7.2 and 26.2.4.2, magnitude within
   0.003; the same string in an ordinary slide text box on the same slide of the same deck stays
   within 0.7% of the design metric at every one of them; four chart frame widths give identical
   output, so no metafile scale is involved. `probes/chart-text-metafile/`.

   **The same device decides every *vertical* metric, and that half was on the sheets track
   alone.** A font instantiated at a whole number of device pixels answers a rounded ascent, a
   rounded descent and therefore a rounded line height, so a chart's baseline pitch is
   `round(asc/upem × hpx) + round(desc/upem × hpx)` device pixels of 0.75 pt, with `hpx =
   round(size × 96/72)` and **no external leading** — `IsAddExtLeading()` is false in EditEngine
   and a chart's label is an EditEngine text. Round 60 put `SheetBandText` on it; `SlideChart` and
   `FrameChart` are now on it too, through `SlideTextBody.Device` and `ChartFace`'s own grid.
   Measured over **three faces × twelve sizes × two binaries × a deck and a Writer document**:
   144 of 144 baseline-to-baseline distances within **0.019 pt**, against as much as **1.208 pt**
   for exact scaling. The ascent is measured separately, as a value-axis label's offset from its
   own tick read off the PDF's path operators: `ascent − height/2` is right on **72 of 72 with no
   free parameter** against 24.2.7.2, and on 72 of 72 against 26.2.4.2 once one constant is
   allowed. `probes/chart-vertical/`.

   **Two things about the vertical that the earlier rounds' figures did not cover.** (1) *The rule
   is not a property of 26.2.4.2.* Round 60 measured it on one workbook against 26.2.4.2 alone;
   **24.2.7.2 follows the identical vertical rule**, and the two binaries' pitches agree to
   0.002 pt at every one of twelve sizes. (2) *24.2.7.2's whole-pixel snapping is horizontal
   only.* Neither binary snaps a baseline; what separates them vertically is a single constant of
   **one hundredth of a millimetre** — 0.028 pt — in where 26.2.4.2 places a label's block, and
   that is the whole of it.

   **The height and the ascent have to move together, and a single-line label hides it.** A label
   is drawn at `blockCentre − height/2 + ascent`, so an error shared by the two cancels out of it:
   that is why round 60's sheets defect stayed invisible until a label wrapped or was measured for
   a fit. It is also a trap for a test — on Liberation Mono at 10 pt `chart2`'s device and
   Impress's differ by 0.014 pt in that quantity and on Liberation Serif by 0.043, so a deck-level
   assertion has to say which deck it is on and why.

   **Nothing about how much text fits moved with it**, which was the risk worth measuring: on the
   78 chart-bearing slides and words documents, 61 rendered differently and **0** changed a page
   or slide count, the number of text runs drawn, or the number of turned ones — so no label
   wrapped, none was thinned away, and no axis reached for `ChartAxisLabels.Resolve`'s 45 degree
   rotation. 77 of the 78 match 26.2.4.2's page count, before and after.

   **Three things this file used to say about it are wrong and should not be re-derived.**
   (1) *"The seat is in the metafile a chart is drawn into and replayed from"* — there is no
   metafile. `ViewContactOfSdrOle2Obj` takes a chart's content as **primitives** straight from the
   chart's own draw page (`ChartHelper::tryToGetChartContentAsPrimitive2DSequence`), and the
   quantisation happens when `chart2` measures, long before any playback. (2) *"`tdf#168002` and
   `GetSubpixelPositioning` are the leads"* — they explain only the difference **between** the two
   binaries: 24.2.7.2 additionally snaps each glyph position to a whole 96 dpi pixel (its gaps are 7
   or 8 px where 26.2.4.2's are a flat 7.79), and removing that made 26.2.4.2 *better*. (3) *"the
   chart's `Tm` origins move between the two binaries where a Writer document's do not"* — the
   chart's move by 0.17 to 0.31 pt, and a Writer document's move too, on **4 of 39** runs of
   `tabbed.docx`, by one twip each. The magnitudes are the claim.

   **And 24.2.7.2 never "sat on the design metric".** Its mean advance follows the same 96 dpi rule
   at every size. The 6.010 the fidelity test used to read there is 24.2.7.2 right-aligning the value
   axis' labels on their **design** widths while drawing them from the device's narrower array — it
   reserves 18.012 pt for `100` and draws 17.249. 26.2.4.2 uses one width for both, and so do we.

   **And a chart's line height was never "the face's own ascent plus descent plus leading".**
   `FrameChart` and `SlideChart.Measurer.Body` both said so — 1.1499 em for Liberation Sans — and
   both halves are wrong: the leading is not in it, and the metrics go through a device, so it is
   not a fixed fraction of the em at all. 1.1254 em at 10 pt and 1.1596 at 11.

   **And the anisotropic scale a chart is sometimes drawn at is not a mystery: an embedded chart
   is fitted to its own *drawn extent*, not to its page.**
   `ViewContactOfSdrOle2Obj::createPrimitive2DSequenceWithParameters`
   (`svx/source/sdr/contact/viewcontactofsdrole2obj.cxx`:88-116) takes the bounding range of every
   primitive the chart's draw page produced — `aRetval.getB2DRange(...)`,
   `svx/source/svdraw/charthelper.cxx`:96-100 — translates its minimum to the origin, scales it by
   `1/width, 1/height`, and multiplies by the OLE object's own matrix. So whenever a chart's labels
   overflow its page, the whole chart is squeezed until the overflow fits, by **two different
   factors, one per axis**. That is the whole of "*the chart is scaled unequally*" recorded against
   `tdf106217.pptx`, and it is general. Measured on `N2_E_Maestroni_Swarm_COP.pptx` page 7: the
   chart's own background rectangle is drawn at 119.083–719.660 × 92.58–516.98 inside a frame of
   0–720 × 92.57–540.0, which is **0.834135 across and 0.948534 down**, and its leftmost category
   label starts at x = 0.02, or −142.7 pt in the chart's own coordinates. `ChartLayout.Place`
   already composes at `plot.Space` and stretches onto the frame, carrying the residual `sx/sy` on
   `ChartLabel.Stretch`; what it does not yet do is take the *drawn* extent as the rectangle it
   stretches from. `probes/chart-layout/results.md` §0.

   ***Done, and the vertical half of it is confirmed to 0.24%.*** `ChartLayout.DrawnExtent` is that
   rectangle and `Stretch` now subtracts its origin, which may be negative. On the same page 7 this
   tree draws the chart's background at 1.847–720 × 92.572–518.002, which is **0.95084 down**
   against the reference's 0.948534, with no free parameter. **The horizontal half cannot be
   confirmed there and the reason is not the fit**: 26.2.4.2 draws that chart's category labels on
   *one line each* and we wrap them into the band, so ours overflow by 1.86 pt where the
   reference's overflow by 142.7 — while the plot rectangle itself agrees exactly, 127.65 pt in the
   chart's own coordinates on both sides. The seat of the wrap is
   `VCartesianAxis::createTextShapes`:888-905, where the first label after the first that breaks
   **inside a word** sets `m_bLineBreakAllowed = false` and restarts the axis with breaking off;
   `probes/chart-layout` §2 refuted one of that test's two arms (a line *starting* with
   punctuation) and the other — an over-long word — is what this document needs. `ChartAxisLabels.Wraps`
   models it and answers false here.

   ***`Wraps` models half of that test, and the missing half is HYPHENATION.*** *"A line starting
   in the middle of a word"* is not the same thing as *"a word wider than the slot"*, because
   `PropertyMapper::getTextLabelMultiPropertyLists` sets **`ParaIsHyphenation` true** beside
   `TextMaximumFrameWidth`, inside the same `if (nLimitedSpace > 0)` and nowhere else
   (`chart2/source/view/main/PropertyMapper.cxx`:550-557), and `DrawModelWrapper` installs
   `LinguMgr::GetHyphenator()` on the drawing outliner under the comment *"Hyphenation and
   spellchecking"* (`DrawModelWrapper.cxx`:72-86). So EditEngine may put a **hyphenated fragment of
   the next word** on the current line, and `lcl_hasWordBreak` reports that as the mid-word break —
   turning line breaking off and, if the one-line labels then collide, turning the axis 45°.
   Measured on 26.2.4.2 with one-attribute variants of `038_Competitive_Advantage_Card.pptx`, whose
   five category labels the reference turns and this tree wraps: **`Cost Efficiency` turns and
   `Efficiency Cost` wraps** — the same two words, order swapped — and `Cost Efficiency` (word
   43.638 pt) turns while `Cost Thoughts` (42.610), `Cost Strengths` (43.742), `Cost Stretched`
   (44.401) and `Cost Scratched` (45.181) all wrap, so **no width rule can produce the ordering.**
   **`TextBreak` is not what a brief will tell you it is**: `axisconverter.cxx`:356-365 sets it
   **true** for every non-date category axis at zero rotation, an out-of-range `rot` reads as zero,
   and both this tree and the reference have it on here. `probes/chart-axisrot-r91/results.md`;
   the fix needs a hyphenator, the limit that applies to a line *inside* a wrapped label is
   measurably below 0.95 of the pitch and is not yet characterised, and **no corpus document states
   `c:layoutTarget val="inner"` at all**, which retires that lead for good.

   ***Done, and it ships a dictionary — but it does NOT close `038`'s gate row, and r105's
   arithmetic saying it would is a misreading.*** `hyph_en_US.dic` (106,414 bytes, BSD-style,
   *"unlimited copying, redistribution and modification with this copyright and license
   information"*) and `README_hyph_en_US.txt` are vendored into
   `XRay.Office.Core/Globalization/Hyphenation` and copied beside the assembly like the bundled
   faces; **`hyph_fr.dic` (LGPL-2.1+) and `hyph_es.dic` (GPL-3.0+/LGPL-3.0+/MPL-1.1+) are
   deliberately not committed** and are supplied through `XRAY_OFFICE_HYPHEN_DICTS` (a path list,
   searched one level deep so a LibreOffice `share/extensions` resolves `dict-fr/hyph_fr.dic`) or
   `Hyphenators.Register(language, stream)`; `=0` turns the lot off, which is the state a
   deployment without the data is in. `IHyphenator` moved to `XRay.Office.Core.Globalization`
   because `ChartAxisLabels` is in Core and Core may not reference `XRay.Office.Text`.

   **Two rules to keep.** The leading limit EditEngine imposes is a **character count, not a
   width**: `nMaxBreakPos - nWordStart - 1`, whose `+1` is commented *"Before the dickey letter"*
   (`editeng/source/editeng/impedit3.cxx`:2143-2160), so one character's room is *reserved* for
   the hyphen rather than the hyphen being measured, and measuring it admits breaks the reference
   refuses. And **a turned label the reference outlines still costs us its characters**: we turn
   `038`'s axis now and our column 9 is **1585 before and after**, because 26.2.4.2 reads 1449 by
   drawing those labels as filled outlines. r105's *"the reference with its hyphenator off is
   1585 too"* is an identity between the reference's own two states and says nothing about ours.
   Closing that row is the shear rule's job, and outlining glyphs to green a text gate is not a
   fix.

   Reach, measured rather than censused: **3 renderings of 568 scored move, all chart-bearing,
   all three better against 26.2.4.2 and none worse** (summed |ink|% 57.00 → 54.72 over r105's
   eight candidates, one MAJOR page cleared), **0 gate verdicts**, **0 of 399 non-chart
   documents**. Confirmed twice — the four hyphens 26.2.4.2 itself draws on corpus charts are
   reproduced 4 of 4, and 22 authored one-word fixtures rendered with each renderer's dictionary
   both on and off agree **19 of 22 against a base rate of 13**. Residual: `Service`, one word of
   22, which this tree hyphenates and 26.2.4.2 does not; the leading room is refuted as the cause
   and nothing replaces it. `probes/hyphen-r106/results.md`.

   ***The ODF twin of that document closed on a different attribute, and the `.pptx` is untouched.***
   `N2_E_Maestroni_Swarm_COP.odp` went from 340 alphanumeric characters clear of 26.2.4.2 to **84**
   when `OdfChartPlot` was taught to read **`text:line-break`**, which it had been passing as false
   for every axis: `canAutoAdjustLabelPlacement` refuses while line breaking is on
   (`VCartesianAxis.cxx`:544-545), so an ODF axis whose labels collided was turning them 45° where
   the reference wraps them. That says nothing about the OOXML side — `.pptx` already reads
   `a:bodyPr` and the `.pptx` row does not move — but it does say that **the wrap question and the
   fit question are separable, and the ODF half of it was a reader defect rather than a layout
   one.** `probes/odp-chart-r72/results.md` §3.3.

   **Three things had to be true before the fit could be, and two of them were defects of their
   own.** (1) *A bar is clipped to its value axis' range* — `clipYRange`
   (`PlottingPositionHelper.hxx`:401-415), called by `BarChart::createShapes` (`:789`) before any
   geometry and `continue`d on, so a rejected point gets no data label either. A Gantt is a stack of
   an invisible "start" series over an explicit `c:min`, so page 7's 55 bars were each drawn from
   serial zero — **192 386 points** left of the plot, read out of the PDF. (2) *A label with no ink
   has no rectangle*: `DocRect.Empty` is the point (0, 0), and one empty value-axis label fitted
   `048_Expense_trends_budget` at `sy = 0.6561`. (3) *A series mark contributes only what falls
   inside the plot*, because every plotter clips its polygon first
   (`Clipping::clipPolygonAtRectangle`, `AreaChart.cxx`:318 and eight more). **We do not clip a
   polyline yet** — on `171128IPAP.pptx` one runs 932 pt left of a 576 pt chart page — so
   `DrawnExtent` intersects a shape's extent with the plot rather than trusting it. Clipping the
   geometry itself is left.

   ***Done for a line and a scatter, and the reference clips the geometry rather than painting
   under a clip path.*** `ChartClipping` is Liang–Barsky per segment (`Clipping.cxx`:47-128) with
   chart2's own piece joining (`:340-421`), against the plot rectangle — which is what
   `getScaledLogicClipDoubleRect` (`PlottingPositionHelper.cxx`:295-311) maps onto. Two details
   decide it. **A line that leaves the plot and comes back is two strokes with no chord across the
   gap**, because `bSplitPiecesToDifferentPolygons` defaults to *true* (`Clipping.hxx`:51) and only
   the two *filled*-polygon callers pass false. And **the bounding-box short circuit is the
   reference's own** (`Clipping.cxx`:350-365), which is why this is free: a series that fits its
   plot is returned untouched, so **174 of the 176 chart documents and 180 of the 182 chart-bearing
   converted-ODF files do not change by a byte**, and the two movers are the same documents in both
   columns. Beside it, `AreaChart::createShapes` puts a point in the polygon and only *then* asks
   `isLogicVisible` (`:715`), and `if( !bIsVisible ) continue;` (`:760-761`) sits above the symbol,
   the error bars **and** the data label — so an out-of-range point holds its place in the line and
   gets no mark of its own. On `171128IPAP.pptx` slide 38 the three series now run 119.55 … 615.56
   against 26.2.4.2's 119.54 … 615.49, where they ran −866.50 … 758.03 on a 720 pt page; all four
   renderings improve and no page or alphanumeric count moves. **The area, net/radar and regression
   clips are still open.** `probes/chart-resid-r75/results.md` §1.

   ***The splined line that stood beside them is closed and this paragraph said otherwise for
   eleven rounds.*** *"That chart's three series state `c:smooth val="1"` and 26.2.4.2 draws 801
   segments through 132 points where we draw 131"* was true when it was written and round 102
   closed it: `ChartSpline` is the natural cubic spline over the point index at
   `CurveResolution` 20, reproducing 26.2.4.2's own flattened polyline to **0.0495 pt**, which is
   1.75 hundredths of a millimetre. Its reach is **12 documents by markup and 2 in drawn
   geometry** — nine of the eleven that state `c:smooth` carry arithmetic-progression data, whose
   spline *is* the straight line, and one suppresses its line altogether.
   `probes/chart-smooth-r102`. A round briefed off the old sentence would re-derive a feature the
   tree already has; `171128IPAP`'s residual is real and is not this.

   **And a chart's range excludes the cells of hidden rows and columns, which is not a chart rule
   the record had.** `ScChart2DataSequence::BuildDataCache` asks `ColHidden` and `RowHidden` per
   cell and `continue`s past a hidden one — dropped from the sequence, not blanked
   (`sc/source/ui/unoobj/chart2uno.cxx`:2636-2646) — unless the diagram says `IncludeHiddenCells`,
   which the OOXML importer sets to `!c:plotVisOnly`
   (`oox/source/drawingml/chart/chartspaceconverter.cxx`:264). **`plotVisOnly`'s own default is
   `!bMSO2007Document`** (`chartspacefragment.cxx`:130-131, `chartspacemodel.cxx`:30), so it is the
   Office generation and not a constant. `TODO.batches.md`'s *"`plotVisOnly` is refuted"* is a claim
   about `029_Annual_budget`, whose sheet hides nothing, and does not generalise: the attribute is
   exactly what decides `053_Personal_asset_inventory`, `026_Monthly_cash_flow_statement` and
   `027_Simple_personal_cash_flow_statement`. **Reach is 4 of 947** and in every one of the four
   *every* cell of the sequence is hidden; the fourth, `055_Project_timeline_with_milestones`,
   states `val="0"` and is the control that must not move. **And `053`'s missing seventh category
   was never the `Assets` table-name collision** the previous round diagnosed — renaming that table
   and deleting it outright both leave 26.2.4.2's rendering unchanged, as does removing the
   `pivotTable` part; unhiding columns H and I is what makes it draw seven. Two neighbouring rules
   in the same loop answer *nothing survived* differently, and that is measured rather than derived:
   a range wholly inside a totals row resolves to an **empty** sequence and the chart draws nothing,
   a range wholly hidden resolves to **null** and the cached points stand.
   `probes/chart-resid-r75/results.md` §2.

   **The wrap-restart defect does not exist and should not be sent after again.** *"An axis whose
   labels wrap does not restart the wrap when the layout is retried, so the wrapped width from an
   earlier attempt survives into the final one"* is false at both seats. `nLimitedSpaceForText`
   comes from `nScreenDistanceBetweenTicks` (`VCartesianAxis.cxx`:749), which `createLabels`
   computes **once** at `:1733` and passes unchanged into every attempt of
   `while (!createTextShapes(…)) {}` (`:1753-1755`) — **the reference deliberately does not widen
   the wrap when the rhythm rises** — and the line-break restart drops every shape
   (`removeTextShapesFromTicks()`, `:903`) before turning breaking off. `ChartAxisLabels.Resolve`
   matches it: `spacing` is loop-invariant and `Wrap` is recomputed from the *original* strings
   every attempt. What the same loop *does* carry forward, on purpose, is `m_nRhythm`,
   `m_bLineBreakAllowed`, `m_fRotationAngleDegree` and `m_eStaggering`, because the same
   `AxisLabelProperties` object is passed each time. `probes/chart-resid-r75/results.md` §3.

   **Reach and cost, measured rather than censused**: of the 176 chart-bearing documents, **24
   renderings move** and the other 152 do not change by a byte; 13 improve against 26.2.4.2, 8
   worsen, 3 are level, and the sum of their means goes 157.46 → 154.45. **No gate verdict moves in
   either direction**, checked by scoring each mover with a binary built at the round's base as
   well. The eight that worsen are one class and worth knowing before the next round: **the fit
   makes our own label-arrangement errors visible as a global squeeze instead of as local
   overflow** — where our labels leave the chart's page and the reference's do not, we shrink and it
   does not. `probes/chart-fit/results.md`.

   **The category axis stands where it crosses, and that was ours.** `c:catAx/c:crosses
   val="autoZero"` is *value zero on the crossing axis* — `m_pfMainLinePositionAtOtherAxis = 0.0`,
   `VAxisProperties.cxx`:224-225 — clamped into the value range by `get2DAxisMainLine`
   (`:1253-1256`), and under the default `nextTo` the labels take that same line
   (`getLabelLineIntersectionValue`, `:1103-1113`) and therefore take **no band** off the plot. On
   `Demick_JetBlue.pptx` page 5 26.2.4.2 draws the plot to y = 401.56 with its axis at 374.83, its
   own `$-` gridline; we drew the plot to 376.18 with the axis on that edge and now draw 405.62 and
   378.40. Reach 6 documents. The clamp is what keeps it there: an all-positive chart is unmoved.

   **`055_Project_timeline`'s date-axis maximum is `TODAY()` and not a scale question at all.** Its
   DATE column is twelve volatile `DATE(YEAR(TODAY()),m,d)`; substituting `2023` makes 26.2.4.2 draw
   this tree's exact axis and `2026` reproduces the reference. The stated `c:majorUnit val="10"` is
   honoured throughout and the apparent 30-day step is the label *rhythm*, which `createTextShapes`
   raises until the labels stop overlapping — the same document at 6 pt draws 57 labels 20 days
   apart and at 24 pt draws 19 at 60. It is the volatile-formula class this file already records,
   and the same dates are wrong in the sheet's own cells. `probes/chart-datemax/results.md`.

   **And an instrument warning that cost this round a false finding: PyMuPDF reports a rotated
   span's *axis-aligned* box.** A 45° label comes back as a square, which reads exactly like an
   unrotated label far too wide for its slot — 26 of them at 32.62 pt on a 19.27 pt pitch, which
   looked like a whole missing arrangement and was not. The discriminator is the line's own `dir`
   vector, `(0.7071, −0.7071)` here, and instrumenting the layout confirmed it had been returning
   `rot = 0.785` all along.

   ***And the `dir` vector answers a second question the same instrument cannot: whether the
   reference drew that label as text at all.*** A turned chart label the reference *outlines* is
   absent from `get_text` entirely, so a `dir` census over both sides reads as *we invented a whole
   arrangement*. It is the opposite: our arrangement agrees and only the representation differs.
   **The rule is a shear, not a rotation** — a run is outlined when it is both turned off a right
   angle and inside a chart the fit had to squeeze anisotropically, because
   `VclProcessor2D::RenderTextSimpleOrDecoratedPortionPrimitive2D`
   (`drawinglayer/source/processor2d/vclprocessor2d.cxx`:126-141) accepts a text primitive only
   while `abs(fontScaling.getY() * fShearX) < 1` and decomposes everything else to filled polygons.
   Established on thirteen one-attribute variants of one chart in
   `probes/odp-chart-r72/variants.py`; the discriminator is a 45° label whose text is *short enough
   to fit*, which stays text. Count the reference's glyph-sized filled paths before calling a
   turned label missing — `probes/odp-chart-r72/classify.py` does it — and see
   `TODO.raster-ceiling.md`, whose *the reference outlines its glyphs* section asked for exactly
   this rule and did not have it.

   **`VDiagram::adjustInnerSize` is not reached by a chart stating `c:layoutTarget val="inner"`,
   and a brief has already sent a round after it.** That target sets `PosSizeExcludeAxes`
   (`DiagramWrapper.cxx`:816-823) and therefore `CreateShapeParam2D::mbUseFixedInnerSize`
   (`ChartView.cxx`:946-980), and all four `adjustInnerSize` calls in
   `impl_createDiagramAndContent` are guarded by `!rParam.mbUseFixedInnerSize` (`:559`, `:594`,
   `:619`, `:690`), as is `reduceToMinimumSize()`. The one correction such a rectangle gets is in
   the importer: `DiagramHelper::setDiagramPositioning`
   (`chart2/source/tools/DiagramHelper.cxx`:434-476) clamps the four fractions to `[0, 1]` and then
   moves the **position** — `aNewPos.Primary = 1.0 - aNewSize.Primary` — rather than shrinking the
   size.

   **Reach: 176 of 947 corpus documents carry a chart** — sheets 99, slides 67, words 10 — and 131
   of them are at 1% or worse, 111 at 2% or worse, at the sizes their charts declare. The sheets
   documents were already right; the slides and words ones are what this moved.

   *The figure stood at "168 — sheets 90, slides 68, words 10" and was short by eight, because it
   was counted by walking zip parts for a `c:chartSpace`.* **A `.xls` chart lives in a BIFF
   substream, not a zip part**, and counting a `BOF` of substream type `0x0020` in *any* OLE2
   stream — the ObjectPool included — finds **seven more `.xls` charts** that no zip walk can see.
   No `.ppt` or `.doc` in the corpus embeds one. The OOXML part count is **309**, against the 281,
   307 and 61 quoted in various places. **Census a chart both ways or the legacy binaries vanish
   from your reach figure**, which is exactly the track a chart round is least likely to have
   tested.

   **A single wrapped line short is amplified by section breaks into whole pages, and that is
   why some documents are wildly out.** Worked through on `AWR OPS-AOC 044` (metrics-001, then
   ours 12 pages against 15). *This used to be filed as the advance divergence arriving at
   corpus scale, and that attribution is withdrawn with the rest of it.*

   **`AWR OPS-AOC 044` no longer shows it, and the "12 against 15" has been quoted in briefs
   after it stopped being true.** At `260611dae` it is **15 pages against the reference's 15**,
   before and after the fonts round that re-measured it (`probes/fonts-r65`). The mechanism
   below — a short wrap amplified by a `nextPage` break — is real and general and is kept for
   that reason; the document that demonstrated it is not a witness for it any more, so
   re-measure before working from it.

   **What that document still shows is a `w:rFonts` family-code question, not a fallback one.**
   It draws 103 `U+2610` in runs naming `MS Gothic`, which its own font table files `modern` —
   a code `FontTable::lcl_sprm` maps to nothing
   (`sw/source/writerfilter/dmapper/FontTable.cxx`:127-141, only `roman` and `swiss` are
   mapped), so no `PROP_CHAR_FONT_FAMILY` is inserted and the class is whatever an ancestor set.
   Its face-set distance from 26.2.4.2 is **2 both before and after** the fonts round, and a
   probe reproducing the shape (`probes/fonts-r65/gen-awr.py`) answers FreeSerif on 26.2 exactly
   as the tree does — so the disagreement is about which layer supplies the class in the real
   file, and that is where whoever takes it next should start.

   A narrow table cell whose text wraps one line short makes its row shorter; a
   shorter row lets one extra row onto the page; and the document's **ten `nextPage` section
   breaks** each convert that one-row overshoot into a full blank page, because the section ends
   wherever the overshoot has left it. Measured, full-width rules per page: **page 1 ours 11 to
   the reference's 10, page 2 ours 34 to 33, page 3 ours 29 to 27.** The reference's page 4 holds
   its header, the single row `ACAS II System (with Version 7.1 or later)`, and nothing else.

   Two suspects were refuted on the way, both by probe, and neither should be re-derived:

   - **`w:trHeight` is exact.** Twenty rows at each of 324, 432 and 576 twips: total table height
     agrees with the reference to **0 twips** at all three. `atLeast`, an absent `w:hRule` and
     `exact` all behave. Only *auto* rows (no `w:trHeight` at all) differ, and AWR has none —
     all 141 of its rows declare one.
   - **A `nextPage` section break after a table is honoured**, both as a bare `w:sectPr` alone in
     a `w:pPr` and in AWR's actual shape, where `pStyle`, `tabs`, `spacing`, `ind` and a `w:rPr`
     precede it. 3 pages of 3 in both.

   The probes cost twenty minutes and the first cut of them was wrong in an instructive way: the
   CLI rejects `-o` (it is `--outdir`), so "our" PDF was never written, and a `glob` picked up the
   *reference's* raster instead. That reported the two sides as pixel-identical — a clean,
   confident, entirely fabricated match. **Assert your instrument produced output before
   comparing it**; the guard is one line and it is the difference between a refutation and a
   fiction.
4. **Detect formats by content, never by extension.** Mislabelled files are common, and
   some distinctions (DOCX vs DOCM, which application owns an OLE2 file) cannot be made
   from a name at all. The extension is a tie-breaker hint only.
5. **Be lenient when reading.** Real files violate their own specifications constantly.
   Repair what you can, skip what you cannot, record it as a `Diagnostic`. Reserve
   exceptions for genuinely unreadable input.
6. **Zero build warnings.** `TreatWarningsAsErrors` is on solution-wide. Keep it that way.

## Layout

```
dotnet/
  Directory.Build.props        shared MSBuild settings; read the licensing note
  Directory.Packages.props     central package versions
  XRay.Office.slnx               solution (the newer XML format; dotnet 10 default)
  research/                    in-depth notes on the LibreOffice implementation
  src/                         the libraries
  tools/XRay.Office.Cli          the `xray-office` command-line tool
  tests/                       unit tests, the test kit, and the fidelity harness
```

### Dependency layering

Arrows point at dependencies. Nothing may point back up.

```
                       XRay.Office.Core          (zero external dependencies)
                            |
      +---------------+-----+------+---------------+-------------+
      |               |            |               |             |
 Containers        Text          Vector        Rendering       Markup
 (OLE2/OPC/ODF)  (fonts,        (EMF/WMF/SVG)  (Skia, PDF,   (XHTML and
                  shaping,                      SVG)          Markdown out)
                  layout)
      |               |            |
      +-------+-------+------------+
              |
    +---------+----------+-------------+
    |                    |             |
  Ooxml            OpenDocument     MsBinary      (shared per-family infrastructure)
    |                    |             |
    +---------+----------+-------------+
              |
   +----------+-----------+--------------+
   |                      |              |
 WordProcessing      Spreadsheets    Presentations
   |                      |              |
   +----------+-----------+--------------+
              |
          XRay.Office          (facade: sniff and dispatch)
              |
        XRay.Office.Cli
```

**`XRay.Office.Markup` serves all three families, so it cannot live in any of them.** It projects
the shared `ContentNode` tree onto semantic XHTML and then onto Markdown, needs nothing but
`XRay.Office.Core`, and sits beside the other Core-only libraries rather than inside Core, which
holds the abstractions everything agrees on rather than projections of them.

**`XRay.Office.Core` has no external dependencies and must stay that way.** It holds the
abstractions everything else agrees on: units, geometry, colour, the format catalogue, the
document model, and the drawing IR. A dependency added here is inherited by every
consumer.

`Core/Charts` is the test of that rule and shows where the line falls. A chart's *model* and its
*layout* — `ChartPlot`, `ChartScale`, `ChartLayout` — are geometry over the abstractions Core
already holds, so they belong here; the readers that turn a `c:chartSpace` or a `chart:chart` into
that model parse XML and stay in `XRay.Office.Ooxml` and `XRay.Office.OpenDocument`. Putting the model
one layer up instead is what forced the ODF reader into `XRay.Office.Presentations`, where a
spreadsheet could not reach it.

**SmartArt is the third instance, and the first that moved *sideways* rather than down.** Ten
files implementing DrawingML diagrams — the parts, the baked `dsp:spTree`, and a layout-atom
evaluator — sat in `XRay.Office.Presentations` and were unreachable from a word-processing document,
which drew an empty frame where the reference drew the diagram. Nine of the ten imported nothing
above `XRay.Office.Ooxml`; the tenth needed two lookups from the package (*resolve a relationship id
stated on a part*, *load a part by name*) and those became two delegates. They belong in
`XRay.Office.Ooxml/DrawingML` and **not** in Core, by the second half of the same test: they parse
markup and emit an element tree, so they are readers, and readers of OOXML that serve more than one
family live one layer above Core rather than in it. Reach: 18 corpus documents carry a diagram —
slides 15, words 3, sheets **0** — and all 38 of their data parts have a usable baked drawing, so
the evaluator is not what the corpus needs. See `probes/words-diagram-01/results.md`.

`Core/Numbers` came down for the same reason and by the same test, and it is worth stating as a
rule rather than as a second exception. **The question is not "who uses it" but "what does it
depend on".** The number-format engine — parsing `#,##0.00` and rendering a double through it —
began in `XRay.Office.Spreadsheets` because a cell is what wanted it, and a chart's axis composed in
`Core/Charts` then could not reach it; every tick was written in its shortest round-trip form, which
is right for a whole-number scale and wrong for every currency, percentage and date axis. The move
was safe because the engine is pure computation over a string: its five files import
`System.Globalization` and `System.Text` and nothing else, so Core's zero-dependency rule is intact.
Read it as: **a thing belongs in Core when it depends on nothing above Core, whatever it was written
for.** What did *not* move is the reading — `XlsxStyles`, `OdsCellFormats` and
`OdfNumberFormat` parse markup and stay in their own libraries, the last of them compiling an ODF
`number:*-style` element tree into a format code exactly as `xmloff` does before handing it to one
formatter.

## Key design decisions, and why

**All lengths are EMUs, in a `Length` struct.** 914400 per inch divides evenly by twips
(the DOC/DOCX/RTF unit), 1/100 mm (the ODF and draw-layer unit), and points. Storing a
single exact integer avoids the rounding drift that accumulates when converting through
`double` at every boundary.

**Extraction and rendering are separate paths.** `IDocument` gives you content;
`IPaginatedDocument.Layout()` is a distinct, deferred step. Extraction is the common case
and must not pay for fonts, layout or a rasteriser — it costs a small fraction of
rendering.

**So making something *draw* does not make it *extract*, and the two want different answers.**
Round 66 taught a DOCX to draw a SmartArt diagram and round 67 found `xray-office extract` still
blind to the same five nodes: the drawing path reads the baked `dsp:spTree` through a theme and
a frame extent, and extraction may pay for none of that. It also wants the *data model* instead —
the baked tree is what the author sees, so it repeats a node's text wherever the layout drew it,
while the model is what the author typed, once each. An index wants the second. When a round
closes a rendering gap, check the other path before calling the feature done.

**One drawing IR, `IDrawingSink`.** Modelled on LibreOffice's `GDIMetaFile`/`MetaAction`
display list and its `drawinglayer` primitives, because those are the two chokepoints all
LibreOffice output passes through — so anything a supported document can express fits
through them. Coordinates stay resolution-independent; text stays glyph runs rather than
outlines so PDF output can be real searchable text.

**One content tree for all three families.** Callers indexing a mixed corpus want text,
tables and structure without branching on whether a file was a deck or a spreadsheet.

**Shared infrastructure is factored by what the formats actually share**, not by
tidiness: Escher/MS-ODRAW is one library because DOC, XLS and PPT all delegate their
drawings to it, so implementing it once buys shapes in all three.

**Every long operation can be cancelled, and the token travels ambiently.** A public entry point
that does real work has a token overload (`OfficeDocument.Open/Extract/OpenAsync(…, CancellationToken)`,
`IPaginatedDocument.Layout(options, token)`, `IDocumentReader.Read(source, token)`,
`PdfRenderer.Render(…, token)`, `RasterRenderer.Render/Rasterise(…, token)`,
`SheetHtmlWriter.Write(…, token)`, `MarkdownWriter.ToMarkdown(…, token)`). They do not thread the
token through the hundreds of methods beneath them: they open a per-thread scope,
`XRay.Office.Core.Threading.OfficeOperation`, and the loops that bound latency call
`OfficeOperation.Poll()` (every 32nd call checks) or `ThrowIfCancelled()`. Outside a scope both are a
thread-static read and a null test. Where the checks sit, so that a new reader or layout loop can follow:

- **Containers**: `ZipPart.Open` and `CompoundFile` part open throw, and a zip part comes back wrapped in
  `PollingStream`, so tokenising or decompressing a large part is cancellable without a checkpoint in each
  parser. Sector-chain reads poll.
- **Parsers**: per paragraph, row, cell, record or token (docx/odt body walks, xlsx rows and cells, xlsb and
  BIFF records, RTF tokens, DOC characters, CSV every 1024 characters, WMF/EMF/EMF+ records).
- **Layout**: `Paginator` per block, per page and per frame pass, `TableLayouter` per table, `LineFiller` per
  line, every shaper call, sheet pagination (the zoom bisection included), slides per shape.
- **Drawing**: `PageDrawing` per line, the two sinks per glyph run and fill, `RasterImageDecoder` before a decode.

Two rules matter more than where the checks are. **Cancellation is never swallowed**: a `catch` that turns a
failure into a diagnostic must not catch `OperationCanceledException` (the Vector decoders' `catch (Exception)
when (…)` filters name it explicitly), or the caller gets a document with a quiet hole in it instead of a stop.
**The timeout is a separate type**: `LayoutOptions.Timeout` arms a linked token for `Layout` and an expiry
throws `LayoutTimeoutException : OperationCanceledException`, so `catch (OperationCanceledException)` handles
both and a caller can still tell "I cancelled" from "it took too long". `ACancellationDuringTheReadIsNever…`
in `CancellationTests` cancels from inside the first read of each format's bytes; add a fixture there when a
format gets a reader.

`LayoutOptions.MaxImagePixels` (+ `OversizedImages`: skip, or decode at the codec's reduced scale) rides on the
pages, not on the renderer: the layout captures it from the ambient scope into each page (`IPageRenderLimits`),
and both renderers apply the stricter of that and their own option. The size comes from the header, so an
oversized picture costs a header read.

**A PDF says what the document says, and it is written as it is produced.** `PdfDocumentWriter` writes each
object to the destination the moment it has a body (page content as each page ends; the page tree, fonts
and resources last), so memory is one page plus the offset table and the stream need not be seekable. The
file carries the information dictionary and an XMP packet from `DocumentMetadata`
(`PdfRenderer.Render(document, pages, stream)`, or `PdfRenderOptions.Metadata`), `/Lang`, link annotations,
named destinations (`/Names /Dests`), an outline and `/PageLabels`. All of it is declared through four default
methods on `IDrawingSink` (`AddLink`, `AddInternalLink`, `AddDestination`, `AddOutlineEntry`); a sink that
does not care ignores them, which is why the raster sink and every test sink needed no change. The layout
supplies them: `PageRun.Link` (an address, or `#name` for a bookmark), `PageParagraph.Anchors` (bookmarks) and
`PageParagraph.OutlineLevel` for DOCX, ODT, RTF and DOC, sheet names for a workbook, and a title taken from
the extracted content for a deck. Not declared: hyperlinks in spreadsheet cells and on slide shapes, because
those readers keep a range or nothing and not a target. **A link in a header, a footer or a note is resolved
only when it names a bookmark**: relationship ids are scoped to their part and the layout source does not
carry a part's table, and a wrong address is worse than none. `DocumentLinkTests` validates the file against
pypdf when it is installed (and skips when it is not); `FormatLinkTests` checks every format against what its
own extraction says the document holds.

## Fidelity: the thing that will bite you

### Look at the rendering. Do not chase it through metrics alone.

**This is the standing instruction and it comes before the rest of this section.** The gate is
page count, extractable text within max(2%, floor), and unembedded fonts. **It is blind to most real
defects** — a whole track can be 163 of 163 page-exact while the pages are visibly wrong.

*The band was described here as "2%+3" for several rounds and that is not the rule.
`batch-check.sh` fails a document when `d > b*0.02 && d > floor` — an AND, so it is
**max(2%, floor)**, not their sum. It matters at the boundary, and one regression found on
2026-08-14 sat at exactly 27.*

***And the input is no longer words.*** As of **2026-09-05** `batch-check.sh`:279 compares
**alphanumeric characters** with a floor of **15**, not tokens with a floor of 3 — the same band
shape over a different count, transferred rather than loosened, and replayed over 9552 stored rows
without changing one verdict. Two consequences a round pays for. The line number and the "max(2%,
3)" above were both stale within a fortnight of being written down, so **read the block rather than
quoting this**. And a scorer written from the older rule silently disagrees with the scoreboard:
round 67's replay of a banked gate reported eight verdict movements of which **five were the rule
and not the tree**. The discriminator is in the data — a banked `parity.tsv` scored the new way
carries a ninth column, `glyphs`, as `ours/ref`.

```bash
export XRAY_OFFICE_CLI=<the tree you mean to measure>/dotnet/tools/…/XRay.Office.Cli
python3 .claude/skills/render-comparison/scripts/look.py "<doc>__pptx" --worst   # two PNGs
.claude/skills/page-vision/scripts/pair.sh "<doc>__pptx" --worst --outdir /abs/pairs  # one labelled image
```

It renders the most divergent page both ways. **Open them and read them.**

**Better: do not read them yourself — hand the pair to a fresh subagent.** You cannot un-see
a page, so your second look at one is recall rather than observation, and it will agree with
whatever you already believed. A reviewer that has never seen the document and is forbidden
to grep the repo is the only reader whose agreement is evidence. The `page-vision` skill has
the brief to give it, the pixel-budget arithmetic that decides your dpi, and when to crop.

**That reader IS available, and the paragraph that stood here for four rounds was looking at the
wrong tool.** Measured 2026-09-27: the in-process **`Agent`** tool with
`subagent_type: "general-purpose"` spawns a subagent in *this* container, which opens
`/home/user/...`, reads the composed PNG, and hands its report back. Six readings were delegated
that way in one session and all six came back usable — four independently named a one-page offset
that turned out to be the round's whole finding, and one caught a dropped tab leader the numbers
could not show.

What is not available is the **sibling session**, `mcp__Claude_Code_Remote__create_session`: it
runs in its own container, cannot see the PNG, and has no channel back. Four rounds checked for
*that*, and for a tool called `Task`, found neither, and wrote off the whole practice. **Delegate
the reading.** Only if `Agent` is absent in some future container does the old advice apply — say
so explicitly, treat your own readings as contaminated, and corroborate anything you lean on with
arithmetic that does not depend on the reading.

Three things this changes about how a round is run:

1. **Look before you theorise, and look at documents that PASS.** The failing set is picked
   over. Rank the *passing* documents by `|ink|%` and open the worst — the first three tried
   that way produced three findings, two of them previously unrecorded (a missing custom bullet,
   and a hanging indent we invent where the reference has none). Done a fourth time in round 67 over
   the whole words track against 26.2.4.2 — 307 of the 312 gate-passing documents scoreable — it
   produced three more, and the first of them was **an entire unimplemented feature**: `w:pgBorders`
   appeared nowhere in `dotnet/src`, **7 of the 272 corpus DOCX declare a real one**, two of those
   seven are in the ink ranking's top ten, and every one of the seven passes the gate — because a
   border adds no words and no pages, so no gate column can see one. Closed in the same round, and
   the gate is `MATCH 314` before and after it, which is the whole argument for ranking on ink. The other
   two are a cover page's anchored artwork drawn one page late on `PES-Technical-Report-Template`
   (measured by sign: page 1 reads *ink missing from ours*, page 2 reads *ink we draw*, in the same
   regions) and a per-page pagination difference on `hdss-bulletin-issue-285` that is not a
   one-way drift. See `probes/words-ink-r67/`.

   **Both of those are now settled and neither was any of the candidates named with it.** The cover
   art was **the anchor character**: the DOCX walk emitted one `U+0001` per `w:drawing`, floating or
   inline, and Writer inserts that character for `FLY_AS_CHAR` alone (`SwFormatFlyCnt` and
   `GetCharOfTextAttr`, `sw/source/core/txtnode/thints.cxx`:3633-3652, put in by
   `SwDoc::SetFlyFrameAnchor`, `docfly.cxx`:337-348). A control character is zero-width, so what the
   four `wp:anchor` runs before that document's cover picture cost was a **break opportunity** — an
   inline object widens every prefix past its boundary, so a picture wider than the measure is placed
   anyway on a line it starts and pushed onto the next line when anything precedes it. `|ink|%`
   against 26.2.4.2 **35.91 → 2.37**, gate `MATCH 314` unchanged, 12 of 338 renderings moved and
   their total ink 63.31 → 29.11. The two candidates carried with it are both refuted: the frame at
   `posOffset` 733.5 pt is 40.5 pt tall on a 792 pt page and is `wp:wrapNone`, and our
   first-on-the-page overflow rule already matches 26.2.4.2 on four authored cases. `w:titlePg` is
   real but is a *different* defect — see the next paragraph.

   `hdss-bulletin-issue-285` is **furniture height, with the sign the other way**: we draw
   `header2.xml` on nine of ten pages and the reference draws it on none, so our body starts 43.54 pt
   lower from page 2 on. The section that names that header is a `continuous` break, and changing
   only `<w:type w:val="continuous"/>` to a page break makes the reference draw it on nine pages
   exactly as we do. **Diagnosed and left**; reach 16 of 272 DOCX.
   `probes/words-firstpage-r70/` and `probes/words-continuous-header-r70/`.

   Two more rules fell out of those two documents, both measured and one left:

   - **A content-anchored frame is captured on its page and a page-anchored one is not.**
     `SwAnchoredObjectPosition::ImplAdjustVertRelPos` (`anchoredobjectposition.cxx`:504-667) pulls an
     object back inside the page frame — bottom corrected first, then top — and
     `SwToLayoutAnchoredObjectPosition` never calls it. **Which formats do it is the DOC/DOCX
     distinction**: `sw/source/writerfilter/filter/WriterFilter.cxx`:332 sets
     `DoNotCaptureDrawObjsOnPage` for every writerfilter import, DOCX *and* RTF, and the WW8 and ODF
     filters do not. Clamping every format instead moves 30 renderings against 3 and takes several
     DOCX shape templates away from the reference. `probes/words-apo-capture-r70/`.
     ***And RTF is captured anyway, which the flag does not decide.*** Round 73 measured 26.2.4.2
     clamping an RTF `{\shp}` onto the page in both axes on 21 of 21 probes, because such a shape is
     a Writer *fly* rather than a drawing object and a fly is clipped by `SwFlyFreeFrame::CheckClip`.
     The RTF reader now turns the capture on and `FrameLayout` has the horizontal half of it; DOCX is
     untouched. See *An RTF shape is a Writer fly* below.
   - **`w:titlePg` with a header (or footer) declared and no `first` one still costs the first page an
     empty one's height** — one empty paragraph in the document's `header`- (`footer`-) named
     paragraph style, drawn as nothing. `SectionPropertyMap::CloseSectionGroup` sets
     `PROP_HEADER_NO_FIRST` rather than turning the header off (`dmapper/PropertyMap.cxx`:596-618).
     12 corpus DOCX each way; it only moves a page where `w:header + one empty line > w:top`.
     **Left.** `probes/words-firstpage-r70/`.
2. **Looking gives direction and kind; it does not give cause.** *"Every line breaks earlier in
   the reference, so our glyphs are narrower"* is a lead that no ink percentage contains. But an
   image cannot tell a picture bullet from a character bullet in a substituted symbol font.
   **Name the causes the image cannot decide between, then measure.**
3. **Describe before checking the record.** Reading a page blind and only then looking up what is
   known is a control on the reading, and it works: a gradient description produced that way
   matched a diagnosis made a week earlier from source.

The user's own visual reviews remain **primary evidence** — see
`dotnet/probes/user-review-slides-02/review.md`, where 17 of 30 observations turned out to be a
single class no gate column can see. Where a brief has contradicted one of their observations,
the brief has been wrong.

### Rendering errors cascade

One wrong measurement — a font metric, a margin, a line
break — shifts everything after it, so a single bug manufactures hundreds of unrelated-
looking failures across a corpus. Fix cascades before anything else; they are cheap to fix
and expensive to work around.

The three highest-risk areas, in order:

1. **Font resolution and metrics.** A substitution that is not metric-compatible changes
   advance widths, hence line breaks, hence pagination. The machine must have Carlito and
   Caladea installed (`fc-match Calibri` → `Carlito`) or every OOXML comparison is
   meaningless. Line height derivation from hhea vs OS/2 metrics has specific precedence
   rules — see `research/06-rendering.md` section B.

   **`XRay.Office.Text` now ships those faces itself, as a floor rather than an override.**
   28 files under `Fonts/Bundled/`, copied to a `fonts` folder beside the assembly and
   searched **last**, so an installed face always wins and a machine missing one renders
   correctly instead of substituting silently. `BundledFonts` is the switch:
   `XRAY_OFFICE_BUNDLED_FONTS=0` turns them off, `=prefer` puts them first.

   **The default direction is measured, not chosen, and the obvious choice is the wrong
   one.** Against LibreOffice 26.2.4.2 from the TDF tarball — the build these very files
   came from — `XRay.Office.Fidelity.Tests` over 552 comparisons:

   | | failed |
   |---|---:|
   | bundle as a fallback (installed wins) | **36** |
   | bundle preferred over installed | **68** |

   Preferring them is twice as bad, because **LibreOffice does not read its own bundled
   fonts either** — it resolves through fontconfig, which sees `/usr/share/fonts`. Its
   copies are its own floor for systems without them, exactly as ours are.

   Two things fell out of establishing that, and both are worth keeping:

   - **It comes down to one family.** Comparing the shipped files against Ubuntu 24.04's by
     their own `hmtx`, Carlito and Liberation Sans are *metrically identical* — bundling
     them changes no advance at all — while **Caladea genuinely differs**: `A` is 599 units
     installed against 623 shipped, `o` 480 against 531, `M` 888 against 815.
   - **Ship only the faces the distro packages ship.** The first cut bundled TDF's fuller
     DejaVu, and `fonts-dejavu-core` carries **no Sans or Serif italic at all**. LibreOffice
     therefore synthesises a lean for those, and a bundled real italic makes us draw
     something the reference does not — it broke ten synthetic-oblique tests across three
     projects and emptied three more into green skips. Trimming to the 28 faces the packages
     actually carry returned the suite to its baseline exactly.

   A test whose premise is "this face is not installed" is one that a later change to what
   ships can silently empty. `SyntheticObliqueResolutionTests.ARomanOnlyFamilyHasItsSlantDrawn`
   builds its own directory for that reason.
2. **DrawingML theme colour resolution.** Get the `lumMod`/`shade`/`tint` chain wrong and
   every themed shape on every slide is the wrong colour at once.
3. **Vector import (WMF/EMF/EMF+).** Full support is committed and there is no C# prior
   art — roughly fifty EMF+ record types alone. Real `.pptx` and `.docx` files embed these
   constantly, so this is the largest single body of work in the project rather than a
   tail-end detail. Port from LibreOffice's `emfio/`. SVG is the exception: it reuses
   `Svg.SceneGraph`/`Svg.Model`, translated from `ShimSkiaSharp`'s command list into
   `IDrawingSink`.

## Workflow

```bash
cd dotnet
dotnet build XRay.Office.slnx          # must stay warning-free
dotnet test  XRay.Office.slnx          # ~1100 tests, a few minutes
```

**Do not add `-r`/`--runtime`.** The SDK rejects it on a solution outright —
`NETSDK1134: Building a solution with a specific RuntimeIdentifier is not supported` — and it
is unnecessary: `Directory.Build.props` already pins every test and tool project to the host
RID, computed from the OS and process architecture. Passing `-r linux-x64` to an individual
project is accepted and does nothing, which is the intended state. Read the comment beside the
setting before changing it; it records two traps that both look exactly like the property
having no effect.

That pin is not a tidiness measure. Without it the build resolves SkiaSharp's and
HarfBuzzSharp's native binaries for **twenty-one** runtime identifiers and copies all of them
into every output directory — 687 MB per test project, of which the host can run one. A clean
whole-solution build costs **463 MB with the pin and 6095 MB without it**, which is the
difference between fitting in a container's disk allowance and exhausting it.

### Running less than everything

A full run rebuilds nothing if the tree is already built, so the cost is the tests themselves —
and **essentially all of it is `XRay.Office.Fidelity.Tests`**, which shells out to `soffice` once
per document. It is also the *only* project that does: the other seven reach LibreOffice not at
all, so they need none of the setup below and finish in seconds.

| Project | Needs `soffice` | Rough cost |
|---|---|---|
| `XRay.Office.Fidelity.Tests` | yes, 23 files | minutes |
| everything else | no | seconds |

Those are wall-clock figures on an already-built tree; most of each is the SDK's up-to-date
check rather than the tests, which is why naming one project is worth doing but naming one
*test* rarely is.

So when iterating, name the project — and reach for the filter only inside the slow one:

```bash
dotnet test tests/XRay.Office.Text.Tests/XRay.Office.Text.Tests.csproj                # ~10 s
dotnet test tests/XRay.Office.WordProcessing.Tests/XRay.Office.WordProcessing.Tests.csproj   # ~15 s
dotnet test tests/XRay.Office.Fidelity.Tests/XRay.Office.Fidelity.Tests.csproj \
    --filter "FullyQualifiedName~TableComparisonTests"                            # ~45 s
```

Run every project before committing anyway. The failure this project cares about most is the
cascade — one wrong measurement moving every line after it — and it surfaces in projects you had
no reason to think you had touched.

### Under load a test run can also report failures that are not there

The truncation above is one half of it. The other half was seen twice on 2026-08-14, in
`XRay.Office.Vector.Tests`, on a binary nothing had touched: one run reported **1 failed of 295**
and nine subsequent runs reported 0; another agent, hours later, saw **16 failed of 295**
followed by four clean runs. Neither captured a failing name.

So a run under load can drop tests *and* invent failures. Both look like signal.

The habit that survives both: **a failure you cannot reproduce on a second run is not a
failure yet.** Re-run the project alone before acting on it, and say in the write-up that you
did — an agent that reports "16 failed, then 0 on four re-runs, nothing here touches Vector"
has given a far more useful account than one that reports either number on its own.

### Never pipe `batch-check.sh` into `head` or `tail`

It runs its documents in parallel workers writing to stdout. Closing the pipe early sends
SIGPIPE to a worker, which dies without a word — and the run **silently writes 155 of 156
rows** while the summary line still looks entirely plausible. There is no error and no warning.

Redirect to a file and read the file:

```sh
batch-check.sh "$CORPUS" 'sheets/done-*' out 3 > sweep.log 2>&1
grep '^TOTAL' sweep.log
```

The `TOTAL` line is computed by the script from what it actually processed, so it is the
column to check — a run that lost a worker reports a smaller total, not a wrong verdict. But
that is only a safety net if you read it; a truncated per-document TSV looks fine on its own.

### A parallel sweep must give each document its own directory

`probes/chart-layout/sweep.py` allocates a worker directory as `out / f'w{i % jobs}'`, which is
correct for a fixed pool working consecutive indices and **wrong for a thread pool**, which does
not. Two live renders land in one directory and one `rm -rf`s the other's output. It cost a round
**124 of 947 renders** before it was found, and the failure is silent: the sweep reports fewer
rows, not an error, so it reads as documents that could not be rendered.

`probes/chart-secaxis/sweep-parallel.py` is the corrected form — one directory per *document*, not
per worker slot. Prefer it, and if you write your own, key the directory on something unique to
the item rather than on a slot index.

**The same collision happens between two *runs*, and it reads as a catastrophic regression.**
Round 80 started an `.odp` sweep, believed it dead — it had stopped at 267 of 302 rows and did not
appear in `ps` under the pattern that was grepped — deleted the directory and restarted into the
same path. Both runs then appended to one `rows.tsv` and each `rm -rf`'d the other's `t0`/`t1`
between documents: **27 of 302 rows came back `ours-failed` and 4 documents were scored twice**,
and every one of the 27 rendered correctly on its own. **The check is not `pgrep`** — the harness
wraps a background command in a shell whose command line the obvious pattern does not match — but
the row file's own growth: *a directory that gains rows after its sweep has been declared finished
has a second writer in it.* Restart into a **fresh** directory rather than reusing one.

### A sweep and a rebuild must never overlap

`batch-check.sh` reads `XRAY_OFFICE_CLI` per document, so a rebuild that lands mid-sweep swaps
the binary under it and the run silently mixes two trees. The output looks entirely normal —
there is no error, no warning, and the totals are plausible.

It has bitten once: an agent building the "unfixed" binary to check that its new tests fail
started that build while its own `done-*` sweep was still running, and had to kill and re-run
the sweep. It noticed. The next one might not.

Two habits: sequence them explicitly rather than backgrounding a sweep and then working, and
when a fix must be merged while a sweep is in flight, **merge the source but do not rebuild**
until the sweep finishes — the built binary is what the sweep is measuring, and it is unaffected
by a source-only merge.

### A truncated run reports success

**Check the count, not just the colour.** Under heavy load the test host can die part-way and
still print `Passed! - Failed: 0`, having silently dropped the tests it never reached. Measured
on one commit with several parallel builds running: the fidelity project reported **470 passed**
on one run and **353 passed** on the next, both `Failed: 0`, against **471 discovered**
(`dotnet test --list-tests`). Nothing had changed between them.

This is worse than a failure, because it looks like a pass. Two habits make it safe:

- Compare the passed count against the previous known-good count for that project. A drop with
  zero failures is a truncated run, not a fixed test.
- `dotnet test XRay.Office.slnx` is the most likely to truncate and the least likely to say so —
  it has also been OOM-killed outright. Run the projects individually and total them yourself.

### Before trusting a green run

`XRay.Office.Fidelity.Tests` needs an installed LibreOffice and **skips with a reason when it is
missing**, so a bare `dotnet test` on a fresh container reports a green run while that project
covers nothing at all. A fresh container has none of what it needs. Install it, then confirm
with `check-env.sh` below:

```bash
apt-get install -y --no-install-recommends \
    libreoffice-writer libreoffice-calc libreoffice-impress \
    fonts-crosextra-carlito fonts-crosextra-caladea fonts-liberation \
    poppler-utils
```

`libreoffice-core` alone gives an `soffice` that starts, reports a version and then fails on
every document — which is why `LibreOfficeRunner.IsAvailable` decides by converting a probe file
rather than by finding the binary. The fonts are not optional either: without Carlito and
Caladea every OOXML comparison measures a substituted face and is meaningless. A correct run
reports **0 skipped**; any other number means part of the suite covered nothing.

Comparing against LibreOffice — use the skills, they encode hard-won details:

| Skill | Use for |
|---|---|
| `libreoffice-reference` | Generating reference PDFs, page PNGs and text with headless `soffice` |
| `render-comparison` | Comparing renderings and diagnosing *why* they differ |
| `page-vision` | Actually looking at a page — resolution, cropping, and getting it read by someone uncontaminated |
| `extraction-comparison` | Comparing extracted text; also the right first step for a visual bug |
| `office-corpus` | Building and curating test documents |

**`render-comparison/SKILL.md` opens with a router — which tool answers which question — and
four rules that each cost a round.** Read it before writing any comparison of your own.

- **Set `REF_SOFFICE=/opt/libreoffice26.2/program/soffice`.** PATH here is 24.2.7.2 and this
  tree is calibrated to 26.2.4.2, and they disagree on real documents:
  `f445896eb008d14c1746fc37d412dc22.docx` is 16 pages under one and 15 under the other, so a
  sweep on PATH banks a `pages` failure for a document that matches. Until round 180 only
  `batch-check.sh` and `verdict.py` honoured the variable; the other eight scripts — the *ink*
  sweep, the reference *bank*, and `look.py`, which `page-vision` names for every page reading
  — hard-coded `soffice` and said nothing. All ten now resolve it and **print the resolved path
  and version as their first line of output.** Read that line before the numbers.
- **Do not write your own scorer.** Twice now a hand-rolled mean absolute difference has
  produced a wrong *published* call: one scored a page 11.04 pt out of place as agreement, and
  one ranked a whole track, putting `02_mcar` first at 881.67 for a one-page offset it could
  not see and `Annex-10` first among passing documents for one part in 255 of grey.
  `track-ink-sweep.sh` already does this and already guards both traps.
- **Check page alignment before reading an ink figure.** Ink is compared page against page, so
  one lost page reads as hundreds of defects — it has misread three documents in one session,
  every one of them a single page. Equal page counts do **not** prove alignment: two documents
  can carry different content on the same-numbered pages where a block lost early is made up
  later. `ink.tsv`'s `drift` column counts it, from the text layer rather than the pixels.
- **Rank on the worst page, not on summed ink.** `abs_ink` is the per-page figure *summed*, so a
  long document outranks a badly wrong short one for being long. `ink.tsv` also carries `worst`
  and `mean`; rank on `worst`, and read a `mean` below about 0.3 as "no defect, only
  antialiasing". Round 183 spent most of a round on a 44-page document that headed the ranking at
  10.22 and whose mean is 0.23 — the raster floor — while three one-page charts it outranked were
  5.18, 4.64 and 4.25 on their only page.
- **A raw mean difference is biased by colour quantisation**, at one grey level across every
  flat fill, because the two writers emit colour to different decimal precision and the
  rasteriser floors. `differing_fraction`, `differing_tiles` and `ink_delta` are safe; a raw
  mean is not.
- **For an import question, `soffice --convert-to fodt`/`fods`/`fodp` is the first
  instrument**, not the last: it prints the reference's own resolved view with no rasteriser
  and no tolerance in it.

### The sample corpus

`theolivenbaum/sample-files` holds real-world documents — collected from the open web
and kept as found, mislabelled extensions and malformed markup included — ordered by what
their LibreOffice rendering demands of a renderer and cut into batches of at most ten:

**Every figure in this section is the 534-document corpus's and the corpus is now 947.**
Counted 2026-09-06 from `MANIFEST.tsv`, which is the authority: **947 rows — words 338,
slides 302, sheets 307** — of which 803 are `done` and 144 `open`, and the kinds are
`ceiling` 69, `text` 61, `pagination` 7, `metrics` 3, `unstable` 2, and one each of `missing`,
`extra` and `chart`. The whole-corpus gate at `2f4709c08` is **947 documents, 860 match, 87
mismatch**. So "459 of 534", the batch-size arithmetic below, and the per-track 200 / 163 / 171
in the version table further down are all pre-expansion and none of them is a denominator to
score against. What survives is the *shape* — grouped by what is wrong, ordered by complexity
within a group, `MANIFEST.tsv` as the undo — and that is why the prose is kept.

**The corpus is seven extensions and holds no ODF, no RTF and no template at all.** Counted
2026-09-06 from `MANIFEST.tsv`'s own `ext` column: **docx 272, pptx 251, xlsx 241, doc 66,
xls 64, ppt 51, xlsm 2** — and nothing else. So `odt`, `ott`, `fodt`, `ods`, `odp`, `rtf`,
`sxw`, `sxc`, `sxi`, `docm`, `dotx`, `pptm`, `potx`, `xltx`, `xlsb` and `csv` — most of the
*Scope* list — have **zero corpus coverage**, and a claim of the form "N of 947 documents do
X" is a claim about four OOXML formats and three MS binary ones.

Two consequences that have each cost a round. **A reach census that finds nothing in ODF has
found nothing about ODF**, only that the corpus contains none; the right instrument there is a
hand-built or LibreOffice-converted document, and the write-up has to say which. And **a defect
that only the OOXML readers happen to avoid is invisible to every corpus figure this project
takes** — round 67's as-character alignment defect was wrong in ODF, RTF, WW8 and flat ODF and
right in DOCX, so fixing it moved **10 of the words track's 338 renderings and no gate verdict at
all**, every one of the ten a `.doc`, while a five-format probe of a single paragraph moved four of
five by the picture's whole width. See `probes/words-aschar-band/`.

**There is now a converted ODF corpus and it is where those censuses belong.**
`/home/user/corpus-odf/` holds 26.2.4.2's own `--convert-to` of the whole corpus — **1285 files:
odt 338, rtf 338, ods 307, odp 302** — so both renderers read identical bytes and every divergence
on it is ours. Its `.odp`
column is 302 documents, banked as `probes/odf-gate-01/rows.tsv` and re-rendered fresh by
`probes/odp-master-r70/ref.tsv`, which reproduced it 302 of 302 on pages and glyphs. **Reuse the
reference half rather than re-rendering it** whenever the diff under test is confined to
`dotnet/src`, which cannot reach `soffice`; a round costs about four minutes of our half instead
of forty of both.

**What that column found in three rounds is that the ODF readers were years behind the OOXML ones
on things no corpus figure could ever have shown.** It opened at 120 of 302 with a master page
drawn nowhere; it reached 285 of 302 on the master's running objects, `style:shrink-to-fit` and
`loext:shadow-blur`, reached 289 of 302 on the chart reader's axis resolution,
`draw:text-rotate-angle` and `text:line-break` (`probes/odp-chart-r72`), and is **295 of 302**
after an ODF document's own embedded fonts and an empty paragraph's height
(`probes/odp-embed-r79`). **It is still 295 of 302 after round 80, and that is the point of that
round rather than a failure of it**: a break position, a bullet's picture, a marker's colour and a
shape's extent add no glyphs and no pages, so no gate column can see any of them. The seven that
remain are six raster-ceiling documents where *we draw more* and one glyph-exact `unembedded`.
Two rules from it are general enough to carry:

- **An ODF attribute LibreOffice's own exporter writes is very often not in the namespace the
  specification puts it in, and the ODF-namespace spelling then appears in no real file at all.**
  Five instances are now in the tree and they were each found by a different round the hard way:
  `drawooo:display` for `draw:display` (`OdpSlideLayout.IsPrinted`, 887 occurrences and not one
  `draw:` spelling), `loext:shadow-blur` for `draw:shadow-blur` (`sdpropls.cxx`:169; 1252 non-zero
  occurrences in 120 of the 302, and **zero** `draw:shadow-blur` anywhere),
  `chartext:coordinate-region` (`OdfNamespaces.ChartExtension`), **`text:line-break`** for an
  axis' line breaking (`xmloff/source/chart/PropertyMaps.cxx`:188 maps `PROP_TextBreak` under
  `XML_NAMESPACE_TEXT`; 379 statements in 92 of the 302), and **`drawooo:sub-view-size`** for a
  custom shape's coordinate space (`shapeexport.cxx`:5006 writes `XML_NAMESPACE_DRAW_EXT` **under
  a comment that says it writes `draw:sub-view-size`**; **5673 occurrences in 178 documents — 151
  of the 302 `.odp`, 23 `.odt`, 4 `.ods` — and not one `draw:` spelling**). **Grep the corpus for
  both spellings before implementing an ODF attribute**, and read `sdpropls.cxx`'s `GMAPV` rows and
  `xmloff/source/chart/PropertyMaps.cxx`'s, which name the namespace each property is exported in.

  **The fifth is the most expensive, because the attribute it hides is what makes a shape the
  right size.** A `draw:custom-shape` LibreOffice imported from OOXML carries
  `svg:viewBox="0 0 0 0"` and states its real space per subpath in `sub-view-size`;
  `EnhancedCustomShape2d::SetPathSize` (`EnhancedCustomShape2d.cxx`:650-670) takes it whenever both
  numbers are non-zero. Read in the wrong namespace every such shape falls back to its own bounding
  box in hundredths of a millimetre and is drawn at a fraction of its extent — on `Sean Monogue`'s
  master, four freeforms at 20 × 24 and 12 × 12 points instead of 89.86 × 105.59 and 50.97 × 51.73.
  The importer itself is namespace-blind (`EASGet(nToken)` reduces the token to its *local name*,
  `EnhancedCustomShapeToken.cxx`:200-203), which is why the wrong spelling costs LibreOffice
  nothing and costs a reader everything. Its reach here is the `.odp` alone:
  **`OdfEnhancedGeometry` has two callers and both are `OdpSlideLayout`**, and all 26 of the
  `.odt`/`.ods` that state one render byte-identically before and after.

  **The fourth is the one to remember, because it wears a different disguise.** The first three
  look like a wrong prefix; this one looked like an attribute that does not exist —
  `OdfChartPlot.AxisTextOf` carried *"line breaking has no ODF attribute at all"* for three rounds
  and hard-coded false. It sits on a `style:chart-properties` between two `chart:` attributes, so
  nothing about the element it is on suggests a second namespace. **"ODF states no such thing" is
  a claim about a namespace, and it needs the same two greps as any other.**
- **A property that ODF states two ways has to be read both ways, because the merge is what
  disambiguates it.** `drawing::TextFitToSizeType` is mapped from *both* `draw:fit-to-size` and
  `style:shrink-to-fit` with `MID_FLAG_MERGE_PROPERTY` (`sdpropls.cxx`:143-144), the second
  existing only because an ODF 1.1 consumer reads `draw:fit-to-size="true"` as *stretch*. So
  LibreOffice writes an autofitted shape as `draw:fit-to-size="false" style:shrink-to-fit="true"`,
  and a reader consulting the first attribute alone concludes it does not autofit. `SlideAutofit`
  had been a full port since round 52 and the ODF path reached none of it; the attribute appears
  **3515 times in all 302** of the column, and wiring it moved the gate 259 → 283.
- **And reading both spellings is not enough — the *level* has to be decided before the
  spelling.** Where the two spellings are one item rather than two properties, resolving each
  independently through the parent chain lets an outer style's spelling beat an inner style's.
  `style:font-name` and `fo:font-family` are that case: `style:font-name` is imported through
  `XMLTextImportPropertyMapper::handleSpecialItem`'s `CTF_FONTNAME` branch, which fills the
  `CTF_FONTFAMILYNAME` slot beside it (`xmloff/source/text/txtimppr.cxx`:58-101), and
  `fo:font-family` writes that same slot — so a child stating either shadows whatever its parent
  stated. LibreOffice writes the named style with both spellings and the automatic styles with
  `style:font-name` alone, so an inherited `fo:font-family` wins nearly everywhere the question is
  asked the wrong way: **290 of 307 `.ods` and 109 of 338 `.odt`** of the converted corpus hold at
  least one style where the two sit at different levels. It draws every such sheet in the document
  default's face, and a narrower face wraps fewer lines, which shortens every measured row height on
  it. `probes/odf-rowpitch-r72/`.
- **An ODF axis' index is its position among the axes of its own dimension, and nothing states
  it.** `SchXMLAxisContext` counts how many axes of the same `chart:dimension` have already been
  read (`xmloff/source/chart/SchXMLAxisContext.cxx`:266-274), so the first `chart:dimension="y"`
  is the primary value axis and the second the secondary, and the same rule picks the category
  axis. `OdfChartPlot` *assigned* the category axis on every x axis rather than defaulting it, and
  LibreOffice writes a combination chart's `secondary-x` — carrying `chart:visible="false"` —
  **after** the primary, so a chart's category labels were read off an axis that is not drawn.
  A series names its y axis by `chart:name` through `chart:attached-axis` and belongs to the
  secondary exactly when that axis' index is above zero (`SchXMLSeries2Context.cxx`:333-345,
  :388-394). It is the same shape of defect the BIFF round found in `XlsChartBuilder`'s single
  `_valueScale`, and it bites for the same reason: **the file writes the primary first, so taking
  the last one read is wrong in exactly the files that have two.**
- **A `draw:frame` whose automatic graphic style has no *parent* is a drawing shape, not a Writer
  text frame — and an authored ODF probe that forgets to name one measures the wrong object.**
  `XMLTextFrameContext`'s constructor sets `m_HasAutomaticStyleWithoutParentStyle`
  (`xmloff/source/text/XMLTextFrameContext.cxx`:1374-1394, *"New distinguish attribute between
  Writer objects and Draw objects is: Draw objects have an automatic style without a parent style
  (#i51726#)"*) and `createFastChildContext` then hands the element to
  `XMLShapeImportHelper::CreateFrameChildContext` (:1500-1507) instead of building a
  `com.sun.star.text.TextFrame`. A shape fits itself to its text by a different rule and ignores
  `fo:min-height` outright. Round 75 wrote sixteen probes without a parent style, watched
  26.2.4.2 draw `fo:min-height="3in"` on a two-line box as **27.65 pt**, and confirmed that wrong
  conclusion three ways — flat ODF, the same file zipped, and a patched corpus document — before
  the cause was found, because all three shared the defect. LibreOffice's own exporter always
  names `Frame`, so no real document is affected; **every hand-built `draw:frame` probe must name
  a parent style**, and the same trap is waiting in `.odp` and `.ods`.
- **And the sixth instance of the namespace rule is not a name at all — it is a *value*.**
  `writing-mode` is exported as `style:` for every value ODF 1.3 defines and as `loext:` for the two
  it does not, `bt-lr` and `tb-rl90`: `CheckExtendedNamespace` re-qualifies the attribute inside
  `SvXMLExportPropertyMapper::_exportXML` (`xmloff/source/style/xmlexppr.cxx`:947-952 and
  :1108-1118, under a comment saying there is no generic mechanism for it), and Writer's own table
  item map does the same by hand (`sw/source/filter/xml/xmlexpit.cxx`:193-220). **The importer reads
  both spellings** — `xmlitemm.cxx`:281-282 holds `style:writing-mode` and `loext:writing-mode` side
  by side against `RES_FRAMEDIR`, and `xmlimpit.cxx`:1008-1030 takes the value whichever namespace
  carried it — and 26.2.4.2 draws the two identically, measured one cell per spelling. So the
  extension namespace is an export convention, and **grepping the corpus for the attribute name
  finds it and tells you nothing**: the converted `.odt` state `style:writing-mode` on 14 264 table
  cells of which 14 261 say `lr-tb`, while all 86 `bt-lr` cells — in 11 documents — are `loext:`.
  Reading only the specification's spelling drew each of them a glyph per line.
  `probes/odt-split-r82/results.md` §1.

- **An ODF `draw:frame` holding nothing but a table is OOXML's `w:tblpPr`, and this engine already
  splits one.** LibreOffice's DOCX importer turns a positioned table into a fly holding a table and
  marks it splittable without exception (`DomainMapperTableHandler.cxx`:1765); its ODF export writes
  that fly back out as a `draw:frame` carrying `loext:may-break-between-pages="true"` **on the
  element**, not on its graphic style (`txtparae.cxx`:3115-3119 writes it, and
  `XMLTextFrameContext.cxx`:1104-1107 reads that spelling and the `draw:` one alike). One object,
  two spellings. `Paginator.PlaceFloatedTable` has carried the rows a page cannot take since the
  round that closed `Case-Study-Heathrow-Airport.docx`, and `ContinueFloatedTables` places them at
  the top of the next page — **starting pages that no block created** when the body's flow has run
  out. **So "a fly's continuation needs pages no block created and obstacles keyed by page" is not
  the blocker two rounds recorded: the blocker was the ODF reader, which routed the same object
  through the frame path.** `OdfFrames.FloatingTable` and `OdtLayoutSource.Floated`; `.odt`
  **281 → 290**, the original `.docx`/`.doc` track 338 of 338 rows identical and the converted
  `.rtf` column 338 of 338 identical.

  Three things fell out of it and each is worth more than the fix:

  - **What splitting is worth is measurable at the reference, and it is six rows rather than the
    13 and 24 two rounds recorded.** Strip `loext:may-break-between-pages` from the file, render the
    patched document through 26.2.4.2 and compare: a document whose counts do not move is one the
    reference never split. Of the 20 failing rows holding a splittable fly, **six** move — and on
    four of those the reference *with splitting off* reproduces this tree's own page and glyph
    counts **exactly**, which is as clean a statement as this corpus offers that splitting was the
    whole of the gap. All 51 splittable flies in the column hold a table and nothing else, so the
    paragraph-flow slicer a round could have written would have reached nothing.
    `probes/odt-split-r82/unsplit.py`.
  - **Three document settings decide more of it than the attribute does.** `TabOverMargin` picks the
    deadline a split fly is cut at — the page's bottom rather than the body's, `GetFlyAnchorBottom`
    and `isLegacyBehavior`, `sw/source/core/layout/fly.cxx`:101-162 — and **177 of the 338 `.odt`
    state it true, 161 false**; reading it as absent split three graph-paper templates onto a second
    page each. `DoNotBreakWrappedTables` is a document-level veto over every
    `may-break-between-pages` in the file, tested before the fly is looked at (`fly.cxx`:696-700),
    and **30 of the 338 state it**. Both default to false.
    The third is `TabsRelativeToIndent`, and it is the one that defaults the *other* way. It decides
    whether a tab stop's stated position is measured from the paragraph's own indent or from the text
    area's edge -- `frameArea.Left() + (bTabsRelativeToIndent ? GetTabLeft() : 0)`,
    `sw/source/core/text/txttab.cxx`:94-98 -- and **absent means true**
    (`DocumentSettingManager.cxx`:80), because that is Writer's native answer. Every Word-family
    importer sets it false (`ww8par.cxx`:1951; `DomainMapper.cxx`:128-132), and the ODF export writes
    the result into `settings.xml`, so **all 338 converted `.odt` state it false** and a reader that
    does not look laid every one of them out by the wrong rule. The reach is narrower than that
    census -- the two rules differ only where a paragraph holds both a tab and a non-zero indent, 38
    documents and 3728 paragraphs -- but on `A_320.odt` it was the whole of a 16-page gap.
    **The general lesson is the default direction.** `TabOverMargin` and `DoNotBreakWrappedTables`
    default false, so failing to read them is failing to turn something on, and the damage is
    confined to files that state them. This one defaults *true*, so failing to read it silently
    applies Writer's own rule to documents that explicitly asked for Word's -- and it does that on
    every converted file in the corpus at once. When you meet a new `config-item`, establish its
    default before you estimate its reach; a true default makes an unread setting a whole-track
    defect rather than a rare one.
  - **A "shape that over-draws" may be a reference that outlines its glyphs, and the check is one
    measurement.** Screened over the eight `chartset` templates the `.odt` gate reads as drawing
    3 % to 43 % too much: on `051_Organogram_Template_Basic_Theme` 26.2.4.2 draws 49 characters as
    text and **29 glyph-sized filled paths**, which is most of the 37-character "excess", and on
    `055_Organogram_Template_Horizontal_Structure` 37 more; `024_Unit_Circle_Chart_Colorful_Circles`
    draws **35 characters fewer** than the reference, so the group is not even one sign. Count the
    reference's outlines before working one of these — `probes/odt-split-r82/overdraw.py` does it —
    and take `011`, `018`, `031` and `040`, which carry none on either side, as the four that really
    are an autofit or clip question.
  - **The instrument that shows a fly's tail drawn below the sheet has to read `Td` as well as
    `Tm`, and `get_text` cannot see it at all.** PyMuPDF clips text to the page, so it reports a few
    descender-sized overhangs and none of the real thing; a content-stream scan matching only `Tm`
    finds **one** document in 338, because Writer's PDF writer positions most text objects with
    `Td`. Matching both finds 30, Heathrow's page one reaching y = −945.6 and `ESPN-R`'s page 41
    −6732.1. This round nearly published *"round 77's observation no longer holds"* on the strength
    of the `Tm`-only scan. `probes/odt-split-r82/tdrange.py`; the two wrong instruments are kept
    beside it.

**An ODF `text:section` is a Writer text section, and a multi-column stretch inside a page is the one
thing that object exists for.** The ODF reader walked straight through the element, so a two-column
section was laid out one column wide across the whole measure. Both Word importers reach for the same
object and say so: `SectionPropertyMap::CloseSectionGroup` — *"prefer setting column properties into a
section, not a page style if at all possible"* — then `appendTextSectionAfter`
(`sw/source/writerfilter/dmapper/PropertyMap.cxx`:1905-1913), and `wwSectionManager::InsertSection`
builds a `SwSectionFormat`, puts the section's **difference** from the page style's margins on it as an
`SvxLRSpaceItem` — `nSectionLeft = rSection.GetPageLeft() - nPageLeft` — and only then calls `SetCols`
(`sw/source/filter/ww8/ww8par6.cxx`:735-745). LibreOffice's ODF export writes that back out as a
section-family style carrying `style:columns` and `fo:margin-left`/`fo:margin-right`, so reading it is
the exact inverse of what produced the file — on `644730BRI0mna000BOX361539B00public0.odt` the page's
`0.4165in` and the section's `0.5835in` sum to the round inch its `.doc` declares.

**Three things about it are worth carrying.** *A section frame's geometry applies where the flow
reaches it*, not on the next sheet — `aRectFnSet.SetLeft(aPrt, rLRSpace.ResolveLeft())` under
`#109700# LRSpace for sections`, `sw/source/core/layout/sectfrm.cxx`:130-181 — which is the opposite of
a continuous section's *page style* margins, and `WritingSection.IsTextSection` is what separates the
two. *The per-column widths are read only where LibreOffice reads them*:
`XMLTextColumnsContext::endFastElement` takes the `style:column` descriptions under
`!bAutomatic && maColumns.size() == nCount`, and `bAutomatic` is set by the mere **presence** of
`fo:column-gap` (`xmloff/source/text/XMLTextColumnsContext.cxx`:216-222, :268-315) — so of the corpus's
112 section `style:columns`, **107 state a gap and are even (33 of those also state unequal widths that
LibreOffice ignores) and 5 in 4 documents state widths with no gap and are apportioned**. And a
`style:rel-width` is the column's *outer* width, its own indents included
(`pColumn[nCol].Width = (fWidth + fLeft + fRight) * fRel`, `dmapper/PropertyMap.cxx`:868-874), so the
text width is the share less that column's `fo:start-indent` and `fo:end-indent` and the gap is the
first's end indent plus the second's start indent. Reach **33 of the 338 converted `.odt`**, of which
23 are `chartset` templates with no flowed text; **11 renderings move**, mean |Δx| from 26.2.4.2
**8.593 → 4.083 pt**, `.odt` gate 291 → 292. `.ods` and `.odp` hold no `text:section` at all.
`probes/odt-startx-r88/results.md`.

**And a section inside a section is a *sibling* of it, so the enclosing one's columns never reach
it — an ODF index included.** Writer inserts a nested section's frame behind its parent, into the
parent's own upper (`pFrame->InsertBehind(pTmp->GetUpper(), pTmp)`,
`sw/source/core/layout/frmtool.cxx`:1795-1803), and splits the parent at the nested section's end
node so that what follows is a **second frame of the parent's format** (`SwSectionFrame::SplitSect`,
`:1954-1960`) — so a page reads *two columns, a full-measure index, two columns again*. What the
child does take from the parent is the **indents**, because a nested section's format is derived
from the enclosing one's (`pFormat->SetDerivedFrom(pSectNd ? pSectNd->GetSection().GetFormat() : …)`,
`sw/source/core/docnode/ndsect.cxx`:1345) and `SwSectionFrame::Init` insets its print area by
`GetFormat()->GetLRSpace()` (`sectfrm.cxx`:129-166); what it never takes is the **columns**, because
`SwSectionFormat`'s constructor puts the pool's default one-column item on every section format
outright — `LockModify(); SetFormatAttr(*GetDfltAttr(RES_COL)); UnlockModify();`,
`sw/source/core/docnode/section.cxx`:608-614. The two indents are one `SvxLRSpaceItem`, so a child
stating either **replaces the pair** and the side it left out is nought. Measured over sixteen
one-element variants of one fixture (`probes/odt-sectable-r92/nested.py`). **Reach is 1 of the 338
converted `.odt`** — `absrc-pac-01-info-note-en`, whose two-column section holds a
`text:table-of-content` — and closing it takes the `.odt` gate **292 → 293**, recovering the verdict
round 88 cost, with exactly one row and two renderings moving in the whole column.

**Two of round 88's readings of that document are withdrawn, and one instrument produced both.**
*"A table inside a columned `text:section` is laid out against the page rather than against its
column"* is false at all three documents that hold one: every cell of all three is within **0.35 pt**
of 26.2.4.2's, and a probe shows both renderers laying a table inside its column and both letting a
table wider than the column overflow to the right of the *page*. *"26.2.4.2 draws that document's
section in ONE column although the file states two"* is false too — it draws two, at the positions
this tree computes, and removing `style:columns` moves the second column's paragraph to the next
page. Both came from **a histogram of line starts taken with every span of one baseline merged,
which reads a two-column page as one column** because a baseline crosses the gap;
`probes/odt-sectable-r89/xhist.py`'s docstring is where that correction is written down and is the
only thing that cut-off round left. **Cut a baseline into segments at a gap wider than a tab before
counting columns.**

**A third rule fell out of it and it is not an ODF one: a page carries one text area, and it is the
last section's.** `LaidOutPage.BodyArea` is `geometry.TextArea` at the moment the page is *emitted*,
so a `text:section`'s own indents reach the layout and not the drawing wherever a later section on
the same page restores the master's margins — 26.2.4.2 draws a 0.5 in-indented two-column section's
first column at 108.10 and this tree at 72.00, with the same wrap.

***Done, and the gate could not have scored it — the fixture is where the 36 pt is.*** A
`PlacedLine` now carries the text area it was laid out in (`BodyLeft`/`BodyWidth`, horizontal only:
a text section's top and height are the page's) and `LaidOutPage.BodyAreaOf` divides *that* rather
than `BodyArea`; `PageDrawing.DrawBody` groups by it as well as by the column, **and its
single-column fast path had to learn about it too**, or a page of one-column text drawn from two
different left edges takes that branch and is drawn from one. On `odt-sectable-r92/nested.py`'s
`plain-margins` the three left edges go 72.00/—/324.00 to **72.00/108.00/315.00** against
26.2.4.2's 72.10/108.10/315.10, with every span count equal; over five variants mean |Δx|
**14.500 → 0.100 pt**, which is the two writers' constant text-origin offset. On the corpus **5 of
338 `.odt` renderings move and they are exactly the five documents with an indented `text:section`**
— 71 spans in all, each document moving its own by one constant — and the two whose pagination
agrees with 26.2.4.2 improve (2.931 → 2.885 and 2.462 → 1.347 pt of left-edge distance). `.odt`
gate 293 of 338 before and after, **no verdict either way**, and 1305 words, slides and sheets
renderings byte-identical. `probes/words-seat-r94/`.

**The instrument that hides this is the one two rounds already used.** A merged-baseline line
matcher samples only the *first* column of a two-column page, so a wrong *second* column is
invisible in it — `odt-startx-r88/startx.py` reports the three `150_5300_13` revisions as
unchanged when 6, 14 and 1 of their spans moved. `probes/words-seat-r94/columnx.py` clusters every
span's own left edge and merges nothing; `colscore.py` pairs those clusters with the reference's;
`selfmove.py` pairs spans between our own two legs, which is what a document that sits a page out
needs.

What was closed with the line-level half of the same confusion in the round before: `PageContent.ColumnArea(PlacedLine)` used to send a line stating one column to the
**page's** column at that line's index, which drew a full-measure heading inside a column — 4 of 696
`.docx`/`.doc`/`.rtf` renderings move with it, none changes a page or a glyph count, and
`150_5300_13_chg8`'s centred `Chapter 3.  RUNWAY DESIGN` goes from 205.05 to **231.75** against
26.2.4.2's 231.80 in all three of its formats. `probes/odt-sectable-r92/results.md`.

**And two instrument corrections came out of it that invalidate a stored ranking.**
`probes/odt-page-r87/residual-startx.txt`'s columns are *matched, mean before, mean after, within-0.1pt
before/after* — not *max, mean, pages agreeing*, which is how a brief read them, so
`011_Project_Timeline_Template_Beautiful_Theme` was carried as "page-exact with a 342 pt error" when it
is a one-page document failing the gate on **glyphs**. And the 342 pt itself is `difflib` pairing **nine
identical repeated labels in draw order**: that template's nine `Lorem ipsum` blocks sit at the *same
nine* x on both sides to 0.10 pt, and the two Venn templates' three labels likewise. Corrected — spans
merged per baseline, and a unique-line column beside the mean — the three read **0.142, 0.114 and
0.111 pt**. The second correction is separate and costs as much: **this tree writes a justified line as
one text object per stretch and 26.2.4.2 as one per line**, so `get_text('dict')` reports 635 "lines"
against 479 for one page and the matcher pairs a fragment against a whole line. `probes/odt-startx-r88/startx.py`
is the instrument to reuse; **read a `|Δx|` ranking with the matched-line count and the unique-line
count beside it.**

**An ODF document's own embedded fonts were never loaded, and the seat is `svg:font-face-uri`.**
A `style:font-face` that carries a face holds one `svg:font-face-uri` per style under an
`svg:font-face-src`, naming a package part through `xlink:href` or carrying the bytes as an
`office:binary-data` child; `XMLFontStyleContextFontFaceUri::endFastElement`
(`xmloff/source/style/XMLFontStylesContext.cxx`:250-277) picks the container off
`svg:font-face-format` and hands the stream to `addEmbeddedFont`. Three details decide whether a
reader reproduces it. **The style is read out of the face**, because that `SetAttribute`
(`:230-237`) handles `xlink:href` and nothing else — LibreOffice's own export writes
`loext:font-style` and `loext:font-weight` on every one of them and the importer reads neither.
**The face is registered under its own typographic family**, name 16 → name 1 → PostScript name
(`vcl/source/font/TrueTypeFont.cxx`:140-145), falling back to the document's `svg:font-family`
only when the face carries none (`embeddedfontsmanager.cxx`:355-362, tdf#172647). And **an
external URL is not fetched** (`:305`). Reach **6 of 302 `.odp`, zero `.odt`, zero `.ods`** — the
corpus's embedding all arrives through LibreOffice's own ODF export of a `.pptx`. `pdffonts` over
the six goes from 2 of 6 face sets matching 26.2.4.2 to **6 of 6**. `OdfEmbeddedFonts`; the deck
reader's `PptxEmbeddedFonts` was already the same shape and is the model.

**And the half of that which was not in an ODF reader at all: a zero-length `hdmx` makes
`hb_subset_or_fail` fail, and a failed subset is a face this tree names and does not embed.**
`FontSubsetter` named the tables it *dropped* — GSUB, GPOS, GDEF. LibreOffice inverts the drop
set and keeps fourteen tags (`PhysicalFontFace::CreateFontSubset`,
`vcl/source/font/PhysicalFontFace.cxx`:546-562, *"Keep only tables needed for PDF embedding, drop
everything else"*), and that is not a weight measure: the four `Font_Verdana_*.ttf` inside
`Sean Monogue.odp` carry `hdmx` and `VDMX` at length **zero**, and subsetting returns null on all
four as they stand and succeeds on all four with `hdmx` alone removed — not `VDMX`, not `LTSH`.
The symptom is a PDF that draws the reader's own substitute for a family the document carried,
and **the only gate column that can see it is `unembedded`**. Naming the keepers is now what this
tree does too.

**An ODF sheet recalculates its automatic row heights for its first 200 rows only, and past them
the writer's stored height stands.** `ScXMLTableRowContext::endFastElement` adds every row block to
the recalculation ranges and then takes it straight back out when the block ends past row 200 and
its style states a height *and* the optimal flag — `maRanges.setFalse(nFirstRow, nCurrentRow)`,
`sc/source/filter/xml/xmlrowi.cxx`:215-244, the test at `:228`, under the comment *"recalc only the
first 200 row in case of optimal document loading"*. **The test reads as "a style with no
`CTF_SC_ROWHEIGHT`" and means the opposite**: `ScXMLRowImportPropertyMapper::finished`
(`xmlstyli.cxx`:245-258) moves the stated height *into* the optimal-height property and clears the
height one for exactly those styles, and the `any2bool` that follows is then reading the height. A
style stating the flag and no height has both cleared and is recalculated wherever it sits. The
rows are zero-based, so rows 0–200 are recomputed and 201 onwards is not — measured on a 206-row
probe where 26.2.4.2's baseline gap steps from 12.78 pt to the stated 21.60 exactly between `r200`
and `r201` (`dotnet/tests/corpus/features/sheet-row-height-limit.fods`). **Reach 182 744 rows in
166 of the 307 converted `.ods`**, and it cuts both ways: `Laser Report 2024 FOIA __Oct (1).ods`
was 486 pages against 506 because we recomputed a shorter row, and
`afn-afn-20250801-fy25-jan25-mar25.ods` was 282 against 270 because we recomputed a taller one. It
is the **importer's** rule — the BIFF and SpreadsheetML filters have no such limit —
so it belongs in `OdsPrintSetup` and not in `SheetOptimalRowHeights`.
`probes/ods-resid-r80/results.md` §2.

**Which of Calc's two row-height answers a row gets is decided by two things that are not
properties of its text, and both were unread.** `ScColumn::GetOptimalHeight` takes the cheap
arithmetic — `lcl_GetAttribHeight`, `trunc(sizeTwips × 1.18) + margins − 23` — only while
`bStdOnly` holds, and clears it for a cell whose **pattern carries a conditional format**,
whatever the condition says and whether or not it fires (`column2.cxx`:937-941, *"conditional
formatting: loop all cells"*). It is the *pattern* that is tested, so a rule declared over a whole
column makes every row of that column measured. Separately a **hyperlink** cell is an
`EditTextObject` holding one field, the same object a rich string makes. Both come out at one
EditEngine line of the cell's own face — **298 twips for Calibri 11 against the arithmetic's
276** — and `StandingEditLine` had computed exactly that since round 56 without ever being asked.

**The instrument is the reference's own `--convert-to fods`, and on a scaled sheet nothing else
will do.** It prints the height Calc computed for every row as `style:row-height`;
`Special-Procedures_2025-07-10.ods` prints at `style:scale-to="39%"`, so its rows reach the page at
5.373 pt against 5.804 and neither number is a row height. One-attribute variants then settle it:
deleting that document's `calcext:conditional-formats` element moves rows 6–200 from 298 to 276,
and on `hdss-bulletin-index-2019-2022.ods` — a table of hyperlinks — rewriting every `<text:a>` to
its own text does the same, while its header row, the one row holding no link, is 276 either way.
A twelve-size sweep confirms the arithmetic branch is exact at every size (6–10 pt on the 256-twip
floor, 11 → 276, 12 → 300, 24 → 583), so the 298 is the other branch and not a different sum.
Reach **95 of the 307 converted `.ods` state a conditional format and 74 hold a `text:a`**; the
`.ods` gate goes **274 → 278** and the same 307 documents as `.xlsx`/`.xls`/`.xlsm` stay at 294.
The spelling read is `calcext:conditional-format`, which is what LibreOffice writes.
`probes/ods-notes-r92/results.md` §2.

***And reading that spelling alone is exact rather than a compromise — the "one of the 307 states a
`style:map` without it" that stood here counted the wrong element.*** `style:map` is also how a
`number:*-style` states its positive, negative and zero sub-formats, and a *conditional* one is the
one that sits on a `style:style style:family="table-cell"` and carries `style:base-cell-address`.
Censused that way over the 307 converted `.ods`: **53 documents carry a conditional `style:map`,
every one of the 53 also states `calcext:conditional-format`, and 0 of them hold a single mapped
cell outside a `calcext:target-range-address`.** LibreOffice writes both from one
`ScConditionalFormatList` in one pass — `ScXMLExport::ExportConditionalFormat`
(`sc/source/filter/xml/xmlexprt.cxx`:4779-4800) and `ScXMLAutoStylePoolP::exportStyleContent`
(`sc/source/filter/xml/xmlstyle.cxx`:700-810) — so for any file it wrote they cannot disagree. The
document that census named, `2025_Active_Civil_Airmen_Statistics_FINAL.ods`, states 23
number-format maps and **no conditional format at all**.
`probes/ods-residue-r95/results.md` §2.

***The BIFF half is nil too, and for a reason that is not about `CONDFMT`.*** 4 of the corpus's 64
`.xls` state one — 29 records over 35 ranges, reproducing 26.2.4.2's own
`calcext:target-range-address` export of the same four workbooks 35 of 35 — and **no BIFF8 row
height is ever recomputed**, because `ImportExcel8::Read` holds its `AdjustRowHeight()` inside an
`#if 0` whose comment is the rule: *"Excel documents look much better without this call; better in
the sense that the row heights are identical to the original heights in Excel"*
(`sc/source/filter/excel/read.cxx`:1284-1288). `ImportExcel::Read`, BIFF2 through BIFF7, calls it
unguarded (`:779-780`), which is exactly the version test `SheetGrid.RowHeightsAreManual` already
makes. Measured: a BIFF8 workbook whose `ROW` records are patched to a uniform **100 twips** with
`fUnsynced` clear comes back from `--convert-to fods` at 100. Reading `CONDFMT` anyway leaves 64 of
64 `.xls` renderings byte-identical. *And `Special-Procedures_2025-07-10.xls`, named as the witness
that the two spellings disagree, opens `PK\x03\x04` — an OPC zip wearing a `.xls` name, read by the
SpreadsheetML path, and 22 pages of 22 before round 92 as well as after.*
`probes/ods-residue-r95/results.md` §1.

**And the ODF half of "Comments: at end of sheet" needed three inputs, not two.** The flag is a
token inside a list — `style:print="… annotations …"`, mapped to `PROP_PrintAnnotations` by
`XMLPMPropHdl_Print(XML_ANNOTATIONS)` (`xmloff/source/style/PageMasterStyleMap.cxx`:80,
`PageMasterPropHdlFactory.cxx`:85) and read into `aTableParam.bNotes`
(`sc/source/ui/view/printfun.cxx`:944). The notes are fastened to their cells by **containment**,
as `office:annotation` children of the `table:table-cell`, so the address comes from a walk of the
table and cannot come from the content tree, which hoists an annotation into a section of its own.
And **the author line is inside the text rather than in `dc:creator`**: every annotation in both
witnesses says `<dc:creator>Unknown Author</dc:creator>` and 26.2.4.2 prints none of them, while
the page opens each note with the name the note's own first paragraph carries. Reach 2 of the 307
and both were failing; both are page-exact after it. *`XRay.Office.Spreadsheets/TODO.md`'s "the
flat-ODS export drops cell annotations entirely" is refuted — 12 of 12 and 8 of 8 survive
`--convert-to fods`, and the wrong figure is `grep -c` on a one-line `content.xml`. Count
occurrences, not lines.*

**And a Calc cell the importer made several paragraphs of is measured by the EditEngine, which
answers the widest *paragraph* and a line per paragraph — not the whole string and not one line.**
`ScColumn::GetNeededSize` takes the edit branch for `CELLTYPE_EDIT` (`column2.cxx`:297-300),
formats against a paper 1000000 units wide (`:447`), and reads `pEngine->CalcTextWidth()` (`:565`,
a maximum over the paragraphs, `editeng/source/editeng/impedit2.cxx`:3507-3525) for the width and
`pEngine->GetTextHeight()` (`:571-577`) for the height. **Which cells those are is the importer's
answer**: Calc's ODF filter never calls `SetSingleLine`, so a hard break or a second `text:p` makes
a paragraph, while the BIFF and SpreadsheetML filters do call it for a non-wrapping cell and the
break stays a character inside one line. Measuring the whole string instead is the whole of
`CIS_Debian_Linux_8_Benchmark_v1.0.0.ods`'s 88 pages against 61, established at the reference with
no free parameter: rewriting every newline in that file as a space makes **26.2.4.2 itself** print
its two sheets in 51 and 36 pages, which were this tree's counts exactly, and keeping only each
cell's longest paragraph makes this tree print the reference's 39 and 21.
**The 246 styled-but-empty columns two rounds blamed for it are irrelevant** — deleting them leaves
26.2.4.2 at 61. `probes/ods-resid-r80/results.md` §1 and §3.

**A census of "multi-line cells" that looks only for a raw newline understates the reach by
two-thirds**, because a cell with several `text:p` children is equally a multi-paragraph edit cell:
649 raw-newline cells against 1246 several-paragraph ones over the same 33 documents.

**And a `draw:` shape anchored in a cell is in the drawing layer, so no cell question may see it.**
ODF fastens a cell-anchored object by *containment* — `ScXMLExport`'s `WriteShapes` puts it inside
the `table:table-cell` it belongs to — and a reader that walks the cell therefore finds the shape's
paragraphs among the cell's. Calc never does: `ScColumn::GetOptimalHeight` walks the column's own
cell storage (`sc/source/core/data/column2.cxx`:894-949), `ScTable::GetCellArea` counts cells rather
than objects (`table1.cxx`:1091-1120), `ExtendPrintArea` spills a *cell's* string (`:2127`), and an
object reaches the page only through `ScDrawLayer::GetPrintArea` (`drwlayer.cxx`:1344-1424), which
`ScDocument::GetPrintArea` maxes in afterwards (`documen2.cxx`:644-664). Reading the shape as cell
text made its row a line per paragraph, spilled its longest line across the neighbours, and made a
cell holding nothing else count as content — `SSRO_Quarterly_Statistical_Bulletin_Q3201617_DATA.ods`
was **11 pages against 4** and drew 1244 of the reference's 2779 characters. **Closed**:
`ContentTableCell.GetOwnText()` skips a `SectionKind.Frame` child and every cell question in
`XRay.Office.Spreadsheets` asks it, while `GetText()` is unchanged so extraction still finds the words
under that cell.

**Which meant the shape had to be drawn, and the sheet reader read only `draw:frame`.** Censused
over the 307 converted `.ods`, walking through the transparent `draw:g` and `draw:a` wrappers:
**604 `draw:custom-shape` in the cells of 54 documents, of which 137 in 32 carry ink**, against
495 frames, 72 `draw:control`, 41 `draw:line` and 31 `draw:connector` that carry **none**. So the
text is entirely in the custom shapes; `OdsShapeText` builds the same `SheetShapeText` the
SpreadsheetML and BIFF readers build and `SheetShapePainter` already knew how to draw. `.ods`
**255 → 268 of 307**, sixteen verdicts gained and none lost, and the original `.xls`/`.xlsx` track
byte-identical on all 307. `probes/ods-draw-r82/results.md`.

- ***Two of the four styles a shape's text could inherit from do not reach it, and that is
  measured.*** On a **Calc** sheet a run's size, face and weight come from the paragraph's own
  `text:style-name` and its spans' and from nowhere else: neither the shape's `draw:style-name`
  graphic style's `style:text-properties` nor its `draw:text-style-name` paragraph style carries.
  Four one-attribute variants of `features/sheet-shape-text.fods` — removing the graphic style's
  `fo:font-size="18pt"`, changing it to 8 pt, putting 14 pt on the text style, removing
  `draw:text-style-name` outright — leave 26.2.4.2's rendering identical, and the paragraph naming
  no style of its own is drawn at **11.99 pt in Liberation Serif**, which is the EditEngine pool's
  12 pt and `DefaultFontType::LATIN_TEXT`. The control moves: the same style named on the `text:p`
  centres that line and sets it at its own size and face. `SdXMLShapeContext::SetStyle` resolves
  `draw:text-style-name` in the *shape import's* automatic-styles context
  (`xmloff/source/draw/ximpshap.cxx`:740-757), and Calc registers its paragraph automatic styles
  elsewhere. **A slide is the other way round**, which is why `OdfTextBody` resolves four levels and
  `OdsShapeText` resolves two — so do not "fix" one by copying the other.
- ***The box properties do come from the graphic style, and one of them is the fourth disguise
  again.*** The four `fo:padding-*` are the insets and default to **zero**
  (`SDRATTR_TEXT_LEFTDIST`, `svx/source/svdraw/svdattr.cxx`:247-250) rather than to DrawingML's
  tenth and twentieth of an inch; `fo:wrap-option` is `PROP_TextWordWrap` and defaults on (`:267`);
  `draw:textarea-vertical-align` defaults TOP (`include/svx/sdtaitm.hxx`:38) and
  `draw:textarea-horizontal-align` defaults **BLOCK** (`:64`), under which each paragraph's own
  `fo:text-align` decides. And **`style:overflow-behavior` is ODF's spelling of DrawingML's
  `vertOverflow="clip"`** — the same `PROP_TextClipVerticalOverflow`, mapped at
  `xmloff/source/draw/sdpropls.cxx`:159 through a named boolean whose true token is `clip` and whose
  false one is `auto-create-new-frame` (`xmloff/source/style/prhdlfac.cxx`:483-487). **95 of the 137
  inked shapes state it.** It is on a `style:graphic-properties` between two `draw:` attributes, so
  nothing about where it sits suggests it exists at all — the same disguise `text:line-break` wore.
- ***A turned shape's rectangle is its `draw:transform`'s, and the end cell must not be added to
  it.*** A shape stating a transform states no `svg:x`, so its top-left corner comes from the
  transform — which a half turn puts to the *left* of the anchor cell — while
  `table:end-cell-address` is Calc's cached far corner for the same object. Taking one from each
  gives a rectangle neither describes: on `Foreign_SA-CAT-I_and_CAT-II-III_Pub_0.ods` the union ran
  47.73 inches of columns where the shape is 23.66 wide and cost **four pages against 26.2.4.2's
  sixteen**. 26.2.4.2 prints sixteen with the end cell and sixteen without it. **The control is the
  other way round and matters**: a text box stating `svg:x` *is* resized by its end cell — a
  1-inch box at A2 on 1-inch columns with `table:end-cell-address="Probe.E2"` has its right-aligned
  line drawn at x 289.644 against 73.644 without it, and this tree answers 289.686 and 73.686. Reach
  2 shapes in 2 of the 307. `probes/ods-draw-r82/endcell.py`.
- ***A `text:tab` in a sheet shape draws no glyph and is not in the reference's text layer***, and
  costs only 1.09 pt of advance — measured character by character, `the` ending at 244.642 and
  `shape` beginning at 245.735. 30 tabs in 5 of the 307. ***And a percentage `fo:font-size` is of
  the parent style, not of the enclosing level***: a span stating `50%` inside a paragraph style
  stating 14 pt is drawn at **14 pt**, because an automatic text style has no parent to take a
  proportion of.

**An empty ODF paragraph is as tall as its own empty `text:span`, not as the shape's default.**
LibreOffice writes an empty line as `<text:p><text:span text:style-name="T15"/></text:p>` and
EditEngine measures it from the character attributes at the paragraph's own position
(`editeng/source/editeng/impedit3.cxx`:1896-1902), so the excess a reader sees is
`1.2 × (default − span)` **per empty paragraph** — 19.2 pt on a 16 pt span in a 32 pt
placeholder. That is the whole of round 78's *"inter-paragraph spacing too large by a constant"*,
and **all three causes that reading left open are refuted**: nothing is added, no margins are
summed that the reference collapses, and no shrink-to-fit is involved. Which span, where there
are several, is **the last one entered** — not the largest and not the outermost. The control
that hides it is a *bare* `<text:p/>`, which does take the shape's default and on which both
renderers already agreed. Measured over ten one-attribute variants of one slide, 10 of 10 exact.

**A `text:a` in slide text is an EditEngine *field*, and that explains two things an image
cannot.** `txtparai.cxx`:1352-1370 asks the cursor for a `HyperLinkURL` property and builds an
`XMLUrlFieldImportContext` when it has none, which is every Draw, Impress and Calc text and no
Writer one. An over-long field is broken at **cell boundaries** through
`nextCharacters(…, SKIPCELL, …)` into an `ExtraPortionInfo::lineBreaksList`
(`impedit3.cxx`:1101-1200) — a break opportunity at *every character* — and it is neither moved
onto the next line nor offered to the break iterator at all. So *"the reference breaks a long URL
mid-token"* is not a URL rule or a hyphenation rule; it is what a field does. Established by
variant — stripping the `<text:a>` elements and keeping their text makes 26.2.4.2 draw every pitch
at 1.2 em. **Reach 665 in 156 of the 302 `.odp`** (the "485 in 107" that stood here counted
`content.xml` alone), and **closed**: `SlideTextRun.IsField`, `CellBrokenSpan` and
`PlacedLine.ContinuesField`. `0335fab9…odp` page 6 goes from four lines short to **fifteen lines
agreeing with 26.2.4.2 character for character and every baseline to 0.001 pt**.

***And the other half of it is a paint-time rule, not the one this file used to give.*** The
1.0 em pitch was recorded here as *"a field portion is the one portion kind that is never given
the fixed cell height"*. It is not: `RecalcFormatterFontMetrics` (`impedit3.cxx`:3119-3183) runs
over every portion of the line whose kind is not `LINEBREAK`, a `FIELD` included, so the line
holding a field is 1.2 em like any other. What is short is the **spill** — the lines the field
overflows onto — and the distance is `aTmpPos += MoveToNextLine(aStartPos, nMaxAscent, nColumn)`
(`:3793`, whose own comment says *"only use GetMaxAscent(), pLine->GetHeight() will not proceed as
needed"*). Two consequences the old reading does not predict, both measured on
`probes/odp-visual-r80/field-variants.py`: the distance is the **ascent**, which equals the em
only under fixed cell height — with the flag off 26.2.4.2 draws the spill 14.400 pt apart where
the ordinary pitch is 17.773 — and **the formatter never sees a spill line**, so the height that
anchors the block and that the shrink-to-fit search measures counts the field's line once and the
spill hangs out of the bottom of a middle- or bottom-anchored box.

***The `.pptx` side is the same rule and is now closed too — and the test is not the element.***
`TextRun::insertAt` branches on whether the run's own **hyperlink property map is empty**
(`oox/source/drawingml/textrun.cxx`:88) and builds a `com.sun.star.text.TextField.URL` whose
`Representation` is the run's own `a:t` when it is not (`:149-157`), imposing the theme's `hlink`
colour and an automatic underline in the same branch (`:162-168`). So **one field per `a:r`** —
a link PowerPoint split across runs is several fields and fills exactly as one does — and **an
`a:hlinkClick` that states nothing is not a hyperlink at all**: `HyperLinkContext`
(`oox/source/drawingml/hyperlinkcontext.cxx`:40-156) records a property only for a resolvable `r:id`, a `tooltip`, a
`tgtFrame`, an `action`, an `invalidUrl`, `history` off, `highlightClick`/`endSnd` on, or an
`a:extLst`, and `<a:hlinkClick r:id=""/>` — what PowerPoint writes to *clear* an inherited link —
leaves the map empty, so 26.2.4.2 draws such a run black, word-broken and not underlined.
`DrawingHyperlink.MakesField` is that test and `PptxTextBody` sets `SlideTextRun.IsField` from it.

**The reach figure that stood here counted the wrong elements.** Of the 2537 `a:hlinkClick` in the
251 `.pptx`, **1922 sit on a `p:cNvPr`** — a click action on the *shape*, not text — 7 on an
`a:endParaRPr`, which carries no run, and only **608 on an `a:rPr` in 138 documents**, of which 416
are on a slide proper in 89. **Count the parent element, not the string.**

**And the rule is far wider than "a long URL", because the cell opportunities are *added* to the
paragraph's**: a hyperlink run of any length moves the break whenever the line's boundary falls
inside it. 375 of the 416 slide hyperlink runs are under 40 characters and **117 of the 251 decks
still change**, every one of them a deck with a text-run `a:hlinkClick` and none of the other 113.
One more entry was needed for it — `lineBreaksList`'s unconditional first, `push_back(0)`, which is
what carries a whole field onto the next line when even its first cell does not fit and the line
has content (`bFieldStartNextLine`, `impedit3.cxx`:1148-1149, :1173-1180); `CellBreaks.Merge` offers the
stretch's own start for that reason.

**The three adjacent formats each answer differently, and all three were measured at the
reference.** A **`.docx`** hyperlink is *not* a field — in a paragraph or inside a `wps` text box,
whose content is a Writer fly's text and not EditEngine's, so all four probes are span-identical.
An **`.xlsx`** *cell* hyperlink is one, and the cell break fires there too (`insertHyperlink`,
`sc/source/filter/oox/worksheethelper.cxx`:1062-1080); `SheetLayout.HoldsField` already models the
painted consequence and is left as it is. A **`.ppt`** hyperlink is one as well
(`filter/source/msfilter/svdfppt.cxx`:6936, :7069-7090), and **it is now read**:
`PptHyperlinks`, `PptHyperlinkRange` and `PptTextBody`'s split. `probes/pptx-field-r82/results.md`
and `probes/ppt-autofit-r84/results.md`.

***And the `.ppt`'s condition is not in the record at all — it is whether the deck declares the
hyperlink the record names.*** `PPTTextObj` searches `SdrPowerPointImport::m_aHyperList` for an
entry whose index matches the `InteractiveInfoAtom`'s `exHyperlinkId`, and everything that makes
the run a field happens inside that loop (`svdfppt.cxx`:6907-6941); the list is built from the
`_PID_HLINKS` blob of `\005DocumentSummaryInformation` and indexed from the `ExObjList`'s
`ExHyperlinkAtom`s (`sd/source/filter/ppt/pptin.cxx`:353-547). **Three of the corpus's 51 `.ppt`
state 60 text ranges and declare no hyperlink whatsoever** — 57 of them in `BUS-Chapter 05.ppt` —
and 26.2.4.2 draws every one in the body's own colour, word-broken and unlinked. The reach figure
that stood here, *"17 of 51, 91 atoms"*, was a byte-pattern scan for record type 4063 and is wrong
in both directions: **the live record tree holds 171 ranges in 23 documents, of which 111 in 20
resolve.** A `.ppt` census that walks the stream instead of the persist directory counts orphaned
objects — on `080214-Intl-pol-frameworks…ppt` a dead `ExObjList` declares one link where the live
one declares two.

***Two more legacy-only rules came with it.*** A hyperlink's emphasis is **replaced** rather than
added to — `:7054-7056` sets the underline bit in the attribute mask and then *assigns*
`mnFlags = 1 << PPT_CharAttr_Underline`, so a bold linked run is drawn light — and a soft bullet
whose paragraph opens on a link keeps **the colour the link replaced**, not the scheme's hyperlink
slot (`:6037-6042`).

***And 26.2.4.2 declines to make a field on five decks whose ids do resolve, for a reason nobody
has named.*** Established by a one-attribute variant series on `RESPA_-_Section_8_Webinar.ppt`:
rewriting one `exHyperlinkId` to the file's first two `ExHyperlinkAtom` values makes a field and
every other value in the list does not, so the reference behaves as though `m_aHyperList` held two
entries where the file declares ten. The `_PID_HLINKS` count, the blob's declared size, the
dictionary read and the `ExObjList`'s shape are each refuted by measurement. It costs 26 reference
spans against the 56 the rule gains. `probes/ppt-autofit-r84/patch-id.py`.

**And an ODF bullet level's Private Use Area slot and its colour are both read now.**
`OdfListStyle.FormatLabel` is the *extraction* answer and puts the bullet through
`OutlineNumbers.NormaliseBullet`, which collapses a Wingdings slot to U+2022 — right for an index
and a black dot where the reference draws a green check mark. `OdfTextBody.Marker` takes the raw
`text:bullet-char` instead when the level's family has a recode table, exactly as
`PptxTextBody.Marked` does; **3598 bullet levels in 74 of the 302 `.odp` state a Private Use Area
character**, every one in the F000 block. Beside it, `fo:color` on the level's own
`style:text-properties` — with `style:use-window-font-color="true"` meaning the item's own colour
— is DrawingML's `a:buClr` and nothing read it: **22 436 bullet levels in all 302 state one.**
The picture is checked rather than assumed: a 600 dpi crop of the recoded check mark is
byte-identical to 26.2.4.2's. `probes/odp-visual-r80/`.

**`style:font-independent-line-spacing` is honoured only on the shape's `draw:text-style-name`
paragraph style.** The flag is EditEngine-wide (`SetFixedCellHeight`), so it belongs to the
shape's text and not to a paragraph inside it: the same four-slide probe with the attribute on
the paragraphs' own `text:style-name` comes back from 26.2.4.2 at ascent + descent, and moved to
the frame's text style at 1.2 em. This tree reads it from either place and is therefore more
permissive; no corpus document distinguishes them, because the exporter writes it on both. **A
hand-built `.odp` probe that puts it on the paragraph style is measuring the wrong law** — that
cost the first cut of `odp-empty-paragraph.fodp`.

**And a shadow's blur radius decides whether the shadow's *text* is real text**, which is the
sharpest example this project has of a one-attribute defect that no gate column can see and that
`|ink|%` cannot either: LibreOffice rasterises a blurred shadow
(`drawinglayer/source/primitive2d/shadowprimitive2d.cxx`:91-140) so its PDF holds a picture with
no words, while a hard shadow stays vector and its words are real and extractable. Reading the
radius wrong puts a second offset copy of every shadowed shape's text into the text layer —
visible in `pdftotext -bbox` as pairs a shadow-offset apart, and nowhere else.

**The corpus is no longer batched by complexity — it is grouped by what is wrong.** As of
2026-08-14, with 459 of 534 passing, the old ordering had stopped earning its keep: the 75
remaining failures were scattered across sixty batches, so a session taking "the next batch"
got nine documents it could learn nothing from and one it could.

```
<family>/done-NNN/      459 documents that pass the gate
<family>/<kind>-NNN/     75 that do not, grouped by what is wrong with them

  ceiling 20   pagination 20   metrics 10   extra 9
  missing 7    table 6         chart 2      unstable 1
```

Every failing document was classified by **looking at its rendered page** — six reviewers,
one fixed vocabulary, each pairing the two renderings with `page-vision` and measuring rather
than eyeballing. The kinds are defined in `.claude/skills/corpus-batches/` and the regrouping
is reproducible with `regroup-batches.py`.

Documents keep their complexity score and are ordered by it **within** each group, so
`pagination-001` is still the cheapest ten pagination failures.

**`MANIFEST.tsv` is the undo.** Batch membership is the directory layout — `batch-check.sh`
globs directories — so every stored figure naming a batch path stopped resolving when this
landed, and `dotnet/probes/` is full of archival scripts that name them. The manifest keeps
`source` untouched and gained `previous_batch`, `status` and `kind`; any old path can be
followed forward through it.

What the grouping surfaced immediately, and the old layout hid: three `ABCD-*` documents share
one bug, the two Holdover Tables were carried as sharing one bug and one 13-page gap (**they do
not — see the correction below**), three documents share
rotated cell text drawn upright a glyph per line, two share a background raster emitted after
the text, and two share a first-page header repeated on every page. Every one of those was
split across different batches before.

Some of those extensions are **upper-case on disk** — four files are `.DOC`, `.XLS`, `.XLSX`.
A case-sensitive glob quietly counts 530 instead of 534, which is the same mistake as
trusting an extension at all, in miniature. Match case-insensitively or, better, do not
filter by extension.

Per-family tracks, because a single global ordering front-loads the easy end almost
entirely with word processing and leaves the other two families idle for forty batches.
Three tracks let three workers run in parallel and never touch the same file.

**Sheets is not deferred.** It was originally scheduled last on the grounds that a
spreadsheet's value is in its cells rather than its pagination; that was retired once the
track turned out to hold the corpus's largest systematic defects — one workbook paginating
1170 pages against 220 — so deferring it was hiding them rather than deprioritising them.
All three tracks now advance in parallel and never wait for one another.

```sh
.claude/skills/corpus-batches/scripts/batch-check.sh /c/sandbox/workdir/sample-files 'words/batch-003' out 3
.claude/skills/corpus-batches/scripts/batch-check.sh /c/sandbox/workdir/sample-files 'words/batch-00[1-2]' out 3
```

**Both of those runs are the workflow, and the second is not optional.** Make the current
batch match, then re-prove every earlier batch in the track. This is the cascade rule
again in corpus form: a fix aimed at batch *n* routinely breaks batch *n−4* in a way that
looks nothing like the change, and advancing on the first condition alone is how a corpus
rots from the front.

**Set `SOURCE_DATE_EPOCH` when comparing two renderings byte for byte.** Reach is measured by
rendering a track twice and diffing, and a document that prints the date — a spreadsheet header
holding `&D` or `&T` — draws different ink on a different day. Measured on the sheets track:
rendering all 171 twice in succession is byte-identical once `/CreationDate` is masked, and
rendering them a day apart moves **17 of 171**. `xray-office render` honours the
reproducible-builds convention (seconds since the Unix epoch, read as UTC) in both the PDF's
`/CreationDate` and the header fields, so with it set two runs are byte-equal with nothing masked
at all. Leave it unset for ordinary rendering; a printout's date is meant to be today's.

**The signature of a forgotten pin is a *total*, and it reads as catastrophe.** The figure above
— 17 of 171 — is what a day's drift costs on a track where only some documents print a date.
Forget the pin on a track where the difference is the PDF's own `/CreationDate` and *every*
document moves: round 111's first, unpinned sweep of the chart track reported **176 of 176
moved**, with every page count and every alphanumeric count identical on both sides; pinned, the
same sweep reported **0 of 176**. So a reach figure equal to the whole set is not a finding, it
is a missing `SOURCE_DATE_EPOCH` until proven otherwise — and the tell is exactly that pairing,
everything moving while nothing the gate scores does.

**`TODO.raster-ceiling.md` lists 37 pages the word gate cannot win.** LibreOffice rasterises
an embedded object on those, so its PDF holds a picture where ours holds real searchable text —
ours is the better output and `wc -w` scores it as failure. An embedded metafile is the
commonest cause and not the only one: 16 of the 37 are on documents holding none. Check that
list before working any word-count failure; several agents have each re-derived it the hard way.

**And `TODO.word-parity.md` lists the rows the gate cannot win for the opposite reason: places
where 26.2.4.2 is wrong about the document, Word is right, and this project follows Word.** Each
entry is measured, each has an environment variable that switches the reference's behaviour back
on, and each names the corpus rows it costs. The first of them is a `TOC \t` switch voiding a
built-in heading's direct paragraph formatting, which is the whole of why
`24-25_FAA_Holdover_Tables.docx` prints 154 pages against the reference's 155 while sitting within
51 characters of it. Check that list too before working a words-track row that will not close.

The `corpus-batches` skill holds the rest — why the ordering and the batch size are what
they are, what parity does and does not prove, and what a dispatch brief for a parallel
agent has to contain. `TODO.batches.md` is the scoreboard.

Verify the environment before trusting any comparison:

```bash
.claude/skills/libreoffice-reference/scripts/check-env.sh
```

## This container — read before reproducing any stored figure

The project has moved containers, and two of the three things a measurement depends on are
not what the stored figures were taken against. Neither is a defect in the tree.

**Roots have moved again, and `/c/sandbox` does not exist here.** As of 2026-09-06 the primary
checkout is `/home/user/libreoffice-core`, agent worktrees are `/home/user/wt-*`, and the corpus is
`/home/user/sample-files`. The `grep -r` doubling described below does **not** occur on that corpus:
`find` and `git ls-files` both count 963. Check `pwd` before pasting any stored path.

**Roots.** The repository is at `/c/sandbox/workdir/libreoffice-core` and the corpus at
`/c/sandbox/workdir/sample-files`. The live scripts and documents have been rewritten to
these. The archival probe scripts under `dotnet/probes/` and `dotnet/research/probes/` have
**not** been, deliberately — they are the record of what a given round actually ran, and
rewriting them would falsify it. A `/workspace/sample-files` symlink points at the corpus so
they remain runnable as written.

**That is no longer this container, and the direction has reversed — check before you measure.**
As of 2026-09-04, `/usr/bin/soffice` is **24.2.7.2** and 26.2.4.2 is present only as the TDF tarball
under `/opt/libreoffice26.2`. So `batch-check.sh`, `ref-baseline.sh` and every ink figure taken here
are measured against **24.2**, while the tree is calibrated to **26.2** — the paragraph below, and
several stored figures, assume the opposite. One line settles which you have:

```sh
soffice --version                       # LibreOffice 24.2.7.2 420(Build:2)
/opt/libreoffice26.2/program/soffice --version
```

**Knowing this is not enough — a sweep takes the reference from `PATH` and says nothing.** Round 75
read the paragraph above, launched a 1285-document sweep of the converted corpus without pointing
`PATH` at the tarball, and got a table that looked entirely normal: `.odp` 285 of 302, `.ods` 188,
`.odt` 260, `.rtf` 238. It was scored against 24.2.7.2 and is not comparable to any figure a round
has quoted, all of which are against 26.2.4.2. Three hours, discarded. **The wrong reference does
not fail; it answers a different question fluently.**

`batch-check.sh` no longer takes it silently. It resolves `$REF_SOFFICE`, falling back to `PATH`,
and prints the resolved path *and version* beside the `measuring <CLI>` line, so the run announces
which of the two it is:

```sh
REF_SOFFICE=/opt/libreoffice26.2/program/soffice \
  .claude/skills/corpus-batches/scripts/batch-check.sh <root> '*' <outdir> 3
# measuring .../XRay.Office.Cli
# reference /opt/libreoffice26.2/program/soffice -- LibreOffice 26.2.4.2 0229ac93...
```

Read that second line before you read the table. A stored sweep that does not record it cannot be
attributed to a reference at all.

**A gate run under CPU contention undercounts, and it undercounts on the REFERENCE side — which
looks exactly like our regression.** Measured 2026-09-08: the same corpus at two commits scored
`.ods` 234 then 225, and the nine lost rows were eight `ref-failed` plus one `ours-failed`. Nothing
had regressed; three rounds were building and sweeping in their own worktrees and the reference's
renders hit the 240 s bound. `REF-CANNOT-RENDER` went 2 → 13 in the same run and is the tell.

So **never compare two sweeps on their match totals alone.** Exclude every row that failed on
either side in *either* run and compare what is left:

```sh
awk -F'\t' '$7 ~ /failed/{print $1}' A/rows.tsv B/rows.tsv | sort -u > /tmp/bad
awk -F'\t' 'NR==FNR{bad[$0];next} !($1 in bad) && $7=="match"' /tmp/bad A/rows.tsv | wc -l
```

A row that is `match` in one run and `ref-failed` in the other is evidence about the box, not
about the tree.

***And the cure is the bound, not patience: every one of those rows renders in seconds.***
Round 88 took `probes/odf-gate-r80`'s eleven `.ods` `ref-failed` documents and rendered each one
**alone** through the same 26.2.4.2 with a 900 s bound: **eleven of eleven succeeded, in one to
three seconds each**, the largest being a 163-page, 189 068-glyph workbook at two seconds. Its
whole-column re-sweep at `RENDER_TIMEOUT=900` then reported `REF-CANNOT-RENDER 0` over 307
documents, and the two `ours-failed` rows came back as matches as well. So a `ref-failed` row is
**not evidence that the document is slow** — it is a wedge, and a wedge is not proportional to
the work. `probes/ods-track-r88/batch-check-tmo.sh` is `batch-check.sh` with the two hard-coded
240 s bounds behind `$RENDER_TIMEOUT`; raise it when the box is busy rather than banking the
failures. **The cost of not doing so is a whole track's headline figure**: that column was
carried as *225 of 307, the worst track we have* on the strength of r80, and measured **269 of
307** at the same commit, with the residual 38 rows rather than 82.

**Before you rebuild, check the sweep is finished — and check it with a file, not a clock.** The
gate measures `dotnet/tools/XRay.Office.Cli/…/XRay.Office.Cli` in the tree it runs from, so a rebuild
mid-sweep swaps the binary and the rows either side describe different programs. On 2026-09-07 a
merge was built in this tree at 22:37 against a sweep started at 21:16; it survived only because
every render had already finished, which was luck. The two facts that settle it:

```sh
ls -t <outdir>/ours/*.pdf | head -1 | xargs stat -c %y   # newest render
stat -c %y dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
```

If the newest render is older than the binary, every row predates the rebuild and the run is
clean. `pgrep` alone is not enough — a finished sweep can leave a process behind, and a live one
can be between documents. And a completed run is still worth this check, because the contamination
is invisible in the table it produces.

**And `timeout 240 soffice` never bounded `soffice`.** `soffice` execs `oosplash`, which *ignores
SIGTERM*, and GNU `timeout` without `-k` sends SIGTERM once and then waits forever. On
`sheets/done-016/ods/STC_WebList.ods` the reference render hung for **87 minutes** with its
`soffice.bin` child defunct and `oosplash` refusing to reap it, holding one of three workers; the
other two had already drained their share, so the sweep would never have finished and looked merely
slow the whole time. Both render lines now use `timeout -k 30 240`. If a sweep stops appending
rows, look for an `oosplash` older than the timeout before assuming contention:

```sh
ps -eo pid,etimes,args | grep [o]osplash | awk '$2 > 300'
```

**A divergence from the gate is therefore not automatically a defect**, and one round has already
been spent finding that out. The seven `Printable_Graph_Paper_Template` documents sat at 32-to-51
first-page ink on a row pitch a fraction of a point out; we match 26.2.4.2's pitch **to the twip**
and 24.2.7.2 has no `MinRowHeightInclBorder` at all. Before working a difference, render the
document through both binaries — `probes/words-row-height/results.md` is the worked example, and
this is checkable without rendering anything:

```sh
strings /usr/lib/libreoffice/program/libswlo.so | grep -c MinRowHeightInclBorder   # 0
strings /opt/libreoffice26.2/program/libswlo.so | grep -c MinRowHeightInclBorder   # 1
```

Read `Installing a specific LibreOffice` below before treating the tarball as the target: it is a
fourth reference rather than the distro-packaged 26.2 the tree is really calibrated to, because it
bundles its own fonts. Move the 33 duplicates aside first and it is close enough to screen with:

```sh
D=/opt/libreoffice26.2/share/fonts/truetype
mkdir -p $D/.duplicates-aside && mv $D/{Carlito,Caladea,Liberation,DejaVu}*.ttf $D/.duplicates-aside/
# And the Latin Noto, which duplicates nothing installed and is the worse trap of the two:
mkdir -p $D/.noto-aside && mv $D/Noto{Sans,Serif}-*.ttf $D/.noto-aside/
# And, as of 2026-09-07, the eight DejaVu Condensed faces -- the FIFTH confound:
mkdir -p $D/.condensed-aside && mv $D/DejaVu*Condensed*.ttf $D/.condensed-aside/
fc-cache -f

**The fifth one is the sharpest, because almost nobody asks for it by name.** The tarball ships
eight `DejaVu*Condensed` faces and this system has **none** of them installed (`fc-list | grep -c
'DejaVu.*Condensed'` = 0), so the reference could reach a face we cannot. Found by round 77 while
measuring the `.rtf` column: its reach is **39 of 336 `.rtf` and 130 documents across the converted
corpus** -- and **33 of those 39 name no condensed family at all**. They are being chosen as a
*fallback*, which is why four rounds of font work never noticed them: a confound you can find by
grepping the corpus for a family name is the easy kind, and this is the other kind. Same shape as
the `LiberationSansNarrow` confound, wider.

**What moving them aside invalidates.** Every figure taken before 2026-09-07 was measured with them
present, including `probes/odf-gate-r76/` and `probes/orig-gate-r76/`, whose `results.md` both say
"all four tarball confounds moved aside" and now mean four of five. Those sweeps remain valid as the
measurements they were; they are simply not reproducible in this container any more. **Do not move a
font while a round is live** -- every round measures against a reference bank taken with the fonts
as they stood, and changing them mid-flight silently rewrites the control.
```

### The sixth confound is not a font: `TODAY()` is recalculated on load

The reference recalculates volatile formulas when it opens a file; we render the cached value
the file was saved with. So on any document containing `TODAY()` or `NOW()` the reference prints
*today's* date and we print the date of the last save, and every derived cell moves with it --
`065_Weight_loss_tracker` shows `04/08/26 Tuesday` on the reference against `08/21/22 Sunday`
on ours, weekday included.

**Ten of the fourteen `chartset` failures in `probes/orig-gate-r83` carry one.** Census a
suspect row before reading its glyph delta:

```sh
unzip -p FILE 'xl/worksheets/*.xml' | grep -o 'TODAY()\|NOW()' | wc -l
```

This confound is worse than the font ones in one specific way: it is **not reproducible**. The
five font confounds give the same wrong number every run, so a before/after comparison still
cancels them. A volatile date gives a *different* number tomorrow, so two sweeps taken on
different days disagree on these rows for no reason connected to any change. Never attribute a
delta on such a row to a patch without re-measuring both sides the same day.

**Move the Latin Noto aside too, and leave the script-specific Noto in place.** The line above
was written for the metric-compatible duplicates, and it is not sufficient. The tarball also
ships `NotoSans-*` and `NotoSerif-*`, which duplicate *nothing* on this system, so they are not
caught by that `mv` and they become fontconfig's answer for every unfiled family — which makes
`ink26` unscoreable on any document naming a font the system lacks. Two agents lost hours to it
independently in one session before anyone read the faces out of the PDFs. Keep
`NotoSansArabic`, `NotoSerifHebrew` and the rest: they carry script coverage the system genuinely
lacks (`fc-list :lang=ar` here answers DejaVu Sans Mono), and removing them changes what a CJK or
Arabic document can draw at all.

With only the eight Latin faces aside, the tarball answers **DejaVu**, like a distro build.

**And there is a third face to know about, which is neither a duplicate nor a Noto:
`LiberationSansNarrow`.** As of 2026-09-06 the four `LiberationSansNarrow-*.ttf` are still in
`/opt/libreoffice26.2/share/fonts/truetype` — the `mv` above matched them but they are back, or were
never moved — and the system carries **no narrow face at all** (`fc-list | grep -i narrow` is
empty, `fc-match "Arial Narrow"` answers DejaVu Sans). So the tarball is the only stack on this
machine that can resolve *Arial Narrow*, and on a document naming it the tarball's 26.2.4.2 lays out
in a face **36% narrower** than anything else here: every line that fits for it wraps for everyone
else, and the page is not comparable line for line.

Measured on `words/done-013/doc/omrIMInterpretiveGuideLine.doc`: the tarball embeds
`LiberationSansNarrow` and `LiberationSansNarrow-Bold`, while **24.2.7.2 and X-Ray.Office both embed
DejaVu Sans and DejaVu Sans-Bold** and agree with each other, line for line, to **0.1 pt on 30 of
the 31 lines of page 1**. Against the tarball the same page disagrees everywhere from the title
down. **49 of the corpus's documents name a Narrow family** — sheets 25, words 22, slides 2 — so
this is not a curiosity. On any of them, screen against `/usr/bin/soffice`, not against the tarball;
the version rule the section above describes is not what separates them.

**So move them aside too, and do not bundle a Narrow face to match.** They belong in the same
`.duplicates-aside/` as the rest — the recipe's `mv` pattern does catch `Liberation*`, so a fresh
run of it is enough; check afterwards that `ls $D/LiberationSansNarrow*` is empty, because these
came back once already.

***Done 2026-09-07.*** All four are in `.duplicates-aside/`, which now holds 38 faces, and 71
remain in the directory. Confirmed on the witness: 26.2.4.2 renders
`omrIMInterpretiveGuideLine.doc` in **DejaVu Sans and Liberation Sans**, with no narrow face
anywhere — *Arial* still reaching Liberation Sans, which is right, and *Arial Narrow* now reaching
DejaVu Sans, which is what the system and X-Ray.Office both answer. **Any 26.2 figure taken on one of
the 49 Narrow-naming documents before this date is suspect** — but *only* those 49, and the blast
radius is smaller than it sounds. Measured immediately afterwards on the `.odp` column of
`probes/odf-gate-01/`: a freshly rendered reference reproduces the banked reference **302 of 302**
on pages and glyphs, and our half at the same commit reproduces the banked verdict 302 of 302. So
**re-render the Narrow-naming documents you actually work on and leave the rest of the bank
standing** rather than discarding it.

The reason **not** to answer this by bundling a narrow face is rule 3's own test, *ship only the
faces the distro packages ship*: Liberation Sans Narrow is not in `fonts-liberation2`, it is the
separate `fonts-liberation-sans-narrow` package, and nothing in this project's stated environment
installs it. `XRay.Office.Text/Fonts/Bundled/` holds 28 faces and no Narrow, the system holds none,
and both answer DejaVu Sans for *Arial Narrow* — so **we agree with a stock machine and the tarball
is the outlier.** Bundling one would be the same mistake as the first cut of the bundle, which
shipped TDF's fuller DejaVu and made us draw a real italic where the reference synthesises a lean.

### The seventh confound: on the draw layer 26.2.4.2 measures in one face and draws in another

**A slide's text can be laid out at one font's advances and painted with another's, and it is a
LibreOffice defect rather than anything to reproduce.** The declared family class survives into the
*measurement* and is thrown away before the *drawing*:
`drawinglayer::attribute::FontAttribute` (`include/drawinglayer/attribute/fontattribute.hxx`) has
fields for the family *name*, weight, italic, symbol, vertical, outline and **monospaced** — and
none for the family class or the charset — so `getVclFontFromFontAttribute`
(`drawinglayer/source/primitive2d/textlayoutdevice.cxx`:416-448) rebuilds the font at
`FAMILY_DONTKNOW`, while the DX array the primitive carries was measured by editeng with the class
still on it. `VclProcessor2D` then draws the class-less face at the class-ful face's advances
(`drawinglayer/source/processor2d/vclprocessor2d.cxx`:485-491).

The one-line test, which needs no LibreOffice at all:

```sh
fc-match "Helvetica:bold"        # LiberationSans-Bold.ttf   <- what 26.2.4.2 DRAWS
fc-match "Helvetica,sans:bold"   # DejaVuSans-Bold.ttf       <- what 26.2.4.2 MEASURES
```

**When those two agree there is no confound** — install `urw-base35` and both answer Nimbus Sans;
remove Liberation and both answer DejaVu. So the effect is a property of this container's
fontconfig graph, and a rule fitted to the gap between the two answers is fitted to
`/etc/fonts/conf.d`. It is also *silent*: both sides draw the same characters, so no gate column
moves, and the only symptom is a line that wraps a word early and a title that looks tracked out.
Round 90 filed exactly that as a letter-spacing defect before correcting it.

**Reach 101 of 803 zip corpus documents — slides 99, sheets 2, words 0 — and 91 of them are
`Helvetica`.** Writer body text cannot show it: `SwTextPainter` never becomes a drawinglayer
primitive, which is why the 24-of-24 agreement in the next section stands. `probes/title-font-r92/`
has the census, fourteen one-attribute variants, and why no code changed.

***And it reaches the LEGACY BINARIES, which that census could not see — a `.ppt` states the same
class in the top nibble of `lfPitchAndFamily`.*** `ReadPptFontEntityAtom`
(`filter/source/msfilter/svdfppt.cxx`:410-435) maps `FF_ROMAN`/`FF_SWISS` to
`FAMILY_ROMAN`/`FAMILY_SWISS`, `:2186` puts it on the `vcl::Font`, and from there it is the same
`FontConfigManager::Substitute`. The 101-of-803 figure is a **zip** census, so the whole
`.ppt`/`.doc`/`.xls` track was invisible to it — the same trap this file already records for
charts. Measured rather than censused, by rendering all **51** legacy PPT-family corpus documents
twice with 26.2.4.2, as authored and with every `FontEntityAtom`'s family nibble cleared and
nothing else changed: **20 of 51 state** a splitting class-ful family, and **3 of 51 have it reach
drawn text** — `architecture6.ppt`, `RRM-training-syllabus-…` (O15's own witness, which the
instrument rediscovered without being told) and `pres_ioc_phuket.ppt`, which is new. The other 48
do not move by a thousandth, three of them from a *high* `TJ` level that is justification and
kerning — which is why the discriminator has to be run as a **difference** and never as a level.

***And a `.ppt` cannot state character spacing, so do not go looking for a record.***
`PPTStyleTextPropReader::ReadCharProps` (`svdfppt.cxx`:5096-5185) reads the whole of a
`TextCFException` and there is no tracking field in it and no `PPT_CharAttr_*` constant for one
(`include/filter/msfilter/svdfppt.hxx`:1416-1428). O57 stood for three rounds as *"the reference
draws the same string 3.3 % wider"* and then as a possible spacing record; it is this confound, and
it is worth 268 → **52** run differences and 322.07 → **1.60 pt** worst origin shift over
`architecture6.ppt`'s 31 pages when one byte of the file removes it. `probes/ppt-spacing-r115`.

### The two references differ in a *rule*, not only in their fonts, and it decides font fallback

**24.2.7.2 lets the family name decide; 26.2.4.2 lets a declared family class beat it.**
`FontConfigManager::Substitute` appends `"serif"` as a second `FC_FAMILY` for `FAMILY_ROMAN` and
`"sans"` for `FAMILY_SWISS` (`vcl/unx/generic/font/fontconfig.cxx`:1075-1088). **That switch does
not exist in 24.2.** Measured on three hand-built DOCX naming one uninstalled family, differing
only in what `word/fontTable.xml` declares, with the Latin Noto aside:

| `w:family` | 24.2.7.2 | 26.2.4.2 | X-Ray.Office |
|---|---|---|---|
| *(no font table)* | DejaVu Sans | **DejaVu Serif** | DejaVu Serif |
| `roman` | DejaVu Sans | **DejaVu Serif** | DejaVu Serif |
| `swiss` | DejaVu Sans | **DejaVu Sans** | DejaVu Sans |

24.2 answers the bare `fc-match` of the name in all three; 26.2 honours the declaration, and a
DOCX with no font table still inherits Writer's roman default. Over 24 families the tree matches
clean 26.2 **24 of 24** and 24.2 only 7 of 24.

**So a font-family divergence measured against `/usr/bin/soffice` is very probably not a defect.**
A `pdffonts` census over the 947 gate renders showed **119 documents** drawing a different family
from the reference, 85 of them `DejaVu Sans` against our `DejaVu Serif` — and every one of those
85 is this version rule, not a bug. A round was dispatched to "fix" it and would have broken
correct behaviour; the agent challenged the brief from the source and was right. Read the faces
out of both PDFs, with the Latin Noto aside, before believing any of it.

**Measured over the whole gate, not a sample: 49 of the 87 mismatches — 56% — are this.** Round 66
screened every one against 26.2.4.2 before touching anything
(`probes/mismatch-classify-01/`): 49 the version gap, 7 the two references disagreeing with each
other, 7 the raster ceiling, 6 a cell holding `TODAY()`, and **no group of three sharing a cause**
in what is left. The largest movers are outright: `sectors-defense-and-aerospace.xlsx` is 449 pages
ours, 227 on 24.2 and **449 on 26.2**; `A_320.doc` 118/150/**118**; `CIS_Debian…xls` 88/109/**88**.
The two Holdover Tables, carried above as sharing a 13-page gap, are **page-exact against 26.2**
(155/155 and 167/167) and within 0.4% on glyphs.

**And the trap inside that: 21 of the 49 are sheets where 24.2 draws far more text than we do, with
our output carrying truncated fragments** — which reads exactly like "we clip cell text we should
spill" and is not. `essd-16-3433-2024-t02.xlsx` is the witness: our pages 2–4 are empty, 24.2's
carry an overflowing column's continuation, and **26.2 gives 2349 glyphs against our 2346**. A round
that takes that family on against the gate's own reference will implement 24.2 behaviour and
regress the tree.

**Screen a document against 26.2 before working it.** `probes/words-version-screen/screen.py` does
the whole queue and `bucket.py` one catalogued cause. Rescoring the worst thirty words documents
that way put **eleven of them** — the whole top of the table — in the version gap rather than in the
tree, three of them under 3.5 ink against the target; and it cleared two of the nine documents
catalogued under *overlap and clipping*, including the one carried as rendering blank, which matches
26.2 at **0.00**. It cuts the other way too: two of those nine paginate differently under 26.2 than
under both 24.2 and us, so where the references disagree with each other the document needs reading
rather than scoring.

### The same rule decides *glyph* fallback, and `fc-match` on a bare charset does not

**`FontConfigManager::Substitute` is one function and the glyph-fallback hook goes through it too.**
`FcGlyphFallbackSubstitution::FindFontSubstitute` calls it with the missing characters as an
`FC_CHARSET` (`vcl/unx/generic/font/fontsubst.cxx`:173-184), so the declared class appends `serif` or
`sans` to *that* pattern as well — and since `FC_CHARSET` outranks `FC_FAMILY`, the answer is **the
first face on that one generic's `<prefer>` list that covers the character**. Measured on 26.2.4.2
over six declared classes and thirteen characters (`probes/fonts-r64/gen-generic.py`, one DOCX per
cell, faces read out of the PDFs):

| character | roman / modern / script / decorative / undeclared | swiss |
|---|---|---|
| `U+2713` ✓ | **FreeSerif** (69-unifont's serif list) | **DejaVu Sans** (60-latin's sans-serif list) |
| `U+2011` non-breaking hyphen | **DejaVu Serif** | **DejaVu Sans** |
| `U+4E00` 一 | WenQuanYi Zen Hei | WenQuanYi Zen Hei |
| `U+2714` ✔, `U+2611` ☑, `U+263A` ☺ | Noto Color Emoji | Noto Color Emoji |

Only `swiss` differs, which is the same switch as for family substitution — and *undeclared* behaves
as roman because Writer's own pool default is roman, so a word-processing document lands on the serif
list unless its font table says otherwise.

**That table is the *western* item's, and two of its rows are not.** `U+4E00` is an East Asian
character and selects the CJK item whatever the class; the emoji rows are a language rule. Read the
next section before using it: a character's own script decides which of Writer's three font items
answers for it, and only the western one reads the declared class at all.

**So `fc-match ":charset=XXXX"` is not the question LibreOffice asks.** Asked bare it answers DejaVu
Sans for every one of the characters above, because `49-sansserif.conf` appends `sans-serif` to a
pattern that named no generic — which is the *swiss* row, not the common one. `fc-match
"Calibri,serif:charset=2713"` answers FreeSerif and `fc-match "Calibri,sans:charset=2713"` answers
DejaVu Sans; the bare form answers the second. A round has already been misled by this: the previous
fonts round's probe read `fc-match ":charset=25cf"` and concluded DejaVu Sans, which was right only
because its witness was a `.pptx`.

**The emoji row is a language rule, not a family one.** `getExemplarLangTagForCodePoint` answers
`und-zsye` for a character with the Unicode `Emoji` property (`fontconfig.cxx`:1026-1029) and
fontconfig scores `PRI_LANG` above `PRI_FAMILY_WEAK`, so an emoji code point goes to the emoji face
whatever generic the pattern named — `U+2714` answers Noto Color Emoji under all six classes although
FreeSerif holds it and is on the serif list. `U+2713`, which the property excludes, does not.

### A colour bitmap glyph is a Type 3 font, and `GlyphOutlines` was never in that path

**The emoji row above is only useful if the face can be painted, and until round 65 it could not
be.** Noto Color Emoji carries `CBDT`/`CBLC` and **neither `glyf` nor `CFF `**, so the PDF writer
embedded a `glyf`-less TrueType program, announced it as one, and drew a blank at exactly the right
advance; every gate the corpus harness has passed while it did. **A blank is worse for a reader
than a wrong glyph**, so this is closed rather than recorded.

**It was not `GlyphOutlines`.** That reader is `glyf`-only and is reached *only* by Fontwork
(`Ooxml/DrawingML/FontworkFitting.cs`); text never touches it. The two seats were
`PdfFontCatalogue`, which mis-described the program, and `SkiaDrawingSink.DrawOutlines`, which asks
*Skia's* `SKFont.GetGlyphPath` and gets an empty path. Widening `GlyphOutlines` would have fixed
nothing.

**What LibreOffice writes, measured on 26.2.4.2's own PDF of a `U+2714` probe:** a
`/Subtype/Type3` font with `/FontMatrix[0.001 0 0 0.001 0 0]`, a `/CharProcs` keyed by glyph, an
`/Encoding /Differences` naming them, a `/ToUnicode`, **no font program**, and one char proc per
glyph reading `1245.1171875 0 d0` then `q … cm /Im12 Do Q` over a `/DeviceRGB` image with an
`/SMask`. `pdffonts` says *Type 3, Custom, emb yes, uni yes* and `pdftotext` gives the character
back. Ours now writes that shape, the deflated colour plane byte-for-byte the same length, and the
page's content stream is untouched — **the text layer is what keeps a colour glyph searchable, so
the bitmap goes inside the font rather than beside the text as a picture.**

**The placement is `round(pixels × upem / ppem)` per side, and rounding rather than truncation is
measured, not assumed.** Noto is 2048 upem with one 109 ppem strike of 136 × 128 pixel glyphs at
`bearingY 101`; the reference's three constants are `2555`, `2405` and `−507` design units, and
2555.30 rounds down while 2404.99 rounds up. See `probes/colour-r65/results.md`.

**`COLR`/`CPAL` is deferred and `sbix`/`SVG ` with it, because the census says so.** Of 150
installed faces, 120 are `glyf`, 29 are `CFF `, **one** is `CBDT`/`CBLC` and **none** carries
`COLR`, `sbix` or `SVG `. Nothing on this machine can render a page to measure a layer composition
against. In its absence such a face is reported unpaintable and the fallback search moves to the
next candidate, which draws a monochrome glyph — visible, and wrong, rather than absent.

**That fall-through is the floor and it sits in `SystemFontResolver.Covers`**, which all three
fallback stages go through. It changes no preference: a candidate is only ever skipped, never
promoted, so the advance follows whichever face actually draws. It fires nowhere on this machine.
**`CFF ` counts as paintable on purpose** — the rasteriser draws it and only the PDF writer declines
to embed the program (`PdfFontCatalogue.IsCompactFontFormat`), so rejecting it here would move a
line break to work around a writer.

**The corpus reach is two documents and it predates the round that reported it.** Of 947,
**two** draw a character landing on the colour face — `019_Free_Blood_Sugar_Chart…xlsx` (six
distinct emoji) and `jobs-bulletin-51-22-december-2025.xlsx` (one) — and both appear in
`probes/fonts-r64/faces-before.tsv` as well as `faces-after.tsv`, so `fonts-r64` created the
*probe*'s blank and not the corpus's. **One** document reaches a `CFF ` face (Unifont, on
`vvsummit2022-Research-Roadmap…pptx`), where the reference draws `NotoSansArmenian-Regular` — a
resolution difference, not a painting one.

**The residual this did not close is the script-specific font item, and the section below settles
it.** The reading recorded here — that a CTL or CJK run takes an item with *its own family and its
own class* — was wrong: the class never reaches those two items at all. What decides them is the
item's own language.

### The script-specific font item decides it, and the deciding half is the *language*

**Writer keeps three character-font items and selects one per script item of the text**, and only the
western one behaves the way the two sections above describe. `SwScriptInfo::WhichFont` maps
`i18n::ScriptType` onto `SwFontScript` (`sw/source/core/text/porlay.cxx`:879-901); a **weak**
character — every symbol, dingbat, arrow and punctuation mark — takes the script of the text around
it, or the one `w:rFonts/@w:hint` names, and nothing else can move it
(`i18nutil/source/utility/scriptchangescanner.cxx`:246-268, `DomainMapper.cxx`:969-988).

**The class never reaches the other two items at all.** `LN_CT_Fonts_ascii` inserts
`PROP_CHAR_FONT_FAMILY`; `LN_CT_Fonts_eastAsia` and `LN_CT_Fonts_cs` insert the *name* and nothing
else (`sw/source/writerfilter/dmapper/DomainMapper.cxx`:436-508). So the CJK and CTL items keep the
pool default's family type, and `OutputDevice::GetDefaultFont` sets `FAMILY_SYSTEM` for `CJK_TEXT`
and `CTL_TEXT` — *"don't care, but don't use font subst config later…"*
(`vcl/source/outdev/font.cxx`) — which appends no generic to the pattern at all.

**And each item carries its own language, which outranks the generic's preference list.**
`SwDoc::SwDoc` resolves the document's three default languages through
`MsLangId::resolveSystemLanguageByScriptType` (`sw/source/core/doc/docnew.cxx`:383-398), which
answers `LANGUAGE_ENGLISH_US`, **`LANGUAGE_CHINESE_SIMPLIFIED`** and **`LANGUAGE_HINDI`**
(`i18nlangtag/source/isolang/mslangid.cxx`:135-165). `Substitute` puts it in the pattern as
`FC_LANG` (`fontconfig.cxx`:1092, 1118-1119) and `fcmatch.c` scores `PRI_LANG` above
`PRI_FAMILY_WEAK`. `mapToFontConfigLangTag` then reduces the tag to what `FcGetLangs()` knows:
`hi-IN` is not a member and `hi` is, `en-US` is not and `en` is, `zh-CN` **is**.

Measured on 26.2.4.2, one DOCX per cell, faces read out of the PDFs
(`probes/fonts-r65/gen-scriptitem.py`, 25 cells, **25/25** reproduced by the tree):

| run | 26.2.4.2 draws | the pattern that explains it |
|---|---|---|
| western, `U+2610` | FreeSerif (DejaVu Sans under `swiss`) | `Calibri,serif:lang=en:charset=2610` |
| `w:hint="eastAsia"`, `U+2610` or `U+2713` | **Unifont** | `Calibri:lang=zh-cn:charset=…` |
| complex, `U+05D0`, or `w:hint="cs"` `U+2610` | **FreeSans** | `Calibri:lang=hi:charset=…` |
| complex, `U+0E01` or `U+0627` | **FreeSerif** | `Calibri:lang=hi:charset=…` |
| asian, `U+4E00` | WenQuanYi Zen Hei | `…:lang=zh-cn:charset=4e00` |

**The declared class moves none of the CJK or CTL rows** — `roman`, `swiss` and no font table at all
give the same answer — which is what the paragraph this replaces got wrong when it said those items
have "their own family and their own class". They have their own *language*, and the class is simply
absent.

**A document that states `w:lang` overrides those defaults, and Word writes one into `docDefaults`
for nearly every file.** `<w:lang w:val="en-US" w:eastAsia="en-US" w:bidi="ar-SA"/>` is what both
`150-5370-10H.docx` and `AWR OPS-AOC 044…docx` carry, which is why their `w:hint="eastAsia"` runs
answer **DejaVu Sans** and not the Unifont a document stating no language gets. Round 64 measured the
answer and inferred the wrong cause from it.

**A face's language support cannot be read from the configuration this tree parses** — fontconfig
derives it from an orthography per language compiled into the library — so `FontLanguages` models it
as coverage of one exemplar character of the language's script. Checked against `fc-list :lang=X`
over 25 languages: **24 agree face for face**, and the twenty-fifth (Gurmukhi) names two fewer.
Two exemplars are deliberately not the first letter of their alphabet, because the first letter does
not discriminate: an accented Greek vowel excludes a face carrying only the mathematical Greek, and a
simplified-only Chinese ideograph excludes a Japanese face carrying only the shared ones.

**And the pattern carries a *set* of characters, not one.** `ImplGlyphFallbackLayout` gathers every
unmapped code unit of a layout into one `OUString`; every code point of it goes into one `FcCharSet`;
and `FcCompareCharSet` scores by *how many of the set the candidate is missing*, at `PRI_CHARSET` —
fontconfig's highest priority, above both the family list and the language. The chosen face is then
subtracted from the set and the next fallback level asks with the remainder
(`vcl/source/outdev/font.cxx`, `fontconfig.cxx`:1229-1245). So a face further down the family list
wins when it covers more of the run.

**The generic must travel with the *run*, not be recorded against the face it resolved to.** Round 64
recorded it against the face, first writer winning — and in a word-processing document the first
request to reach a face is the paragraph mark's, so a run on any other item silently took the
paragraph's. It hid the swiss row as well as the script items: `west-swiss-2713` answered FreeSerif
until the item was passed in, because the mark's own Calibri had already claimed Carlito for `serif`.

**That is also why round 64's stored 65/72 is 64/72.** Re-measured at `260611dae`, its own probe
agrees on 64 cells: the residual is Hebrew under all six declared classes plus `swiss__2713` and
`swiss__27A2`, and *not* the "Thai under swiss" it named — `swiss__0E01` agrees on both sides by the
accident that the complex item's Hindi answer for Thai and the western serif list's answer are the
same face. All three defects above are closed and both probes now agree 25/25 and 72/72; over round
64's own fourteen movers the corpus face-set distance goes **19 → 16**, and 4 of the 947 move.

---

**The reference binary is `26.2.4.2`, not the `24.2.7.2` every stored figure was measured
against.** *(Written of an earlier container; see the correction directly above.)* The base image is
Ubuntu 26.04 and its archives offer no earlier LibreOffice.
This is not a nuance to note and move past — ground truth genuinely moved, measured over the
whole corpus by re-rendering the reference half of the gate at both versions:

| track | reference page count changed | total \|Δ\| pages | reference words beyond the 2% band |
|---|---:|---:|---:|
| words | **47 of 200** | 453 | large |
| slides | **0 of 163** | 0 | 160 of 163 moved at all |
| sheets | **16 of 171** | 305 | large |
| total | **63 of 534** | 758 | **210 of 534** |

So **the 465/534 scoreboard is not reproducible here**, and the §7 rule "if your baseline
sweep does not reproduce the briefed numbers, stop" would fire on almost every round. It has
to be re-baselined against 26.2.4.2 before any verdict movement means anything. Slides is the
exception worth knowing: a deck's page count is its slide count, so check 1 is structurally
stable there and slide-count claims survive the version change intact.

**The table above is confounded, and the correction is the more useful fact.** Two things
differed from the environment the stored figures were taken in, not one: the LibreOffice
version *and* a missing `fonts-dejavu-core`. Attributing all of that movement to the version
bump was wrong. Holding LibreOffice constant at 26.2.4.2 and varying only the font set moves
**53 of 534 page counts and 426 pages** on its own — the same order as the whole figure above,
on overlapping documents (`AC-150-5370-10G` appears in both). See `MISSING_PACKAGES.md` in the
repository root for the per-track split and the reasoning that establishes DejaVu *was* present
originally: `SheetColumnDigitsTests` pins its metrics against values read from 24.2.7.2's own
output, so the repository's test suite is a statement about the environment.

The lesson generalises past this container. **The gate's inputs include the font set**, and
nothing in the harness declares it. Before trusting any figure, check `fc-match "DejaVu Sans"`
resolves to DejaVu rather than a fallback — `fc-match` never fails, it always returns
*something*, which is why the gap survived a whole pass unnoticed.

**But do not use `fc-match` as ground truth for what LibreOffice resolves.** Measured over the
296 families the corpus names, it agrees with the installed 26.2.4.2 on 288 — and **all eight
disagreements are `FcNameParse`**, which reads `-` in a family name as a size and `,` as a family
separator. LibreOffice does no such parsing, so it and `fc-match` genuinely answer different
questions for any punctuated name. `fc-match "Century Schoolbook"` is safe; `fc-match
"Foo-Bar, Inc Sans"` is not. When the answer matters, render a one-cell probe through `soffice`
and read the face out of the PDF.

**Do not use `fc-match` as ground truth for what LibreOffice resolves.** Measured over the 296
families the corpus names, it agrees with the installed 26.2.4.2 on 288 — and **all eight
disagreements are `FcNameParse`**, which reads `-` in a family name as a size and `,` as a family
separator. LibreOffice does no such parsing, so the two genuinely answer different questions for
any punctuated name. `fc-match "Century Schoolbook"` is safe; `fc-match "Foo-Bar, Inc Sans"` is
not. When the answer matters, render a one-cell probe through `soffice` and read the face out of
the PDF.

**The two binaries disagree about the `?` digit placeholder, and the tree follows 26.2.4.2.**
An unfilled `?` in a number format — the two that hold an accounting format's zero row clear of
its column, and every `# ??/??` fraction — is **U+2007 FIGURE SPACE** on 26.2.4.2 and **U+0020**
on 24.2.7.2. The seat is `cBlankDigit = 0x2007`, `svl/source/numbers/zformat.cxx`:71,
*"tdf#158890 use figure space for '?'"*. Measured over sixteen cells and seven codes, both
binaries, glyphs read out of the PDFs rather than out of `pdftotext`, which cannot show the
difference: `probes/chart-cat-reverse/make-numfmt.py`. It reaches **47 corpus documents** — six
chart parts and 45 `xl/styles.xml`. So a `?` cell compared against `/usr/bin/soffice` will differ
by a whitespace character per placeholder and that is the version gap, not a defect.

**And check it at the start of every session, because the install does not survive.**
`fonts-dejavu-core` was installed and documented as fixed, and a later session found
`fc-match "DejaVu Sans"` answering `wqy-zenhei.ttc` again — the package was simply absent
from `dpkg -l` in the new container. Everything else the reference needs (Carlito, Caladea,
Liberation, OpenSymbol, IPAGothic, WenQuanYi) *was* still installed, so nothing looks wrong
until you check the one font that decides 267 of 534 reference renderings.

Reinstalling has a trap of its own worth writing down, because it reads as the package having
been withdrawn:

```sh
apt-get install -y --no-install-recommends fonts-dejavu-core
# E: Package 'fonts-dejavu-core' has no installation candidate
apt-get update && apt-get install -y --no-install-recommends fonts-dejavu-core   # works
```

The container's package index is stale, not the archive. `apt-get update` first, always, and
re-check `fc-match` afterwards rather than trusting the installer's exit code.

**`grep -r` and `find` over this repository return exactly double. Use `git grep`.**
The case-insensitive mount has produced alias directory entries *inside the checkout* as well as
in the corpus: every project under `dotnet/src` now has a lower-case twin —
`dotnet/src/xray.office.core` beside `dotnet/src/XRay.Office.Core`, same inode
(`4785074604717685`), link count 1. `git ls-tree` lists only the canonical spelling and
`git status` is clean, so nothing is wrong with the tree; but anything that walks the filesystem
visits both names.

Measured 2026-08-20 on the same query:

| | hits | files |
|---|---:|---:|
| `grep -rn … dotnet/src --include=*.cs` | 96 | 60 |
| `git grep -n … -- 'dotnet/src/**/*.cs'` | **48** | **30** |

**Exactly 2×.** A reach census run with `grep -r` is therefore inflated by a factor of two, and
this project dispatches rounds on reach censuses. `git grep` and `git ls-files` operate on tracked
paths and cannot see an alias, so they are the correct instruments here; if you must walk the
filesystem, fold case and deduplicate before counting.

**Do not delete the aliases.** As in the corpus, `rm -rf dotnet/src/xray.office.core` is a request to
unlink that inode, and the inode is the source tree.

**`/tmp` is on the 20 GB overlay and this workflow fills it, which reads as an 11-verdict
regression.** A words round's post-change sweep returned **`REF-CANNOT-RENDER 13`** with `/` at
100%: **~120 000 stale entries, 17 GB**. It discarded that figure rather than reporting it, which is
the right call and is `HANDOVER.md`'s "a full disk looks exactly like a catastrophic regression"
arriving for the second time.

Measured by the parent shortly afterwards, on a `/tmp` holding **119 512 entries and 14 GB**:

| class | aged >2 h |
|---|---:|
| `MSBuildTemp*` | **114 122** |
| `xray-office-lo-*` (soffice profiles) | 759 |
| `clr-debug-pipe-*` | 350 |

**`MSBuildTemp*` is the bulk and it is ours** — every `dotnet build` leaves one, and this session
runs a build per merge. Clearing entries older than two hours took `/` from 4.6 GB free to 5.4 GB
and `/tmp` from 119 512 entries to 4 275, with a sweep running throughout and unaffected.

```sh
find /tmp -maxdepth 1 -name 'MSBuildTemp*'    -mmin +120 -print0 | xargs -0 -r rm -rf
find /tmp -maxdepth 1 -name 'xray-office-lo-*'  -mmin +120 -print0 | xargs -0 -r rm -rf
find /tmp -maxdepth 1 -name 'clr-debug-pipe-*' -mmin +120 -print0 | xargs -0 -r rm -rf
```

**The age bound is what makes it safe to run beside a live sweep** — nothing the sweep owns is two
hours old. Better still, point a sweep's `TMPDIR` at the host mount (`/c/sandbox/workdir/...`),
which has 150 GB free where `/` has five.

**A reference PDF differs byte for byte between two sweeps and it means nothing — but one document's
reference genuinely is non-deterministic, and the two must not be confused.**

The spurious case: **98 bytes of XMP `dc:date`**, length unchanged, with page, word and font counts
**identical across three sweeps of all 337 words paths**. `soffice` stamps the conversion time into
the metadata. So a byte diff of two reference renderings is not evidence of anything until the date
is masked out, and a round that byte-compares reference PDFs will otherwise find "changes"
everywhere.

The real case: **`ans_mappings_of_eccairs_terms.xlsx`** renders **191 pages eight times and 190
once** over nine renderings, with words wandering across four values and our side pinned throughout.
No `TODAY`/`NOW`/`RAND` — layout instability. It is filed `unstable`.

**The discriminator is the gate columns, not the bytes.** Identical page/word/font counts with
differing bytes is the date. Differing counts across renderings of one unchanged input is the real
thing, and there is exactly one such document known.

**`verify-test.sh` rebuilds twice, so running it during a sweep replaces the binary under that
sweep.** The rule "a sweep and a rebuild must never overlap" has always been written as though the
rebuild would be an explicit `dotnet build`. It need not be: the mutation harness builds on both
legs by design, and a round that runs it while a cross-track sweep is in flight silently swaps
`XRay.Office.Core.dll` mid-sweep.

**It announces itself as documents moving between two sweeps of the same unmodified tree** — round
60 saw **31 words documents** differ that way, one by 19.82 of ink on a chartless questionnaire.
Rendering is deterministic, so that cannot happen; a fresh render matched one sweep's copy and not
the other's (157 696 against 157 807 bytes), which is what identified it.

The check is cheap and belongs in the routine: **re-render one document after a sweep and compare it
byte for byte against that sweep's own copy.** If they differ, the binary changed under you and the
sweep is void. Anything that builds — `verify-test.sh`, a test run without `--no-build`, an IDE —
counts as a rebuild.

**An agent's cross-track figure is measured at its own base, and the manifest tracks HEAD. They
disagree, and the disagreement is not an error.** Three times in one session a round has swept the
other two tracks, found a manifest row it could not reproduce, and proposed a correction — each time
because the round that closed that document merged *after* its own base commit. The clearest case:
a words round proposed re-opening two documents on the grounds that "278 recorded against 276
measured, three rounds running, both sides stable, so not date volatility". Its reasoning was sound
and its conclusion wrong; the fix that closed them was three commits newer than its base.

So: **a cross-track sweep from an agent's worktree is evidence about that worktree**, and only the
parent's gate at HEAD settles a manifest row. Agents should say which commit they measured at — and
when a cross-track figure disagrees with the manifest, `git log <agent-base>..HEAD -- <the relevant
source>` is the first thing to run, not a manifest edit.

**The reference half of the gate is not reproducible for date-bearing sheets, and it decays the
manifest on its own.** Measured across three sweeps hours apart in round 51: four documents'
*reference* word counts moved with the wall clock while ours stayed pinned, so **the two halves of
the gate do not have the same reproducibility properties**, and a stored verdict on such a document
can go stale with nobody touching the code.

**The mechanism given here was wrong and the correct one has no environment-variable fix.** This
said `batch-check.sh` renders the reference with no `SOURCE_DATE_EPOCH` while `xray-office render`
honours it — but the script sets the variable on **neither** side, so a `&D`/`&T` header prints
today in both renderings and cancels. What actually diverges is **`TODAY()` in a cell**: the
reference recalculates the formula on open and we print the value cached in the file. Six documents
are affected and they drift *further apart* every day. Setting `SOURCE_DATE_EPOCH` on both sides
does not close it; only evaluating the volatile function would.

The practical rule when a sweep diff appears: **split it by which side moved.** Round 51 separated
nine real movements from three calendar ones that way. Volatile dates reach **16 of the 40 open
sheets documents**, not the ~7 previously carried.

**`/c/sandbox/workdir` is a case-insensitive virtiofs mount, and this invalidates sweep totals.**
The four corpus files described elsewhere as "upper-case on disk" are not a second file that a
case-sensitive glob would miss — on this mount `049_….pptx` and `049_….PPTX` are the **same
inode**, one md5, one file. The live trap is that a tool which probes both spellings *materialises*
the second one permanently: `look.py` resolves a document by `CORPUS.rglob(stem.ext)` **plus**
`CORPUS.rglob(stem.EXT)`, and a slides sweep total went **305 → 311 with the corpus unchanged**
because of it. Reconcile every `find`-based total case-folded, and treat a total that grew without
a corpus commit as this until proven otherwise.

**And the alias count is not static — it grows when you look at a page.** Measured across one
session: the corpus held 45 alias entries, then 38 more materialised on the sheets track alone, and
a whole-corpus sweep's `TOTAL` went **991 → 1033 with the corpus unchanged and not one commit to
it**. `look.py` and `pair.sh` create them by resolving a document. So a sweep `TOTAL` is not
comparable with the same sweep's `TOTAL` an hour earlier, let alone with a stored one. **Score every
sweep against `MANIFEST.tsv`'s path list, and have the scorer refuse to print unless every manifest
path found a row** — that check is what keeps the figure meaningful while the denominator drifts
underneath it.

Measured 2026-08-20, so the shape of it is not in doubt: `grants-2005.xls` and `grants-2005.XLS`
report the **same inode** (`35184372089472271`), the same size, and a **link count of 1**. `git
ls-files` lists only the lower-case name and `git status` reports **nothing untracked**, so git
resolves the second spelling to the tracked file. There is one file wearing two names in
`readdir`, and the `nlink` of 1 is the filesystem telling you so.

**Do not `rm` one. `rm <NAME>.XLS` deletes the document** — measured on a scratch file 2026-08-21:
three names, one inode, link count 1, and `rm` on one name destroyed the file while leaving the
others as stale entries pointing at nothing. The earlier form of this warning was inferred; it is now
demonstrated.

**They can be cleared safely, by renaming.** A rename round trip on the *tracked* name
(`mv x .tmp && mv .tmp x`) invalidates every case-variant entry for that inode and leaves the file
untouched. `.claude/skills/corpus-batches/scripts/dealias-corpus.py` does this; `--check` reports
without changing anything.

**Done 2026-08-21**: 77 aliased inodes carrying 87 extra names, all cleared, **zero hash changes**,
`git status` clean, and the corpus now holds **946 files in 946 distinct inodes with no case-only
collision anywhere**. A gate's `TOTAL` line therefore equals the manifest again, and the figures
below (355 / 311 / 325, and 1033 corpus-wide) are the *historical* over-counts, not current ones.

**git is the only authority for which spelling is real**, because no ordering rule works: some
aliases upper-case the extension and some lower-case the whole filename. `core.quotePath=false` is
required, since git escapes non-ASCII and the corpus holds a CJK filename.

**The aliases can come back.** They are created by case-variant lookups — `look.py`'s upper-case
`rglob` was one source and is fixed — so run `dealias-corpus.py --check` if a sweep `TOTAL` exceeds
the manifest again. There are **45** such entries corpus-wide
(words 18, sheets 18, slides 9), which is exactly the gap between `find` counts (words 355, slides
311, sheets 325) and manifest rows (337, 302, 307).

The corpus *does* separately contain four documents whose only name is upper-case — they are rows
in `MANIFEST.tsv` and are real. Distinguishing them from an alias is what the manifest is for,
which is the whole mitigation: **score against `MANIFEST.tsv`'s path list, never against a sweep's
own `TOTAL`.** Verdicts are unaffected either way, because per-format identity keys on the
extension as spelled (`report__xls` and `report__XLS` are two identities, so neither overwrites the
other) — it is only the counts that inflate.

Canonical reference renderings for this environment, all 534 documents at 26.2.4.2 with the
correct font set, were kept at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/` with a
`ref-baseline-all.tsv` beside them.

**That bank does not exist in this container and there is nothing to reuse.** `/c/sandbox` is
absent — as the *Roots have moved again* paragraph above already says — and no `refpdfs-*`
directory exists anywhere under `/home/user` either (checked 2026-09-06). A round that reads
this and plans around a banked reference discovers the gap only after it has been dispatched;
budget for rendering the reference half yourself, with `ref-baseline.sh` when only the
reference changed. The bank was also 534 documents, so it could not have covered the corpus
as it now stands in any case.

Individual claims calibrated to 24.2.7.2 behaviour — "the document-level `w:widowControl` is
inert", the 720 dpi device round trip, the reference's own table-only-header import defect —
are now claims about a superseded binary and each needs one re-check before it is relied on.
*The table-only-header one has now had its re-check (2026-08-15, round `words-ug-01`): the
mechanism survives the version move unchanged on 26.2.4.2, and re-measuring the **cost** of not
reproducing it is what reversed the standing decision — it was a page count as well as words. See
`SectionInheritedHeaderTests`, which asserted the opposite until that round.*
The largest single movers were `sectors-defense-and-aerospace.xlsx` (reference 227 → 449
pages), `CIS_Debian_Linux_8_Benchmark_v1.0.0.xls` (109 → 88), `A_320.doc` (150 → 118) and
`grants-2005.xls` (220 → 201).

### A banked gate *does* exist here, and the paragraph above is out of date

As of 2026-09-06 `/home/user/gate-2f47/` holds the whole-corpus gate at `2f4709c08` — `ref/` and
`ours/` with **947 PDFs each**, `rows.tsv` and `parity.tsv`. So the *"there is nothing to reuse"*
above is true only of `/c/sandbox/workdir/refpdfs-*`. Round 69 re-scored the sheets track in
**14 minutes** off it, rendering our half alone; `probes/overflow-r69/sweep-ours.sh` is that
script. It is sound whenever the diff under test cannot touch `soffice`, which a change confined
to `dotnet/src` cannot, and it applies the gate's own verdict rule to exactly the reference bytes
the scoreboard was built from. Check the bank before budgeting for a reference render.

**But a bank built with `SWEEP_ONLY` is a claim about the documents on the list and nothing else.**
Round 71's `/home/user/odsgap-work/bank/ours-head2` is a full render at one binary with **49**
documents re-rendered at the next, and that round proved its last fix's confinement by diffing that
bank against the one it was seeded from — which is circular, because the off-list rows are the same
bytes by construction. Rendering the whole ODF half at `6ab0681b9` gives `.odt` **259** of 338
where the bank scores 257, and `TE.CAO.00125 Foreign Part 145 approvals - OJT Logbook.odt`, one of
the documents whose rendering differs, holds **no `draw:frame` at all** — so the fix under test
could not have reached it. The same bank's original-track half differs from a clean base render on
**22 of 645**, of which 15 are `.docx` that nothing in that round's diff could touch. The check
that is not circular is a full render, and on this machine it costs about forty minutes for 645
documents at three workers. `probes/odf-rowpitch-r72/results.md` §6.

### A Calc drawing shape's text and a Calc *cell's* are three different rulers, and two of them were guesses

Three facts landed in round 69, all on the sheets track, all measured against 26.2.4.2.

**A centred print block is centred whether or not it fits.** `ScPrintFunc::PrintPage` writes
`nLeftSpace += ( aPageRect.GetWidth() - nDataWidth ) / 2` and `nTopSpace` the same way, with no
clamp on either sign (`sc/source/ui/view/printfun.cxx`:2165 and :2188) — so a block wider than
the paper hangs off **both** edges and the reference loses text at the left margin as well as at
the right. Guarding that addition on a positive remainder, which `SpreadsheetPages.BodyOrigin`
did, turns centring into left-alignment on exactly the sheets where the flag shows most.
`048_Expense_trends_budget`'s one-column `tips` sheet sat **167.3 pt** right of 26.2.4.2's on
every one of page 1's nineteen lines. **73 of the corpus's 243 xlsx-family workbooks** state a
centring flag. The case is reachable only because a *single* column cannot be split across page
columns; several columns wider than the page paginate instead.

**A bare `U+000A` inside an `a:t` is a paragraph break, not a character.** Every importer hands
its string to the EditEngine, and `ImpEditEngine::ImpInsertText`
(`editeng/source/editeng/impedit2.cxx`:2864-2983) normalises the line ends and calls
`ImpInsertParaBreak` at each separator, with `// Start == End => empty line` for two in a row.
Shaping it instead loses a whole line of height, which on a `vertOverflow="clip"` body stops the
clip firing at all — so *a lost paragraph* and *text that should have been clipped* are one
cause and not two. 19 of the corpus's 615 worksheet shape text bodies, in 9 documents.

**And `horzOverflow` is not the horizontal sibling of that clip: nothing reads it.**
`TextBodyPropertiesContext` stores it as a string (`textbodypropertiescontext.cxx`:83), which is
put in a grab bag (`shape.cxx`:2189) and re-exported (`drawingml.cxx`:4141, 4379). Those three
are the only uses in the tree and all three are writes. Only `vertOverflow` sets a property.
And the corpus could not witness one anyway: **516 worksheet bodies in 34 documents state
`horzOverflow` and every one of them also states `vertOverflow`**, so there is no document
where the horizontal attribute could decide anything the vertical one does not. Do not send a
round after it.

**A shape's line height is `ascent + descent` with no external leading and no device**, and
`SheetBandText.ShapeLineHeightAt` carried the leading for nine rounds. Measured on a probe of
sixteen wrapping text boxes — four faces × four sizes, no print scale — against 26.2.4.2 over 19
of them: `ascent + descent` is right to a mean of **0.008 pt**, carrying the leading is out by
**0.237 pt** and by **1.02 pt at 24 pt**, and putting the metrics through Calc's own 720 dpi grid
is out by 0.035 and wrong in both directions. So the device is not the missing half here — a
drawing object is formatted against the model's reference device, `RefDevMode::MSO1` at 8640 dpi
(`sc/source/core/data/documen8.cxx`:182-193), which is no grid at these sizes — and
`IsAddExtLeading()` being false in EditEngine is.

**Why that survived nine rounds is the line-gap trap this file already records once**, in the
"39/39 exact CJK fit measured on a face whose line gap is zero" case below. Carlito's `hhea` line
gap is zero and DejaVu Sans' is zero, so the two candidate laws agree *exactly* on them;
Liberation Sans' is 67/2048 and Liberation Serif's 87/2048. Every workbook the sheet-shape path
was built against resolved to Carlito. **A vertical-metric law tested only on Carlito has not
been tested.** `probes/overflow-r69/`.

### An RTF shape is a Writer *fly*, and four readings of one followed from getting that wrong

Round 73's four defects are one fact and its consequences, and the fact is in the importer rather
than in the layout: `RTFSdrImport::createShape` turns every top-level `shapeType` 1 (rectangle) or
202 (text box) into a **`com.sun.star.text.TextFrame`**
(`sw/source/writerfilter/rtftok/rtfsdrimport.cxx`:323-334), not into a drawing object. Everything
below follows from that, and each was read the other way here for as long as the reader has had
shapes. `probes/rtf-shape-r73/results.md`; `.rtf` gate **216 → 239 of 328**, original words track
not one column of one row moved.

- **It is captured on its page in both axes**, so `DoNotCaptureDrawObjsOnPage` does not exempt it:
  a fly is clipped by `SwFlyFreeFrame::CheckClip` (`sw/source/core/layout/flylay.cxx`:471-545),
  which that flag does not reach. Measured on 21 probes — five `\shpwr` values × {fits, overflows
  right, overflows left}, plus one that begins entirely off the sheet and the vertical pair: the
  clamp is `paperw − width` and `paperh − height` against the **page**, right edge first then left,
  with no exception for wrap-through and none for a shape carrying no text. `FrameLayout` had only
  the vertical half of it; `ImplAdjustHoriRelPos`
  (`sw/source/core/objectpositioning/anchoredobjectposition.cxx`:674-721) is the other half and
  sits under the same guard.
- **`posrelh` and `posrelv` have a case for the value 1 and no other** (`rtfsdrimport.cxx`:696-717);
  everything else keeps the `RelOrientation::FRAME` `getTextFrameDefaults` (:111-124) gave the
  frame, which is the body column across and the anchor paragraph down. So **0 is not the page
  margin** although MS-ODRAW's names invite it, and 0 and 3 place a shape identically — nine
  horizontal probes and five vertical ones, on a page whose margin, body area and indented column
  are three different origins. The corpus states `posrelh` 3 3536 times, 2 201 times, 1 33 times and
  **0 not once**, so that correction has no reach; the useful half is that *the brief's premise was
  wrong* — this tree already agreed with 26.2.4.2 on nine of nine before the round began.
- **Its text is inset by 0.1 inch across and 0.05 inch down when the shape states nothing**, from
  the same `getTextFrameDefaults`; `dxTextLeft` and its three siblings are EMUs over 360
  (:600-625). Zero and absent are two different answers, exactly as they are for the wrap distance.
- **`\shpwr`'s numbering is `rtfdispatchvalue.cxx`:1222-1247 and not the specification's prose.**
  1 is `WrapTextMode_NONE` — no text beside the shape at all — 2 and 4 are `PARALLEL`, and **3 and
  5 are both `THROUGH`**. Three of the five were read wrongly, and 3 is the expensive one: **1341
  occurrences in 178 of the 338 converted `.rtf`**, each narrowing every line beside it. An
  unstated `\shpwr` is the fly default, which is parallel; the field defaulted to 1.

**And a table inside `{\shptxt}` was dropped whole**, which is a different seat and the larger
half of the text loss. `FinishTable` named three destinations for a finished table — the body, a
note, a running head — and not the frame, while `RecordLayoutParagraph` beside it named all four.
LibreOffice's export writes a Writer text frame's content verbatim, so a boxed table is
`{\shptxt\trowd…\cellx…\intbl…\row}`: **20 such groups in 11 of the 338 `.rtf`**, and
`043_Visual_Product_Roadmap` drew 6 of the reference's 184 words.

**The instrument that found all five is the previous round's**, `xray-office extract` against our
own rendering: every one of these documents reported the reference's character count *exactly*
while drawing a fraction of it, because extraction walks the content tree and the drawing path is
where the loss is. Use it before looking at a pixel.

**One hypothesis about the `.rtf` column's pagination is refuted and should not be re-derived.**
*"The reference keeps a table row whole where we split it."* It does not: a twenty-file synthetic
(`probes/rtf-shape-r73/rowsplit.py`) walks a three-line row across a page boundary and **both
renderers split it in the same place at all twenty**. The two disagree on two files only, where the
reference breaks the page and we fit one more row on it — a one-row capacity difference whose sign
is the wrong way round to explain `Annex-10`'s 169 pages against 148 at 189432 glyphs against
189432. What that document does show is that **our page *N*+1 holds the reference's page *N*** for
long stretches, so the excess is inserted at page boundaries rather than accumulated; that eight of
our 169 pages carry fewer than 12 lines against none of the reference's 148; and that our body
starts 12 pt higher on every page although the document declares no `{\header}` group at all.

### An RTF style that names one of Word's keeps Writer's style, and a dropped `\sbasedon` therefore lands on a *pool* parent rather than on nothing

Round 80 established the first half: `\sbasedon N` is converted to a **name** where the control word
is dispatched (`pIntValue = new RTFValue(getStyleName(nParam))`,
`sw/source/writerfilter/rtftok/rtfdispatchvalue.cxx`:131-134), and `getStyleName`
(`rtfdocumentimpl.cxx`:873-885) reads a map filled one entry at a time as each stylesheet entry
*ends* (`:1594`) — so a style based on one declared **later in the same `{\stylesheet}`** resolves to
the empty string and `lcl_findParentStyle` (`:275-298`) finds no parent. **54 of the 338 converted
`.rtf` hold at least one such forward reference.**

**What that round did not have is that "no parent" is not where such a style ends up.**
`StyleSheetTable::ApplyStyleSheets`
(`sw/source/writerfilter/dmapper/StyleSheetTable.cxx`:1099-1121) converts the entry's name through
`ConvertStyleName` (`:1620-1660`, a two-hundred-name map holding `heading 1`…`heading 9` and
`Heading 1`…`Heading 9`, `Normal`→`Standard`, `Body Text`→`Text Body` and the rest) and, where
`xStyles->hasByName` answers yes, **reuses Writer's existing style of that name**, resets its own
properties and leaves its parent alone. The one branch that would clear the parent needs
`m_bHasImportedDefaultParaProps`, which only an OOXML `w:docDefaults` sets (`:653-667`) — never RTF.
A `\sbasedon` that *does* resolve calls `setParentStyle` and takes the pool parent's place
(`:1156-1169`), so this is a fallback and not an override.

`Heading 4`'s parent is `Heading`, and `SwPoolFormatId::COLL_HEADLINE_BASE`
(`sw/source/core/doc/DocumentStylePoolManager.cxx`:768-819) is where four values a paragraph can take
come from: `SvxFontHeightItem aFntSize(PT_14, …)` at `:809`,
`SvxULSpaceItem aUL(PT_12, PT_6, RES_UL_SPACE)` at `:810` and `SvxFormatKeepItem(true, RES_KEEP)` at
`:814`. **The per-level percentages in `aHeadlineSizes` (`:107-115`) do not arrive with them** — the
import resets the level style's own properties — so `heading 1` through `heading 9` all answer 14 pt
rather than 18.2, 16.1, 14.1 and so on. Measured against 26.2.4.2 on nine levels, both spellings of
"does not resolve", a control name Writer answers nothing for, and twenty files sweeping a page
boundary for the keep-with-next; `.rtf` gate **259 → 260 of 336**, reach **17 of 338**.
`probes/rtf-holdover-r87/results.md` §2.

**Two traps in measuring it.** A style's own `\fs` still reaches no paragraph, so a heading that
states one gets neither its own size nor the pool's but `\pard\plain`'s 12 pt — the pool sits in the
*inherited* half, which is the half round 80's rule lets through. And `\sa`'s absence from
`RtfStyleFormatting` is a statement about **RTF sprms**: `getDefaultSPRM` writes `after = 0` over a
style's space after, and the pool's six points is not an sprm at all, so it needs a carrier of its
own.

***`Title` and `Subtitle` are the same rule under two names that look nothing like a heading, and
`Body Text` and `caption` are a different one.*** Round 87 left the four as *"26.2.4.2 answers 10,
10, 14 and 14 where this tree answers 12"*; the numbers are right and **neither of the two 10s is a
pool value**. What decides it is `GetPoolParent` (`sw/source/core/doc/poolfmt.cxx`:279-289): every
id in `COLL_DOC_BITS` that is not `COLL_HEADLINE_BASE` itself has that style for a parent, so
`COLL_DOC_TITLE` and `COLL_DOC_SUBTITLE` answer *Heading*'s 14 pt, 12/6 and keep-with-next — and
**not** the 28 pt bold centring and 18 pt those two pool entries state for themselves
(`DocumentStylePoolManager.cxx`:1365-1387), because the entry's own properties are what the import
resets. `COLL_LABEL` and `COLL_TEXT` have `COLL_STANDARD` for a parent (`:201-204`, `:229-235`), so
`caption` and `Body Text` inherit **the document's own `Normal` entry**: the same probe with
`{\s0 … \fs20 Normal;}` draws them at 10 pt and with `\fs28` at 14, while `heading 4`, `Title` and
`Subtitle` answer 14 to both. So the discriminator is one attribute of the *control* style, and a
round that measures only one `Normal` size cannot tell a pool constant from inheritance.
`Title` and `Subtitle` are **done**; the *Standard* half is measured, pinned by
`APoolStyleUnderStandardIsNotModelledYet`, and left — it needs the whole of `ConvertStyleName`'s
two hundred names to be safe, and the pool entry a reader would copy is the wrong half of it.
**Reach is nil for the two that are implemented**, and finding that out corrected round 87's own
figure: a census whose *used* set is taken from the whole file counts each stylesheet entry's own
`\sN` as a paragraph using it, and both `Title` candidates are declared and never applied. Excluding
the stylesheet, `Title` 0 documents of 338, `Subtitle` 0, `Body Text` 2, `caption` 0 — and the nine
headings **17 → 12**, which is round 87's reach re-counted rather than its fix re-measured.
`probes/rtf-bookmark-r88/results.md` §4.

### An RTF `REF` field expands from its bookmark, and writerfilter gives the bookmarks the wrong names

**The reference draws more text from an RTF than the same document's `.odt` twin, and the extra text
is real.** On `24-25_FAA_Holdover_Tables` 26.2.4.2 draws `Table 48: Snowfall Intensities as a
Function of Prevailing Visibility` 422 times where the `.odt` draws the short `Table 48` — the file
states `{\field{\*\fldinst  REF _Ref107225632 \\h }{\fldrslt Table 48}}` and the bookmark covers
`Table 48` alone.

RTF sends a bookmark half's **name before its id** (`lcl_getBookmarkProperties`,
`sw/source/writerfilter/rtftok/rtfdocumentimpl.cxx`:224-236, whose own comment says the name
*"should be sent first"*; the two halves at `:2735-2764`), and `DomainMapper::lcl_attribute`
(`dmapper/DomainMapper.cxx`:340-347) routes them to `SetBookmarkName` and `StartOrEndBookmark`. But
`SetBookmarkName` (`DomainMapper_Impl.cxx`:9426-9447) is written for OOXML, where `w:bookmarkStart`
states `w:id` **then** `w:name`: it looks up `m_sCurrentBkmkId` — *the previously opened start* — and
where that start is still open writes the incoming name onto **it**, reaching `m_sCurrentBkmkName`
only when the map misses. **So every name lands one bookmark early**, and a paragraph opening five
bookmarks hands out five names one place shifted. Three lines of RTF reproduce it
(`probes/rtf-holdover-r87/genbookmarks.py`); with two starts the importer emits two bookmarks called
`R1` and `R1 Copy 1`, which is the same fault with nowhere to hide.

A bookmark that thereby ends in a different text node from its start makes
`SwGetRefFieldType::FindAnchor` answer `*pEnd = -1` (`sw/source/core/fields/reffld.cxx`:1588) and
`SwGetRefField::UpdateField`'s `Bookmark` case read that as *to the end of the paragraph*
(`nEnd = nNumEnd<0 ? nLen : nNumEnd`, `:604-607`).

***Done in round 88, and it is worth more than the hand-patched estimate.*** `RtfBookmarkRotation`
is those three functions in the order a bookmark half goes through them and `RtfReferenceFields` is
the expansion; `24-25_FAA_Holdover_Tables` goes from **158 pages to 221 against the reference's
223** and from 6.71 % to **0.33 %** on alphanumeric characters, drawing the long caption 253 times
where the reference draws it 253 times. **Reach is 11 of the 338 `.rtf`**, counted on the `fldinst`
group rather than on the string `REF ` — which also matches `PAGEREF` and gave 13.

**A fourth link the brief did not name, and the rotation is what makes it expensive.** A group
nested inside a bookmark's name is *part of the name* and not a half of its own — `popState` guards
both destinations with `if (&getDestinationText() != getCurrentDestinationText()) break; // not for
nested group` (`:2736-2740`, `:2751-2755`), which skips the nested half and, because the two states
share one buffer, keeps its text. Under a reader that pairs by name that is harmless; under the
rotation a spurious half takes an id, and a spurious **end** naming nothing takes
`m_aBookmarks[""]` — value-initialised to **0** — so it closes the document's *first* bookmark under
the wrong name. Measured: 26.2.4.2 reads `{\*\bkmkstart A}first{\*\bkmkend {x}A}` as one bookmark
called **`xA`**, so its `REF A` finds nothing, and the same file without the nested group draws
`first`.

Three things about it are worth carrying. **The rotation is verified against 26.2.4.2 on ten
one-shape probes, 27 of 28 predictions exact** (`probes/rtf-bookmark-r88/`), the twenty-eighth being
the flat-ODF *shape* of a cross-reference bookmark rather than what it expands to. **A document
stating a `REF` is read twice**: a `REF` may name a bookmark the walk has not reached, and rewriting
a paragraph after it has closed would rebase every offset counted against it — its runs, its notes,
its frames and its own marks — so `RtfReader` computes the expansions from the first read's marks
and hands them to a second. And **the "Error: Reference source not found" the reference draws for a
name nobody holds is deliberately not reproduced**: it would fire wherever *our* bookmark table is
the incomplete one, and it costs two occurrences across the eleven documents.

**No gate verdict moves, and the verdict column is the wrong instrument here.** Both holdover
documents fail on pages before and after; what the fix moves is the page count and the characters
drawn. The other document to know about is `FAA 2025-26 Holdover Tables`, which is **172 pages
against 233** with its glyph distance now at 1.70 %: whatever is left there adds pages without
adding text, and it is not this mechanism — the long expansion appears 578 times on both sides.

**The instrument for both of these is worth more than either.** `soffice --convert-to fodt` on the
`.rtf` prints the reference's own answer for every bookmark, every style name and every parent, in
eight seconds on a six-megabyte file and with no rendering at all. It is what turned "the reference
draws a longer string" into a named permutation and "the heading is two points bigger" into a pool
parent, and it should be the first call in any round that suspects an RTF *import* rather than a
layout.

### A worksheet shape's fill and outline are read now, in all three formats — and the reach census that briefed it was misread

**Closed in round 84.** `SheetDrawing.Fill` and `.Stroke` had exactly one writer,
`XlsxNoteCaptions`, and every other shape on every worksheet was drawn as bare text over whatever
was under it. All three readers now answer the fill, the outline, the line width and the shape's
own preset, and `SheetShapeInk` paints it through `CustomShapeGeometry`'s two outlines.
`probes/sheet-fill-r84/results.md`.

**The census the brief carried is right and the inference from it is wrong, and the correction is
the general point.** *644 worksheet shapes in 49 documents, 421 an `a:solidFill`, 205 an
`a:ln/a:solidFill`, 583 an `xdr:style`, 332 `schemeClr`, 28 presets* all reproduce exactly. But
*"over half the work is theme-colour resolution"* conflated the **colour** scheme with the
**format matrix**: resolved per shape rather than counted per element, **497 of the 583 styled
shapes also state a fill of their own and it wins** (`Shape::getActualFillProperties`,
`oox/source/drawingml/shape.cxx`:2816-2841), so the matrix decides **86 shapes in 3 documents**
while theme *colour* resolution is needed on nearly all 421. **Count the property that survives
the merge, not the element that states it.**

Three figures the census did not have decided the shape of the work: **174 of the 644 sit inside
an `xdr:grpSp`** and 98 say `a:grpFill`, so the ink has to live on `SheetDrawingPart` as well as
on the anchor; **31 shapes in 3 documents are turned into `[45°, 135°)` or `[225°, 315°)`**, where
Excel writes the anchor for the *turned* rectangle and Calc reflects it in `y = x` before drawing
(`sc/source/filter/oox/drawingfragment.cxx`:299-330) — without which an organisation chart's
elbow connectors run off the top of the page; and `a:xfrm/@flipH`/`@flipV` mirror the geometry
*before* `@rot` turns it, so a connector written `rot="10800000" flipH="1" flipV="1"` is the
identity and honouring the rotation alone reflects it twice.

**And "an inked-but-textless shape does not widen the printed block" was true of two readers of
the three.** `ScDrawLayer::GetPrintArea` covers every object on the draw page and excludes only
the hidden-comment layer (`sc/source/core/data/drwlayer.cxx`:1397-1414); the SpreadsheetML reader
has always produced a drawing for every `xdr:sp`, the ODF one answered null for a shape with
neither text nor a picture, and the BIFF one still does. The ODF half is closed —
**444 custom shapes in 26 of the 307 converted `.ods` carry ink and no text** and were drawn
nowhere and counted nowhere — and the BIFF half is deliberately left, because keeping every BIFF
shape **costs two gate verdicts**: that path has neither of LibreOffice's two guards,
`IsProcessSdrObj()`'s `!mbHidden` (`sc/source/filter/inc/xiescher.hxx`:118 — a hidden BIFF object
is dropped where a hidden DrawingML one is kept, which is the two formats disagreeing) and
`IsValidSize`'s phantom test (`xiescher.cxx`:414-420, 3658-3665). Calc's ODF import has no
equivalent guard, which is why the same change is right there and wrong here.

**No gate column can see any of it** — a fill and an outline add no glyphs and no pages — so the
round is scored on PDF marks and pixels: the whole-corpus gate is **914 of 947 before and after**,
**35 of the 307 sheets renderings changed and not one changed a page, word or glyph count**, and
the words and slides tracks are byte-identical document for document. That is the `w:pgBorders`
shape again and the argument for ranking on ink.

**Two instrument notes worth more than the numbers.** A filled-path count is *not* comparable
between the two sides wherever a gradient is involved: 26.2.4.2 writes a themed gradient as a Form
XObject full of band fills (`Do`) and this tree writes one PDF shading (`sh`), which on one
organisation chart reads as 914 fills against 159. **Count `sh` and `Do` before calling a fill
count a defect**; the comparable column there is the strokes, 123 against 128. And a hand-built
flat `.ods` shape probe is a trap of its own — a `draw:custom-shape` whose `draw:enhanced-geometry`
names an `ooxml-` type and states no `draw:enhanced-path` is drawn by 26.2.4.2 as **nothing at
all**, silently, so the fixture for this was authored as `.xlsx` and converted.

### And round 84's two conservative choices meant the BIFF half read almost nothing

**A `.xls` shape's fill and line colour are a palette reference, not a literal, and both defaults
are real.** Round 84 wrote its own caution into `EscherInk`'s remarks — *presence is the test*,
because MS-ODRAW's white and black defaults would otherwise "put a white box under the text of
every shape that mentions neither", and *only the literal `MSO_CLR` form is honoured*, because the
palette "cannot be seen from this layer". Both are wrong, and together they resolved **14 of the
106** fill and line colours the corpus's 64 `.xls` state on a worksheet shape; the other 92 are
scheme references. `TICAPCapability_Final.xls` — a document that **passes the gate** — was 16.87 %
off on summed unsigned ink for exactly this: its two `Instructions` text boxes state
`fillColor 0x08000041` and `lineColor 0x08000040`, nothing else at all, and those are palette 65
and 64, Excel's *window background* and *window text*. The reference draws a white panel with a
0.42 pt black border and we drew neither. `probes/sheet-shapefill-r92/results.md`.

Three rules replace the two choices, and only one of them is "the shape said so".

- **`EscherColour` is `SvxMSDffManager::MSO_CLR_ToColor`** (`filter/source/msfilter/msdffimp.cxx`:3420):
  a `0xfe` header masks to the low three bytes; `nUpper & 0x08` makes the low *word* a scheme
  index and `nUpper & 0x19` without `0x10` makes the top byte one; a bare `nUpper & 4` with no low
  bits is a third scheme form; everything else is a literal `0x00BBGGRR`. An unresolvable
  reference falls back **per property** — white for a fill, black for a line (`:3440-3453`). The
  host's palette arrives as a callback because it is the host's: Excel's is
  `XclImpPalette::GetColor` over the BIFF8 defaults, and its indices past the table are the ones
  that matter — 64 black, 65 white, 77/79/81 black, 78 white, 80 the note background.
  **The system-colour branch is not implemented and should not be**: it is the desktop theme's
  answer, and **zero** of the corpus's 106 colours state one.
- **Absence is not "no ink".** `mso_PropSetDefaults` (`filter/source/msfilter/dffpropset.cxx`)
  gives property 385 the value `0xffffff` and 448 zero, and Calc puts the window background on a
  filled object with no colour a second time (`xiescher.cxx`:3693-3695).
  `014_Contextures_chart_sample_991ecfc5.xls`' `Rectangle 6` is the witness: `fLine` true, no
  `lineColor`, and 26.2.4.2 draws `#000000`.
- **An unstated boolean is the shape *type's* answer, not a constant.** The same table gives
  property 447 the value `0x001C` and 511 `0x001E`, so `fFilled` and `fLine` both default *true* —
  and `ApplyFillAttributes`/`ApplyLineAttributes` (`msdffimp.cxx`:1313-1323, :904-911) then clear
  the bit again unless the shape stated it **hard** or the type is filled (stroked) by default.
  `mso_DefaultFillingTable` and `mso_DefaultStrokingTable`
  (`svx/source/customshapes/EnhancedCustomShapeGeometry.cxx`:6156-6213) are the tables and they
  reduce to `stated ? statedValue : defaultForType`. A text box and a rectangle are both; a
  **picture frame is neither**, which is the one entry in the stroking table. A stated boolean
  wins either way, and the corpus needs both directions: **216** worksheet shapes state property
  511 as `0x00080000` — `fLine` hard and *false*.

**Three object kinds must take no Escher ink at all, and the OLE test is not the object type.**
Calc replaces the DFF-built `SdrObject` for a chart, a TBX form control and an OLE object — the
three that `SetCustomDffObj(true)` marks (`sc/source/filter/excel/xiescher.cxx`:1666, 2066, 2954) —
and the replacement is not the object `ApplyAttributes` filled. A plain BIFF8 picture and an
embedded object are *both* `ftCmo` type 8, and `SvxMSDffManager::ImportGraphic` separates them on
the shape's own **`pictureId` (267)** (`msdffimp.cxx`:4025-4030): with it an `SdrOle2Obj` and no
attributes, without it an `SdrGrafObj` that keeps them. `TICAPCapability_Final.xls`' `Picture 228`
hard-states `fFilled` and `fLine` true with a white fill and a black line, carries `pictureId`, and
26.2.4.2 draws neither.

***The instrument that established every one of those answers rendered nothing.***
`soffice --convert-to fods` on the `.xls` prints the reference's own graphic style for every shape
— `draw:fill`, `draw:fill-color`, `draw:stroke`, `svg:stroke-color`, `svg:stroke-width` — in eight
seconds, and joining it to a dump of the raw `msofbtOPT` by `draw:name` gives an exact expected
value for one `MSO_CLR` per shape with no rasteriser and no tolerance anywhere in it. It is the
`.rtf` rounds' *"convert to flat ODF first"* arriving on the sheets track;
`probes/sheet-shapefill-r92/expected-ink.py` is the join.

**It is BIFF-only, and the `.ods` twin was already ahead of its `.xls` original.** The scheme-index
question does not exist in the other two readers — DrawingML states a theme colour and ODF a hex
string — and 26.2.4.2's own `.ods` of `TICAPCapability_Final` states
`draw:fill-color="#ffffff"` outright, which `OdsShapeInk` has read since round 84: this tree draws
that twin's panel at `(85.9, 61.1, 515.8, 433.8)` against the reference's
`(85.9, 61.0, 515.8, 433.8)`, stroked at 0.419 pt against 0.42, while the `.xls` of the same
workbook drew nothing. **A converted-ODF column can be right where its original is wrong, and
checking the twin is how you find out which half of a reader is at fault.**

**Reach, measured on ink rather than censused.** Rendering our half of the whole 947-document
corpus twice — at the round's base and with this fix, under `SOURCE_DATE_EPOCH`, one output
directory per document — moves **6 renderings, all `.xls`, all on the sheets track**, and leaves
the other **941 byte-identical**. Summed unsigned ink over the six: `TICAPCapability_Final`
**20.66 → 7.45**, `SIL_TDB609` 2.73 → 1.07, `SIL_TDB605` 1.99 → 1.00 — and `EHEST` 14.16 → 14.95,
`PC1000` 2.77 → 5.11, `apron-area` 1.41 → 1.53, which is the section below.
**No gate verdict moves on this half**, because a fill adds no glyph and no page.

### The drawing layer's paint region is a clip, and only the cull half of it was implemented

**Reading a BIFF shape's fill is what made this visible, and it is worth more than the fill.**
`ScOutputData::PrePrintDrawingLayer` builds the page's own cell-block rectangle and hands it to
`SdrView::BeginDrawLayers` as the paint **region** (`sc/source/ui/view/output3.cxx`:41-102);
`ScPrintFunc::PrintArea` calls the pair once per printed area (`printfun.cxx`:1641, 1651-1713). So
every object the drawing layer paints is *clipped* to that rectangle as well as culled by it.
`SheetPageGraphics.ReachesTheBlock` has computed exactly that rectangle since the round that
closed `Part_375_Operators.xlsx` — with the same citation — and used it only to decide which page
a drawing belongs on. `SheetPageGraphics.Block` is now both.

**26.2.4.2 emits the whole shape and then clips it away, so a path census reads it as a shape the
reference draws.** `PC1000.xls`' `Rectangle 16` is 244 pt wide and starts 6 pt inside the sheet's
last column; the reference's page 2 opens the figure with
`q 55.389 552.019 681.846 23.981 re W* n` and paints a 244 pt rectangle inside it, of which 4 pt
show. **Count the pixels.**

**And count the pixels rather than the bytes when measuring it too.** A clip emitted round the
drawing pass changes the content stream of every page carrying a drawing: over the sheets track it
changes **164 of 307** renderings' bytes and **83 of those 164 are pixel-identical**.
`probes/sheet-shapefill-r92/pixel-diff.py` is the instrument, and a hash-based mover list without
it overstates this change by a factor of two.

Of the 81 that do move: **68 improve on summed unsigned ink, 3 worsen** (by 0.07, 0.12 and 0.48),
10 are level, the sum goes **331.20 → 238.54** and MAJOR pages **148 → 85**.

**This is the half the gate can see, and it gains 25 verdicts.** PyMuPDF and `pdftotext` both drop
the text a clip removes, and the reference's counts already have it dropped — so the characters we
lose are characters we were never entitled to draw. Scored with the gate's own `max(2 %, 15)` rule
over all 307 sheets documents against the banked 26.2.4.2 reference: **262 → 287 match**, 40 → 15
`glyphs`, pages unmoved at 3. Four of the 25 land on the reference's count **exactly** —
`SSRO_Quarterly_Statistical_Bulletin` 2783 → 2532 against 2532, `044_Cash_flow_forecast`
2313 → 2200 against 2200, `064_Small_business_cash_flow` 1803 → 1635 against 1635,
`Foreign_SA-CAT-I_and_CAT-II-III` 7842 → 7558 against 7557 — which is as clean a statement as this
corpus offers that the rule is the reference's own.

**One test asserted the opposite and its premise was the right one.**
`SheetPictureCropTests.AnUncroppedPictureIsNotClipped` held that *"an unconditional clip would put
a `q`/`W n`/`Q` into every rendering carrying a picture and change all of them for nothing"*. The
first half is exactly what this does; the second was never measured, and it is false.

**The passing sheets set re-ranked on ink afterwards is `probes/sheet-shapefill-r92/ink-ranking.tsv`,
and 41 of the 287 passing documents are at 5 % or worse.** `TICAPCapability_Final` has gone from the
top of the seating probe's 48-document sample to **32nd of 287**. The next seats are
`TK-Syllabus-Comparison-Document-v2.xlsx` (304.57 over 1235 pages), `alle einzeln.xlsx` (225.44),
`Background_Declaration_Template.xls` (136.07 over 25) and `grants-2005.xls` (96.11) — and
`6880ac7361ca…ST Capability List` is the one in the top ten with **no MAJOR page at all**, so its
27 % is spread thin rather than concentrated. Read the column beside the MAJOR count and beside the
page count: a summed percentage over pages is not comparable between a 25-page document and a
1235-page one.

**What is left, with its seat.** A BIFF text box's `TXO` formatting runs are not read at all —
`XlsDrawingCollector.ReadText` takes the string and stops, and `TextOf` builds one run per line at
a hardcoded ten point in the default face. The runs are eight bytes each in the `TXO`'s second
`CONTINUE`, a character offset and a `FONT` index (`XclImpDrawing::ReadTxo`, `xiescher.cxx`:4242),
and `XlsCellFormats.FontAt` already turns that index into a face, a size and a weight. Reach:
**155 text boxes with text in 17 `.xls`, 522 runs, of which 62 boxes in 13 documents state more
than the opening run**. On TICAP page 3 the reference draws its shape text at 6.30 pt with five
bold spans and we draw all of it at 5.70 pt regular, which is most of the residual there.

### And the head of that ranking is 413 conditional formats, which nothing read

**`TK-Syllabus-Comparison-Document-v2.xlsx` was 304.57 % summed unsigned ink over 1235 pages with
142 MAJOR pages, and it passes the gate.** It carries no drawing at all, so neither the Escher-ink
round nor the clip could reach it. What it carries is **413 `<conditionalFormatting>` blocks, every
one `type="expression"`**, and `XlsxConditionalFormats` read only `colorScale` — its own remarks
named the gap and left it. Closed in round 94: `XlsxConditionalStyles` evaluates every `cfRule`
naming a `dxfId` and hands the fill to `SheetFormatting.SetConditionalBackground` and the font to a
differential `SheetConditionalText` over `SheetCellFormats`. **304.57 → 205.57, MAJOR 142 → 66.**
`probes/sheet-ink-r94/results.md`.

**The rank-8 document is the same workbook revised and the 6× between them is the rule count.**
`tk-syllabus-comparison-document-v5.xlsx` states **3** expression rules where v2 states 413 — the
revision baked the formatting into the cells and left 728 unused `dxf` entries behind — and their
per-page ink was 0.0544 against 0.2466. *Two revisions of one document differing on a measure is a
lead about what one of them stopped stating.*

Three rules decide it and each is the reference's rather than the specification's:

- **Exactly one rule wins a cell and two matching rules' properties are never merged.**
  `ScDocument::GetCondResult` (`sc/source/core/data/documen4.cxx`) returns the *first* non-empty
  style name's item set and `ScConditionalFormat::GetCellStyle` (`conditio.cxx`) the first matching
  entry's style. A cell takes one `dxf` or none.
- **What a `dxf` is silent about falls through to the cell's own pattern**, so the overlay is
  differential and its toggles are three-valued: **287 of that document's 413 rules state
  `<strike val="0"/>`**, which *removes* a strikethrough the cell states, and 26.2.4.2 writes that
  out as `style:text-line-through-style="none"`.
- **A `dxf`'s fill states its colour in `bgColor`, the opposite of a cell's.**
  `Fill::finalizeImport`'s `if (mbDxf)` branch (`sc/source/filter/oox/stylesbuffer.cxx`) moves the
  `bgColor` into the pattern colour and forces the pattern solid. **2734 of the corpus's `dxf`
  fills state only a background**, so reading `fgColor` finds nothing on any of them.

**The formula subset is where the corpus is, not where the specification is.** Of the 601
`expression` rules in 34 documents, **461 are the single shape `<reference> = "<text>"`** — one
comparison between a relative reference and a string, shifted from the `sqref`'s own top-left
corner, which is what `calcext:base-cell-address` states in the reference's own view of the file.
`cellIs` (123 rules, 18 documents) is read too. The rest — `AND`, `MOD(ROW())`, `ISERROR`,
`TODAY()`, defined names, `#REF!` — paint nothing. *"`containsText` and its five siblings (1040
rules in 8 documents) are the cheapest thing left in the area"* was right about the cost and wrong
about the value: round 96 read them and they move three renderings — see the section below.

**Reach and cost, measured rather than censused.** 55 of the 947 corpus documents state a `cfRule`
naming a `dxf`; rendering our half of the whole corpus twice moves **21 renderings and leaves 926
byte-identical**, every mover an `.xlsx` on the sheets track. 12 improve, 3 worsen, 6 are level, and
the sum over the 21 goes **354.87 → 254.85** with MAJOR pages 165 → 86. **No gate verdict can move**
— a colour, a strikethrough and a fill add no alphanumeric character and no page.

***And the one worsening located a palette defect whose obvious fix is refuted.***
`Computer and Software Services_50 State Comparison.xlsx` goes 20.15 → 27.95 because its
`cellIs equal 0` rule names an `indexed` colour and the workbook overrides the palette with entries
written **`ffRRGGBB`**. `ColorPalette::importPaletteColor` builds
`::Color(ColorTransparency, decodeIntegerHex(rgb))` and `decodeIntegerHex_impl`
(`oox/source/helper/attributelist.cxx`:72-79) is `o3tl::toUInt32(value, 16)` cast to signed — so an
eight-digit entry's **top byte becomes the transparency**, and 26.2.4.2's own
`--convert-to fods` gives all three of that workbook's `ConditionalStyle_N` a
`fo:background-color="#ffffff"`. **Modelling that in `XlsxPalette` takes the same document to
68.17** and was reverted: its *stated* cell fills name the same indices and the reference does draw
those, so the transparency reaches the conditional style and not an ordinary fill, and where the two
part company is not established. **Reach if anyone takes it is exactly one document** — 35 of the 55
workbooks stating an `indexedColors` table carry a non-zero top byte and this is the only one that
names such an entry by index.

**What is left on the witness is a row-height question, not a formatting one.** The residual 205.57
is 93.5 on pages 1-100 and is a within-sheet drift: on page 52 the same rows appear in the same
order with ours two rows lower than the reference's, and the total page count is 1235 on both sides.

**And ranks 2 and 3 of that ranking are neither of these.** `alle einzeln.xlsx` (225.44 over 186
pages) states **no conditional formatting at all** and holds a `pivotTable` over `A4:I1013`;
`Background_Declaration_Template.xls` (136.07 over 25) is BIFF, whose conditional formatting is
`CONDFMT`/`CF` records and a different reader. Both keep their seat.

### And the other seven `cfRule` families are read now — but 1215 rules move three renderings

**Closed in round 96**, `probes/cond-format-r96/`. `containsText`, `endsWith`, `containsBlanks`,
`notContainsBlanks` and `duplicateValues` are evaluated; `dataBar` and `iconSet` are left with
their seat (O22) and belong beside the colour scale rather than beside the conditions. Four of the
reference's arms are **not** what 18.3.1.10 would have you write, and each was confirmed against
26.2.4.2's own view of a corpus document as well as of an authored fixture:

- **`containsBlanks` is not a mode.** `CondFormatRule::finalizeImport`
  (`sc/source/filter/oox/condformatbuffer.cxx`:860-866) replaces it with the formula
  `LEN(TRIM(#B))=0` — `>0` for `notContainsBlanks` — and sets `ScConditionMode::Direct`. So **a
  cell holding only spaces is blank** and **a number never is**, because `TRIM` stringifies it
  first. `Application_Compliance_Checklist_5_Apr_2021.xlsx`'s 99 such rules come out of
  `--convert-to fods` as `formula-is(LEN(TRIM([.M270]))=0)`.
- **A text rule's `<formula>` is dead markup.** `finalizeImport`:938-947 builds a token array
  holding only the `text` **attribute**, interned; the `NOT(ISERROR(SEARCH("xxx",A373)))` Excel
  writes beside it is never compiled. Hence `contains-text("xxx")` in the fods, with no reference
  in it.
- **Case is folded for a text cell and not for a numeric one.** `IsValidStr`
  (`conditio.cxx`:1219-1229) lowercases both sides; `IsValid`'s numeric arm (`:1130-1143`)
  stringifies with `OUString::number(nArg)` and calls a bare `indexOf`. A numeric cell is searched
  as its **number**, not as what is drawn.
- **A multi-range `sqref`'s base cell is the smallest *column*, not the componentwise minimum.**
  `ScRangeList::GetTopLeftCorner` (`sc/source/core/tool/rangelst.cxx`:1142-1155) is the smallest
  range start under `ScAddress`'s `(tab, col, row)` ordering (`sc/inc/address.hxx`:396). **Excel
  writes the formula against the componentwise minimum**, so the two disagree: `$G376="N/A"` on
  `G443:G444 D491 G446:G490 G377` gets `base-cell-address="…D491"` in 26.2.4.2's own fods, and its
  sibling block resolves to `formula-is(#ref!="N/A")` because the shift takes the row negative. 15
  blocks in 3 corpus documents. On the fixture `sheet-cf-multi-range-anchor.xlsx` the two readings
  paint **opposite** cells and the reference paints the top-left-corner one.

**A fifth: precedence between two blocks is document order, then priority *within* a block** — not
a global sort on `priority`. A cell's pattern carries an `ScCondFormatIndexes`, a
`sorted_vector<sal_uInt32>` of format indices (`sc/inc/attrib.hxx`:271) handed out as each
`<conditionalFormatting>` finalises, and `GetCondResult` walks it in that order;
`CondFormat::insertRule` keys `maRules` by priority in a `std::map` inside one block.

***And the headline is the reach, not the fix: a rule count is not a reach figure.***
1215 unread rules over the corpus move **3 of 947 renderings** — sum `|ink|%` 27.62 → 27.02, no
page count anywhere — and the nine candidates that do not move are each explained rather than
broken. **358 `containsText` rules on `flightstandards-doc-Cross-reference-table_version02.xlsx`
search for `xxx` and no cell on the sheet holds it**, the leftovers of a search-highlight;
`Application_Compliance_Checklist`'s 668 are split between a `veryHidden` sheet and columns L and
M of a sheet that hides columns 10 to 19; 14 `duplicateValues` rules over 357 positions in three
documents find **no** value occurring twice; and 66 `notContainsBlanks` hits sit in helper columns
outside their printed block. So the 27.04, 21.67 and 18.17 ink seats these families were expected
to take are **not conditional formatting at all** and keep their seats. **Census what a rule
*paints*, not how many rules there are** — the arithmetic is cheap, it is the same parse, and it
would have re-scoped this round before a line was written.

Two instrument notes. **`wiley-cancelled-title-list` is exact**: 0.42 → 0.01 on ink, and its
`#FFC7CE` fill and `#9C0006` font operators go 0/0 → 2/2 against the reference's 2/2. And **count
text spans, not colour operators, when the question is which cells took a font colour** — the fill
count is subject to the reference's rectangle coalescing, while `get_text("dict")`'s per-span
colour is not: on `6880ac7361ca…ST Capability List` the two sides then agree span for span on all
sixteen of the reference's, and the two we add are §4's tab.

**And an extension-keyed census misses a mislabelled file.** `Special-Procedures_2025-07-10.xls` is
`Microsoft Excel 2007+` — a zip — with six `containsText` rules, and is one of the three movers. No
census filtered on `.xlsx`/`.xlsm` can see it.

***And the `iconSet` half of that census was corrected in the wrong direction: it is 20 rules in 10
documents, not 2 in 2.*** Round 96 took an earlier over-count of 20 down to 2 by parsing each
worksheet's `cfRule` elements instead of grepping for `type="iconSet"`, which is the right
instrument for the wrong element — **an `x14:cfRule` in a worksheet's `extLst` can be a rule of its
own** rather than an extension of a main-namespace one, carrying its own `xm:sqref`, and the
reference imports and paints it. The source says so in as many words:
`ExtLstLocalContext` builds a fresh format for an `x14:cfRule` whose `id` matches nothing, under
the comment *"an ext entry does not need to have an existing corresponding entry"*
(`sc/source/filter/oox/extlstcontext.cxx`:165-194). 18 of the corpus's 20 are of that kind, and
`088_To-do_list_with_progress_tracker`'s `H3:H7` set is the proof in one file: it exists nowhere
but the extension list and 26.2.4.2 draws an image for it. The test is whether some
main-namespace rule claims the `x14` rule's `id` through an `<x14:id>`; `dataBar` is unaffected,
because all nine of its extensions are claimed that way and r96's 9-in-6 is exact.
**Census the extension list as well as the sheet, and match the two by `id`.**
`probes/cond-format-r97/census-drawrules.py`.

***The document count in that paragraph said 9 for a day, and the round's own data file said 10.***
The two main-namespace `iconSet` documents (`075_Idea_planner_tasks`,
`sistem-rekod-markah-srm`) are **disjoint** from the eight `x14`-only ones, so the union is ten, and
`probes/cond-format-r97/icon-ink.txt` listed ten rows the whole time. **When a prose figure and a
banked table disagree, the table is the measurement** — the prose is a transcription of it.

***And `drawIconSets` does not paint the icon at a fixed ten points square.*** That constant
(`output.cxx`:967) is a fallback for a null `mnHeight`, and `GetIconSetInfo` **always** sets that
field from the cell's own `ATTR_FONT_HEIGHT` (`colorscale.cxx`:1222-1224), so the branch at
`:969-980` always wins and the glyph is as tall as the cell's font. The banked areas are the
refutation and were on disk before the claim was written: 19.4 pt² per icon on
`066_Agile_Gantt_chart` against 103.9 on `069_Blue_modern_balance_sheet`, where a constant ten
points would be 100 pt² throughout. **Divide a banked area by its count before quoting a size.**

***And `mbNeg` is not "an `x14:negativeFillColor` was stated" — it defaults to TRUE.*** A `dataBar`
read from OOXML therefore treats every negative value as negative, and one whose extension states
no negative colour paints it in the source's own `COL_LIGHTRED` (`colorscale.cxx`:1082) rather than
in the bar's positive colour. The default is at `sc/inc/colorscale.hxx`:107; the OOXML importer only
ever sets it true again (`condformatbuffer.cxx`:1658-1664); **the one place in `sc/` that clears it
is the ODF importer** (`sc/source/filter/xml/xmlcondformat.cxx`:483). So the fallback is not
unreachable from SpreadsheetML, it is the *usual* answer there — eight of the corpus's nine rules
resolve to it, and 26.2.4.2's `fods` writes `calcext:negative-color="#ff0000"` for every one.
**Reach is nil** (75 numeric cells under a corpus `dataBar`, 0 negative;
`probes/cond-format-r97/negative-under-databar.py`), which is exactly why the fixture that pinned
the arm — one that *did* state a negative colour — could not see it. **A fixture built to exercise
a feature will state the optional attribute; the corpus mostly does not, so author the absent case
too.**

### A wrapping cell whose text begins outside its own column draws nothing at all

**Only a wrapping cell is clipped to its column, and only a wrapping cell has a paper.**
`DrawEditParam::calcPaperSize` (`sc/source/ui/view/output2.cxx`:2684-2700) gives the EditEngine
`rAlignRect.GetWidth() − nLeftM − nRightM` under `if (rParam.mbBreak)` alone; a cell that does not
wrap keeps the initial `Size(1000000, 1000000)` and writes across whatever is beside it.
`calcMargins` (`:2665-2682`) adds `ATTR_INDENT` to whichever side the cell is aligned to, so an
indented cell in a narrow column reaches a **negative** paper — and a negative paper does not stop
the breaking: `ImpEditEngine::calculateMaxLineWidth` (`editeng/source/editeng/impedit3.cxx`:530-545)
ends with `if (nMaxLineWidth <= 0) nMaxLineWidth = 1;`, so every character takes a line of its own.
What suppresses the cell is **where the block starts**: laid out at the cell's left edge plus margin
plus indent, it meets nothing of the cell's own rectangle once that is past the column's right edge,
and `DrawText_ToRectangle` returns on `!aContentRange.overlaps(aClipRange)` (`impedit3.cxx`:3408-3440).

**The predicate a reading of `calcPaperSize` suggests — "the paper is not positive" — is wrong, and
it fits the first eight measurements.** Eight column widths of `075_Idea_planner_tasks` put the step
between 5.25 pt and 6.11 pt of column against a margin and indent of 5.70 pt, which both candidate
cliffs straddle. `features/sheet-narrow-wrap.fods` separates them: a **1.42 pt column with no
indent** has a negative paper and 26.2.4.2 draws `N A R R O W` on six lines in it. Four variants
with `indent="1"` removed are drawn in full at every width. `SheetTextLayout.StartsOutsideItsCell`
is `margin + indent >= column width` for that reason. Reach on the gate is **one document in 947 and
one in the 307 converted `.ods`**, both the same workbook, and both gain their verdict;
21 of 21 one-attribute variants agree with 26.2.4.2 afterwards. `probes/xlsx-chart-r84/`.

### To ask whether a workbook's divergence is its chart's, take the chart out

**`strip-charts.py` removes every `xdr:` anchor holding a `graphicFrame` and leaves the sheet
alone**, so the chart's own contribution is the difference between the two renderings on each side
separately — no guess about where on the page the chart sits. Over the thirteen `.xlsx` gate
failures the brief called "charts in workbooks": **six are the chart and nothing else, one is a
slicer's fallback shape, and six are the sheet with the chart taking no part.** Three of the
thirteen — and both of the two non-`chartset` `.xlsx` failures — hold **no chart part at all**;
`chartset-*` is a batch name and not a census, so census the zip before believing a batch label.

**Two of the six chart rows are a measured ceiling and cannot be won.** On
`057_Simple_balance_sheet` we draw twenty category labels as real text turned 45° and 26.2.4.2
draws **no turned text at all** on that page while carrying 331 glyph-sized filled paths against our
114 — the shear rule this file already records, arriving on the sheets track. `065_Weight_loss_tracker`
is the same shape, 120 fills against 12. Closing either would mean outlining glyphs to make a
text-extraction gate greener.

**And the volatile-date class is not what a `chartset` failure is made of.** Ten of the thirteen
carry `TODAY()`; freezing each at the serial its own cache was built with (`devolatile.py`) moves
**one** verdict and leaves the other nine's glyph deltas within five of where they were. A changed
date changes *which* characters are drawn and not *how many* — `4/14/2017` and `8/24/2026` are both
eight alphanumerics — so a volatile workbook drifts in ink and barely at all in the column the gate
scores. The corollary is the useful half: **`TODAY()` can be frozen at the file's own cached serial**,
which makes the reference recompute what the author saw and turns a decaying row into a stable one.

### A chart type LibreOffice "supports" is not one it draws, and two readers decided a type after reading a record

**`chart2/source/inc/servicenames_charttypes.hxx` declares nineteen chart types and
`VSeriesPlotter::createSeriesPlotter` (`chart2/source/view/charttypes/VSeriesPlotter.cxx`
:2882-2907) has a plotter for eleven.** The other eight reach `UnsupportedChart`, which draws
`STR_UNSUPPORTED_CHART_TYPE` and nothing else (`UnsupportedChart.cxx`:88-111). So *"the reference
supports a treemap"* and *"the reference draws a treemap"* are different statements, and
reproducing the eight means reproducing a string rather than a plot. `oox` says the same thing in
its own words: `SERVICE_CHART2_SURFACE` is `"com.sun.star.chart2.ColumnChartType"; // Todo`
(`oox/source/drawingml/chart/typegroupconverter.cxx`:79).

**The whole corpus was swept by chart type to find out where that matters, and the first cut of
that sweep published a wrong conclusion because `|ink|%` is the wrong instrument for a chart.**
168 documents holding a `c:` or `cx:` chart part, rendered both ways and grouped by the *set* of
plot elements their parts state. Ranked on unsigned ink, no type looked systematically wrong.
That was false, and twice over:

- **`|ink|%` and `diff%` are both fractions of the whole PAGE**, and a chart is a small part of
  one. `pie-chart-result.docx` scores **0.30** unsigned ink while the reference draws an extruded
  3-D pie there and this tree draws a flat circle — as wrong as a pie can be.
- **`|ink|%` is signed per region before the page's absolute value is taken**, so ink we add
  where the reference removes it cancels *within* the page. A wrong-shaped pie in the right
  colours is exactly the shape that cancels. `diff%` — the fraction of pixels that differ at all
  — has neither property, and reads 2.76 on that same document.

**Rank a chart on `diff%`.** (The standing *rank on the worst page, not on summed ink* rule is
not what failed here; the quantity is.) On that column the types separate into three groups:
right (`cx:` chartex 0.16, `areaChart` 0.39, `bubbleChart` 0.58), right in the main with bad
individual documents (`barChart` 2.81 over 55 with a worst of 28.50, `lineChart` 2.59,
`scatterChart` 2.93, `doughnutChart` 3.32, `pieChart` 3.57), and **wrong wherever they appear:
`ofPieChart` 11.00 over both its documents and `pie3DChart` 5.06 over all three of its.**
`probes/chartsweep-r186`.

**A 3-D chart's reach is three documents and all three are pies** — no `bar3DChart`,
`line3DChart`, `area3DChart` or `surface3DChart` exists anywhere in the corpus — so the 3-D work
the corpus can witness is the pie alone and a general 3-D scene buys nothing beyond it.

***Done, and four things about it are worth more than the fix.*** `ChartLayout.Pie3D` draws the
extruded solid; `021_Unit_Circle_Chart_3D_Pie_Chart` goes **12.20 → 5.64 `diff%`** and
`pie-chart-result` 2.76 → 1.66, with **2 of the 168 chart documents moving and 166
byte-identical**. `probes/pie3d-r187`.

- ***26.2.4.2 rasterises a 3-D chart.*** The solid reaches the page as one `Do` of an RGB image
  with a soft mask — the labels and the frame are still operators — so its geometry can only be
  read out of that bitmap's silhouette, and a vector rendering can never be byte-equal to it.
  **Do not chase a residual on one of these to zero.**
- ***The projection is parallel in practice and the squash is `sin(rotX)`.*** Over nine
  elevations the silhouette's upper boundary fits an ellipse to a mean residual of 0.28–0.45
  pixels on a 1370-pixel image, and `B/A` is within 0.7% of the sine at seven of the nine —
  although `View3DConverter` asks for `ProjectionMode_PERSPECTIVE`, because a `c:perspective` of
  30 halves to a non-zero 15 (`plotareaconverter.cxx`:299-308). The camera is far enough away
  that it does not show.
- ***`c:depthPercent` is not read.*** Thirty-six variants — nine elevations by four depths — give
  nine distinct geometries repeated four times each, identical to the hundredth of a point. The
  thickness comes from `PieChart::getPreferredDiagramAspectRatio`'s `Direction3D(1, 1, 0.10)`.
- ***A three-dimensional diagram is NOT squared, and that was worth more than the solid.***
  `VDiagram::adjustPosAndSize` branches on the dimension count (`VDiagram.cxx`:89-101) and
  `adjustPosAndSize_3d` fits the scene's own *projected* bounding box into the available
  rectangle (`:409-421`); the preferred ratio never squares anything. With the solid drawn but
  `ChartLayout.Squared` still squaring the rectangle, `021` came out 274 pt across against the
  reference's 469 and its `diff%` went **12.20 → 16.33 — worse than before the feature**.

**And closing it turned up a four-round-old gap one layer down: a chart's bold text was drawn
light on the whole words track.** `FrameChart.ChartFace` resolved one face and shaped every
label through it, so a chart's bold title, axis labels, axis titles, legend and data labels were
all light on a DOCX, a DOC and an RTF while the model carried the right value — `SlideChart` and
`SheetChart` having acted on `ChartLabel.IsBold` since they were written. The class's own remarks
named the gap and deferred it for want of a words sweep. **The face set is the instrument, not
`diff%`**: on `021_Unit_Circle_Chart_3D_Pie_Chart` the page went from embedding `Carlito-Regular`
to embedding `Carlito-Bold`, which is what 26.2.4.2 embeds, while its `diff%` went 5.64 → 5.68
because bold labels are wider and move where a label wraps. Reach **5 of the 10 chart-bearing
words documents — exactly the 5 that state a `b="1"` in a chart part** — and nothing on the other
two tracks, `ChartFace` having one consumer. `probes/chartbold-r188`.

**A per-point override has to be counted against what it inherits.** The first reading of that
defect was that `021` states its weight only on each per-point `c:dLbl`, which no reader reads —
taken from the first `c:dLbls` in document order, which is the per-point one nested inside the
series-level one. Censused properly, a per-point `c:dLbl` states a size, weight or colour
*beyond its enclosing `c:dLbls`* in **4 of the corpus's 699 chart parts**, and `021` is not one
of them: its series-level `c:dLbls` carries the same `<a:defRPr sz="1400" b="1">` after its four
`c:dLbl` children. An override that repeats its parent cannot be seen on the page, and a census
that does not subtract the parent sends a round after a reader gap that is not there.

**One instrument warning from the shading measurement.** A colour that is a flat multiple of a
series' declared colour is not necessarily a *wall*: `(64, 99, 41)` is accent6 × 0.574 and is the
10% sector's **top face**, in the darker shade `c:varyColors` hands the fourth point. Locate a
colour's pixels before deciding what surface they are — reading that one as the wall would have
made every wall 0.57 of its face instead of the measured 0.87.

**What the same review did find is two readers that switch on a record id where the reference
switches on a field inside it.** Both are closed, and the shape is worth remembering because it
is invisible to a `grep` of type names:

- **ODF: an unrecognised `chart:class` is a bar chart, not nothing.**
  `SchXMLChartContext::ParseAttributes` sets the service name only for a `chart:`-namespace class
  `GetChartTypeEnum` knows, or for the `ooo:` add-in branch
  (`xmloff/source/chart/SchXMLChartContext.cxx`:372-402); everything else falls through to
  `:470-478`, which reads `aChartClass_Bar`. Measured on 26.2.4.2 over fifteen one-attribute
  variants, the reference draws the same 126 path operators for every one of them as it does for
  `chart:bar` itself, and `OdfChartPlot` drew nothing. The `ooo:` spelling is the exception and is
  left returning null. `probes/chartclass-r186`.
- **BIFF: a doughnut and a bubble chart have no record id.** `XclImpChType::Finalize`
  (`sc/source/filter/excel/xichart.cxx`:2288-2303) reads the body first —
  `(mnPieHole > 0) ? DONUT : PIE` and `mnFlags & EXC_CHSCATTER_BUBBLES` — and a bubble series'
  sizes arrive on source-link destination **3**, `EXC_CHSRCLINK_BUBBLES`, beside title 0, values 1
  and categories 2. `XlsChartReader` read none of the three. On 26.2.4.2's own
  `--convert-to 'xls:MS Excel 97'` of the corpus chart workbooks, worst-page `|ink|%` falls
  0.88 → 0.37 and 0.36 → 0.16 with six controls unmoved to the hundredth.
  `probes/chartbiff-r186`.

**Three fields of those two records are read by the reference and then used by nothing**, so
reproducing them would be a disagreement rather than a refinement: `CHPIE`'s `anStart` becomes a
`StartingAngle` that `ChartPlot` does not model (and neither does `c:firstSliceAng`), and
`CHSCATTER`'s `pcBubble`/`wBubble` are the BIFF spellings of `c:bubbleScale` and
`c:sizeRepresents`, which `oox` also parses and ignores. `pcDonut` is dropped once the type is
chosen, because a ring's hole is always half the radius.

**And the reach of both is nil on this corpus, which is the point rather than a disappointment.**
Every `chart:class` the converted-ODF corpus states is one the reader already drew; exactly one of
the corpus's 180 OLE2 files states a `CHPIE` or `CHSCATTER` at all, and it is a plain scatter that
was already right. The corpus is seven OOXML and MS-binary extensions and holds no hand-written
ODF, so a reach census is the wrong instrument for an ODF reader rule — the right one is the
reference's own answer to an authored file, which is what both probes measure.

**Two census traps recurred here and both are already in this file under other headings.** A
chart census keyed on `.xlsx`/`.pptx`/`.docx` misses a `.xlsm` — the first cut of the sweep above
was 167 documents and is 168 — and one keyed on *zip parts* misses every BIFF chart substream,
which is the same error that made an older reach figure short by eight. **Census the package's
contents and walk OLE2 files, not extensions.**

### A bar on a date axis is placed by its date, and for years it was placed by its index

**`ChartLayout.AddBars` put every bar at `(double)at / categories`, whatever the axis was.** A
line series has gone through `ChartDateAxis` since it was written, so on a date-axis combination
chart the line spanned the plot and the bars did not: on `171128IPAP.pptx` page 40 this tree drew
**67 bars across the left two-thirds of a plot where 26.2.4.2 draws 44 across the whole of it**,
at a pitch that changed partway along — which is the signature of a layout that does not know
what its x coordinate means. Page 40 is now **16.01 → 7.94 `diff%`** and MAJOR → shifted, with
its bars at 350.07…635.85 pt against the reference's 350.05…635.75. `probes/bardate-r189`.

Four rules, and only the first two are readable from the source alone:

- **A category is one unit of the axis' own time resolution wide**, not one n-th of the points —
  `PlottingPositionHelper::setTimeResolution` sets it to 1, and to 12 at year resolution because
  the scaling counts months (`PlottingPositionHelper.cxx`:670-690).
- **The axis is linear in that unit and not in days.** `DateScaling::doScaling`
  (`chart2/source/view/axes/DateScaling.cxx`:56-91) returns `year × 12 + month` plus the fraction
  of the month elapsed. Over 132 months the difference is under a tenth of a percent; over eleven
  it is 5%, and 26.2.4.2 spaces `044_Cash_flow_forecast`'s twelve monthly bars at a flat 35.26 pt
  where days give 30.2 to 33.4. **A date span measured in days over 30.44 is not the span in
  months.**
- **A shifted axis carries one extra interval** — `"for explicit scales we need one interval more
  (maximum excluded)"`, `ScaleAutomatism.cxx`:565-597 — **whether or not the range is stated**,
  and the interval is chosen from the span *after* the extension, so it moves every tick too.
- **And a shifted axis' categories lead their own dates**, where an unshifted axis' straddle
  them. That half is *not* in the source: `CategoryPositionHelper::getScaledSlotPos` subtracts
  half a category width unconditionally and something adds it back. It was solved instead.

***The experiment that solved it is the one to copy.*** One rendering cannot separate "centred on
the date" from "running from the date", because the plot's left edge is unknown and both readings
fit at different edges. Render the same file twice with two *different stated ranges* and the
edge has to come out the same — and, better, **read the plot's own rule out of the page instead
of inferring it**: the widest horizontal is the plot rectangle, 414.31 in both variants. The
first cut of this probe inferred the edge from the bars, concluded "centred", and was refuted by
the tick row in the same PDF.

**Which arm applies is the crossing *value* axis' `c:crossBetween`**, read as `== "between"` with
a fallback of true for a bar, line or stock group
(`oox/source/drawingml/chart/axisconverter.cxx`:292-301). Measured: the same file with the
attribute deleted renders identically to `between`, and rewritten to `midCat` loses the extra
interval and straddles. **`ChartDateScale.Resolve`'s own default is `false`**, because that OOXML
fallback belongs to the OOXML reader — the BIFF reader passes nothing and keeps the geometry it
had, `CHVALUERANGE`'s crossing flag being unmeasured.

**Reach 7 chart parts in 5 documents by markup; 8 of the 168 chart-bearing documents move and 160
are byte-identical**, the three extra being charts whose *lines and ticks* moved with the
month-linear axis. Two improve (16.01 → 12.91 and 2.57 → 2.12), five are level, one worsens by
0.05 on a chart already within 0.13 pt.

### An of-pie's composite wedge, its bar and its opening angle are `PieChart`'s, not the file's

**Read a pie's geometry out of the page's fill operators.** `pdf-ops.py dump --only fill` gives
every filled path's colour and bounding box, and a wedge's path runs centre → rim → centre, so
the boxes fix the centre, the radius and each wedge's span exactly and the colour is read rather
than sampled. A blind reading of the same page named five differences and **two were wrong** — it
read the reference's legend as 12 entries where the operators show all 16, and the bar's split as
75/25 where they give 47.7/52.3. It had the *direction* right on the rest, which is what a
reading is for.

Three things the reference decides for itself on `028_Unit_Circle_Chart_Optimized_Graph`, the
corpus's one drawn `c:ofPieChart`:

- **The composite wedge's colour is not in the document, and the document states one.** That
  series carries sixteen values and *seventeen* `c:dPt`, the last of them `accent4 lumMod 50%` =
  `#806000`; 26.2.4.2 draws `#7E0021`. `propIndex` gives the composite the property index
  `getTotalPointCount()` (`PieChart.cxx`:1185-1191) — one past the last value, **always** — and
  `DataSeries::getDataPointByIndex` answers an empty reference outside
  `0 <= nIndex < getData().getLength()` (`DataSeries.cxx`:312-337), so the import drops that
  `c:dPt`, `hasPointOwnColor` is false, and `createOneRing` falls through to
  `m_xColorScheme->getColorByIndex` (`:1325-1331`). That is
  `Office.Chart/DefaultColor/Series` — twelve values in
  `officecfg/registry/schema/org/openoffice/Office/Chart.xcs`:35-36, indexed `nIndex % 12` — and
  `16 % 12 = 4` is `0x7E0021`. Reproduced deliberately; it disagrees with the writer's intent and
  agrees with the reference.
- **The bar stacks upward in data order**, so the *first* split point is its foot:
  `createOneBar` opens at `fBarTop = -0.5` and adds each share, `// make the bar go from -0.5 to
  0.5` (`:1416-1430`), against a value axis that points up.
- **The main ring opens at minus half the composite sweep**, so the composite closes across three
  o'clock where the two connecting lines meet it — `sAngle`'s
  `clockwiseWedges() ? 360 - degAng : degAng` (`:1229-1244`). Opening at plus half puts the whole
  composite above the axis.

**What the of-pie constants already had right, measured the same way:** the unit centre to 0.6 pt,
`bar width / unit radius` 0.747 against 0.750, `bar height / unit radius` 1.495 against 1.499. So
`m_fLeftShift`, `m_fLeftScale` and the three bar constants are exact and **only the unit radius is
wrong, by 1.37×** — round 104's figure, still open.

**And the labels went through a placer of their own, which is also why the radius is stuck.**
`AddOfPie` put each label flat on its bisector at half the radius where a plain pie goes through
`PieLabels`, and `PieConsumedRect` — which is what the pie's second pass shrinks against —
measured them **on the unit circle**, which nothing is drawn on, rather than on the main ring at
two thirds of it. Both are now the same placer at the same place, and on `028` it changes nothing,
because that chart states `<c:dLblPos val="inEnd"/>` and both placers take the fixed branch. The
arithmetic says what is missing: for the reference's final square to be 270.9 rather than 370.68
its pass-1 consumed rectangle must be **268.27** tall against our 168.49, about 50 pt of label
overflow each way — and `INSIDE` is not half the radius, it is **the rim less a flat 150** in the
radius direction (`PieChart.cxx`:439-452). That is the next measurement.

**Reach 1 of the 168 chart documents and 167 byte-identical.** `029`, the other `c:ofPieChart`,
is exploded and both renderers draw it as a plain pie. `probes/ofpie-r190`.

***And a page-fraction metric cannot see any of it on that document, because the chart is 71.6 pt
out of place.*** `028` goes **16.48 → 16.72 `diff%`**; scored against a reference rendered from a
variant that puts 26.2.4.2's chart where ours is and changes nothing else, the same two
renderings give **15.21 → 14.47**. When a chart is displaced by more than its own features, the
headline number measures the displacement and nothing else — **render the reference with the
displacement removed and score against that.**

### An autofitted slide body shrinks its text and not its bullet's label, and the first line moves

**The width half of a rule this file already had the height half of.** `Outliner::ImplGetBulletSize`
measures the bullet once and caches the whole `Size` on the paragraph, and the autofit search formats
the outliner once *unscaled* before it walks `constScaleLevels` — so the cached width, like the
cached height `SlideFittedBulletBoxTests` already pins, is the unfitted one. EditEngine then takes
that width for the first line alone, `nStartX = max(textLeft + firstLineOffset, bulletX)`
(`editeng/source/editeng/impedit3.cxx`:846-851 over the `BulletX` set at `:798-802`), while
`StripBullet` asks `ImpCalcBulletFont` again and *does* scale what it draws. So a hanging indent wide
enough for the shrunk bullet can still be too narrow for the cached one:

```
firstLineTextX = max(marL, marL + indent + advance(bullet at the run's STATED size))
```

`Sector_Skills_Insights_Advanced_Manufacturing_summary_slide_pack.pptx` page 13 is the witness. Its
master states `lvl2 marL="742950" indent="-285750"` — a 58.5 pt margin with the bullet 22.5 pt to its
left — and its runs state **56 pt** under a `normAutofit` that draws them at 14. Its en-dash bullet is
0.5562 em, so 31.13 pt at 56 and 7.79 at 14: 26.2.4.2 starts every second-level first line 31.13 pt
past the bullet and this tree started it at the 22.5 pt margin. **Pinned over twelve one-attribute
variants of that one master** (`probes/slidebullet-r191`): four hanging indents and two margins give
five first-line positions exact and two clamped as predicted; three bullet characters give `–` 31.13,
`W` 52.87 and `i` clamped; three stated run sizes give 28 pt clamped, 56 → 31.13 and 112 → 62.25
against 15.57, 31.14 and 62.29 predicted.

**Two variants that decide nothing and are worth knowing.** The bullet's own stated size — the
`<a:defRPr sz="2800">` on the same `lvl2pPr` — moves the offset not at all at 14, 28 or 56 pt, so the
label takes the *run's* size and not the level's. And **rewriting `fontScale` or deleting it outright
changes nothing, because LibreOffice recomputes the autofit rather than honouring the stated scale** —
all three variants draw the text at 14.00 pt. A probe cannot vary the shrink through the file; the
unshrunk size has to be read off the runs.

***A generated NUMBER is the exception, and the confinement sweep is what found it.*** Of the ten
documents the unrestricted rule moved, nine improved and
`30-04-2021 merged NDoH and NICD_Presentation…pptx` went 85.02 → 90.72 summed `diff%`, all of it on
page 9, whose second level is numbered: its five first lines moved from 63.01 to 65.74 where 26.2.4.2
draws **62.90**. The cache's own key explains it — `Paragraph::IsBulletInvalid` compares the bullet's
**text** as well as the scaling parameters, so a number, whose text differs per paragraph, refills the
cache while the search already has a scale. Restricting the rule to a fixed character keeps every gain
and removes the loss. **So the one-line version of this is: a character bullet's label is measured
before the fit and a generated number's after it.**

**It is a cascade, which is what makes it worth more than its 8.63 pt.** Page 13 was `diff%` **15.93**
at `|ink|%` **0.14** — the signature of a page where nothing is drawn differently and a great deal is
drawn in the wrong place: our line fitted 94 glyphs where the reference's fitted 92, so a two-line
paragraph took three, everything below dropped a pitch, and the reference's last line is clipped by
the slide's edge. After the fix the reach is **31.15 against 31.13**, the two sides draw the same 49
text records, and the page is **15.93 → 13.13** — the rest of that page being a 0.4 pt baseline drift
and the reference's own clipped last line.

**Reach: 9 of the 302 slides documents move and 293 are byte-identical, and all nine improve** —
summed `diff%` over them 1148.05 → **1121.17**, MAJOR pages 14 → 14. One worst page rises,
`5b_upasana_dasgupta` 9.98 → 10.27, while that document's sum falls by 3.04. The slides track is the
whole reach by construction: `SlideTextLayout` has no consumer outside `XRay.Office.Presentations`, and
the two mentions of it from `XRay.Office.Text` and `XRay.Office.WordProcessing` are doc comments. **No
gate column can see any of it** — a changed wrap keeps the same characters and a slide's page count
is its slide count.

**And the same page taught a census rule again**: that deck heads the chart ranking at 15.93 and its
worst page holds **no chart at all**. Census which page the feature is on before working the number.

***A second lead from the same deck, read blind and not measured: our drop shadow ignores its blur and
its lateral offset.*** On page 11 ours is a uniform RGB 153 band 4 rows deep offset straight down by
~2.2 pt with **zero** spread to the left, the right or above, where 26.2.4.2 ramps 157 → 249 over
about 11 rows below, 8 px right, 7 px left and 3 px above — a 4–6 pt blur offset down *and* to the
right. That is what an ignored `a:outerShdw/@blurRad` with a substituted vertical `@dist` looks like.
It is not only cosmetic: this file already records that **LibreOffice rasterises a blurred shadow**,
so its PDF holds a picture with no words where a hard one stays vector and searchable — which makes
the text layer the cheap instrument for it. The same reading found a shadowed white container the
reference draws and we appear not to, plausibly the same defect (under a no-blur no-lateral shadow the
only visible strip is covered by the box below), and one difference where **the reference is the wrong
one**: its first teal box is 11 px shorter than ours and its last line's white glyphs fall onto white
paper.

### A hatch's distance is a number and the drawing layer decides the unit — Writer's is twips

**Measured, implemented, and withdrawn; the patch is banked at `probes/charthatch-r191/withdrawn.patch`
and nothing in `dotnet/src` carries it.** Read this before sending a round after a chart's pattern
fill: the feature works, the reach is three documents, and what blocks it is a *hatch phase* rule
shared with every slide shape.

**`a:pattFill` on a chart's own backdrop is real ink the reference draws and we draw none of.** On
`028_Unit_Circle_Chart_Optimized_Graph.docx` 26.2.4.2's page carries **298 strokes, of which 295
are the `dkDnDiag` hatch** of the chart area. `ChartPlot.Background` and `ChartPlot.PlotBackground`
are `Colour?` and have nowhere to put one.

**Census the element the fill sits on, not the string.** Parsing all 947 documents' chart parts:
`c:chartSpace/c:spPr` holds 161 `noFill`, 113 `solidFill`, **1 `gradFill`** (`Intersil_Italy_CAN_Bus…pptx`)
and **1 `pattFill`** (`028`); `c:plotArea/c:spPr` holds 222, 47, 0 and **1** (`065_Weight_loss_tracker.xlsx`).
That is the whole non-solid chart-backdrop surface of the corpus: **three fills in three documents**.
A `grep` for `a:pattFill` finds 13 in 6 documents and **11 of them are in `style1.xml`**, the chart
*style* part, which LibreOffice does not read at all.

***The transferable finding is not about charts: a Writer document's drawing layer works in twips,
so a hatch's stated hundredths of a millimetre are drawn as twips — 2540/1440 = 1.7639 times as far
apart.*** Measured over four one-attribute variants of that document's `a:pattFill/@prst`: `dkDnDiag`
states 50 and draws 88.4, `wdDnDiag` 100 → 176.3, `ltHorz` 50 → 88.4, `dkHorz` 25 → 45.9 — flat
1.76–1.77 with **no dependence on the angle**. Confirmed out of VCL as well:
`CalcHatchValues`' `nDist = fround(nDist / cos(45°))` gives `round(50/cos45) = 71`, and **71 twips
is 3.55 pt, which is exactly the corner step on the reference's page**; in hundredths of a
millimetre the same arithmetic gives 125 = 3.5433 pt, which the page does not hold. **The other two
tracks do not do it**, which is what makes it the unit and not the chart: on
`slide-pattern-fill.pptx` this tree already draws 112/70/70/70/28 hairlines against 26.2.4.2's
115/71/71/70/28 with no correction, and on `065`'s hatched wall the reference draws 238 lines where
the stated 100 predicts about 219.

**What blocks it is the phase, and there are two phases in evidence.** With the twips correction the
implementation drew **296 lines against the reference's 295** — and in the reference's *gaps*. Read
out of the raw content streams: our family's invariant `x + y` runs `218.045 + 3.5356k` and
26.2.4.2's `1259.239 − 3.55k`, and a line through the chart rectangle's **top-left corner** is
`k = 167` exactly in theirs and `k = 126.81` in ours. `CalcHatchValues` phases on
`rRect.TopLeft()`; `Hatching.Family` deliberately phases on the box's *centre*, reproducing
`GeoTexSvxHatch`, and **that has its own measurement behind it** (its remark records
`BMFE-06-03 (Gerflor)` page 3 at 3.28 of unaccounted ink centred against 0.00 in phase). Re-checked
here: on the slide fixture the two phases agree to **0.296 pt out of a 3.97 pt step** and a
corner-phased family would be a half-step out. So one rule does not fit both, and choosing needs the
corpus's 65 pattern fills in 7 decks re-measured — which is the next round's, not a line of
arithmetic.

**What it scored, which is why it is withdrawn**: `028` against the `incell0` reference 14.42 →
14.43 `diff%` with `|ink|%` 3.58 → 3.95, and `065` page 1 26.71 → 26.70 with 2.88 → 3.24. Flat both
ways and about 0.36 of `|ink|%` worse on each — exactly what a correct hatch half a step out of
phase costs. The third document is the gradient, which the patch leaves collapsing to one colour
because `DrawingGradient.Paint` needs the rectangle it fills and a chart's frame is not known until
layout.

**Two instrument warnings from it.** A `pdf-ops.py` stroke record is a **bounding box**, and for one
of the two diagonal senses its corners are *not* the segment's endpoints — so a midpoint-based
reading reports 295 strokes as 42 lines and 522 as 3. Count strokes and divide the box's projected
extent by them (`probes/charthatch-r191/spacing.py`), or read the content stream's own `m`/`l`. And
**LibreOffice draws a gradient chart background as a Form XObject of band fills where this tree
writes one shading**, so a fill count is not comparable on the gradient document either.

### A chart label's width includes its trailing blank, and an accounting axis puts one on every tick

**A chart label is an EditEngine text shape autogrown around its paragraph whole, so its trailing
blanks are part of its width** — and the value axis reserves its widest label's width, so a lost
blank makes the band narrow and the plot area that much too wide, with every gridline and every bar
in it displaced. Excel's accounting formats end their positive and zero subformats in `_)`, a blank
the width of a closing parenthesis, so this fires on every currency axis.

**Only the slides path had it, and the reason is the mechanism rather than the track.**
`SlideChart.Measurer` sums the advances `SlideTextLayout.Place` emitted, and `Emit` runs to
`TextLine.VisibleEnd` — which exists precisely to keep a line's trailing blanks out of its width,
because they hang past the right edge rather than pushing a word onto the next line. That is right
for a wrapped body and wrong for a label that never wraps. `SheetBandText.ChartShape` and
`FrameChart.ChartFace.Shape` shape the string and sum every advance, so both were already right.
**This is not the rule round 190 refuted** — *"a slide line break charges its trailing blank"* is
about where a line **breaks** and stays refuted; this is the width of a paragraph that does not
break at all.

**Measured by deleting one token of one format and nothing else** on `Demick_JetBlue.pptx` page 5
(`probes/chartblank-r191`), reading the plot rectangle out of the extent of the major gridlines with
`pdf-ops.py dump --only stroke` grouped by colour:

| rendering | plot left | widest label's ink right | gap |
|---|---:|---:|---:|
| ours, base | 161.99 | 159.33 | **2.66** |
| 26.2.4.2, as authored | 165.71 | 159.80 | **5.91** |
| 26.2.4.2, `_)` deleted | 162.65 | 159.80 | **2.85** |
| 26.2.4.2, the zero row's `??` deleted | 165.71 | 159.80 | 5.91 |

The base tree renders the reference's *`_)`-deleted* answer, and the `??` control does not move
because the zero row is not the widest label. Deleting `_(` instead moves the edge by the same blank
again — **both ends count and only the trailing one was lost.** On the authored fixture
`slide-chart-accounting-axis.pptx` the wall's left edge goes **128.70 → 131.41 against 26.2.4.2's
131.41**, exact with no free parameter; `SlideChartTrailingBlankTests` pins it.

**Reach: 4 of the 67 chart-bearing decks move and 63 are byte-identical.** Three improve and none
worsens — `Demick_JetBlue` worst 20.31 → 19.03 and summed 86.78 → 82.14 over four pages,
`southern-classic-kennesaw-state-university-final` 75.64 → 74.94, `Sector_Skills_Insights` 139.85 →
139.74, `171128IPAP` level in pixels although its bytes moved. **No gate column can see it** and
that is checked: the four movers' alphanumeric and page counts are identical before and after,
because a blank is not an alphanumeric character.

**Two traps.** Replacing `_)` with `)` leaves a literal closing parenthesis behind and answers a
different question — delete both characters. And a glyph-count difference between the two sides is
*not* evidence here: our PDF draws 14 glyphs for `" $1,200,000.00 "` and the reference 15, but that
is the writer declining to emit an inkless trailing space and it is true before and after the fix.
**Measure the gap from the last glyph's ink to the plot's edge, not the glyph count.**

***What is still open on that page, measured and not taken.*** The chart's text is drawn at 10.01 pt
where the reference draws 9.89 at a horizontal scale of 9.92 — the anisotropic fit squeezing a chart
whose labels overflow — which is 0.6% of every advance and the whole of the residual 0.62 pt. The
plot is 190.58 pt tall against 186.98 with the top edges 0.46 pt apart, so the reference gives up
4.06 pt more below it: the category axis is interior here (the `$-` gridline) and `PlotAreaOf`
reserves nothing for an interior axis' labels, which is right as far as it goes —
`VDiagram::adjustInnerSize` then shrinks the inner rectangle by how far the drawn labels *overflow*
the available one, and we do not. Our rotated labels also hang about 3.3 pt further below the axis
line than the reference's, so **the overflow shrink and the label depth have to be separated before
either is worth implementing.** The legend sits 4.20 pt lower in the reference and the rotated axis
title 4.09 pt further right. And 26.2.4.2 **outlines** that page's 26 turned category labels — 156
glyph-sized fills, no text records at all — so part of the page is the shear ceiling.

***And the tiled ground is not the answer, which a round could easily have spent itself on.*** That
page's theme repeats a 5 × 5 px bitmap through `<a:tile sx="65000" sy="65000"/>` about 37 000 times
and our pitch is 3.25 pt against the reference's ≈ 3.2325, so the phase drifts across the page — but
the plot rectangle is what the 20.31 was, and the blind reader called the tiled ground *"the closest
match on the page"*. Censused for whoever takes it: **91 documents, 400 `a:tile` elements** — by part
`theme` 171, `slides` 160, `document.xml` 41, `slideLayouts` 16, `slideMasters` 8, `charts` 4 — the
heaviest being `048_Visual_Product_Roadmap_Template_Quality_Layout` (26) and
`082_Infographic_Funnel_with_4_Stages_for_PowerPoint` (24). `Demick_JetBlue` is not in the top
fifteen.

### An `inEnd` pie label is at the rim, and an of-pie's radius is its BAR's labels

**`INSIDE` and `OUTSIDE` share one anchor and differ only in a sign.**
`PolarLabelPositionHelper::getLabelScreenPositionAndAlignmentForUnitCircleValues` takes the
ring's *outer* radius for both and the middle of the ring for everything else — `bCenter` is
exactly `nLabelPlacement != OUTSIDE && != INSIDE` (`PolarLabelPositionHelper.cxx`:68-76), and
`AVOID_OVERLAP` has already become `CENTER` by the time it is asked
(`PieChart.cxx`:434-437) — and `createTextLabelShape` then pulls an `INSIDE` anchor **back**
along the radius by the same flat 150 hundredths of a millimetre it pushes an `OUTSIDE` one out
by (`:439-452`, applied at `:490-497`). The eight-row alignment table is `bOutside ? A : B` with
B the opposite of A on every row (`:112-137`), so the block hangs back over the slice instead of
away from it; the wrapping width stays the default `0.8 * fPieRadius`, the room-to-the-edge arm
being guarded by `nLabelPlacement == OUTSIDE` (`:544-576`). This tree treated `inEnd` as
`CENTER` and drew it at half the radius.

Measured on `029_Unit_Circle_Chart_Pie_Theme`, each label's distance from the pie's centre as a
fraction of its radius — the ratio, because the two pies differ by 2 pt:

```
reference  0.804  0.839  0.917  0.953  0.890  0.923  0.841
before     0.414  0.451  0.563  0.588  0.527  0.549  0.467
after      0.806  0.841  0.918  0.953  0.891  0.924  0.842
```

**Seven of seven within 0.002 with no free parameter**, and the distance is to the *block*
rather than to the anchor, so it fixes the mirrored alignment as well. Reach: the corpus's pie
charts state `inEnd` on **76 points in 5 documents** and `outEnd` on 37 in 4, against 188 points
in 19 that state nothing; **5 of the 168 chart documents move and 163 are byte-identical**, four
improving or level.

***And the of-pie radius is not the ring's labels — it is the bar's, through a unit bug.*** All
sixteen of `028`'s ring labels sit within 93 pt of a centre whose radius is 90.3 in the
reference's own rendering, so they never leave the ring and cannot consume anything. The bar's
do: `createBarLabelShape` sets
`fTextMaximumFrameWidth = 0.8 * (m_fBarRight - m_fBarLeft)` (`PieChart.cxx`:780) where those two
are `1.25` and `0.75` in **unit-circle logic** units, so the wrapping width is `ceil(0.4)` = **1
hundredth of a millimetre** and every bar label is drawn one character per line. Measured on the
reference's page: two columns at x ≈ 437.8, pitch 10.47, of **17 and 25 rows**, centred on the
two segments to 2.4 and 3.2 pt. That is what makes the pie's second pass shrink the diagram, and
`AddOfPieBar` emits the label unwrapped while `PieConsumedRect` does not measure it at all.
**Do not model the line count from either obvious guess** — "one line per non-space character"
predicts 17 and 21 and "one per character" 19 and 26, so one is exact on one label and wrong on
the other and neither is settled.

### A `relativeFrom="page"` vertical anchor counts from the page — unless the object is EMBEDDED, and then it counts from the page BODY

***Closed. The missing variable was the object kind, and the section below is kept because its
refutation is what a round would otherwise re-derive.*** An embedded object — a chart or an OLE
object — follows the text flow whatever its anchor, so *the page* means the page body frame for it
and not the sheet. The seat is **one unguarded line**: a chart's `a:graphicData` becomes a
`com.sun.star.drawing.OLE2Shape` (`oox/source/drawingml/shape.cxx`:348-358), `DomainMapper_Impl`
replaces that with a `SwXTextEmbeddedObject` (`:5096-5112`), and `:9792` then sets
`IsFollowingTextFlow` straight from `layoutInCell` **with no `IsInTable()` test** — where both of
`GraphicImport`'s own writes of the property have one (`:1316-1318`, `:1859-1861`). So a census of
`PROP_FOLLOW_TEXT_FLOW`'s writers that finds "always false outside a table" is a census of the two
guarded seats, and this project carried that conclusion for three rounds. And `m_bLayoutInCell` is
initialised **true** (`GraphicImport.cxx`:340), so an anchor stating nothing follows the text flow too.

`SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrame`
(`environmentofanchoredobject.cxx`:64-95) is where it all comes from: `FindPageFrame()` when the object
does not follow the text flow, otherwise a walk up to the first cell, fly, header, footer, footnote,
**page body** or page frame. Four things follow, and only the first is the 71.6 pt: the `PAGE_FRAME`
base (`tocntntanchoredobjectposition.cxx`:590-596); the object being captured at all, since
`mbDoNotCaptureAnchoredObj` is a product with `!mbFollowTextFlow` in it; and its being captured in the
**sheet** rather than the body, because the `compatibilityMode` 15 narrowing needs
`rPageAlignLayFrame.IsPageFrame()` (:562-566) — the opposite of what the flag's name suggests. The
**horizontal** is untouched, and that is the same file's own doing:
`GetHoriEnvironmentLayoutFrame`'s walk stops at a cell, a fly or a page and has no body frame in it.

***A fourth consequence was implemented on a correct reading of the source and the corpus refuted it in
one document — read this before trusting a `bCheckBottom` argument you have found.***
`bCheckBottom = !DoesObjFollowsTextFlow()` really is written at `tocntntanchoredobjectposition.cxx`:457,
:678 and :718 — and the offset arm has a **fourth** `AdjustVertRelPos` call, at :810-813, which
**omits the argument** and so takes the `= true` default (`anchoredobjectposition.hxx`:185-192). That
call is under *"do not follow text flow respectively align at 'page areas', but stay inside given
environment"* and is the path an offset which does not fit its upper's print area takes, which is the
path a page-relative chart reaches. `027_Unit_Circle_Chart_Graphical_Chart` decides it: 470.30 pt of
chart at a body base of 156.95 + 111.65 would reach 738.90 on a 595.30 pt landscape page and 26.2.4.2
draws its top at **125.00**, exactly `595.30 − 470.30`. Skipping the correction drew it at 268.60 and
took the document from 9.48 to **28.42** `diff%`; keeping it draws 470.30 against the reference's
470.30. **Grep for every call of a function whose guard you are modelling, not for the guard.**

**The experiment is the one to copy, because the refuted fixture below differs from it by one
variable.** Eight minimal DOCX stating the same `wp:anchor` at the same 180 pt offset, varying the
object kind, `w:pgMar/@w:top` and `layoutInCell` (`probes/pagev-r192/fixture.py`). 26.2.4.2 draws the
**shape at 180.00 at every margin** and the **chart at 252.35** (`w:top="1440"`), **324.35**
(`w:top="2880"`) and **180.35** (`layoutInCell="0"`), with an absent `layoutInCell` matching
`"1"` — so the kind decides it, the base tracks the top margin one for one, and the attribute
switches it. This tree reproduces **8 of 8** within the two writers' own 0.36 pt border-origin
constant. Reach by markup: **5 embedded-object anchors in 5 documents**, all five `Unit_Circle`
charts, `page` on two and `margin` on three — and `margin` is `PAGE_PRINT_AREA`, which resolves to the
body either way, so the base moves two documents and the capture rules reach all five. Measured
rather than censused: **2 of the words track's 337 renderings move and 335 are byte-identical**, the two
being exactly the `page` pair, `027` going 9.48 → **4.61** worst-page `diff%` and `028` 16.78 →
**14.18**. No gate column can see a frame's position, and `|ink|%` rises on both (0.56 → 0.73, 3.32 →
3.60) because it is signed per region before the page's absolute value — a frame moving *into* place
cancels less than one sitting 72 pt away, which is worth knowing before reading that column as a
regression.
**`PAGE_PRINT_AREA_TOP` shares `PAGE_FRAME`'s branch and is deliberately left**: no corpus DOCX pairs
a `topMargin` relation with an embedded object, and the band's height under following is unmeasured.
`probes/pagev-r192/results.md`.

---

*What follows is the round that measured the displacement and could not explain it. Read it for the
refutation in its second paragraph, which is correct and is exactly why the rule is not "page means
margin".*

Measured, mechanism not found, **not implemented**. On `028_Unit_Circle_Chart_Optimized_Graph`
26.2.4.2 places the chart's `<wp:positionV relativeFrom="page"><wp:posOffset>` **71.6 pt lower
than the offset states** — the frame is the same size and every line of body text agrees to a
twentieth of a point, so only the frame moves. One-attribute variants
(`probes/ofpie-r190/variants.py`) put the rule beyond doubt: `margin` renders identically to
`page`; offset 0 gives 72.36 and offset −36 gives 36.36, so it is linear and not a clamp at the
margin; offset −200 gives 0.36, so it *is* clamped to the page; and the base tracks
`w:pgMar/@w:top` one for one — `w:top="0"` renders it exactly where this tree does.
**`layoutInCell="0"` also renders it exactly where this tree does**, although the object is not
in a table and `IsInTable()` guards every place `GraphicImport` acts on that attribute
(`GraphicImport.cxx`:777-788, `:1315-1317`).

**The general rule is the page edge and this tree is right about it**, which is what kept this
from being "fixed": `probes/ofpie-r190/page-anchor-fixture.py` builds a minimal DOCX — one
`wps:wsp` at `relativeFrom="page"`, `layoutInCell="1"`, and a `word/settings.xml` so the OOXML
compatibility defaults apply — and 26.2.4.2 draws it at the page's own top edge at both
`w:top="1440"` and `w:top="2880"`. So `028` meets a second condition the fixture does not
reproduce. Reach if anyone takes it: **16 corpus DOCX, 41 page-relative vertical offsets**
(`probes/ofpie-r190/census-pagev.py`). **Do not implement "page means margin" from the corpus
document alone** — the fixture refutes it.

*That second condition is the object kind, and the 41-offset figure is the wrong denominator for it:
the rule needs an **embedded** object, of which the corpus holds 5. And one row of that round's variant
table does not reconcile — it gives `layoutInCell="0"` a frame top of 214.94 and `w:top="0"` one of
175.86 while annotating both "i.e. ours", where 247.86 − 175.86 is exactly the 72.00 the mechanism
predicts and 247.86 − 214.94 is 32.92. The `w:top` linearity is sound and the 214.94 is not settled;
re-measure it from the PDFs before using the `incell0` variant as a control.*

### A chart sweep's worst page is not the chart's, and "the line break charges its trailing blank" is refuted

**Two findings from one page, and the first is about instruments.** The chart-type sweep ranked
`Intersil_Italy_CAN_Bus_Transceiver_Presentation_Final.pptx` at 18.08 `diff%` and a task carried
that as a `barChart` seat. **The deck's one chart is on slide 30, which scores 5.38**; the 18.08
is slide **37**, a bulleted text slide, and 15.91 is slide 13. *A sweep that ranks a document on
its worst page attributes that page to whatever the document was selected for* — census which
page the feature is on before working the number.

**The second is a negative result, and it was implemented and measured before it was refuted, so
do not re-derive it.** Slide 37's body placeholder is
`<a:off x="609599"/><a:ext cx="11012681"/>`, right edge **915.14 pt**. Our line
`One transceiver will be active while the other transceiver` ends at 906.82 and we keep it; the
reference breaks before the last word. It is not metrics at the scale of a word — five identical
lines measure the same on the two sides to **0.6 pt**. Rewriting `cx` alone
(`probes/slidewrap-r190/widths.py`) puts the boundary between a right edge of **915.0**, where the
reference still breaks, and **915.6**, where it keeps the word; the kept line's own extent with its
trailing blank is **915.70** and without it 906.82. **Read alone that says the blank is charged.
It is wrong, and the reason is that the placeholder is not the text area.**

***The master states a 7.2 pt right inset and both renderers honour it.***
`<a:bodyPr vert="horz" lIns="0" tIns="45720" rIns="91440" bIns="45720" rtlCol="0">` on the
master's body placeholder; the slide's own and the layout's are bare `<a:bodyPr/>`. So the
available width is **859.94**, our visible candidate is 858.83, it fits by **1.11 pt**, and the
reference breaks it anyway. Sweeping that one attribute (`insets.py`) brackets how much more than
its visible text a line must be given: the reference keeps the word at `rIns` 6.3 pt and breaks it
at 7.2, so the residue is **1.11 to 2.01 pt — about 0.2% of the line**, against a trailing blank
of **8.26 pt**. Four times too small.

***And charging the blank regresses the slides track, which is the measurement that settles it.***
`LineFiller.chargesTrailingBlanks` was written, `SlideTextLayout` set it, and on slide 37 it is
exact — all thirteen of our baselines become the reference's to the hundredth. Over the track:
**210 of 302 documents move**, and against a freshly rendered 26.2.4.2 the summed worst-page
`diff%` goes **1750.8 → 1950.8**, every page's summed `diff%` **15405.8 → 16839.0**, MAJOR pages
**280 → 401**, and **16 documents improve against 109 worse**. Both formats regress and the worst
movers are large (4.79 → 16.01, 8.45 → 18.68, 9.63 → 19.30) — an extra line tipping a body into
autofit where the reference does not shrink. **Reverted; nothing in `dotnet/src` carries it.** The
residue may not be a rule at all: 0.2% on one line is the same order as the ±0.6 pt spread the
five agreeing lines already show, and "the reference measured this line a little wider" would
explain the track result exactly.

***The leading difference on that page is a consequence, not a second defect.*** The extra line
tips the body past its autofit box, so the reference takes `constScaleLevels[0]` —
`{1.000, 1.000, 1.0, 0.9}`, font unscaled and **spacing 0.9**, applied as
`nHeight = round(GetHeight() * fSpacingY)` in the `InterLineSpaceRule::Off` arm
(`editeng/source/editeng/impedit3.cxx`:1584-1600) — and draws a 28.15 pt pitch where we draw
31.27 at the same 25.99 pt size, `31.27 × 0.9 = 28.14`. Widen the placeholder by 1 pt and the
reference draws our two lines at our exact baselines, 241.97 and 274.68. **A page that reads as
wholly displaced can be one word's worth of wrap plus an autofit that followed it; measure the
line count before the pitch.**

### Stored evidence decays silently, and the prose knows it while the data does not

Three cases surfaced in a single day: a `words-after.tsv` carrying numbers from a sweep that
had overlapped a rebuild; a "39/39 exact" CJK fit measured on a face whose line gap is zero,
so it could not have discriminated between the hypotheses it was cited for; and a
`printer-metric-advance.py` whose "exact on all 96" is 16 of 96 against this container's
binary. None of the three announced itself. Each stayed quotable.

Censused over all 410 stored figures under `probes/` and `research/probes/`, the pattern is
sharp:

| | records the environment it was measured in |
|---|---|
| prose write-ups | **122 of 154** |
| stored TSVs | **3 of 256** |

**The write-up says what it was measured against; the data does not — and the data is what a
later round greps, pastes into a brief, and acts on.** A TSV is a grid of numbers with no way
to tell its reader that the reference bank behind it no longer exists. 215 of the 410 predate
the 2026-08-13 container move, and **35 of those are still cited by live guidance**.

`probes/PROVENANCE.tsv` is the index — path, date added, era, which LibreOffice, whether
DejaVu was present, and which live documents cite it. Regenerate it after adding probe output:

```sh
python3 dotnet/probes/provenance-index.py          # rewrite the index
python3 dotnet/probes/provenance-index.py --check  # exit 1 if stale
```

**Do not regenerate it in this container — it destroys the era column, and the reason is
sharper than either guess.** `provenance-index.py` takes each file's `added` date from
`git log --diff-filter=A`. Not from filesystem mtimes, and not defeated by missing history:
this clone reaches back to 2026-07-29, which is *before* the boundary. What defeats it is that
the probe tree entered this clone in **one bulk commit dated 2026-08-21** — `3418` of the
A-records under `dotnet/probes` carry that single date, which is *after* the 2026-08-13
boundary, so every row classifies `current` and the committed index's 215 `pre-container`
rows collapse to zero.

The era column is the whole point of the file: it is what tells a later round that a stored
figure was measured against a reference bank that no longer exists. An index stale by a few
probe directories is much the lesser harm, so **add rows by hand**.

Three rounds have now run it, watched it rewrite hundreds of rows and reverted, each reporting
a different mechanism (`git log` cannot see the dates; the column comes from mtimes). Both are
wrong and the fix is the same either way: don't run it. Worth knowing if anyone rewrites the
script — `added_dates` also uses `setdefault` over `git log`'s newest-first output, so a file
added, deleted and re-added keeps its *newest* A-record, not the first one its comment claims.

It deliberately does **not** stamp the probe files themselves. They are the record of what a
round actually ran; rewriting them would falsify it, and a `#` header would break every
consumer that reads line 1 as the column names. A sidecar records provenance without touching
the record.

The rule that follows, and it is the one to carry: **a stored figure is evidence about an
environment, not about the code.** Before quoting one, check which era produced it. What
survives a reference change is the *mechanism* a round identified; what does not survive is
every number attached to it.

**The package feed was firewalled and is now open; the build works.** `dotnet restore`
succeeds for all 26 projects and `dotnet build -v q -nologo` gives **0 warnings, 0 errors**.
Recorded because the diagnosis cost a round and the shape of it recurs:

- The host that must be allowed is **`api.nuget.org`**, named literally. The policy matches
  hosts **exactly**, so an apex allow does not cover subdomains — and `nuget.org` was already
  allowed here throughout (it answers 301 from IIS) while `api.nuget.org` answered the proxy's
  403. A request phrased as "allow nuget.org" therefore changes nothing and looks like the
  allow having failed. A wildcard (`*.nuget.org`) does cover it.
- `api.nuget.org` alone is sufficient: it serves the service index, `RegistrationsBaseUrl`,
  the `PackageBaseAddress` flat-container download endpoint, and the `VulnerabilityInfo` feed.
  `www` / `globalcdn` / `azuresearch-*` are the gallery UI, the legacy V2 redirect target and
  `SearchQueryService` — a V3 restore under central package management touches none of them.
- There is **no offline route**, established rather than assumed: the SDK's five bundled packs
  are all first-party, there is no fallback folder or cache anywhere on the filesystem, and
  every upstream GitHub release for these packages ships **zero** attached assets. Even a
  dependency-free project cannot restore offline, because the bundled apphost is
  `ubuntu.26.04-x64` while `Directory.Build.props` correctly computes the portable
  `linux-x64` — so `Microsoft.NETCore.App.Host.linux-x64` is always one download.
- `NuGetAudit` is on by default and `TreatWarningsAsErrors` promotes an unreachable
  vulnerability feed to a hard error (NU1900). Any scheme that leaves that feed unreachable
  also needs `<NuGetAudit>false</NuGetAudit>`.
- `HOME=/tmp` here, so the package cache lands in `/tmp/.nuget/packages` on the 20 GB overlay,
  not on the large host mount. It counts against the disk budget.

`github.com` and `archive.ubuntu.com` are reachable; the LibreOffice download hosts are not,
which is why the reference binary cannot be pinned back to 24.2.7.2.

### Installing a specific LibreOffice, and why the tarball is not the distro package

**One host has to be allowed and it must be a wildcard: `*.documentfoundation.org`.** The apex
alone is not enough — `download.documentfoundation.org` is a redirector, and the file server that
actually carries every release including superseded ones is
`downloadarchive.documentfoundation.org`. `www.libreoffice.org`,
`ppa.launchpadcontent.net`, `api.launchpad.net`, `api.snapcraft.io`, `dl.flathub.org`,
`flathub.org`, `dev-www.libreoffice.org` and `git.libreoffice.org` are each separately denied;
`launchpad.net` and `keyserver.ubuntu.com` happen to be allowed and are not sufficient on their
own. The archive route needs none of them.

```sh
V=26.2.4.2
curl -sSL -o /tmp/lo.tar.gz \
  "https://downloadarchive.documentfoundation.org/libreoffice/old/$V/deb/x86_64/LibreOffice_${V}_Linux_x86-64_deb.tar.gz"
mkdir -p /tmp/lo && tar xzf /tmp/lo.tar.gz -C /tmp/lo
dpkg -i /tmp/lo/LibreOffice_${V}_Linux_x86-64_deb/DEBS/*.deb     # ~220 MB, 42 packages
/opt/libreoffice26.2/program/soffice --version                    # does NOT take over `soffice`
```

It installs beside the distro's under `/opt/libreoffice<major>.<minor>` and leaves
`/usr/bin/soffice` alone, so switching is a symlink and reverting is the same symlink back.
**Record the old target before repointing** — `readlink -f /usr/bin/soffice` — because every
stored figure in this repository was measured against whichever one was live.

**And now the part that matters: a TDF tarball 26.2.4.2 is NOT the distro-packaged 26.2.4.2 this
tree is calibrated against.** Measured 2026-09-03, `XRay.Office.Fidelity.Tests`, same commit, same
corpus, only the reference binary swapped:

| reference | failed of 552 |
|---|---:|
| distro 24.2.7.2 (this container's own) | **18** |
| TDF tarball 26.2.4.2, as shipped | **36** |
| TDF tarball 26.2.4.2, bundled font duplicates removed | **31** |

The version move fixes six test classes outright — `TableComparisonTests`,
`SlideTableComparisonTests` (both), `PdfOutputComparisonTests`,
`FootnoteComparisonTests.TheRuleAboveTheNotes…` and
`SheetSpilledTextComparisonTests.EveryPageShowsAsManyWords…` — so those really were the version
gap and nothing else. But it *breaks* thirteen more, and they cluster: `TabStopComparisonTests`,
`LineHeightComparisonTests`, `JustificationShrinkComparisonTests`, `MixedRunComparisonTests`,
`TableAutoLayoutComparisonTests`, `SheetTextComparisonTests`. Every one of those is a text-metric
comparison.

**Half the cause is that the tarball bundles its own fonts, including the metric-compatible
families.** `/opt/libreoffice26.2/share/fonts/truetype` ships 136 faces, among them Carlito,
Caladea, Liberation and DejaVu — *different builds* from the system's: Caladea-Regular is 58 964
bytes bundled against 81 600 installed, Carlito-Regular 635 996 against 628 032, and all differ by
md5. LibreOffice reads its own; X-Ray.Office reads the system's through its own OpenType reader; so
the two stacks measure different files and every advance width diverges. Moving the 45 duplicates
aside takes 36 failures to 31, which confirms the mechanism and also shows it is **not the whole
story** — the remaining gap is the tarball's other bundled libraries (its own HarfBuzz, ICU and
FreeType) against the distro's.

So: **do not treat "install 26.2.4.2 from TDF" as reproducing the environment the stored figures
came from.** It is a fourth reference, not the third one. If a round needs the tree's real target,
it needs the distro package on the distro the project develops on, which is Ubuntu 26.04 — and
this container is 24.04, whose archives stop at 24.2 with a 25.8.7 backport. `fc-match` will not
warn you about any of this: it answers for the system font set and knows nothing about what a
bundled application resolves. Read the face out of a PDF the binary itself produced.

**`git status` shows 56 files modified that are not modified.** This mount reports a
symlink's size as 0, so git reads every symlink in the tree as having been emptied — the
`sysui` and `android` icon PNGs, `.vsconfig`, 56 in all. They are all mode `120000` in HEAD
and all still correct on disk; `readlink` returns the right target for every one.

The consequence is the dangerous part. **`git add -A` or `git add .` in this container
replaces 56 symlinks with empty files** and commits that as real work — a corruption of the
LibreOffice tree that no test would catch, because nothing under `dotnet/` reads them. Stage
explicit paths, always. `git status --short | grep -v '\.png$'` is not sufficient as a filter
either: `.vsconfig` is in the list and is a symlink too. The reliable test is the mode:

```sh
git ls-files -s <paths-you-are-about-to-stage> | awk '$1=="120000"'   # must print nothing
```

The second consequence is milder and shows up at the end of a round rather than the start:
**`git worktree remove` refuses**, with `contains modified or untracked files`, because it sees
those same 56 phantom modifications. Check that the branch is genuinely merged and that nothing
else is dirty, and then `--force` is correct rather than a shortcut:

```sh
git -C <primary> merge-base --is-ancestor <worktree-head> HEAD   # must succeed
git -C <worktree> status --short | grep -vE '\.(png|ico)$' | grep -v '\.vsconfig'   # must be empty
git -C <primary> worktree remove --force <worktree>
```

### Three worktree branches hold commits that must NOT be merged

**They are not in this clone, and the four assertions this section sends a round to fetch have
been re-derived instead.** Checked 2026-09-06 from `/home/user/wt-slidechart`: `git branch -a`
lists 27 refs — 23 `agent/*`, `master`, and one `claude/*` local and remote — and none of
`wt-paint-b`, `wt-slides-chart`, `wt-slides-text`. `git grep CategoriesReversed` over every branch
head finds one hit and it is this file; no branch has ever held `ChartStackingTests.cs` or
`DrawingChartStackingTests.cs`. So `git show wt-slides-chart:<path>` cannot be run here, and a
round briefed to read those tests should say so rather than hunt.

Three of the four assertions — a reversed *category* axis putting the first category at the top,
moving its labels with the bars, and swapping the series within a category — are now
`ChartReversedCategoryAxisTests`, derived from 26.2.4.2 by patching one attribute of a corpus
chart rather than from the branch, with the C++ rule at
`chart2/source/view/axes/VAxisProperties.cxx`:232-234. `probes/chart-cat-reverse/results.md` has
the measurements. **The fourth is still open**: a series with `a:noFill` holding its place in a
stack. Keep the paragraphs below for the general point about diff direction, which is the reason
they were written.

Triaged 2026-08-15. `wt-paint-b` (2 commits), `wt-slides-chart` (4) and `wt-slides-text` (5) each
carry work that never reached this branch, and merging any of them **reverts newer work**. They
are survivors of the round that crashed, and the fixes in them were subsequently re-derived and
landed by another route — better, in at least the autofit case.

The tell is in the diff direction. Against this branch they show large *deletions*:
`ChartLayout.cs` −251, `SlideAutofit.cs` −213, `SlideText.cs` −207, `PptxTextBody.cs` −155. That
is not work to recover, it is an older file. Confirmed by content rather than by inference —
`percentStacked` is already in `Charts/ChartPlot.cs` and `DrawingML/DrawingChartPlot.cs`, the
twelve `constScaleLevels` autofit rows are already in `SlideAutofit.cs:32-116` with the 0.250
floor, and `a:noFill` suppression is already at `DrawingChartPlot.cs:405,1583`.

**Keep the branches; do not merge them, and do not delete them without reading this.** The one
thing they hold that this branch does not is *test coverage*: `wt-slides-chart` has
`ChartStackingTests.cs` (288 lines) and `DrawingChartStackingTests.cs` (252). They do not compile
here — they are written against a `ChartPlot.CategoryTotal` / `ChartPlot.CategoriesReversed` API
this branch never adopted. Most of what they assert is covered under other names
(`APercentStackIsDrawnZeroToOneHundredInTenSteps`,
`EveryPercentStackedColumnIsTheSameHeightAndSplitByRatio`,
`AReversedAxisRunsFromTheMaximumDownwards`), but four assertions appear to have no counterpart:
a reversed *category* axis putting the first category at the top, moving its labels with the
bars, and swapping series within a category; and a series with `a:noFill` still holding its place
in a stack. Adapting those four is worth a round; merging the branch to get them is not.

The general point, which is the reason this is written down at all: **a branch that is behind is
indistinguishable from a branch that is ahead until you look at which side the deletions are
on.** `git log --oneline main..branch` shows commits either way and says nothing about it.

**`git stash` is repository-global, and this clone has many worktrees.** Stashing a file in
one worktree to build a "before" binary, and popping it later, popped *another branch's* stash
into the wrong worktree — the stash stack is one per repository, not one per worktree. Nothing
was lost that time (both entries were recovered with `git stash store` and the sweeps either
side re-checked), but the failure is silent and lands in a tree an agent is mid-measurement in.
**Copy the file aside instead.** `cp file file.before` costs nothing and cannot reach another
branch.

**And restore it with `cp`, never with `mv` — this is where that advice has actually failed.**
`mv file.before file` keeps the *original* modification time, so the restored source looks older
than the compiled assembly and **MSBuild's up-to-date check skips the project**. The build then
reports `0 Warning(s), 0 Error(s)` in fourteen seconds and the binary still carries the
experiment. Measured on 2026-08-15: a one-twip throwaway patch to `LineSpacing.cs` survived
*three* subsequent builds whose whole purpose was to be free of it, and silently contaminated a
`words/done-*` sweep, a 200-document reach measurement and two `--page` comparisons before a
contradiction — a line height one twip *above* a value the source cannot produce — gave it away.

There is no output that distinguishes "nothing needed rebuilding" from "the thing you just changed
was skipped", so the habit has to be unconditional:

```sh
cp file.before file && touch file      # or: git checkout -- file && touch file
```

`rm -rf src/<project>/{obj,bin}` before the rebuild is the certain version and costs one project's
compile. Worth it whenever a measurement is about to be trusted, and the check that catches it
afterwards is cheap: render one document and compare it byte for byte against the run you are
claiming to have reproduced.

**The reference half of the gate can be banked without a build.** `batch-check.sh` refuses to
start without a CLI, which is right for a round and wrong when the reference binary is what
changed. `ref-baseline.sh` is the reference-only half, with `batch-check.sh`'s conventions
column for column, so the two are comparable:

```sh
.claude/skills/corpus-batches/scripts/ref-baseline.sh \
  /c/sandbox/workdir/sample-files 'words/batch-0*' /abs/out 6
```

It is resumable, records the binary version in its header, and was validated against an
independent known answer before use — reference page counts against `ppt/slides/slideN.xml`
counts taken from the zip, 4 of 4 exact.

### A corpus document whose page number never changes is O88, and the ODF copy agrees for the wrong reason

26.2.4.2 does not import a field that sits in a text box inside a `wpg:wgp` group anchored in a
table cell: it keeps the field's cached result as ordinary text. On the two words-track documents
that carry that structure the footer prints one constant page number on every page —
`Page 6 of 7` on all seven of `A1. EASA Form 2.docx`, `Page 3 of 6` on all six of
`B11. TE.CAO.00129  Experience  logbook.docx` — and **ours counting 1..N is the right answer**.
Established both ways on mutated corpus documents and on a 2×2 synthetic factorial, and present
in 24.2.7.2 as well, so it is not a version artefact. `probes/pagefield-r137`, seat **O88**.

Two traps follow. **The gate cannot see it**: a frozen page number has the same glyph count as a
counted one, so both documents are `match` rows and always will be. And **`/home/user/corpus-odf`
carries the defect forward** — those `.odt` were written by 26.2.4.2's own exporter from the
already-broken import, so they hold the frozen string and no field, and an ODF-track comparison on
them agrees with us because both sides are reading the same wrong text. Agreement on a converted
file is not evidence about the construct that the conversion destroyed.

## Research notes

Written from a deep read of the C++ implementation. Consult the relevant one *before*
implementing an area — they contain exact record layouts, algorithms and file:line
citations, and will save far more time than they cost to read.

| File | Covers |
|---|---|
| `research/01-formats-and-detection.md` | The filter/type registry; the detection algorithm with concrete signatures |
| `research/02-writer.md` | Writer's document model, layout engine, and the DOCX/DOC/RTF/ODT importers |
| `research/03-calc.md` | Calc's cell storage, formula engine, importers, and print pagination |
| `research/04-impress.md` | The shape model, custom-shape geometry, PPTX/PPT/ODP importers, slide rendering |
| `research/05-infrastructure.md` | OLE2/CFB byte layouts, ZIP/OPC/ODF packaging, encryption, EditEngine, item sets, encodings |
| `research/06-rendering.md` | VCL output, fonts and metrics, drawinglayer primitives, PDF export, headless entry points |

## Conventions

- British spelling in identifiers and prose (`Colour`, `normalise`) — consistent with the
  existing code.
- XML doc comments on public API. Say *why*, not just what; the what is usually evident
  from the signature.
- Avoid the name `Path` for new types: it collides with `System.IO.Path` under implicit
  usings. The geometry type is `GraphicsPath`.
- Prefer `readonly record struct` for small value types, `sealed record` for immutable
  reference types.
- `Span`/`ReadOnlySpan` for binary parsing hot paths. `AllowUnsafeBlocks` is on.
