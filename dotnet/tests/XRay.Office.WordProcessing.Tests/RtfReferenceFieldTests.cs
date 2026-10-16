using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.WordProcessing.Model;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// An RTF <c>REF</c> field draws its bookmark's text, and the RTF import names the bookmarks one
/// place out.
/// </summary>
/// <remarks>
/// <para>
/// Two mechanisms, both the reference's. RTF sends a bookmark half's <em>name</em> before its
/// <em>id</em> (<c>lcl_getBookmarkProperties</c>,
/// <c>sw/source/writerfilter/rtftok/rtfdocumentimpl.cxx</c>:224-236, whose comment says the name
/// "should be sent first") while <c>DomainMapper_Impl::SetBookmarkName</c>
/// (<c>dmapper/DomainMapper_Impl.cxx</c>:9426-9447) is written for OOXML's opposite order and writes
/// each incoming name onto the previously opened start — so every name after the first lands one
/// bookmark early. And <c>SwGetRefField::UpdateField</c> (<c>sw/source/core/fields/reffld.cxx</c>:594-670)
/// recomputes what a <c>REF</c> draws from whichever bookmark ends up holding its name, reading
/// <c>FindAnchor</c>'s <c>*pEnd = -1</c> — two ends in different nodes — as <em>to the end of the
/// paragraph</em>.
/// </para>
/// <para>
/// Every expectation here was measured against 26.2.4.2 before it was written, one file per case,
/// through <c>--convert-to fodt</c> for where the bookmarks landed and <c>--convert-to pdf</c> for
/// what the <c>REF</c> drew: <b>27 of 28 predictions exact</b>, the twenty-eighth being the flat-ODF
/// <em>shape</em> of a cross-reference bookmark rather than what it expands to.
/// <c>probes/rtf-bookmark-r88/</c> holds the probes, the simulator and the comparison.
/// </para>
/// </remarks>
public sealed class RtfReferenceFieldTests
{
    /// <summary>
    /// A bookmark opened inside another takes its name, and the outer one is left holding the inner's.
    /// </summary>
    /// <remarks>
    /// <c>probes/rtf-bookmark-r88</c>'s <c>c_nested</c>: 26.2.4.2 writes the outer mark out as
    /// <c>B Copy 1</c> and the inner as <c>B</c>, and no mark called <c>A</c> survives anywhere.
    /// </remarks>
    [Fact]
    public void ANestedBookmarkTakesTheNameOfTheOneAroundIt()
    {
        WritingMarks marks = Marks(
            @"{\*\bkmkstart A}outer {\*\bkmkstart B}inner{\*\bkmkend B} tail{\*\bkmkend A}");

        marks.Bookmarks.Select(bookmark => bookmark.Name).ShouldBe(["B", "B Copy 1"]);
        Covered(marks.Bookmarks[0]).ShouldBe("inner");
        Covered(marks.Bookmarks[1]).ShouldBe("outer inner tail");
    }

    /// <summary>
    /// Two bookmarks that do not overlap keep their own names, which is most of every document.
    /// </summary>
    /// <remarks>
    /// The control that bounds the rotation's reach: <c>SetBookmarkName</c> only overwrites while a
    /// start is still open, so a closed bookmark's name has nowhere else to go. Measured as
    /// <c>b_sequential</c>.
    /// </remarks>
    [Fact]
    public void TwoBookmarksThatDoNotOverlapAreUnaffected()
    {
        WritingMarks marks = Marks(
            @"{\*\bkmkstart B1}Alpha one{\*\bkmkend B1} and {\*\bkmkstart B2}Beta two{\*\bkmkend B2}");

        marks.Bookmarks.Select(bookmark => bookmark.Name).ShouldBe(["B1", "B2"]);
        Covered(marks.Bookmarks[0]).ShouldBe("Alpha one");
        Covered(marks.Bookmarks[1]).ShouldBe("Beta two");
    }

    /// <summary>
    /// Two starts and then two ends hand one name out twice, and Writer renames the second.
    /// </summary>
    /// <remarks>
    /// <c>MarkManager::getUniqueMarkName</c> (<c>sw/source/core/doc/docbm.cxx</c>:1826-1863) with
    /// <c>STR_MARK_COPY</c>. This is the same fault as the nesting case with nowhere to hide — the
    /// document states <c>T3</c> and <c>R1</c> and 26.2.4.2 draws <c>R1</c> and <c>R1 Copy 1</c>.
    /// </remarks>
    [Fact]
    public void OneNameHandedOutTwiceIsRenamedAsWriterRenamesIt()
    {
        WritingMarks marks = Marks(
            @"{\*\bkmkstart T3}{\*\bkmkstart R1}Table 48{\*\bkmkend R1}: Caption{\*\bkmkend T3}");

        marks.Bookmarks.Select(bookmark => bookmark.Name).ShouldBe(["R1", "R1 Copy 1"]);
    }

