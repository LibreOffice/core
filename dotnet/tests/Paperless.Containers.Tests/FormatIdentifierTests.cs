using Paperless.Containers;
using Paperless.Core.Formats;
using Paperless.TestKit;

namespace Paperless.Containers.Tests;

/// <summary>
/// Tests that identification works on files produced by LibreOffice itself, which is the
/// corpus that matters: hand-built fixtures would only prove we agree with ourselves.
/// </summary>
public class FormatIdentifierTests
{
    /// <summary>
    /// Every format the corpus covers, with the format each file must be identified as.
    /// </summary>
    public static TheoryData<string, DocumentFormat> CorpusExpectations()
    {
        TheoryData<string, DocumentFormat> data = new();
        // Writer
        data.Add("prose-odt.odt", DocumentFormat.Odt);
        data.Add("prose-ott.ott", DocumentFormat.Ott);
        data.Add("prose-fodt.fodt", DocumentFormat.Fodt);
        data.Add("prose-doc.doc", DocumentFormat.Doc);
        data.Add("prose-docx.docx", DocumentFormat.Docx);
        data.Add("prose-rtf.rtf", DocumentFormat.Rtf);
        // Calc
        data.Add("sheet-ods.ods", DocumentFormat.Ods);
        data.Add("sheet-ots.ots", DocumentFormat.Ots);
        data.Add("sheet-fods.fods", DocumentFormat.Fods);
        data.Add("sheet-xls.xls", DocumentFormat.Xls);
        data.Add("sheet-xlsx.xlsx", DocumentFormat.Xlsx);
        data.Add("sheet-csv.csv", DocumentFormat.Csv);
        // Impress
        data.Add("slides-odp.odp", DocumentFormat.Odp);
        data.Add("slides-otp.otp", DocumentFormat.Otp);
        data.Add("slides-fodp.fodp", DocumentFormat.Fodp);
        data.Add("slides-ppt.ppt", DocumentFormat.Ppt);
        data.Add("slides-pptx.pptx", DocumentFormat.Pptx);
        return data;
    }

    [Theory]
    [MemberData(nameof(CorpusExpectations))]
    public void IdentifiesEveryCorpusFormat(string fileName, DocumentFormat expected)
    {
        string path = Corpus.Require(fileName);
        using FileStream stream = File.OpenRead(path);

        FormatIdentification result = FormatIdentifier.Instance.Identify(stream, fileName);

        Assert.Equal(expected, result.Format);
    }

    /// <summary>
    /// The point of content-based detection: a wrong extension must not mislead it.
    /// </summary>
    [Theory]
    [InlineData("prose-docx.docx", DocumentFormat.Docx)]
    [InlineData("sheet-xlsx.xlsx", DocumentFormat.Xlsx)]
    [InlineData("slides-pptx.pptx", DocumentFormat.Pptx)]
    [InlineData("prose-odt.odt", DocumentFormat.Odt)]
    [InlineData("sheet-xls.xls", DocumentFormat.Xls)]
    [InlineData("slides-ppt.ppt", DocumentFormat.Ppt)]
    public void IgnoresAMisleadingExtension(string fileName, DocumentFormat expected)
    {
        string path = Corpus.Require(fileName);
        using FileStream stream = File.OpenRead(path);

        // Claim it is something else entirely. Content must win.
        FormatIdentification result = FormatIdentifier.Instance.Identify(stream, "misnamed.pptx");

        Assert.Equal(expected, result.Format);
    }

    /// <summary>
    /// Identification must work with no file name at all, except for CSV, which genuinely
    /// cannot be told from any other text file by content.
    /// </summary>
    [Theory]
    [InlineData("prose-docx.docx", DocumentFormat.Docx)]
    [InlineData("sheet-ods.ods", DocumentFormat.Ods)]
    [InlineData("slides-ppt.ppt", DocumentFormat.Ppt)]
    [InlineData("prose-fodt.fodt", DocumentFormat.Fodt)]
    [InlineData("prose-rtf.rtf", DocumentFormat.Rtf)]
    public void IdentifiesWithoutAnyFileName(string fileName, DocumentFormat expected)
    {
        string path = Corpus.Require(fileName);
        using FileStream stream = File.OpenRead(path);

        FormatIdentification result = FormatIdentifier.Instance.Identify(stream, fileNameHint: null);

        Assert.Equal(expected, result.Format);
    }

