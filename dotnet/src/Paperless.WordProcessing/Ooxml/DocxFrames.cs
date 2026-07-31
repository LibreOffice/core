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
    public static PageFrame? Read(
        XElement drawing,
        Func<XElement, IReadOnlyList<PageBlock>>? content,
        int anchorOffset)
    {
        ArgumentNullException.ThrowIfNull(drawing);

        XElement? anchor = Child(drawing, "anchor");
        XElement? inline = anchor is null ? Child(drawing, "inline") : null;
        XElement? placed = anchor ?? inline;
        if (placed is null) return null;

        XElement? extent = Child(placed, "extent");
        if (extent is null) return null;

        Length width = Emu(extent.Attribute("cx")?.Value);
        Length height = Emu(extent.Attribute("cy")?.Value);
        if (width <= Length.Zero || height <= Length.Zero) return null;

        XElement? box = Descendant(placed, "txbxContent");
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
            IsImage = box is null,
            Name = Child(placed, "docPr")?.Attribute("name")?.Value,
            Blocks = box is not null && content is not null ? content(box) : [],
        };
    }

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
