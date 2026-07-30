using Paperless.Core.Formats;

namespace Paperless.Core.Tests;

public class FormatCatalogueTests
{
    /// <summary>
    /// Every declared format must be in the catalogue. Without this, a format can be added
    /// to the enum and silently never described, which surfaces as a null reference far away
    /// from the omission.
    /// </summary>
    [Fact]
    public void DescribesEveryDeclaredFormat()
    {
        List<DocumentFormat> missing = [];
        foreach (DocumentFormat format in Enum.GetValues<DocumentFormat>())
        {
            if (format == DocumentFormat.Unknown) continue;
            if (FormatCatalogue.Instance.GetInfo(format) is null) missing.Add(format);
        }

        Assert.Empty(missing);
    }

    [Fact]
    public void ReturnsNullForUnknown()
        => Assert.Null(FormatCatalogue.Instance.GetInfo(DocumentFormat.Unknown));

    [Fact]
    public void EveryEntryIsSelfConsistent()
    {
        foreach (FormatInfo info in FormatCatalogue.Instance.All)
        {
            Assert.NotEqual(DocumentFamily.Unknown, info.Family);
            Assert.NotEqual(ContainerKind.Unknown, info.Container);
            Assert.False(string.IsNullOrWhiteSpace(info.Extension), $"{info.Format} has no extension");
            Assert.False(info.Extension.StartsWith('.'), $"{info.Format} extension should not lead with a dot");
            Assert.False(string.IsNullOrWhiteSpace(info.MediaType), $"{info.Format} has no media type");
            Assert.False(string.IsNullOrWhiteSpace(info.DisplayName), $"{info.Format} has no display name");
            // Nothing is readable yet; this flips per format as readers land, and an
            // optimistic value here would make `paperless identify` lie.
            Assert.False(info.IsReadSupported);
        }
    }

    [Theory]
    [InlineData("docx", DocumentFormat.Docx)]
    [InlineData("DOCX", DocumentFormat.Docx)]
    [InlineData(".docx", DocumentFormat.Docx)]
    [InlineData("xlsb", DocumentFormat.Xlsb)]
    [InlineData("fodp", DocumentFormat.Fodp)]
    [InlineData("ppsm", DocumentFormat.Ppsm)]
    public void FindsFormatsByExtensionCaseInsensitivelyAndWithOrWithoutADot(
        string extension, DocumentFormat expected)
    {
        IReadOnlyList<FormatInfo> hits = FormatCatalogue.Instance.FindByExtension(extension);
        Assert.Contains(expected, hits.Select(h => h.Format));
    }

    /// <summary>
    /// '.xml' is genuinely ambiguous — flat ODF and both Office 2003 XML dialects use it —
    /// which is exactly why extensions are only ever a hint.
    /// </summary>
    [Fact]
    public void ReportsAmbiguousExtensionsAsAmbiguous()
    {
        IReadOnlyList<FormatInfo> hits = FormatCatalogue.Instance.FindByExtension("xml");
        Assert.True(hits.Count > 1, "'.xml' should map to several formats");
    }

    [Fact]
    public void ReturnsEmptyForAnUnknownExtension()
        => Assert.Empty(FormatCatalogue.Instance.FindByExtension("nosuchextension"));

    [Fact]
    public void MacroCapableFormatsAreFlagged()
    {
        // Callers surface this as a risk indicator, so the obvious ones must be right.
        foreach (DocumentFormat format in (DocumentFormat[])
                 [DocumentFormat.Docm, DocumentFormat.Xlsm, DocumentFormat.Pptm,
                  DocumentFormat.Doc, DocumentFormat.Xls, DocumentFormat.Ppt])
        {
            Assert.True(FormatCatalogue.Instance.GetInfo(format)!.CanCarryMacros,
                        $"{format} should be flagged as macro-capable");
        }

        // ODF and the plain OOXML variants cannot carry macros.
        foreach (DocumentFormat format in (DocumentFormat[])
                 [DocumentFormat.Odt, DocumentFormat.Ods, DocumentFormat.Odp,
                  DocumentFormat.Docx, DocumentFormat.Xlsx, DocumentFormat.Pptx])
        {
            Assert.False(FormatCatalogue.Instance.GetInfo(format)!.CanCarryMacros,
                         $"{format} should not be flagged as macro-capable");
        }
    }

    [Fact]
    public void TemplateFormatsAreFlagged()
    {
        foreach (DocumentFormat format in (DocumentFormat[])
                 [DocumentFormat.Dotx, DocumentFormat.Xltx, DocumentFormat.Potx,
                  DocumentFormat.Ott, DocumentFormat.Ots, DocumentFormat.Otp,
                  DocumentFormat.Dot, DocumentFormat.Xlt, DocumentFormat.Pot])
        {
            Assert.True(FormatCatalogue.Instance.GetInfo(format)!.IsTemplate,
                        $"{format} should be flagged as a template");
        }
    }

    [Fact]
    public void FamiliesArePartitionedTheWayLibreOfficeSplitsThem()
    {
        // A sanity check that nothing landed in the wrong application.
        Assert.Equal(DocumentFamily.WordProcessing, Family(DocumentFormat.Docx));
        Assert.Equal(DocumentFamily.WordProcessing, Family(DocumentFormat.Rtf));
        Assert.Equal(DocumentFamily.Spreadsheet, Family(DocumentFormat.Xlsb));
        Assert.Equal(DocumentFamily.Spreadsheet, Family(DocumentFormat.Csv));
        Assert.Equal(DocumentFamily.Presentation, Family(DocumentFormat.Ppsx));
        Assert.Equal(DocumentFamily.Presentation, Family(DocumentFormat.Odp));

        static DocumentFamily Family(DocumentFormat f) => FormatCatalogue.Instance.GetInfo(f)!.Family;
    }

    [Fact]
    public void LegacyBinaryFormatsUseTheCompoundFileContainer()
    {
        foreach (DocumentFormat format in (DocumentFormat[])
                 [DocumentFormat.Doc, DocumentFormat.Dot, DocumentFormat.Xls, DocumentFormat.Xlt,
                  DocumentFormat.Xls5, DocumentFormat.Ppt, DocumentFormat.Pot, DocumentFormat.Pps])
        {
            Assert.Equal(ContainerKind.Ole2CompoundFile,
                         FormatCatalogue.Instance.GetInfo(format)!.Container);
        }
    }

    [Fact]
    public void FlatOdfFormatsAreBareXmlNotPackages()
    {
        foreach (DocumentFormat format in (DocumentFormat[])
                 [DocumentFormat.Fodt, DocumentFormat.Fods, DocumentFormat.Fodp])
        {
            Assert.Equal(ContainerKind.Xml, FormatCatalogue.Instance.GetInfo(format)!.Container);
        }
    }
}
