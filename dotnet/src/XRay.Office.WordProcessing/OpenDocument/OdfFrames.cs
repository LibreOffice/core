using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.WordProcessing.Layout;

namespace XRay.Office.WordProcessing.OpenDocument;

/// <summary>
/// Reads a <c>draw:frame</c> — ODF's floating frame — into the layout engine's own model.
/// </summary>
/// <remarks>
/// <para>
/// ODF splits a frame across two places and neither is complete on its own. The element carries the
/// anchor and the geometry — <c>text:anchor-type</c>, <c>svg:x</c>, <c>svg:y</c>, <c>svg:width</c>,
/// <c>svg:height</c> — and its <em>graphic style</em> carries everything about how text behaves near it:
/// <c>style:wrap</c>, the two <c>-pos</c>/<c>-rel</c> pairs, and the margins that widen the hole without
/// moving the frame. So reading the element alone gives a frame in the right place that no text avoids.
/// </para>
/// <para>
/// Two of the attribute vocabularies are worth spelling out, because their obvious readings are wrong.
/// <c>style:wrap="none"</c> means "no text <em>beside</em> it" — the text goes above and below — while
/// ODF's word for "ignore it" is <c>run-through</c>. And <c>style:horizontal-pos="from-left"</c> is the
/// only value that uses <c>svg:x</c> at all: <c>left</c>, <c>center</c> and <c>right</c> align against
/// whatever <c>style:horizontal-rel</c> names and ignore the coordinate entirely.
/// </para>
/// </remarks>
internal static class OdfFrames
{
    /// <summary>
    /// Every frame one anchored element places, which is more than one when it is a group.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>draw:g</c> states no geometry of its own — no <c>svg:width</c>, no <c>svg:height</c> — so
    /// the public <c>Read</c> answers null for one and the whole group vanishes. ODF gives a group no
    /// coordinate system either: there is no <c>svg:viewBox</c> and no transform on the element, so
    /// every child already carries its position in the same space the group is anchored in
    /// (<c>xmloff/source/draw/ximpgrp.cxx</c> creates an <c>SdrObjGroup</c> and reads the children
    /// straight into the page's coordinates). Flattening the group to its shapes is therefore the whole
    /// of it, and the shapes need no adjustment.
    /// </para>
    /// <para>
    /// What a child does <em>not</em> restate is where the group sits relative to the text —
    /// <c>style:wrap</c> and the two <c>-pos</c>/<c>-rel</c> pairs are on the group's own graphic style
    /// and its children's styles carry only fill and stroke. Those five values travel down; fill,
    /// stroke and the margins do not, because ODF does not cascade a group's graphic style onto its
    /// children.
    /// </para>
    /// </remarks>
    /// <param name="element">The anchored <c>draw:</c> element.</param>
    /// <param name="styles">The document's styles.</param>
    /// <param name="content">
    /// How to read a shape's own paragraphs, or null to record the frames without their content —
    /// which is what an image needs and all the wrap ever depends on.
    /// </param>
    /// <param name="anchorOffset">Where in the paragraph's text the element is anchored.</param>
    /// <param name="pictures">
    /// How to reach the bytes behind a <c>draw:image</c>, or null to record the geometry without them.
    /// </param>
    public static List<PageFrame> ReadAll(
        XElement element,
        OdfStyles styles,
        Func<XElement, IReadOnlyList<PageBlock>>? content,
        int anchorOffset,
        OdfPictures? pictures = null)
    {
        ArgumentNullException.ThrowIfNull(element);
        ArgumentNullException.ThrowIfNull(styles);

        List<PageFrame> frames = [];
        Collect(element, null, null, 0);
        return frames;

        void Collect(XElement node, OdfGraphicStyle? outer, string? anchor, int depth)
        {
            if (frames.Count >= MaxGroupMembers) return;

            if (IsGroup(node))
            {
                if (depth >= MaxGroupNesting) return;

                OdfGraphicStyle group = Placed(GraphicStyle(styles, StyleName(node)), outer);
                string? groupAnchor = AnchorType(node) ?? anchor;
                foreach (XElement child in node.Elements())
                {
                    if (child.Name.NamespaceName == OdfNamespaces.Draw)
                        Collect(child, group, groupAnchor, depth + 1);
                }

                return;
            }

            if (Read(node, styles, content, anchorOffset, pictures, outer, anchor) is { } frame)
            {
                frames.Add(frame);
            }
        }
    }

