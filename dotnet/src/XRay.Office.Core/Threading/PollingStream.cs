namespace XRay.Office.Core.Threading;

/// <summary>
/// A read-only stream wrapper that checks the current operation for cancellation as it is read.
/// </summary>
/// <remarks>
/// Every parser in the family reads through a <see cref="Stream"/>, so wrapping the streams a package
/// hands out makes the work of decompressing and tokenising a part cancellable without a checkpoint in
/// each parser. The check is <see cref="OfficeOperation.Poll"/>, so it costs nothing outside an
/// operation scope and little inside one.
/// </remarks>
public sealed class PollingStream : Stream
{
    private readonly Stream _inner;

    /// <summary>Wraps a stream. Disposing the wrapper disposes the stream.</summary>
    public PollingStream(Stream inner)
    {
        ArgumentNullException.ThrowIfNull(inner);
        _inner = inner;
    }

    /// <inheritdoc/>
    public override bool CanRead => _inner.CanRead;

    /// <inheritdoc/>
    public override bool CanSeek => _inner.CanSeek;

    /// <inheritdoc/>
    public override bool CanWrite => false;

    /// <inheritdoc/>
    public override long Length => _inner.Length;

    /// <inheritdoc/>
    public override long Position
    {
        get => _inner.Position;
        set => _inner.Position = value;
    }

    /// <inheritdoc/>
    public override int Read(byte[] buffer, int offset, int count)
    {
        OfficeOperation.Poll();
        return _inner.Read(buffer, offset, count);
    }

    /// <inheritdoc/>
    public override int Read(Span<byte> buffer)
    {
        OfficeOperation.Poll();
        return _inner.Read(buffer);
    }

    /// <inheritdoc/>
    public override int ReadByte()
    {
        OfficeOperation.Poll();
        return _inner.ReadByte();
    }

    /// <inheritdoc/>
    public override long Seek(long offset, SeekOrigin origin) => _inner.Seek(offset, origin);

    /// <inheritdoc/>
    public override void SetLength(long value) => throw new NotSupportedException();

    /// <inheritdoc/>
    public override void Write(byte[] buffer, int offset, int count) => throw new NotSupportedException();

    /// <inheritdoc/>
    public override void Flush() { }

    /// <inheritdoc/>
    protected override void Dispose(bool disposing)
    {
        if (disposing) _inner.Dispose();
        base.Dispose(disposing);
    }
}
