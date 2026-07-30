using Paperless.Core.Formats;

namespace Paperless.Containers;

/// <summary>
/// Uniform read-only access to the container underneath a document, whatever its
/// physical form.
/// </summary>
/// <remarks>
/// <para>
/// The three container families expose fundamentally the same shape — named parts
/// holding bytes — so the format readers above this layer should not care which they
/// are sitting on. An OLE2 directory entry, an OPC part and an ODF package entry all
/// become an <see cref="IPackagePart"/>.
/// </para>
/// <para>
/// Part names are normalised to forward-slash-separated paths without a leading
/// slash, e.g. <c>word/document.xml</c> or <c>ObjectPool/_1234/WordDocument</c>.
/// Comparison is ordinal and case-sensitive: OPC part names are case-sensitive per
/// spec, and OLE2 stream names are too.
/// </para>
/// </remarks>
public interface IPackage : IDisposable
{
    /// <summary>Which physical container this is.</summary>
    ContainerKind Kind { get; }

    /// <summary>Every part in the container, in no guaranteed order.</summary>
    IEnumerable<IPackagePart> Parts { get; }

    /// <summary>Looks up a part by normalised name, returning null if absent.</summary>
    IPackagePart? GetPart(string name);
}

/// <summary>A single named stream of bytes inside an <see cref="IPackage"/>.</summary>
public interface IPackagePart
{
    /// <summary>The normalised part name.</summary>
    string Name { get; }

    /// <summary>
    /// The declared media type, where the container records one. OPC always does;
    /// ODF records it only for the package as a whole; OLE2 never does.
    /// </summary>
    string? MediaType { get; }

    /// <summary>The uncompressed length in bytes, or -1 when not known without reading.</summary>
    long Length { get; }

    /// <summary>
    /// Opens the part's contents, decompressed and decrypted. Callers dispose the
    /// returned stream; parts may be opened more than once.
    /// </summary>
    Stream Open();
}
