using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Where a <c>HYPERLINK</c> field leads, read from its instruction.
/// </summary>
/// <remarks>
/// <see cref="FieldInstructions.HyperlinkTarget"/> answers the quoted text after the name, which for a
/// <c>\l</c> link is a bookmark and not an address. <see cref="FieldInstructions.HyperlinkReference"/> reads
/// the switches, which is what a PDF needs to tell a link to the web from a link within the document.
/// </remarks>
public sealed class HyperlinkFieldTests
{
    [Theory]
    [InlineData("HYPERLINK \"https://example.com/a\"", "https://example.com/a")]
    [InlineData("  HYPERLINK https://example.com/a  ", "https://example.com/a")]
    [InlineData("HYPERLINK \"https://example.com/a b\" \\o \"A tooltip\"", "https://example.com/a b")]
    [InlineData("hyperlink \"mailto:ada@example.com\"", "mailto:ada@example.com")]
    [InlineData("HYPERLINK \\l \"_Toc123\"", "#_Toc123")]
    [InlineData("HYPERLINK \\l _Toc123", "#_Toc123")]
    [InlineData("HYPERLINK \"report.docx\" \\l \"Chapter2\"", "report.docx#Chapter2")]
    [InlineData("HYPERLINK \\o \"Tip\" \\l \"Here\"", "#Here")]
    [InlineData("HYPERLINK \\t \"_blank\" \"https://example.com/\"", "https://example.com/")]
    public void AnInstructionNamesWhereItLeads(string instruction, string expected)
        => FieldInstructions.HyperlinkReference(instruction).ShouldBe(expected);

    [Theory]
    [InlineData(null)]
    [InlineData("")]
    [InlineData("PAGE")]
    [InlineData("PAGEREF _Toc123 \\h")]
    [InlineData("HYPERLINKED \"https://example.com/\"")]
    [InlineData("HYPERLINK")]
    [InlineData("HYPERLINK \\o \"Only a tooltip\"")]
    public void AnythingElseLeadsNowhere(string? instruction)
        => FieldInstructions.HyperlinkReference(instruction).ShouldBeNull();
}
