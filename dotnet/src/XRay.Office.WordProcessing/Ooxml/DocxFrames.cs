using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.WordProcessing.Layout;

namespace XRay.Office.WordProcessing.Ooxml;

/// <summary>
/// Reads a <c>w:drawing</c> — OOXML's floating frame — into the layout engine's own model.
/// </summary>
/// <remarks>
/// <para>
/// A drawing is either a <c>wp:inline</c>, which is a frame set in the text like a very large character,
/// or a <c>wp:anchor</c>, which floats. Only the anchored one has a position and a wrap, and it states
/// both in the same shape as ODF does but with a different vocabulary: <c>wp:positionH</c> pairs a
/// <c>relativeFrom</c> with either a <c>wp:posOffset</c> in EMUs or a <c>wp:align</c> naming an edge.
/// </para>
/// <para>
/// The wrap is an <em>element</em> rather than an attribute, which is the first thing to get right:
/// <c>wp:wrapNone</c>, <c>wp:wrapSquare</c>, <c>wp:wrapTight</c>, <c>wp:wrapThrough</c> and
/// <c>wp:wrapTopAndBottom</c> are five siblings and exactly one appears. And the names lie in the same
/// direction ODF's do, in the opposite place: <c>wp:wrapNone</c> means the text runs <em>through</em> the
/// frame, while <c>wp:wrapTopAndBottom</c> is what ODF calls <c>none</c>. Only <c>wp:wrapSquare</c>
/// carries the side, in <c>wrapText</c>.
/// </para>
/// <para>
/// <c>wp:wrapTight</c> and <c>wp:wrapThrough</c> ask for a contour wrap, which is a later item. They are
/// read as the square wrap their <c>wrapText</c> names, which is the same hole with straight sides — a
/// visible approximation rather than a wrong one, and much closer than ignoring the frame.
/// </para>
/// </remarks>
internal static class DocxFrames
{
    /// <summary>
    /// Reads the frame a <c>w:drawing</c> holds, or null when it holds nothing placeable.
    /// </summary>
    /// <param name="drawing">The <c>w:drawing</c> element.</param>
    /// <param name="content">How to read a text frame's own paragraphs, or null to skip them.</param>
    /// <param name="anchorOffset">Where in the paragraph's text the drawing sits.</param>
    /// <param name="pictures">
    /// How to resolve an <c>a:blip</c>'s <c>r:embed</c> into bytes, or null to record the frame's
    /// geometry without them — which is all the wrap ever needed.
    /// </param>
    public static PageFrame? Read(
        XElement drawing,
        Func<XElement, IReadOnlyList<PageBlock>>? content,
        int anchorOffset,
        DocxPictures? pictures = null)
        => ReadAll(drawing, content, anchorOffset, pictures) is [PageFrame first, ..] ? first : null;

    /// <summary>
    /// Reads every frame a <c>w:drawing</c> holds: one, or one per member of a shape group.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A <c>wpg:wgp</c> is many shapes in one drawing, and each of them can hold text.</strong>
    /// A letterhead written in Word is routinely a group of a dozen text boxes and a logo, and reading
    /// only the first text box in the drawing — the first <c>txbxContent</c> under the anchor — draws one
    /// of them and silently loses the rest. Measured on
    /// <c>Press release_EUREKA labels ITEA 3 Cluster.docx</c>: nineteen shapes, eighteen of them text
    /// boxes, of which one drew.
    /// </para>
    /// <para>
    /// LibreOffice imports a group as a <c>SdrObjGroup</c> and keeps the nesting
    /// (<c>oox/source/drawingml/shapegroupcontext.cxx</c>); this flattens it instead, because the layout
    /// engine places one rectangle per frame and a member's rectangle is fully determined once the
    /// group's is. The flattening is what <see cref="PageFrame.GroupSize"/> and
    /// <see cref="PageFrame.GroupOffset"/> carry.
    /// </para>
    /// <para>
    /// The first frame returned is always the group's own envelope, which keeps the anchor's wrap so the
    /// hole in the text is the group's rather than one per member.
    /// </para>
    /// </remarks>
    /// <param name="drawing">The <c>w:drawing</c> element.</param>
    /// <param name="content">How to read a text frame's own paragraphs, or null to skip them.</param>
    /// <param name="anchorOffset">Where in the paragraph's text the drawing sits.</param>
    /// <param name="pictures">How to resolve an <c>a:blip</c>'s <c>r:embed</c> into bytes, or null.</param>
    /// <param name="context">
    /// What the drawing's surroundings decide about it — the theme its colours resolve against, whether
    /// it sits in a header or a footer, and the file's compatibility mode. Default for a caller that has
    /// none of it, which costs a scheme-coloured fill and the header rule and nothing else.
    /// </param>
    public static IReadOnlyList<PageFrame> ReadAll(
        XElement drawing,
        Func<XElement, IReadOnlyList<PageBlock>>? content,
        int anchorOffset,
        DocxPictures? pictures = null,
        DocxFrameContext context = default)
    {
        ArgumentNullException.ThrowIfNull(drawing);

        XElement? anchor = Child(drawing, "anchor");
        XElement? inline = anchor is null ? Child(drawing, "inline") : null;
        XElement? placed = anchor ?? inline;
        if (placed is null) return [];

        XElement? extent = Child(placed, "extent");
        if (extent is null) return [];

        Length width = Emu(extent.Attribute("cx")?.Value);
        Length height = Emu(extent.Attribute("cy")?.Value);
        if (IsEmpty(width, height)) return [];

        // A SmartArt diagram is a graphic frame rather than a shape, so it names its parts through
        // relationships and arrives as a group of its own — one shape per node, already laid out.
        // Before the group check, because the drawing holds no `wpg:wgp` to find: the group is
        // synthesised from a part `DocxDiagram` resolves. See `DocxPictures.Diagram`.
        if (pictures?.Diagram(placed, new DocSize(width, height), context.Theme) is { } diagram)
        {
            return Members(diagram, placed, anchor, new DocSize(width, height), content,
                           anchorOffset, pictures, context);
        }

        if (Group(placed) is { } group)
        {
            return Members(group, placed, anchor, new DocSize(width, height), content, anchorOffset,
                           pictures, context);
        }

        PageFrame? single = One(placed, anchor, new DocSize(width, height), content, anchorOffset,
                                pictures, context);
        return single is null ? [] : [single];
    }

    /// <summary>
    /// Whether a <c>w:drawing</c> floats — a <c>wp:anchor</c> rather than a <c>wp:inline</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Asked by the reader that builds the paragraph, not by this one, and it is the difference between
    /// a run that takes room on its line and a run that takes none. A <c>wp:inline</c> is laid out as a
    /// character; a <c>wp:anchor</c> becomes a fly, and Writer's own import leaves the paragraph it was
    /// written in **empty** — which is why an anchor character standing for one must not make the
    /// paragraph count as having text.
    /// </para>
    /// <para>
    /// Measured on <c>088_Printable_Graph_Paper_Template_Quality_layout</c>, whose last paragraph is a
    /// 2 pt mark holding one anchored logo 8.45 pt above the bottom margin. Read as text-bearing it takes
    /// the 11 pt body size, overflows, and costs the document a whole second page; read as empty it takes
    /// the mark's 2 pt and fits, which is what 26.2.4.2 does. Eleven authored variants of that document
    /// are in <c>dotnet/probes/words-r50-chartset/</c>: deleting the drawing run fixes it and no property
    /// of the frame — offset, extent, <c>behindDoc</c>, wrap mode or anchor origin — changes anything.
    /// </para>
    /// </remarks>
    /// <param name="drawing">A <c>w:drawing</c> element.</param>
    /// <returns>True when it carries a <c>wp:anchor</c>.</returns>
    public static bool IsFloating(XElement drawing)
    {
        ArgumentNullException.ThrowIfNull(drawing);
        return Child(drawing, "anchor") is not null;
    }

    /// <summary>
    /// Whether an anchored drawing belongs on the layer Writer paints before the text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>m_bOpaque</c> in <c>sw/source/writerfilter/dmapper/GraphicImport.cxx</c>, reproduced in the
    /// order that file assigns it, because the order is the rule:
    /// </para>
    /// <list type="number">
    ///   <item><description>
    ///   it starts as <c>!IsInHeaderFooter()</c> (:342), so <strong>every drawing in a header or a
    ///   footer is behind the text</strong> whether or not it says so;
    ///   </description></item>
    ///   <item><description><c>behindDoc="1"</c> clears it (:698-702);</description></item>
    ///   <item><description>
    ///   and for <c>wrapSquare</c>, <c>wrapThrough</c>, <c>wrapTight</c> and <c>wrapTopAndBottom</c>, a
    ///   file whose <c>compatibilityMode</c> is 15 or more puts it back (:1589, :1697) — tdf#137850,
    ///   "Word >= 2013 seems to ignore bBehindDoc except for wrapNone, but older versions honour it".
    ///   </description></item>
    /// </list>
    /// <para>
    /// The resulting item is <c>SvxOpaqueItem</c>, and false is the hell layer.
    /// </para>
    /// <para>
    /// Only an anchored drawing is asked. A <c>wp:inline</c> takes room on its line rather than floating
    /// over anything, so its layer decides nothing that is visible; LibreOffice does still push it to the
    /// bottom of the z-order (:242-246), and following that here would move as-character pictures in
    /// every header in the corpus to buy nothing measurable.
    /// </para>
    /// </remarks>
    private static bool BehindText(XElement? anchor, DocxFrameContext context)
    {
        if (anchor is null) return false;

        bool opaque = !context.InHeaderFooter;

        if (anchor.Attribute("behindDoc")?.Value is not ("1" or "true" or "on")) return !opaque;

        opaque = false;
        if (context.CompatibilityMode >= 15 && WrapsAside(anchor)) opaque = !context.InHeaderFooter;

        return !opaque;
    }

    /// <summary>The anchor's declared place in the z order, or zero when it declares none.</summary>
    /// <remarks>
    /// <c>relativeHeight</c> is a <c>ST_RelativeHeight</c>, an unsigned 32-bit value, and real files
    /// use the top of that range — the corpus templates sit around 251 660 000, which overflows a
    /// signed <c>int</c> only above 2^31 but is parsed as <c>uint</c> here so that the whole declared
    /// range round-trips rather than most of it. An unparseable or absent value is zero, which sorts
    /// below every anchor that declares one and leaves document order untouched.
    /// </remarks>
    private static long ZOrder(XElement? anchor) =>
        anchor?.Attribute("relativeHeight")?.Value is { } text
        && uint.TryParse(text, System.Globalization.NumberStyles.Integer,
                         System.Globalization.CultureInfo.InvariantCulture, out uint z)
            ? z
            : 0L;

    /// <summary>Whether the anchor asks for one of the four wraps that leave a hole in the text.</summary>
    private static bool WrapsAside(XElement anchor)
    {
        foreach (XElement child in anchor.Elements())
        {
            if (child.Name.LocalName
                is "wrapSquare" or "wrapThrough" or "wrapTight" or "wrapTopAndBottom")
            {
                return true;
            }
        }

        return false;
    }

