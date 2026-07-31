using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Tests the layout-facing document model's structure: document order, positions, and style chains.
/// </summary>
/// <remarks>
/// Document order is what makes "before", "after" and "inside" answerable without walking the tree,
/// and layout, bookmarks and tracked changes all ask those questions constantly. Style chains are the
/// fourth implementation of parent-chain resolution in Paperless — the ODF, DOCX and WW8 readers each
/// have one over their own encoding — so this one exists to be the single version layout uses.
/// </remarks>
public class WritingDocumentTests
{
    /// <summary>A document with a paragraph, a two-cell table row, and a footnote flow.</summary>
    private static WritingDocument Sample(
        out WritingParagraph first,
        out WritingParagraph inCell,
        out WritingParagraph inNote)
    {
        WritingDocument document = new();

        first = document.Body.Add(new WritingParagraph());
        first.Append("Before the table.");

        WritingTable table = document.Body.Add(new WritingTable());
        WritingTableRow row = table.Add(new WritingTableRow());
        WritingTableCell left = row.Add(new WritingTableCell());
        WritingTableCell right = row.Add(new WritingTableCell());

        inCell = left.Content.Add(new WritingParagraph());
        inCell.Append("Left");
        right.Content.Add(new WritingParagraph()).Append("Right");

        WritingBody note = document.AddFlow(new WritingBody(WritingBodyKind.Note) { Name = "1" });
        inNote = note.Add(new WritingParagraph());
        inNote.Append("The note.");

        document.AssignDocumentOrder();
        return document;
    }

    [Fact]
    public void DocumentOrderFollowsTheTextRatherThanTheTreeDepth()
    {
        WritingDocument document = Sample(out WritingParagraph first, out WritingParagraph inCell, out _);

        // A paragraph inside a cell comes after the one before the table, even though it is deeper.
        first.DocumentOrder.ShouldBeLessThan(inCell.DocumentOrder);

        // Every node is numbered, and the order is a permutation of 0..n with no gaps — so an index
        // can be compared, and a range of them means exactly the nodes between two points.
        List<int> order = [.. document.InDocumentOrder().Select(n => n.DocumentOrder)];
        order.ShouldBe([.. Enumerable.Range(0, order.Count)]);
    }

    [Fact]
    public void ContainmentIsTwoIntegerComparisonsRatherThanAWalk()
    {
        WritingDocument document = Sample(out WritingParagraph first, out WritingParagraph inCell, out _);
        WritingTable table = document.Body.Children.OfType<WritingTable>().Single();

        table.Contains(inCell).ShouldBeTrue();
        table.Contains(first).ShouldBeFalse();
        document.Body.Contains(table).ShouldBeTrue();

        // A node contains itself, which is what makes "is this inside the range I am laying out"
        // answerable without a special case for the range's own endpoints.
        table.Contains(table).ShouldBeTrue();
    }

    [Fact]
    public void AFlowIsBesideTheBodyRatherThanInsideIt()
    {
        WritingDocument document = Sample(out _, out _, out WritingParagraph inNote);

        // A footnote's text is not at the point that cites it, so the body must not contain it — a
        // layout that walked the body and found note text would lay the note out twice.
        document.Body.Contains(inNote).ShouldBeFalse();
        document.Flows.Single().Contains(inNote).ShouldBeTrue();
    }

    [Fact]
    public void PositionsCompareByDocumentOrderThenByOffset()
    {
        Sample(out WritingParagraph first, out WritingParagraph inCell, out _);

        WritingPosition early = new(first, 2);
        WritingPosition late = new(first, 9);
        WritingPosition elsewhere = new(inCell, 0);

        (early < late).ShouldBeTrue();
        (late < elsewhere).ShouldBeTrue();
        (elsewhere > early).ShouldBeTrue();
        early.CompareTo(early).ShouldBe(0);
    }

    [Fact]
    public void ARangeIsNormalisedSoBackwardsAndForwardsAreTheSame()
    {
        Sample(out WritingParagraph first, out WritingParagraph inCell, out _);

        WritingPosition a = new(first, 3);
        WritingPosition b = new(inCell, 1);

        new WritingRange(a, b).ShouldBe(new WritingRange(b, a));
        new WritingRange(b, a).Start.ShouldBe(a);

        WritingRange range = new(a, b);
        range.Contains(new WritingPosition(first, 5)).ShouldBeTrue();
        range.Contains(new WritingPosition(first, 1)).ShouldBeFalse();

        // Half-open: the end is not in the range, so two adjacent ranges do not overlap.
        range.Contains(b).ShouldBeFalse();
        range.Overlaps(new WritingRange(b, new WritingPosition(inCell, 4))).ShouldBeFalse();
    }

    [Fact]
    public void AnEmptyRangeContainsNothing()
    {
        Sample(out WritingParagraph first, out _, out _);

        WritingRange empty = new(new WritingPosition(first, 4), new WritingPosition(first, 4));
        empty.IsEmpty.ShouldBeTrue();
        empty.Contains(new WritingPosition(first, 4)).ShouldBeFalse();
    }

