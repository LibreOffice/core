using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// A line whose only content is an as-character object is as wide as the object, so its alignment
/// moves it.
/// </summary>
/// <remarks>
/// <para>
/// An inline object is a <em>boundary</em> in the prefix table: it widens every prefix past the
/// position it occupies and none at or before it. That is right everywhere except at the end of the
/// text, where the range that would have paid for it does not exist — and a paragraph whose whole
/// content is a logo is exactly that shape, because ODF, RTF and WW8 put no character in the text
/// where a picture stands. Only OOXML does, which is why the defect was invisible in a DOCX.
/// </para>
/// <para>
/// Measured on one right-aligned picture-only paragraph, 145.5 pt wide on a text area running to
/// 540 pt, written five ways by LibreOffice 26.2.4.2: its own PDFs put the picture's left edge at
/// 394.55, 394.55, 394.60, 394.60 and 394.55 for the <c>.docx</c>, <c>.odt</c>, <c>.doc</c>,
/// <c>.rtf</c> and <c>.fodt</c>; ours gave 394.50 from the <c>.docx</c> and <b>540.00</b> from the
/// other four — the picture's whole width out, drawn from the right margin rightwards, half of it
/// off the paper on a wide enough logo. <c>probes/words-aschar-band/</c>.
/// </para>
/// <para>
/// Asserted through the layouter rather than through the measurement alone, because the alignment
/// offset is what the corpus feels: <c>AlignmentOffset</c> is <c>available - line.Width</c>, so a line
/// that measured nothing has full slack whatever it carries.
/// </para>
/// </remarks>
public sealed class TrailingInlineObjectAlignmentTests
{
    private static readonly Length Picture = Length.FromPoints(145.5);
    private static readonly Length Area = Length.FromPoints(468);

    /// <summary>A right-aligned picture-only line ends at the margin instead of starting there.</summary>
    [Fact]
    public void ARightAlignedPictureOnlyLineEndsAtTheMargin()
    {
        LaidOutParagraph laid = Laid(TextAlignment.End);

        laid.Lines.Count.ShouldBe(1);
        laid.Lines[0].Width.ShouldBe(Picture);
        laid.Lines[0].Left.ShouldBe(Area - Picture);
    }

    /// <summary>And a centred one is centred on the object, not on nothing.</summary>
    [Fact]
    public void ACentredPictureOnlyLineIsCentredOnTheObject()
    {
        LaidOutParagraph laid = Laid(TextAlignment.Centre);

        laid.Lines[0].Left.ShouldBe((Area - Picture) / 2);
    }

    /// <summary>
    /// The control: left alignment was already right, and stays exactly where it was.
    /// </summary>
    /// <remarks>
    /// Worth an assertion of its own because the fix adds width to a line that had none, and a width
    /// that reached the wrong consumer would move every inline picture in the corpus rather than only
    /// the aligned ones.
    /// </remarks>
    [Fact]
    public void ALeftAlignedPictureOnlyLineDoesNotMove()
    {
        Laid(TextAlignment.Start).Lines[0].Left.ShouldBe(Length.Zero);
    }

    private static LaidOutParagraph Laid(TextAlignment alignment)
    {
        OpenTypeFace face = Carlito();

        MeasuredParagraph measured = MeasuredParagraph.Measure(
            string.Empty, [], objects: [new InlineObject(0, Picture, Picture)]);

        return new ParagraphLayouter(face).Layout(
            measured,
            ParagraphFormat.Default with { Alignment = alignment },
            Area,
            emSize: Length.FromPoints(11));
    }

    private static OpenTypeFace Carlito()
    {
        foreach (string directory in new[]
                 {
                     "/usr/share/fonts/truetype/crosextra",
                     "/usr/share/fonts/truetype/liberation",
                     "/usr/share/fonts",
                 })
        {
            if (!Directory.Exists(directory)) continue;

            foreach (string found in Directory.EnumerateFiles(
                         directory, "Carlito-Regular.ttf", SearchOption.AllDirectories))
            {
                return OpenTypeFace.ReadFile(found).ShouldNotBeNull();
            }
        }

        Assert.Skip("Carlito is not installed; see check-env.sh");
        return null!;
    }
}
