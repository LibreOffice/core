using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.Core.Numbers;

namespace Paperless.Spreadsheets.MsBinary;

/// <summary>A <c>FONT</c> record, as far as drawing a cell needs it.</summary>
/// <param name="Name">The family name.</param>
/// <param name="Height">The em size; BIFF states it in twentieths of a point.</param>
/// <param name="Weight">400 or 700 in every file, but the field is a full weight.</param>
/// <param name="IsItalic">Whether the face is italic.</param>
/// <param name="ColourIndex">An index into the workbook's palette, or 0x7FFF for automatic.</param>
/// <param name="Underline">The line under the text, from the record's own underline byte.</param>
/// <param name="IsStruckThrough">Whether a line is drawn through it.</param>
internal readonly record struct BiffFont(
    string Name, Length Height, int Weight, bool IsItalic, int ColourIndex,
    SheetUnderline Underline = SheetUnderline.None, bool IsStruckThrough = false);

/// <summary>A cell format's alignment half, as an <c>XF</c> record states it.</summary>
internal readonly record struct BiffAlignment(
    SheetHorizontalAlignment Horizontal,
    SheetVerticalAlignment Vertical,
    bool Wraps,
    bool Shrinks,
    int IndentLevels,
    int Rotation,
    bool Stacked);

/// <summary>
/// Turns BIFF's <c>FONT</c> and <c>XF</c> records into the formats a cell is drawn in.
/// </summary>
/// <remarks>
/// <para>
/// Ported from <c>XclImpFont::ReadFontData5</c> and <c>XclImpCellAlign::FillFromXF5</c>/
/// <c>FillFromXF8</c> (<c>sc/source/filter/excel/xistyle.cxx:439, 801, 809</c>). Three of BIFF's
/// details are easy to get wrong and each is visible in a rendering.
/// </para>
/// <para>
/// <strong>Font index 4 does not exist.</strong> A workbook's fifth <c>FONT</c> record is font
/// index 5: the gap is historical and every BIFF writer honours it, so a reader that indexes the
/// records directly draws every cell above the fourth font in the wrong face
/// (<c>XclImpFontBuffer::GetFont</c>).
/// </para>
/// <para>
/// <strong>The alignment fields moved between BIFF5 and BIFF8.</strong> Vertical alignment is
/// three bits in both but the rotation is a two-bit <em>orientation</em> at bit 8 in BIFF5 and a
/// full 0–180 degree byte in BIFF8, and indent and shrink-to-fit exist only in BIFF8, in a field
/// BIFF5 does not have at all.
/// </para>
/// <para>
/// <strong>An indent level is 200 twips here and three space widths in SpreadsheetML.</strong>
/// The BIFF filter says so outright — "1 Excel unit == 10 pt == 200 twips",
/// <c>xistyle.cxx:846</c> — while the OOXML filter measures the default font. The same workbook
/// saved both ways therefore indents differently, and both are right.
/// </para>
/// </remarks>
internal sealed class XlsCellFormats
{
    /// <summary>One indent level, in twips.</summary>
    private const int TwipsPerIndentLevel = 200;

    /// <summary>The <c>FONT</c> index that is never used.</summary>
    private const int SkippedFontIndex = 4;

    private readonly List<BiffFont> _fonts = [];
    private readonly List<Colour> _palette = [.. BiffPalette];

    /// <summary>Records a <c>FONT</c>, keeping the gap at index four.</summary>
    /// <param name="font">The font just read.</param>
    public void AddFont(BiffFont font)
    {
        // The list is indexed the way the file indexes it, so the hole is filled with a copy of
        // whatever preceded it rather than being closed up.
        if (_fonts.Count == SkippedFontIndex) _fonts.Add(font);
        _fonts.Add(font);
    }

