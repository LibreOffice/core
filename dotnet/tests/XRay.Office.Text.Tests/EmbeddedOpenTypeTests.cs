using System.Buffers.Binary;
using System.Text;
using XRay.Office.Core.Graphics;
using XRay.Office.Text.Fonts;
using Shouldly;
using Xunit;

namespace XRay.Office.Text.Tests;

/// <summary>
/// The container a deck carries its fonts in.
/// </summary>
/// <remarks>
/// <para>
/// These tests exist because the mechanism was described wrongly for at least one round, and the
/// wrong description is the plausible one: PowerPoint's <c>ppt/fonts/*.fntdata</c> is widely said
/// to be a TrueType file whose first 32 bytes are XORed with a key derived from a GUID. That is
/// WordprocessingML's obfuscation — <c>w:embedRegular/@w:fontKey</c> — and applying it to a deck
/// produces 32 bytes of noise in front of an EOT header.
/// </para>
/// <para>
/// So <see cref="BytesThatAreNotAContainerAreDeclined"/> is the load-bearing negative: a plain
/// sfnt must not open as one of these, because a reader that accepted it would be the "deobfuscate
/// a TTF" hypothesis wearing this type's name.
/// </para>
/// </remarks>
public class EmbeddedOpenTypeTests
{
    /// <summary>
    /// The bytes of a real installed face, used as the payload every container here wraps.
    /// </summary>
    /// <remarks>
    /// A real font rather than a stub, because half of what these tests assert is that the payload
    /// comes back out in a state <see cref="OpenTypeFace"/> can read — an assertion a placeholder
    /// would pass by not being checked.
    /// </remarks>
    private static byte[] Payload()
    {
        SystemFontIndex index = SystemFontIndex.Build();
        InstalledFace face = index.Best("DejaVu Sans", 400, italic: false)
            ?? index.Best("Liberation Sans", 400, italic: false)
            ?? index.Faces.First();

        return File.ReadAllBytes(face.Path);
    }

    [Fact]
    public void AnUncompressedContainerYieldsTheFontInsideIt()
    {
        byte[] font = Payload();
        byte[] container = Container(font, flags: 0, family: "Alegreya Sans", style: "Medium");

        EmbeddedOpenTypeFont embedded = EmbeddedOpenTypeFont.Read(container).ShouldNotBeNull();

        embedded.IsCompressed.ShouldBeFalse();
        embedded.HasFontData.ShouldBeTrue();
        embedded.FontData.ToArray().ShouldBe(font);

        // And it is a font, not merely the right number of bytes.
        OpenTypeFace.Read(embedded.FontData.ToArray()).ShouldNotBeNull();
    }

    [Fact]
    public void TheHeaderStatesWhatTheFaceIsWithoutOpeningIt()
    {
        // The declared name is not the face's own — this deck declares `Alegreya Sans Regular Bold`
        // for a face whose family is `Alegreya Sans` — so the wrapper's names are a fact of their
        // own and the walk that finds them has to be exact rather than approximately right.
        byte[] container = Container(
            Payload(), flags: 0, family: "Alegreya Sans Regular", style: "Medium",
            weight: 500, italic: true, fsType: 8);

        EmbeddedOpenTypeFont embedded = EmbeddedOpenTypeFont.Read(container).ShouldNotBeNull();

        embedded.FamilyName.ShouldBe("Alegreya Sans Regular");
        embedded.StyleName.ShouldBe("Medium");
        embedded.Weight.ShouldBe(500);
        embedded.IsItalic.ShouldBeTrue();
        embedded.EmbeddingRights.ShouldBe(8);
        embedded.Version.ShouldBe(0x0002_0002u);
    }

    [Fact]
    public void ACompressedContainerIsReportedRatherThanDecoded()
    {
        // MicroType Express, which LibreOffice hands to libeot and this reader does not implement.
        // Reported rather than refused: "a face we cannot decompress" and "not a font part at all"
        // want different answers from a caller, and 18 of the slides track's 28 parts are this one.
        byte[] container = Container(
            Payload(), flags: EmbeddedOpenTypeFont.CompressedFlag | EmbeddedOpenTypeFont.SubsetFlag);

        EmbeddedOpenTypeFont embedded = EmbeddedOpenTypeFont.Read(container).ShouldNotBeNull();

        embedded.IsCompressed.ShouldBeTrue();
        embedded.IsSubset.ShouldBeTrue();
        embedded.HasFontData.ShouldBeFalse();
    }

    [Fact]
    public void AnXorMaskedContainerIsUnmasked()
    {
        byte[] font = Payload();
        byte[] masked = (byte[])font.Clone();
        for (int i = 0; i < masked.Length; i++) masked[i] ^= EmbeddedOpenTypeFont.XorKey;

        byte[] container = Container(masked, flags: EmbeddedOpenTypeFont.XorEncryptedFlag);

        EmbeddedOpenTypeFont embedded = EmbeddedOpenTypeFont.Read(container).ShouldNotBeNull();

        embedded.FontData.ToArray().ShouldBe(font);
    }

