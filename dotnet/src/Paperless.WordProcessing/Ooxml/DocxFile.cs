using System.Xml.Linq;
using Paperless.Containers;
using Paperless.Containers.Ooxml;
using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Ooxml;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>
/// An open DOCX package, with the parts a reader needs already loaded and its styles and
/// numbering resolved.
/// </summary>
/// <remarks>
/// <para>
/// A DOCX spreads one document over a dozen parts, and which part is which is stated by
/// <em>relationship</em> rather than by name. Conventional names (<c>word/styles.xml</c>) are
/// used only as a fallback, because a producer is free to name them anything and some do: the
/// relationship from <c>document.xml</c> is the authority.
/// </para>
/// <para>
/// Every part is normalised on load — strict namespaces rewritten to transitional,
/// <c>mc:AlternateContent</c> resolved to a single branch — so that the walker above never
/// deals with either. See <see cref="OoxmlXml.Normalise"/>.
/// </para>
/// </remarks>
public sealed class DocxFile : IDisposable
{
    private const string RelationshipBase =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/";

    private readonly OpcPackage _package;
    private readonly List<Diagnostic> _diagnostics = [];
    private readonly Dictionary<string, OpcXml.Relationship> _documentRelationships =
        new(StringComparer.Ordinal);

    private DocxFile(OpcPackage package, IPackagePart mainPart, XElement documentRoot)
    {
        _package = package;
        MainPartName = mainPart.Name;
        Document = documentRoot;
        _diagnostics.AddRange(package.Diagnostics);

        foreach (OpcXml.Relationship relationship in package.GetRelationships(mainPart.Name))
            _documentRelationships[relationship.Id] = relationship;

        Styles = new WordStyles();
        if (LoadRelated("styles", "word/styles.xml") is { } styles) Styles.Add(styles, _diagnostics);

        Numbering = new WordNumbering();
        if (LoadRelated("numbering", "word/numbering.xml") is { } numbering) Numbering.Add(numbering);

        Settings = LoadRelated("settings", "word/settings.xml");
        FootnoteNumbering = ReadNoteNumbering(Word.Child(Settings, "footnotePr"));
        EndnoteNumbering = ReadNoteNumbering(Word.Child(Settings, "endnotePr"));
        Footnotes = ReadNotes(LoadRelated("footnotes", "word/footnotes.xml"), "footnote");
        Endnotes = ReadNotes(LoadRelated("endnotes", "word/endnotes.xml"), "endnote");
        Comments = ReadComments(LoadRelated("comments", "word/comments.xml"));
    }

    /// <summary>The main document part's name, for diagnostics and relationship resolution.</summary>
    public string MainPartName { get; }

    /// <summary>The <c>w:document</c> root of the main part.</summary>
    public XElement Document { get; }

    /// <summary>The <c>w:body</c> element, or null in the pathological case of a document without one.</summary>
    public XElement? Body => Word.Child(Document, "body");

    /// <summary>The document's styles.</summary>
    public WordStyles Styles { get; }

    /// <summary>The document's numbering definitions and live counters.</summary>
    public WordNumbering Numbering { get; }

    /// <summary>
    /// The <c>w:settings</c> root, when the package has one.
    /// </summary>
    /// <remarks>
    /// Not read during extraction. Kept because the compatibility-options block genuinely
    /// changes layout maths to match a particular Word version, so layout will need it.
    /// </remarks>
    public XElement? Settings { get; }

    /// <summary>How footnote citations are numbered.</summary>
    public WordNoteNumbering FootnoteNumbering { get; }

    /// <summary>How endnote citations are numbered.</summary>
    public WordNoteNumbering EndnoteNumbering { get; }

    /// <summary>Footnote bodies by <c>w:id</c>, excluding the separator pseudo-notes.</summary>
    public IReadOnlyDictionary<string, XElement> Footnotes { get; }

    /// <summary>Endnote bodies by <c>w:id</c>, excluding the separator pseudo-notes.</summary>
    public IReadOnlyDictionary<string, XElement> Endnotes { get; }

    /// <summary>Comments by <c>w:id</c>.</summary>
    public IReadOnlyDictionary<string, XElement> Comments { get; }

    /// <summary>The package, for reaching image parts.</summary>
    public IPackage Package => _package;

    /// <summary>Problems found while opening the document.</summary>
    public IReadOnlyList<Diagnostic> Diagnostics => _diagnostics;

