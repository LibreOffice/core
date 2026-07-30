using Paperless.Containers.Zip;

namespace Paperless.Containers.Ooxml;

/// <summary>
/// Reader for Open Packaging Conventions packages — the ZIP-plus-relationships container
/// under every OOXML format.
/// </summary>
/// <remarks>
/// Adds two things over a plain ZIP: content types resolved from
/// <c>[Content_Types].xml</c> (defaults by extension, then per-part overrides), and
/// relationships from the <c>_rels/</c> parts.
/// <para>
/// Both matter before a reader can find anything. The main document part is located by
/// following a relationship from the package root — never by assuming <c>word/document.xml</c>
/// — because the specification permits any part name and real producers use several.
/// </para>
/// </remarks>
public sealed class OpcPackage : ZipPackageBase
{
    private readonly OpcXml.ContentTypeMap _contentTypes;
    private readonly Dictionary<string, List<OpcXml.Relationship>> _relationshipCache = new(StringComparer.Ordinal);

    private OpcPackage(Stream stream, bool leaveOpen, Limits? limits)
        : base(stream, leaveOpen, limits)
    {
        string xml = ReadPartText(OpcXml.ContentTypesPartName, 4 * 1024 * 1024);
        if (xml.Length == 0)
        {
            // Not fatal: parts are still readable by name, and identification may already
            // know what this is. But nothing can be resolved by content type.
            Warn("PL1210",
                 $"Package has no readable '{OpcXml.ContentTypesPartName}'; content types are unavailable.");
        }
        _contentTypes = OpcXml.ContentTypeMap.Parse(xml);
    }

    /// <summary>Opens an OPC package over a seekable stream.</summary>
    /// <param name="stream">A seekable stream over the package.</param>
    /// <param name="leaveOpen">When true, disposing this does not dispose the stream.</param>
    /// <param name="limits">Safety limits; null uses the defaults.</param>
    /// <exception cref="Core.MalformedDocumentException">The stream is not a readable ZIP.</exception>
    public static OpcPackage Open(Stream stream, bool leaveOpen = false, Limits? limits = null)
        => new(stream, leaveOpen, limits);

    /// <summary>The content types the package declares.</summary>
    public OpcXml.ContentTypeMap ContentTypes => _contentTypes;

    /// <inheritdoc/>
    protected override string? GetMediaType(string partName) => _contentTypes.Resolve(partName);

    /// <summary>
    /// Returns the relationships declared by a part, or by the package root when
    /// <paramref name="partName"/> is null.
    /// </summary>
    public IReadOnlyList<OpcXml.Relationship> GetRelationships(string? partName = null)
    {
        string relsPart = OpcXml.GetRelationshipsPartName(partName);
        if (_relationshipCache.TryGetValue(relsPart, out List<OpcXml.Relationship>? cached))
            return cached;

        List<OpcXml.Relationship> parsed =
            OpcXml.ParseRelationships(ReadPartText(relsPart, 4 * 1024 * 1024), partName);
        _relationshipCache[relsPart] = parsed;
        return parsed;
    }

    /// <summary>
    /// Finds the relationships of a given type declared by a part.
    /// </summary>
    public IEnumerable<OpcXml.Relationship> GetRelationshipsByType(string relationshipType, string? partName = null)
    {
        ArgumentNullException.ThrowIfNull(relationshipType);
        foreach (OpcXml.Relationship rel in GetRelationships(partName))
        {
            if (string.Equals(rel.Type, relationshipType, StringComparison.OrdinalIgnoreCase))
                yield return rel;
        }
    }

    /// <summary>
    /// Follows the officeDocument relationship from the package root to the main document part.
    /// </summary>
    /// <remarks>
    /// Matches on the type's final segment as well as the full URI, because the strict and
    /// transitional OOXML variants use different namespace prefixes for the same
    /// relationship, and real packages mix them.
    /// </remarks>
    public IPackagePart? GetMainDocumentPart()
    {
        foreach (OpcXml.Relationship rel in GetRelationships(null))
        {
            if (rel.IsExternal) continue;
            bool isOfficeDocument =
                string.Equals(rel.Type, OpcXml.OfficeDocumentRelationshipType, StringComparison.OrdinalIgnoreCase)
                || rel.Type.EndsWith("/officeDocument", StringComparison.OrdinalIgnoreCase);
            if (!isOfficeDocument) continue;

            IPackagePart? part = GetPart(rel.Target);
            if (part is not null) return part;

            Warn("PL1211",
                 $"Root relationship points at '{rel.Target}', which is not present in the package.");
        }
        return null;
    }

    /// <summary>
    /// The content type of the main document part — the value that identifies which OOXML
    /// format the package holds.
    /// </summary>
    public string? GetMainDocumentContentType()
    {
        IPackagePart? main = GetMainDocumentPart();
        return main is null ? null : _contentTypes.Resolve(main.Name);
    }
}