    [Fact]
    public void BytesThatAreNotAContainerAreDeclined()
    {
        // The plain face. If this opened, the reader would be finding a container in anything.
        byte[] font = Payload();

        EmbeddedOpenTypeFont.Read(font).ShouldBeNull();
        EmbeddedOpenTypeFont.Looks(font).ShouldBeFalse();

        // And the same face with the Word obfuscation applied over it, which is the shape the
        // mechanism is usually mis-stated as having. It is not this format either.
        byte[] obfuscated = (byte[])font.Clone();
        for (int i = 0; i < 32; i++) obfuscated[i] ^= (byte)(0xA5 + i);

        EmbeddedOpenTypeFont.Read(obfuscated).ShouldBeNull();
    }

    [Fact]
    public void ATruncatedContainerIsDeclinedRatherThanRead()
    {
        byte[] container = Container(Payload(), flags: 0);

        EmbeddedOpenTypeFont.Read(container.AsMemory(0, container.Length / 2)).ShouldBeNull();
        EmbeddedOpenTypeFont.Read(Array.Empty<byte>()).ShouldBeNull();
    }

    [Fact]
    public void AStoredFaceIsAddressableAsAPathAndStoredOnlyOnce()
    {
        // The point of the store: resolution ends at a FaceKey and every backend reads that key as
        // a path. A key nothing can open costs the document its embedded fonts silently.
        byte[] font = Payload();

        string path = EmbeddedFontStore.Store(font).ShouldNotBeNull();

        File.Exists(path).ShouldBeTrue();
        OpenTypeFace.ReadFile(path).ShouldNotBeNull();
        File.ReadAllBytes(path).ShouldBe(font);

        // Content-addressed, so a deck asking for the same face on every slide writes it once.
        EmbeddedFontStore.Store(font).ShouldBe(path);
        EmbeddedFontStore.Store(ReadOnlySpan<byte>.Empty).ShouldBeNull();
    }

    [Fact]
    public void AResolverLoadsTheFaceAnEmbeddedKeyNames()
    {
        // The whole path, end to end: a request carrying an embedded key resolves to that key
        // without consulting the installed faces, and the key loads.
        byte[] font = Payload();
        string path = EmbeddedFontStore.Store(font).ShouldNotBeNull();

        SystemFontResolver resolver = new(SystemFontIndex.Build());
        FontReference reference = resolver.Resolve(
            new FontRequest("A Family Nothing Has Installed", EmbeddedFaceKey: path));

        reference.FaceKey.ShouldBe(path);
        resolver.Substitutions.ShouldBeEmpty();
        resolver.LoadFace(reference).UnitsPerEm.ShouldBeGreaterThan(0);
    }

    /// <summary>
    /// Builds a version 0x00020002 container around a face, which is the version every part in the
    /// corpus uses.
    /// </summary>
    /// <remarks>
    /// Written out field by field rather than copied from a fixture so that the layout this reader
    /// assumes is stated once, in a form a reviewer can check against the specification without
    /// opening a binary.
    /// </remarks>
    private static byte[] Container(
        byte[] font,
        uint flags,
        string family = "Test Family",
        string style = "Regular",
        int weight = 400,
        bool italic = false,
        int fsType = 0)
    {
        MemoryStream buffer = new();
        BinaryWriter writer = new(buffer);

        writer.Write(0u);                        // EOTSize, patched below
        writer.Write((uint)font.Length);         // FontDataSize
        writer.Write(0x0002_0002u);              // Version
        writer.Write(flags);
        writer.Write(new byte[10]);              // FontPANOSE
        writer.Write((byte)0);                   // Charset
        writer.Write((byte)(italic ? 1 : 0));    // Italic
        writer.Write((uint)weight);
        writer.Write((ushort)fsType);
        writer.Write(EmbeddedOpenTypeFont.Magic);
        writer.Write(new byte[16]);              // UnicodeRange1..4
        writer.Write(new byte[8]);               // CodePageRange1..2
        writer.Write(0u);                        // CheckSumAdjustment
        writer.Write(new byte[16]);              // Reserved1..4

        Block(writer, family);
        Block(writer, style);
        Block(writer, "1.0");                    // VersionName
        Block(writer, family + " " + style);     // FullName
        Block(writer, string.Empty);             // RootString, from 0x00020001
        writer.Write(0u);                        // RootStringCheckSum, from 0x00020002
        writer.Write(0u);                        // EUDCCodePage
        Block(writer, string.Empty);             // Signature
        writer.Write(0u);                        // EUDCFlags
        writer.Write(0u);                        // EUDCFontSize

        writer.Write(font);
        writer.Flush();

        byte[] bytes = buffer.ToArray();
        BinaryPrimitives.WriteUInt32LittleEndian(bytes, (uint)bytes.Length);
        return bytes;

        static void Block(BinaryWriter writer, string value)
        {
            byte[] utf16 = Encoding.Unicode.GetBytes(value);
            writer.Write((ushort)0);             // Padding
            writer.Write((ushort)utf16.Length);
            writer.Write(utf16);
        }
    }
}
