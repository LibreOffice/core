using System.Xml.Linq;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// The table styles PowerPoint knows by GUID and does not write into a deck's
/// <c>tableStyles.xml</c>.
/// </summary>
/// <remarks>
/// <para>
/// A <c>a:tblPr</c> may name a <c>a:tableStyleId</c> that the package's own <c>a:tblStyleLst</c>
/// does not define. Those are PowerPoint's seventy-four built-in styles: the application has them
/// compiled in and writes only the id. A reader with no table of its own draws the table's text
/// and none of its fills — no header band, no banding, no grid — which is exactly the defect a
/// human review reported on <c>8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx</c> as "missing orange
/// table backgrounds", and which a previous round wrote off as a review made against PowerPoint
/// rather than against the reference.
/// </para>
/// <para>
/// <strong>It was ours.</strong> The reference draws that deck's page 14 with 30 cells at
/// <c>#FBECE7</c> and 25 at <c>#F8D7CD</c> and we draw none of either; the deck's theme has
/// <c>accent2 = ED7D31</c>, and those two colours are exactly accent2 under <c>a:tint</c> 20000 and
/// 40000 through DrawingML's gamma-corrected tint — which is what
/// <c>{21E4AEA4-8DFA-4A89-87EB-49C32662AFE0}</c>, "Medium Style 2 - Accent 2", puts on
/// <c>wholeTbl</c> and on <c>band1H</c>. Its <c>a:tblPr</c> does name a style id; the earlier note
/// that it names none does not reproduce.
/// </para>
/// <para>
/// Ported from <c>oox/source/drawingml/table/predefined-table-styles.cxx</c>, which builds the same
/// styles for the same reason (tdf#107604). This emits <c>a:tblStyle</c> markup and hands it to the
/// ordinary reader rather than building a parallel model, so every part, every merge order and
/// every colour transformation is the one a package-defined style already goes through.
/// </para>
/// <para>
/// <strong>Five of the eleven groups are implemented — the ones the corpus exercises.</strong> The
/// id map is complete, so a style from an unimplemented group is recognised and named rather than
/// silently absent, and adding a group is a table entry rather than a change of shape. Measured
/// over the 163-document slides track: ten decks name a style their package does not define,
/// across thirty tables and eight distinct ids, and those eight fall in the five groups here.
/// </para>
/// <para>
/// One deliberate deviation. LibreOffice gives an explicit 12700 EMU width only to
/// <c>wholeTbl</c>'s six sides and to <c>firstRow</c>'s bottom, and leaves the other borders it
/// switches on at its own default; this emits no <c>w</c> for those, which our reader reads as
/// 12700 as well. It is a sub-point stroke width on a handful of sides and it is not the fills,
/// which are what these styles are for.
/// </para>
/// </remarks>
public static class DrawingPredefinedTableStyles
{
    /// <summary>A built-in style's group and the accent it is themed on, or an empty accent.</summary>
    private readonly record struct Identity(string Group, string Accent);

