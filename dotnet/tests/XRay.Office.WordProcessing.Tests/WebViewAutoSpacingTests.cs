using System.Xml.Linq;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// That <c>w:beforeAutospacing</c> is under two and a half points in a document saved in web view.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice branches on the document's <c>w:view</c> in both
/// <c>LN_CT_Spacing_beforeAutospacing</c> and <c>LN_CT_Spacing_afterAutospacing</c>
/// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:927 and :948): 49 twips in web view and 280
/// everywhere else. The difference is 11.55 pt at every auto-spaced paragraph boundary, and
/// <c>w:view</c> is easy to miss because it is not a <c>w:compat</c> flag — it sits directly under
/// <c>w:settings</c> beside the zoom, and reads like a preference.
/// </para>
/// <para>
/// Found on <c>May 25 bulletin focus on carers in the workplace.docx</c>, the only document in the words
/// corpus that declares web view, and found by a blind reading rather than by a metric: a reviewer given
/// nothing but a paired image of page 2 reported our bullets "spaced apart" against the reference's
/// "tightly stacked". The document paginated 5 pages against the reference's 4 before this and 4 after.
/// </para>
/// </remarks>
public sealed class WebViewAutoSpacingTests
{
    [Theory]
    [InlineData(null, 280)]
    [InlineData("print", 280)]
    [InlineData("web", 49)]
    public void AutoSpacingFollowsTheDocumentsView(string? view, int twips)
    {
        SpacingOf(Settings(view)).ShouldBe((Length.FromTwips(twips), Length.FromTwips(twips)));
    }

    /// <summary>
    /// <c>w:doNotUseHTMLParagraphAutoSpacing</c> outranks the view, as it does in the importer.
    /// </summary>
    [Fact]
    public void TheHtmlAutoSpacingFlagOutranksTheView()
    {
        XElement settings = Settings("web");
        settings.Add(new XElement(
            W + "compat", new XElement(W + "doNotUseHTMLParagraphAutoSpacing")));

        SpacingOf(settings).ShouldBe((Length.FromTwips(100), Length.FromTwips(100)));
    }

    private static readonly XNamespace W =
        "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    private static XElement Settings(string? view)
    {
        XElement settings = new(W + "settings");
        if (view is not null) settings.Add(new XElement(W + "view", new XAttribute(W + "val", view)));
        return settings;
    }

    private static (Length Before, Length After) SpacingOf(XElement settings)
    {
        XElement body = new(
            W + "body",
            Auto("first"),
            Auto("second"));

        List<PageBlock> blocks = new DocxLayoutSource(new WordStyles(), settings).Read(body);

        // The second paragraph, so that neither reading is the first-in-flow special case.
        PageParagraph paragraph = blocks.OfType<PageParagraph>().ElementAt(1);

        return (paragraph.Format.SpaceBefore, paragraph.Format.SpaceAfter);
    }

    private static XElement Auto(string text) => new(
        W + "p",
        new XElement(
            W + "pPr",
            new XElement(
                W + "spacing",
                new XAttribute(W + "beforeAutospacing", "1"),
                new XAttribute(W + "afterAutospacing", "1"))),
        new XElement(W + "r", new XElement(W + "t", text)));
}
