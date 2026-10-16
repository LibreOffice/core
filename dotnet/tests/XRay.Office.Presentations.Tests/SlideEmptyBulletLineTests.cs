using System.Xml.Linq;
using XRay.Office.Core;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Presentations.Ooxml;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// An empty paragraph's line is never shorter than the box its bullet would be drawn in — on the
/// two readers that leave such a paragraph at its numbering level.
/// </summary>
/// <remarks>
/// <para>
/// The last block of <c>ImpEditEngine::CreateAndInsertEmptyLine</c>
/// (<c>editeng/source/editeng/impedit3.cxx</c>:1974-1985) raises a line whose paragraph has no
/// characters to <c>GetBulletArea().GetHeight()</c> when the box is the taller of the two,
/// halving the difference into the ascent. It sits <em>after</em> the four line-spacing arms, so
/// a proportional spacing below 100 % cannot shrink such a line past the box — and the box does
/// not scale with the percentage.
/// </para>
/// <para>
/// <strong>Measured at 26.2.4.2 by single-variable experiment</strong>, first on its own flat
/// ODP of <c>slides/done-012/ppt/JesuitAssocOfStudentPersonnel.ppt</c> page 24 and then on the
/// authored fixture below, whose six slides differ in one thing each. The full record, including
/// the <c>fo:line-height</c> sweep that pins the floor at a constant 787 hundredths of a
/// millimetre from 90 % downwards, is in <c>probes/slides-size2-r110</c>.
/// </para>
/// <para>
/// Every number asserted here is the reference's, read off its PDF of the same fixture, and the
/// fixture's own header lists them beside what each slide varies.
/// </para>
/// </remarks>
public class SlideEmptyBulletLineTests
{
    private static SlidePages Layout()
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("odp-empty-bullet-line.fodp")));

        document.ShouldBeAssignableTo<IPaginatedDocument>();
        return (SlidePages)((IPaginatedDocument)document).Layout();
    }

    /// <summary>
    /// The drawn baselines of the slide's <em>words</em>, top first — the bullets excluded,
    /// because a symbol marker sits on a baseline of its own and would come first.
    /// </summary>
    private static List<double> Baselines(LaidOutSlide slide) =>
    [
        .. slide.Shapes.Where(shape => shape.Text is not null)
            .SelectMany(shape => shape.Text!.Runs)
            .Where(run => run.Run.Text is "AAA" or "BBB" or "x")
            .Select(run => run.Run.Origin.Y.Points)
            .Distinct()
            .Order(),
    ];

    /// <summary>
    /// The reference's own AAA and BBB baselines on each of the fixture's six slides.
    /// </summary>
    /// <remarks>
    /// Slide 1 against slide 2 is the floor itself — 41.53 pt of pitch against the control's
    /// 38.44, and 3.09 pt is the bullet's 787 hundredths of a millimetre against the
    /// <c>fround(fround(20 pt × 1.2) × 0.8) = 678</c> the line would otherwise have. Slides 3
    /// and 5 are the two ways the reference makes it zero — a level with no numbering format,
    /// and a line already taller than the box — and slides 4 and 6 identify the box as the
    /// bullet's own by moving its size and its face.
    /// </remarks>
    [Theory]
    [InlineData(1, 100.40, 141.93)]
    [InlineData(2, 100.40, 138.84)]
    [InlineData(3, 100.40, 138.81)]
    [InlineData(4, 100.40, 164.38)]
    [InlineData(5, 105.05, 153.07)]
    [InlineData(6, 100.40, 142.89)]
    public void AnEmptyBulletedParagraphIsNoShorterThanItsBullet(int slide, double aaa, double bbb)
    {
        SlidePages pages = Layout();
        pages.Slides.Count.ShouldBe(6);

        List<double> baselines = Baselines(pages.Slides[slide - 1]);

        baselines[0].ShouldBe(aaa, 0.02);
        baselines[^1].ShouldBe(bbb, 0.02);
    }

    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>The same three paragraphs as the fixture's first slide, written as DrawingML.</summary>
    private static XElement Ooxml(bool empty)
    {
        const string properties =
            """<a:pPr marL="360000" indent="-360000"><a:lnSpc><a:spcPct val="80000"/></a:lnSpc><a:buFont typeface="Liberation Sans"/><a:buChar char="&#8226;"/></a:pPr>""";

        string middle = empty
            ? $"<a:p>{properties}<a:endParaRPr sz=\"2000\"/></a:p>"
            : $"<a:p>{properties}<a:r><a:rPr lang=\"en-GB\" sz=\"2000\"/><a:t>x</a:t></a:r></a:p>";

        return XElement.Parse(
            $"""
             <a:txBody xmlns:a="{A}">
               <a:bodyPr/>
               <a:p>{properties}<a:r><a:rPr lang="en-GB" sz="2000"/><a:t>AAA</a:t></a:r></a:p>
               {middle}
               <a:p>{properties}<a:r><a:rPr lang="en-GB" sz="2000"/><a:t>BBB</a:t></a:r></a:p>
             </a:txBody>
             """);
    }

    /// <summary>
    /// The reader control: an OOXML deck has no floor at all, because its importer sets an empty
    /// paragraph's <em>level</em> to −1 and not merely its bullet state.
    /// </summary>
    /// <remarks>
    /// Measured rather than read: the fixture converted to <c>.pptx</c> by 26.2.4.2 and rendered
    /// back by it answers a pitch of 38.43 pt on slides 1, 3, 4 <em>and</em> 6 — the floor gone
    /// from every one of them, the 200 % bullet included — where the ODF original answers 41.53,
    /// 38.41, 63.98 and 42.49. Asserted here as the difference between the empty middle
    /// paragraph and the same paragraph holding a character, which is zero for OOXML and 3.09 pt
    /// for the ODF fixture above.
    /// </remarks>
    [Fact]
    public void AnOoxmlEmptyParagraphLosesItsLevelAndSoHasNoBoxToClear()
    {
        DocRect area = new(
            Length.Zero, Length.Zero, Length.FromPoints(500), Length.FromPoints(400));

        double Pitch(bool empty)
        {
            List<PlacedGlyphRun> placed = SlideTextLayout.Place(
                PptxTextBody.Read(Ooxml(empty)), area, new SlideFonts());

                List<double> baselines =
            [
                .. placed.Where(run => run.Run.Text is "AAA" or "BBB" or "x")
                    .Select(run => run.Run.Origin.Y.Points)
                    .Distinct()
                    .Order(),
            ];
            return baselines[^1] - baselines[0];
        }

        Pitch(empty: true).ShouldBe(Pitch(empty: false), 0.04);
    }
}
