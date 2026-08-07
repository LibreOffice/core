using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Ooxml;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// An integer attribute written with a decimal point, which real files do and the schema does not
/// allow.
/// </summary>
/// <remarks>
/// <para>
/// <c>w:w="8730.0"</c>, <c>w:tblInd w:w="-85.0"</c>, <c>w:spacing w:line="360.0"</c> — 129 of them in
/// one corpus document, written by something that is not Word. LibreOffice reads every one: its
/// attribute list hands the string to <c>rtl_ustr_toInt32</c>, which parses as far as it can and
/// stops. <c>int.TryParse</c> rejects the lot, and the property then falls back to its default — for
/// a cell margin no margin, for a column no width, for a table indent no indent.
/// </para>
/// <para>
/// It truncates and does not round, which is the part worth measuring because rounding is the
/// plausible alternative. Three documents differing only in one attribute — <c>w:before</c> of 240,
/// 240.9 and 241 — put LibreOffice's second paragraph at 96.996, 96.996 and 97.046 pt: the decimal
/// one lands on 240.
/// </para>
/// <para>
/// Measured reach on the words track: <b>one</b> document of 134, and it is stated plainly rather
/// than dressed up. What justifies the change is that it makes the reader agree with the reference
/// reader on a class of malformed input, not the corpus arithmetic —
/// <c>words/batch-013/docx/template---tpr…docx</c> gains its cell margins and its table geometry and
/// still fails its page count for an unrelated reason.
/// </para>
/// </remarks>
public sealed class DecimalMeasureTests
{
    [Theory]
    [InlineData("240", 240)]
    [InlineData("240.9", 240)]
    [InlineData("-85.0", -85)]
    [InlineData("8730.0", 8730)]
    [InlineData("0.5", 0)]
    [InlineData("12pt", 12)]
    public void ADecimalIsTruncatedAtTheFirstNonDigit(string text, int expected)
    {
        Word.Integer(text, out int value).ShouldBeTrue();
        value.ShouldBe(expected);
    }

    [Theory]
    [InlineData("")]
    [InlineData("auto")]
    [InlineData("-")]
    [InlineData(".5")]
    public void SomethingWithNoLeadingDigitsIsStillUnreadable(string text)
    {
        Word.Integer(text, out int value).ShouldBeFalse();
        value.ShouldBe(0);
    }

    /// <summary>
    /// A number too long to hold saturates rather than wrapping into a small one.
    /// </summary>
    /// <remarks>
    /// The failure mode this rules out is the worst kind: a page width of 99999999999 twips silently
    /// becoming a small positive number and laying the document out plausibly and wrongly.
    /// </remarks>
    [Fact]
    public void ANumberTooLongToHoldSaturates()
    {
        Word.Integer("99999999999", out int value).ShouldBeTrue();
        value.ShouldBe(int.MaxValue);
    }

    /// <summary>
    /// And through the whole reader: the decimal document lays out exactly like the integral one.
    /// </summary>
    [Fact]
    public void TheDecimalDocumentMatchesItsIntegralTwin()
    {
        SecondParagraphTop("decimal-measure.docx")
            .ShouldBe(SecondParagraphTop("decimal-measure-integral.docx"));
    }

    /// <summary>And that is the 12 pt the attribute asks for, not the nothing a failed parse gives.</summary>
    [Fact]
    public void TheDecimalDocumentKeepsItsSpaceBefore()
    {
        SecondParagraphTop("decimal-measure.docx").ShouldBe(Length.FromTwips(240));
    }

    private static Length SecondParagraphTop(string name)
    {
        using IDocument document =
            new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        // The second paragraph's own upper spacing, which is what `w:before` states and what
        // `PlacedLine.UpperSpace` records after collapsing and the top-of-page rule.
        return pages.Pages[0].Lines[1].UpperSpace;
    }
}
