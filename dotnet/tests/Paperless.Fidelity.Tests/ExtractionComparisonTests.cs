using System.Text.RegularExpressions;
using Paperless.Core.Extraction;
using Paperless.TestKit;
using Paperless.TestKit.Comparison;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares Paperless extraction against LibreOffice's own text export.
/// </summary>
/// <remarks>
/// <para>
/// This is the oracle that makes extraction work verifiable rather than plausible. It needs an
/// installed LibreOffice with its application modules, so every test here skips — loudly, with
/// the reason — when that is missing, rather than failing on a machine that was never set up
/// for it. <c>.claude/skills/libreoffice-reference/scripts/check-env.sh</c> prints what to
/// install.
/// </para>
/// <para>
/// The assertion is deliberately one-sided: nothing the reference found may be missing from
/// the Paperless extraction, but Paperless finding <em>more</em> is expected and correct. The
/// reference filters drop headers, footers, comments, notes and shape text, and Paperless
/// keeps all five.
/// </para>
/// </remarks>
public class ExtractionComparisonTests : IDisposable
{
    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory = Path.Combine(
        Path.GetTempPath(), "paperless-fidelity-" + Guid.NewGuid().ToString("N")[..12]);

    private static void RequireLibreOffice()
    {
        Assert.SkipUnless(
            LibreOfficeRunner.IsAvailable,
            "LibreOffice with its application modules is not installed, so there is no "
            + "reference to compare against. Run "
            + ".claude/skills/libreoffice-reference/scripts/check-env.sh for the apt-get lines. "
            + "A container with libreoffice-core alone has an soffice that runs and then fails "
            + "on every document, which is why this checks by converting rather than by "
            + "finding the binary.");
    }

    private string Reference(string corpusName)
    {
        string input = Corpus.Require(corpusName);
        string isolated = Path.Combine(_workDirectory, Path.GetFileName(input));
        return File.ReadAllText(_libreOffice.ExtractText(input, isolated));
    }

    private static string Extract(string corpusName)
        => PaperlessDocument.Extract(Corpus.Require(corpusName)).GetText();

    [Theory]
    [InlineData("prose-odt.odt")]
    [InlineData("prose-ott.ott")]
    [InlineData("prose-fodt.fodt")]
    public void MinimalTextDocumentsExtractIdenticallyToTheReferenceFilter(string name)
    {
        RequireLibreOffice();

        // These hold nothing the text filter cannot express — no tables, lists, notes or
        // shapes — so anything short of a match is a real defect rather than a filter
        // limitation. That makes them the tightest assertion available.
        string reference = Reference(name);
        string actual = Extract(name);

        TextComparer.Normalise(actual).ShouldBe(
            TextComparer.Normalise(reference),
            TextComparer.DescribeFirstDivergence(reference, actual));
    }

    /// <summary>
    /// Tokens a document's reference output contains that Paperless deliberately does not
    /// produce, with the reason.
    /// </summary>
    /// <remarks>
    /// Deliberately tiny and named one token at a time. A blanket loosening of the comparison
    /// would hide the next real regression; an explicit entry with a reason is a decision that
    /// can be reviewed.
    /// </remarks>
    private static HashSet<string> KnownDeviations(string name) => name switch
    {
        // LibreOffice numbers this document's single footnote 0 rather than 1. It is a
        // LibreOffice import quirk, not a difference of opinion: with the section-level
        // w:footnotePr removed from the same file LibreOffice renders 1, and on a minimal DOCX
        // carrying one footnote and no footnote properties at all both LibreOffice and Paperless
        // render 1. ECMA-376 §17.11.17 puts the default w:numStart at 1, so 1 is what the file
        // says. Copying the quirk is explicitly a non-goal.
        // The same quirk shows in the RTF conversion of the same document, where LibreOffice
        // wrote \ftnstart1 and then renders 0 — which is what makes it clearly LibreOffice's
        // restart-per-page path rather than anything either file says.
        "word-features.docx" or "word-features.dotx" or "word-features.rtf"
            => new HashSet<string>(StringComparer.Ordinal) { "reference0" },
        _ => new HashSet<string>(StringComparer.Ordinal),
    };

    [Theory]
    [InlineData("text-features.odt")]
    [InlineData("text-features-flat.fodt")]
    [InlineData("word-features.docx")]
    [InlineData("word-features.dotx")]
    [InlineData("word-features.rtf")]
    public void NothingTheReferenceFindsIsMissingFromTheFeatureDocument(string name)
    {
        RequireLibreOffice();

        string reference = Reference(name);
        string actual = Extract(name);

        HashSet<string> allowed = KnownDeviations(name);
        IReadOnlyList<(string Token, int Missing)> missing =
            [.. TextComparer.FindMissingTokens(reference, actual)
                            .Where(m => !allowed.Contains(m.Token))];

        missing.ShouldBeEmpty(
            "content the reference filter found is absent from the Paperless extraction: "
            + string.Join(", ", missing.Select(m => $"'{m.Token}' x{m.Missing}")));

        // Similarity is well below 1 by design here and the shortfall is accounted for: list
        // indentation, the filter's substituted bullet characters, tables rendered one cell per
        // line rather than as tab-separated rows, and the headers, footers, notes, comments and
        // shape text the filter drops and Paperless keeps.
        TextComparer.Similarity(reference, actual).ShouldBeGreaterThan(0.85);
    }