    /// <summary>
    /// The style-id map, from
    /// <c>learn.microsoft.com</c>'s published list of the seventy-four ids and reproduced by
    /// <c>predefined-table-styles.cxx</c>'s own <c>mStyleIdMap</c>.
    /// </summary>
    private static readonly Dictionary<string, Identity> Map = new(StringComparer.OrdinalIgnoreCase)
    {
        ["{2D5ABB26-0587-4C30-8999-92F81FD0307C}"] = new("Themed-Style-1", ""),
        ["{3C2FFA5D-87B4-456A-9821-1D502468CF0F}"] = new("Themed-Style-1", "accent1"),
        ["{284E427A-3D55-4303-BF80-6455036E1DE7}"] = new("Themed-Style-1", "accent2"),
        ["{69C7853C-536D-4A76-A0AE-DD22124D55A5}"] = new("Themed-Style-1", "accent3"),
        ["{775DCB02-9BB8-47FD-8907-85C794F793BA}"] = new("Themed-Style-1", "accent4"),
        ["{35758FB7-9AC5-4552-8A53-C91805E547FA}"] = new("Themed-Style-1", "accent5"),
        ["{08FB837D-C827-4EFA-A057-4D05807E0F7C}"] = new("Themed-Style-1", "accent6"),
        ["{5940675A-B579-460E-94D1-54222C63F5DA}"] = new("Themed-Style-2", ""),
        ["{D113A9D2-9D6B-4929-AA2D-F23B5EE8CBE7}"] = new("Themed-Style-2", "accent1"),
        ["{18603FDC-E32A-4AB5-989C-0864C3EAD2B8}"] = new("Themed-Style-2", "accent2"),
        ["{306799F8-075E-4A3A-A7F6-7FBC6576F1A4}"] = new("Themed-Style-2", "accent3"),
        ["{E269D01E-BC32-4049-B463-5C60D7B0CCD2}"] = new("Themed-Style-2", "accent4"),
        ["{327F97BB-C833-4FB7-BDE5-3F7075034690}"] = new("Themed-Style-2", "accent5"),
        ["{638B1855-1B75-4FBE-930C-398BA8C253C6}"] = new("Themed-Style-2", "accent6"),
        ["{9D7B26C5-4107-4FEC-AEDC-1716B250A1EF}"] = new("Light-Style-1", ""),
        ["{3B4B98B0-60AC-42C2-AFA5-B58CD77FA1E5}"] = new("Light-Style-1", "accent1"),
        ["{0E3FDE45-AF77-4B5C-9715-49D594BDF05E}"] = new("Light-Style-1", "accent2"),
        ["{C083E6E3-FA7D-4D7B-A595-EF9225AFEA82}"] = new("Light-Style-1", "accent3"),
        ["{D27102A9-8310-4765-A935-A1911B00CA55}"] = new("Light-Style-1", "accent4"),
        ["{5FD0F851-EC5A-4D38-B0AD-8093EC10F338}"] = new("Light-Style-1", "accent5"),
        ["{68D230F3-CF80-4859-8CE7-A43EE81993B5}"] = new("Light-Style-1", "accent6"),
        ["{7E9639D4-E3E2-4D34-9284-5A2195B3D0D7}"] = new("Light-Style-2", ""),
        ["{69012ECD-51FC-41F1-AA8D-1B2483CD663E}"] = new("Light-Style-2", "accent1"),
        ["{72833802-FEF1-4C79-8D5D-14CF1EAF98D9}"] = new("Light-Style-2", "accent2"),
        ["{F2DE63D5-997A-4646-A377-4702673A728D}"] = new("Light-Style-2", "accent3"),
        ["{17292A2E-F333-43FB-9621-5CBBE7FDCDCB}"] = new("Light-Style-2", "accent4"),
        ["{5A111915-BE36-4E01-A7E5-04B1672EAD32}"] = new("Light-Style-2", "accent5"),
        ["{912C8C85-51F0-491E-9774-3900AFEF0FD7}"] = new("Light-Style-2", "accent6"),
        ["{616DA210-FB5B-4158-B5E0-FEB733F419BA}"] = new("Light-Style-3", ""),
        ["{BC89EF96-8CEA-46FF-86C4-4CE0E7609802}"] = new("Light-Style-3", "accent1"),
        ["{5DA37D80-6434-44D0-A028-1B22A696006F}"] = new("Light-Style-3", "accent2"),
        ["{8799B23B-EC83-4686-B30A-512413B5E67A}"] = new("Light-Style-3", "accent3"),
        ["{ED083AE6-46FA-4A59-8FB0-9F97EB10719F}"] = new("Light-Style-3", "accent4"),
        ["{BDBED569-4797-4DF1-A0F4-6AAB3CD982D8}"] = new("Light-Style-3", "accent5"),
        ["{E8B1032C-EA38-4F05-BA0D-38AFFFC7BED3}"] = new("Light-Style-3", "accent6"),
        ["{793D81CF-94F2-401A-BA57-92F5A7B2D0C5}"] = new("Medium-Style-1", ""),
        ["{B301B821-A1FF-4177-AEE7-76D212191A09}"] = new("Medium-Style-1", "accent1"),
        ["{9DCAF9ED-07DC-4A11-8D7F-57B35C25682E}"] = new("Medium-Style-1", "accent2"),
        ["{1FECB4D8-DB02-4DC6-A0A2-4F2EBAE1DC90}"] = new("Medium-Style-1", "accent3"),
        ["{1E171933-4619-4E11-9A3F-F7608DF75F80}"] = new("Medium-Style-1", "accent4"),
        ["{FABFCF23-3B69-468F-B69F-88F6DE6A72F2}"] = new("Medium-Style-1", "accent5"),
        ["{10A1B5D5-9B99-4C35-A422-299274C87663}"] = new("Medium-Style-1", "accent6"),
        ["{073A0DAA-6AF3-43AB-8588-CEC1D06C72B9}"] = new("Medium-Style-2", ""),
        ["{5C22544A-7EE6-4342-B048-85BDC9FD1C3A}"] = new("Medium-Style-2", "accent1"),
        ["{21E4AEA4-8DFA-4A89-87EB-49C32662AFE0}"] = new("Medium-Style-2", "accent2"),
        ["{F5AB1C69-6EDB-4FF4-983F-18BD219EF322}"] = new("Medium-Style-2", "accent3"),
        ["{00A15C55-8517-42AA-B614-E9B94910E393}"] = new("Medium-Style-2", "accent4"),
        ["{7DF18680-E054-41AD-8BC1-D1AEF772440D}"] = new("Medium-Style-2", "accent5"),
        ["{93296810-A885-4BE3-A3E7-6D5BEEA58F35}"] = new("Medium-Style-2", "accent6"),
        ["{8EC20E35-A176-4012-BC5E-935CFFF8708E}"] = new("Medium-Style-3", ""),
        ["{6E25E649-3F16-4E02-A733-19D2CDBF48F0}"] = new("Medium-Style-3", "accent1"),
        ["{85BE263C-DBD7-4A20-BB59-AAB30ACAA65A}"] = new("Medium-Style-3", "accent2"),
        ["{EB344D84-9AFB-497E-A393-DC336BA19D2E}"] = new("Medium-Style-3", "accent3"),
        ["{EB9631B5-78F2-41C9-869B-9F39066F8104}"] = new("Medium-Style-3", "accent4"),
        ["{74C1A8A3-306A-4EB7-A6B1-4F7E0EB9C5D6}"] = new("Medium-Style-3", "accent5"),
        ["{2A488322-F2BA-4B5B-9748-0D474271808F}"] = new("Medium-Style-3", "accent6"),
        ["{D7AC3CCA-C797-4891-BE02-D94E43425B78}"] = new("Medium-Style-4", ""),
        ["{69CF1AB2-1976-4502-BF36-3FF5EA218861}"] = new("Medium-Style-4", "accent1"),
        ["{8A107856-5554-42FB-B03E-39F5DBC370BA}"] = new("Medium-Style-4", "accent2"),
        ["{0505E3EF-67EA-436B-97B2-0124C06EBD24}"] = new("Medium-Style-4", "accent3"),
        ["{C4B1156A-380E-4F78-BDF5-A606A8083BF9}"] = new("Medium-Style-4", "accent4"),
        ["{22838BEF-8BB2-4498-84A7-C5851F593DF1}"] = new("Medium-Style-4", "accent5"),
        ["{16D9F66E-5EB9-4882-86FB-DCBF35E3C3E4}"] = new("Medium-Style-4", "accent6"),
        ["{E8034E78-7F5D-4C2E-B375-FC64B27BC917}"] = new("Dark-Style-1", ""),
        ["{125E5076-3810-47DD-B79F-674D7AD40C01}"] = new("Dark-Style-1", "accent1"),
        ["{37CE84F3-28C3-443E-9E96-99CF82512B78}"] = new("Dark-Style-1", "accent2"),
        ["{D03447BB-5D67-496B-8E87-E561075AD55C}"] = new("Dark-Style-1", "accent3"),
        ["{E929F9F4-4A8F-4326-A1B4-22849713DDAB}"] = new("Dark-Style-1", "accent4"),
        ["{8FD4443E-F989-4FC4-A0C8-D5A2AF1F390B}"] = new("Dark-Style-1", "accent5"),
        ["{AF606853-7671-496A-8E4F-DF71F8EC918B}"] = new("Dark-Style-1", "accent6"),
        ["{5202B0CA-FC54-4496-8BCA-5EF66A818D29}"] = new("Dark-Style-2", ""),
        ["{0660B408-B3CF-4A94-85FC-2B1E0A45F4A2}"] = new("Dark-Style-2", "accent1"),
        ["{91EBBBCC-DAD2-459C-BE2E-F6DE35CF9A28}"] = new("Dark-Style-2", "accent3"),
        ["{46F890A9-2807-4EBB-B81D-B2AA78EC7F39}"] = new("Dark-Style-2", "accent5"),
    };

