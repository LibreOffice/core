using System.Globalization;
using System.Xml.Linq;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>A colour as an <c>rPr</c> states it, before a palette or a theme has resolved it.</summary>
/// <param name="Rgb">An explicit ARGB value's RGB half.</param>
/// <param name="Indexed">An index into the workbook's palette.</param>
/// <param name="Theme">An index into the theme's colour scheme.</param>
/// <param name="Tint">How far towards white or black the chosen colour is shifted.</param>
internal readonly record struct XlsxRunColour(uint? Rgb, int? Indexed, int? Theme, double Tint);

/// <summary>What one formatting run of a rich string states about its font.</summary>
/// <remarks>
/// Every field is optional and an absent one means "keep the cell's". An <c>rPr</c> is a full
/// <c>CT_Font</c> by schema but is written as a delta by every producer, and reading an absent
/// <c>b</c> as "not bold" would make a run inside a bold cell regular.
/// </remarks>
/// <param name="Family">The face name, from <c>rFont</c> or <c>name</c>.</param>
/// <param name="Points">The em size in points.</param>
/// <param name="Bold">Whether the run is bold.</param>
/// <param name="Italic">Whether it is italic.</param>
/// <param name="Colour">Its colour, unresolved.</param>
internal sealed record XlsxRunFont(
    string? Family, double? Points, bool? Bold, bool? Italic, XlsxRunColour? Colour);

/// <summary>One stretch of a rich string, as character offsets into the flattened text.</summary>
/// <param name="Start">Its first character.</param>
/// <param name="Length">How many characters it covers.</param>
/// <param name="Font">What it states about its font, or null when it states nothing.</param>
internal sealed record XlsxRichRun(int Start, int Length, XlsxRunFont? Font);

/// <summary>
/// The formatting runs of a rich string: <c>&lt;si&gt;</c>, <c>&lt;is&gt;</c> and their
/// <c>&lt;r&gt;</c> children.
/// </summary>
/// <remarks>
/// <para>
/// Parsed into a neutral record here rather than resolved against the workbook's palette, because
/// the shared string table is read during <em>extraction</em> and the palette belongs to the
/// rendering half of <c>styles.xml</c>. Keeping the two apart is what lets the table hold the runs
/// without the extraction path ever loading a font or a theme; resolving them costs nothing until
/// a page is drawn.
/// </para>
/// <para>
/// The offsets are into the same flattened text
/// <see cref="XlsxSharedStrings.ReadRichString(XElement?)"/> produces, so the two must walk the
/// element in the same order and drop the same things — the phonetic <c>rPh</c> guides above all,
/// which are shown above the text rather than in it.
/// </para>
/// </remarks>
internal static class XlsxRichRuns
{
    /// <summary>
    /// The runs of a rich string, or null when it is all one format.
    /// </summary>
    /// <remarks>
    /// Null for the common shapes — a bare <c>t</c>, or a single <c>r</c> stating nothing — so that
    /// a workbook whose strings are all plain records no rich cells at all.
    /// </remarks>
    /// <param name="element">The <c>si</c> or <c>is</c> element.</param>
    public static IReadOnlyList<XlsxRichRun>? Read(XElement? element)
    {
        if (element is null) return null;

        List<XlsxRichRun> runs = [];
        int at = 0;
        bool stated = false;

        foreach (XElement child in element.Elements())
        {
            if (Xlsx.Is(child, "t"))
            {
                at += child.Value.Length;
            }
            else if (Xlsx.Is(child, "r"))
            {
                int length = 0;
                foreach (XElement text in Xlsx.Children(child, "t")) length += text.Value.Length;
                if (length == 0) continue;

                XlsxRunFont? font = ReadFont(Xlsx.Child(child, "rPr"));
                if (font is not null) stated = true;

                runs.Add(new XlsxRichRun(at, length, font));
                at += length;
            }
        }

        return stated ? runs : null;
    }

    private static XlsxRunFont? ReadFont(XElement? properties)
    {
        if (properties is null) return null;

        string? family = Xlsx.Attribute(Xlsx.Child(properties, "rFont"), "val")
                         ?? Xlsx.Attribute(Xlsx.Child(properties, "name"), "val");

        double? points = Number(Xlsx.Child(properties, "sz"), "val");
        bool? bold = Toggle(Xlsx.Child(properties, "b"));
        bool? italic = Toggle(Xlsx.Child(properties, "i"));
        XlsxRunColour? colour = ReadColour(Xlsx.Child(properties, "color"));

        return family is null && points is null && bold is null && italic is null && colour is null
            ? null
            : new XlsxRunFont(family, points, bold, italic, colour);
    }

    /// <summary>
    /// A toggle element such as <c>&lt;b/&gt;</c>, whose absence and whose <c>val="0"</c> differ.
    /// </summary>
    /// <remarks>
    /// Null for absent rather than false, which is the whole point of reading a run as a delta: a
    /// run inside a bold cell that states nothing stays bold, and one that states <c>val="0"</c>
    /// turns the bold off.
    /// </remarks>
    private static bool? Toggle(XElement? element)
        => element is null
            ? null
            : Xlsx.Attribute(element, "val") is not { } value || value is not ("0" or "false");

    private static XlsxRunColour? ReadColour(XElement? element)
    {
        if (element is null) return null;

        double tint = Number(element, "tint") ?? 0;

        if (Xlsx.Attribute(element, "rgb") is { Length: >= 6 } rgb
            && uint.TryParse(
                rgb[^6..], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out uint value))
        {
            return new XlsxRunColour(value, null, null, tint);
        }

        if (Xlsx.Integer(element, "indexed") is { } indexed)
            return new XlsxRunColour(null, indexed, null, tint);

        if (Xlsx.Integer(element, "theme") is { } theme)
            return new XlsxRunColour(null, null, theme, tint);

        // auto="1" is the window text colour, which is black on every printed page — and stating
        // it is stating something, so it is not the same as an absent element.
        return Xlsx.Flag(element, "auto") ? new XlsxRunColour(0, null, null, 0) : null;
    }

    private static double? Number(XElement? element, string name)
        => element is not null
           && Xlsx.Attribute(element, name) is { } text
           && double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture, out double value)
            ? value
            : null;
}
