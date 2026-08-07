using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Shouldly;

namespace Paperless.Text.Tests;

/// <summary>
/// A manual line break on the last character opens a line, and that line is empty.
/// </summary>
/// <remarks>
/// <para>
/// The fill loop runs <c>while (lineStart &lt; text.Length)</c>, so a break at the very end leaves
/// <c>lineStart == text.Length</c> and the loop exits without emitting the line the break asked
/// for. Nothing else in the paragraph looks wrong, which is why it survived: the text is all
/// there, in the right face, in the right place, and the paragraph is simply one line short.
/// </para>
/// <para>
/// Measured against LibreOffice 24.2.7.2 on two fixtures, one per family, each with four cases
/// differing only in where the break sits. <c>slide-trailing-break.pptx</c>, next paragraph's
/// baseline in points:
/// </para>
/// <code>
///                 reference   before   after
///   no break         24.01    24.01    24.01
///   trailing a:br    48.02    24.01    48.02
///   empty paragraph  48.02    45.61    45.61
///   break in middle  24.01    24.01    24.01
/// </code>
/// <para>
/// and <c>trailing-line-break.fodt</c>, the same four cases through the word-processing path,
/// where the reference gives 13.80, 27.60, 27.60 and 13.80 and we now give the same four to the
/// point. So a trailing break costs exactly what an explicit empty paragraph costs, in both
/// families.
/// </para>
/// <para>
/// <b>Only a line separator counts.</b> The characters <c>EndsLine</c> accepts include
/// <c>'\r'</c>, <c>'\n'</c> and U+2029, and a reader may be leaving one of those on the end of a
/// paragraph's text to mean the paragraph ends there — a line for those would lengthen every
/// paragraph in the corpus rather than the 612 in 72 of the 134 corpus DOCX that really do end
/// in a <c>w:br</c>.
/// </para>
/// </remarks>
public class TrailingLineBreakTests
{
    private static readonly Length Size = Length.FromPoints(12);
    private static readonly Length Wide = Length.FromPoints(400);

    [Fact]
    public void ATrailingLineSeparatorAddsAnEmptyLine()
    {
        LineFiller filler = Filler();

        filler.Fill("One", Size, Wide).Count.ShouldBe(1);

        List<TextLine> broken = filler.Fill("One\u2028", Size, Wide);
        broken.Count.ShouldBe(2);

        TextLine last = broken[1];
        last.Start.ShouldBe(last.End);
        last.Width.ShouldBe(Length.Zero);
        last.EndsParagraph.ShouldBeTrue();
    }

    /// <summary>
    /// Every separator a reader can produce for a manual break, and no others.
    /// </summary>
    /// <remarks>
    /// U+000B is what a binary PowerPoint writes and U+2028 is what the OOXML and ODF readers
    /// produce; U+000C and U+0085 round out UAX #14's set of line separators. The three excluded
    /// are the ones that can also mean "the paragraph ends".
    /// </remarks>
    [Theory]
    [InlineData('\u2028', 2)]
    [InlineData('\u000B', 2)]
    [InlineData('\u000C', 2)]
    [InlineData('\u0085', 2)]
    [InlineData('\n', 1)]
    [InlineData('\r', 1)]
    [InlineData('\u2029', 1)]
    public void OnlyASeparatorThatCannotEndAParagraphOpensTheLine(char separator, int expected)
        => Filler().Fill("One" + separator, Size, Wide).Count.ShouldBe(expected);

    /// <summary>
    /// A break in the middle is unaffected, and two trailing breaks give two empty lines.
    /// </summary>
    /// <remarks>
    /// The first half is the control the fixtures' fourth box is: the loop already handled a break
    /// that has text after it, and a rule that changed those would have moved every wrapped
    /// paragraph in the corpus. The second half is the loop's own doing rather than this rule's —
    /// the first of the two breaks ends a line the ordinary way and only the last is trailing.
    /// </remarks>
    [Fact]
    public void ABreakWithTextAfterItIsUnchangedAndTwoTrailingBreaksGiveTwoLines()
    {
        LineFiller filler = Filler();

        filler.Fill("One\u2028Two", Size, Wide).Count.ShouldBe(2);
        filler.Fill("One\u2028\u2028", Size, Wide).Count.ShouldBe(3);
    }

    /// <summary>A paragraph that is nothing but a break is two lines, not one.</summary>
    /// <remarks>
    /// The degenerate case, and the one that would crash a rule written as "look at the last line"
    /// rather than "look at the last character": the loop produces one line for the break itself
    /// and the rule adds the empty one after it.
    /// </remarks>
    [Fact]
    public void AParagraphOfNothingButABreakIsTwoLines()
        => Filler().Fill("\u2028", Size, Wide).Count.ShouldBe(2);

    /// <summary>Empty text is still one line, and the rule must not turn it into none or two.</summary>
    [Fact]
    public void EmptyTextIsOneLine()
        => Filler().Fill(string.Empty, Size, Wide).Count.ShouldBe(1);

    private static LineFiller Filler()
    {
        string? path = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(path is null, "Carlito is not installed; see check-env.sh");
        return new LineFiller(new TextMeasurer(OpenTypeFace.ReadFile(path!).ShouldNotBeNull()));
    }

    private static string? FindFont(string fileName)
    {
        foreach (string directory in new[]
                 {
                     "/usr/share/fonts/truetype/crosextra",
                     "/usr/share/fonts/truetype",
                     "/usr/share/fonts",
                 })
        {
            if (!Directory.Exists(directory)) continue;

            string? found = Directory
                .EnumerateFiles(directory, fileName, SearchOption.AllDirectories)
                .FirstOrDefault();
            if (found is not null) return found;
        }

        return null;
    }
}
