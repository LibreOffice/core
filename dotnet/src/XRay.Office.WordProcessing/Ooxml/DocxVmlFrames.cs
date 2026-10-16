using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml;
using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Layout;

namespace XRay.Office.WordProcessing.Ooxml;

/// <summary>
/// The room a <c>w:pict</c> or <c>w:object</c> takes on the line it sits on.
/// </summary>
/// <remarks>
/// <para>
/// A <c>w:drawing</c> states its size in a <c>wp:extent</c> and VML does not: a <c>v:shape</c> carries a
/// CSS <c>style</c>, and a <c>w:object</c> additionally carries <c>w:dxaOrig</c>/<c>w:dyaOrig</c>. Until
/// this existed the anchor character was the whole of the reserved height, so a figure occupied one text
/// line and every page after it was wrong.
/// </para>
/// <para>
/// <strong>Only an inline shape reserves anything, and that is the whole of why an earlier attempt at
/// this was reverted.</strong> A shape whose style says <c>position:absolute</c> is floating: it is
/// placed against the page or the paragraph and the text does not make room for it on a line.
/// Reserving for those as well is what added seven pages to <c>33004.docx</c>, which holds five
/// floating shapes and one inline. Measured over the corpus, the two are not a rare split — 161 of the
/// 224 sized VML shapes in the words track are floating.
/// </para>
/// <para>
/// <strong>The size is the one the style declares.</strong> Read off the reference's own content stream
/// on <c>EHEST-SMS-Safety-Management-Manual-V2.docx</c> page 18, whose Visio object declares
/// <c>style="width:425pt;height:190pt"</c>: LibreOffice draws the replacement image with a
/// <c>425.00 0 0 190.00 … cm</c>, exactly the declared box.
/// </para>
/// <para>
/// <strong>Take the <c>v:shape</c>, not the first VML child.</strong> A <c>w:object</c> Word writes
/// opens with a <c>v:shapetype</c> — the reusable geometry definition — and only then the <c>v:shape</c>
/// that uses it. The <c>v:shapetype</c> carries no <c>style</c>, so a reader that takes the first VML
/// element finds no size and silently reserves nothing. That is the difference between this and the
/// three probes the earlier attempt passed, which wrote a bare <c>v:shape</c>.
/// </para>
/// </remarks>
internal static class DocxVmlFrames
{
    /// <summary>Every frame a <c>w:pict</c> or <c>w:object</c> contributes.</summary>
    /// <remarks>
    /// A list rather than one frame because a single <c>w:pict</c> routinely holds several shapes —
    /// the Work Breakdown Structure templates put 49 across 13 of them — and taking the first was how
    /// the other 48 went missing.
    /// </remarks>
    /// <param name="element">The <c>w:pict</c> or <c>w:object</c>.</param>
    /// <param name="anchorOffset">Where in the paragraph's text it sits.</param>
    /// <param name="pictures">How to resolve <c>v:imagedata</c> into bytes, or null for geometry only.</param>
    /// <param name="content">How to read a <c>w:txbxContent</c> into blocks, or null to skip its text.</param>
    /// <param name="typeface">
    /// How to resolve a font family into a face, or null to draw no WordArt. Only a
    /// <c>v:textpath</c> needs it: its text is an attribute rather than a run, so it never reaches
    /// the layout that resolves every other face in the document.
    /// </param>
    public static List<PageFrame> ReadAll(
        XElement element,
        int anchorOffset,
        DocxPictures? pictures,
        Func<XElement, IReadOnlyList<PageBlock>>? content = null,
        Func<string?, OpenTypeFace?>? typeface = null)
    {
        ArgumentNullException.ThrowIfNull(element);

        List<PageFrame> frames = [];
        foreach (XElement top in TopLevel(element))
        {
            if (top.Name.LocalName is "group")
            {
                frames.AddRange(Group(top, anchorOffset, pictures, content));
                continue;
            }

            if (One(top, element, anchorOffset, pictures, content, typeface) is { } frame)
            {
                frames.Add(frame);
            }
        }

        return frames;
    }

    /// <summary>The <c>v:shapetype</c> a shape's <c>type="#id"</c> names, or null.</summary>
    /// <remarks>
    /// Word writes the definition once per <c>w:pict</c> and the shape refers to it, so the number
    /// that says which WordArt shape this is — <c>o:spt</c>, and the <c>136</c> in the id — lives on
    /// the sibling rather than on the shape. Searched from the whole <c>w:pict</c> because a
    /// <c>v:group</c>'s members refer to a definition written outside the group.
    /// </remarks>
    private static XElement? ShapeTypeOf(XElement shape, XElement scope)
    {
        if (shape.Attribute("type")?.Value is not { Length: > 1 } reference) return null;
        if (reference[0] != '#') return null;

        string id = reference[1..];
        foreach (XElement candidate in scope.Descendants(XName.Get("shapetype", OoxmlNamespaces.Vml)))
        {
            if (candidate.Attribute("id")?.Value == id) return candidate;
        }

        return null;
    }

    /// <summary>The VML shapes and groups of a <c>w:pict</c> that are not inside another group.</summary>
    /// <remarks>
    /// <para>
    /// <c>v:shapetype</c> is excluded — it is the reusable geometry definition Word writes ahead of
    /// the shape that uses it, it carries no <c>style</c>, and a reader that takes the first VML
    /// element finds no size and silently reserves nothing.
    /// </para>
    /// <para>
    /// <strong>It was not excluded, and nor was anything else.</strong> This filter read
    /// <c>IsShape(child) is not false</c> against a predicate that answered <c>true</c> or
    /// <c>null</c> and never <c>false</c>, so the only condition that did any work was the group
    /// test below: every descendant of the element was offered to <see cref="One"/>, a
    /// <c>v:imagedata</c>, a <c>v:f</c> and a <c>v:path</c> among them. Inside a <c>w:pict</c> that
    /// cost nothing, because <see cref="One"/> wants a size and those elements stated none. Inside
    /// a <c>w:object</c> it cost a duplicate of everything: <see cref="One"/> falls back to the
    /// <em>object's</em> <c>w:dxaOrig</c>/<c>w:dyaOrig</c>, which is the same pair for every
    /// descendant, and <c>DocxPictures.ReadVml</c> searches <c>DescendantsAndSelf</c>, so the
    /// <c>v:imagedata</c> element resolved itself and the replacement picture was drawn twice.
    /// </para>
    /// <para>
    /// Members of a <c>v:group</c> are excluded here and reached through <see cref="Group"/> instead,
    /// because their coordinates are in the group's space and mean nothing on their own. Measured on
    /// the Work Breakdown Structure templates: 18 of their 24 text boxes sit inside a group, so a
    /// reader that walked every descendant placed three quarters of the document's text at the origin.
    /// </para>
    /// </remarks>
    private static IEnumerable<XElement> TopLevel(XElement element)
        => element.Descendants().Where(child =>
            IsShape(child)
            && !child.Ancestors()
                .TakeWhile(ancestor => ancestor != element)
                .Any(ancestor => ancestor.Name == XName.Get("group", OoxmlNamespaces.Vml)));

    /// <summary>True for a VML element that is a shape in its own right.</summary>
    /// <remarks>
    /// <para>
    /// The ten are VML's own shape elements. <c>v:shape</c>, <c>v:rect</c>, <c>v:roundrect</c>,
    /// <c>v:oval</c> and <c>v:group</c> were always here; <c>v:line</c>, <c>v:polyline</c>,
    /// <c>v:curve</c>, <c>v:arc</c> and <c>v:image</c> are named rather than admitted by the
    /// accident of the predicate never answering false. Everything else answers false —
    /// <c>v:shapetype</c>, and the property elements (<c>v:fill</c>, <c>v:stroke</c>,
    /// <c>v:shadow</c>, <c>v:path</c>, <c>v:formulas</c>, <c>v:f</c>, <c>v:handles</c>,
    /// <c>v:textbox</c>, <c>v:textpath</c>, <c>v:imagedata</c>) which describe a shape rather
    /// than being one. <strong>Only that second half has any reach</strong>, and the first is
    /// named because it is what this method claims to answer rather than because the corpus
    /// needs it.
    /// </para>
    /// <para>
    /// <strong>Widening to the ten is a measured no-op, and an earlier draft of this remark said
    /// the opposite.</strong> It claimed 72 of the corpus's 150 top-level <c>v:line</c> state a
    /// sized <c>style</c>, so that narrowing back to the original five would drop them. The
    /// figure is wrong: <b>all 150 state <c>from</c>/<c>to</c> attributes and none states a
    /// <c>style</c> width or height at all</b>, every one is <c>position:absolute</c>, and
    /// <see cref="Floating"/> wants both from the style — so it returns null for each of them
    /// under either predicate. <see cref="Group"/> answers the same way for the other end: all
    /// <b>715</b> <c>v:line</c> that are a direct child of a corpus <c>v:group</c> are missing at
    /// least one of the <c>left</c>, <c>top</c>, <c>width</c> and <c>height</c> its own guard
    /// requires. Rendering the <b>53</b> corpus documents that hold any of the five added
    /// elements, once with this set and once narrowed to the original five, gives <b>53
    /// byte-identical PDFs</b>. <c>probes/vmldup-r140/census-vml.py</c>.
    /// </para>
    /// <para>
    /// What that measurement also says is that <strong>none of those 150 rules is drawn by this
    /// tree at all</strong>, because a <c>v:line</c> states its extent in <c>from</c>/<c>to</c>
    /// and nothing here reads them. That is a gap of its own and not this method's to close.
    /// </para>
    /// </remarks>
    private static bool IsShape(XElement child)
        => child.Name.Namespace == OoxmlNamespaces.Vml
           && child.Name.LocalName is
               "shape" or "rect" or "roundrect" or "oval" or "group"
               or "line" or "polyline" or "curve" or "arc" or "image";

