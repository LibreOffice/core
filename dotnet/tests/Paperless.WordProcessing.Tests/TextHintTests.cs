using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Tests the interval-tagged character formatting the layout-facing model stores.
/// </summary>
/// <remarks>
/// This is the part of the model that is an algorithm rather than a container, so it is the part
/// worth pinning: overlapping intervals, the resolution order between kinds, the placeholder
/// characters that occupy a position without being text, and what an edit does to intervals that
/// straddle it.
/// </remarks>
public class TextHintTests
{
    [Fact]
    public void OverlappingRangesAreStoredAsTheyWereAppliedRatherThanSplit()
    {
        TextHints hints = new();
        hints.Add(TextHintKind.DirectFormatting, 0, 10, "bold");
        hints.Add(TextHintKind.Hyperlink, 5, 15, "https://example.invalid/");

        // Three runs' worth of formatting, two intervals. Storing runs instead would split both at 5
        // and 10 and produce three fragments, none of which is anything the document said.
        hints.Count.ShouldBe(2);
        hints.At(3).Select(h => h.Kind).ShouldBe([TextHintKind.DirectFormatting]);
        hints.At(7).Select(h => h.Kind).ShouldBe(
            [TextHintKind.Hyperlink, TextHintKind.DirectFormatting]);
        hints.At(12).Select(h => h.Kind).ShouldBe([TextHintKind.Hyperlink]);
    }

    [Fact]
    public void ResolutionOrderPutsDirectFormattingLast()
    {
        TextHints hints = new();
        hints.Add(TextHintKind.DirectFormatting, 0, 5, "direct");
        hints.Add(TextHintKind.CharacterStyle, 0, 5, "Emphasis");
        hints.Add(TextHintKind.Hyperlink, 0, 5, "target");

        // Lowest priority first, so a caller applies them in order and the last wins. Direct
        // formatting is what the user typed, so it must override both the character style and the
        // link's own formatting — reversing this loses ad-hoc bold inside a styled hyperlink.
        hints.At(0).Select(h => h.Kind).ShouldBe(
            [TextHintKind.CharacterStyle, TextHintKind.Hyperlink, TextHintKind.DirectFormatting]);
    }

    [Fact]
    public void AnAnchorIsExactlyOneCharacterWideHoweverItIsAdded()
    {
        TextHints hints = new();

        // A caller computing a range from a byte offset can be off by one, and an anchor whose width
        // is not one would attribute a character it does not stand for.
        hints.Add(new TextHint(TextHintKind.NoteAnchor, 4, 40, "note"));
        hints.All[0].Length.ShouldBe(1);
        hints.All[0].End.ShouldBe(5);

        hints.AddAnchor(TextHintKind.Field, 9, "PAGE");
        hints.At(TextHintKind.Field, 9)!.Value.Value.ShouldBe("PAGE");
        hints.At(TextHintKind.Field, 10).ShouldBeNull();
    }

    [Fact]
    public void AnEmptyRangeIsRejectedRatherThanStored()
    {
        TextHints hints = new();
        hints.Add(TextHintKind.DirectFormatting, 5, 5, "nothing");
        hints.Add(TextHintKind.DirectFormatting, -3, 2, "before the start");

        hints.Count.ShouldBe(0);
    }

    [Fact]
    public void BoundariesAreWhereTheFormattingChanges()
    {
        TextHints hints = new();
        hints.Add(TextHintKind.DirectFormatting, 3, 7, "bold");
        hints.Add(TextHintKind.CharacterStyle, 5, 12, "Emphasis");

        // This is how intervals become runs: between two boundaries the formatting is uniform by
        // construction, so a renderer reads the hints once per span rather than once per character.
        hints.Boundaries(20).ShouldBe([0, 3, 5, 7, 12, 20]);
    }

    [Fact]
    public void AParagraphWithNoFormattingStillHasOneSpan()
    {
        TextHints hints = new();
        hints.Boundaries(8).ShouldBe([0, 8]);
        hints.Boundaries(0).ShouldBe([0]);
    }

