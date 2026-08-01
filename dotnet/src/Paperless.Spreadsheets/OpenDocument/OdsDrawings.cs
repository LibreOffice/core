using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.Spreadsheets.Layout;
using Paperless.Vector;

namespace Paperless.Spreadsheets.OpenDocument;

/// <summary>
/// The pictures anchored on one ODF sheet: <c>draw:frame</c> inside a <c>table:table-cell</c>.
/// </summary>
/// <remarks>
/// <para>
/// ODF anchors a drawing by <em>containment</em> where SpreadsheetML anchors it by address: the
/// frame is a child of the cell it is fastened to, and <c>svg:x</c> and <c>svg:y</c> are measured
/// from that cell's top-left corner. So the walk that finds the drawings is the same walk that
/// counts the columns, and the repeat counts have to be honoured on the way or every frame after
/// a repeated column lands in the wrong place.
/// </para>
/// <para>
/// <strong>The end cell wins over the stated size, and that is measurable.</strong> A frame may
/// carry <c>table:end-cell-address</c> with <c>table:end-x</c> and <c>table:end-y</c> as well as
/// <c>svg:width</c> and <c>svg:height</c>; the first is ODF's "resize with the cells" anchor,
/// Calc's <c>ScDrawObjData</c>. When the two disagree Calc believes the end cell and rewrites the
/// lengths: a hand-written frame stating <c>svg:width="1.28in"</c> and ending at C3 is saved back
/// as <c>svg:width="1.3201in"</c>, which is the two columns it spans less its own start offset.
/// So a frame with an end address is read as a two-cell anchor and one without it as a one-cell
/// anchor, which is exactly the distinction the attribute exists to make.
/// </para>
/// <para>
/// <strong>Two ways to hold the bytes, and a flat file needs the second.</strong> A packaged ODS
/// writes <c>xlink:href="Pictures/…"</c>; a flat one writes the bytes inline as base64 in
/// <c>office:binary-data</c>, because there is no package to put them in. Both are read, and
/// neither is decoded — <see cref="RasterImage.Encoded"/> carries the file's own bytes to whichever
/// backend wants pixels.
/// </para>
/// </remarks>
internal static class OdsDrawings
{
    /// <summary>How many repeats of one row or column element are walked into.</summary>
    /// <remarks>The same cap the format readers use, and for the same reason.</remarks>
    private const int MaxRepeat = 4096;

    /// <summary>Reads the drawings anchored on one sheet.</summary>
    /// <param name="file">The document, for its package.</param>
    /// <param name="table">The <c>table:table</c> element.</param>
    public static SheetDrawings Read(OdfFile file, XElement table)
    {
        ArgumentNullException.ThrowIfNull(file);
        ArgumentNullException.ThrowIfNull(table);

        List<SheetDrawing> drawings = [];
        int row = 0;

        foreach (XElement rowElement in Rows(table))
        {
            int repeat = Repeat(rowElement, "number-rows-repeated");
            int span = Math.Min(repeat, MaxRepeat);

            for (int at = 0; at < span && row < SheetAddress.MaxRow; at++, row++)
            {
                // Only the first copy of a repeated row carries its drawings, which is what the
                // extraction path does with the same attribute: a row repeated a million times is
                // the sheet's padding rather than a million pictures.
                if (at == 0) ReadCells(file, rowElement, row, drawings);
            }

            if (repeat > span) row += repeat - span;
        }

        return drawings.Count == 0 ? SheetDrawings.Empty : new SheetDrawings(drawings);
    }

    private static void ReadCells(
        OdfFile file, XElement rowElement, int row, List<SheetDrawing> drawings)
    {
        int column = 0;

        foreach (XElement cell in rowElement.Elements())
        {
            if (cell.Name.NamespaceName != OdfNamespaces.Table) continue;
            if (cell.Name.LocalName is not ("table-cell" or "covered-table-cell")) continue;

            int repeat = Repeat(cell, "number-columns-repeated");

            foreach (XElement frame in cell.Elements(XName.Get("frame", OdfNamespaces.Draw)))
            {
                if (Read(file, frame, row, column) is { } drawing) drawings.Add(drawing);
            }

            column += Math.Min(repeat, MaxRepeat);
            if (repeat > MaxRepeat) column += repeat - MaxRepeat;
            if (column >= SheetAddress.MaxColumn) break;
        }
    }

