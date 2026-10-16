using XRay.Office.Core.Units;

namespace XRay.Office.Text.Fonts;

/// <summary>Which of a font's competing metric sets a line height was derived from.</summary>
/// <remarks>
/// Reported rather than hidden. A line-height difference is one of the most visible ways two
/// renderers diverge on the same input, and knowing which set was believed turns an inexplicable
/// half-page offset into a one-line answer.
/// </remarks>
public enum LineMetricSource
{
    /// <summary>The <c>hhea</c> table, which is what a line is measured from unless something wins.</summary>
    HorizontalHeader,

    /// <summary>
    /// <c>OS/2</c>'s <c>usWinAscent</c> and <c>usWinDescent</c> — the historical Windows metrics,
    /// used when <c>hhea</c> states nothing usable.
    /// </summary>
    WindowsMetrics,

    /// <summary>
    /// <c>OS/2</c>'s typographic metrics, used when the font's <c>fsSelection</c> asks for them.
    /// </summary>
    TypographicMetrics,

    /// <summary>Nothing usable; the em square was assumed.</summary>
    Fallback,
}

/// <summary>
/// The logical unit a reference device's map mode is set to.
/// </summary>
/// <remarks>
/// <b>A resolution alone does not describe a device.</b> The quantisation a grid applies is the
/// size of one device pixel expressed in the unit the application measures in, and that is the
/// resolution <em>and</em> the map unit together: 8640 dpi in twips is six pixels to the unit,
/// 8640 dpi in hundredths of a millimetre is 3.4, and 600 dpi in hundredths of a millimetre is
/// 0.236 — a pixel worth 4.23 units, eighteen times the coarsest thing Writer ever sees.
/// </remarks>
public enum MetricUnit
{
    /// <summary>A twentieth of a point, 1440 to the inch. Writer's map mode.</summary>
    Twip,

    /// <summary>A hundredth of a millimetre, 2540 to the inch. The draw layer's, Calc's and Impress's.</summary>
    Mm100,
}

