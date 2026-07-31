using System.Globalization;
using System.Xml.Linq;
using Paperless.OpenDocument;
using Paperless.WordProcessing.Model;

namespace Paperless.WordProcessing.OpenDocument;

/// <summary>
/// Turns the marks ODF's content walk steps over into the model's tracked changes, bookmarks and
/// fields.
/// </summary>
/// <remarks>
/// <para>
/// ODF is the odd one of the four in every respect here. A tracked change is not around the text it
/// concerns: the change's <em>description</em> — who, when, and for a deletion the removed text
/// itself — is hoisted into a <c>text:tracked-changes</c> region at the top of the body, and what is
/// left at the position is an empty element naming it by id. That hoisting is what makes ODF the one
/// format where extraction gets deletions right without doing anything, and it is also what makes
/// recording them a lookup rather than a walk.
/// </para>
/// <para>
/// A field is not an instruction either. Where DOCX, DOC and RTF all write <c>PAGE \* ARABIC</c> and
/// leave a reader to parse it, ODF has one element per kind — <c>text:page-number</c>,
/// <c>text:date</c> — and the element's content is the cached result. So the definition is the
/// element's name, mapped onto the same <see cref="WritingFieldKind"/> the instruction parser
/// produces, and <see cref="WritingField.Instruction"/> stays null because there is no such string.
/// </para>
/// </remarks>
internal sealed class OdtMarkSink : IOdfMarkSink
{
    private readonly WritingMarkBuilder _builder = new();
    private Dictionary<string, ChangeRegion>? _regions;

    /// <summary>The marks collected.</summary>
    public WritingMarks Marks => _builder.Build();

    /// <inheritdoc/>
    public void OpenParagraph() => _builder.OpenParagraph();

    /// <inheritdoc/>
    public void CloseParagraph(string text) => _builder.CloseParagraph(text);

    /// <inheritdoc/>
    public void Mark(XElement element, int startOffset, int endOffset)
    {
        ArgumentNullException.ThrowIfNull(element);
        if (element.Name.NamespaceName != OdfNamespaces.Text) return;

        switch (element.Name.LocalName)
        {
            case "bookmark":
                // A point bookmark: one element rather than a pair, which is what a cross-reference
                // target usually is.
                if (Name(element) is { } point)
                    _builder.AddPointBookmark(point, _builder.At(startOffset));
                return;

            case "bookmark-start":
                if (Name(element) is { } opening)
                    _builder.OpenBookmark(opening, opening, _builder.At(startOffset));
                return;

            case "bookmark-end":
                if (Name(element) is { } closing)
                    _builder.CloseBookmark(closing, _builder.At(startOffset));
                return;

            case "change-start":
                OpenChange(element, startOffset);
                return;

            case "change-end":
                if (ChangeId(element) is { } ending) _builder.CloseChange(ending, _builder.At(startOffset));
                return;

            case "change":
                // A change with no extent: a deletion, whose text is in the region rather than here.
                AddPointChange(element, startOffset);
                return;

            default:
                if (KindOf(element.Name.LocalName) is { } kind)
                {
                    _builder.AddField(
                        instruction: null,
                        result: null,
                        _builder.At(startOffset),
                        _builder.At(endOffset),
                        kind);
                }
                return;
        }
    }

    /// <summary>
    /// ODF pairs a bookmark's halves by name, which is also what content refers to it by.
    /// </summary>
    private static string? Name(XElement element)
    {
        string? name = element.Attribute(XName.Get("name", OdfNamespaces.Text))?.Value;
        return string.IsNullOrEmpty(name) ? null : name;
    }

    private static string? ChangeId(XElement element)
    {
        string? id = element.Attribute(XName.Get("change-id", OdfNamespaces.Text))?.Value;
        return string.IsNullOrEmpty(id) ? null : id;
    }

    private void OpenChange(XElement element, int offset)
    {
        if (ChangeId(element) is not { } id) return;

        ChangeRegion region = Region(element, id);
        _builder.OpenChange(id, region.Kind, region.Author, region.When, _builder.At(offset));
    }

    private void AddPointChange(XElement element, int offset)
    {
        if (ChangeId(element) is not { } id) return;

        ChangeRegion region = Region(element, id);
        WritingPosition? at = _builder.At(offset);
        _builder.AddChange(region.Kind, region.Author, region.When, region.Text, at, at);
    }

    /// <summary>
    /// The change region an id names, read from the document on first use.
    /// </summary>
    /// <remarks>
    /// Lazily, and from the mark's own document rather than from a file handle, because the regions
    /// sit at the top of <c>office:text</c> and are therefore already parsed by the time any mark
    /// referring to one is met. A document with no tracked changes never builds the index at all.
    /// </remarks>
    private ChangeRegion Region(XElement element, string id)
    {
        _regions ??= ReadRegions(element.Document?.Root);
        return _regions.TryGetValue(id, out ChangeRegion? found) ? found : ChangeRegion.Unknown;
    }