    /// <summary>
    /// Where a fly holding nothing but a table puts that table, or null when the element is not one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A <em>floating table</em> is one object with two spellings. Word states it on the table itself as
    /// <c>w:tblpPr</c> and LibreOffice's DOCX importer turns that into a fly holding a table
    /// (<c>TablePositionHandler::getTablePosition</c>), always splittable —
    /// <c>sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx</c>:1765, <em>"A text frame created
    /// for floating tables is always allowed to split"</em>. ODF states the same object the other way up:
    /// a <c>draw:frame</c> whose <c>draw:text-box</c> holds one table and whose element carries
    /// <c>loext:may-break-between-pages="true"</c>
    /// (<c>xmloff/source/text/XMLTextFrameContext.cxx</c>:1104-1107 reads both that spelling and the
    /// <c>draw:</c> one into <c>IsSplitAllowed</c>; <c>xmloff/source/text/txtparae.cxx</c>:3115-3119
    /// writes it, in the extension namespace and only when true).
    /// </para>
    /// <para>
    /// They are the same object and this engine already models it — as a <em>positioned block table</em>
    /// rather than as a frame, because that is the model that splits: <c>Paginator.PlaceFloatedTable</c>
    /// carries the rows a page cannot take and <c>ContinueFloatedTables</c> puts them at the top of the
    /// next one, starting a page of its own when the flow has none left. Read as an ordinary frame
    /// instead, the same table is measured whole and its tail is drawn off the bottom of the sheet.
    /// </para>
    /// <para>
    /// The refusals are <c>SwFlyFrame::IsFlySplitAllowed</c>'s
    /// (<c>sw/source/core/layout/fly.cxx</c>:689-737): the anchor must be at-content —
    /// <c>text:anchor-type</c> <c>paragraph</c> or <c>char</c>, since <c>IsFlyAtContentFrame</c> is what
    /// the first test asks — and a fly in a header, a footer, a footnote or a multi-column section is
    /// refused whatever it states. The last three are the caller's to enforce, because only the caller
    /// knows which flow it is walking; the anchor and the attribute are here.
    /// </para>
    /// <para>
    /// <b>All 51 such frames in the converted corpus hold a table and nothing else</b> — not one holds a
    /// paragraph — so the "or a paragraph flow" arm this could grow has no witness and is deliberately
    /// absent rather than written blind.
    /// </para>
    /// </remarks>
    /// <param name="element">The candidate <c>draw:frame</c>.</param>
    /// <param name="styles">The document's styles, for the frame's own graphic style.</param>
    public static FloatingTableFrame? FloatingTable(XElement element, OdfStyles styles)
    {
        ArgumentNullException.ThrowIfNull(element);
        ArgumentNullException.ThrowIfNull(styles);

        if (element.Name != XName.Get("frame", OdfNamespaces.Draw)) return null;
        if (AnchorType(element) is not ("paragraph" or "char")) return null;
        if (!MayBreakBetweenPages(element)) return null;
        if (Measure(element, "width") is not { } width) return null;

        XElement? box = element.Element(XName.Get("text-box", OdfNamespaces.Draw));
        if (box is null) return null;

        XElement? table = null;
        foreach (XElement child in box.Elements())
        {
            if (child.Name.LocalName != "table" || !OdfNamespaces.IsTable(child.Name.NamespaceName))
                return null;
            if (table is not null) return null;

            table = child;
        }

        if (table is null) return null;

        OdfGraphicStyle style = GraphicStyle(styles, StyleName(element));

        return new FloatingTableFrame(
            table,
            width,
            HorizontalAlignmentOf(style.HorizontalPosition),
            Measure(element, "x") ?? Length.Zero,
            style.HorizontalRelative is "page",
            VerticalOriginOf(style.VerticalRelative),
            Measure(element, "y") ?? Length.Zero,
            style.Spacing.Bottom);
    }

    /// <summary>
    /// Whether the frame says its fly may be broken across pages.
    /// </summary>
    /// <remarks>
    /// Both spellings, because the importer reads both — the extension one is what LibreOffice's own
    /// exporter writes and the <c>draw:</c> one is what a producer following a later specification would.
    /// </remarks>
    private static bool MayBreakBetweenPages(XElement element)
        => element.Attribute(XName.Get("may-break-between-pages", OdfNamespaces.LoExt))?.Value == "true"
           || element.Attribute(XName.Get("may-break-between-pages", OdfNamespaces.Draw))?.Value == "true";

