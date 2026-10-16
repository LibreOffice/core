using XRay.Office.MsBinary.Records;
using XRay.Office.Presentations.Layout;
using XRay.Office.Presentations.MsBinary;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// The pitch a deck declares for a typeface, and why reading it decides which face is drawn.
/// </summary>
/// <remarks>
/// <para>
/// A family fontconfig files under no generic at all takes its sans-serif default, which is what
/// LibreOffice draws it in — and a declared <em>fixed</em> pitch overrides that, because a document
/// relying on its columns lining up is making the stronger statement.
/// <c>Lucida Console</c> is both at once: <c>VCL.xcu</c> calls it monospaced, fontconfig files it
/// nowhere, and which of the two the reference follows turns entirely on whether the deck says so.
/// </para>
/// <para>
/// <strong>Measured on the running 26.2.4.2, twice.</strong> On the corpus,
/// <c>airbus-powerpoint-presentation-2019-20…pptx</c> and <c>introduction_to_bea_tuxedo.ppt</c>
/// both embed DejaVu Sans Mono in the reference PDF and both declare Lucida Console fixed. In
/// isolation, re-zipping the first deck with the single attribute <c>pitchFamily="49"</c> removed
/// and nothing else changed makes LibreOffice draw DejaVu <em>Sans</em> instead — the probe is
/// <c>dotnet/probes/font-class-01/pitchprobe.sh</c>.
/// </para>
/// <para>
/// <strong>No committed fixture can carry the PPTX half.</strong> Every <c>.pptx</c> in
/// <c>tests/corpus/features</c> is a LibreOffice export and its exporter writes
/// <c>pitchFamily="0"</c> on every typeface it emits — scanned, 16 of 16 — so a fixture-based test
/// would pin the no-op and nothing else. The corpus rules forbid committing a deck from the web.
/// The binary half <em>can</em> be built by hand and is, below; the shared decoding is pinned
/// directly; and the PPTX plumbing rests on the two measurements above.
/// </para>
/// </remarks>
public class SlideFontPitchTests
{
    [Theory]
    [InlineData(0x31, FontPitch.Fixed)]        // Lucida Console: fixed pitch, modern family — the
                                               // 49 a PPTX writes decimal is this same byte
    [InlineData(0x12, FontPitch.Variable)]     // Times New Roman: variable pitch, roman family
    [InlineData(0x22, FontPitch.Variable)]     // Arial as PowerPoint usually writes it
    [InlineData(0x00, FontPitch.Unknown)]      // nothing stated, which is most of the corpus
    [InlineData(0x30, FontPitch.Unknown)]      // a family with no pitch beside it
    public void ThePitchIsTheLowTwoBitsOfTheLogFontByte(int declared, FontPitch expected)
    {
        // One decoder for both formats, because it is one byte in both: PPTX writes it decimal as
        // `pitchFamily`, PPT as the last byte of a FontEntityAtom. The family lives in the high
        // four bits of the same byte and is deliberately not read — see SlideFonts.DeclaredPitches.
        SlideFonts.PitchIn(declared).ShouldBe(expected);
    }

    [Fact]
    public void ABinaryDecksFontCollectionStatesThePitchPerFace()
    {
        // The offset is the thing this pins: the sixty-four-byte name is followed by lfCharSet, a
        // flags byte and fontType, and lfPitchAndFamily is the last of the sixty-eight. Read one
        // byte early and Lucida Console comes back with fontType's 0x06 — variable pitch — which is
        // wrong in a way no other test would catch.
        PptFontTable fonts = Collection(
            ("Times New Roman", (byte)0x12), ("Arial", (byte)0x00), ("Lucida Console", (byte)0x31));

        fonts.PitchOf("Lucida Console").ShouldBe(FontPitch.Fixed);
        fonts.PitchOf("Times New Roman").ShouldBe(FontPitch.Variable);
        fonts.PitchOf("Arial").ShouldBe(FontPitch.Unknown);
        fonts.PitchOf("A Face The Deck Never Named").ShouldBe(FontPitch.Unknown);
        fonts.PitchOf(null).ShouldBe(FontPitch.Unknown);

        // Those three values are not invented: they are what `introduction_to_bea_tuxedo.ppt`'s own
        // collection holds, read out of the file.
        fonts.Count.ShouldBe(3);
    }

    [Fact]
    public void ADeckWithNoEnvironmentDeclaresNoPitch()
        => PptFontTable.Empty.PitchOf("Lucida Console").ShouldBe(FontPitch.Unknown);

    /// <summary>
    /// Builds an <c>Environment</c> holding a <c>FontCollection</c> of faces with stated pitches.
    /// </summary>
    private static PptFontTable Collection(params (string Name, byte PitchAndFamily)[] faces)
    {
        List<byte> entities = [];
        foreach ((string name, byte pitchAndFamily) in faces)
        {
            byte[] entity = new byte[8 + 68];
            BitConverter.GetBytes((ushort)0x0000).CopyTo(entity, 0);
            BitConverter.GetBytes(PptRecordTypes.FontEntityAtom).CopyTo(entity, 2);
            BitConverter.GetBytes(68).CopyTo(entity, 4);
            System.Text.Encoding.Unicode.GetBytes(name).CopyTo(entity, 8);
            entity[8 + 67] = pitchAndFamily;
            entities.AddRange(entity);
        }

        byte[] collection = new byte[8 + entities.Count];
        BitConverter.GetBytes((ushort)0x000F).CopyTo(collection, 0);
        BitConverter.GetBytes(PptRecordTypes.FontCollection).CopyTo(collection, 2);
        BitConverter.GetBytes(entities.Count).CopyTo(collection, 4);
        entities.CopyTo(collection, 8);

        byte[] environment = new byte[8 + collection.Length];
        BitConverter.GetBytes((ushort)0x000F).CopyTo(environment, 0);
        BitConverter.GetBytes(PptRecordTypes.Environment).CopyTo(environment, 2);
        BitConverter.GetBytes(collection.Length).CopyTo(environment, 4);
        collection.CopyTo(environment, 8);

        DffRecordBuffer stream = new(environment);
        stream.TryReadHeader(0, out DffRecordHeader header).ShouldBeTrue();
        return PptFontTable.Read(stream, header);
    }
}
