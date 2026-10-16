using System.Buffers.Binary;
using System.Text;
using XRay.Office.WordProcessing.Ww8;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Tests the font a WW8 style that names none of its own is set in.
/// </summary>
/// <remarks>
/// <para>
/// It is not in any style's CHPX and not in the DOP: it is <c>Stshi.ftcAsci</c>, a bare font-table
/// index twelve bytes into the stylesheet's own header, and
/// <c>WW8RStyle::Set1StyleDefaults</c> applies it to every paragraph style based on nothing that
/// changed no font (<c>sw/source/filter/ww8/ww8par2.cxx</c>:3714-3725).
/// </para>
/// <para>
/// Word 2007 and later write .doc that way whenever the default is not the font table's first
/// entry. Without it every run that states no font of its own falls to the resolver's own default —
/// on <c>AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc</c> that is Liberation Serif where
/// 26.2.4.2 sets the body in Carlito, which is not metric-compatible and so reflows the document.
/// </para>
/// </remarks>
public class Ww8StyleSheetDefaultFontTests
{
    private const ushort FontIndexSprm = 0x4A4F;

    /// <summary>The <c>Stshi</c> header, with <c>ftcAsci</c> where <c>WW8Style</c> reads it.</summary>
    /// <remarks>
    /// Six words come first — <c>cstd</c>, <c>cbSTDBaseInFile</c>, the flags,
    /// <c>stiMaxWhenSaved</c>, <c>istdMaxFixedWhenSaved</c> and <c>nVerBuiltInNamesWhenSaved</c> —
    /// and the three font codes follow: ASCII, Far East, other.
    /// </remarks>
    private static byte[] Header(int styleCount, ushort defaultFont, int length = 18)
    {
        byte[] header = new byte[length];
        BinaryPrimitives.WriteUInt16LittleEndian(header, (ushort)styleCount);
        BinaryPrimitives.WriteUInt16LittleEndian(header.AsSpan(2), 18);
        if (length >= 14) BinaryPrimitives.WriteUInt16LittleEndian(header.AsSpan(12), defaultFont);
        return header;
    }

    /// <summary>One STD: a paragraph or character style with a base and its own character sprms.</summary>
    private static byte[] Style(byte kind, ushort baseIndex, string name, params byte[] chpx)
    {
        // The second word packs the kind into its low nibble and the base index into the rest.
        ushort kindAndBase = (ushort)((baseIndex << 4) | kind);
        List<byte> std = [0, 0, (byte)(kindAndBase & 0xFF), (byte)(kindAndBase >> 8)];

        // The fixed part is eighteen bytes; the name follows it as a counted UTF-16 string with a
        // terminator, and the UPXs follow that.
        std.AddRange(new byte[18 - std.Count]);
        std.Add((byte)name.Length);
        std.Add(0);
        std.AddRange(Encoding.Unicode.GetBytes(name));
        std.AddRange([0, 0]);

        // A paragraph style's first UPX is its PAPX, which opens with its own istd; a character
        // style has only the CHPX.
        if (kind == 1)
        {
            std.AddRange([2, 0, 0, 0]);
        }

        std.Add((byte)chpx.Length);
        std.Add(0);
        std.AddRange(chpx);
        if ((chpx.Length & 1) != 0) std.Add(0);

        return [.. std];
    }

    private static byte[] Sheet(byte[] header, params byte[][] styles)
    {
        List<byte> bytes = [(byte)header.Length, (byte)(header.Length >> 8), .. header];

        foreach (byte[] style in styles)
        {
            bytes.Add((byte)style.Length);
            bytes.Add((byte)(style.Length >> 8));
            bytes.AddRange(style);
            if ((style.Length & 1) != 0) bytes.Add(0);
        }

        return [.. bytes];
    }

    private static byte[] FontCode(ushort index)
    {
        byte[] sprm = new byte[4];
        BinaryPrimitives.WriteUInt16LittleEndian(sprm, FontIndexSprm);
        BinaryPrimitives.WriteUInt16LittleEndian(sprm.AsSpan(2), index);
        return sprm;
    }

    private static ushort? FontIn(IEnumerable<ReadOnlyMemory<byte>> chain)
    {
        ushort? found = null;
        foreach (ReadOnlyMemory<byte> grpprl in chain)
        {
            if (Ww8DocumentReader.ApplyLayoutSprms(
                    default, grpprl, Ww8DocumentProperties.Default).FontIndex is { } index)
            {
                found = (ushort)index;
            }
        }

        return found;
    }

    [Fact]
    public void TheHeaderStatesTheDefaultFont()
        => Ww8StyleSheet.Parse(Sheet(Header(1, 4), Style(1, 4095, "Normal")))
            .DefaultFontIndex.ShouldBe((ushort)4);

    /// <summary>A header too short to hold the field leaves it nought, as <c>WW8Style</c> does.</summary>
    [Fact]
    public void AHeaderTooShortToStateOneLeavesItAtTheFirstFontTableEntry()
        => Ww8StyleSheet.Parse(Sheet(Header(1, 4, length: 12), Style(1, 4095, "Normal")))
            .DefaultFontIndex.ShouldBe((ushort)0);

    /// <summary>
    /// A root paragraph style that sets no font of its own is set in the document's default.
    /// </summary>
    [Fact]
    public void ARootParagraphStyleSettingNoFontTakesTheDocumentsDefault()
    {
        Ww8StyleSheet sheet = Ww8StyleSheet.Parse(Sheet(Header(1, 4), Style(1, 4095, "Normal")));

        FontIn(sheet.ResolveCharacterChain(0)).ShouldBe((ushort)4);
    }

    /// <summary>Its own font still wins: the default is prepended, so the style states it later.</summary>
    [Fact]
    public void ARootParagraphStyleThatStatesAFontKeepsIt()
    {
        Ww8StyleSheet sheet = Ww8StyleSheet.Parse(
            Sheet(Header(1, 4), Style(1, 4095, "Normal", FontCode(6))));

        FontIn(sheet.ResolveCharacterChain(0)).ShouldBe((ushort)6);
    }

    /// <summary>And so does a style based on it, whose own chain carries the same default.</summary>
    [Fact]
    public void ADerivedStyleInheritsTheDefaultThroughItsBase()
    {
        Ww8StyleSheet sheet = Ww8StyleSheet.Parse(
            Sheet(Header(2, 4), Style(1, 4095, "Normal"), Style(1, 0, "Body")));

        FontIn(sheet.ResolveCharacterChain(1)).ShouldBe((ushort)4);
    }

    /// <summary>
    /// A character style based on nothing does not take it: <c>Set1StyleDefaults</c> is guarded on
    /// <c>rSI.m_bColl</c>, which is true only of a paragraph style.
    /// </summary>
    [Fact]
    public void ARootCharacterStyleDoesNotTakeTheDocumentsDefault()
    {
        Ww8StyleSheet sheet = Ww8StyleSheet.Parse(
            Sheet(Header(2, 4), Style(1, 4095, "Normal"), Style(2, 4095, "Emphasis")));

        FontIn(sheet.ResolveCharacterChain(1)).ShouldBeNull();
    }

    /// <summary>The paragraph half of a chain never carries it: it is a character property.</summary>
    [Fact]
    public void TheParagraphChainDoesNotCarryTheDefaultFont()
        => Ww8StyleSheet.Parse(Sheet(Header(1, 4), Style(1, 4095, "Normal")))
            .ResolveChain(0)
            .ShouldBeEmpty();
}
