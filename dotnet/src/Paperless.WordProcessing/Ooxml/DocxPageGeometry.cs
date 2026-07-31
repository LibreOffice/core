using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.WordProcessing.Model;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>
/// Reads a DOCX's section properties into page geometry.
/// </summary>
/// <remarks>
/// <para>
/// Everything here is in twentieths of a point, which is why the conversion is
/// <see cref="Length.FromTwips"/> throughout and never a floating-point scale: 1440 twips is exactly
/// one inch and exactly 914400 EMUs, so the arithmetic is lossless as long as nothing goes through a
/// <c>double</c> on the way.
/// </para>
/// <para>
/// Two of the properties are not in the section at all. <c>w:evenAndOddHeaders</c> and
/// <c>w:mirrorMargins</c> live in <c>settings.xml</c> and apply to the whole document, so a reader that
/// looks only at <c>w:sectPr</c> concludes that no document has ever distinguished even from odd pages.
/// </para>
/// </remarks>
internal static class DocxPageGeometry
{
    /// <summary>
    /// A page dimension outside this range is treated as absent.
    /// </summary>
    /// <remarks>
    /// Twenty-two inches, a little over the largest paper any producer writes. A zero or a wildly wrong
    /// dimension is common enough in generated files, and falling back to A4 renders something, where
    /// honouring it produces a page with no text area at all.
    /// </remarks>
    private const long MaxDimensionTwips = 22 * 1440;

    /// <summary>Reads a <c>w:sectPr</c>, filling in from the document's settings and the defaults.</summary>
    /// <param name="sectionProperties">The <c>w:sectPr</c> element, or null for a document with none.</param>
    /// <param name="settings">The document's <c>w:settings</c> root, or null when it has none.</param>
    internal static WritingSection Read(XElement? sectionProperties, XElement? settings)
    {
        PageGeometry page = ReadGeometry(sectionProperties, settings);

        return new WritingSection
        {
            Page = page,
            RestartPageNumberAt = RestartAt(sectionProperties),

            // w:titlePg is per-section; even-and-odd is per-document. Mixing the two up is easy and
            // shows up as a first-page header appearing on every page or on none.
            HasDifferentFirstPage = Word.IsOn(Word.Child(sectionProperties, "titlePg")),

            // w:type names the break, and its absence means nextPage — which is both the schema's default
            // and what a document that never thought about it wants.
            Break = Word.Attribute(Word.Child(sectionProperties, "type"), "val") switch
            {
                "continuous" => SectionBreak.Continuous,
                "evenPage" => SectionBreak.EvenPage,
                "oddPage" => SectionBreak.OddPage,
                _ => SectionBreak.NextPage,
            },
            HasDifferentEvenPages = Word.IsOn(Word.Child(settings, "evenAndOddHeaders")),
        };
    }

    private static PageGeometry ReadGeometry(XElement? sectionProperties, XElement? settings)
    {
        XElement? size = Word.Child(sectionProperties, "pgSz");
        XElement? margins = Word.Child(sectionProperties, "pgMar");
        XElement? columns = Word.Child(sectionProperties, "cols");

        // The orientation attribute and the dimensions can disagree. Word believes the dimensions and
        // treats the attribute as a note about how the user got there, so this records the flag but
        // does not swap anything on its strength.
        bool landscape = string.Equals(
            Word.Attribute(size, "orient"), "landscape", StringComparison.OrdinalIgnoreCase);

        Length width = Dimension(size, "w") ?? PageGeometry.Default.Size.Width;
        Length height = Dimension(size, "h") ?? PageGeometry.Default.Size.Height;

        Length top = Twips(margins, "top") ?? PageMargins.Default.Top;
        Length bottom = Twips(margins, "bottom") ?? PageMargins.Default.Bottom;
        Length headerDistance = Twips(margins, "header") ?? Length.Zero;
        Length footerDistance = Twips(margins, "footer") ?? Length.Zero;

        return new PageGeometry
        {
            Size = new DocSize(width, height),
            Margins = new PageMargins(
                Twips(margins, "left") ?? PageMargins.Default.Left,
                Twips(margins, "right") ?? PageMargins.Default.Right,
                top,
                bottom),
            Gutter = Twips(margins, "gutter") ?? Length.Zero,
            HeaderDistance = headerDistance,
            FooterDistance = footerDistance,

            // Word states the header's distance from the page edge and the body's top margin, and
            // leaves the header's own height implied by the gap between them. Deriving it keeps the
            // two Word-family readers and the ODF one reporting the same pair of numbers.
            HeaderHeight = Gap(headerDistance, top),
            FooterHeight = Gap(footerDistance, bottom),
            Columns = ColumnCount(columns),
            ColumnGap = Twips(columns, "space") ?? Length.Zero,
            IsLandscape = landscape,
            HasMirroredMargins = Word.IsOn(Word.Child(settings, "mirrorMargins")),
        };
    }

    /// <summary>
    /// How much room is left between the furniture's edge and the body's.
    /// </summary>
    /// <remarks>
    /// Never negative. A document whose header distance exceeds its top margin is telling Word to let
    /// the header overlap the body, which is legal and means the header has no reserved height of its
    /// own rather than a negative one.
    /// </remarks>
    private static Length Gap(Length furnitureEdge, Length bodyEdge)
    {
        Length gap = bodyEdge - furnitureEdge;
        return gap > Length.Zero ? gap : Length.Zero;
    }

    /// <summary>
    /// A page dimension, or null when it is missing or implausible.
    /// </summary>
    /// <remarks>
    /// Separate from <see cref="Twips"/> because a margin of zero is meaningful and a page width of
    /// zero is not. Treating them the same either rejects a legitimate zero margin or accepts a page
    /// with no width, and the second produces a document where every line overflows.
    /// </remarks>
    private static Length? Dimension(XElement? element, string attribute)
        => Word.Attribute(element, attribute) is { } text
           && long.TryParse(text, CultureInfo.InvariantCulture, out long twips)
           && twips is > 0 and <= MaxDimensionTwips
            ? Length.FromTwips(twips)
            : null;

    /// <summary>
    /// A measurement in twips, or null when the attribute is absent or not a number.
    /// </summary>
    /// <remarks>
    /// Signed on purpose. A negative top margin is how a document puts a header above the page's own
    /// top edge, and clamping it to zero moves the body text down by however far the header was meant
    /// to hang.
    /// </remarks>
    private static Length? Twips(XElement? element, string attribute)
        => Word.Attribute(element, attribute) is { } text
           && long.TryParse(text, CultureInfo.InvariantCulture, out long twips)
            ? Length.FromTwips(twips)
            : null;

    /// <summary>
    /// How many columns the section has.
    /// </summary>
    /// <remarks>
    /// From <c>w:num</c> when it is there, otherwise from the count of <c>w:col</c> children — a
    /// section with unequal columns lists them individually and need not state the number twice.
    /// </remarks>
    private static int ColumnCount(XElement? columns)
    {
        if (columns is null) return 1;

        if (Word.Attribute(columns, "num") is { } text
            && int.TryParse(text, CultureInfo.InvariantCulture, out int declared)
            && declared > 0)
        {
            return declared;
        }

        int listed = Word.Children(columns, "col").Count();
        return listed > 0 ? listed : 1;
    }

    private static int? RestartAt(XElement? sectionProperties)
        => Word.Attribute(Word.Child(sectionProperties, "pgNumType"), "start") is { } text
           && int.TryParse(text, CultureInfo.InvariantCulture, out int start)
            ? start
            : null;
}
