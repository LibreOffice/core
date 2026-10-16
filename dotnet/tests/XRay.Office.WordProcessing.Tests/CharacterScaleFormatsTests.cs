using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// The character width in the two readers that were left: <c>\charscalex</c> and <c>sprmCCharScale</c>.
/// </summary>
/// <remarks>
/// <para>
/// One item — <c>RES_CHRATR_SCALEW</c> / <c>PROP_CharScaleWidth</c> — under four names:
/// <c>w:rPr/w:w</c>, <c>style:text-scale</c>, <c>\charscalex</c> and <c>sprmCCharScale</c>. VCL
/// applies it by setting the font's width away from its height, and the width it builds the face at
/// is a whole number of twips, so <see cref="XRay.Office.Text.Layout.TextWidthScale"/>'s truncation
/// applies to all four and is not re-derived here. <c>CharacterScaleTests</c> pins that arithmetic;
/// these two assert that these readers reach it.
/// </para>
/// <para>
/// [src] RTF: <c>{ "charscalex", { VALUE, CHARSCALEX, <b>100</b> } }</c>
/// (<c>sw/source/writerfilter/rtftok/rtftokenizer.cxx</c>:253) dispatched to
/// <c>NS_ooxml::LN_EG_RPrBase_w</c> (<c>rtfdispatchvalue.cxx</c>:193) — the very sprm <c>w:w</c>
/// uses, so out of range resets to 100 in the same place, and a <em>bare</em> <c>\charscalex</c> is
/// 100 rather than 0, which is the opposite of <c>\kerning</c> beside it. WW8:
/// <c>sprmChr&lt;0x52, 0, SPRA::operand_2b_2&gt;</c> (<c>sprmids.hxx</c>:335) handled by
/// <c>SwWW8ImplReader::Read_ScaleWidth</c> (<c>ww8par6.cxx</c>:4985-4997), which replaces anything
/// outside 1..600 with 100.
/// </para>
/// <para>
/// [bin] <c>words-char-scale.rtf</c> and its reference-converted <c>.doc</c> twin, rendered through
/// 26.2.4.2: both draw <b>1.00000, 1.00000, 0.98665, 0.59974, 1.29981</b> and the scaled span at
/// <b>0.59974</b> — identical to each other and to the ODF arms of
/// <see cref="OdtCharacterScaleTests"/> to five places, because it is one item in VCL. The sprm was
/// verified present in the <c>.doc</c>'s CHPX before it was measured; a fixture converted and not
/// checked measures nothing and reports agreement. <c>probes/charscale-r150/</c>.
/// </para>
/// </remarks>
public sealed class CharacterScaleFormatsTests
{
    private const int Absent = 0;
    private const int Hundred = 1;
    private const int NinetyNine = 2;
    private const int Sixty = 3;
    private const int OneThirty = 4;
    private const int UnscaledRunOfTheLastParagraph = 5;
    private const int ScaledSpan = 6;

    /// <summary>A stated width squeezes the run and stretches it, in both formats.</summary>
    [Theory]
    [InlineData("words-char-scale.rtf", Sixty, 0.60)]
    [InlineData("words-char-scale.rtf", OneThirty, 1.30)]
    [InlineData("words-char-scale.doc", Sixty, 0.60)]
    [InlineData("words-char-scale.doc", OneThirty, 1.30)]
    public void AStatedWidthSqueezesOrStretchesTheRun(string fixture, int arm, double expected)
        => Ratio(fixture, arm, Absent).ShouldBe(expected, 0.0005);

    /// <summary>99 per cent lands on the twip below it, as it does in the other two readers.</summary>
    /// <remarks>
    /// The value that matters: 99 is the corpus's commonest by a factor of ten, and a reader using
    /// the percentage itself is wrong by a quarter of a per cent on nearly all of them.
    /// </remarks>
    [Theory]
    [InlineData("words-char-scale.rtf")]
    [InlineData("words-char-scale.doc")]
    public void NinetyNinePerCentLandsOnTheTwipBelowIt(string fixture)
        => Ratio(fixture, NinetyNine, Absent).ShouldBe(237.0 / 240.0, 0.0005);

    /// <summary>A stated 100 costs exactly nothing.</summary>
    [Theory]
    [InlineData("words-char-scale.rtf")]
    [InlineData("words-char-scale.doc")]
    public void AHundredPerCentIsNoScaleAtAll(string fixture)
        => Ratio(fixture, Hundred, Absent).ShouldBe(1.0, 1e-9);

    /// <summary>A scaled run inside an unscaled paragraph survives both folds these readers have.</summary>
    /// <remarks>
    /// These two readers need the width in <b>three</b> places, not the two the DOCX and ODF paths
    /// needed, and this is the test for the extra one. Besides the uniform-paragraph shortcut they
    /// each merge adjacent runs whose formatting matches — and <b>WW8 builds its runs one character at
    /// a time</b>, so a property missing from that comparison does not merely fail to vary the
    /// paragraph: the scaled characters are absorbed into the unscaled run beside them and take
    /// <em>its</em> width, and the file's own boundary is gone before the shortcut is ever asked.
    /// </remarks>
    [Theory]
    [InlineData("words-char-scale.rtf")]
    [InlineData("words-char-scale.doc")]
    public void AScaledSpanInsideAnUnscaledParagraphSurvivesBothFolds(string fixture)
    {
        // Against the absent arm rather than against the word beside it, because the reference's own
        // span for that word carries the trailing blank and so measures 86.712 against 83.712. Every
        // arm draws the same word so that like is compared with like; comparing against the neighbour
        // reads 0.57899 for a 60 per cent span.
        Ratio(fixture, UnscaledRunOfTheLastParagraph, Absent).ShouldBe(1.0, 0.0005);
        Ratio(fixture, ScaledSpan, Absent).ShouldBe(0.60, 0.0005);
    }

    private static double Ratio(string fixture, int arm, int against)
    {
        List<DrawnWord> words = Words(fixture);
        return Width(words[arm]) / Width(words[against]);
    }

    private static double Width(DrawnWord word) => word.Right - word.Left;

    private static List<DrawnWord> Words(string fixture)
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(fixture)))
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
