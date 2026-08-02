using Paperless.MsBinary.Escher;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Where a blip store keeps its pictures, and the two ways a <c>foDelay</c> of zero can be meant.
/// </summary>
/// <remarks>
/// Tested from the presentations suite because PowerPoint is the host that puts a whole deck's
/// pictures in a stream of its own — the <c>Pictures</c> stream, which begins with the first of
/// them at offset zero. That is the case a reader treating zero as "nowhere" loses entirely, and
/// on a deck with one picture it loses the only one.
/// </remarks>
public class EscherBlipStoreTests
{
    /// <summary>A one-pixel PNG, small enough to inline and real enough to sniff.</summary>
    private static readonly byte[] Png =
    [
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
        0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
        0x08, 0x02, 0x00, 0x00, 0x00, 0x90, 0x77, 0x53, 0xDE,
    ];

    [Fact]
    public void APictureAtOffsetZeroOfTheDelayStreamIsFound()
    {
        // PowerPoint's layout: the entry carries its thirty-six bytes and nothing else, and the
        // picture is the first thing in the Pictures stream, so foDelay is 0.
        byte[] officeArt = Store(size: (uint)Png.Length + 25, delay: 0, inline: false);
        byte[] pictures = BlipRecord();

        Dictionary<int, EscherBlip> blips = EscherBlips.Read(officeArt, pictures, []);

        blips.ShouldContainKey(1);
        blips[1].RecordType.ShouldBe((ushort)0xF01E);
        blips[1].Bytes.ToArray().ShouldBe(Png);
    }

    [Fact]
    public void APictureInsideTheEntryIsStillPreferredToTheDelayStream()
    {
        // The other meaning of zero: "the image is in the FBSE", which LibreOffice infers when
        // the blip is smaller than the entry (msdffimp.cxx:6084). Reading the entry's own record
        // first settles it without having to compare the two sizes at all — and this is what
        // stops the change above from turning every inline picture into whatever happens to sit
        // at offset zero of some other stream.
        byte[] officeArt = Store(size: (uint)Png.Length + 25, delay: 0, inline: true);

        byte[] decoy = BlipRecord(0x5A);
        Dictionary<int, EscherBlip> blips = EscherBlips.Read(officeArt, decoy, []);

        blips.ShouldContainKey(1);
        blips[1].Bytes.ToArray().ShouldBe(Png);
    }

    [Fact]
    public void AnEntryWhoseDelayStreamHoldsNoBlipYieldsNoPicture()
    {
        // Zero is only an offset when something is there. An empty delay stream must not become
        // a picture, and must not throw either.
        byte[] officeArt = Store(size: 64, delay: 0, inline: false);

        EscherBlips.Read(officeArt, [], []).ShouldBeEmpty();
    }

    /// <summary>An <c>OfficeArtMetafileHeader</c>-free raster blip record holding the PNG.</summary>
    private static byte[] BlipRecord(byte firstByte = 0x89)
    {
        // Sixteen bytes of checksum and one tag byte precede the picture on every raster blip;
        // instance 0x6E0 is even, so there is one checksum rather than two.
        List<byte> payload = [];
        payload.AddRange(new byte[16]);
        payload.Add(0xFF);
        payload.AddRange(Png);
        payload[17] = firstByte;

        List<byte> record = [];
        Record(record, 0xF01E, container: false, instance: 0x6E0, [.. payload]);
        return [.. record];
    }

    /// <summary>A drawing group whose blip store holds one entry.</summary>
    private static byte[] Store(uint size, uint delay, bool inline)
    {
        List<byte> entry = [];
        entry.Add(0x06);                       // btWin32: PNG
        entry.Add(0x06);                       // btMacOS
        entry.AddRange(new byte[16]);          // rgbUid
        entry.Add(0x00);                       // tag
        entry.Add(0x00);
        Append(entry, size);                   // size
        Append(entry, 1);                      // cRef
        Append(entry, delay);                  // foDelay
        entry.Add(0x00);                       // usage
        entry.Add(0x00);                       // cbName
        entry.Add(0x00);
        entry.Add(0x00);

        entry.Count.ShouldBe(36);
        if (inline) entry.AddRange(BlipRecord());

        List<byte> store = [];
        Record(store, EscherRecordTypes.BlipStoreEntry, container: false, instance: 0x06, [.. entry]);

        List<byte> container = [];
        Record(container, EscherRecordTypes.BlipStoreContainer, container: true, 1, [.. store]);

        List<byte> group = [];
        Record(group, EscherRecordTypes.DrawingGroupContainer, container: true, 0, [.. container]);
        return [.. group];
    }

    private static void Record(
        List<byte> into, ushort type, bool container, ushort instance, byte[] payload)
    {
        ushort versionAndInstance = (ushort)((instance << 4) | (container ? 0x0F : 0x02));
        into.Add((byte)versionAndInstance);
        into.Add((byte)(versionAndInstance >> 8));
        into.Add((byte)type);
        into.Add((byte)(type >> 8));
        Append(into, (uint)payload.Length);
        into.AddRange(payload);
    }

    private static void Append(List<byte> into, uint value)
    {
        for (int i = 0; i < 4; i++) into.Add((byte)(value >> (i * 8)));
    }
}
