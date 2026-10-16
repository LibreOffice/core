using System.Xml.Linq;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Model;
using XRay.Office.WordProcessing.OpenDocument;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A <c>text:section</c> is a Writer text section, and its columns and indents are laid out.
/// </summary>
/// <remarks>
/// <para>
/// A multi-column stretch in the middle of a page is not a page style in Writer's model, and every
/// Word-family importer says so: <c>SectionPropertyMap::CloseSectionGroup</c> — <em>"prefer setting
/// column properties into a section, not a page style if at all possible"</em> — calls
/// <c>appendTextSectionAfter</c> (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:1905-1913), and
/// <c>wwSectionManager::InsertSection</c> builds the same object, putting
/// <c>rSection.GetPageLeft() - nPageLeft</c> on it as an <c>SvxLRSpaceItem</c> before <c>SetCols</c>
/// (<c>sw/source/filter/ww8/ww8par6.cxx</c>:735-745). LibreOffice's ODF export writes that object back
/// out as a <c>text:section</c> whose section style carries <c>style:columns</c> and, where the margins
/// differ from the page's, <c>fo:margin-left</c>/<c>fo:margin-right</c>.
/// </para>
/// <para>
/// The ODF reader walked straight through the element, so a two-column stretch was laid out one column
/// wide across the whole measure. <b>33 of the 338 converted <c>.odt</c> hold such a section</b> —
/// 63 columned sections of 139 — and none of the 307 <c>.ods</c> or 302 <c>.odp</c> holds one at all.
/// <c>probes/odt-startx-r88/results.md</c>.
/// </para>
/// <para>
/// The fixture is a DOCX converted by 26.2.4.2, not a hand-written flat ODF, for the reason
/// <c>dotnet/CLAUDE.md</c> records: LibreOffice's own writers are the only source of a realistic
/// <c>text:section</c>. <c>probes/odt-startx-r88/gen-section-columns.py</c> builds the DOCX.
/// </para>
/// </remarks>
public sealed class OdtTextSectionTests
{
    /// <summary>How close a drawn position has to be to the reference's, in points.</summary>
    /// <remarks>
    /// The two renderers' page origins differ by a constant 0.10 pt on this corpus — every line of the
    /// fixture is drawn at 72.00 here against 26.2.4.2's 72.10 — so a quarter of a point separates
    /// "the same place" from one column's worth of error, which is 252 pt.
    /// </remarks>
    private const double Tolerance = 0.25;

    /// <summary>A section that changes nothing about the geometry is not a section at all.</summary>
    /// <remarks>
    /// LibreOffice's exporter writes a <c>text:section</c> for a protected region, an index and a
    /// linked file as well as for a columned one, and 76 of the corpus's 139 state one column and no
    /// indents. Answering null for those is what keeps them on the walk's transparent path, where they
    /// have always been and belong.
    /// </remarks>
    [Fact]
    public void ASectionThatChangesNothingIsReadAsNothing()
        => OdfSectionGeometry.Read(Styles(@"<style:columns fo:column-count=""1"" fo:column-gap=""0in""/>"), "S")
            .ShouldBeNull();

    /// <summary>The column count and the gap, which is the shape LibreOffice's exporter writes.</summary>
    [Fact]
    public void ColumnsAndTheirGapAreRead()
    {
        OdfSectionGeometry geometry = OdfSectionGeometry.Read(
            Styles(@"<style:columns fo:column-count=""2"" fo:column-gap=""0.5in""/>"), "S")!.Value;

        geometry.Columns.ShouldBe(2);
        geometry.ColumnGap.Points.ShouldBe(36.0, 0.01);
    }

    /// <summary>
    /// The indents are read, and they are a difference from the page's rather than a replacement.
    /// </summary>
    /// <remarks>
    /// <c>644730BRI0mna000BOX361539B00public0.odt</c> is the witness: its page layout states
    /// <c>fo:margin-left="0.4165in"</c> and its section <c>fo:margin-left="0.5835in"</c>, which sum to
    /// the round inch its <c>.doc</c> original declares for that section, and 26.2.4.2 draws its 203
    /// first-column lines at x = 72.1. Reading the section's indent as an absolute margin would put
    /// them at 42.
    /// </remarks>
    [Fact]
    public void TheSectionsOwnIndentsAreRead()
    {
        OdfSectionGeometry geometry = OdfSectionGeometry.Read(
            Styles(@"<style:columns fo:column-count=""1""/>", @"fo:margin-left=""0.5835in"" fo:margin-right=""0.6835in"""),
            "S")!.Value;

        geometry.IndentLeft.Points.ShouldBe(42.01, 0.05);
        geometry.IndentRight.Points.ShouldBe(49.21, 0.05);
    }