    /// <summary>The one frame an ordinary drawing holds.</summary>
    private static PageFrame? One(
        XElement placed,
        XElement? anchor,
        DocSize size,
        Func<XElement, IReadOnlyList<PageBlock>>? content,
        int anchorOffset,
        DocxPictures? pictures,
        DocxFrameContext context)
    {
        Length width = size.Width;
        Length height = size.Height;

        XElement? box = Descendant(placed, "txbxContent");
        FramePicture picture = box is null && pictures is not null ? pictures.Read(placed) : FramePicture.None;

        // A chart is a graphic frame rather than a picture, so it names its part through a different
        // relationship and is asked for separately. Only where the drawing holds no text box, which is
        // the one thing it can be that is neither.
        DocxChart chart = box is null && pictures is not null ? pictures.Chart(placed) : default;

        (Length x, FrameHorizontalOrigin horigin, FrameHorizontalAlignment halign) = Horizontal(anchor);
        (Length y, FrameVerticalOrigin vorigin, FrameVerticalAlignment valign) = Vertical(anchor);
        XElement? shapeProperties = ShapeProperties(placed);
        FrameAppearance paint = Appearance(shapeProperties, context);
        (bool isLine, bool isLineMirrored, bool isLineReversed) = LineGeometry(shapeProperties);
        (string? preset, IReadOnlyDictionary<string, double>? adjustments) =
            PresetGeometry(shapeProperties);
        (GraphicsPath? fillOutline, GraphicsPath? strokeOutline) =
            CustomOutline(shapeProperties, new DocSize(width, height));

        IReadOnlyList<PageBlock> blocks = box is not null && content is not null ? content(box) : [];

        // WordArt replaces the shape rather than decorating it, so it is settled before the frame is
        // built: the curves become the frame's outline, the character fill and outline become the
        // shape's paint, and the text leaves the flow. See `DocxFontwork`.
        FontworkDrawing warp = box is null
            ? default
            : DocxFontwork.Read(
                placed,
                shapeProperties,
                blocks,
                new DocSize(width, height),
                context.Theme);

        if (warp.IsWarped)
        {
            fillOutline = warp.Outline;
            strokeOutline = warp.Outline;
            paint = paint with
            {
                Fill = warp.Fill,
                Gradient = warp.Gradient,
                Line = warp.Line,
                Width = warp.LineWidth,
            };

            preset = null;
            adjustments = null;
        }

        if (warp.SuppressesText) blocks = [];

        return new PageFrame
        {
            Size = new DocSize(width, height),
            RotationDegrees = Rotation(shapeProperties),
            TextRotationDegrees = TextRotation(placed, Rotation(shapeProperties)),
            Fill = paint.Fill,
            Gradient = paint.Gradient,
            BorderColour = paint.Line,
            BorderWidth = paint.Width,
            BorderDash = paint.Dash,
            BorderCap = paint.Cap,
            HeadEnd = paint.HeadEnd,
            TailEnd = paint.TailEnd,
            BehindText = BehindText(anchor, context),
            ZOrder = ZOrder(anchor),
            Preset = preset,
            Adjustments = adjustments,
            FillOutline = fillOutline,
            StrokeOutline = strokeOutline,
            IsLine = isLine,
            IsLineMirrored = isLineMirrored,
            IsLineReversed = isLineReversed,
            Anchor = anchor is null ? FrameAnchor.AsCharacter : FrameAnchor.Paragraph,
            AnchorOffset = anchorOffset,
            Wrap = anchor is null ? TextWrap.Through : WrapOf(anchor),
            ObjectKind = ObjectKindOf(placed, box),
            FollowsTextFlow = FollowsTextFlow(anchor, chart.Plot is not null),
            HorizontalOrigin = horigin,
            HorizontalAlignment = halign,
            HorizontalOffset = x,
            VerticalOrigin = vorigin,
            VerticalAlignment = valign,
            VerticalOffset = y,
            Spacing = Spacing(placed),
            EffectExtent = EffectExtent(placed, anchor, shapeProperties),
            IsImage = box is null && chart.Plot is null,
            Image = picture.Raster,
            Crop = picture.Crop,
            Vector = picture.Vector,
            Chart = chart.Plot,
            ChartFontFamily = chart.Family,
            Name = Child(placed, "docPr")?.Attribute("name")?.Value,
            Blocks = blocks,
            Padding = box is null ? default : Insets(placed),
            TextAlignment = box is null ? default : TextAlignment(placed),
            HasFixedHeight = box is not null && !GrowsWithText(placed),
            GrowsToContent = box is not null && FitsShapeToText(placed),
            HeightFloor = box is not null && FitsShapeToText(placed) ? Length.Zero : null,
        };
    }

    /// <summary>The <c>wpg:wgp</c> a drawing's graphic data holds, or null when it holds something else.</summary>
    /// <remarks>
    /// <c>wpg:wpc</c> — a drawing <em>canvas</em> — is the same shape with a different name and is taken
    /// too: Word writes one whenever a user draws several shapes on a canvas rather than grouping them,
    /// and the members are laid out by the same transform.
    /// </remarks>
    private static XElement? Group(XElement placed)
    {
        XElement? data = Child(Child(placed, "graphic") ?? placed, "graphicData");
        if (data is null) return null;

        foreach (XElement child in data.Elements())
        {
            if (child.Name.LocalName is "wgp" or "wpc") return child;
        }

        return null;
    }

    /// <summary>
    /// A group flattened into its envelope and one frame per leaf shape.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The transform is the one every DrawingML group carries: a child stated at <c>a:off</c> in the
    /// group's own child coordinate space — the space <c>a:chOff</c> and <c>a:chExt</c> describe — maps
    /// to <c>(off − chOff) × ext ÷ chExt</c> inside the group's rectangle. Nested groups compose, which
    /// is why this recurses with the transform rather than with the element.
    /// </para>
    /// <para>
    /// A group with no <c>a:chExt</c> — which a canvas usually has — is read as one-to-one, since the
    /// child coordinates are then the group's own.
    /// </para>
    /// </remarks>
    private static List<PageFrame> Members(
        XElement group,
        XElement placed,
        XElement? anchor,
        DocSize size,
        Func<XElement, IReadOnlyList<PageBlock>>? content,
        int anchorOffset,
        DocxPictures? pictures,
        DocxFrameContext context)
    {
        (Length x, FrameHorizontalOrigin horigin, FrameHorizontalAlignment halign) = Horizontal(anchor);
        (Length y, FrameVerticalOrigin vorigin, FrameVerticalAlignment valign) = Vertical(anchor);

        // The envelope of a group paints nothing of its own — an `SdrObjGroup` has no fill and no line —
        // so it takes no appearance here, only the paint order its members inherit.
        PageFrame envelope = new()
        {
            Size = size,
            BehindText = BehindText(anchor, context),
            ZOrder = ZOrder(anchor),
            Anchor = anchor is null ? FrameAnchor.AsCharacter : FrameAnchor.Paragraph,
            AnchorOffset = anchorOffset,
            Wrap = anchor is null ? TextWrap.Through : WrapOf(anchor),

            // A group is an `SdrObjGroup`, which is a drawing object with no text box of its own, so
            // `bConsidered` is true for it and the page capture never reaches it — see
            // `FrameObjectKind`. Its members carry `TextWrap.Through` and are exempt by that instead.
            ObjectKind = FrameObjectKind.Shape,
            HorizontalOrigin = horigin,
            HorizontalAlignment = halign,
            HorizontalOffset = x,
            VerticalOrigin = vorigin,
            VerticalAlignment = valign,
            VerticalOffset = y,
            Spacing = Spacing(placed),
            EffectExtent = EffectExtent(placed, anchor, ShapeProperties(placed)),
            IsImage = false,
            Name = Child(placed, "docPr")?.Attribute("name")?.Value,
        };

        List<PageFrame> frames = [envelope];

        // The outermost group's own fill counts too: `wpg:wgp` carries a `wpg:grpSpPr` exactly
        // as a nested `wpg:grpSp` does, and a member of it asking for `a:grpFill` means that one.
        Walk(group, TransformOf(group, size, orientation: false), 0,
             GroupFill(group, context, default));

        // A canvas is a frame of its own and is left alone; only a group takes its size from what
        // is in it — and only then is the group itself turned.
        if (group.Name.LocalName is "wgp") Orient(frames, group, Fit(frames, size));

        return frames;

        // `inherited` is the fill the enclosing group offers a child that asks for it with
        // `a:grpFill` -- see `Appearance`. Resolved on the way down because a group has no
        // geometry of its own: its `wpg:grpSpPr` fill exists only to be inherited.
        void Walk(
            XElement container, GroupTransform transform, int depth, FrameAppearance inherited)
        {
            if (depth > MaxGroupNesting) return;

            foreach (XElement child in container.Elements())
            {
                switch (child.Name.LocalName)
                {
                    case "grpSp" or "wgp" or "wpc":
                        Walk(
                            child,
                            transform.Around(child, TransformOf(child, size)),
                            depth + 1,
                            GroupFill(child, context, inherited));
                        break;

                    case "wsp" or "pic" or "sp":
                    {
                        if (Leaf(child, transform, envelope, size, content, anchorOffset, pictures,
                                 context, inherited)
                            is { } leaf)
                        {
                            frames.Add(leaf);
                        }

                        break;
                    }

                    default:
                        continue;
                }
            }
        }
    }

    /// <summary>
    /// A group's members scaled so that between them they cover exactly the anchor's extent.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A group has no size of its own — its rectangle is whatever its members happen to cover — so
    /// Writer sizes the imported group by <em>resizing it to the anchor's <c>wp:extent</c></em>,
    /// which is the one size the document actually declares for the drawing. Where the members fill
    /// their child space that resize is the identity and nothing here moves; where they do not, every
    /// member is scaled, and a file whose <c>a:chExt</c> is twice what its members use draws them at
    /// twice the size the arithmetic alone gives.
    /// </para>
    /// <para>
    /// Established by probe rather than by reading, because <c>oox</c>'s side of it
    /// (<c>Shape::createAndInsert</c>, the <c>aParentScale / maChSize</c> block) composes the child
    /// transform exactly as this reader does and so cannot be where the difference lives. Nine
    /// one-shape files, each varying one thing, in <c>dotnet/probes/words-group-extent-fit/</c>:
    /// </para>
    /// <list type="bullet">
    ///   <item><description>
    ///   a 100 × 50 pt member alone in a child space four times its size is drawn by 26.2.4.2 at
    ///   <b>400 × 200 pt</b>, the whole extent, and by this reader at 50 × 25;
    ///   </description></item>
    ///   <item><description>
    ///   a member that fills its child space exactly is drawn identically by both, which is the
    ///   control;
    ///   </description></item>
    ///   <item><description>
    ///   the fit is to <c>wp:extent</c> and not to the group's own <c>a:ext</c> — halving the
    ///   latter changes nothing in the reference and halves the shape here;
    ///   </description></item>
    ///   <item><description>
    ///   it is two independent factors, not one: members covering half the width and a quarter of
    ///   the height come back stretched 1.6 across and 4.0 down;
    ///   </description></item>
    ///   <item><description>
    ///   and it shrinks as readily as it grows, a member overflowing its child space coming back
    ///   inside the extent.
    ///   </description></item>
    /// </list>
    /// <para>
    /// The reference point is the members' own top-left corner, not the anchor's: a lone member
    /// stated 200 pt into its child space stays 200 pt in and grows right and down from there, so
    /// the drawn content can sit outside the rectangle the anchor reserved. That is what
    /// <c>SdrObjGroup</c> resizing about its own snap rectangle does, and the probe measures it.
    /// </para>
    /// <para>
    /// Censused over the corpus, <b>13 group anchors across 10 <c>docx</c></b> are out by more than
    /// 2 per cent, and they are the documents this reader has been wrong about all along: five of
    /// the eight <c>Free_Genogram</c> templates, whose worst group is out by 36 per cent, the
    /// disease concept map at 67 per cent, and the management-system manual at 157 per cent.
    /// </para>
    /// </remarks>
    /// <returns>The rectangle the members cover once they have been fitted.</returns>
    private static DocRect Fit(List<PageFrame> frames, DocSize size)
    {
        if (frames.Count < 2) return new DocRect(Length.Zero, Length.Zero, size.Width, size.Height);

        double left = double.MaxValue;
        double top = double.MaxValue;
        double right = double.MinValue;
        double bottom = double.MinValue;

        for (int index = 1; index < frames.Count; index++)
        {
            (double x, double y, double width, double height) = Covered(frames[index]);
            left = Math.Min(left, x);
            top = Math.Min(top, y);
            right = Math.Max(right, x + width);
            bottom = Math.Max(bottom, y + height);
        }

        double spanX = right - left;
        double spanY = bottom - top;
        double scaleX = spanX > 0 ? size.Width.Emu / spanX : 1;
        double scaleY = spanY > 0 ? size.Height.Emu / spanY : 1;

        // A group whose members already fill it — which is nearly every group in the corpus — is
        // left exactly alone, so this cannot move a well-formed drawing by a rounding step.
        if (Math.Abs(scaleX - 1) < FitTolerance && Math.Abs(scaleY - 1) < FitTolerance)
        {
            return new DocRect(Snap(left), Snap(top), Snap(spanX), Snap(spanY));
        }

        for (int index = 1; index < frames.Count; index++)
        {
            PageFrame frame = frames[index];

            double statedWidth = frame.Size.Width.Emu;
            double statedHeight = frame.Size.Height.Emu;

            // A quarter-turned member is held as its unturned rectangle and turned about its centre
            // when it is drawn, so the two factors reach it the same way round as the group's own
            // scales do — see `GroupTransform.MapQuarterTurned`.
            bool turned = IsQuarterTurn(frame.RotationDegrees);
            double width = statedWidth * (turned ? scaleY : scaleX);
            double height = statedHeight * (turned ? scaleX : scaleY);

            double centreX = left + ((frame.GroupOffset.X.Emu + (statedWidth / 2) - left) * scaleX);
            double centreY = top + ((frame.GroupOffset.Y.Emu + (statedHeight / 2) - top) * scaleY);

            frames[index] = frame with
            {
                Size = new DocSize(Snap(width), Snap(height)),
                GroupOffset = new DocPoint(Snap(centreX - (width / 2)), Snap(centreY - (height / 2))),
            };
        }

        return new DocRect(Snap(left), Snap(top), size.Width, size.Height);
    }

