using System.Globalization;
using System.Text;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml.DrawingML;
using Paperless.Presentations.Layout;
using Paperless.Text.Layout;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// Reads a DrawingML text body into the paragraphs slide layout takes.
/// </summary>
/// <remarks>
/// <para>
/// A second reading of the same <c>a:p</c>/<c>a:r</c> tree
/// <see cref="Paperless.Ooxml.DrawingML.DrawingTextBody"/> extracts from, keeping what extraction
/// throws away: the size, the typeface, the colour, the alignment and the spacing. Sharing one
/// walk would mean the content tree carrying a font size, which is a rendering property that
/// nothing indexing text has any use for.
/// </para>
/// <para>
/// <strong>What is resolved here, and what is not.</strong> A run's own <c>a:rPr</c> is read, and
/// so is the paragraph's <c>a:defRPr</c> and the body's own <c>a:lstStyle</c> entry for the
/// paragraph's level. What is <em>not</em> resolved is the rest of the inheritance chain — the
/// layout placeholder's list style, the master's, <c>p:txStyles</c> and the theme's
/// <c>txDef</c> — even though <see cref="PptxTextStyles"/> already walks exactly that chain for
/// bullets and emphasis. The reason is honesty about coverage rather than difficulty: every deck
/// in the corpus states size, face and colour on every run, so an implementation and a plausible
/// guess would be indistinguishable on everything there is to measure. Recorded in the TODO with
/// the measurement that would settle it.
/// </para>
/// </remarks>
internal static class PptxTextBody
{
    /// <summary>DrawingML's default run size when nothing in the chain states one: 18 pt.</summary>
    private const int DefaultSizeHundredthsOfPoint = 1800;

    /// <summary>
    /// The character an <c>a:br</c> becomes.
    /// </summary>
    /// <remarks>
    /// U+2028, whose UAX #14 class is a mandatory break, so the break iterator honours it with no
    /// special case in layout. A newline would break the same way and would read as the end of a
    /// paragraph to anything scanning the text later, which a break inside one is not.
    /// </remarks>
    private const char LineSeparator = '\u2028';

    /// <summary>Reads a <c>p:txBody</c> or <c>a:txBody</c>.</summary>
    /// <param name="body">The text body element.</param>
    /// <param name="theme">The theme, for themed run colours.</param>
    /// <param name="defaultTypeface">The typeface a run that names none falls back to.</param>
    public static SlideTextBody Read(
        XElement body, DrawingTheme? theme = null, string? defaultTypeface = null)
    {
        ArgumentNullException.ThrowIfNull(body);

        XElement? properties = Drawing.Child(body, "bodyPr");
        XElement? listStyle = Drawing.Child(body, "lstStyle");

        List<SlideParagraph> paragraphs = [];
        foreach (XElement paragraph in Drawing.Children(body, "p"))
        {
            paragraphs.Add(Paragraph(paragraph, listStyle, theme, defaultTypeface));
        }

        XElement? autofit = Drawing.Child(properties, "normAutofit");

        return new SlideTextBody
        {
            Paragraphs = paragraphs,
            Insets = Insets(properties),
            Anchor = Anchor(Drawing.Attribute(properties, "anchor")),
            Wraps = Drawing.Attribute(properties, "wrap") != "none",
            FontScale = Thousandth(autofit, "fontScale", 1.0),
            LineSpaceReduction = Thousandth(autofit, "lnSpcReduction", 0.0),
        };
    }

    /// <summary>
    /// The four text insets, each defaulting to DrawingML's own default rather than to zero.
    /// </summary>
    /// <remarks>
    /// 91440 EMU across and 45720 down — a tenth and a twentieth of an inch. A body that states
    /// none gets exactly those, so defaulting them to zero moves every line of every unstated
    /// text box 7.2 pt left and 3.6 pt up. Measured on <c>shape-geometry.pptx</c> slide 3, whose
    /// second box states them explicitly and whose first states zero: LibreOffice draws the two
    /// pens 7.2 pt apart.
    /// </remarks>
    private static Margins Insets(XElement? properties) => new(
        Length.FromEmu(Emu(properties, "lIns", 91440)),
        Length.FromEmu(Emu(properties, "tIns", 45720)),
        Length.FromEmu(Emu(properties, "rIns", 91440)),
        Length.FromEmu(Emu(properties, "bIns", 45720)));

