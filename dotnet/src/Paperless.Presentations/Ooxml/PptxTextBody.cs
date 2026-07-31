using System.Globalization;
using System.Text;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Numbering;
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

        // One counter and one "is this level numbering" flag per outline level, carried across
        // the whole body: a:buAutoNum numbers a *run* of paragraphs, and the run is broken by a
        // paragraph that draws a different kind of marker or none at all.
        int[] counters = new int[9];
        bool[] counting = new bool[9];

        List<SlideParagraph> paragraphs = [];
        foreach (XElement paragraph in Drawing.Children(body, "p"))
        {
            paragraphs.Add(
                Paragraph(paragraph, listStyle, theme, defaultTypeface, counters, counting));
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
        XElement paragraph,
        XElement? listStyle,
        DrawingTheme? theme,
        string? defaultTypeface,
        int[] counters,
        bool[] counting)
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
            Language(Drawing.Child(paragraph, "r")),
            Marker(
                paragraphProperties, levelStyle, theme, level, counters, counting,
                hasText: text.Length > 0));
    }

    /// <summary>
    /// The bullet a paragraph draws, or null when it draws none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The bullet elements are a choice: <c>a:buNone</c>, <c>a:buChar</c> or <c>a:buAutoNum</c>,
    /// and whichever the paragraph states settles it — so a paragraph with <c>a:buNone</c> has
    /// cancelled the bullet its level would have given it, and one that states nothing inherits.
    /// Only the paragraph's own properties and the body's own list style are consulted, which is
    /// the same rung the character properties reach; the layout and master rungs are the open
    /// item recorded in the TODO.
    /// </para>
    /// <para>
    /// A Private Use Area character is substituted for U+2022 the way extraction already does.
    /// Those code points are Wingdings and Symbol positions and mean nothing outside those fonts,
    /// which are not installed on the machines this runs on.
    /// </para>
    /// <para>
    /// <c>a:buAutoNum</c> is numbered by the counters the caller carries across the body, which
    /// is the same walk and the same arrays extraction uses — <see cref="DrawingTextBody"/> owns
    /// the arithmetic so that a nested list cannot be numbered two different ways by the two
    /// readers. <strong>An empty paragraph draws no marker and consumes no number:</strong> the
    /// blank line an author leaves between two items is still an <c>a:p</c> and still inherits
    /// the level's bullet, and counting it makes the next item jump from 2 to 4.
    /// </para>
    /// </remarks>
    private static SlideMarker? Marker(
        XElement? paragraphProperties,
        XElement? levelStyle,
        DrawingTheme? theme,
        int level,
        int[] counters,
        bool[] counting,
        bool hasText)
    {
        int slot = Math.Clamp(level, 0, counters.Length - 1);

        foreach (XElement? source in (XElement?[])[paragraphProperties, levelStyle])
        {
            if (source is null) continue;

            if (Drawing.Child(source, "buNone") is not null)
            {
                counting[slot] = false;
                return null;
            }

            if (Drawing.Child(source, "buAutoNum") is { } number)
            {
                if (!hasText) return null;

                return Marked(
                    DrawingTextBody.AutoNumber(number, slot, counters, counting),
                    source, paragraphProperties, levelStyle, theme);
            }

            if (Drawing.Child(source, "buChar") is not { } bullet) continue;

            counting[slot] = false;

            string? character = Drawing.Attribute(bullet, "char");
            if (string.IsNullOrEmpty(character)) return null;

            return Marked(
                OutlineNumbers.NormaliseBullet(character),
                source, paragraphProperties, levelStyle, theme);
        }

        counting[slot] = false;
        return null;
    }

    /// <summary>A marker's text with the font, size and colour the chain gives it.</summary>
    private static SlideMarker Marked(
        string text,
        XElement source,
        XElement? paragraphProperties,
        XElement? levelStyle,
        DrawingTheme? theme)
        => new(
                text,
                Drawing.Attribute(Bullet(source, "buFont", paragraphProperties, levelStyle), "typeface"),
                Drawing.Number(
                    Bullet(source, "buSzPct", paragraphProperties, levelStyle), "val") is { } percent
                    && percent > 0
                    ? percent / 100000.0
                    : 1.0,
                ColourIn(Bullet(source, "buClr", paragraphProperties, levelStyle), theme));

    /// <summary>
    /// One of the bullet's satellite properties, from wherever in the chain states it.
    /// </summary>
    /// <remarks>
    /// Separately from the bullet character itself, because a paragraph routinely states the
    /// character and leaves the font, size and colour to its level — and because the three are
    /// each their own element rather than attributes of the bullet.
    /// </remarks>
    private static XElement? Bullet(
        XElement source, string name, XElement? paragraphProperties, XElement? levelStyle)
        => Drawing.Child(source, name)
           ?? Drawing.Child(paragraphProperties, name)
           ?? Drawing.Child(levelStyle, name);

    /// <summary>
    /// The colour a wrapper element holds directly, rather than through an <c>a:solidFill</c>.
    /// </summary>
    /// <remarks>
    /// <c>a:buClr</c> is one: it holds the colour reference outright, where an <c>a:rPr</c> wraps
    /// it in a fill. Two readers rather than one because the two shapes really do differ, and a
    /// reader looking for a fill inside a <c>buClr</c> finds nothing and draws a black bullet on a
    /// deck that asked for a coloured one.
    /// </remarks>
    private static Colour? ColourIn(XElement? wrapper, DrawingTheme? theme)
    {
        if (wrapper is null) return null;

        foreach (XElement child in wrapper.Elements())
        {
            if (DrawingColour.Read(child)?.Resolve(theme) is { } colour) return colour;
        }

        return null;
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
