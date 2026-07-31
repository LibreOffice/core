using System.Globalization;
using System.Xml.Linq;
using Paperless.Containers;
using Paperless.Containers.Ooxml;
using Paperless.Core.Documents;
using Paperless.Core.Formats;

namespace Paperless.Ooxml;

/// <summary>
/// Reads an OPC package's <c>docProps</c> parts into <see cref="DocumentMetadata"/>.
/// </summary>
/// <remarks>
/// OOXML spreads metadata over three parts and two vocabularies, and which part a field lives
/// in is not guessable: the title is Dublin Core in <c>core.xml</c>, the page count is an
/// extended property in <c>app.xml</c>, and anything the user invented is in
/// <c>custom.xml</c> with an MS-OLEPS-style typed value. All three are read here, shared by all
/// three OOXML format readers, because none of it is format-specific.
/// </remarks>
public static class OoxmlMetadata
{
    /// <summary>The relationship type of the core-properties part.</summary>
    public const string CorePropertiesRelationship =
        "http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties";

    /// <summary>The relationship type of the extended-properties part.</summary>
    public const string ExtendedPropertiesRelationship =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties";

    /// <summary>The relationship type of the custom-properties part.</summary>
    public const string CustomPropertiesRelationship =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/custom-properties";

    private const string CoreProperties =
        "http://schemas.openxmlformats.org/package/2006/metadata/core-properties";
    private const string DublinCore = "http://purl.org/dc/elements/1.1/";
    private const string DublinCoreTerms = "http://purl.org/dc/terms/";
    private const string ExtendedProperties =
        "http://schemas.openxmlformats.org/officeDocument/2006/extended-properties";
    private const string CustomProperties =
        "http://schemas.openxmlformats.org/officeDocument/2006/custom-properties";
    private const string VariantTypes =
        "http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes";

    /// <summary>
    /// Reads the metadata a package records.
    /// </summary>
    /// <param name="package">The package to read.</param>
    /// <param name="family">
    /// The document's family, which decides how the statistics counters are read: OOXML records
    /// a spreadsheet's sheet count and a deck's slide count in the same
    /// <c>Slides</c>/<c>Pages</c> family of extended properties.
    /// </param>
    public static DocumentMetadata Read(OpcPackage package, DocumentFamily family = DocumentFamily.Unknown)
    {
        ArgumentNullException.ThrowIfNull(package);

        XElement? core = LoadByRelationship(package, CorePropertiesRelationship, "docProps/core.xml");
        XElement? app = LoadByRelationship(package, ExtendedPropertiesRelationship, "docProps/app.xml");
        XElement? custom = LoadByRelationship(package, CustomPropertiesRelationship, "docProps/custom.xml");

        return new DocumentMetadata
        {
            Title = Text(core, DublinCore, "title"),
            Subject = Text(core, DublinCore, "subject"),
            Description = Text(core, DublinCore, "description"),
            Author = Text(core, DublinCore, "creator"),
            LastModifiedBy = Text(core, CoreProperties, "lastModifiedBy"),
            Language = Text(core, DublinCore, "language"),
            Category = Text(core, CoreProperties, "category"),
            Created = Timestamp(Text(core, DublinCoreTerms, "created")),
            Modified = Timestamp(Text(core, DublinCoreTerms, "modified")),
            Printed = Timestamp(Text(core, CoreProperties, "lastPrinted")),
            RevisionNumber = Integer(Text(core, CoreProperties, "revision")),
            GeneratorApplication = Text(app, ExtendedProperties, "Application"),

            // A single space-or-comma-delimited string, unlike ODF's one element per keyword —
            // so the separator has to be guessed. Commas win when present because a keyword
            // may legitimately contain a space.
            Keywords = SplitKeywords(Text(core, CoreProperties, "keywords")),

            TotalEditingTime = Minutes(Text(app, ExtendedProperties, "TotalTime")),
            Statistics = ReadStatistics(app, family),
            CustomProperties = ReadCustomProperties(custom),
        };
    }