    /// <summary>The <c>a:tblStyle</c> a built-in style id stands for, or null when it is not one.</summary>
    /// <param name="styleId">The table's <c>a:tblPr/a:tableStyleId</c>.</param>
    /// <returns>
    /// Markup shaped exactly as a <c>tableStyles.xml</c> entry, so it goes through the ordinary
    /// reader; null when the id is unknown or its group is not one of the five implemented here.
    /// </returns>
    public static XElement? Create(string? styleId)
    {
        if (string.IsNullOrEmpty(styleId)) return null;
        if (!Map.TryGetValue(styleId, out Identity identity)) return null;

        List<XElement> parts = identity.Group switch
        {
            "Themed-Style-1" => ThemedStyle1(identity.Accent),
            "Light-Style-2" => LightStyle2(identity.Accent),
            "Medium-Style-2" => MediumStyle2(identity.Accent),
            "Medium-Style-3" => MediumStyle3(identity.Accent),
            "Medium-Style-4" => MediumStyle4(identity.Accent),
            _ => [],
        };

        if (parts.Count == 0) return null;

        return new XElement(
            Drawing.Name("tblStyle"),
            new XAttribute("styleId", styleId),
            new XAttribute("styleName", identity.Group),
            parts);
    }

    /// <summary>True for an id one of PowerPoint's built-in styles, implemented here or not.</summary>
    /// <param name="styleId">The table's <c>a:tblPr/a:tableStyleId</c>.</param>
    public static bool IsKnown(string? styleId)
        => !string.IsNullOrEmpty(styleId) && Map.ContainsKey(styleId);

