using System.Globalization;
using System.Text;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.Presentations.Layout;
using Paperless.Text.Layout;

namespace Paperless.Presentations.OpenDocument;

/// <summary>
/// Reads a shape's <c>text:p</c> children into the paragraphs slide layout takes.
/// </summary>
/// <remarks>
/// The ODF counterpart of <c>PptxTextBody</c>. What differs is where the formatting lives: none
/// of it is on the run, all of it is in a style reached by name, and the run's own
/// <c>text:style-name</c> is only the innermost link of a chain that starts at the shape's
/// graphic style. <see cref="OdfTextFormat.Resolve"/> already walks exactly that chain, so this
/// is mostly assembling the cascade and turning what comes back into layout's own vocabulary.
/// </remarks>
internal static class OdfTextBody
{
    /// <summary>The size a run gets when nothing in its cascade states one: 18 pt.</summary>
    /// <remarks>
    /// Impress's own default for a text frame. A word processor's default is 12 pt, and using
    /// that here would make every unstated line of every slide a third too small.
    /// </remarks>
    private static readonly Length DefaultSize = Length.FromPoints(18);

    /// <summary>The character a <c>text:line-break</c> becomes.</summary>
    private const char LineSeparator = '\u2028';

    /// <summary>Reads a shape's paragraphs.</summary>
    /// <param name="file">The document, for its styles.</param>
    /// <param name="paragraphs">The <c>text:p</c> elements.</param>
    /// <param name="shapeCascade">
    /// The shape's own style references, which the paragraph and run styles sit inside.
    /// </param>
    public static SlideTextBody Read(
        OdfFile file,
        IEnumerable<XElement> paragraphs,
        IReadOnlyList<OdfStyleReference> shapeCascade)
    {
        ArgumentNullException.ThrowIfNull(file);
        ArgumentNullException.ThrowIfNull(paragraphs);
        ArgumentNullException.ThrowIfNull(shapeCascade);

        List<SlideParagraph> read = [];
        bool fontIndependent = false;
        string? outlineBase = OutlineStyleBase(file, shapeCascade);

        foreach (XElement paragraph in paragraphs)
        {
            OdfStyleReference style = new(
                paragraph.Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value,
                OdfStyleFamily.Paragraph);

            int level = OutlineLevel(paragraph);

            List<OdfStyleReference> cascade =
                [.. shapeCascade, OutlineStyle(outlineBase, level), style];

            if (file.Styles.ResolveProperty(
                    cascade, OdfPropertyKind.Paragraph,
                    OdfNamespaces.Style, "font-independent-line-spacing").AsBoolean() == true)
            {
                fontIndependent = true;
            }

            SlideParagraph read1 = Paragraph(file, paragraph, cascade);

            read.Add(Label(file, paragraph, level) is { } label
                ? read1 with
                {
                    StartIndent = label.Start,
                    FirstLineIndent = label.Hanging,
                    Marker = label.Marker,
                }
                : read1);
        }

        return new SlideTextBody
        {
            Paragraphs = read,
            Insets = Insets(file, shapeCascade),
            Anchor = Anchor(file, shapeCascade),
            FontIndependentLineSpacing = fontIndependent,
        };
    }

    /// <summary>
    /// A list item's label: where its text starts, how far its marker hangs back, and what it is.
    /// </summary>
    private readonly record struct ListLabel(Length Start, Length Hanging, SlideMarker? Marker);

    /// <summary>
    /// How deeply a paragraph is nested in <c>text:list</c> elements, counted from one.
    /// </summary>
    /// <remarks>
    /// ODF states list <em>structure</em> by nesting and list <em>appearance</em> by a separately
    /// named style, which is the opposite way round from OOXML's flat "paragraph plus a level
    /// number". So the level is the nesting depth, and zero means the paragraph is not in a list
    /// at all — in which case its own <c>fo:margin-left</c> decides and nothing here applies.
    /// </remarks>
    private static int OutlineLevel(XElement paragraph)
    {
        int level = 0;

        for (XElement? ancestor = paragraph.Parent; ancestor is not null;
             ancestor = ancestor.Parent)
        {
            if (ancestor.Name.NamespaceName == OdfNamespaces.Text
                && ancestor.Name.LocalName == "list")
            {
                level++;
            }
        }

        return level;
    }

