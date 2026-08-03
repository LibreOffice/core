using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// A slide whose background is a <c>p:bgRef</c> into the theme rather than a stated fill.
/// </summary>
/// <remarks>
/// <para>
/// <c>p:bg</c> holds one of two children and only one of them is a fill outright. <c>p:bgPr</c>
/// carries it; <c>p:bgRef</c> carries an index into <c>a:bgFillStyleLst</c> — a fourth style list
/// beside the three <c>a:fillRef</c> uses — plus the colour those entries' <c>phClr</c> stands
/// for. Nothing read the second form, so every deck using it came out white.
/// </para>
/// <para>
/// White is the reason this went unseen: it changes no page count and no word count, and the
/// three checks the corpus gate runs are exactly those plus font embedding. It surfaced only
/// under the image comparison, on
/// <c>slides/batch-010/pptx/HENTZEN_COMPOSITE_MATERIALS_IN_THE_AEROSPACE_INDUSTRY.pptx</c>, whose
/// word count matches at 327 against 326 while its white title text is drawn on white paper
/// because the dark red band behind it is missing from all twelve pages. 60 of the 112 corpus
/// <c>pptx</c> decks state a <c>p:bgRef</c> somewhere.
/// </para>
/// <para>
/// <c>slide-background-ref.pptx</c> is built for the two cases that carry that reach. Its master
/// states <c>idx="1001"</c> against <c>bg1</c>, under a colour map sending <c>bg1</c> to
/// <c>dk1</c> — the arrangement that turns a missed reference into white-on-white rather than
/// into a slightly wrong shade — and its second layout overrides that with <c>idx="1002"</c>,
/// the same theme's gradient. Both expectations below are LibreOffice 24.2.7.2's own flat-ODF
/// export of that deck: <c>draw:fill="solid" draw:fill-color="#921f07"</c> for the first, and a
/// <c>draw:gradient</c> from <c>#1f6fc4</c> to <c>#124174</c> for the second.
/// </para>
/// </remarks>
public class SlideBackgroundReferenceTests
{
    private const string Deck = "slide-background-ref.pptx";

    [Fact]
    public void AMastersBackgroundReferenceReachesASlideThatStatesNone()
    {
        SolidPaint background = Slides()[0].Background.ShouldBeOfType<SolidPaint>();

        // dk1, not lt1: the master's p:clrMap sends bg1 to dk1, and a reader resolving the
        // reference's colour without the map paints this deck white on white.
        background.Colour.ShouldBe(Colour.FromRgb(0x921F07));
    }

    [Fact]
    public void ALayoutsBackgroundReferenceWinsOverTheMasters()
    {
        GradientPaint background = Slides()[1].Background.ShouldBeOfType<GradientPaint>();

        background.Stops[0].Colour.ShouldBe(Colour.FromRgb(0x1F6FC4));
        background.Stops[^1].Colour.ShouldBe(Colour.FromRgb(0x124174));
    }

    private static IReadOnlyList<LaidOutSlide> Slides()
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(Deck)));

        return ((SlidePages)((IPaginatedDocument)document).Layout()).Slides;
    }
}
