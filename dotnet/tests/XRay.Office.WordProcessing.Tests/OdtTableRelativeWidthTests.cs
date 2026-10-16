using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A table's width and its columns can both be stated as proportions, and both are read.
/// </summary>
/// <remarks>
/// <para>
/// <c>style:rel-width</c> on <c>style:table-properties</c> maps to <c>RES_FRM_SIZE</c>'s
/// <c>MID_FRMSIZE_REL_WIDTH</c> (<c>sw/source/filter/xml/xmlitemm.cxx</c>:47) and becomes
/// <c>SwFormatFrameSize::SetWidthPercent</c>, clamped to 1..100
/// (<c>sw/source/filter/xml/xmlimpit.cxx</c>:936-949). <c>style:rel-column-width="2677*"</c> is the
/// column's own proportion: <c>xmlimpit.cxx</c>:971-986 stores the number and marks the size type
/// <c>Variable</c>, and <c>SwXMLTableColContext</c> passes that to <c>InsertColumn</c> as
/// <c>bRelWidth = true</c> (<c>sw/source/filter/xml/xmltbli.cxx</c>:694-714).
/// </para>
/// <para>
/// The percentage counts only for a table with a real horizontal orientation.
/// <c>SwXMLTableContext::MakeTable_</c> reads the size in the <c>default:</c> arm of its
/// orientation switch alone; under <c>FULL</c> and <c>NONE</c> — <c>table:align="margins"</c> or no
/// <c>table:align</c> at all — it sets <c>m_nWidth = MAX_WIDTH</c> and never looks at it
/// (<c>xmltbli.cxx</c>:2540-2582). That is the same condition the reader already applied to a
/// stated <c>style:width</c>.
/// </para>
/// <para>
/// Reach on the converted corpus: <b>29 of the 338 <c>.odt</c> hold an oriented
/// <c>style:rel-width</c> table</b>, and <b>2399 columns in 24 documents state
/// <c>style:rel-column-width</c> and no <c>style:column-width</c></b> — 1069 of them in
/// <c>FAA 2025-26 Holdover Tables.odt</c>, whose columns all came out equal without this and
/// which paginated 249 pages against the reference's 167.
/// </para>
/// <para>
/// Ground truth is 26.2.4.2's own PDF of the fixture, read with <c>pdftotext -bbox</c>: RELA at
/// <c>xMin</c> 177.25, RELB at 237.50, BETWEEN and FULLA at 56.80, FULLB at 177.25.
/// </para>
/// </remarks>
public sealed class OdtTableRelativeWidthTests
{
    /// <summary>Half a point, the tolerance the rest of this suite compares a position at.</summary>
    private const double Tolerance = 0.5;

    /// <summary>
    /// A table stating half the width and a centred alignment is half as wide and centred.
    /// </summary>
    /// <remarks>
    /// The fixture's text area is 481.89 pt wide, so half of it is 240.94 and the left edge sits
    /// 120.47 pt in from the body's. 26.2.4.2 draws RELA 120.45 pt right of BETWEEN, which is a
    /// full-width paragraph on the same page.
    /// </remarks>
    [Fact]
    public void ATableStatingAPercentageIsThatFractionOfTheAreaAndCentredInIt()
    {
        Dictionary<string, DrawnWord> words = Words();

        (words["RELA"].Left - words["BETWEEN"].Left).ShouldBe(177.25 - 56.80, Tolerance);
    }

    /// <summary>
    /// And its columns are in the proportions their own styles state.
    /// </summary>
    /// <remarks>
    /// <c>1000*</c> against <c>3000*</c> over a 240.94 pt table is 60.24 and 180.70. 26.2.4.2 puts
    /// RELB 60.25 pt right of RELA. Without <c>style:rel-column-width</c> both columns arrive as
    /// <c>MINLAY</c> and come out equal, which would put RELB 120.47 pt along.
    /// </remarks>
    [Fact]
    public void ItsColumnsAreInTheProportionsTheyState()
    {
        Dictionary<string, DrawnWord> words = Words();

        (words["RELB"].Left - words["RELA"].Left).ShouldBe(237.50 - 177.25, Tolerance);
    }

    /// <summary>
    /// The same percentage on a table aligned to the margins is ignored outright.
    /// </summary>
    /// <remarks>
    /// This is the control on the orientation gate, and it is not a nicety: <c>table:align</c> is
    /// absent far more often than it is present, and honouring the percentage there would narrow
    /// every such table by whatever the file happened to say. The second table's columns keep their
    /// 1:3 proportions over the <em>full</em> 481.89 pt, so FULLB sits 120.45 pt along — exactly
    /// where the first table's left <em>edge</em> is, which is a coincidence of the numbers and the
    /// reason both are asserted.
    /// </remarks>
    [Fact]
    public void ThePercentageIsIgnoredWhenTheTableIsAlignedToTheMargins()
    {
        Dictionary<string, DrawnWord> words = Words();

        words["FULLA"].Left.ShouldBe(words["BETWEEN"].Left, Tolerance);
        (words["FULLB"].Left - words["FULLA"].Left).ShouldBe(177.25 - 56.80, Tolerance);
    }

    /// <summary>The first page's words, by their text.</summary>
    private static Dictionary<string, DrawnWord> Words()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require("odt-table-relative.fodt")))
        {
            using IDocument document = new WordProcessingReader().Read(source);
            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(1);
            pages[0].Draw(sink);
        }

        return DrawnWords.On(sink.Pages[0]).ToDictionary(word => word.Text, word => word);
    }
}
