namespace Paperless.MsBinary.Records;

/// <summary>
/// Reads the record streams the legacy Microsoft binary formats are built from.
/// </summary>
/// <remarks>
/// <para>
/// DOC, XLS and PPT all store their content as sequences of length-prefixed records,
/// and although the three differ in detail they share enough structure to justify one
/// reader: a small header giving a type and a length, followed by that many bytes of
/// payload, sometimes nested.
/// </para>
/// <para>
/// Robustness is the main design concern. These files are thirty years old, written by
/// dozens of applications, and frequently contain records whose declared length runs
/// past the end of the stream. The reader must clamp and report rather than throw, or
/// a large fraction of a real corpus becomes unreadable.
/// </para>
/// </remarks>
public interface IRecordReader
{
    /// <summary>The current read position within the stream.</summary>
    long Position { get; }

    /// <summary>The stream's total length.</summary>
    long Length { get; }

    /// <summary>Advances to the next record.</summary>
    /// <returns>False at end of stream.</returns>
    bool MoveNext();

    /// <summary>The current record's type identifier.</summary>
    ushort RecordType { get; }

    /// <summary>
    /// The current record's payload, already clamped to what the stream actually holds.
    /// A record whose declared length overran will be shorter than declared, and that
    /// discrepancy is reported as a diagnostic.
    /// </summary>
    ReadOnlySpan<byte> Payload { get; }
}
