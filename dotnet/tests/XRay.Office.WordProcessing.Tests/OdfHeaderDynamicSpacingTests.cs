using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A header's <c>fo:margin-bottom</c> is a gap it adds to itself or one it eats, and
/// <c>style:dynamic-spacing</c> says which.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice maps the attribute to <c>SwHeaderAndFooterEatSpacingItem</c>
/// (<c>sw/source/core/bastyp/init.cxx</c>:434). With it set, <c>SwHeadFootFrame::FormatPrt</c>
/// starts the header's print area at <c>fo:min-height</c> less both spacings and then gives back
/// out of the gap exactly what the content overruns that by
/// (<c>sw/source/core/layout/hffrm.cxx</c>:116-170), so the frame's total comes to
/// </para>
/// <code>
/// total = max(min-height, content + (dynamic-spacing ? 0 : gap))
/// </code>
/// <para>
/// The declared minimum therefore <em>includes</em> the gap, and a header whose content fits
/// occupies the minimum and nothing more. Adding the gap on top instead pushes the body down by
/// it on every page of the document — which is silent, because every page is wrong the same way
/// and the text that falls off the bottom simply starts another page.
/// </para>
/// <para>
/// Reach on the converted corpus: 152 of the 338 <c>.odt</c> declare a dynamic-height header or
/// footer with the flag set and a gap worth more than half a point, and the gaps run to 89 pt.
/// </para>
/// </remarks>
public sealed class OdfHeaderDynamicSpacingTests
{
    /// <summary>1 cm, in points — the fixture's <c>fo:min-height</c> and its gap alike.</summary>
    private const double Centimetre = 720.0 / 25.4;

    /// <summary>
    /// The header with the flag occupies its declared minimum rather than the minimum plus its gap.
    /// </summary>
    /// <remarks>
    /// Measured as the drop from the header's own baseline to the first body baseline rather than
    /// from the top of the page, so the assertion says how much room the header took and does not
    /// also depend on where a Carlito ascender starts. 26.2.4.2's own PDF of the fixture, read with
    /// <c>pdftotext -bbox</c>, puts both header lines at yMin 56.7289 — the 2 cm top margin — and
    /// page 1's first body line at 85.0789, a drop of 28.35 pt, which is 1 cm to a hundredth.
    /// </remarks>
    [Fact]
    public void TheFlaggedHeaderOccupiesItsDeclaredMinimum()
    {
        (double eat, _) = HeaderDrops();

        // max(1 cm, one 11 pt line) with the gap absorbed = 1 cm.
        eat.ShouldBe(Centimetre, 0.5);
    }

    /// <summary>
    /// And a header that turns the flag off adds the gap to its content, which is 41.80 pt here.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The unflagged branch of the same formula: <c>max(min-height, content + gap)</c> — 1 cm of
    /// floor against one 11 pt line plus 1 cm of gap — and 26.2.4.2 drops 41.80 pt on this
    /// fixture's second page. It is asserted as a number rather than as a difference from the
    /// flagged page because both halves are now exact.
    /// </para>
    /// <para>
    /// <b>It was not, until the content could be measured.</b> This assertion used to read
    /// <c>add - eat &gt; 1 cm</c>, and its own remarks said the exact answer needed a content
    /// height that could not be had before the page the header sits on was known — so the older
    /// <c>minimum + gap</c> stood and the tree drew 56.70 pt against the reference's 41.80, one
    /// header line too many. The content is measured in <c>OdtWordDocument.GrownTo</c>, which
    /// re-reads the geometry once the walk that produces the header's blocks has run, and both
    /// branches then land on the reference. The old assertion could not survive that: closing the
    /// gap narrows the two readings from 28.35 pt apart to 13.45.
    /// </para>
    /// </remarks>
    [Fact]
    public void AHeaderThatTurnsTheFlagOffAddsItsGapToItsContent()
    {
        (double eat, double add) = HeaderDrops();

        // 26.2.4.2: 41.80 pt, which is one 13.45 pt line plus the 1 cm gap.
        add.ShouldBe(41.80, 0.5);

        // And the flag is still what separates them, which is the whole point of the pair.
        (add - eat).ShouldBeGreaterThan(1.0);
    }

    /// <summary>
    /// The drop from the header's baseline to the first body baseline, on each of the two pages.
    /// </summary>
    private static (double Eat, double Add) HeaderDrops()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source =
               DocumentSource.FromFile(Corpus.Require("header-dynamic-spacing.fodt")))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(2);
            pages[0].Draw(sink);
            pages[1].Draw(sink);
        }

        return (Drop(sink.Pages[0], "HEADEREAT", "EATFIRST"),
                Drop(sink.Pages[1], "HEADERADD", "ADDFIRST"));
    }

    private static double Drop(DrawnPage page, string head, string body)
    {
        List<DrawnWord> words = DrawnWords.On(page);
        DrawnWord header = words.Single(word => word.Text == head);
        DrawnWord first = words.Single(word => word.Text == body);
        return first.Baseline - header.Baseline;
    }
}