/// <summary>
/// The device a font's metrics are quantised through before layout sees them.
/// </summary>
/// <remarks>
/// <para>
/// <b>There is always a device.</b> Writer never scales a face's design units straight to the size the
/// document asks for — it formats against a reference device, and every vertical metric is quantised
/// onto that device's pixel grid on the way in and back onto the document's own unit on the way out.
/// Which device it is depends on one compatibility flag:
/// </para>
/// <list type="bullet">
/// <item><description>
/// Normally a <c>VirtualDevice</c> in <c>VirtualDevice::RefDevMode::MSO1</c> with
/// <c>MapUnit::MapTwip</c> — <c>DocumentDeviceManager::CreateVirtualDevice_</c>,
/// <c>sw/source/core/doc/DocumentDeviceManager.cxx</c>:259. <c>MSO1</c> is <b><c>6*1440</c> = 8640
/// dpi</b> (<c>vcl/source/gdi/virdev.cxx</c>:407), so one twip is exactly six device pixels. See
/// <see cref="Reference"/>.
/// </description></item>
/// <item><description>
/// A real printer when the document asks for one: Word's "use printer metrics to lay out document"
/// makes <c>WW8Dop::fUsePrinterMetrics</c> into <c>!USE_VIRTUAL_DEVICE</c>
/// (<c>sw/source/filter/ww8/ww8par.cxx</c>:2008) and <c>getReferenceDevice</c> hands out an
/// <c>SfxPrinter</c>. See <see cref="Printer"/>.
/// </description></item>
/// </list>
/// <para>
/// The difference between the two is only the coarseness, and on the printer it is not small: at
/// 600 dpi a pixel is 2.4 twips, so Liberation Serif at 9.5 pt measures 10.80 pt per line rather
/// than the 10.95 pt its design units give, and Liberation Sans at 11 pt measures 12.60 against
/// 12.65. On the virtual device a pixel is a sixth of a twip and the effect is worth exactly one
/// twip, in either direction, on about one line height in nine. That one twip is what
/// <c>dotnet/probes/lineheight-01/</c> is about: it moved 22 of 195 measured (face, size) pairs and
/// two earlier rounds could not reconstruct it because they swept 72–6000 dpi and the answer is
/// 8640.
/// </para>
/// <para>
/// <b>The printer's resolution has to be measured, not read.</b> This said 300 dpi and cited
/// <c>PPDParser</c> defaulting both axes to 300 when the queue names no resolution and when there is
/// no PPD at all (<c>vcl/unx/generic/printer/ppdparser.cxx</c>:1500 and :1524). That reading is not
/// what the installed 26.2.4.2 does here: an authored pair differing only in
/// <c>fUsePrinterMetrics</c> gives eight (face, size) line heights that 600 dpi reproduces on 8 of 8
/// and 300 dpi on 3, and two corpus documents' banked references agree. The resolution is the whole
/// of what the device contributes, nothing in the file decides it, and a machine whose default queue
/// says otherwise would need a different number again — which is the honest cost of a document
/// asking to be laid out against hardware. See <see cref="Printer"/> for the table.
/// </para>
/// <para>
/// <b>Writer's logical unit is the twip and the other two applications' is the hundredth of a
/// millimetre</b>, which is why this carries a <see cref="MetricUnit"/> as well as a resolution.
/// See <see cref="Presentation"/> and <see cref="Spreadsheet"/>; both were measured on the
/// installed binary rather than read out of the C++, and for Calc the two answers differ.
/// </para>
/// </remarks>
/// <param name="Dpi">The device resolution the metrics are rounded onto.</param>
/// <param name="Unit">The logical unit the device's map mode is set to.</param>
/// <param name="QuantisesAdvances">
/// Whether horizontal advances go through the grid as well as the vertical metrics.
/// <para>
/// True only for a real printer, and measured rather than assumed: <c>probes/printer-metric-advance.py</c>
/// sweeps 96 authored rows with <c>fUsePrinterMetrics</c> varied on one body. Re-run on 26.2.4.2 at the
/// 600 dpi the same probe establishes, the quantised rule reproduces 37 of the 96 exactly and the rest
/// within two twips, where at 300 dpi it was out by as much as 137; with the flag clear, unquantised
/// scaling is the right answer and this rule is not. The dominant term is the em rounding, which a
/// 600 dpi device makes worth up to 0.4% of every advance and an 8640 dpi device makes exactly
/// nothing — so on the virtual reference device the two rules differ by less than a twip and the
/// evidence says to take the one that was measured.
/// </para>
/// <para>
/// The two-twip residue is real and open: it is pinned by
/// <c>MetricGridTests.TheAdvanceRuleIsNotExactAndTheResidueIsRecordedRatherThanHidden</c>. Dropping
/// the truncation fits 52 of 96 rather than 37 but is <em>worse</em> on 17 of them, so the evidence
/// does not choose between the two and the floor stays because it is what the C++ says.
/// </para>
/// </param>
/// <param name="ScalesEastAsianFaces">
/// Whether Word's <c>MS_WORD_COMP_GRID_METRICS</c> compatibility rule applies: a face declaring one of
/// four East Asian code pages has its ascent, and its ascent and descent together, scaled by 127%
/// before the leading is added. See <see cref="AsWordDocument"/> and <see cref="EastAsianScaled"/>.
/// <para>
/// <b>Only ever true on a Writer grid</b>, and the two rules on this type are orthogonal because of
/// it: this one is <c>SwFntObj</c>'s, and the taller-of-two-roundings in
/// <see cref="LineMetrics.ScaledLineHeight"/> is EditEngine's. See <see cref="EastAsianScaled"/>.
/// </para>
/// </param>
/// <param name="PageScale">
/// The page transform the device's map mode already carries, as a plain multiplier; 1.0 for a page
/// drawn at its own size.
/// <para>
/// <b>A scale in the map mode moves the quantisation to the far side of the multiply, and that is
/// the whole of it.</b> A sheet printed at a zoom, a slide scaled onto a page and a shrunk cell are
/// all drawn through a scaled map mode, so <c>OutputDevice::ImplLogicHeightToDevicePixel</c> takes
/// the scale <em>into</em> the device pixel count and <c>DevicePixelToLogicHeight</c> takes it back
/// <em>out</em> again — and the <c>llround</c> at the end of that second conversion
/// (<c>vcl/source/outdev/CoordinateMapper.cxx</c>:279) therefore lands on a whole logical unit of
/// the <em>unscaled</em> page, which is then multiplied by the scale on its way to the paper. A
/// renderer that scales first and quantises afterwards lands on a whole unit of the scaled page
/// instead, which is a different number.
/// </para>
/// <para>
/// Only <see cref="ToLength"/> reads it, because that is the only conversion that crosses the map
/// mode in the outward direction. <see cref="ToPixelEm"/> needs no correction: the em it is handed
/// is already the drawn size, so the scale is inside it — measured on the witness below, where the
/// reference's own device pixel count and this tree's agree at 60 either way.
/// </para>
/// <para>
/// <b>Measured.</b> <c>RMP 2011-2014 and Inventory.xls</c> prints at Calc's own 60% and states a
/// 10 pt Arial, so the device sets 60 pixels and <c>ImplInitTextLineSize</c>'s descent branch
/// answers 3 pixels of thickness at 7 of depth. Quantising after the scale gives
/// <c>round(3 × 2540/720) = 11</c> hundredths of a millimetre — <b>0.3118 pt</b>, which is what
/// this tree drew — and before it gives <c>round(3 × 2540/720 / 0.6) × 0.6 = 18 × 0.6 = 10.8</c>,
/// which is <b>0.30614 pt</b> and is written <c>0.306</c>. 26.2.4.2 draws 0.306. The depth is the
/// same arithmetic on 7 pixels: 25 hundredths against <c>41 × 0.6 = 24.6</c>, 0.7086 pt against
/// 0.69732, and the reference draws 0.697. <c>probes/rulescale-r132/</c>.
/// </para>
/// </param>
public readonly record struct MetricGrid(
    int Dpi, bool QuantisesAdvances, MetricUnit Unit, bool ScalesEastAsianFaces = false,
    double PageScale = 1.0)
{
    /// <summary>A grid at a resolution in twips, quantising advances as a real device does.</summary>
    public MetricGrid(int dpi) : this(dpi, true, MetricUnit.Twip) { }

    /// <summary>A grid at a resolution in twips.</summary>
    /// <param name="dpi">The device resolution.</param>
    /// <param name="quantisesAdvances">Whether horizontal advances go through it too.</param>
    public MetricGrid(int dpi, bool quantisesAdvances)
        : this(dpi, quantisesAdvances, MetricUnit.Twip) { }

    /// <summary>The grid a document asking for printer metrics is laid out on.</summary>
    /// <remarks>
    /// <para>
    /// <b>600 dpi, and it was 300 until it was measured against this container's binary rather
    /// than inherited.</b> `probes/printer-metric-advance.py` varies <c>fUsePrinterMetrics</c> on
    /// one authored body and reads the baseline pitch off both renderings; run against the
    /// installed 26.2.4.2 it gives eight (face, size) pairs that 600 dpi reproduces on **8 of 8**
    /// and 300 dpi on 3:
    /// </para>
    /// <code>
    ///   face              pt   printer   600 dpi   300 dpi
    ///   Liberation Serif   9    10.300    10.30     10.60
    ///   Liberation Serif  10    11.550    11.55     11.55
    ///   Liberation Serif  11    12.750    12.75     12.75
    ///   Liberation Serif  12    13.800    13.80     13.95
    ///   Liberation Sans    9    10.350    10.35     10.35
    ///   Liberation Sans   10    11.500    11.50     11.55
    ///   Liberation Sans   11    12.600    12.60     13.00
    ///   Liberation Sans   12    13.800    13.80     13.95
    /// </code>
    /// <para>
    /// The 300 dpi figures are exactly what that probe's own write-up records as measured, so this
    /// is a stored figure that no longer reproduces rather than a rule anyone read wrong — the
    /// headless default printer is what decides it and it is not a property of the file. The same
    /// re-run moves the probe's advance verdict the same way: <c>exact-em@600</c> is the best
    /// rule at 0.022 pt mean error where every 300 dpi rule is above 0.68.
    /// </para>
    /// <para>
    /// The correction is worth 2.8% of a line at 11 pt in Liberation Sans — 13.00 pt against 12.60 —
    /// which is
    /// what put 23 spurious pages into <c>A_320.doc</c>: its MMEL tables are padded to exactly one
    /// page each, so a table 16.8 pt too tall no longer fits under the empty page-break paragraph
    /// in front of it and every one of them was pushed onto a page of its own.
    /// </para>
    /// </remarks>
    public static MetricGrid Printer { get; } = new(600, quantisesAdvances: true);

    /// <summary>
    /// The virtual reference device Writer formats every other document against: 8640 dpi, six
    /// device pixels to the twip.
    /// </summary>
    public static MetricGrid Reference { get; } = new(6 * 1440, quantisesAdvances: false);

    /// <summary>
    /// The same grid with Word's East Asian line scale switched on.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>MS_WORD_COMP_GRID_METRICS</c> is a document compatibility setting rather than a property of
    /// the device, and it is off by default — <c>DocumentSettingManager</c> initialises
    /// <c>mbMsWordCompGridMetrics(false)</c>, and an ODF file carries its own value. So the DOC and
    /// DOCX readers ask for this and the ODF one does not, which is measured rather than reasoned:
    /// the same two lines of WenQuanYi Zen Hei at 12 pt are 406 twips apart when LibreOffice reads
    /// them from a <c>.docx</c> and 325 apart when it reads them from a <c>.fodt</c>.
    /// </para>
    /// <para>
    /// Applied to whichever grid the document already asked for, because the flag and the device are
    /// independent in the C++ too: every call site of <c>lcl_ApplyCjkHeightAdjustment</c> passes the
    /// reference device it happens to have and asks the document for the flag separately. The
    /// printer combination is unmeasured — no corpus document sets <c>usePrinterMetrics</c> and
    /// names an East Asian face — and is written this way because that is what the C++ does, not
    /// because it has been seen.
    /// </para>
    /// </remarks>
    public MetricGrid AsWordDocument() => this with { ScalesEastAsianFaces = true };

    /// <summary>
    /// The 127% East Asian line scale, or the value unchanged.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>lcl_ApplyCjkHeightAdjustment</c> (<c>sw/source/core/txtnode/fntcache.cxx</c>:270-292,
    /// tdf#129808): with <c>MS_WORD_COMP_GRID_METRICS</c> set and the face declaring CP932, CP936,
    /// CP949 or CP950, <c>(nBase * 127) / 100</c> — integer division, on the value in twips.
    /// </para>
    /// <para>
    /// <b>What it multiplies is not the finished line height.</b> <c>GetFontHeight</c> reads
    /// <c>lcl_ApplyCjkHeightAdjustment(m_nPrtHeight, …) + GetFontLeading(…)</c>, so the scale
    /// reaches the device's ascent-plus-descent and the face's leading is added afterwards,
    /// unscaled; <c>GetFontAscent</c> is the same shape. That distinction is invisible on IPAGothic,
    /// whose <c>hhea</c> line gap is zero — which is why <c>probes/lineheight-01</c> §7(a) recorded
    /// the rule as scaling the height itself and was exact on all 39 of its pairs anyway. WenQuanYi
    /// Zen Hei has a gap of 92/1024 and separates them: at 12 pt the two rules give 412 twips and
    /// 406, and LibreOffice draws 406. See <c>probes/words-metrics-01/probe-cjk127.py</c>, which
    /// scores both against 117 measured pairs over three faces — this rule 117, the other 78.
    /// </para>
    /// </remarks>
    /// <param name="value">The ascent, or the ascent and descent together, before the leading.</param>
    /// <param name="face">Whether the face declares one of the four East Asian code pages.</param>
    public Length EastAsianScaled(Length value, bool face)
        => ScalesEastAsianFaces && face
            ? FromLogical(ToLogical(value) * 127 / 100)
            : value;

    /// <summary>
    /// The reference device Impress and Draw format against: <b>600 dpi in 1/100 mm</b>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SdModule::SdModule</c> creates one <c>VirtualDevice</c> for the whole application, sets
    /// its map mode to <c>MapUnit::Map100thMM</c> and then
    /// <c>VirtualDevice::RefDevMode::Dpi600</c> — <c>sd/source/ui/app/sdmod.cxx</c>:83-85, whose
    /// comment says it is for "a visually better formatting of text in small sizes". A device
    /// pixel is therefore <b>4.233 hundredths of a millimetre</b>, 2.4 twips, where Writer's is a
    /// sixth of a twip: the same class of quantisation worth eighteen times as much.
    /// </para>
    /// <para>
    /// Measured, not read. <c>probes/refdev-01/probe-impress.py</c> puts six baselines on each of
    /// 507 (face, size) slides and reads them out of the reference PDF's own text matrices: this
    /// grid is exact on <b>507 of 507</b> ascents and 507 of 507 line heights, where 720 dpi
    /// manages 75 and 8640 dpi 89, and exact scaling — what the tree did before — 82.
    /// </para>
    /// </remarks>
    public static MetricGrid Presentation { get; } = new(600, false, MetricUnit.Mm100);

    /// <summary>
    /// The device a printed or exported Calc cell is formatted against: <b>720 dpi in 1/100 mm</b>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>This is not <c>ScDocument</c>'s own reference device and that is the whole point.</b>
    /// <c>ScDocument::GetVirtualDevice_100th_mm</c> really is <c>RefDevMode::MSO1</c>, 8640 dpi
    /// (<c>sc/source/core/data/documen8.cxx</c>:182-193), and it is not what draws a cell:
    /// <c>ScOutputData</c> formats against the <em>output</em> device, which on a PDF export is
    /// the PDF writer's own reference device, <c>RefDevMode::PDF1</c> = 720 dpi
    /// (<c>vcl/source/gdi/virdev.cxx</c>:410). Reading the tree gives 8640 and the binary draws
    /// 720; <c>probes/refdev-01/probe-calc.py</c> scores 92 of 273 for the first and
    /// <b>273 of 273</b> for the second, and 468 of 468 ascents across both the multi-paragraph
    /// <c>EditCell</c> path and the single-line <c>ScOutputData::LayoutStrings</c> one.
    /// </para>
    /// <para>
    /// 720 dpi in 1/100 mm is the same device <c>SheetDeviceUnits.ReferenceDpi</c> already
    /// carried for the font-size round trip. That it turned up independently from the line
    /// metrics is a check on both.
    /// </para>
    /// </remarks>
    public static MetricGrid Spreadsheet { get; } = new(720, false, MetricUnit.Mm100);

    /// <summary>
    /// The device <c>chart2</c>'s own view formats a chart's text against: <b>96 dpi in
    /// 1/100 mm</b>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A chart's labels, titles and axis text are not laid out by Calc, Writer or Impress at all:
    /// <c>chart2</c>'s view builds them as plain <c>SvxShapeText</c> objects on its own
    /// <c>VirtualDevice</c>, which is created with no <c>RefDevMode</c> at all and therefore keeps
    /// the platform default — 96 dpi — where Impress asks for 600 and the PDF writer for 720. A
    /// device pixel is <b>0.75 pt</b>, twenty-six hundredths of a millimetre, and it is coarse
    /// enough to be the dominant term in a ten-point line: 96 dpi rounds a 10 pt em down to 13
    /// pixels, a 2.5% shrink before a single metric is read.
    /// </para>
    /// <para>
    /// <b>Measured off 26.2.4.2, not read.</b> <c>probes/sheets-r60/probe-chartvmetrics2.py</c>
    /// renders 117 one-variable rewrites of <c>003_advanced_excel_pie.xlsx</c>'s own chart part —
    /// thirteen sizes from 6 to 40 pt on Carlito, Liberation Sans and Liberation Serif, each as a
    /// one-line and a three-line chart title — and reads the baseline pitch back out of the
    /// reference's own PDF. <b>Every one of the 39 pitches is an integer multiple of 0.75 pt</b>,
    /// and the integer is <c>round(asc/upem × hpx) + round(−desc/upem × hpx)</c> with
    /// <c>hpx = round(size × 96/72)</c>. Against the four candidate laws
    /// (<c>probes/sheets-r60/law-chartvmetrics.py</c>):
    /// </para>
    /// <code>
    ///   law                                      max err   mean err
    ///   pixel, no line gap                        0.089      0.036
    ///   pixel, with the line gap                  1.498      0.473
    ///   continuous, no line gap                   0.988      0.309
    ///   continuous, with the gap (was shipped)    2.476      0.613
    /// </code>
    /// <para>
    /// Carlito's <c>hhea</c> line gap is zero and both Liberation faces' is not, so the three faces
    /// separate the gap term outright rather than by argument; and the sizes where
    /// <c>size × 4/3</c> is not a whole number separate the pixel rounding from exact scaling. The
    /// ascent is the same rounding, checked independently by requiring a CENTER data label's block
    /// centre to come out size-independent: it does, to 0.042–0.069 pt over ten sizes, where the
    /// continuous model spreads it by 0.58–0.70.
    /// </para>
    /// <para>
    /// This is why <c>SheetBandText.ChartLineHeightAt</c> read 1.2207 em for Carlito at every size
    /// while 26.2.4.2 stacked it at 1.1219 em at 10 pt and 1.2241 at 15.89 — <b>chart2 does not use
    /// no device, it uses a different one</b>, and dropping the grid entirely was the defect.
    /// </para>
    /// <para>
    /// <b>And it is not a property of the newer binary.</b> Round 60's 39 pitches were measured on
    /// one workbook against 26.2.4.2 alone. Re-measured on <b>three faces × twelve sizes × two
    /// binaries × a deck and a Writer document</b> (<c>probes/chart-vertical/pitch.py</c>),
    /// <b>144 of 144</b> baseline-to-baseline distances lie within 0.019 pt of this rule and the
    /// two binaries agree with each other to <b>0.002 pt at every size</b> — so 24.2.7.2 quantises
    /// the vertical exactly as 26.2.4.2 does, and the whole-pixel <em>position</em> snapping that
    /// separates them horizontally has no vertical counterpart. Exact scaling is out by as much as
    /// 1.208 pt over the same 144.
    /// </para>
    /// <para>
    /// <b>The ascent is the other half and was measured on its own.</b> A value-axis label is
    /// centred on its tick, so <c>tick − baseline</c> is <c>ascent − height/2</c>, and both the
    /// tick and the baseline are stated outright in the reference's PDF —
    /// <c>probes/chart-vertical/tickoffset.py</c> reads the tick off the path operators rather than
    /// inferring it, so no constant has to be fitted. Against 24.2.7.2 the rule is right on
    /// <b>72 of 72</b> with no free parameter, mean residual −0.0004 pt; against 26.2.4.2 on 72 of
    /// 72 once one constant of <b>0.98 hundredths of a millimetre</b> is allowed, which is that
    /// binary placing a label's block one map unit lower and is the only vertical difference
    /// between the two. Exact scaling, given the same fitted constant, is right on 3 of 72.
    /// </para>
    /// </remarks>
    public static MetricGrid Chart { get; } = new(96, false, MetricUnit.Mm100);

    /// <summary>
    /// The device every underline and strikethrough is measured on: <b>720 dpi</b>, in the map
    /// unit of the application whose page is being written — 1/100 mm for Calc and Impress, and
    /// <see cref="WriterTextLine"/>'s twip for Writer.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The same numbers as <see cref="Spreadsheet"/> and a different fact, which is why it is
    /// named separately. A rule's thickness is not decided where the text is laid out. It is
    /// decided at PDF-writing time, twice over:
    /// </para>
    /// <para>
    /// <b>The resolution is the PDF writer's own reference device.</b>
    /// <c>PDFWriterImpl</c> is a <c>VirtualDevice</c> and its constructor asks for
    /// <c>RefDevMode::PDF1</c> = <b>720 dpi</b> (<c>vcl/source/pdf/pdfwriter_impl.cxx</c>:424-428,
    /// <c>vcl/source/gdi/virdev.cxx</c>:409-411). <c>FontMetricData::ImplInitTextLineSize</c>
    /// answers in that device's whole pixels — every thickness on both of its branches is a
    /// <c>tools::Long</c> — and <c>PDFWriterImpl::drawStraightTextLine</c> reads it from there,
    /// noting in its own first line that "units in pFontInstance are ref device pixel".
    /// </para>
    /// <para>
    /// <b>The thickness is then rounded to a whole LOGICAL unit</b>, because
    /// <c>drawStraightTextLine</c> runs it through <c>HCONV</c> =
    /// <c>DevicePixelToLogicHeight</c> = <c>CoordinateMapper::ViewToLogicDistanceY</c>, an
    /// <c>llround</c> (<c>vcl/source/outdev/CoordinateMapper.cxx</c>:278-283), and the logical
    /// unit is the one the graphics state is in when the metafile is replayed.
    /// </para>
    /// <para>
    /// <b>Which unit that is has to be measured, and reading the source alone gets it wrong.</b>
    /// <c>PDFExport::ExportSelection</c> records every page with
    /// <c>const MapMode aMapMode(MapUnit::Map100thMM)</c>
    /// (<c>filter/source/pdf/pdfexport.cxx</c>:168-179, 1305-1333), which reads as *1/100 mm for
    /// every module* — but the recording device's map mode is then whatever the application sets
    /// while it paints, and <b>Writer paints in twips</b>. Measured on 810 rules over six faces,
    /// fifteen sizes and three kinds authored identically as <c>.fodt</c>, <c>.fods</c> and
    /// <c>.fodp</c>: the Calc and Impress renderings are a whole hundredth of a millimetre on
    /// 270 of 270 each, the Writer one is a whole twip on 270 of 270 and a whole hundredth of a
    /// millimetre on <b>0</b>. It is the document's application and not the object: a rule inside
    /// a Writer text frame and inside a Writer draw object are both twips.
    /// <c>probes/quantise-r120/</c>.
    /// </para>
    /// <para>
    /// A twip is exactly two of these pixels, so a Writer rule is a whole tenth of a point and
    /// carries no rounding at all; on Calc and Impress the only widths the reference can draw are
    /// 18, 21, 25, 28, 32, … hundredths of a millimetre — <c>llround(px × 2540/720)</c>. That is
    /// the whole of O64: seven pairs measured over four spreadsheets came out 18, 21, 18, 21, 49,
    /// 28 and 14 hundredths to four decimal places, which is 5, 6, 5, 6, 14, 8 and 4 pixels. The
    /// last step of the chain shows in the same numbers — <c>PDFPage::appendMappedLength</c>
    /// writes thousandths of a point (<c>pdfwriter_utils.hxx</c>:40, <c>nLog10Divisor = 3</c>),
    /// and those five are 0.51, 0.595, 1.389, 0.794 and 0.397 pt rounded there, which is exactly
    /// what was read off the reference's own PDFs.
    /// </para>
    /// </remarks>
    public static MetricGrid TextLine { get; } = new(720, false, MetricUnit.Mm100);

    /// <summary>
    /// The same device with Writer's own map unit: <b>720 dpi in twips</b>, so a rule is a whole
    /// tenth of a point.
    /// </summary>
    /// <remarks>
    /// See <see cref="TextLine"/>, which carries the chain and the measurement. This is the grid
    /// for anything drawn on a word-processing page, a Writer text frame and a Writer draw object
    /// included — the unit belongs to the application, not to the object.
    /// </remarks>
    public static MetricGrid WriterTextLine { get; } = new(720, false, MetricUnit.Twip);

    /// <summary>Logical units to the inch, for this grid's map unit.</summary>
    private int UnitsPerInch => Unit == MetricUnit.Mm100 ? 2540 : 1440;

    /// <summary>Logical units per device pixel on this grid.</summary>
    private double UnitsPerPixel => (double)UnitsPerInch / Dpi;

    /// <summary>A length in this grid's whole logical units.</summary>
    private long ToLogical(Length value) => Unit == MetricUnit.Mm100 ? value.Mm100 : value.Twips;

    /// <summary>Whole logical units back as a length.</summary>
    private Length FromLogical(long units)
        => Unit == MetricUnit.Mm100 ? Length.FromMm100(units) : Length.FromTwips(units);

    /// <summary>
    /// A design-unit measurement in whole device pixels at an em size.
    /// </summary>
    /// <remarks>
    /// <c>FontMetricData::ImplCalcLineSpacing</c> ends with three separate <c>round()</c> calls, one
    /// per metric (<c>vcl/source/font/fontmetric.cxx</c>:538-540). C++ <c>round</c> takes a half away
    /// from zero; .NET's <c>Math.Round</c> takes it to even, and on a grid this fine the halves are
    /// common rather than exotic — a sixth of the line gaps in the corpus land on one.
    /// </remarks>
    public long ToPixels(int designUnits, int unitsPerEm, Length emSize)
    {
        if (unitsPerEm <= 0 || Dpi <= 0) return 0;

        double em = ToPixelEm(emSize);
        return (long)Math.Round(designUnits * em / unitsPerEm, MidpointRounding.AwayFromZero);
    }

    /// <summary>
    /// The em itself in whole device pixels — the size the device can actually instantiate the
    /// font at.
    /// </summary>
    /// <remarks>
    /// <c>LogicalFontInstance::GetScale</c> divides <c>m_aFontSelData.mnHeight</c> by the units per
    /// em, and that height is already whole device pixels: <c>OutputDevice::ImplNewFont</c> sets it
    /// through <c>ImplLogicHeightToDevicePixel</c>. Every other rounding on this grid is downstream
    /// of it, which is why it is one method rather than a line repeated in four.
    /// </remarks>
    public long ToPixelEm(Length emSize)
        => Dpi <= 0
            ? 0
            : (long)Math.Round(ToLogical(emSize) / UnitsPerPixel, MidpointRounding.AwayFromZero);

    /// <summary>Whole device pixels back in whole logical units of this grid's map mode.</summary>
    /// <remarks>
    /// <para>
    /// <c>CoordinateMapper::ViewToLogicDistanceY</c> is an <c>llround</c>
    /// (<c>vcl/source/outdev/CoordinateMapper.cxx</c>:279), which is again half away from zero.
    /// </para>
    /// <para>
    /// <b>And the map mode carries the page transform, so the rounding happens on the unscaled
    /// page.</b> See <see cref="PageScale"/>: the divide and the multiply do not cancel, because
    /// there is a rounding between them.
    /// </para>
    /// </remarks>
    public Length ToLength(long pixels)
    {
        if (Dpi <= 0) return Length.Zero;

        // The unscaled page is the early return and it is the ORIGINAL EXPRESSION, character for
        // character, rather than the scaled one with a 1.0 in it. Every grid but Calc's zoomed one
        // takes this branch, so the identity is the code's and not a claim about what dividing a
        // double by one does. See `PageScale`.
        double scale = double.IsFinite(PageScale) && PageScale > 0.0 ? PageScale : 1.0;

        if (scale == 1.0)
        {
            return FromLogical(
                (long)Math.Round(pixels * UnitsPerPixel, MidpointRounding.AwayFromZero));
        }

        return FromLogical((long)Math.Round(
            pixels * UnitsPerPixel / scale, MidpointRounding.AwayFromZero)) * scale;
    }

    /// <summary>The same grid with a page transform in its map mode.</summary>
    /// <remarks>
    /// See <see cref="PageScale"/>. A scale of one, or anything that is not a positive finite
    /// number, gives the grid back unchanged rather than a grid that quietly draws nothing.
    /// </remarks>
    /// <param name="scale">The multiplier the whole page is drawn through.</param>
    public MetricGrid Scaled(double scale)
        => !double.IsFinite(scale) || scale <= 0.0 || scale == 1.0
            ? this
            : this with { PageScale = scale };

    /// <summary>
    /// An advance width as the device measures it: the whole run's advance in device pixels,
    /// <b>truncated</b>, and only then converted back to a length.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Two quantisations, and the first is much the larger. The em is rounded to whole device
    /// pixels before any advance is scaled through it — <see cref="ToEmSize"/>'s rounding — so at
    /// 11 pt on a 600 dpi grid the device sets 92 pixels for 91.67 and <em>every</em> advance comes
    /// out 0.36% wider than the size the document asked for, while at 10 pt it sets 83 for 83.33 and
    /// they come out 0.4% narrower. The truncation that follows is worth
    /// at most one pixel, 0.24 pt, on a whole portion, and pulls the other way.
    /// </para>
    /// <para>
    /// <b>The truncation is of the total, not of each glyph.</b> That distinction is the whole of
    /// what <c>dotnet/probes/printer-metric-advance.py</c> settles: over 96 authored rows — two
    /// faces, four sizes, three glyphs, four run lengths, with <c>fUsePrinterMetrics</c> varied on
    /// one body — this rule is exact on 96 and rounding each glyph's advance separately is out by
    /// up to 6.96 pt. Rounding per glyph is what <c>GenericSalLayout::LayoutText</c> appears to
    /// say (<c>vcl/source/gdi/CommonSalLayout.cxx</c>:826-831, <c>std::round</c> when subpixel
    /// positioning is off) and it is not what the binary does: a mapped device turns subpixel
    /// positioning <em>on</em>, so the advances stay exact and the truncation happens once, where
    /// the device width is converted to logical units.
    /// </para>
    /// <para>
    /// On the virtual reference device none of this applies and the caller passes no grid — the
    /// same probe's control half has unquantised scaling exact on 96 of 96 there, and this rule
    /// out by 6.73 pt.
    /// </para>
    /// </remarks>
    /// <param name="designUnits">The run's advance in the face's design units.</param>
    /// <param name="unitsPerEm">The design grid that advance is in.</param>
    /// <param name="emSize">The font size the document asks for.</param>
    public Length ToAdvance(long designUnits, int unitsPerEm, Length emSize)
    {
        if (unitsPerEm <= 0 || Dpi <= 0) return Length.Zero;

        double em = ToPixelEm(emSize);
        return ToLength((long)Math.Floor(designUnits * em / unitsPerEm));
    }

    /// <summary>
    /// How much narrower — or wider — every advance comes out because the device can only set the
    /// em at a whole number of pixels.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>round(size × dpi / 72) / (size × dpi / 72)</c>. At 10 pt on a 96 dpi device that is
    /// <b>13/13.333 = 0.975</b> and at 11 pt it is <b>15/14.667 = 1.023</b>, so the correction is a
    /// <b>sawtooth in the size</b> and not a constant narrowing — which is the half of it that any
    /// "charts measure narrow" rule gets wrong. At 9, 12 and 18 pt it is exactly one.
    /// </para>
    /// <para>
    /// <b>Measured off 26.2.4.2 at fourteen sizes, and our renderer never ran.</b>
    /// <c>probes/sheets-r62/probe-chartwidth.py</c> reads the drawn advance out of the reference's
    /// own <c>TJ</c> arrays, whose per-glyph adjustments are in thousandths of the text em and are
    /// therefore independent of the chart's scale, of the page and of the size the PDF writer
    /// chose. The measured ratio follows this at every one of the fourteen, residual ≤0.005 and
    /// almost always in the direction the estimator is known to be biased.
    /// </para>
    /// <para>
    /// <b>The pixel count is taken from the size in points and not from the map unit, and that is a
    /// decision rather than a shortcut.</b> Going through hundredths of a millimetre — as
    /// <see cref="ToPixels"/> and <see cref="ToEmSize"/> do, because that is the device's own map
    /// mode — makes 9 pt come out at 12.0189 pixels rather than 12, and so applies a 0.16%
    /// correction at a size where the law says there is none. The probe cannot separate the two
    /// readings: at 9, 12 and 18 pt they differ by less than the estimator's own 0.005 noise, while
    /// the term this exists for is 2.5%. Taking the reading that leaves the exact sizes exactly
    /// alone means a chart stating 9 pt renders byte for byte as it did before, which is a property
    /// worth having when the alternative is unmeasurable either way.
    /// </para>
    /// <para>
    /// <b>What is deliberately not here.</b> The reference also rounds <em>each glyph's</em>
    /// advance to a whole hundredth of a millimetre: on <c>003_advanced_excel_pie</c>'s page 1
    /// every one of the thirty <c>;</c> and twenty-two spaces carries an identical per-glyph
    /// adjustment, which rounding the cumulative position cannot produce, and
    /// <c>round(designUnits × ppem / upem)</c> matches 9 of 9 distinct glyphs where <c>floor</c>
    /// fails 3. It is worth at most 0.014 pt a glyph and it is left out of the shipped rule,
    /// because a perturbation that small moves knife-edge axis-label collision decisions in
    /// documents where nothing else changed and this round has no measurement saying it moves them
    /// the right way. See <c>probes/sheets-r62/results.md</c> § 6.
    /// </para>
    /// </remarks>
    /// <param name="emSize">The font size the document asks for.</param>
    public double PixelEmScale(Length emSize)
    {
        if (Dpi <= 0 || emSize <= Length.Zero) return 1.0;

        double pixels = emSize.Points * Dpi / 72.0;
        return pixels <= 0.0 ? 1.0 : Math.Round(pixels, MidpointRounding.AwayFromZero) / pixels;
    }

    /// <summary>
    /// An em size as the device can actually set it: rounded to whole pixels and back.
    /// </summary>
    /// <remarks>
    /// A font is instantiated at an integer pixel size, so 11 pt on a 96 dpi device is 15 pixels
    /// rather than 14.667 and every advance it measures is 2.3% wider than the size asked for.
    /// That is invisible when the same device draws the text, and it is not invisible when one
    /// device measures and another draws — which is exactly what Calc does to decide a row's
    /// height (<see cref="MetricGrid"/>'s other users round the vertical metrics for the same
    /// reason).
    /// </remarks>
    public Length ToEmSize(Length emSize)
        => Dpi <= 0 || emSize <= Length.Zero
            ? emSize
            : ToLength(ToPixelEm(emSize));
}