    /// <summary>
    /// The indents and the marker a list level gives its items.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>ODF's default label geometry is not <c>fo:margin-left</c> and
    /// <c>fo:text-indent</c>.</strong> Those belong to the ODF 1.2 <em>label-alignment</em> mode,
    /// which LibreOffice writes for Writer; a presentation's list style uses the older
    /// <em>label-width-and-position</em> mode, whose two quantities are <c>text:space-before</c>
    /// and <c>text:min-label-width</c>. The text starts at the sum of them and the marker at the
    /// space alone, which is exactly the <c>marL</c> and <c>marL + indent</c> pair PresentationML
    /// states directly. Measured on <c>slides-features.odp</c>, whose level 1 states no space and
    /// a 0.6 cm label: LibreOffice draws the bullet at 56.693 and the text at 73.701, and 17.008 pt
    /// is 0.6 cm.
    /// </para>
    /// <para>
    /// The label's own size is a percentage of the item's text — 45% in every deck LibreOffice
    /// writes — and its face is the level's, which is a symbol font that will not be installed and
    /// will substitute. Both come from the level's <c>style:text-properties</c> rather than from
    /// the paragraph's.
    /// </para>
    /// </remarks>
    private static ListLabel? Label(OdfFile file, XElement paragraph, int level)
    {
        if (level < 1) return null;
        if (ListStyle(file, paragraph) is not { } style) return null;
        if (style.GetLevel(level) is not { } definition) return null;

        Length space = Measure(definition, "space-before");
        Length label = Measure(definition, "min-label-width");

        return new ListLabel(space + label, -label, Marker(definition, level, style));

        static Length Measure(OdfListLevel definition, string name)
            => OdfValue.ParseLength(
                   definition.LevelProperties?.Get(OdfNamespaces.Text, name))
               ?? Length.Zero;
    }

    /// <summary>
    /// The list style a paragraph's innermost <c>text:list</c> names.
    /// </summary>
    /// <remarks>
    /// Innermost first, then outwards: LibreOffice writes the style on the outermost list of a
    /// run and leaves the nested ones bare, so a reader taking only the immediate parent finds
    /// nothing on every level past the first.
    /// </remarks>
    private static OdfListStyle? ListStyle(OdfFile file, XElement paragraph)
    {
        for (XElement? ancestor = paragraph.Parent; ancestor is not null;
             ancestor = ancestor.Parent)
        {
            if (ancestor.Name.NamespaceName != OdfNamespaces.Text) continue;
            if (ancestor.Name.LocalName != "list") continue;

            string? name = ancestor.Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value;
            if (file.Styles.FindListStyle(name) is { } style) return style;
        }

        return null;
    }

    /// <summary>
    /// The marker a level draws, or null when it draws none.
    /// </summary>
    /// <remarks>
    /// Only a bullet, deliberately. A numbered level needs a counter carried across the body and
    /// restarted where the level rises, and inventing "1." for every item is a worse answer than
    /// none — the same decision the OOXML path records for <c>a:buAutoNum</c>. The counters passed
    /// here are all ones so that <see cref="OdfListStyle.FormatLabel"/> returns the bullet without
    /// pretending to number anything.
    /// </remarks>
    private static SlideMarker? Marker(OdfListLevel definition, int level, OdfListStyle style)
    {
        if (definition.Kind != OdfListLabelKind.Bullet) return null;
        if (style.FormatLabel(level, [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]) is not { Length: > 0 } text)
            return null;

        return new SlideMarker(text, definition.Typeface, definition.RelativeSize ?? 1.0);
    }

    /// <summary>
    /// The name a presentation's per-level outline styles share, or null when the shape has none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An outline placeholder's formatting is per level and lives in a style the shape
    /// only reaches through its parent.</strong> LibreOffice creates one presentation style per
    /// master page and outline level — <c>Default-outline1</c> … <c>Default-outline9</c>,
    /// chained parent to child — and the shape's own <c>presentation:style-name</c> inherits from
    /// level one (<c>xmloff/source/draw/ximpstyl.cxx</c>'s <c>ImpSetGraphicStyles</c>). So a
    /// paragraph at level two must resolve against <c>Default-outline2</c>, which nothing in the
    /// shape's own cascade points at.
    /// </para>
    /// <para>
    /// What it carries is the font size per level and the space above a paragraph. Measured on
    /// <c>slides-features.odp</c>: <c>Default-outline2</c> states <c>fo:margin-top="0.4cm"</c> and
    /// nothing else, and without it the deck's third outline paragraph sat 11.23 pt above where
    /// LibreOffice draws it — a drift that grows with every level-two paragraph and looks like a
    /// line-height bug.
    /// </para>
    /// </remarks>
    private static string? OutlineStyleBase(
        OdfFile file, IReadOnlyList<OdfStyleReference> cascade)
    {
        foreach (OdfStyleReference reference in cascade)
        {
            if (reference.Family != OdfStyleFamily.Presentation) continue;

            string? name = reference.Name;
            for (int depth = 0; name is not null && depth < OdfStyles.MaxParentChainDepth; depth++)
            {
                if (name.Length > 8
                    && char.IsAsciiDigit(name[^1])
                    && name.AsSpan(..^1).EndsWith("-outline", StringComparison.Ordinal))
                {
                    return name[..^1];
                }

                name = file.Styles.Find(name, OdfStyleFamily.Presentation)?.ParentStyleName;
            }
        }

        return null;
    }