    private static TextAnchor Anchor(string? anchor) => anchor switch
    {
        "ctr" => TextAnchor.Middle,
        "b" => TextAnchor.Bottom,
        _ => TextAnchor.Top,
    };

    private static SlideParagraph Paragraph(
        XElement paragraph, XElement? listStyle, DrawingTheme? theme, string? defaultTypeface)
    {
        XElement? paragraphProperties = Drawing.Child(paragraph, "pPr");
        int level = Drawing.Number(paragraphProperties, "lvl") ?? 0;

        // The body's own list style for this level, which is the one rung of the inheritance
        // chain that lives in the same part and needs nothing resolved to reach it.
        XElement? levelStyle = LevelStyle(listStyle, level);
        XElement?[] defaults =
        [
            Drawing.Child(paragraphProperties, "defRPr"),
            Drawing.Child(levelStyle, "defRPr"),
        ];

        StringBuilder text = new();
        List<SlideTextRun> runs = [];

        foreach (XElement child in paragraph.Elements())
        {
            if (Drawing.Is(child, "r"))
            {
                string content = Drawing.Child(child, "t")?.Value ?? string.Empty;
                if (content.Length == 0) continue;

                runs.Add(Run(
                    Drawing.Child(child, "rPr"), defaults, text.Length, content.Length,
                    theme, defaultTypeface));
                text.Append(content);
            }
            else if (Drawing.Is(child, "br"))
            {
                text.Append(LineSeparator);
            }
            else if (Drawing.Is(child, "fld"))
            {
                // The cached value, not a recomputed one: what the file says a reader saw is what
                // a reference renderer draws, and recomputing a slide number would disagree with
                // it on any deck whose fields are stale.
                string content = Drawing.Child(child, "t")?.Value ?? string.Empty;
                if (content.Length == 0) continue;

                runs.Add(Run(
                    Drawing.Child(child, "rPr"), defaults, text.Length, content.Length,
                    theme, defaultTypeface));
                text.Append(content);
            }
        }

        if (runs.Count == 0)
        {
            // An empty paragraph is still a line, and it is as tall as the text that would go on
            // it — which is what a:endParaRPr records and the only thing it is for.
            runs.Add(Run(
                Drawing.Child(paragraph, "endParaRPr"), defaults, 0, 0, theme, defaultTypeface));
        }

        return new SlideParagraph(
            text.ToString(),
            runs,
            Alignment(Drawing.Attribute(paragraphProperties, "algn")),
            Spacing(Drawing.Child(paragraphProperties, "spcBef")),
            Spacing(Drawing.Child(paragraphProperties, "spcAft")),
            LineSpacing(Drawing.Child(paragraphProperties, "lnSpc")),
            Length.FromEmu(Emu(paragraphProperties, "marL", 0)),
            Length.FromEmu(Emu(paragraphProperties, "indent", 0)),
            Language(Drawing.Child(paragraph, "r")));
    }

    private static string? Language(XElement? run)
        => Drawing.Attribute(Drawing.Child(run, "rPr"), "lang");

    private static SlideTextRun Run(
        XElement? runProperties,
        XElement?[] defaults,
        int start,
        int length,
        DrawingTheme? theme,
        string? defaultTypeface)
    {
        int size = First(runProperties, defaults, element => Drawing.Number(element, "sz"))
                   ?? DefaultSizeHundredthsOfPoint;

        bool bold = First(runProperties, defaults, element => Drawing.Flag(element, "b")) ?? false;
        bool italic = First(runProperties, defaults, element => Drawing.Flag(element, "i")) ?? false;

        string? typeface = First(
            runProperties, defaults,
            element => Drawing.Attribute(Drawing.Child(element, "latin"), "typeface"));

        Colour? colour = First(
            runProperties, defaults, element => SolidColour(element, theme));

        return new SlideTextRun(
            start,
            length,
            typeface ?? defaultTypeface,
            Length.FromEmu(size * Length.EmuPerPoint / 100),
            bold ? 700 : 400,
            italic,
            colour ?? Colour.Black);
    }

