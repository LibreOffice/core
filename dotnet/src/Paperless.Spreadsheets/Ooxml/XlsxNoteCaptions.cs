using System.Globalization;
using System.Xml.Linq;
using Paperless.Containers;
using Paperless.Containers.Ooxml;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// The cell comments a sheet shows on the page, as drawings.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A shown comment is an object on the sheet, not a note listed at the end.</strong> The
/// two are different features and both exist: <see cref="SheetNotes"/> is Excel's "comments at end
/// of sheet", which prints an address and a note on pages of its own, while this is the caption a
/// user pinned open, which Calc keeps in the drawing layer and prints where it sits
/// (<c>PrintDrawingLayer(SC_LAYER_INTERN)</c>, <c>sc/source/ui/view/printfun.cxx:1713</c>). A
/// workbook can have neither, either or both.
/// </para>
/// <para>
/// <strong>Visibility is the VML shape's CSS, not the <c>x:Visible</c> element beside it.</strong>
/// <c>Comment::finalizeImport</c> reads <c>pVmlNoteShape-&gt;getTypeModel().mbVisible</c>
/// (<c>sc/source/filter/oox/commentsbuffer.cxx:257</c>), which is
/// <c>style='…;visibility:visible'</c>. Excel writes <c>&lt;x:Visible/&gt;</c> on shapes it also
/// marks <c>visibility:hidden</c> — three of the four notes on one corpus sheet do exactly that —
/// so keying on the element shows comments the reference does not.
/// </para>
/// <para>
/// <strong>The rectangle comes from <c>x:Anchor</c>, and only from the CSS when there is
/// none.</strong> <c>ShapeBase::calcShapeRectangle</c> tries the client anchor first and falls
/// back to the style's <c>margin-left</c> and friends
/// (<c>oox/source/vml/vmlshape.cxx:509-517</c>). The two disagree on real files: on
/// <c>Application_Compliance_Checklist_5_Apr_2021.xlsx</c> the CSS puts the first caption at
/// 1013.25 pt and the anchor at 1067.6 pt, and LibreOffice's own flat-ODF export of that workbook
/// reports the anchor's answer.
/// </para>
/// <para>
/// <strong>The anchor's offsets are screen pixels, not EMUs.</strong>
/// <c>ShapeAnchor::importVmlAnchor</c> sets <c>CellAnchorType::Pixel</c>
/// (<c>sc/source/filter/oox/drawingbase.cxx:152-155</c>) and <c>calcCellAnchorEmu</c> scales them
/// through <c>Unit::ScreenX</c>, which is 96 per inch. Checked against LibreOffice 24.2.7.2's own
/// export of the workbook above: all four of its shown captions come back within two hundredths of
/// a millimetre of the rectangle this arithmetic produces, and the CSS rectangle is out by inches.
/// </para>
/// <para>
/// <strong>One known difference, and it is in the height.</strong> Calc freezes the caption's size
/// at import — <c>ScNoteUtil::CreateNoteData</c> stores <c>maCaptionSize</c>
/// (<c>sc/source/core/data/postit.cxx:973</c>) and the caption is placed later at that size
/// relative to its cell — so a caption spanning rows whose heights the optimal-height pass then
/// changes keeps the height the file's own <c>ht</c> values gave it. Here the anchor is resolved
/// against the grid the page is drawn on, so such a caption is as tall as its rows end up. It
/// moves no text and it is not corrected, because separating the two grids costs a second
/// resolution of every sheet's geometry.
/// </para>
/// </remarks>
internal static class XlsxNoteCaptions
{
    private const string RelationshipNamespace =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    private const string VmlNamespace = OoxmlNamespaces.Vml;

    /// <summary>The <c>x:</c> namespace, VML's Excel extensions.</summary>
    private const string VmlExcelNamespace = "urn:schemas-microsoft-com:office:excel";