    /// <summary>A group nested deeper than this is not read; real documents nest two or three.</summary>
    private const int MaxGroupNesting = 16;

    /// <summary>A guard on untrusted input: one anchor cannot place more frames than this.</summary>
    private const int MaxGroupMembers = 4096;

    private static bool IsGroup(XElement element)
        => element.Name == XName.Get("g", OdfNamespaces.Draw);

    private static string? StyleName(XElement element)
        => element.Attribute(XName.Get("style-name", OdfNamespaces.Draw))?.Value;

    private static string? AnchorType(XElement element)
        => element.Attribute(XName.Get("anchor-type", OdfNamespaces.Text))?.Value;

    private static PageFrame? Read(
        XElement element,
        OdfStyles styles,
        Func<XElement, IReadOnlyList<PageBlock>>? content,
        int anchorOffset,
        OdfPictures? pictures,
        OdfGraphicStyle? outer,
        string? inheritedAnchor)
    {
        Length? width = Measure(element, "width");
        if (width is not { } frameWidth) return null;

        OdfGraphicStyle style = Placed(GraphicStyle(styles, StyleName(element)), outer);

        XElement? textBox = element.Element(XName.Get("text-box", OdfNamespaces.Draw));
        XElement? box = textBox ?? ShapeTextBody(element);

        // svg:height is optional and a frame written without one grows to its text. Read after the
        // box, because the box is what says how tall an unstated height starts out.
        Length? stated = Measure(element, "height");
        if ((stated ?? AutoHeight(box)) is not { } frameHeight) return null;

        XElement? image = element.Element(XName.Get("image", OdfNamespaces.Draw));
        FramePicture picture =
            image is not null && pictures is not null ? pictures.Read(element) : FramePicture.None;

        // The chart before the picture, because a frame holding one holds both: ODF lists a frame's
        // children as alternatives in decreasing order of preference, and a chart is written as a
        // draw:object followed by a draw:image of it for a reader that cannot embed one.
        ChartPlot? chart = pictures?.Chart(element);

        return new PageFrame
        {
            Size = new DocSize(frameWidth, frameHeight),
            Anchor = AnchorOf(AnchorType(element) ?? inheritedAnchor),
            AnchorOffset = anchorOffset,
            Wrap = WrapOf(style.Wrap),
            HorizontalOrigin = HorizontalOriginOf(style.HorizontalRelative),
            HorizontalAlignment = HorizontalAlignmentOf(style.HorizontalPosition),
            HorizontalOffset = Measure(element, "x") ?? Translation(element).X,
            VerticalOrigin = VerticalOriginOf(style.VerticalRelative),
            VerticalAlignment = VerticalAlignmentOf(style.VerticalPosition),
            VerticalOffset = Measure(element, "y") ?? Translation(element).Y,
            Spacing = style.Spacing,
            Padding = style.Padding,
            Fill = style.Fill,
            BorderColour = style.BorderColour,
            BorderWidth = style.BorderWidth,
            GrowsToContent = GrowsToContent(textBox, stated),
            IsImage = image is not null && chart is null,
            Image = picture.Raster,
            Vector = picture.Vector,
            Chart = chart,
            Name = element.Attribute(XName.Get("name", OdfNamespaces.Draw))?.Value,
            Blocks = box is not null && content is not null ? content(box) : [],
        };
    }