/// <summary>
/// A font's vertical metrics as a line height, resolved from the several sets a font may carry.
/// </summary>
/// <param name="Ascent">Distance from the baseline to the line's top, in design units.</param>
/// <param name="Descent">Distance from the baseline to the line's bottom, positive, in design units.</param>
/// <param name="LineGap">Recommended extra leading between lines, in design units.</param>
/// <param name="Source">Which of the font's metric sets these came from.</param>
/// <param name="UnitsPerEm">The design grid the three measurements are in.</param>
/// <param name="Grid">
/// The device the measurements are rounded through, or null to scale them exactly — which is the usual
/// case and what a virtual reference device does. See <see cref="MetricGrid"/>.
/// </param>
/// <param name="LeadingAboveText">
/// Whether the face's external leading is charged to the ascent rather than left below the text.
/// <b>This is a property of the application, not of the font</b>, and LibreOffice's two text engines
/// disagree about it — see the remark on <see cref="ScaledAscent"/>.
/// </param>
/// <param name="DeclaresEastAsianCodePage">
/// Whether the face claims coverage of CP932, CP936, CP949 or CP950, which is what
/// <see cref="MetricGrid.ScalesEastAsianFaces"/> acts on. A property of the font rather than of the
/// text: Word scales the line for such a face even where the run holds nothing but Latin.
/// </param>
public readonly record struct LineMetrics(
    int Ascent,
    int Descent,
    int LineGap,
    LineMetricSource Source,
    int UnitsPerEm,
    MetricGrid? Grid = null,
    bool LeadingAboveText = false,
    bool DeclaresEastAsianCodePage = false)
{
    /// <summary>The distance from one baseline to the next, in design units.</summary>
    public int LineHeight => Ascent + Descent + LineGap;

    /// <summary>
    /// The line height at an em size.
    /// </summary>
    /// <param name="emSize">The font size the document asks for.</param>
    public Length ScaledLineHeight(Length emSize)
        => Grid is { } grid
            ? LeadingAboveText
                ? grid.EastAsianScaled(TextHeightOn(grid, emSize), DeclaresEastAsianCodePage)
                  + LeadingOn(grid, emSize)
                : EditHeightOn(grid, emSize)
            : Scale(LineHeight, emSize);

    /// <summary>The ascent at an em size.</summary>
    /// <remarks>
    /// <para>
    /// Where a face's external leading sits in the line box is decided by the application, not by the
    /// font, and LibreOffice's two text engines answer differently — so this is
    /// <see cref="LeadingAboveText"/>'s question rather than a rule that can be applied here once.
    /// </para>
    /// <para>
    /// <b>Writer puts it above.</b> <c>SwFntObj::GetFontAscent</c> adds the external leading to the
    /// ascent it read from the device, guarded only by <c>#if !defined(MACOSX)</c> and carrying a TODO
    /// to do it on the other platforms too (<c>sw/source/core/txtnode/fntcache.cxx</c>:326-329);
    /// <c>GetFontHeight</c> adds the same leading to ascent-plus-descent (<c>:370-371</c>), so the
    /// descent stays the face's own and the three quantities close.
    /// </para>
    /// <para>
    /// <b>EditEngine does not</b>, which is what Impress, Calc and Writer's own drawing objects format
    /// through. <c>ImpEditEngine::RecalcFormatterFontMetrics</c> adds it only when
    /// <c>IsAddExtLeading()</c> (<c>editeng/source/editeng/impedit3.cxx</c>:3133-3135), and that flag is
    /// false unless something turns it on — <c>ImpEditEngine</c> initialises it so
    /// (<c>impedit2.cxx</c>:118), as does <c>SdrModel</c> (<c>svx/source/svdraw/svdmodel.cxx</c>:161).
    /// Only Writer's document compatibility setting and Math's own engine ever set it. A
    /// <c>FormatterFontMetric</c>'s height is then <c>nMaxAscent + nMaxDescent</c> with no gap in it.
    /// </para>
    /// <para>
    /// Measured on Liberation Sans, whose <c>hhea</c> gap is 67/2048, both ways round: Writer's first
    /// baseline sits 206 twips below a 72 pt top margin at 11 pt (72 + 9.958 + 0.360, each rounded to
    /// whole twips) where the ascent alone would give 199; and Impress puts two 18 pt baselines in a
    /// table cell 20.154 pt apart, which is ascent-plus-descent and not the 20.698 the gap would add.
    /// Both figures are read out of LibreOffice's own PDF content stream. A face whose gap is zero —
    /// Carlito, which nearly every OOXML document in the corpus resolves to through its theme — is
    /// identical either way, which is why getting this wrong stayed invisible for so long.
    /// </para>
    /// </remarks>
    public Length ScaledAscent(Length emSize)
    {
        if (Grid is not { } grid)
        {
            return Scale(LeadingAboveText ? Ascent + LineGap : Ascent, emSize);
        }

        Length ascent = grid.ToLength(grid.ToPixels(Ascent, UnitsPerEm, emSize));

        // Both of this type's application-specific rules hang off `LeadingAboveText`, and they are
        // written on the same branch so that they are mutually exclusive *by construction* rather
        // than merely unlikely to meet. Writer's ascent carries the external leading and is subject
        // to the East Asian scale (`SwFntObj::GetFontAscent`); EditEngine's carries neither, and its
        // line height is the taller of two roundings instead — see `EditHeightOn`.
        //
        // It matters because `ScaledDescent` is `height − ascent`: a grid that scaled the ascent
        // while the height took EditEngine's branch could return a negative descent.
        return LeadingAboveText
            ? grid.EastAsianScaled(ascent, DeclaresEastAsianCodePage) + LeadingOn(grid, emSize)
            : ascent;
    }

    /// <summary>The descent at an em size.</summary>
    /// <remarks>
    /// Derived from the line height rather than measured, on a grid, so that the three quantities
    /// close: the device rounds the ascent and the height, and whatever is left over is the
    /// descent. It comes out right for both engines because <see cref="ScaledLineHeight"/> already
    /// distinguishes them — Writer's leaves the leading in and EditEngine's takes the taller of two
    /// roundings, and subtracting Writer's leading-bearing ascent or EditEngine's plain one gives
    /// the descent each of them actually draws.
    /// </remarks>
    public Length ScaledDescent(Length emSize)
        => Grid is { } grid
            ? ScaledLineHeight(emSize) - ScaledAscent(emSize)
            : Scale(Descent, emSize);

    /// <summary>
    /// The ascent and descent together, as one rounding rather than two.
    /// </summary>
    /// <remarks>
    /// <c>OutputDevice::GetTextHeight</c> converts the summed device-pixel ascent and descent to logical
    /// units in a single step, so rounding each and adding gives a different answer on the grids where it
    /// matters.
    /// </remarks>
    private Length TextHeightOn(MetricGrid grid, Length emSize)
        => grid.ToLength(
            grid.ToPixels(Ascent, UnitsPerEm, emSize) + grid.ToPixels(Descent, UnitsPerEm, emSize));

    private Length LeadingOn(MetricGrid grid, Length emSize)
        => grid.ToLength(grid.ToPixels(LineGap, UnitsPerEm, emSize));

    /// <summary>
    /// The line height EditEngine gives this face: the <b>taller</b> of the two roundings, and no
    /// external leading.
    /// </summary>
    /// <remarks>
    /// <para>
    /// EditEngine measures a line twice and keeps the larger. The text portion's own height is
    /// <c>OutputDevice::GetTextHeight</c>, which converts the summed device-pixel ascent and
    /// descent in one step; the formatter metric is <c>FormatterFontMetric::GetHeight</c>, which
    /// is <c>GetFontMetric().GetAscent() + GetDescent()</c> and so converts each on its own
    /// (<c>vcl/source/outdev/font.cxx</c>:351-352). <c>ImpEditEngine::CreateLines</c> then does
    /// <c>if (nLineHeight > pLine-&gt;GetHeight()) pLine-&gt;SetHeight(nLineHeight)</c> —
    /// <c>editeng/source/editeng/impedit3.cxx</c>:1516-1518.
    /// </para>
    /// <para>
    /// <b>Neither rounding fits on its own and the maximum of them fits exactly.</b> On Impress's
    /// 600 dpi device, converting each separately is right on 274 of 312 measured (face, size)
    /// pairs and converting the sum once is right on 270 — and the two disagree in opposite
    /// directions, so their maximum is right on 312 of 312. The same holds on Calc's 720 dpi
    /// device, 273 of 273. See <c>probes/refdev-01/</c>.
    /// </para>
    /// <para>
    /// The external leading is absent because <c>IsAddExtLeading()</c> is false in every engine
    /// that reaches here — see the remark on <see cref="ScaledAscent"/>, which is the same
    /// discriminator: <see cref="LeadingAboveText"/> is Writer, and everything else is EditEngine.
    /// </para>
    /// </remarks>
    private Length EditHeightOn(MetricGrid grid, Length emSize)
    {
        long ascent = grid.ToPixels(Ascent, UnitsPerEm, emSize);
        long descent = grid.ToPixels(Descent, UnitsPerEm, emSize);

        return Length.Max(
            grid.ToLength(ascent) + grid.ToLength(descent),
            grid.ToLength(ascent + descent));
    }

    /// <summary>
    /// The internal leading at an em size: how much of the line height is above and below the em.
    /// </summary>
    /// <remarks>
    /// Derived, not read from the font — ascent plus descent minus the requested size, which is the
    /// classic Windows definition and what single line spacing consumes. A font whose Windows metrics
    /// exceed its em square, which most do, therefore has positive internal leading, and that is
    /// where "single-spaced" lines get the gap they visibly have.
    /// </remarks>
    public Length ScaledInternalLeading(Length emSize)
        => Scale(Ascent + Descent, emSize) - emSize;

    private Length Scale(int designUnits, Length emSize)
        => UnitsPerEm <= 0
            ? Length.Zero
            : Length.FromEmu((long)Math.Round((double)designUnits * emSize.Emu / UnitsPerEm));
}

