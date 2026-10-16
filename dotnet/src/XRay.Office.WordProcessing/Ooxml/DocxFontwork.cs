using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;

namespace XRay.Office.WordProcessing.Ooxml;

/// <summary>
/// A Writer text box whose body carries an <c>a:prstTxWarp</c>: WordArt, drawn as curves.
/// </summary>
/// <remarks>
/// <para>
/// <c>oox/source/shape/WpsContext.cxx:940-1025</c> is the whole of this on the reference's side, and
/// it does four things a reader has to reproduce together or not at all. It <strong>replaces the
/// shape</strong> with a Fontwork custom shape, so the box, its fill and its border are not drawn;
/// it moves the text out of the frame, so the words leave the text layer; it takes the
/// <em>character</em> fill and outline — <c>w14:textFill</c> and <c>w14:textOutline</c> — and makes
/// them the shape's, because Fontwork has one fill for the whole object and cannot style a portion;
/// and it reads the paragraph's alignment as the shape's text anchor, because Fontwork ignores
/// paragraph alignment.
/// </para>
/// <para>
/// <strong>It also refuses to do any of it unless the shape is a plain rectangle</strong>
/// (<c>WpsContext.cxx:966-970</c>: <c>if (sType != u"ooxml-rect") return;</c>). Word combines its
/// "abc Transform" with any shape it likes and LibreOffice can only render the rectangle-based kind,
/// so a warped pentagon keeps its text as text. That is a real branch and not a detail: reproducing
/// the warp without it would draw curves where the reference draws a labelled shape.
/// </para>
/// <para>
/// <strong>This is the one place <c>w14:textFill</c> and <c>w14:textOutline</c> may be read, and
/// that is measured.</strong> The same catalogue states 104 <c>w14:textFill</c>, 348
/// <c>w14:textOutline</c> and 96 <c>w14:shadow</c> on ordinary unwarped runs, and LibreOffice's DOCX
/// import draws none of them — its pages 3-6 score 0.00 unaccounted ink against ours, which draws
/// the run's plain <c>w:color</c>. They reach a shape only through the copy this class makes.
/// </para>
/// </remarks>
internal static class DocxFontwork
{
    /// <summary>The <c>a:prstGeom/@prst</c> the reference insists on before it warps anything.</summary>
    private const string RectangularPreset = "rect";

    /// <summary>A body the reference turns into curves and this cannot: no text, and no curves.</summary>
    private static FontworkDrawing Suppressed { get; } =
        new(null, null, null, null, Length.Zero, true);

    /// <summary>
    /// The warped outlines of a text box's body, and the paint they take, or nothing.
    /// </summary>
    /// <param name="shape">The <c>wps:wsp</c> or the drawing that holds one.</param>
    /// <param name="properties">Its <c>wps:spPr</c>, which has to state a rectangle.</param>
    /// <param name="blocks">
    /// Its text, already read into paragraphs with their faces and sizes resolved. Taken from here
    /// rather than from the markup because a run's family and size routinely come from a style, and
    /// this is where that has already been worked out.
    /// </param>
    /// <param name="size">The shape's rectangle.</param>
    /// <param name="theme">The theme a themed stop resolves against.</param>
    public static FontworkDrawing Read(
        XElement shape,
        XElement? properties,
        IReadOnlyList<PageBlock> blocks,
        DocSize size,
        DrawingTheme? theme)
    {
        if (Warp(shape) is not { } warp) return default;

        // The reference converts a warped body only when the shape is a plain rectangle; anything
        // else keeps its text as text, so nothing here applies to it.
        if (!IsRectangular(properties)) return default;

        List<string> lines = [];
        PageParagraph? first = null;

        foreach (PageBlock block in blocks)
        {
            if (block is not PageParagraph paragraph) continue;

            lines.Add(paragraph.Text);
            if (first is null && paragraph.Text.Length > 0) first = paragraph;
        }

        // From here the reference has taken the text out of the frame, whether or not this can
        // draw the curves that replace it. Everything below therefore answers `Suppressed` rather
        // than nothing, so that a warp X-Ray.Office cannot draw still leaves the text layer — which
        // is what the reference's own output does, and what the slides side already did.
        if (first is null) return Suppressed;

        (OpenTypeFace face, Length em) = FaceOf(first);
        XElement? runProperties = FirstRunProperties(shape);

        GraphicsPath? outline = Fontwork.Outline(new FontworkRequest
        {
            Preset = warp.Preset,
            Adjustments = warp.Adjustments,

            // A Writer text box is never a binary WordArt object, so the arch family keeps its
            // stated font size. `WpsContext.cxx:989` passes the flag false unconditionally.
            FromWordArt = false,
            Box = size,
            Lines = lines,
            Face = face,
            FontSize = em,
            Alignment = Alignment(first.Format.Alignment),
        });

        if (outline is null) return Suppressed;

        (Colour? fill, GradientDescription? gradient) = Fill(runProperties, first.Colour, theme);
        (Colour? line, Length width) = Outline(runProperties, theme);

        return new FontworkDrawing(outline, fill, gradient, line, width, true);
    }