    private static Dictionary<string, ChangeRegion> ReadRegions(XElement? root)
    {
        Dictionary<string, ChangeRegion> regions = new(StringComparer.Ordinal);
        if (root is null) return regions;

        foreach (XElement region in root.Descendants(XName.Get("changed-region", OdfNamespaces.Text)))
        {
            // Two spellings of the same identity: text:id is the original and xml:id the one ODF 1.2
            // added, and LibreOffice writes both with the same value. Either may be what a mark
            // names, so both are indexed.
            string? textId = region.Attribute(XName.Get("id", OdfNamespaces.Text))?.Value;
            string? xmlId = region.Attribute(XName.Get("id", XNamespace.Xml.NamespaceName))?.Value;

            foreach (XElement description in region.Elements())
            {
                if (description.Name.NamespaceName != OdfNamespaces.Text) continue;

                WritingChangeKind? kind = description.Name.LocalName switch
                {
                    "insertion" => WritingChangeKind.Insertion,
                    "deletion" => WritingChangeKind.Deletion,
                    "format-change" => WritingChangeKind.Formatting,
                    _ => null,
                };
                if (kind is not { } changeKind) continue;

                XElement? info = description.Element(XName.Get("change-info", OdfNamespaces.Office));

                ChangeRegion parsed = new(
                    changeKind,
                    Value(info, OdfNamespaces.DublinCore, "creator"),
                    ParseDate(Value(info, OdfNamespaces.DublinCore, "date")),
                    DeletedText(description));

                if (textId is { Length: > 0 }) regions[textId] = parsed;
                if (xmlId is { Length: > 0 }) regions[xmlId] = parsed;
                break;
            }
        }

        return regions;
    }

    /// <summary>
    /// The text a deletion removed, which the region holds as whole paragraphs.
    /// </summary>
    /// <remarks>
    /// Paragraphs, not runs: ODF moves the removed content wholesale, so a deletion spanning a
    /// paragraph break arrives as several <c>text:p</c> elements. They are joined with newlines,
    /// which is what the extracted text would have said had the deletion not happened.
    /// </remarks>
    private static string? DeletedText(XElement description)
    {
        List<string> paragraphs = [];
        foreach (XElement paragraph in description.Elements())
        {
            if (paragraph.Name.NamespaceName == OdfNamespaces.Text
                && paragraph.Name.LocalName is "p" or "h")
            {
                paragraphs.Add(paragraph.Value);
            }
        }

        return paragraphs.Count == 0 ? null : string.Join('\n', paragraphs);
    }

    private static string? Value(XElement? parent, string ns, string name)
    {
        string? value = parent?.Element(XName.Get(name, ns))?.Value;
        return string.IsNullOrEmpty(value) ? null : value;
    }

    /// <summary>
    /// A <c>dc:date</c>, which is an ISO 8601 date and often carries no time at all.
    /// </summary>
    /// <remarks>
    /// Round-trip parsing rather than a fixed format: LibreOffice writes <c>1970-01-01</c> for a
    /// change whose source stated no date, and a full timestamp when it has one, and both are valid.
    /// </remarks>
    private static DateTime? ParseDate(string? value)
        => DateTime.TryParse(
            value, CultureInfo.InvariantCulture,
            DateTimeStyles.AdjustToUniversal | DateTimeStyles.AllowWhiteSpaces,
            out DateTime parsed)
            ? parsed
            : null;

    /// <summary>
    /// What an ODF field element computes, or null when the element is not a field.
    /// </summary>
    /// <remarks>
    /// A list rather than a fall-through, because the walk reaches this for every inline element it
    /// does not otherwise handle — <c>text:meta</c>, a <c>loext:</c> wrapper, a vendor extension —
    /// and treating all of those as fields would fill the record with markup. So membership of this
    /// list is what makes something a field, and the kind is a second question.
    /// </remarks>
    private static WritingFieldKind? KindOf(string name) => name switch
    {
        "page-number" => WritingFieldKind.PageNumber,
        "page-count" => WritingFieldKind.PageCount,
        "date" => WritingFieldKind.Date,
        "time" => WritingFieldKind.Time,
        "creation-date" or "creation-time" => WritingFieldKind.CreationDate,
        "modification-date" or "modification-time" or "print-date" or "print-time"
            => WritingFieldKind.ModificationDate,
        "author-name" or "author-initials" or "initial-creator" or "creator"
            => WritingFieldKind.Author,
        "file-name" or "template-name" => WritingFieldKind.FileName,
        "title" => WritingFieldKind.Title,
        "subject" => WritingFieldKind.Subject,
        "keywords" => WritingFieldKind.Keywords,
        "description" => WritingFieldKind.Description,
        "chapter" => WritingFieldKind.Chapter,
        "bookmark-ref" or "reference-ref" or "note-ref" or "sequence-ref"
            => WritingFieldKind.Reference,
        "sequence" => WritingFieldKind.Sequence,
        "variable-get" or "variable-set" or "variable-input" or "user-defined"
            or "user-field-get" or "user-field-input" or "expression" or "text-input"
            or "placeholder" or "database-display" => WritingFieldKind.Variable,
        "word-count" => WritingFieldKind.WordCount,
        "page-continuation" or "editing-cycles" or "editing-duration" or "character-count"
            or "paragraph-count" or "image-count" or "table-count" or "object-count"
            or "conditional-text" or "hidden-text" or "sheet-name" or "table-formula"
            or "page-variable-get" or "measure" or "dde-connection"
            => WritingFieldKind.Unknown,
        _ => null,
    };

    /// <summary>One change region: what the change was, by whom, when, and what it removed.</summary>
    private sealed record ChangeRegion(
        WritingChangeKind Kind, string? Author, DateTime? When, string? Text)
    {
        /// <summary>What a mark naming a region the document does not define resolves to.</summary>
        public static readonly ChangeRegion Unknown =
            new(WritingChangeKind.Formatting, null, null, null);
    }
}
