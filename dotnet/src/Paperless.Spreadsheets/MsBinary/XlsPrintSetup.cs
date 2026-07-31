using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.MsBinary;

/// <summary>The BIFF record identifiers that carry a sheet's print setup and geometry.</summary>
/// <remarks>
/// Named and numbered as LibreOffice names them in <c>sc/source/filter/inc/xlpage.hxx</c>,
/// <c>xltable.hxx</c>, <c>xlconst.hxx</c> and <c>xlname.hxx</c>.
/// </remarks>
internal static class BiffPageRecords
{
    public const ushort Header = 0x0014;
    public const ushort Footer = 0x0015;
    public const ushort VerticalPageBreaks = 0x001A;
    public const ushort HorizontalPageBreaks = 0x001B;
    public const ushort LeftMargin = 0x0026;
    public const ushort RightMargin = 0x0027;
    public const ushort TopMargin = 0x0028;
    public const ushort BottomMargin = 0x0029;
    public const ushort PrintHeaders = 0x002A;
    public const ushort PrintGridLines = 0x002B;
    public const ushort DefColWidth = 0x0055;
    public const ushort ColInfo = 0x007D;
    public const ushort WsBool = 0x0081;
    public const ushort HorizontalCentre = 0x0083;
    public const ushort VerticalCentre = 0x0084;
    public const ushort Setup = 0x00A1;
    public const ushort Row = 0x0208;
    public const ushort DefaultRowHeight = 0x0225;
    public const ushort Name = 0x0018;

    /// <summary>Fit-to-pages lives in <c>WSBOOL</c>, not in <c>SETUP</c>.</summary>
    /// <remarks>
    /// <c>EXC_WSBOOL_FITTOPAGE</c>. LibreOffice calls the separation out as odd — "for whatever
    /// reason, this flag is still stored separated from the page settings",
    /// <c>sc/source/filter/oox/worksheetsettings.cxx:194</c> — and it is the same trap the OOXML
    /// path has: <c>SETUP</c>'s <c>fitToWidth</c> and <c>fitToHeight</c> are written as 1 in
    /// every file whether or not anything is fitting to them.
    /// </remarks>
    public const ushort WsBoolFitToPage = 0x0100;

    /// <summary>The <c>SETUP</c> flags, from <c>EXC_SETUP_*</c>.</summary>
    public const ushort SetupInRows = 0x0001;

    /// <summary>Set when the paper size and orientation are the printer's rather than the file's.</summary>
    public const ushort SetupInvalid = 0x0004;

    /// <summary>Set when <c>SETUP</c>'s start page is meant rather than the sheet continuing.</summary>
    public const ushort SetupStartPage = 0x0080;

    /// <summary>Portrait, when <see cref="SetupInvalid"/> is clear.</summary>
    public const ushort SetupPortrait = 0x0002;

    /// <summary>A built-in name's code for the sheet's print area.</summary>
    public const byte BuiltInPrintArea = 0x06;

    /// <summary>A built-in name's code for its repeated rows and columns.</summary>
    public const byte BuiltInPrintTitles = 0x07;

    /// <summary>The <c>NAME</c> flag marking a built-in name.</summary>
    public const ushort NameBuiltIn = 0x0020;
}

/// <summary>
/// Accumulates one sheet's print setup as its records go by.
/// </summary>
/// <remarks>
/// <para>
/// A BIFF sheet states its page geometry across a dozen small records rather than in one
/// structure, and half of them are optional. So this starts from Excel's own defaults — 1.9 cm
/// sides, 2.5 cm top and bottom, 1.3 cm to the header and footer
/// (<c>EXC_MARGIN_DEFAULT_*</c>, <c>sc/source/filter/inc/xlpage.hxx:55-57</c>) — and lets each
/// record it meets override one of them, which is what <c>XclImpPageSettings</c> does.
/// </para>
/// <para>
/// The header band is derived the same way the OOXML one is, and for the same reason: Calc
/// measures its top margin to the header and BIFF measures it to the first row, so with a
/// header present the band is <c>TOPMARGIN - SETUP.headerMargin</c> and the top margin becomes
/// the header's (<c>XclImpPageSettings::Finalize</c>,
/// <c>sc/source/filter/excel/xipage.cxx:296-315</c>).
/// </para>
/// </remarks>
internal sealed class XlsSheetPrintState
{
    private const double DefaultSideMarginMm100 = 1900;
    private const double DefaultEndMarginMm100 = 2500;
    private const double DefaultBandMarginMm100 = 1300;

