using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A <c>PAGE</c> field in an RTF running head prints the page's own number, not the producer's cache.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="PageFields"/> has done this since round 45 and the DOCX, ODF and WW8 readers have all fed
/// it since; the RTF reader recorded the field for extraction and laid out the cached
/// <c>\fldrslt</c> string, so every page of a document printed whichever number the writer last saved.
/// On <c>1_tpr_template__from_fy14_.rtf</c> that is <b>2 on page 1</b>, where 26.2.4.2 prints 1 — the
/// document's first <c>PAGE</c> field caches <c>2</c>, and the ones after it cache 3, 0, 7, 7 and 8.
/// </para>
/// <para>
/// Reach: <b>122 of the 328 converted RTF carry a <c>PAGE</c> or <c>NUMPAGES</c> field</b>, 924 and 94
/// occurrences. No gate column can see most of it — a page number is one or two characters — which is
/// why it survived the first RTF gate untouched. See <c>probes/rtf-gate-r71/</c>.
/// </para>
/// </remarks>
public sealed class RtfPageNumberFieldTests
{
    /// <summary>Each page's footer carries its own number, and the cached one appears nowhere.</summary>
    [Fact]
    public void EveryPageGetsItsOwnNumber()
    {
        List<LaidOutPage> pages = Paginate();

        pages.Count.ShouldBeGreaterThan(2);
        FooterText(pages[0]).ShouldBe("Page 1");
        FooterText(pages[1]).ShouldBe("Page 2");
        FooterText(pages[2]).ShouldBe("Page 3");
    }

    /// <summary>
    /// The cached result is what a reader without this draws, and it is drawn on no page at all.
    /// </summary>
    [Fact]
    public void TheCachedResultIsNotDrawnAnywhere()
        => Paginate().ShouldAllBe(page => !FooterText(page).Contains("Page 7"));

    /// <summary>
    /// <c>NUMPAGES</c> takes the document's own length, which needs the second pass.
    /// </summary>
    [Fact]
    public void ThePageCountIsTheDocumentsOwn()
    {
        List<LaidOutPage> pages = Paginate(@" of {\field{\*\fldinst NUMPAGES }{\fldrslt 99}}");

        FooterText(pages[0]).ShouldBe($"Page 1 of {pages.Count}");
        FooterText(pages[^1]).ShouldBe($"Page {pages.Count} of {pages.Count}");
    }

    /// <summary>
    /// A field that is neither keeps its cache, which is what a reference renderer draws for it.
    /// </summary>
    /// <remarks>
    /// The control on the reading. <c>PAGE</c> and <c>NUMPAGES</c> are the two whose cached result is
    /// wrong on every page but one; a <c>REF</c> or a <c>HYPERLINK</c> result is the answer the writing
    /// application resolved and is worth keeping — see <see cref="PageFieldKind"/>.
    /// </remarks>
    [Fact]
    public void AnOrdinaryFieldKeepsItsCachedResult()
    {
        List<LaidOutPage> pages = Paginate(@" and {\field{\*\fldinst REF _Ref1 }{\fldrslt Annex B}}");

        FooterText(pages[1]).ShouldBe("Page 2 and Annex B");
    }

    private static string FooterText(LaidOutPage page)
    {
        page.Footer.ShouldNotBeNull();
        return string.Concat(page.Footer!.Blocks.OfType<PageParagraph>().Select(p => p.Text)).Trim();
    }

    /// <summary>
    /// Three sheets of body text under a footer reading <c>Page {PAGE}</c>, cached as 7.
    /// </summary>
    private static List<LaidOutPage> Paginate(string extra = "")
    {
        StringBuilder body = new();
        for (int i = 0; i < 120; i++) body.Append(@"\pard A line of ordinary body text.\par ");

        string rtf =
            @"{\rtf1\ansi\deff0{\fonttbl{\f0\froman Liberation Serif;}}"
            + @"\paperw11906\paperh16838\margl1440\margr1440\margt1440\margb1440\sectd"
            + @"{\footer\pard Page {\field{\*\fldinst PAGE }{\fldrslt 7}}" + extra + @"\par}"
            + body
            + "}";

        using DocumentSource source = DocumentSource.FromStream(
            new MemoryStream(Encoding.ASCII.GetBytes(rtf)), "pagefield.rtf");
        using IDocument document = new WordProcessingReader().Read(source);
        return [.. ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages];
    }
}
