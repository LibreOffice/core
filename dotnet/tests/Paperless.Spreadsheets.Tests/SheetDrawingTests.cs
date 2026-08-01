using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// What the readers make of a picture anchored to a cell.
/// </summary>
/// <remarks>
/// About the anchor rather than about the pixels, and needs neither LibreOffice nor a codec: the
/// bytes are carried encoded and only a backend that wants pixels decodes them. Where the picture
/// lands on the page is <c>SheetDrawingComparisonTests</c>'s business.
/// </remarks>
public sealed class SheetDrawingTests
{
    /// <summary>The corpus document's picture sheet, which is its third.</summary>
    private const int PictureSheet = 2;

    [Theory]
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    public void APictureIsAnchoredToTheCellItSitsIn(string name)
    {
        SheetDrawing drawing = Only(name);

        // Row 2, column A, with a small offset into the cell: the anchor is a cell and a distance,
        // which is why inserting a column moves every picture to its right.
        drawing.From.Column.ShouldBe(0, $"{name}: anchor column");
        drawing.From.Row.ShouldBe(1, $"{name}: anchor row");
        drawing.From.ColumnOffset.ShouldBeGreaterThan(Length.Zero, $"{name}: offset across");
        drawing.From.RowOffset.ShouldBeGreaterThan(Length.Zero, $"{name}: offset down");
    }

    [Theory]
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    public void APictureCarriesItsBytesUndecoded(string name)
    {
        SheetDrawing drawing = Only(name);

        drawing.Image.ShouldNotBeNull($"{name}: the picture was read");
        drawing.Image.IsDecoded.ShouldBeFalse($"{name}: and was not decoded");
        drawing.Image.EncodedBytes.Length.ShouldBeGreaterThan(0, $"{name}: it has bytes");

        // The declared media type where the file declares one, and null where it does not — which
        // is the difference between the two containers here. OPC gives every part a content type
        // and always has one; ODF states draw:mime-type only when the writer bothered, and the
        // hand-written flat source deliberately does not. Either way it is a hint rather than an
        // answer: a decoder sniffs the bytes, because office files mislabel images as routinely as
        // they mislabel themselves, and the invariant worth asserting is that nothing was invented.
        drawing.Image.EncodedMediaType.ShouldBeOneOf(
            [null, "image/png"], $"{name}: declared media type");

        // A PNG signature, which is the check that the right part was fetched rather than some
        // other entry of the package that happened to be reachable.
        drawing.Image.EncodedBytes.Span[..4].ToArray()
               .ShouldBe([0x89, (byte)'P', (byte)'N', (byte)'G'], $"{name}: PNG signature");
    }

    [Theory]
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    public void ATwoCellAnchorSpansTheCellsBetweenItsCorners(string name)
    {
        SheetDrawing drawing = Only(name);

        // Both formats state the far corner as a cell: SpreadsheetML writes an <xdr:to> and ODF a
        // table:end-cell-address. That is what makes the picture resize when a column does, and
        // it is what LibreOffice believes when a frame's stated size disagrees with it — saving
        // this document's 1.28 in frame back out rewrites the width as 1.3201 in, the two columns
        // it spans less its own start offset.
        drawing.Anchor.ShouldBe(SheetAnchorKind.TwoCell, $"{name}: anchor kind");
        drawing.To.Column.ShouldBeGreaterThan(drawing.From.Column, $"{name}: spans a column");
    }

    [Theory]
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    public void ASheetWithoutDrawingsHasNone(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        IReadOnlyList<SheetLayout> sheets = ((SpreadsheetPages)document.Layout()).Sheets;

        // A sheet that holds no drawing costs one static instance rather than an empty list per
        // sheet, which is what makes reading them free for the workbooks that have none.
        sheets[0].Drawings.IsEmpty.ShouldBeTrue($"{name}: the text sheet");
        sheets[1].Drawings.IsEmpty.ShouldBeTrue($"{name}: the turned sheet");
    }

    private static SheetDrawing Only(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        SheetLayout sheet = ((SpreadsheetPages)document.Layout()).Sheets[PictureSheet];

        sheet.Drawings.Items.Count.ShouldBe(1, $"{name}: drawings on the picture sheet");
        return sheet.Drawings.Items[0];
    }
}