    /// <summary>How many of a VML anchor's pixels make an inch.</summary>
    /// <remarks>
    /// <c>UnitConverter</c>'s <c>Unit::ScreenX</c>, which is the reference device's resolution and
    /// is 96 on every platform Calc runs headless on. Measured rather than assumed: an anchor
    /// stating a row offset of 111 pixels comes back as 83.25 pt in the flat-ODF export, and
    /// 111 / 96 x 72 is 83.25 exactly.
    /// </remarks>
    private const double PixelsPerInch = 96;

    /// <summary>The fill Excel gives a comment caption when its VML states none.</summary>
    private static readonly Colour DefaultFill = Colour.FromRgb(0xFFFFE1);

    /// <summary>Reads the shown comments of one sheet as drawings.</summary>
    /// <param name="package">The workbook's package.</param>
    /// <param name="sheetPartName">The worksheet part the comments hang off.</param>
    /// <returns>One drawing per shown comment, in the order the comments part lists them.</returns>
    public static List<SheetDrawing> Read(IPackage package, string? sheetPartName)
    {
        ArgumentNullException.ThrowIfNull(package);

        List<SheetDrawing> captions = [];
        if (sheetPartName is null || package is not OpcPackage opc) return captions;

        Dictionary<(int Column, int Row), NoteShape> shapes = ReadShapes(opc, sheetPartName);
        if (shapes.Count == 0) return captions;

        foreach (XElement comment in Comments(opc, sheetPartName))
        {
            if (!SheetAddress.TryParseCell(Xlsx.Attribute(comment, "ref"),
                                           out int column, out int row))
            {
                continue;
            }

            if (!shapes.TryGetValue((column, row), out NoteShape shape)) continue;
            if (!shape.IsVisible) continue;

            SheetShapeText text = CaptionText(Xlsx.Child(comment, "text"));
            if (text.IsEmpty) continue;

            captions.Add(new SheetDrawing
            {
                Anchor = SheetAnchorKind.TwoCell,
                From = shape.From,
                To = shape.To,
                Text = text,
                Fill = shape.Fill ?? DefaultFill,
                Stroke = Colour.Black,
                NoteCell = (column, row),
                Name = "Comment",
            });
        }

        return captions;
    }

    /// <summary>Every comment the sheet's legacy comments part lists.</summary>
    private static IEnumerable<XElement> Comments(OpcPackage package, string sheetPartName)
    {
        foreach (OpcXml.Relationship relationship in package.GetRelationshipsByType(
                     RelationshipNamespace + "/comments", sheetPartName))
        {
            if (relationship.IsExternal) continue;
            if (package.GetPart(relationship.Target) is not { } part) continue;

            XElement? root;
            using (Stream content = part.Open())
            {
                root = OoxmlXml.TryLoad(content, out _);
            }

            if (root is null) continue;
            foreach (XElement comment in Xlsx.Children(Xlsx.Child(root, "commentList"), "comment"))
                yield return comment;
        }
    }

    /// <summary>One VML note shape: where it sits, whether it is shown, and its fill.</summary>
    private readonly record struct NoteShape(
        SheetCellPoint From, SheetCellPoint To, bool IsVisible, Colour? Fill);