    /// <summary>Opens a DOCX over a seekable stream.</summary>
    /// <param name="stream">The package bytes.</param>
    /// <param name="leaveOpen">When true, disposing this does not dispose the stream.</param>
    /// <exception cref="MalformedDocumentException">
    /// There is no readable main document part, so there is nothing to read.
    /// </exception>
    public static DocxFile Open(Stream stream, bool leaveOpen = false)
    {
        ArgumentNullException.ThrowIfNull(stream);

        OpcPackage package = OpcPackage.Open(stream, leaveOpen);
        try
        {
            IPackagePart? main = package.GetMainDocumentPart() ?? package.GetPart("word/document.xml");
            if (main is null)
            {
                throw new MalformedDocumentException(
                    "The OOXML package has no main document part: neither the officeDocument "
                    + "relationship nor the conventional 'word/document.xml' resolves.");
            }

            XElement? root;
            string? error;
            using (Stream content = main.Open()) root = OoxmlXml.TryLoad(content, out error);

            if (root is null)
            {
                throw new MalformedDocumentException(
                    $"The main document part '{main.Name}' is not readable XML"
                    + (error is null ? "." : $": {error}"));
            }

            return new DocxFile(package, main, root);
        }
        catch
        {
            package.Dispose();
            throw;
        }
    }

    /// <summary>
    /// The relationship a <c>r:id</c> names, or null when the document declares no such
    /// relationship.
    /// </summary>
    public OpcXml.Relationship? Relationship(string? relationshipId)
        => relationshipId is not null
           && _documentRelationships.TryGetValue(relationshipId, out OpcXml.Relationship found)
            ? found
            : null;

    /// <summary>
    /// Loads a header or footer part by relationship id.
    /// </summary>
    /// <remarks>
    /// Headers and footers are reached only this way — a section names them by id — so there is
    /// no conventional name to fall back to.
    /// </remarks>
    public XElement? LoadHeaderOrFooter(string? relationshipId)
    {
        if (Relationship(relationshipId) is not { IsExternal: false } relationship) return null;

        IPackagePart? part = _package.GetPart(relationship.Target);
        if (part is null) return null;

        using Stream content = part.Open();
        XElement? root = OoxmlXml.TryLoad(content, out string? error);
        if (root is null && error is not null)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Error, "PL2110",
                $"A header or footer part is malformed and has been skipped: {error.TrimEnd('.')}.",
                new DiagnosticLocation(relationship.Target)));
        }
        return root;
    }

    /// <inheritdoc/>
    public void Dispose() => _package.Dispose();

    /// <summary>
    /// Reads a <c>w:footnotePr</c> or <c>w:endnotePr</c> numbering block.
    /// </summary>
    /// <remarks>
    /// Both the format and the start value default rather than being required. ECMA-376
    /// §17.11.17 fixes the default start at 1 and §17.11.18 the default format at decimal —
    /// which matters because a DOCX caches no note number at all, so a wrong default shows up in
    /// every extracted citation.
    /// </remarks>
    private static WordNoteNumbering ReadNoteNumbering(XElement? properties)
        => new(
            int.TryParse(Word.Value(properties, "numStart"), out int start) ? start : 1,
            Word.Value(properties, "numFmt") ?? "decimal");

    private XElement? LoadRelated(string relationshipSuffix, string conventionalName)
    {
        string? target = _documentRelationships.Values
            .FirstOrDefault(r => !r.IsExternal
                                 && string.Equals(r.Type, RelationshipBase + relationshipSuffix,
                                                  StringComparison.Ordinal))
            .Target;

        IPackagePart? part = (target is null ? null : _package.GetPart(target))
                             ?? _package.GetPart(conventionalName);
        if (part is null) return null;

        using Stream content = part.Open();
        XElement? root = OoxmlXml.TryLoad(content, out string? error);
        if (root is null && error is not null)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Error, "PL2111",
                $"The '{relationshipSuffix}' part is malformed and has been skipped: "
                + $"{error.TrimEnd('.')}.",
                new DiagnosticLocation(part.Name)));
        }
        return root;
    }

    /// <summary>
    /// Indexes note bodies by id, dropping the separator notes.
    /// </summary>
    /// <remarks>
    /// Every footnotes part begins with two pseudo-notes holding the separator line and its
    /// continuation. They are drawing furniture, not content, and reading them would add an
    /// empty note section to every document that has footnotes at all.
    /// </remarks>
    private static Dictionary<string, XElement> ReadNotes(XElement? root, string elementName)
    {
        Dictionary<string, XElement> notes = new(StringComparer.Ordinal);
        if (root is null) return notes;

        foreach (XElement note in Word.Children(root, elementName))
        {
            string? id = Word.Attribute(note, "id");
            if (id is null) continue;

            string? type = Word.Attribute(note, "type");
            if (type is "separator" or "continuationSeparator" or "continuationNotice") continue;

            notes[id] = note;
        }
        return notes;
    }

    private static Dictionary<string, XElement> ReadComments(XElement? root)
    {
        Dictionary<string, XElement> comments = new(StringComparer.Ordinal);
        if (root is null) return comments;

        foreach (XElement comment in Word.Children(root, "comment"))
        {
            if (Word.Attribute(comment, "id") is { } id) comments[id] = comment;
        }
        return comments;
    }
}

/// <summary>
/// How a document numbers its footnote or endnote citations.
/// </summary>
/// <param name="Start">The number the first note takes.</param>
/// <param name="Format">
/// The <c>w:numFmt</c> the citation is rendered in: <c>decimal</c>, <c>lowerRoman</c> and so on.
/// </param>
public readonly record struct WordNoteNumbering(int Start, string Format);
