using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// Reads the fills and borders SpreadsheetML keeps in <c>styles.xml</c>, and which cells use them.
/// </summary>
/// <remarks>
/// <para>
/// Beside <see cref="XlsxStyles"/> rather than inside it, because the two answer different
/// questions for different callers: extraction needs the number format a cell's <c>xf</c> names
/// and nothing else, and giving it the fills and borders as well would make every extraction
/// pay for a rendering. This reads the same part for the other half.
/// </para>
/// <para>
/// The indirection is three deep and every step matters. A cell's <c>s</c> attribute indexes
/// <c>cellXfs</c>; the <c>xf</c> there names a <c>fillId</c> and a <c>borderId</c>, and may
/// defer to a <c>cellStyleXfs</c> entry through <c>xfId</c> when its <c>applyFill</c> or
/// <c>applyBorder</c> flag is off. Then the fill itself is a <em>pattern</em>: a solid fill
/// writes its colour in <c>fgColor</c> and not in <c>bgColor</c>, which is the trap — a reader
/// taking <c>bgColor</c> paints every solid-filled cell the wrong colour, and LibreOffice's own
/// export writes a different colour in each of the two.
/// </para>
/// </remarks>
internal static class XlsxCellDecoration
{
    /// <summary>How far one <c>&lt;col&gt;</c> run is honoured.</summary>
    /// <remarks>
    /// A <c>&lt;col min="1" max="16384" style="3"/&gt;</c> is ordinary and is stored as a run,
    /// so this bounds nothing that matters — it only stops a <c>max</c> outside the format's own
    /// limit from being taken at its word.
    /// </remarks>
    private const int MaxColumn = 16383;

    /// <summary>Reads one sheet's decoration.</summary>
    /// <param name="styles">The <c>styleSheet</c> root, or null when the workbook has none.</param>
    /// <param name="theme">The <c>theme</c> root, for colours named by index.</param>
    /// <param name="worksheet">The sheet's own root.</param>
    public static SheetFormatting Read(XElement? styles, XElement? theme, XElement? worksheet)
    {
        if (worksheet is null) return SheetFormatting.Empty;

        Palette palette = Palette.Read(styles, theme);
        List<SheetCellFormat> formats = ReadCellFormats(styles, palette);
        if (formats.Count == 0) return SheetFormatting.Empty;

        SheetFormatting formatting = new();
        int[] handles = new int[formats.Count];
        for (int i = 0; i < formats.Count; i++)
        {
            handles[i] = formats[i].IsNone ? 0 : formatting.Intern(formats[i]);
        }

        int Handle(int? index)
            => index is { } at && at >= 0 && at < handles.Length ? handles[at] : 0;

        foreach (XElement column in Xlsx.Children(Xlsx.Child(worksheet, "cols"), "col"))
        {
            int handle = Handle(Xlsx.Integer(column, "style"));
            if (handle == 0) continue;

            int first = (Xlsx.Integer(column, "min") ?? 1) - 1;
            int last = (Xlsx.Integer(column, "max") ?? 1) - 1;
            formatting.SetColumns(Math.Max(0, first), Math.Min(MaxColumn, last), handle);
        }

        int expectedRow = 0;
        foreach (XElement row in Xlsx.Children(Xlsx.Child(worksheet, "sheetData"), "row"))
        {
            int index = (Xlsx.Integer(row, "r") - 1) ?? expectedRow;
            if (index < 0) index = expectedRow;
            expectedRow = index + 1;

            // customFormat is what makes a row's own s attribute mean anything: without it the
            // attribute is there but inert, and honouring it anyway paints rows Excel does not.
            if (Xlsx.Flag(row, "customFormat")) formatting.SetRow(index, Handle(Xlsx.Integer(row, "s")));

            foreach (XElement cell in Xlsx.Children(row, "c"))
            {
                int handle = Handle(Xlsx.Integer(cell, "s"));
                if (handle == 0) continue;

                if (Xlsx.TryParseCellReference(Xlsx.Attribute(cell, "r"), out int column, out int at))
                    formatting.SetCell(at, column, handle);
            }
        }

        return formatting.IsEmpty ? SheetFormatting.Empty : formatting;
    }

