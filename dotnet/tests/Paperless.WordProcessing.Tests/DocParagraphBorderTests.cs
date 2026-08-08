using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Ww8;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A DOC's paragraph borders: the five <c>BRC</c> sprms, and the box two of them share.
/// </summary>
/// <remarks>
/// <para>
/// The reader-side half of what <see cref="ParagraphBorderTests"/> measures for DOCX. It is written
/// against the sprms rather than against a document because LibreOffice's own DOC <em>export</em> cannot
/// be used to make a fixture that isolates a border — it rewrites the whole paragraph — and no binary
/// document in the tree carries one. What a real <c>.doc</c> proves instead is measured by rendering the
/// corpus; see the round's notes in <c>TODO.batches.md</c>.
/// </para>
/// <para>
/// The quantities are the ones LibreOffice's <c>WW8_BRCVer9::DetermineBorderProperties</c> computes:
/// <c>dptLineWidth</c> in eighths of a point, <c>dptSpace</c> in whole points. They are the same two
/// quantities, in the same two units, that <c>w:sz</c> and <c>w:space</c> carry — which is the claim that
/// lets one <see cref="ParagraphBorderSet"/> and one paginator serve both readers, and the reason it is
/// worth asserting rather than assuming.
/// </para>
/// </remarks>
public sealed class DocParagraphBorderTests
{
    /// <summary>
    /// Each of the ten border sprms names its side, and the two bars name none.
    /// </summary>
    /// <remarks>
    /// The ids are dense — top, left, bottom, right, between, bar — so an off-by-one reads a paragraph's
    /// left rule as its top, and running one id too far reads the revision bar as a border of the box.
    /// </remarks>
    [Theory]
    [InlineData(0x6424, 0, false)]
    [InlineData(0x6425, 1, false)]
    [InlineData(0x6426, 2, false)]
    [InlineData(0x6427, 3, false)]
    [InlineData(0x6428, 4, false)]
    [InlineData(0xC64E, 0, true)]
    [InlineData(0xC64F, 1, true)]
    [InlineData(0xC650, 2, true)]
    [InlineData(0xC651, 3, true)]
    [InlineData(0xC652, 4, true)]
    public void EachBorderSprmNamesItsSideAndItsForm(int identifier, int side, bool isVersion9)
    {
        (int Side, bool IsVersion9)? found =
            Ww8DocumentReader.ParagraphBorderSprm((ushort)identifier);

        found.ShouldNotBeNull();
        found.Value.Side.ShouldBe(side);
        found.Value.IsVersion9.ShouldBe(isVersion9);
    }

    /// <summary>The bar sprms and the ids either side of the two runs set no border.</summary>
    [Theory]
    [InlineData(0x6423)]
    [InlineData(0x6429)]
    [InlineData(0xC64D)]
    [InlineData(0xC653)]
    public void ARevisionBarIsNotABorderOfTheBox(int identifier)
        => Ww8DocumentReader.ParagraphBorderSprm((ushort)identifier).ShouldBeNull();

    /// <summary>
    /// A <c>BRC80</c>'s width is eighths of a point and its distance is whole points.
    /// </summary>
    /// <remarks>
    /// The bytes are <c>dptLineWidth</c>, <c>brcType</c>, <c>ico</c>, then a byte whose low five bits are
    /// <c>dptSpace</c>. Reading that last field as twips — the unit every other length in the structure's
    /// neighbourhood uses — puts a 10 pt distance at half a point, which is the mistake this asserts
    /// against.
    /// </remarks>
    [Theory]
    [InlineData(4, 0, 0.50, 0.00)]
    [InlineData(4, 1, 0.50, 1.00)]
    [InlineData(18, 1, 2.25, 1.00)]
    [InlineData(24, 10, 3.00, 10.00)]
    public void AnEightyFormBorderCarriesItsWidthInEighthsAndItsDistanceInPoints(
        int eighths, int space, double width, double distance)
    {
        ParagraphBorder side = Top([(byte)eighths, 1, 0, (byte)space], isVersion9: false);

        side.Width.ShouldBe(Length.FromPoints(width));
        side.Space.ShouldBe(Length.FromPoints(distance));
        side.Allowance.ShouldBe(Length.FromPoints(width + distance));
    }

    /// <summary>
    /// The bits above <c>dptSpace</c> are flags and are not part of the distance.
    /// </summary>
    /// <remarks>
    /// <c>fShadow</c> and <c>fFrame</c> sit at bits 5 and 6 of the same byte. Taking the byte whole turns
    /// a shadowed 10 pt border into a 42 pt one, which is most of a page's text width on the wrong axis.
    /// </remarks>
    [Fact]
    public void TheShadowAndFrameFlagsDoNotReachTheDistance()
        => Top([4, 1, 0, 10 | 0x20 | 0x40], isVersion9: false)
            .Space.ShouldBe(Length.FromPoints(10));

