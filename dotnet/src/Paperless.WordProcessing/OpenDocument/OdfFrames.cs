using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.OpenDocument;

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
    /// Reads a frame, or returns null when the element is not one that can be placed.
    /// </summary>
    /// <param name="element">The <c>draw:frame</c>.</param>
    /// <param name="styles">The document's styles, for the graphic style the frame names.</param>
    /// <param name="content">
    /// How to read a text frame's own paragraphs, or null to record the frame without its content — which
    /// is what an image needs and all the wrap ever depends on.
    /// </param>
    /// <param name="anchorOffset">Where in the paragraph's text the frame is anchored.</param>
    public static PageFrame? Read(
        XElement element,
        OdfStyles styles,
        Func<XElement, IReadOnlyList<PageBlock>>? content,
        int anchorOffset)
    {
        ArgumentNullException.ThrowIfNull(element);
        ArgumentNullException.ThrowIfNull(styles);

        Length? width = Measure(element, "width");
        Length? height = Measure(element, "height");
        if (width is not { } frameWidth || height is not { } frameHeight) return null;

        OdfGraphicStyle style = GraphicStyle(
            styles, element.Attribute(XName.Get("style-name", OdfNamespaces.Draw))?.Value);

        XElement? box = element.Element(XName.Get("text-box", OdfNamespaces.Draw));
        XElement? image = element.Element(XName.Get("image", OdfNamespaces.Draw));

        return new PageFrame
        {
            Size = new DocSize(frameWidth, frameHeight),
            Anchor = AnchorOf(element.Attribute(XName.Get("anchor-type", OdfNamespaces.Text))?.Value),
            AnchorOffset = anchorOffset,
            Wrap = WrapOf(style.Wrap),
            HorizontalOrigin = HorizontalOriginOf(style.HorizontalRelative),
            HorizontalAlignment = HorizontalAlignmentOf(style.HorizontalPosition),
            HorizontalOffset = Measure(element, "x") ?? Length.Zero,
            VerticalOrigin = VerticalOriginOf(style.VerticalRelative),
            VerticalAlignment = VerticalAlignmentOf(style.VerticalPosition),
            VerticalOffset = Measure(element, "y") ?? Length.Zero,
            Spacing = style.Spacing,
            Padding = style.Padding,
            Fill = style.Fill,
            BorderColour = style.BorderColour,
            BorderWidth = style.BorderWidth,
            IsImage = image is not null,
            Name = element.Attribute(XName.Get("name", OdfNamespaces.Draw))?.Value,
            Blocks = box is not null && content is not null ? content(box) : [],
        };
    }

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
