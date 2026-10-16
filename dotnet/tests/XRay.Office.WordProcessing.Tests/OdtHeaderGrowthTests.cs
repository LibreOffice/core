using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A header whose stated height is a floor is as tall as its own content.
/// </summary>
/// <remarks>
/// <para>
/// <c>fo:min-height</c> on <c>style:header-style</c> is <c>SwFrameSize::Minimum</c>, and
/// <c>SwHeadFootFrame::FormatPrt</c> takes <c>nHeight = lcl_CalcContentHeight(*this)</c> whenever
/// <c>!HasFixSize()</c>, raising it to the stated minimum only if it falls short
/// (<c>sw/source/core/layout/hffrm.cxx</c>:114-145). With <c>style:dynamic-spacing</c> set the
/// frame eats the gap below it rather than adding it, so the whole rule is
/// <c>total = max(min-height, content + (dynamic-spacing ? 0 : gap))</c> —
/// <see cref="OdfHeaderDynamicSpacingTests"/> pins the spacing half of it and this pins the
/// content half.
/// </para>
/// <para>
/// The content half could not be read while the geometry was resolved from the file alone, and the
/// reading that stood in for it was the floor. LibreOffice's exporter writes
/// <c>fo:min-height="0.0398in"</c> — two and a half points — for a running head of any height, so a
/// four-line header was given three points of room and everything below it sat a whole header too
/// high on every page. <b>Thirty of the converted corpus's failing <c>.odt</c> declare a dynamic
/// header or footer whose content outruns its stated minimum</b>; measuring it moved seven of them
/// onto the reference and none off it.
/// </para>
/// <para>
/// Ground truth is 26.2.4.2's own PDF of the fixture, read with <c>pdftotext -bbox</c>: HEADA at
/// 56.7288, HEADB at 70.1788, HEADC at 83.6288, HEADD at 97.0788, BODY at 110.5288; and on page 2
/// ROOF at 56.7288 with ROOFED at 141.7788.
/// </para>
/// </remarks>
public sealed class OdtHeaderGrowthTests
{
    /// <summary>Half a point, the tolerance the rest of this suite compares a baseline at.</summary>
    private const double Tolerance = 0.5;

    /// <summary>
    /// The body starts below the header's fourth line, not below its stated 0.4 cm.
    /// </summary>
    /// <remarks>
    /// Four 13.45 pt lines is 53.80 pt, and 26.2.4.2 puts BODY exactly that far below HEADA. The
    /// floor is 0.4 cm — 11.34 pt, less than one line — so a reader taking it as the height puts the
    /// body 42.46 pt too high, which on this page is three lines and on a real document is a page
    /// every dozen.
    /// </remarks>
    [Fact]
    public void TheBodyStartsBelowTheHeadersOwnContent()
    {
        Dictionary<string, DrawnWord> page = Pages()[0];

        (page["BODY"].Baseline - page["HEADA"].Baseline).ShouldBe(110.5288 - 56.7288, Tolerance);
    }

    /// <summary>
    /// And a floor larger than the content still binds, which is the control.
    /// </summary>
    /// <remarks>
    /// The second master's header holds one line under a 3 cm floor, and 26.2.4.2 puts its body
    /// 85.05 pt below the header's baseline — the floor exactly, not the 13.45 pt the line needs.
    /// Without this a reader that simply took the content height would pass the first assertion and
    /// be wrong on every header sized deliberately.
    /// </remarks>
    [Fact]
    public void AFloorLargerThanTheContentStillBinds()
    {
        Dictionary<string, DrawnWord> page = Pages()[1];

        (page["ROOFED"].Baseline - page["ROOF"].Baseline).ShouldBe(141.7788 - 56.7288, Tolerance);
    }

    /// <summary>Every page's words, by their text.</summary>
    private static List<Dictionary<string, DrawnWord>> Pages()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require("odt-header-grows.fodt")))
        {
            using IDocument document = new WordProcessingReader().Read(source);
            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(2);
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return [.. sink.Pages.Select(
            page => DrawnWords.On(page).ToDictionary(word => word.Text, word => word))];
    }
}