    /// <summary>The field draws the bookmark's text rather than the result the file cached.</summary>
    [Fact]
    public void AReferenceDrawsWhatItsBookmarkCovers()
        => Drawn(@"{\*\bkmkstart A}first {\*\bkmkstart B}middle{\*\bkmkend A} last{\*\bkmkend B}", "A")
            .ShouldBe("[A] middle last ;");

    /// <summary>
    /// A bookmark whose two ends are in different paragraphs draws the whole of the first of them.
    /// </summary>
    /// <remarks>
    /// <c>FindAnchor</c> answers <c>*pEnd = -1</c> for a mark spanning two nodes
    /// (<c>reffld.cxx</c>:1588) and <c>UpdateField</c> reads that as <c>nEnd = nLen</c>
    /// (<c>:604-607</c>). It is what turns the rotation into whole paragraphs of drawn text: the
    /// rotation moves an end onto a bookmark that started elsewhere, and this expands it.
    /// </remarks>
    [Fact]
    public void ABookmarkEndingInAnotherParagraphDrawsThatParagraphWhole()
        => Drawn(@"{\*\bkmkstart A}first para\par second para{\*\bkmkend A}", "A")
            .ShouldBe("[A] first para ;");

    /// <summary>A collapsed bookmark draws nothing at all, cached result included.</summary>
    /// <remarks><c>*pEnd = *pStart</c> (<c>reffld.cxx</c>:1577), so the range is empty.</remarks>
    [Fact]
    public void ACollapsedBookmarkDrawsNothing()
        => Drawn(@"{\*\bkmkstart A}{\*\bkmkend A}Alpha one", "A").ShouldBe("[A]  ;");

    /// <summary>
    /// A collapsed <em>cross-reference</em> bookmark draws its whole paragraph instead.
    /// </summary>
    /// <remarks>
    /// #i81002#, <c>reffld.cxx</c>:1578-1586 — the one exception to the line above, and the shape
    /// Writer's own heading references take.
    /// </remarks>
    [Fact]
    public void ACollapsedCrossReferenceBookmarkDrawsItsParagraph()
        => Drawn(
                @"{\*\bkmkstart __RefHeading___Toc1}{\*\bkmkend __RefHeading___Toc1}Heading text here",
                "__RefHeading___Toc1")
            .ShouldBe("[__RefHeading___Toc1] Heading text here ;");

    /// <summary>
    /// A <c>REF</c> asking for something other than the text keeps the producer's cached result.
    /// </summary>
    /// <remarks>
    /// <c>\p</c>, <c>\r</c>, <c>\n</c> and <c>\w</c> set a <c>ReferenceFieldPart</c> other than
    /// <c>TEXT</c> (<c>DomainMapper_Impl.cxx</c>:8611-8632) — an above/below word or one of three
    /// numberings, none of which this computes. The cache is a better answer than a wrong one, which
    /// is the same rule <c>FILENAME \p</c> follows.
    /// </remarks>
    [Fact]
    public void AReferenceAskingForANumberKeepsItsCache()
        => Drawn(@"{\*\bkmkstart A}first {\*\bkmkstart B}middle{\*\bkmkend A} last{\*\bkmkend B}",
                "A", switches: @"\\r ")
            .ShouldBe("[A] CACHED ;");

    /// <summary>
    /// A <c>REF</c> naming a bookmark this reader does not hold keeps its cache, where the reference
    /// draws an error.
    /// </summary>
    /// <remarks>
    /// The one measured divergence in this family and a deliberate one. 26.2.4.2 draws
    /// <c>STR_GETREFFLD_REFITEMNOTFOUND</c> — "Error: Reference source not found" — for a name no
    /// mark holds (<c>reffld.cxx</c>:594-598), which the rotation itself arranges whenever it leaves
    /// a name on nobody. Reproducing that would mean drawing an error message wherever <em>our</em>
    /// bookmark table is the incomplete one, and the cost of not reproducing it is measured:
    /// <b>two occurrences across the 13 converted <c>.rtf</c> that state a <c>REF</c> at all</b>.
    /// </remarks>
    [Fact]
    public void AReferenceToANameNobodyHoldsKeepsItsCache()
        => Drawn(@"{\*\bkmkstart A}outer {\*\bkmkstart B}inner{\*\bkmkend B} tail{\*\bkmkend A}", "A")
            .ShouldBe("[A] CACHED ;");

