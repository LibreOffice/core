using System.Globalization;
using System.IO.Compression;
using System.Text;
using System.Text.RegularExpressions;
using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Settles what a reference renderer does with a tracked change, by measuring rather than by opinion.
/// </summary>
/// <remarks>
/// <para>
/// The question this answers is whether <c>LayoutOptions.AcceptTrackedChanges</c> defaults the right
/// way round. It does: LibreOffice renders all four formats of the corpus's revision document with
/// the change <em>marks</em> showing — the deleted phrase is on the page, struck through, and a
/// change bar sits in the margin beside the line — so <c>false</c>, "show marks", is what matching
/// the reference means.
/// </para>
/// <para>
/// That is the opposite of what Paperless <em>extracts</em>, deliberately and permanently.
/// Extraction says what the changes leave: the insertion is content and the deletion is not, because
/// deleted text is still in the file and emitting it invents content the document does not say.
/// LibreOffice's text filter emits it because that filter renders the document as a reader sees it,
/// and its reader shows marks. Both halves are pinned here, together, because the pair is the
/// finding: the same default is right for rendering and wrong for extraction.
/// </para>
/// <para>
/// Layout does not honour the option yet — it accepts the changes whichever way the flag is set —
/// which is recorded in <c>Paperless.WordProcessing/TODO.md</c> rather than asserted here.
/// </para>
/// </remarks>
public sealed class TrackedChangeComparisonTests : IDisposable
{
    /// <summary>The corpus document's left text margin: 1134 twips.</summary>
    private const double TextMarginPoints = 56.7;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-redlines").FullName;

    public void Dispose()
    {
        _libreOffice.Dispose();
        try
        {
            Directory.Delete(_workDirectory, recursive: true);
        }
        catch (IOException)
        {
            // A leftover temporary directory is not worth failing a test over.
        }
    }

    /// <summary>
    /// LibreOffice draws a change bar beside the changed line, in every one of the four formats.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The change bar is the unambiguous evidence, and it is a <em>stroke</em> rather than text — so a
    /// word-box comparison could never see it. Measured on A4 with a 1134-twip margin: one vertical stroke at x = 49.65 pt, which is
    /// 7.05 pt to the left of where body text starts, running the height of a single 12 pt line.
    /// </para>
    /// <para>
    /// The strike-through and the coloured underline are in the PDF too — <c>0 3.1 m 80.2 3.1 l S</c>
    /// and <c>0 -1.4 m 89.6 -1.4 l S</c> at a pen width of 0.7 — but they are emitted inside a text
    /// matrix, so their coordinates are relative to the run rather than to the page and are not
    /// comparable against a layout engine's without tracking that matrix. The change bar is in page
    /// space and needs no such care.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("revisions.docx")]
    [InlineData("revisions.odt")]
    [InlineData("revisions.doc")]
    [InlineData("revisions.rtf")]
    public void TheReferenceShowsChangeMarksByDefault(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string pdf = _libreOffice.ConvertToPdf(Corpus.Require(fileName), _workDirectory);

        List<(double X, double Length)> bars =
        [
            .. VerticalStrokes(pdf).Where(bar => bar.X < TextMarginPoints && bar.Length > 5),
        ];

        bars.Count.ShouldBe(
            1,
            $"{fileName}: LibreOffice marks the one changed line with one change bar in the margin");

        // Between the page edge and the text, close enough to the margin to be a change bar rather
        // than a border or a table rule.
        bars[0].X.ShouldBeInRange(TextMarginPoints - 12, TextMarginPoints);

        // One line tall: the change is inside a single line of 12 pt text.
        bars[0].Length.ShouldBeInRange(10, 20);
    }

    /// <summary>
    /// The vertical strokes a PDF draws, as (x, length) pairs in page space.
    /// </summary>
    /// <remarks>
    /// Local rather than <see cref="PdfStrokes"/> for one specific reason: that reader pairs each
    /// stroke with the pen width stated <em>beside</em> it, which is how LibreOffice writes a table
    /// border — <c>q 0.5 w … m … l S Q</c>. A change bar restates no width; it inherits the 0.1 pt
    /// default set once at the top of the page, with several other paths in between. So the shared
    /// reader legitimately does not see it, and widening its pattern would make it see more of every
    /// other document too — including the tables whose stroke counts other comparisons pin exactly.
    /// </remarks>
    private static List<(double X, double Length)> VerticalStrokes(string pdfPath)
    {
        List<(double, double)> strokes = [];

        foreach (string content in ContentStreams(File.ReadAllBytes(pdfPath)))
        {
            foreach (Match match in Regex.Matches(
                content,
                @"(-?[\d.]+)\s+(-?[\d.]+)\s+m\s+(-?[\d.]+)\s+(-?[\d.]+)\s+l\s+S\b"))
            {
                double[] point = [.. match.Groups.Values.Skip(1)
                    .Select(group => double.Parse(group.Value, CultureInfo.InvariantCulture))];

                if (Math.Abs(point[0] - point[2]) < 0.05)
                    strokes.Add((point[0], Math.Abs(point[3] - point[1])));
            }
        }

        return strokes;
    }

    private static IEnumerable<string> ContentStreams(byte[] bytes)
    {
        string text = Encoding.Latin1.GetString(bytes);

        foreach (Match match in Regex.Matches(text, @"stream\r?\n"))
        {
            int start = match.Index + match.Length;
            int end = text.IndexOf("endstream", start, StringComparison.Ordinal);
            if (end < 0) continue;

            string? content = null;
            try
            {
                using MemoryStream compressed = new(bytes, start, end - start);
                using ZLibStream inflater = new(compressed, CompressionMode.Decompress);
                using MemoryStream plain = new();
                inflater.CopyTo(plain);
                content = Encoding.Latin1.GetString(plain.ToArray());
            }
            catch (InvalidDataException)
            {
                // Not a Flate stream — a font file or an image, neither of which strokes anything.
            }

            if (content is not null && content.Contains("BT", StringComparison.Ordinal))
                yield return content;
        }
    }

    /// <summary>
    /// The reference's own text keeps the deletion; Paperless's does not, and that is deliberate.
    /// </summary>
    /// <remarks>
    /// Recorded as a *paired* assertion rather than as a tolerance in the extraction comparison,
    /// because the difference is not an error margin: it is one phrase, present in one output and
    /// absent from the other for a reason each side is right about. What Paperless now adds is that
    /// the phrase is not lost — it is on the <see cref="WritingChange"/> with the name of the person
    /// who removed it.
    /// </remarks>
    [Theory]
    [InlineData("revisions.docx")]
    [InlineData("revisions.odt")]
    [InlineData("revisions.doc")]
    [InlineData("revisions.rtf")]
    public void TheDeletedPhraseIsInTheReferenceAndInTheRecordButNotInTheExtraction(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        const string Deleted = "a deleted phrase";
        string path = Corpus.Require(fileName);

        string reference = File.ReadAllText(_libreOffice.ExtractText(path, _workDirectory));
        reference.ShouldContain(
            Deleted,
            customMessage: $"{fileName}: LibreOffice's text filter shows what its renderer shows, and "
                + "its renderer shows change marks by default");

        using IWordProcessingDocument document =
            (IWordProcessingDocument)new WordProcessingReader().Read(DocumentSource.FromFile(path));

        document.Content.GetText().Contains(Deleted, StringComparison.Ordinal).ShouldBeFalse(
            $"{fileName}: extraction says what the changes leave");

        document.Marks.Changes
            .Single(change => change.Kind == WritingChangeKind.Deletion)
            .Text.ShouldNotBeNull()
            .ShouldContain(Deleted);
    }
}
