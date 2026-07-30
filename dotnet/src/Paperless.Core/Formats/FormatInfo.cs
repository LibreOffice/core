namespace Paperless.Core.Formats;

/// <summary>
/// Static facts about a <see cref="DocumentFormat"/>: how it is packaged, which
/// application owns it, and how it presents itself to the outside world.
/// </summary>
/// <remarks>
/// This is the Paperless equivalent of a LibreOffice type/filter registry entry
/// (<c>filter/source/config/fragments/types/*.xcu</c>). The authoritative table
/// this is derived from lives in <c>dotnet/research/01-formats-and-detection.md</c>.
/// </remarks>
public sealed record FormatInfo
{
    /// <summary>The format this describes.</summary>
    public required DocumentFormat Format { get; init; }

    /// <summary>Which application the format belongs to.</summary>
    public required DocumentFamily Family { get; init; }

    /// <summary>The physical container the payload is wrapped in.</summary>
    public required ContainerKind Container { get; init; }

    /// <summary>The canonical file extension, without a leading dot.</summary>
    public required string Extension { get; init; }

    /// <summary>Additional extensions that map to this format.</summary>
    public IReadOnlyList<string> AlternateExtensions { get; init; } = [];

    /// <summary>The primary IANA media type.</summary>
    public required string MediaType { get; init; }

    /// <summary>A short human-readable name, e.g. "Word 2007-365".</summary>
    public required string DisplayName { get; init; }

    /// <summary>True when the format is a document template rather than a document.</summary>
    public bool IsTemplate { get; init; }

    /// <summary>
    /// True when the format is able to carry macros. Paperless never executes
    /// them; this exists so callers can surface the risk.
    /// </summary>
    public bool CanCarryMacros { get; init; }

    /// <summary>
    /// True when Paperless can read the format. Formats are listed in the
    /// catalogue before they are implemented, so this starts out false.
    /// </summary>
    public bool IsReadSupported { get; init; }
}

/// <summary>
/// The registry of every format Paperless knows about.
/// </summary>
public interface IFormatCatalogue
{
    /// <summary>Every format in the catalogue.</summary>
    IReadOnlyCollection<FormatInfo> All { get; }

    /// <summary>Looks up static facts about a format.</summary>
    FormatInfo? GetInfo(DocumentFormat format);

    /// <summary>
    /// Finds the formats a file extension could denote. An extension can be
    /// ambiguous, so this returns a list — but note that extensions are only ever
    /// a hint: <see cref="IFormatIdentifier"/> decides based on content.
    /// </summary>
    IReadOnlyList<FormatInfo> FindByExtension(string extension);
}

/// <summary>
/// The outcome of sniffing a stream for its format.
/// </summary>
/// <param name="Format">The format identified, or <see cref="DocumentFormat.Unknown"/>.</param>
/// <param name="Container">The container that was recognised, even when the specific format was not.</param>
/// <param name="Confidence">How sure the identifier is.</param>
/// <param name="IsEncrypted">True when the payload is password-protected and needs a password to read.</param>
/// <param name="Detail">A human-readable note on how the decision was reached, for diagnostics.</param>
public readonly record struct FormatIdentification(
    DocumentFormat Format,
    ContainerKind Container,
    IdentificationConfidence Confidence,
    bool IsEncrypted,
    string? Detail)
{
    /// <summary>Nothing was recognised.</summary>
    public static readonly FormatIdentification Unknown = new(
        DocumentFormat.Unknown, ContainerKind.Unknown, IdentificationConfidence.None, false, null);
}

/// <summary>How firmly a format was identified.</summary>
public enum IdentificationConfidence
{
    /// <summary>Nothing matched.</summary>
    None = 0,

    /// <summary>Only the file extension suggested this format; content was not conclusive.</summary>
    ExtensionOnly,

    /// <summary>The container was recognised but the specific format was inferred rather than proven.</summary>
    Probable,

    /// <summary>Content uniquely identifies the format — a matched signature, stream name or content type.</summary>
    Certain,
}

/// <summary>
/// Determines a document's real format from its content.
/// </summary>
/// <remarks>
/// Content always wins over the file extension: mislabelled files are extremely
/// common in the wild, and several important distinctions (DOCX versus DOCM, or
/// which application owns an OLE2 file) cannot be made from the name at all.
/// </remarks>
public interface IFormatIdentifier
{
    /// <summary>
    /// Sniffs a seekable stream. The stream position is restored before returning.
    /// </summary>
    /// <param name="stream">A seekable stream positioned anywhere.</param>
    /// <param name="fileNameHint">
    /// An optional file name used only to break ties that content cannot resolve.
    /// </param>
    FormatIdentification Identify(Stream stream, string? fileNameHint = null);
}