/// <summary>
/// Derives a line height from a face's metrics, the way LibreOffice does.
/// </summary>
/// <remarks>
/// <para>
/// Fonts disagree about which of their own metric sets to believe, so there is no single field to
/// read. The precedence is specific, and it is specific for historical reasons rather than
/// typographic ones (<c>research/06-rendering.md</c> section B.4):
/// </para>
/// <list type="number">
/// <item>
/// <description>
/// <c>hhea</c> first, since it is mandatory — but only if its signs are right. A font whose ascent is
/// negative or whose descent is positive has them the wrong way round, real fonts do this, and
/// believing one puts the baseline outside the line. This is what a line is normally measured from.
/// </description>
/// </item>
/// <item>
/// <description>
/// <c>OS/2</c>'s <c>usWinAscent</c> and <c>usWinDescent</c> only when <c>hhea</c> yielded nothing.
/// They carry no leading of their own, and none is borrowed from <c>hhea</c> — a line measured from
/// the Windows metrics is exactly ascent plus descent.
/// </description>
/// </item>
/// <item>
/// <description>
/// Over either of those, the typographic metrics when <c>fsSelection</c> bit 7 is set, which is the
/// font saying "believe my real typographic metrics, not the historically bloated Windows ones".
/// </description>
/// </item>
/// </list>
/// <para>
/// <b>The Windows metrics are not the default, and this is worth stating because the received wisdom
/// says they are.</b> They were the default in LibreOffice once; today
/// <c>FontMetricData::ImplCalcLineSpacing</c> (<c>vcl/source/font/fontmetric.cxx</c>:434) reaches them
/// only when <c>hhea</c> gave nothing or when the family is one of four entries in the
/// <c>Office::Common::Misc::FontsUseWinMetrics</c> exception list — fonts known to state metrics that
/// make them unreadable. The list is not reproduced here: none of its four faces is one a document in
/// any corpus measured so far asks for, and honouring it needs the family name that
/// <see cref="Resolve"/> deliberately does not take.
/// </para>
/// <para>
/// Measured rather than read: a paragraph set in IPAGothic at 20pt, whose <c>hhea</c> and Windows
/// metrics differ by 7.6% of the em, renders with a 20.00pt line advance in LibreOffice 24.2 — the
/// <c>hhea</c> figure exactly. Across every font installed on the reference machine the two rules
/// disagree on three faces, all CJK, which is why believing the wrong one went unnoticed.
/// </para>
/// <para>
/// Getting the order wrong does not produce an error. It produces a line height a few per cent out,
/// which moves every baseline on the page and eventually moves a page break — so a document renders
/// plausibly and disagrees with the reference everywhere.
/// </para>
/// </remarks>
public static class LineSpacing
{
    /// <summary>
    /// The ascent and descent, as a fraction of the em, assumed for a face with no usable metrics.
    /// </summary>
    /// <remarks>
    /// Four-fifths above the baseline and one-fifth below, which is roughly where a Latin font puts
    /// them. A face this broken will not lay out correctly whatever is assumed; the point is to
    /// produce something rather than a zero-height line that makes every page infinitely long.
    /// </remarks>
    private const double FallbackAscentFraction = 0.8;