    /// <summary>
    /// The advance of the widest digit of the workbook's default font, in twips.
    /// </summary>
    /// <remarks>
    /// BIFF states a column's width in 256ths of a character, so a length only exists once this
    /// is known. LibreOffice measures it from the workbook's own default font and falls back to
    /// 110 twips when it has no device to measure with (<c>XclRoot</c>'s <c>mnCharWidth</c>,
    /// <c>sc/source/filter/excel/xlroot.cxx:107</c>); 111 is what 10-point Liberation Sans
    /// measures, which is the font LibreOffice writes these files in. Reading the
    /// <c>FONT</c> table and measuring properly is on the module's TODO.
    /// </remarks>
    private const double DigitWidthTwips = 111;

    private double _leftMargin = DefaultSideMarginMm100 / 2540;
    private double _rightMargin = DefaultSideMarginMm100 / 2540;
    private double _topMargin = DefaultEndMarginMm100 / 2540;
    private double _bottomMargin = DefaultEndMarginMm100 / 2540;
    private double _headerMargin = DefaultBandMarginMm100 / 2540;
    private double _footerMargin = DefaultBandMarginMm100 / 2540;

    private readonly List<SheetSizeRun> _columns = [];
    private readonly List<SheetSizeRun> _rows = [];
    private readonly List<int> _columnBreaks = [];
    private readonly List<int> _rowBreaks = [];

    private Length _defaultColumnWidth = SheetGrid.StandardColumnWidth;
    private Length _defaultRowHeight = SheetGrid.StandardRowHeight;

    private int _paperSize = 1;
    private int _scale = 100;
    private int _startPage;
    private int _fitToWidth = 1;
    private int _fitToHeight = 1;
    private bool _fitsToPages;
    private bool _portrait = true;
    private bool _paperIsValid;
    private bool _usesStartPage;
    private bool _printsInRows;
    private bool _centresHorizontally;
    private bool _centresVertically;
    private bool _printsHeadings;
    private bool _printsGrid;
    private string? _header;
    private string? _footer;

    /// <summary>Records a page margin, in inches as BIFF states it.</summary>
    public void SetMargin(ushort record, double inches)
    {
        switch (record)
        {
            case BiffPageRecords.LeftMargin: _leftMargin = inches; break;
            case BiffPageRecords.RightMargin: _rightMargin = inches; break;
            case BiffPageRecords.TopMargin: _topMargin = inches; break;
            case BiffPageRecords.BottomMargin: _bottomMargin = inches; break;
            default: break;
        }
    }

    /// <summary>Records the <c>SETUP</c> record's fields.</summary>
    public void SetSetup(
        int paperSize,
        int scale,
        int startPage,
        int fitToWidth,
        int fitToHeight,
        ushort flags,
        double? headerMargin,
        double? footerMargin)
    {
        _paperSize = paperSize;
        _scale = scale > 0 ? scale : 100;
        _startPage = startPage;
        _fitToWidth = fitToWidth;
        _fitToHeight = fitToHeight;
        _printsInRows = (flags & BiffPageRecords.SetupInRows) != 0;
        _portrait = (flags & BiffPageRecords.SetupPortrait) != 0;

        // A SETUP whose "invalid" bit is set means the paper size and orientation came from a
        // printer rather than from the document, so neither is used.
        _paperIsValid = (flags & BiffPageRecords.SetupInvalid) == 0;
        _usesStartPage = (flags & BiffPageRecords.SetupStartPage) != 0;

        if (headerMargin is { } header) _headerMargin = header;
        if (footerMargin is { } footer) _footerMargin = footer;
    }

    /// <summary>Records the <c>WSBOOL</c> fit-to-pages flag.</summary>
    public void SetFitsToPages(bool fits) => _fitsToPages = fits;

    /// <summary>Records a header or footer string, in Excel's own field syntax.</summary>
    public void SetFurniture(ushort record, string text)
    {
        if (record == BiffPageRecords.Header) _header = text;
        else _footer = text;
    }

    /// <summary>Records the centring, headings and grid flags.</summary>
    public void SetFlag(ushort record, bool value)
    {
        switch (record)
        {
            case BiffPageRecords.HorizontalCentre: _centresHorizontally = value; break;
            case BiffPageRecords.VerticalCentre: _centresVertically = value; break;
            case BiffPageRecords.PrintHeaders: _printsHeadings = value; break;
            case BiffPageRecords.PrintGridLines: _printsGrid = value; break;
            default: break;
        }
    }

    /// <summary>Records the manual page breaks a break record lists.</summary>
    public void AddBreaks(ushort record, IEnumerable<int> positions)
    {
        List<int> target = record == BiffPageRecords.VerticalPageBreaks ? _columnBreaks : _rowBreaks;
        foreach (int at in positions)
        {
            if (at > 0) target.Add(at);
        }
    }

