using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Ww8;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Tests the DOC font table: the families a <c>sprmCRgFtc0</c> indexes, and the shape each entry
/// declares for its family.
/// </summary>
/// <remarks>
/// <para>
/// The shape is not decoration. A family that is neither installed nor substitutable falls back by
/// shape, and on a Linux box that is the common case rather than the exception — so an <c>FFN</c>
/// whose <c>ff</c> field says roman is the difference between a document rendering in DejaVu Serif,
/// as LibreOffice renders it, and in DejaVu Sans, which moves every line break in it.
/// </para>
/// <para>
/// It also reaches families the substitution chain <em>would</em> have answered, which is what makes
/// this worth reading rather than merely worth having. <c>words/batch-004/doc/1447.doc</c> sets its
/// body in a family called <c>Times</c> with <c>ff = FF_ROMAN</c>. `Times` has a chain entry that is
/// installed — <c>liberationserif</c> — so before the declaration was read at the right point we
/// answered Liberation Serif, while LibreOffice 26.2.4.2 draws the whole document in DejaVu Serif.
/// The line advances name the two faces outright, 13.80 pt against 14.00 at 12 pt, and the reference
/// fits about 11% fewer characters to the line: its first paragraph takes nine lines where ours took
/// seven. See <c>dotnet/probes/words-pages-01/results.md</c>.
/// </para>
/// </remarks>
public class Ww8FontTableTests
{
    /// <summary>Builds an <c>SttbfFfn</c> holding one entry per (flags, name) pair given.</summary>
    /// <remarks>
    /// The header is four bytes, not two: a count that duplicates the FIB's and a two-byte
    /// extra-data length that is always zero for this table. Each entry is a one-byte payload length
    /// followed by the payload, whose name sits after the flags, weight, character set, alternate
    /// name index, ten PANOSE bytes and twenty-four of font signature.
    /// </remarks>
    private static byte[] Table(params (byte Flags, string Name)[] fonts)
    {
        const int nameOffset = 1 + 2 + 1 + 1 + 10 + 24;

        List<byte> bytes = [(byte)fonts.Length, 0, 0, 0];

        foreach ((byte flags, string name) in fonts)
        {
            byte[] payload = new byte[nameOffset + ((name.Length + 1) * 2)];
            payload[0] = flags;

            for (int at = 0; at < name.Length; at++)
            {
                payload[nameOffset + (at * 2)] = (byte)(name[at] & 0xFF);
                payload[nameOffset + (at * 2) + 1] = (byte)(name[at] >> 8);
            }

            bytes.Add((byte)payload.Length);
            bytes.AddRange(payload);
        }

        return [.. bytes];
    }

    [Fact]
    public void TheNamesComeOutInOrder()
    {
        Ww8FontTable table = Ww8FontTable.Parse(Table((0x00, "Times New Roman"), (0x00, "Arial")));

        table.Count.ShouldBe(2);
        table.Name(0).ShouldBe("Times New Roman");
        table.Name(1).ShouldBe("Arial");
        table.Name(2).ShouldBeNull();
        table.Name(-1).ShouldBeNull();
    }

    [Theory]
    [InlineData(0x00, FontFamilyClass.Unknown)]   // ff = FF_DONTCARE
    [InlineData(0x10, FontFamilyClass.Serif)]     // ff = FF_ROMAN
    [InlineData(0x20, FontFamilyClass.SansSerif)] // ff = FF_SWISS
    [InlineData(0x30, FontFamilyClass.Unknown)]   // ff = FF_MODERN
    [InlineData(0x40, FontFamilyClass.Unknown)]   // ff = FF_SCRIPT
    [InlineData(0x50, FontFamilyClass.Unknown)]   // ff = FF_DECORATIVE
    public void TheFontFamilyBitsBecomeAShapeTheResolverActsOn(int flags, FontFamilyClass expected)
    {
        // Only roman and swiss are carried across, because only those two move LibreOffice's answer:
        // probed on 26.2.4.2 with the family name held constant, modern, script and decorative each
        // leave the fallback exactly where an undeclared request left it. Mapping modern onto a
        // monospaced fallback is the tempting mistake and would invent a divergence.
        Ww8FontTable.Parse(Table(((byte)flags, "Some Font")))
            .ShapeOf("Some Font").Class.ShouldBe(expected);
    }