    /// <summary>
    /// The note shapes of a sheet's legacy VML drawings, indexed by the cell they belong to.
    /// </summary>
    /// <remarks>
    /// Keyed on <c>x:Row</c> and <c>x:Column</c>, which is what
    /// <c>VmlDrawing::buildNoteShapesMap</c> keys on, rather than on the shape's own order — a
    /// sheet's VML lists its notes in z-order and its comments part in address order, and the two
    /// agree on nothing.
    /// </remarks>
    private static Dictionary<(int, int), NoteShape> ReadShapes(
        OpcPackage package, string sheetPartName)
    {
        Dictionary<(int, int), NoteShape> shapes = [];

        foreach (OpcXml.Relationship relationship in package.GetRelationshipsByType(
                     RelationshipNamespace + "/vmlDrawing", sheetPartName))
        {
            if (relationship.IsExternal) continue;
            if (package.GetPart(relationship.Target) is not { } part) continue;

            XElement? root;
            using (Stream content = part.Open())
            {
                // A VML part is not namespace-well-formed by XML's rules — Excel writes bare
                // `<xml>` roots with undeclared prefixes on some producers — so a failed load is
                // ordinary and silent, as it is for every other optional part.
                root = OoxmlXml.TryLoad(content, out _);
            }

            if (root is null) continue;

            foreach (XElement shape in root.Descendants(XName.Get("shape", VmlNamespace)))
            {
                XElement? client = shape.Element(XName.Get("ClientData", VmlExcelNamespace));
                if (client is null) continue;
                if (!string.Equals(client.Attribute("ObjectType")?.Value, "Note",
                                   StringComparison.Ordinal))
                {
                    continue;
                }

                int? column = Integer(client, "Column");
                int? row = Integer(client, "Row");
                if (column is not { } atColumn || row is not { } atRow) continue;

                string? anchor = client.Element(XName.Get("Anchor", VmlExcelNamespace))?.Value;
                if (ParseAnchor(anchor) is not { } placed) continue;

                shapes[(atColumn, atRow)] = new NoteShape(
                    placed.From,
                    placed.To,
                    IsVisible(shape.Attribute("style")?.Value),
                    Fill(shape.Attribute("fillcolor")?.Value));
            }
        }

        return shapes;
    }

    /// <summary>An <c>x:ClientData</c> child parsed as an integer, or null.</summary>
    private static int? Integer(XElement client, string localName)
        => int.TryParse(client.Element(XName.Get(localName, VmlExcelNamespace))?.Value,
                        NumberStyles.Integer, CultureInfo.InvariantCulture, out int value)
            ? value
            : null;

    /// <summary>
    /// Whether a VML shape's style says it is shown.
    /// </summary>
    /// <remarks>
    /// A shape stating no <c>visibility</c> at all is shown, which is CSS's own default and what
    /// <c>ShapeTypeModel</c> initialises <c>mbVisible</c> to
    /// (<c>oox/source/vml/vmlshape.cxx</c>, <c>ShapeTypeModel::ShapeTypeModel</c>).
    /// </remarks>
    private static bool IsVisible(string? style)
    {
        if (style is null) return true;

        foreach (string declaration in style.Split(';'))
        {
            int colon = declaration.IndexOf(':', StringComparison.Ordinal);
            if (colon < 0) continue;

            if (!declaration.AsSpan(0, colon).Trim().Equals("visibility", StringComparison.Ordinal))
                continue;

            return !declaration.AsSpan(colon + 1).Trim().Equals("hidden", StringComparison.Ordinal);
        }

        return true;
    }

    /// <summary>A VML <c>fillcolor</c>, or null when it names nothing this understands.</summary>
    private static Colour? Fill(string? value)
    {
        if (value is null) return null;

        ReadOnlySpan<char> text = value.AsSpan().Trim();
        if (text.Length != 7 || text[0] != '#') return null;

        return uint.TryParse(text[1..], NumberStyles.HexNumber, CultureInfo.InvariantCulture,
                             out uint rgb)
            ? Colour.FromRgb(rgb)
            : null;
    }

    /// <summary>
    /// The eight comma-separated numbers of an <c>x:Anchor</c>, as two cell points.
    /// </summary>
    /// <remarks>
    /// The order is column, column offset, row, row offset, twice —
    /// <c>ShapeAnchor::importVmlAnchor</c> assigns them in exactly that sequence
    /// (<c>drawingbase.cxx:157-172</c>) — which is <em>not</em> the row-first order the
    /// <c>x:Row</c> and <c>x:Column</c> elements above it are written in.
    /// </remarks>
    private static (SheetCellPoint From, SheetCellPoint To)? ParseAnchor(string? anchor)
    {
        if (anchor is null) return null;

        string[] parts = anchor.Split(',');
        if (parts.Length < 8) return null;

        Span<int> values = stackalloc int[8];
        for (int at = 0; at < 8; at++)
        {
            if (!int.TryParse(parts[at].Trim(), NumberStyles.Integer,
                              CultureInfo.InvariantCulture, out values[at]))
            {
                return null;
            }
        }

        if (values[0] < 0 || values[2] < 0 || values[4] < 0 || values[6] < 0) return null;

        return (new SheetCellPoint(values[0], Pixels(values[1]), values[2], Pixels(values[3])),
                new SheetCellPoint(values[4], Pixels(values[5]), values[6], Pixels(values[7])));
    }