    /// <summary>
    /// A <c>v:group</c> flattened into one frame per member, each mapped out of the group's own
    /// coordinate space — nested groups included.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A group states where it sits and how big it is the way any floating shape does — in its
    /// <c>style</c>, in real units — and then declares a <em>child</em> coordinate space with
    /// <c>coordsize</c> (and <c>coordorigin</c>, which defaults to <c>0,0</c>). Its members'
    /// <c>left</c>, <c>top</c>, <c>width</c> and <c>height</c> are bare numbers in that space, so a
    /// member at <c>left:5400</c> in a group whose <c>coordsize</c> is <c>21600,21600</c> sits a
    /// quarter of the way across the group, whatever the group's width in points happens to be.
    /// </para>
    /// <para>
    /// Reading those bare numbers as points is the failure this exists to avoid: they are routinely
    /// in the thousands, so every member lands metres off the page and its text is clipped away.
    /// </para>
    /// <para>
    /// <strong>A nested group resolves to a rectangle by exactly that arithmetic, and is then the
    /// origin and extent its own members are measured against.</strong> This used to skip one —
    /// <c>if (member.Name.LocalName is "group") continue;</c> — and with it the whole subtree beneath
    /// it. Measured on <c>068_Work_Breakdown_Structure_Template_Green_Theme</c>, which draws its 41
    /// boxes as a root, five phase groups each holding a connector and a nested group, and 35 task
    /// boxes inside those five nested groups: we drew the root and the five phase labels and nothing
    /// deeper, 19 words against the reference's 86, and <b>the words inside the nested groups number
    /// exactly 67</b>. A blind reviewer given only the rendered page, and knowing nothing about the
    /// markup, reported the same shape from the other side — <em>"the surviving items are exactly the
    /// top two levels of the tree, in tree order, not a random scatter and not a partial-column
    /// truncation; there is no piling up and no overlapping, the failure is omission"</em> — and an
    /// earlier reviewer proposed the discriminator itself: if every rendered item is at nesting depth
    /// ≤ 1 and every missing one is ≥ 2, it is a recursion limit and not a fill problem.
    /// </para>
    /// <para>
    /// One words document in 337 holds a nested <c>v:group</c> this reader reaches. Six others —
    /// <c>056</c>, <c>057</c>, <c>025</c>, <c>030</c>, <c>008</c>, <c>071</c> — hold 19 to 40 of them
    /// each, and every one is inside an <c>mc:Fallback</c> whose <c>mc:Choice</c> DrawingML is what we
    /// read. Deleting <c>056</c>'s entire <c>mc:Fallback</c> leaves its rendering's word count
    /// unchanged, which is what establishes that rather than the markup's shape.
    /// </para>
    /// </remarks>
    private static List<PageFrame> Group(
        XElement group,
        int anchorOffset,
        DocxPictures? pictures,
        Func<XElement, IReadOnlyList<PageBlock>>? content)
    {
        List<PageFrame> frames = [];

        Dictionary<string, string> style = Style(group);
        if ((style.TryGetValue("width", out string? gw) ? Css(gw) : null) is not { } groupWidth
            || (style.TryGetValue("height", out string? gh) ? Css(gh) : null) is not { } groupHeight
            || groupWidth <= Length.Zero
            || groupHeight <= Length.Zero)
        {
            return frames;
        }

        Length originX = (style.TryGetValue("margin-left", out string? ml) ? Css(ml) : null)
                         ?? Length.Zero;
        Length originY = (style.TryGetValue("margin-top", out string? mt) ? Css(mt) : null)
                         ?? Length.Zero;

        Flatten(
            group,
            new DocRect(originX, originY, groupWidth, groupHeight),
            HorizontalOriginOf(style),
            VerticalOriginOf(style),
            LayerOf(style),
            depth: 0,
            frames,
            anchorOffset,
            pictures,
            content);

        return frames;
    }

    /// <summary>How deep a <c>v:group</c> may nest before the walk gives up.</summary>
    /// <remarks>
    /// A bound against a file that says something absurd, not a modelled limit — the corpus's deepest
    /// is two. The same number <c>DocxFrames</c> uses for the DrawingML side.
    /// </remarks>
    private const int MaxGroupNesting = 8;

    /// <summary>
    /// One group's members, resolved against the rectangle the group itself occupies.
    /// </summary>
    /// <param name="group">The <c>v:group</c> whose members are being walked.</param>
    /// <param name="area">Where that group sits and how big it is, in real units.</param>
    /// <param name="horizontal">What a member's horizontal offset is measured from.</param>
    /// <param name="vertical">What a member's vertical offset is measured from.</param>
    /// <param name="layer">Which layer the group paints on, inherited by every member.</param>
    /// <param name="depth">How many groups deep this one is.</param>
    /// <param name="frames">The frames collected so far, appended to.</param>
    /// <param name="anchorOffset">Where in the paragraph's text the drawing sits.</param>
    /// <param name="pictures">How to resolve <c>v:imagedata</c> into bytes, or null for geometry only.</param>
    /// <param name="content">How to read a <c>w:txbxContent</c> into blocks, or null to skip its text.</param>
    private static void Flatten(
        XElement group,
        DocRect area,
        FrameHorizontalOrigin horizontal,
        FrameVerticalOrigin vertical,
        VmlLayer layer,
        int depth,
        List<PageFrame> frames,
        int anchorOffset,
        DocxPictures? pictures,
        Func<XElement, IReadOnlyList<PageBlock>>? content)
    {
        if (depth > MaxGroupNesting) return;

        (double spaceX, double spaceY) = Pair(group.Attribute("coordsize")?.Value) ?? (0, 0);
        (double baseX, double baseY) = Pair(group.Attribute("coordorigin")?.Value) ?? (0, 0);
        if (spaceX <= 0 || spaceY <= 0) return;

        foreach (XElement member in group.Elements().Where(IsShape))
        {
            Dictionary<string, string> box = Style(member);

            // A `v:line` inside a group states its box in `from`/`to`, in the group's own
            // coordinate units — see `RelativeLineOf` — and its `style` rectangle is not read.
            (double Left, double Top, double Width, double Height, bool Mirrored)? segment =
                RelativeLineOf(member);

            double left, top, wide, tall;
            if (segment is { } drawn)
            {
                (left, top, wide, tall) = (drawn.Left, drawn.Top, drawn.Width, drawn.Height);
            }
            else if (Number(box.GetValueOrDefault("left", "")) is { } statedLeft
                     && Number(box.GetValueOrDefault("top", "")) is { } statedTop
                     && Number(box.GetValueOrDefault("width", "")) is { } statedWide
                     && Number(box.GetValueOrDefault("height", "")) is { } statedTall)
            {
                (left, top, wide, tall) = (statedLeft, statedTop, statedWide, statedTall);
            }
            else
            {
                continue;
            }

            // A straight connector is how VML writes a rule, and a vertical one is
            // `width:0;height:7035`. A `v:line` is the other shape with no area that draws.
            bool rule = IsStraightConnector(member) || segment is not null;
            if (wide < 0 || tall < 0) continue;
            if (!rule && (wide <= 0 || tall <= 0)) continue;

            DocRect placed = new(
                area.X + (area.Width * ((left - baseX) / spaceX)),
                area.Y + (area.Height * ((top - baseY) / spaceY)),
                area.Width * (wide / spaceX),
                area.Height * (tall / spaceY));

            if (member.Name.LocalName is "group")
            {
                Flatten(
                    member, placed, horizontal, vertical, layer, depth + 1,
                    frames, anchorOffset, pictures, content);
                continue;
            }

            XElement? text = TextBox(member);
            FramePicture picture = text is null && pictures is not null
                ? pictures.ReadVml(member)
                : FramePicture.None;

            string? memberPreset = PresetOf(member, member);
            VmlPaint paint = PaintOf(member, box, memberPreset);
            if (segment is { } sense) paint = paint with { IsLineMirrored = sense.Mirrored };

            frames.Add(new PageFrame
            {
                Size = new DocSize(placed.Width, placed.Height),
                Anchor = FrameAnchor.Paragraph,
                AnchorOffset = anchorOffset,
                Wrap = TextWrap.Through,
                HorizontalOrigin = horizontal,
                HorizontalOffset = placed.X,
                VerticalOrigin = vertical,
                VerticalOffset = placed.Y,
                BehindText = layer.BehindText,
                ZOrder = layer.ZOrder,
                IsImage = text is null,
                Image = picture.Raster,
                Crop = picture.Crop,
                Vector = picture.Vector,
                Fill = paint.Fill,
                BorderColour = paint.Line,
                BorderWidth = paint.Width,
                Preset = memberPreset,
                IsLine = paint.IsLine,
                IsLineMirrored = paint.IsLineMirrored,
                HeadEnd = paint.HeadEnd,
                TailEnd = paint.TailEnd,
                Blocks = text is not null && content is not null ? content(text) : [],
                Padding = text is null ? default : Insets(text),
                HasFixedHeight = text is not null && !GrowsWithText(member, text),
            });
        }
    }