    /// <summary>One entry per <c>cellXfs</c> index, in that order.</summary>
    private static List<SheetCellFormat> ReadCellFormats(XElement? styles, Palette palette)
    {
        List<SheetCellFormat> formats = [];
        if (styles is null) return formats;

        List<Colour?> fills = [.. Xlsx.Children(Xlsx.Child(styles, "fills"), "fill")
            .Select(fill => ReadFill(fill, palette))];
        List<SheetCellBorders> borders = [.. Xlsx.Children(Xlsx.Child(styles, "borders"), "border")
            .Select(border => ReadBorders(border, palette))];

        List<XElement> styleXfs = [.. Xlsx.Children(Xlsx.Child(styles, "cellStyleXfs"), "xf")];

        foreach (XElement xf in Xlsx.Children(Xlsx.Child(styles, "cellXfs"), "xf"))
        {
            // applyFill/applyBorder off means "take the named style's", which is what xfId
            // points at. Absent, the attribute defaults to on for a cell xf.
            XElement? parent = Xlsx.Integer(xf, "xfId") is { } id && id >= 0 && id < styleXfs.Count
                ? styleXfs[id]
                : null;

            XElement fillFrom = Xlsx.Flag(xf, "applyFill", true) || parent is null ? xf : parent;
            XElement borderFrom = Xlsx.Flag(xf, "applyBorder", true) || parent is null ? xf : parent;

            Colour? fill = At(fills, Xlsx.Integer(fillFrom, "fillId"));
            SheetCellBorders border = At(borders, Xlsx.Integer(borderFrom, "borderId"));

            formats.Add(fill is null && border.IsNone
                ? SheetCellFormat.None
                : new SheetCellFormat(fill, border));
        }

        return formats;

        static T? At<T>(List<T> list, int? index)
            => index is { } at && at >= 0 && at < list.Count ? list[at] : default;
    }

    /// <summary>
    /// The colour a <c>fill</c> paints, or null when it paints nothing.
    /// </summary>
    /// <remarks>
    /// Only <c>solid</c> is a colour. <c>none</c> is transparent, and the eighteen hatch
    /// patterns are a foreground drawn over a background, which is not something a single
    /// colour can stand for — they are reported as their <em>background</em> colour, which is
    /// what Calc falls back to when it cannot hatch (<c>XclImpCellArea</c>,
    /// <c>sc/source/filter/excel/xistyle.cxx:1075</c>), and recorded in the module's TODO.
    /// </remarks>
    private static Colour? ReadFill(XElement fill, Palette palette)
    {
        XElement? pattern = Xlsx.Child(fill, "patternFill");
        if (pattern is null) return null;

        string type = Xlsx.Attribute(pattern, "patternType") ?? "none";
        if (string.Equals(type, "none", StringComparison.Ordinal)) return null;

        // A solid fill's colour is its foreground; a hatch's visible mass is its background.
        return string.Equals(type, "solid", StringComparison.Ordinal)
            ? palette.Read(Xlsx.Child(pattern, "fgColor"))
            : palette.Read(Xlsx.Child(pattern, "bgColor"))
              ?? palette.Read(Xlsx.Child(pattern, "fgColor"));
    }

    private static SheetCellBorders ReadBorders(XElement border, Palette palette)
        => new(
            Edge(Xlsx.Child(border, "left"), palette),
            Edge(Xlsx.Child(border, "right"), palette),
            Edge(Xlsx.Child(border, "top"), palette),
            Edge(Xlsx.Child(border, "bottom"), palette));

