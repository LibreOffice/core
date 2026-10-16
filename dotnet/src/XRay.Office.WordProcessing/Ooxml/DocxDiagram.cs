using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml;
using XRay.Office.Ooxml.DrawingML;

namespace XRay.Office.WordProcessing.Ooxml;

/// <summary>
/// Turns a SmartArt diagram anchored in a document into a shape group <see cref="DocxFrames"/>
/// already knows how to place.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The diagram itself is read one library down, by <see cref="DiagramParts"/>.</strong>
/// Resolving <c>dgm:relIds</c>, preferring the baked <c>dsp:spTree</c> the authoring application
/// wrote and falling back to evaluating the layout definition are the same in every OOXML family,
/// and none of it knows what a document is. What is family-specific — and all that is here — is
/// the last hop: the shape tree comes back in PresentationML, and Writer's frame reader speaks
/// <c>wps:</c>/<c>w:</c>.
/// </para>
/// <para>
/// <strong>That hop is smaller than it looks, and deliberately so.</strong>
/// <see cref="DocxFrames"/> matches every element it reads on its <em>local</em> name — <c>spPr</c>,
/// <c>xfrm</c>, <c>prstGeom</c>, <c>gradFill</c>, <c>style</c>, <c>bodyPr</c> — so a
/// <c>p:sp</c> carrying DrawingML shape properties is already a shape it can place, fill, outline
/// and give a preset geometry to. The only thing it cannot find in one is the text: a Word shape
/// states its text as <c>w:txbxContent</c> full of <c>w:p</c>, and DrawingML states it as
/// <c>a:txBody</c> full of <c>a:p</c>. So the whole translation is
/// <see cref="TextBox"/> — one <c>w:txbxContent</c> hung off each shape — and everything else is
/// carried across untouched.
/// </para>
/// <para>
/// <strong>The child space is the frame, and the group must not be refitted to it.</strong>
/// A diagram's baked shapes are stated in the frame's own coordinates, which is what
/// <c>SmartArtDiagram::createShapeHierarchyFromModel</c> says in one line —
/// <c>pParentShape-&gt;setChildSize(pParentShape-&gt;getSize())</c>
/// (<c>oox/source/drawingml/diagram/diagram.cxx</c>) — so the mapping is the identity and the
/// members do not fill their space. <see cref="DocxFrames"/> refits a <c>wpg:wgp</c> whose members
/// do not fill it and leaves a <c>wpc</c> canvas alone, so the synthesised container is named
/// <c>wpc</c>: naming it <c>wgp</c> would stretch a diagram stated in a 5 804 749 EMU square
/// across a 6 998 335 EMU frame.
/// </para>
/// <para>
/// Measured on <c>024_Unit_Circle_Chart_Colorful_Circles</c>, whose five nodes and four connectors
/// are baked: 26.2.4.2 draws the five ellipses with their horizontal centres 165.13, 165.14 pt
/// apart, and the baked <c>a:off</c> differ by 2 097 140 EMU, which is 165.128 pt — the identity,
/// to a fiftieth of a point. Its vertical spacing is 163.14–163.18 pt against the same
/// 165.128, so the reference squashes the diagram by about 1.2 % in the one direction where the
/// frame is not exactly the drawing's own width. That residual is recorded rather than
/// reproduced; see <c>dotnet/probes/words-diagram-01/results.md</c>.
/// </para>
/// </remarks>
internal static class DocxDiagram
{
    /// <summary>How deep a synthesised group may nest before the walk stops.</summary>
    private const int MaxDepth = 8;

    /// <summary>
    /// A DrawingML text body states no size as often as not, and 18 pt is what the schema's
    /// <c>CT_TextCharacterProperties</c> default resolves to through the presentation defaults.
    /// </summary>
    private const int DefaultSizeHundredths = 1800;

    /// <summary>Twips per point, for the spacing properties Word states in twips.</summary>
    private const double TwipsPerPoint = 20;

    /// <summary>The container a translated diagram is handed to <see cref="DocxFrames"/> in.</summary>
    /// <remarks>
    /// A drawing canvas rather than a group, for the reason given on the class: a canvas keeps the
    /// child space it was given and a group is refitted to its members.
    /// </remarks>
    private static XName Canvas => XName.Get("wpc", OoxmlNamespaces.WordCanvas);

    /// <summary>The name a translated shape takes, which is what a Word shape is called.</summary>
    private static XName Shape => XName.Get("wsp", OoxmlNamespaces.WordShape);

