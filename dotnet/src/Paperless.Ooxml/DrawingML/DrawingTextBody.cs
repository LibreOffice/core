using System.Text;
using System.Xml.Linq;
using Paperless.Core.Extraction;
using Paperless.Core.Numbering;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// Options a caller supplies when reading a DrawingML text body.
/// </summary>
/// <remarks>
/// Everything here is something only the surrounding vocabulary knows. A text body is the same
/// in a deck, a spreadsheet drawing and a Word shape; where the properties it does <em>not</em>
/// state come from is not, and neither is how an <c>r:id</c> resolves — the relationship is
/// declared by the part that holds the shape, which this reader never sees.
/// </remarks>
public sealed record DrawingTextOptions
{
    /// <summary>Read nothing from outside the body itself.</summary>
    public static readonly DrawingTextOptions Default = new();

    /// <summary>
    /// Supplies the <c>a:lvlNpPr</c> elements a paragraph at a given zero-based outline level
    /// inherits, most specific first.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A delegate rather than a resolved property set, because DrawingML inheritance is
    /// <em>per level</em> and per property: a paragraph at level 2 takes its bullet from the
    /// first source in the chain that states one at level 2, which may not be the source its
    /// indent comes from. Handing over the chain lets that be resolved property by property
    /// here, once, instead of each caller pre-merging and losing the distinction.
    /// </para>
    /// <para>
    /// The body's own <c>a:lstStyle</c> is <em>not</em> part of this; it sits inside the body
    /// and is read directly, above whatever this returns.
    /// </para>
    /// </remarks>
    public Func<int, IReadOnlyList<XElement>>? InheritedLevelProperties { get; init; }

    /// <summary>
    /// Resolves an <c>r:id</c> from an <c>a:hlinkClick</c> to a target, or null when the
    /// package declares no such relationship.
    /// </summary>
    public Func<string, string?>? ResolveHyperlink { get; init; }

    /// <summary>
    /// The name recorded on every paragraph read, so a caller can tell which shape or
    /// placeholder a paragraph came from. Null records none.
    /// </summary>
    public string? StyleName { get; init; }

    /// <summary>
    /// The theme in force, for the properties that cannot be resolved without one.
    /// </summary>
    /// <remarks>
    /// Nothing extraction reports needs it — a bullet and an emphasis flag are stated literally.
    /// A colour and a typeface are not: both are normally stated as a reference into the theme,
    /// so <see cref="DrawingCharacterStyle"/> cannot answer at all without this.
    /// </remarks>
    public DrawingTheme? Theme { get; init; }

    /// <summary>
    /// The character properties the shape's own style contributes, or null when it has none.
    /// </summary>
    /// <remarks>
    /// The rung of the inheritance chain with no element inside the text body: it comes from the
    /// shape's <c>p:style/a:fontRef</c> and sits between the body's list style and everything the
    /// shape inherits. See <see cref="DrawingCharacterStyle.Resolve"/> for why that position is
    /// not a detail.
    /// </remarks>
    public DrawingCharacterStyle? ShapeTextStyle { get; init; }
}

/// <summary>
/// Reads a DrawingML text body — <c>a:txBody</c>, <c>p:txBody</c>, <c>xdr:txBody</c> — into
/// content paragraphs.
/// </summary>
/// <remarks>
/// <para>
/// One reader for all three OOXML families. A text body is pure DrawingML wherever it appears:
/// the element that <em>wraps</em> it is namespaced per family (<c>p:txBody</c> on a slide,
/// <c>xdr:txBody</c> in a spreadsheet drawing, <c>wps:txbx</c> in a Word shape) but everything
/// inside it — <c>a:bodyPr</c>, <c>a:lstStyle</c>, <c>a:p</c>, <c>a:r</c> — is identical, so
/// writing it once is what stops three readers from disagreeing about the same markup.
/// </para>
/// <para>
/// Extraction only. Body properties that exist purely to place text — insets, anchoring,
/// <c>normAutofit</c>'s font scale, text warping — are deliberately not read here; they change
/// where the text sits, not what it says.
/// </para>
/// </remarks>
public static class DrawingTextBody
{
    /// <summary>
    /// Reads a text body's paragraphs, appending them to a target node.
    /// </summary>
    /// <param name="textBody">The <c>txBody</c> element, in whatever namespace wraps it.</param>
    /// <param name="target">The node the paragraphs are appended to.</param>
    /// <param name="options">Inheritance and relationship resolution; null reads the body alone.</param>
    public static void Read(XElement textBody, ContentNode target, DrawingTextOptions? options = null)
    {
        ArgumentNullException.ThrowIfNull(textBody);
        ArgumentNullException.ThrowIfNull(target);
        options ??= DrawingTextOptions.Default;

        XElement? listStyle = Drawing.Child(textBody, "lstStyle");

        // One counter per outline level, live for the length of this body. DrawingML has no
        // element that restarts numbering; a new body is the restart, which is why the state
        // is local to this call rather than held by the caller.
        int[] counters = new int[9];
        bool[] counting = new bool[9];

        foreach (XElement paragraph in Drawing.Children(textBody, "p"))
            target.Children.Add(ReadParagraph(paragraph, listStyle, options, counters, counting));
    }

