using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A SmartArt diagram anchored in a document contributes its text to extraction, which until now
/// it did not.
/// </summary>
/// <remarks>
/// <para>
/// Rendering a diagram and extracting one are two different readings of the same five parts, and
/// making the first work does not make the second work: <c>IDocument</c> gives content and
/// <c>IPaginatedDocument.Layout()</c> is a distinct deferred step, so extraction reaches the data
/// model directly rather than through anything that needs a theme, a font or a frame extent. On
/// <c>024_Unit_Circle_Chart_Colorful_Circles</c> the whole deficit was the five nodes'
/// <c>YOUR TEXT</c> — the words were in the package the entire time and nothing asked for them.
/// </para>
/// <para>
/// <strong>The two readings want different answers, which is why this is not "call the
/// renderer".</strong> The baked <c>dsp:spTree</c> is what the author sees, so it repeats a node's
/// text wherever the layout drew it and adds text the layout generated; the data model is what the
/// author typed, once each. <see cref="DocxDiagramPackage"/> makes the difference observable — its
/// model holds a third node the baked drawing never draws, and two points on types no reader sees.
/// </para>
/// </remarks>
public sealed class FrameDiagramExtractionTests
{
    private static IDocument Open()
        => new WordProcessingReader().Read(
            DocumentSource.FromBytes(DocxDiagramPackage.Bytes(), "diagram.docx"));

    /// <summary>The author's nodes are extracted, from the data model rather than the drawing.</summary>
    /// <remarks>
    /// <c>Third node</c> is the discriminator: it is in <c>data1.xml</c> and not in
    /// <c>drawing1.xml</c>, so an extraction that read the baked shape tree would return two
    /// nodes here and this would fail.
    /// </remarks>
    [Fact]
    public void TheAuthorsNodesAreExtractedFromTheDataModel()
    {
        using IDocument document = Open();

        ContentSection frame = document.Content.Children.OfType<ContentSection>()
            .Single(section => section.Kind == SectionKind.Frame);

        frame.Children.OfType<ContentParagraph>().Select(p => p.GetText().TrimEnd('\n'))
            .ShouldBe(["First node", "Second node", "Third node"]);
    }

    /// <summary>
    /// A generated presentation node and a connector are not the author's text and are skipped.
    /// </summary>
    /// <remarks>
    /// A <c>pres</c> point can carry a duplicate of a real node's text, so admitting it would
    /// double a diagram's contribution to an index; <c>parTrans</c> and <c>sibTrans</c> are the
    /// arrows between nodes and carry labels the reader never sees as prose.
    /// </remarks>
    [Fact]
    public void GeneratedAndTransitionPointsAreNotExtracted()
    {
        using IDocument document = Open();

        string text = document.Content.GetText();

        text.ShouldNotContain("Layout generated");
        text.ShouldNotContain("Connector");
    }

    /// <summary>
    /// The diagram hoists to a frame of its own instead of splicing into the anchoring paragraph.
    /// </summary>
    /// <remarks>
    /// The same reason a text box does: a diagram holds its own paragraphs, and dropping five
    /// circle labels into the middle of the sentence that happens to anchor them would join two
    /// unrelated pieces of prose and split that paragraph in two at the anchor point. The frame
    /// keeps the object's name, so a caller can tell which drawing the words came from.
    /// </remarks>
    [Fact]
    public void TheDiagramIsHoistedToAFrameOfItsOwn()
    {
        using IDocument document = Open();

        List<ContentSection> sections = [.. document.Content.Children.OfType<ContentSection>()];

        sections[0].Kind.ShouldBe(SectionKind.Body);
        sections[0].GetText().ShouldNotContain("node");

        sections.Single(s => s.Kind == SectionKind.Frame).Name.ShouldBe("Diagram 1");
    }
}