    /// <summary>The <c>wps:txbx</c> wrapper a Word shape's text sits in.</summary>
    private static XName TextWrapper => XName.Get("txbx", OoxmlNamespaces.WordShape);

    /// <summary>
    /// The shape group a <c>w:drawing</c>'s diagram becomes, or null when it holds none that draws.
    /// </summary>
    /// <param name="graphicData">The <c>a:graphicData</c> whose <c>uri</c> names the diagram vocabulary.</param>
    /// <param name="frame">The frame's extent, which is the diagram's own coordinate space.</param>
    /// <param name="source">The package, as the two lookups the diagram's parts need.</param>
    /// <param name="partName">The part stating the <c>dgm:relIds</c>, which scopes its ids.</param>
    /// <param name="themePart">The <c>a:theme</c> root, for the evaluator's style resolution.</param>
    /// <param name="theme">The theme as a colour model.</param>
    /// <returns>A <c>wpc</c> group of translated shapes, or null.</returns>
    public static XElement? Read(
        XElement graphicData,
        DocSize frame,
        DiagramPartSource source,
        string partName,
        XElement? themePart,
        DrawingTheme? theme)
    {
        ArgumentNullException.ThrowIfNull(graphicData);
        ArgumentNullException.ThrowIfNull(source);

        if (frame.Width <= Length.Zero || frame.Height <= Length.Zero) return null;

        DiagramParts.BakedDrawing? drawing =
            DiagramParts.Baked(source, partName, graphicData)
            ?? DiagramParts.Evaluated(
                source,
                partName,
                graphicData,
                themePart,
                theme,
                (int)frame.Width.Emu,
                (int)frame.Height.Emu);

        if (drawing is not { } baked) return null;

        XElement canvas = new(Canvas);
        Translate(baked.ShapeTree, canvas, theme, depth: 0);

        return canvas.Elements().Any() ? canvas : null;
    }

    /// <summary>Copies one level of the shape tree into the synthesised group.</summary>
    private static void Translate(XElement parent, XElement into, DrawingTheme? theme, int depth)
    {
        if (depth >= MaxDepth) return;

        foreach (XElement element in parent.Elements())
        {
            switch (element.Name.LocalName)
            {
                case "sp" or "cxnSp":
                    into.Add(Shaped(element, theme));
                    break;

                case "grpSp":
                {
                    // Kept as a group, since a nested one really does state a child space of its
                    // own; only the outermost container is the frame's.
                    XElement group = new(XName.Get("grpSp", OoxmlNamespaces.WordShapeGroup));
                    foreach (XElement properties in element.Elements()
                                 .Where(child => child.Name.LocalName is "grpSpPr"))
                    {
                        group.Add(new XElement(properties));
                    }

                    Translate(element, group, theme, depth + 1);
                    into.Add(group);
                    break;
                }

                default:
                    continue;
            }
        }
    }

    /// <summary>
    /// One diagram shape as a Word shape: everything it stated, plus its text as Word states text.
    /// </summary>
    /// <remarks>
    /// The original <c>p:txBody</c> is carried across as well as translated, because
    /// <see cref="DocxFrames"/> reads the <c>a:bodyPr</c> inside it for the shape's insets, its
    /// vertical anchor and whether it grows with its text — all of which a diagram states and none
    /// of which has a WordprocessingML spelling worth inventing.
    /// </remarks>
    private static XElement Shaped(XElement shape, DrawingTheme? theme)
    {
        XElement translated = new(Shape, shape.Elements().Select(child => new XElement(child)));

        if (TextBox(shape, theme) is { } text) translated.Add(new XElement(TextWrapper, text));

        return translated;
    }