    /// <summary>
    /// The outermost group's own <c>rot</c> and flips, applied to the members once they are placed.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A nested group's orientation is part of the child transform and composes with everything
    /// below it; the outermost group's is not, because LibreOffice turns that one as an <em>object</em>
    /// — after it has been sized to the anchor. The two orders give different answers whenever the
    /// turn is a quarter, since the fit is then stretching a rectangle that the turn has stood on
    /// its side.
    /// </para>
    /// <para>
    /// Measured against 26.2.4.2 on a group turned 90° whose one member fills it: nested, both this
    /// reader and the reference stretch what the turn covers back onto the anchor, and the mark lands
    /// at 350 pt; stated on the outermost <c>wpg:wgp</c> the reference does not, and the mark stays
    /// at 275 pt, exactly where the turn alone puts it. Ten of the corpus's 74 oriented groups are
    /// outermost ones, across five documents.
    /// </para>
    /// </remarks>
    private static void Orient(List<PageFrame> frames, XElement group, DocRect content)
    {
        if (frames.Count < 2) return;

        XElement? properties = group.Elements()
            .FirstOrDefault(child => child.Name.LocalName is "grpSpPr" or "spPr");
        XElement? transformation = properties is null ? null : Child(properties, "xfrm");
        if (transformation is null) return;

        double width = content.Width.Emu;
        double height = content.Height.Emu;

        GroupTransform orientation =
            new GroupTransform(1, 0, 0, 1, content.X.Emu, content.Y.Emu)
                .Compose(Turned(transformation, width, height))
                .Compose(Flipped(transformation, width, height))
                .Compose(new GroupTransform(1, 0, 0, 1, -content.X.Emu, -content.Y.Emu));

        if (orientation == GroupTransform.Identity) return;

        for (int index = 1; index < frames.Count; index++)
        {
            PageFrame frame = frames[index];

            double turned = orientation.Mirrors
                ? Modulo(orientation.TurnDegrees - frame.RotationDegrees, 180)
                : orientation.TurnDegrees + frame.RotationDegrees;

            DocRect placed = orientation.Turn(
                new DocRect(frame.GroupOffset.X, frame.GroupOffset.Y,
                            frame.Size.Width, frame.Size.Height),
                IsQuarterTurn(frame.RotationDegrees),
                IsQuarterTurn(turned));

            frames[index] = frame with
            {
                Size = new DocSize(placed.Width, placed.Height),
                GroupOffset = new DocPoint(placed.X, placed.Y),
                RotationDegrees = turned,
            };
        }
    }

    /// <summary>What one member of a group covers, once the file's own rotation is allowed for.</summary>
    /// <remarks>
    /// A member is held as the rectangle the file states and turned about its centre when it is
    /// drawn, so a turned one covers its rotated bounding box rather than its stated rectangle — and
    /// that is what the group's own rectangle is the union of. Measured: a member filling its child
    /// space and turned a quarter is drawn by 26.2.4.2 at 400 × 200 pt where the stated rectangle
    /// would give 200 × 400, which is the difference between taking the rotation into account here
    /// and not.
    /// </remarks>
    private static (double X, double Y, double Width, double Height) Covered(PageFrame frame)
    {
        double width = frame.Size.Width.Emu;
        double height = frame.Size.Height.Emu;
        double x = frame.GroupOffset.X.Emu;
        double y = frame.GroupOffset.Y.Emu;

        if (frame.RotationDegrees == 0) return (x, y, width, height);

        double radians = frame.RotationDegrees * Math.PI / 180;
        double across = Math.Abs(Math.Cos(radians));
        double down = Math.Abs(Math.Sin(radians));

        double turnedWidth = (width * across) + (height * down);
        double turnedHeight = (width * down) + (height * across);

        return (
            x + ((width - turnedWidth) / 2),
            y + ((height - turnedHeight) / 2),
            turnedWidth,
            turnedHeight);
    }

    /// <summary>How far from one the fit has to be before it is applied at all.</summary>
    /// <remarks>
    /// Half a tenth of a per cent, which on a 400 pt drawing is a fifth of a point — below the twip
    /// the frames are snapped to, so a group inside it cannot move whatever this does.
    /// </remarks>
    private const double FitTolerance = 0.0005;

    /// <summary>One length on the twip grid the rest of the frames sit on.</summary>
    private static Length Snap(double emu)
        => Length.FromTwips(Length.FromEmu((long)Math.Round(emu)).Twips);

    /// <summary>
    /// The fill a group offers the children that ask for it with <c>a:grpFill</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Never painted. A group has no geometry of its own, so the fill on its <c>wpg:grpSpPr</c>
    /// exists only to be inherited — which is why it is resolved on the way down the tree rather
    /// than turned into a frame. A group may itself say <c>a:grpFill</c>, so what the group above
    /// offered is passed in and the chain resolves as far up as it is written.
    /// </para>
    /// <para>
    /// Censused over the corpus, <b>661 shapes across 14 <c>docx</c></b> state <c>a:grpFill</c>, and
    /// they are concentrated: eight genogram templates carry 573 of them between them. Every one
    /// drew unfilled, which on those documents is most of the ink.
    /// </para>
    /// </remarks>
    private static FrameAppearance GroupFill(
        XElement group, DocxFrameContext context, FrameAppearance inherited)
    {
        XElement? properties = group.Elements()
            .FirstOrDefault(child => child.Name.LocalName is "grpSpPr" or "spPr");

        return properties is null ? inherited : Appearance(properties, context, inherited);
    }

    /// <summary>How deep a group may nest before the walk gives up.</summary>
    /// <remarks>
    /// <para>
    /// This was 8, on the stated grounds that "real files nest a group inside a group and stop".
    /// They do not. Censused over the corpus, the deepest grouped shape in
    /// <c>055_Organogram_Template_Horizontal_Structure</c> sits <b>twelve</b> groups down, and
    /// <b>291 shapes across 10 documents</b> are deeper than eight — which is to say they were
    /// dropped, without a diagnostic, by a bound that was a guess.
    /// </para>
    /// <para>
    /// It shows as content simply absent. <c>002_Free_Genogram_Diagram_Template_Customizable_Format</c>
    /// loses 46 shapes that way, among them exactly the 9 <c>ellipse</c> and 9 <c>rect</c> that are
    /// the people in the top two generations of its family tree: the reference fills its
    /// <c>#D9D9D9</c> 15 times and its <c>#F8CBAD</c> 14, and we filled them 6 and 5.
    /// </para>
    /// <para>
    /// 64 rather than a larger number or none at all. An <c>XElement</c> tree from a parse cannot
    /// cycle, so the walk is finite whatever this says and the bound guards the stack rather than
    /// the loop; a document nested past 64 would have exhausted the stack in <c>XDocument.Parse</c>
    /// before reaching here. Five times the deepest file anyone has produced is enough margin to
    /// stop this being the thing that quietly loses a diagram again.
    /// </para>
    /// </remarks>
    private const int MaxGroupNesting = 64;

    /// <summary>One leaf shape of a group, placed inside the group's rectangle.</summary>
    /// <remarks>
    /// A shape with no <c>a:xfrm</c> of its own has no rectangle to be placed at and is skipped rather
    /// than drawn at the group's origin, where it would sit on top of the member that is really there.
    /// </remarks>
    private static PageFrame? Leaf(
        XElement shape,
        GroupTransform transform,
        PageFrame envelope,
        DocSize size,
        Func<XElement, IReadOnlyList<PageBlock>>? content,
        int anchorOffset,
        DocxPictures? pictures,
        DocxFrameContext context,
        FrameAppearance inherited)
    {
        XElement? properties = shape.Elements()
            .FirstOrDefault(child => child.Name.LocalName is "spPr");
        XElement? transformation = properties is null ? null : Child(properties, "xfrm");
        if (transformation is null) return null;

        XElement? offset = Child(transformation, "off");
        XElement? extent = Child(transformation, "ext");
        if (offset is null || extent is null) return null;

        // The member's own turn, and the turn it ends up with once the groups above it are added.
        // A group that mirrors what is in it reverses the member's own turn, since a mirror and a
        // rotation do not commute: `R(phi) . Fh . R(r)` is `R(phi - r) . Fh`.
        double stated = Rotation(properties);
        double rotation = transform.Mirrors
            ? Modulo(transform.TurnDegrees - stated, 180)
            : transform.TurnDegrees + stated;

        DocRect within = transform.Map(
            Raw(offset, "x"), Raw(offset, "y"), Raw(extent, "cx"), Raw(extent, "cy"),
            IsQuarterTurn(stated), IsQuarterTurn(rotation));

        if (IsEmpty(within.Width, within.Height)) return null;

        XElement? box = Descendant(shape, "txbxContent");
        FramePicture picture = box is null && pictures is not null
            ? pictures.Read(shape)
            : FramePicture.None;

        FrameAppearance paint = Appearance(properties, context, inherited);
        (bool isLine, bool isLineMirrored, bool isLineReversed) = LineGeometry(properties);
        (string? preset, IReadOnlyDictionary<string, double>? adjustments) =
            PresetGeometry(properties);
        (GraphicsPath? fillOutline, GraphicsPath? strokeOutline) =
            CustomOutline(properties, new DocSize(within.Width, within.Height));

        // No WordArt here, and the omission is measured rather than an oversight. The reference
        // converts a warped body at the end of its `wps:bodyPr`, and by then a *group member* is
        // not yet an `SdrObjCustomShape` — `WpsContext::onEndElement`'s first guard
        // (`oox/source/shape/WpsContext.cxx:944-947`) fails and the member keeps its text as text.
        // Both organogram templates in `words/chartset-005` carry a `textPlain` inside a
        // `wpg:wgp`, and the reference extracts their "Organogram Template" as words; warping it
        // here cost each of them two words against a reference that had none to lose.
        return envelope with
        {
            Size = new DocSize(within.Width, within.Height),

            // A member states its own geometry exactly as a shape standing alone does, and this
            // read it for the one and not the other -- so every preset shape inside a group was
            // painted as its bounding rectangle. Censused over the 271 corpus docx: 247 such shapes
            // across 25 documents, of which 142 are `ellipse`. That is the genogram templates,
            // whose people are circles and squares and which came out as squares and squares.
            Preset = preset,
            Adjustments = adjustments,
            FillOutline = fillOutline,
            StrokeOutline = strokeOutline,

            // The member's own, not the envelope's: a group states one rotation per shape and none
            // of its own beyond the child transform, which is a scale and a translation.
            RotationDegrees = rotation,
            TextRotationDegrees = TextRotation(shape, stated),
            Fill = paint.Fill,
            Gradient = paint.Gradient,
            BorderColour = paint.Line,
            BorderWidth = paint.Width,
            BorderDash = paint.Dash,
            BorderCap = paint.Cap,
            HeadEnd = paint.HeadEnd,
            TailEnd = paint.TailEnd,
            GroupSize = size,
            GroupOffset = new DocPoint(within.X, within.Y),
            IsLine = isLine,
            IsLineMirrored = isLineMirrored,
            IsLineReversed = isLineReversed,

            // The envelope keeps the anchor's wrap; a member must not punch a hole of its own, or a
            // nineteen-shape letterhead would narrow the text nineteen times over.
            Wrap = TextWrap.Through,
            Spacing = default,
            IsImage = box is null,
            Image = picture.Raster,
            Crop = picture.Crop,
            Vector = picture.Vector,
            Chart = null,
            ChartFontFamily = null,
            AnchorOffset = anchorOffset,
            Name = Descendant(shape, "cNvPr")?.Attribute("name")?.Value,
            Blocks = box is not null && content is not null ? content(box) : [],
            Padding = box is null ? default : Insets(shape),
            TextAlignment = box is null ? default : TextAlignment(shape),
            HasFixedHeight = box is not null && !GrowsWithText(shape),
            GrowsToContent = box is not null && FitsShapeToText(shape),
            HeightFloor = box is not null && FitsShapeToText(shape) ? Length.Zero : null,
        };
    }

