using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Text.Fonts;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// RTF has a control word per underline pattern, and two of the seventeen draw two lines.
/// </summary>
/// <remarks>
/// <para>
/// <c>\uldb</c> is <c>LINESTYLE_DOUBLE</c> and <c>\ululdbwave</c> is <c>LINESTYLE_DOUBLEWAVE</c>;
/// the other fifteen are one line whatever pattern they name, because this engine draws no pattern.
/// Reading the whole family as one switch — which is what this reader did — turns
/// <c>RobertQ_Service</c>'s 162 <c>\uldb</c> into 162 single underlines when the same document is
/// read through 26.2.4.2's own RTF conversion of it.
/// </para>
/// <para>
/// The prefix is the trap. <c>\uld</c>, <c>\uldash</c>, <c>\uldashd</c> and <c>\uldashdd</c> all
/// begin with the letters of <c>\uldb</c>, so a control word matched by prefix rather than in full
/// double-underlines four patterns that draw one line — and the corpus's converted <c>.rtf</c> hold
/// all four.
/// </para>
/// </remarks>
public sealed class RtfDoubleUnderlineTests
{
    [Theory]
    [InlineData(@"\uldb", TextUnderline.DoubleLine)]
    [InlineData(@"\ululdbwave", TextUnderline.DoubleLine)]
    [InlineData(@"\ul", TextUnderline.SingleLine)]
    [InlineData(@"\uld", TextUnderline.SingleLine)]
    [InlineData(@"\uldash", TextUnderline.SingleLine)]
    [InlineData(@"\uldashd", TextUnderline.SingleLine)]
    [InlineData(@"\uldashdd", TextUnderline.SingleLine)]
    [InlineData(@"\ulwave", TextUnderline.SingleLine)]
    [InlineData(@"\ulth", TextUnderline.BoldLine)]
    [InlineData(@"\ulthdashdd", TextUnderline.BoldLine)]
    [InlineData(@"\ulhwave", TextUnderline.BoldLine)]
    [InlineData(@"\ulnone", TextUnderline.None)]
    [InlineData("", TextUnderline.None)]
    // Every one of the family takes a parameter, and nought turns it off.
    [InlineData(@"\uldb0", TextUnderline.None)]
    [InlineData(@"\ul0", TextUnderline.None)]
    public void EachUnderlineControlWordAsksForItsOwnNumberOfLines(
        string word, TextUnderline expected)
        => Underline(word).ShouldBe(expected);

    /// <summary>And a double-underlined run draws two rules where a single one draws one.</summary>
    [Fact]
    public void ADoubleUnderlinedRunDrawsTwoRules()
    {
        Rules(@"\uldb").ShouldBe(2);
        Rules(@"\ul").ShouldBe(1);
        Rules(string.Empty).ShouldBe(0);
    }

    private static TextUnderline Underline(string word) => Single(word).Underline;

    private static int Rules(string word)
    {
        PlacedDrawingSink sink = new();
        using DocumentSource source = Source(word);
        using IDocument document = new WordProcessingReader().Read(source);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();
        pages[0].Draw(sink);

        // A rule is a filled rectangle far wider than it is tall, which nothing else on this page
        // is: the fixture holds two words and no border, table or frame.
        return sink.Fills.Count(f => f.Bounds.Width > f.Bounds.Height * 4);
    }

    private static PageRun Single(string word)
    {
        using DocumentSource source = Source(word);
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        // Two runs, so the paragraph is not uniform and keeps them: the decoration is on the
        // second, which is what a uniform-paragraph shortcut would otherwise drop.
        return pages.Paragraphs.First(p => p.Text.Contains("MARKED")).Runs[^1];
    }

    private static DocumentSource Source(string word)
    {
        string rtf =
            @"{\rtf1\ansi\deff0{\fonttbl{\f0\froman Liberation Serif;}}"
            + @"\paperw11906\paperh16838\margl1440\margr1440\margt1440\margb1440\sectd"
            // The leading stretch is italic so the paragraph is never uniform, whatever the
            // control word does: a uniform paragraph keeps no runs at all and there would be
            // nothing to read the underline off in the cases that turn it off.
            + @"\pard\plain\fs24\i plain\i0 " + word + @" MARKED\par}";

        return DocumentSource.FromStream(
            new MemoryStream(Encoding.ASCII.GetBytes(rtf)), "underline.rtf");
    }
}
