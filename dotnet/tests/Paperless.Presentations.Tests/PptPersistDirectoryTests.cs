using Paperless.Core.Diagnostics;
using Paperless.MsBinary.Records;
using Paperless.Presentations.MsBinary;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Tests the walk that turns a persist id into a stream offset.
/// </summary>
/// <remarks>
/// Hand-built streams rather than corpus files, because the behaviour that matters here — an
/// object superseded by a later edit, and a chain that points at itself — is exactly what a
/// file written once by LibreOffice cannot exercise. Every deck in the corpus has a single
/// edit session, so a directory reader that ignored the chain entirely would pass on all of
/// them and then read a stale slide out of the first real PowerPoint file it met.
/// </remarks>
public class PptPersistDirectoryTests
{
    [Fact]
    public void TheNewestEditWinsWhenTwoSessionsBothStateAnObject()
    {
        StreamBuilder builder = new();

        // The older session put object 2 at 16 and object 3 at 24; the newer moved object 2.
        int oldBlock = builder.PersistBlock(first: 2, [16, 24]);
        int oldEdit = builder.UserEdit(previousEdit: 0, persistDirectory: oldBlock, documentReference: 1);
        int newBlock = builder.PersistBlock(first: 2, [40]);
        int newEdit = builder.UserEdit(previousEdit: (uint)oldEdit, persistDirectory: newBlock, documentReference: 1);

        PptPersistDirectory directory = Read(builder, (uint)newEdit);

        directory.Resolve(2).ShouldBe(40);

        // The older session is still the only statement about object 3, so it must survive:
        // the newer block replaces entries, not the directory.
        directory.Resolve(3).ShouldBe(24);
    }

    [Fact]
    public void AChainThatPointsForwardsOrAtItselfStopsRatherThanLooping()
    {
        StreamBuilder builder = new();
        int block = builder.PersistBlock(first: 1, [24]);
        int edit = builder.UserEdit(previousEdit: 0, persistDirectory: block, documentReference: 1);

        // Rewrite the edit atom to name itself as its own predecessor, which is what a
        // corrupted or maliciously crafted file does to make a naive walk spin forever.
        builder.Patch(edit + 8 + 8, (uint)edit);

        PptPersistDirectory directory = Read(builder, (uint)edit);
        directory.Resolve(1).ShouldBe(24);
    }

    [Fact]
    public void TheLastEditAtomIsFoundWhenTheCurrentUserStreamIsUseless()
    {
        StreamBuilder builder = new();
        int block = builder.PersistBlock(first: 1, [8]);
        builder.UserEdit(previousEdit: 0, persistDirectory: block, documentReference: 1);

        // A 'Current User' stream naming an offset with no edit atom at it is common in files
        // that have been through a repair tool; scanning for the last one recovers.
        PptPersistDirectory directory = Read(builder, currentUserEdit: 7, out List<Diagnostic> diagnostics);

        directory.Resolve(1).ShouldBe(8);
        diagnostics.ShouldContain(d => d.Code == "PL2400");
    }

    [Fact]
    public void AnEntryPointingPastTheEndOfTheStreamIsRejectedRatherThanStored()
    {
        StreamBuilder builder = new();
        int block = builder.PersistBlock(first: 1, [16, 0x7FFFFFF]);
        int edit = builder.UserEdit(previousEdit: 0, persistDirectory: block, documentReference: 1);

        PptPersistDirectory directory = Read(builder, (uint)edit, out List<Diagnostic> diagnostics);

        directory.Resolve(1).ShouldBe(16);
        directory.Resolve(2).ShouldBeNull();
        diagnostics.ShouldContain(d => d.Code == "PL2402");
    }

    [Fact]
    public void AStreamWithNoEditAtomAtAllIsReportedRatherThanThrowing()
    {
        StreamBuilder builder = new();
        builder.PersistBlock(first: 1, [16]);

        PptPersistDirectory directory = Read(builder, currentUserEdit: 0, out List<Diagnostic> diagnostics);

        directory.Count.ShouldBe(0);
        diagnostics.ShouldContain(d => d.Code == "PL2401");
    }

    private static PptPersistDirectory Read(StreamBuilder builder, uint currentUserEdit)
        => Read(builder, currentUserEdit, out _);

    private static PptPersistDirectory Read(
        StreamBuilder builder, uint currentUserEdit, out List<Diagnostic> diagnostics)
    {
        diagnostics = [];
        return PptPersistDirectory.Read(new DffRecordBuffer(builder.Build()), currentUserEdit, diagnostics);
    }

    /// <summary>Assembles a document stream one record at a time.</summary>
    private sealed class StreamBuilder
    {
        private readonly List<byte> _bytes = [];

        /// <summary>
        /// Starts the stream with a filler record, because offset zero means "absent" in this
        /// format: an edit atom whose persist-directory field is zero states that it has none.
        /// A real file has its document container there, so nothing else ever begins at zero.
        /// </summary>
        public StreamBuilder() => Record(PptRecordTypes.Document, []);

        /// <summary>Appends a persist block mapping consecutive ids to offsets.</summary>
        /// <returns>The block's offset.</returns>
        public int PersistBlock(uint first, uint[] offsets)
        {
            List<byte> payload = [];
            Append(payload, (first & 0x000FFFFF) | ((uint)offsets.Length << 20));
            foreach (uint offset in offsets) Append(payload, offset);
            return Record(PptRecordTypes.PersistPtrIncrementalBlock, payload);
        }

        /// <summary>Appends an edit atom.</summary>
        /// <returns>The atom's offset.</returns>
        public int UserEdit(uint previousEdit, int persistDirectory, uint documentReference)
        {
            List<byte> payload = [];
            Append(payload, 0u);                    // last slide id
            Append(payload, 0u);                    // writer version
            Append(payload, previousEdit);
            Append(payload, (uint)persistDirectory);
            Append(payload, documentReference);
            Append(payload, 8u);                    // highest persist id written
            payload.AddRange([1, 0]);               // last view type
            return Record(PptRecordTypes.UserEditAtom, payload);
        }

        /// <summary>Overwrites a 32-bit value already written.</summary>
        public void Patch(int offset, uint value)
        {
            for (int i = 0; i < 4; i++) _bytes[offset + i] = (byte)(value >> (i * 8));
        }

        public byte[] Build() => [.. _bytes];

        private int Record(ushort type, List<byte> payload)
        {
            int position = _bytes.Count;
            _bytes.Add(0);
            _bytes.Add(0);
            _bytes.Add((byte)type);
            _bytes.Add((byte)(type >> 8));
            Append(_bytes, (uint)payload.Count);
            _bytes.AddRange(payload);
            return position;
        }

        private static void Append(List<byte> into, uint value)
        {
            into.Add((byte)value);
            into.Add((byte)(value >> 8));
            into.Add((byte)(value >> 16));
            into.Add((byte)(value >> 24));
        }
    }
}