    /// <summary>
    /// Balancing is the absent state, because ODF states the negative.
    /// </summary>
    /// <remarks>
    /// <c>text:dont-balance-text-columns</c> is <c>SwFormatNoBalancedColumns</c>, which both Word
    /// importers set for a section that a page break follows and for the last section of a document
    /// (<c>ww8par.cxx</c>:4570-4578, <c>dmapper/PropertyMap.cxx</c>:1919). So a converted file usually
    /// states it and a file Writer itself made usually does not.
    /// </remarks>
    [Theory]
    [InlineData(@" text:dont-balance-text-columns=""true""", false)]
    [InlineData("", true)]
    public void BalancingIsTheAbsentState(string stated, bool balances)
        => OdfSectionGeometry.Read(
                Styles(@"<style:columns fo:column-count=""2"" fo:column-gap=""0.5in""/>", stated), "S")!
            .Value.BalancesColumns.ShouldBe(balances);

    /// <summary>
    /// A stated <c>fo:column-gap</c> makes the columns even, whatever widths the file also states.
    /// </summary>
    /// <remarks>
    /// The presence of the attribute is the whole test — <c>bAutomatic</c>,
    /// <c>XMLTextColumnsContext.cxx</c>:216-222 — and the per-column descriptions are then dropped on
    /// the floor by <c>endFastElement</c>'s <c>!bAutomatic</c> guard (:268-315). <b>45 of the corpus's
    /// 125 gap-stating <c>style:columns</c> also state unequal <c>style:rel-width</c></b>, so reading
    /// the widths there would move 45 sections that LibreOffice draws even.
    /// </remarks>
    [Fact]
    public void AStatedGapMakesTheColumnsEvenHoweverTheyAreDescribed()
        => OdfSectionGeometry.Read(
                Styles("""
                       <style:columns fo:column-count="2" fo:column-gap="0.5in">
                         <style:column style:rel-width="3240*" fo:start-indent="0in" fo:end-indent="0.25in"/>
                         <style:column style:rel-width="6120*" fo:start-indent="0.25in" fo:end-indent="0in"/>
                       </style:columns>
                       """), "S")!
            .Value.RulerFor(Length.FromPoints(468)).ShouldBeNull();

    /// <summary>
    /// Per-column widths with no gap are apportioned, and a <c>rel-width</c> is the column's outer width.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>absrc-pac-01-info-note-en.odt</c>'s section states 3240 and 6120 against 0.25 inch indents on
    /// the inner edges, so of a 468 pt measure the first column's share is
    /// 3240 / 9360 × 468 = 162.0 pt and its text width is 162.0 − 18 = 144.0, the gap is 18 + 18 = 36,
    /// and the second takes the remaining 306 less its own 18. Splitting the measure evenly instead —
    /// which is what this reader did — puts the second column 45 pt to the left of where 26.2.4.2 draws
    /// it.
    /// </para>
    /// <para>
    /// <b>The whole reach of this arm is five <c>style:columns</c> in four documents</b>, and they are
    /// the three <c>150_5300_13</c> revisions and this one.
    /// </para>
    /// </remarks>
    [Fact]
    public void PerColumnWidthsWithNoGapAreApportioned()
    {
        ColumnRuler ruler = OdfSectionGeometry.Read(
            Styles("""
                   <style:columns fo:column-count="2">
                     <style:column style:rel-width="3240*" fo:start-indent="0in" fo:end-indent="0.25in"/>
                     <style:column style:rel-width="6120*" fo:start-indent="0.25in" fo:end-indent="0in"/>
                   </style:columns>
                   """), "S")!.Value.RulerFor(Length.FromPoints(468))!;

        ruler.Count.ShouldBe(2);
        ruler.WidthAt(0).Points.ShouldBe(144.0, 0.05);
        ruler.WidthAt(1).Points.ShouldBe(288.0, 0.05);
        ruler.OffsetOf(1).Points.ShouldBe(180.0, 0.05);
        ruler.Total.Points.ShouldBe(468.0, 0.05);
    }