    private static DocumentStatistics? ReadStatistics(XElement? app, DocumentFamily family)
    {
        if (app is null) return null;

        int? pages = Integer(Text(app, ExtendedProperties, "Pages"));
        int? slides = Integer(Text(app, ExtendedProperties, "Slides"));

        return new DocumentStatistics
        {
            PageCount = family == DocumentFamily.Presentation ? null : pages,
            // A deck records its slide count as Slides; some producers write Pages instead.
            SlideCount = family == DocumentFamily.Presentation ? slides ?? pages : slides,
            SheetCount = family == DocumentFamily.Spreadsheet
                ? Integer(Text(app, ExtendedProperties, "Sheets"))
                : null,
            WordCount = Integer(Text(app, ExtendedProperties, "Words")),
            // CharactersWithSpaces is the count a person means by "characters"; Characters
            // excludes them.
            CharacterCount = Integer(Text(app, ExtendedProperties, "CharactersWithSpaces"))
                             ?? Integer(Text(app, ExtendedProperties, "Characters")),
            ParagraphCount = Integer(Text(app, ExtendedProperties, "Paragraphs")),
        };
    }

    /// <summary>
    /// Reads <c>custom.xml</c>, converting each property to the type its variant element names.
    /// </summary>
    /// <remarks>
    /// The value's type is the child element's name — <c>vt:lpwstr</c>, <c>vt:r8</c>,
    /// <c>vt:bool</c>, <c>vt:filetime</c> — which is MS-OLEPS's variant model carried into XML.
    /// A value that does not parse keeps its text rather than being dropped.
    /// </remarks>
    private static Dictionary<string, object?> ReadCustomProperties(XElement? custom)
    {
        Dictionary<string, object?> properties = new(StringComparer.Ordinal);
        if (custom is null) return properties;

        foreach (XElement property in custom.Elements(XName.Get("property", CustomProperties)))
        {
            string? name = property.Attribute("name")?.Value;
            if (string.IsNullOrEmpty(name)) continue;

            XElement? value = property.Elements().FirstOrDefault(e => e.Name.NamespaceName == VariantTypes);
            if (value is null) { properties[name] = null; continue; }

            string text = value.Value;
            properties[name] = value.Name.LocalName switch
            {
                "bool" => text is "1" or "true" ? true : text is "0" or "false" ? false : (object?)text,
                "i1" or "i2" or "i4" or "i8" or "int" or "ui1" or "ui2" or "ui4" or "ui8" or "uint"
                    => long.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out long i)
                        ? i
                        : (object?)text,
                "r4" or "r8" or "decimal"
                    => double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture, out double d)
                        ? d
                        : (object?)text,
                "filetime" or "date" => Timestamp(text) ?? (object?)text,
                _ => text,
            };
        }
        return properties;
    }

    private static XElement? LoadByRelationship(
        OpcPackage package, string relationshipType, string conventionalName)
    {
        // The relationship is authoritative; the conventional name is a fallback for packages
        // that omit the relationship but still carry the part, which does happen.
        string? target = package.GetRelationshipsByType(relationshipType)
                                .FirstOrDefault(r => !r.IsExternal).Target;

        IPackagePart? part = (target is null ? null : package.GetPart(target))
                             ?? package.GetPart(conventionalName);
        if (part is null) return null;

        using Stream content = part.Open();
        return OoxmlXml.TryLoad(content, out _);
    }

    private static string? Text(XElement? parent, string ns, string localName)
    {
        string? value = parent?.Element(XName.Get(localName, ns))?.Value;
        // An empty element is not a recorded value: DocumentMetadata's contract is that null
        // means "not recorded", and OOXML writers emit empty elements freely.
        return string.IsNullOrWhiteSpace(value) ? null : value.Trim();
    }

    private static DateTimeOffset? Timestamp(string? value)
        => DateTimeOffset.TryParse(value, CultureInfo.InvariantCulture,
                                   DateTimeStyles.RoundtripKind, out DateTimeOffset parsed)
            ? parsed
            : null;

    private static int? Integer(string? value)
        => int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out int parsed)
            ? parsed
            : null;

    /// <summary>
    /// <c>TotalTime</c> is a count of minutes, not a duration string.
    /// </summary>
    private static TimeSpan? Minutes(string? value)
        => Integer(value) is { } minutes and >= 0 ? TimeSpan.FromMinutes(minutes) : null;

    private static string[] SplitKeywords(string? keywords)
    {
        if (string.IsNullOrWhiteSpace(keywords)) return [];

        char[] separators = keywords.Contains(',', StringComparison.Ordinal)
            ? [',', ';']
            : [' ', ';'];
        return [.. keywords.Split(separators, StringSplitOptions.RemoveEmptyEntries
                                             | StringSplitOptions.TrimEntries)];
    }
}
