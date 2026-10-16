using System.Xml.Linq;
using XRay.Office.Containers;
using XRay.Office.Containers.OpenDocument;
using XRay.Office.Core;
using XRay.Office.Core.Diagnostics;
using XRay.Office.OpenDocument.Styles;

namespace XRay.Office.OpenDocument;

/// <summary>
/// An open ODF document, whether packaged or flat, with its styles already resolved into
/// one collection.
/// </summary>
/// <remarks>
/// <para>
/// ODF comes in two physical forms. The packaged form is a ZIP holding
/// <c>content.xml</c>, <c>styles.xml</c>, <c>meta.xml</c> and <c>settings.xml</c>; the flat
/// form (<c>.fodt</c>, <c>.fods</c>, <c>.fodp</c>) is a single XML file with the same four
/// sections inline under one <c>office:document</c> root. This type hides that difference
/// entirely, so nothing above it has to care which it was handed — which is the whole
/// reason flat XML costs almost nothing to support.
/// </para>
/// <para>
/// Styles from both files are merged into a single <see cref="OdfStyles"/>, since
/// content in <c>content.xml</c> routinely references named styles declared in
/// <c>styles.xml</c> and callers should not have to know which file a style came from.
/// </para>
/// </remarks>
public sealed class OdfFile : IDisposable
{
    private readonly OdfPackage? _package;
    private readonly List<Diagnostic> _diagnostics = [];

    private OdfFile(OdfPackage? package, XElement? content, XElement? styles)
    {
        _package = package;
        ContentRoot = content;
        StylesRoot = styles;

        if (package is not null) _diagnostics.AddRange(package.Diagnostics);

        // styles.xml first, content.xml second: where both declare an automatic style of
        // the same name, the one the body actually references should win.
        if (styles is not null) Styles.AddDocument(styles, _diagnostics);
        if (content is not null && !ReferenceEquals(content, styles))
            Styles.AddDocument(content, _diagnostics);
    }

    /// <summary>The document's styles, merged across both style-bearing parts.</summary>
    public OdfStyles Styles { get; } = new();

    /// <summary>
    /// The root of <c>content.xml</c> (<c>office:document-content</c>), or of the whole file
    /// for flat XML.
    /// </summary>
    public XElement? ContentRoot { get; }

    /// <summary>
    /// The root of <c>styles.xml</c> (<c>office:document-styles</c>). For flat XML this is
    /// the same element as <see cref="ContentRoot"/>, because one root holds everything.
    /// </summary>
    public XElement? StylesRoot { get; }

    /// <summary>
    /// The <c>office:body</c> child holding the document content: <c>office:text</c>,
    /// <c>office:spreadsheet</c> or <c>office:presentation</c>. Null when the document has
    /// no body at all.
    /// </summary>
    public XElement? Body
    {
        get
        {
            XElement? body = ContentRoot?.Element(XName.Get("body", OdfNamespaces.Office));
            return body?.Elements().FirstOrDefault(e => e.Name.NamespaceName == OdfNamespaces.Office);
        }
    }

    /// <summary>The <c>office:meta</c> element, from <c>meta.xml</c> or inline.</summary>
    /// <remarks>
    /// A packaged document normally keeps metadata in <c>meta.xml</c>, but it is legal —
    /// and some writers do it — to put <c>office:meta</c> inside <c>content.xml</c>
    /// instead, so both places are searched.
    /// </remarks>
    public XElement? Meta { get; private set; }

    /// <summary>The <c>office:settings</c> element, when the document has one.</summary>
    public XElement? Settings { get; private set; }

    /// <summary>The <c>office:version</c> the document declares, e.g. <c>1.3</c>.</summary>
    public string? Version =>
        ContentRoot?.Attribute(XName.Get("version", OdfNamespaces.Office))?.Value
        ?? StylesRoot?.Attribute(XName.Get("version", OdfNamespaces.Office))?.Value;

    /// <summary>
    /// The media type the document declares: the <c>mimetype</c> entry for a package, or
    /// the <c>office:mimetype</c> attribute for flat XML.
    /// </summary>
    public string? MimeType { get; private set; }

    /// <summary>True when the document is flat XML rather than a package.</summary>
    public bool IsFlatXml => _package is null;

    /// <summary>The underlying package, or null for flat XML. Used to reach image parts.</summary>
    public IPackage? Package => _package;

    /// <summary>Problems found while opening the document.</summary>
    public IReadOnlyList<Diagnostic> Diagnostics => _diagnostics;

    /// <summary>
    /// Opens an ODF document, choosing the packaged or flat reader by looking at the bytes.
    /// </summary>
    /// <param name="stream">A seekable stream over the whole document.</param>
    /// <param name="leaveOpen">When true, disposing this does not dispose the stream.</param>
    /// <exception cref="MalformedDocumentException">
    /// The stream is neither a readable ZIP nor parseable XML, so there is nothing to read.
    /// </exception>
    /// <exception cref="PasswordRequiredException">The document's parts are encrypted.</exception>
    public static OdfFile Open(Stream stream, bool leaveOpen = false)
    {
        ArgumentNullException.ThrowIfNull(stream);
        return LooksLikeZip(stream) ? OpenPackage(stream, leaveOpen) : OpenFlat(stream);
    }