    /// <summary>Resolves a face's line metrics.</summary>
    /// <param name="face">The face to measure.</param>
    /// <param name="grid">
    /// The device grid to round the metrics through, or null to scale them exactly. Only a document that
    /// asks to be laid out against a printer passes one — see <see cref="MetricGrid"/>.
    /// </param>
    /// <param name="leadingAboveText">
    /// Whether to charge the face's external leading to the ascent. True for Writer's own text, false
    /// for everything that formats through EditEngine — Impress, Calc and Writer's drawing objects.
    /// See <see cref="LineMetrics.ScaledAscent"/>, which carries the citations and the measurements.
    /// </param>
    public static LineMetrics Resolve(
        OpenTypeFace face, MetricGrid? grid = null, bool leadingAboveText = false)
    {
        ArgumentNullException.ThrowIfNull(face);

        int unitsPerEm = face.UnitsPerEm;

        // Step one: hhea, if its signs make sense. Descent is stored negative and used positive.
        int ascent = 0;
        int descent = 0;
        int lineGap = 0;
        LineMetricSource source = LineMetricSource.Fallback;

        if (face.Horizontal.IsPlausible
            && (face.Horizontal.Ascender != 0 || face.Horizontal.Descender != 0))
        {
            ascent = face.Horizontal.Ascender;
            descent = -face.Horizontal.Descender;
            lineGap = Math.Max(0, face.Horizontal.LineGap);
            source = LineMetricSource.HorizontalHeader;
        }

        // Step two: OS/2, for the two cases that beat hhea — hhea having said nothing at all, and the
        // font asking for its typographic metrics by name.
        if (face.Os2 is { } os2)
        {
            if (source == LineMetricSource.Fallback
                && (os2.WindowsAscent != 0 || os2.WindowsDescent != 0))
            {
                ascent = os2.WindowsAscent;
                descent = os2.WindowsDescent;

                // No leading. The Windows metrics state none, and hhea's cannot be borrowed: hhea is
                // why this branch was taken, so whatever it holds was already rejected.
                lineGap = 0;
                source = LineMetricSource.WindowsMetrics;
            }

            if (os2.UseTypoMetrics
                && os2.TypoAscender >= 0
                && os2.TypoDescender <= 0
                && (os2.TypoAscender != 0 || os2.TypoDescender != 0))
            {
                ascent = os2.TypoAscender;
                descent = -os2.TypoDescender;
                lineGap = Math.Max(0, os2.TypoLineGap);
                source = LineMetricSource.TypographicMetrics;
            }
        }

        if (ascent + descent <= 0)
        {
            ascent = (int)Math.Round(unitsPerEm * FallbackAscentFraction);
            descent = unitsPerEm - ascent;
            lineGap = 0;
            source = LineMetricSource.Fallback;
        }

        return new LineMetrics(
            ascent, descent, lineGap, source, unitsPerEm, grid, leadingAboveText,
            DeclaresEastAsianCodePage: face.Os2?.DeclaresEastAsianCodePage ?? false);
    }