    /// <summary>
    /// One edge, from the fourteen style names SpreadsheetML allows.
    /// </summary>
    /// <remarks>
    /// The widths are LibreOffice's, in twips: hair 1, thin 15, medium 35 and thick 50
    /// (<c>API_LINE_*</c>, <c>sc/source/filter/inc/stylesbuffer.hxx:63-67</c>), assigned by the
    /// same switch this mirrors (<c>stylesbuffer.cxx:1700-1748</c>). They are not what the names
    /// suggest: <c>thin</c> draws at 0.75 pt and <c>hair</c> at a twentieth of a point.
    /// </remarks>
    private static SheetBorder Edge(XElement? edge, Palette palette)
    {
        if (edge is null) return SheetBorder.None;

        string style = Xlsx.Attribute(edge, "style") ?? "none";
        Colour colour = palette.Read(Xlsx.Child(edge, "color")) ?? Colour.Black;

        (int twips, SheetBorderPattern pattern, bool doubled) = style switch
        {
            "hair" => (1, SheetBorderPattern.Solid, false),
            "thin" => (15, SheetBorderPattern.Solid, false),
            "medium" => (35, SheetBorderPattern.Solid, false),
            "thick" => (50, SheetBorderPattern.Solid, false),
            "double" => (50, SheetBorderPattern.Solid, true),
            "dotted" => (15, SheetBorderPattern.Dotted, false),
            "dashed" => (15, SheetBorderPattern.FineDashed, false),
            "dashDot" => (15, SheetBorderPattern.DashDot, false),
            "dashDotDot" => (15, SheetBorderPattern.DashDotDot, false),
            "mediumDashed" => (35, SheetBorderPattern.FineDashed, false),
            "mediumDashDot" => (35, SheetBorderPattern.DashDot, false),
            "mediumDashDotDot" => (35, SheetBorderPattern.DashDotDot, false),
            "slantDashDot" => (35, SheetBorderPattern.DashDot, false),
            _ => (0, SheetBorderPattern.Solid, false),
        };

        if (twips == 0) return SheetBorder.None;

        Length width = Length.FromTwips(twips);
        if (!doubled) return SheetBorder.Line(width, colour, pattern);

        Length line = width / 3;
        return new SheetBorder(line, width - line - line, line, colour, pattern);
    }

    /// <summary>
    /// Resolves the four ways SpreadsheetML can name a colour.
    /// </summary>
    /// <remarks>
    /// <c>rgb</c> is an ARGB string; <c>indexed</c> points into the legacy 56-entry palette,
    /// which a workbook may override with its own <c>indexedColors</c>; <c>theme</c> points into
    /// the theme's colour scheme in a fixed order that is <em>not</em> the order the scheme
    /// element writes them in; and any of the three may carry a <c>tint</c> that lightens or
    /// darkens the result.
    /// </remarks>
    private sealed class Palette
    {
        /// <summary>
        /// The theme slots, in the order <c>theme="n"</c> numbers them.
        /// </summary>
        /// <remarks>
        /// Light and dark are <em>swapped</em> against the scheme's own element order: slot 0 is
        /// <c>lt1</c> and slot 1 is <c>dk1</c>, which is a documented quirk of SpreadsheetML's
        /// indices rather than a mistake. LibreOffice writes the same swap
        /// (<c>oox/source/drawingml/themeelementscontext.cxx</c> reads the scheme in element
        /// order, and <c>sc/source/filter/oox/stylesbuffer.cxx</c> maps the index through
        /// <c>getColorByIndex</c>).
        /// </remarks>
        private static readonly string[] ThemeSlots =
            ["lt1", "dk1", "lt2", "dk2", "accent1", "accent2", "accent3", "accent4", "accent5",
             "accent6", "hlink", "folHlink"];

        private readonly Dictionary<int, Colour> _indexed = [];
        private readonly List<Colour> _theme = [];

        public static Palette Read(XElement? styles, XElement? theme)
        {
            Palette palette = new();

            int at = 0;
            foreach (XElement colour in Xlsx.Children(
                         Xlsx.Child(Xlsx.Child(styles, "colors"), "indexedColors"), "rgbColor"))
            {
                if (ParseRgb(Xlsx.Attribute(colour, "rgb")) is { } parsed) palette._indexed[at] = parsed;
                at++;
            }

            XElement? scheme = theme?
                .Element(XName.Get("themeElements", OoxmlNamespaces.DrawingML))?
                .Element(XName.Get("clrScheme", OoxmlNamespaces.DrawingML));

            foreach (string slot in ThemeSlots)
            {
                palette._theme.Add(SchemeColour(scheme, slot) ?? Colour.Black);
            }

            return palette;
        }

        /// <summary>The colour a <c>color</c> element names, or null when it names none.</summary>
        public Colour? Read(XElement? element)
        {
            if (element is null) return null;
            if (Xlsx.Flag(element, "auto")) return null;

            Colour? colour = null;

            if (Xlsx.Attribute(element, "rgb") is { } rgb) colour = ParseRgb(rgb);
            else if (Xlsx.Integer(element, "indexed") is { } indexed) colour = Indexed(indexed);
            else if (Xlsx.Integer(element, "theme") is { } theme)
                colour = theme >= 0 && theme < _theme.Count ? _theme[theme] : null;

            if (colour is not { } found) return null;

            double tint = Xlsx.Double(Xlsx.Attribute(element, "tint")) ?? 0;
            return Math.Abs(tint) < 0.0001 ? found : Tint(found, tint);
        }

