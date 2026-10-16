using System.Xml.Linq;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.MsBinary;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A column width is a count of digits of the workbook's default font, not a length.
/// </summary>
/// <remarks>
/// <para>
/// Both Excel formats state it that way — SpreadsheetML in digits and BIFF in 256ths of one — so
/// a width is not a measurement until that face has been measured. It was measured as a constant
/// 111 twips, which is ten-point Liberation Sans, and every workbook whose default font is
/// anything else therefore had proportionally wrong columns: the corpus's
/// <c>Patent Index 2024 - Top 100 applicants 2024.xlsx</c> defaults to twelve-point Arial, whose
/// digit is 133 twips, and its columns came out 20% too narrow.
/// </para>
/// <para>
/// The measurement belongs to layout and the reading does not, so the readers carry the digits
/// and <see cref="SheetLayout.Grid"/> converts them. That split is what these tests pin: that the
/// digits survive the reader, that the font reaches them, and that nothing on the extraction path
/// has to resolve a face for a width to exist.
/// </para>
/// </remarks>
public sealed class SheetColumnDigitsTests
{
    private const string Namespace = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

    /// <summary>Ten-point Liberation Sans, whose digits are 1139/2048 of an em.</summary>
    private static readonly SheetDefaultFont CalcDefault =
        new("Liberation Sans", Length.FromPoints(10));

    [Fact]
    public void AWidthIsTheDigitCountTimesTheFontsDigit()
    {
        SheetDigitWidth width = new(20.76);

        // sheet-ooxml-features.xlsx writes width="20.76", and LibreOffice's rendering of it puts
        // the columns 2304 twips apart — 20.76 × 111. The same column in a twelve-point Arial
        // workbook is 20.76 × 133.
        width.At(111).Twips.ShouldBe(2304);
        width.At(133).Twips.ShouldBe(2761);
    }

    [Fact]
    public void TheFixedPartOfAWidthDoesNotScaleWithTheFont()
    {
        // baseColWidth carries five screen pixels of padding — 75 twips — which `#i3006#` adds in
        // digits and multiplies back, so in twips it is the five pixels whatever the digit is
        // worth. Scaling it with the font would widen every default column by a fifth of the
        // padding for every fifth the font grows.
        SheetDigitWidth padded = new(8, 75.5);

        padded.At(111).Twips.ShouldBe(963);
        padded.At(133).Twips.ShouldBe(1139);
    }

    [Fact]
    public void AFixedWidthCarriesNoDigitsAndSoIgnoresTheFont()
    {
        // Calc's own 64-point standard column, which a BIFF file with no DEFCOLWIDTH falls back
        // to. It is a length in LibreOffice too, so remeasuring it in the workbook's font would
        // be wrong rather than more accurate.
        SheetDigitWidth fixedWidth = SheetDigitWidth.Fixed(SheetGrid.StandardColumnWidth);

        fixedWidth.At(111).Twips.ShouldBe(1280);
        fixedWidth.At(133).Twips.ShouldBe(1280);
    }

    [Fact]
    public void TheGridStatesTheDigitsAsWellAsAWidth()
    {
        (_, SheetGrid grid) = Read("<cols><col min=\"1\" max=\"1\" width=\"20.76\"/></cols>");

        // The reader still materialises a grid, so nothing that never resolves a font is left
        // without one — and it carries the digits beside it so that layout can measure again.
        grid.Columns.SizeAt(0).Twips.ShouldBe(2304);
        grid.ColumnDigits.ShouldNotBeNull();
        grid.ColumnDigits.Runs[0].Width.Digits.ShouldBe(20.76);
    }

    [Fact]
    public void RemeasuringInATwelvePointFontWidensEveryColumn()
    {
        (_, SheetGrid grid) = Read("<cols><col min=\"1\" max=\"1\" width=\"20.76\"/></cols>");

        SheetGrid wider = grid.WithDigitWidth(133);

        wider.Columns.SizeAt(0).Twips.ShouldBe(2761);

        // And the original is untouched, because a grid is a value.
        grid.Columns.SizeAt(0).Twips.ShouldBe(2304);
    }