    /// <summary>
    /// The height a frame starts at when it states none, or null when it cannot grow to one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>svg:height</c> is optional on <c>draw:frame</c> (ODF 1.3 §10.4.2 makes both lengths
    /// optional) and Writer leaves it off a frame whose height is its content's — the shape a text
    /// box or a framed table has after "autofit height". What it writes instead is the floor, on the
    /// box: <c>&lt;draw:text-box fo:min-height="0in"&gt;</c>. The frame is <c>SwFrameSize::Minimum</c>
    /// vertically and grows from there.
    /// </para>
    /// <para>
    /// <strong>Requiring both lengths therefore dropped the frame and everything inside it</strong>,
    /// which for these documents is not a decoration: 49 of the 338 converted <c>.odt</c> hold 58 such
    /// frames between them carrying <strong>50 942 alphanumeric characters</strong>, and in ten of
    /// them the frame is the page — <c>020_Project_Timeline_Template_Modern_Theme</c> rendered 0
    /// characters against the reference's 316, and <c>ESPN-R - MCF - RA - Ed1</c> keeps 21 623 in a
    /// single one. Nothing reported it: an anchored element that cannot be measured is skipped, and a
    /// page with no frame on it looks like a page whose author put no frame on it.
    /// </para>
    /// <para>
    /// Null rather than zero when there is no text box at all, so a picture or a shape written without
    /// a height keeps being skipped: those have nothing to grow from, and a zero-tall image is not an
    /// improvement on an absent one.
    /// </para>
    /// </remarks>
    private static Length? AutoHeight(XElement? box)
    {
        if (box is null) return null;

        return OdfWriterUnits.ToCore(
                   OdfValue.ParseLength(box.Attribute(XName.Get("min-height", OdfNamespaces.FoCompatible))?.Value))
               ?? Core.Units.Length.Zero;
    }

    /// <summary>
    /// Whether the frame's height is a <em>floor</em> that its own text grows past, rather than the
    /// height it keeps.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer keeps three height kinds on a fly — <c>SwFrameSize::Fixed</c>, <c>Minimum</c> and
    /// <c>Variable</c> — and only the first keeps the stated number. ODF states the choice by which
    /// attribute carries the length: <c>fo:min-height</c> on the <c>draw:text-box</c> sets
    /// <c>bMinHeight</c> and therefore <c>SizeType::MIN</c>, and <c>svg:height</c> on the
    /// <c>draw:frame</c> sets <c>SizeType::FIX</c>
    /// (<c>xmloff/source/text/XMLTextFrameContext.cxx</c>:997-1010 and :655-661). Both write the
    /// <em>same</em> variable, the box's attributes first and the frame's second (:1113-1116), so a
    /// frame stating both keeps <c>svg:height</c> as the floor and still grows — which no document in
    /// the converted corpus does, but which is the rule rather than a simplification.
    /// </para>
    /// <para>
    /// A frame that states neither is neither: the <c>SizeType</c> is left at the text frame's own
    /// default, which is <c>Variable</c> — grow to the content with no floor at all. That is the same
    /// behaviour as a floor of nought, which is what <see cref="AutoHeight"/> already answers, so the
    /// two cases need not be told apart here.
    /// </para>
    /// <para>
    /// Only a <c>draw:text-box</c> counts. A <c>draw:custom-shape</c> or a <c>draw:rect</c> holding
    /// paragraphs is an <c>SdrTextObj</c> rather than a fly, and how a drawing shape fits itself to its
    /// text is a different rule with a different seat — see the remarks on
    /// <see cref="ShapeTextBody"/>.
    /// </para>
    /// </remarks>
    private static bool GrowsToContent(XElement? textBox, Length? statedHeight)
        => textBox is not null
           && (statedHeight is null
               || textBox.Attribute(XName.Get("min-height", OdfNamespaces.FoCompatible)) is not null);

    /// <summary>
    /// Where <c>draw:transform</c> puts a shape that states no <c>svg:x</c> or <c>svg:y</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A turned shape is written with a transform instead of a coordinate pair —
    /// <c>rotate (-1.5707963267949) translate (0.1253in 5.2586in)</c> — and its <c>svg:width</c> and
    /// <c>svg:height</c> stay on the element. Only the translation is read here: it is where the shape
    /// ends up, and taking it is the difference between a turned label sitting where the file put it
    /// and every turned label in the document piling up on the anchor. The rotation is deliberately
    /// not applied to the text; drawing it upright at the right place is the smaller error of the two,
    /// and the one a reader can still read.
    /// </para>
    /// <para>
    /// Reach on the converted corpus: <strong>581 alphanumeric characters across 11 of the 338
    /// <c>.odt</c></strong> sit in a shape placed this way, against 27 856 in 102 documents placed by
    /// <c>svg:x</c>. Small, and it is the case where two shapes otherwise land on the same point —
    /// which costs more than its share, because a page's extractable text counts overlapping copies
    /// once.
    /// </para>
    /// </remarks>
    private static (Length X, Length Y) Translation(XElement element)
    {
        string? transform = element.Attribute(XName.Get("transform", OdfNamespaces.Draw))?.Value;
        if (string.IsNullOrWhiteSpace(transform)) return (Length.Zero, Length.Zero);

        int at = transform.IndexOf("translate", StringComparison.Ordinal);
        if (at < 0) return (Length.Zero, Length.Zero);

        int open = transform.IndexOf('(', at);
        int close = open < 0 ? -1 : transform.IndexOf(')', open);
        if (close < 0) return (Length.Zero, Length.Zero);

        string[] parts = transform[(open + 1)..close]
            .Split([' ', ','], StringSplitOptions.RemoveEmptyEntries);
        if (parts.Length < 2) return (Length.Zero, Length.Zero);

        return (OdfWriterUnits.ToCore(OdfValue.ParseLength(parts[0])) ?? Length.Zero,
                OdfWriterUnits.ToCore(OdfValue.ParseLength(parts[1])) ?? Length.Zero);
    }