    /// <summary>The <c>prstTxWarp</c> a body states, or null when it states none or the identity.</summary>
    private static (string Preset, List<FontworkAdjustment> Adjustments)? Warp(XElement shape)
    {
        XElement? body = shape.Descendants()
            .FirstOrDefault(child => child.Name.LocalName == "bodyPr");

        XElement? warp = Drawing.Child(body, "prstTxWarp");
        string? preset = warp?.Attribute("prst")?.Value;
        if (warp is null || !Fontwork.IsWarp(preset)) return null;

        List<FontworkAdjustment> adjustments = [];
        foreach (XElement guide in Drawing.Children(Drawing.Child(warp, "avLst"), "gd"))
        {
            string name = guide.Attribute("name")?.Value ?? string.Empty;
            string formula = guide.Attribute("fmla")?.Value ?? string.Empty;

            // Every guide inside a `prstTxWarp` is a literal: `fmla="val 10800000"`.
            if (!formula.StartsWith("val ", StringComparison.Ordinal)) continue;
            if (!double.TryParse(
                    formula.AsSpan(4),
                    System.Globalization.NumberStyles.Float,
                    System.Globalization.CultureInfo.InvariantCulture,
                    out double value))
            {
                continue;
            }

            adjustments.Add(new FontworkAdjustment(name, value));
        }

        return (preset!, adjustments);
    }

    /// <summary>Whether the shape is the plain rectangle the reference requires.</summary>
    private static bool IsRectangular(XElement? properties)
        => Drawing.Child(properties, "prstGeom")?.Attribute("prst")?.Value == RectangularPreset;

    /// <summary>The face and size the whole warp is set in: its first run's.</summary>
    /// <remarks>
    /// Fontwork reads <c>EE_CHAR_FONTINFO</c> off the shape, and the shape got it from the first
    /// non-empty run of the frame's text (<c>lcl_getTextPropsFromFrameText</c>,
    /// <c>WpsContext.cxx:150-190</c>). A body whose runs differ is drawn entirely in the first
    /// one's face, which is what "Fontwork cannot style text portions individually" means in
    /// practice.
    /// </remarks>
    private static (OpenTypeFace Face, Length EmSize) FaceOf(PageParagraph paragraph)
    {
        foreach (PageRun run in paragraph.Runs)
        {
            if (run.Length > 0) return (run.Face, run.EmSize);
        }

        return (paragraph.Face, paragraph.EmSize);
    }

    /// <summary>The alignment of the first paragraph, as the shape's text anchor.</summary>
    /// <remarks><c>lcl_setTextAnchorFromTextProps</c>, <c>WpsContext.cxx:465-488</c>.</remarks>
    private static FontworkAlignment Alignment(TextAlignment alignment) => alignment switch
    {
        TextAlignment.Start => FontworkAlignment.Left,
        TextAlignment.End => FontworkAlignment.Right,
        _ => FontworkAlignment.Centre,
    };

    /// <summary>
    /// The first non-empty run's properties, which carry the character fill and outline.
    /// </summary>
    private static XElement? FirstRunProperties(XElement shape)
    {
        XElement? box = shape.Descendants()
            .FirstOrDefault(child => child.Name.LocalName == "txbxContent");

        if (box is null) return null;

        foreach (XElement run in box.Descendants(Word.Name("r")))
        {
            bool empty = true;
            foreach (XElement text in run.Elements(Word.Name("t")))
            {
                if (text.Value.Length > 0) empty = false;
            }

            if (!empty) return Word.Child(run, "rPr");
        }

        return null;
    }

    /// <summary>
    /// What the warp is filled with: its <c>w14:textFill</c>, else the run's own colour.
    /// </summary>
    /// <remarks>
    /// <c>lcl_generateFillPropertiesFromTextProps</c>, <c>WpsContext.cxx:222-290</c>. The theme
    /// colour arm between the two is not reproduced — it reads a grab bag the Writer importer fills
    /// in, and it decides only a colour that a themed run already resolves to here.
    /// </remarks>
    private static (Colour? Fill, GradientDescription? Gradient) Fill(
        XElement? runProperties, Colour colour, DrawingTheme? theme)
    {
        XElement? effect = Word14.Child(runProperties, "textFill");
        if (effect is null) return (colour, null);

        XElement drawing = Word14.AsDrawingML(effect);

        if (Drawing.Child(drawing, "gradFill") is { } ramp
            && DrawingGradient.Read(ramp, theme) is { } gradient)
        {
            return (null, gradient);
        }

        if (Drawing.Child(drawing, "solidFill") is { } solid
            && DrawingColour.Read(solid.Elements().FirstOrDefault())?.Resolve(theme) is { } stated)
        {
            return (stated, null);
        }

        if (Drawing.Child(drawing, "noFill") is not null) return (null, null);

        return (colour, null);
    }

