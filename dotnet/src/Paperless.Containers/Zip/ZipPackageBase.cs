using System.IO.Compression;
using System.Text;
using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Formats;

namespace Paperless.Containers.Zip;

/// <summary>
/// Shared behaviour for the two ZIP-based container families, OPC and ODF.
/// </summary>
/// <remarks>
/// Both are a ZIP of named parts; they differ only in how they describe themselves. Keeping
/// the ZIP handling — including the safety limits, which are the part most costly to get
/// wrong twice — in one place means a fix applies to both.
/// </remarks>
public abstract class ZipPackageBase : IPackage
{
    /// <summary>
    /// Limits applied while reading a package.
    /// </summary>
    /// <remarks>
    /// These exist because packages arrive from untrusted sources. A ZIP can declare a
    /// modest compressed size and expand to terabytes ("zip bomb"), and entry names can
    /// contain <c>..</c> segments aimed at escaping a target directory. Neither is
    /// hypothetical for office documents, which are routinely emailed around.
    /// </remarks>
    public sealed record Limits
    {
        /// <summary>The defaults, generous enough for any real document.</summary>
        public static readonly Limits Default = new();

        /// <summary>
        /// Largest total uncompressed size to extract across all parts. 2 GiB is far above
        /// any legitimate office document.
        /// </summary>
        public long MaxTotalUncompressedBytes { get; init; } = 2L * 1024 * 1024 * 1024;

        /// <summary>Largest uncompressed size for a single part.</summary>
        public long MaxPartBytes { get; init; } = 512L * 1024 * 1024;

        /// <summary>Largest number of entries. Real documents have hundreds, not millions.</summary>
        public int MaxEntryCount { get; init; } = 64 * 1024;

        /// <summary>
        /// Largest permitted compression ratio for a single part.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This is a NARROW guard, and the number is not arbitrary. DEFLATE cannot exceed
        /// about 1028:1 in practice (measured), so any threshold above that can never fire
        /// — which is worse than no guard, because it looks like protection. Meanwhile
        /// realistic repetitive office XML reaches around 342:1 (also measured), so a
        /// threshold much below 500 would reject legitimate documents.
        /// </para>
        /// <para>
        /// 1024 therefore catches only near-maximal payloads — a part that is essentially
        /// one byte repeated, which no real document contains — and nothing else. The guards
        /// that actually bound the work are
        /// <see cref="MaxPartBytes"/> and <see cref="MaxTotalUncompressedBytes"/>;
        /// treat those as the real defence and this as an early signal.
        /// </para>
        /// </remarks>
        public int MaxCompressionRatio { get; init; } = 1024;
    }

    private readonly ZipArchive _archive;
    private readonly bool _ownsStream;
    private readonly Stream _stream;
    private readonly Dictionary<string, ZipArchiveEntry> _entries = new(StringComparer.Ordinal);
    private readonly List<Diagnostic> _diagnostics = [];
    private long _bytesExtracted;
    private bool _disposed;

    /// <summary>Opens the archive and indexes its entries.</summary>
    /// <param name="stream">A seekable stream over the package.</param>
    /// <param name="leaveOpen">When true, disposing this does not dispose the stream.</param>
    /// <param name="limits">Safety limits; null uses the defaults.</param>
    protected ZipPackageBase(Stream stream, bool leaveOpen, Limits? limits)
    {
        ArgumentNullException.ThrowIfNull(stream);
        if (!stream.CanSeek)
            throw new ArgumentException("A package must be read from a seekable stream.", nameof(stream));

        _stream = stream;
        _ownsStream = !leaveOpen;
        PackageLimits = limits ?? Limits.Default;

        stream.Position = 0;
        try
        {
            _archive = new ZipArchive(stream, ZipArchiveMode.Read, leaveOpen: true);
        }
        catch (InvalidDataException ex)
        {
            throw new MalformedDocumentException("The package is not a readable ZIP archive.", ex);
        }

        IndexEntries();
    }

    /// <inheritdoc/>
    public ContainerKind Kind => ContainerKind.Zip;

    /// <summary>The limits in force.</summary>
    public Limits PackageLimits { get; }

    /// <summary>Problems found while reading the package.</summary>
    public IReadOnlyList<Diagnostic> Diagnostics => _diagnostics;

    /// <inheritdoc/>
    public IEnumerable<IPackagePart> Parts
    {
        get
        {
            ObjectDisposedException.ThrowIf(_disposed, this);
            foreach (KeyValuePair<string, ZipArchiveEntry> kv in _entries)
                yield return CreatePart(kv.Key, kv.Value);
        }
    }

