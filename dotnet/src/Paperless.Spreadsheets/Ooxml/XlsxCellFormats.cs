using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// The part of <c>styles.xml</c> that decides how a cell's text is <em>drawn</em>.
/// </summary>
/// <remarks>
/// <para>
/// Separate from <see cref="XlsxStyles"/>, which reads the number formats extraction needs and
/// nothing else. Fonts, alignment, wrapping, indent and rotation are worth nothing to a caller
/// asking what a workbook says and are the whole of what a renderer needs, so they are read on
/// demand rather than for every extraction.
/// </para>
/// <para>
/// Ported from <c>sc/source/filter/oox/stylesbuffer.cxx</c>. Two of its rules are not obvious from
/// the schema. A <c>cellXf</c>'s attributes only take effect when the matching <c>apply…</c> flag
/// says so, and the flag's <em>default</em> is not false: <c>applyFont</c> defaults to true
/// whenever <c>fontId</c> is non-zero (<c>Xf::importXf</c>, <c>:2176</c>), because files written
/// by third-party tools state the id and omit the flag. And <c>applyAlignment</c> is forced true
/// by the mere presence of an <c>&lt;alignment&gt;</c> child (<c>:2186</c>).
/// </para>
/// </remarks>
internal static class XlsxCellFormats
{
    /// <summary>One Excel indent level, in twips.</summary>
    /// <remarks>
    /// "1 Excel unit == 10 pt == 200 twips", <c>sc/source/filter/excel/xistyle.cxx:846</c>. The
    /// OOXML filter uses the same conversion.
    /// </remarks>
    private const int TwipsPerIndentLevel = 200;

    /// <summary>Reads the cell formats a workbook's <c>styleSheet</c> declares.</summary>
    /// <param name="styleSheet">The <c>styleSheet</c> root, or null when the part is missing.</param>
    /// <param name="styles">The already-read number formats, so a cell keeps its own.</param>
    public static IReadOnlyList<SheetCellFormat> Read(XElement? styleSheet, XlsxStyles styles)
    {
        ArgumentNullException.ThrowIfNull(styles);
        if (styleSheet is null) return [SheetCellFormat.Default];

        Colour[] palette = ReadPalette(styleSheet);
        List<Font> fonts =
        [
            .. Xlsx.Children(Xlsx.Child(styleSheet, "fonts"), "font")
                   .Select(font => ReadFont(font, palette)),
        ];

        List<Record> styleXfs = [.. Xlsx.Children(Xlsx.Child(styleSheet, "cellStyleXfs"), "xf")
                                        .Select(ReadRecord)];

        List<SheetCellFormat> formats = [];
        foreach (XElement xf in Xlsx.Children(Xlsx.Child(styleSheet, "cellXfs"), "xf"))
        {
            Record record = ReadRecord(xf);
            Record? parent = record.StyleXf is { } id && id >= 0 && id < styleXfs.Count
                ? styleXfs[id]
                : null;

            formats.Add(Resolve(record, parent, fonts, styles, formats.Count));
        }

        return formats.Count == 0 ? [SheetCellFormat.Default] : formats;
    }

    // ------------------------------------------------------------------------------ records

    private readonly record struct Record(
        int FontId,
        bool FontUsed,
        int NumberFormatId,
        bool NumberFormatUsed,
        int? StyleXf,
        bool AlignmentUsed,
        Alignment Alignment);

    private readonly record struct Alignment(
        SheetHorizontalAlignment Horizontal,
        SheetVerticalAlignment Vertical,
        bool Wraps,
        bool Shrinks,
        int Indent,
        int Rotation,
        bool Stacked);

    private readonly record struct Font(
        string? Family, Length Size, int Weight, bool Italic, Colour Colour, bool HasColour);

    private static Record ReadRecord(XElement xf)
    {
        int fontId = Xlsx.Integer(xf, "fontId") ?? 0;
        int numberFormatId = Xlsx.Integer(xf, "numFmtId") ?? 0;
        XElement? alignment = Xlsx.Child(xf, "alignment");

        return new Record(
            fontId,
            Flag(xf, "applyFont") ?? fontId > 0,
            numberFormatId,
            Flag(xf, "applyNumberFormat") ?? numberFormatId > 0,
            Xlsx.Integer(xf, "xfId"),
            alignment is not null || (Flag(xf, "applyAlignment") ?? false),
            ReadAlignment(alignment));
    }

