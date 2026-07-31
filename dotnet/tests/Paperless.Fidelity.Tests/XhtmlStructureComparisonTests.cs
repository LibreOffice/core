using System.Text;
using System.Xml;
using System.Xml.Linq;
using Paperless.Core.Extraction;
using Paperless.Markup;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares the structure of Paperless's XHTML against LibreOffice's own XHTML export.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice has no Markdown export filter, and the first conclusion from that is that
/// structured text output cannot be compared against the reference at all. It is wrong. The
/// reference exports <b>XHTML for all three families</b> — <c>XHTML Writer File</c>,
/// <c>XHTML Calc File</c>, <c>XHTML Impress File</c> — and XHTML carries exactly the structure
/// Markdown needs. Being well-formed XML it can be parsed rather than scraped, so the
/// comparison is structural and exact: every heading's level, every list's nesting depth, every
/// cell's row and column and span.
/// </para>
/// <para>
/// <b>The reference is an oracle for structure, never a target for output.</b> LibreOffice's
/// export is presentation-oriented — inline CSS, <c>span</c> wrappers, absolutely positioned
/// <c>div</c>s, a stylesheet longer than the document — and reproducing its shape would defeat
/// the point of emitting semantic XHTML. What is asserted is what both agree the document *is*,
/// with the deliberate differences named one at a time below.
/// </para>
/// <para>
/// <b>Measured: the three families are not equally useful as oracles.</b> The Writer export is
/// fully semantic and agrees with Paperless on headings, list nesting, table grids and links.
/// The Calc and Impress exports are not: Calc emits a bare <c>table</c> with no sheet name and
/// no headings at all, and Impress emits one absolutely positioned <c>div</c> per slide with no
/// heading anywhere. So Writer is compared on everything and the other two only on what they
/// actually express — which is worth having, and worth being explicit about rather than
/// asserting a parity that is not there to be had.
/// </para>
/// </remarks>
public class XhtmlStructureComparisonTests : IDisposable
{
    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory = Path.Combine(
        Path.GetTempPath(), "paperless-xhtml-" + Guid.NewGuid().ToString("N")[..12]);

    private static void RequireLibreOffice()
    {
        Assert.SkipUnless(
            LibreOfficeRunner.IsAvailable,
            "LibreOffice with its application modules is not installed, so there is no "
            + "reference to compare against. Run "
            + ".claude/skills/libreoffice-reference/scripts/check-env.sh for the apt-get lines.");
    }

    /// <summary>
    /// Word-processing documents, where the reference export is fully semantic.
    /// </summary>
    public static TheoryData<string> WriterDocuments() =>
    [
        "text-features.odt",
        "text-features-flat.fodt",
        "word-features.docx",
        "word-features.doc",
        "word-features.rtf",
        "tables.odt",
        "tables.docx",
        "tables.doc",
        "tables.rtf",
        "table-nested.odt",
        "table-nested.docx",
    ];

    [Theory]
    [MemberData(nameof(WriterDocuments))]
    public void EveryHeadingKeepsItsLevel(string name)
    {
        (XElement reference, XElement actual) = Both(name, "xhtml:XHTML Writer File");

        Headings(actual).ShouldBe(
            Headings(reference),
            "the heading levels differ from LibreOffice's own reading of the same document");
    }

    [Theory]
    [MemberData(nameof(WriterDocuments))]
    public void EveryListItemKeepsItsNestingDepth(string name)
    {
        (XElement reference, XElement actual) = Both(name, "xhtml:XHTML Writer File");

        ListDepths(actual).ShouldBe(
            ListDepths(reference),
            "the list nesting differs from LibreOffice's own reading of the same document");
    }

    [Theory]
    [MemberData(nameof(WriterDocuments))]
    public void EveryCellKeepsItsRowColumnAndSpan(string name)
    {
        (XElement reference, XElement actual) = Both(name, "xhtml:XHTML Writer File");

        Grids(actual).ShouldBe(
            Grids(reference),
            "the table grids differ from LibreOffice's own reading of the same document");
    }

    [Theory]
    [MemberData(nameof(WriterDocuments))]
    public void EveryHyperlinkKeepsItsTarget(string name)
    {
        (XElement reference, XElement actual) = Both(name, "xhtml:XHTML Writer File");

        Links(actual).ShouldBe(Links(reference));
    }

    [Fact]
    public void ASlideDeckHasTheSameSlidesAndTheSameTableGrid()
    {
        // Impress's export is presentational to the point of being nearly structureless: one
        // absolutely positioned div per slide, no headings, the slide's title reduced to the
        // div's id. So what it can be asked is how many slides there are and what is in their
        // tables — and it answers both exactly.
        (XElement reference, XElement actual) = Both("deck-features.pptx", "xhtml:XHTML Impress File");

        int slides = reference.Elements(Xhtml.Namespace + "div")
            .Count(e => (e.Attribute("class")?.Value ?? string.Empty).StartsWith("page-", StringComparison.Ordinal));

        actual.Elements()
            .Count(e => e.Attribute("class")?.Value == "slide")
            .ShouldBe(slides, "the deck has a different number of slides than LibreOffice found");

        Grids(actual).ShouldBe(Grids(reference));
        ListDepths(actual).ShouldBe(ListDepths(reference));
    }

