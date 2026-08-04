using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Checks that a paragraph's own <c>w:tabs</c> adds to its style's rather than replacing it.
/// </summary>
/// <remarks>
/// <para>
/// <c>DomainMapper</c> seeds the working tab-stop vector from the paragraph style sheet on
/// <c>LN_CT_PPrBase_tabs</c> and then folds each stated stop in through <c>IncorporateTabStop</c>,
/// which replaces by equal position and appends otherwise
/// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx:2604-2620</c>,
/// <c>DomainMapper_Impl.cxx:1485-1498</c>). <c>w:val="clear"</c> removes the inherited stop at its
/// position.
/// </para>
/// <para>
/// The stop this costs is the right stop with a dot leader that every table-of-contents style
/// carries. Word writes a direct <c>&lt;w:tab w:val="left"/&gt;</c> on a TOC entry to place its
/// heading number, so under the replacing rule such a paragraph lost its leader and its right
/// margin together — no dots, no right alignment, and the entry wrapping onto a second line.
/// </para>
/// <para>
/// The fixture's four paragraphs are the four cases, and LibreOffice's own PDF of it distinguishes
/// all four: <c>Inherited.....9</c> with dots to the right margin; <c>1.1  Added.....9</c>, where
/// the added left stop places the number and the style's leader still reaches the margin;
/// <c>Cleared   9</c> with the stop removed and the default interval taking over; and
/// <c>Replaced ... 9</c> aligned left at the same position with no leader.
/// </para>
/// </remarks>
public sealed class TabStopInheritanceTests
{
    private const string Fixture = "tab-stops-inherited.docx";

    [Fact]
    public void AParagraphStatingNoTabsTakesItsStylesStops()
    {
        TabStop stop = StopsOf(0).ShouldHaveSingleItem();

        stop.Position.Twips.ShouldBe(9000);
        stop.Alignment.ShouldBe(TabAlignment.Right);
        stop.Leader.ShouldBe('.');
    }

    [Fact]
    public void ADirectTabAddsToTheStylesStopsRatherThanReplacingThem()
    {
        // The table-of-contents case, and the one the whole change is about: the paragraph states a
        // left stop of its own and must still hold the style's dot leader at the right margin.
        List<TabStop> stops = StopsOf(1);

        stops.Count.ShouldBe(2, "the style's right stop and the paragraph's own left one");
        stops[0].Position.Twips.ShouldBe(1200);
        stops[0].Alignment.ShouldBe(TabAlignment.Left);
        stops[0].Leader.ShouldBe('\0');

        stops[1].Position.Twips.ShouldBe(9000);
        stops[1].Alignment.ShouldBe(TabAlignment.Right);
        stops[1].Leader.ShouldBe('.');
    }

    [Fact]
    public void AClearedStopRemovesTheStylesOneAndAddsNothing()
    {
        StopsOf(2).ShouldBeEmpty();
    }

    [Fact]
    public void AStopAtAnInheritedPositionReplacesIt()
    {
        TabStop stop = StopsOf(3).ShouldHaveSingleItem();

        stop.Position.Twips.ShouldBe(9000);
        stop.Alignment.ShouldBe(TabAlignment.Left);
        stop.Leader.ShouldBe('\0');
    }

    /// <summary>The tab stops of the fixture's <paramref name="index"/>th body paragraph.</summary>
    private static List<TabStop> StopsOf(int index)
    {
        string path = Corpus.Require(Fixture);

        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        List<PageParagraph> paragraphs = [.. pages.Paragraphs.Where(p => p.Text.Length > 0)];
        paragraphs.Count.ShouldBe(4, $"{Fixture}: the document's paragraphs");

        return [.. paragraphs[index].Format.TabStops];
    }
}