    /// <summary>
    /// True when a text body holds no text at all.
    /// </summary>
    /// <remarks>
    /// The distinction matters on a slide: an empty placeholder is one the author left alone,
    /// and it must not pick up the prompt text its layout carries. LibreOffice draws the same
    /// line — <c>TextBody::isEmpty</c>, <c>oox/source/drawingml/textbody.cxx</c> — and treats a
    /// single empty run as empty, which a naive "has any <c>a:r</c>" test would not.
    /// </remarks>
    public static bool IsEmpty(XElement? textBody)
    {
        if (textBody is null) return true;

        foreach (XElement paragraph in Drawing.Children(textBody, "p"))
        {
            foreach (XElement child in paragraph.Elements())
            {
                if (Drawing.Is(child, "r") || Drawing.Is(child, "fld"))
                {
                    if (Drawing.Child(child, "t")?.Value.Length > 0) return false;
                }
                else if (Drawing.Is(child, "br"))
                {
                    return false;
                }
            }
        }
        return true;
    }

    /// <summary>
    /// A text body's plain text, paragraphs separated by newlines.
    /// </summary>
    /// <remarks>
    /// For the places a caller wants a string rather than a tree — a shape's title, a table
    /// cell's value, a diagnostic message — without building content nodes to throw away.
    /// </remarks>
    public static string Text(XElement? textBody)
    {
        if (textBody is null) return string.Empty;

        StringBuilder text = new();
        foreach (XElement paragraph in Drawing.Children(textBody, "p"))
        {
            if (text.Length > 0) text.Append('\n');
            foreach (XElement child in paragraph.Elements())
            {
                if (Drawing.Is(child, "br")) text.Append('\n');
                else if (Drawing.Is(child, "r") || Drawing.Is(child, "fld"))
                    text.Append(Drawing.Child(child, "t")?.Value);
            }
        }
        return text.ToString();
    }

    private static ContentParagraph ReadParagraph(
        XElement paragraph,
        XElement? bodyListStyle,
        DrawingTextOptions options,
        int[] counters,
        bool[] counting)
    {
        XElement? properties = Drawing.Child(paragraph, "pPr");

        // a:pPr/@lvl is zero-based and defaults to 0. Levels beyond the ninth do not exist in
        // the schema; a file stating one is clamped rather than refused.
        int level = Math.Clamp(Drawing.Number(properties, "lvl") ?? 0, 0, 8);

        // Whether the paragraph has any text at all decides whether it draws its bullet, so it
        // has to be known before the marker is resolved rather than after the runs are read.
        bool hasText = HasText(paragraph);

        // Materialised because the chain is now walked twice — once for the bullet and once for
        // the character defaults — and the caller's part of it costs a search of the master's
        // shape tree.
        List<XElement> chain = [.. LevelChain(properties, bodyListStyle, level, options)];

        string? marker = ResolveMarker(chain, level, counters, counting, hasText);

        // Every <c>a:defRPr</c> the chain offers, most specific first. A run states only what
        // differs from these, and on a PowerPoint-authored deck it commonly states nothing.
        List<XElement> characterDefaults =
            [.. chain.Select(source => Drawing.Child(source, "defRPr")).OfType<XElement>()];

        ContentParagraph result = new()
        {
            StyleName = options.StyleName,
            // A DrawingML paragraph is a list item exactly when something in its chain gives it
            // a bullet or a number. Its outline level exists either way — every paragraph has
            // one — so reporting the level alone would make every line of every text box a
            // list item, which is not what a reader sees.
            ListLevel = marker is null ? null : level,
            ListMarker = marker,
        };

        foreach (XElement child in paragraph.Elements())
        {
            if (Drawing.Is(child, "r")) ReadRun(child, result, options, characterDefaults);
            else if (Drawing.Is(child, "fld")) ReadRun(child, result, options, characterDefaults);
            else if (Drawing.Is(child, "br")) result.Children.Add(new ContentRun { Text = "\n" });
        }

        return result;
    }