    [Fact]
    public void ASheetsCellsLandInTheSameRowsAndColumns()
    {
        // Calc's export is a bare table: no sheet name, no headings, and the used range padded
        // with empty rows LibreOffice's own print range implies and the file does not state. So
        // the assertion is that our rows are a prefix of theirs, cell for cell — which catches a
        // shifted column or a transposed grid and does not chase padding.
        (XElement reference, XElement actual) = Both("sheet-xlsx.xlsx", "xhtml:XHTML Calc File");

        List<string> expected = Grids(reference);
        List<string> ours = Grids(actual);

        ours.Count.ShouldBeLessThanOrEqualTo(expected.Count);
        ours.ShouldBe(expected.Take(ours.Count));
    }

    [Fact]
    public void TheReferenceExportIsRichEnoughToBeAnOracle()
    {
        // The guard against a vacuous suite. Every comparison above is an equality, so an oracle
        // that silently produced nothing would agree with anything that also produced nothing —
        // and soffice exits 0 when it converts nothing at all. These are the measured contents
        // of the reference's own export of text-features.odt.
        (XElement reference, _) = Both("text-features.odt", "xhtml:XHTML Writer File");

        Headings(reference).ShouldBe([1, 2, 2, 3]);
        ListDepths(reference).ShouldBe([1, 1, 2, 1, 1, 2, 2, 1]);
        Grids(reference).Count.ShouldBe(8);
        Links(reference).ShouldBe(["https://www.libreoffice.org/"]);
    }

    // ---- the two projections, side by side -------------------------------------------

    private (XElement Reference, XElement Actual) Both(string name, string filter)
    {
        RequireLibreOffice();

        string input = Corpus.Require(name);
        string outputDirectory = Path.Combine(_workDirectory, Path.GetFileName(input));
        IReadOnlyList<string> produced = _libreOffice.ConvertToMany(input, filter, outputDirectory);

        string? xhtml = produced.FirstOrDefault(p => p.EndsWith(".xhtml", StringComparison.Ordinal));
        xhtml.ShouldNotBeNull($"LibreOffice produced no XHTML for {name}; it wrote {produced.Count} file(s).");

        ContentDocument tree = PaperlessDocument.Extract(input);
        return (Body(File.ReadAllText(xhtml)), Body(XhtmlWriter.ToXhtml(tree)));
    }

    /// <summary>
    /// Parses an XHTML document and returns its <c>body</c>.
    /// </summary>
    /// <remarks>
    /// The resolver is disabled deliberately. LibreOffice's export declares
    /// <c>-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN</c> with an <c>http://</c> system
    /// identifier, and a reader that resolves it goes to the network — so the test would pass or
    /// fail depending on whether the machine had one, which is not what it is measuring.
    /// </remarks>
    private static XElement Body(string xhtml)
    {
        XmlReaderSettings settings = new() { DtdProcessing = DtdProcessing.Ignore, XmlResolver = null };
        using StringReader text = new(xhtml);
        using XmlReader reader = XmlReader.Create(text, settings);
        XDocument document = XDocument.Load(reader);
        return document.Root!.Element(Xhtml.Namespace + "body")
            ?? throw new InvalidOperationException("The XHTML has no body element.");
    }

    private static List<int> Headings(XElement body) =>
    [
        .. body.Descendants()
               .Where(e => e.Name.Namespace == Xhtml.Namespace && e.Name.LocalName is ['h', >= '1' and <= '6'])
               .Select(e => e.Name.LocalName[1] - '0'),
    ];

    /// <summary>
    /// The nesting depth of every list item that carries text of its own.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Items are counted rather than <c>ul</c>/<c>ol</c> elements, and only those with text,
    /// because the two writers structure the same list differently in two ways that are both
    /// correct. LibreOffice hangs a nested list off a wrapper <c>li</c> of its own
    /// (<c>&lt;ul&gt;&lt;li&gt;&lt;ul&gt;…</c>) where Paperless puts it inside the item it
    /// belongs to; counting the wrappers would report a difference that is not one.
    /// </para>
    /// <para>
    /// Items whose content is a heading are skipped, and that is the second deliberate
    /// difference. Word attaches its heading styles to an outline list, so in a DOCX every
    /// heading is also a list item — LibreOffice's export wraps them in <c>li</c> accordingly and
    /// Paperless emits the heading alone. Both keep the heading's level, which
    /// <see cref="EveryHeadingKeepsItsLevel"/> asserts separately.
    /// </para>
    /// </remarks>
    private static List<int> ListDepths(XElement body)
    {
        List<int> depths = [];
        Walk(body, 0);
        return depths;

        void Walk(XElement element, int depth)
        {
            foreach (XElement child in element.Elements())
            {
                switch (child.Name.LocalName)
                {
                    case "ul" or "ol":
                        Walk(child, depth + 1);
                        break;
                    case "li":
                        if (ItemText(child).Length > 0
                            && !child.Elements().Any(e => e.Name.LocalName is ['h', >= '1' and <= '6']))
                        {
                            depths.Add(depth);
                        }
                        Walk(child, depth);
                        break;
                    default:
                        Walk(child, depth);
                        break;
                }
            }
        }
    }

