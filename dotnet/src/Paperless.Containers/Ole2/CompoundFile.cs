using System.Buffers.Binary;
using System.Text;
using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Formats;

namespace Paperless.Containers.Ole2;

/// <summary>
/// Reader for OLE2 / Compound File Binary containers — the format underneath DOC, XLS and
/// PPT, and underneath encrypted OOXML files.
/// </summary>
/// <remarks>
/// <para>
/// Hand-rolled rather than delegated to a library. The requirement that decided it is
/// tolerance of malformed input: real corpora are full of files with wrong CLSIDs, FAT
/// chains running past the end of the file, cyclic chains, and directory trees whose
/// red-black invariants do not hold. A reader that throws on any of those loses a large
/// fraction of a real corpus, so this one repairs what it can, records the rest as a
/// <see cref="Diagnostic"/>, and keeps going.
/// </para>
/// <para>
/// On-disk layout is documented in <c>dotnet/research/05-infrastructure.md</c> section A.
/// </para>
/// </remarks>
public sealed class CompoundFile : IPackage
{
    /// <summary>The CFB signature: the first eight bytes of every compound file.</summary>
    private static ReadOnlySpan<byte> Signature => [0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1];

    // Sector-chain sentinels.
    private const uint FreeSector = 0xFFFFFFFF;
    private const uint EndOfChain = 0xFFFFFFFE;
    private const uint MaxRegularSector = 0xFFFFFFFA;

    private const int NoStream = -1;
    private const int DirectoryEntrySize = 128;
    private const int HeaderSize = 512;

    /// <summary>
    /// Upper bound on sectors followed in one chain. A malformed or hostile file can
    /// describe a cyclic or absurdly long chain; this caps the work.
    /// </summary>
    private const int MaxChainLength = 1 << 22;

    private readonly Stream _stream;
    private readonly bool _ownsStream;
    private readonly List<Diagnostic> _diagnostics = [];
    private readonly Dictionary<string, DirectoryEntry> _entriesByPath = new(StringComparer.Ordinal);

    private uint[] _fat = [];
    private uint[] _miniFat = [];
    private DirectoryEntry[] _directory = [];
    private byte[] _miniStream = [];
    private int _sectorSize;
    private int _miniSectorSize;
    private uint _miniStreamCutoff;
    private bool _disposed;

    private CompoundFile(Stream stream, bool ownsStream)
    {
        _stream = stream;
        _ownsStream = ownsStream;
    }

    /// <inheritdoc/>
    public ContainerKind Kind => ContainerKind.Ole2CompoundFile;

    /// <summary>Problems found while reading the container structure.</summary>
    public IReadOnlyList<Diagnostic> Diagnostics => _diagnostics;

    /// <summary>The root storage's CLSID.</summary>
    /// <remarks>
    /// Present for completeness, but <b>do not use it to identify the document type</b>.
    /// LibreOffice deliberately identifies DOC/XLS/PPT by stream name instead, because real
    /// files frequently carry a zero or wrong CLSID here.
    /// </remarks>
    public Guid RootClassId { get; private set; }

    /// <summary>
    /// The names of the streams directly under the root storage.
    /// </summary>
    /// <remarks>
    /// This is what format identification needs: the presence of <c>WordDocument</c>,
    /// <c>Workbook</c>, <c>Book</c> or <c>PowerPoint Document</c> at the top level is what
    /// distinguishes the legacy binary formats from one another.
    /// </remarks>
    public IReadOnlyCollection<string> RootStreamNames { get; private set; } = [];

    /// <inheritdoc/>
    public IEnumerable<IPackagePart> Parts
    {
        get
        {
            ObjectDisposedException.ThrowIf(_disposed, this);
            foreach (KeyValuePair<string, DirectoryEntry> kv in _entriesByPath)
                yield return new CompoundFilePart(this, kv.Key, kv.Value);
        }
    }