    /// <summary>
    /// The newer <c>BRC</c> carries a full RGB colour in front of the same two quantities.
    /// </summary>
    /// <remarks>
    /// Eight bytes: a <c>COLORREF</c>, then <c>dptLineWidth</c> and <c>brcType</c>, then a word whose low
    /// five bits are <c>dptSpace</c>. The colour is why it is worth reading at all — the older form names
    /// one of Word's seventeen palette entries and returns <c>#C0C0C0</c> where the document said
    /// <c>#CCCCCC</c>.
    /// </remarks>
    [Fact]
    public void TheNinetyFormCarriesRgbBesideTheSameWidthAndDistance()
    {
        ParagraphBorder side = Top([0xCC, 0xCC, 0xCC, 0x00, 12, 1, 3, 0], isVersion9: true);

        side.Width.ShouldBe(Length.FromPoints(1.5));
        side.Space.ShouldBe(Length.FromPoints(3));
        side.Colour.ShouldBe(new Colour(0xCC, 0xCC, 0xCC));
    }

    /// <summary>
    /// A sprm whose <c>BRC</c> states nothing is a border removed, not a border unstated.
    /// </summary>
    /// <remarks>
    /// <c>##826##</c> in <c>SwWW8ImplReader::SetBorder</c>: a style sets a rule and the paragraph takes it
    /// away, and the only way WW8 can say so is the sprm present with a nil or zero-typed structure.
    /// Reading that as silence leaves the style's rule drawn on a paragraph that asked for none — and
    /// reserves the room for it as well, so the text below moves too.
    /// </remarks>
    [Theory]
    [InlineData(new byte[] { 0xFF, 0xFF, 0xFF, 0xFF })]
    [InlineData(new byte[] { 8, 0, 0, 4 })]
    public void ABorderRemovedIsStatedRatherThanSilent(byte[] brc)
    {
        Ww8LayoutFormat stated = Ww8DocumentReader.WithParagraphBorder(
            default, side: 0, brc, isVersion9: false);

        stated.BorderTop.ShouldNotBeNull();

        ParagraphBorderSet? set = stated.ToParagraphBorders();
        set.ShouldNotBeNull();
        set.Top.ShouldNotBeNull();
        set.Top.Value.Draws.ShouldBeFalse();
        set.Above.ShouldBe(Length.Zero);
    }

    /// <summary>A paragraph none of whose sprms mention a border has no border set at all.</summary>
    [Fact]
    public void AParagraphWithNoBorderSprmsHasNoSet()
        => default(Ww8LayoutFormat).ToParagraphBorders().ShouldBeNull();

    /// <summary>
    /// Two paragraphs bordered alike are one box: no rule between them and one allowance, not two.
    /// </summary>
    /// <remarks>
    /// The join is the shared half — it compares two <see cref="ParagraphBorderSet"/>s and knows nothing
    /// about which reader made them — so this exercises the same code the DOCX flow runs and the DOC flow
    /// now runs too.
    /// </remarks>
    [Fact]
    public void TwoParagraphsBorderedAlikeShareOneBox()
    {
        ParagraphBorderSet boxed = Box();
        List<PageBlock> blocks = [Paragraph(boxed), Paragraph(boxed), Paragraph(null)];

        ParagraphBorderJoin.Apply(blocks);

        PageParagraph upper = (PageParagraph)blocks[0];
        PageParagraph lower = (PageParagraph)blocks[1];

        upper.Borders!.Top.ShouldNotBeNull();
        upper.Borders.Bottom.ShouldBeNull();
        upper.BorderBelow.ShouldBe(Length.Zero);

        lower.Borders!.Top.ShouldBeNull();
        lower.Borders.JoinsAbove.ShouldBeTrue();
        lower.BorderAbove.ShouldBe(Length.Zero);
        lower.Borders.Bottom.ShouldNotBeNull();
    }

    /// <summary>Two paragraphs bordered differently keep a box each.</summary>
    [Fact]
    public void TwoParagraphsBorderedDifferentlyKeepABoxEach()
    {
        List<PageBlock> blocks =
        [
            Paragraph(Box()),
            Paragraph(Box() with { Left = null }),
        ];

        ParagraphBorderJoin.Apply(blocks);

        ((PageParagraph)blocks[0]).Borders!.Bottom.ShouldNotBeNull();
        ((PageParagraph)blocks[1]).Borders!.Top.ShouldNotBeNull();
        ((PageParagraph)blocks[1]).Borders!.JoinsAbove.ShouldBeFalse();
    }

    /// <summary>The top side a <c>BRC</c> resolves to, read through the reader's own two steps.</summary>
    private static ParagraphBorder Top(byte[] brc, bool isVersion9)
    {
        ParagraphBorderSet? set = Ww8DocumentReader
            .WithParagraphBorder(default, side: 0, brc, isVersion9)
            .ToParagraphBorders();

        set.ShouldNotBeNull();
        set.Top.ShouldNotBeNull();
        return set.Top.Value;
    }

    /// <summary>A four-sided box: a half-point rule at one point's distance, which is Word's default.</summary>
    private static ParagraphBorderSet Box()
    {
        ParagraphBorder rule = new(Length.FromPoints(0.5), Length.FromPoints(1), Colour.Black);
        return new ParagraphBorderSet
        {
            Top = rule, Left = rule, Bottom = rule, Right = rule,
        };
    }

    private static PageParagraph Paragraph(ParagraphBorderSet? borders)
        => new() { Text = "text", Face = Face, Borders = borders };

    /// <summary>A real face, since a <see cref="PageParagraph"/> requires one.</summary>
    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