    private static SheetDrawing? Read(OdfFile file, XElement frame, int row, int column)
    {
        XElement? image = frame.Element(XName.Get("image", OdfNamespaces.Draw));
        XElement? objectFrame = frame.Element(XName.Get("object", OdfNamespaces.Draw));

        SheetDrawing drawing = new()
        {
            Anchor = SheetAnchorKind.OneCell,
            From = new SheetCellPoint(
                column, OdfValue.ParseLength(Attribute(frame, OdfNamespaces.SvgCompatible, "x")) ?? Length.Zero,
                row, OdfValue.ParseLength(Attribute(frame, OdfNamespaces.SvgCompatible, "y")) ?? Length.Zero),
            Extent = new DocSize(
                OdfValue.ParseLength(Attribute(frame, OdfNamespaces.SvgCompatible, "width")) ?? Length.Zero,
                OdfValue.ParseLength(Attribute(frame, OdfNamespaces.SvgCompatible, "height")) ?? Length.Zero),
            Name = Attribute(frame, OdfNamespaces.Draw, "name"),
            Description = Description(frame),
        };

        if (EndCell(Attribute(frame, OdfNamespaces.Table, "end-cell-address")) is { } end)
        {
            drawing = drawing with
            {
                Anchor = SheetAnchorKind.TwoCell,
                To = new SheetCellPoint(
                    end.Column,
                    OdfValue.ParseLength(Attribute(frame, OdfNamespaces.Table, "end-x")) ?? Length.Zero,
                    end.Row,
                    OdfValue.ParseLength(Attribute(frame, OdfNamespaces.Table, "end-y")) ?? Length.Zero),
            };
        }

        // An embedded object is a chart, a formula or another document. The flag is set for all of
        // them, so that "there is something here" stays distinguishable from "there is nothing
        // here"; only a chart of a drawable kind also carries a model.
        //
        // **The object is looked at before the picture, and the order is what makes a sheet's
        // chart draw.** A frame holding an object carries a *replacement* picture beside it —
        // `draw:image xlink:href="./ObjectReplacements/Object 1"` — which is what an application
        // that cannot open the object shows instead. Reading the picture first found one on every
        // chart in every ODS LibreOffice has ever written, recorded the frame as a plain picture,
        // and then painted nothing, because all 82 of those streams are `VCLMTF` and no decoder
        // here reads StarView metafiles. A deck never hit it: an ODP frame carries the object
        // alone.
        if (objectFrame is not null)
        {
            SheetDrawing chart = drawing with { IsChart = true, Chart = Plot(file, objectFrame) };

            // A chart of a kind the engine does not draw falls back to the replacement picture,
            // which is better than nothing wherever a backend can decode one.
            if (chart.Chart is not null || image is null) return chart;

            (RasterImage? fallback, Lazy<VectorImage>? drawn) = Load(file, image);
            return chart with { Image = fallback, Vector = drawn };
        }

        if (image is null) return null;

        (RasterImage? raster, Lazy<VectorImage>? vector) = Load(file, image);
        return drawing with { Image = raster, Vector = vector };
    }

    /// <summary>
    /// The chart a <c>draw:object</c> holds, or null when it holds something else.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One reader for both shapes an embedded chart takes — a packaged sheet's
    /// <c>Object 1/content.xml</c> reached by <c>xlink:href</c>, and a flat sheet's inlined
    /// <c>office:document</c> — because <see cref="OdfChart.Locate"/> already hides the
    /// difference. This is the same call the ODP layout makes, which is what the move of
    /// <see cref="OdfChartPlot"/> down into <c>Paperless.OpenDocument</c> bought: before it, a
    /// sheet would have needed a second copy of the reader.
    /// </para>
    /// <para>
    /// The styles are the chart sub-document's own — <c>ch1</c>, <c>ch2</c>, … in its own
    /// <c>office:automatic-styles</c> — and not the workbook's, so they are read from whichever
    /// root the chart was found under.
    /// </para>
    /// </remarks>
    private static ChartPlot? Plot(OdfFile file, XElement objectFrame)
    {
        if (OdfChart.Locate(objectFrame, file) is not { } chart) return null;

        return OdfChartPlot.Read(chart, new OdfChartStyles(chart.AncestorsAndSelf().Last()));
    }