    /// <summary>A VML <c>x,y</c> attribute pair, or null when it is not one.</summary>
    private static (double X, double Y)? Pair(string? text)
    {
        if (text is null) return null;

        string[] parts = text.Split(',', StringSplitOptions.TrimEntries);
        return parts.Length == 2 && Number(parts[0]) is { } x && Number(parts[1]) is { } y
            ? (x, y)
            : null;
    }

    /// <summary>The same pair read as lengths, for the attributes that carry units.</summary>
    /// <param name="text">The attribute's value.</param>
    /// <param name="measure">How one coordinate is read.</param>
    private static (Length X, Length Y)? Pair(string? text, Func<string, Length?> measure)
    {
        if (text is null) return null;

        string[] parts = text.Split(',', StringSplitOptions.TrimEntries);
        return parts.Length == 2 && measure(parts[0]) is { } x && measure(parts[1]) is { } y
            ? (x, y)
            : null;
    }

    /// <summary>
    /// The leading integer of a token, which is what <c>o3tl::toInt32</c> takes.
    /// </summary>
    /// <remarks>
    /// A unit suffix is dropped rather than converted, so <c>100pt</c> is a hundred. That is the
    /// rule inside a <c>v:group</c>, where the numbers are coordinate units — see
    /// <see cref="RelativeLineOf"/>.
    /// </remarks>
    private static double? LeadingInteger(string text)
    {
        string value = text.Trim();
        int at = 0;
        if (at < value.Length && (value[at] is '-' or '+')) at++;
        int start = at;
        while (at < value.Length && char.IsAsciiDigit(value[at])) at++;
        if (at == start) return null;

        return double.TryParse(
            value[..at], NumberStyles.Integer, CultureInfo.InvariantCulture, out double parsed)
            ? parsed
            : null;
    }

    private static PageFrame? One(
        XElement shape,
        XElement element,
        int anchorOffset,
        DocxPictures? pictures,
        Func<XElement, IReadOnlyList<PageBlock>>? content,
        Func<string?, OpenTypeFace?>? typeface = null)
    {
        Dictionary<string, string> style = Style(shape);

        // Floating: the page places it, not the line. It still gets drawn — see the remarks.
        if (style.TryGetValue("position", out string? position)
            && position.Equals("absolute", StringComparison.OrdinalIgnoreCase))
        {
            return Floating(shape, style, element, anchorOffset, pictures, content, typeface);
        }

        Length? width = style.TryGetValue("width", out string? w) ? Css(w) : null;
        Length? height = style.TryGetValue("height", out string? h) ? Css(h) : null;

        // `w:dxaOrig`/`w:dyaOrig` are the object's original size in twentieths of a point, and are what
        // a `w:object` carries when its shape's style states no box.
        width ??= Twips(element, "dxaOrig");
        height ??= Twips(element, "dyaOrig");

        if (width is not { } across || height is not { } down) return null;
        if (across <= Length.Zero || down <= Length.Zero) return null;

        XElement? box = TextBox(shape);
        FramePicture picture = box is null && pictures is not null
            ? pictures.ReadVml(shape)
            : FramePicture.None;

        string? preset = PresetOf(shape, element);
        VmlPaint paint = PaintOf(shape, style, preset);
        VmlFontwork warp = DocxVmlFontwork.Read(
            shape, ShapeTypeOf(shape, element), new DocSize(across, down), typeface);

        if (warp.Outline is not null)
        {
            (across, down) = (warp.Box.Width, warp.Box.Height);
            paint = FontworkPaint(shape, style);
        }

        return new PageFrame
        {
            Size = new DocSize(across, down),
            Anchor = FrameAnchor.AsCharacter,
            AnchorOffset = anchorOffset,
            Wrap = TextWrap.Through,
            FillOutline = warp.Outline,
            StrokeOutline = warp.Outline,
            RotationDegrees = Rotation(style),
            IsImage = box is null && warp.Outline is null,
            Image = picture.Raster,
            Crop = picture.Crop,
            Vector = picture.Vector,
            Fill = paint.Fill,
            BorderColour = paint.Line,
            BorderWidth = paint.Width,
            Preset = preset,
            IsLine = paint.IsLine,
            IsLineMirrored = paint.IsLineMirrored,
            HeadEnd = paint.HeadEnd,
            TailEnd = paint.TailEnd,
            Blocks = box is not null && content is not null ? content(box) : [],
            Padding = box is null ? default : Insets(box),
            HasFixedHeight = box is not null && !GrowsWithText(shape, box),
        };
    }

    /// <summary>
    /// A floating VML shape: placed against its own origin, drawn, and reserving no line room.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The two halves have to be kept apart. <strong>Reserving room for a floating shape is wrong</strong>
    /// — that is what added seven pages to <c>33004.docx</c> and got the first attempt reverted — but
    /// <strong>not drawing it is equally wrong</strong>, and cost seven documents their entire text.
    /// <c>TextWrap.Through</c> is what says both at once: the frame is placed and painted, and no line
    /// makes room for it.
    /// </para>
    /// <para>
    /// A shape with no size is skipped rather than drawn at nothing: VML writes <c>width:0</c> for the
    /// bare connector lines these templates use as leader rules, and a zero-area frame has no text to
    /// draw and nothing to paint.
    /// </para>
    /// </remarks>
    private static PageFrame? Floating(
        XElement shape,
        Dictionary<string, string> style,
        XElement element,
        int anchorOffset,
        DocxPictures? pictures,
        Func<XElement, IReadOnlyList<PageBlock>>? content,
        Func<string?, OpenTypeFace?>? typeface = null)
    {
        // A `v:line` states its box in `from`/`to` and its `style` rectangle is not read at all —
        // see `LineOf`. It is taken first for that reason: a line that also states a `style`
        // width would otherwise be placed by the half the reference ignores.
        VmlLine? segment = LineOf(shape);

        Length across, down;
        if (segment is { } drawn)
        {
            (across, down) = (drawn.Width, drawn.Height);
        }
        else
        {
            if ((style.TryGetValue("width", out string? w) ? Css(w) : null) is not { } wide) return null;
            if ((style.TryGetValue("height", out string? h) ? Css(h) : null) is not { } tall) return null;
            (across, down) = (wide, tall);
        }

        // A straight connector states one extent as zero — `width:0;height:12.75pt` is how VML
        // writes a vertical rule — and is the one shape with no area that still draws something.
        // A `v:line` is the other: every horizontal rule in a header is `to="468pt,15.85pt"`
        // against a `from` at the same height, so its box is exactly as tall as nothing.
        bool rule = IsStraightConnector(shape) || segment is not null;
        if (across < Length.Zero || down < Length.Zero) return null;
        if (!rule && (across <= Length.Zero || down <= Length.Zero)) return null;

        XElement? box = TextBox(shape);
        FramePicture picture = box is null && pictures is not null
            ? pictures.ReadVml(shape)
            : FramePicture.None;

        Length x = segment is { } placed
            ? placed.Left
            : (style.TryGetValue("margin-left", out string? ml) ? Css(ml) : null) ?? Length.Zero;
        Length y = segment is { } put
            ? put.Top
            : (style.TryGetValue("margin-top", out string? mt) ? Css(mt) : null) ?? Length.Zero;

        string? preset = PresetOf(shape, element);
        VmlPaint paint = PaintOf(shape, style, preset);

        // Which diagonal of the box is drawn comes from the endpoints for a `v:line` and from
        // `style:flip` for everything else, so the two cannot share `IsMirrored`.
        if (segment is { } sense) paint = paint with { IsLineMirrored = sense.Mirrored };
        VmlLayer layer = LayerOf(style);
        VmlFontwork warp = DocxVmlFontwork.Read(
            shape, ShapeTypeOf(shape, element), new DocSize(across, down), typeface);

        if (warp.Outline is not null)
        {
            (across, down) = (warp.Box.Width, warp.Box.Height);
            paint = FontworkPaint(shape, style);
        }

        return new PageFrame
        {
            Size = new DocSize(across, down),
            Anchor = FrameAnchor.Paragraph,
            AnchorOffset = anchorOffset,

            // No line makes room for a floating shape. See the remarks; this is the half of the rule
            // the reverted attempt got right.
            Wrap = TextWrap.Through,

            HorizontalOrigin = HorizontalOriginOf(style),
            HorizontalAlignment = HorizontalAlignmentOf(style),
            HorizontalOffset = x,
            VerticalOrigin = VerticalOriginOf(style),
            VerticalAlignment = VerticalAlignmentOf(style),
            VerticalOffset = y,
            BehindText = layer.BehindText,
            ZOrder = layer.ZOrder,
            FillOutline = warp.Outline,
            StrokeOutline = warp.Outline,
            RotationDegrees = Rotation(style),
            IsImage = box is null && warp.Outline is null,
            Image = picture.Raster,
            Crop = picture.Crop,
            Vector = picture.Vector,
            Fill = paint.Fill,
            BorderColour = paint.Line,
            BorderWidth = paint.Width,
            Preset = preset,
            IsLine = paint.IsLine,
            IsLineMirrored = paint.IsLineMirrored,
            HeadEnd = paint.HeadEnd,
            TailEnd = paint.TailEnd,
            Blocks = box is not null && content is not null ? content(box) : [],
            Padding = box is null ? default : Insets(box),
            HasFixedHeight = box is not null && !GrowsWithText(shape, box),
        };
    }