    /// <summary>
    /// The workbook's "app font", which a column width is a count of digits of.
    /// </summary>
    /// <remarks>
    /// <c>FONT</c> record zero, which <c>XclImpFontBuffer::UpdateAppFont</c> hands straight to
    /// <c>XclRoot::SetCharWidth</c> (<c>sc/source/filter/excel/xistyle.cxx:632</c>). Null before
    /// the workbook globals have been read, which leaves the width on Calc's own default face.
    /// See <see cref="SheetColumnDigits"/>.
    /// </remarks>
    public SheetDefaultFont? DefaultFont => _fonts.Count > 0
        ? new SheetDefaultFont(
            _fonts[0].Name.Length == 0 ? null : _fonts[0].Name,
            _fonts[0].Height,
            _fonts[0].Weight,
            _fonts[0].IsItalic)
        : null;

    /// <summary>Replaces the palette from index eight upwards, which is what <c>PALETTE</c> sets.</summary>
    /// <param name="colours">The colours the record listed, in order.</param>
    public void SetPalette(IReadOnlyList<Colour> colours)
    {
        ArgumentNullException.ThrowIfNull(colours);

        for (int at = 0; at < colours.Count && FirstPaletteIndex + at < _palette.Count; at++)
        {
            _palette[FirstPaletteIndex + at] = colours[at];
        }
    }

    /// <summary>Where a <c>PALETTE</c> record's first colour lands.</summary>
    private const int FirstPaletteIndex = 8;

    /// <summary>Builds one XF's text format.</summary>
    /// <param name="fontIndex">The XF's font index.</param>
    /// <param name="alignment">Its alignment fields.</param>
    /// <param name="format">The number format it resolved to.</param>
    public SheetCellFormat Resolve(int fontIndex, BiffAlignment alignment, NumberFormatCode format)
    {
        ArgumentNullException.ThrowIfNull(format);

        BiffFont font = fontIndex >= 0 && fontIndex < _fonts.Count
            ? _fonts[fontIndex]
            : new BiffFont(string.Empty, Length.FromPoints(10), 400, false, AutomaticColour);

        return new SheetCellFormat
        {
            FontFamily = font.Name.Length == 0 ? null : font.Name,
            FontSize = font.Height,
            FontWeight = font.Weight,
            IsItalic = font.IsItalic,
            Underline = font.Underline,
            IsStruckThrough = font.IsStruckThrough,
            Colour = ColourAt(font.ColourIndex),
            Horizontal = alignment.Horizontal,
            Vertical = alignment.Vertical,
            Wraps = alignment.Wraps,
            ShrinksToFit = alignment.Shrinks,
            Indent = Length.FromTwips((long)alignment.IndentLevels * TwipsPerIndentLevel),
            RotationDegrees = alignment.Rotation,
            IsStacked = alignment.Stacked,
            NumberFormatKind = format.IsGeneral || format.Sections.Count == 0
                ? NumberFormatKind.General
                : format.Sections[0].Kind,
            NumberFormat = format,
        };
    }

    /// <summary>
    /// Puts one <c>FONT</c> record's whole face over a cell's format, for a rich-text run.
    /// </summary>
    /// <remarks>
    /// A replacement rather than a delta, which is where BIFF differs from SpreadsheetML: a
    /// formatting run inside an <c>SST</c> string names a complete <c>FONT</c> record, so it
    /// restates the family, the size, the weight, the posture and the colour whether or not it
    /// changes any of them. Reading it as a delta would leave a run bold merely because its cell
    /// was. Only the alignment and the number format stay the cell's, because a font record states
    /// neither.
    /// </remarks>
    /// <param name="cellFormat">What the cell resolved to.</param>
    /// <param name="fontIndex">The run's <c>FONT</c> index, with the hole at four already in it.</param>
    public SheetCellFormat ApplyFont(SheetCellFormat cellFormat, int fontIndex)
    {
        ArgumentNullException.ThrowIfNull(cellFormat);

        if (fontIndex < 0 || fontIndex >= _fonts.Count) return cellFormat;

        BiffFont font = _fonts[fontIndex];
        return cellFormat with
        {
            FontFamily = font.Name.Length == 0 ? cellFormat.FontFamily : font.Name,
            FontSize = font.Height,
            FontWeight = font.Weight,
            IsItalic = font.IsItalic,
            Underline = font.Underline,
            IsStruckThrough = font.IsStruckThrough,
            Colour = ColourAt(font.ColourIndex),
        };
    }