    /// <summary>
    /// The families whose <c>post</c> underline metrics LibreOffice refuses to use.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Not a workaround of ours. It is LibreOffice's own shipped configuration —
    /// <c>Office::Common::Misc::FontsDontUseUnderlineMetrics</c>, tdf#152267 and tdf#154235 —
    /// consulted by <c>FontMetricData::ShouldNotUseUnderlineMetrics</c>
    /// (<c>vcl/source/font/fontmetric.cxx:190</c>) before it will read the face's own numbers.
    /// </para>
    /// <para>
    /// It matters far more than three names suggest, because these three <em>are</em> the
    /// metric-compatible substitutes for Arial, Times New Roman and Courier New, so they are what
    /// most of a real corpus is actually set in. Their <c>post</c> tables are wrong in a way that
    /// shows: Liberation Serif Bold declares a thickness of 195 units and an offset of 28, which
    /// at 28 pt is a 2.67 pt rule drawn 0.38 pt under the baseline — nearly touching the text and
    /// almost twice as thick as the 1.53 pt rule at 2.30 pt that LibreOffice actually draws.
    /// </para>
    /// </remarks>
    private static readonly string[] FontsWithoutUsableUnderlineMetrics =
        ["Liberation Serif", "Liberation Sans", "Liberation Mono"];

    /// <summary>
    /// The underline and strikethrough metrics, in design units.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Both come from tables that may be absent or zero, and a zero-thickness line draws nothing —
    /// so each falls back to a fraction of the em rather than being left at zero. The underline sits
    /// below the baseline, which the font records as a negative offset; the strikethrough sits above
    /// it, recorded positive.
    /// </para>
    /// <para>
    /// A face on <see cref="FontsWithoutUsableUnderlineMetrics"/> is treated as declaring nothing
    /// at all, which sends it down the same descent-derived path LibreOffice uses — see
    /// <see cref="FromDescent"/>.
    /// </para>
    /// </remarks>
    public static FontVerticalMetrics ResolveDecorations(OpenTypeFace face, LineMetrics line)
    {
        ArgumentNullException.ThrowIfNull(face);

        return ResolveDecorations(face.FamilyName, face.Post, face.Os2, line);
    }

    /// <summary>
    /// The same, from the four things the answer actually depends on.
    /// </summary>
    /// <remarks>
    /// The family name is one of them, and not incidentally: it is the whole discriminator for
    /// <see cref="FontsWithoutUsableUnderlineMetrics"/>.
    /// </remarks>
    /// <param name="family">The face's family name, as the blacklist spells it.</param>
    /// <param name="post">Its <c>post</c> table.</param>
    /// <param name="os2">Its <c>OS/2</c> table, or null when it has none.</param>
    /// <param name="line">Its resolved line metrics, in design units.</param>
    public static FontVerticalMetrics ResolveDecorations(
        string? family, PostTable post, Os2Table? os2, LineMetrics line)
    {
        if (family is not null
            && Array.IndexOf(FontsWithoutUsableUnderlineMetrics, family) >= 0)
        {
            return FromDescent(line);
        }

        int unitsPerEm = line.UnitsPerEm > 0 ? line.UnitsPerEm : 1000;

        int underlineThickness = post.UnderlineThickness > 0
            ? post.UnderlineThickness
            : Math.Max(1, unitsPerEm / 20);

        int underlinePosition = post.UnderlinePosition != 0
            ? post.UnderlinePosition
            : -Math.Max(1, unitsPerEm / 10);

        int strikeoutThickness = os2?.StrikeoutSize > 0
            ? os2.Value.StrikeoutSize
            : underlineThickness;

        // A quarter of the em above the baseline is roughly the middle of a lower-case letter, which
        // is where a strikethrough belongs when the font declines to say.
        int strikeoutPosition = os2?.StrikeoutPosition is > 0
            ? os2!.Value.StrikeoutPosition
            : Math.Max(1, unitsPerEm / 4);

        return new FontVerticalMetrics(
            line.Ascent,
            line.Descent,
            line.LineGap,
            underlinePosition,
            underlineThickness,
            strikeoutPosition,
            strikeoutThickness);
    }

