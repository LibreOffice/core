using System.Xml;

namespace Paperless.Containers.Ooxml;

/// <summary>
/// Parsers for the two XML parts that describe an OPC package: <c>[Content_Types].xml</c>
/// and the <c>_rels/</c> relationship parts.
/// </summary>
/// <remarks>
/// Shared by <see cref="OpcPackage"/> and by format identification, so the two cannot drift
/// apart on how a content type is resolved — which is exactly the sort of divergence that
/// makes a file identify as one format and then load as another.
/// </remarks>
public static class OpcXml
{
    /// <summary>The relationship type of the package's main document part.</summary>
    public const string OfficeDocumentRelationshipType =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument";

    /// <summary>The name of the content-types part. Fixed by the specification.</summary>
    public const string ContentTypesPartName = "[Content_Types].xml";

    /// <summary>The name of the package-level relationships part.</summary>
    public const string RootRelationshipsPartName = "_rels/.rels";

    /// <summary>
    /// Creates a reader configured for untrusted XML.
    /// </summary>
    /// <remarks>
    /// <see cref="DtdProcessing.Prohibit"/> and a null resolver are the important parts:
    /// office documents are untrusted input, and an external DTD or entity reference would
    /// otherwise be an XXE and SSRF vector. Every XML read in this library goes through here.
    /// </remarks>
    public static XmlReader CreateSafeReader(TextReader input)
    {
        ArgumentNullException.ThrowIfNull(input);
        return XmlReader.Create(input, new XmlReaderSettings
        {
            DtdProcessing = DtdProcessing.Prohibit,
            XmlResolver = null,
            IgnoreWhitespace = true,
            IgnoreComments = true,
            IgnoreProcessingInstructions = false,
            CloseInput = false,
        });
    }

    /// <summary>
    /// The content types declared by a package: defaults keyed by extension, plus per-part
    /// overrides.
    /// </summary>
    public sealed class ContentTypeMap
    {
        private readonly Dictionary<string, string> _defaults = new(StringComparer.OrdinalIgnoreCase);
        private readonly Dictionary<string, string> _overrides = new(StringComparer.OrdinalIgnoreCase);

        /// <summary>Every declared content type, for diagnostics and fallback matching.</summary>
        public IEnumerable<string> AllDeclaredTypes => _defaults.Values.Concat(_overrides.Values);

        /// <summary>
        /// Resolves a part's content type: an exact override wins, otherwise the default for
        /// its extension. Returns null when neither is declared.
        /// </summary>
        public string? Resolve(string partName)
        {
            ArgumentNullException.ThrowIfNull(partName);
            string normalised = "/" + Zip.ZipPackageBase.NormalisePartName(partName);
            if (_overrides.TryGetValue(normalised, out string? exact)) return exact;

            string extension = Path.GetExtension(normalised).TrimStart('.');
            return extension.Length > 0 && _defaults.TryGetValue(extension, out string? byExtension)
                ? byExtension
                : null;
        }

        /// <summary>Parses a <c>[Content_Types].xml</c> document. Never throws on bad XML.</summary>
        public static ContentTypeMap Parse(string xml)
        {
            ContentTypeMap map = new();
            if (string.IsNullOrEmpty(xml)) return map;

            try
            {
                using XmlReader reader = CreateSafeReader(new StringReader(xml));
                while (reader.Read())
                {
                    if (reader.NodeType != XmlNodeType.Element) continue;

                    if (string.Equals(reader.LocalName, "Default", StringComparison.Ordinal))
                    {
                        string? ext = reader.GetAttribute("Extension");
                        string? type = reader.GetAttribute("ContentType");
                        if (ext is not null && type is not null) map._defaults[ext.TrimStart('.')] = type;
                    }
                    else if (string.Equals(reader.LocalName, "Override", StringComparison.Ordinal))
                    {
                        string? part = reader.GetAttribute("PartName");
                        string? type = reader.GetAttribute("ContentType");
                        if (part is not null && type is not null)
                            map._overrides["/" + Zip.ZipPackageBase.NormalisePartName(part)] = type;
                    }
                }
            }
            catch (XmlException)
            {
                // Partial results are more useful than none: a truncated content-types part
                // still tells us about the entries that were parsed before the break.
            }
            return map;
        }
    }

    /// <summary>One OPC relationship.</summary>
    /// <param name="Id">The relationship id, unique within its source part.</param>
    /// <param name="Type">The relationship type URI.</param>
    /// <param name="Target">The target, normalised when internal.</param>
    /// <param name="IsExternal">True when the target lies outside the package.</param>
    public readonly record struct Relationship(string Id, string Type, string Target, bool IsExternal);

    /// <summary>
    /// Parses a relationships part.
    /// </summary>
    /// <param name="xml">The <c>.rels</c> content.</param>
    /// <param name="sourcePartName">
    /// The part these relationships belong to, or null for the package root. Relative targets
    /// resolve against its directory.
    /// </param>
    public static List<Relationship> ParseRelationships(string xml, string? sourcePartName)
    {
        List<Relationship> result = [];
        if (string.IsNullOrEmpty(xml)) return result;

        string baseDirectory = sourcePartName is null
            ? string.Empty
            : GetDirectory(Zip.ZipPackageBase.NormalisePartName(sourcePartName));

        try
        {
            using XmlReader reader = CreateSafeReader(new StringReader(xml));
            while (reader.Read())
            {
                if (reader.NodeType != XmlNodeType.Element ||
                    !string.Equals(reader.LocalName, "Relationship", StringComparison.Ordinal))
                    continue;

                string? id = reader.GetAttribute("Id");
                string? type = reader.GetAttribute("Type");
                string? target = reader.GetAttribute("Target");
                if (type is null || target is null) continue;

                bool external = string.Equals(reader.GetAttribute("TargetMode"), "External",
                                              StringComparison.OrdinalIgnoreCase);

                // An absolute target is package-rooted; a relative one resolves against the
                // source part's directory. Getting this backwards is a classic OPC bug.
                string resolved = external
                    ? target
                    : target.StartsWith('/')
                        ? Zip.ZipPackageBase.NormalisePartName(target)
                        : Zip.ZipPackageBase.NormalisePartName(
                              baseDirectory.Length == 0 ? target : baseDirectory + "/" + target);

                result.Add(new Relationship(id ?? string.Empty, type, resolved, external));
            }
        }
        catch (XmlException)
        {
            // Return whatever parsed. A malformed .rels is recoverable: callers fall back to
            // scanning the content types.
        }
        return result;
    }

    /// <summary>The name of the relationships part describing a given part.</summary>
    /// <remarks>
    /// <c>word/document.xml</c> is described by <c>word/_rels/document.xml.rels</c>, and the
    /// package root by <c>_rels/.rels</c>.
    /// </remarks>
    public static string GetRelationshipsPartName(string? partName)
    {
        if (partName is null) return RootRelationshipsPartName;
        string normalised = Zip.ZipPackageBase.NormalisePartName(partName);
        string directory = GetDirectory(normalised);
        string file = Path.GetFileName(normalised);
        return directory.Length == 0 ? $"_rels/{file}.rels" : $"{directory}/_rels/{file}.rels";
    }

    private static string GetDirectory(string partName)
    {
        int slash = partName.LastIndexOf('/');
        return slash < 0 ? string.Empty : partName[..slash];
    }
}
