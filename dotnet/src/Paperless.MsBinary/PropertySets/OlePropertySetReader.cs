using Paperless.Core.Documents;

namespace Paperless.MsBinary.PropertySets;

/// <summary>
/// Reads the OLE property sets that carry document metadata in legacy binary files:
/// the summary information and document summary information streams.
/// </summary>
/// <remarks>
/// Both stream names begin with the character U+0005, which is part of the name rather
/// than an escape sequence — hence the explicit construction in
/// <see cref="SummaryInformationStreamName"/>. Property identifiers are numeric, and
/// their meaning depends on which of the two property sets they appear in, so the same
/// identifier means different things in each. See
/// <c>dotnet/research/05-infrastructure.md</c> section G.
/// </remarks>
public static class OlePropertySetReader
{
    /// <summary>The name of the summary information stream, including its leading U+0005.</summary>
    public const string SummaryInformationStreamName = "\u0005SummaryInformation";

    /// <summary>The name of the document summary information stream, including its leading U+0005.</summary>
    public const string DocumentSummaryInformationStreamName = "\u0005DocumentSummaryInformation";

    /// <summary>
    /// Reads both property-set streams from a compound file, if present, and maps them
    /// onto the normalised metadata model.
    /// </summary>
    public static DocumentMetadata Read(Containers.IPackage package)
        => throw new NotImplementedException();
}
