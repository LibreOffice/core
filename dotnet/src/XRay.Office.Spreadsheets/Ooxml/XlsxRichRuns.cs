using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.Ooxml;

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
/// <param name="Underline">The line under the run, or null when it states none.</param>
/// <param name="StruckThrough">Whether the run is struck through.</param>
/// <param name="DeclaredFamily">
/// The Windows <c>FF_*</c> code the run's <c>&lt;family val="N"/&gt;</c> states, or null when it
/// states none. Carried because an <c>rPr</c> may name a face the cell's own font does not, and the
/// declaration is what decides the fallback when that face is not installed.
/// </param>
/// <remarks>
/// <see cref="Underline"/> and <see cref="StruckThrough"/> were absent from this record for
/// several rounds while the same two properties were read on the <em>cell</em> path, so an
/// <c>rPr</c> stating <c>&lt;u/&gt;</c> was parsed, discarded, and drawn as plain text. Found by
/// looking at a page: a reviewer given a German price table reported one cell's first line
/// underlined in the reference and not in ours. 13 of the corpus's 109 workbooks with a shared
/// string table state <c>&lt;u&gt;</c> on a run and 6 state <c>&lt;strike&gt;</c> — an upper bound,
/// since a shared string no cell references is still in the table.
///
/// It moves no words, so no gate column can see it, which is why it survived.
///
/// <see cref="DeclaredFamily"/> arrived by the same route from the other direction: three fields
/// of a <c>CT_Font</c> were being read and four were not. The pattern is worth naming — this record
/// is a <em>subset</em> of a schema type, and every field left out of it is silently discarded
/// rather than failing anywhere. <c>vertAlign</c> is the one still missing, and it is left out
/// deliberately: a raised run has nowhere to store its magnitude and must also make its line
/// taller, so it needs a round rather than a field.
/// </remarks>
internal sealed record XlsxRunFont(
    string? Family,
    double? Points,
    bool? Bold,
    bool? Italic,
    XlsxRunColour? Colour,
    SheetUnderline? Underline = null,
    bool? StruckThrough = null,
    int? DeclaredFamily = null);

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
/// <para>
/// That is why the lengths here are measured after <see cref="XlsxCellText.Of"/> and not on the
/// raw <c>t</c> value. Decoding <c>ST_Xstring</c> on one side only would leave every run in a
/// rich string that holds one <c>_x000D_</c> pointing seven characters too far right, so a fix
/// aimed at the text would come back as mis-formatted runs.
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
                at += XlsxCellText.Of(child.Value).Length;
            }
            else if (Xlsx.Is(child, "r"))
            {
                int length = 0;
                foreach (XElement text in Xlsx.Children(child, "t"))
                    length += XlsxCellText.Of(text.Value).Length;
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
        int? declared = Number(Xlsx.Child(properties, "family"), "val") is { } code
            ? (int)code
            : null;

        // Null for an absent <u>, which is not the same as SheetUnderline.None: absent means
        // "keep what the run inherits" and an explicit val="none" turns an inherited line off.
        // Collapsing the two would leave every underlined run plain, which is the bug this fixes.
        SheetUnderline? underline = Xlsx.Child(properties, "u") is { } stated
            ? XlsxCellFormats.UnderlineOf(stated)
            : null;
        bool? struckThrough = Toggle(Xlsx.Child(properties, "strike"));

        // Every field, or a run stating only the one left out is read as stating nothing and is
        // discarded whole. The merge that brought DeclaredFamily and Underline together in this
        // record initially dropped `declared` from this guard, which would have lost exactly the
        // <family>-only run the field exists for.
        return family is null && points is null && bold is null && italic is null && colour is null
               && underline is null && struckThrough is null && declared is null
            ? null
            : new XlsxRunFont(
                family, points, bold, italic, colour, underline, struckThrough, declared);
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
