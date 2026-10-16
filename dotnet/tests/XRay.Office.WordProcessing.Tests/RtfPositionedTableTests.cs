using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// RTF's <em>Positioned Wrapped Tables</em> — <c>\tpvpg</c>, <c>\tposy</c> and their family — name a
/// place on the page rather than a place in the text, exactly as OOXML's <c>w:tblpPr</c> does.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice's RTF tokeniser writes every one of these words into <c>w:tblpPr</c> and hands it to the
/// same <c>DomainMapper</c> the DOCX path uses — <c>sw/source/writerfilter/rtftok/rtfdispatchflag.cxx</c>
/// :39-115 for the anchors and the specs, <c>rtfdispatchvalue.cxx</c>:1802-1837 for <c>\tposx</c>,
/// <c>\tposy</c> and the four <c>\tdfrmtxt*</c> distances. So the two readers of one document are
/// supposed to reach <see cref="PageTable.IsPositioned"/> by the same route, and until this round the
/// RTF one did not reach it at all: `IsPositioned` was set from `word/document.xml` and nowhere else.
/// </para>
/// <para>
/// What it costs is a page count rather than a look. A fly is not in the flow, so the paragraphs after
/// the table start where it started; read as an ordinary table it consumes the flow and pushes them off
/// the sheet. Measured on the <c>.rtf</c> column of <c>/home/user/corpus-odf</c> against 26.2.4.2:
/// <c>089_Printable_Graph_Paper_Template_Simlpe_Format</c> went from 2 pages to the reference's 1 with
/// its 10 characters unchanged, and 080 and 087 with it. <b>1135 of the 22 720 row definitions in the
/// column state a position, in 46 of its 338 documents</b> — and not one row states a
/// <c>\tdfrmtxt*</c> distance without one, which is why treating those four as position-bearing (as
/// LibreOffice does) cannot bite here either way. See <c>probes/rtf-gate-r71/</c>.
/// </para>
/// </remarks>
public sealed class RtfPositionedTableTests
{
    /// <summary>A page-anchored row puts its table <c>\tposy</c> below the sheet's own top edge.</summary>
    [Fact]
    public void APageAnchoredTableIsDrawnAtItsStatedOffsetFromTheSheet()
    {
        WordProcessingPages pages = Lay(@"\tpvpg\tphmrg\tposxc\tposy2880");

        PlacedTable table = pages.Pages[0].Tables.ShouldHaveSingleItem();
        table.Area.Y.ShouldBe(Length.FromTwips(2880));
    }

    /// <summary>
    /// And the flow is left where it was: the paragraph after the table stays at the top margin.
    /// </summary>
    /// <remarks>
    /// The assertion that carries the page count. Without it the paragraph starts under the table.
    /// </remarks>
    [Fact]
    public void AFlyDoesNotPushTheFlowDown()
    {
        WordProcessingPages pages = Lay(@"\tpvpg\tphmrg\tposxc\tposy2880");

        // Body-relative, so nought is the top margin: the paragraph starts where it would with no
        // table in the document at all.
        pages.Pages[0].Lines[0].Top.ShouldBe(Length.Zero);
    }

    /// <summary>A row stating none of the family is an ordinary table in the flow.</summary>
    /// <remarks>
    /// The control. It is what says the reading is the position words and not the table.
    /// </remarks>
    [Fact]
    public void ATableStatingNoPositionStaysInTheFlow()
    {
        WordProcessingPages pages = Lay(string.Empty);

        PlacedTable table = pages.Pages[0].Tables.ShouldHaveSingleItem();
        table.Area.Y.ShouldBe(Length.FromTwips(1440));
        pages.Pages[0].Lines[0].Top.ShouldBeGreaterThan(Length.Zero);
    }

    /// <summary>
    /// <c>\tposxc</c> centres the table in the text area rather than leaving it at its indent.
    /// </summary>
    [Fact]
    public void TheHorizontalSpecIsRead()
    {
        Length centred = Lay(@"\tpvpg\tphmrg\tposxc\tposy2880")
            .Pages[0].Tables.ShouldHaveSingleItem().Area.X;
        Length plain = Lay(@"\tpvpg\tphmrg\tposy2880")
            .Pages[0].Tables.ShouldHaveSingleItem().Area.X;

        centred.ShouldBeGreaterThan(plain);
    }