    [Fact]
    public void TheFamilyBitsAreMaskedOutOfTheByteTheyShareWithThePitchAndTheTrueTypeFlag()
    {
        // The first byte packs prq in bits 0-1, fTrueType in bit 2 and ff in bits 4-6 — mask 0x70,
        // per WW8_FFN_BASE in sw/source/filter/ww8/ww8struc.hxx, which declares `ff : 3` between two
        // reserved single bits. A reader that shifts without masking picks bit 7 up as part of the
        // family; one that does not shift at all finds a family on nearly every entry. 0xAF is
        // ff = FF_SWISS with every other bit in the byte set, so it catches both mistakes: unmasked
        // it reads 0xA, unshifted it reads 0xAF, and only (byte >> 4) & 0x07 reads 2.
        Ww8FontTable.Parse(Table((0xAF, "Some Font")))
            .ShapeOf("Some Font").Class.ShouldBe(FontFamilyClass.SansSerif);

        // 0x91 is ff = FF_ROMAN with the reserved high bit set and prq = fixed.
        Ww8FontTable.Parse(Table((0x91, "Some Font")))
            .ShapeOf("Some Font").Class.ShouldBe(FontFamilyClass.Serif);
    }

    [Theory]
    [InlineData(0x00, FontPitch.Unknown)]
    [InlineData(0x01, FontPitch.Fixed)]
    [InlineData(0x02, FontPitch.Variable)]
    public void ThePitchBitsAreReadEvenThoughLayoutDoesNotActOnThem(int flags, FontPitch expected)
    {
        // Read because the entry states it, not passed to the resolver because the only measurement
        // there is says LibreOffice's Word filters do not act on it — see
        // FontTableTests.TheDeclaredPitchIsReadButNotActedOnFromThisPart for the probe, which was run
        // on the DOCX side. The DOC side has not been probed either way, so it follows the measured
        // half rather than the guessed one.
        Ww8FontTable.Parse(Table(((byte)flags, "Some Font")))
            .ShapeOf("Some Font").Pitch.ShouldBe(expected);
    }

    [Fact]
    public void AFamilyTheTableDoesNotNameHasNoDeclaredShape()
    {
        Ww8FontTable table = Ww8FontTable.Parse(Table((0x10, "Some Font")));

        table.ShapeOf("Another Font").ShouldBe(default(DeclaredFontShape));
        table.ShapeOf(null).ShouldBe(default(DeclaredFontShape));
        Ww8FontTable.Empty.ShouldNotBeNull().ShapeOf("Some Font")
            .ShouldBe(default(DeclaredFontShape));
    }

    [Fact]
    public void ADuplicateNameKeepsTheFirstEntrysShape()
    {
        // Same rule as the DOCX table's by-name lookup, and for the same reason: a run names its font
        // as a string, so a table declaring one name twice has to answer with one of them rather
        // than throw.
        Ww8FontTable.Parse(Table((0x10, "Some Font"), (0x20, "Some Font")))
            .ShapeOf("Some Font").Class.ShouldBe(FontFamilyClass.Serif);
    }

    [Fact]
    public void AMalformedLengthStopsTheWalkRatherThanLosingTheEntriesBeforeIt()
    {
        byte[] bytes = Table((0x10, "First"), (0x20, "Second"));

        // Truncate the second entry's payload so its declared length runs past the end.
        Ww8FontTable table = Ww8FontTable.Parse(bytes.AsSpan(0, bytes.Length - 4));

        table.Count.ShouldBe(1);
        table.ShapeOf("First").Class.ShouldBe(FontFamilyClass.Serif);
    }
}