    [Fact]
    public void AGridWhoseWidthsAreAlreadyLengthsIsNotRemeasured()
    {
        // ODF states a real length on every table:table-column, so there is nothing to resolve
        // and asking must not change it. A caller should not have to know which format a sheet
        // came from before deciding whether to ask.
        SheetGrid ods = new(
            new SheetAxis(Length.FromTwips(1500)), new SheetAxis(SheetGrid.StandardRowHeight));

        ods.WithDigitWidth(133).Columns.DefaultSize.Twips.ShouldBe(1500);
    }

    [Fact]
    public void ASheetsGeometryIsMeasuredInTheWorkbooksOwnDefaultFont()
    {
        (_, SheetGrid stated) = Read(
            "<cols><col min=\"1\" max=\"1\" width=\"20.76\"/></cols>",
            new SheetDefaultFont("Liberation Sans", Length.FromPoints(12)));

        SheetLayout sheet = new() { Name = "S", Grid = stated };

        // Twelve-point Liberation Sans has a 133-twip digit against ten point's 111, so the same
        // column is a fifth wider. Reading `Grid` is what resolves the face: the reader could not,
        // because reading a workbook is the extraction path.
        sheet.Grid.Columns.SizeAt(0).Twips.ShouldBe(2761);
    }

    [Fact]
    public void AWorkbookNamingNoFontKeepsCalcsOwnTenPointFace()
    {
        (_, SheetGrid stated) = Read(
            "<cols><col min=\"1\" max=\"1\" width=\"20.76\"/></cols>", CalcDefault);

        SheetLayout sheet = new() { Name = "S", Grid = stated };

        sheet.Grid.Columns.SizeAt(0).Twips.ShouldBe(2304);
    }

    [Theory]
    [InlineData(10, 111)]
    [InlineData(11, 122)]
    [InlineData(12, 133)]
    public void ADigitIsMeasuredFromTheFacesOwnMetrics(double points, int twips)
    {
        // 1139 units of a 2048-unit em: 111.23, 122.35 and 133.48 twips, which LibreOffice's own
        // device reports as 111, 122 and 133. Round-tripping one-column probe workbooks through
        // LibreOffice 24.2.7.2 and reading the style:column-width it wrote gives those three.
        SheetFonts.DigitWidthTwips(new SheetDefaultFont("Liberation Sans", Length.FromPoints(points)))
                  .ShouldBe(twips);
    }

    /// <summary>
    /// A digit width is neither truncated nor rounded, and the two faces that say so straddle
    /// the carry from opposite sides.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Carlito is 1038/2048 of an em, so eleven point is 111.50 twips exactly and twelve point
    /// 121.64 — LibreOffice writes 111 and 122, so neither truncating nor rounding is right on
    /// its own, and Carlito is the default of 65 of the 171 corpus spreadsheets. DejaVu Sans is
    /// 1303/2048, so eleven point is 139.97 and twelve point 152.70 — LibreOffice writes 140 and
    /// 153.
    /// </para>
    /// <para>
    /// <strong>Carlito 12 was 121 and is now 122, because the reference binary moved.</strong>
    /// These figures were read out of the <c>style:column-width</c> LibreOffice wrote for a
    /// one-column probe workbook; taken against 24.2.7.2 the answer was 121, and taken against
    /// the installed 26.2.4.2 — here, off the filled cell's rectangle in its own PDF export, and
    /// independently off a flat-ODF export, which agree — it is 122. That single twip is
    /// <c>sectors-defense-and-aerospace.xlsx</c>: 40 columns wide, it is 2 pt per column, and it
    /// decides whether two columns fit an A4 page or one does. 227 pages against 449.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("Carlito", 11, 111)]
    [InlineData("Carlito", 12, 122)]
    [InlineData("DejaVu Sans", 11, 140)]
    [InlineData("DejaVu Sans", 12, 153)]
    public void ADigitWidthIsNeitherTruncatedNorRounded(string family, double points, int twips)
    {
        SheetFonts.DigitWidthTwips(new SheetDefaultFont(family, Length.FromPoints(points)))
                  .ShouldBe(twips);
    }

