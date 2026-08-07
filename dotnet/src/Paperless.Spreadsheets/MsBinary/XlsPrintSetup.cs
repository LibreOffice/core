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

    /// <summary>Set when <c>SETUP</c>'s values are the printer's rather than the file's.</summary>
    /// <remarks>
    /// <c>EXC_SETUP_INVALID</c>. It disqualifies the scale as well as the paper size and the
    /// orientation — <c>mbValidPaper = maData.mbValid = !get_flag(nFlags, EXC_SETUP_INVALID)</c>
    /// (<c>sc/source/filter/excel/xipage.cxx:68</c>) sets one flag for all three.
    /// </remarks>
    public const ushort SetupInvalid = 0x0004;

    /// <summary>Set when <c>SETUP</c>'s start page is meant rather than the sheet continuing.</summary>
    public const ushort SetupStartPage = 0x0080;

    /// <summary>
    /// Excel's "Comments: at end of sheet": the cell notes are listed after the sheet.
    /// </summary>
    /// <remarks>
    /// <c>EXC_SETUP_PRINTNOTES</c> (<c>sc/source/filter/inc/xlpage.hxx:89</c>), read only from
    /// BIFF5 — <c>XclImpPageSettings::ReadSetup</c> takes it inside the same
    /// <c>GetBiff() &gt;= EXC_BIFF5</c> branch as the draft-quality and start-page flags
    /// (<c>xipage.cxx:75-86</c>).
    /// </remarks>
    public const ushort SetupPrintNotes = 0x0020;

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
    /// The half twip <c>XclTools::GetScColumnWidth</c> takes off before truncating.
    /// </summary>
    /// <remarks>
    /// Not rounding, a deliberate bias (<c>sc/source/filter/excel/xltools.cxx:304</c>): it is
    /// what makes an eight-character column come out at the width Excel shows rather than one
    /// twip wider. Carried as <see cref="SheetDigitWidth.BiasTwips"/> because it does not scale
    /// with the font.
    /// </remarks>
    private const double BiasTwips = -0.5;

    private double _leftMargin = DefaultSideMarginMm100 / 2540;
    private double _rightMargin = DefaultSideMarginMm100 / 2540;
    private double _topMargin = DefaultEndMarginMm100 / 2540;
    private double _bottomMargin = DefaultEndMarginMm100 / 2540;
    private double _headerMargin = DefaultBandMarginMm100 / 2540;
    private double _footerMargin = DefaultBandMarginMm100 / 2540;

    private readonly List<SheetDigitRun> _columns = [];
    private readonly List<SheetSizeRun> _rows = [];
    private readonly List<int> _columnBreaks = [];
    private readonly List<int> _rowBreaks = [];

    private SheetDigitWidth _defaultColumnWidth = SheetDigitWidth.Fixed(SheetGrid.StandardColumnWidth);
    private Length _defaultRowHeight = SheetGrid.StandardRowHeight;

    private int _paperSize = 1;
    private int _scale = 100;
    private int _startPage;
    private int _fitToWidth = 1;
    private int _fitToHeight = 1;
    private bool _fitsToPages;
    private bool _portrait = true;
    private bool _setupIsValid;
    private bool _usesStartPage;
    private bool _printsInRows;
    private bool _centresHorizontally;
    private bool _centresVertically;
    private bool _printsHeadings;
    private bool _printsGrid;
    private bool _printsNotes;
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

        // A SETUP whose "invalid" bit is set carries values that came from a printer rather
        // than from the document, so none of them is used — the paper size, the orientation
        // and, less obviously, the scale.
        _setupIsValid = (flags & BiffPageRecords.SetupInvalid) == 0;
        _usesStartPage = (flags & BiffPageRecords.SetupStartPage) != 0;
        _printsNotes = (flags & BiffPageRecords.SetupPrintNotes) != 0;

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

    /// <summary>
    /// Records <c>DEFCOLWIDTH</c>, which is stated in whole characters and carries a correction.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The correction is <c>#i3006#</c>'s and depends on the default font's size</strong>,
    /// which is why it could not be applied until that font was read.
    /// <c>ImportExcel::DefColWidth</c> (<c>sc/source/filter/excel/impop.cxx:640-657</c>) adds
    /// <see cref="DefaultWidthCorrection"/> in 256ths of a digit before converting, with the
    /// comment "additional space for default width — Excel adds space depending on font size".
    /// It is the same five screen pixels SpreadsheetML's <c>baseColWidth</c> carries, expressed
    /// the way BIFF needs it: a count of digit-widths rather than a length, because BIFF's own
    /// conversion multiplies by the digit afterwards.
    /// </para>
    /// <para>
    /// Worth 110 twips on a twelve-point Calibri sheet, which is 9% of the column.
    /// <c>aircraft_analysis_2016-04-27.xls</c> states <c>DEFCOLWIDTH</c> 10 in twelve-point
    /// Calibri, and LibreOffice's own flat-ODF export of it puts the default column at 1319
    /// twips against the 1209 ten digits alone give.
    /// </para>
    /// </remarks>
    /// <param name="characters">The width in whole characters, as the record states it.</param>
    public void SetDefaultColumnWidth(int characters)
    {
        if (characters <= 0) return;

        double units = (characters * 256.0) + DefaultWidthCorrection(
            DefaultFont?.Size ?? Length.FromPoints(10));

        _defaultColumnWidth = FromCharacterWidth((int)Math.Clamp(units, 0, ushort.MaxValue));
    }

    /// <summary>
    /// Excel's five-pixel padding on a default column, in 256ths of a digit.
    /// </summary>
    /// <remarks>
    /// <c>XclTools::GetXclDefColWidthCorrection</c>
    /// (<c>sc/source/filter/excel/xltools.cxx:318-343</c>), whose own comment reconstructs it as
    /// <c>5 × 256 × 1440 × 2.1333 / (96 × max(N−15, 60)) + 50</c> — five pixels, 256ths of a
    /// digit, twips to inches, an empirical quotient turning a font <em>height</em> into a digit
    /// <em>width</em>, and 96 DPI — and admits the 15, the 60 and the 50 are of unknown origin.
    /// Reproduced as written rather than rederived, because the constants are what a file was
    /// authored against.
    /// </remarks>
    /// <param name="fontHeight">The default font's em size.</param>
    private static double DefaultWidthCorrection(Length fontHeight)
        => (40960.0 / Math.Max(fontHeight.Twips - 15, 60)) + 50.0;

    /// <summary>Records <c>DEFAULTROWHEIGHT</c>, which is stated in twips.</summary>
    /// <param name="twips">The default height.</param>
    /// <param name="manual">
    /// The record's <c>fUnsynced</c>, which makes every row of the sheet a user's choice rather
    /// than a writer's measurement — see <see cref="SheetGrid.RowHeightsAreManual"/>.
    /// </param>
    public void SetDefaultRowHeight(int twips, bool manual = false)
    {
        if (twips > 0) _defaultRowHeight = Length.FromTwips(twips);
        if (manual) RowHeightsAreManual = true;
    }

    /// <summary>
    /// True when no row of this sheet is to be re-measured from its content.
    /// </summary>
    /// <remarks>
    /// Set from <c>DEFAULTROWHEIGHT</c>'s own <c>fUnsynced</c>, and set outright for BIFF8 —
    /// see <see cref="SheetGrid.RowHeightsAreManual"/> for why the version decides it.
    /// </remarks>
    public bool RowHeightsAreManual { get; set; }

    /// <summary>Records a <c>COLINFO</c>, whose width is in 256ths of a character.</summary>
    public void AddColumns(int first, int last, int width, bool hidden)
    {
        if (last < first) return;

        _columns.Add(new SheetDigitRun(
            first,
            Math.Min(last, SheetAddress.MaxColumn),
            width > 0 ? FromCharacterWidth(width) : _defaultColumnWidth,
            hidden));
    }

    /// <summary>Records a <c>ROW</c>, whose height is in twips.</summary>
    /// <param name="row">The zero-based row.</param>
    /// <param name="twips">Its height.</param>
    /// <param name="hidden">Whether the row is hidden.</param>
    /// <param name="manualHeight">
    /// <c>fUnsynced</c>: the height was set by hand and is not to be recomputed.
    /// </param>
    public void AddRow(int row, int twips, bool hidden, bool manualHeight = true)
    {
        if (row < 0) return;

        _rows.Add(new SheetSizeRun(
            row, row, twips > 0 ? Length.FromTwips(twips) : _defaultRowHeight, hidden,
            !manualHeight));
    }

    /// <summary>The sheet's print areas, from its <c>Print_Area</c> built-in name.</summary>
    public List<SheetRange> PrintAreas { get; } = [];

    /// <summary>The columns repeated on every page, from <c>Print_Titles</c>.</summary>
    public SheetRange? RepeatColumns { get; set; }

    /// <summary>The rows repeated on every page.</summary>
    public SheetRange? RepeatRows { get; set; }

    /// <summary>
    /// The height of a header or footer band, floored at Calc's own default.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The floor is the whole point and it is BIFF-only. The BIFF filter never puts an
    /// <c>ATTR_PAGE_SIZE</c> on the header's item set: it sets the band <em>dynamic</em> and
    /// writes only the distance to the body (<c>XclImpPageSettings::Finalize</c>,
    /// <c>sc/source/filter/excel/xipage.cxx:310-331</c>). So <c>nManHeight</c> — the minimum
    /// <c>UpdateHFHeight</c> will not go below — stays at whatever a fresh page style has, which
    /// is 0.5 cm of text plus a 0.25 cm gap (<c>ScStyleSheet::GetItemSet</c>,
    /// <c>sc/source/core/data/stlsheet.cxx:184</c>). The OOXML filter is different: it writes an
    /// explicit height, so its band can be shorter than this.
    /// </para>
    /// <para>
    /// Measured, that is four points a page. <c>sheet-decor-xls.xls</c> states a 1.025 in top
    /// margin and a 0.7875 in header margin — 17.1 pt between them — and LibreOffice still puts
    /// the first printed row 21.11 pt below the top margin, because 0.75 cm is 21.26 pt and
    /// wins.
    /// </para>
    /// </remarks>
    private static Length Band(double inches)
    {
        Length stated = Length.FromInches(Math.Max(0, inches));
        return Length.Max(stated, DefaultBandHeight);
    }

    /// <summary>Calc's default header and footer band: 0.5 cm of text and a 0.25 cm gap.</summary>
    private static readonly Length DefaultBandHeight = Length.FromTwips(425);

    /// <summary>The accumulated setup, resolved.</summary>
    public SheetPrintSetup ToSetup()
    {
        bool hasHeader = !string.IsNullOrEmpty(_header);
        bool hasFooter = !string.IsNullOrEmpty(_footer);

        (Length paperWidth, Length paperHeight) = _setupIsValid
            ? ExcelPaperSizes.Portrait(_paperSize)
            : ExcelPaperSizes.A4;

        bool landscape = _setupIsValid && !_portrait;
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
            HeaderHeight = hasHeader ? Band(_topMargin - _headerMargin) : Length.Zero,
            FooterHeight = hasFooter ? Band(_bottomMargin - _footerMargin) : Length.Zero,
            HeaderText = _header,
            FooterText = _footer,

            // BIFF's HEADER and FOOTER records carry the same &-code language SpreadsheetML
            // does, which is not a coincidence: the OOXML spelling was inherited from it.
            Header = _header is null ? null : SheetHeaderFooter.ParseCodes(_header),
            Footer = _footer is null ? null : SheetHeaderFooter.ParseCodes(_footer),
            ScaleMode = _fitsToPages ? PrintScaleMode.FitToPages : PrintScaleMode.Percentage,

            // The scale is SETUP's, and SETUP's fields are only meant when the record is valid.
            // XclImpPageSettings::Finalize puts ATTR_PAGE_SCALE under `else if (maData.mbValid)`
            // (sc/source/filter/excel/xipage.cxx:274-276), so a SETUP marked "these came from a
            // printer" leaves the page style's own 100%. It is not a corner case: a quarter of
            // the corpus's .xls files set the bit, and the scale beside it is arbitrary —
            // 255, 285, 300, once 20480 — so honouring it multiplies the whole sheet and turns
            // a two-page workbook into a twelve-page one. Fit-to-pages is unaffected: it comes
            // from WSBOOL and Finalize applies it whether or not SETUP is valid.
            ScalePercentage = _setupIsValid ? _scale : 100,
            FitToPagesWide = Math.Max(0, _fitToWidth),
            FitToPagesTall = Math.Max(0, _fitToHeight),
            PageOrder = _printsInRows ? PagePrintOrder.AcrossThenDown : PagePrintOrder.DownThenAcross,
            PrintsGrid = _printsGrid,
            PrintsHeadings = _printsHeadings,
            PrintsNotes = _printsNotes,
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

    /// <summary>
    /// The workbook's default font, which a column width is a count of digits of.
    /// </summary>
    /// <remarks>
    /// BIFF's "app font" — <c>FONT</c> record zero, which
    /// <c>XclImpFontBuffer::UpdateAppFont</c> (<c>sc/source/filter/excel/xistyle.cxx:632</c>)
    /// hands straight to <c>SetCharWidth</c>. Set from the workbook globals, which are read before
    /// any sheet; null leaves the widths on Calc's own ten-point face.
    /// </remarks>
    public SheetDefaultFont? DefaultFont { get; init; }

    /// <summary>The sheet's column widths and row heights.</summary>
    /// <remarks>
    /// The columns are still stated in digits of <see cref="DefaultFont"/> and are materialised at
    /// the fallback digit width here; <see cref="SheetLayout.Grid"/> remeasures them once it can
    /// resolve a face. See <see cref="SheetColumnDigits"/>.
    /// </remarks>
    public SheetGrid ToGrid()
    {
        SheetColumnDigits digits = new(
            DefaultFont ?? SheetDefaultFont.Calc, _defaultColumnWidth, _columns);

        return new SheetGrid(
            digits.Resolve(SheetColumnDigits.FallbackDigitWidthTwips),
            new SheetAxis(_defaultRowHeight, _rows))
        {
            ColumnDigits = digits,
            RowHeightsAreManual = RowHeightsAreManual,
        };
    }

    /// <summary>
    /// Turns a BIFF column width, in 256ths of a character, into a count of digits.
    /// </summary>
    /// <remarks>
    /// <c>XclTools::GetScColumnWidth</c> (<c>sc/source/filter/excel/xltools.cxx:304</c>).
    /// </remarks>
    private static SheetDigitWidth FromCharacterWidth(int width)
        => new(width / 256.0, BiasTwips);
}
