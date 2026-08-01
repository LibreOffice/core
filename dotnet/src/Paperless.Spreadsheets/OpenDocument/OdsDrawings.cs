using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.Spreadsheets.Layout;

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
                column, Measure(Attribute(frame, OdfNamespaces.SvgCompatible, "x")) ?? Length.Zero,
                row, Measure(Attribute(frame, OdfNamespaces.SvgCompatible, "y")) ?? Length.Zero),
            Extent = new DocSize(
                Measure(Attribute(frame, OdfNamespaces.SvgCompatible, "width")) ?? Length.Zero,
                Measure(Attribute(frame, OdfNamespaces.SvgCompatible, "height")) ?? Length.Zero),
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
                    Measure(Attribute(frame, OdfNamespaces.Table, "end-x")) ?? Length.Zero,
                    end.Row,
                    Measure(Attribute(frame, OdfNamespaces.Table, "end-y")) ?? Length.Zero),
            };
        }

        // An embedded object is a chart, a formula or another document: recorded so that "there is
        // something here" stays distinguishable from "there is nothing here", and not painted.
        if (image is null)
            return objectFrame is null ? null : drawing with { IsChart = true };

        return drawing with { Image = Load(file, image) };
    }

    private static RasterImage? Load(OdfFile file, XElement image)
    {
        if (Attribute(image, OdfNamespaces.XLink, "href") is { Length: > 0 } href)
        {
            // A path with a scheme points outside the package; Paperless does not fetch those.
            if (href.Contains("://", StringComparison.Ordinal)) return null;

            string part = href.StartsWith("./", StringComparison.Ordinal) ? href[2..] : href;
            using Stream? content = file.OpenPart(part);
            if (content is null) return null;

            using MemoryStream buffer = new();
            content.CopyTo(buffer);
            return buffer.Length == 0
                ? null
                : RasterImage.Encoded(
                    buffer.ToArray(), Attribute(image, OdfNamespaces.Draw, "mime-type"));
        }

        XElement? data = image.Element(XName.Get("binary-data", OdfNamespaces.Office));
        if (data is null) return null;

        try
        {
            byte[] bytes = Convert.FromBase64String(data.Value);
            return bytes.Length == 0
                ? null
                : RasterImage.Encoded(bytes, Attribute(image, OdfNamespaces.Draw, "mime-type"));
        }
        catch (FormatException)
        {
            // Base64 a writer mangled is a picture that cannot be drawn, not a document that
            // cannot be read.
            return null;
        }
    }

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

    /// <summary>An ODF length, in any of the units the format allows.</summary>
    private static Length? Measure(string? value)
    {
        if (string.IsNullOrWhiteSpace(value)) return null;

        string text = value.Trim();
        int at = 0;
        while (at < text.Length && (char.IsAsciiDigit(text[at]) || text[at] is '-' or '+' or '.')) at++;
        if (at == 0) return null;

        if (!double.TryParse(
                text[..at], NumberStyles.Float, CultureInfo.InvariantCulture, out double number))
        {
            return null;
        }

        return text[at..].Trim().ToLowerInvariant() switch
        {
            "cm" => Length.FromMillimetres(number * 10),
            "mm" => Length.FromMillimetres(number),
            "in" => Length.FromInches(number),
            "pt" => Length.FromPoints(number),
            "pc" => Length.FromPoints(number * 12),
            "px" => Length.FromPixels(number, 96),
            _ => Length.FromPoints(number),
        };
    }
}
