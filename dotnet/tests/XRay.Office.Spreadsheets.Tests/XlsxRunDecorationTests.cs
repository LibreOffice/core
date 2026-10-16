using System.Xml.Linq;
using XRay.Office.Ooxml;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// The underline and strikethrough a formatting run states about itself.
/// </summary>
/// <remarks>
/// An <c>rPr</c> is a full <c>CT_Font</c>, and for several rounds this reader took five of its
/// fields and dropped <c>u</c> and <c>strike</c> on the floor — so a run that says it is underlined
/// was parsed, discarded and drawn plain. Both properties were already read on the *cell* path and
/// already drawn by <c>SheetTextLayout</c>; only the run path was missing, which is why nothing
/// failed and nothing looked wrong.
///
/// Reading rather than drawing is the claim here, matching <c>SheetRichTextTests</c>: these need no
/// LibreOffice and no font.
/// </remarks>
public sealed class XlsxRunDecorationTests
{
    private static XElement RichString(params string[] runs)
        => XElement.Parse(
            $"""<si xmlns="{OoxmlNamespaces.SpreadsheetML}">{string.Concat(runs)}</si>""");

    private static string Run(string properties, string text)
        => $"<r><rPr>{properties}</rPr><t>{text}</t></r>";

    [Fact]
    public void ABareUnderlineElementMeansASingleLine()
    {
        IReadOnlyList<XlsxRichRun>? runs =
            XlsxRichRuns.Read(RichString(Run("<u/>", "underlined"), Run("", "plain")));

        runs.ShouldNotBeNull();
        runs[0].Font!.Underline.ShouldBe(SheetUnderline.SingleLine);
    }

    [Theory]
    [InlineData("single", SheetUnderline.SingleLine)]
    [InlineData("singleAccounting", SheetUnderline.SingleLine)]
    [InlineData("double", SheetUnderline.DoubleLine)]
    [InlineData("doubleAccounting", SheetUnderline.DoubleLine)]
    public void AStatedStyleNamesTheLine(string stated, SheetUnderline expected)
    {
        IReadOnlyList<XlsxRichRun>? runs =
            XlsxRichRuns.Read(RichString(Run($"""<u val="{stated}"/>""", "text")));

        runs.ShouldNotBeNull();
        runs[0].Font!.Underline.ShouldBe(expected);
    }

    [Fact]
    public void AnAbsentUnderlineIsNullRatherThanNone()
    {
        // The distinction the bug turned on. Absent means "keep what the run inherits"; an explicit
        // val="none" turns an inherited line off. Reading absent as None would have applied the
        // workbook default over the top of every run and left underlined text plain — which is the
        // failure this file exists to pin.
        IReadOnlyList<XlsxRichRun>? runs =
            XlsxRichRuns.Read(RichString(Run("<b/>", "bold only"), Run("", "plain")));

        runs.ShouldNotBeNull();
        runs[0].Font!.Underline.ShouldBeNull();

        IReadOnlyList<XlsxRichRun>? off =
            XlsxRichRuns.Read(RichString(Run("""<u val="none"/>""", "off"), Run("", "plain")));

        off.ShouldNotBeNull();
        off[0].Font!.Underline.ShouldBe(SheetUnderline.None);
    }

    [Fact]
    public void StrikethroughIsAToggleLikeBold()
    {
        IReadOnlyList<XlsxRichRun>? on =
            XlsxRichRuns.Read(RichString(Run("<strike/>", "struck"), Run("", "plain")));

        on.ShouldNotBeNull();
        on[0].Font!.StruckThrough.ShouldBe(true);

        IReadOnlyList<XlsxRichRun>? off =
            XlsxRichRuns.Read(RichString(Run("""<strike val="0"/>""", "not struck"), Run("", "plain")));

        off.ShouldNotBeNull();
        off[0].Font!.StruckThrough.ShouldBe(false);
    }

    [Fact]
    public void ARunStatingOnlyADecorationStillCountsAsStated()
    {
        // Read returns null when no run states anything, so a string whose only formatting is an
        // underline has to register — otherwise the runs are discarded wholesale before anything
        // downstream can apply them, which is the shape the original defect took.
        XlsxRichRuns.Read(RichString(Run("<u/>", "underlined"), Run("", "plain")))
            .ShouldNotBeNull("a stated underline is stated formatting");

        XlsxRichRuns.Read(RichString(Run("<strike/>", "struck"), Run("", "plain")))
            .ShouldNotBeNull("so is a stated strikethrough");

        XlsxRichRuns.Read(RichString(Run("", "one"), Run("", "two")))
            .ShouldBeNull("a string whose runs state nothing is not rich");
    }
}
