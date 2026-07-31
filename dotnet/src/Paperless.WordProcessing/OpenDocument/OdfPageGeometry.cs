using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.WordProcessing.Model;

namespace Paperless.WordProcessing.OpenDocument;

/// <summary>
/// Reads an ODF master page's geometry.
/// </summary>
/// <remarks>
/// <para>
/// ODF splits what the other three formats state in one place. A <c>style:master-page</c> pairs a
/// <em>name</em> and its header and footer content with a <c>style:page-layout</c> holding the
/// geometry, and a paragraph reaches its master page through its paragraph style's
/// <c>style:master-page-name</c>. So finding a section's page setup means resolving a style chain
/// first — which is why this takes the resolved <see cref="OdfStyles"/> rather than an element.
/// </para>
/// <para>
/// The header and footer are stated differently too. Word gives a distance from the page edge; ODF
/// gives the header its own height and a margin between it and the body, both inside the page's top
/// margin. The conversion is the page's top margin minus the header's own extent, which is what makes
/// <see cref="PageGeometry.HeaderDistance"/> comparable across all four formats.
/// </para>
/// <para>
/// ODF is also the one format that states its lengths with units — <c>2cm</c>, <c>0.79in</c>,
/// <c>1134twip</c> — so the parsing goes through <see cref="OdfValue.ParseLength"/> rather than
/// assuming a scale.
/// </para>
/// </remarks>
internal static class OdfPageGeometry
{
    /// <summary>A page dimension beyond this is treated as a producer error.</summary>
    private const double MaxDimensionMillimetres = 22 * 25.4;

    /// <summary>
    /// Reads a section from a master page and the styles it resolves through.
    /// </summary>
    /// <param name="styles">The document's styles, for the page layout the master names.</param>
    /// <param name="master">The master page, or null to get the defaults.</param>
    internal static WritingSection Read(OdfStyles styles, OdfMasterPage? master)
    {
        ArgumentNullException.ThrowIfNull(styles);

        OdfStyle? layout = styles.FindPageLayout(master?.PageLayoutName);
        OdfPropertySet? properties = layout?.Properties(OdfPropertyKind.PageLayout);

        // ODF's fo:margin-top is the distance from the page edge to the top of the *header*, not to
        // the body — so it is Word's w:header, and the body's own top margin is this plus whatever
        // the header occupies. Reading it as the body's margin puts every line of text too high by
        // the height of the header.
        Length headerDistance = Length(properties, "margin-top") ?? PageMargins.Default.Top;
        Length footerDistance = Length(properties, "margin-bottom") ?? PageMargins.Default.Bottom;

        // Measuring the furniture needs its own style, which is a child of the page layout rather
        // than a style in its own right. A master page with no header contributes nothing, which is
        // right: there is no header area to leave room for.
        Length headerHeight = master?.HasHeader() == true
            ? FurnitureExtent(layout?.HeaderProperties)
            : Core.Units.Length.Zero;
        Length footerHeight = master?.HasFooter() == true
            ? FurnitureExtent(layout?.FooterProperties)
            : Core.Units.Length.Zero;

        PageGeometry page = new()
        {
            Size = new DocSize(
                Dimension(properties, "page-width") ?? PageGeometry.Default.Size.Width,
                Dimension(properties, "page-height") ?? PageGeometry.Default.Size.Height),
            Margins = new PageMargins(
                Length(properties, "margin-left") ?? PageMargins.Default.Left,
                Length(properties, "margin-right") ?? PageMargins.Default.Right,
                headerDistance + headerHeight,
                footerDistance + footerHeight),
            HeaderDistance = headerDistance,
            FooterDistance = footerDistance,
            HeaderHeight = headerHeight,
            FooterHeight = footerHeight,
            Columns = ColumnCount(properties),
            ColumnGap = ColumnGap(properties),
            IsLandscape = string.Equals(
                properties?.Get(OdfNamespaces.Style, "print-orientation"),
                "landscape",
                StringComparison.OrdinalIgnoreCase),

            // ODF says which pages a master applies to rather than that margins mirror, so mirrored
            // margins show up as a page-usage of "mirrored" instead of a flag on the margins.
            HasMirroredMargins = string.Equals(
                properties?.Get(OdfNamespaces.Style, "page-usage"),
                "mirrored",
                StringComparison.OrdinalIgnoreCase),
        };

        return new WritingSection
        {
            Page = page,

            // ODF has no first-page flag: a document that wants a different first page gives it its
            // own master, reached through the first paragraph's style. So the separate slots are
            // populated only when the master itself names them, which is how a "left page" master
            // distinguishes even pages.
            HasDifferentFirstPage = master?.FirstHeader is not null || master?.FirstFooter is not null,
            HasDifferentEvenPages = master?.LeftHeader is not null || master?.LeftFooter is not null,
        };
    }