    /// <summary>
    /// A group's child-coordinate to anchor-rectangle mapping: a scale, a shift, and quarter turns.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Written as an affine map rather than as a scale and a shift because a group may state a
    /// rotation of its own, and a rotated group is not a scale. Restricted to quarter turns, which
    /// keeps every entry either zero or a scale and so keeps an axis-aligned rectangle axis-aligned:
    /// censused over the corpus, all <b>31 rotated groups across 9 <c>docx</c></b> state a multiple
    /// of ninety degrees — 19 at 90, 6 at 270 and 6 at 180 — and a group at any other angle maps a
    /// rectangle onto a parallelogram, which a frame cannot hold.
    /// </para>
    /// <para>
    /// <c>x' = M11·x + M12·y + Tx</c> and <c>y' = M21·x + M22·y + Ty</c>. With no rotation that is
    /// the scale and shift it replaces, entry for entry.
    /// </para>
    /// </remarks>
    /// <param name="M11">The child <c>x</c> axis's contribution to <c>x</c>.</param>
    /// <param name="M12">The child <c>y</c> axis's contribution to <c>x</c>, non-zero on a turn.</param>
    /// <param name="M21">The child <c>x</c> axis's contribution to <c>y</c>, non-zero on a turn.</param>
    /// <param name="M22">The child <c>y</c> axis's contribution to <c>y</c>.</param>
    /// <param name="Tx">Where the mapped rectangle starts inside the anchor, in EMUs.</param>
    /// <param name="Ty">The same, vertically.</param>
    private readonly record struct GroupTransform(
        double M11, double M12, double M21, double M22, double Tx, double Ty)
    {
        /// <summary>The identity, for a group that states no child space of its own.</summary>
        public static GroupTransform Identity => new(1, 0, 0, 1, 0, 0);

        /// <summary>Whether the groups above lay a child's two axes the other way round.</summary>
        public bool Swaps => M12 != 0 || M21 != 0;

        /// <summary>Whether they mirror it, which is a negative determinant and nothing else.</summary>
        public bool Mirrors => ((M11 * M22) - (M12 * M21)) < 0;

        /// <summary>
        /// How far the groups above turn what is inside them, in degrees.
        /// </summary>
        /// <remarks>
        /// <para>
        /// Read back out of the matrix rather than carried beside it, because composing a rotation
        /// with a flip gives a map that is <em>both</em> and the two cannot be added separately:
        /// <c>rot="10800000" flipH="1"</c> — which <c>051_Organogram_Template_Basic_Theme</c> states
        /// — is a plain vertical mirror, no rotation at all, and adding 180° to its members would
        /// stand every box in the diagram on its head.
        /// </para>
        /// <para>
        /// A mirroring map is factored as <c>R(φ) ∘ Fh</c>, which leaves φ free by 180° — the same
        /// map is also <c>R(φ+180) ∘ Fv</c> — so the smaller of the two is taken. Nothing here can
        /// mirror a frame, and LibreOffice does not mirror a shape's text either, so dropping the
        /// mirror and keeping the turn is the closer of the two available answers.
        /// </para>
        /// </remarks>
        public double TurnDegrees
            => Swaps
                ? (M12 < 0 ? 90 : 270)
                : ((Mirrors ? -M11 : M11) < 0 ? 180 : 0);

        /// <summary>
        /// A nested group's transform, composed inside this one — <em>including where the nested group
        /// itself sits</em>.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This used to be <c>Composed(inner)</c>, which added <c>inner.ShiftX</c> — and
        /// <see cref="TransformOf"/> never sets a shift, so it added nought. A nested
        /// <c>a:grpSpPr/a:xfrm/a:off</c>, which is where the nested group sits inside its parent, was
        /// therefore **dropped**, and every nested group's members were laid out as though the group
        /// were at the parent's own origin. The scale composed correctly throughout, so the members
        /// came out the right size in the wrong place, which is the hardest kind of this defect to see.
        /// </para>
        /// <para>
        /// Measured on <c>056_Organogram_Template_Square_Theme</c>, whose <c>wpg:wgp</c> holds five
        /// text-bearing <c>a:grpSp</c> at <c>a:off/@x</c> 141890, 1623848, 3200400, 4761186 and
        /// 6258911 EMU. Every one has <c>chOff="0,0"</c> and a <c>chExt</c> equal to its <c>ext</c>, so
        /// all five resolved to the same rectangle: their twenty <c>Text here</c> boxes landed on top of
        /// one another at the drawing's own left edge, and the PDF's text layer holds **four** of the
        /// twenty. 26.2.4.2 draws them as a 5 × 5 lattice. A blind reviewer given only the image, and
        /// told nothing, reported *"the surviving leaves are piled into the left edge of the page, a
        /// single vertical stack … the remaining 20 leaves are absent as boxes"*.
        /// </para>
        /// <para>
        /// The composition is the ordinary one. The nested group's own <c>off</c> is a point in
        /// <em>this</em> group's child space, so the nested map is shifted by it and then run through
        /// this one — which is a matrix product, and is where a rotated group above pays for itself.
        /// </para>
        /// </remarks>
        /// <param name="group">The nested group, for its own <c>a:off</c>.</param>
        /// <param name="inner">The nested group's own child-space transform.</param>
        public GroupTransform Around(XElement group, GroupTransform inner)
        {
            XElement? properties = group.Elements()
                .FirstOrDefault(child => child.Name.LocalName is "grpSpPr" or "spPr");
            XElement? transformation = properties is null ? null : Child(properties, "xfrm");
            XElement? offset = transformation is null ? null : Child(transformation, "off");

            double x = offset is null ? 0 : Raw(offset, "x");
            double y = offset is null ? 0 : Raw(offset, "y");

            return Compose(inner with { Tx = inner.Tx + x, Ty = inner.Ty + y });
        }

        /// <summary>This transform applied to the output of <paramref name="inner"/>.</summary>
        public GroupTransform Compose(GroupTransform inner)
            => new(
                (M11 * inner.M11) + (M12 * inner.M21),
                (M11 * inner.M12) + (M12 * inner.M22),
                (M21 * inner.M11) + (M22 * inner.M21),
                (M21 * inner.M12) + (M22 * inner.M22),
                (M11 * inner.Tx) + (M12 * inner.Ty) + Tx,
                (M21 * inner.Tx) + (M22 * inner.Ty) + Ty);

        /// <summary>A child rectangle mapped into the anchor's own.</summary>
        /// <remarks>
        /// <para>
        /// The centre is what is mapped and the corners are derived from it, because a turn is what
        /// leaves a centre alone: taking the top-left instead and giving it the turned extent moves
        /// the shape by half the difference between the two scales, which on
        /// <c>071_Storyboard_Template_Cartoon_Theme</c> is 4.3 pt across and 7.3 pt down — enough to
        /// put a picture's frame off its picture.
        /// </para>
        /// <para>
        /// The two turns are asked separately because they answer different questions. A member's own
        /// <c>rot</c> turns it in the group's child space, so a group whose two scales differ meets it
        /// <em>after</em> that turn — scaling first and turning afterwards stretches the wrong axis,
        /// and the two answers differ by the ratio of the scales. Measured on <c>071</c>, whose picture
        /// frames are quarter-turned rectangles in groups scaled 1.000 across and 0.945 down: the frame
        /// is 156.9 × 265.0 pt as written; scaled then turned it is 250.3 × 156.9, and turned then
        /// scaled it is 265.0 × 148.2 — against pictures 261 × 145, which the reference borders evenly.
        /// Whether it ends up on its side is a different question, because the groups above may turn it
        /// again, and it decides only how the frame is <em>held</em>: a frame is stored unturned and
        /// turned about its centre when it is drawn.
        /// </para>
        /// </remarks>
        /// <param name="x">The member's <c>a:off/@x</c>, in the group's child units.</param>
        /// <param name="y">The member's <c>a:off/@y</c>.</param>
        /// <param name="cx">The member's <c>a:ext/@cx</c>.</param>
        /// <param name="cy">The member's <c>a:ext/@cy</c>.</param>
        /// <param name="turnedWhereItIsStated">Whether the member's own <c>rot</c> is a quarter turn.</param>
        /// <param name="turnedInTheEnd">Whether it is a quarter turn once the groups above are added.</param>
        public DocRect Map(
            double x, double y, double cx, double cy,
            bool turnedWhereItIsStated, bool turnedInTheEnd)
            => Placed(
                x + (cx / 2), y + (cy / 2),
                turnedWhereItIsStated ? cy : cx,
                turnedWhereItIsStated ? cx : cy,
                turnedInTheEnd);

        /// <summary>An already-placed rectangle mapped again, for the outermost group's own turn.</summary>
        /// <remarks>
        /// The same arithmetic as <see cref="Map"/>, differing only in that its rectangle is in the
        /// anchor's coordinates already rather than in a group's child units.
        /// </remarks>
        public DocRect Turn(DocRect rect, bool turnedNow, bool turnedInTheEnd)
            => Placed(
                rect.X.Emu + (rect.Width.Emu / 2.0), rect.Y.Emu + (rect.Height.Emu / 2.0),
                turnedNow ? rect.Height.Emu : rect.Width.Emu,
                turnedNow ? rect.Width.Emu : rect.Height.Emu,
                turnedInTheEnd);

        private DocRect Placed(
            double x, double y, double coveredX, double coveredY, bool turnedInTheEnd)
        {
            double centreX = (M11 * x) + (M12 * y) + Tx;
            double centreY = (M21 * x) + (M22 * y) + Ty;

            double width = (Math.Abs(M11) * coveredX) + (Math.Abs(M12) * coveredY);
            double height = (Math.Abs(M21) * coveredX) + (Math.Abs(M22) * coveredY);

            if (turnedInTheEnd) (width, height) = (height, width);

            return new DocRect(
                Round(centreX - (width / 2)),
                Round(centreY - (height / 2)),
                Round(width),
                Round(height));
        }

        private static Length Round(double emu)
            => Length.FromTwips(Length.FromEmu((long)Math.Round(emu)).Twips);
    }

    /// <summary>
    /// The transform a group's own <c>a:xfrm</c> describes — its scale, its flips and its rotation.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A group's <c>a:xfrm</c> carries <c>rot</c>, <c>flipH</c> and <c>flipV</c> exactly as a shape's
    /// does, and all three were read for the shape and not for the group — so every member of a
    /// turned or mirrored group was laid out upright and unmirrored. Censused over the corpus,
    /// <b>74 groups across 15 <c>docx</c></b> state one: 43 a flip alone, 25 a rotation alone and 6
    /// both.
    /// </para>
    /// <para>
    /// It is not a cosmetic difference. <c>055_Organogram_Template_Horizontal_Structure</c> puts each
    /// of its four rows of connectors in a group turned 90°; unturned, they run <em>down</em> the page
    /// through the boxes instead of across between them, as one black rule and sixteen arrows pointing
    /// the wrong way.
    /// </para>
    /// <para>
    /// The order is the file's: scale the child space onto <c>a:ext</c>, mirror, then turn, the last
    /// two about the group's own rectangle's centre. That centre is why a rotation cannot be folded
    /// into the scale — a group 960k × 8030k EMU turned a quarter covers 8030k × 960k about it, so its
    /// members reach well outside the rectangle the file states. Which is also what made this worth
    /// finding: <see cref="Fit"/> takes the union of what the members cover, and members put in the
    /// wrong place take the whole drawing with them.
    /// </para>
    /// <para>
    /// Rotations that are not a multiple of ninety degrees are left upright rather than approximated,
    /// since they map the group's rectangles onto parallelograms; the corpus states none.
    /// </para>
    /// </remarks>
    private static GroupTransform TransformOf(XElement group, DocSize size, bool orientation = true)
    {
        XElement? properties = group.Elements()
            .FirstOrDefault(child => child.Name.LocalName is "grpSpPr" or "spPr");
        XElement? transformation = properties is null ? null : Child(properties, "xfrm");
        if (transformation is null) return GroupTransform.Identity;

        XElement? childOffset = Child(transformation, "chOff");
        XElement? childExtent = Child(transformation, "chExt");
        XElement? extent = Child(transformation, "ext");

        // The group's own extent when it states one, and the anchor's otherwise: `wp:extent` is what the
        // document says the whole drawing is, and the two agree in every file that states both.
        double width = extent is not null && Raw(extent, "cx") > 0 ? Raw(extent, "cx") : size.Width.Emu;
        double height = extent is not null && Raw(extent, "cy") > 0 ? Raw(extent, "cy") : size.Height.Emu;

        double spanX = childExtent is null ? 0 : Raw(childExtent, "cx");
        double spanY = childExtent is null ? 0 : Raw(childExtent, "cy");

        double scaleX = spanX > 0 ? width / spanX : 1;
        double scaleY = spanY > 0 ? height / spanY : 1;
        double originX = childOffset is null ? 0 : Raw(childOffset, "x");
        double originY = childOffset is null ? 0 : Raw(childOffset, "y");

        GroupTransform scale = new(
            scaleX, 0, 0, scaleY, -(scaleX * originX), -(scaleY * originY));

        // The outermost group's own orientation is applied after the fit rather than here — see
        // `Orient` — because LibreOffice turns the group as an object, once it has been sized.
        return orientation
            ? Turned(transformation, width, height)
                .Compose(Flipped(transformation, width, height))
                .Compose(scale)
            : scale;
    }