    /// <summary>
    /// True when a paragraph holds any text.
    /// </summary>
    /// <remarks>
    /// The blank line an author leaves between two bulleted points is still an <c>a:p</c>, and it
    /// still inherits the level's bullet — but neither PowerPoint nor Impress draws one on it.
    /// Measured against LibreOffice's own layout expectations for
    /// <c>sd/qa/unit/data/pptx/NumberedList-12ab-ab-34.pptx</c>
    /// (<c>sd/qa/unit/layout-tests.cxx:270-292</c>), whose trailing empty item is expected to
    /// produce no text at all: emitting the marker gave a stray "a." after the last real item.
    /// </remarks>
    private static bool HasText(XElement paragraph)
    {
        foreach (XElement child in paragraph.Elements())
        {
            if (Drawing.Is(child, "br")) return true;
            if ((Drawing.Is(child, "r") || Drawing.Is(child, "fld"))
                && Drawing.Child(child, "t")?.Value.Length > 0)
                return true;
        }
        return false;
    }

    /// <summary>
    /// Reads one run. <c>a:fld</c> is read like <c>a:r</c> deliberately.
    /// </summary>
    /// <remarks>
    /// A field carries its last-rendered value in an <c>a:t</c> exactly as a run does, and that
    /// cached value is what a reader saw. Recomputing it — a slide number, a date — would report
    /// something the file does not say and that a reference renderer, which uses the cache, does
    /// not show either.
    /// </remarks>
    private static void ReadRun(
        XElement run,
        ContentParagraph paragraph,
        DrawingTextOptions options,
        IReadOnlyList<XElement> characterDefaults)
    {
        string? text = Drawing.Child(run, "t")?.Value;
        if (string.IsNullOrEmpty(text)) return;

        XElement? properties = Drawing.Child(run, "rPr");
        paragraph.Children.Add(new ContentRun
        {
            Text = text,
            Emphasis = EmphasisOf(properties, characterDefaults),
            Language = Stated(properties, characterDefaults, "lang"),
            HyperlinkTarget = HyperlinkOf(properties, options),
        });
    }

    /// <summary>
    /// A run's effective emphasis: its own <c>a:rPr</c> over the <c>a:defRPr</c> chain.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Resolved attribute by attribute rather than element by element, because a run that states
    /// <c>b="1"</c> and nothing else has not cancelled the italic its level's <c>defRPr</c>
    /// gives it. LibreOffice does the same with <c>assignUsed</c>, applying the master list
    /// style, then the body's, then the paragraph's <c>defRPr</c>, then the run
    /// (<c>oox/source/drawingml/textparagraph.cxx:51-67</c> and
    /// <c>textrun.cxx:80</c>) — every step overwriting only what it sets.
    /// </para>
    /// <para>
    /// The step this cannot do is the one between the master's list style and the body's: the
    /// shape's own text style, which comes from the theme's <c>txDef</c> and the shape style's
    /// <c>a:fontRef</c>. That needs theme resolution, and neither of the properties it carries
    /// — typeface and colour — is anything extraction reports.
    /// </para>
    /// </remarks>
    private static RunEmphasis EmphasisOf(
        XElement? properties, IReadOnlyList<XElement> characterDefaults)
    {
        RunEmphasis emphasis = RunEmphasis.None;

        if (Flag(properties, characterDefaults, "b") == true) emphasis |= RunEmphasis.Bold;
        if (Flag(properties, characterDefaults, "i") == true) emphasis |= RunEmphasis.Italic;

        // u and strike are enumerations whose "none" member is the off state, and both are
        // written explicitly by LibreOffice's exporter on every run — so testing for presence
        // rather than for a value would mark every run struck through.
        if (Stated(properties, characterDefaults, "u") is { Length: > 0 } and not "none")
            emphasis |= RunEmphasis.Underline;
        if (Stated(properties, characterDefaults, "strike") is { Length: > 0 } and not "noStrike")
            emphasis |= RunEmphasis.Strikethrough;

        // baseline is a signed percentage of the em: positive raises, negative lowers.
        if (Stated(properties, characterDefaults, "baseline") is { } text
            && int.TryParse(text, System.Globalization.NumberStyles.Integer,
                            System.Globalization.CultureInfo.InvariantCulture, out int baseline)
            && baseline != 0)
        {
            emphasis |= baseline > 0 ? RunEmphasis.Superscript : RunEmphasis.Subscript;
        }

        return emphasis;
    }