    /// <summary>
    /// A shape's <c>a:txBody</c> as the <c>w:txbxContent</c> Writer's frame reader lays out, or
    /// null when the shape has no text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Every property is stated explicitly rather than left to inherit, because the paragraphs
    /// this builds are handed to the same walk the body's are and would otherwise take the
    /// document's default style: an 11 pt body face, and — in nearly every file Word writes — 8 pt
    /// of space after each paragraph and 1.08 line spacing. A diagram node is a fixed circle with
    /// two words in it, and inheriting a paragraph gap pushes them out of it.
    /// </para>
    /// <para>
    /// The colour and the face come from the shape's <c>a:fontRef</c> when the run states none,
    /// which is the usual case: a SmartArt run carries a size and nothing else, and the theme
    /// reference beside it is what makes the text white on a coloured node. Resolving it here
    /// rather than leaving it to the run means a node's text is legible instead of black on its
    /// own accent colour.
    /// </para>
    /// </remarks>
    private static XElement? TextBox(XElement shape, DrawingTheme? theme)
    {
        XElement? body = shape.Elements().FirstOrDefault(child => child.Name.LocalName == "txBody");
        if (body is null) return null;

        List<XElement> paragraphs = [.. body.Elements()
            .Where(child => child.Name.LocalName == "p")];

        if (paragraphs.Count == 0) return null;

        XElement? style = shape.Elements().FirstOrDefault(child => child.Name.LocalName == "style");
        XElement? fontReference = style?.Elements()
            .FirstOrDefault(child => child.Name.LocalName == "fontRef");

        Colour? referenced = fontReference?.Elements().FirstOrDefault() is { } colour
            ? DrawingColour.Read(colour)?.Resolve(theme)
            : null;

        string? face = Face(fontReference, theme);
        double scale = FontScale(body);

        XElement content = new(Word.Name("txbxContent"));
        bool any = false;

        foreach (XElement paragraph in paragraphs)
        {
            XElement translated = Paragraph(paragraph, referenced, face, scale);
            content.Add(translated);
            any |= translated.Descendants(Word.Name("t")).Any();
        }

        return any ? content : null;
    }

    /// <summary>One <c>a:p</c> as a <c>w:p</c>, with its alignment, indent and spacing.</summary>
    private static XElement Paragraph(
        XElement paragraph, Colour? referenced, string? face, double scale)
    {
        XElement? properties = paragraph.Elements()
            .FirstOrDefault(child => child.Name.LocalName == "pPr");
        XElement? defaults = properties?.Elements()
            .FirstOrDefault(child => child.Name.LocalName == "defRPr");

        int size = ParagraphSize(paragraph, defaults, scale);

        XElement formatting = new(
            Word.Name("pPr"),
            Alignment(properties),
            Indent(properties),
            Spacing(properties, size));

        XElement translated = new(Word.Name("p"), formatting);

        foreach (XElement element in paragraph.Elements())
        {
            switch (element.Name.LocalName)
            {
                case "r":
                    if (Run(element, defaults, referenced, face, scale) is { } run)
                    {
                        translated.Add(run);
                    }

                    break;

                case "fld":
                    // A field's cached result is what the file draws, and a diagram's fields are
                    // slide numbers and dates rather than anything that needs evaluating.
                    if (Run(element, defaults, referenced, face, scale) is { } field)
                    {
                        translated.Add(field);
                    }

                    break;

                case "br":
                    translated.Add(new XElement(Word.Name("r"), new XElement(Word.Name("br"))));
                    break;

                default:
                    continue;
            }
        }

        return translated;
    }

    /// <summary>One <c>a:r</c> as a <c>w:r</c>, or null when it carries no text.</summary>
    private static XElement? Run(
        XElement run, XElement? defaults, Colour? referenced, string? face, double scale)
    {
        string text = string.Concat(run.Elements()
            .Where(child => child.Name.LocalName == "t")
            .Select(child => child.Value));

        if (text.Length == 0) return null;

        XElement? properties = run.Elements()
            .FirstOrDefault(child => child.Name.LocalName == "rPr");

        int size = Size(properties, defaults, scale);
        string? typeface = Typeface(properties) ?? Typeface(defaults) ?? face;
        Colour? colour = Colour(properties) ?? Colour(defaults) ?? referenced;

        XElement formatting = new(Word.Name("rPr"));

        if (typeface is not null)
        {
            formatting.Add(new XElement(
                Word.Name("rFonts"),
                new XAttribute(Word.Name("ascii"), typeface),
                new XAttribute(Word.Name("hAnsi"), typeface),
                new XAttribute(Word.Name("cs"), typeface)));
        }

        if (Flag(properties, defaults, "b")) formatting.Add(new XElement(Word.Name("b")));
        if (Flag(properties, defaults, "i")) formatting.Add(new XElement(Word.Name("i")));

        if ((Underline(properties) ?? Underline(defaults)) is { } underline and not "none")
        {
            formatting.Add(new XElement(
                Word.Name("u"), new XAttribute(Word.Name("val"), underline)));
        }

        if (colour is { } resolved)
        {
            formatting.Add(new XElement(
                Word.Name("color"),
                new XAttribute(
                    Word.Name("val"),
                    $"{resolved.R:X2}{resolved.G:X2}{resolved.B:X2}")));
        }

        // Half-points, from DrawingML's hundredths of a point.
        string halves = (size / 50).ToString(CultureInfo.InvariantCulture);
        formatting.Add(new XElement(Word.Name("sz"), new XAttribute(Word.Name("val"), halves)));
        formatting.Add(new XElement(Word.Name("szCs"), new XAttribute(Word.Name("val"), halves)));

        return new XElement(
            Word.Name("r"),
            formatting,
            new XElement(
                Word.Name("t"),
                new XAttribute(XNamespace.Xml + "space", "preserve"),
                text));
    }