    /// <summary>The name of a built-in style's group, for a diagnostic.</summary>
    /// <param name="styleId">The table's <c>a:tblPr/a:tableStyleId</c>.</param>
    public static string? GroupOf(string? styleId)
        => !string.IsNullOrEmpty(styleId) && Map.TryGetValue(styleId, out Identity found)
            ? found.Group
            : null;

    private static List<XElement> ThemedStyle1(string accent)
    {
        if (accent.Length == 0) return [Part("wholeTbl", text: "tx1")];

        string[] sides = ["left", "right", "top", "bottom", "insideH", "insideV"];

        return
        [
            Part("wholeTbl", text: "dk1", borders: Borders(sides, accent, wide: true)),
            Part(
                "firstRow",
                text: "lt1",
                fill: Fill(accent),
                borders: [
                    .. Borders(["left", "right", "top"], accent),
                    .. Borders(["bottom"], "lt1", wide: true)]),
            Part("lastRow", borders: Borders(["left", "right", "top", "bottom"], accent)),
            Part("firstCol", borders: Borders(["left", "right", "top", "bottom", "insideH"], accent)),
            Part("lastCol", borders: Borders(["left", "right", "top", "bottom", "insideH"], accent)),

            // The banded fill is the accent at 40% alpha rather than a tint of it, so the slide
            // behind shows through — which is what makes this group "themed" rather than tinted.
            Part("band1H", fill: Fill(accent, alpha: 40000)),
            Part("band1V", fill: Fill(accent, alpha: 40000)),
        ];
    }

    private static List<XElement> LightStyle2(string accent)
    {
        string colour = accent.Length == 0 ? "tx1" : accent;

        return
        [
            Part(
                "wholeTbl",
                text: "tx1",
                borders: Borders(["left", "right", "top", "bottom"], colour, wide: true)),
            Part("firstRow", text: "bg1", fill: Fill(colour)),
            Part("lastRow", borders: Borders(["top"], colour)),
            Part("band1H", borders: Borders(["top", "bottom"], colour)),
            Part("band1V", borders: Borders(["left", "right"], colour)),
            Part("band2V", borders: Borders(["left", "right"], colour)),
        ];
    }

    private static List<XElement> MediumStyle2(string accent)
    {
        string colour = accent.Length == 0 ? "dk1" : accent;
        string[] sides = ["left", "right", "top", "bottom", "insideH", "insideV"];

        return
        [
            Part(
                "wholeTbl",
                text: "dk1",
                fill: Fill(colour, tint: 20000),
                borders: Borders(sides, "lt1", wide: true)),
            Part("firstRow", text: "lt1", fill: Fill(colour), borders: Borders(["bottom"], "lt1", wide: true)),
            Part("lastRow", text: "lt1", fill: Fill(colour), borders: Borders(["top"], "lt1")),
            Part("firstCol", text: "lt1", fill: Fill(colour)),
            Part("lastCol", text: "lt1", fill: Fill(colour)),
            Part("band1H", fill: Fill(colour, tint: 40000)),
            Part("band1V", fill: Fill(colour, tint: 40000)),
        ];
    }