    /// <summary>A screen pixel offset as a length.</summary>
    private static Length Pixels(int pixels)
        => Length.FromInches(pixels / PixelsPerInch);

    /// <summary>
    /// A comment's rich text as the paragraphs of a caption.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A comment states its own face and size per run, and both are kept: the caption's width is
    /// the anchor's, so what decides how many lines it takes is where the text wraps, and a body
    /// measured at a default size wraps somewhere else. On the workbook above the runs alternate
    /// 12 pt and 9 pt Tahoma inside one comment, and LibreOffice's export carries both.
    /// </para>
    /// <para>
    /// A newline inside a run's <c>t</c> starts a paragraph. Excel writes the author's name and
    /// the note as one string separated that way rather than as two elements, so splitting here is
    /// what keeps them on two lines.
    /// </para>
    /// </remarks>
    private static SheetShapeText CaptionText(XElement? text)
    {
        List<SheetShapeParagraph> paragraphs = [];
        List<SheetShapeRun> current = [];

        foreach (XElement run in Xlsx.Children(text, "r"))
        {
            string content = Xlsx.Child(run, "t")?.Value ?? string.Empty;
            if (content.Length == 0) continue;

            XElement? properties = Xlsx.Child(run, "rPr");
            Length size = Points(properties);
            string? family = Xlsx.Attribute(Xlsx.Child(properties, "rFont"), "val");

            string[] lines = content.Replace("\r\n", "\n", StringComparison.Ordinal)
                                    .Split(['\n', '\r']);
            for (int at = 0; at < lines.Length; at++)
            {
                if (at > 0)
                {
                    paragraphs.Add(Paragraph(current, size, family));
                    current = [];
                }

                if (lines[at].Length > 0) current.Add(new SheetShapeRun(lines[at], size, family));
            }
        }

        // A comment stating its text as a bare `t` rather than as runs, which the schema allows
        // and a few producers write.
        if (paragraphs.Count == 0 && current.Count == 0
            && Xlsx.Child(text, "t")?.Value is { Length: > 0 } plain)
        {
            current.Add(new SheetShapeRun(plain, DefaultSize));
        }

        paragraphs.Add(Paragraph(current, DefaultSize, null));

        return new SheetShapeText { Paragraphs = paragraphs };
    }

    /// <summary>One paragraph, with an empty one still carrying the size it would be typed at.</summary>
    private static SheetShapeParagraph Paragraph(
        List<SheetShapeRun> runs, Length size, string? family)
        => new()
        {
            Runs = runs.Count > 0 ? runs : [new SheetShapeRun(string.Empty, size, family)],
        };

    /// <summary>The em size a comment run that states none is set at.</summary>
    /// <remarks>
    /// Ten point, which is the default <c>CT_Font</c> size Excel writes a comment at and what the
    /// EditEngine gives a caption whose runs state nothing.
    /// </remarks>
    private static Length DefaultSize { get; } = Length.FromPoints(10);

    /// <summary>An <c>rPr/sz</c> in points, or the default.</summary>
    private static Length Points(XElement? properties)
        => Xlsx.Double(Xlsx.Attribute(Xlsx.Child(properties, "sz"), "val")) is { } points
           && points > 0
            ? Length.FromPoints(points)
            : DefaultSize;
}