    /// <summary>The run's size in hundredths of a point, after any autofit scale.</summary>
    private static int Size(XElement? properties, XElement? defaults, double scale)
    {
        int stated = Integer(properties, "sz")
                     ?? Integer(defaults, "sz")
                     ?? DefaultSizeHundredths;

        return Math.Max(100, (int)Math.Round(stated * scale));
    }

    /// <summary>
    /// The size a paragraph's own text is set at, for turning a percentage spacing into twips.
    /// </summary>
    private static int ParagraphSize(XElement paragraph, XElement? defaults, double scale)
    {
        foreach (XElement run in paragraph.Elements()
                     .Where(child => child.Name.LocalName is "r" or "fld"))
        {
            XElement? properties = run.Elements()
                .FirstOrDefault(child => child.Name.LocalName == "rPr");
            if (Integer(properties, "sz") is { } stated)
            {
                return Math.Max(100, (int)Math.Round(stated * scale));
            }
        }

        return Size(properties: null, defaults, scale);
    }

    /// <summary>
    /// <c>a:normAutofit/@fontScale</c> as a factor, or one when the body does not shrink its text.
    /// </summary>
    /// <remarks>
    /// The authoring application writes the scale it settled on rather than asking a reader to
    /// re-derive it, so honouring the stated one reproduces what the file was saved looking like.
    /// It is in thousandths of a per cent, as every DrawingML percentage is.
    /// </remarks>
    private static double FontScale(XElement body)
    {
        XElement? properties = body.Elements()
            .FirstOrDefault(child => child.Name.LocalName == "bodyPr");
        XElement? autofit = properties?.Elements()
            .FirstOrDefault(child => child.Name.LocalName == "normAutofit");

        return Integer(autofit, "fontScale") is { } scale and > 0 ? scale / 100000.0 : 1;
    }

    /// <summary>The paragraph's alignment, or null when it states none.</summary>
    private static XElement? Alignment(XElement? properties)
        => properties?.Attribute("algn")?.Value switch
        {
            "ctr" => Value("jc", "center"),
            "r" => Value("jc", "right"),
            "just" => Value("jc", "both"),
            "dist" => Value("jc", "distribute"),
            "l" => Value("jc", "left"),
            _ => null,
        };

    /// <summary>
    /// The paragraph's indents, always stated — a diagram node's text is flush inside its shape
    /// and would otherwise take the document default style's.
    /// </summary>
    private static XElement Indent(XElement? properties)
    {
        long left = Integer(properties, "marL") ?? 0;
        long first = Integer(properties, "indent") ?? 0;

        XElement indent = new(
            Word.Name("ind"),
            new XAttribute(Word.Name("left"), Twips(left)),
            new XAttribute(Word.Name("right"), Twips(Integer(properties, "marR") ?? 0)));

        if (first < 0)
        {
            indent.Add(new XAttribute(Word.Name("hanging"), Twips(-first)));
        }
        else if (first > 0)
        {
            indent.Add(new XAttribute(Word.Name("firstLine"), Twips(first)));
        }

        return indent;
    }

