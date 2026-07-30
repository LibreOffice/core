using Paperless.Core.Formats;

namespace Paperless.Containers.Ole2;

/// <summary>
/// Reader for OLE2 / Compound File Binary containers — the format underneath DOC,
/// XLS and PPT, and underneath encrypted OOXML files.
/// </summary>
/// <remarks>
/// See <c>dotnet/research/05-infrastructure.md</c> section A for the on-disk layout
/// this implements, including exact header and directory-entry field offsets.
/// Tolerance of malformed files is a requirement, not a nicety: files with wrong
/// CLSIDs, oversized FAT chains and inconsistent directory trees are common enough
/// in real corpora that rejecting them is not an option.
/// </remarks>
public sealed class CompoundFile : IPackage
{
    private CompoundFile() { }

    /// <inheritdoc/>
    public ContainerKind Kind => ContainerKind.Ole2CompoundFile;

    /// <inheritdoc/>
    public IEnumerable<IPackagePart> Parts => throw new NotImplementedException();

    /// <summary>The root storage's CLSID, when the file records a meaningful one.</summary>
    public Guid RootClassId => throw new NotImplementedException();

    /// <summary>Opens a compound file over a seekable stream.</summary>
    public static CompoundFile Open(Stream stream, bool leaveOpen = false)
        => throw new NotImplementedException();

    /// <inheritdoc/>
    public IPackagePart? GetPart(string name) => throw new NotImplementedException();

    /// <inheritdoc/>
    public void Dispose() { }
}
