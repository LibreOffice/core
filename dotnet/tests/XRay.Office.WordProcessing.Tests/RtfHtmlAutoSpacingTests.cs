using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// <c>\htmautsp</c>, which decides whether two adjacent paragraphs' spacings are summed or
/// collapsed — and the window in which stating it still counts.
/// </summary>
/// <remarks>
/// <para>
/// RTF is the opt-in direction: <c>SettingsTable</c>'s constructor sets
/// <c>m_bDoNotUseHTMLParagraphAutoSpacing</c> true for an RTF import outright —
/// <em>"HTML paragraph auto-spacing is opt-in for RTF, opt-out for OOXML"</em>,
/// <c>sw/source/writerfilter/dmapper/SettingsTable.cxx</c>:118-126 — so an RTF that states nothing
/// <em>adds</em> the two spacings, and <c>\htmautsp</c> clears the flag
/// (<c>rtfdispatchflag.cxx</c>:1354-1357) so the larger wins instead.
/// </para>
/// <para>
/// <strong>But the settings table is sent once, at the document's first run.</strong>
/// <c>RTFDocumentImpl::checkFirstRun</c> calls <c>outputSettingsTable</c>
/// (<c>rtfdocumentimpl.cxx</c>:414-435), so a <c>\htmautsp</c> written after the body has begun is
/// stored in an <c>m_aSettingsTableSprms</c> nobody reads again — the same fact
/// <c>rtfsprm.cxx</c>:238-241 records in a comment of its own. <c>150-5370-10H.rtf</c> writes it
/// 265 KB into a 5 MB document, and honouring it there collapsed every paragraph boundary in a
/// 746-page specification.
/// </para>
/// </remarks>
public sealed class RtfHtmlAutoSpacingTests
{
    /// <summary>With no <c>\htmautsp</c> the two spacings add: 24 pt above 24 pt.</summary>
    [Fact]
    public void WithoutTheWordTheSpacingsAdd()
        => UpperSpace(string.Empty).Points.ShouldBe(13.8 + 24 + 24, 0.4);

    /// <summary>Stated before any body content, the larger wins.</summary>
    [Fact]
    public void StatedBeforeTheBodyTheLargerWins()
        => UpperSpace(@"\htmautsp").Points.ShouldBe(13.8 + 24, 0.4);

    /// <summary>Stated after the body has begun, it is too late to be read.</summary>
    [Fact]
    public void StatedAfterTheBodyHasBegunItIsIgnored()
        => UpperSpace(string.Empty, late: true).Points.ShouldBe(13.8 + 24 + 24, 0.4);

    /// <summary>
    /// A <c>{\header}</c> group does not begin the body, because a header is a substream resolved
    /// when its section ends rather than where its group was written.
    /// </summary>
    [Fact]
    public void AHeaderGroupDoesNotBeginTheBody()
        => UpperSpace(@"{\header\pard\plain\fs20 HEAD\par }\htmautsp")
            .Points.ShouldBe(13.8 + 24, 0.4);

    /// <summary>
    /// <c>\super</c> in a list level ends the window, which is the one caller that can stand in a
    /// document's preamble.
    /// </summary>
    /// <remarks>
    /// <c>RTFDocumentImpl::dispatchFlag</c>'s <c>SUPER</c> case calls <c>checkFirstRun</c> for
    /// anything that is not a style-sheet entry
    /// (<c>sw/source/writerfilter/rtftok/rtfdispatchflag.cxx</c>:887-895), and a list table is
    /// <c>Destination::LISTTABLE</c> rather than <c>SKIP</c>, so its control words really are
    /// dispatched. LibreOffice's own RTF export writes <c>\super</c> into a numbering level whose
    /// text is superscript and writes <c>\htmautsp</c> after the list table, so the word is lost:
    /// on <c>150-5370-10H.rtf</c> 26.2.4.2 draws 746 pages with the word and 746 without it.
    /// </remarks>
    [Fact]
    public void ASuperInAListLevelEndsTheWindow()
        => UpperSpace(ListTable(@"\super") + @"\htmautsp").Points.ShouldBe(13.8 + 24 + 24, 0.4);

    /// <summary>The same list table without that one control word leaves the window open.</summary>
    [Fact]
    public void AListLevelWithoutASuperLeavesTheWindowOpen()
        => UpperSpace(ListTable(string.Empty) + @"\htmautsp").Points.ShouldBe(13.8 + 24, 0.4);