    [Fact]
    public void InsertingInsideARangeExtendsItAndInsertingBeforeItDoesNot()
    {
        TextHints hints = new();
        hints.Add(TextHintKind.DirectFormatting, 4, 8, "bold");

        // Typing inside a bold run is bold; typing immediately before it is not. Both are what a
        // user expects, and only the first is what a naive "shift everything after" would do.
        hints.Insert(6, 3);
        hints.All[0].Start.ShouldBe(4);
        hints.All[0].End.ShouldBe(11);

        hints.Insert(4, 2);
        hints.All[0].Start.ShouldBe(6);
        hints.All[0].End.ShouldBe(13);
    }

    [Fact]
    public void InsertingInsideAnAnchorDoesNotStretchIt()
    {
        TextHints hints = new();
        hints.AddAnchor(TextHintKind.Field, 5, "PAGE");

        // An anchor's width is the placeholder character it stands for, so it cannot grow: a field
        // that covered two characters would claim one that belongs to the text.
        hints.Insert(5, 4);
        hints.All[0].Length.ShouldBe(1);
        hints.All[0].Start.ShouldBe(9);
    }

    [Fact]
    public void DeletingTrimsWhatItOverlapsAndDropsWhatItConsumes()
    {
        TextHints hints = new();
        hints.Add(TextHintKind.DirectFormatting, 0, 4, "before");
        hints.Add(TextHintKind.CharacterStyle, 4, 8, "inside");
        hints.Add(TextHintKind.Hyperlink, 8, 12, "after");

        hints.Delete(4, 4);

        // The one the deletion consumed is gone rather than left as an empty interval, and the one
        // after it has moved back by exactly what was removed.
        hints.Count.ShouldBe(2);
        hints.All[0].Kind.ShouldBe(TextHintKind.DirectFormatting);
        hints.All[0].End.ShouldBe(4);
        hints.All[1].Kind.ShouldBe(TextHintKind.Hyperlink);
        hints.All[1].Start.ShouldBe(4);
        hints.All[1].End.ShouldBe(8);
    }

    [Fact]
    public void DeletingPartOfARangeShortensIt()
    {
        TextHints hints = new();
        hints.Add(TextHintKind.DirectFormatting, 2, 12, "bold");

        hints.Delete(5, 3);
        hints.All[0].Start.ShouldBe(2);
        hints.All[0].End.ShouldBe(9);
    }

    [Fact]
    public void OverlappingFindsEveryHintTouchingARange()
    {
        TextHints hints = new();
        hints.Add(TextHintKind.DirectFormatting, 0, 3, "a");
        hints.Add(TextHintKind.DirectFormatting, 3, 6, "b");
        hints.Add(TextHintKind.DirectFormatting, 6, 9, "c");

        // Half-open on both sides: a hint ending exactly where the range begins does not touch it.
        hints.Overlapping(3, 6).Select(h => h.Value).ShouldBe(["b"]);
        hints.Overlapping(2, 7).Select(h => h.Value).ShouldBe(["a", "b", "c"]);
    }

    [Fact]
    public void AParagraphCountsItsPlaceholdersInItsLengthButNotInItsText()
    {
        WritingParagraph paragraph = new();
        paragraph.Append("See note");
        int anchor = paragraph.AppendAnchor();
        paragraph.Hints.AddAnchor(TextHintKind.NoteAnchor, anchor, "1");
        paragraph.Append(" here.");

        // The placeholder is part of the paragraph's length, because every offset in the document —
        // a bookmark, a tracked change, a formatting run — is counted against the same string. It is
        // not part of the text a reader sees.
        paragraph.Length.ShouldBe("See note".Length + 1 + " here.".Length);
        paragraph.GetText().ShouldBe("See note here.\n");
        paragraph.Hints.At(TextHintKind.NoteAnchor, anchor)!.Value.Value.ShouldBe("1");
    }

    [Fact]
    public void EditingAParagraphMovesItsFormattingWithItsText()
    {
        WritingParagraph paragraph = new();
        paragraph.Append("hello world");
        paragraph.Hints.Add(TextHintKind.DirectFormatting, 6, 11, "bold");

        paragraph.Insert(0, "oh, ");
        paragraph.Text.ShouldBe("oh, hello world");
        paragraph.Hints.All[0].Start.ShouldBe(10);

        paragraph.Delete(0, 4);
        paragraph.Text.ShouldBe("hello world");
        paragraph.Hints.All[0].Start.ShouldBe(6);
        paragraph.Hints.All[0].End.ShouldBe(11);
    }
}