    /// <summary>
    /// Decorations derived from the line metrics rather than from the face's own tables.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>FontMetricData::ImplInitTextLineSize</c>, <c>vcl/source/font/fontmetric.cxx:261-330</c>,
    /// which is the path VCL takes for every font whose <c>post</c> metrics it will not read. A
    /// rule is a quarter of the descent thick and hangs half a descent below the baseline, less
    /// half its own thickness so that the stated offset is to its top; a strikethrough sits a
    /// third of the way up the ascent, less its internal leading.
    /// </para>
    /// <para>
    /// <strong>The clamp applies to the thickness and not to the offset</strong>, which is easy to
    /// miss and reads as arbitrary until you see the two variables: the C++ reassigns its local
    /// <c>nDescent</c> for the line height while <c>nUnderlineOffset</c> is computed from the
    /// member <c>mnDescent</c>. It fires on a face whose descent is more than a third of its
    /// ascent — #i55341, "for some fonts it is not a good idea to calculate their text line
    /// metrics from the real font descent".
    /// </para>
    /// <para>
    /// LibreOffice does this arithmetic in device units, so its results carry a rounding of one
    /// hundredth of a millimetre — 0.028 pt — that design units cannot reproduce. Measured against
    /// its own PDF for 28 pt Liberation Serif Bold, this gives 2.269 pt where it draws 2.296 and
    /// 1.518 pt thick where it draws 1.531: a tenth of a pixel at 300 dpi.
    /// </para>
    /// </remarks>
    /// <param name="line">The face's resolved line metrics, in design units.</param>
    private static FontVerticalMetrics FromDescent(LineMetrics line)
    {
        int descent = line.Descent > 0 ? line.Descent : Math.Max(1, line.Ascent / 10);
        int clamped = 3 * descent > line.Ascent ? line.Ascent / 3 : descent;

        int thickness = Math.Max(1, (clamped * 25 + 50) / 100);
        int half = thickness / 2;

        // The face's internal leading: how much of its line box sits outside the em.
        int internalLeading = Math.Max(0, line.Ascent + line.Descent - line.UnitsPerEm);

        return new FontVerticalMetrics(
            line.Ascent,
            line.Descent,
            line.LineGap,

            // Negative below the baseline, which is the sign convention a post table uses and the
            // opposite of VCL's own — its offsets are positive downwards.
            -((descent / 2) - half),
            thickness,
            ((line.Ascent - internalLeading) / 3) + half,
            thickness);
    }

    /// <summary>
    /// The thickness the reference actually draws each kind of rule at, already quantised onto the
    /// device that draws it.
    /// </summary>
    /// <remarks>
    /// A <see cref="Length"/> rather than a design-unit count, because the quantisation is the
    /// answer: see <see cref="MetricGrid.TextLine"/>. Every value is a whole hundredth of a
    /// millimetre.
    /// </remarks>
    /// <param name="Underline">One line under the text.</param>
    /// <param name="DoubleUnderline">
    /// Each of the two lines of a double underline, which is <em>thinner</em> than a single one on
    /// both of the reference's branches and was drawn at the single thickness here.
    /// </param>
    /// <param name="Strikeout">The line through the text.</param>
    /// <param name="BoldUnderline">
    /// The single line of a <em>bold</em> underline, which is about twice an ordinary one:
    /// <c>ceil(2 × underlineThickness × scale)</c> on the HarfBuzz branch and
    /// <c>((descent × 50) + 50) / 100</c> on the descent one, with a guard bumping it by one where
    /// that comes out equal to the single (<c>fontmetric.cxx</c>:289-291).
    /// </param>
    /// <param name="BoldUnderlineOffset">Where its top edge sits, below the baseline.</param>
    /// <param name="UnderlineOffset">
    /// How far below the baseline the <em>top edge</em> of a single underline sits.
    /// <para>
    /// The same device chain as the thickness beside it, and for the same reason: the offset is a
    /// whole number of the writer's 720 dpi pixels rounded to a whole logical unit, so no fraction
    /// of the em reproduces it. This tree took it from the design units until round 123 measured
    /// the difference — 0.063 pt on average and 0.139 at worst over 84 authored rules, against the
    /// 0.02 pt figure on which round 120 declined to touch it.
    /// </para>
    /// </param>
    /// <param name="StrikeoutOffset">
    /// The same for the line through the text, and <em>negative</em>: a strikeout sits above the
    /// baseline.
    /// </param>
    /// <param name="DoubleUnderlineFirst">
    /// How far below the baseline the <em>top edge</em> of a double underline's first line sits.
    /// <para>
    /// A double underline is the one rule whose position cannot be taken from the design units the
    /// other two still come from, and it is not a rounding difference: the two lines straddle where
    /// a single one would sit, their separation is floored on the device rather than scaled with
    /// the em, and the writer adds a whole further thickness to the second. See
    /// <see cref="DoubleUnderlineSecond"/> for the arithmetic and the measurement.
    /// </para>
    /// </param>
    /// <param name="DoubleUnderlineSecond">
    /// The same for its second line — <em>three</em> thicknesses below the first where the gap is
    /// not floored, and not the two a reader of the metric alone would expect.
    /// <para>
    /// <c>ImplInitTextLineSize</c> puts the two lines' tops <c>mnDUnderlineSize × 2</c> apart on the
    /// HarfBuzz branch (<c>fontmetric.cxx</c>:238) and <c>n2LineDY + n2LineHeight</c> apart on the
    /// descent one (:319), where <c>n2LineDY</c> is <c>max(n2LineHeight, 1 + DPIY/150)</c> — a floor
    /// of <b>five pixels</b> on the PDF writer's 720 dpi device, which binds at every size where a
    /// double underline is thinner than that. <c>drawStraightTextLine</c> then centres the first
    /// stroke by adding half a thickness (tdf#154235, <c>pdfwriter_impl.cxx</c>:6741) and centres
    /// the second by adding half a thickness <em>and a whole one more</em> (:6857), which is
    /// asymmetric and is reproduced because it is what the reference draws.
    /// </para>
    /// </param>
    public readonly record struct RuleWidths(
        Length Underline,
        Length DoubleUnderline,
        Length Strikeout,
        Length BoldUnderline = default,
        Length BoldUnderlineOffset = default,
        Length UnderlineOffset = default,
        Length StrikeoutOffset = default,
        Length DoubleUnderlineFirst = default,
        Length DoubleUnderlineSecond = default);

    /// <summary>
    /// The thicknesses of a face's rules at a size, as <c>FontMetricData::ImplInitTextLineSize</c>
    /// computes them and the PDF writer quantises them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The companion of <see cref="ResolveDecorations(OpenTypeFace, LineMetrics)"/>, which answers
    /// the same two branches in <em>design units</em> and is what the offsets still come from. A
    /// thickness cannot be answered that way, because the reference's is an integer count of
    /// 720 dpi device pixels converted to a whole hundredth of a millimetre — so it is neither a
    /// fraction of the em nor a continuous function of the size, and no design-unit value can
    /// reproduce it. See <see cref="MetricGrid.TextLine"/> for the chain and the citations.
    /// </para>
    /// <para>
    /// <b>The two branches are the same two, and the fallback is the one most of a corpus takes</b>
    /// — <see cref="FontsWithoutUsableUnderlineMetrics"/> holds the three metric-compatible
    /// substitutes for Arial, Times New Roman and Courier New. The HarfBuzz branch takes
    /// <c>std::ceil</c> of the face's own <c>post</c> and <c>OS/2</c> values scaled to the device,
    /// and a double underline is two thirds of a single one there; the fallback derives everything
    /// from the device's rounded descent, a quarter of it for a single rule and
    /// <c>((descent × 16) + 50) / 100</c> for each line of a double.
    /// </para>
    /// <para>
    /// <b>Where this departs from the C++, deliberately and in one place.</b> HarfBuzz answers a
    /// present-but-zero <c>post.underlineThickness</c> successfully, so the reference would take
    /// that branch and draw nothing at all; this treats a face declaring no underline metric as one
    /// whose tables cannot be read and sends it down the descent branch, exactly as
    /// <see cref="ResolveDecorations(string?, PostTable, Os2Table?, LineMetrics)"/> already did.
    /// <b>No installed face on this machine declares one</b> — 0 of 137 across
    /// <c>/usr/share/fonts</c> and the reference's own bundle state a zero <c>underlineThickness</c>
    /// or <c>yStrikeoutSize</c> — so the divergence is unmeasurable here and is recorded rather
    /// than defended.
    /// </para>
    /// </remarks>
    /// <param name="face">The face the rule sits under.</param>
    /// <param name="line">Its resolved line metrics, in design units.</param>
    /// <param name="size">The size the document asks for.</param>
    /// <param name="grid">The device and map unit the page is written in.</param>
    public static RuleWidths ResolveRuleWidths(
        OpenTypeFace face, LineMetrics line, Length size, MetricGrid grid)
    {
        ArgumentNullException.ThrowIfNull(face);

        return ResolveRuleWidths(face.FamilyName, face.Post, face.Os2, line, size, grid);
    }

