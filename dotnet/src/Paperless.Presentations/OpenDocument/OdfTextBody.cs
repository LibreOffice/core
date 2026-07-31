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

        foreach (XElement paragraph in paragraphs)
        {
            OdfStyleReference style = new(
                paragraph.Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value,
                OdfStyleFamily.Paragraph);

            List<OdfStyleReference> cascade = [.. shapeCascade, style];

            if (file.Styles.ResolveProperty(
                    cascade, OdfPropertyKind.Paragraph,
                    OdfNamespaces.Style, "font-independent-line-spacing").AsBoolean() == true)
            {
                fontIndependent = true;
            }

            read.Add(Paragraph(file, paragraph, cascade));
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
            Length.Zero,
            Length.Zero,
            LineSpacing(file, cascade),
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
            format.Colour ?? Colour.Black);
    }

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
