using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ww8;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A WW8 tracked deletion is drawn or dropped according to the document's own <c>fRMView</c>.
/// </summary>
/// <remarks>
/// <para>
/// Writer reads it straight off the <c>Dop</c> — <c>isHideRedlines = !m_xWDop-&gt;fRMView</c>,
/// <c>sw/source/filter/ww8/ww8par.cxx</c>:5262 — and the bit is byte 0x07's 0x08
/// (<c>ww8scan.cxx</c>:7681). Hidden text, <c>sprmCFVanish</c>, is a separate question and is never
/// drawn whatever the document says.
/// </para>
/// <para>
/// <strong>Both behaviours are in the corpus, which is what makes this a document property rather than
/// a policy.</strong> <c>revisions.doc</c> sets the bit and LibreOffice's own PDF of it reads "an
/// inserted phrase and a deleted phrase in the middle"; <c>150_5300_13_chg8.doc</c> clears it and the
/// reference shows none of its deletions — that document rendered 19 pages against 18, with its
/// deletions run together with the insertions replacing them ("may varyis determined by TERPS" for the
/// reference's "is determined by TERPS", "Visibility Mminimums" for "visibility minimums"), and now
/// renders 18.
/// </para>
/// <para>
/// The layout path needed this because it resolves character formatting separately from the content
/// path, and only the content path tested for a deletion at all — so <c>xray-office extract</c> on chg8
/// was clean while rendering it was not.
/// </para>
/// <para>
/// The first cut dropped deletions unconditionally, on the reasoning that layout treats hidden text and
/// deletions alike. That rendered <c>revisions.doc</c> without its deleted phrase — a regression no
/// corpus sweep could have caught, because the file is a fixture rather than a corpus document, and one
/// the reference itself contradicted the moment it was checked.
/// </para>
/// </remarks>
public sealed class TrackedDeletionRenderingTests
{
    /// <summary>
    /// A document that asks for its changes to be shown keeps its deleted text on the page.
    /// </summary>
    /// <remarks>
    /// The guard against over-dropping, and the case the reference settles: LibreOffice draws the
    /// deletion struck through, so the words are in its PDF's text layer and must be in ours.
    /// </remarks>
    [Fact]
    public void ADeletionIsDrawnWhenTheDocumentShowsItsChanges()
    {
        string text = DrawnText("revisions.doc");

        text.ShouldContain(
            "deleted", Case.Insensitive,
            "revisions.doc sets fRMView, so LibreOffice draws the deletion and so must we");
    }

    /// <summary>And its inserted text too, which is drawn in every document.</summary>
    /// <remarks>
    /// Without this the test above would pass against a reader that drew nothing at all from the
    /// paragraph.
    /// </remarks>
    [Fact]
    public void AnInsertionIsDrawnAsOrdinaryText()
    {
        DrawnText("revisions.doc").ShouldContain("inserted", Case.Insensitive);
    }

    /// <summary>The bit itself, read off a <c>Dop</c> byte rather than through a document.</summary>
    /// <remarks>
    /// A corpus document is only available for the cleared case, so the set case is pinned here
    /// instead. Byte 0x07 bit 0x08 — its neighbours in the same byte are
    /// <c>fPagSuppressTopSpacing</c>, <c>fProtEnabled</c>, <c>fDispFormFieldSel</c>, <c>fRMPrint</c>,
    /// <c>fWriteReservation</c>, <c>fLockRev</c> and <c>fEmbedFonts</c>, so a mask that slipped by one
    /// would read a different setting and still look plausible.
    /// </remarks>
    [Theory]
    [InlineData(0x08, false)]
    [InlineData(0x00, true)]
    [InlineData(0xFF, false)]
    [InlineData(0xF7, true)]
    public void TheDopSaysWhetherChangesAreHidden(byte flags, bool hides)
    {
        byte[] dop = new byte[64];
        dop[0x07] = flags;

        Ww8DocumentProperties.Parse(dop).HidesTrackedChanges.ShouldBe(hides);
    }

    /// <summary>
    /// A <c>Dop</c> too short to reach the byte shows its changes rather than dropping them.
    /// </summary>
    /// <remarks>
    /// The safe direction, and why the property is stated as "hides" rather than "shows": this is a
    /// struct, so a default instance cannot carry an initialiser, and dropping text on a truncated
    /// record would be silent content loss. It also matches <c>WW8Dop</c>'s own default constructor,
    /// which sets <c>fRMView(true)</c> (<c>ww8scan.cxx</c>:7845).
    /// </remarks>
    [Fact]
    public void ATruncatedDopShowsItsChanges()
    {
        Ww8DocumentProperties.Parse(new byte[4]).HidesTrackedChanges.ShouldBeFalse();
        Ww8DocumentProperties.Parse([]).HidesTrackedChanges.ShouldBeFalse();
        default(Ww8DocumentProperties).HidesTrackedChanges.ShouldBeFalse();
    }

    /// <summary>Every word the document's pages actually draw, in order.</summary>
    private static string DrawnText(string fileName)
    {
        using DocumentSource source = DocumentSource.FromFile(Corpus.Require(fileName));
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return string.Join(
            " ", pages.Pages.SelectMany(page => page.Lines).Select(pages.TextOf));
    }
}