    /// <summary>The same, from the five things the answer depends on.</summary>
    /// <param name="family">The face's family name, as the blacklist spells it.</param>
    /// <param name="post">Its <c>post</c> table.</param>
    /// <param name="os2">Its <c>OS/2</c> table, or null when it has none.</param>
    /// <param name="line">Its resolved line metrics, in design units.</param>
    /// <param name="size">The size the document asks for.</param>
    /// <param name="grid">
    /// <see cref="MetricGrid.TextLine"/> for a spreadsheet or a presentation and
    /// <see cref="MetricGrid.WriterTextLine"/> for a word-processing page: same device, different
    /// map unit, and the difference is measured rather than read.
    /// </param>
    public static RuleWidths ResolveRuleWidths(
        string? family, PostTable post, Os2Table? os2, LineMetrics line, Length size,
        MetricGrid grid)
    {
        int unitsPerEm = line.UnitsPerEm > 0 ? line.UnitsPerEm : 1000;
        long em = grid.ToPixelEm(size);
        if (em <= 0) return default;

        bool blacklisted = family is not null
                           && Array.IndexOf(FontsWithoutUsableUnderlineMetrics, family) >= 0;

        // Both tables have to be readable for the metric branch: the C++ bails to the descent as
        // soon as any one of its four HarfBuzz queries fails, and a query fails when the table is
        // absent. A `post` reporting nothing at all is how this reader represents an absent one.
        bool declares = os2 is { StrikeoutSize: > 0 } && post.UnderlineThickness > 0;

        if (!blacklisted && declares)
        {
            double scale = (double)em / unitsPerEm;
            double single = post.UnderlineThickness * scale;

            // `nBSize = nSize * 2; n2Size = nBSize / 3`, so two thirds and not a half.
            long doubleSize = (long)Math.Ceiling(single * 2.0 / 3.0);

            // `mnBUnderlineOffset = ceil(nOffset - nSize/2)` and the double underline's first
            // line sits there, `mnDUnderlineOffset1 = mnBUnderlineOffset` (fontmetric.cxx:234-238).
            // The face states the underline's position as negative below the baseline and VCL
            // negates it, so this is positive downwards.
            long first = (long)Math.Ceiling((-post.UnderlinePosition * scale) - (single / 2.0));

            long singleSize = (long)Math.Ceiling(single);
            long strikeSize = (long)Math.Ceiling(os2!.Value.StrikeoutSize * scale);

            // `mnBUnderlineSize = ceil(nBSize)` with `nBSize = nSize * 2`, and a bold underline
            // shares the double's own offset — both are `mnBUnderlineOffset` (fontmetric.cxx:232-237).
            long boldSize = (long)Math.Ceiling(single * 2.0);

            return DoublePositions(
                new RuleWidths(
                    grid.ToLength(singleSize),
                    grid.ToLength(doubleSize),
                    grid.ToLength(strikeSize),
                    grid.ToLength(boldSize),
                    Stroked(boldSize, first, grid),
                    Stroked(singleSize, (long)Math.Ceiling(-post.UnderlinePosition * scale), grid),
                    Stroked(
                        strikeSize,
                        (long)Math.Ceiling(-os2.Value.StrikeoutPosition * scale),
                        grid)),
                doubleSize, first, first + (doubleSize * 2), grid);
        }

        long ascent = grid.ToPixels(line.Ascent, unitsPerEm, size);
        long rawDescent = grid.ToPixels(line.Descent, unitsPerEm, size);
        long descent = rawDescent;

        if (descent <= 0) descent = Math.Max(1, ascent / 10);

        // #i55341 again, and on the device's own integers this time.
        if (3 * descent > ascent) descent = ascent / 3;

        long lineHeight = Math.Max(1, ((descent * 25) + 50) / 100);
        long doubleHeight = Math.Max(1, ((descent * 16) + 50) / 100);

        // **The two descents are different and the C++ says so.** The thicknesses above take the
        // #i55341-clamped descent; `nUnderlineOffset` takes the raw `mnDescent`
        // (`fontmetric.cxx`:315), so a face whose clamp fires — Liberation Mono at every size —
        // has its rules sized off one number and placed off another.
        long underlineOffset = (rawDescent / 2) + 1;

        // `n2LineDY = max(n2LineHeight, 1 + DPIY/150)`, and on a 720 dpi device that floor is FIVE
        // pixels (fontmetric.cxx:299-306, #117909's "add some pixels ... on higher resolution
        // devices"). It binds wherever a double underline is thinner than five pixels, which is
        // every Liberation face below about 12 pt.
        long gap = Math.Max(doubleHeight, 1 + (grid.Dpi / 150));
        long firstOffset = underlineOffset - Math.Max(1, gap / 2) - doubleHeight;

        // `mnIntLeading = mnAscent + mnDescent - mnHeight`, with no clamp at zero
        // (fontmetric.cxx:543), and a strikeout sits a third of the ascent net of it above the
        // baseline (:313). Negative, because this axis points down.
        long internalLeading = ascent != 0 || rawDescent != 0 ? ascent + rawDescent - em : 0;
        long strikeoutOffset = -((ascent - internalLeading) / 3);

        long halfLine = Math.Max(1, lineHeight / 2);

        // `nBLineHeight = ((nDescent * 50) + 50) / 100`, and then the guard that makes this a
        // branch rather than a doubling: **where that comes out equal to the single, it is bumped
        // by one** (fontmetric.cxx:289-291). It bites only at a descent of one or two device
        // pixels, where the single's own `!nLineHeight -> 1` clamp has already floored both at 1 —
        // sub-point text on a 720 dpi device, which no corpus document reaches. It is here because
        // it is in the source, and it is recorded as UNWITNESSED.
        long boldHeight = ((descent * 50) + 50) / 100;
        if (boldHeight == lineHeight) boldHeight++;
        long halfBold = Math.Max(1, boldHeight / 2);

        return DoublePositions(
            new RuleWidths(
                grid.ToLength(lineHeight),
                grid.ToLength(doubleHeight),
                grid.ToLength(lineHeight),
                grid.ToLength(boldHeight),
                Stroked(boldHeight, underlineOffset - halfBold, grid),
                Stroked(lineHeight, underlineOffset - halfLine, grid),
                Stroked(lineHeight, strikeoutOffset - halfLine, grid)),
            doubleHeight, firstOffset, firstOffset + gap + doubleHeight, grid);
    }

    /// <summary>
    /// Where the two lines of a double underline are actually stroked, from the metric's own two
    /// offsets in device pixels.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>PDFWriterImpl::drawStraightTextLine</c> takes both offsets as the distance to the
    /// <em>top</em> of the line and converts them to the stroke's centre, which is what a PDF
    /// segment states. tdf#154235 does that by adding <c>nLineHeight / 2</c> — an integer division
    /// on the device's pixels — to each (<c>pdfwriter_impl.cxx</c>:6741, :6751-6753); the second
    /// line then has a whole further <c>nLineHeight</c> added when it is emitted (:6857). The
    /// asymmetry is the reference's, not a misreading: measured on 84 double rules over a Writer
    /// and a Calc rendering it is exact on all 84, and dropping the extra thickness makes every one
    /// of them wrong.
    /// </para>
    /// <para>
    /// A rule is filled here and stroked there (C16), so each centre is turned back into a top edge
    /// by half the drawn thickness — which is half a logical unit, not a whole one, and is why
    /// these are <see cref="Length"/> rather than a unit count.
    /// </para>
    /// </remarks>
    /// <summary>
    /// Where one line's top edge lands, from the metric's own offset in device pixels.
    /// </summary>
    /// <remarks>
    /// <c>drawStraightTextLine</c> takes the offset as the distance to the line's <em>top</em> and
    /// states the stroke's <em>centre</em>, adding <c>nLineHeight / 2</c> — an integer division on
    /// the device's pixels, so it is not half the drawn thickness — before converting
    /// (<c>pdfwriter_impl.cxx</c>:6741, :6751-6752). This tree fills the rule rather than stroking
    /// it (C16), so the centre is turned back into a top edge by half the drawn thickness, which
    /// is a different number again.
    /// </remarks>
    private static Length Stroked(long sizePixels, long offsetPixels, MetricGrid grid)
        => grid.ToLength(offsetPixels + (sizePixels / 2)) - (grid.ToLength(sizePixels) / 2);

    private static RuleWidths DoublePositions(
        RuleWidths widths, long sizePixels, long firstPixels, long secondPixels, MetricGrid grid)
    {
        long centring = sizePixels / 2;
        Length height = grid.ToLength(sizePixels);
        Length half = height / 2;

        return widths with
        {
            DoubleUnderlineFirst = grid.ToLength(firstPixels + centring) - half,
            DoubleUnderlineSecond = grid.ToLength(secondPixels + centring) + height - half,
        };
    }

    /// <summary>
    /// The advance width of the CJK ideograph U+6C34, in design units, or zero when absent.
    /// </summary>
    /// <remarks>
    /// LibreOffice measures the CJK advance from this one character — 水, "water" — rather than from
    /// the em square, because a CJK font's ideographs are not always exactly one em wide and the
    /// grid CJK text is laid out on is what its ideographs actually measure.
    /// </remarks>
    public static int CjkAdvance(OpenTypeFace face)
    {
        ArgumentNullException.ThrowIfNull(face);
        return face.AdvanceForCharacter(0x6C34);
    }
}
