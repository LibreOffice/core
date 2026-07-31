using System.Xml.Linq;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>Which of a family's four faces an embedded font part supplies.</summary>
public enum WordEmbeddedFontStyle
{
    /// <summary>Upright, normal weight (<c>w:embedRegular</c>).</summary>
    Regular,

    /// <summary>Bold (<c>w:embedBold</c>).</summary>
    Bold,

    /// <summary>Italic (<c>w:embedItalic</c>).</summary>
    Italic,

    /// <summary>Bold italic (<c>w:embedBoldItalic</c>).</summary>
    BoldItalic,
}

/// <summary>
/// One font part the document embeds.
/// </summary>
/// <remarks>
/// <para>
/// The bytes are not read. They are an ODTTF — a TrueType file with the first thirty-two bytes
/// exclusive-ORed against the sixteen bytes of <see cref="Key"/>, applied twice — so decoding
/// them is cheap, but a subsetted face is missing most of its glyphs and substituting for it
/// silently would be worse than substituting for the family by name. What the record is for is
/// telling a caller that the document carries its own copy of a face, which is exactly the case
/// where a name-based substitution is going to be wrong.
/// </para>
/// </remarks>
/// <param name="Style">Which face of the family this part is.</param>
/// <param name="RelationshipId">The <c>r:id</c> naming the part; it is relative to the font table's own part.</param>
/// <param name="Key">
/// The <c>w:fontKey</c> GUID the obfuscation uses, or null when the part is not obfuscated.
/// </param>
/// <param name="IsSubsetted">
/// True when <c>w:subsetted</c> says the face holds only the glyphs the document uses, so it
/// cannot serve as a general substitute for the family.
/// </param>
public readonly record struct WordEmbeddedFont(
    WordEmbeddedFontStyle Style,
    string? RelationshipId,
    string? Key,
    bool IsSubsetted);

/// <summary>
/// One entry in <c>fontTable.xml</c>.
/// </summary>
/// <param name="Name">The family name, which is what <c>w:rFonts</c> refers to.</param>
/// <param name="AlternativeName">
/// <c>w:altName</c>: the family to try when <see cref="Name"/> is not installed. Word writes it
/// for fonts whose real name is non-Latin, so it is the only ASCII name some entries have.
/// </param>
/// <param name="Panose">
/// The ten <c>w:panose1</c> bytes as the twenty hex digits the file states, or null. PANOSE
/// classifies a face by shape — serif style, weight, proportion, contrast — and is what a
/// substitution engine can match on when the name means nothing to it.
/// </param>
/// <param name="Family">
/// <c>w:family</c>: <c>roman</c>, <c>swiss</c>, <c>modern</c>, <c>script</c>, <c>decorative</c>
/// or <c>auto</c>. The coarse version of the same question.
/// </param>
/// <param name="Pitch"><c>w:pitch</c>: <c>fixed</c>, <c>variable</c> or <c>default</c>.</param>
/// <param name="Charset">
/// <c>w:charset</c>, a Windows character-set number in hexadecimal — <c>02</c> is the symbol
/// set, which is the one that changes how a run's bytes are read rather than how it looks.
/// </param>
/// <param name="IsTrueType">
/// False when the entry carries <c>w:notTrueType</c>, which marks a bitmap or device font.
/// </param>
/// <param name="Embedded">The font parts the document carries for this family.</param>
public sealed record WordFont(
    string Name,
    string? AlternativeName,
    string? Panose,
    string? Family,
    string? Pitch,
    string? Charset,
    bool IsTrueType,
    IReadOnlyList<WordEmbeddedFont> Embedded);