    private static Alignment ReadAlignment(XElement? alignment)
    {
        if (alignment is null)
        {
            return new Alignment(
                SheetHorizontalAlignment.General, SheetVerticalAlignment.Standard,
                false, false, 0, 0, false);
        }

        int rotation = Xlsx.Integer(alignment, "textRotation") ?? 0;

        return new Alignment(
            Horizontal(Xlsx.Attribute(alignment, "horizontal")),
            Vertical(Xlsx.Attribute(alignment, "vertical")),
            Xlsx.Flag(alignment, "wrapText"),
            Xlsx.Flag(alignment, "shrinkToFit"),
            Xlsx.Integer(alignment, "indent") ?? 0,
            rotation,
            rotation == 255);
    }

    private static SheetHorizontalAlignment Horizontal(string? value) => value switch
    {
        "left" => SheetHorizontalAlignment.Left,
        "center" or "centerContinuous" => SheetHorizontalAlignment.Centre,
        "right" => SheetHorizontalAlignment.Right,
        "fill" => SheetHorizontalAlignment.Fill,
        "justify" => SheetHorizontalAlignment.Justify,
        "distributed" => SheetHorizontalAlignment.Distributed,
        _ => SheetHorizontalAlignment.General,
    };

    private static SheetVerticalAlignment Vertical(string? value) => value switch
    {
        "top" => SheetVerticalAlignment.Top,
        "center" => SheetVerticalAlignment.Centre,
        "bottom" => SheetVerticalAlignment.Bottom,
        "justify" => SheetVerticalAlignment.Justify,
        "distributed" => SheetVerticalAlignment.Distributed,
        _ => SheetVerticalAlignment.Standard,
    };

    // -------------------------------------------------------------------------------- fonts

    private static Font ReadFont(XElement font, Colour[] palette)
    {
        double? points = Number(Xlsx.Child(font, "sz"), "val");
        Colour colour = ColourOf(Xlsx.Child(font, "color"), palette, out bool stated);

        return new Font(
            Xlsx.Attribute(Xlsx.Child(font, "name"), "val")
                ?? Xlsx.Attribute(Xlsx.Child(font, "rFont"), "val"),
            Length.FromPoints(points is > 0 ? points.Value : 10),
            Toggle(Xlsx.Child(font, "b")) ? 700 : 400,
            Toggle(Xlsx.Child(font, "i")),
            colour,
            stated);
    }

    /// <summary>
    /// A toggle element such as <c>&lt;b/&gt;</c>, whose absence and whose <c>val="0"</c> differ.
    /// </summary>
    /// <remarks>
    /// The element on its own means "on"; only an explicit <c>val</c> of 0 or false turns it off.
    /// Reading a bare <c>&lt;b/&gt;</c> as false leaves every bold cell in the workbook regular.
    /// </remarks>
    private static bool Toggle(XElement? element)
        => element is not null && (Xlsx.Attribute(element, "val") is not { } value
                                   || value is not ("0" or "false"));

    /// <summary>
    /// A colour element, resolved as far as a workbook without its theme allows.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>rgb</c> is an eight-digit ARGB and is exact. <c>indexed</c> is the BIFF palette, which
    /// the workbook may override in <c>colors/indexedColors</c>. <c>theme</c> needs
    /// <c>theme1.xml</c>, which is not read yet: the standard Office palette is used instead,
    /// which is right for every workbook that has not been re-themed and wrong by a hue for one
    /// that has. <c>tint</c> lightens towards white or darkens towards black, and applies to all
    /// three.
    /// </para>
    /// <para>
    /// <c>auto="1"</c> is the window text colour, which is black on every printed page.
    /// </para>
    /// </remarks>
    private static Colour ColourOf(XElement? element, Colour[] palette, out bool stated)
    {
        stated = false;
        if (element is null) return Colour.Black;

        Colour colour;

        if (Xlsx.Attribute(element, "rgb") is { Length: >= 6 } rgb
            && uint.TryParse(
                rgb[^6..], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out uint value))
        {
            colour = Colour.FromRgb(value);
        }
        else if (Xlsx.Integer(element, "indexed") is { } indexed)
        {
            colour = indexed >= 0 && indexed < palette.Length ? palette[indexed] : Colour.Black;
        }
        else if (Xlsx.Integer(element, "theme") is { } theme)
        {
            colour = ThemeColour(theme);
        }
        else
        {
            return Colour.Black;
        }

        stated = true;

        if (Number(element, "tint") is { } tint && tint != 0) colour = Tint(colour, tint);
        return colour;
    }

