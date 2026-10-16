using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A <c>draw:frame</c> may state <c>svg:width</c> and no <c>svg:height</c>, and it still holds
/// its text.
/// </summary>
/// <remarks>
/// <para>
/// Both lengths are optional (ODF 1.3 §10.4.2) and Writer leaves the height off a frame that
/// grows to its content — a text box or a framed table after "autofit height". What it writes
/// instead is the floor, on the box: <c>&lt;draw:text-box fo:min-height="0in"&gt;</c>. Requiring
/// both lengths to place a frame therefore dropped the frame <em>and everything inside it</em>,
/// and said nothing: a page with no frame on it looks like a page whose author put none there.
/// </para>
/// <para>
/// Reach on the converted corpus: 49 of the 338 <c>.odt</c> hold 58 such frames carrying
/// <strong>50 942 alphanumeric characters</strong> between them, and in ten of those the frame is
/// the page — <c>020_Project_Timeline_Template_Modern_Theme</c> drew 0 characters against the
/// reference's 316, <c>part-147_approval list_20230119</c> 114 against 3570.
/// </para>
/// <para>
/// <strong>Growing the frame is a separate rule and it is now implemented</strong> — see
/// <see cref="OdtFrameGrowthTests"/> and <see cref="Layout.PageFrame.GrowsToContent"/>. It did not
/// need <c>FrameLayout</c>'s order inverted, which is what the round that placed the frame
/// supposed: <c>FrameLayout.Place</c> is a pure function of the frame's stated size and the page
/// and anchor geometry, and the content's own layout needs only the frame's <em>width</em>, which
/// the file states — so the height is measured in a pass that runs first.
/// </para>
/// <para>
/// This fixture is <em>not</em> a growth witness, and deliberately so: its <c>fr1</c> is an
/// automatic graphic style with no parent, which LibreOffice imports as a drawing shape rather
/// than as a Writer text frame (<c>xmloff/source/text/XMLTextFrameContext.cxx</c>:1374-1394,
/// <em>"#i51726#"</em>, and :1500-1507), and a shape fits itself to its text by a different rule.
/// What it pins is that such a frame is read and placed at all.
/// </para>
/// </remarks>
public sealed class OdfFrameAutoHeightTests
{
    /// <summary>The frame's text is drawn, and where the file puts the frame.</summary>
    /// <remarks>
    /// 26.2.4.2's own PDF of the fixture puts PLAIN at yMin 56.7289 — the 2 cm top margin — and
    /// BOXED at 85.5589, which is that plus the frame's <c>svg:y</c> of 1 cm. A reading that lost
    /// the frame keeps PLAIN and looks like an ordinary one-word document, which is why the
    /// assertion names both words rather than only the one that matters.
    /// </remarks>
    [Fact]
    public void AFrameWithNoStatedHeightStillDrawsItsText()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source =
               DocumentSource.FromFile(Corpus.Require("odt-frame-auto-height.fodt")))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(1);
            pages[0].Draw(sink);
        }

        Dictionary<string, DrawnWord> words =
            DrawnWords.On(sink.Pages[0])
                      .Where(word => word.Text is "PLAIN" or "BOXED")
                      .ToDictionary(word => word.Text, word => word);

        words.Keys.Order(StringComparer.Ordinal).ShouldBe(["BOXED", "PLAIN"]);

        // 1 cm = 28.35 pt of svg:y separates the two baselines, both being the same 11 pt line.
        (words["BOXED"].Baseline - words["PLAIN"].Baseline).ShouldBe(720.0 / 25.4, 0.5);
    }
}
