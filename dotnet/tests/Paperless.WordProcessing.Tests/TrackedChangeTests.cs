using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Tests how all four word-processing readers treat tracked changes.
/// </summary>
/// <remarks>
/// <para>
/// One document in four formats, all converted from the same hand-written RTF so the revisions are
/// identical in every one: an inserted phrase and a deleted phrase in the same sentence.
/// </para>
/// <para>
/// Paperless extracts the document as the changes leave it: the insertion is content, the deletion is
/// not. Deleted text is still in the file — that is what makes the change reversible — so emitting it
/// puts words into the extraction that the document does not say, which is worse than the alternative
/// for every use extraction has. LibreOffice's own text filter does the opposite, because it renders
/// the document as a reader sees it and its default is to show change marks; that difference is
/// deliberate and recorded in <c>ExtractionComparisonTests.KnownDeviations</c>.
/// </para>
/// <para>
/// The four formats mark the change four different ways — <c>w:del</c> elements in DOCX, a
/// <c>\deleted</c> destination in RTF, <c>sprmCFRMarkDel</c> on a run in DOC, and a change region
/// hoisted out of the body in ODF — so agreement between them is evidence about the readers rather
/// than about one shared code path.
/// </para>
/// </remarks>
public class TrackedChangeTests
{
    private const string Deleted = "a deleted phrase";
    private const string Inserted = "an inserted phrase";

    private static IDocument Open(string name)
        => new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    [Theory]
    [InlineData("revisions.rtf")]
    [InlineData("revisions.doc")]
    [InlineData("revisions.docx")]
    [InlineData("revisions.odt")]
    public void AnInsertionIsContentAndADeletionIsNot(string name)
    {
        using IDocument document = Open(name);
        string text = document.Content.GetText();

        text.ShouldContain(Inserted);
        text.Contains(Deleted, StringComparison.Ordinal).ShouldBeFalse(
            $"{name}: text a tracked change removed is still in the file, and emitting it invents "
            + "content the document does not say");

        // The paragraph around the change is otherwise intact — a reader that drops the deletion by
        // skipping too much loses the text on either side of it as well.
        text.ShouldContain("A paragraph with ");
        text.ShouldContain(" in the middle.");
        text.ShouldContain("A second paragraph that nothing touched.");
    }

    [Fact]
    public void TheFourFormatsAgreeOnWhatTheDocumentSays()
    {
        using IDocument rtf = Open("revisions.rtf");
        using IDocument doc = Open("revisions.doc");
        using IDocument docx = Open("revisions.docx");
        using IDocument odt = Open("revisions.odt");

        string expected = rtf.Content.GetText();
        doc.Content.GetText().ShouldBe(expected);
        docx.Content.GetText().ShouldBe(expected);
        odt.Content.GetText().ShouldBe(expected);
    }
}