    /// <summary>
    /// The section is laid out in two columns where the file asks for them, and the flow returns to the
    /// full measure below it on the same page.
    /// </summary>
    /// <remarks>
    /// Every figure is 26.2.4.2's own PDF of the fixture, read with PyMuPDF's span origins. It draws
    /// ONEBEFORE at x 72.1 y 82.5; PARA01 to PARA08 in the first column at 72.1, PARA01 at y 122.85;
    /// PARA09 to PARA14 and TWOEND in the second at <b>324.1</b>, PARA09 at y 163.20; and THREEAFTER
    /// back at 72.1, y <b>512.90</b> — on the same page, below the deeper of the two columns.
    /// <c>72 + 216 + 36 = 324</c> is the whole arithmetic: an 8.5 inch page, inch margins, an even
    /// split of the 468 pt measure less a half-inch gap.
    /// </remarks>
    [Fact]
    public void ASectionsColumnsAreLaidOutAndTheFlowReturnsBelowThem()
    {
        Dictionary<string, DrawnWord> words = Words("odt-section-columns.odt", 1)[0];

        words["PARA01"].Left.ShouldBe(72.1, Tolerance);
        words["PARA08"].Left.ShouldBe(72.1, Tolerance);
        words["PARA09"].Left.ShouldBe(324.1, Tolerance);
        words["TWOEND"].Left.ShouldBe(324.1, Tolerance);

        // The balance is where the reference put it: eight paragraphs in the first column and the rest
        // in the second, which is what a section frame's height search settles on.
        words["PARA09"].Baseline.ShouldBe(163.20, Tolerance);

        // And the section ends where its deeper column does, on the same page rather than the next.
        words["THREEAFTER"].Left.ShouldBe(72.1, Tolerance);
        words["THREEAFTER"].Baseline.ShouldBe(512.90, Tolerance);
    }

    /// <summary>A section inside a section takes one column, whatever the enclosing one has.</summary>
    /// <remarks>
    /// Writer inserts a nested section's frame behind its parent rather than inside it, so the parent's
    /// <c>SwColumnFrame</c>s never reach it (<c>sw/source/core/layout/frmtool.cxx</c>:1795-1803) — and
    /// it does not inherit the parent's columns either, although its format <em>is</em> derived from the
    /// parent's (<c>ndsect.cxx</c>:1345): <c>SwSectionFormat</c>'s constructor puts the pool's default
    /// one-column item on every section format outright (<c>section.cxx</c>:608-614). Measured on
    /// 26.2.4.2 with a nested section that names no style at all and with one whose style states a
    /// <c>style:section-properties</c> and no <c>style:columns</c>: both are drawn across the whole
    /// measure, wrapping at 521.9 and 530.5 pt inside a parent whose columns are 216 pt wide.
    /// </remarks>
    [Fact]
    public void ANestedSectionTakesOneColumnWhereItStatesNone()
    {
        OdfSectionGeometry parent =
            OdfSectionGeometry.Read(
                Styles(@"<style:columns fo:column-count=""2"" fo:column-gap=""0.5in""/>"), "S")!.Value;

        // No style at all, and a style whose section-properties states no `style:columns`.
        OdfSectionGeometry.Nested(Styles(""), styleName: null, parent).Columns.ShouldBe(1);
        OdfSectionGeometry.Nested(Styles(""), "S", parent).Columns.ShouldBe(1);
    }

    /// <summary>And it takes its own columns where it states them.</summary>
    [Fact]
    public void ANestedSectionTakesItsOwnColumnsWhereItStatesThem()
    {
        OdfSectionGeometry parent =
            OdfSectionGeometry.Read(
                Styles(@"<style:columns fo:column-count=""2"" fo:column-gap=""0.5in""/>"), "S")!.Value;

        OdfSectionGeometry nested = OdfSectionGeometry.Nested(
            Styles(@"<style:columns fo:column-count=""3"" fo:column-gap=""0.25in""/>"), "S", parent);

        nested.Columns.ShouldBe(3);
        nested.ColumnGap.Points.ShouldBe(18.0, 0.05);
    }