        private Colour? Indexed(int index)
        {
            if (_indexed.TryGetValue(index, out Colour stated)) return stated;

            // 64 and 65 are "automatic foreground" and "automatic background", which have no
            // colour of their own: they resolve to the window text and window background, and
            // Calc renders them as black on white.
            return index switch
            {
                >= 0 and < 64 => Colour.FromRgb(DefaultIndexed[index]),
                64 or 81 => Colour.Black,
                65 => Colour.White,
                _ => null,
            };
        }

        private static Colour? SchemeColour(XElement? scheme, string slot)
        {
            XElement? entry = scheme?.Element(XName.Get(slot, OoxmlNamespaces.DrawingML));
            if (entry is null) return null;

            XElement? srgb = entry.Element(XName.Get("srgbClr", OoxmlNamespaces.DrawingML));
            if (srgb?.Attribute("val")?.Value is { } value) return ParseRgb(value);

            XElement? system = entry.Element(XName.Get("sysClr", OoxmlNamespaces.DrawingML));
            if (system?.Attribute("lastClr")?.Value is { } last) return ParseRgb(last);

            return null;
        }

        private static Colour? ParseRgb(string? value)
        {
            if (value is null) return null;

            string text = value.Trim().TrimStart('#');
            if (!uint.TryParse(text, NumberStyles.HexNumber, CultureInfo.InvariantCulture,
                               out uint packed))
            {
                return null;
            }

            // Eight digits are ARGB and six are RGB; the alpha is dropped either way, because a
            // half-transparent cell fill is not something any of the three formats really means.
            return Colour.FromRgb(text.Length > 6 ? packed & 0x00FFFFFFu : packed);
        }

        /// <summary>
        /// Lightens or darkens a colour by a tint, which is the ECMA-376 formula on luminance.
        /// </summary>
        /// <remarks>
        /// A negative tint scales the luminance towards black and a positive one towards white,
        /// which is not the same as blending with either — a mid grey tinted by 0.5 comes out
        /// lighter than a simple blend would put it.
        /// </remarks>
        private static Colour Tint(Colour colour, double tint)
        {
            double max = Math.Max(colour.R, Math.Max(colour.G, colour.B)) / 255.0;
            double min = Math.Min(colour.R, Math.Min(colour.G, colour.B)) / 255.0;
            double luminance = (max + min) / 2;

            double target = tint < 0 ? luminance * (1 + tint) : (luminance * (1 - tint)) + tint;
            double shift = target - luminance;

            return new Colour(
                Component(colour.R, shift), Component(colour.G, shift), Component(colour.B, shift));

            static byte Component(byte value, double shift)
                => (byte)Math.Clamp(Math.Round((value / 255.0 + shift) * 255), 0, 255);
        }

        /// <summary>
        /// The legacy 64-entry palette a workbook that declares none falls back to.
        /// </summary>
        /// <remarks>
        /// LibreOffice's own copy, <c>spPreDefColors</c>
        /// (<c>sc/source/filter/oox/stylesbuffer.cxx</c>) — entries 0-7 repeat as 8-15, which is
        /// not an error in the transcription but how the palette has always been defined.
        /// </remarks>
        private static readonly uint[] DefaultIndexed =
        [
            0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF,
            0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF,
            0x800000, 0x008000, 0x000080, 0x808000, 0x800080, 0x008080, 0xC0C0C0, 0x808080,
            0x9999FF, 0x993366, 0xFFFFCC, 0xCCFFFF, 0x660066, 0xFF8080, 0x0066CC, 0xCCCCFF,
            0x000080, 0xFF00FF, 0xFFFF00, 0x00FFFF, 0x800080, 0x800000, 0x008080, 0x0000FF,
            0x00CCFF, 0xCCFFFF, 0xCCFFCC, 0xFFFF99, 0x99CCFF, 0xFF99CC, 0xCC99FF, 0xFFCC99,
            0x3366FF, 0x33CCCC, 0x99CC00, 0xFFCC00, 0xFF9900, 0xFF6600, 0x666699, 0x969696,
            0x003366, 0x339966, 0x003300, 0x333300, 0x993300, 0x993366, 0x333399, 0x333333,
        ];
    }
}