    /// <summary>
    /// The value of an attribute on the run's own properties, or on the nearest
    /// <c>a:defRPr</c> that states one.
    /// </summary>
    private static string? Stated(
        XElement? properties, IReadOnlyList<XElement> characterDefaults, string name)
    {
        if (Drawing.Attribute(properties, name) is { } own) return own;

        foreach (XElement source in characterDefaults)
        {
            if (Drawing.Attribute(source, name) is { } inherited) return inherited;
        }

        return null;
    }

    /// <summary>The same, read as an ST_Boolean.</summary>
    private static bool? Flag(
        XElement? properties, IReadOnlyList<XElement> characterDefaults, string name)
        => Stated(properties, characterDefaults, name) switch
        {
            "1" or "true" or "on" => true,
            "0" or "false" or "off" => false,
            _ => null,
        };

    private static string? HyperlinkOf(XElement? properties, DrawingTextOptions options)
    {
        XElement? link = Drawing.Child(properties, "hlinkClick");
        if (link is null) return null;

        // An action with no r:id is an internal jump — "go to the next slide" — which has no
        // target the content tree can express, so it is reported as the action itself rather
        // than dropped.
        string? relationshipId = Drawing.RelationshipId(link);
        if (relationshipId is null || options.ResolveHyperlink is null)
            return Drawing.Attribute(link, "action");

        return options.ResolveHyperlink(relationshipId) ?? Drawing.Attribute(link, "action");
    }

    /// <summary>
    /// The chain of property sources for one paragraph, most specific first.
    /// </summary>
    /// <remarks>
    /// The paragraph's own <c>a:pPr</c>, then the body's <c>a:lstStyle</c> entry for its level,
    /// then whatever the caller inherits from — a layout placeholder, a master placeholder, the
    /// master's <c>p:txStyles</c>, the presentation's <c>p:defaultTextStyle</c>. Deliberately
    /// lazy in the caller's part: most paragraphs state their own bullet and never ask.
    /// </remarks>
    private static IEnumerable<XElement> LevelChain(
        XElement? properties, XElement? bodyListStyle, int level, DrawingTextOptions options)
    {
        if (properties is not null) yield return properties;

        if (LevelProperties(bodyListStyle, level) is { } own) yield return own;

        if (options.InheritedLevelProperties is null) yield break;
        foreach (XElement inherited in options.InheritedLevelProperties(level)) yield return inherited;
    }

    /// <summary>
    /// The <c>a:lvlNpPr</c> of an <c>a:lstStyle</c> for a zero-based level.
    /// </summary>
    /// <remarks>
    /// The elements are named for a <em>one</em>-based level — level 0 is <c>lvl1pPr</c> — which
    /// is the off-by-one that makes a nested bullet inherit the wrong level's properties.
    /// </remarks>
    public static XElement? LevelProperties(XElement? listStyle, int level)
        => listStyle is null || level is < 0 or > 8
            ? null
            : Drawing.Child(listStyle, $"lvl{level + 1}pPr");

    /// <summary>
    /// The marker a paragraph draws, or null when it draws none.
    /// </summary>
    /// <remarks>
    /// The first source in the chain that states <em>any</em> of the three bullet elements
    /// settles the question, including <c>a:buNone</c> — which is the point of writing one.
    /// Continuing past a <c>buNone</c> to a source that does specify a bullet would put bullets
    /// back on every title, because a master's title placeholder is exactly a <c>buNone</c> over
    /// a body style that bullets.
    /// </remarks>
    private static string? ResolveMarker(
        IEnumerable<XElement> chain, int level, int[] counters, bool[] counting, bool hasText)
    {
        foreach (XElement source in chain)
        {
            if (Drawing.Child(source, "buNone") is not null)
            {
                counting[level] = false;
                return null;
            }

            if (Drawing.Child(source, "buChar") is { } character)
            {
                counting[level] = false;
                if (!hasText) return null;

                string? bullet = Drawing.Attribute(character, "char");
                return string.IsNullOrEmpty(bullet) ? null : OutlineNumbers.NormaliseBullet(bullet);
            }

            if (Drawing.Child(source, "buAutoNum") is { } autoNumber)
            {
                // An empty item does not consume a number either, so the counter is left alone
                // rather than advanced and hidden — otherwise a blank line between two items
                // makes the second jump from 2 to 4.
                return hasText ? AutoNumber(autoNumber, level, counters, counting) : null;
            }
        }

        // Nothing in the chain said anything. The schema's default is no bullet, which is what
        // a plain text box gets and is why one does not come out bulleted.
        counting[level] = false;
        return null;
    }