    /// <summary>
    /// A list item's own text, without its sub-lists and without LibreOffice's marker markup.
    /// </summary>
    /// <remarks>
    /// The reference renders the marker into the item as a floated <c>span</c> and closes it
    /// with <c>span.odfLiEnd</c>; Paperless parks the marker on a <c>data-marker</c> attribute
    /// instead, since it is not text the document contains. Left in, every reference item would
    /// read "•First bullet" and no item would ever match.
    /// </remarks>
    private static string ItemText(XElement item)
    {
        StringBuilder text = new();
        Walk(item);
        return string.Join(' ', text.ToString().Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries));

        void Walk(XElement element)
        {
            foreach (XNode node in element.Nodes())
            {
                switch (node)
                {
                    case XText raw: text.Append(raw.Value).Append(' '); break;
                    case XElement { Name.LocalName: "ul" or "ol" }: break;
                    case XElement child when IsMarkerMarkup(child): break;
                    case XElement child: Walk(child); break;
                    default: break;
                }
            }
        }

        static bool IsMarkerMarkup(XElement element)
            => element.Name.LocalName == "span"
               && ((element.Attribute("class")?.Value.Contains("odfLiEnd", StringComparison.Ordinal) ?? false)
                   || (element.Attribute("style")?.Value.Replace(" ", string.Empty, StringComparison.Ordinal)
                              .Contains("float:left", StringComparison.Ordinal) ?? false));
    }

    /// <summary>
    /// Every table's grid, one string per cell: its table, row, column, spans and text.
    /// </summary>
    /// <remarks>
    /// Rows are attributed to their own table rather than to every ancestor table, which matters
    /// exactly where the corpus is hardest: a nested table's rows would otherwise be counted
    /// twice, once as themselves and once as rows of the table they sit inside.
    /// </remarks>
    private static List<string> Grids(XElement body)
    {
        List<string> cells = [];
        List<XElement> tables = [.. body.Descendants(Xhtml.Namespace + "table")];

        for (int t = 0; t < tables.Count; t++)
        {
            List<XElement> rows =
            [
                .. tables[t].Descendants(Xhtml.Namespace + "tr")
                            .Where(tr => tr.Ancestors(Xhtml.Namespace + "table").First() == tables[t]),
            ];

            for (int r = 0; r < rows.Count; r++)
            {
                int column = 0;
                foreach (XElement cell in rows[r].Elements())
                {
                    // th and td together: Paperless marks a declared header row with th, which
                    // the reference does not do at all — it writes td throughout and puts the
                    // heading-ness in a class. That is a difference of vocabulary, not of grid.
                    if (cell.Name.LocalName is not ("td" or "th")) continue;

                    string columnSpan = cell.Attribute("colspan")?.Value ?? "1";
                    string rowSpan = cell.Attribute("rowspan")?.Value ?? "1";
                    string text = string.Join(
                        ' ', cell.Value.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries));

                    cells.Add($"t{t} r{r} c{column} {columnSpan}x{rowSpan} '{text}'");
                    column += int.TryParse(columnSpan, out int span) ? span : 1;
                }
            }
        }
        return cells;
    }

    /// <summary>
    /// Every hyperlink target that leaves the document.
    /// </summary>
    /// <remarks>
    /// Fragment references are excluded because the two writers mint them differently and
    /// neither is wrong: the reference generates an anchor per heading and a
    /// <c>#ftn1</c> per footnote, which are artefacts of its own linking scheme rather than
    /// links the document contains.
    /// </remarks>
    private static List<string> Links(XElement body) =>
    [
        .. body.Descendants(Xhtml.Namespace + "a")
               .Select(a => a.Attribute("href")?.Value ?? string.Empty)
               .Where(href => href.Length > 0 && !href.StartsWith('#')),
    ];

    /// <inheritdoc/>
    public void Dispose()
    {
        _libreOffice.Dispose();
        try
        {
            if (Directory.Exists(_workDirectory)) Directory.Delete(_workDirectory, recursive: true);
        }
        catch (IOException)
        {
        }
        GC.SuppressFinalize(this);
    }
}