    /// <summary>
    /// The carry threshold sits inside the window every corpus default font requires of it.
    /// </summary>
    /// <remarks>
    /// The constant is fitted rather than derived — see <c>SheetFonts.DigitWidthCarry</c> — so
    /// what pins it is the set of configurations it has to satisfy at once, taken from the
    /// default font of all 171 corpus spreadsheets and measured one probe workbook at a time
    /// through the installed 26.2.4.2. Carlito 11 is the tightest constraint from below
    /// (111.5039 must truncate) and Carlito 12 the tightest from above (121.6406 must carry),
    /// which leaves only <c>0.5039 &lt;= c &lt; 0.6406</c>. Rounding half up scores better on a
    /// uniform sweep of sizes and is ruled out by the first of those: it would take 65 documents'
    /// default font to 112.
    /// </remarks>
    [Theory]
    [InlineData("Liberation Sans", 10, 111)]     // 111.2305, truncates
    [InlineData("Liberation Sans", 11, 122)]     // 122.3535, truncates
    [InlineData("Liberation Sans", 12, 133)]     // 133.4766, truncates
    [InlineData("Liberation Serif", 10, 100)]    // 100.0000, exact
    [InlineData("DejaVu Sans", 10, 127)]         // 127.2461, truncates
    public void EveryCorpusDefaultFontAgreesWithTheReference(
        string family, double points, int twips)
    {
        SheetFonts.DigitWidthTwips(new SheetDefaultFont(family, Length.FromPoints(points)))
                  .ShouldBe(twips);
    }

    [Fact]
    public void AnUnreadableFontFallsBackRatherThanCollapsingTheSheet()
    {
        // A face that will not resolve leaves the widths where they were rather than at zero:
        // a page whose columns are all nothing is one page per column, which is worse than a
        // page measured in the wrong font.
        SheetFonts.DigitWidthTwips(new SheetDefaultFont("Liberation Sans", Length.Zero))
                  .ShouldBe(SheetColumnDigits.FallbackDigitWidthTwips);
    }

    [Fact]
    public void ABiffDefaultColumnCarriesExcelsFontDependentPadding()
    {
        // `#i3006#`: `ImportExcel::DefColWidth` (impop.cxx:640) adds
        // 40960 / max(fontHeight - 15, 60) + 50 in 256ths of a digit before converting, because
        // "Excel adds space depending on font size". Twelve-point Calibri is 240 twips, so the
        // correction is 40960/225 + 50 = 232.04, and ten characters becomes 2792/256 digits.
        // At Carlito 12's 122-twip digit that is 1330 twips, which is what LibreOffice's own
        // flat-ODF export of `aircraft_analysis_2016-04-27.xls` states — `0.9236in`, re-measured
        // against the installed 26.2.4.2. Ten digits alone give 1220, nine per cent narrow.
        //
        // This figure was 1319 when it was taken against 24.2.7.2, whose digit width for the
        // same face was 121. It moved with the binary, not with this reader, and it is a second
        // and independent witness to that move: a BIFF default column width reaches the digit
        // width by a different path from a SpreadsheetML `<col width>`, and both changed by the
        // same one twip.
        XlsSheetPrintState state = new()
        {
            DefaultFont = new SheetDefaultFont("Carlito", Length.FromPoints(12)),
        };
        state.SetDefaultColumnWidth(10);

        SheetLayout sheet = new() { Name = "S", Grid = state.ToGrid() };

        sheet.Grid.Columns.DefaultSize.Twips.ShouldBe(1330);
    }

    [Fact]
    public void ABiffSheetWithNoDefaultColumnWidthKeepsCalcsOwnStandardColumn()
    {
        // 64 points, and a length rather than a count of digits — so remeasuring it in the
        // workbook's font would be wrong rather than more accurate.
        XlsSheetPrintState state = new()
        {
            DefaultFont = new SheetDefaultFont("Carlito", Length.FromPoints(12)),
        };

        SheetLayout sheet = new() { Name = "S", Grid = state.ToGrid() };

        sheet.Grid.Columns.DefaultSize.ShouldBe(SheetGrid.StandardColumnWidth);
    }

    private static (SheetPrintSetup Setup, SheetGrid Grid) Read(
        string body, SheetDefaultFont? font = null)
        => XlsxPrintSetup.Read(
            XElement.Parse($"<worksheet xmlns=\"{Namespace}\">{body}</worksheet>"),
            [], null, null, font);
}