    /// <summary>
    /// A row anchored horizontally to the <em>page</em> keeps its indent, because the rectangle
    /// <c>\tphpg</c> names is one <see cref="PageTable.HorizontalPosition"/> cannot express.
    /// </summary>
    /// <remarks>
    /// The same exception <c>DocxLayoutSource.HorizontalPositionOf</c> makes for
    /// <c>w:horzAnchor="page"</c>, and it has to be made in both readers or one document renders two
    /// ways.
    /// </remarks>
    [Fact]
    public void APageAnchoredRowIgnoresItsHorizontalSpec()
    {
        Length onThePage = Lay(@"\tpvpg\tphpg\tposxc\tposy2880")
            .Pages[0].Tables.ShouldHaveSingleItem().Area.X;
        Length onTheMargin = Lay(@"\tpvpg\tphmrg\tposxc\tposy2880")
            .Pages[0].Tables.ShouldHaveSingleItem().Area.X;

        onThePage.ShouldBeLessThan(onTheMargin);
    }

    /// <summary>
    /// <c>\tdfrmtxtLeft</c> alone writes a <c>w:tblpPr</c> and no anchor with it, and a table anchored
    /// to the text is left in the flow.
    /// </summary>
    /// <remarks>
    /// Faithful to <c>rtfdispatchvalue.cxx</c>:1814-1837, which writes each of the four distances into
    /// <c>w:tblpPr</c> and nothing else into it. The resulting table is positioned and its vertical
    /// origin is <see cref="FrameVerticalOrigin.Paragraph"/> — which is where the flow already is, so
    /// <c>Paginator</c> deliberately leaves it there rather than floating it and losing the space it
    /// owes above and below. Exactly what the DOCX reader does for <c>w:vertAnchor="text"</c>. No corpus
    /// row reaches this arm — all 1135 positioned rows state a real anchor as well.
    /// </remarks>
    [Fact]
    public void ADistanceFromTextStatesNoAnchorAndSoDoesNotMoveTheTable()
    {
        Lay(@"\tdfrmtxtLeft180").Pages[0].Lines[0].Top
            .ShouldBe(Lay(string.Empty).Pages[0].Lines[0].Top);
    }

    /// <summary>
    /// The six words LibreOffice tokenises and then drops make no position at all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>\tposxl</c>, <c>\tposxi</c> and <c>\tposxo</c> are recognised by
    /// <c>rtftokenizer.cxx</c>:1619-1633 and handled by neither <c>dispatchFloatingTableFlag</c> — which
    /// names only <c>\tposxc</c> and <c>\tposxr</c> — nor anything else, and <c>\tposyt</c>,
    /// <c>\tposyil</c>, <c>\tposyin</c>, <c>\tposyout</c>, <c>\tposnegx</c> and <c>\tposnegy</c> the
    /// same. So none of them reaches a <c>w:tblpPr</c>, none makes a table a fly, and none states an
    /// alignment the reference applies.
    /// </para>
    /// <para>
    /// The test that says the reading is LibreOffice's dispatch table rather than the RTF
    /// specification's prose: read as the prose reads them, <c>\tposxl</c> would left-align a table the
    /// reference leaves at its indent, and any of the six alone would lift a table out of a flow the
    /// reference leaves it in. Not one of the nine occurs in the corpus's 22 720 row definitions
    /// (<c>probes/rtf-gate-r71/census.py</c>), so only this test holds the line.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(@"\tposxl")]
    [InlineData(@"\tposxi")]
    [InlineData(@"\tposxo")]
    [InlineData(@"\tposyt")]
    [InlineData(@"\tposyil")]
    [InlineData(@"\tposnegy720")]
    public void AWordLibreOfficeDropsMakesNoPosition(string word)
    {
        WordProcessingPages pages = Lay(word);
        WordProcessingPages bare = Lay(string.Empty);

        pages.Pages[0].Tables.ShouldHaveSingleItem().Area.X
            .ShouldBe(bare.Pages[0].Tables.ShouldHaveSingleItem().Area.X);
        pages.Pages[0].Lines[0].Top.ShouldBe(bare.Pages[0].Lines[0].Top);
    }

    /// <summary>
    /// Lays out one A4 page holding a one-cell table with the given row-position words, followed by a
    /// paragraph.
    /// </summary>
    private static WordProcessingPages Lay(string position)
    {
        string rtf =
            @"{\rtf1\ansi\deff0{\fonttbl{\f0\froman Liberation Serif;}}"
            + @"\paperw11906\paperh16838\margl1440\margr1440\margt1440\margb1440\sectd"
            + @"\trowd" + position + @"\trgaph0\cellx5000"
            + @"\pard\intbl One cell.\cell\row"
            + @"\pard\plain After the table.\par}";

        using DocumentSource source = DocumentSource.FromStream(
            new MemoryStream(Encoding.ASCII.GetBytes(rtf)), "positioned.rtf");
        using IDocument document = new WordProcessingReader().Read(source);
        return (WordProcessingPages)((IPaginatedDocument)document).Layout();
    }
}