    /// <summary>Records <c>DEFCOLWIDTH</c>, which is stated in whole characters.</summary>
    public void SetDefaultColumnWidth(int characters)
    {
        if (characters > 0) _defaultColumnWidth = FromCharacterWidth(characters * 256);
    }

    /// <summary>Records <c>DEFAULTROWHEIGHT</c>, which is stated in twips.</summary>
    public void SetDefaultRowHeight(int twips)
    {
        if (twips > 0) _defaultRowHeight = Length.FromTwips(twips);
    }

    /// <summary>Records a <c>COLINFO</c>, whose width is in 256ths of a character.</summary>
    public void AddColumns(int first, int last, int width, bool hidden)
    {
        if (last < first) return;

        _columns.Add(new SheetSizeRun(
            first,
            Math.Min(last, SheetAddress.MaxColumn),
            width > 0 ? FromCharacterWidth(width) : _defaultColumnWidth,
            hidden));
    }

    /// <summary>Records a <c>ROW</c>, whose height is in twips.</summary>
    public void AddRow(int row, int twips, bool hidden)
    {
        if (row < 0) return;

        _rows.Add(new SheetSizeRun(
            row, row, twips > 0 ? Length.FromTwips(twips) : _defaultRowHeight, hidden));
    }

    /// <summary>The sheet's print areas, from its <c>Print_Area</c> built-in name.</summary>
    public List<SheetRange> PrintAreas { get; } = [];

    /// <summary>The columns repeated on every page, from <c>Print_Titles</c>.</summary>
    public SheetRange? RepeatColumns { get; set; }

    /// <summary>The rows repeated on every page.</summary>
    public SheetRange? RepeatRows { get; set; }

    /// <summary>The accumulated setup, resolved.</summary>
    public SheetPrintSetup ToSetup()
    {
        bool hasHeader = !string.IsNullOrEmpty(_header);
        bool hasFooter = !string.IsNullOrEmpty(_footer);

        (Length paperWidth, Length paperHeight) = _paperIsValid
            ? ExcelPaperSizes.Portrait(_paperSize)
            : ExcelPaperSizes.A4;

        bool landscape = _paperIsValid && !_portrait;
        DocSize page = landscape
            ? new DocSize(paperHeight, paperWidth)
            : new DocSize(paperWidth, paperHeight);

        return new SheetPrintSetup
        {
            PageSize = page,
            IsLandscape = landscape,
            LeftMargin = Length.FromInches(_leftMargin),
            RightMargin = Length.FromInches(_rightMargin),
            TopMargin = Length.FromInches(hasHeader ? _headerMargin : _topMargin),
            BottomMargin = Length.FromInches(hasFooter ? _footerMargin : _bottomMargin),
            HeaderHeight = hasHeader
                ? Length.FromInches(Math.Max(0, _topMargin - _headerMargin))
                : Length.Zero,
            FooterHeight = hasFooter
                ? Length.FromInches(Math.Max(0, _bottomMargin - _footerMargin))
                : Length.Zero,
            HeaderText = _header,
            FooterText = _footer,
            ScaleMode = _fitsToPages ? PrintScaleMode.FitToPages : PrintScaleMode.Percentage,
            ScalePercentage = _scale,
            FitToPagesWide = Math.Max(0, _fitToWidth),
            FitToPagesTall = Math.Max(0, _fitToHeight),
            PageOrder = _printsInRows ? PagePrintOrder.AcrossThenDown : PagePrintOrder.DownThenAcross,
            PrintsGrid = _printsGrid,
            PrintsHeadings = _printsHeadings,
            CentresHorizontally = _centresHorizontally,
            CentresVertically = _centresVertically,
            FirstPageNumber = _usesStartPage ? _startPage : 0,
            PrintAreas = PrintAreas,
            RepeatColumns = RepeatColumns,
            RepeatRows = RepeatRows,
            ManualColumnBreaks = _columnBreaks,
            ManualRowBreaks = _rowBreaks,
        };
    }

    /// <summary>The sheet's column widths and row heights.</summary>
    public SheetGrid ToGrid() => new(
        new SheetAxis(_defaultColumnWidth, _columns),
        new SheetAxis(_defaultRowHeight, _rows));

    /// <summary>
    /// Turns a BIFF column width into twips.
    /// </summary>
    /// <remarks>
    /// <c>XclTools::GetScColumnWidth</c> (<c>sc/source/filter/excel/xltools.cxx:304</c>) — the
    /// half-twip taken off before truncating is not rounding, it is a deliberate bias, and it is
    /// what makes an eight-character column come out at the width Excel shows rather than one
    /// twip wider.
    /// </remarks>
    private static Length FromCharacterWidth(int width)
        => Length.FromTwips((long)((width / 256.0 * DigitWidthTwips) - 0.5));
}