    /// <summary>
    /// The element whose children are a shape's text, which for a shape is the shape itself.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A <c>draw:frame</c> holds its text in a <c>draw:text-box</c> child; a <c>draw:custom-shape</c>
    /// or a <c>draw:rect</c> holds it in <c>text:p</c> children of its own, with no box in between
    /// (ODF 1.3 §10.6.2 — the shape elements take <c>&lt;text:p&gt;*</c> directly, and
    /// <c>SdrTextObj</c> is what the importer hangs it on). Looking only for the box therefore finds
    /// nothing on every shape in the document and the text is silently absent: the frame is still
    /// placed, still filled and still stroked, so nothing about the page says a paragraph is missing.
    /// </para>
    /// <para>
    /// Measured on the converted corpus: <strong>2839 <c>draw:custom-shape</c> across 129 of the 338
    /// <c>.odt</c> carry 42 773 alphanumeric characters between them</strong>, and 9 <c>draw:rect</c>
    /// carry 45 more. No other <c>draw:</c> element carries one character — lines, connectors and
    /// controls are all empty — which is why this looks for the paragraphs rather than for a list of
    /// element names.
    /// </para>
    /// </remarks>
    private static XElement? ShapeTextBody(XElement element)
    {
        if (element.Name == XName.Get("frame", OdfNamespaces.Draw)) return null;

        foreach (XElement child in element.Elements())
        {
            if (child.Name.NamespaceName == OdfNamespaces.Text
                && child.Name.LocalName is "p" or "h" or "list")
            {
                return element;
            }

            // A shape whose whole content is a table has no paragraph of its own, and a roadmap
            // diagram's boxes are written exactly that way — see OdfNamespaces.IsTable for why the
            // element is usually spelled `loext:table` in this position.
            if (OdfNamespaces.IsTable(child.Name.NamespaceName) && child.Name.LocalName == "table")
            {
                return element;
            }
        }

        return null;
    }

    /// <summary>
    /// A shape's own style with the enclosing group's placement laid <em>over</em> it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Only the five values that decide where the thing sits relative to the text travel down a group:
    /// the wrap and the two position/relation pairs. Fill, stroke and the margins do not — ODF does not
    /// cascade a group's graphic style onto its children, and a child that states no fill is unfilled
    /// rather than filled like its group.
    /// </para>
    /// <para>
    /// <strong>The group wins, rather than filling a gap the child left</strong>, and the difference is
    /// not academic. A group's child is not independently anchored: the group is the one object in the
    /// text, and the child is a drawing object inside it whose <c>svg:x</c> and <c>svg:y</c> are
    /// coordinates in the group's space. Its graphic style is nonetheless derived from the named
    /// <c>Frame</c> style, which states <c>style:horizontal-pos="center"</c> and
    /// <c>style:vertical-pos="top"</c> — so a rule that only fills gaps reads that inherited default,
    /// discards both coordinates, and stacks every shape of every group at the top centre of the page.
    /// Measured on <c>003_Free_Genogram_Diagram_Template</c>: fourteen text boxes at fourteen distinct
    /// positions all drawn at 386.35, 501 on an A4 landscape page, which is exactly
    /// <c>margin + (content width − frame width) / 2</c> and the top margin.
    /// </para>
    /// </remarks>
    private static OdfGraphicStyle Placed(OdfGraphicStyle style, OdfGraphicStyle? outer)
        => outer is null
            ? style
            : style with
            {
                Wrap = outer.Wrap ?? style.Wrap,
                HorizontalPosition = outer.HorizontalPosition ?? style.HorizontalPosition,
                HorizontalRelative = outer.HorizontalRelative ?? style.HorizontalRelative,
                VerticalPosition = outer.VerticalPosition ?? style.VerticalPosition,
                VerticalRelative = outer.VerticalRelative ?? style.VerticalRelative,
            };