/// <summary>
/// The document's font table.
/// </summary>
/// <remarks>
/// <para>
/// <b>Layout does not need this part.</b> <c>w:rFonts</c> names a family outright, so a
/// paragraph can be measured without ever opening the table — which is why it went unread for so
/// long, and why nothing here changes a measurement.
/// </para>
/// <para>
/// What it holds that is not available anywhere else is the two things a <em>substitution</em>
/// would use. First, the embedded-font relationships: a document that carries its own copy of a
/// face is precisely the one where matching by name gives the wrong metrics, and until the part
/// is read there is no way to know that it does. Second, PANOSE — ten bytes describing the
/// shape of the face — plus <c>w:altName</c>, <c>w:family</c> and <c>w:pitch</c>, which is what
/// LibreOffice's own importer reads it for (<c>sw/source/writerfilter/dmapper/FontTable.cxx</c>
/// builds an <c>SvxFontItem</c> per entry and hands the pitch and family to the font matcher).
/// </para>
/// <para>
/// So this is read and reported, and nothing consumes it yet. Loading the font bytes is a
/// separate job and a larger one — see <see cref="WordEmbeddedFont"/> for what it would involve
/// and why a subsetted face makes it less useful than it sounds.
/// </para>
/// </remarks>
public sealed class WordFontTable
{
    private readonly Dictionary<string, WordFont> _byName;

    private WordFontTable(List<WordFont> fonts)
    {
        Fonts = fonts;

        // Case-insensitive, because w:rFonts and the table disagree about case in real files
        // more often than they disagree about anything else.
        _byName = new Dictionary<string, WordFont>(StringComparer.OrdinalIgnoreCase);
        foreach (WordFont font in fonts) _byName.TryAdd(font.Name, font);
    }

    /// <summary>An empty table, which is what a package without the part has.</summary>
    public static WordFontTable Empty { get; } = new([]);

    /// <summary>The entries, in the order the part declares them.</summary>
    public IReadOnlyList<WordFont> Fonts { get; }

    /// <summary>True when any entry embeds a font part.</summary>
    public bool HasEmbeddedFonts => Fonts.Any(font => font.Embedded.Count > 0);

    /// <summary>The entry for a family name, or null when the table has none.</summary>
    public WordFont? Find(string? name)
        => name is not null && _byName.TryGetValue(name, out WordFont? font) ? font : null;

    /// <summary>Reads a <c>w:fonts</c> root.</summary>
    /// <param name="root">The part's root element, or null when the package has no such part.</param>
    public static WordFontTable Read(XElement? root)
    {
        if (root is null) return Empty;

        List<WordFont> fonts = [];

        foreach (XElement font in Word.Children(root, "font"))
        {
            // w:name is an attribute on w:font rather than a child element, which is the one
            // place this part departs from WordprocessingML's usual val-on-a-child shape.
            string? name = Word.Attribute(font, "name");
            if (string.IsNullOrEmpty(name)) continue;

            fonts.Add(new WordFont(
                name,
                Word.Value(font, "altName"),
                Word.Value(font, "panose1"),
                Word.Value(font, "family"),
                Word.Value(font, "pitch"),
                Word.Value(font, "charset"),
                Word.Child(font, "notTrueType") is null,
                Embedded(font)));
        }

        return new WordFontTable(fonts);
    }

    private static List<WordEmbeddedFont> Embedded(XElement font)
    {
        List<WordEmbeddedFont> embedded = [];

        Add("embedRegular", WordEmbeddedFontStyle.Regular);
        Add("embedBold", WordEmbeddedFontStyle.Bold);
        Add("embedItalic", WordEmbeddedFontStyle.Italic);
        Add("embedBoldItalic", WordEmbeddedFontStyle.BoldItalic);

        return embedded;

        void Add(string localName, WordEmbeddedFontStyle style)
        {
            if (Word.Child(font, localName) is not { } element) return;

            embedded.Add(new WordEmbeddedFont(
                style,
                element.Attribute(
                    XName.Get("id", Paperless.Ooxml.OoxmlNamespaces.Relationships))?.Value,
                Word.Attribute(element, "fontKey"),
                Word.Attribute(element, "subsetted") is "true" or "1"));
        }
    }
}