    [Theory]
    [MemberData(nameof(CorpusExpectations))]
    public void ReportsTheRightContainerAndConfidence(string fileName, DocumentFormat expected)
    {
        string path = Corpus.Require(fileName);
        using FileStream stream = File.OpenRead(path);

        FormatIdentification result = FormatIdentifier.Instance.Identify(stream, fileName);

        FormatInfo info = FormatCatalogue.Instance.GetInfo(expected)!;
        Assert.Equal(info.Container, result.Container);

        // CSV is the sole format identified from its name rather than its content, and the
        // reported confidence has to say so.
        if (expected == DocumentFormat.Csv)
            Assert.Equal(IdentificationConfidence.ExtensionOnly, result.Confidence);
        else
            Assert.Equal(IdentificationConfidence.Certain, result.Confidence);

        Assert.False(result.IsEncrypted);
        Assert.NotNull(result.Detail);
    }

    [Fact]
    public void LeavesTheStreamPositionUntouched()
    {
        string path = Corpus.Require("prose-docx.docx");
        using FileStream stream = File.OpenRead(path);
        stream.Position = 3;

        FormatIdentifier.Instance.Identify(stream, "prose-docx.docx");

        Assert.Equal(3, stream.Position);
    }

    [Fact]
    public void ReturnsUnknownForEmptyInput()
    {
        using MemoryStream stream = new();
        FormatIdentification result = FormatIdentifier.Instance.Identify(stream, "empty.docx");
        Assert.Equal(DocumentFormat.Unknown, result.Format);
    }

    [Fact]
    public void ReturnsUnknownForArbitraryBytesRatherThanThrowing()
    {
        // A parser fed junk must classify it, not fail. Identification runs on untrusted
        // input before anything else, so it is the least acceptable place to throw.
        byte[] junk = new byte[4096];
        Random.Shared.NextBytes(junk);
        using MemoryStream stream = new(junk);

        FormatIdentification result = FormatIdentifier.Instance.Identify(stream, "junk.docx");

        Assert.Equal(DocumentFormat.Unknown, result.Format);
    }

    [Fact]
    public void DoesNotMistakeAPlainZipForAnOfficeDocument()
    {
        using MemoryStream zip = new();
        using (System.IO.Compression.ZipArchive archive =
               new(zip, System.IO.Compression.ZipArchiveMode.Create, leaveOpen: true))
        {
            System.IO.Compression.ZipArchiveEntry entry = archive.CreateEntry("hello.txt");
            using StreamWriter writer = new(entry.Open());
            writer.Write("not an office document");
        }
        zip.Position = 0;

        FormatIdentification result = FormatIdentifier.Instance.Identify(zip, "archive.docx");

        Assert.Equal(DocumentFormat.Unknown, result.Format);
        // The container is still worth reporting even when the format is not recognised.
        Assert.Equal(ContainerKind.Zip, result.Container);
    }

    [Fact]
    public void SurvivesATruncatedCompoundFile()
    {
        // Real corpora contain truncated files; identification must degrade, not throw.
        byte[] full = File.ReadAllBytes(Corpus.Require("prose-doc.doc"));
        using MemoryStream truncated = new(full.AsSpan(0, full.Length / 3).ToArray());

        FormatIdentification result = FormatIdentifier.Instance.Identify(truncated, "prose-doc.doc");

        Assert.Equal(ContainerKind.Ole2CompoundFile, result.Container);
    }

    [Fact]
    public void SurvivesATruncatedZipPackage()
    {
        byte[] full = File.ReadAllBytes(Corpus.Require("prose-docx.docx"));
        using MemoryStream truncated = new(full.AsSpan(0, full.Length / 3).ToArray());

        FormatIdentification result = FormatIdentifier.Instance.Identify(truncated, "prose-docx.docx");

        // The central directory is gone, so the format is unknowable -- but the container
        // signature is still evidence and must be reported.
        Assert.Equal(ContainerKind.Zip, result.Container);
        Assert.Equal(DocumentFormat.Unknown, result.Format);
    }
}
