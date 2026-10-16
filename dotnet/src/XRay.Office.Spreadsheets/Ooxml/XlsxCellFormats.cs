using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Text.Fonts;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// A workbook's cell formats, with what a rich-text run needs to be resolved against them.
/// </summary>
/// <remarks>
/// The palette travels with the formats because a formatting run states its colour the same three
/// ways a font does — <c>rgb</c>, <c>indexed</c> or <c>theme</c> — and only the workbook's own
/// <c>indexedColors</c> can answer the second. Reading it twice would be two chances to disagree.
/// </remarks>
/// <param name="Formats">The formats, indexed as <c>cellXfs</c> orders them.</param>
/// <param name="Palette">
/// The workbook's colours: its <c>indexedColors</c> overrides and its theme's colour scheme.
/// </param>
/// <param name="DefaultFont">
/// The workbook's own default font, which is what a rich-text run's unstated properties fall back
/// to. See <see cref="Apply"/>.
/// </param>
/// <param name="StyleDefault">
/// The <c>Normal</c> cell style's own format — Calc's <c>Default</c> cell style. What an emptied
/// pivot cell falls back to, and what a cell that states no <c>s</c> in a row and column that
/// state none resolves to. See <see cref="XlsxCellFormats.NormalStyleXf"/>.
/// </param>
internal sealed record XlsxCellFormatTable(
    IReadOnlyList<SheetCellFormat> Formats,
    XlsxPalette Palette,
    SheetCellFormat DefaultFont,
    SheetCellFormat StyleDefault)
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
        DefaultFont.FontFamily, DefaultFont.FontSize, DefaultFont.FontWeight, DefaultFont.IsItalic,
        DefaultFont.DeclaredFontClass);
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
    /// <param name="theme">
    /// The <c>theme</c> part's root, for the colour scheme a font's <c>color/@theme</c> indexes
    /// into. Null falls back to black for every scheme slot, exactly as
    /// <see cref="XlsxPalette"/> does for the fills and borders it already serves.
    /// </param>
    public static XlsxCellFormatTable Read(XElement? styleSheet, XlsxStyles styles, XElement? theme)
    {
        ArgumentNullException.ThrowIfNull(styles);
        if (styleSheet is null)
        {
            return new XlsxCellFormatTable(
                [SheetCellFormat.Default], XlsxPalette.Read(null, theme), SheetCellFormat.Default,
                SheetCellFormat.Default);
        }

        XlsxPalette palette = XlsxPalette.Read(styleSheet, theme);
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
                DeclaredFontClass = fonts[0].DeclaredClass,
                FontSize = fonts[0].Size,
                FontWeight = fonts[0].Weight,
                IsItalic = fonts[0].Italic,
                Colour = fonts[0].HasColour ? fonts[0].Colour : Colour.Black,
            }
            : SheetCellFormat.Default;

        // The `Normal` cell style's own format, which is a different question from `cellXfs[0]`
        // and is the one an emptied cell answers. Its number format is `styles.Default`, which
        // `FormatFor` returns for any index outside `cellXfs` — hence the -1.
        int normal = NormalStyleXf(styleSheet);
        SheetCellFormat styleDefault =
            normal >= 0 && normal < styleXfs.Count
                ? Resolve(styleXfs[normal], null, fonts, styles, indentUnit, -1)
                : formats.Count > 0 ? formats[0] : SheetCellFormat.Default;

        return new XlsxCellFormatTable(
            formats.Count == 0 ? [SheetCellFormat.Default] : formats, palette, defaultFont,
            styleDefault);
    }

    /// <summary>
    /// Which <c>cellStyleXfs</c> entry is the <c>Normal</c> cell style's.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Calc's <c>Default</c> cell style is this entry and not <c>cellXfs[0]</c>, and a
    /// probe workbook says so outright.</strong> The two are the same in every workbook of the
    /// corpus, so nothing real can tell them apart;
    /// <c>dotnet/probes/pivot-fmt-r110/make-default-fixture.py</c> writes one that states
    /// Liberation Sans 11 black in the <c>Normal</c> <c>cellStyleXf</c>, Liberation Serif 18 red
    /// in <c>cellXfs[0]</c> and Liberation Mono 8 bold green on a yellow fill in the cell format
    /// its pivot's own cells carry. In 26.2.4.2's <c>.fods</c> of it the emptied pivot cells come
    /// back Liberation Sans 11 black — the <c>Normal</c> style — while a cell of the control
    /// sheet stating <c>s="0"</c> comes back Liberation Serif 18 red.
    /// </para>
    /// <para>
    /// The same rule, measured the same way, decides which number format an unstated cell takes
    /// (<c>XlsxStyles.DefaultFormatId</c>, <c>probes/numfmt-r68</c>) and — since round 111 — which
    /// font and alignment it takes (<see cref="XlsxSheetFormats"/>, <c>probes/sheet-default-r111</c>).
    /// The three had to agree: a tree where a cell's number format came from the <c>Normal</c>
    /// style and its font from <c>cellXfs[0]</c> would be wrong on a workbook separating them in a
    /// way neither reading explains.
    /// </para>
    /// <para>
    /// <strong>The corpus can separate them, unlike the pivot half.</strong> Of the 243
    /// <c>.xlsx</c>/<c>.xlsm</c> of the sheets track carrying both tables, six state different
    /// content in the two and three still differ once the <c>apply…</c> flags and the schema's own
    /// alignment defaults are folded out: <c>jobs-bulletin-51-22-december-2025.xlsx</c> (a font
    /// declaring no generic class against one declaring <c>swiss</c>),
    /// <c>sectors-defense-and-aerospace.xlsx</c> and <c>Published_Issuances_2024.xlsx</c> (an
    /// <c>&lt;alignment&gt;</c> on the <c>cellXfs</c> side only). 26.2.4.2's own <c>.fods</c> of
    /// all three builds its <c>Default</c> cell style out of the <c>Normal</c> entry.
    /// </para>
    /// </remarks>
    /// <param name="styleSheet">The <c>styleSheet</c> root.</param>
    /// <returns>The index, or 0 when the workbook names no <c>Normal</c> style.</returns>
    private static int NormalStyleXf(XElement styleSheet)
    {
        // `builtinId="0"` is the Normal style; a workbook that names none falls back to the first
        // entry, which is where every producer writes it.
        foreach (XElement style in Xlsx.Children(Xlsx.Child(styleSheet, "cellStyles"), "cellStyle"))
        {
            if (Xlsx.Integer(style, "builtinId") != 0) continue;
            return Xlsx.Integer(style, "xfId") ?? 0;
        }

        return 0;
    }

    /// <inheritdoc cref="XlsxCellFormatTable.Apply"/>
    /// <param name="cellFormat">What the cell resolved to, for everything but the font.</param>
    /// <param name="defaultFont">The workbook's default font, which supplies what the run omits.</param>
    /// <param name="font">What the run states.</param>
    /// <param name="palette">The workbook's colours.</param>
    public static SheetCellFormat Apply(
        SheetCellFormat cellFormat, SheetCellFormat defaultFont, XlsxRunFont font,
        XlsxPalette palette)
    {
        ArgumentNullException.ThrowIfNull(cellFormat);
        ArgumentNullException.ThrowIfNull(defaultFont);
        ArgumentNullException.ThrowIfNull(font);
        ArgumentNullException.ThrowIfNull(palette);

        return cellFormat with
        {
            FontFamily = font.Family ?? defaultFont.FontFamily,

            // The declaration follows the name it qualifies rather than falling back on its own:
            // an rPr that renames the face and says nothing about its shape has said the shape is
            // unknown, and inheriting the cell's would file the new name under the old one's
            // family. Only a run that renames nothing keeps the cell's declaration.
            DeclaredFontClass = font.DeclaredFamily is { } code
                ? SheetDeclaredFonts.FromWindowsCode(code)
                : font.Family is null ? defaultFont.DeclaredFontClass : FontFamilyClass.Unknown,
            FontSize = font.Points is > 0
                ? Length.FromPoints(font.Points.Value)
                : defaultFont.FontSize,
            FontWeight = font.Bold is { } bold ? bold ? 700 : 400 : defaultFont.FontWeight,
            IsItalic = font.Italic ?? defaultFont.IsItalic,
            Colour = Resolve(font.Colour, palette) ?? defaultFont.Colour,
            Underline = font.Underline ?? defaultFont.Underline,
            IsStruckThrough = font.StruckThrough ?? defaultFont.IsStruckThrough,
        };
    }

    private static Colour? Resolve(XlsxRunColour? stated, XlsxPalette palette)
    {
        if (stated is not { } colour) return null;

        Colour? resolved =
            colour.Rgb is { } rgb ? Colour.FromRgb(rgb)
            : colour.Indexed is { } indexed ? palette.Indexed(indexed)
            : colour.Theme is { } theme ? palette.Theme(theme)
            : null;

        if (resolved is not { } found) return null;
        return colour.Tint != 0 ? XlsxTint.Apply(found, colour.Tint) : found;
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
        SheetUnderline Underline, bool Strike,
        FontFamilyClass DeclaredClass = FontFamilyClass.Unknown);

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

    /// <summary>
    /// The face a <c>&lt;font&gt;</c> that names none is set in.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Not the workbook's own <c>fonts[0]</c>, and not a generic sans.</strong> Every
    /// <c>&lt;font&gt;</c> the OOXML filter builds starts life as a copy of the theme buffer's
    /// default model, and that model is a hard-coded <c>Cambria</c> at 11 pt —
    /// <c>ThemeBuffer::ThemeBuffer</c> (<c>sc/source/filter/oox/themebuffer.cxx:31-33</c>), where
    /// it is marked "TODO: locale dependent font name" and has never been made one. Nothing in
    /// the theme part overrides it: <c>getDefaultFontModel</c> returns that same object however
    /// the workbook's own major and minor fonts are declared.
    /// </para>
    /// <para>
    /// Measured on <c>dotnet/probes/sheets-rest-01/mkfontprobe.py</c> under the installed
    /// 26.2.4.2, whose <c>fonts[0]</c> is Arial 10 so that the two candidate answers are
    /// distinguishable: <c>&lt;font/&gt;</c> draws in Caladea-Regular at 11.00 pt,
    /// <c>&lt;font&gt;&lt;b/&gt;&lt;/font&gt;</c> in Caladea-Bold at 11.00,
    /// <c>&lt;font&gt;&lt;sz val="20"/&gt;&lt;/font&gt;</c> in Caladea-Regular at 20.01, and
    /// <c>&lt;font&gt;&lt;name val="Arial"/&gt;&lt;/font&gt;</c> in LiberationSans at
    /// <em>11.00</em> — so the size default is the theme's eleven and not the ten a bare BIFF
    /// font would take. Caladea is Cambria's metric-compatible substitute, which is what makes
    /// the face readable off the PDF at all.
    /// </para>
    /// </remarks>
    private const string UnnamedFontFamily = "Cambria";

    /// <summary>The size a <c>&lt;font&gt;</c> that states none takes.</summary>
    /// <remarks>The other half of the same model — see <see cref="UnnamedFontFamily"/>.</remarks>
    private const double UnnamedFontPoints = 11.0;

    private static Font ReadFont(XElement font, XlsxPalette palette)
    {
        double? points = Number(Xlsx.Child(font, "sz"), "val");
        Colour? found = palette.Read(Xlsx.Child(font, "color"));
        Colour colour = found ?? Colour.Black;
        bool stated = found is not null;

        return new Font(
            Xlsx.Attribute(Xlsx.Child(font, "name"), "val")
                ?? Xlsx.Attribute(Xlsx.Child(font, "rFont"), "val")
                ?? UnnamedFontFamily,
            Length.FromPoints(points is > 0 ? points.Value : UnnamedFontPoints),
            Toggle(Xlsx.Child(font, "b")) ? 700 : 400,
            Toggle(Xlsx.Child(font, "i")),
            colour,
            stated,
            UnderlineOf(Xlsx.Child(font, "u")),
            Toggle(Xlsx.Child(font, "strike")),
            DeclaredClassOf(font));
    }

    /// <summary>
    /// The generic family a <c>&lt;font&gt;</c> declares, for a family nobody has installed.
    /// </summary>
    /// <remarks>
    /// <c>&lt;family val="N"/&gt;</c>, whose N is the Windows <c>FF_*</c> code — the same one BIFF's
    /// <c>FONT</c> record carries as a byte, which is why both go through
    /// <see cref="SheetDeclaredFonts.FromWindowsCode"/>. Excel writes it on nearly every font it
    /// emits and it has no effect at all until the name fails to resolve, at which point it is the
    /// whole answer.
    /// </remarks>
    private static FontFamilyClass DeclaredClassOf(XElement font)
        => Xlsx.Attribute(Xlsx.Child(font, "family"), "val") is { } stated
           && int.TryParse(stated, NumberStyles.Integer, CultureInfo.InvariantCulture, out int code)
            ? SheetDeclaredFonts.FromWindowsCode(code)
            : FontFamilyClass.Unknown;

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
    internal static SheetUnderline UnderlineOf(XElement? element) => element is null
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

    // ----------------------------------------------------------------------------- resolving

    /// <summary>What one <c>indent</c> level is worth, measured in the default font.</summary>
    private static Length IndentUnit(List<Font> fonts)
    {
        if (fonts.Count == 0) return Length.FromTwips(FallbackTwipsPerIndentLevel);

        Font first = fonts[0];
        SheetCellFormat probe = new()
        {
            FontFamily = first.Family,
            DeclaredFontClass = first.DeclaredClass,
            FontSize = first.Size,
            FontWeight = first.Weight,
            IsItalic = first.Italic,
        };

        if (SheetFonts.For(probe) is not { } face)
            return Length.FromTwips(FallbackTwipsPerIndentLevel);

        // Rounded to the nearest whole twip, because that is the unit LibreOffice's own
        // measurement lands in before it is multiplied: XFont::getCharWidth is
        // OutputDevice::GetTextWidth cast to sal_Int16 (toolkit/source/awt/vclxfont.cxx:77), so
        // the space is a whole number of twips and the only question is which way it goes.
        //
        // It rounds. Measured over the six default font sizes at which Liberation Sans' 5.5566
        // twips per point separate floor from round -- 10, 12, 14, 16, 28 and 30 pt, one
        // workbook each, indent against no indent in the same column so the pen difference is
        // the indent and nothing else (probes/advance-ppem/indent-twip-rounding.py):
        // 26.2.4.2 rounds at 6 of 6 and 24.2.7.2 at 4 of 6, and truncating is wrong at every
        // one of the six against the target. This used to truncate, which was calibrated on the
        // two sizes where 24.2.7.2 happens to agree with it.
        long space = SheetText.Measure(" ", face, first.Size).Twips;
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
            DeclaredFontClass = font.DeclaredClass,
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
