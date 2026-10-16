using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Containers;
using XRay.Office.Containers.Ooxml;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.Ooxml;

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
/// through <c>Unit::ScreenX</c>, which is 96 per inch.
/// </para>
/// <para>
/// [24.2.7-audit: VERIFIED 2026-08-21, round sheets-r57 — 96 dpi exactly on 26.2.4.2, with the
/// control first and a slope over three steps.] <c>probes/sheets-r57/audit_vmlanchor.py</c>:
/// eight 60 pt rows and one shown comment whose anchor's row offset is the only thing that
/// varies. The control — offset 0 against a row-2 anchor — puts the exported annotation at
/// <strong>119.988 pt</strong> against the 120.0 the row grid alone predicts, so the fixture is
/// read correctly before any law is fitted; the offsets 20, 40 and 60 px then step the annotation
/// by <strong>14.998, 14.998 and 14.990 pt</strong>, which is 96.0, 96.0 and 96.1 implied dots
/// per inch. Seventy-two would have given 20 pt a step and EMUs nothing.
/// </para>
/// <para>
/// <strong>And a rule the site did not state, which authoring the probe exposed: 26.2.4.2 clamps
/// the offset to the anchored cell's own extent.</strong> A row offset of 200 or 400 px into a
/// 60 pt (80 px) row both land on exactly the next row's top — 179.885 pt against a row-3 top of
/// 180.0 — and <c>XlsxVml.ParseAnchor</c> converts the offset without clamping it. <strong>Recorded
/// and not implemented</strong>, because the clamp needs the sheet's grid at anchor-resolution
/// time and because the corpus barely exercises it:
/// <c>probes/sheets-r57/census-vmlclamp.py</c> resolves each VML part to its own worksheet and
/// finds <strong>5 anchors of 365, in one document of fifteen</strong>
/// (<c>023_Waterfall_Chart_Template_for_Excel</c>), overshooting their own cell, the worst by 20 %.
/// The first cut of that census answered <em>zero</em> because it compared every anchor against
/// the tallest row anywhere in the workbook — a bound generous enough that nothing can exceed it.
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

    private const string VmlNamespace = XlsxVml.Namespace;

    /// <summary>The <c>x:</c> namespace, VML's Excel extensions.</summary>
    private const string VmlExcelNamespace = XlsxVml.ExcelNamespace;

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
                if (XlsxVml.ParseAnchor(anchor) is not { } placed) continue;

                shapes[(atColumn, atRow)] = new NoteShape(
                    placed.From,
                    placed.To,
                    XlsxVml.IsVisible(shape.Attribute("style")?.Value),
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
            // ST_Xstring, exactly as in a shared string: a comment's `text` is a CT_Rst too, and
            // decoding it in two of the three places that read one is how the third drifts.
            string content = XlsxCellText.Of(Xlsx.Child(run, "t")?.Value);
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
            && XlsxCellText.Of(Xlsx.Child(text, "t")?.Value) is { Length: > 0 } plain)
        {
            current.Add(new SheetShapeRun(plain, DefaultSize));
        }

        paragraphs.Add(Paragraph(current, DefaultSize, null));

        return new SheetShapeText
        {
            Paragraphs = paragraphs,
            LeftInset = CaptionInset,
            RightInset = CaptionInset,
            TopInset = CaptionInset,
            BottomInset = CaptionInset,
        };
    }

    /// <summary>How far a caption's text sits inside its box, on every side.</summary>
    /// <remarks>
    /// A hundredth-millimetre hundred — one millimetre — on all four sides, which is what Calc's
    /// own <c>Note</c> frame style states (<c>makeSdrTextLeftDistItem(100)</c> and its three
    /// siblings, <c>ScDrawLayer::CreateDefaultStyles</c>,
    /// <c>sc/source/core/data/drwlayer.cxx:391-394</c>). That is not the 250/125 an ordinary
    /// drawing text object defaults to, and the difference shows: measured on the fixture, using
    /// the default put the first glyph 4.3 pt right of where LibreOffice draws it.
    /// </remarks>
    private static Length CaptionInset { get; } = Length.FromMm100(100);

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