    /// <summary>A group's <c>flipH</c> and <c>flipV</c>, as a mirror about its rectangle's centre.</summary>
    private static GroupTransform Flipped(XElement transformation, double width, double height)
    {
        double across = IsSet(transformation, "flipH") ? -1 : 1;
        double down = IsSet(transformation, "flipV") ? -1 : 1;

        return new GroupTransform(
            across, 0, 0, down, (1 - across) * width / 2, (1 - down) * height / 2);
    }

    /// <summary>A group's <c>rot</c>, as a quarter turn about its rectangle's centre.</summary>
    /// <remarks>
    /// Clockwise, which in a downward <c>y</c> takes <c>(dx, dy)</c> to <c>(-dy, dx)</c>.
    /// </remarks>
    private static GroupTransform Turned(XElement transformation, double width, double height)
    {
        double turn = Angle(transformation.Attribute("rot")?.Value) ?? 0;
        double clockwise = ((turn % 360) + 360) % 360;
        int quarters = (int)Math.Round(clockwise / 90);
        if (Math.Abs((quarters * 90) - clockwise) > 0.01) quarters = 0;

        return (quarters % 4) switch
        {
            1 => new GroupTransform(0, -1, 1, 0, (width + height) / 2, (height - width) / 2),
            2 => new GroupTransform(-1, 0, 0, -1, width, height),
            3 => new GroupTransform(0, 1, -1, 0, (width - height) / 2, (width + height) / 2),
            _ => GroupTransform.Identity,
        };
    }

    /// <summary>An OOXML boolean attribute that is set.</summary>
    private static bool IsSet(XElement element, string name)
        => element.Attribute(name)?.Value is "1" or "true" or "on";