    /// <summary>
    /// A <c>draw:image</c>'s picture, told apart into a raster and a metafile.
    /// </summary>
    /// <remarks>
    /// <c>draw:mime-type</c> is passed on as a hint and the bytes decide, because ODF's own exporters
    /// disagree with themselves: LibreOffice writes <c>image/x-emf</c> for a file it stored under a
    /// <c>.emf</c> name and <c>image/x-wmf</c> for one it stored under <c>.wmf</c>, and neither name
    /// nor type tells an EMF+ from the EMF that carries it. A vector is left undecoded until something
    /// draws it; see <c>SheetDrawing.Vector</c> for what that costs otherwise.
    /// </remarks>
    private static (RasterImage? Raster, Lazy<VectorImage>? Vector) Load(OdfFile file, XElement image)
    {
        string? mediaType = Attribute(image, OdfNamespaces.Draw, "mime-type");

        if (Attribute(image, OdfNamespaces.XLink, "href") is { Length: > 0 } href)
        {
            // A path with a scheme points outside the package; Paperless does not fetch those.
            if (href.Contains("://", StringComparison.Ordinal)) return default;

            string part = href.StartsWith("./", StringComparison.Ordinal) ? href[2..] : href;
            using Stream? content = file.OpenPart(part);
            if (content is null) return default;

            using MemoryStream buffer = new();
            content.CopyTo(buffer);
            return buffer.Length == 0 ? default : Drawable(buffer.ToArray(), mediaType);
        }

        XElement? data = image.Element(XName.Get("binary-data", OdfNamespaces.Office));
        if (data is null) return default;

        try
        {
            byte[] bytes = Convert.FromBase64String(data.Value);
            return bytes.Length == 0 ? default : Drawable(bytes, mediaType);
        }
        catch (FormatException)
        {
            // Base64 a writer mangled is a picture that cannot be drawn, not a document that
            // cannot be read.
            return default;
        }
    }

    /// <summary>Which of the two kinds some picture bytes are.</summary>
    private static (RasterImage? Raster, Lazy<VectorImage>? Vector) Drawable(
        ReadOnlyMemory<byte> bytes, string? mediaType)
        => VectorImages.For(bytes.Span) is not null
            ? (null, new Lazy<VectorImage>(() => VectorImages.Decode(bytes)))
            : (RasterImage.Encoded(bytes, mediaType), null);

    /// <summary>
    /// A <c>table:end-cell-address</c>: a sheet name, a dot, and an A1 reference.
    /// </summary>
    /// <remarks>
    /// The sheet name is dropped rather than checked. A frame that names another sheet is not
    /// something any writer produces, and honouring it would mean placing a picture on a sheet it
    /// is not stored in; taking the address as this sheet's is the lenient reading and the one
    /// that keeps the picture. The name may itself contain dots when it is quoted, so the split is
    /// on the <em>last</em> one.
    /// </remarks>
    private static (int Column, int Row)? EndCell(string? address)
    {
        if (string.IsNullOrWhiteSpace(address)) return null;

        string reference = address[(address.LastIndexOf('.') + 1)..].Replace("$", string.Empty,
            StringComparison.Ordinal);

        int at = 0;
        int column = 0;
        while (at < reference.Length && char.IsAsciiLetter(reference[at]))
        {
            column = (column * 26) + (char.ToUpperInvariant(reference[at]) - 'A' + 1);
            at++;
        }

        if (at == 0 || at >= reference.Length) return null;
        if (!int.TryParse(reference[at..], NumberStyles.Integer, CultureInfo.InvariantCulture,
                          out int row) || row <= 0)
        {
            return null;
        }

        return (column - 1, row - 1);
    }

    /// <summary>The frame's description, which ODF writes as a child element rather than an attribute.</summary>
    private static string? Description(XElement frame)
        => frame.Element(XName.Get("desc", OdfNamespaces.SvgCompatible))?.Value
           ?? frame.Element(XName.Get("title", OdfNamespaces.SvgCompatible))?.Value;

    private static IEnumerable<XElement> Rows(XElement table)
    {
        foreach (XElement child in table.Elements())
        {
            if (child.Name.NamespaceName != OdfNamespaces.Table) continue;

            if (child.Name.LocalName == "table-row")
            {
                yield return child;
            }
            else if (child.Name.LocalName is "table-header-rows" or "table-row-group")
            {
                foreach (XElement nested in Rows(child)) yield return nested;
            }
        }
    }

    private static string? Attribute(XElement element, string ns, string name)
        => element.Attribute(XName.Get(name, ns))?.Value;

    private static int Repeat(XElement element, string name)
        => element.Attribute(XName.Get(name, OdfNamespaces.Table))?.Value is { } value
           && int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out int count)
           && count > 0
            ? count
            : 1;
}