    /// <summary>True for an element that carries a floating frame.</summary>
    public static bool IsFrame(XElement element)
    {
        ArgumentNullException.ThrowIfNull(element);
        return element.Name.NamespaceName == OdfNamespaces.Draw
               && element.Name.LocalName is "frame" or "custom-shape" or "rect" or "g";
    }

    /// <summary>
    /// One <c>draw:frame</c> measurement, rounded to whole twips as it is read.
    /// </summary>
    /// <remarks>
    /// The same rounding every other ODF measure in this library takes, and for the same reason: Writer's
    /// core unit is twips and <c>SvXMLUnitConverter</c> converts straight into it, so 4 cm is 2267.7 twips
    /// and LibreOffice keeps 2268. Rounding later would leave the frame's right edge half a twip from
    /// where the reference draws it.
    /// </remarks>
    private static Length? Measure(XElement element, string name)
    {
        string? text = element.Attribute(XName.Get(name, OdfNamespaces.SvgCompatible))?.Value;
        return text is null ? null : OdfWriterUnits.ToCore(OdfValue.ParseLength(text));
    }

    private static FrameAnchor AnchorOf(string? value) => value switch
    {
        "page" => FrameAnchor.Page,
        "char" => FrameAnchor.Character,
        "as-char" => FrameAnchor.AsCharacter,
        _ => FrameAnchor.Paragraph,
    };

    /// <summary>
    /// ODF's <c>style:wrap</c>, whose value set does not line up with its name.
    /// </summary>
    /// <remarks>
    /// <c>none</c> is the trap: it means no text may sit beside the frame, so the text is pushed above and
    /// below it — the opposite of "no wrapping". <c>run-through</c> is the value that leaves the text
    /// alone. <c>biggest</c> is a synonym for <c>dynamic</c> that ODF 1.0 used and LibreOffice still
    /// writes for an imported Word document.
    /// </remarks>
    private static TextWrap WrapOf(string? value) => value switch
    {
        "none" => TextWrap.TopAndBottom,
        "left" => TextWrap.Left,
        "right" => TextWrap.Right,
        "parallel" => TextWrap.Both,
        "dynamic" or "biggest" => TextWrap.Optimal,
        _ => TextWrap.Through,
    };

    /// <remarks>
    /// <c>paragraph</c> is the anchor paragraph's whole rectangle and <c>paragraph-content</c> its text
    /// area; the two differ only by the paragraph's indents, which are not resolved here — so both map to
    /// the column. <c>page-start-margin</c> and its three siblings name the margin strips themselves,
    /// which are outside anything text can flow in, and are mapped to the page for want of anywhere
    /// better.
    /// </remarks>
    private static FrameHorizontalOrigin HorizontalOriginOf(string? value) => value switch
    {
        "page" or "page-start-margin" or "page-end-margin" => FrameHorizontalOrigin.Page,
        "page-content" => FrameHorizontalOrigin.PageMargin,
        "char" => FrameHorizontalOrigin.Character,
        "frame" or "frame-content" => FrameHorizontalOrigin.Column,
        _ => FrameHorizontalOrigin.Paragraph,
    };

    private static FrameHorizontalAlignment HorizontalAlignmentOf(string? value) => value switch
    {
        "left" => FrameHorizontalAlignment.Left,
        "center" => FrameHorizontalAlignment.Centre,
        "right" => FrameHorizontalAlignment.Right,
        "inside" => FrameHorizontalAlignment.Inside,
        "outside" => FrameHorizontalAlignment.Outside,
        _ => FrameHorizontalAlignment.Offset,
    };

    private static FrameVerticalOrigin VerticalOriginOf(string? value) => value switch
    {
        "page" => FrameVerticalOrigin.Page,
        "page-content" => FrameVerticalOrigin.PageMargin,
        "line" or "char" or "baseline" or "text" => FrameVerticalOrigin.Line,
        _ => FrameVerticalOrigin.Paragraph,
    };

    private static FrameVerticalAlignment VerticalAlignmentOf(string? value) => value switch
    {
        "top" => FrameVerticalAlignment.Top,
        "middle" => FrameVerticalAlignment.Middle,
        "bottom" or "below" => FrameVerticalAlignment.Bottom,
        _ => FrameVerticalAlignment.Offset,
    };

