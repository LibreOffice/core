using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// An ODF document can ask for Word 2013's justification, and then a full justified line squeezes
/// its blanks to hold one more word.
/// </summary>
/// <remarks>
/// <para>
/// The DOCX spelling of this is <c>compatibilityMode</c> 15 and has been read since
/// <see cref="Text.Layout.JustificationShrink"/> was written; the ODF spelling is the document
/// setting <c>JustifyLinesWithShrinking</c>, which <c>SwXMLImport::SetConfigurationSettings</c>
/// sets straight through as a document property (<c>sw/source/filter/xml/xmlimp.cxx</c>:1238) and
/// which <c>SwTextPortion::Format_</c> reads as <c>bInteropSmartJustify</c>
/// (<c>sw/source/core/text/portxt.cxx</c>:543-545). Its default is <b>false</b> —
/// <c>mbJustifyLinesWithShrinking = false</c>,
/// <c>sw/source/core/inc/DocumentSettingManager.hxx</c>:180 — which makes it the one compatibility
/// flag the ODF reader reads whose absent case is <em>off</em>.
/// </para>
/// <para>
/// Reach on the converted corpus: <b>161 of the 338 <c>.odt</c> state it true</b>, and 51 of those
/// also hold a justified paragraph, which is what it takes for the flag to decide anything.
/// </para>
/// <para>
/// The two fixtures differ in that one config item and in nothing else. Ground truth is 26.2.4.2's
/// own PDF of each, read with <c>pdftotext -bbox</c>.
/// </para>
/// </remarks>
public sealed class OdtJustifyShrinkTests
{
    /// <summary>
    /// With the setting on, the sixteenth word stays on the first line.
    /// </summary>
    /// <remarks>
    /// 26.2.4.2 puts <c>SHRINKS</c> … <c>november</c> on one line at <c>yMin</c> 72.4 and starts the
    /// second at 84.0 with <c>THELAST</c>. The line's own blanks carry it: at natural widths it is
    /// wider than the 2 in right margin leaves, and squeezing each of its fourteen blanks to
    /// three-quarters recovers more than the word costs.
    /// </remarks>
    [Fact]
    public void AShrinkingLineHoldsTheWordThatDoesNotFitAtNaturalWidths()
    {
        Dictionary<string, DrawnWord> words = Words("odt-justify-shrink.fodt");

        words["november"].Baseline.ShouldBe(words["SHRINKS"].Baseline);
        words["THELAST"].Baseline.ShouldBeGreaterThan(words["SHRINKS"].Baseline);
    }

    /// <summary>
    /// And with it off the same line stops two words earlier, which is the control.
    /// </summary>
    /// <remarks>
    /// <para>
    /// 26.2.4.2 breaks the unflagged file after <c>mike</c> and sets the paragraph in three lines
    /// rather than two. Without the control the first assertion would pass on a reader that
    /// shrank every justified line whatever the document said, which is the failure mode a
    /// compatibility flag has.
    /// </para>
    /// <para>
    /// <b>The fixtures are LibreOffice-written files with one item changed, and that is not
    /// tidiness.</b> A minimal hand-authored flat ODF carrying this one config item is read
    /// correctly by XRay.Office and ignored outright by 26.2.4.2: forty such probes over twenty page
    /// widths gave byte-identical renderings with the item true and false. The same item inside a
    /// real file's full <c>ooo:configuration-settings</c> set decides the break at nine of
    /// thirty-six widths.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheSameLineWithoutTheSettingStopsTwoWordsEarlier()
    {
        Dictionary<string, DrawnWord> words = Words("odt-justify-noshrink.fodt");

        words["mike"].Baseline.ShouldBe(words["SHRINKS"].Baseline);
        words["november"].Baseline.ShouldBeGreaterThan(words["SHRINKS"].Baseline);
    }

    /// <summary>The first page's words, by their text.</summary>
    private static Dictionary<string, DrawnWord> Words(string fixture)
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(fixture)))
        {
            using IDocument document = new WordProcessingReader().Read(source);
            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(1);
            pages[0].Draw(sink);
        }

        return DrawnWords.On(sink.Pages[0]).ToDictionary(word => word.Text, word => word);
    }
}
