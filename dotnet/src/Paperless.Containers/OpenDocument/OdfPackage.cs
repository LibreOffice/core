using Paperless.Core.Formats;

namespace Paperless.Containers.OpenDocument;

/// <summary>
/// Reader for OpenDocument packages: a ZIP whose first entry is an uncompressed
/// <c>mimetype</c> stream, described by <c>META-INF/manifest.xml</c>.
/// </summary>
/// <remarks>
/// The <c>mimetype</c> entry is the authoritative statement of what the document is,
/// and LibreOffice cross-checks it against the manifest's root media type. Paperless
/// does the same but treats a mismatch as a diagnostic rather than a hard error, so
/// that slightly malformed files still read.
/// </remarks>
public sealed class OdfPackage : IPackage
{
    private OdfPackage() { }

    /// <inheritdoc/>
    public ContainerKind Kind => ContainerKind.Zip;

    /// <inheritdoc/>
    public IEnumerable<IPackagePart> Parts => throw new NotImplementedException();

    /// <summary>The value of the package's <c>mimetype</c> entry.</summary>
    public string MimeType => throw new NotImplementedException();

    /// <summary>Opens an ODF package over a seekable stream.</summary>
    public static OdfPackage Open(Stream stream, bool leaveOpen = false)
        => throw new NotImplementedException();

    /// <inheritdoc/>
    public IPackagePart? GetPart(string name) => throw new NotImplementedException();

    /// <inheritdoc/>
    public void Dispose() { }
}
