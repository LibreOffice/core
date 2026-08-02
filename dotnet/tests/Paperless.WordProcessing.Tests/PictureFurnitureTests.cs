using Paperless.Containers;
using Paperless.Containers.Ole2;
using Paperless.Core.Diagnostics;
using Paperless.TestKit;
using Paperless.WordProcessing.Model;
using Paperless.WordProcessing.Ww8;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A DOC whose running head and foot hold a picture and no words at all.
/// </summary>
/// <remarks>
/// <para>
/// Word writes all six of a section's header stories whether the section uses them or not, so the reader
/// has to tell "this section has no such header" from "it has an empty one" — and the only thing that
/// distinguishes them is emptiness. The test it made was on the paragraph's <em>text</em>, which is the
/// wrong question: a paragraph whose only content is an inline picture reads back with no text, because
/// the U+0001 that stands for the picture is consumed by the frame it makes.
/// </para>
/// <para>
/// So a logo-only running head was thrown away entirely, and with it the room it occupies. Measured on
/// <c>words/batch-018/doc/120509coss.doc</c>, whose header is one 97.7 pt logo and whose footer is
/// another of 60.7 pt: the body ran the full height of the sheet and the document came out 20 pages
/// against LibreOffice's 25.
/// </para>
/// </remarks>
public sealed class PictureFurnitureTests
{
    /// <summary>The header story survives when its one paragraph holds only a picture.</summary>
    [Fact]
    public void APictureOnlyHeaderIsNotMistakenForAnEmptyOne()
    {
        Ww8LayoutFurniture furniture = Furniture();

        furniture.Headers.ShouldContainKey(PageFurnitureSlot.Default);

        Ww8LayoutBlock block = furniture.Headers[PageFurnitureSlot.Default].ShouldHaveSingleItem();

        block.Paragraph.ShouldNotBeNull();
        block.Paragraph!.Value.Text.ShouldBeEmpty();
        block.Paragraph!.Value.Frames.ShouldNotBeNull();
        block.Paragraph!.Value.Frames!.Count.ShouldBe(1);
    }

    /// <summary>And so does the footer's, which is the same story read from another of the six slots.</summary>
    [Fact]
    public void APictureOnlyFooterIsNotMistakenForAnEmptyOne()
    {
        Ww8LayoutFurniture furniture = Furniture();

        furniture.Footers.ShouldContainKey(PageFurnitureSlot.Default);
        furniture.Footers[PageFurnitureSlot.Default].ShouldHaveSingleItem()
            .Paragraph!.Value.Frames!.Count.ShouldBe(1);
    }

    /// <summary>
    /// A story that really does hold nothing but a paragraph mark is still no header.
    /// </summary>
    /// <remarks>
    /// The other half of the rule, and the reason the emptiness test exists at all: Word writes the
    /// even-page and first-page stories for a section that uses neither, and filling those slots with the
    /// placeholder paragraph would draw a blank line at the top of every left-hand page.
    /// </remarks>
    [Fact]
    public void AStoryHoldingOnlyAParagraphMarkIsStillNoHeader()
    {
        Furniture().Headers.ShouldNotContainKey(PageFurnitureSlot.Even);
    }

    private static Ww8LayoutFurniture Furniture()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("picture-furniture.doc"));
        using CompoundFile file = CompoundFile.Open(stream, leaveOpen: true);

        byte[] wordDocument = StreamOf(file, DocReader.WordDocumentStreamName)!;
        Ww8Fib fib = Ww8Fib.Parse(wordDocument);
        byte[] table = StreamOf(file, fib.UsesTable1Stream ? "1Table" : "0Table") ?? [];

        List<Diagnostic> diagnostics = [];
        Ww8DocumentReader reader = new(
            wordDocument, table, fib, diagnostics, StreamOf(file, DocReader.PictureStreamName));

        return reader.ReadLayoutFurniture();
    }

    private static byte[]? StreamOf(CompoundFile file, string name)
    {
        IPackagePart? part = file.GetPart(name);
        if (part is null) return null;

        using Stream stream = part.Open();
        using MemoryStream buffer = new();
        stream.CopyTo(buffer);
        return buffer.ToArray();
    }
}