    /// <summary>How a VML shape is painted: its area, its outline, and what shape that outline is.</summary>
    /// <param name="Fill">The area colour, or null to paint none.</param>
    /// <param name="Line">The outline colour, or null to stroke nothing.</param>
    /// <param name="Width">How thick the outline is.</param>
    /// <param name="IsLine">Whether the outline is the rectangle's diagonal rather than its four sides.</param>
    /// <param name="IsLineMirrored">Which diagonal, when it is one.</param>
    private readonly record struct VmlPaint(
        Colour? Fill, Colour? Line, Length Width, bool IsLine, bool IsLineMirrored)
    {
        public static readonly VmlPaint None = new(null, null, Length.Zero, false, false);

        /// <summary>The marker the outline carries at its first point, or none.</summary>
        public LineEnd HeadEnd { get; init; }

        /// <summary>The one at its last, or none.</summary>
        public LineEnd TailEnd { get; init; }
    }

    /// <summary>
    /// The fill and outline a VML shape states, or nothing when it states none this draws.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>We drew neither on any VML shape until this existed</strong>, which is a defect no
    /// column of the gate can see. Measured on the five Work Breakdown Structure templates before
    /// the change: <c>068</c>'s reference emits 41 <c>f*</c> fills and 36 strokes and ours emits
    /// <b>zero of each</b>, while placing all 41 labels in the right places — and a blind reviewer
    /// given only the rendered pair, knowing nothing about the markup, reported exactly that:
    /// <em>"the reference draws pale-green filled boxes with green borders around every label;
    /// ours draws nothing — bare text on white."</em>
    /// </para>
    /// <para>
    /// <strong>A theme-indexed VML colour resolves to the literal RGB beside the index, and the
    /// index is never consulted.</strong> <c>fillcolor="#e2efd9 [665]"</c> is <c>#E2EFD9</c>:
    /// <c>ConversionHelper::decodeColor</c> splits the value at its space and returns on a
    /// seven-character <c>#RRGGBB</c> (<c>oox/source/vml/vmlformatting.cxx:252-257</c>) long
    /// before the palette branch at line 282. Confirmed twice in the reference's own content
    /// stream — <c>068</c> draws 41 fills at <c>#E2EFD9</c>, <c>069</c> draws 22 at
    /// <c>#F2F2F2</c> from <c>fillcolor="#f2f2f2 [3052]"</c>.
    /// </para>
    /// <para>
    /// <strong>Nothing is defaulted.</strong> LibreOffice gives an unstated fill white and an
    /// unstated stroke black (<c>FillModel::pushToPropMap</c>, <c>StrokeModel::pushToPropMap</c>,
    /// the same file). Reproducing that here would put a white fill and a black box around all 37
    /// <c>#_x0000_t75</c> picture shapes in the words corpus, none of which states either — so
    /// this reads what the shape states and never invents ink, which is the rule
    /// <see cref="DocxFrames"/> already applies to a DrawingML <c>a:fillRef</c>.
    /// </para>
    /// <para>
    /// <strong>Only two geometries are painted here, because only two of them are the rectangle we
    /// would draw.</strong> A <c>v:rect</c> and a <c>v:roundrect</c> are; a straight connector is
    /// its box's diagonal, which is what <see cref="Layout.PageFrame.IsLine"/> already means. A
    /// <c>#_x0000_t136</c> WordArt states a <c>fillcolor</c> that fills glyph outlines and a
    /// <c>#_x0000_t15</c> a pentagon, and filling their rectangles would be a confident wrong
    /// answer; the corpus holds 15 and 3 of them.
    /// </para>
    /// <para>
    /// <strong>The WordArt half of that has been answered rather than reversed.</strong> The rule
    /// stands — a rectangle is still not painted for one — and the fifteen <c>#_x0000_t136</c>
    /// shapes now draw the thing they actually are: <see cref="DocxVmlFontwork"/> builds their glyph
    /// outlines and <see cref="FontworkPaint"/> paints <em>those</em> with the <c>fillcolor</c>. A
    /// <c>#_x0000_t15</c> pentagon is still unpainted, and still for the same reason.
    /// </para>
    /// <para>
    /// <strong><c>strokeweight</c> is honoured and its absence is a hairline</strong>, read off
    /// the 300 dpi reference raster rather than off the <c>w</c> operator — LibreOffice's export
    /// writes a single <c>0.1 w</c> for the whole page, which is not the drawn width. A
    /// <c>v:rect</c> border stating no weight comes out one device pixel; a connector stating
    /// <c>strokeweight="1pt"</c> comes out 4 px at 300 dpi, which is 0.96 pt.
    /// </para>
    /// </remarks>
    /// <summary>
    /// A <c>v:fill/@opacity</c> as a fraction of one, or null when the element states none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Three spellings and all three are in the wild: <c>26214f</c> is VML's 16.16 fixed point, so
    /// 26214/65536 = 0.4; <c>40%</c> is a percentage; <c>.4</c> is a plain fraction.
    /// <c>ConversionHelper::decodePercent</c> reads all three, and the <c>f</c> suffix is the one a
    /// reader misses — the corpus's 48 opacity attributes are every one of them in that form, so
    /// reading it as a plain number gives 26214 rather than 0.4.
    /// </para>
    /// <para>
    /// <strong>This was read by nothing at all, and it is not only a matter of how the box is
    /// painted.</strong> It is the term that decides whether the text inside the box comes out
    /// black or white — see <see cref="Layout.AutomaticColour"/>, and
    /// <c>069_Work_Breakdown_Structure_Template_Professional_Format</c>, whose
    /// <c>fillcolor="#8496b0"</c> is dark and whose <c>opacity="26214f"</c> makes it bright.
    /// </para>
    /// </remarks>
    /// <param name="value">The attribute's text, or null.</param>
    private static double? Opacity(string? value)
    {
        if (value is not { Length: > 0 }) return null;

        string text = value.Trim();
        double scale = 1.0;

        if (text.EndsWith('f'))
        {
            text = text[..^1];
            scale = 1.0 / 65536.0;
        }
        else if (text.EndsWith('%'))
        {
            text = text[..^1];
            scale = 1.0 / 100.0;
        }

        if (!double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture, out double raw))
            return null;

