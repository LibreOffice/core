using System.Xml;
using Paperless.Containers.Ooxml;
using Paperless.Containers.Zip;

namespace Paperless.Containers.OpenDocument;

/// <summary>
/// Reader for OpenDocument packages: a ZIP whose first entry is an uncompressed
/// <c>mimetype</c> stream, described by <c>META-INF/manifest.xml</c>.
/// </summary>
/// <remarks>
/// The <c>mimetype</c> entry is the authoritative statement of what the document is.
/// LibreOffice cross-checks it against the manifest's root media type and throws on a
/// mismatch; Paperless records a diagnostic and prefers <c>mimetype</c> instead, so that
/// slightly malformed files still read. There is no upside to refusing a document whose two
/// self-descriptions merely disagree.
/// </remarks>
public sealed class OdfPackage : ZipPackageBase
{
    /// <summary>The well-known ODF part names.</summary>
    public static class PartNames
    {
        /// <summary>The document body.</summary>
        public const string Content = "content.xml";

        /// <summary>Named and automatic styles, and the master styles.</summary>
        public const string Styles = "styles.xml";

        /// <summary>Document metadata.</summary>
        public const string Meta = "meta.xml";

        /// <summary>Application settings, some of which affect layout.</summary>
        public const string Settings = "settings.xml";

        /// <summary>The package manifest.</summary>
        public const string Manifest = "META-INF/manifest.xml";

        /// <summary>The media-type declaration.</summary>
        public const string MimeType = "mimetype";
    }

    private readonly Dictionary<string, string> _manifestMediaTypes = new(StringComparer.Ordinal);

    private OdfPackage(Stream stream, bool leaveOpen, Limits? limits)
        : base(stream, leaveOpen, limits)
    {
        MimeType = ReadPartText(PartNames.MimeType, 512).Trim();
        if (MimeType.Length == 0)
            Warn("PL1220", "ODF package has no readable 'mimetype' entry.");

        ParseManifest();

        // Cross-check, but do not fail on a mismatch.
        if (ManifestRootMediaType is { Length: > 0 } manifestType
            && MimeType.Length > 0
            && !string.Equals(manifestType, MimeType, StringComparison.Ordinal))
        {
            Warn("PL1221",
                 $"The 'mimetype' entry says '{MimeType}' but the manifest root says "
                 + $"'{manifestType}'. Trusting 'mimetype'.");
        }
    }

    /// <summary>Opens an ODF package over a seekable stream.</summary>
    /// <param name="stream">A seekable stream over the package.</param>
    /// <param name="leaveOpen">When true, disposing this does not dispose the stream.</param>
    /// <param name="limits">Safety limits; null uses the defaults.</param>
    /// <exception cref="Core.MalformedDocumentException">The stream is not a readable ZIP.</exception>
    public static OdfPackage Open(Stream stream, bool leaveOpen = false, Limits? limits = null)
        => new(stream, leaveOpen, limits);

    /// <summary>
    /// The value of the package's <c>mimetype</c> entry, which is the authoritative statement
    /// of the document type. Empty when the entry is missing.
    /// </summary>
    public string MimeType { get; }

    /// <summary>The media type the manifest gives for the package root, if it declares one.</summary>
    public string? ManifestRootMediaType { get; private set; }

    /// <summary>
    /// True when the manifest declares any encrypted entry.
    /// </summary>
    /// <remarks>
    /// ODF encrypts per entry rather than wholesale, and an encrypted document's
    /// <c>mimetype</c> stays readable — so this is the only reliable signal, and a document
    /// can be partially encrypted.
    /// </remarks>
    public bool IsEncrypted { get; private set; }

    /// <inheritdoc/>
    protected override string? GetMediaType(string partName)
        => _manifestMediaTypes.TryGetValue(NormalisePartName(partName), out string? type) ? type : null;

    /// <summary>Opens <c>content.xml</c>, or null when absent.</summary>
    public Stream? OpenContent() => GetPart(PartNames.Content)?.Open();

    /// <summary>Opens <c>styles.xml</c>, or null when absent.</summary>
    public Stream? OpenStyles() => GetPart(PartNames.Styles)?.Open();

    /// <summary>Opens <c>meta.xml</c>, or null when absent.</summary>
    public Stream? OpenMeta() => GetPart(PartNames.Meta)?.Open();

    private void ParseManifest()
    {
        string xml = ReadPartText(PartNames.Manifest, 8 * 1024 * 1024);
        if (xml.Length == 0)
        {
            Warn("PL1222", "ODF package has no readable manifest; per-part media types are unavailable.");
            return;
        }

        try
        {
            using XmlReader reader = OpcXml.CreateSafeReader(new StringReader(xml));
            while (reader.Read())
            {
                if (reader.NodeType != XmlNodeType.Element ||
                    !string.Equals(reader.LocalName, "file-entry", StringComparison.Ordinal))
                    continue;

                // Attributes are manifest-namespaced; match on local name so a package using
                // an unexpected prefix still parses.
                string? path = GetAttributeByLocalName(reader, "full-path");
                string? mediaType = GetAttributeByLocalName(reader, "media-type");
                if (path is null) continue;

                if (path == "/")
                {
                    ManifestRootMediaType = mediaType;
                    continue;
                }
                if (mediaType is not null)
                    _manifestMediaTypes[NormalisePartName(path)] = mediaType;
            }

            // Encryption is declared as a manifest:encryption-data child of a file-entry.
            // A substring test is enough and avoids a second pass: the element name cannot
            // legitimately appear anywhere else.
            IsEncrypted = xml.Contains("encryption-data", StringComparison.Ordinal);
        }
        catch (XmlException ex)
        {
            Warn("PL1223", $"ODF manifest is malformed: {ex.Message}. Per-part media types may be incomplete.");
        }
    }

    private static string? GetAttributeByLocalName(XmlReader reader, string localName)
    {
        if (!reader.HasAttributes) return null;
        for (int i = 0; i < reader.AttributeCount; i++)
        {
            reader.MoveToAttribute(i);
            if (string.Equals(reader.LocalName, localName, StringComparison.Ordinal))
            {
                string value = reader.Value;
                reader.MoveToElement();
                return value;
            }
        }
        reader.MoveToElement();
        return null;
    }
}