    /// <summary>One attribute as the number the file wrote, before any unit is assumed.</summary>
    /// <remarks>
    /// A group's child coordinates are in a space of the file's own choosing — the corpus letterhead
    /// counts in twips — so they must not be read as EMUs on the way in. Only the mapped result is a
    /// length.
    /// </remarks>
    private static double Raw(XElement element, string name)
        => element.Attribute(name)?.Value is { } value
           && double.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out double parsed)
            ? parsed
            : 0;

    /// <summary>
    /// How far text must stay clear, from the four <c>dist*</c> attributes.
    /// </summary>
    /// <remarks>
    /// On the anchor itself rather than in the wrap element, even though it is the wrap that uses it —
    /// <c>wp:wrapSquare</c> can restate the same four and usually does not.
    /// <para>
    /// The <c>wp:effectExtent</c> beside them is the room a shadow or a glow needs, and LibreOffice does
    /// fold it into the wrap margins (<c>GraphicImport.cxx</c>, the <c>WrapTextMode_PARALLEL</c> branch:
    /// <c>m_nRightMargin += aMSOBaseLeftTop.X + aMSOBaseSize.Width - (aLOBoundRect.X + aLOBoundRect.Width)</c>,
    /// which for an unrotated shape comes to the effect extent). It is deliberately <em>not</em> read, and
    /// the reason is a measurement rather than a principle: adding it horizontally moves the wrapped lines
    /// the right way by a twip, and adding it vertically raises the hole's top edge by a twip too — which
    /// on the corpus document makes the line above the frame touch it and narrows one line more than
    /// LibreOffice does. A whole line in the wrong place is a worse error than a twip, so neither is added
    /// until the asymmetry is understood. See the note in <c>XRay.Office.WordProcessing/TODO.md</c>.
    /// </para>
    /// </remarks>
    private static Margins Spacing(XElement anchor)
        => new(
            Emu(anchor.Attribute("distL")?.Value),
            Emu(anchor.Attribute("distT")?.Value),
            Emu(anchor.Attribute("distR")?.Value),
            Emu(anchor.Attribute("distB")?.Value));

    /// <summary>
    /// The <c>wp:effectExtent</c> of an inline drawing — the room its effects need beyond its extent.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Read for a <c>wp:inline</c> and not for a <c>wp:anchor</c>, which is the split LibreOffice makes:
    /// <c>GraphicImport.cxx</c>:1036-1055 handles <c>IMPORT_AS_DETECTED_INLINE</c> with a zero rotation by
    /// adding all four edges straight to the object's margins, where every other anchoring reaches the
    /// extent by a longer route through the wrap area. Those margins are what
    /// <c>SwAsCharAnchoredObjectPosition</c> enlarges the object rectangle by, so on an inline drawing the
    /// extent is simply part of how much room the drawing takes on its line.
    /// </para>
    /// <para>
    /// The four <c>dist*</c> attributes beside it are <strong>not</strong> added, and that is not an
    /// omission: on a <c>wp:inline</c> LibreOffice discards them, setting the matching margin to zero
    /// merely because the attribute is present (<c>GraphicImport.cxx</c>:1387-1398,
    /// <c>case LN_CT_Inline_distT: m_nTopMargin = 0;</c>). Measured — a fixture stating
    /// <c>distT="137160" distB="137160"</c> and no effect extent moves the line below it by
    /// <strong>0.00 pt</strong> against the control, on both installed references.
    /// </para>
    /// <para>
    /// <strong>Only for a drawing that stays a <em>shape</em>, which a plain picture does not.</strong>
    /// The whole block that folds the extent in sits inside <c>if (m_xShape.is())</c>
    /// (<c>GraphicImport.cxx</c>:879-883), and <c>bUseShape = !m_xGraphicObject.is()</c> two dozen
    /// lines above it: a picture with no rotation and no DrawingML effects is turned into a Writer
    /// graphic object by <c>createGraphicObject</c>, its shape is disposed, and the margins are never
    /// touched. The conversion is refused — so the drawing stays a shape and does get the extent —
    /// when the picture is rotated (fdo#70457) or carries <c>EffectProperties</c>,
    /// <c>3DEffectProperties</c> or <c>ArtisticEffectProperties</c> in its grab bag.
    /// </para>
    /// <para>
    /// Measured, because reading that from the source alone would not have been enough to act on.
    /// Same fixture as above with the shape replaced by a <c>pic:pic</c>, both references identical:
    /// a plain picture at <c>137160</c> on all four edges moves its line by <b>+0.00 pt</b>, and so
    /// does one carrying <c>gpp-pr</c>'s own asymmetric <c>l=19050 t=19050 r=21590 b=23495</c>; add an
    /// <c>a:effectLst</c> outer shadow and the same picture moves it by <b>+21.65</b>, exactly as the
    /// shape does. Applying it to plain pictures cost <c>gpp-pr-top-7-office-markets-4q-2023.docx</c>
    /// 3.35 pt on everything below its chart.
    /// </para>
    /// <para>
    /// A <strong>rotated</strong> drawing is skipped too, and that is a separate finding rather than
    /// the same one. It takes the other branch of the <c>nOOXAngle == 0</c> test, which derives the
    /// margins from the rotated snap rectangle; worked through for a 20 degree, 144 x 50.4 pt picture
    /// those margins come out <em>negative</em> on both edges and clamp to zero. The measurement
    /// agrees and settles it: that fixture grows its line by <b>+46.25 pt</b> with a
    /// <c>137160</c> extent and by <b>+46.25 pt</b> with <em>no extent at all</em>. So the growth
    /// there is the rotated bounding box, which we do not yet size a rotated inline drawing by, and
    /// none of it is the effect extent.
    /// </para>
    /// </remarks>
    /// <param name="placed">The <c>wp:inline</c> or <c>wp:anchor</c>.</param>
    /// <param name="anchor">The <c>wp:anchor</c>, or null when the drawing is inline.</param>
    /// <param name="shapeProperties">The drawing's <c>spPr</c>, for its rotation and its effects.</param>
    /// <returns>
    /// The four edges, or zero for an anchored drawing, a rotated one, a plain picture, or one
    /// stating no extent.
    /// </returns>
    private static Margins EffectExtent(XElement placed, XElement? anchor, XElement? shapeProperties)
    {
        if (anchor is not null) return Margins.Zero;
        if (Child(placed, "effectExtent") is not { } extent) return Margins.Zero;

        // The picture rule is the *unrotated* branch's alone. A rotated picture is never converted to
        // a Writer graphic object (fdo#70457), so it stays a shape and reaches the margin code like
        // any other drawing.
        if (Rotation(shapeProperties) == 0 && IsPlainPicture(placed, shapeProperties)) return Margins.Zero;

        return new Margins(
            Emu(extent.Attribute("l")?.Value),
            Emu(extent.Attribute("t")?.Value),
            Emu(extent.Attribute("r")?.Value),
            Emu(extent.Attribute("b")?.Value));
    }

    /// <summary>
    /// Whether a drawing is a picture LibreOffice would turn into a Writer graphic object rather than
    /// keep as a shape.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The two refusals in <c>GraphicImport.cxx</c>:820-841, in the order that file applies them: a
    /// non-zero rotation, and anything that puts <c>EffectProperties</c>, <c>3DEffectProperties</c>
    /// or <c>ArtisticEffectProperties</c> into the shape's grab bag. Rotation is checked by the
    /// caller, since it disqualifies a shape from this reading as well.
    /// </para>
    /// <para>
    /// The grab bag entries are written by <c>oox</c> from <c>a:effectLst</c> / <c>a:effectDag</c>,
    /// from <c>a:scene3d</c> / <c>a:sp3d</c>, and from the <c>a14:imgProps</c> artistic effects, so
    /// those are what is looked for here rather than the grab-bag names themselves.
    /// </para>
    /// </remarks>
    private static bool IsPlainPicture(XElement placed, XElement? shapeProperties)
    {
        if (Descendant(placed, "graphicData")?.Attribute("uri")?.Value is not { } uri) return false;
        if (!uri.EndsWith("/picture", StringComparison.Ordinal)) return false;

        bool statesEffects = shapeProperties is not null && shapeProperties.Descendants().Any(
            child => child.Name.LocalName is "effectLst" or "effectDag" or "scene3d" or "sp3d");

        return !statesEffects && Descendant(placed, "imgProps") is null;
    }

    /// <summary>
    /// The wrap, which is which of five sibling elements is present.
    /// </summary>
    /// <remarks>
    /// <c>wp:wrapNone</c> is the one whose name means the opposite of what it says: it is the mode in
    /// which the text ignores the frame entirely, ODF's <c>run-through</c>. Word's own UI calls it
    /// "behind text" or "in front of text" depending on the anchor's <c>behindDoc</c>, which changes the
    /// paint order and not the layout.
    /// </remarks>
    private static TextWrap WrapOf(XElement anchor)
    {
        foreach (XElement child in anchor.Elements())
        {
            switch (child.Name.LocalName)
            {
                case "wrapNone":
                    return TextWrap.Through;

                case "wrapTopAndBottom":
                    return TextWrap.TopAndBottom;

                case "wrapSquare" or "wrapTight" or "wrapThrough":
                    return child.Attribute("wrapText")?.Value switch
                    {
                        "left" => TextWrap.Left,
                        "right" => TextWrap.Right,
                        "largest" => TextWrap.Optimal,
                        _ => TextWrap.Both,
                    };

                default:
                    continue;
            }
        }

        return TextWrap.Through;
    }

    /// <summary>
    /// The horizontal position: an origin, and either an offset or an edge to align against.
    /// </summary>
    /// <remarks>
    /// <c>wp:align</c> and <c>wp:posOffset</c> are alternatives, not a pair — a frame stating the former
    /// has no coordinate at all, and reading a missing offset as zero would put every centred frame at
    /// the start margin.
    /// </remarks>
    private static (Length Offset, FrameHorizontalOrigin Origin, FrameHorizontalAlignment Alignment)
        Horizontal(XElement? anchor)
    {
        XElement? position = anchor is null ? null : Child(anchor, "positionH");
        if (position is null)
        {
            return (Length.Zero, FrameHorizontalOrigin.Column, FrameHorizontalAlignment.Left);
        }

        FrameHorizontalOrigin origin = position.Attribute("relativeFrom")?.Value switch
        {
            "page" => FrameHorizontalOrigin.Page,
            "margin" => FrameHorizontalOrigin.PageMargin,
            "character" => FrameHorizontalOrigin.Character,
            "leftMargin" => FrameHorizontalOrigin.LeftMarginArea,
            "rightMargin" => FrameHorizontalOrigin.RightMarginArea,
            // `insideMargin` is the page frame with a page toggle, which this model expresses through
            // the inside/outside *alignments* and a right-hand page, so the origin is the sheet.
            "insideMargin" => FrameHorizontalOrigin.Page,
            // And `outsideMargin` reaches no case at all: `PositionHandler::lcl_attribute` warns and
            // leaves `m_nRelation` at its `RelOrientation::FRAME` default, which is the text column
            // (`sw/source/writerfilter/dmapper/GraphicHelpers.cxx`:130-132). Both installed references
            // draw a fixture stating it at the column's own left edge, 72.00 pt, and not at 0.
            _ => FrameHorizontalOrigin.Column,
        };

        if (Child(position, "align")?.Value is { } align)
        {
            return (Length.Zero, origin, align switch
            {
                "left" => FrameHorizontalAlignment.Left,
                "center" => FrameHorizontalAlignment.Centre,
                "right" => FrameHorizontalAlignment.Right,
                "inside" => FrameHorizontalAlignment.Inside,
                "outside" => FrameHorizontalAlignment.Outside,
                _ => FrameHorizontalAlignment.Left,
            });
        }

        return (
            Emu(Child(position, "posOffset")?.Value), origin, FrameHorizontalAlignment.Offset);
    }

    /// <summary>The vertical position, the same shape as the horizontal one.</summary>
    private static (Length Offset, FrameVerticalOrigin Origin, FrameVerticalAlignment Alignment)
        Vertical(XElement? anchor)
    {
        XElement? position = anchor is null ? null : Child(anchor, "positionV");
        if (position is null)
        {
            return (Length.Zero, FrameVerticalOrigin.Paragraph, FrameVerticalAlignment.Top);
        }

        FrameVerticalOrigin origin = position.Attribute("relativeFrom")?.Value switch
        {
            "page" => FrameVerticalOrigin.Page,
            "margin" => FrameVerticalOrigin.PageMargin,
            // Not the same rectangle as `margin`, and not each other's: the two margin bands lie
            // either side of the body. `GraphicHelpers.cxx`:74-80 maps them to
            // `PAGE_PRINT_AREA_TOP` and `PAGE_PRINT_AREA_BOTTOM`.
            "topMargin" => FrameVerticalOrigin.TopMarginArea,
            "bottomMargin" => FrameVerticalOrigin.BottomMarginArea,
            "line" => FrameVerticalOrigin.Line,
            _ => FrameVerticalOrigin.Paragraph,
        };

        if (Child(position, "align")?.Value is { } align)
        {
            return (Length.Zero, origin, align switch
            {
                "top" => FrameVerticalAlignment.Top,
                "center" => FrameVerticalAlignment.Middle,
                "bottom" => FrameVerticalAlignment.Bottom,
                _ => FrameVerticalAlignment.Top,
            });
        }

        return (Emu(Child(position, "posOffset")?.Value), origin, FrameVerticalAlignment.Offset);
    }

    /// <summary>
    /// One EMU measurement, rounded to Writer's whole-twip grid.
    /// </summary>
    /// <remarks>
    /// Rounded here rather than kept exact, because every other measurement in the engine already is and
    /// a frame edge half a twip from a line's is the sort of difference that decides whether the line is
    /// narrowed at all — see the touching rule in <see cref="Layout.FrameObstacles"/>.
    /// </remarks>
    private static Length Emu(string? value)
        => value is not null && long.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out long emu)
            ? Length.FromTwips(Length.FromEmu(emu).Twips)
            : Length.Zero;

    /// <summary>
    /// The <c>wps:bodyPr</c> governing a shape's text, or null when it states none.
    /// </summary>
    /// <remarks>
    /// Searched as a descendant rather than as a child because the element sits beside
    /// <c>wps:txbx</c> inside <c>wps:wsp</c>, which is itself three levels below the anchor — and a
    /// group's leaf carries its own.
    /// </remarks>
    private static XElement? BodyProperties(XElement shape) => Descendant(shape, "bodyPr");

    /// <summary>
    /// The distance between a text box's edge and its text, from <c>wps:bodyPr</c>.
    /// </summary>
    /// <remarks>
    /// The defaults are ECMA-376 §20.1.2.2.9's, and they are not zero: 91440 EMU (0.1 in) left and
    /// right, 45720 (0.05 in) top and bottom. Reading them matters twice over — they narrow the lines,
    /// and they are what <see cref="Layout.PageFrame.HasFixedHeight"/> measures the fit against, so a
    /// box 15 pt tall holds 7.8 pt of text rather than 15.
    /// </remarks>
    private static Margins Insets(XElement shape)
    {
        XElement? body = BodyProperties(shape);
        return new Margins(
            Inset(body, "lIns", DefaultHorizontalInsetEmu),
            Inset(body, "tIns", DefaultVerticalInsetEmu),
            Inset(body, "rIns", DefaultHorizontalInsetEmu),
            Inset(body, "bIns", DefaultVerticalInsetEmu));

        static Length Inset(XElement? body, string name, long fallback)
            => Emu(body?.Attribute(name)?.Value ?? fallback.ToString(CultureInfo.InvariantCulture));
    }

    /// <summary>ECMA-376 §20.1.2.2.9's default left and right text inset, in EMUs.</summary>
    private const long DefaultHorizontalInsetEmu = 91440;

    /// <summary>Its default top and bottom inset.</summary>
    private const long DefaultVerticalInsetEmu = 45720;

    /// <summary>
    /// Whether a shape's text body grows to fit its text rather than keeping the stated height.
    /// </summary>
    /// <remarks>
    /// Only <c>a:spAutoFit</c> does. <c>a:noAutofit</c>, <c>a:normAutofit</c> and a body stating
    /// nothing all keep the height — measured, not assumed: LibreOffice truncates a
    /// <c>normAutofit</c> box exactly as it truncates a <c>noAutofit</c> one rather than shrinking the
    /// text to fit. See <see cref="Layout.PageFrame.HasFixedHeight"/>.
    /// </remarks>
    /// <summary>
    /// Where a shape's text sits in a box taller than itself — <c>wps:bodyPr/@anchor</c>.
    /// </summary>
    /// <remarks>
    /// <c>just</c> and <c>dist</c> come back as top. Both ask for the lines to be spread through the
    /// box rather than for the block to be moved, which is a different mechanism from an anchor, and
    /// no corpus document states either.
    /// </remarks>
    private static VerticalTextAlignment TextAlignment(XElement shape)
        => BodyProperties(shape)?.Attribute("anchor")?.Value switch
        {
            "ctr" => VerticalTextAlignment.Middle,
            "b" => VerticalTextAlignment.Bottom,
            _ => VerticalTextAlignment.Top,
        };

    /// <summary>
    /// Whether the shape's height is its text's rather than the <c>a:ext</c> it states.
    /// </summary>
    /// <remarks>
    /// <c>a:spAutoFit</c>, which <c>TextBodyPropertiesContext</c>
    /// (<c>oox/source/drawingml/textbodypropertiescontext.cxx</c>:245-251) turns into
    /// <c>TextAutoGrowHeight</c> — for horizontal text only; a vertical body keeps its stated height.
    /// It decides two separate things, and both are set from it: the shape is not truncated to the
    /// stated height (<see cref="PageFrame.HasFixedHeight"/>) and it is not held at it either
    /// (<see cref="PageFrame.GrowsToContent"/> with a floor of zero — see
    /// <see cref="PageFrame.HeightFloor"/> for the twelve boxes that measured it).
    /// </remarks>
    private static bool GrowsWithText(XElement shape)
        => BodyProperties(shape) is { } body && Child(body, "spAutoFit") is not null;

    /// <summary>
    /// Whether the shape's <em>height</em> is taken from its text, which is narrower than
    /// <see cref="GrowsWithText"/> by exactly one case.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>A WordArt shape keeps the height the file states however it autofits.</b>
    /// <c>WpsContext::correctTextBoxesForWordArt</c>
    /// (<c>oox/source/shape/WpsContext.cxx</c>:1021-1023) turns a shape whose body carries an
    /// <c>a:prstTxWarp</c> other than <c>textNoShape</c> into a Fontwork and then clears both
    /// <c>TextAutoGrowHeight</c> and <c>TextAutoGrowWidth</c>, with the reason in the source:
    /// <em>"Fontwork stretches the text to the given path. So adapt shape size to text is
    /// nonsensical."</em>
    /// </para>
    /// <para>
    /// Found by sweeping the change that added this: fitting every <c>a:spAutoFit</c> shape to its
    /// text took <c>words/done-014/docx/exhibit-06---technical-architecture-template.docx</c> from
    /// <b>0.13 to 3.18</b> unsigned ink and from no major page to two, because each of its two
    /// headers holds a <c>fromWordArt="1"</c> shape stating 412 × 248 pt around one run of 1 pt
    /// <c>DRAFT</c>. Fitted to that run, the diagonal watermark collapses to a speck. Nothing else
    /// in the 337 moved either way.
    /// </para>
    /// <para>
    /// <see cref="PageFrame.HasFixedHeight"/> is deliberately left on the wider test. The reference
    /// does record <c>draw:auto-grow-height="false"</c> for such a shape, but what that means for a
    /// Fontwork — whose text is stretched along a path rather than broken into lines — is a
    /// different question from whether a text frame truncates, and no measurement was taken on it.
    /// Narrowing only the new half keeps this document exactly where it was before the round.
    /// </para>
    /// <para>
    /// The conversion has two further conditions this does not model, both narrowing it further:
    /// LibreOffice keeps the frame when the shape's geometry is not <c>ooxml-rect</c>, and when the
    /// frame's text yields no properties to copy. The first is checked here; the second is a shape
    /// with no text at all, whose fitted and stated heights differ by the insets alone.
    /// </para>
    /// </remarks>
    private static bool FitsShapeToText(XElement shape)
        => GrowsWithText(shape) && !IsFontwork(shape);

    /// <summary>Whether the shape is one Writer turns into a Fontwork.</summary>
    private static bool IsFontwork(XElement shape)
    {
        if (BodyProperties(shape) is not { } body) return false;
        if (Child(body, "prstTxWarp")?.Attribute("prst")?.Value is not { } warp) return false;
        if (warp is "textNoShape") return false;

        // `sType != "ooxml-rect"` — a warp on any other geometry leaves the frame alone.
        return Descendant(shape, "spPr") is { } properties
               && Child(properties, "prstGeom")?.Attribute("prst")?.Value == "rect";
    }

    /// <summary>A child by local name, in whichever namespace it was written.</summary>
    /// <remarks>
    /// By local name because a drawing spans four namespaces — <c>wp:</c> for the anchor, <c>a:</c> for
    /// the graphic, <c>wps:</c> for the shape, <c>w:</c> for the text inside it — and matching the
    /// namespace of each would be four constants standing in for one distinction the file never makes.
    /// </remarks>
    /// <summary>
    /// Whether an extent has nothing in it — as opposed to having no <em>area</em>, which is what a
    /// line has and is not the same thing.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This asked <c>width &lt;= 0 || height &lt;= 0</c>, and that is the wrong question by exactly
    /// one case. <strong>A straight connector states a zero extent on the axis it does not span</strong>
    /// — a vertical rule is <c>&lt;a:ext cx="0" cy="3834765"/&gt;</c> — so an "or" drops every
    /// axis-aligned line in the document before anything can decide whether to stroke it. The
    /// diagonal in <c>PageDrawing.DrawFrame</c> that draws such a shape could not have run: nothing
    /// ever reached it.
    /// </para>
    /// <para>
    /// Censused over the 271 corpus <c>docx</c>: <b>733 group members across 52 documents</b> have a
    /// zero axis, and <b>every one of the 733 is a <c>line</c> or a <c>straightConnector1</c></b> —
    /// 640 and 93. Not one is a rectangle or a picture, so nothing else was being kept out. A
    /// further 94 top-level anchors across 31 documents are the same shape of thing. The genogram
    /// and organogram templates are where it shows: their boxes are joined by nothing but these,
    /// so the diagram drew as a grid of captions with no lines between them.
    /// </para>
    /// <para>
    /// Both axes zero is still nothing, and a negative extent is malformed rather than degenerate;
    /// both are refused.
    /// </para>
    /// </remarks>
    /// <summary>
    /// The angle a shape's <c>a:xfrm</c> turns it through, clockwise, in degrees.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>rot</c> is in sixtieths of a degree — <c>ST_Angle</c> — so 90° is <c>5400000</c> and the
    /// three quarter turns account for 213 of the corpus's 298 rotated shapes.
    /// </para>
    /// <para>
    /// Not reading it does not present as a shape at the wrong angle. It presents as a shape in the
    /// wrong <em>place</em> and the wrong shape: the arrows joining an organogram's boxes are
    /// horizontal connectors turned through 270°, so drawn unrotated they come out as short
    /// horizontal dashes beside the boxes rather than as vertical arrows between them. Censused
    /// over the 271 corpus <c>docx</c>: 298 shapes across 29 documents, of which 128 are
    /// <c>rect</c>, 122 <c>line</c> or <c>straightConnector1</c>, and 17 <c>downArrow</c>.
    /// </para>
    /// </remarks>
    /// <summary>The marker one end of an <c>a:ln</c> carries, or none.</summary>
    /// <remarks>
    /// <c>none</c> is written out as often as the attribute is omitted and means the same thing, so
    /// both come back as the default rather than as a marker whose type nothing can draw.
    /// </remarks>
    private static LineEnd Marker(XElement line, string which)
    {
        if (Child(line, which) is not { } end) return default;
        if (end.Attribute("type")?.Value is not { Length: > 0 } type || type == "none") return default;

        return new LineEnd(type, end.Attribute("w")?.Value, end.Attribute("len")?.Value);
    }

    /// <summary>
    /// The path an <c>a:custGeom</c> states, evaluated at the shape's own size, or nulls.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A custom geometry writes out its guides and paths instead of naming a preset, so it needs no
    /// catalogue — and <see cref="CustomShapeGeometry.Custom"/> has evaluated them for the slide
    /// side all along. This side asked only for <c>a:prstGeom</c>, so all <b>124 custom shapes
    /// across 21 corpus documents</b> were painted as their bounding rectangles: the storyboard
    /// templates' rings came out as squares, and their arrows — which are rotated — as diamonds.
    /// </para>
    /// <para>
    /// Evaluated here rather than at drawing time because the formulae need a size and the shape's
    /// is known here, where a preset is a name that costs nothing to carry. The two paths are kept
    /// apart because a subpath states whether it is filled and whether it is stroked, and every
    /// connector is one open subpath saying <c>fill="none"</c>.
    /// </para>
    /// </remarks>
    private static (GraphicsPath? Fill, GraphicsPath? Stroke) CustomOutline(
        XElement? properties, DocSize size)
    {
        if (properties is null) return (null, null);
        if (Child(properties, "custGeom") is not { } geometry) return (null, null);
        if (size.Width <= Length.Zero || size.Height <= Length.Zero) return (null, null);
        if (CustomShapeGeometry.Custom(geometry, size) is not { } shape) return (null, null);

        return (shape.FillOutline, shape.StrokeOutline);
    }

    private static double Rotation(XElement? properties)
        => Angle((properties is null ? null : Child(properties, "xfrm"))?.Attribute("rot")?.Value) ?? 0;

    /// <summary>
    /// The angle a shape's own text is drawn at, which its <c>wps:bodyPr</c> may state
    /// independently of the shape's.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>rot</c> on a body is the text's angle rather than an addition to the shape's, so a label
    /// turned 345° whose body says <c>rot="0"</c> is upright text across a slanting box. Absent, the
    /// text takes the shape's, which is the ordinary case of a label turning with what it labels.
    /// </para>
    /// <para>
    /// <b>Every one of the 112 rotated text-bearing shapes in the corpus states <c>rot="0"</c></b> —
    /// 107 plainly, 5 with <c>upright="1"</c> beside it — so taking the shape's angle would have been
    /// wrong on all 112. The reference agrees:
    /// <c>025_Unit_Circle_Chart_Cos_and_Sin_Model</c> puts 32 labels round a circle at 32 angles and
    /// LibreOffice draws every one horizontal.
    /// </para>
    /// </remarks>
    /// <summary>Whether an angle is a quarter turn either way, to within rounding.</summary>
    /// <remarks>
    /// The tolerance is a hundredth of a degree rather than exact equality because the angle comes
    /// from an integer count of sixtieths and a file may state 5399999 as readily as 5400000.
    /// </remarks>
    /// <summary>A positive remainder, for an angle that a mirror has left free by a half turn.</summary>
    private static double Modulo(double value, double by)
        => ((value % by) + by) % by;

    private static bool IsQuarterTurn(double degrees)
        => Math.Abs((((degrees % 180) + 180) % 180) - 90) < 0.01;

    /// <remarks>
    /// <para>
    /// A text box states its text's own turn in <c>wps:bodyPr/@rot</c> and takes the shape's when it
    /// states none. The <em>group's</em> turn is deliberately not added to either, because
    /// LibreOffice does not turn a member's text with its group: measured against 26.2.4.2 on a text
    /// box inside a group stating <c>rot="10800000"</c>, the shape lands at the opposite corner —
    /// which this reader reproduces to the point — and its text is still drawn upright at the top
    /// left, where turning it would stand it on its head. <c>oox</c>'s <c>lcl_mirrorAtCenter</c> is
    /// why: a parent's negative scale becomes the child's own <c>flipH</c>/<c>flipV</c>, and a half
    /// turn decomposes into exactly that pair — two mirrors, which move a rectangle and leave its
    /// text alone.
    /// </para>
    /// <para>
    /// At a quarter turn the reference draws the text nowhere at all, so there is nothing there to
    /// match; upright is no further from it than turned, and it is the same rule.
    /// </para>
    /// </remarks>
    private static double TextRotation(XElement shape, double shapeRotation)
        => Angle(BodyProperties(shape)?.Attribute("rot")?.Value) ?? shapeRotation;

    /// <summary>An <c>ST_Angle</c> — sixtieths of a degree — as degrees, or null when unstated.</summary>
    private static double? Angle(string? value)
        => value is not null
           && long.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out long angle)
            ? angle / 60000.0
            : null;

    private static bool IsEmpty(Length width, Length height)
        => width < Length.Zero
           || height < Length.Zero
           || (width <= Length.Zero && height <= Length.Zero);

    /// <summary>
    /// What Writer builds for this drawing, which is what decides whether the page capture reaches it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <see cref="FrameObjectKind"/> carries the C++ rule. The mapping is the one
    /// <c>oox</c>/<c>writerfilter</c> makes: a <c>pic:pic</c> becomes an <c>SwFlyFrameFormat</c>
    /// holding a graphic and an <c>a:graphicData</c> naming a chart or an embedded object becomes one
    /// holding an OLE object, so both are flies; a <c>wps:wsp</c> is an <c>SdrObject</c>, and one
    /// carrying a <c>wps:txbx</c> is half of a TextBox pair, which is what
    /// <c>SwTextBoxHelper::isTextBox</c> answers to.
    /// </para>
    /// <para>
    /// The fall-through is <see cref="FrameObjectKind.Fly"/> rather than
    /// <see cref="FrameObjectKind.Shape"/>, because the one thing an anchored drawing can be that is
    /// none of the three named elements is a top-level chart or OLE object — which states no
    /// <c>a:graphicFrame</c> of its own at this level, only an <c>a:graphicData</c> with a chart URI.
    /// The three <c>Unit_Circle</c> documents are exactly that, one chart each.
    /// </para>
    /// </remarks>
    /// <param name="placed">The <c>wp:anchor</c> or <c>wp:inline</c>.</param>
    /// <param name="box">Its <c>w:txbxContent</c>, already looked up by the caller, or null.</param>
    private static FrameObjectKind ObjectKindOf(XElement placed, XElement? box)
    {
        if (box is not null) return FrameObjectKind.TextBoxShape;
        if (Descendant(placed, "pic") is not null) return FrameObjectKind.Fly;
        if (Descendant(placed, "wsp") is not null) return FrameObjectKind.Shape;

        return FrameObjectKind.Fly;
    }

    /// <summary>
    /// Whether this drawing follows the text flow, which is a question only an <em>embedded object</em>
    /// answers yes to.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <see cref="PageFrame.FollowsTextFlow"/> carries the seats and
    /// <see cref="FrameLayout.Place"/> what it costs. Three conditions, and each of them is one the
    /// writerfilter makes rather than one the specification does:
    /// </para>
    /// <list type="bullet">
    ///   <item><description><b>an anchor</b>, not a <c>wp:inline</c>: the write is guarded by
    ///     <c>m_eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR</c>
    ///     (<c>DomainMapper_Impl.cxx</c>:9788);</description></item>
    ///   <item><description><b>an embedded object</b>, which is the whole of why the guard is missing —
    ///     a chart's or an OLE object's graphic data becomes a
    ///     <c>com.sun.star.drawing.OLE2Shape</c> and is replaced by a <c>SwXTextEmbeddedObject</c>
    ///     (:5096-5112), and only that replacement's property writes skip the
    ///     <c>IsInTable()</c> test the shape and graphic paths make. A chart is the only embedded object
    ///     this reader builds a frame for;</description></item>
    ///   <item><description><b><c>layoutInCell</c> not explicitly off</b>. The attribute's absence is
    ///     <em>true</em> here, because <c>m_bLayoutInCell</c> is initialised true
    ///     (<c>GraphicImport.cxx</c>:340) and only the attribute's own handler ever clears it
    ///     (:707-712). <c>layoutInCell="0"</c> outside a table clears it, since the compatibility
    ///     force-on beside it needs <c>IsInTable()</c>.</description></item>
    /// </list>
    /// </remarks>
    /// <param name="anchor">The <c>wp:anchor</c>, or null for a <c>wp:inline</c>.</param>
    /// <param name="isEmbedded">Whether the drawing holds a chart or another embedded object.</param>
    private static bool FollowsTextFlow(XElement? anchor, bool isEmbedded)
        => anchor is not null
            && isEmbedded
            && anchor.Attribute("layoutInCell")?.Value is not ("0" or "false" or "off");

    private static XElement? Child(XElement parent, string name)
        => parent.Elements().FirstOrDefault(child => child.Name.LocalName == name);

    private static XElement? Descendant(XElement parent, string name)
        => parent.Descendants().FirstOrDefault(child => child.Name.LocalName == name);

    /// <summary>
    /// How a shape is painted: its fill, its outline colour, and how thick that outline is.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>We drew neither of these on a DrawingML shape until this existed</strong>, which is a
    /// defect no column of the gate can see and two rounds of blind reviewers found by looking. On
    /// <c>ABCD-WB-08-00</c> and <c>ABCD-SDE-23-00</c> the reference draws a grey header panel, a
    /// bordered box round a logo placeholder and a solid grey bar behind "Document reference:"; we drew
    /// none of them, so that bar's white text landed on white paper and was invisible. Confirmed in the
    /// reference's operators rather than in a raster — its page 2 carries nine transparency-group
    /// XObjects and six strokes against our none and four — because the fills carry
    /// <c>a:alpha</c> and LibreOffice's PDF export writes an alpha fill as a transparency group rather
    /// than as a plain <c>re f</c>, so grepping the content stream for a fill finds nothing.
    /// </para>
    /// <para>
    /// Only <c>a:solidFill</c> is read, on both the area and the line. A gradient, a pattern or a
    /// picture fill is a real fill this cannot yet draw, and painting its first stop as a flat colour
    /// would be a confident wrong answer rather than an absent one; each leaves the frame as it was.
    /// </para>
    /// <para>
    /// <strong>A shape that states no fill and no width still paints, because its
    /// <c>wps:style</c> names both out of the theme's format matrix.</strong> This paragraph used
    /// to say the opposite — that reading only what the shape itself states "is the conservative
    /// half of that and never invents ink" — and the corpus says the conservative half is not a
    /// half of anything. Censused over the 271 <c>docx</c> in the corpus, <b>511 shapes across 49
    /// documents</b> state an <c>a:fillRef</c> and no <c>a:*Fill</c> of their own, and every one
    /// of them was drawn as an empty outline: the organogram and genogram templates are nothing
    /// but such boxes, so the whole diagram came out blank.
    /// </para>
    /// <para>
    /// <c>020_Project_Timeline_Template_Modern_Theme</c> is the case that pins both halves to the
    /// reference's own operators. Its thirteen Gantt bars are <c>homePlate</c> shapes whose
    /// <c>wps:spPr</c> carries an <c>a:xfrm</c>, an <c>a:prstGeom</c> and an <c>a:ln</c> naming a
    /// colour — and no fill and no <c>w</c> at all. The theme's first fill style is
    /// <c>phClr</c>, so <c>a:fillRef idx="1"</c> over <c>accent1</c> is <c>#5B9BD5</c>, which is
    /// the blue the reference paints them; its second line style is <c>w="12700"</c>, so
    /// <c>a:lnRef idx="2"</c> is one point, which is the <c>1 w</c> the reference's content stream
    /// sets fourteen times. Neither number is anywhere in the document part.
    /// </para>
    /// <para>
    /// The width is why the shape's own <c>a:ln</c> is laid <em>over</em> the theme's rather than
    /// replacing it. Taking the shape's element alone gives a stroke of zero, and
    /// <c>PageDrawing.DrawFrame</c> drops a zero-width border — so a bar that states its outline
    /// colour explicitly lost that outline to the absence of an attribute it never had to state.
    /// <see cref="DrawingStyleMatrix.Overlay"/> is the same merge the slide side does, and
    /// <c>a:noFill</c> still beats the matrix on both sides: a shape suppressing its outline under
    /// an <c>a:lnRef</c> has none.
    /// </para>
    /// <para>
    /// <c>a:gradFill</c> is read too, on the area, and by the same
    /// <see cref="DrawingGradient"/> the slide side reads a stated one with — so a themed gradient
    /// is read by the code that reads a stated one rather than by a second copy of it, and
    /// <c>a:fillRef idx="2"</c> and <c>idx="3"</c>, which every Office theme writes as gradients,
    /// resolve as well as <c>idx="1"</c> does. It is carried unplaced, because a
    /// <c>GradientPaint</c> holds absolute points and a frame does not know where it lands until
    /// the layout engine has placed it; <c>PageDrawing</c> supplies the rectangle.
    /// </para>
    /// <para>
    /// A pattern or a picture fill is still a real fill this cannot yet draw, and painting its
    /// first stop as a flat colour would be a confident wrong answer rather than an absent one;
    /// each leaves the frame as it was. So does a gradient on the <em>line</em>, which LibreOffice
    /// reduces to one colour through <c>getBestSolidColor</c> and this does not.
    /// </para>
    /// </remarks>
    /// <param name="properties">The shape's own <c>spPr</c>, or null.</param>
    /// <param name="context">The theme and format matrix its style reference resolves against.</param>
    /// <param name="inherited">
    /// What the enclosing group offers a shape saying <c>a:grpFill</c>, which is the group's own
    /// fill resolved on the way down. Default for a shape that is not in one.
    /// </param>
    private static FrameAppearance Appearance(
        XElement? properties, DocxFrameContext context, FrameAppearance inherited = default)
    {
        if (properties is null) return default;

        DrawingTheme? theme = context.Theme;

        // The style sits beside the spPr under the same wps:wsp, so it is reached through the
        // parent rather than by descending: a shape's own style must not be taken from a drawing
        // nested inside its text box.
        XElement? style = properties.Parent?.Elements()
            .FirstOrDefault(child => child.Name.LocalName == "style");

        Colour? fill = Solid(Child(properties, "solidFill"), theme);
        GradientDescription? gradient = DrawingGradient.Read(Child(properties, "gradFill"), theme);

        // `a:grpFill` is not a fill but a reference to the enclosing group's, so it takes what the
        // group offered and ends the search: a shape asking for one that has none is unfilled
        // rather than falling through to its style's.
        if (Child(properties, "grpFill") is not null)
        {
            fill = inherited.Fill;
            gradient = inherited.Gradient;
        }

        if (fill is null && gradient is null && !StatesFill(properties)
            && context.Styles?.Fill(style, theme) is { } themedFill)
        {
            fill = Solid(Child(themedFill, "solidFill"), theme);
            gradient = DrawingGradient.Read(Child(themedFill, "gradFill"), theme);
        }

        XElement? themedLine = context.Styles?.Line(style, theme);
        XElement? line = Child(properties, "ln");

        if (line is null) line = themedLine;
        else if (themedLine is not null && Child(line, "noFill") is null)
            line = DrawingStyleMatrix.Overlay(themedLine, line);

        if (line is null) return new FrameAppearance { Fill = fill, Gradient = gradient };

        Colour? stroke = Solid(Child(line, "solidFill"), theme);
        if (stroke is null) return new FrameAppearance { Fill = fill, Gradient = gradient };

        return new FrameAppearance
        {
            Fill = fill,
            Gradient = gradient,
            Line = stroke,
            Width = Emu(line.Attribute("w")?.Value),
            HeadEnd = Marker(line, "headEnd"),
            TailEnd = Marker(line, "tailEnd"),
            Dash = Child(line, "prstDash")?.Attribute("val")?.Value,
            Cap = Cap(line.Attribute("cap")?.Value),
        };

        static Colour? Solid(XElement? solidFill, DrawingTheme? palette)
            => solidFill is null
                ? null
                : DrawingColour.Read(solidFill.Elements().FirstOrDefault())?.Resolve(palette);

        // `a:ln/@cap`, whose default is `flat`. It decides the ends of the line and also, through
        // `DashPresets`, how long each dash is: MSO measures a round or square cap inside the ink
        // and LibreOffice compensates by moving 99% of it into the gap
        // (`oox/source/drawingml/lineproperties.cxx`:470-479).
        static LineCap Cap(string? value) => value switch
        {
            "rnd" => LineCap.Round,
            "sq" => LineCap.Square,
            _ => LineCap.Butt,
        };

        // Any of the six, not just a:solidFill: a shape stating a:noFill means it, and one
        // stating a gradient or a picture means that rather than the theme's flat colour.
        static bool StatesFill(XElement shapeProperties)
            => shapeProperties.Elements().Any(child => child.Name.LocalName
                is "noFill" or "solidFill" or "gradFill" or "blipFill" or "pattFill" or "grpFill");
    }

    /// <summary>
    /// The <c>spPr</c> of the shape a drawing holds, or null when it holds none.
    /// </summary>
    /// <remarks>
    /// A descendant rather than a child, for the reason <see cref="BodyProperties"/> is: the element
    /// sits three levels below the anchor, under <c>wps:wsp</c> or <c>pic:pic</c>. The first in document
    /// order is the outermost shape's — a <c>wps:spPr</c> precedes the <c>wps:txbx</c> that could hold a
    /// nested drawing of its own — so this reads the shape asked about and not one inside its text.
    /// </remarks>
    private static XElement? ShapeProperties(XElement placed) => Descendant(placed, "spPr");

    /// <summary>The shape's preset name and stated adjustments, or nulls when it declares none.</summary>
    /// <remarks>
    /// <para>
    /// <c>rect</c> is returned as null on purpose. It is the bounding box the drawing code already
    /// paints, so resolving it through the preset catalogue would build a four-point path to
    /// arrive exactly where not asking arrives — and it is by far the commonest preset in the
    /// corpus, 64 of the 148 uses across the six templates that showed this.
    /// </para>
    /// <para>
    /// <c>line</c> and <c>straightConnector1</c> are left to <see cref="LineGeometry"/>, which
    /// already draws the diagonal they mean: their preset outline is the box, so taking it here
    /// would put three sides on the page that are not in the file.
    /// </para>
    /// </remarks>
    private static (string? Preset, IReadOnlyDictionary<string, double>? Adjustments)
        PresetGeometry(XElement? properties)
    {
        if (properties is null) return (null, null);

        XElement? geometry = Child(properties, "prstGeom");
        if (geometry?.Attribute("prst")?.Value is not { Length: > 0 } preset) return (null, null);
        if (preset is "rect" or "line" or "straightConnector1") return (null, null);

        Dictionary<string, double>? adjustments = null;
        XElement? values = Child(geometry, "avLst");
        foreach (XElement guide in
                 values?.Elements().Where(e => e.Name.LocalName == "gd") ?? [])
        {
            if (guide.Attribute("name")?.Value is not { Length: > 0 } name) continue;

            // `a:avLst` states a formula, and for an adjustment it is `val <n>` -- a literal. The
            // other forms are computed guides and belong to the preset rather than to the shape,
            // so anything that is not a plain value is left for the catalogue's own default.
            string? formula = guide.Attribute("fmla")?.Value;
            if (formula is null || !formula.StartsWith("val ", StringComparison.Ordinal)) continue;

            if (double.TryParse(
                    formula.AsSpan(4), System.Globalization.NumberStyles.Float,
                    System.Globalization.CultureInfo.InvariantCulture, out double value))
            {
                (adjustments ??= [])[name] = value;
            }
        }

        return (preset, adjustments);
    }

    /// <summary>
    /// Whether a shape's outline is its box's diagonal rather than its four sides, and which
    /// diagonal.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>a:prstGeom prst="line"</c> and <c>prst="straightConnector1"</c> are the two presets
    /// LibreOffice turns into a straight connector — <c>Shape::createAndInsert</c> maps
    /// <c>XML_line</c> and <c>XML_straightConnector1</c> to <c>ConnectorType_LINE</c>
    /// (<c>oox/source/drawingml/shape.cxx</c>:2124-2127) — which is drawn corner to opposite
    /// corner of the shape's own rectangle. Word writes a flowchart's arrows exactly this way, as
    /// a <c>wps:wsp</c> whose non-visual properties are a <c>wps:cNvCnPr</c> and whose extent is
    /// often zero in one dimension.
    /// </para>
    /// <para>
    /// Drawing the box instead puts three sides on the page that are not in the file, which is
    /// what this path did: measured on an authored one-line document, LibreOffice drew a red
    /// diagonal and X-Ray.Office a red rectangle. The VML and Escher front ends already model it —
    /// see <see cref="Layout.PageFrame.IsLine"/> — so only the DrawingML reading was missing.
    /// </para>
    /// <para>
    /// The flips choose the diagonal, and the pair of them cancels: a shape flipped in both
    /// directions is the same line it started as, rotated by half a turn.
    /// </para>
    /// </remarks>
    private static (bool IsLine, bool IsMirrored, bool IsReversed) LineGeometry(XElement? properties)
    {
        if (properties is null) return (false, false, false);

        string? preset = Child(properties, "prstGeom")?.Attribute("prst")?.Value;
        if (preset is not ("line" or "straightConnector1")) return (false, false, false);

        XElement? transform = Child(properties, "xfrm");
        bool flipH = IsTrue(transform?.Attribute("flipH")?.Value);
        bool flipV = IsTrue(transform?.Attribute("flipV")?.Value);

        // The diagonal is the exclusive-or; the direction along it is flipH alone. Writing the two
        // endpoints out settles it: the start is (flipH ? right : left, flipV ? bottom : top) and
        // the end is the opposite corner, so the x ordering — and only the x ordering — is what
        // flipH decides.
        return (true, flipH ^ flipV, flipH);

        // DrawingML's booleans, which are "1"/"true" and their negatives rather than w:val's.
        static bool IsTrue(string? value) => value is "1" or "true" or "on";
    }
}