    private static OdfStyleReference OutlineStyle(string? outlineBase, int level)
        => outlineBase is null || level < 1
            ? new OdfStyleReference(null, OdfStyleFamily.Presentation)
            : new OdfStyleReference(
                outlineBase + Math.Clamp(level, 1, 9).ToString(CultureInfo.InvariantCulture),
                OdfStyleFamily.Presentation);

    /// <summary>
    /// The text insets, which ODF spells as the shape's padding.
    /// </summary>
    /// <remarks>
    /// Zero when the style states none, unlike DrawingML — ODF has no implied default here, and a
    /// LibreOffice-written shape states all four. A deck converted from PPTX carries the OOXML
    /// defaults written out explicitly, which is how the two paths agree on the same document.
    /// </remarks>
    private static Margins Insets(OdfFile file, IReadOnlyList<OdfStyleReference> cascade) => new(
        Padding(file, cascade, "padding-left"),
        Padding(file, cascade, "padding-top"),
        Padding(file, cascade, "padding-right"),
        Padding(file, cascade, "padding-bottom"));

    private static Length Padding(
        OdfFile file, IReadOnlyList<OdfStyleReference> cascade, string name)
        => file.Styles.ResolveProperty(
               cascade, OdfPropertyKind.Graphic, OdfNamespaces.FoCompatible, name)
               .AsLength()
           ?? Length.Zero;

    private static TextAnchor Anchor(OdfFile file, IReadOnlyList<OdfStyleReference> cascade)
    {
        OdfProperty alignment = file.Styles.ResolveProperty(
            cascade, OdfPropertyKind.Graphic, OdfNamespaces.Draw, "textarea-vertical-align");

        if (alignment.Is("middle")) return TextAnchor.Middle;
        if (alignment.Is("bottom")) return TextAnchor.Bottom;
        return TextAnchor.Top;
    }

    private static SlideParagraph Paragraph(
        OdfFile file, XElement paragraph, IReadOnlyList<OdfStyleReference> cascade)
    {
        StringBuilder text = new();
        List<SlideTextRun> runs = [];

        Collect(file, paragraph, cascade, text, runs);

        if (runs.Count == 0)
        {
            // An empty paragraph is still a line, as tall as the text that would go on it.
            runs.Add(Run(file, cascade, 0, 0));
        }

        return new SlideParagraph(
            text.ToString(),
            runs,
            Alignment(file, cascade),
            Spacing(file, cascade, "margin-top"),
            Spacing(file, cascade, "margin-bottom"),
            LineSpacing(file, cascade),
            Spacing(file, cascade, "margin-left"),
            Spacing(file, cascade, "text-indent"),
            Language: null);
    }

    /// <summary>
    /// Walks a paragraph's children, appending text and one run per style change.
    /// </summary>
    /// <remarks>
    /// Spans nest, so the cascade grows as the walk descends and each nested span's formatting
    /// resolves through every span above it. Flattening to the innermost style instead would lose
    /// the bold of a bold span containing a coloured one.
    /// </remarks>
    private static void Collect(
        OdfFile file,
        XElement element,
        IReadOnlyList<OdfStyleReference> cascade,
        StringBuilder text,
        List<SlideTextRun> runs)
    {
        foreach (XNode node in element.Nodes())
        {
            if (node is XText literal)
            {
                if (literal.Value.Length == 0) continue;

                runs.Add(Run(file, cascade, text.Length, literal.Value.Length));
                text.Append(literal.Value);
                continue;
            }

            if (node is not XElement child) continue;
            if (child.Name.NamespaceName != OdfNamespaces.Text) continue;

            switch (child.Name.LocalName)
            {
                case "span":
                    Collect(
                        file,
                        child,
                        [.. cascade, new OdfStyleReference(
                            child.Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value,
                            OdfStyleFamily.Text)],
                        text,
                        runs);
                    break;

                case "s":
                    // A run of spaces, collapsed in the file and expanded here: text:c says how
                    // many, and its absence means one.
                    int count = int.TryParse(
                        child.Attribute(XName.Get("c", OdfNamespaces.Text))?.Value,
                        out int stated) ? Math.Clamp(stated, 1, 4096) : 1;

                    runs.Add(Run(file, cascade, text.Length, count));
                    text.Append(' ', count);
                    break;

                case "tab":
                    runs.Add(Run(file, cascade, text.Length, 1));
                    text.Append('\t');
                    break;

                case "line-break":
                    text.Append(LineSeparator);
                    break;

                default:
                    // A field, a bookmark, a note anchor: whatever text it carries is its own.
                    Collect(file, child, cascade, text, runs);
                    break;
            }
        }
    }

