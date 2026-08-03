using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Ww8;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Word's <em>text frame</em> — the APO — and how its paragraph sprms become a floating frame.
/// </summary>
/// <remarks>
/// <para>
/// Only the mapping is covered here, not the sprm reading, and that gap is deliberate rather than an
/// oversight: <strong>LibreOffice's own DOC export cannot write one.</strong> Round-tripping a flat-ODF
/// document holding a paragraph-anchored text frame through <c>--convert-to 'doc:MS Word 97'</c>
/// produces an Escher drawing with its text in a text-box story — no <c>sprmPPc</c>, no
/// <c>sprmPDyaAbs</c>, nothing this file is about — so no fixture can be generated and none in the tree
/// contains the case. The reading half rests on a corpus measurement instead: <c>07-04.doc</c> in
/// <c>words/batch-004</c> paginated 4 pages against LibreOffice's 3 with its masthead laid out in the
/// flow, and 3 against 3 with it lifted, its first glyph landing within 0.75 pt of the reference's.
/// </para>
/// <para>
/// The values below are <c>WW8SwFlyPara</c>'s (<c>sw/source/filter/ww8/ww8par6.cxx:1885</c>). The two
/// axes deliberately disagree about nought — vertically it means "in line with the text", horizontally
/// it means "against the left edge" — which is the sort of asymmetry that is only ever got right by
/// copying it.
/// </para>
/// </remarks>
public sealed class TextFrameTests
{
    [Fact]
    public void APositionStatingNothingIsEmpty()
        => Ww8TextFramePosition.None.IsEmpty.ShouldBeTrue();

    [Fact]
    public void AWrapOfNoughtCountsAsTheDefaultOfTwo()
    {
        // `WW8FlyPara::IsEmpty`, ww8par6.cxx:1871: "wr of 0 like 2 appears to me to be equivalent for
        // checking here". A frame is not conjured out of a paragraph that only restated the default.
        (Ww8TextFramePosition.None with { Wrap = 0 }).IsEmpty.ShouldBeTrue();
    }

    [Fact]
    public void AStatedOffsetIsNotEmpty()
        => (Ww8TextFramePosition.None with { YOffset = -1079 }).IsEmpty.ShouldBeFalse();

    [Fact]
    public void TheBindingByteSplitsIntoTwoOrigins()
    {
        // 0xA0: bits 6-7 are 2 — relative to the page — and bits 4-5 are 2, relative to the paragraph.
        // The masthead of the corpus document this was found on states exactly this byte.
        Ww8TextFramePosition frame = Ww8TextFramePosition.None with { Binding = 0xA0 };
        frame.HorizontalBinding.ShouldBe<byte>(2);
        frame.VerticalBinding.ShouldBe<byte>(2);
    }

    [Fact]
    public void ANegativeVerticalOffsetLiftsTheFrameAboveItsAnchor()
    {
        PageFrame frame = Build(Ww8TextFramePosition.None with
        {
            Binding = 0xA0,
            XOffset = 1614,
            YOffset = -1079,
            Width = 9322,
            Height = 1585,
            FromTextX = 187,
            StatesVerticalPosition = true,
        });

        frame.VerticalOrigin.ShouldBe(FrameVerticalOrigin.Paragraph);
        frame.VerticalAlignment.ShouldBe(FrameVerticalAlignment.Offset);
        frame.VerticalOffset.ShouldBe(Length.FromTwips(-1079));

        frame.HorizontalOrigin.ShouldBe(FrameHorizontalOrigin.Page);
        frame.HorizontalAlignment.ShouldBe(FrameHorizontalAlignment.Offset);
        frame.HorizontalOffset.ShouldBe(Length.FromTwips(1614));

        frame.Size.Width.ShouldBe(Length.FromTwips(9322));
        frame.Size.Height.ShouldBe(Length.FromTwips(1585));

        // sprmPDxaFromText sets both horizontal margins and sprmPDyaFromText both vertical ones, which
        // is why one sprm lands in two of these.
        frame.Spacing.Left.ShouldBe(Length.FromTwips(187));
        frame.Spacing.Right.ShouldBe(Length.FromTwips(187));
        frame.Spacing.Top.ShouldBe(Length.Zero);

        // Never at-character: an APO has no anchor character anywhere in the text.
        frame.Anchor.ShouldBe(FrameAnchor.Paragraph);
    }