/// <summary>
/// How a shape is painted: its area, its outline, and the markers its outline's ends carry.
/// </summary>
/// <remarks>
/// A record rather than the four-tuple this was, because the tuple had reached the width where the
/// call site said <c>(fill, gradient, line, lineWidth)</c> and a reader had to count commas to see
/// which was which — and the line ends made it six.
/// </remarks>
internal readonly record struct FrameAppearance
{
    /// <summary>The flat background colour, or null for none or for a gradient.</summary>
    public Colour? Fill { get; init; }

    /// <summary>The gradient background, or null. Never set together with <see cref="Fill"/>.</summary>
    public GradientDescription? Gradient { get; init; }

    /// <summary>The outline colour, or null when the shape is not stroked.</summary>
    public Colour? Line { get; init; }

    /// <summary>How thick that outline is.</summary>
    public Length Width { get; init; }

    /// <summary>The marker at the start of the line, if any.</summary>
    public LineEnd HeadEnd { get; init; }

    /// <summary>The marker at its end, if any.</summary>
    public LineEnd TailEnd { get; init; }

    /// <summary>The <c>a:prstDash/@val</c> naming the outline's pattern, or null for a solid line.</summary>
    public string? Dash { get; init; }

    /// <summary>How the outline's ends and its dashes are capped.</summary>
    public LineCap Cap { get; init; }
}