    /// <inheritdoc/>
    public IPackagePart? GetPart(string name)
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
        ArgumentNullException.ThrowIfNull(name);
        string key = NormalisePartName(name);
        return _entries.TryGetValue(key, out ZipArchiveEntry? entry) ? CreatePart(key, entry) : null;
    }

    /// <summary>Returns the media type a subclass assigns to a part, if any.</summary>
    protected abstract string? GetMediaType(string partName);

    /// <summary>Records a warning-level diagnostic.</summary>
    protected void Warn(string code, string message)
        => _diagnostics.Add(new Diagnostic(DiagnosticSeverity.Warning, code, message));

    /// <summary>
    /// Normalises a part name: forward slashes, no leading slash, no <c>.</c>/<c>..</c>
    /// segments.
    /// </summary>
    /// <remarks>
    /// Comparison stays ordinal and case-sensitive, because both OPC part names and ODF
    /// entry names are case-sensitive per their specifications.
    /// </remarks>
    public static string NormalisePartName(string name)
    {
        ArgumentNullException.ThrowIfNull(name);
        string flat = name.Replace('\\', '/').TrimStart('/');
        if (!flat.Contains("./", StringComparison.Ordinal) && !flat.EndsWith("/.", StringComparison.Ordinal))
            return flat;

        // Resolve '.' and '..' rather than rejecting outright: some producers emit them
        // legitimately in relationship targets.
        List<string> segments = [];
        foreach (string segment in flat.Split('/'))
        {
            if (segment is "" or ".") continue;
            if (segment == "..")
            {
                if (segments.Count > 0) segments.RemoveAt(segments.Count - 1);
                continue;
            }
            segments.Add(segment);
        }
        return string.Join('/', segments);
    }

    /// <summary>Reads a part fully as text, up to a cap, returning empty when unreadable.</summary>
    protected string ReadPartText(string partName, int maxBytes)
    {
        if (!_entries.TryGetValue(NormalisePartName(partName), out ZipArchiveEntry? entry))
            return string.Empty;
        try
        {
            using Stream s = entry.Open();
            byte[] buffer = new byte[Math.Min(maxBytes, Math.Max(1, entry.Length))];
            int total = 0;
            while (total < buffer.Length)
            {
                int read = s.Read(buffer, total, buffer.Length - total);
                if (read == 0) break;
                total += read;
            }
            return DecodeXmlText(buffer.AsSpan(0, total));
        }
        catch (InvalidDataException)
        {
            Warn("PL1201", $"Part '{partName}' could not be decompressed; treated as empty.");
            return string.Empty;
        }
    }

    /// <summary>Decodes bytes as text, honouring a byte-order mark.</summary>
    private static string DecodeXmlText(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF)
            return Encoding.UTF8.GetString(bytes[3..]);
        if (bytes.Length >= 2 && bytes[0] == 0xFF && bytes[1] == 0xFE)
            return Encoding.Unicode.GetString(bytes[2..]);
        if (bytes.Length >= 2 && bytes[0] == 0xFE && bytes[1] == 0xFF)
            return Encoding.BigEndianUnicode.GetString(bytes[2..]);
        return Encoding.UTF8.GetString(bytes);
    }

    private void IndexEntries()
    {
        int count = 0;
        foreach (ZipArchiveEntry entry in _archive.Entries)
        {
            if (++count > PackageLimits.MaxEntryCount)
            {
                Warn("PL1202",
                     $"Package has more than {PackageLimits.MaxEntryCount} entries; the rest were ignored.");
                break;
            }

            // Directory entries carry no content.
            if (entry.FullName.EndsWith('/')) continue;

            string normalised = NormalisePartName(entry.FullName);
            if (normalised.Length == 0) continue;

            if (!_entries.TryAdd(normalised, entry))
            {
                // Duplicate names are malformed but occur; the first wins, matching how
                // most readers behave.
                Warn("PL1203", $"Duplicate package entry '{normalised}'; the first was kept.");
            }
        }
    }

    private ZipPart CreatePart(string name, ZipArchiveEntry entry)
        => new(this, name, entry, GetMediaType(name));

    /// <summary>
    /// Opens an entry, enforcing the per-part and total extraction limits.
    /// </summary>
    private Stream OpenEntry(ZipArchiveEntry entry, string name)
    {
        long declared = entry.Length;
        if (declared > PackageLimits.MaxPartBytes)
        {
            throw new MalformedDocumentException(
                $"Part '{name}' declares {declared} bytes, above the {PackageLimits.MaxPartBytes}-byte limit.");
        }

        // Only near-maximal ratios are distinguishable from legitimate XML; see the remarks
        // on MaxCompressionRatio. The size caps above and below do the real work.
        if (entry.CompressedLength > 0 && declared / entry.CompressedLength > PackageLimits.MaxCompressionRatio)
        {
            throw new MalformedDocumentException(
                $"Part '{name}' has a compression ratio of "
                + $"{declared / entry.CompressedLength}:1, above the "
                + $"{PackageLimits.MaxCompressionRatio}:1 limit. Refusing to expand it.");
        }

        long projected = _bytesExtracted + declared;
        if (projected > PackageLimits.MaxTotalUncompressedBytes)
        {
            throw new MalformedDocumentException(
                $"Extracting part '{name}' would exceed the "
                + $"{PackageLimits.MaxTotalUncompressedBytes}-byte total limit for this package.");
        }
        _bytesExtracted = projected;

        try
        {
            return entry.Open();
        }
        catch (InvalidDataException ex)
        {
            throw new MalformedDocumentException($"Part '{name}' could not be decompressed.", ex);
        }
    }

    /// <inheritdoc/>
    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        _archive.Dispose();
        if (_ownsStream) _stream.Dispose();
        GC.SuppressFinalize(this);
    }

    private sealed class ZipPart(ZipPackageBase owner, string name, ZipArchiveEntry entry, string? mediaType)
        : IPackagePart
    {
        public string Name { get; } = name;
        public string? MediaType { get; } = mediaType;
        public long Length => entry.Length;
        public Stream Open() => owner.OpenEntry(entry, Name);
    }
}