    /// <summary>The pen the warp is stroked with, from <c>w14:textOutline</c>.</summary>
    /// <remarks>
    /// <c>lcl_generateLinePropertiesFromTextProps</c>, <c>WpsContext.cxx:196-220</c>: no outline
    /// element means <c>noFill</c>, so an unstroked warp rather than a black one.
    /// </remarks>
    private static (Colour? Colour, Length Width) Outline(XElement? runProperties, DrawingTheme? theme)
    {
        XElement? effect = Word14.Child(runProperties, "textOutline");
        if (effect is null) return (null, Length.Zero);

        XElement drawing = Word14.AsDrawingML(effect);
        if (Drawing.Child(drawing, "solidFill") is not { } solid) return (null, Length.Zero);
        if (DrawingColour.Read(solid.Elements().FirstOrDefault())?.Resolve(theme) is not { } colour)
        {
            return (null, Length.Zero);
        }

        long width = 0;
        if (drawing.Attribute("w")?.Value is { } stated
            && long.TryParse(
                stated,
                System.Globalization.NumberStyles.Integer,
                System.Globalization.CultureInfo.InvariantCulture,
                out long emu))
        {
            width = emu;
        }

        return (colour, Length.FromEmu(width));
    }
}

/// <summary>
/// A warped body's geometry and the paint that replaces its shape's.
/// </summary>
/// <param name="Outline">The curves, in the shape's own coordinates, or null when there are none.</param>
/// <param name="Fill">The colour they are filled with, or null when a gradient or nothing fills them.</param>
/// <param name="Gradient">The ramp they are filled with, or null.</param>
/// <param name="Line">The colour they are stroked with, or null when they are not stroked.</param>
/// <param name="LineWidth">The pen's width.</param>
/// <param name="SuppressesText">
/// Whether the reference would have taken this body's text out of the frame, which it does for
/// every warped rectangle whether or not the warp is one X-Ray.Office can draw. It is the flag that
/// keeps the two families' fallbacks the same: a warp neither side can draw leaves no text on
/// either, as the reference leaves none.
/// </param>
internal readonly record struct FontworkDrawing(
    GraphicsPath? Outline,
    Colour? Fill,
    GradientDescription? Gradient,
    Colour? Line,
    Length LineWidth,
    bool SuppressesText)
{
    /// <summary>Whether the body warped at all.</summary>
    public bool IsWarped => Outline is not null;
}

/// <summary>
/// The <c>w14</c> text-effect vocabulary, which is DrawingML under another namespace.
/// </summary>
/// <remarks>
/// <c>w14:textFill</c> and <c>w14:textOutline</c> hold the same <c>gradFill</c>, <c>solidFill</c>,
/// <c>srgbClr</c> and <c>gs</c> elements <c>a:</c> does, with the same attributes — except that the
/// attributes are namespace-qualified where DrawingML's are not: a stop is
/// <c>&lt;w14:srgbClr w14:val="22d3ee"/&gt;</c> and not <c>&lt;a:srgbClr val="22d3ee"/&gt;</c>. So
/// the readers in <c>XRay.Office.Ooxml.DrawingML</c> can read one once the names have been put back,
/// which is cheaper and far less error-prone than a second colour and gradient reader.
/// </remarks>
internal static class Word14
{
    /// <summary>The <c>w14</c> namespace, the 2010 Word extensions.</summary>
    private static readonly XNamespace Namespace =
        "http://schemas.microsoft.com/office/word/2010/wordml";

    /// <summary>A <c>w14:</c>-namespaced child with this local name, or null.</summary>
    public static XElement? Child(XElement? element, string localName)
        => element?.Element(Namespace + localName);

    /// <summary>
    /// The same subtree with DrawingML's naming: <c>a:</c> elements and bare attributes.
    /// </summary>
    public static XElement AsDrawingML(XElement source)
    {
        XNamespace drawing = OoxmlNamespaces.DrawingML;
        XElement copy = new(drawing + source.Name.LocalName);

        foreach (XAttribute attribute in source.Attributes())
        {
            if (attribute.IsNamespaceDeclaration) continue;
            copy.Add(new XAttribute(XName.Get(attribute.Name.LocalName), attribute.Value));
        }

        foreach (XElement child in source.Elements()) copy.Add(AsDrawingML(child));

        return copy;
    }
}