    private static List<XElement> MediumStyle3(string accent)
    {
        string colour = accent.Length == 0 ? "dk1" : accent;

        return
        [
            Part(
                "wholeTbl",
                text: "dk1",
                fill: Fill("lt1"),
                borders: Borders(["top", "bottom"], "dk1", wide: true)),
            Part("firstRow", text: "lt1", fill: Fill(colour), borders: Borders(["bottom"], "dk1", wide: true)),
            Part("lastRow", fill: Fill("lt1"), borders: Borders(["top"], "dk1")),
            Part("firstCol", text: "lt1", fill: Fill(colour)),
            Part("lastCol", text: "lt1", fill: Fill(colour)),
            Part("band1H", fill: Fill("dk1", tint: 20000)),
            Part("band1V", fill: Fill("dk1", tint: 20000)),
        ];
    }

    private static List<XElement> MediumStyle4(string accent)
    {
        string colour = accent.Length == 0 ? "dk1" : accent;
        string[] sides = ["left", "right", "top", "bottom", "insideH", "insideV"];

        return
        [
            Part(
                "wholeTbl",
                text: "dk1",
                fill: Fill(colour, tint: 20000),
                borders: Borders(sides, colour, wide: true)),

            // LibreOffice colours lastRow's top border and never switches it on, so it is not
            // part of the style; its fill is. Emitting the border anyway is the easiest way to
            // get this group visibly wrong.
            Part("lastRow", fill: Fill("dk1", tint: 20000)),
            Part("firstRow", text: colour, fill: Fill(colour, tint: 20000)),
            Part("band1H", fill: Fill(colour, tint: 40000)),
            Part("band1V", fill: Fill(colour, tint: 40000)),
        ];
    }

    /// <summary>One <c>a:tblStyle</c> part — <c>a:wholeTbl</c>, <c>a:band1H</c> and the rest.</summary>
    private static XElement Part(
        string name,
        string? text = null,
        XElement? fill = null,
        List<XElement>? borders = null)
    {
        List<object> children = [];

        if (text is not null)
        {
            children.Add(new XElement(
                Drawing.Name("tcTxStyle"),
                new XElement(Drawing.Name("schemeClr"), new XAttribute("val", text))));
        }

        List<object> cellStyle = [];
        if (borders is { Count: > 0 }) cellStyle.Add(new XElement(Drawing.Name("tcBdr"), borders));
        if (fill is not null) cellStyle.Add(fill);

        if (cellStyle.Count > 0) children.Add(new XElement(Drawing.Name("tcStyle"), cellStyle));

        return new XElement(Drawing.Name(name), children);
    }

    /// <summary>A part's <c>a:fill</c>, as a scheme colour under an optional tint or alpha.</summary>
    private static XElement Fill(string scheme, int? tint = null, int? alpha = null)
        => new(
            Drawing.Name("fill"),
            new XElement(Drawing.Name("solidFill"), SchemeColour(scheme, tint, alpha)));

    private static XElement SchemeColour(string scheme, int? tint = null, int? alpha = null)
    {
        XElement colour = new(Drawing.Name("schemeClr"), new XAttribute("val", scheme));

        if (tint is { } t) colour.Add(new XElement(Drawing.Name("tint"), new XAttribute("val", t)));
        if (alpha is { } a) colour.Add(new XElement(Drawing.Name("alpha"), new XAttribute("val", a)));

        return colour;
    }

    /// <summary>The named sides of one part's <c>a:tcBdr</c>, all one colour.</summary>
    /// <param name="sides">Which of <c>left right top bottom insideH insideV</c>.</param>
    /// <param name="scheme">The scheme colour the lines take.</param>
    /// <param name="wide">
    /// True for the sides LibreOffice gives an explicit 12700 EMU and a solid dash. The rest are
    /// left at the reader's default rather than stated, which comes to the same width.
    /// </param>
    private static List<XElement> Borders(IEnumerable<string> sides, string scheme, bool wide = false)
    {
        List<XElement> borders = [];

        foreach (string side in sides)
        {
            XElement line = new(
                Drawing.Name("ln"),
                new XElement(Drawing.Name("solidFill"), SchemeColour(scheme)));

            if (wide)
            {
                line.SetAttributeValue("w", 12700);
                line.Add(new XElement(Drawing.Name("prstDash"), new XAttribute("val", "solid")));
            }

            borders.Add(new XElement(Drawing.Name(side), line));
        }

        return borders;
    }
}