    private static SlideTextRun Run(
        OdfFile file, IReadOnlyList<OdfStyleReference> cascade, int start, int length)
    {
        OdfTextFormat format = OdfTextFormat.Resolve(file.Styles, cascade);

        return new SlideTextRun(
            start,
            length,
            Family(file, format.FontName),
            format.FontSize ?? DefaultSize,
            format.IsBold ? 700 : 400,
            format.IsItalic,
            format.Colour ?? Colour.Black,
            Escapement: Escaped(format.Position));
    }

    /// <summary>
    /// The rise and shrink an ODF text position asks for.
    /// </summary>
    /// <remarks>
    /// <c>style:text-position</c> can state both numbers and <see cref="OdfTextFormat"/> keeps
    /// only the direction, so this is LibreOffice's automatic pair: 58% of the size, raised or
    /// lowered by <c>0.8 × (100 − 58)</c> of it (<c>editeng/source/items/svxfont.cxx:85-91</c>,
    /// which is where <c>DFLT_ESC_SUPER</c>'s 33 comes from).
    /// </remarks>
    private static SlideEscapement Escaped(OdfTextPosition position) => position switch
    {
        OdfTextPosition.Superscript => new SlideEscapement(33, SlideEscapement.AutomaticProportion),
        OdfTextPosition.Subscript => new SlideEscapement(-33, SlideEscapement.AutomaticProportion),
        _ => SlideEscapement.None,
    };

    /// <summary>
    /// The family name a <c>style:font-name</c> refers to.
    /// </summary>
    /// <remarks>
    /// ODF names a <em>declaration</em> rather than a family, and the declaration's
    /// <c>svg:font-family</c> is a CSS-style list that may be quoted. Passing the declaration's
    /// own name to a font resolver works only when the two happen to coincide, which is common
    /// enough to hide the bug and not universal.
    /// </remarks>
    private static string? Family(OdfFile file, string? fontName)
    {
        if (fontName is null) return null;
        if (!file.Styles.FontFaces.TryGetValue(fontName, out OdfFontFace? face)) return fontName;

        string? family = face.FontFamily;
        if (string.IsNullOrEmpty(family)) return fontName;

        int comma = family.IndexOf(',', StringComparison.Ordinal);
        if (comma >= 0) family = family[..comma];

        return family.Trim().Trim('\'', '"');
    }

    /// <summary>
    /// A paragraph measurement stated as an <c>fo:</c> length, or zero.
    /// </summary>
    /// <remarks>
    /// The indents and the space around a paragraph, which ODF spells as margins on the paragraph
    /// itself. Not the ones an <em>outline</em> paragraph gets: those come from the list style its
    /// <c>text:list</c> names, which is a different resolution path and is the open item recorded
    /// in the TODO.
    /// </remarks>
    private static Length Spacing(
        OdfFile file, IReadOnlyList<OdfStyleReference> cascade, string name)
        => file.Styles.ResolveProperty(
               cascade, OdfPropertyKind.Paragraph, OdfNamespaces.FoCompatible, name)
               .AsLength()
           ?? Length.Zero;

    private static TextAlignment Alignment(OdfFile file, IReadOnlyList<OdfStyleReference> cascade)
    {
        OdfProperty alignment = file.Styles.ResolveProperty(
            cascade, OdfPropertyKind.Paragraph, OdfNamespaces.FoCompatible, "text-align");

        if (alignment.Is("center")) return TextAlignment.Centre;
        if (alignment.Is("end") || alignment.Is("right")) return TextAlignment.End;
        if (alignment.Is("justify")) return TextAlignment.Justify;
        return TextAlignment.Start;
    }

    private static LineSpacingRule LineSpacing(
        OdfFile file, IReadOnlyList<OdfStyleReference> cascade)
    {
        OdfProperty height = file.Styles.ResolveProperty(
            cascade, OdfPropertyKind.Paragraph, OdfNamespaces.FoCompatible, "line-height");

        if (height.AsPercentage() is { } proportion && proportion > 0)
            return LineSpacingRule.Multiple(proportion);

        if (height.AsLength() is { } exact && exact > Length.Zero)
            return LineSpacingRule.Exactly(exact);

        return LineSpacingRule.SingleSpaced;
    }
}
