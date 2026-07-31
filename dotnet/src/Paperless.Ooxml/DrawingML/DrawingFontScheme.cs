using System.Xml.Linq;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// A theme's font scheme: the six typefaces <c>a:fontScheme</c> declares.
/// </summary>
/// <remarks>
/// <para>
/// Two sets of three — a major face for headings and a minor face for body text, each with a
/// Latin, an East Asian and a complex-script member. That is all of the scheme this holds; the
/// supplemental per-script lists (<c>a:font script="Hang"</c>) are a substitution table rather
/// than part of the scheme's identity, and nothing that resolves a typeface here needs them
/// until script itemisation does.
/// </para>
/// <para>
/// It exists because a typeface can be stated <em>indirectly</em>. <c>&lt;a:latin
/// typeface="+mn-lt"/&gt;</c> means "whatever the theme's minor Latin face is", and a reader
/// that takes the attribute at face value reports a font called <c>+mn-lt</c> — which resolves
/// to nothing, so every run of a PowerPoint-authored deck falls back to a substitute.
/// LibreOffice resolves the same six names in <c>Theme::resolveFont</c>
/// (<c>oox/source/drawingml/theme.cxx</c>:71).
/// </para>
/// </remarks>
/// <param name="MajorLatin">The heading face for Latin script.</param>
/// <param name="MajorAsian">The heading face for East Asian script.</param>
/// <param name="MajorComplex">The heading face for complex script.</param>
/// <param name="MinorLatin">The body face for Latin script.</param>
/// <param name="MinorAsian">The body face for East Asian script.</param>
/// <param name="MinorComplex">The body face for complex script.</param>
public sealed record DrawingFontScheme(
    string? MajorLatin,
    string? MajorAsian,
    string? MajorComplex,
    string? MinorLatin,
    string? MinorAsian,
    string? MinorComplex)
{
    /// <summary>Reads an <c>a:fontScheme</c>, or null when there is none.</summary>
    /// <param name="fontScheme">The <c>a:fontScheme</c> element, or null.</param>
    public static DrawingFontScheme? Read(XElement? fontScheme)
    {
        if (fontScheme is null) return null;

        XElement? major = Drawing.Child(fontScheme, "majorFont");
        XElement? minor = Drawing.Child(fontScheme, "minorFont");

        return new DrawingFontScheme(
            Typeface(major, "latin"), Typeface(major, "ea"), Typeface(major, "cs"),
            Typeface(minor, "latin"), Typeface(minor, "ea"), Typeface(minor, "cs"));
    }

    /// <summary>
    /// The face a <c>a:fontRef</c>'s <c>idx</c> names, for a given script.
    /// </summary>
    /// <param name="index"><c>major</c>, <c>minor</c>, or <c>none</c>.</param>
    /// <param name="script">One of <c>latin</c>, <c>ea</c> or <c>cs</c>.</param>
    public string? ForReference(string? index, string script) => index switch
    {
        "major" => Member(major: true, script),
        "minor" => Member(major: false, script),
        _ => null,
    };

    /// <summary>
    /// A stated typeface with the theme's indirection followed, or the name unchanged.
    /// </summary>
    /// <remarks>
    /// Only the six-character <c>+mj-lt</c> shape is indirect; every other value is a real
    /// family name and is returned as it stands. An unrecognised <c>+</c> name resolves to null
    /// rather than to itself, because reporting the placeholder as a font name is worse than
    /// reporting no font at all — one falls back to the next source in the chain, the other
    /// silently ends the search on a face that does not exist.
    /// </remarks>
    /// <param name="typeface">The stated <c>typeface</c> attribute.</param>
    public string? Resolve(string? typeface)
    {
        if (string.IsNullOrEmpty(typeface)) return null;
        if (typeface[0] != '+') return typeface;
        if (typeface.Length != 6 || typeface[3] != '-') return null;

        bool major = typeface[1] == 'm' && typeface[2] == 'j';
        bool minor = typeface[1] == 'm' && typeface[2] == 'n';
        if (!major && !minor) return null;

        return Member(major, typeface[4..]);
    }

    private string? Member(bool major, string script) => script switch
    {
        "latin" or "lt" => major ? MajorLatin : MinorLatin,
        "ea" => major ? MajorAsian : MinorAsian,
        "cs" => major ? MajorComplex : MinorComplex,
        _ => null,
    };

    /// <summary>
    /// One member of a font set, with the empty string read as absent.
    /// </summary>
    /// <remarks>
    /// Every theme Word ships writes <c>&lt;a:ea typeface=""/&gt;</c> and
    /// <c>&lt;a:cs typeface=""/&gt;</c> rather than omitting the elements, so an attribute test
    /// alone reports two typefaces named "" and hands them to font resolution.
    /// </remarks>
    private static string? Typeface(XElement? fontSet, string localName)
        => Drawing.Attribute(Drawing.Child(fontSet, localName), "typeface") is { Length: > 0 } name
            ? name
            : null;
}