    /// <summary>
    /// The graphic style a frame names, resolved up its parent chain.
    /// </summary>
    /// <remarks>
    /// Up the chain because LibreOffice writes a per-frame automatic style whose parent is the named
    /// <c>Frame</c> style, and the wrap can be stated on either. Taking only the automatic style's own
    /// properties finds a frame with no wrap at all whenever the document set it through a style.
    /// </remarks>
    private static OdfGraphicStyle GraphicStyle(OdfStyles styles, string? name)
    {
        OdfGraphicStyle style = new();
        if (string.IsNullOrEmpty(name)) return style;

        foreach (OdfStyle level in Chain(styles, name))
        {
            if (level.Properties(OdfPropertyKind.Graphic) is { } properties)
            {
                style = style.With(properties);
            }
        }

        return style;
    }

    /// <summary>The style and its ancestors, outermost first, so a child's value overrides.</summary>
    private static List<OdfStyle> Chain(OdfStyles styles, string name)
    {
        List<OdfStyle> chain = [];
        HashSet<string> seen = new(StringComparer.Ordinal);
        string? at = name;

        while (at is not null && seen.Add(at))
        {
            OdfStyle? style = styles.Find(at, OdfStyleFamily.Graphic);
            if (style is null) break;

            chain.Insert(0, style);
            at = style.ParentStyleName;
        }

        return chain;
    }

    /// <summary>
    /// The graphic properties that decide where a frame goes and what text does about it.
    /// </summary>
    /// <remarks>
    /// A record built up level by level rather than read once, because a value stated by a parent style
    /// must survive a child that says nothing about it — which is exactly what LibreOffice's own export
    /// relies on when it puts the wrap on the named <c>Frame</c> style and the position on the automatic
    /// one.
    /// </remarks>
    private sealed record OdfGraphicStyle
    {
        public string? Wrap { get; init; }

        public string? HorizontalPosition { get; init; }

        public string? HorizontalRelative { get; init; }

        public string? VerticalPosition { get; init; }

        public string? VerticalRelative { get; init; }

        public Margins Spacing { get; init; }

        public Margins Padding { get; init; }

        public Colour? Fill { get; init; }

        public Colour? BorderColour { get; init; }

        public Length BorderWidth { get; init; }

        /// <summary>This style with one level's stated properties laid over it.</summary>
        public OdfGraphicStyle With(OdfPropertySet properties)
        {
            (Colour? borderColour, Length borderWidth) = Border(properties);

            return new OdfGraphicStyle
            {
                Wrap = Attribute(properties, "wrap", OdfNamespaces.Style) ?? Wrap,
                HorizontalPosition =
                    Attribute(properties, "horizontal-pos", OdfNamespaces.Style) ?? HorizontalPosition,
                HorizontalRelative =
                    Attribute(properties, "horizontal-rel", OdfNamespaces.Style) ?? HorizontalRelative,
                VerticalPosition =
                    Attribute(properties, "vertical-pos", OdfNamespaces.Style) ?? VerticalPosition,
                VerticalRelative =
                    Attribute(properties, "vertical-rel", OdfNamespaces.Style) ?? VerticalRelative,
                Spacing = Sides(properties, "margin", Spacing),
                Padding = Sides(properties, "padding", Padding),
                Fill = FillOf(properties) ?? Fill,
                BorderColour = borderColour ?? BorderColour,
                BorderWidth = borderWidth != Length.Zero ? borderWidth : BorderWidth,
            };
        }

        private static string? Attribute(OdfPropertySet properties, string name, string ns)
            => properties.Get(ns, name);

        /// <summary>
        /// The four sides of a shorthand-plus-per-side pair, keeping what the level does not state.
        /// </summary>
        private static Margins Sides(OdfPropertySet properties, string name, Margins inherited)
        {
            Length? all = Measure(properties, name);

            return new Margins(
                Measure(properties, name + "-left") ?? all ?? inherited.Left,
                Measure(properties, name + "-top") ?? all ?? inherited.Top,
                Measure(properties, name + "-right") ?? all ?? inherited.Right,
                Measure(properties, name + "-bottom") ?? all ?? inherited.Bottom);
        }

