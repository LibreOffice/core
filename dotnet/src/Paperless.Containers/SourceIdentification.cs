using Paperless.Core.Documents;
using Paperless.Core.Formats;

namespace Paperless.Containers;

/// <summary>
/// Identifies the format of a <see cref="DocumentSource"/> without consuming it.
/// </summary>
/// <remarks>
/// Every format reader needs the same two steps — honour an explicitly requested format,
/// otherwise sniff the content and rewind — and getting the rewind wrong leaves the stream
/// mid-header, which fails later in a way that looks like a corrupt file. Doing it once here
/// keeps the three family readers from each having their own version.
/// </remarks>
public static class SourceIdentification
{
    /// <summary>
    /// Determines what a source holds, leaving its stream positioned where it started.
    /// </summary>
    /// <param name="source">The source to inspect.</param>
    /// <returns>
    /// The format from <see cref="DocumentSource.Format"/> when the caller named one,
    /// otherwise the result of sniffing the content. The file name is used only as a
    /// tie-breaker, never as the primary signal.
    /// </returns>
    public static DocumentFormat Resolve(DocumentSource source)
    {
        ArgumentNullException.ThrowIfNull(source);
        if (source.Format != DocumentFormat.Unknown) return source.Format;

        long start = source.Stream.Position;
        try
        {
            return FormatIdentifier.Instance.Identify(source.Stream, source.FileName).Format;
        }
        finally
        {
            source.Stream.Position = start;
        }
    }
}