    [Fact]
    public void TheFeatureDocumentsExtraContentIsTheKnownSetOfFilterOmissions()
    {
        RequireLibreOffice();

        // Guards the other direction: extra content is expected, but only from the flows the
        // reference filter cannot express. Anything else appearing would be invented text.
        ContentDocument content = PaperlessDocument.Extract(Corpus.Require("text-features.odt"));
        string bodyOnly = string.Concat(
            content.Children.OfType<ContentSection>()
                   .Where(s => s.Kind == SectionKind.Body)
                   .Select(s => s.GetText()));

        string reference = Reference("text-features.odt");
        TextComparer.FindMissingTokens(reference, bodyOnly).ShouldBeEmpty(
            "the body section alone should already account for everything the reference found");

        content.Children.OfType<ContentSection>().Select(s => s.Kind).Distinct()
               .ShouldBe([SectionKind.Body, SectionKind.Note, SectionKind.Comment,
                          SectionKind.Frame, SectionKind.Header, SectionKind.Footer],
                         ignoreOrder: true);
    }

    [Theory]
    [InlineData("sheet-ods.ods")]
    [InlineData("sheet-ots.ots")]
    [InlineData("sheet-fods.fods")]
    public void SpreadsheetCellsMatchTheCsvReference(string name)
    {
        RequireLibreOffice();

        // The CSV filter covers the first sheet only, and these workbooks have exactly one.
        // Comparison is per token so that the filter's commas and Paperless's tabs agree.
        string reference = Reference(name);
        string actual = Extract(name);

        TextComparer.FindMissingTokens(reference, actual).ShouldBeEmpty();
        TextComparer.Similarity(reference, actual).ShouldBeGreaterThan(0.99);
    }

    [Fact]
    public void TheFeatureWorkbooksFirstSheetMatchesExceptWhereTheFilterDropsFormatting()
    {
        RequireLibreOffice();

        string reference = Reference("sheet-features.ods");
        string actual = Extract("sheet-features.ods");

        // The CSV filter writes a currency cell as its raw number (4.5), while the file's own
        // cached display text — what Calc actually shows, and what Paperless reports — is
        // "£4.50". So the formatted cells legitimately differ and only the unformatted content
        // is compared here.
        IReadOnlyList<(string Token, int Missing)> missing =
            TextComparer.FindMissingTokens(reference, actual);

        missing.Where(m => !LooksLikeUnformattedNumber(m.Token)).ShouldBeEmpty(
            "non-numeric content the reference found is missing: "
            + string.Join(", ", missing.Select(m => m.Token)));

        // Every sheet is extracted, not just the first the CSV filter covers.
        actual.ShouldContain("Content of a hidden sheet");

        static bool LooksLikeUnformattedNumber(string token)
            => double.TryParse(token, System.Globalization.NumberStyles.Float,
                               System.Globalization.CultureInfo.InvariantCulture, out _);
    }

    [Theory]
    [InlineData("slides-odp.odp")]
    [InlineData("slides-otp.otp")]
    [InlineData("slides-fodp.fodp")]
    [InlineData("slides-features.odp")]
    public void NothingTheHtmlReferenceFindsIsMissingFromTheDeck(string name)
    {
        RequireLibreOffice();

        // Impress has no plain-text filter, so the reference is HTML and the markup has to come
        // off before comparing. Its CSS block is stripped first: selector names and property
        // values would otherwise be compared as document text.
        string reference = StripHtml(Reference(name));
        string actual = Extract(name);

        TextComparer.FindMissingTokens(reference, actual).ShouldBeEmpty();
    }

    [Fact]
    public void TheFontEnvironmentIsReportedRatherThanAssumed()
    {
        RequireLibreOffice();

        FontEnvironmentReport report = LibreOfficeRunner.CheckFontEnvironment();

        // Not an assertion: font resolution does not affect text extraction at all, and failing
        // here would be a spurious failure for this phase. It becomes a hard requirement once
        // layout starts, because a substitution that is not metric-compatible changes advance
        // widths, hence line breaks, hence pagination.
        Assert.SkipWhen(
            !report.IsUsableForComparison,
            "The metric-compatible fonts are not installed, which will matter for layout "
            + "comparison but not for extraction: " + string.Join(" ", report.Problems));

        report.Resolutions["Calibri"].ShouldContain("Carlito");
        report.Resolutions["Cambria"].ShouldContain("Caladea");
    }

    [Fact]
    public void TheReferenceVersionIsRecordedWithAnyComparison()
    {
        RequireLibreOffice();

        // A LibreOffice upgrade legitimately changes reference output, so which version
        // produced it is part of the result rather than an incidental detail.
        _libreOffice.GetVersion().ShouldStartWith("LibreOffice");
    }

    private static string StripHtml(string html)
    {
        // The <head> goes first, and all of it. It holds the CSS — whose selectors and property
        // values are full of words that would compare as document text — and the page <title>,
        // which is the document's metadata title rather than anything on a slide. Comparing
        // that against slide content would demand the deck's title appear twice.
        string body = Regex.Replace(html, "<head.*?</head>", " ",
                                    RegexOptions.Singleline | RegexOptions.IgnoreCase);
        string withoutTags = Regex.Replace(body, "<[^>]+>", "\n");
        return System.Net.WebUtility.HtmlDecode(withoutTags);
    }

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
            // Temporary reference output left behind is harmless.
        }
        GC.SuppressFinalize(this);
    }
}
