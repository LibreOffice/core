using Paperless.MsBinary.Records;
using Paperless.Presentations.MsBinary;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Which of a page's several <c>ColorSchemeAtom</c> records is the one it draws in.
/// </summary>
/// <remarks>
/// A slide master carries the scheme-picker palette as well as its own scheme, and reading the
/// first of them gives a deck colours it never named — on the corpus deck that found this, a
/// white-paper palette entry in place of a navy page, so every placeholder filling with "text and
/// lines" came out black on black.
/// </remarks>
public class PptPageSchemeTests
{
    [Fact]
    public void APageTakesTheSchemeMarkedAsItsOwnRatherThanTheFirstOneListed()
    {
        // A slide master as PowerPoint writes one: the scheme-picker palette first, the page's
        // own scheme last. The two disagree, which is the case that separates them.
        byte[] page = Page(
            (6, 0xFF, 0x00),
            (6, 0xEE, 0x11),
            (1, 0x00, 0xFF));

        DffRecordBuffer buffer = new(page);
        buffer.TryReadHeader(0, out DffRecordHeader master).ShouldBeTrue();

        PptColourScheme? scheme = PptColourScheme.OfPage(buffer, master);

        scheme.ShouldNotBeNull();
        scheme[PptColourScheme.BackgroundSlot].R.ShouldBe((byte)0x00);
        scheme[PptColourScheme.TextSlot].R.ShouldBe((byte)0xFF);
    }

    [Fact]
    public void APageStatingOneSchemeOfNoStatedKindStillUsesIt()
    {
        // Leniency: a writer that omits the instance still gets its scheme read, rather than the
        // page falling back to the all-purpose default and drawing in colours it never named.
        byte[] page = Page((6, 0x40, 0x80));

        DffRecordBuffer buffer = new(page);
        buffer.TryReadHeader(0, out DffRecordHeader master).ShouldBeTrue();

        PptColourScheme? scheme = PptColourScheme.OfPage(buffer, master);

        scheme.ShouldNotBeNull();
        scheme[PptColourScheme.BackgroundSlot].R.ShouldBe((byte)0x40);
    }

    /// <summary>A <c>MainMaster</c> container holding the given colour scheme atoms in order.</summary>
    private static byte[] Page(params (ushort Instance, byte Background, byte Text)[] schemes)
    {
        const ushort MainMaster = 1016;
        const ushort ColorSchemeAtom = 2032;

        List<byte> body = [];
        foreach ((ushort instance, byte background, byte text) in schemes)
        {
            byte[] slots = new byte[32];
            slots[0] = background;
            slots[4] = text;
            Record(body, ColorSchemeAtom, container: false, instance, slots);
        }

        List<byte> page = [];
        Record(page, MainMaster, container: true, 0, [.. body]);
        return [.. page];
    }

    private static void Record(
        List<byte> into, ushort type, bool container, ushort instance, byte[] payload)
    {
        ushort versionAndInstance = (ushort)((instance << 4) | (container ? 0x0F : 0x00));
        into.Add((byte)versionAndInstance);
        into.Add((byte)(versionAndInstance >> 8));
        into.Add((byte)type);
        into.Add((byte)(type >> 8));
        for (int i = 0; i < 4; i++) into.Add((byte)((uint)payload.Length >> (i * 8)));
        into.AddRange(payload);
    }
}
