using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// A workbook's cell formats, with what a rich-text run needs to be resolved against them.
/// </summary>
/// <remarks>
/// The palette travels with the formats because a formatting run states its colour the same three
/// ways a font does — <c>rgb</c>, <c>indexed</c> or <c>theme</c> — and only the workbook's own
/// <c>indexedColors</c> can answer the second. Reading it twice would be two chances to disagree.
/// </remarks>
/// <param name="Formats">The formats, indexed as <c>cellXfs</c> orders them.</param>
/// <param name="Palette">The indexed colours, the workbook's overrides included.</param>
/// <param name="DefaultFont">
/// The workbook's own default font, which is what a rich-text run's unstated properties fall back
/// to. See <see cref="Apply"/>.
/// </param>
internal sealed record XlsxCellFormatTable(
    IReadOnlyList<SheetCellFormat> Formats, Colour[] Palette, SheetCellFormat DefaultFont)
{
    /// <summary>
    /// Builds a rich-text run's format from what its <c>rPr</c> states.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An <c>rPr</c> is a complete font, not a delta over the cell's</strong> — and this
    /// is measurable rather than a reading of the schema. Saving a cell whose first word is bold,
    /// LibreOffice writes the <em>cell's</em> <c>fontId</c> as the bold one and then writes the
    /// second run with an <c>rPr</c> that states a size and a name and no <c>b</c>; its own
    /// rendering draws that run regular. Its importer says why: a portion's font is constructed
    /// from the theme's default font model with every "used" flag already set
    /// (<c>Font::Font(rHelper, bDxf=false)</c>, <c>sc/source/filter/oox/stylesbuffer.cxx:584</c>),
    /// and the <c>rPr</c> then overwrites what it names, so the cell's own font never enters the
    /// portion at all (<c>RichStringPortion::convert</c>,
    /// <c>sc/source/filter/oox/richstring.cxx:109-118</c>). Reading it as a delta leaves the whole
    /// cell bold.
    /// </para>
    /// <para>
    /// The fallback is the workbook's <c>fonts[0]</c> rather than LibreOffice's literal
    /// <c>Cambria 11</c> (<c>ThemeBuffer::ThemeBuffer</c>,
    /// <c>sc/source/filter/oox/themebuffer.cxx:33</c>, marked as a locale TODO there). It differs
    /// only for a file whose <c>rPr</c> omits <c>rFont</c> or <c>sz</c>, which no producer writes,
    /// and the workbook's own default is the better answer when one does.
    /// </para>
    /// <para>
    /// Everything that is not a font stays the cell's: alignment, wrapping, rotation and the
    /// number format are properties of the cell, and a formatting run cannot state any of them.
    /// </para>
    /// </remarks>
    /// <param name="cellFormat">What the cell resolved to, for everything but the font.</param>
    /// <param name="font">What the run states.</param>
    public SheetCellFormat Apply(SheetCellFormat cellFormat, XlsxRunFont font)
        => XlsxCellFormats.Apply(cellFormat, DefaultFont, font, Palette);

    /// <summary>
    /// The same default font, in the shape a column width needs it.
    /// </summary>
    /// <remarks>
    /// A column width is a count of digits of this face, so pagination cannot happen until it
    /// has been measured — see <see cref="SheetColumnDigits"/>. It is the same
    /// <see cref="DefaultFont"/> a rich-text run falls back to because LibreOffice reads it from
    /// the same place for both (<c>StylesBuffer::getDefaultFont</c>).
    /// </remarks>
    public SheetDefaultFont DefaultColumnFont { get; } = new(
        DefaultFont.FontFamily, DefaultFont.FontSize, DefaultFont.FontWeight, DefaultFont.IsItalic);
}

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
    /// <summary>
    /// How many space widths one <c>indent</c> level is worth.
    /// </summary>
    /// <remarks>
    /// Three, measured in the <em>workbook's default font</em> rather than the cell's:
    /// <c>rUnitConverter.scaleValue(3.0 * mnIndent, Unit::Space, Unit::Twip)</c>
    /// (<c>sc/source/filter/oox/stylesbuffer.cxx:1263</c>), where one <c>Space</c> is the space
    /// character's advance in the default font (<c>unitconverter.cxx:139</c>). That is not the
    /// BIFF rule — <c>xistyle.cxx:846</c> uses a flat 200 twips a level — and the difference is
    /// visible: two levels of ten-point Liberation Sans is 330 twips here and 400 there, which
    /// is 3.5 pt of indent.
    /// </remarks>
    private const int SpacesPerIndentLevel = 3;

    /// <summary>What one indent level is worth when no font can be measured.</summary>
    /// <remarks>The BIFF conversion, which is the closest answer available without a face.</remarks>
    private const int FallbackTwipsPerIndentLevel = 200;

    /// <summary>Reads the cell formats a workbook's <c>styleSheet</c> declares.</summary>
    /// <param name="styleSheet">The <c>styleSheet</c> root, or null when the part is missing.</param>
    /// <param name="styles">The already-read number formats, so a cell keeps its own.</param>
    public static XlsxCellFormatTable Read(XElement? styleSheet, XlsxStyles styles)
    {
        ArgumentNullException.ThrowIfNull(styles);
        if (styleSheet is null)
        {
            return new XlsxCellFormatTable(
                [SheetCellFormat.Default], [.. DefaultPalette], SheetCellFormat.Default);
        }

        Colour[] palette = ReadPalette(styleSheet);
        List<Font> fonts =
        [
            .. Xlsx.Children(Xlsx.Child(styleSheet, "fonts"), "font")
                   .Select(font => ReadFont(font, palette)),
        ];

        List<Record> styleXfs = [.. Xlsx.Children(Xlsx.Child(styleSheet, "cellStyleXfs"), "xf")
                                        .Select(ReadRecord)];

        Length indentUnit = IndentUnit(fonts);

        List<SheetCellFormat> formats = [];
        foreach (XElement xf in Xlsx.Children(Xlsx.Child(styleSheet, "cellXfs"), "xf"))
        {
            Record record = ReadRecord(xf);
            Record? parent = record.StyleXf is { } id && id >= 0 && id < styleXfs.Count
                ? styleXfs[id]
                : null;

            formats.Add(Resolve(record, parent, fonts, styles, indentUnit, formats.Count));
        }

        // The workbook's own default font, which is what a rich-text run falls back to for
        // anything its rPr does not name.
        SheetCellFormat defaultFont = fonts.Count > 0
            ? new SheetCellFormat
            {
                FontFamily = fonts[0].Family,
                FontSize = fonts[0].Size,
                FontWeight = fonts[0].Weight,
                IsItalic = fonts[0].Italic,
                Colour = fonts[0].HasColour ? fonts[0].Colour : Colour.Black,
            }
            : SheetCellFormat.Default;

        return new XlsxCellFormatTable(
            formats.Count == 0 ? [SheetCellFormat.Default] : formats, palette, defaultFont);
    }

    /// <inheritdoc cref="XlsxCellFormatTable.Apply"/>
    /// <param name="cellFormat">What the cell resolved to, for everything but the font.</param>
    /// <param name="defaultFont">The workbook's default font, which supplies what the run omits.</param>
    /// <param name="font">What the run states.</param>
    /// <param name="palette">The workbook's indexed colours.</param>
    public static SheetCellFormat Apply(
        SheetCellFormat cellFormat, SheetCellFormat defaultFont, XlsxRunFont font, Colour[] palette)
    {
        ArgumentNullException.ThrowIfNull(cellFormat);
        ArgumentNullException.ThrowIfNull(defaultFont);
        ArgumentNullException.ThrowIfNull(font);
        ArgumentNullException.ThrowIfNull(palette);

        return cellFormat with
        {
            FontFamily = font.Family ?? defaultFont.FontFamily,
            FontSize = font.Points is > 0
                ? Length.FromPoints(font.Points.Value)
                : defaultFont.FontSize,
            FontWeight = font.Bold is { } bold ? bold ? 700 : 400 : defaultFont.FontWeight,
            IsItalic = font.Italic ?? defaultFont.IsItalic,
            Colour = Resolve(font.Colour, palette) ?? defaultFont.Colour,
        };
    }

    private static Colour? Resolve(XlsxRunColour? stated, Colour[] palette)
    {
        if (stated is not { } colour) return null;

        Colour resolved;

        if (colour.Rgb is { } rgb) resolved = Colour.FromRgb(rgb);
        else if (colour.Indexed is { } indexed)
            resolved = indexed >= 0 && indexed < palette.Length ? palette[indexed] : Colour.Black;
        else if (colour.Theme is { } theme) resolved = ThemeColour(theme);
        else return null;

        return colour.Tint != 0 ? Tint(resolved, colour.Tint) : resolved;
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
        string? Family, Length Size, int Weight, bool Italic, Colour Colour, bool HasColour,
        SheetUnderline Underline, bool Strike);

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
            stated,
            UnderlineOf(Xlsx.Child(font, "u")),
            Toggle(Xlsx.Child(font, "strike")));
    }

    /// <summary>
    /// The line under a font, whose <c>val</c> is optional and whose default is not "none".
    /// </summary>
    /// <remarks>
    /// A bare <c>&lt;u/&gt;</c> means single, which is what makes this a different question from
    /// <see cref="Toggle"/>'s: the attribute names a <em>style</em>, so its absence names the
    /// commonest one rather than the off state, and <c>val="none"</c> is how a font that inherits
    /// an underline turns it off. The two accounting styles differ from the plain ones only in how
    /// wide the line is drawn, which is not reproduced — see <see cref="SheetUnderline"/>.
    /// </remarks>
    private static SheetUnderline UnderlineOf(XElement? element) => element is null
        ? SheetUnderline.None
        : Xlsx.Attribute(element, "val") switch
        {
            null or "single" or "singleAccounting" => SheetUnderline.SingleLine,
            "double" or "doubleAccounting" => SheetUnderline.DoubleLine,
            _ => SheetUnderline.None,
        };

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

    /// <summary>What one <c>indent</c> level is worth, measured in the default font.</summary>
    private static Length IndentUnit(List<Font> fonts)
    {
        if (fonts.Count == 0) return Length.FromTwips(FallbackTwipsPerIndentLevel);

        Font first = fonts[0];
        SheetCellFormat probe = new()
        {
            FontFamily = first.Family,
            FontSize = first.Size,
            FontWeight = first.Weight,
            IsItalic = first.Italic,
        };

        if (SheetFonts.For(probe) is not { } face)
            return Length.FromTwips(FallbackTwipsPerIndentLevel);

        // Truncated to whole twips, because that is the unit LibreOffice's own measurement lands
        // in before it is multiplied: XFont::getCharWidth answers in twips. Ten-point Liberation
        // Sans has a 55.57-twip space, and rounding it to 56 rather than truncating to 55 puts a
        // two-level indent 0.3 pt out.
        long space = SheetText.Measure(" ", face, first.Size).Emu / 635;
        return space > 0
            ? Length.FromTwips(space * SpacesPerIndentLevel)
            : Length.FromTwips(FallbackTwipsPerIndentLevel);
    }

    private static SheetCellFormat Resolve(
        Record record,
        Record? parent,
        List<Font> fonts,
        XlsxStyles styles,
        Length indentUnit,
        int index)
    {
        int fontId = record.FontUsed || parent is null ? record.FontId : parent.Value.FontId;
        Font font = fontId >= 0 && fontId < fonts.Count
            ? fonts[fontId]
            : new Font(null, Length.FromPoints(10), 400, false, Colour.Black, false,
                SheetUnderline.None, false);

        Alignment alignment = record.AlignmentUsed || parent is null
            ? record.Alignment
            : parent.Value.Alignment;

        Core.Numbers.NumberFormatCode code = styles.FormatFor(index);

        return new SheetCellFormat
        {
            FontFamily = font.Family,
            FontSize = font.Size,
            FontWeight = font.Weight,
            IsItalic = font.Italic,
            Underline = font.Underline,
            IsStruckThrough = font.Strike,
            Colour = font.HasColour ? font.Colour : Colour.Black,
            Horizontal = alignment.Horizontal,
            Vertical = alignment.Vertical,
            Wraps = alignment.Wraps,
            ShrinksToFit = alignment.Shrinks,
            Indent = indentUnit * alignment.Indent,
            RotationDegrees = Rotation(alignment.Rotation),
            IsStacked = alignment.Stacked,
            NumberFormatKind = code.IsGeneral || code.Sections.Count == 0
                ? Core.Numbers.NumberFormatKind.General
                : code.Sections[0].Kind,
            NumberFormat = code,
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