    /// <summary>The index that means "whatever the window's text colour is", which prints black.</summary>
    private const int AutomaticColour = 0x7FFF;

    private Colour ColourAt(int index)
        => index >= 0 && index < _palette.Count ? _palette[index] : Colour.Black;

    /// <summary>
    /// The alignment fields of a BIFF5 <c>XF</c>.
    /// </summary>
    /// <param name="alignment">The two-byte alignment field.</param>
    public static BiffAlignment Align5(ushort alignment)
    {
        int orientation = (alignment >> 8) & 0x03;

        return new BiffAlignment(
            Horizontal(alignment & 0x07),
            Vertical((alignment >> 4) & 0x07),
            (alignment & 0x0008) != 0,
            false,
            0,
            RotationFromOrientation(orientation),
            orientation == 1);
    }

    /// <summary>The alignment fields of a BIFF8 <c>XF</c>.</summary>
    /// <param name="alignment">The two-byte alignment field.</param>
    /// <param name="misc">The two-byte field that follows it, which BIFF5 does not have.</param>
    public static BiffAlignment Align8(ushort alignment, ushort misc)
    {
        int rotation = (alignment >> 8) & 0xFF;

        return new BiffAlignment(
            Horizontal(alignment & 0x07),
            Vertical((alignment >> 4) & 0x07),
            (alignment & 0x0008) != 0,
            (misc & 0x0010) != 0,
            misc & 0x000F,
            Rotation(rotation),
            rotation == StackedRotation);
    }

    /// <summary>BIFF8's rotation byte: 0–90 anticlockwise, 91–180 clockwise, 255 stacked.</summary>
    private const int StackedRotation = 255;

    private static int Rotation(int stated) => stated switch
    {
        >= 0 and <= 90 => stated,
        > 90 and <= 180 => -(stated - 90),
        _ => 0,
    };

    /// <summary>BIFF5's two-bit orientation, which has no angles between the right angles.</summary>
    private static int RotationFromOrientation(int orientation) => orientation switch
    {
        2 => 90,
        3 => -90,
        _ => 0,
    };

    private static SheetHorizontalAlignment Horizontal(int value) => value switch
    {
        1 => SheetHorizontalAlignment.Left,
        2 or 6 => SheetHorizontalAlignment.Centre,
        3 => SheetHorizontalAlignment.Right,
        4 => SheetHorizontalAlignment.Fill,
        5 => SheetHorizontalAlignment.Justify,
        7 => SheetHorizontalAlignment.Distributed,
        _ => SheetHorizontalAlignment.General,
    };

    private static SheetVerticalAlignment Vertical(int value) => value switch
    {
        0 => SheetVerticalAlignment.Top,
        1 => SheetVerticalAlignment.Centre,
        2 => SheetVerticalAlignment.Bottom,
        3 => SheetVerticalAlignment.Justify,
        4 => SheetVerticalAlignment.Distributed,
        _ => SheetVerticalAlignment.Standard,
    };

    /// <summary>The palette a workbook starts from, before any <c>PALETTE</c> record.</summary>
    private static readonly Colour[] BiffPalette =
    [
        .. new uint[]
        {
            0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF,
            0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF,
            0x800000, 0x008000, 0x000080, 0x808000, 0x800080, 0x008080, 0xC0C0C0, 0x808080,
            0x9999FF, 0x993366, 0xFFFFCC, 0xCCFFFF, 0x660066, 0xFF8080, 0x0066CC, 0xCCCCFF,
            0x000080, 0xFF00FF, 0xFFFF00, 0x00FFFF, 0x800080, 0x800000, 0x008080, 0x0000FF,
            0x00CCFF, 0xCCFFFF, 0xCCFFCC, 0xFFFF99, 0x99CCFF, 0xFF99CC, 0xCC99FF, 0xFFCC99,
            0x3366FF, 0x33CCCC, 0x99CC00, 0xFFCC00, 0xFF9900, 0xFF6600, 0x666699, 0x969696,
            0x003366, 0x339966, 0x003300, 0x333300, 0x993300, 0x993366, 0x333399, 0x333333,
        }.Select(Colour.FromRgb),
    ];
}
