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

        string? marker = ResolveMarker(
            LevelChain(properties, bodyListStyle, level, options), level, counters, counting);

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
            if (Drawing.Is(child, "r")) ReadRun(child, result, options);
            else if (Drawing.Is(child, "fld")) ReadRun(child, result, options);
            else if (Drawing.Is(child, "br")) result.Children.Add(new ContentRun { Text = "\n" });
        }

        return result;
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
    private static void ReadRun(XElement run, ContentParagraph paragraph, DrawingTextOptions options)
    {
        string? text = Drawing.Child(run, "t")?.Value;
        if (string.IsNullOrEmpty(text)) return;

        XElement? properties = Drawing.Child(run, "rPr");
        paragraph.Children.Add(new ContentRun
        {
            Text = text,
            Emphasis = EmphasisOf(properties),
            Language = Drawing.Attribute(properties, "lang"),
            HyperlinkTarget = HyperlinkOf(properties, options),
        });
    }

    private static RunEmphasis EmphasisOf(XElement? properties)
    {
        if (properties is null) return RunEmphasis.None;

        RunEmphasis emphasis = RunEmphasis.None;
        if (Drawing.Flag(properties, "b") == true) emphasis |= RunEmphasis.Bold;
        if (Drawing.Flag(properties, "i") == true) emphasis |= RunEmphasis.Italic;

        // u and strike are enumerations whose "none" member is the off state, and both are
        // written explicitly by LibreOffice's exporter on every run — so testing for presence
        // rather than for a value would mark every run struck through.
        if (Drawing.Attribute(properties, "u") is { Length: > 0 } and not "none")
            emphasis |= RunEmphasis.Underline;
        if (Drawing.Attribute(properties, "strike") is { Length: > 0 } and not "noStrike")
            emphasis |= RunEmphasis.Strikethrough;

        // baseline is a signed percentage of the em: positive raises, negative lowers.
        if (Drawing.Number(properties, "baseline") is { } baseline && baseline != 0)
            emphasis |= baseline > 0 ? RunEmphasis.Superscript : RunEmphasis.Subscript;

        return emphasis;
    }

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
        IEnumerable<XElement> chain, int level, int[] counters, bool[] counting)
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
                string? bullet = Drawing.Attribute(character, "char");
                return string.IsNullOrEmpty(bullet) ? null : OutlineNumbers.NormaliseBullet(bullet);
            }

            if (Drawing.Child(source, "buAutoNum") is { } autoNumber)
                return AutoNumber(autoNumber, level, counters, counting);
        }

        // Nothing in the chain said anything. The schema's default is no bullet, which is what
        // a plain text box gets and is why one does not come out bulleted.
        counting[level] = false;
        return null;
    }

    private static string AutoNumber(XElement autoNumber, int level, int[] counters, bool[] counting)
    {
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
