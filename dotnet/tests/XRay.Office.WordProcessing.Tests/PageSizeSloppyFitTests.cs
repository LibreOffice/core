using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A <c>w:pgSz</c> within 0.44 mm of a standard paper dimension is that dimension.
/// </summary>
/// <remarks>
/// <para>
/// Word files carry near-miss page sizes constantly — 11910 × 16840 twips, 11912 × 16851, 11900 ×
/// 16830 — all of them A4 with a rounding scar from some earlier round trip. LibreOffice erases
/// them on import: <c>DomainMapper.cxx</c>:827 and :836 pass each <c>w:pgSz</c> dimension through
/// <c>PaperInfo::sloppyFitPageDimension</c>, and <c>ww8par6.cxx</c>:521 and :1083 do the same for
/// DOC through <c>SvxPaperInfo::GetSloppyPaperDimension</c>. Twenty-two DOCX and nine DOC of the
/// two hundred documents on the words track state such a size.
/// </para>
/// <para>
/// The numbers below are not read off the C++ tree, which in this checkout is 27.2.0.0.alpha0+ and
/// describes a binary that made none of the reference renderings. They are measured on the
/// installed 26.2.4.2 by sweeping an authored DOCX's stated page height one twip at a time
/// (<c>dotnet/probes/words-d-01/papersnap.py</c>): 16814 through 16862 twips all come back as a
/// 841.89 pt media box, 16813 and 16863 come back as themselves. Those two edges are what the
/// boundary tests here pin.
/// </para>
/// </remarks>
public sealed class PageSizeSloppyFitTests
{
    /// <summary>A4 as LibreOffice lays it out: 21000 × 29700 hundredths of a millimetre, in twips.</summary>
    private static readonly Length A4Width = Length.FromTwips(11906);

    private static readonly Length A4Height = Length.FromTwips(16838);

    /// <summary>
    /// The case the corpus is full of: a stated size a few tenths of a millimetre off A4.
    /// </summary>
    /// <remarks>
    /// 11912 × 16851 twips is <c>docs-quality-MA.IMS.00001</c>'s stated size, and the reference PDF
    /// for it has a 595.304 × 841.89 pt media box — A4, not what the document says.
    /// </remarks>
    [Fact]
    public void APageSizeJustOffA4IsFittedToA4()
    {
        (Length width, Length height) = Sheet(11912, 16851);

        width.ShouldBe(A4Width);
        height.ShouldBe(A4Height);
    }

    /// <summary>The far edge of the window, one twip inside it.</summary>
    [Fact]
    public void ThePageDimensionOneTwipInsideTheWindowIsStillFitted()
    {
        Sheet(11906, 16862).Height.ShouldBe(A4Height);
        Sheet(11906, 16814).Height.ShouldBe(A4Height);
    }

    /// <summary>
    /// One twip further out and the stated size stands, exactly as it is written.
    /// </summary>
    /// <remarks>
    /// This is the assertion that makes the rule a window rather than a magnet, and it is the one a
    /// looser tolerance would silently break: 16863 twips is 0.4407 mm from A4 and 16862 is 0.4295,
    /// so the two differ by a hundredth of a millimetre and by nothing else.
    /// </remarks>
    [Fact]
    public void ThePageDimensionOneTwipOutsideTheWindowIsLeftAlone()
    {
        Sheet(11906, 16863).Height.ShouldBe(Length.FromTwips(16863));
        Sheet(11906, 16813).Height.ShouldBe(Length.FromTwips(16813));
    }

    /// <summary>
    /// A page size nowhere near a standard one passes through untouched.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The control that a corpus-wide fit would fail: most of the two hundred documents state a
    /// size that is already exact, and a rule that moved those would be a regression on every one
    /// of them.
    /// </para>
    /// <para>
    /// 8561 × 13850 twips — 151 × 244 mm — is the unusual one, and finding it took a search: the
    /// table holds 96 distinct dimensions between 26 mm and 1414 mm, so a round number like 10000
    /// twips is <em>not</em> unusual. It is 176.39 mm, which is 0.39 mm from ISOB5's 176 and inside
    /// the window. Two of this file's own assertions asserted the wrong answer for exactly that
    /// reason before they were run.
    /// </para>
    /// </remarks>
    [Fact]
    public void AnExactAndAnUnusualPageSizeBothPassThrough()
    {
        Sheet(11906, 16838).ShouldBe((A4Width, A4Height));
        Sheet(12240, 15840).ShouldBe((Length.FromTwips(12240), Length.FromTwips(15840)));
        Sheet(8561, 13850).ShouldBe((Length.FromTwips(8561), Length.FromTwips(13850)));
    }

