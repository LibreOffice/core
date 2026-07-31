using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Tests <c>w:altChunk</c>: a whole other document embedded in the package by reference.
/// </summary>
/// <remarks>
/// The corpus document embeds three at once — a DOCX, an RTF and an HTML fragment — because the
/// interesting behaviour is the mixture. Two of the three are formats Paperless reads and are
/// spliced into the host at the position of their placeholder; the third is not, and has to stay
/// a diagnostic rather than becoming silence.
/// </remarks>
public class AltChunkTests
{
    private static IDocument Open()
        => new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require("alt-chunk.docx")));

    [Fact]
    public void AChunkInAFormatPaperlessReadsIsSplicedInAtItsPlaceholder()
    {
        using IDocument document = Open();

        string[] paragraphs =
        [
            .. Descendants(document.Content).OfType<ContentParagraph>()
                                            .Select(paragraph => paragraph.GetText().Trim())
                                            .Where(text => text.Length > 0),
        ];

        // In document order, host and chunks interleaved: the placeholder's position is where
        // the chunk's blocks go, not the end of the body.
        paragraphs.ShouldBe(
        [
            "Host paragraph before the chunks.",
            "A DOCX chunk, read by the DOCX reader.",
            "Its second paragraph.",
            "An RTF chunk, read by the RTF reader.",
            "Its second paragraph.",
            "Host paragraph after the chunks.",
        ]);
    }

    /// <summary>
    /// A chunk in a format Paperless does not read stays a diagnostic.
    /// </summary>
    /// <remarks>
    /// HTML and plain text are the two content types <c>w:altChunk</c> was invented for and
    /// neither is a word-processing format this library claims, so the diagnostic is not an edge
    /// case — it is the common one. Reporting it matters more than for most unread constructs,
    /// because what is missing is a whole document rather than a property of one.
    /// </remarks>
    [Fact]
    public void AChunkInAFormatPaperlessDoesNotReadIsStillReported()
    {
        using IDocument document = Open();

        Diagnostic chunk = document.Diagnostics.ShouldHaveSingleItem();

        chunk.Code.ShouldBe("PL2120");
        chunk.Severity.ShouldBe(DiagnosticSeverity.Warning);
        chunk.Message.ShouldContain("w:altChunk");
        chunk.Location?.PartName.ShouldBe("word/chunk3.html");
    }

    /// <summary>
    /// The format is sniffed rather than believed.
    /// </summary>
    /// <remarks>
    /// The corpus document declares its RTF chunk through a <c>Default</c> extension mapping,
    /// which is the loosest possible statement of what it holds, and the DOCX chunk through an
    /// <c>Override</c>. Both read the same way because neither declaration is consulted: an
    /// <c>altChunk</c>'s content type is wrong often enough that Word ignores it too, and
    /// detection by content is the house rule regardless.
    /// </remarks>
    [Fact]
    public void TheRtfChunkIsReadAsRtfWithoutConsultingItsContentType()
    {
        using IDocument document = Open();

        document.Content.GetText().ShouldContain("An RTF chunk, read by the RTF reader.");
    }

    private static IEnumerable<ContentNode> Descendants(ContentNode node)
    {
        foreach (ContentNode child in node.Children)
        {
            yield return child;
            foreach (ContentNode descendant in Descendants(child)) yield return descendant;
        }
    }
}