    [Fact]
    public void ANodeBelongsToOneOwner()
    {
        WritingDocument document = new();
        WritingParagraph paragraph = document.Body.Add(new WritingParagraph());
        WritingBody other = document.AddFlow(new WritingBody(WritingBodyKind.Header));

        // Both the document order and any position into a node come from where it sits, so a node in
        // two places would have two of each.
        Should.Throw<InvalidOperationException>(() => other.Add(paragraph));

        // The guard covers rows and cells too, not just paragraphs in bodies: a cell's own content
        // body already has an owner, so it cannot also be listed as a body's child.
        WritingTable table = document.Body.Add(new WritingTable());
        WritingTableRow row = table.Add(new WritingTableRow());
        WritingTableCell cell = row.Add(new WritingTableCell());

        Should.Throw<InvalidOperationException>(() => document.Body.Add(cell.Content));
        Should.Throw<InvalidOperationException>(() => table.Add(row));
        Should.Throw<InvalidOperationException>(() => row.Add(cell));

        cell.Row.ShouldBe(row);
        row.Table.ShouldBe(table);
    }

    // -------------------------------------------------------------------- styles

    [Fact]
    public void AStyleChainRunsOutermostFirst()
    {
        WritingStyles styles = new();
        styles.Add(new WritingStyle("Default", WritingStyleFamily.Paragraph, null, "default"));
        styles.Add(new WritingStyle("Heading", WritingStyleFamily.Paragraph, "Default", "heading"));
        styles.Add(new WritingStyle("Heading 1", WritingStyleFamily.Paragraph, "Heading", "h1"));
        styles.SetDefault(WritingStyleFamily.Paragraph, "Default");

        // Outermost first so a caller applies them in order and the nearest wins, which is the shape
        // all four readers already use over four different encodings of the same idea.
        styles.Chain(WritingStyleFamily.Paragraph, "Heading 1")
              .Select(s => s.Name)
              .ShouldBe(["Default", "Heading", "Heading 1"]);
    }

    [Fact]
    public void TheFamilyDefaultLeadsAChainThatDoesNotReachIt()
    {
        WritingStyles styles = new();
        styles.Add(new WritingStyle("Default", WritingStyleFamily.Paragraph, null, "default"));
        styles.Add(new WritingStyle("Orphan", WritingStyleFamily.Paragraph, null, "orphan"));
        styles.SetDefault(WritingStyleFamily.Paragraph, "Default");

        // A style with no parent still inherits the document's defaults — that is what "default"
        // means — so the chain has to reach them whether or not the file says so.
        styles.Chain(WritingStyleFamily.Paragraph, "Orphan")
              .Select(s => s.Name)
              .ShouldBe(["Default", "Orphan"]);
    }

    [Fact]
    public void TheDefaultIsNotRepeatedWhenTheChainAlreadyReachesIt()
    {
        WritingStyles styles = new();
        styles.Add(new WritingStyle("Default", WritingStyleFamily.Paragraph, null, "default"));
        styles.Add(new WritingStyle("Body", WritingStyleFamily.Paragraph, "Default", "body"));
        styles.SetDefault(WritingStyleFamily.Paragraph, "Default");

        styles.Chain(WritingStyleFamily.Paragraph, "Body")
              .Select(s => s.Name)
              .ShouldBe(["Default", "Body"]);
    }

    [Fact]
    public void TwoFamiliesMayShareAName()
    {
        WritingStyles styles = new();
        styles.Add(new WritingStyle("Standard", WritingStyleFamily.Paragraph, null, "paragraph"));
        styles.Add(new WritingStyle("Standard", WritingStyleFamily.Character, null, "character"));

        // ODF documents routinely have both. Resolving across families would take one style's parent
        // from the other family's chain, which is formatting from nowhere.
        styles.Find(WritingStyleFamily.Paragraph, "Standard")!.Properties.ShouldBe("paragraph");
        styles.Find(WritingStyleFamily.Character, "Standard")!.Properties.ShouldBe("character");
    }

    [Fact]
    public void ACircularParentChainStopsRatherThanLooping()
    {
        WritingStyles styles = new();
        styles.Add(new WritingStyle("A", WritingStyleFamily.Paragraph, "B", "a"));
        styles.Add(new WritingStyle("B", WritingStyleFamily.Paragraph, "A", "b"));

        // Malformed, but it does occur, and this walks a chain built from untrusted input.
        styles.Chain(WritingStyleFamily.Paragraph, "A")
              .Select(s => s.Name)
              .ShouldBe(["B", "A"]);
    }

    [Fact]
    public void AnUnknownStyleResolvesToTheFamilyDefaultAlone()
    {
        WritingStyles styles = new();
        styles.Add(new WritingStyle("Default", WritingStyleFamily.Paragraph, null, "default"));
        styles.SetDefault(WritingStyleFamily.Paragraph, "Default");

        // A document naming a style it does not define is common, and the defaults are still what
        // applies — so this is a fallback rather than an error.
        styles.Chain(WritingStyleFamily.Paragraph, "Missing")
              .Select(s => s.Name)
              .ShouldBe(["Default"]);
        styles.Find(WritingStyleFamily.Paragraph, "Missing").ShouldBeNull();
    }
}
