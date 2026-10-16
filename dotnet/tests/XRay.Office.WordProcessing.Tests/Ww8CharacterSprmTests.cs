using System.Buffers.Binary;
using XRay.Office.WordProcessing.Ww8;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Tests the order-dependent rules in a CHPX: which sprm wins when two of them set the same thing.
/// </summary>
/// <remarks>
/// A grpprl is a list, and a reader that simply applies it left to right gets the font wrong on any
/// document Word wrote a symbol into — because Word writes both <c>sprmCSymbol</c> and
/// <c>sprmCRgFtc0</c>, in that order, and only the first of them is the run's font.
/// </remarks>
public class Ww8CharacterSprmTests
{
    private const ushort FontIndexSprm = 0x4A4F;
    private const ushort SymbolSprm = 0x6A09;
    private const ushort FontSizeSprm = 0x4A43;

    /// <summary>A two-byte-operand sprm.</summary>
    private static byte[] Word(ushort identifier, ushort operand)
    {
        byte[] bytes = new byte[4];
        BinaryPrimitives.WriteUInt16LittleEndian(bytes, identifier);
        BinaryPrimitives.WriteUInt16LittleEndian(bytes.AsSpan(2), operand);
        return bytes;
    }

    /// <summary><c>sprmCSymbol</c>: a font-table index and the code point it addresses.</summary>
    private static byte[] Symbol(ushort fontIndex, char slot)
    {
        byte[] bytes = new byte[6];
        BinaryPrimitives.WriteUInt16LittleEndian(bytes, SymbolSprm);
        BinaryPrimitives.WriteUInt16LittleEndian(bytes.AsSpan(2), fontIndex);
        BinaryPrimitives.WriteUInt16LittleEndian(bytes.AsSpan(4), slot);
        return bytes;
    }

    private static Ww8LayoutFormat Apply(params byte[][] sprms)
        => Ww8DocumentReader.ApplyLayoutSprms(
            default, ((byte[])[.. sprms.SelectMany(s => s)]).AsMemory(),
            Ww8DocumentProperties.Default);

    /// <summary>
    /// The whole of <c>Read_FontCode</c>'s first two lines, on the byte layout Word writes.
    /// </summary>
    /// <remarks>
    /// <c>if (m_bSymbol) return;</c> — "if bSymbol, the symbol's font (see sprmCSymbol) is valid!"
    /// (<c>sw/source/filter/ww8/ww8par6.cxx</c>:3963-3966). The CHPX behind
    /// <c>150_5300_13_chg12.doc</c>'s greater-or-equal sign is exactly this shape:
    /// <c>096A 0100 B3F0</c> — Symbol, <c>U+F0B3</c> — and then <c>4F4A 0000</c>, Times New Roman.
    /// </remarks>
    [Fact]
    public void AFontCodeAfterASymbolIsTheParagraphsFontAndIsDropped()
    {
        Ww8LayoutFormat format = Apply(
            Symbol(1, ''),
            Word(FontSizeSprm, 18),
            Word(FontIndexSprm, 0));

        format.SymbolSlot.ShouldBe('');
        format.FontIndex.ShouldBe(1);
        format.FontSizeHalfPoints.ShouldBe(18);
    }

    /// <summary>A font code before the symbol is overwritten by it, as applying in order does.</summary>
    [Fact]
    public void AFontCodeBeforeASymbolLosesToTheSymbolsOwnFace()
    {
        Ww8LayoutFormat format = Apply(Word(FontIndexSprm, 0), Symbol(1, ''));

        format.SymbolSlot.ShouldBe('');
        format.FontIndex.ShouldBe(1);
    }

    /// <summary>Without a symbol, the last font code in the grpprl still wins.</summary>
    [Fact]
    public void TheLastFontCodeWinsWhenNoSymbolSilencesIt()
        => Apply(Word(FontIndexSprm, 0), Word(FontIndexSprm, 5)).FontIndex.ShouldBe(5);

    /// <summary>The rule is per grpprl: a style's symbol does not silence a run's own font.</summary>
    /// <remarks>
    /// <c>m_bSymbol</c> is reader state that <c>WW8RStyle::PostStyle</c> clears when a style has
    /// been read and <c>ReadChars</c> clears when the character has been drawn, so it never spans
    /// two grpprls. Applying the two separately is how a style and the direct formatting over it
    /// reach this, so the second call must start clean.
    /// </remarks>
    [Fact]
    public void ASymbolInOneGrpprlDoesNotSilenceAFontCodeInTheNext()
    {
        Ww8LayoutFormat fromStyle = Apply(Symbol(1, ''));
        Ww8LayoutFormat withRun = Ww8DocumentReader.ApplyLayoutSprms(
            fromStyle, ((byte[])[.. Word(FontIndexSprm, 3)]).AsMemory(),
            Ww8DocumentProperties.Default);

        withRun.FontIndex.ShouldBe(3);
    }

    /// <summary>
    /// A slot is not formatting, so two runs that differ only in it are two runs.
    /// </summary>
    /// <remarks>
    /// The characters a WW8 symbol run covers are placeholders and the slot is what replaces every
    /// one of them, so merging a symbol run into its neighbour either loses the symbol or spreads
    /// it over the neighbour's text.
    /// </remarks>
    [Fact]
    public void TwoRunsDifferingOnlyInTheirSlotAreNotOneRun()
    {
        Ww8DocumentReader.Ww8LayoutRun plain = new(
            0, 1, "Symbol", Core.Units.Length.FromPoints(10), 400, false, null, null);

        Ww8DocumentReader.MatchesFormatting(plain, plain with { SymbolSlot = '' })
            .ShouldBeFalse();
        Ww8DocumentReader.MatchesFormatting(
                plain with { SymbolSlot = '' }, plain with { SymbolSlot = '' })
            .ShouldBeFalse();
        Ww8DocumentReader.MatchesFormatting(
                plain with { SymbolSlot = '' }, plain with { SymbolSlot = '' })
            .ShouldBeTrue();
    }
}
