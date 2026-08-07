using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.Ooxml;

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
    public static IReadOnlyList<PageFrame> ReadAll(
        XElement drawing,
        Func<XElement, IReadOnlyList<PageBlock>>? content,
        int anchorOffset,
        DocxPictures? pictures = null)
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
        if (width <= Length.Zero || height <= Length.Zero) return [];

        if (Group(placed) is { } group)
        {
            return Members(group, placed, anchor, new DocSize(width, height), content, anchorOffset,
                           pictures);
        }

        PageFrame? single = One(placed, anchor, new DocSize(width, height), content, anchorOffset,
                                pictures);
        return single is null ? [] : [single];
    }

    /// <summary>The one frame an ordinary drawing holds.</summary>
    private static PageFrame? One(
        XElement placed,
        XElement? anchor,
        DocSize size,
        Func<XElement, IReadOnlyList<PageBlock>>? content,
        int anchorOffset,
        DocxPictures? pictures)
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

        return new PageFrame
        {
            Size = new DocSize(width, height),
            Anchor = anchor is null ? FrameAnchor.AsCharacter : FrameAnchor.Paragraph,
            AnchorOffset = anchorOffset,
            Wrap = anchor is null ? TextWrap.Through : WrapOf(anchor),
            HorizontalOrigin = horigin,
            HorizontalAlignment = halign,
            HorizontalOffset = x,
            VerticalOrigin = vorigin,
            VerticalAlignment = valign,
            VerticalOffset = y,
            Spacing = Spacing(placed),
            IsImage = box is null && chart.Plot is null,
            Image = picture.Raster,
            Vector = picture.Vector,
            Chart = chart.Plot,
            ChartFontFamily = chart.Family,
            Name = Child(placed, "docPr")?.Attribute("name")?.Value,
            Blocks = box is not null && content is not null ? content(box) : [],
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
        DocxPictures? pictures)
    {
        (Length x, FrameHorizontalOrigin horigin, FrameHorizontalAlignment halign) = Horizontal(anchor);
        (Length y, FrameVerticalOrigin vorigin, FrameVerticalAlignment valign) = Vertical(anchor);

        PageFrame envelope = new()
        {
            Size = size,
            Anchor = anchor is null ? FrameAnchor.AsCharacter : FrameAnchor.Paragraph,
            AnchorOffset = anchorOffset,
            Wrap = anchor is null ? TextWrap.Through : WrapOf(anchor),
            HorizontalOrigin = horigin,
            HorizontalAlignment = halign,
            HorizontalOffset = x,
            VerticalOrigin = vorigin,
            VerticalAlignment = valign,
            VerticalOffset = y,
            Spacing = Spacing(placed),
            IsImage = false,
            Name = Child(placed, "docPr")?.Attribute("name")?.Value,
        };

        List<PageFrame> frames = [envelope];

        Walk(group, TransformOf(group, size), 0);
        return frames;

        void Walk(XElement container, GroupTransform transform, int depth)
        {
            // Real files nest a group inside a group and stop; the bound is against a file that says
            // otherwise, since the walk is the only thing keeping it finite.
            if (depth > MaxGroupNesting) return;

            foreach (XElement child in container.Elements())
            {
                switch (child.Name.LocalName)
                {
                    case "grpSp" or "wgp" or "wpc":
                        Walk(child, transform.Composed(TransformOf(child, size)), depth + 1);
                        break;

                    case "wsp" or "pic" or "sp":
                    {
                        if (Leaf(child, transform, envelope, size, content, anchorOffset, pictures)
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

    /// <summary>How deep a group may nest before the walk gives up.</summary>
    private const int MaxGroupNesting = 8;

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
        DocxPictures? pictures)
    {
        XElement? properties = shape.Elements()
            .FirstOrDefault(child => child.Name.LocalName is "spPr");
        XElement? transformation = properties is null ? null : Child(properties, "xfrm");
        if (transformation is null) return null;

        XElement? offset = Child(transformation, "off");
        XElement? extent = Child(transformation, "ext");
        if (offset is null || extent is null) return null;

        DocRect within = transform.Map(
            Raw(offset, "x"), Raw(offset, "y"), Raw(extent, "cx"), Raw(extent, "cy"));

        if (within.Width <= Length.Zero || within.Height <= Length.Zero) return null;

        XElement? box = Descendant(shape, "txbxContent");
        FramePicture picture = box is null && pictures is not null
            ? pictures.Read(shape)
            : FramePicture.None;

        return envelope with
        {
            Size = new DocSize(within.Width, within.Height),
            GroupSize = size,
            GroupOffset = new DocPoint(within.X, within.Y),

            // The envelope keeps the anchor's wrap; a member must not punch a hole of its own, or a
            // nineteen-shape letterhead would narrow the text nineteen times over.
            Wrap = TextWrap.Through,
            Spacing = default,
            IsImage = box is null,
            Image = picture.Raster,
            Vector = picture.Vector,
            Chart = null,
            ChartFontFamily = null,
            AnchorOffset = anchorOffset,
            Name = Descendant(shape, "cNvPr")?.Attribute("name")?.Value,
            Blocks = box is not null && content is not null ? content(box) : [],
        };
    }

    /// <summary>
    /// A group's child-coordinate to group-rectangle mapping.
    /// </summary>
    /// <param name="OriginX">The child space's origin, <c>a:chOff/@x</c>.</param>
    /// <param name="OriginY">The child space's origin, <c>a:chOff/@y</c>.</param>
    /// <param name="ScaleX">Group width divided by <c>a:chExt/@cx</c>.</param>
    /// <param name="ScaleY">Group height divided by <c>a:chExt/@cy</c>.</param>
    /// <param name="ShiftX">Where the mapped rectangle starts inside the group, in EMUs.</param>
    /// <param name="ShiftY">The same, vertically.</param>
    private readonly record struct GroupTransform(
        double OriginX, double OriginY, double ScaleX, double ScaleY, double ShiftX, double ShiftY)
    {
        /// <summary>The identity, for a group that states no child space of its own.</summary>
        public static GroupTransform Identity => new(0, 0, 1, 1, 0, 0);

        /// <summary>This transform applied inside an enclosing one.</summary>
        public GroupTransform Composed(GroupTransform inner)
            => new(
                inner.OriginX, inner.OriginY,
                inner.ScaleX * ScaleX, inner.ScaleY * ScaleY,
                ShiftX + (inner.ShiftX * ScaleX), ShiftY + (inner.ShiftY * ScaleY));

        /// <summary>A child rectangle mapped into the group's own.</summary>
        public DocRect Map(double x, double y, double cx, double cy)
            => new(
                Round(ShiftX + ((x - OriginX) * ScaleX)),
                Round(ShiftY + ((y - OriginY) * ScaleY)),
                Round(cx * ScaleX),
                Round(cy * ScaleY));

        private static Length Round(double emu)
            => Length.FromTwips(Length.FromEmu((long)Math.Round(emu)).Twips);
    }

    /// <summary>The transform a group's own <c>a:xfrm</c> describes.</summary>
    private static GroupTransform TransformOf(XElement group, DocSize size)
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

        return new GroupTransform(
            childOffset is null ? 0 : Raw(childOffset, "x"),
            childOffset is null ? 0 : Raw(childOffset, "y"),
            spanX > 0 ? width / spanX : 1,
            spanY > 0 ? height / spanY : 1,
            0,
            0);
    }

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
    /// until the asymmetry is understood. See the note in <c>Paperless.WordProcessing/TODO.md</c>.
    /// </para>
    /// </remarks>
    private static Margins Spacing(XElement anchor)
        => new(
            Emu(anchor.Attribute("distL")?.Value),
            Emu(anchor.Attribute("distT")?.Value),
            Emu(anchor.Attribute("distR")?.Value),
            Emu(anchor.Attribute("distB")?.Value));

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
            "leftMargin" or "rightMargin" or "insideMargin" or "outsideMargin" =>
                FrameHorizontalOrigin.Page,
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
            "margin" or "topMargin" or "bottomMargin" => FrameVerticalOrigin.PageMargin,
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

    /// <summary>A child by local name, in whichever namespace it was written.</summary>
    /// <remarks>
    /// By local name because a drawing spans four namespaces — <c>wp:</c> for the anchor, <c>a:</c> for
    /// the graphic, <c>wps:</c> for the shape, <c>w:</c> for the text inside it — and matching the
    /// namespace of each would be four constants standing in for one distinction the file never makes.
    /// </remarks>
    private static XElement? Child(XElement parent, string name)
        => parent.Elements().FirstOrDefault(child => child.Name.LocalName == name);

    private static XElement? Descendant(XElement parent, string name)
        => parent.Descendants().FirstOrDefault(child => child.Name.LocalName == name);
}