    /// <summary>
    /// A group nested inside a bookmark's name is part of that name, not a half of its own.
    /// </summary>
    /// <remarks>
    /// <para>
    /// It inherits the destination, so a reader that records a half per closing group records a
    /// spurious one — and under the rotation that is not free: it takes an id, and an <em>end</em>
    /// naming nothing takes <c>m_aBookmarks[""]</c>, which <c>std::map</c> value-initialises to
    /// <b>0</b> and which therefore closes the document's first bookmark under the wrong name.
    /// <c>RTFDocumentImpl::popState</c> guards it with <c>if (&amp;getDestinationText() !=
    /// getCurrentDestinationText()) break; // not for nested group</c>
    /// (<c>rtfdocumentimpl.cxx</c>:2736-2740, :2751-2755).
    /// </para>
    /// <para>
    /// The same test says the two groups share one <em>buffer</em>, so the nested text is part of the
    /// name rather than dropped with the half: 26.2.4.2's flat ODF for <c>{\*\bkmkend {x}A}</c>
    /// holds one bookmark called <b><c>xA</c></b>, and its <c>REF A</c> then finds nothing.
    /// The control beside it — the same document without the nested group — draws <c>first</c>.
    /// <c>probes/rtf-bookmark-r88/gennested.py</c>.
    /// </para>
    /// </remarks>
    [Fact]
    public void AGroupInsideABookmarksNameIsPartOfTheName()
    {
        const string nested = @"{\*\bkmkstart A}first{\*\bkmkend {x}A} second";
        const string control = @"{\*\bkmkstart A}first{\*\bkmkend A} second";

        WritingBookmark marked = Marks(nested).Bookmarks.Single();
        marked.Name.ShouldBe("xA");
        Covered(marked).ShouldBe("first");

        Marks(control).Bookmarks.Single().Name.ShouldBe("A");
        Drawn(control, "A").ShouldBe("[A] first ;");
        Drawn(nested, "A").ShouldBe("[A] CACHED ;");
    }

    /// <summary>The text a bookmark covers, as extraction reports the paragraph.</summary>
    private static string Covered(WritingBookmark bookmark)
    {
        WritingRange range = bookmark.Range;
        string text = range.Start.Paragraph.Text;
        return text[range.Start.Offset..Math.Min(range.End.Offset, text.Length)];
    }

    private static WritingMarks Marks(string body)
    {
        using IWordProcessingDocument document = Open(body, name: null, switches: string.Empty);
        return document.Marks;
    }

    /// <summary>The last paragraph's text, which is the one holding the <c>REF</c> field.</summary>
    private static string Drawn(string body, string name, string switches = @"\\h ")
    {
        using IWordProcessingDocument document = Open(body, name, switches);
        return Paragraphs(document).Last().Trim();
    }

    private static IEnumerable<string> Paragraphs(IDocument document)
        => Descendants(document.Content).OfType<ContentParagraph>()
                                        .Select(paragraph => paragraph.GetText())
                                        .Where(text => text.Trim().Length > 0);

    private static IEnumerable<ContentNode> Descendants(ContentNode node)
    {
        foreach (ContentNode child in node.Children)
        {
            yield return child;
            foreach (ContentNode descendant in Descendants(child)) yield return descendant;
        }
    }

    private static IWordProcessingDocument Open(string body, string? name, string switches)
    {
        string reference = name is null
            ? string.Empty
            : $@"\pard\plain [{name}] {{\field{{\*\fldinst  REF {name} {switches}}}{{\fldrslt CACHED}}}} ;\par ";

        string rtf =
            @"{\rtf1\ansi\deff0{\fonttbl{\f0\froman Liberation Serif;}}"
            + @"\paperw11906\paperh16838\margl1134\margr1134\margt1134\margb1134"
            + @"\pard\plain " + body + @"\par "
            + reference
            + "}";

        DocumentSource source = DocumentSource.FromStream(
            new MemoryStream(Encoding.ASCII.GetBytes(rtf)), "reference.rtf");
        return (IWordProcessingDocument)new WordProcessingReader().Read(source);
    }
}
