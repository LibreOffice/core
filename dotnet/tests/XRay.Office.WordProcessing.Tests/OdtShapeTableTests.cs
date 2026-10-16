using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A table inside a drawing shape's text is spelled <c>loext:table</c>, and it is a table.
/// </summary>
/// <remarks>
/// <para>
/// ODF 1.3 does not allow a table everywhere LibreOffice can put one, so LibreOffice writes such a
/// table in its own extension namespace and reads the two spellings as one thing:
/// <c>XMLTextImportHelper::CreateTextChildContext</c> falls
/// <c>case XML_ELEMENT(TABLE, XML_TABLE): case XML_ELEMENT(LO_EXT, XML_TABLE):</c> through to the
/// same <c>CreateTableChildContext</c> (<c>xmloff/source/text/txtimp.cxx</c>:1787-1795). The
/// <em>attributes</em> stay in <c>table:</c>, so only the element names move — which is why
/// <c>OdfNamespaces.IsTable</c> asks about a namespace rather than about a name.
/// </para>
/// <para>
/// It is the fourth instance of the rule this project already records for attributes — <em>an ODF
/// name LibreOffice's own exporter writes is very often not in the namespace the specification puts
/// it in</em> — and the first where the name in question is an <em>element</em>. Reach: <b>seven of
/// the 338 converted <c>.odt</c> hold one</b>, and
/// <c>043_Visual_Product_Roadmap_Template_Customizable_Format</c> drew 41 of its 1099 characters
/// without it, because every table of its roadmap diagram is a <c>loext:table</c>.
/// </para>
/// <para>
/// Ground truth is 26.2.4.2's own PDF of the fixture, read with <c>pdftotext -bbox</c>: ANCHOR at
/// 56.80, 56.7288, LOEXTA at 85.15, 113.4288 and LOEXTB at 170.20, 113.4288.
/// </para>
/// </remarks>
public sealed class OdtShapeTableTests
{
    /// <summary>Half a point, the tolerance the rest of this suite compares a position at.</summary>
    private const double Tolerance = 0.5;

    /// <summary>The cells of a <c>loext:table</c> inside a shape are drawn.</summary>
    [Fact]
    public void ALoextTableInsideAShapeIsDrawn()
    {
        Dictionary<string, DrawnWord> words = Words();

        words.Keys.ShouldContain("LOEXTA");
        words.Keys.ShouldContain("LOEXTB");
    }

    /// <summary>
    /// And it is laid out as a grid rather than as loose paragraphs.
    /// </summary>
    /// <remarks>
    /// The two cells share a baseline and sit one 3 cm column apart. Reading the element as unknown
    /// and walking into it anyway would stack the two cells' paragraphs as separate lines, which is
    /// the failure this distinguishes from simple absence.
    /// </remarks>
    [Fact]
    public void ItsCellsAreSideBySideOnOneRow()
    {
        Dictionary<string, DrawnWord> words = Words();

        words["LOEXTB"].Baseline.ShouldBe(words["LOEXTA"].Baseline, Tolerance);
        (words["LOEXTB"].Left - words["LOEXTA"].Left).ShouldBe(170.20 - 85.15, Tolerance);
    }

    /// <summary>
    /// Extraction finds it too, which is a different walk and was equally blind to the spelling.
    /// </summary>
    /// <remarks>
    /// Making something draw does not make it extract: the two paths share no code, and
    /// <c>OdfContentReader</c> dispatched a table on the <c>table:</c> namespace exactly as the
    /// layout walk did.
    /// </remarks>
    [Fact]
    public void ExtractionFindsItAsWell()
    {
        using DocumentSource source =
            DocumentSource.FromFile(Corpus.Require("odt-shape-loext-table.fodt"));
        using IDocument document = new WordProcessingReader().Read(source);

        string text = document.Content.GetText();

        text.ShouldContain("LOEXTA");
        text.ShouldContain("LOEXTB");
    }

    /// <summary>The first page's words, by their text.</summary>
    private static Dictionary<string, DrawnWord> Words()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source =
               DocumentSource.FromFile(Corpus.Require("odt-shape-loext-table.fodt")))
        {
            using IDocument document = new WordProcessingReader().Read(source);
            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(1);
            pages[0].Draw(sink);
        }

        return DrawnWords.On(sink.Pages[0]).ToDictionary(word => word.Text, word => word);
    }
}