    [Theory]
    [InlineData(-4, FrameVerticalAlignment.Top)]
    [InlineData(-8, FrameVerticalAlignment.Middle)]
    [InlineData(-12, FrameVerticalAlignment.Bottom)]
    public void TheNegativeVerticalSentinelsNameAnEdgeRatherThanADistance(
        short offset, FrameVerticalAlignment expected)
    {
        PageFrame frame = Build(Ww8TextFramePosition.None with
        {
            Binding = 0xA0,
            YOffset = offset,
            StatesVerticalPosition = true,
        });

        frame.VerticalAlignment.ShouldBe(expected);
        frame.VerticalOffset.ShouldBe(Length.Zero);
    }

    [Theory]
    [InlineData(0, FrameHorizontalAlignment.Left)]
    [InlineData(-4, FrameHorizontalAlignment.Centre)]
    [InlineData(-8, FrameHorizontalAlignment.Right)]
    [InlineData(-12, FrameHorizontalAlignment.Inside)]
    [InlineData(-16, FrameHorizontalAlignment.Outside)]
    public void TheHorizontalSentinelsIncludeNoughtWhereTheVerticalOnesDoNot(
        short offset, FrameHorizontalAlignment expected)
        => Build(Ww8TextFramePosition.None with { XOffset = offset })
            .HorizontalAlignment.ShouldBe(expected);

    [Fact]
    public void AVerticalOffsetOfNoughtKeepsTheFrameWithItsText()
    {
        // #i18732#: whatever origin the binding names, a dyaAbs of nought means the frame sits in line
        // with the text it was written beside — so a page-relative binding is ignored.
        Build(Ww8TextFramePosition.None with { Binding = 0x10, YOffset = 0 })
            .VerticalOrigin.ShouldBe(FrameVerticalOrigin.Paragraph);
    }

    [Theory]
    [InlineData(1, TextWrap.TopAndBottom)]
    [InlineData(3, TextWrap.TopAndBottom)]
    [InlineData(2, TextWrap.Both)]
    [InlineData(4, TextWrap.Both)]
    [InlineData(5, TextWrap.Through)]
    [InlineData(0, TextWrap.Optimal)]
    public void EachWrapModeMapsAsTheImporterMapsIt(byte wrap, TextWrap expected)
        => Build(Ww8TextFramePosition.None with { Wrap = wrap, YOffset = -1079 })
            .Wrap.ShouldBe(expected);

    [Fact]
    public void AWidthOfTenOrLessMeansAsWideAsTheTextBesideIt()
    {
        Length column = Length.FromTwips(9360);

        Build(Ww8TextFramePosition.None with { Width = 0, YOffset = -1079 }, column)
            .Size.Width.ShouldBe(column);
        Build(Ww8TextFramePosition.None with { Width = 10, YOffset = -1079 }, column)
            .Size.Width.ShouldBe(column);

        // Eleven is a width, not a request for one — and it still clears Word's minimum.
        Build(Ww8TextFramePosition.None with { Width = 11, YOffset = -1079 }, column)
            .Size.Width.ShouldBe(Length.FromTwips(23));
    }

    [Fact]
    public void TheHighBitOfTheHeightAsksForAMinimumRatherThanASize()
    {
        // 0x8000 | 1585. Reading the word without masking it gives a frame 39 feet tall.
        Build(Ww8TextFramePosition.None with { Height = unchecked((short)0x8631), YOffset = -1079 })
            .Size.Height.ShouldBe(Length.FromTwips(1585));
    }

    private static PageFrame Build(Ww8TextFramePosition position, Length columnWidth = default)
    {
        PageFrame? frame = Ww8TextFrames.Build(
            new Ww8LayoutTextFrame(position, []), [], columnWidth);

        frame.ShouldNotBeNull();
        return frame;
    }
}