    /// <summary>
    /// Each dimension is fitted on its own, against every dimension in the table — a page's width
    /// may end up as some other paper's height.
    /// </summary>
    /// <remarks>
    /// Measured rather than assumed: a stated 20638 × 25000 twips renders on 26.2.4.2 with a
    /// 1031.81 pt wide media box, which is 364 mm — B4(JIS)'s <em>height</em>, and no paper's
    /// width — while its height, 441 mm and near nothing, is left alone. A rule that matched whole
    /// paper formats would have left both.
    /// </remarks>
    [Fact]
    public void ADimensionIsFittedAgainstEveryDimensionAndNotAgainstWholeFormats()
    {
        (Length width, Length height) = Sheet(20638, 25000);

        // 364 mm = 36400 hundredths of a millimetre = 20636 twips.
        width.ShouldBe(Length.FromTwips(20636));
        height.ShouldBe(Length.FromTwips(25000));
    }

    /// <summary>RTF reaches the same rule, through <c>\paperw</c> and <c>\paperh</c>.</summary>
    /// <remarks>
    /// Not a parallel implementation on LibreOffice's side either:
    /// <c>rtfdispatchvalue.cxx</c>:1274-1289 dispatches those tokens through
    /// <c>LN_CT_PageSz_w</c>/<c>_h</c>, which is the <c>DomainMapper</c> case that applies the fit.
    /// The words track holds no RTF at all, so this test is the only evidence for that arm — the
    /// corpus sweep cannot reach it, and saying so is the point of the test existing.
    /// </remarks>
    [Fact]
    public void AnRtfPaperSizeIsFittedTheSameWay()
    {
        using DocumentSource source = DocumentSource.FromStream(
            new MemoryStream(Encoding.ASCII.GetBytes(
                @"{\rtf1\ansi\paperw11912\paperh16851\margl720\margr720\margt720\margb720"
                + @"\pard One paragraph on one sheet.\par}")),
            "page.rtf");
        using IDocument document = new WordProcessingReader().Read(source);

        LaidOutPage page = ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages[0];

        page.Size.Width.ShouldBe(A4Width);
        page.Size.Height.ShouldBe(A4Height);
    }

    private static (Length Width, Length Height) Sheet(int widthTwips, int heightTwips)
    {
        MemoryStream package = BuildPackage(widthTwips, heightTwips);
        using DocumentSource source = DocumentSource.FromStream(package, "page.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        LaidOutPage page = ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages[0];
        return (page.Size.Width, page.Size.Height);
    }

    private static MemoryStream BuildPackage(int widthTwips, int heightTwips)
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
              <Override PartName="/word/settings.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
            </Types>
            """;

        const string RootRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
            </Relationships>
            """;

        const string DocumentRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="settings.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"/>
            </Relationships>
            """;

        // Carried from the neighbouring geometry tests: a hand-built DOCX with no settings part misses
        // LibreOffice's OOXML compatibility defaults and can give a clean, consistent, wrong answer.
        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>
            """;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p><w:r><w:t>One paragraph on one sheet.</w:t></w:r></w:p>
                <w:sectPr>
                  <w:pgSz w:w="{widthTwips}" w:h="{heightTwips}"/>
                  <w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720"
                           w:header="0" w:footer="0" w:gutter="0"/>
                </w:sectPr>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/settings.xml", Settings);
            Write(archive, "word/document.xml", document);
        }

        result.Position = 0;
        return result;

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(Encoding.UTF8.GetBytes(content));
        }
    }
}
