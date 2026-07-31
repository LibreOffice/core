using System.Globalization;
using System.Xml.Linq;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// A shape's <c>p:ph</c> identity: what kind of placeholder it is and which one.
/// </summary>
/// <param name="Type">The resolved placeholder type token, never empty.</param>
/// <param name="Index">
/// The <c>idx</c>, or null when the shape states none. Null is not zero: a placeholder with no
/// index matches a layout placeholder with no index, and both differ from index 0.
/// </param>
internal readonly record struct PptxPlaceholder(string Type, int? Index)
{
    /// <summary>
    /// The type a <c>p:ph</c> with no <c>type</c> attribute takes.
    /// </summary>
    /// <remarks>
    /// <strong>This is the trap.</strong> ECMA-376 gives <c>CT_Placeholder/@type</c> a default of
    /// <c>body</c>; LibreOffice reads it as <c>obj</c>
    /// (<c>oox/source/ppt/pptshapecontext.cxx:68</c>, <c>getToken(XML_type, XML_obj)</c>), and
    /// LibreOffice's own PPTX export writes a bare <c>&lt;p:ph/&gt;</c> for every outline
    /// placeholder it emits — so the disagreement is not hypothetical, it is in the first deck
    /// you round-trip through Impress. Following the specification here makes such a
    /// placeholder match a <c>body</c> on the layout, which mostly gives the same answer since
    /// <c>obj</c>'s second-choice type <em>is</em> <c>body</c>; following LibreOffice keeps the
    /// two readers agreeing on the type that is reported. LibreOffice wins, because the whole
    /// point of the exercise is to agree with the reference.
    /// </remarks>
    public const string DefaultType = "obj";

    /// <summary>
    /// The <c>idx</c> that means "do not look this placeholder up at all".
    /// </summary>
    /// <remarks>
    /// <c>4294967295</c> — <c>SAL_MAX_UINT32</c>, an unsigned -1
    /// (<c>oox/source/ppt/pptshapecontext.cxx:76-78</c>). Real files carry it, and reading it as
    /// a signed index gives -1, which then matches nothing and quietly works by accident; read
    /// as an unsigned it overflows an <c>int</c>. Named here so neither happens silently.
    /// </remarks>
    public const long UnmatchedIndex = 4294967295L;

    /// <summary>
    /// Reads a shape's placeholder identity, or null when the shape is not a placeholder.
    /// </summary>
    /// <param name="shape">A <c>p:sp</c>, <c>p:pic</c> or <c>p:graphicFrame</c>.</param>
    /// <param name="master">
    /// The tree to consult when the shape states an <c>idx</c> but no <c>type</c>, since the
    /// type is then whatever the placeholder of that index on the master is. Null skips it.
    /// </param>
    public static PptxPlaceholder? Read(XElement shape, XElement? master)
    {
        XElement? ph = Element(shape);
        if (ph is null) return null;

        string? type = Ppt.Attribute(ph, "type");
        int? index = null;

        string? rawIndex = Ppt.Attribute(ph, "idx");
        if (rawIndex is not null)
        {
            if (!long.TryParse(rawIndex, NumberStyles.Integer, CultureInfo.InvariantCulture,
                               out long parsed))
                return new PptxPlaceholder(type ?? DefaultType, null);

            // The sentinel means the shape is a placeholder that deliberately inherits nothing.
            if (parsed == UnmatchedIndex) return new PptxPlaceholder(type ?? DefaultType, null);

            index = (int)Math.Clamp(parsed, int.MinValue, int.MaxValue);

            // An idx with no type takes its type from the master's placeholder of that index —
            // which is how a slide can say "the second content box" without repeating what kind
            // of box the layout decided it was.
            if (type is null && master is not null
                && FindByIndex(index.Value, ShapesOf(master)) is { } byIndex
                && Element(byIndex) is { } masterPh
                && Ppt.Attribute(masterPh, "type") is { Length: > 0 } masterType)
            {
                type = masterType;
            }
        }

        return new PptxPlaceholder(type ?? DefaultType, index);
    }

    /// <summary>
    /// The <c>p:ph</c> a shape declares, or null.
    /// </summary>
    /// <remarks>
    /// The non-visual wrapper is named per shape kind — <c>p:nvSpPr</c>, <c>p:nvPicPr</c>,
    /// <c>p:nvGraphicFramePr</c>, <c>p:nvCxnSpPr</c> — but all of them hold a <c>p:nvPr</c>, so
    /// the search is for that rather than for four spellings of its parent.
    /// </remarks>
    public static XElement? Element(XElement shape)
    {
        ArgumentNullException.ThrowIfNull(shape);
        foreach (XElement child in shape.Elements())
        {
            if (Ppt.Child(child, "nvPr") is { } nonVisual) return Ppt.Child(nonVisual, "ph");
        }
        return null;
    }

    /// <summary>
    /// The two types a placeholder will match against, most preferred first.
    /// </summary>
    /// <remarks>
    /// Three types have a fallback because PowerPoint names the same slot differently on the
    /// slide and on the layout: a slide's <c>ctrTitle</c> is a layout's <c>title</c>, and both
    /// <c>subTitle</c> and <c>obj</c> are a layout's <c>body</c>. Everything else matches only
    /// itself, and a type not listed is not looked up at all
    /// (<c>oox/source/ppt/pptshapecontext.cxx:104-133</c>).
    /// </remarks>
    public (string? First, string? Second) MatchTypes => Type switch
    {
        "ctrTitle" => ("ctrTitle", "title"),
        "subTitle" => ("subTitle", "body"),
        "obj" => ("obj", "body"),
        "dt" or "sldNum" or "ftr" or "hdr" or "body" or "title" or "chart" or "tbl"
            or "clipArt" or "dgm" or "media" or "sldImg" or "pic" => (Type, null),
        _ => (null, null),
    };

    /// <summary>
    /// Which of the master's <c>p:txStyles</c> governs this placeholder's text.
    /// </summary>
    /// <remarks>
    /// <c>oox/source/ppt/pptshape.cxx:116-139</c>. Only three types reach a text style at all;
    /// a date, footer or slide-number placeholder falls straight through to the presentation
    /// default.
    /// </remarks>
    public string? TextStyle(bool isNotesPage) => Type switch
    {
        "ctrTitle" or "title" => "titleStyle",
        "subTitle" or "obj" or "body" => isNotesPage ? "notesStyle" : "bodyStyle",
        _ => null,
    };

    /// <summary>
    /// Finds the placeholder shape this one inherits from, searching a tree.
    /// </summary>
    /// <param name="shapes">
    /// The trees to search, in the order LibreOffice concatenates them: the master's shapes
    /// then the layout's, since the layout is imported <em>into</em> the master's shape list
    /// (<c>oox/source/ppt/presentationfragmenthandler.cxx:287</c>). The search runs backwards
    /// over the result, which is what makes the layout's placeholder beat the master's.
    /// </param>
    /// <remarks>
    /// The five-way priority is LibreOffice's, verbatim
    /// (<c>oox/source/ppt/pptshape.cxx:715-820</c>): same index and preferred type, then
    /// preferred type at any index, then same index and fallback type, then fallback type at
    /// any index, then anything at the same index. The last rung is what makes a slide's
    /// <c>idx="1"</c> find the layout's content placeholder even when the two disagree about
    /// what kind of content it holds — and its own comment concedes the ordering is "probably
    /// unnecessarily complicated".
    /// </remarks>
    public XElement? Find(IEnumerable<XElement> shapes)
    {
        ArgumentNullException.ThrowIfNull(shapes);
        (string? first, string? second) = MatchTypes;
        if (first is null) return null;

        XElement?[] choices = new XElement?[5];

        foreach (XElement shape in Reversed(shapes))
        {
            if (Read(shape, master: null) is not { } candidate) continue;

            bool sameFirst = candidate.Type == first;
            bool sameSecond = second is not null && candidate.Type == second;
            bool sameIndex = candidate.Index == Index;

            int priority =
                sameIndex && sameFirst ? 0
                : sameFirst ? 1
                : sameIndex && sameSecond ? 2
                : sameSecond ? 3
                : sameIndex ? 4
                : -1;

            if (priority < 0) continue;
            choices[priority] ??= shape;
            if (choices[0] is not null) break;
        }

        foreach (XElement? choice in choices)
        {
            if (choice is not null) return choice;
        }
        return null;
    }

    /// <summary>The shapes of a slide, layout or master part, or nothing when it has none.</summary>
    public static IEnumerable<XElement> ShapesOf(XElement? part)
        => Ppt.Child(Ppt.Child(part, "cSld"), "spTree")?.Elements() ?? [];

    private static XElement? FindByIndex(int index, IEnumerable<XElement> shapes)
    {
        foreach (XElement shape in Reversed(shapes))
        {
            if (Element(shape) is { } ph
                && int.TryParse(Ppt.Attribute(ph, "idx"), NumberStyles.Integer,
                                CultureInfo.InvariantCulture, out int candidate)
                && candidate == index)
                return shape;
        }
        return null;
    }

    /// <summary>
    /// Enumerates shapes back to front, descending into groups.
    /// </summary>
    /// <remarks>
    /// Backwards because that is the order LibreOffice's search runs in, and it is load-bearing:
    /// the layout's placeholders sit after the master's in the concatenated list, so only a
    /// reverse walk reaches them first. Groups are descended into because nothing stops a
    /// producer from wrapping a placeholder in one, and LibreOffice recurses too.
    /// </remarks>
    private static IEnumerable<XElement> Reversed(IEnumerable<XElement> shapes)
    {
        List<XElement> ordered = [.. shapes];
        for (int i = ordered.Count - 1; i >= 0; i--)
        {
            yield return ordered[i];
            if (Ppt.Is(ordered[i], "grpSp"))
            {
                foreach (XElement nested in Reversed(ordered[i].Elements())) yield return nested;
            }
        }
    }
}