    /// <summary>
    /// <c>\sub</c> is not <c>\super</c>: only the superscript arm calls <c>checkFirstRun</c>, and
    /// <c>\nosupersub</c> beside it does not either (<c>rtfdispatchflag.cxx</c>:904-918).
    /// </summary>
    [Fact]
    public void ASubInAListLevelLeavesTheWindowOpen()
        => UpperSpace(ListTable(@"\sub") + @"\htmautsp").Points.ShouldBe(13.8 + 24, 0.4);

    /// <summary>
    /// A <c>\super</c> in a <c>{\header}</c> does not end it, because nothing inside a
    /// <c>Destination::SKIP</c> group is dispatched at all
    /// (<c>RTFTokenizer::dispatchKeyword</c>, <c>rtftokenizer.cxx</c>:2156-2164).
    /// </summary>
    /// <remarks>
    /// <c>FRE-03_mcar_part-3_and_IS_v2.9.rtf</c> is the corpus witness: <c>\super</c> at byte
    /// 330901 inside a <c>{\header}</c>, <c>\htmautsp</c> at 331546, and counting the first costs
    /// that document three pages.
    /// </remarks>
    [Fact]
    public void ASuperInAHeaderDoesNotEndTheWindow()
        => UpperSpace(@"{\header\pard\plain\fs20\super HEAD\par }\htmautsp")
            .Points.ShouldBe(13.8 + 24, 0.4);

    /// <summary>
    /// Nor does one in a style-sheet entry, which the caller excludes by name.
    /// </summary>
    [Fact]
    public void ASuperInAStyleSheetEntryDoesNotEndTheWindow()
        => UpperSpace(@"{\stylesheet{\s0\snext0\ql Normal;}{\s1\super Note reference;}}\htmautsp")
            .Points.ShouldBe(13.8 + 24, 0.4);

    /// <summary>
    /// One in the body does end it, with no text and no <c>\par</c> to end it instead.
    /// </summary>
    [Fact]
    public void ASuperInTheBodyEndsTheWindow()
        => UpperSpace(@"\pard\plain\super \htmautsp").Points.ShouldBe(13.8 + 24 + 24, 0.4);

    /// <summary>
    /// Reading the list table rather than skipping it must not put any of it on the page: it is
    /// dispatched for its control words alone.
    /// </summary>
    [Fact]
    public void AListTableContributesNoText()
    {
        string text = Extract(ListTable(@"\super"));
        text.ShouldNotContain("Level");
        text.ShouldContain("AAAA");
        text.ShouldContain("BBBB");
    }

    /// <summary>
    /// A one-level list table in the shape LibreOffice's RTF export writes, with an optional extra
    /// control word on the level itself.
    /// </summary>
    private static string ListTable(string extra) =>
        @"{\*\listtable{\list\listtemplateid1{\listlevel\levelnfc0\leveljc0\levelstartat1"
        + @"\levelfollow0{\leveltext \'01\'00;}{\levelnumbers\'01;}" + extra
        + @"\fi-360\li540 Level;}\listid1}}";

    /// <summary>Everything the reader extracts, as one string.</summary>
    private static string Extract(string prologue)
    {
        using DocumentSource source = DocumentSource.FromStream(
            new MemoryStream(Encoding.ASCII.GetBytes(Document(prologue, late: false))),
            "spacing.rtf");
        using IDocument document = new WordProcessingReader().Read(source);
        return document.Content.GetText();
    }

    /// <summary>
    /// The distance from the first paragraph's line to the second's: one 13.8 pt line plus
    /// whatever the two spacings come to once the collapsing rule has been applied.
    /// </summary>
    private static Length UpperSpace(string prologue, bool late = false)
    {
        using DocumentSource source = DocumentSource.FromStream(
            new MemoryStream(Encoding.ASCII.GetBytes(Document(prologue, late))), "spacing.rtf");
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        PlacedLine Line(string text) => pages.Pages[0].Lines
            .First(line => pages.TextOf(line).Contains(text, StringComparison.Ordinal));

        return Line("BBBB").Top - Line("AAAA").Top;
    }

    /// <summary>Two paragraphs of 24 pt spacing, behind whatever the case wants to state first.</summary>
    private static string Document(string prologue, bool late) =>
        @"{\rtf1\ansi\deff0{\fonttbl{\f0\froman Liberation Serif;}}"
        + @"\paperw11906\paperh16838\margl1440\margr1440\margt1440\margb1440\sectd"
        + prologue
        + (late ? @"\pard\plain ZERO\par \htmautsp" : string.Empty)
        + @"\pard\plain\sa480 AAAA\par "
        + @"\pard\plain\sb480 BBBB\par }";
}