    /// <summary>
    /// The next number in an <c>a:buAutoNum</c> run, advancing the counters.
    /// </summary>
    /// <remarks>
    /// Public because rendering needs the same answer from a second walk over the same tree, and
    /// two implementations of "what number is this item" would drift the moment one of them met a
    /// nested list. The caller owns the two arrays — one per outline level, nine of each — so the
    /// state belongs to the text body being read rather than to this type.
    /// </remarks>
    /// <param name="autoNumber">The <c>a:buAutoNum</c> element.</param>
    /// <param name="level">The paragraph's zero-based outline level.</param>
    /// <param name="counters">The current number at each level.</param>
    /// <param name="counting">Whether each level is inside a run of numbering.</param>
    public static string AutoNumber(
        XElement autoNumber, int level, int[] counters, bool[] counting)
    {
        ArgumentNullException.ThrowIfNull(autoNumber);
        ArgumentNullException.ThrowIfNull(counters);
        ArgumentNullException.ThrowIfNull(counting);

        if (counting[level])
        {
            counters[level]++;
        }
        else
        {
            // startAt applies when the run of numbering begins, not to every item in it.
            counters[level] = Drawing.Number(autoNumber, "startAt") ?? 1;
            counting[level] = true;
        }

        // A nested list restarts each time its parent advances, so anything deeper than this
        // level stops counting the moment this one produces a number.
        for (int deeper = level + 1; deeper < counting.Length; deeper++) counting[deeper] = false;

        return Format(Drawing.Attribute(autoNumber, "type"), counters[level]);
    }

    /// <summary>
    /// Renders an <c>ST_TextAutonumberScheme</c> value.
    /// </summary>
    /// <remarks>
    /// The scheme names are a naming convention rather than an enumeration to memorise: an
    /// alphabet (<c>arabic</c>, <c>alphaLc</c>, <c>alphaUc</c>, <c>romanLc</c>, <c>romanUc</c>)
    /// followed by a suffix (<c>Period</c>, <c>ParenR</c>, <c>ParenBoth</c>, <c>Plain</c>).
    /// Decomposing them is what keeps the forty-odd members — including the CJK, Hebrew, Hindi
    /// and Thai ones LibreOffice leaves unhandled
    /// (<c>oox/source/drawingml/textparagraphproperties.cxx:150-160</c>) — from each needing a
    /// case of their own; an alphabet this does not know still gets its punctuation right.
    /// </remarks>
    private static string Format(string? scheme, int value)
    {
        scheme ??= string.Empty;

        string numeral =
            scheme.StartsWith("alphaLc", StringComparison.Ordinal)
                ? OutlineNumbers.Alphabetic(value, upperCase: false, synchronised: true)
                : scheme.StartsWith("alphaUc", StringComparison.Ordinal)
                    ? OutlineNumbers.Alphabetic(value, upperCase: true, synchronised: true)
                    : scheme.StartsWith("romanLc", StringComparison.Ordinal)
                        ? OutlineNumbers.Roman(value, upperCase: false)
                        : scheme.StartsWith("romanUc", StringComparison.Ordinal)
                            ? OutlineNumbers.Roman(value, upperCase: true)
                            : OutlineNumbers.Digits(value);

        if (scheme.EndsWith("ParenBoth", StringComparison.Ordinal)) return $"({numeral})";
        if (scheme.EndsWith("ParenR", StringComparison.Ordinal)) return numeral + ")";
        if (scheme.EndsWith("Plain", StringComparison.Ordinal)) return numeral;
        if (scheme.EndsWith("Minus", StringComparison.Ordinal)) return numeral + "-";
        return numeral + ".";
    }
}