    /// <summary>
    /// The paragraph's line and paragraph spacing, always stated.
    /// </summary>
    /// <remarks>
    /// DrawingML measures a paragraph gap as a percentage of the text's own size where Word
    /// measures it in twips, so the size has to be resolved before the gap can be — which is why
    /// this takes it rather than reading it again. A percentage line spacing is Word's
    /// <c>auto</c> rule scaled from its own 240, and an exact one is <c>exact</c>.
    /// </remarks>
    private static XElement Spacing(XElement? properties, int sizeHundredths)
    {
        double points = sizeHundredths / 100.0;

        XElement spacing = new(
            Word.Name("spacing"),
            new XAttribute(Word.Name("before"), Gap(properties, "spcBef", points)),
            new XAttribute(Word.Name("after"), Gap(properties, "spcAft", points)));

        XElement? line = properties?.Elements()
            .FirstOrDefault(child => child.Name.LocalName == "lnSpc");

        if (Percentage(line) is { } percent)
        {
            spacing.Add(new XAttribute(
                Word.Name("line"),
                Math.Max(1, (int)Math.Round(240 * percent)).ToString(CultureInfo.InvariantCulture)));
            spacing.Add(new XAttribute(Word.Name("lineRule"), "auto"));
        }
        else if (Points(line) is { } exact)
        {
            spacing.Add(new XAttribute(
                Word.Name("line"),
                Math.Max(1, (int)Math.Round(exact * TwipsPerPoint))
                    .ToString(CultureInfo.InvariantCulture)));
            spacing.Add(new XAttribute(Word.Name("lineRule"), "exact"));
        }
        else
        {
            spacing.Add(new XAttribute(Word.Name("line"), "240"));
            spacing.Add(new XAttribute(Word.Name("lineRule"), "auto"));
        }

        return spacing;

        static string Gap(XElement? paragraph, string name, double sizePoints)
        {
            XElement? element = paragraph?.Elements()
                .FirstOrDefault(child => child.Name.LocalName == name);

            double twips =
                Percentage(element) is { } percent ? percent * sizePoints * TwipsPerPoint
                : Points(element) is { } points ? points * TwipsPerPoint
                : 0;

            return Math.Max(0, (int)Math.Round(twips)).ToString(CultureInfo.InvariantCulture);
        }
    }

    /// <summary>An <c>a:spcPct</c>'s value as a factor, or null when the element states none.</summary>
    private static double? Percentage(XElement? element)
        => Integer(
               element?.Elements().FirstOrDefault(child => child.Name.LocalName == "spcPct"),
               "val") is { } value
            ? value / 100000.0
            : null;

    /// <summary>An <c>a:spcPts</c>'s value in points, or null when the element states none.</summary>
    private static double? Points(XElement? element)
        => Integer(
               element?.Elements().FirstOrDefault(child => child.Name.LocalName == "spcPts"),
               "val") is { } value
            ? value / 100.0
            : null;

    /// <summary>The literal face a run names, ignoring a <c>+mn-lt</c>-style theme reference.</summary>
    private static string? Typeface(XElement? properties)
    {
        string? name = properties?.Elements()
            .FirstOrDefault(child => child.Name.LocalName == "latin")
            ?.Attribute("typeface")?.Value;

        return string.IsNullOrWhiteSpace(name) || name[0] == '+' ? null : name;
    }

    /// <summary>The face an <c>a:fontRef</c> names out of the theme, or null.</summary>
    private static string? Face(XElement? fontReference, DrawingTheme? theme)
        => fontReference?.Attribute("idx")?.Value switch
        {
            "major" => theme?.Fonts?.MajorLatin,
            "minor" => theme?.Fonts?.MinorLatin,
            _ => null,
        };

    /// <summary>A run's explicit colour, or null when it states none.</summary>
    private static Colour? Colour(XElement? properties)
    {
        XElement? fill = properties?.Elements()
            .FirstOrDefault(child => child.Name.LocalName == "solidFill");

        return fill?.Elements().FirstOrDefault() is { } colour
            ? DrawingColour.Read(colour)?.Resolve(null)
            : null;
    }

    /// <summary>An <c>a:u</c> value as Word spells it, or null when the run states none.</summary>
    private static string? Underline(XElement? properties)
        => properties?.Attribute("u")?.Value switch
        {
            null => null,
            "none" => "none",
            "sng" => "single",
            "dbl" => "double",
            "dotted" => "dotted",
            "dash" => "dash",
            "wavy" => "wave",
            "heavy" => "thick",
            _ => "single",
        };

    /// <summary>An ST_OnOff attribute on the run or, failing that, on the paragraph's defaults.</summary>
    private static bool Flag(XElement? properties, XElement? defaults, string name)
        => (properties?.Attribute(name)?.Value ?? defaults?.Attribute(name)?.Value)
            is "1" or "true" or "on";

    /// <summary>An unprefixed integer attribute, or null when it is absent or unparseable.</summary>
    private static int? Integer(XElement? element, string name)
        => element?.Attribute(name)?.Value is { } text
           && int.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out int value)
            ? value
            : null;

    /// <summary>An EMU measurement as the twips Word states one in.</summary>
    private static string Twips(long emu)
        => Length.FromEmu(emu).Twips.ToString(CultureInfo.InvariantCulture);

    /// <summary>A <c>w:val</c>-carrying property element.</summary>
    private static XElement Value(string name, string value)
        => new(Word.Name(name), new XAttribute(Word.Name("val"), value));
}
