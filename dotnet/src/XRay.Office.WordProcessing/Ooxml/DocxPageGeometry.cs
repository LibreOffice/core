using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Model;

namespace XRay.Office.WordProcessing.Ooxml;

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

    /// <summary>What a <c>w:cols</c> with no <c>w:space</c> means by it: 1.25 cm.</summary>
    /// <remarks>
    /// <c>SectionPropertyMap</c> initialises <c>m_nColumnDistance( 1249 )</c> — hundredths of a
    /// millimetre, so 12.49 mm — and only overwrites it where the attribute is present. Every
    /// multi-column section in the sample corpus's DOCX states the attribute, so this is a correctness
    /// fallback rather than a measured reach; the same figure is load-bearing on the WW8 side, where
    /// sections routinely omit it.
    /// </remarks>
    private static readonly Length DefaultColumnGap = Length.FromTwips(708);

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

            // w:pgNumType/@w:fmt, the sequence the section's PAGE fields are written in. 21 of this
            // corpus's DOCX name lowerRoman for their front matter and decimal for the body.
            PageNumberFormat = Layout.NoteNumbering.Parse(
                    Word.Attribute(Word.Child(sectionProperties, "pgNumType"), "fmt"))
                ?? Layout.NoteNumberFormat.Arabic,

            // w:titlePg is per-section; even-and-odd is per-document. Mixing the two up is easy and
            // shows up as a first-page header appearing on every page or on none.
            HasDifferentFirstPage = Word.IsOn(Word.Child(sectionProperties, "titlePg")),

            // w:type names the break, and its absence means nextPage — which is both the schema's default
            // and what a document that never thought about it wants.
            Break = Word.Attribute(Word.Child(sectionProperties, "type"), "val") switch
            {
                "continuous" => SectionBreak.Continuous,
                "nextColumn" => SectionBreak.NewColumn,
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

        // What a section that states nothing is: Letter with one-inch margins, from
        // `SectionPropertyMap`'s constructor. See `PageGeometry.Letter` for why this is not the
        // locale's paper — a DOCX with no `w:sectPr` renders 612x792 on a machine whose own default
        // page is A4.
        PageGeometry fallback = PageGeometry.Letter;

        Length width = Dimension(size, "w") ?? fallback.Size.Width;
        Length height = Dimension(size, "h") ?? fallback.Size.Height;

        Length top = Twips(margins, "top") ?? fallback.Margins.Top;
        Length bottom = Twips(margins, "bottom") ?? fallback.Margins.Bottom;
        Length headerDistance = Twips(margins, "header") ?? Length.Zero;
        Length footerDistance = Twips(margins, "footer") ?? Length.Zero;

        return new PageGeometry
        {
            Size = new DocSize(width, height),
            Margins = new PageMargins(
                Twips(margins, "left") ?? fallback.Margins.Left,
                Twips(margins, "right") ?? fallback.Margins.Right,
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
            // 1.25 cm when the element says nothing, which is what `SectionPropertyMap`'s
            // `m_nColumnDistance( 1249 )` (hundredths of a millimetre) means and what the WW8 reader's
            // `sprmSDxaColumns` fallback of 708 twips says in the other unit. Zero would be a gutter of
            // nothing *and* a column 8% too wide, since the width is the measure less the gaps.
            ColumnGap = Twips(columns, "space") ?? DefaultColumnGap,
            IsLandscape = landscape,

            // w:sectPr/w:bidi, which is a different statement from a paragraph's w:bidi and does a
            // different thing: it reverses the section's columns rather than mirroring its text.
            IsRightToLeft = Word.IsOn(Word.Child(sectionProperties, "bidi")),

            HasMirroredMargins = Word.IsOn(Word.Child(settings, "mirrorMargins")),

            Borders = Borders(Word.Child(sectionProperties, "pgBorders")),
        };
    }

    /// <summary>
    /// The border drawn round the page, or null when the section declares none that draws.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>w:pgBorders</c> is the same four-sided shape as <c>w:pBdr</c> and <c>w:tblBorders</c> and
    /// is read the same way — <c>w:sz</c> in eighths of a point through
    /// <see cref="BorderRules"/> so that Word's style names and its width clamps are applied once,
    /// and <c>w:color</c> through <see cref="WordThemeColour"/>. What is different is
    /// <c>w:space</c>: on a paragraph border it is the gap between the line and the text, and here
    /// it is the gap between the line and whatever <c>w:offsetFrom</c> names, which is the
    /// <em>paper's edge</em> unless the attribute says <c>text</c>. Word states it in whole points
    /// and clamps it to 31.
    /// </para>
    /// <para>
    /// No theme is threaded in, so a border stating only <c>w:themeColor</c> falls back to black.
    /// Nothing in the corpus does: all seven documents that declare a page border state
    /// <c>w:color</c> outright.
    /// </para>
    /// </remarks>
    private static PageBorders? Borders(XElement? pageBorders)
    {
        if (pageBorders is null) return null;

        PageBorders borders = new()
        {
            Top = Side(Word.Child(pageBorders, "top")),
            Left = Side(Word.Child(pageBorders, "left")),
            Bottom = Side(Word.Child(pageBorders, "bottom")),
            Right = Side(Word.Child(pageBorders, "right")),
            // `w:offsetFrom` absent means *text*, which is the opposite of the obvious reading and is
            // measured: 26.2.4.2 draws a `w:pgBorders` with no `w:offsetFrom` at the text's own inset
            // and not at `w:space` from the paper. The seat is `PageBordersHandler`'s constructor,
            // which initialises `m_eOffsetFrom` to `BorderOffsetFrom::Text`
            // (`sw/source/writerfilter/dmapper/PageBordersHandler.cxx`:34-38), while its `default:`
            // arm for a *stated* value falls to `page`. All seven corpus documents state `page`, so
            // this arm is reached only by a fixture — and by RTF, whose `\pgbrdropt` reaches dmapper
            // through the same handler and states nothing at all for the text case.
            OffsetFromText = Word.Attribute(pageBorders, "offsetFrom") is not { } offset
                || !string.Equals(offset, "page", StringComparison.Ordinal),
            Shadow = Shadow(Word.Child(pageBorders, "right")),
            // `w:display` is deliberately not read. It reaches `PageBordersHandler::lcl_attribute`,
            // which turns it into `m_eBorderApply` and hands that to
            // `SectionPropertyMap::ApplyBorderToPageStyles` — whose signature names the parameter
            // `BorderApply /*eBorderApply*/` and never touches it
            // (`sw/source/writerfilter/dmapper/PropertyMap.cxx`:649-721). One page style per section
            // carries the border, so every page of the section gets it. Measured as well as read: two
            // four-page fixtures declaring `firstPage` and `notFirstPage` both render on 26.2.4.2 with
            // the border on all four pages. **The DOC reader is not the same** — `pgbApplyTo` really
            // is honoured there (`ww8par.cxx`:4303-4306) — so this stays a property of the model and
            // is simply not stated by this reader.
            Display = PageBorderDisplay.AllPages,
        };

        return borders.Draws ? borders : null;
    }

    /// <summary>One side of a page border, or a side that draws nothing.</summary>
    private static PageBorderSide Side(XElement? stated)
    {
        if (stated is null) return default;

        string? val = Word.Attribute(stated, "val");
        if (val is null or "none" or "nil") return default;

        Length stateWidth =
            Word.Integer(Word.Attribute(stated, "sz"), out int eighths) && eighths > 0
                ? Length.FromPoints(eighths / 8.0)
                : Length.FromPoints(0.5);

        if (BorderRules.FromWord(BorderRules.WordStyleOf(val), stateWidth) is not { } rule)
        {
            return default;
        }

        Length space =
            Word.Integer(Word.Attribute(stated, "space"), out int points) && points > 0
                ? Length.FromPoints(Math.Min(points, 31))
                : Length.Zero;

        return new PageBorderSide(
            rule.Width,
            WordThemeColour.Read(stated, null, "color", "themeColor", "themeTint", "themeShade")
                ?? Colour.Black,
            space);
    }

    /// <summary>
    /// The shadow the border casts, from the <em>right</em> side alone.
    /// </summary>
    /// <remarks>
    /// <c>w:shadow</c> is an attribute of each side and the shadow is a property of the whole box, so
    /// a reader has to choose which side decides. Writer reads the right one and only the right one:
    /// <c>SectionPropertyMap::ApplyBorderToPageStyles</c> tests
    /// <c>m_bBorderShadows[BORDER_RIGHT]</c> and <c>PropertyMap::getShadowFromBorder</c> takes that
    /// side's own <c>LineWidth</c> for the offset
    /// (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:715-734). Reading "any side asks for
    /// it" agrees on every corpus document — all seven declare the same flag on all four sides — and
    /// disagrees on a file that shadows one side only, which Word can write.
    /// </remarks>
    private static Length Shadow(XElement? right)
    {
        if (right is null) return Length.Zero;

        return Word.Attribute(right, "shadow") is "1" or "true" or "on"
            ? Side(right).Width
            : Length.Zero;
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
    /// <remarks>
    /// Fitted to the nearest standard paper dimension when it is within 0.44 mm of one, which is
    /// what <c>DomainMapper</c> does to <c>w:pgSz</c>'s two attributes and to nothing else — see
    /// <see cref="Model.PaperSizes"/>. Margins are left exactly as stated: the fit is applied to
    /// the sheet, so a page whose width moves keeps the measure it was written for only to within
    /// the same 0.44 mm, and that is the behaviour, not an oversight.
    /// </remarks>
    private static Length? Dimension(XElement? element, string attribute)
        => Word.Attribute(element, attribute) is { } text
           && Word.Long(text, out long twips)
           && twips is > 0 and <= MaxDimensionTwips
            ? Model.PaperSizes.SloppyFit(Length.FromTwips(twips))
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
           && Word.Long(text, out long twips)
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
            && Word.Integer(text, out int declared)
            && declared > 0)
        {
            return declared;
        }

        int listed = Word.Children(columns, "col").Count();
        return listed > 0 ? listed : 1;
    }

    private static int? RestartAt(XElement? sectionProperties)
        => Word.Attribute(Word.Child(sectionProperties, "pgNumType"), "start") is { } text
           && Word.Integer(text, out int start)
            ? start
            : null;
}