    /// <summary>
    /// Lightens towards white for a positive tint and darkens towards black for a negative one.
    /// </summary>
    /// <remarks>
    /// The linear form, which is what LibreOffice's OOXML filter applies to a plain RGB value
    /// (<c>oox/source/helper/graphichelper.cxx</c>). The exact form works in HSL luminance and
    /// differs by a shade or two on saturated colours.
    /// </remarks>
    private static Colour Tint(Colour colour, double tint)
    {
        double factor = Math.Clamp(tint, -1.0, 1.0);
        return new Colour(Channel(colour.R), Channel(colour.G), Channel(colour.B), colour.A);

        byte Channel(byte component) => (byte)Math.Clamp(
            factor >= 0
                ? component + ((255 - component) * factor)
                : component * (1 + factor),
            0, 255);
    }

    private static Colour[] ReadPalette(XElement styleSheet)
    {
        Colour[] palette = [.. DefaultPalette];

        int at = 0;
        foreach (XElement entry in Xlsx.Children(
                     Xlsx.Child(Xlsx.Child(styleSheet, "colors"), "indexedColors"), "rgbColor"))
        {
            if (at >= palette.Length) break;
            if (Xlsx.Attribute(entry, "rgb") is { Length: >= 6 } rgb
                && uint.TryParse(
                    rgb[^6..], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out uint value))
            {
                palette[at] = Colour.FromRgb(value);
            }
            at++;
        }

        return palette;
    }

    /// <summary>
    /// The standard Office theme's twelve colours, in <c>clrScheme</c> order.
    /// </summary>
    /// <remarks>
    /// SpreadsheetML swaps the first four round relative to DrawingML — a <c>theme</c> index of 0
    /// is the *light* background and 1 the dark text, where the theme part lists dark first
    /// (<c>oox/source/drawingml/theme.cxx</c>, <c>getColorByToken</c>). Getting that pair the
    /// wrong way round paints every default-coloured cell white on white.
    /// </remarks>
    private static Colour ThemeColour(int index) => index switch
    {
        0 => Colour.White,
        1 => Colour.Black,
        2 => Colour.FromRgb(0xE7E6E6),
        3 => Colour.FromRgb(0x44546A),
        4 => Colour.FromRgb(0x4472C4),
        5 => Colour.FromRgb(0xED7D31),
        6 => Colour.FromRgb(0xA5A5A5),
        7 => Colour.FromRgb(0xFFC000),
        8 => Colour.FromRgb(0x5B9BD5),
        9 => Colour.FromRgb(0x70AD47),
        10 => Colour.FromRgb(0x0563C1),
        11 => Colour.FromRgb(0x954F72),
        _ => Colour.Black,
    };

    /// <summary>The BIFF colour palette, which <c>indexed</c> selects from.</summary>
    private static readonly Colour[] DefaultPalette =
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

    // ----------------------------------------------------------------------------- resolving

    private static SheetCellFormat Resolve(
        Record record,
        Record? parent,
        List<Font> fonts,
        XlsxStyles styles,
        int index)
    {
        int fontId = record.FontUsed || parent is null ? record.FontId : parent.Value.FontId;
        Font font = fontId >= 0 && fontId < fonts.Count
            ? fonts[fontId]
            : new Font(null, Length.FromPoints(10), 400, false, Colour.Black, false);

        Alignment alignment = record.AlignmentUsed || parent is null
            ? record.Alignment
            : parent.Value.Alignment;

        Numbers.NumberFormatCode code = styles.FormatFor(index);

        return new SheetCellFormat
        {
            FontFamily = font.Family,
            FontSize = font.Size,
            FontWeight = font.Weight,
            IsItalic = font.Italic,
            Colour = font.HasColour ? font.Colour : Colour.Black,
            Horizontal = alignment.Horizontal,
            Vertical = alignment.Vertical,
            Wraps = alignment.Wraps,
            ShrinksToFit = alignment.Shrinks,
            Indent = Length.FromTwips((long)alignment.Indent * TwipsPerIndentLevel),
            RotationDegrees = Rotation(alignment.Rotation),
            IsStacked = alignment.Stacked,
            NumberFormat = code.IsGeneral ? null : code,
        };
    }

    /// <summary>
    /// SpreadsheetML's 0–180 rotation folded into Calc's -90 to 90.
    /// </summary>
    /// <remarks>
    /// 0–90 is anticlockwise and 91–180 is clockwise by 1–90; 255 means stacked and is handled
    /// separately. <c>XclTools::GetScRotation</c>.
    /// </remarks>
    private static int Rotation(int stated) => stated switch
    {
        >= 0 and <= 90 => stated,
        > 90 and <= 180 => -(stated - 90),
        _ => 0,
    };

    private static bool? Flag(XElement element, string name)
        => Xlsx.Attribute(element, name) switch
        {
            null => null,
            "0" or "false" => false,
            _ => true,
        };

    private static double? Number(XElement? element, string name)
        => element is not null
           && Xlsx.Attribute(element, name) is { } text
           && double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture, out double value)
            ? value
            : null;
}
