using System.Xml.Linq;
using Paperless.Core.Documents;
using Paperless.Core.Formats;

namespace Paperless.OpenDocument;

/// <summary>
/// Reads an ODF <c>office:meta</c> element into <see cref="DocumentMetadata"/>.
/// </summary>
/// <remarks>
/// ODF splits metadata across two vocabularies: Dublin Core for the fields it defines, and
/// the ODF-specific <c>meta:</c> namespace for everything else. The split is not a tidy
/// one, and two pairs in particular are easy to get backwards:
/// <list type="bullet">
///   <item><description>
///     <c>meta:initial-creator</c> is the original author; <c>dc:creator</c> is whoever
///     saved the document most recently. Reading <c>dc:creator</c> as "the author" — the
///     obvious guess — reports the last editor instead.
///   </description></item>
///   <item><description>
///     <c>meta:creation-date</c> is when the document was created; <c>dc:date</c> is when it
///     was last modified.
///   </description></item>
/// </list>
/// </remarks>
public static class OdfMetadata
{
    /// <summary>
    /// Reads the metadata from an <c>office:meta</c> element.
    /// </summary>
    /// <param name="meta">
    /// The <c>office:meta</c> element, or null when the document records none — in which
    /// case <see cref="DocumentMetadata.Empty"/> comes back rather than an exception, since
    /// metadata is optional in ODF.
    /// </param>
    /// <param name="family">
    /// The document's family, which decides how the statistics counters are interpreted:
    /// ODF records a spreadsheet's sheet count in <c>meta:table-count</c> and a
    /// presentation's slide count in <c>meta:page-count</c>, reusing the same attributes
    /// that mean tables and printed pages in a text document.
    /// </param>
    public static DocumentMetadata Read(XElement? meta, DocumentFamily family = DocumentFamily.Unknown)
    {
        if (meta is null) return DocumentMetadata.Empty;

        return new DocumentMetadata
        {
            Title = Text(meta, OdfNamespaces.DublinCore, "title"),
            Subject = Text(meta, OdfNamespaces.DublinCore, "subject"),
            Description = Text(meta, OdfNamespaces.DublinCore, "description"),
            Author = Text(meta, OdfNamespaces.Meta, "initial-creator"),
            LastModifiedBy = Text(meta, OdfNamespaces.DublinCore, "creator"),
            Language = Text(meta, OdfNamespaces.DublinCore, "language"),
            GeneratorApplication = Text(meta, OdfNamespaces.Meta, "generator"),
            Created = OdfValue.ParseDateTime(Text(meta, OdfNamespaces.Meta, "creation-date")),
            Modified = OdfValue.ParseDateTime(Text(meta, OdfNamespaces.DublinCore, "date")),
            Printed = OdfValue.ParseDateTime(Text(meta, OdfNamespaces.Meta, "print-date")),
            RevisionNumber = OdfValue.ParseInt(Text(meta, OdfNamespaces.Meta, "editing-cycles")),
            TotalEditingTime = OdfValue.ParseDuration(Text(meta, OdfNamespaces.Meta, "editing-duration")),

            // Each keyword is its own element, so unlike OOXML's single delimited string
            // there is nothing to split and no ambiguity about the separator.
            Keywords = [.. meta.Elements(XName.Get("keyword", OdfNamespaces.Meta))
                              .Select(e => e.Value.Trim())
                              .Where(k => k.Length > 0)],

            Statistics = ReadStatistics(
                meta.Element(XName.Get("document-statistic", OdfNamespaces.Meta)), family),
            CustomProperties = ReadUserDefined(meta),
        };
    }

    private static DocumentStatistics? ReadStatistics(XElement? statistic, DocumentFamily family)
    {
        if (statistic is null) return null;

        int? pages = Number(statistic, "page-count");
        int? tables = Number(statistic, "table-count");

        return new DocumentStatistics
        {
            PageCount = family == DocumentFamily.Presentation ? null : pages,
            SlideCount = family == DocumentFamily.Presentation ? pages : null,
            SheetCount = family == DocumentFamily.Spreadsheet ? tables : null,
            TableCount = family == DocumentFamily.Spreadsheet ? null : tables,
            WordCount = Number(statistic, "word-count"),
            CharacterCount = Number(statistic, "character-count"),
            ParagraphCount = Number(statistic, "paragraph-count"),
            ImageCount = Number(statistic, "image-count"),
        };

        static int? Number(XElement element, string name)
            => OdfValue.ParseInt(element.Attribute(XName.Get(name, OdfNamespaces.Meta))?.Value);
    }

    /// <summary>
    /// Reads <c>meta:user-defined</c> properties, converting each to the type it declares.
    /// </summary>
    /// <remarks>
    /// A property whose declared type does not parse is kept as its original string rather
    /// than dropped: the name and the text a user typed are still the information they were
    /// after, and losing the property entirely because a date was malformed would be worse.
    /// </remarks>
    private static Dictionary<string, object?> ReadUserDefined(XElement meta)
    {
        Dictionary<string, object?> properties = new(StringComparer.Ordinal);
        foreach (XElement element in meta.Elements(XName.Get("user-defined", OdfNamespaces.Meta)))
        {
            string? name = element.Attribute(XName.Get("name", OdfNamespaces.Meta))?.Value;
            if (string.IsNullOrEmpty(name)) continue;

            string text = element.Value;
            properties[name] = element.Attribute(XName.Get("value-type", OdfNamespaces.Meta))?.Value switch
            {
                "float" => OdfValue.ParseDouble(text) ?? (object?)text,
                "boolean" => OdfValue.ParseBoolean(text) ?? (object?)text,
                "date" or "time" => OdfValue.ParseDateTime(text) ?? (object?)text,
                _ => text,
            };
        }
        return properties;
    }

    private static string? Text(XElement parent, string ns, string localName)
    {
        string? value = parent.Element(XName.Get(localName, ns))?.Value;
        // ODF metadata elements are frequently written empty rather than omitted. An empty
        // string is not a recorded value, and DocumentMetadata's contract is that a null
        // means "not recorded" — so collapse the two.
        return string.IsNullOrWhiteSpace(value) ? null : value.Trim();
    }
}
