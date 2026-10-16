using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.WordProcessing.Model;

namespace XRay.Office.WordProcessing.OpenDocument;

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
    /// <param name="headerContent">
    /// How tall the header's own content is, when the caller has laid it out. Zero — the default —
    /// means it has not, and a dynamic height then falls back to what the file states; see
    /// <see cref="FurnitureExtent"/>. Extraction never measures it and must not be made to.
    /// </param>
    /// <param name="footerContent">The same for the footer.</param>
    internal static WritingSection Read(
        OdfStyles styles,
        OdfMasterPage? master,
        Length headerContent = default,
        Length footerContent = default)
    {
        ArgumentNullException.ThrowIfNull(styles);

        OdfStyle? layout = styles.FindPageLayout(master?.PageLayoutName);
        OdfPropertySet? properties = layout?.Properties(OdfPropertyKind.PageLayout);

        // ODF's fo:margin-top is the distance from the page edge to the top of the *header*, not to
        // the body — so it is Word's w:header, and the body's own top margin is this plus whatever
        // the header occupies. Reading it as the body's margin puts every line of text too high by
        // the height of the header.
        // The page border, which ODF states as the *page style's own* box rather than as a distance
        // like Word does — so it has to be read before the margins, which it moves.
        PageBorders? borders = Borders(properties);

        Length headerDistance =
            (Length(properties, "margin-top") ?? PageMargins.Default.Top) + BorderBand(borders?.Top, properties, "top");
        Length footerDistance =
            (Length(properties, "margin-bottom") ?? PageMargins.Default.Bottom) + BorderBand(borders?.Bottom, properties, "bottom");

        // Measuring the furniture needs its own style, which is a child of the page layout rather
        // than a style in its own right. A master page with no header contributes nothing, which is
        // right: there is no header area to leave room for.
        Length headerHeight = master?.HasHeader() == true
            ? FurnitureExtent(layout?.HeaderProperties, headerContent)
            : Core.Units.Length.Zero;
        Length footerHeight = master?.HasFooter() == true
            ? FurnitureExtent(layout?.FooterProperties, footerContent)
            : Core.Units.Length.Zero;

        DocSize size = new(
            Dimension(properties, "page-width") ?? PageGeometry.Default.Size.Width,
            Dimension(properties, "page-height") ?? PageGeometry.Default.Size.Height);
        Length marginLeft = (Length(properties, "margin-left") ?? PageMargins.Default.Left)
                            + BorderBand(borders?.Left, properties, "left");
        Length marginRight = (Length(properties, "margin-right") ?? PageMargins.Default.Right)
                             + BorderBand(borders?.Right, properties, "right");

        PageGeometry page = new()
        {
            Size = size,
            Margins = new PageMargins(
                marginLeft,
                marginRight,
                headerDistance + headerHeight,
                footerDistance + footerHeight),
            HeaderDistance = headerDistance,
            FooterDistance = footerDistance,
            HeaderHeight = headerHeight,
            FooterHeight = footerHeight,

            // svg:height is the height, fo:min-height is a floor — Writer's SwFrameSize::Fixed against
            // SwFrameSize::Minimum. A fixed one does not grow, so content that outruns it overflows
            // instead of moving the body, and the paginator has to be told which it is.
            HasFixedHeaderHeight = HasFixedHeight(layout?.HeaderProperties),
            HasFixedFooterHeight = HasFixedHeight(layout?.FooterProperties),

            // ODF's footer is top-aligned below the body rather than bottom-aligned against the page, so
            // the spacing that separates the two is where its first line goes. Stated only when there is a
            // footer at all: a page with none should not claim an offset for it.
            FooterOffset = master?.HasFooter() == true
                ? FurnitureSpacing(layout?.FooterProperties)
                : null,
            Columns = ColumnCount(properties),
            ColumnGap = ColumnGap(properties),
            ColumnRuler = ColumnRulerOf(properties, size.Width - marginLeft - marginRight),

            // The page layout's own writing mode, which is a different statement from a paragraph's
            // and does a different thing: it reverses the order of the section's columns. The
            // vertical modes are left alone here as everywhere else.
            IsRightToLeft = properties?.Get(OdfNamespaces.Style, "writing-mode") is "rl-tb" or "rl",
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

            Borders = borders,
        };

        return new WritingSection
        {
            Page = page,

            // ODF has no first-page flag: a document that wants a different first page gives it its
            // own master, reached through the first paragraph's style. So the separate slots are
            // populated only when the master itself names them, which is how a "left page" master
            // distinguishes even pages.
            // style:num-format on the page layout, which ODF states by example — "1", "i", "I", "a", "A".
            PageNumberFormat = Layout.NoteNumbering.Parse(
                    properties?.Get(OdfNamespaces.Style, "num-format"))
                ?? Layout.NoteNumberFormat.Arabic,

            HasDifferentFirstPage = master?.FirstHeader is not null || master?.FirstFooter is not null,
            HasDifferentEvenPages = master?.LeftHeader is not null || master?.LeftFooter is not null,
        };
    }

    /// <summary>
    /// The page border a page layout declares, or null when it declares none that draws.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>ODF states this the other way round from every Word format, and reading it as Word's
    /// moves the text.</strong> Word gives a margin to the text and a distance from an edge to the
    /// border; ODF gives the page layout a box of its own, so <c>fo:margin-left</c> is the distance
    /// to the <em>border</em> and <c>fo:padding-left</c> the gap from the border to the text — the
    /// conversion <c>editeng::BorderDistanceFromWord</c> performs on import
    /// (<c>editeng/source/items/frmitems.cxx</c>:4143-4174), and what LibreOffice's own writer emits
    /// on the way back out. Measured: 26.2.4.2 converting a DOCX with <c>w:pgMar w:left="1440"</c>,
    /// <c>w:sz="36"</c> and <c>w:space="15"</c> writes <c>fo:margin-left="0.2083in"</c> (15 pt),
    /// <c>fo:border="4.51pt solid #396533"</c> and <c>fo:padding="0.7291in"</c> (52.5 pt), which sum
    /// to the inch Word stated. Taking <c>fo:margin-left</c> for the text's margin would start every
    /// line 57 pt too far left.
    /// </para>
    /// <para>
    /// So the border's <see cref="PageBorderSide.Space"/> is <c>fo:margin</c> itself and the offset
    /// is always from the paper's edge — ODF has no equivalent of <c>w:offsetFrom="text"</c> because
    /// it does not need one.
    /// </para>
    /// </remarks>
    private static PageBorders? Borders(OdfPropertySet? properties)
    {
        if (properties is null) return null;

        PageBorderSide all = Side(properties, "border", default);

        PageBorders borders = new()
        {
            Top = Side(properties, "border-top", all) with { Space = Margin(properties, "margin-top") },
            Left = Side(properties, "border-left", all) with { Space = Margin(properties, "margin-left") },
            Bottom = Side(properties, "border-bottom", all) with { Space = Margin(properties, "margin-bottom") },
            Right = Side(properties, "border-right", all) with { Space = Margin(properties, "margin-right") },
            Shadow = Shadow(properties),
        };

        return borders.Draws ? borders : null;
    }

    /// <summary>
    /// One side from CSS's three-part shorthand, or the four-sided value where the side says nothing.
    /// </summary>
    /// <remarks>
    /// <c>fo:border</c> sets all four and <c>fo:border-left</c> and friends override their own side,
    /// which is CSS's cascade and the same one <c>OdtLayoutSource.Tables</c> follows for a cell — with
    /// the same rule that a stated <c>none</c> has to beat the fallback rather than fall through it.
    /// </remarks>
    private static PageBorderSide Side(OdfPropertySet properties, string name, PageBorderSide fallback)
    {
        string? stated = properties.Get(OdfNamespaces.FoCompatible, name);
        if (string.IsNullOrWhiteSpace(stated)) return fallback;
        if (string.Equals(stated.Trim(), "none", StringComparison.Ordinal)) return default;

        Core.Units.Length width = Core.Units.Length.Zero;
        Colour colour = Colour.Black;

        foreach (string part in stated.Split(' ', StringSplitOptions.RemoveEmptyEntries))
        {
            if (OdfValue.ParseLength(part) is { } measured)
            {
                width = OdfWriterUnits.ToCore(measured);
                continue;
            }

            if (part.StartsWith('#') && OdfValue.ParseColour(part) is { } named) colour = named;
        }

        // A shorthand naming a style and a colour but no width is still a border, drawn at Writer's
        // thinnest visible stroke rather than not at all — the rule the table reader already applies.
        if (width <= Core.Units.Length.Zero) width = HairlineBorder;

        return new PageBorderSide(width, colour, Core.Units.Length.Zero);
    }

    /// <summary>The width a border with no stated one is drawn at: half a point, Writer's hairline.</summary>
    private static readonly Core.Units.Length HairlineBorder = Core.Units.Length.FromPoints(0.5);

    /// <summary>One <c>fo:margin-*</c>, which for a bordered page is the distance to the border.</summary>
    private static Core.Units.Length Margin(OdfPropertySet properties, string name)
        => Length(properties, name) ?? Core.Units.Length.Zero;

    /// <summary>
    /// How much of the page one side's border and its padding take out of the margin.
    /// </summary>
    /// <remarks>
    /// Zero for a side that draws no line, which is Writer's own rule rather than a simplification:
    /// <c>SvxBoxItem::CalcLineSpace</c> answers zero for a side with no line unless its caller asks
    /// otherwise, and <c>SwBorderAttrs</c> does not ask. So a page layout carrying a stray
    /// <c>fo:padding</c> and no border keeps its margins.
    /// </remarks>
    private static Core.Units.Length BorderBand(
        PageBorderSide? side, OdfPropertySet? properties, string edge)
    {
        if (side is not { } stated || !stated.Draws || properties is null) return Core.Units.Length.Zero;

        Core.Units.Length padding =
            Length(properties, "padding-" + edge)
            ?? Length(properties, "padding")
            ?? Core.Units.Length.Zero;

        return stated.Width + padding;
    }

    /// <summary>
    /// The shadow <c>style:shadow</c> declares, or zero for none.
    /// </summary>
    /// <remarks>
    /// ODF states the offset outright — <c>#000000 0.0626in 0.0626in</c> — where Word derives it from
    /// the right border's width, so this is the one format that does not have to choose a side. The
    /// value is <c>none</c> when there is no shadow, and the first length in it is the offset; the two
    /// lengths are the horizontal and vertical offsets and LibreOffice writes them equal.
    /// </remarks>
    private static Core.Units.Length Shadow(OdfPropertySet properties)
    {
        string? stated = properties.Get(OdfNamespaces.Style, "shadow");
        if (string.IsNullOrWhiteSpace(stated)) return Core.Units.Length.Zero;
        if (string.Equals(stated.Trim(), "none", StringComparison.Ordinal)) return Core.Units.Length.Zero;

        foreach (string part in stated.Split(' ', StringSplitOptions.RemoveEmptyEntries))
        {
            if (OdfValue.ParseLength(part) is { } measured)
            {
                Core.Units.Length offset = OdfWriterUnits.ToCore(measured);
                if (offset > Core.Units.Length.Zero) return offset;
            }
        }

        return Core.Units.Length.Zero;
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
    ///     a header declaring 6 mm around one 12 pt line renders 4.9 mm tall.
    ///   </item>
    /// </list>
    /// <para>
    /// The dynamic case therefore needs the header's content laid out, which is what
    /// <paramref name="content"/> is: <c>SwHeadFootFrame::FormatPrt</c> takes
    /// <c>nHeight = lcl_CalcContentHeight(*this)</c> whenever <c>!HasFixSize()</c> and raises it to the
    /// stated minimum only if it falls short (<c>sw/source/core/layout/hffrm.cxx</c>:114-145). With no
    /// content measured the declared minimum stands in for it — the same approximation LibreOffice's own
    /// DOC exporter falls back to, and which its comment calls "totally nonoptimum, but the best we can
    /// do" (<c>sw/source/filter/ww8/writerwordglue.cxx</c>) — and that is what every caller but the
    /// layout gets.
    /// </para>
    /// <para>
    /// <strong>Whether the spacing is then added on top of the minimum is
    /// <c>style:dynamic-spacing</c>'s to say, and Writer writes it on.</strong> It maps to
    /// <c>SwHeaderAndFooterEatSpacingItem</c> (<c>sw/source/core/bastyp/init.cxx</c>:434) and the
    /// header <em>eats</em> the gap: <c>SwHeadFootFrame::FormatPrt</c> starts the print area's minimum
    /// at <c>fo:min-height</c> less both spacings, then gives back out of the spacing exactly what the
    /// content overruns that by (<c>sw/source/core/layout/hffrm.cxx</c>:116-170). Rendering the pair of
    /// readings against 26.2.4.2 and measuring the first body line reduces both to one formula:
    /// </para>
    /// <code>
    /// total = max(min-height, content + (dynamic-spacing ? 0 : gap))
    /// </code>
    /// <para>
    /// — verified over sixteen shapes, among them a 1 cm minimum under a 1 cm gap (28.35 pt with the
    /// flag, 41.80 without) and a 2 cm minimum under a 3 cm gap (56.70 against 98.50). So the declared
    /// minimum <em>includes</em> the gap when the flag is set, and a header whose content fits occupies
    /// the minimum and not one twip more. That branch is therefore exact here, and it is the one that
    /// matters: Writer writes the attribute on everything it exports and writes it <c>true</c>.
    /// </para>
    /// <para>
    /// <strong>Both branches are now exact, because the content is measured.</strong> The formula
    /// wants a content height, and the paragraph above used to say it could not be had before the page
    /// the header sits on is known — so the unflagged branch kept the older <c>minimum + gap</c> and
    /// overshot by the difference (56.70 against the reference's 41.80 on the first shape above), and
    /// the flagged branch quietly took the floor as the height. Neither is true of a header: its blocks
    /// are read by the same walk the body's are, its width is the body's text width whatever the
    /// pagination does, and nothing about its height depends on which page it lands on.
    /// <see cref="OdtWordDocument"/> lays it out once and re-reads the geometry with the answer, which
    /// is what <paramref name="content"/> carries. A caller that does not measure it — extraction, which
    /// must not pay for a layout — passes zero and gets exactly the readings this file gave before.
    /// </para>
    /// <para>
    /// Reading the flag at all is the difference between a body that starts where the file says and one
    /// pushed down by the whole gap on every page. 152 of the 338 <c>.odt</c> of the converted corpus
    /// declare a dynamic-height header or footer with the flag on and a gap worth more than half a
    /// point; the gaps run to 89 pt, which on a letter page is four lines of text and a page every
    /// fifteen.
    /// </para>
    /// </remarks>
    private static Length FurnitureExtent(OdfPropertySet? properties, Length content)
    {
        if (properties is null) return Core.Units.Length.Zero;

        if (OdfValue.ParseLength(properties.Get(OdfNamespaces.SvgCompatible, "height")) is { } fixedHeight)
        {
            return OdfWriterUnits.ToCore(fixedHeight);
        }

        Length declared = OdfWriterUnits.ToCore(
            OdfValue.ParseLength(properties.Get(OdfNamespaces.FoCompatible, "min-height")))
            ?? Core.Units.Length.Zero;

        bool eats = EatsSpacing(properties);
        Length gap = FurnitureSpacing(properties);

        // With the content measured, the formula above is exact and no approximation is needed in
        // either branch. Without it — which is every caller that is not laying the document out — the
        // older readings stand unchanged, so nothing that never measures the content moves.
        if (content > Core.Units.Length.Zero)
        {
            return Core.Units.Length.Max(declared, eats ? content : content + gap);
        }

        return eats ? declared : declared + gap;
    }

    /// <summary>
    /// True when the furniture absorbs the gap below it rather than adding it to its own height.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The attribute has to be there and has to say so. <c>xmloff</c> maps it to the MAYBEVOID
    /// property <c>HeaderDynamicSpacing</c> (<c>xmloff/source/style/PageMasterStyleMap.cxx</c>:195,
    /// :251; <c>sw/source/core/unocore/unomap1.cxx</c>:513, :546), so an absent attribute sets nothing
    /// and the item keeps its pool default — and that default is <em>off</em>:
    /// <c>SwHeaderAndFooterEatSpacingItem</c>'s constructor takes <c>bPrt = false</c>
    /// (<c>sw/inc/hfspacingitem.hxx</c>:30-31) and the pool entry uses it
    /// (<c>sw/source/core/bastyp/init.cxx</c>:434).
    /// </para>
    /// <para>
    /// In practice Writer writes the attribute on everything it exports and writes it <c>true</c>: of
    /// the 543 header and footer styles that carry a properties child at all in the 338 converted
    /// <c>.odt</c>, 542 state the attribute, 541 of those
    /// say <c>true</c> and one says <c>false</c>. The absent case is therefore the rare one, and
    /// reading it as off is both what the item does and the older behaviour, so a file that says
    /// nothing does not move.
    /// </para>
    /// </remarks>
    private static bool EatsSpacing(OdfPropertySet properties)
        => string.Equals(
            properties.Get(OdfNamespaces.Style, "dynamic-spacing"),
            "true",
            StringComparison.OrdinalIgnoreCase);

    /// <summary>
    /// True when the furniture states a height rather than a floor, so it does not grow with its content.
    /// </summary>
    /// <remarks>
    /// The same test <see cref="FurnitureExtent"/> makes, asked as a question rather than as a measurement,
    /// because the paginator needs the answer and not the number: <c>svg:height</c> is
    /// <c>SwFrameSize::Fixed</c> and <c>fo:min-height</c> is <c>SwFrameSize::Minimum</c>, and only the
    /// second lets a running head or foot move the body.
    /// </remarks>
    private static bool HasFixedHeight(OdfPropertySet? properties)
        => properties is not null
           && OdfValue.ParseLength(properties.Get(OdfNamespaces.SvgCompatible, "height")) is not null;

    /// <summary>
    /// The gap between a header or footer and the body text.
    /// </summary>
    /// <remarks>
    /// The margin below a header, or above a footer. Both are written as the side facing the body, so
    /// whichever is present is the spacing that separates the furniture from the text — which makes one
    /// lookup with a fallback right for both, and means neither caller has to say which it is asking about.
    /// </remarks>
    private static Length FurnitureSpacing(OdfPropertySet? properties)
        => OdfWriterUnits.ToCore(
               OdfValue.ParseLength(properties?.Get(OdfNamespaces.FoCompatible, "margin-bottom"))
               ?? OdfValue.ParseLength(properties?.Get(OdfNamespaces.FoCompatible, "margin-top")))
           ?? Core.Units.Length.Zero;

    /// <summary>
    /// A measure from a page layout, on Writer's own grid.
    /// </summary>
    /// <remarks>
    /// Rounded to whole twips at the point it is read, because that is what LibreOffice's import does and
    /// because everything downstream compares against a layout that was measured that way — a margin
    /// carrying a third of a twip of extra precision narrows the text area by it on both sides.
    /// </remarks>
    private static Length? Length(OdfPropertySet? properties, string localName)
        => OdfWriterUnits.ToCore(
            OdfValue.ParseLength(properties?.Get(OdfNamespaces.FoCompatible, localName)));

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
    internal static int ColumnCount(OdfPropertySet? properties)
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
    /// The columns as the file states them, one by one, or null when it does not state them that way.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b><c>fo:column-gap</c> wins outright, and this is the whole of the rule.</b>
    /// <c>XMLTextColumnsContext::endFastElement</c> takes the per-column descriptions only when
    /// <c>!bAutomatic &amp;&amp; maColumns.size() == nCount</c>, and <c>bAutomatic</c> is set by the mere
    /// <em>presence</em> of <c>fo:column-gap</c> (<c>xmloff/source/text/XMLTextColumnsContext.cxx</c>
    /// :216-222, :268-315). Everything else takes <c>setColumnCount</c> and an automatic distance, which
    /// is even columns — so a file stating both a gap and unequal <c>style:rel-width</c> is drawn with
    /// <em>even</em> columns, and reading its widths would be wrong. Censused over the 338 converted
    /// <c>.odt</c>: 125 <c>style:columns</c> state a gap, of which 45 also state unequal widths that
    /// LibreOffice ignores, and 21 state per-column widths with no gap — the ones this reads.
    /// </para>
    /// <para>
    /// A <c>style:rel-width</c> is the column's <em>outer</em> width, its own two indents included:
    /// <c>SwFormatCol::Calc</c> apportions the frame between the wish widths and then takes each
    /// column's left and right margin off its own share, and the DOCX importer builds the same shape
    /// (<c>pColumn[nCol].Width = (fWidth + fLeft + fRight) * fRel</c>,
    /// <c>dmapper/PropertyMap.cxx</c>:868-874). So the text width is the apportioned share less the
    /// column's <c>fo:start-indent</c> and <c>fo:end-indent</c>, and the gap between two columns is the
    /// first's end indent plus the second's start indent.
    /// </para>
    /// <para>
    /// A start indent on the <em>first</em> column and an end indent on the <em>last</em> have nowhere to
    /// go in a <see cref="ColumnRuler"/>, which measures from the text area's own edge; both are zero in
    /// every one of the 21, because that is how LibreOffice's exporter splits a gap.
    /// </para>
    /// </remarks>
    /// <param name="properties">The <c>style:page-layout-properties</c> or <c>style:section-properties</c>.</param>
    /// <param name="measure">The width the columns have to fill.</param>
    internal static ColumnRuler? ColumnRulerOf(OdfPropertySet? properties, Length measure)
    {
        if (properties?.Child(OdfNamespaces.Style, "columns") is not { } columns) return null;
        if (columns.Attribute(XName.Get("column-gap", OdfNamespaces.FoCompatible)) is not null) return null;

        List<XElement> stated = [.. columns.Elements(XName.Get("column", OdfNamespaces.Style))];
        int count = ColumnCount(properties);

        if (count < 2 || stated.Count != count || measure <= Core.Units.Length.Zero) return null;

        long[] relative = new long[count];
        Length[] starts = new Length[count];
        Length[] ends = new Length[count];
        long total = 0;
        int described = 0;

        for (int i = 0; i < count; i++)
        {
            string? text = stated[i].Attribute(XName.Get("rel-width", OdfNamespaces.Style))?.Value;
            int star = text?.IndexOf('*') ?? -1;
            if (star > 0 && long.TryParse(text.AsSpan(0, star), out long width) && width > 0)
            {
                relative[i] = width;
                total += width;
                described++;
            }

            starts[i] = Indent(stated[i], "start-indent");
            ends[i] = Indent(stated[i], "end-indent");
        }

        // A column with no width of its own takes the average of those that have one, and a set with none
        // is shared evenly — which is what xmloff's own fill-in does (`:288-306`).
        if (described < count)
        {
            long each = total == 0 ? 1 : total / Math.Max(described, 1);
            for (int i = 0; i < count; i++)
            {
                if (relative[i] == 0) { relative[i] = each; total += each; }
            }
        }

        List<Length> widths = new(count);
        List<Length> gaps = new(Math.Max(count - 1, 0));
        Length taken = Core.Units.Length.Zero;

        for (int i = 0; i < count; i++)
        {
            // The last column takes what the others left, so the apportioning cannot lose an EMU.
            Length outer = i == count - 1
                ? measure - taken
                : Core.Units.Length.FromEmu((long)(measure.Emu * (double)relative[i] / total));
            taken += outer;

            Length width = outer - starts[i] - ends[i];
            widths.Add(width > Core.Units.Length.Zero ? width : Core.Units.Length.Zero);
            if (i < count - 1) gaps.Add(ends[i] + starts[i + 1]);
        }

        return new ColumnRuler(widths, gaps);
    }

    /// <summary>One of a <c>style:column</c>'s two indents, nought when it states none.</summary>
    private static Length Indent(XElement column, string localName)
        => OdfWriterUnits.ToCore(
               OdfValue.ParseLength(
                   column.Attribute(XName.Get(localName, OdfNamespaces.FoCompatible))?.Value))
           ?? Core.Units.Length.Zero;

    /// <summary>
    /// The gap between columns.
    /// </summary>
    /// <remarks>
    /// ODF states it as <c>fo:column-gap</c> on the columns element, or — for unequal columns — as a
    /// margin on each column, in which case the first column's right margin is representative. Taking
    /// it from the first rather than averaging keeps the common case exact.
    /// </remarks>
    internal static Length ColumnGap(OdfPropertySet? properties)
    {
        if (properties?.Child(OdfNamespaces.Style, "columns") is not { } columns)
        {
            return Core.Units.Length.Zero;
        }

        if (OdfValue.ParseLength(columns.Attribute(XName.Get("column-gap", OdfNamespaces.FoCompatible))?.Value)
            is { } gap)
        {
            return OdfWriterUnits.ToCore(gap);
        }

        XElement? first = columns.Elements(XName.Get("column", OdfNamespaces.Style)).FirstOrDefault();
        return OdfWriterUnits.ToCore(
                   OdfValue.ParseLength(
                       first?.Attribute(XName.Get("end-indent", OdfNamespaces.FoCompatible))?.Value))
               ?? Core.Units.Length.Zero;
    }
}