        private static Length? Measure(OdfPropertySet properties, string name)
        {
            string? text = Attribute(properties, name, OdfNamespaces.FoCompatible);
            return text is null ? null : OdfWriterUnits.ToCore(OdfValue.ParseLength(text));
        }

        /// <summary>
        /// The frame's background, which ODF states in two attributes that have to agree.
        /// </summary>
        /// <remarks>
        /// <c>draw:fill="none"</c> beats a <c>draw:fill-color</c> left over from a parent style, and
        /// <c>fo:background-color="transparent"</c> means the same thing in the other vocabulary — a
        /// reader taking the colour alone paints a frame the document asked to be see-through.
        /// </remarks>
        private static Colour? FillOf(OdfPropertySet properties)
        {
            string? kind = Attribute(properties, "fill", OdfNamespaces.Draw);
            if (kind == "none") return null;

            string? colour = Attribute(properties, "fill-color", OdfNamespaces.Draw)
                             ?? Attribute(properties, "background-color", OdfNamespaces.FoCompatible);

            if (colour is null || colour == "transparent") return null;

            return OdfValue.ParseColour(colour);
        }

        /// <summary>
        /// The border, from <c>fo:border</c>'s three-part shorthand.
        /// </summary>
        /// <remarks>
        /// Only the shorthand and only uniform borders: a frame with four different sides is drawn with
        /// the one <c>fo:border</c> states, and one stating none at all draws nothing. The per-side
        /// spellings exist and are left for whoever needs them, since a frame's four sides differing is
        /// rare where a table cell's differing is the norm.
        /// </remarks>
        private static (Colour? Colour, Length Width) Border(OdfPropertySet properties)
        {
            string? border = Attribute(properties, "border", OdfNamespaces.FoCompatible);
            if (border is null || border == "none") return (null, Core.Units.Length.Zero);

            Length width = Core.Units.Length.Zero;
            Colour? colour = null;

            foreach (string part in border.Split(' ', StringSplitOptions.RemoveEmptyEntries))
            {
                if (part.StartsWith('#')) colour = OdfValue.ParseColour(part);
                else if (char.IsAsciiDigit(part[0]))
                {
                    width = OdfWriterUnits.ToCore(OdfValue.ParseLength(part)) ?? width;
                }
            }

            return colour is null ? (null, Core.Units.Length.Zero) : (colour, width);
        }
    }
}

/// <summary>
/// A <c>draw:frame</c> that is a floating table: the table it holds, and where its fly puts it.
/// </summary>
/// <remarks>
/// The fields are exactly what <see cref="PageTable"/> needs to be a positioned table, which is this
/// engine's model of the same object — see <see cref="OdfFrames.FloatingTable"/> for why the two
/// spellings are one thing and for the refusals the caller still has to apply.
/// </remarks>
/// <param name="Table">The <c>table:table</c> — or <c>loext:table</c> — element inside the fly.</param>
/// <param name="Width">The fly's <c>svg:width</c>, which is the room the table has across.</param>
/// <param name="Horizontal">
/// How the fly is aligned across whatever it is relative to. <see cref="FrameHorizontalAlignment.Offset"/>
/// is ODF's <c>from-left</c>, the one value that uses <see cref="HorizontalOffset"/> at all.
/// </param>
/// <param name="HorizontalOffset">The fly's <c>svg:x</c>, meaningful only under <c>from-left</c>.</param>
/// <param name="HorizontalFromSheet">
/// True when <c>style:horizontal-rel="page"</c>, so the offset is measured from the sheet's own left edge
/// rather than from the text area's — the same distinction OOXML draws with <c>w:horzAnchor="page"</c>.
/// </param>
/// <param name="VerticalOrigin">What <see cref="VerticalOffset"/> is measured down from.</param>
/// <param name="VerticalOffset">The fly's <c>svg:y</c>.</param>
/// <param name="LowerSpacing">
/// The fly's <c>fo:margin-bottom</c>: the room it keeps clear below itself, which belongs to the frame
/// rather than to the table and is OOXML's <c>w:bottomFromText</c>.
/// </param>
internal readonly record struct FloatingTableFrame(
    XElement Table,
    Length Width,
    FrameHorizontalAlignment Horizontal,
    Length HorizontalOffset,
    bool HorizontalFromSheet,
    FrameVerticalOrigin VerticalOrigin,
    Length VerticalOffset,
    Length LowerSpacing);