        double fraction = raw * scale;
        return double.IsFinite(fraction) ? Math.Clamp(fraction, 0.0, 1.0) : null;
    }

    /// <summary>
    /// The DrawingML preset a VML shape type names, or null for one this does not model.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A <c>v:shape</c> names its geometry by number — <c>type="#_x0000_t15"</c>, or an
    /// <c>o:spt</c> on the <c>v:shapetype</c> it points at — and the number is Escher's
    /// <c>MSO_SPT</c>. The names here are the reference's own: <c>GETVMLShapeType</c>
    /// (<c>filter/source/msfilter/util.cxx</c>:1072-1280) is the same table read the other way
    /// round, from a DrawingML preset name to an <c>MSO_SPT</c>, and the numbering is the
    /// <c>MSO_SPT</c> enum in <c>include/svx/msdffdef.hxx</c>:274.
    /// </para>
    /// <para>
    /// <strong>Without this every one of them was drawn as its bounding rectangle, and unpainted
    /// at that.</strong> <see cref="PaintOf"/> painted only a <c>v:rect</c> and a
    /// <c>v:roundrect</c>, on the stated grounds that filling a pentagon's box would be a
    /// confident wrong answer — which was right, and left the pentagon itself undrawn. Naming the
    /// geometry answers both halves at once: <see cref="Layout.PageFrame.Preset"/> already routes
    /// a name through <c>CustomShapeGeometry.Preset</c>, which holds all 187 of them.
    /// </para>
    /// <para>
    /// <strong>Censused over the corpus's 271 DOCX</strong>, counting only <c>v:shape</c> that
    /// state a <c>fillcolor</c> or a <c>strokecolor</c> they do not then switch off: <b>202 uses in
    /// 27 documents</b> are <c>t202</c>, the text box, which is a plain rectangle; <b>49 in 5</b>
    /// are <c>t15</c>, the right-pointing pentagon; <b>30 in 2</b> are <c>t13</c>, the right arrow;
    /// and the rest are the two dozen entries below, none more than 23. <c>t32</c> is excluded
    /// because <see cref="IsStraightConnector"/> already draws it as a rule, and <c>t136</c>
    /// because <see cref="DocxVmlFontwork"/> already draws its glyph outlines.
    /// </para>
    /// <para>
    /// Nothing is defaulted by naming a geometry: the shape is still painted only with the fill and
    /// the stroke it states, which is the rule <see cref="PaintOf"/> has always followed.
    /// </para>
    /// </remarks>
    private static string? PresetOf(XElement shape, XElement scope)
    {
        if (shape.Name.LocalName is not "shape") return null;
        if (Spt(shape, ShapeTypeOf(shape, scope)) is not { } spt) return null;

        return spt switch
        {
            1 or 202 => "rect",
            2 => "roundRect",
            3 => "ellipse",
            4 => "diamond",
            5 => "triangle",
            6 => "rtTriangle",
            7 => "parallelogram",
            8 => "trapezoid",
            9 => "hexagon",
            10 => "octagon",
            11 => "plus",
            12 => "star5",
            13 => "rightArrow",
            15 => "homePlate",
            16 => "cube",
            21 => "plaque",
            22 => "can",
            23 => "donut",
            34 => "bentConnector3",
            38 => "curvedConnector3",
            55 => "chevron",
            62 => "wedgeRoundRectCallout",
            66 => "leftArrow",
            67 => "downArrow",
            68 => "upArrow",
            93 => "stripedRightArrow",
            102 => "curvedRightArrow",
            103 => "curvedLeftArrow",
            104 => "curvedUpArrow",
            105 => "curvedDownArrow",
            109 => "flowChartProcess",
            110 => "flowChartDecision",
            111 => "flowChartInputOutput",
            112 => "flowChartPredefinedProcess",
            113 => "flowChartInternalStorage",
            114 => "flowChartDocument",
            116 => "flowChartTerminator",
            117 => "flowChartPreparation",
            118 => "flowChartManualInput",
            119 => "flowChartManualOperation",
            120 => "flowChartConnector",
            125 => "flowChartCollate",
            _ => null,
        };
    }

    /// <summary>The shape's <c>MSO_SPT</c>, from its own <c>o:spt</c>, its type's, or its id.</summary>
    /// <remarks>
    /// The same three places <see cref="DocxVmlFontwork"/> looks, and in the same order: Word
    /// writes the number on the <c>v:shapetype</c> and refers to it as <c>#_x0000_t15</c>, so the
    /// reference is the reliable one and the attribute is the exception.
    /// </remarks>
    private static int? Spt(XElement shape, XElement? shapeType)
    {
        XName name = XName.Get("spt", OoxmlNamespaces.VmlOffice);
        foreach (XElement? carrier in new[] { shape, shapeType })
        {
            if (carrier?.Attribute(name)?.Value is not { Length: > 0 } stated) continue;
            if (double.TryParse(
                    stated, NumberStyles.Float, CultureInfo.InvariantCulture, out double spt))
            {
                return (int)spt;
            }
        }

        const string Prefix = "#_x0000_t";
        return shape.Attribute("type")?.Value is { } reference
               && reference.StartsWith(Prefix, StringComparison.Ordinal)
               && int.TryParse(
                   reference.AsSpan(Prefix.Length), NumberStyles.Integer,
                   CultureInfo.InvariantCulture, out int number)
            ? number
            : null;
    }

    private static VmlPaint PaintOf(XElement shape, Dictionary<string, string> style, string? preset)
    {
        bool box = shape.Name.LocalName is "rect" or "roundrect" || preset is not null;
        bool rule = IsStraightConnector(shape) || IsLine(shape);
        if (!box && !rule) return VmlPaint.None;

        XElement? fillElement = shape.Element(XName.Get("fill", OoxmlNamespaces.Vml));
        XElement? strokeElement = shape.Element(XName.Get("stroke", OoxmlNamespaces.Vml));

        Colour? fill = box && On(shape.Attribute("filled")?.Value)
                       && On(fillElement?.Attribute("on")?.Value)
            ? VmlColour(shape.Attribute("fillcolor")?.Value ?? fillElement?.Attribute("color")?.Value)
            : null;

        if (fill is { } opaque && Opacity(fillElement?.Attribute("opacity")?.Value) is { } opacity)
        {
            fill = opaque.WithAlpha((byte)Math.Clamp(Math.Floor((opacity * 255.0) + 0.5), 0.0, 255.0));
        }

        Colour? line = On(shape.Attribute("stroked")?.Value)
                       && On(strokeElement?.Attribute("on")?.Value)
            ? VmlColour(
                shape.Attribute("strokecolor")?.Value ?? strokeElement?.Attribute("color")?.Value)
            : null;

        if (line is null) return new VmlPaint(fill, null, Length.Zero, false, false);

        Length width =
            Css(shape.Attribute("strokeweight")?.Value
                ?? strokeElement?.Attribute("weight")?.Value
                ?? string.Empty)
            ?? Hairline;

        return new VmlPaint(fill, line, width <= Length.Zero ? Hairline : width, rule,
            IsMirrored(style))
        {
            HeadEnd = Arrow(strokeElement, "start"),
            TailEnd = Arrow(strokeElement, "end"),
        };
    }

    /// <summary>
    /// The marker one end of a <c>v:stroke</c> carries, in the DrawingML vocabulary
    /// <see cref="LineEnds"/> speaks.
    /// </summary>
    /// <remarks>
    /// <para>
    /// VML names five arrow shapes and DrawingML names the same five differently, so the reference
    /// translates rather than reads: <c>lclGetDmlArrowType</c>
    /// (<c>oox/source/vml/vmlformatting.cxx</c>:599-611) maps <c>block</c> to <c>triangle</c>,
    /// <c>classic</c> to <c>stealth</c> and <c>open</c> to <c>arrow</c>, leaving <c>diamond</c> and
    /// <c>oval</c> alone; the two size words map <c>narrow</c>/<c>short</c> to <c>sm</c> and
    /// <c>wide</c>/<c>long</c> to <c>lg</c> (<c>:613-632</c>). An unknown or absent type is
    /// <c>none</c>, which draws nothing.
    /// </para>
    /// <para>
    /// This is on the <c>v:stroke</c> child and never on the shape, unlike every other outline
    /// attribute here, each of which has a shape-level spelling as well.
    /// </para>
    /// </remarks>
    /// <param name="stroke">The shape's <c>v:stroke</c>, or null when it carries none.</param>
    /// <param name="which">Either <c>start</c> or <c>end</c>.</param>
    private static LineEnd Arrow(XElement? stroke, string which)
    {
        if (stroke?.Attribute(which + "arrow")?.Value is not { Length: > 0 } arrow) return default;

        string? type = arrow switch
        {
            "block" => "triangle",
            "classic" => "stealth",
            "diamond" => "diamond",
            "oval" => "oval",
            "open" => "arrow",
            _ => null,
        };

        return type is null
            ? default
            : new LineEnd(type, Size(which + "arrowwidth"), Size(which + "arrowlength"));

        string? Size(string name) => stroke.Attribute(name)?.Value switch
        {
            "narrow" or "short" => "sm",
            "medium" => "med",
            "wide" or "long" => "lg",
            _ => null,
        };
    }

    /// <summary>The thinnest line LibreOffice's PDF export writes, which is what it draws a VML
    /// outline stating no <c>strokeweight</c> as.</summary>
    private static readonly Length Hairline = Length.FromPoints(0.1);

    /// <summary>A <c>v:line</c>, which states its extent in <c>from</c>/<c>to</c> and nowhere else.</summary>
    private static bool IsLine(XElement shape) => shape.Name.LocalName is "line";

    /// <summary>The box a <c>v:line</c>'s two endpoints describe, and which diagonal of it to draw.</summary>
    /// <param name="Left">The box's left edge, from the shape's own origin.</param>
    /// <param name="Top">Its top edge.</param>
    /// <param name="Width">Its width, never negative.</param>
    /// <param name="Height">Its height, never negative.</param>
    /// <param name="Mirrored">True when the line runs bottom-left to top-right.</param>
    private readonly record struct VmlLine(
        Length Left, Length Top, Length Width, Length Height, bool Mirrored);

    /// <summary>
    /// Where a top-level <c>v:line</c> is drawn, or null when it states no usable pair.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A <c>v:line</c>'s <c>style</c> rectangle is not read at all.</strong>
    /// <c>LineShape::getAbsRectangle</c> (<c>oox/source/vml/vmlshape.cxx</c>) builds the shape's
    /// rectangle entirely out of <c>from</c> and <c>to</c> —
    /// <c>X = from.x, Y = from.y, Width = to.x − X, Height = to.y − Y</c> — so
    /// <c>style:left</c>, <c>top</c>, <c>width</c> and <c>height</c> are all ignored where those
    /// two attributes are present. Confirmed at 26.2.4.2 on four one-attribute probes: adding
    /// <c>left:100pt;top:50pt</c>, <c>width:200pt;height:100pt</c>, or both, leaves the drawn
    /// line exactly where the bare shape put it.
    /// </para>
    /// <para>
    /// <strong>A bare number is a pixel at 96 dpi, not a point.</strong> The conversion is
    /// <c>decodeMeasureToHmm(…, bDefaultAsPixel: true)</c>, and the probe stating
    /// <c>to="96,48"</c> is drawn 72 pt across and 36 pt down. That is why this parses the pair
    /// itself rather than through <see cref="Css"/>, whose bare number is a point and which
    /// serves the <c>style</c> properties this round did not measure.
    /// </para>
    /// <para>
    /// <strong>A line whose x decreases is not drawn, and one whose y decreases is.</strong>
    /// `getAbsRectangle` subtracts without normalising, so a backwards line has a negative
    /// width, and 26.2.4.2 draws nothing for it: the probes stating
    /// <c>from="144pt,0" to="0,0"</c> and <c>from="144pt,72pt" to="0,0"</c> both come back with
    /// no ink at all, while <c>from="0,72pt" to="144pt,0"</c> is drawn as the box's
    /// anti-diagonal. Normalising the two points into a rectangle would draw three lines where
    /// the reference draws two, so the asymmetry is reproduced rather than tidied away.
    /// </para>
    /// </remarks>
    private static VmlLine? LineOf(XElement shape)
    {
        if (!IsLine(shape)) return null;
        if (Pair(shape.Attribute("from")?.Value, VmlMeasure) is not ({ } x0, { } y0)) return null;
        if (Pair(shape.Attribute("to")?.Value, VmlMeasure) is not ({ } x1, { } y1)) return null;

        if (x1 < x0) return null;

        return new VmlLine(
            x0, y1 < y0 ? y1 : y0, x1 - x0, y1 < y0 ? y0 - y1 : y1 - y0, y1 < y0);
    }

    /// <summary>
    /// The box a <c>v:line</c> inside a <c>v:group</c> describes, in the group's own coordinates.
    /// </summary>
    /// <remarks>
    /// <c>LineShape::getRelRectangle</c> reads each token with <c>o3tl::toInt32</c>, which takes
    /// the leading integer and <strong>drops any unit suffix</strong> — so inside a group
    /// <c>100pt</c> is a hundred coordinate units and not a length at all. Measured at 26.2.4.2
    /// on a 200 × 100 pt group with <c>coordsize="1000,1000"</c>: <c>to="1000,1000"</c> spans the
    /// whole group, <c>from="250,250" to="750,250"</c> spans its middle half, and
    /// <c>to="100pt,50pt"</c> is drawn 20 × 5 pt, which is a hundred units by fifty.
    /// </remarks>
    private static (double Left, double Top, double Width, double Height, bool Mirrored)? RelativeLineOf(
        XElement shape)
    {
        if (!IsLine(shape)) return null;

        string[] from = (shape.Attribute("from")?.Value ?? string.Empty).Split(',');
        string[] to = (shape.Attribute("to")?.Value ?? string.Empty).Split(',');
        if (from.Length != 2 || to.Length != 2) return null;
        if (LeadingInteger(from[0]) is not { } x0 || LeadingInteger(from[1]) is not { } y0) return null;
        if (LeadingInteger(to[0]) is not { } x1 || LeadingInteger(to[1]) is not { } y1) return null;

        if (x1 < x0) return null;

        return (x0, Math.Min(y0, y1), x1 - x0, Math.Abs(y1 - y0), y1 < y0);
    }

    /// <summary>One endpoint's coordinate, where a bare number is a pixel at 96 dpi.</summary>
    private static Length? VmlMeasure(string text)
    {
        string value = text.Trim();
        if (value.Length == 0) return null;

        return Number(value) is { } pixels ? Length.FromEmu((long)Math.Round(pixels * (914400.0 / 96.0))) : Css(value);
    }

    /// <summary>
    /// True for the straight connector VML writes a rule as.
    /// </summary>
    /// <remarks>
    /// <c>o:connectortype</c>, or a <c>type</c> naming a <c>v:shapetype</c> whose <c>o:spt</c> is
    /// 32 — the preset whose path is the box's own diagonal, <c>m,l21600,21600e</c>. This is the
    /// one shape allowed through the zero-extent check, because a vertical rule is written
    /// <c>width:0;height:12.75pt</c> and there are 87 of them in the words corpus.
    /// </remarks>
    private static bool IsStraightConnector(XElement shape)
    {
        if (shape.Name.LocalName is not "shape") return false;
        if (shape.Attribute(XName.Get("connectortype", OoxmlNamespaces.VmlOffice)) is not null) return true;

        string? type = shape.Attribute("type")?.Value?.TrimStart('#');
        if (string.IsNullOrEmpty(type)) return false;

        if (type is "_x0000_t32") return true;

        return shape.Document?.Descendants(XName.Get("shapetype", OoxmlNamespaces.Vml))
            .FirstOrDefault(candidate => candidate.Attribute("id")?.Value == type)
            ?.Attribute(XName.Get("spt", OoxmlNamespaces.VmlOffice))?.Value is "32";
    }

    /// <summary>
    /// The fill and stroke of a VML WordArt shape, which are its glyphs' own ink.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Separate from <see cref="PaintOf"/> because the rule is the opposite one. That method paints
    /// only a <c>v:rect</c> and a <c>v:roundrect</c> and defaults nothing, so that a picture shape
    /// stating neither is not given a white box and a black border it never asked for. A Fontwork
    /// has no box: the fill and the stroke <em>are</em> the letters, and LibreOffice's own defaults
    /// therefore apply — <c>FillModel::pushToPropMap</c> gives an unstated fill white and
    /// <c>StrokeModel::pushToPropMap</c> gives an unstated stroke black
    /// (<c>oox/source/vml/vmlformatting.cxx</c>).
    /// </para>
    /// <para>
    /// Nothing in the corpus measures those defaults: all 15 <c>#_x0000_t136</c> shapes state
    /// <c>fillcolor</c> and <c>stroked="f"</c> explicitly, and 14 of them state
    /// <c>&lt;v:fill opacity=".5"/&gt;</c> beside it.
    /// </para>
    /// </remarks>
    private static VmlPaint FontworkPaint(XElement shape, Dictionary<string, string> style)
    {
        XElement? fillElement = shape.Element(XName.Get("fill", OoxmlNamespaces.Vml));
        XElement? strokeElement = shape.Element(XName.Get("stroke", OoxmlNamespaces.Vml));

        Colour? fill = On(shape.Attribute("filled")?.Value)
                       && On(fillElement?.Attribute("on")?.Value)
            ? VmlColour(shape.Attribute("fillcolor")?.Value ?? fillElement?.Attribute("color")?.Value)
              ?? Colour.FromRgb(0xFFFFFF)
            : null;

        if (fill is { } opaque && Opacity(fillElement?.Attribute("opacity")?.Value) is { } opacity)
        {
            fill = opaque.WithAlpha((byte)Math.Clamp(Math.Floor((opacity * 255.0) + 0.5), 0.0, 255.0));
        }

        Colour? line = On(shape.Attribute("stroked")?.Value)
                       && On(strokeElement?.Attribute("on")?.Value)
            ? VmlColour(shape.Attribute("strokecolor")?.Value ?? strokeElement?.Attribute("color")?.Value)
              ?? Colour.FromRgb(0x000000)
            : null;

        if (line is null) return new VmlPaint(fill, null, Length.Zero, false, false);

        Length width =
            Css(shape.Attribute("strokeweight")?.Value
                ?? strokeElement?.Attribute("weight")?.Value
                ?? string.Empty)
            ?? Hairline;

        return new VmlPaint(fill, line, width <= Length.Zero ? Hairline : width, false, IsMirrored(style));
    }

    /// <summary>
    /// How far a floating VML shape is turned about its own centre, clockwise, in degrees.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ConversionHelper::decodeRotation</c> (<c>oox/source/vml/vmlformatting.cxx</c>): a bare
    /// number is degrees and an <c>fd</c> suffix is 1/65536 of one, which is how Word writes an
    /// angle a user dragged rather than typed.
    /// </para>
    /// <para>
    /// <strong>Read for a WordArt shape and for nothing else, deliberately.</strong> The corpus's
    /// words track states <c>rotation</c> on <b>347</b> VML shapes across <b>34</b> documents —
    /// genogram connectors, unit-circle labels, storyboard arrows — and turning all of them is a
    /// change with its own reach and its own regression risk. Fifteen of the 347 are the
    /// <c>#_x0000_t136</c> watermarks, every one of them <c>rotation:315</c>, and a watermark drawn
    /// flat instead of diagonally is not the shape the reference draws at all. So the rest wait for
    /// a round that measures them.
    /// </para>
    /// </remarks>
    private static double Rotation(Dictionary<string, string> style)
    {
        if (!style.TryGetValue("rotation", out string? stated)) return 0;

        string text = stated.Trim();
        double scale = 1.0;

        if (text.EndsWith("fd", StringComparison.OrdinalIgnoreCase))
        {
            text = text[..^2];
            scale = 1.0 / 65536.0;
        }

        return double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture, out double degrees)
            ? degrees * scale
            : 0;
    }

    /// <summary>Where <c>mso-position-horizontal</c> puts the shape, or an offset when it says nothing.</summary>
    /// <remarks>
    /// <c>lcl_SetAnchorType</c>, <c>oox/source/vml/vmlshape.cxx:661-680</c>. <c>inside</c> and
    /// <c>outside</c> also set a page toggle, which this does not carry; they appear on no words
    /// document.
    /// </remarks>
    private static FrameHorizontalAlignment HorizontalAlignmentOf(Dictionary<string, string> style)
        => style.GetValueOrDefault("mso-position-horizontal") switch
        {
            "center" => FrameHorizontalAlignment.Centre,
            "left" => FrameHorizontalAlignment.Left,
            "right" => FrameHorizontalAlignment.Right,
            "inside" => FrameHorizontalAlignment.Left,
            "outside" => FrameHorizontalAlignment.Right,
            _ => FrameHorizontalAlignment.Offset,
        };

    /// <summary>Where <c>mso-position-vertical</c> puts the shape.</summary>
    /// <remarks><c>oox/source/vml/vmlshape.cxx:700-710</c>.</remarks>
    private static FrameVerticalAlignment VerticalAlignmentOf(Dictionary<string, string> style)
        => style.GetValueOrDefault("mso-position-vertical") switch
        {
            "center" => FrameVerticalAlignment.Middle,
            "top" or "inside" => FrameVerticalAlignment.Top,
            "bottom" or "outside" => FrameVerticalAlignment.Bottom,
            _ => FrameVerticalAlignment.Offset,
        };

    /// <summary>Which diagonal a connector runs along.</summary>
    /// <remarks>
    /// The preset's path runs from the box's top-left to its bottom-right. <c>flip:x</c> and
    /// <c>flip:y</c> each swap that for the other diagonal, and stating both swaps it back.
    /// </remarks>
    private static bool IsMirrored(Dictionary<string, string> style)
    {
        if (!style.TryGetValue("flip", out string? flip)) return false;

        bool x = flip.Contains('x', StringComparison.OrdinalIgnoreCase);
        bool y = flip.Contains('y', StringComparison.OrdinalIgnoreCase);
        return x ^ y;
    }

    /// <summary>A VML boolean, whose absence means on.</summary>
    /// <remarks><c>filled="f"</c> and <c>stroked="f"</c> are how a shape says no.</remarks>
    private static bool On(string? value)
        => value is null
           || !(value.Equals("f", StringComparison.OrdinalIgnoreCase)
                || value.Equals("false", StringComparison.OrdinalIgnoreCase)
                || value == "0");

    /// <summary>
    /// A VML colour value, or null when it names nothing this understands.
    /// </summary>
    /// <remarks>
    /// The order is <c>ConversionHelper::decodeColor</c>'s: split the value at its first space so
    /// that a trailing <c>[3209]</c> palette index is set aside, then <c>#RRGGBB</c>, then the
    /// three-digit <c>#RGB</c>, then a preset name. A bare palette index with no colour beside it
    /// is not resolved — that would need the theme's palette, and no words document writes one.
    /// An unrecognised name yields null rather than black, so an unknown value draws nothing
    /// instead of drawing the wrong thing.
    /// </remarks>
    private static Colour? VmlColour(string? value)
    {
        if (value is null) return null;

        string text = value.Trim();
        int space = text.IndexOf(' ');
        if (space > 0) text = text[..space];
        if (text.Length == 0) return null;

        if (text[0] == '#')
        {
            ReadOnlySpan<char> digits = text.AsSpan(1);
            if (digits.Length == 6
                && uint.TryParse(digits, NumberStyles.HexNumber, CultureInfo.InvariantCulture,
                                 out uint rgb))
            {
                return Colour.FromRgb(rgb);
            }

            if (digits.Length == 3
                && uint.TryParse(digits, NumberStyles.HexNumber, CultureInfo.InvariantCulture,
                                 out uint packed))
            {
                uint r = (packed >> 8) & 0xF;
                uint g = (packed >> 4) & 0xF;
                uint b = packed & 0xF;
                return Colour.FromRgb((r * 0x11 << 16) | (g * 0x11 << 8) | (b * 0x11));
            }

            return null;
        }

        if (Presets.TryGetValue(text, out uint preset)) return Colour.FromRgb(preset);

        return SystemColours.TryGetValue(text, out uint system) ? Colour.FromRgb(system) : null;
    }

    /// <summary>
    /// The system colour names, which a VML colour may use instead of a preset.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ConversionHelper::decodeColor</c> (<c>oox/source/vml/vmlformatting.cxx</c>) tries
    /// <c>Color::getVmlPresetColor</c> first and falls through to
    /// <c>GraphicHelper::getSystemColor</c>, whose palette is a fixed table of Windows XP's
    /// defaults rather than anything the running desktop decides
    /// (<c>oox/source/helper/graphichelper.cxx</c>:63-100). So these are constants, and a
    /// reader that treats them as "the desktop's theme" and declines to resolve them draws
    /// nothing.
    /// </para>
    /// <para>
    /// <strong>Drawing nothing is what this cost.</strong> <c>JEMIT_Template.docx</c>'s footer
    /// rule is the corpus's only <c>strokecolor="windowText"</c> — against 1015 <c>black</c>, 78
    /// <c>red</c>, 41 <c>white</c>, 34 <c>blue</c>, 4 <c>gray</c> and 3 <c>none</c> — and an
    /// unresolved name leaves <c>PaintOf</c> with no stroke at all, so the shape is built and
    /// painted with nothing. That is the failure mode the preset table's own remark already
    /// warns about: a missing name is invisible.
    /// </para>
    /// </remarks>
    private static readonly Dictionary<string, uint> SystemColours =
        new(StringComparer.OrdinalIgnoreCase)
        {
            ["windowText"] = 0x000000, ["window"] = 0xFFFFFF, ["windowFrame"] = 0x000000,
            ["btnText"] = 0x000000, ["btnFace"] = 0xECE9D8, ["btnShadow"] = 0xACA899,
            ["btnHighlight"] = 0xFFFFFF, ["grayText"] = 0xACA899, ["highlight"] = 0x316AC5,
            ["highlightText"] = 0xFFFFFF, ["infoBk"] = 0xFFFFE1, ["infoText"] = 0x000000,
            ["menu"] = 0xFFFFFF, ["menuText"] = 0x000000, ["scrollBar"] = 0xD4D0C8,
            ["captionText"] = 0xFFFFFF, ["activeBorder"] = 0xD4D0C8,
            ["activeCaption"] = 0x0054E3, ["appWorkspace"] = 0x808080,
            ["background"] = 0x004E98, ["inactiveBorder"] = 0xD4D0C8,
            ["inactiveCaption"] = 0x7A96DF, ["inactiveCaptionText"] = 0xD8E4F8,
        };

    /// <summary>
    /// The VML preset colour names, from <c>Color::getVmlPresetColor</c>'s table.
    /// </summary>
    /// <remarks>
    /// The sixteen HTML 4 names plus the four VML spells differently. Only <c>black</c>,
    /// <c>white</c> and <c>red</c> appear in this corpus — 138, 55 and 78 times — and the rest are
    /// here because the table is small and a missing name draws nothing at all, which is the one
    /// failure mode that is invisible.
    /// </remarks>
    private static readonly Dictionary<string, uint> Presets =
        new(StringComparer.OrdinalIgnoreCase)
        {
            ["black"] = 0x000000, ["silver"] = 0xC0C0C0, ["gray"] = 0x808080, ["grey"] = 0x808080,
            ["white"] = 0xFFFFFF, ["maroon"] = 0x800000, ["red"] = 0xFF0000, ["purple"] = 0x800080,
            ["fuchsia"] = 0xFF00FF, ["green"] = 0x008000, ["lime"] = 0x00FF00, ["olive"] = 0x808000,
            ["yellow"] = 0xFFFF00, ["navy"] = 0x000080, ["blue"] = 0x0000FF, ["teal"] = 0x008080,
            ["aqua"] = 0x00FFFF, ["cyan"] = 0x00FFFF, ["magenta"] = 0xFF00FF, ["orange"] = 0xFFA500,
        };

    /// <summary>The <c>w:txbxContent</c> a VML shape carries, or null when it carries none.</summary>
    private static XElement? TextBox(XElement shape)
        => shape.Descendants(Word.Name("txbxContent")).FirstOrDefault();

    /// <summary>The <c>v:textbox</c> a <c>w:txbxContent</c> sits in, or null when it sits in none.</summary>
    /// <remarks>
    /// The content is the element the readers above carry, and the attributes that decide the box's
    /// insets are on the <c>v:textbox</c> around it. Matched on the local name because a shape inside a
    /// <c>mc:Fallback</c> is written in whichever VML namespace its producer chose.
    /// </remarks>
    private static XElement? Enclosure(XElement content)
        => content.Ancestors().FirstOrDefault(element => element.Name.LocalName == "textbox");

    /// <summary>
    /// The distance between a VML text box's edge and its text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>v:textbox/@inset</c> is a comma list of up to four CSS lengths — left, top, right, bottom —
    /// and a value omitted from it takes that <em>side's</em> default rather than nothing:
    /// <c>TextBoxContext</c> reads each in turn and substitutes <c>0.1in</c> across and <c>0.05in</c>
    /// down for an empty one (<c>oox/source/vml/vmltextboxcontext.cxx</c>:185-211). So an absent
    /// attribute and a stated <c>0,0,0,0</c> are opposite answers, and <c>inset="4mm"</c> moves the
    /// left side alone.
    /// </para>
    /// <para>
    /// <strong><c>insetmode="auto"</c> takes none of that.</strong> The whole block is guarded on the
    /// mode not being <c>auto</c>, so such a box leaves <c>borderDistanceSet</c> false and
    /// <c>vmlshape.cxx</c>:778-784 sets no distance at all — the frame keeps Writer's own
    /// <c>Frame</c> style, whose padding is 1.5 mm. No corpus document states the mode, so it is
    /// modelled for the rule rather than for its reach.
    /// </para>
    /// <para>
    /// Confirmed twice: 26.2.4.2's own <c>--convert-to fodt</c> of ten one-attribute fixtures states
    /// the four <c>fo:padding-*</c> this computes, and its rendering of the same ten puts the first
    /// line where they say. <c>probes/vmlinset-r171</c>.
    /// </para>
    /// </remarks>
    /// <param name="content">The <c>w:txbxContent</c> whose enclosing box is read.</param>
    private static Margins Insets(XElement content)
    {
        XElement? box = Enclosure(content);

        if (box?.Attribute("insetmode")?.Value == "auto")
        {
            return new Margins(
                WriterFrameInset, WriterFrameInset, WriterFrameInset, WriterFrameInset);
        }

        string[] stated = (box?.Attribute("inset")?.Value ?? string.Empty).Split(',');

        return new Margins(
            Side(0, DefaultInsetAcross), Side(1, DefaultInsetDown),
            Side(2, DefaultInsetAcross), Side(3, DefaultInsetDown));

        Length Side(int index, Length fallback)
            => index < stated.Length && Css(stated[index]) is { } length ? length : fallback;
    }

    /// <summary>A VML text box's default inset to left and right.</summary>
    private static readonly Length DefaultInsetAcross = Length.FromEmu(91440);

    /// <summary>Its default inset above and below.</summary>
    private static readonly Length DefaultInsetDown = Length.FromEmu(45720);

    /// <summary>What a box asking for the automatic inset gets: Writer's own <c>Frame</c> padding.</summary>
    private static readonly Length WriterFrameInset = Length.FromMm100(150);

    /// <summary>
    /// Whether a VML shape's box grows to hold its text rather than keeping the stated height.
    /// </summary>
    /// <remarks>
    /// <c>mso-fit-shape-to-text</c> is the VML spelling of <c>a:spAutoFit</c>, and it is read from two
    /// styles into one flag: the shape's (<c>oox/source/vml/vmlshapecontext.cxx</c>:551) and the box's
    /// own (<c>vmltextboxcontext.cxx</c>:221-222). Both set <c>mbAutoHeight</c> true on presence
    /// alone, whatever value follows the colon, and that becomes <c>SizeType::MIN</c> against
    /// <c>FIX</c> (<c>vmlshape.cxx</c>:776-777). Measured: the reference exports a fitting box with no
    /// <c>svg:height</c> at all and draws all four of a fixture's lines in a box that holds two.
    /// </remarks>
    /// <param name="shape">The <c>v:shape</c> or its equivalent, whose own style is read too.</param>
    /// <param name="content">The <c>w:txbxContent</c> whose enclosing box is read.</param>
    private static bool GrowsWithText(XElement shape, XElement content)
        => Style(shape).ContainsKey("mso-fit-shape-to-text")
            || (Enclosure(content) is { } box && Style(box).ContainsKey("mso-fit-shape-to-text"));

    /// <summary>Which layer a floating VML shape paints on, and where in that layer's stack.</summary>
    /// <param name="BehindText">True for the hell layer, painted before the text.</param>
    /// <param name="ZOrder">Where in the layer it sits, low to high.</param>
    private readonly record struct VmlLayer(bool BehindText, long ZOrder);

    /// <summary>
    /// The layer and stacking position a VML shape's <c>z-index</c> asks for.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The sign is the layer and the magnitude is the order, and nothing read either.</strong>
    /// <c>oox/source/vml/vmlshapecontext.cxx:536</c> keeps the declaration verbatim and
    /// <c>vmlshape.cxx:408</c> hands it to Writer as <c>VML-Z-ORDER</c>; the mapper then sets the
    /// shape opaque exactly when it is not negative —
    /// <c>xShapePropertySet-&gt;setPropertyValue(PROP_OPAQUE, uno::Any(zOrder &gt;= 0))</c>,
    /// <c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx:5157</c> and <c>:5203</c> — so a
    /// negative <c>z-index</c> is the hell layer and that is how Word writes a watermark.
    /// </para>
    /// <para>
    /// <strong>A shape that declares a <c>z-index</c> outranks every <c>relativeHeight</c>, whatever
    /// the two numbers are.</strong> <c>GraphicZOrderHelper::adjustRelativeHeight</c>
    /// (<c>sw/source/writerfilter/dmapper/GraphicHelpers.cxx:279-330</c>) says it in as many words —
    /// "in general, all z-index-defined shapes appear on top of relativeHeight graphics regardless of
    /// the value" — and implements it by pushing every DrawingML anchor below zero
    /// (<c>GraphicImport.cxx:695</c>) while leaving a <c>z-index</c> alone. The two ranges are
    /// separated here the other way about, by lifting a <c>z-index</c> clear of the whole unsigned
    /// 32-bit range that <c>relativeHeight</c> occupies, which leaves every stored DrawingML order
    /// exactly as it was.
    /// </para>
    /// <para>
    /// A shape that declares no <c>z-index</c> keeps document order: zero, in front of the text, which
    /// is what every VML shape did before this existed.
    /// </para>
    /// </remarks>
    private static VmlLayer LayerOf(Dictionary<string, string> style)
    {
        if (!style.TryGetValue("z-index", out string? text)) return default;

        if (!long.TryParse(text.Trim(), NumberStyles.Integer, CultureInfo.InvariantCulture,
                           out long z))
        {
            return default;
        }

        return new VmlLayer(z < 0, VmlZOrderBase + z);
    }

    /// <summary>
    /// What a VML <c>z-index</c> is lifted by so that it clears every DrawingML <c>relativeHeight</c>.
    /// </summary>
    /// <remarks>
    /// 2^32, one past the top of <c>ST_RelativeHeight</c>'s unsigned 32-bit range. A negative
    /// <c>z-index</c> still lands above the range — the corpus's are around −251 million — which is
    /// right: it shares the hell layer with the <c>behindDoc</c> anchors and LibreOffice puts it above
    /// them there too (<c>GraphicHelpers.cxx:305-315</c> pushes a behind-text <c>relativeHeight</c>
    /// down a further level and leaves a negative <c>z-index</c> where it is).
    /// </remarks>
    private const long VmlZOrderBase = 4294967296L;

    /// <summary>What <c>margin-left</c> is measured from.</summary>
    /// <remarks>
    /// <para>
    /// <c>mso-position-horizontal-relative</c>, whose default is <c>text</c> — the column the anchor
    /// sits in — and not the page. <c>char</c> is the anchor character's own position, which the column
    /// is the nearest origin this model has.
    /// </para>
    /// <para>
    /// VML has its own mapping and it is <em>not</em> the DrawingML one with different spellings:
    /// <c>lcl_SetAnchorType</c> (<c>oox/source/vml/vmlshape.cxx</c>:687-692) pairs
    /// <c>inner-margin-area</c> with <c>right-margin-area</c> under <c>PAGE_RIGHT</c> and
    /// <c>outer-margin-area</c> with <c>left-margin-area</c> under <c>PAGE_LEFT</c> — a crossover
    /// against DrawingML, where <c>insideMargin</c> is the page and <c>outsideMargin</c> is the
    /// column. Measured on a 40 pt band on A4 with 72 pt margins, both installed references identical:
    /// <c>inner-margin-area</c> is drawn at <b>523.25 pt</b> and <c>outer-margin-area</c> at
    /// <b>0.00</b>.
    /// </para>
    /// </remarks>
    private static FrameHorizontalOrigin HorizontalOriginOf(Dictionary<string, string> style)
        => style.GetValueOrDefault("mso-position-horizontal-relative") switch
        {
            "page" => FrameHorizontalOrigin.Page,
            "margin" => FrameHorizontalOrigin.PageMargin,
            "left-margin-area" or "outer-margin-area" => FrameHorizontalOrigin.LeftMarginArea,
            "right-margin-area" or "inner-margin-area" => FrameHorizontalOrigin.RightMarginArea,
            _ => FrameHorizontalOrigin.Column,
        };

    /// <summary>What <c>margin-top</c> is measured from.</summary>
    /// <remarks>
    /// <c>mso-position-vertical-relative</c>. Its default is <c>text</c>, which is the anchor
    /// paragraph — the origin a negative <c>margin-top</c> is measured up from, and these templates
    /// use negative ones freely. The two margin-area spellings are the same two relations DrawingML's
    /// <c>topMargin</c> and <c>bottomMargin</c> name (<c>vmlshape.cxx</c>:633-640), so unlike the
    /// horizontal pair above this axis does not cross over.
    /// </remarks>
    private static FrameVerticalOrigin VerticalOriginOf(Dictionary<string, string> style)
        => style.GetValueOrDefault("mso-position-vertical-relative") switch
        {
            "page" => FrameVerticalOrigin.Page,
            "margin" => FrameVerticalOrigin.PageMargin,
            "top-margin-area" => FrameVerticalOrigin.TopMarginArea,
            "bottom-margin-area" => FrameVerticalOrigin.BottomMarginArea,
            _ => FrameVerticalOrigin.Paragraph,
        };

    /// <summary>The declarations of a VML shape's <c>style</c> attribute, lower-cased and trimmed.</summary>
    private static Dictionary<string, string> Style(XElement shape)
    {
        Dictionary<string, string> declarations = new(StringComparer.OrdinalIgnoreCase);

        if (shape.Attribute("style")?.Value is not { } text) return declarations;

        foreach (string declaration in text.Split(';', StringSplitOptions.RemoveEmptyEntries))
        {
            int colon = declaration.IndexOf(':');
            if (colon <= 0) continue;

            declarations[declaration[..colon].Trim()] = declaration[(colon + 1)..].Trim();
        }

        return declarations;
    }

    /// <summary>A CSS length in the units VML writes, or null when it is none of them.</summary>
    /// <remarks>
    /// VML's own unit set, from the same table <c>oox/source/vml/vmlformatting.cxx</c>'s
    /// <c>ConversionHelper::decodeMeasureToEmu</c> reads. A bare number is points, which is what Word
    /// writes when it writes any; <c>px</c> is 1/96 inch, the CSS reference pixel, and not a device one.
    /// </remarks>
    private static Length? Css(string text)
    {
        string value = text.Trim();
        if (value.Length == 0) return null;

        (string suffix, double perUnit)[] units =
        [
            ("pt", 12700.0),
            ("in", 914400.0),
            ("cm", 360000.0),
            ("mm", 36000.0),
            ("pc", 152400.0),
            ("pi", 152400.0),
            ("px", 914400.0 / 96.0),
            ("em", 152400.0),
            ("ex", 76200.0),
        ];

        foreach ((string suffix, double perUnit) in units)
        {
            if (!value.EndsWith(suffix, StringComparison.OrdinalIgnoreCase)) continue;

            return Number(value[..^suffix.Length]) is { } scaled
                ? Length.FromEmu((long)Math.Round(scaled * perUnit))
                : null;
        }

        return Number(value) is { } points ? Length.FromPoints(points) : null;
    }

    private static double? Number(string text)
        => double.TryParse(
            text.Trim(), NumberStyles.Float, CultureInfo.InvariantCulture, out double parsed)
            ? parsed
            : null;

    /// <summary>One of <c>w:object</c>'s original-size attributes, in twentieths of a point.</summary>
    private static Length? Twips(XElement element, string name)
        => Word.Attribute(element, name) is { } text && Word.Integer(text, out int twips) && twips > 0
            ? Length.FromTwips(twips)
            : null;
}
