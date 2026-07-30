using Paperless.Core.Formats;

namespace Paperless.Containers.Ooxml;

/// <summary>
/// Reader for Open Packaging Conventions packages — the ZIP-plus-relationships
/// container under every OOXML format.
/// </summary>
/// <remarks>
/// Adds two things over a plain ZIP: content types resolved from
/// <c>[Content_Types].xml</c> (defaults by extension, then per-part overrides), and
/// relationships from the <c>_rels/</c> parts. Both are needed before a reader can
/// find anything — the main document part is located by following a relationship
/// from the package root, never by assuming a path.
/// </remarks>
public sealed class OpcPackage : IPackage
{
    private OpcPackage() { }

    /// <inheritdoc/>
    public ContainerKind Kind => ContainerKind.Zip;

    /// <inheritdoc/>
    public IEnumerable<IPackagePart> Parts => throw new NotImplementedException();

    /// <summary>Opens an OPC package over a seekable stream.</summary>
    public static OpcPackage Open(Stream stream, bool leaveOpen = false)
        => throw new NotImplementedException();

    /// <inheritdoc/>
    public IPackagePart? GetPart(string name) => throw new NotImplementedException();

    /// <summary>
    /// Returns the relationships declared by a part, or by the package root when
    /// <paramref name="partName"/> is null.
    /// </summary>
    public IReadOnlyList<OpcRelationship> GetRelationships(string? partName = null)
        => throw new NotImplementedException();

    /// <inheritdoc/>
    public void Dispose() { }
}

/// <summary>One OPC relationship.</summary>
/// <param name="Id">The relationship id, unique within its source part.</param>
/// <param name="Type">The relationship type URI.</param>
/// <param name="Target">The target, resolved to a normalised part name when internal.</param>
/// <param name="IsExternal">True when the target is outside the package.</param>
public readonly record struct OpcRelationship(string Id, string Type, string Target, bool IsExternal);
