using System.Xml.Linq;
using XRay.Office.Presentations.Layout;
using XRay.Office.Presentations.Ooxml;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A run's <c>a:rPr/@cap</c> decides the case the text is drawn in.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Measured on LibreOffice 26.2.4.2</strong> over the 100 downloaded decks in
/// <c>slides/chartset-005…014</c>. Stripping every space from both <c>pdftotext</c> extractions
/// and comparing what remains, 17 more decks draw the reference's characters exactly once this
/// is applied — 50 of 100 before, 67 after. Worked to the glyph on
/// <c>056_Four-Block_Hub_Spoke_134b89d1.pptx</c>, where the reference extracts <c>LOREM</c> and
/// <c>IPSUM</c> and we extracted <c>Lorem</c> and <c>Ipsum</c>.
/// </para>
/// <para>
/// <strong>It wins no gate verdict and that is the point of writing it down.</strong> The corpus
/// gate compares extracted word *counts*, and upper-casing a word does not change how many words
/// there are: the slides track measured 12 of 100 both before and after, with zero documents
/// moving in either direction across all 160 slide documents swept. This is the "a real fix that
/// moves no verdict" case — the defect is real, the rendering is now right, and no column of the
/// scoreboard can see it. Do not re-derive it by looking for the missing verdicts.
/// </para>
/// <para>
/// Only <c>all</c> is implemented. <c>small</c> needs real or synthesised small capitals rather
/// than upper-casing, LibreOffice's own behaviour there is unmeasured, and seven corpus decks
/// state it — so it deliberately draws the text as authored, which is what we did before this
/// existed. <see cref="SmallCapsIsLeftAloneUntilItIsMeasured"/> pins that as a decision rather
/// than an oversight.
/// </para>
/// </remarks>
public sealed class SlideCapitalisationTests
{
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>The text of the one paragraph in a body built from <paramref name="inner"/>.</summary>
    private static string Drawn(string inner, string? inheritedDefaultRunProperties = null)
    {
        XElement body = XElement.Parse($"<p:txBody xmlns:a=\"{A}\" xmlns:p=\"p\">{inner}</p:txBody>");

        Func<int, IReadOnlyList<XElement>>? inherited = inheritedDefaultRunProperties is null
            ? null
            : _ => [XElement.Parse($"<a:lvl1pPr xmlns:a=\"{A}\">{inheritedDefaultRunProperties}</a:lvl1pPr>")];

        return PptxTextBody.Read(body, inherited: inherited).Paragraphs.Single().Text;
    }

    /// <summary>The control: a run that states no case map is drawn as it was authored.</summary>
    [Fact]
    public void ARunThatStatesNoCaseMapIsUnchanged()
        => Drawn("<a:p><a:r><a:t>Lorem Ipsum</a:t></a:r></a:p>").ShouldBe("Lorem Ipsum");

    /// <summary><c>cap="all"</c> draws every letter as a capital.</summary>
    [Fact]
    public void CapAllIsDrawnInCapitals()
        => Drawn("<a:p><a:r><a:rPr cap=\"all\"/><a:t>Lorem Ipsum</a:t></a:r></a:p>")
            .ShouldBe("LOREM IPSUM");

    /// <summary><c>cap="none"</c> is the schema's own default and changes nothing.</summary>
    [Fact]
    public void CapNoneIsUnchanged()
        => Drawn("<a:p><a:r><a:rPr cap=\"none\"/><a:t>Lorem Ipsum</a:t></a:r></a:p>")
            .ShouldBe("Lorem Ipsum");

    /// <summary>
    /// <c>cap="small"</c> is read and deliberately not acted on — see the remarks.
    /// </summary>
    [Fact]
    public void SmallCapsIsLeftAloneUntilItIsMeasured()
        => Drawn("<a:p><a:r><a:rPr cap=\"small\"/><a:t>Lorem Ipsum</a:t></a:r></a:p>")
            .ShouldBe("Lorem Ipsum");

    /// <summary>
    /// The attribute is inherited: a deck that states it once on its master and nowhere else is
    /// the common case, and reading only the run's own <c>a:rPr</c> would miss every one of them.
    /// </summary>
    [Fact]
    public void CapAllIsInheritedFromTheChain()
        => Drawn(
            "<a:p><a:r><a:t>Lorem Ipsum</a:t></a:r></a:p>",
            "<a:defRPr cap=\"all\"/>").ShouldBe("LOREM IPSUM");

    /// <summary>And a run that states <c>none</c> overrides an inherited <c>all</c>.</summary>
    [Fact]
    public void ARunsOwnNoneBeatsAnInheritedAll()
        => Drawn(
            "<a:p><a:r><a:rPr cap=\"none\"/><a:t>Lorem Ipsum</a:t></a:r></a:p>",
            "<a:defRPr cap=\"all\"/>").ShouldBe("Lorem Ipsum");

    /// <summary>
    /// Run offsets index the paragraph's own buffer, so the casing must not move them.
    /// </summary>
    /// <remarks>
    /// This is the assertion that matters most and the one a casing change can silently break:
    /// every run carries <c>Start</c> and <c>Length</c> into the string this returns, so a
    /// transform that changed the length would leave every later run pointing at the wrong text
    /// — drawn in the wrong font, at the wrong size, in the wrong colour, with nothing failing.
    /// </remarks>
    [Fact]
    public void RunOffsetsStillIndexTheDrawnText()
    {
        XElement body = XElement.Parse(
            $"<p:txBody xmlns:a=\"{A}\" xmlns:p=\"p\"><a:p>"
            + "<a:r><a:rPr cap=\"all\"/><a:t>Lorem</a:t></a:r>"
            + "<a:r><a:t> plain</a:t></a:r>"
            + "<a:r><a:rPr cap=\"all\"/><a:t> Ipsum</a:t></a:r>"
            + "</a:p></p:txBody>");

        SlideParagraph paragraph = PptxTextBody.Read(body).Paragraphs.Single();

        paragraph.Text.ShouldBe("LOREM plain IPSUM");

        List<string> byRun = [.. paragraph.Runs.Select(run => paragraph.Text[run.Start..run.End])];
        byRun.ShouldBe(["LOREM", " plain", " IPSUM"]);
    }

    /// <summary>
    /// A field's text is drawn through the same character properties a run's is, so it is
    /// capitalised on the same terms.
    /// </summary>
    [Fact]
    public void AFieldIsCapitalisedLikeARun()
        => Drawn("<a:p><a:fld id=\"x\" type=\"datetime\"><a:rPr cap=\"all\"/>"
                 + "<a:t>Monday</a:t></a:fld></a:p>").ShouldBe("MONDAY");
}
