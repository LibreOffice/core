using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A <c>\sect</c> that follows a table, which is how LibreOffice's own RTF export writes a
/// document chopped into one-table sections.
/// </summary>
/// <remarks>
/// <para>
/// RTF marks no end to a table: a paragraph back at the enclosing level is what closes one, and
/// until then the rows are still accumulating. A section break written straight after the last row
/// — <c>…\row\pard \sect\sectd…\sbkpage</c> — therefore arrives while the table is open, and the
/// section index the finished table is stamped with is read when it closes, which is after the
/// break. Every block on both sides then claims one section, and the page break the section asked
/// for never happens.
/// </para>
/// <para>
/// <c>Annex-10…GCAA.rtf</c> is 156 sections written in exactly that shape. Reach over the
/// converted corpus's 336 scoreable <c>.rtf</c> is five documents gained and two lost; see
/// <c>dotnet/probes/rtf-page-r77/results.md</c>.
/// </para>
/// </remarks>
public sealed class RtfSectionBreakTests
{
    /// <summary>A <c>\sbkpage</c> section written straight after <c>\row\pard</c> starts a page.</summary>
    /// <remarks>
    /// Measured against 26.2.4.2 on this exact file: two pages, the second table's first cell at
    /// y = 94.0 pt. This tree drew one page with the second table at 105.5.
    /// </remarks>
    [Fact]
    public void ASectionBreakAfterATableStartsANewPage()
        => Pages(TableRow("ONE", "TWO") + @"\pard \sect" + Section + TableRow("AAA", "BBB"))
            .Count.ShouldBe(2);

    /// <summary>And the control: without the break the two tables share a page.</summary>
    [Fact]
    public void WithoutTheBreakTheTwoTablesShareAPage()
        => Pages(TableRow("ONE", "TWO") + @"\pard " + TableRow("AAA", "BBB"))
            .Count.ShouldBe(1);

    private const string Section =
        @"\sectd\pgwsxn11906\pghsxn16838\marglsxn1300\margrsxn1280"
        + @"\margtsxn1620\margbsxn1200\sbkpage";

    private static string TableRow(string first, string second)
        => @"\trowd\trql\trleft118\cellx4000\cellx8000"
           + $@"\pard\plain\intbl\fs20 {first}\cell\pard\plain\intbl\fs20 {second}\cell\row";

    private static WordProcessingPages Pages(string body)
    {
        string rtf =
            @"{\rtf1\ansi\deff0{\fonttbl{\f0\froman Liberation Serif;}}"
            + @"\paperw11906\paperh16838\margl1300\margr1280\margt1620\margb1200\sectd\sbknone"
            + body
            + @"\pard\par}";

        using DocumentSource source = DocumentSource.FromStream(
            new MemoryStream(Encoding.ASCII.GetBytes(rtf)), "section.rtf");
        using IDocument document = new WordProcessingReader().Read(source);

        return (WordProcessingPages)((IPaginatedDocument)document).Layout();
    }
}
