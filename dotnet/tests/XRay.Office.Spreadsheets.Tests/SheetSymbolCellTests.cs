using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A cell set in <c>Symbol</c> draws OpenSymbol's pictures, not the Latin characters its slots
/// happen to spell.
/// </summary>
/// <remarks>
/// <para>
/// The recode is a device-level rule in LibreOffice — <c>ImplFontCache::GetFontInstance</c> hangs a
/// <c>ConvertChar</c> off the font instance whenever the resolved face is OpenSymbol and the
/// requested name is not (<c>vcl/source/font/fontcache.cxx</c>:165-169), and
/// <c>OutputDevice::ImplLayout</c> rewrites every string drawn through it
/// (<c>vcl/source/outdev/text.cxx</c>:1157-1158) — so it applies to a Calc cell exactly as it does
/// to a Writer paragraph. Without it the cell asks OpenSymbol for a code point it does not hold,
/// gets <c>.notdef</c>, and glyph fallback draws the character in DejaVu Sans.
/// </para>
/// <para>
/// Measured on 26.2.4.2: <c>REDAC_SCHEDULE_RPD_135.xls</c> and <c>…_137.xls</c> each draw
/// <c>U+00C4</c> from OpenSymbol where we drew it from DejaVu Sans, and
/// <c>021_Control_Chart_Template…xlsx</c> draws <c>s</c> — Symbol's sigma — from OpenSymbol.
/// </para>
/// </remarks>
public sealed class SheetSymbolCellTests
{
    private static readonly Length Size = Length.FromPoints(10);

    private static SheetFace Symbol =>
        SheetFonts.ForFamily("Symbol")
        ?? throw new InvalidOperationException("no face resolved for Symbol");

    [Fact]
    public void TheFamilySymbolResolvesToOpenSymbol()
        // The premise. On a machine that really had Symbol installed nothing below would recode,
        // which is the rule rather than a limitation, and this says which case is being tested.
        => SymbolFontRecode.IsSubstituteFamily(Symbol.Reference.FamilyName).ShouldBeTrue();

    [Fact]
    public void ASymbolCellDrawsFromTheFaceItResolvedTo()
    {
        // "s" is Symbol's lower-case sigma and OpenSymbol has no ASCII "s" at all, so a cell that
        // is not recoded cannot draw this from the face it resolved to.
        SheetTextRun run = SheetText.Shape("s", Symbol, Size).ShouldNotBeNull();

        run.Segments.ShouldHaveSingleItem();
        run.Segments[0].Face.Reference.FaceKey.ShouldBe(Symbol.Reference.FaceKey);
        run.Segments[0].Glyphs.ShouldAllBe(glyph => glyph.GlyphId != 0);
    }

    [Fact]
    public void ASlotWithNoLatinCounterpartIsDrawnRatherThanFallenBackOn()
    {
        // U+00C4 is Symbol's circled times, U+E136 in OpenSymbol. It is the slot the two REDAC
        // workbooks use, and the one that sent us to DejaVu Sans.
        SheetTextRun run = SheetText.Shape("Ä", Symbol, Size).ShouldNotBeNull();

        run.Segments.ShouldHaveSingleItem();
        run.Segments[0].Face.Reference.FaceKey.ShouldBe(Symbol.Reference.FaceKey);
        run.Segments[0].Glyphs.ShouldAllBe(glyph => glyph.GlyphId != 0);
    }

    [Fact]
    public void TheCellStillExtractsAsTheCharacterTheDocumentHolds()
    {
        // The recode changes what is drawn and must not change what is read: LibreOffice writes the
        // original code point into the PDF's ToUnicode, and the segment's text is what ours carries.
        SheetTextRun run = SheetText.Shape("Äs", Symbol, Size).ShouldNotBeNull();

        string.Concat(run.Segments.Select(segment => segment.Text)).ShouldBe("Äs");
    }

    [Fact]
    public void AnOrdinaryFaceIsLeftAlone()
    {
        SheetFace text = SheetFonts.ForFamily("Liberation Sans")
            ?? throw new InvalidOperationException("Liberation Sans is not installed");

        SheetTextRun recoded = SheetText.Shape("s", Symbol, Size).ShouldNotBeNull();
        SheetTextRun plain = SheetText.Shape("s", text, Size).ShouldNotBeNull();

        // Not an assertion about the widths being equal — they are two different faces — but about
        // the recode being confined to the face that asked for it: a Liberation Sans "s" is an "s".
        plain.Segments.ShouldHaveSingleItem();
        plain.Segments[0].Face.Reference.FaceKey.ShouldBe(text.Reference.FaceKey);
        recoded.Width.ShouldNotBe(plain.Width);
    }
}
