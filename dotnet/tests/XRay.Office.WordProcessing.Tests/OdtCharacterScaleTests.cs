using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// <c>style:text-scale</c>: the ODF spelling of the character width, which nothing read.
/// </summary>
/// <remarks>
/// <para>
/// The same item as <c>w:rPr/w:w</c> and <c>\charscalex</c>. <c>xmloff</c> maps it onto
/// <c>PROP_CharScaleWidth</c> as an <c>XML_TYPE_PERCENT16</c>
/// (<c>xmloff/source/text/txtprmap.cxx</c>:253 and :641) — the property the DOCX importer fills
/// too — so <c>TextWidthScale</c>'s twip truncation applies here unchanged and is not re-derived.
/// </para>
/// <para>
/// <strong>Measured at 26.2.4.2</strong> on five one-attribute flat-ODF arms, the drawn width read
/// out of a right-aligned line's own origin rather than from glyph boxes, which are quantised to
/// whole thousandths of an em: the reference draws <c>130%</c> at <b>1.29981</b> of the unscaled
/// arm, <c>60%</c> at <b>0.59974</b> and <c>99%</c> at <b>0.98665</b> — nearer the truncated
/// 0.98750 than the stated 0.99. This tree answers 1.30000, 0.60000 and 0.98750, which agrees to
/// 0.02, 0.04 and 0.09 per cent. <c>probes/odtscale-r148/</c>.
/// </para>
/// <para>
/// <strong>The specification's namespace is the one written.</strong> Over the 337 converted
/// <c>.odt</c> the attribute appears as <c>style:text-scale</c> and not once as
/// <c>loext:text-scale</c> — worth asserting nowhere but worth recording here, because five other
/// ODF attributes in this reader are the other way round and each cost a round to find.
/// </para>
/// </remarks>
public sealed class OdtCharacterScaleTests
{
    /// <summary>
    /// Every arm of the fixture draws the same word, so the widths are directly comparable and the
    /// ratio is exact rather than a per-character proxy. The arms are in paragraph order: absent,
    /// 100, 99, 60, 130, then an unscaled paragraph holding a 60 per cent span.
    /// </summary>
    private const string Fixture = "odt-text-scale.fodt";

    private const int Absent = 0;
    private const int Hundred = 1;
    private const int NinetyNine = 2;
    private const int Sixty = 3;
    private const int OneThirty = 4;
    private const int UnscaledRunOfTheLastParagraph = 5;
    private const int ScaledSpan = 6;

    /// <summary>A stated scale squeezes the run and stretches it.</summary>
    [Theory]
    [InlineData(Sixty, 0.60)]
    [InlineData(OneThirty, 1.30)]
    public void AStatedScaleSqueezesOrStretchesTheRun(int arm, double expected)
        => Ratio(arm, Absent).ShouldBe(expected, 0.0005);

    /// <summary>
    /// 99 per cent is not 0.99 here either, because the item is the one VCL builds the face from.
    /// </summary>
    /// <remarks>
    /// A 12 pt run is 240 twips tall and <c>trunc(240 x 99 / 100)</c> is 237, so the face is 237/240
    /// wide. <c>CharacterScaleTests</c> pins the arithmetic; what this asserts is that the ODF
    /// reader reaches it at all rather than passing the percentage through.
    /// </remarks>
    [Fact]
    public void NinetyNinePerCentLandsOnTheTwipBelowIt()
        => Ratio(NinetyNine, Absent).ShouldBe(237.0 / 240.0, 0.0005);

    /// <summary>A stated 100 per cent costs exactly nothing.</summary>
    [Fact]
    public void AnAbsentScaleAndAHundredPerCentAreTheSame()
        => Ratio(Hundred, Absent).ShouldBe(1.0, 1e-9);

    /// <summary>A scaled span inside an unscaled paragraph survives the uniform-paragraph shortcut.</summary>
    /// <remarks>
    /// The reader drops the run list for a paragraph whose runs all agree with the paragraph's own
    /// format, and a run that measures differently has to defeat that or it is measured at the
    /// paragraph's width — the ODF twin of the duty <c>CharacterScaleTests</c> asserts for DOCX.
    /// The two words of that paragraph are the same word, so their ratio is the span's scale.
    /// </remarks>
    [Fact]
    public void AScaledSpanInsideAnUnscaledParagraphSurvivesTheUniformShortcut()
    {
        Ratio(UnscaledRunOfTheLastParagraph, Absent).ShouldBe(1.0, 0.0005);
        Ratio(ScaledSpan, UnscaledRunOfTheLastParagraph).ShouldBe(0.60, 0.0005);
    }

    private static double Ratio(int arm, int against)
    {
        List<DrawnWord> words = Words();
        return Width(words[arm]) / Width(words[against]);
    }

    private static double Width(DrawnWord word) => word.Right - word.Left;

    private static List<DrawnWord> Words()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(Fixture)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(1);
            pages[0].Draw(sink);
        }

        List<DrawnWord> words = DrawnWords.On(sink.Pages[0]);
        words.Count.ShouldBe(7, "six paragraphs, the last of which holds two words");
        return words;
    }
}
