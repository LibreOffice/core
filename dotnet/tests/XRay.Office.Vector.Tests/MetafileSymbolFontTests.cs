using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Vector.Metafiles;
using Shouldly;

namespace XRay.Office.Vector.Tests;

/// <summary>
/// A metafile font that states <c>lfCharSet = SYMBOL_CHARSET</c> addresses glyphs by slot.
/// </summary>
/// <remarks>
/// <para>
/// <c>010605Vul.ppt</c>'s page 9 is an EMF whose 25 symbol runs name <c>Monotype Sorts</c> at
/// <c>lfCharSet = 2</c> and store <c>0xE8</c> and <c>0x59</c>. The reference draws an arrow and
/// a star out of OpenSymbol and puts <c>U+F0E8</c> / <c>U+F059</c> in the PDF's
/// <c>ToUnicode</c>; decoding the bytes through Windows-1252 instead draws a Latin e-grave and
/// a Latin <c>Y</c> in a serif face, and nineteen of those tokens then count as <em>words</em>.
/// That is what took the document's extractable count to 963 against a 944 reference and a
/// 962.88 band.
/// </para>
/// <para>
/// The condition is both halves — the character set <em>and</em> a family
/// <c>SymbolFontRecode</c> has a table for — which is LibreOffice's own rule: a symbol-encoded
/// request never reaches fontconfig, so the substitution it lands on is StarSymbol or
/// OpenSymbol and nothing else.
/// </para>
/// </remarks>
public class MetafileSymbolFontTests
{
    private const int Mm = 100;
    private const byte Symbol = 0x02;
    private const byte Ansi = 0x00;

    /// <summary>
    /// The two slots <c>010605Vul.ppt</c> actually draws, and what LibreOffice's own table says
    /// they are.
    /// </summary>
    /// <remarks>
    /// <c>0xE8</c> recodes to <c>U+27A8</c> — a real Unicode arrow, not a Private Use Area code
    /// point — and <c>0x59</c> to <c>U+E223</c>, which is one of OpenSymbol's own. So the two
    /// take different routes to the page and only the second resolves to OpenSymbol: the first
    /// is covered by the ordinary text face and falls to it, which is what the reference does
    /// too. Asserting the <em>face</em> would have made this test a fixture for the font set
    /// rather than for the recode, and it failed on exactly that.
    /// </remarks>
    [Fact]
    public void ASymbolCharsetRunIsRecodedOffItsLatinReading()
    {
        Recorder arrow = Draw(new EmfBuilder()
            .Font(1, "Monotype Sorts", -300, charSet: Symbol)
            .Select(1)
            .Text(0, 10 * Mm, "\u00E8"));

        arrow.GlyphRuns.ShouldHaveSingleItem().Text.ShouldBe("\u27A8");

        Recorder star = Draw(new EmfBuilder()
            .Font(1, "Monotype Sorts", -300, charSet: Symbol)
            .Select(1)
            .Text(0, 10 * Mm, "Y"));

        GlyphRun run = star.GlyphRuns.ShouldHaveSingleItem();
        run.Text.ShouldBe("\uE223");
        run.Font.RequestedFamily.ShouldBe("OpenSymbol");
    }

    /// <summary>
    /// The control: the same face name at the ANSI character set is a Latin run and must stay one.
    /// </summary>
    [Fact]
    public void AnAnsiRunInTheSameFaceIsUntouched()
    {
        Recorder recorder = Draw(new EmfBuilder()
            .Font(1, "Monotype Sorts", -300, charSet: Ansi)
            .Select(1)
            .Text(0, 10 * Mm, "\u00E8"));

        GlyphRun run = recorder.GlyphRuns.ShouldHaveSingleItem();
        run.Font.RequestedFamily.ShouldBe("Monotype Sorts");
        run.Text.ShouldBe("\u00E8");
    }

    /// <summary>
    /// The second control: a symbol character set on a face with no recode table is left alone
    /// rather than being invented into OpenSymbol. Two corpus records are exactly this —
    /// <c>UniversalMath1 BT</c>.
    /// </summary>
    [Fact]
    public void ASymbolCharsetRunInAnUnknownFaceIsUntouched()
    {
        Recorder recorder = Draw(new EmfBuilder()
            .Font(1, "UniversalMath1 BT", -300, charSet: Symbol)
            .Select(1)
            .Text(0, 10 * Mm, "\u00E8"));

        GlyphRun run = recorder.GlyphRuns.ShouldHaveSingleItem();
        run.Font.RequestedFamily.ShouldBe("UniversalMath1 BT");
        run.Text.ShouldBe("\u00E8");
    }

    /// <summary>The unit the round-trip is built on, stated on its own.</summary>
    [Fact]
    public void TheSlotIsMovedIntoThePrivateUseAreaBeforeItIsRecoded()
    {
        MetafileFont symbol = new("Monotype Sorts", Length.FromPoints(12), CharacterSet: Symbol);
        MetafileFont ansi = new("Monotype Sorts", Length.FromPoints(12), CharacterSet: Ansi);

        MetafileTextEngine.IsSlotAddressed(symbol).ShouldBeTrue();
        MetafileTextEngine.IsSlotAddressed(ansi).ShouldBeFalse();

        MetafileTextEngine.FamilyOf(symbol).ShouldBe("OpenSymbol");
        MetafileTextEngine.FamilyOf(ansi).ShouldBe("Monotype Sorts");

        // The move is (c & 0x00ff) | 0xf000: a run already in the Private Use Area is left
        // where it is rather than masked a second time.
        MetafileTextEngine.Symbolise("\uF0E8", symbol)
            .ShouldBe(MetafileTextEngine.Symbolise("\u00E8", symbol));

        // And what it lands on is an OpenSymbol code point, not the slot.
        MetafileTextEngine.Symbolise("\u00E8", symbol).ShouldNotBe("\uF0E8");

        MetafileTextEngine.Symbolise("\u00E8", ansi).ShouldBe("\u00E8");
    }

    private static Recorder Draw(EmfBuilder builder)
    {
        Recorder recorder = new();
        VectorImage image = builder.Decode();
        image.Draw(recorder, new DocRect(DocPoint.Origin, image.IntrinsicSize));
        return recorder;
    }
}
