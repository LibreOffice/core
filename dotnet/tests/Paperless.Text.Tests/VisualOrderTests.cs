using Paperless.Text.Itemisation;
using Shouldly;

namespace Paperless.Text.Tests;

/// <summary>
/// The two pieces of the itemiser that drawing needs: rule L2 over anything, and the guard that
/// keeps a paragraph which cannot reorder out of the reordering path altogether.
/// </summary>
/// <remarks>
/// <see cref="TextItemiser.InVisualOrder"/> is tested beside the rest of the itemisation; this is
/// about the generic form of the same rule, which exists because the glyph runs a line draws are the
/// sub-runs cut again at every change of font, size and colour — and two implementations of L2 would
/// be two chances to disagree about a line that reorders.
/// </remarks>
public sealed class VisualOrderTests
{
    /// <summary>A part of a line: what it is does not matter, only the level it carries.</summary>
    private readonly record struct Part(string Text, byte Level);

    [Fact]
    public void ReorderingSubRunsAndReorderingTheRunsTheyAreCutIntoAgree()
    {
        const string Text = "Start שלום 123 עולם end.";

        List<TextItem> items = TextItemiser.Itemise(Text);
        List<TextItem> visual = TextItemiser.InVisualOrder(items);

        // The same items through the generic entry point, which is the one drawing uses.
        List<TextItem> generic = [.. items];
        TextItemiser.ReorderVisually(generic, item => item.Level);

        generic.ShouldBe(visual);
    }

    [Fact]
    public void ASubRunCutInTwoKeepsItsHalvesInTheOrderItsLevelPutsThem()
    {
        // What a bold word inside a right-to-left phrase becomes: one level run, two draw runs. The
        // halves of a right-to-left run reverse with it, which is the whole reason the runs rather
        // than the sub-runs have to go through L2 — reordering the sub-runs and then drawing each
        // one's pieces left to right would put the bold word on the wrong side of its phrase.
        List<Part> parts =
        [
            new("Latin", 0),
            new("hebrew-first", 1),
            new("hebrew-bold", 1),
            new("tail", 0),
        ];

        TextItemiser.ReorderVisually(parts, part => part.Level);

        parts.Select(part => part.Text).ShouldBe(
            ["Latin", "hebrew-bold", "hebrew-first", "tail"]);
    }

    [Fact]
    public void ALineWithNothingRightToLeftOnItIsLeftAlone()
    {
        List<Part> parts = [new("one", 0), new("two", 0), new("three", 0)];

        TextItemiser.ReorderVisually(parts, part => part.Level);

        parts.Select(part => part.Text).ShouldBe(["one", "two", "three"]);
    }

    [Theory]
    [InlineData("Plain Latin prose, with 123 numbers and (brackets).")]
    [InlineData("")]
    [InlineData("Ελληνικά and Latin together")]
    public void AParagraphThatCannotReorderIsNotWorthItemising(string text)
    {
        // The guard that keeps the common case free. Everything here resolves flat at a left-to-right
        // base, so drawing must not cut the paragraph into sub-runs at all — a run split at a
        // boundary it does not need loses the shaping context across it and measures very slightly
        // wide, which is enough to move a line break.
        TextItemiser.MayReorder(text).ShouldBeFalse();
    }

    [Theory]
    [InlineData("Start שלום end.")]
    [InlineData("مرحبا")]
    [InlineData("Latin with ‏a mark in it")]
    public void AnythingThatCouldRaiseALevelIsWorthItemising(string text)
    {
        TextItemiser.MayReorder(text).ShouldBeTrue();
    }

    [Fact]
    public void ARightToLeftParagraphIsAlwaysWorthItemisingWhateverItHolds()
    {
        // Even one of nothing but Latin: its base level is odd, so its runs sit at level two and its
        // line starts at the right margin rather than the left.
        TextItemiser.MayReorder("Latin only", BidiDirection.RightToLeft).ShouldBeTrue();
    }
}