    /// <summary>
    /// Opens a compound file over a seekable stream.
    /// </summary>
    /// <param name="stream">A seekable stream positioned anywhere.</param>
    /// <param name="leaveOpen">When true, disposing this does not dispose the stream.</param>
    /// <exception cref="MalformedDocumentException">
    /// The stream is not a compound file, or its structure is damaged past recovery.
    /// </exception>
    public static CompoundFile Open(Stream stream, bool leaveOpen = false)
    {
        ArgumentNullException.ThrowIfNull(stream);
        if (!stream.CanSeek)
            throw new ArgumentException("A compound file must be read from a seekable stream.", nameof(stream));

        CompoundFile file = new(stream, ownsStream: !leaveOpen);
        try
        {
            file.Load();
            return file;
        }
        catch
        {
            file.Dispose();
            throw;
        }
    }

    /// <summary>
    /// Returns true when the stream starts with the compound-file signature.
    /// </summary>
    /// <remarks>A cheap pre-check for format identification. The stream position is restored.</remarks>
    public static bool HasSignature(Stream stream)
    {
        ArgumentNullException.ThrowIfNull(stream);
        if (!stream.CanSeek || stream.Length < Signature.Length) return false;

        long origin = stream.Position;
        try
        {
            stream.Position = 0;
            Span<byte> header = stackalloc byte[8];
            return ReadFully(stream, header) && header.SequenceEqual(Signature);
        }
        finally { stream.Position = origin; }
    }