/// <summary>
/// What a drawing's surroundings decide about it, which the drawing itself does not state.
/// </summary>
/// <remarks>
/// A parameter object rather than three arguments because all three are inherited context: they are the
/// same for every frame in a walk and are threaded through the group recursion unchanged. The default —
/// no theme, not in a header, compatibility mode 0 — is what a caller that has none of it gets, and it
/// reproduces the behaviour every caller had before the type existed.
/// </remarks>
/// <param name="Theme">The theme a <c>a:schemeClr</c> resolves against, or null when there is none.</param>
/// <param name="InHeaderFooter">
/// Whether the drawing is anchored in a header or a footer, which decides its paint order on its own —
/// see <see cref="Layout.PageFrame.BehindText"/>.
/// </param>
/// <param name="CompatibilityMode">
/// The <c>compatibilityMode</c> compatibility setting, or 0 when the file states none. 15 and above is
/// Word 2013 and later, which changes what <c>behindDoc</c> means.
/// </param>
internal readonly record struct DocxFrameContext(
    DrawingTheme? Theme = null,
    bool InHeaderFooter = false,
    int CompatibilityMode = 0)
{
    /// <summary>
    /// The theme's <c>a:fmtScheme</c>, which a shape's <c>wps:style</c> indexes into, or null when
    /// the part declared none.
    /// </summary>
    /// <remarks>
    /// Not a positional member, for the reason <see cref="DrawingTheme.Fonts"/> is not: every
    /// caller that predates the format matrix — the whole of this type's test surface among them —
    /// keeps compiling and keeps its old behaviour, which is a shape painted from what it states
    /// and nothing else.
    /// </remarks>
    public DrawingStyleMatrix? Styles { get; init; }
}