    /// <summary>
    /// The height a header or footer occupies inside the page margin.
    /// </summary>
    /// <remarks>
    /// <para>
    /// ODF states the height two ways and they behave differently, which is not obvious from the
    /// attribute names and was settled by rendering both and measuring the result.
    /// </para>
    /// <list type="bullet">
    ///   <item>
    ///     <c>svg:height</c> is a <em>fixed</em> height, honoured exactly — and it absorbs the
    ///     spacing: a 12 mm header 25 mm from the page edge puts the body at 37 mm however large
    ///     <c>fo:margin-bottom</c> is.
    ///   </item>
    ///   <item>
    ///     <c>fo:min-height</c> makes the height <em>dynamic</em>. LibreOffice maps it to
    ///     <c>HeaderIsDynamicHeight</c> (<c>xmloff/source/style/PageMasterImportPropMapper.cxx</c>)
    ///     and then sizes the frame to its content, so the declared value is not a floor in practice —
    ///     a header declaring 6 mm around one 12 pt line renders 4.9 mm tall. The spacing is added on
    ///     top of that.
    ///   </item>
    /// </list>
    /// <para>
    /// The dynamic case therefore needs the header's content laid out to be exact, which cannot happen
    /// before the page it sits on is known. The declared minimum plus the spacing is used instead — the
    /// same approximation LibreOffice's own DOC exporter falls back to, and which its comment calls
    /// "totally nonoptimum, but the best we can do"
    /// (<c>sw/source/filter/ww8/writerwordglue.cxx</c>). It errs towards leaving too much room, so text
    /// starts slightly low rather than overlapping the header.
    /// </para>
    /// </remarks>
    private static Length FurnitureExtent(OdfPropertySet? properties)
    {
        if (properties is null) return Core.Units.Length.Zero;

        if (OdfValue.ParseLength(properties.Get(OdfNamespaces.SvgCompatible, "height")) is { } fixedHeight)
        {
            return fixedHeight;
        }

        Length declared =
            OdfValue.ParseLength(properties.Get(OdfNamespaces.FoCompatible, "min-height"))
            ?? Core.Units.Length.Zero;

        // The margin below a header, or above a footer. Both are written as the side facing the body,
        // so whichever is present is the spacing that separates the furniture from the text.
        Length spacing =
            OdfValue.ParseLength(properties.Get(OdfNamespaces.FoCompatible, "margin-bottom"))
            ?? OdfValue.ParseLength(properties.Get(OdfNamespaces.FoCompatible, "margin-top"))
            ?? Core.Units.Length.Zero;

        return declared + spacing;
    }

    private static Length? Length(OdfPropertySet? properties, string localName)
        => OdfValue.ParseLength(properties?.Get(OdfNamespaces.FoCompatible, localName));

    private static Length? Dimension(OdfPropertySet? properties, string localName)
        => Length(properties, localName) is { } value
           && value > Core.Units.Length.Zero
           && value.Millimetres <= MaxDimensionMillimetres
            ? value
            : null;

    /// <summary>
    /// A distance that cannot go below zero.
    /// </summary>
    /// <remarks>
    /// A header taller than the margin it sits in is legal in ODF and means the body starts lower, not
    /// that the header starts above the page. Clamping keeps the header inside the sheet.
    /// </remarks>
    private static Length Difference(Length margin, Length extent)
    {
        Length remaining = margin - extent;
        return remaining > Core.Units.Length.Zero ? remaining : Core.Units.Length.Zero;
    }

    /// <summary>
    /// How many columns the page layout declares.
    /// </summary>
    /// <remarks>
    /// From <c>style:columns</c>'s own count when it states one, otherwise from the number of
    /// <c>style:column</c> children — a layout with unequal columns lists them and need not repeat the
    /// number.
    /// </remarks>
    private static int ColumnCount(OdfPropertySet? properties)
    {
        if (properties?.Child(OdfNamespaces.Style, "columns") is not { } columns) return 1;

        if (columns.Attribute(XName.Get("column-count", OdfNamespaces.FoCompatible))?.Value is { } text
            && int.TryParse(text, out int declared)
            && declared > 0)
        {
            return Math.Min(declared, 64);
        }

        int listed = columns.Elements(XName.Get("column", OdfNamespaces.Style)).Count();
        return listed > 0 ? Math.Min(listed, 64) : 1;
    }

    /// <summary>
    /// The gap between columns.
    /// </summary>
    /// <remarks>
    /// ODF states it as <c>fo:column-gap</c> on the columns element, or — for unequal columns — as a
    /// margin on each column, in which case the first column's right margin is representative. Taking
    /// it from the first rather than averaging keeps the common case exact.
    /// </remarks>
    private static Length ColumnGap(OdfPropertySet? properties)
    {
        if (properties?.Child(OdfNamespaces.Style, "columns") is not { } columns)
        {
            return Core.Units.Length.Zero;
        }

        if (OdfValue.ParseLength(columns.Attribute(XName.Get("column-gap", OdfNamespaces.FoCompatible))?.Value)
            is { } gap)
        {
            return gap;
        }

        XElement? first = columns.Elements(XName.Get("column", OdfNamespaces.Style)).FirstOrDefault();
        return OdfValue.ParseLength(
                   first?.Attribute(XName.Get("end-indent", OdfNamespaces.FoCompatible))?.Value)
               ?? Core.Units.Length.Zero;
    }
}