    /// <inheritdoc/>
    public IPackagePart? GetPart(string name)
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
        ArgumentNullException.ThrowIfNull(name);
        string key = Normalise(name);
        return _entriesByPath.TryGetValue(key, out DirectoryEntry entry)
            ? new CompoundFilePart(this, key, entry)
            : null;
    }

    // ---------------------------------------------------------------- structure loading

    private void Load()
    {
        byte[] headerBuffer = new byte[HeaderSize];
        _stream.Position = 0;
        if (!ReadFully(_stream, headerBuffer))
            throw new MalformedDocumentException("Truncated compound file: header is shorter than 512 bytes.");
        ReadOnlySpan<byte> header = headerBuffer;

        if (!header[..8].SequenceEqual(Signature))
            throw new MalformedDocumentException("Not a compound file: signature mismatch.");

        RootClassId = new Guid(header.Slice(8, 16));

        ushort sectorShift = BinaryPrimitives.ReadUInt16LittleEndian(header[30..]);
        ushort miniSectorShift = BinaryPrimitives.ReadUInt16LittleEndian(header[32..]);

        // Derive sizes from the shifts rather than the version field: some writers set an
        // inconsistent version but a correct shift.
        if (sectorShift is < 7 or > 20)
            throw new MalformedDocumentException($"Implausible sector shift {sectorShift} in compound file header.");
        if (miniSectorShift is < 2 or > 12)
        {
            Warn("PL1101", $"Implausible mini-sector shift {miniSectorShift}; assuming the standard 6 (64 bytes).");
            miniSectorShift = 6;
        }
        _sectorSize = 1 << sectorShift;
        _miniSectorSize = 1 << miniSectorShift;

        uint fatSectorCount = BinaryPrimitives.ReadUInt32LittleEndian(header[44..]);
        uint firstDirSector = BinaryPrimitives.ReadUInt32LittleEndian(header[48..]);
        _miniStreamCutoff = BinaryPrimitives.ReadUInt32LittleEndian(header[56..]);
        uint firstMiniFatSector = BinaryPrimitives.ReadUInt32LittleEndian(header[60..]);
        uint miniFatSectorCount = BinaryPrimitives.ReadUInt32LittleEndian(header[64..]);
        uint firstDifatSector = BinaryPrimitives.ReadUInt32LittleEndian(header[68..]);

        if (_miniStreamCutoff == 0)
        {
            // 4096 is the only value the specification permits; a zero would send every
            // stream down the mini-FAT path.
            Warn("PL1102", "Mini-stream cutoff is zero; assuming the standard 4096.");
            _miniStreamCutoff = 4096;
        }

        // The DIFAT's first 109 entries live in the header; any further ones chain through
        // dedicated sectors.
        List<uint> difat = new(109);
        for (int i = 0; i < 109; i++)
        {
            uint sector = BinaryPrimitives.ReadUInt32LittleEndian(header[(76 + i * 4)..]);
            if (sector <= MaxRegularSector) difat.Add(sector);
        }
        ReadDifatChain(firstDifatSector, difat);

        if (fatSectorCount != 0 && difat.Count != fatSectorCount)
        {
            Warn("PL1103",
                 $"Header declares {fatSectorCount} FAT sector(s) but {difat.Count} were found; using what was found.");
        }

        _fat = ReadAllocationTable(difat);
        _miniFat = ReadMiniFat(firstMiniFatSector, miniFatSectorCount);
        LoadDirectory(firstDirSector);
    }

    private void ReadDifatChain(uint firstDifatSector, List<uint> difat)
    {
        uint sector = firstDifatSector;
        int guard = 0;
        int perSector = _sectorSize / 4;
        byte[] buffer = new byte[_sectorSize];

        while (sector <= MaxRegularSector && guard++ < MaxChainLength)
        {
            if (!TryReadSector(sector, buffer))
            {
                Warn("PL1104", $"DIFAT sector {sector} lies outside the file; truncating the DIFAT here.");
                return;
            }
            // The final slot of a DIFAT sector links to the next DIFAT sector.
            for (int i = 0; i < perSector - 1; i++)
            {
                uint entry = BinaryPrimitives.ReadUInt32LittleEndian(buffer.AsSpan(i * 4));
                if (entry <= MaxRegularSector) difat.Add(entry);
            }
            sector = BinaryPrimitives.ReadUInt32LittleEndian(buffer.AsSpan((perSector - 1) * 4));
        }

        if (guard >= MaxChainLength)
            Warn("PL1105", "DIFAT chain exceeded the safety limit; it is probably cyclic. Truncated.");
    }

    private uint[] ReadAllocationTable(List<uint> fatSectors)
    {
        int perSector = _sectorSize / 4;
        long total = (long)fatSectors.Count * perSector;
        if (total > int.MaxValue)
            throw new MalformedDocumentException("Compound file FAT is implausibly large.");

        uint[] fat = new uint[total];
        byte[] buffer = new byte[_sectorSize];
        int written = 0;
        foreach (uint sector in fatSectors)
        {
            if (!TryReadSector(sector, buffer))
            {
                Warn("PL1106", $"FAT sector {sector} lies outside the file; that range is treated as free.");
                for (int i = 0; i < perSector && written < fat.Length; i++) fat[written++] = FreeSector;
                continue;
            }
            for (int i = 0; i < perSector && written < fat.Length; i++)
                fat[written++] = BinaryPrimitives.ReadUInt32LittleEndian(buffer.AsSpan(i * 4));
        }
        return fat;
    }

    private uint[] ReadMiniFat(uint firstSector, uint declaredCount)
    {
        if (firstSector > MaxRegularSector) return [];

        List<uint> entries = [];
        byte[] buffer = new byte[_sectorSize];
        int perSector = _sectorSize / 4;
        uint sector = firstSector;
        int guard = 0;
        int limit = declaredCount == 0 ? MaxChainLength : (int)Math.Min(declaredCount, MaxChainLength);

        while (sector <= MaxRegularSector && guard++ < limit)
        {
            if (!TryReadSector(sector, buffer))
            {
                Warn("PL1107", $"Mini-FAT sector {sector} lies outside the file; truncating the mini-FAT here.");
                break;
            }
            for (int i = 0; i < perSector; i++)
                entries.Add(BinaryPrimitives.ReadUInt32LittleEndian(buffer.AsSpan(i * 4)));
            sector = NextInChain(sector);
        }
        return [.. entries];
    }

    private void LoadDirectory(uint firstDirSector)
    {
        byte[] raw = ReadChain(firstDirSector, expectedLength: -1, useMiniFat: false);
        int count = raw.Length / DirectoryEntrySize;
        if (count == 0) throw new MalformedDocumentException("Compound file has an empty directory.");

        DirectoryEntry[] entries = new DirectoryEntry[count];
        for (int i = 0; i < count; i++)
            entries[i] = DirectoryEntry.Parse(raw.AsSpan(i * DirectoryEntrySize, DirectoryEntrySize));
        _directory = entries;

        DirectoryEntry root = entries[0];
        if (root.Type != EntryType.Root)
            Warn("PL1108", $"First directory entry is {root.Type}, not the expected Root.");

        // The root entry's own stream *is* the mini-stream, so it has to be materialised
        // before any small stream can be resolved.
        if (root.StreamLength > 0 && root.StartSector <= MaxRegularSector)
        {
            _miniStream = ReadChain(
                root.StartSector,
                root.StreamLength > int.MaxValue ? -1 : (int)root.StreamLength,
                useMiniFat: false);
        }

        BuildPaths(root);
    }

    /// <summary>
    /// Flattens the directory into normalised paths.
    /// </summary>
    /// <remarks>
    /// The directory is a red-black tree per storage, but we only need every reachable entry,
    /// so this walks it as an ordinary binary tree and never relies on the ordering
    /// invariants — which malformed files routinely violate. The <c>visited</c> set guards
    /// against the cyclic child/sibling pointers such files also contain; without it a
    /// damaged file would loop forever.
    /// </remarks>
    private void BuildPaths(DirectoryEntry root)
    {
        HashSet<int> visited = [];
        List<string> rootStreams = [];
        Walk(root.ChildId, prefix: "", depth: 0, rootStreams);
        RootStreamNames = rootStreams;

        void Walk(int startId, string prefix, int depth, List<string>? collectStreamNames)
        {
            // Storage nesting deeper than this does not occur in real documents.
            if (depth > 64)
            {
                Warn("PL1109", "Directory nesting exceeded 64 levels; deeper entries were skipped.");
                return;
            }

            Stack<int> pending = new();
            pending.Push(startId);
            while (pending.Count > 0)
            {
                int current = pending.Pop();
                if (current == NoStream || (uint)current >= (uint)_directory.Length) continue;
                if (!visited.Add(current)) continue;   // cycle guard

                DirectoryEntry entry = _directory[current];
                if (entry.Type is not (EntryType.Stream or EntryType.Storage)) continue;

                string path = prefix.Length == 0 ? entry.Name : prefix + "/" + entry.Name;

                if (entry.Type == EntryType.Stream)
                {
                    if (!_entriesByPath.TryAdd(path, entry))
                        Warn("PL1110", $"Duplicate directory entry '{path}'; the first was kept.");
                    collectStreamNames?.Add(entry.Name);
                }
                else
                {
                    // Storages are not parts themselves, but their children are.
                    Walk(entry.ChildId, path, depth + 1, collectStreamNames: null);
                }

                if (entry.LeftSiblingId != NoStream) pending.Push(entry.LeftSiblingId);
                if (entry.RightSiblingId != NoStream) pending.Push(entry.RightSiblingId);
            }
        }
    }

    // ------------------------------------------------------------------- stream reading

    internal byte[] ReadEntry(DirectoryEntry entry)
    {
        if (entry.StreamLength == 0) return [];
        if (entry.StreamLength > int.MaxValue)
            throw new MalformedDocumentException($"Stream '{entry.Name}' claims a length above 2 GiB.");

        return ReadChain(entry.StartSector, (int)entry.StreamLength,
                         useMiniFat: entry.StreamLength < _miniStreamCutoff);
    }

    private byte[] ReadChain(uint startSector, int expectedLength, bool useMiniFat)
    {
        int unitSize = useMiniFat ? _miniSectorSize : _sectorSize;
        int capacity = expectedLength >= 0 ? expectedLength : Math.Min(unitSize * 64, 1 << 20);

        using MemoryStream output = new(capacity);
        byte[] buffer = new byte[unitSize];
        uint sector = startSector;
        int guard = 0;
        long remaining = expectedLength >= 0 ? expectedLength : long.MaxValue;

        while (sector <= MaxRegularSector && remaining > 0 && guard++ < MaxChainLength)
        {
            bool ok = useMiniFat ? TryReadMiniSector(sector, buffer) : TryReadSector(sector, buffer);
            if (!ok)
            {
                Warn("PL1111", $"Sector {sector} lies outside the file; the stream was truncated there.");
                break;
            }
            int take = (int)Math.Min(remaining, unitSize);
            output.Write(buffer, 0, take);
            remaining -= take;
            sector = useMiniFat ? NextInMiniChain(sector) : NextInChain(sector);
        }

        if (guard >= MaxChainLength)
            Warn("PL1112", "Sector chain exceeded the safety limit; it is probably cyclic. Truncated.");

        byte[] result = output.ToArray();
        if (expectedLength >= 0 && result.Length < expectedLength)
        {
            // Declaring more than the file holds is common in truncated documents. Report it
            // rather than either throwing or silently zero-padding.
            Warn("PL1113",
                 $"Stream is {result.Length} bytes but declared {expectedLength}; using what was readable.");
        }
        return result;
    }

    private uint NextInChain(uint sector)
        => sector < (uint)_fat.Length ? _fat[sector] : EndOfChain;

    private uint NextInMiniChain(uint sector)
        => sector < (uint)_miniFat.Length ? _miniFat[sector] : EndOfChain;

    private bool TryReadSector(uint sector, Span<byte> destination)
    {
        // Sector 0 begins immediately after the 512-byte header, whatever the sector size.
        long offset = HeaderSize + (long)sector * _sectorSize;
        if (offset < 0 || offset + destination.Length > _stream.Length) return false;
        _stream.Position = offset;
        return ReadFully(_stream, destination);
    }

    private bool TryReadMiniSector(uint sector, Span<byte> destination)
    {
        long offset = (long)sector * _miniSectorSize;
        if (offset < 0 || offset + destination.Length > _miniStream.Length) return false;
        _miniStream.AsSpan((int)offset, destination.Length).CopyTo(destination);
        return true;
    }

    /// <summary>
    /// Reads exactly <paramref name="destination"/>.Length bytes, or returns false.
    /// </summary>
    /// <remarks>
    /// A single Stream.Read can legally return fewer bytes than asked for, so every read
    /// here has to loop. Getting this wrong yields sporadic corruption that only shows up
    /// on some stream implementations.
    /// </remarks>
    private static bool ReadFully(Stream source, Span<byte> destination)
    {
        int total = 0;
        while (total < destination.Length)
        {
            int read = source.Read(destination[total..]);
            if (read == 0) return false;
            total += read;
        }
        return true;
    }

    private void Warn(string code, string message)
        => _diagnostics.Add(new Diagnostic(DiagnosticSeverity.Warning, code, message));

    private static string Normalise(string name) => name.Replace('\\', '/').TrimStart('/');

    /// <inheritdoc/>
    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        if (_ownsStream) _stream.Dispose();
    }

    // ---------------------------------------------------------------------- inner types

    internal enum EntryType : byte
    {
        Empty = 0,
        Storage = 1,
        Stream = 2,
        LockBytes = 3,
        Property = 4,
        Root = 5,
    }

    /// <summary>One 128-byte directory entry.</summary>
    internal readonly struct DirectoryEntry
    {
        public required string Name { get; init; }
        public required EntryType Type { get; init; }
        public required int LeftSiblingId { get; init; }
        public required int RightSiblingId { get; init; }
        public required int ChildId { get; init; }
        public required uint StartSector { get; init; }
        public required ulong StreamLength { get; init; }

        public static DirectoryEntry Parse(ReadOnlySpan<byte> raw)
        {
            // The name is UTF-16LE; the length field counts bytes and includes the
            // terminator. Clamp it: a corrupt length would otherwise read out of the entry.
            ushort nameBytes = BinaryPrimitives.ReadUInt16LittleEndian(raw[64..]);
            if (nameBytes > 64) nameBytes = 64;
            int chars = Math.Max(0, nameBytes / 2 - 1);
            string name = chars == 0 ? string.Empty : Encoding.Unicode.GetString(raw[..(chars * 2)]);

            return new DirectoryEntry
            {
                Name = name,
                Type = (EntryType)raw[66],
                LeftSiblingId = ReadId(raw[68..]),
                RightSiblingId = ReadId(raw[72..]),
                ChildId = ReadId(raw[76..]),
                StartSector = BinaryPrimitives.ReadUInt32LittleEndian(raw[116..]),
                StreamLength = BinaryPrimitives.ReadUInt64LittleEndian(raw[120..]),
            };
        }

        private static int ReadId(ReadOnlySpan<byte> raw)
        {
            uint value = BinaryPrimitives.ReadUInt32LittleEndian(raw);
            return value > MaxRegularSector ? NoStream : (int)value;
        }
    }

    private sealed class CompoundFilePart(CompoundFile owner, string name, DirectoryEntry entry)
        : IPackagePart
    {
        public string Name { get; } = name;

        /// <summary>Always null: OLE2 records no media type for its streams.</summary>
        public string? MediaType => null;

        public long Length => (long)Math.Min(entry.StreamLength, long.MaxValue);

        public Stream Open() => new MemoryStream(owner.ReadEntry(entry), writable: false);
    }
}
