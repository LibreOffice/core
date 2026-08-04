using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Whether two consecutive paragraphs' spacings add or the larger one wins, per format.
/// </summary>
/// <remarks>
/// <para>
/// Writer's <c>PARA_SPACE_MAX</c>, whose name says the opposite of what it does: set, the space above a
/// paragraph is the previous one's space-after <em>plus</em> its own space-before
/// (<c>SwFlowFrame::CalcUpperSpace</c>, <c>sw/source/core/layout/flowfrm.cxx</c>:1652); clear, it is the
/// larger of the two. It is a vertical-budget rule, so getting it wrong moves every paragraph after the
/// first boundary and eventually every page.
/// </para>
/// <para>
/// The four readers do not agree, and each disagreement is a property of the format rather than an
/// oversight:
/// </para>
/// <list type="bullet">
///   <item><description>
///   <b>DOCX collapses</b>, because <c>DomainMapper_Impl::ApplySettingsTable</c> writes the setting from
///   <c>w:doNotUseHTMLParagraphAutoSpacing</c> (<c>DomainMapper_Impl.cxx</c>:10179) and Word's exporter
///   writes that element only when the flag is on — <b>unless the package has no settings part at all</b>,
///   when that method returns at its first line (:10124) and the document keeps the application's
///   configured <c>AddSpacing</c>, whose shipped default is <c>true</c>.
///   </description></item>
///   <item><description>
///   <b>RTF adds</b>, because <c>SettingsTable</c>'s constructor sets the flag for an RTF import and
///   leaves it clear for OOXML (<c>SettingsTable.cxx</c>:119) — and <c>\htmautsp</c>, which reads
///   backwards from its name, asks for the collapsing behaviour by clearing it again
///   (<c>rtfdispatchflag.cxx</c>:1354).
///   </description></item>
///   <item><description>
///   <b>ODF follows <c>AddParaTableSpacing</c></b> in <c>office:settings</c>, adding when the file states
///   none. A document converted from a Word file carries <c>false</c> and collapses.
///   </description></item>
///   <item><description>
///   <b>DOC follows its <c>Dop</c></b>: <c>ww8par.cxx</c>:1946 sets the setting from
///   <c>fDontUseHTMLAutoSpacing</c>, which <c>WW8Dop</c> defaults to true (<c>ww8scan.cxx</c>:7623) and
///   reads from the second compatibility word when the <c>Dop</c> is long enough to hold it. Already
///   right in both directions, and covered by <see cref="DocAutoSpacingTests"/>.
///   </description></item>
/// </list>
/// <para>
/// Each fixture is eight paragraphs each carrying 12 pt of space-before and 8 pt of space-after, on 12 pt
/// exact lines so nothing else can move a baseline. Seven boundaries, each 12 pt of pitch plus a gap that
/// is 12 pt when the spacings collapse and 20 pt when they add. Measured against LibreOffice 24.2.7.2 on
/// every one of them: <c>paragraph-spacing-no-settings.docx</c> at 32.00 pt of pitch and the collapsing
/// fixtures at 24.00 pt, ours and the reference agreeing to the hundredth of a point.
/// </para>
/// </remarks>
public sealed class ParagraphSpacingAccumulationTests
{
    /// <summary>A tenth of a point, two twips, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.1;

    /// <summary>The 12 pt exact line the fixtures are set on, plus the larger of the two spacings.</summary>
    private const double CollapsedPitch = 24.0;

    /// <summary>The same line, plus both spacings.</summary>
    private const double AddedPitch = 32.0;

    /// <summary>
    /// A DOCX with no <c>word/settings.xml</c> adds, where every other DOCX takes the larger.
    /// </summary>
    /// <remarks>
    /// The narrow case, and the one that makes a synthetic fixture answer this question backwards. The
    /// same eight paragraphs in a package that <em>has</em> a settings part collapse at 24.00 pt in
    /// LibreOffice; only removing the part entirely gives 32.00 pt.
    /// </remarks>
    [Fact]
    public void ADocxWithNoSettingsPartAddsBothSpacings()
        => PitchesOf("paragraph-spacing-no-settings.docx")
            .ShouldAllBe(pitch => Math.Abs(pitch - AddedPitch) <= TolerancePoints);

    /// <summary>
    /// A DOCX with a settings part takes the larger, even when the part says nothing at all.
    /// </summary>
    /// <remarks>
    /// The counter-example, and the reason it is a test rather than a remark: this is the case 130 of the
    /// corpus's 134 DOCX are in, so a change that made DOCX add unconditionally would move all of them
    /// the wrong way while making the fixture above pass.
    /// </remarks>
    [Fact]
    public void ADocxWithAnEmptySettingsPartTakesTheLarger()
        => PitchesOf("paragraph-spacing-settings.docx")
            .ShouldAllBe(pitch => Math.Abs(pitch - CollapsedPitch) <= TolerancePoints);

    /// <summary>An RTF asking for HTML auto-spacing takes the larger, against RTF's own default.</summary>
    [Fact]
    public void AnRtfWithHtmlAutoSpacingTakesTheLarger()
        => PitchesOf("paragraph-spacing-collapsed.rtf")
            .ShouldAllBe(pitch => Math.Abs(pitch - CollapsedPitch) <= TolerancePoints);

    /// <summary>
    /// An ODF document whose settings clear <c>AddParaTableSpacing</c> takes the larger, in both
    /// containers.
    /// </summary>
    [Theory]
    [InlineData("paragraph-spacing-collapsed.odt")]
    [InlineData("paragraph-spacing-collapsed.fodt")]
    public void AnOdfDocumentThatClearsAddParaTableSpacingTakesTheLarger(string fileName)
        => PitchesOf(fileName)
            .ShouldAllBe(pitch => Math.Abs(pitch - CollapsedPitch) <= TolerancePoints);

    /// <summary>
    /// The distance from each baseline to the next, in points, for a fixture's one page.
    /// </summary>
    /// <remarks>
    /// Asserting the counts as well as the values, because a fixture that stopped producing eight
    /// paragraphs would satisfy an all-of assertion over an empty sequence while proving nothing.
    /// </remarks>
    private static double[] PitchesOf(string fileName)
    {
        using IDocument document = new WordProcessingReader()
            .Read(DocumentSource.FromFile(Corpus.Require(fileName)));

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        pages.Count.ShouldBe(1, $"{fileName}: the fixture is one page");

        double[] baselines = [.. pages.Pages[0].Lines.Select(line => line.Baseline.Points)];
        baselines.Length.ShouldBe(8, $"{fileName}: eight paragraphs, one line each");

        return [.. baselines.Zip(baselines.Skip(1), (above, below) => below - above)];
    }
}
