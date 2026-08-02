using Paperless.MsBinary.Escher;
using Paperless.Presentations.MsBinary;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Which shapes a binary PowerPoint deck shrinks to fit, which nothing in the file states.
/// </summary>
/// <remarks>
/// <para>
/// DrawingML asks for the fit outright with <c>a:normAutofit</c>; the binary format has no such
/// element, and the import derives it from the <em>kind</em> of text the shape holds. A
/// <c>TextHeaderAtom</c> naming Body, HalfBody or QuarterBody gets
/// <c>TextFitToSizeType_AUTOFIT</c>; a title, a subtitle and an ordinary shape's own text get
/// none (<c>filter/source/msfilter/svdfppt.cxx:1030-1099</c>). Two shape properties then take it
/// away again, because they grow the box instead of shrinking the text.
/// </para>
/// <para>
/// The rule is unit-tested rather than measured because it is a classification, and the sizes it
/// leads to are measured elsewhere: <see cref="SlideAutofitTests"/> pins the search against
/// LibreOffice's own PDFs. What it is worth is measured on the corpus — <c>berlin.ppt</c>, whose
/// 29 slides are outline placeholders throughout, drew 1356 of the reference's 1395 words before
/// this and 1386 after, with the page count already exactly right both times: the missing text
/// was overflowing the shape, running off the bottom of the slide and being clipped by the page.
/// </para>
/// </remarks>
public class PptAutofitRuleTests
{
    /// <summary>A shape whose property table holds one entry.</summary>
    private static EscherShape Shape(ushort property, uint value)
    {
        byte[] entry =
        [
            (byte)(property & 0xFF), (byte)(property >> 8),
            (byte)(value & 0xFF), (byte)((value >> 8) & 0xFF),
            (byte)((value >> 16) & 0xFF), (byte)((value >> 24) & 0xFF),
        ];

        return new EscherShape { Properties = EscherPropertyTable.Read(entry, 1) };
    }

    private static EscherShape Plain() => new();

    private static PptTextRun Text(PptTextKind kind) => new(kind, "text", [], []);

    /// <summary>The three outline kinds shrink; nothing else does.</summary>
    [Theory]
    [InlineData(PptTextKind.Body, true)]
    [InlineData(PptTextKind.HalfBody, true)]
    [InlineData(PptTextKind.QuarterBody, true)]
    [InlineData(PptTextKind.Title, false)]
    [InlineData(PptTextKind.CentreTitle, false)]
    [InlineData(PptTextKind.CentreBody, false)]
    [InlineData(PptTextKind.Notes, false)]
    [InlineData(PptTextKind.Other, false)]
    public void OnlyAnOutlineBodyShrinksToFit(PptTextKind kind, bool expected)
    {
        PptSlideLayout.Autofits(Plain(), Text(kind)).ShouldBe(expected);
    }

    /// <summary>
    /// A shape that grows around its text does not shrink the text instead.
    /// </summary>
    /// <remarks>
    /// <c>fFitShapeToText</c> is bit 1 of <c>DFF_Prop_FitTextToShape</c> and bit 0 is
    /// <c>fFitTextToShape</c>, which the drawing layer ignores — so a shape stating only bit 0
    /// still autofits, and reading the property as a boolean would wrongly stop it.
    /// </remarks>
    [Theory]
    [InlineData(0u, true)]
    [InlineData(1u, true)]
    [InlineData(2u, false)]
    [InlineData(3u, false)]
    public void AShapeThatGrowsToItsTextDoesNotShrinkIt(uint fitTextToShape, bool expected)
    {
        PptSlideLayout
            .Autofits(Shape(PptShapeGeometry.FitTextToShape, fitTextToShape), Text(PptTextKind.Body))
            .ShouldBe(expected);
    }

    /// <summary>
    /// A box whose lines never wrap grows sideways instead, so it does not shrink either.
    /// </summary>
    [Fact]
    public void ANonWrappingBoxDoesNotShrink()
    {
        PptSlideLayout
            .Autofits(Shape(EscherPropertyIds.WrapText, PptShapeGeometry.WrapNone), Text(PptTextKind.Body))
            .ShouldBeFalse();

        // Wrap-at-the-shape and wrap-by-a-margin both wrap, so both still shrink.
        PptSlideLayout.Autofits(Shape(EscherPropertyIds.WrapText, 0), Text(PptTextKind.Body))
                      .ShouldBeTrue();
        PptSlideLayout.Autofits(Shape(EscherPropertyIds.WrapText, 1), Text(PptTextKind.Body))
                      .ShouldBeTrue();
    }
}