    /// <summary>
    /// The first of the run's own properties and the defaults behind it that states something.
    /// </summary>
    /// <remarks>
    /// Attribute by attribute rather than element by element, which is what LibreOffice's
    /// <c>assignUsed</c> does (<c>oox/source/drawingml/textparagraph.cxx:51-67</c>): a run stating
    /// <c>b="1"</c> and nothing else has not cancelled the size its level's <c>defRPr</c> gives
    /// it. Merging whole property sets gives the right answer on every run that states everything
    /// — which is every run LibreOffice writes — and the wrong one everywhere it matters.
    /// </remarks>
    private static T? First<T>(XElement? own, XElement?[] defaults, Func<XElement, T?> read)
    {
        if (own is not null && read(own) is { } fromRun) return fromRun;

        foreach (XElement? source in defaults)
        {
            if (source is not null && read(source) is { } value) return value;
        }

        return default;
    }

    private static Colour? SolidColour(XElement properties, DrawingTheme? theme)
    {
        XElement? solid = Drawing.Child(properties, "solidFill");
        if (solid is null) return null;

        foreach (XElement child in solid.Elements())
        {
            if (DrawingColour.Read(child)?.Resolve(theme) is { } colour) return colour;
        }

        return null;
    }

    private static XElement? LevelStyle(XElement? listStyle, int level)
        => Drawing.Child(listStyle, $"lvl{Math.Clamp(level, 0, 8) + 1}pPr");

    private static TextAlignment Alignment(string? alignment) => alignment switch
    {
        "ctr" => TextAlignment.Centre,
        "r" => TextAlignment.End,
        "just" or "justLow" => TextAlignment.Justify,
        "dist" or "thaiDist" => TextAlignment.Distribute,
        _ => TextAlignment.Start,
    };

    /// <summary>
    /// A <c>a:spcBef</c>/<c>a:spcAft</c> value, which is either points or a percentage.
    /// </summary>
    /// <remarks>
    /// Only <c>a:spcPts</c> is honoured, in hundredths of a point. <c>a:spcPct</c> is a percentage
    /// of the line height, which is not known until the paragraph's runs are — so resolving it
    /// belongs with the line heights rather than here, and it is recorded in the TODO rather than
    /// approximated against a size the paragraph may not use.
    /// </remarks>
    private static Length Spacing(XElement? spacing)
    {
        int? points = Drawing.Number(Drawing.Child(spacing, "spcPts"), "val");
        return points is { } value && value > 0
            ? Length.FromEmu(value * Length.EmuPerPoint / 100)
            : Length.Zero;
    }

    /// <summary>A <c>a:lnSpc</c>, as a percentage of the line height or as an exact height.</summary>
    private static LineSpacingRule LineSpacing(XElement? spacing)
    {
        if (Drawing.Number(Drawing.Child(spacing, "spcPct"), "val") is { } percent && percent > 0)
            return LineSpacingRule.Multiple(percent / 100000.0);

        if (Drawing.Number(Drawing.Child(spacing, "spcPts"), "val") is { } points && points > 0)
            return LineSpacingRule.Exactly(Length.FromEmu(points * Length.EmuPerPoint / 100));

        return LineSpacingRule.SingleSpaced;
    }

    private static double Thousandth(XElement? element, string attribute, double whenAbsent)
        => Drawing.Number(element, attribute) is { } value && value > 0
            ? value / 100000.0
            : whenAbsent;

    private static long Emu(XElement? element, string attribute, long whenAbsent)
        => long.TryParse(
            Drawing.Attribute(element, attribute), NumberStyles.Integer,
            CultureInfo.InvariantCulture, out long value)
            ? value
            : whenAbsent;
}