    /// <summary>Opens a part of the package by name, or null when absent or flat XML.</summary>
    public Stream? OpenPart(string partName) => _package?.GetPart(partName)?.Open();

    /// <summary>Records a problem found while reading the document.</summary>
    /// <remarks>
    /// <para>
    /// The family readers above this one repair what they can and skip what they cannot, and the
    /// skip has to be visible to a caller — rule 5 of the working notes. They cannot reach
    /// <see cref="Diagnostics"/> to add to it, so this is the one way in, mirroring
    /// <c>PptxFile.Report</c> on the other side of the tree.
    /// </para>
    /// <para>
    /// It has to be usable <em>after</em> the document has been read, because some of what a
    /// reader can only find out lazily happens during layout: an embedded font part is opened
    /// when a run first asks for that family, which is long after <c>OdfReader.Read</c> returned.
    /// That is why <see cref="Diagnostics"/> is the live list and <c>OdfDocument</c> hands it
    /// straight through.
    /// </para>
    /// </remarks>
    /// <param name="diagnostic">What was found.</param>
    public void Report(Diagnostic diagnostic) => _diagnostics.Add(diagnostic);

    /// <summary>The live list behind <see cref="Diagnostics"/>, for the reader to append to.</summary>
    /// <remarks>
    /// Handed to <c>OdfContentReader</c> so that everything a document collects — opening it,
    /// reading its styles, walking its body, and anything a layout finds later — is one list
    /// rather than a snapshot and a remainder.
    /// </remarks>
    internal List<Diagnostic> DiagnosticList => _diagnostics;

    /// <inheritdoc/>
    public void Dispose() => _package?.Dispose();

    private static bool LooksLikeZip(Stream stream)
    {
        long start = stream.Position;
        Span<byte> signature = stackalloc byte[2];
        int read = stream.ReadAtLeast(signature, 2, throwOnEndOfStream: false);
        stream.Position = start;
        return read == 2 && signature[0] == 'P' && signature[1] == 'K';
    }

    private static OdfFile OpenPackage(Stream stream, bool leaveOpen)
    {
        OdfPackage package = OdfPackage.Open(stream, leaveOpen);
        try
        {
            if (package.IsEncrypted)
            {
                throw new PasswordRequiredException(
                    "The ODF document's parts are encrypted. ODF encryption is not implemented yet.",
                    passwordWasSupplied: false);
            }

            XElement? content = LoadPart(package, OdfPackage.PartNames.Content, out string? contentError);
            XElement? styles = LoadPart(package, OdfPackage.PartNames.Styles, out string? stylesError);

            if (content is null)
            {
                throw new MalformedDocumentException(
                    $"The ODF package has no readable '{OdfPackage.PartNames.Content}'"
                    + (contentError is null ? "." : $": {contentError}"));
            }

            OdfFile file = new(package, content, styles) { MimeType = package.MimeType };
            if (stylesError is not null)
            {
                file._diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Error, "PL2010",
                    $"'{OdfPackage.PartNames.Styles}' is malformed and has been skipped "
                    + $"({stylesError.TrimEnd('.')}). Named styles and page masters will be missing.",
                    new DiagnosticLocation(OdfPackage.PartNames.Styles)));
            }

            file.Meta = LoadPart(package, OdfPackage.PartNames.Meta, out _)
                            ?.Element(XName.Get("meta", OdfNamespaces.Office))
                        ?? content.Element(XName.Get("meta", OdfNamespaces.Office));

            file.Settings = LoadPart(package, OdfPackage.PartNames.Settings, out _)
                ?.Element(XName.Get("settings", OdfNamespaces.Office));

            return file;
        }
        catch
        {
            package.Dispose();
            throw;
        }
    }

    private static OdfFile OpenFlat(Stream stream)
    {
        XDocument? document = OdfXml.TryLoad(stream, out string? error);
        XElement? root = document?.Root;
        if (root is null)
        {
            throw new MalformedDocumentException(
                $"The document is neither an ODF package nor parseable XML{(error is null ? "." : $": {error}")}");
        }

        OdfFile file = new(null, root, root)
        {
            MimeType = root.Attribute(XName.Get("mimetype", OdfNamespaces.Office))?.Value,
        };
        file.Meta = root.Element(XName.Get("meta", OdfNamespaces.Office));
        file.Settings = root.Element(XName.Get("settings", OdfNamespaces.Office));
        return file;
    }

    private static XElement? LoadPart(OdfPackage package, string partName, out string? error)
    {
        error = null;
        IPackagePart? part = package.GetPart(partName);
        if (part is null) return null;

        using Stream content = part.Open();
        return OdfXml.TryLoad(content, out error)?.Root;
    }
}