    /// <summary>
    /// The indents are the enclosing section's where the nested one states neither of them, and its
    /// own — as a pair — where it states either.
    /// </summary>
    /// <remarks>
    /// The two are one <c>SvxLRSpaceItem</c>, and a nested section's format is derived from its
    /// parent's, so the item is inherited whole or replaced whole. Measured on 26.2.4.2 inside a parent
    /// indented 0.5 in and 0.75 in: a nested section stating nothing is drawn from 108.1 pt to 486, one
    /// stating 1.5 in and 0.25 in from 180.1 to 522, and one stating only the 1.5 in left indent from
    /// 180.1 to the page's own right edge rather than to the parent's.
    /// <c>probes/odt-sectable-r92/nested.py</c>.
    /// </remarks>
    [Fact]
    public void ANestedSectionsIndentsAreItsOwnPairOrTheEnclosingOnes()
    {
        OdfSectionGeometry parent = OdfSectionGeometry.Read(
            Styles(
                @"<style:columns fo:column-count=""2"" fo:column-gap=""0.5in""/>",
                @"fo:margin-left=""0.5in"" fo:margin-right=""0.75in"""),
            "S")!.Value;

        OdfSectionGeometry inherits = OdfSectionGeometry.Nested(Styles(""), "S2", parent);
        inherits.IndentLeft.Points.ShouldBe(36.0, 0.05);
        inherits.IndentRight.Points.ShouldBe(54.0, 0.05);

        OdfSectionGeometry ownPair = OdfSectionGeometry.Nested(
            Styles("", @"fo:margin-left=""1.5in"" fo:margin-right=""0.25in"""), "S", parent);
        ownPair.IndentLeft.Points.ShouldBe(108.0, 0.05);
        ownPair.IndentRight.Points.ShouldBe(18.0, 0.05);

        // One side stated is the whole item stated, so the other is nought and not the parent's 54.
        OdfSectionGeometry oneSide =
            OdfSectionGeometry.Nested(Styles("", @"fo:margin-left=""1.5in"""), "S", parent);
        oneSide.IndentLeft.Points.ShouldBe(108.0, 0.05);
        oneSide.IndentRight.ShouldBe(Length.Zero);
    }

    /// <summary>
    /// An index inside a columned section is drawn across the whole measure, below both columns, and
    /// the section resumes its columns underneath it.
    /// </summary>
    /// <remarks>
    /// The fixture is <c>odt-section-columns.odt</c> with a <c>text:table-of-content</c> put inside the
    /// two-column section after PARA11 and the whole file then round-tripped through 26.2.4.2's own ODF
    /// export, so every element in it is LibreOffice's. Its figures are that binary's PDF: TOCBODY at
    /// x 72.1, y 418.80 — wrapping at 519.0, which is the full 468 pt measure and not the 216 pt
    /// column — PARA12 back at 72.1, y 459.10 in the <em>first</em> column of a second frame of the same
    /// section, and TWOEND at 324.1, y 526.30 in its second. That is
    /// <c>SwSectionFrame::SplitSect</c> (<c>frmtool.cxx</c>:1954-1960) drawn.
    /// </remarks>
    [Fact]
    public void AnIndexInsideAColumnedSectionSplitsItAndTakesTheWholeMeasure()
    {
        Dictionary<string, DrawnWord> words = Words("odt-section-nested-index.odt", 1)[0];

        words["TOCBODY"].Left.ShouldBe(72.1, Tolerance);
        words["TOCBODY"].Baseline.ShouldBe(418.80, Tolerance);

        words["PARA12"].Left.ShouldBe(72.1, Tolerance);
        words["PARA12"].Baseline.ShouldBe(459.10, Tolerance);

        words["TWOEND"].Left.ShouldBe(324.1, Tolerance);
        words["TWOEND"].Baseline.ShouldBe(526.30, Tolerance);
    }

    /// <summary>A section style holding one <c>style:section-properties</c>, as a resolved style set.</summary>
    private static OdfStyles Styles(string columns, string sectionAttributes = "")
    {
        string document = $"""
            <office:document-styles
                xmlns:office="{OdfNamespaces.Office}"
                xmlns:style="{OdfNamespaces.Style}"
                xmlns:fo="{OdfNamespaces.FoCompatible}"
                xmlns:text="{OdfNamespaces.Text}">
              <office:automatic-styles>
                <style:style style:name="S" style:family="section">
                  <style:section-properties {sectionAttributes}>{columns}</style:section-properties>
                </style:style>
              </office:automatic-styles>
            </office:document-styles>
            """;

        OdfStyles styles = new();
        styles.AddDocument(XElement.Parse(document));
        return styles;
    }

    /// <summary>Every page's words, by their text.</summary>
    private static List<Dictionary<string, DrawnWord>> Words(string fixture, int expectedPages)
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(fixture)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(expectedPages);
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        // First occurrence wins: the fixture's prose repeats "column" and "lorem", and it is the
        // markers — which are unique — that the assertions look up.
        return [.. sink.Pages.Select(page =>
        {
            Dictionary<string, DrawnWord> found = new(StringComparer.Ordinal);
            foreach (DrawnWord word in DrawnWords.On(page)) found.TryAdd(word.Text, word);
            return found;
        })];
    }
}
