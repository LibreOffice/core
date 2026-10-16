using System.Xml.Linq;
using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A run inside a <c>TOC</c> field's stored result loses the character style it names, in every
/// paragraph of the result and not only in the first.
/// </summary>
/// <remarks>
/// <para>
/// Word writes a table of contents as a field whose cached result is a run of paragraphs, each entry a
/// <c>w:hyperlink</c> around a run naming the <c>Hyperlink</c> character style — which states an
/// underline. LibreOffice imports the field as a live Writer index and dresses its entries in
/// <c>Index Link</c>, <c>STR_POOLCHR_TOXJUMP</c> (<c>sw/inc/strings.hrc</c>:48), which is an
/// <em>empty</em> pool style: <c>DomainMapper_Impl.cxx</c>:9355-9360 sets
/// <c>VisitedCharStyleName</c>/<c>UnvisitedCharStyleName</c> for every hyperlink under
/// <c>IsInTOC()</c>, and <c>DomainMapper.cxx</c>:3037-3047 declines to insert
/// <c>PROP_CHAR_STYLE_NAME</c> for the same reason — <em>"do not add it elements in TOC: they will
/// receive later another style references from TOC"</em>. It is not an OOXML rule:
/// <c>sw/source/filter/ww8/ww8par5.cxx</c>:2334, 3362-3363 and 3653 give a WW8 index link the same
/// style.
/// </para>
/// <para>
/// <b>The paragraph boundary is the whole of it.</b> The suppression was implemented and could only
/// ever reach one entry, because a <see cref="DocxLayoutSource"/> builds a fresh walker per paragraph
/// while the field's <c>fldChar begin</c>, its instruction and its <c>separate</c> all sit in the
/// first entry's paragraph and its <c>end</c> in the last. On <c>150-5370-10H.docx</c> page 7 that is
/// one of 43 contents lines, and the page drew 9413 pt of rule against 26.2.4.2's 468 before this and
/// 468.00 after. <c>probes/wordsdec-r126</c>.
/// </para>
/// </remarks>
public sealed class DocxIndexFieldTests
{
    /// <summary>Every entry of a multi-paragraph contents list loses the style, not just the first.</summary>
    [Fact]
    public void EveryParagraphOfATocResultDropsTheCharacterStyleAndNotOnlyTheFirst()
    {
        List<PageParagraph> entries = Entries(Body(" TOC \\o \"1-3\" \\h \\z \\u "));

        entries.Count.ShouldBe(3);
        entries.ShouldAllBe(entry => Underlined(entry) == 0);

        // Not vacuously: each entry keeps two runs, because the italic beside the link makes the
        // paragraph non-uniform. An assertion over an empty run list passes for the wrong reason.
        entries.ShouldAllBe(entry => entry.Runs.Count == 2);
    }

    /// <summary>A field that is not an index leaves its result's runs exactly as the file states them.</summary>
    /// <remarks>
    /// The control, and it is the one that says the suppression is keyed on the field's name rather
    /// than on a hyperlink: the same three paragraphs under a <c>HYPERLINK</c> field keep the
    /// <c>Hyperlink</c> style and stay underlined, which is what Word and the reference both draw.
    /// </remarks>
    [Fact]
    public void AFieldThatIsNotAnIndexKeepsItsResultsCharacterStyle()
    {
        List<PageParagraph> entries = Entries(Body(" HYPERLINK \"http://example.invalid\" "));

        entries.Count.ShouldBe(3);
        entries.ShouldAllBe(entry => Underlined(entry) == 1);
    }

    /// <summary>The suppression ends with the field, not with the document.</summary>
    /// <remarks>
    /// The depth is carried across paragraphs, so the <c>fldChar end</c> that closes the field arrives
    /// with nothing of that field's on the paragraph's own stack — and a walk that did not close it
    /// there would strip the character style from every run in the rest of the document.
    /// </remarks>
    [Fact]
    public void ARunAfterTheFieldsEndKeepsItsCharacterStyle()
    {
        XElement body = Body(" TOC \\o \"1-3\" \\h \\z \\u ");
        body.Add(Entry("after the field", closing: false));

        List<PageParagraph> paragraphs = Entries(body);
        paragraphs.Count.ShouldBe(4);
        Underlined(paragraphs[^1]).ShouldBe(1);
    }

    /// <summary>How many of a paragraph's runs carry an underline.</summary>
    private static int Underlined(PageParagraph paragraph)
        => paragraph.Runs.Count(run => run.Underline != TextUnderline.None);

    private static readonly XNamespace W =
        "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    private static List<PageParagraph> Entries(XElement body)
    {
        WordStyles styles = new();
        styles.Add(new XElement(
            W + "styles",
            new XElement(
                W + "style",
                new XAttribute(W + "type", "character"),
                new XAttribute(W + "styleId", "Hyperlink"),
                new XElement(W + "name", new XAttribute(W + "val", "Hyperlink")),
                new XElement(
                    W + "rPr",
                    new XElement(W + "u", new XAttribute(W + "val", "single"))))));

        return [.. new DocxLayoutSource(styles).Read(body).OfType<PageParagraph>()];
    }

    /// <summary>
    /// A field whose <c>begin</c> shares a paragraph with the first entry and whose <c>end</c> shares
    /// one with the last, which is the shape Word writes and the shape a per-paragraph walk cannot see.
    /// </summary>
    private static XElement Body(string instruction)
    {
        XElement first = Entry("first entry", closing: false);
        first.AddFirst(
            new XElement(W + "r", FieldChar("begin")),
            new XElement(
                W + "r",
                new XElement(
                    W + "instrText",
                    new XAttribute(XNamespace.Xml + "space", "preserve"),
                    instruction)),
            new XElement(W + "r", FieldChar("separate")));

        return new XElement(
            W + "body", first, Entry("second entry", closing: false), Entry("third entry", closing: true));
    }

    private static XElement Entry(string text, bool closing)
    {
        XElement paragraph = new(
            W + "p",
            new XElement(
                W + "hyperlink",
                new XAttribute(W + "anchor", "_Toc1"),
                new XElement(
                    W + "r",
                    new XElement(W + "rPr", new XElement(W + "rStyle", new XAttribute(W + "val", "Hyperlink"))),
                    new XElement(W + "t", text))),
            // A second, differently formatted run, so the paragraph can never collapse to a uniform
            // one with no `Runs` at all — an assertion over an empty list passes whatever the fix does.
            new XElement(
                W + "r",
                new XElement(W + "rPr", new XElement(W + "i")),
                new XElement(W + "t", " tail")));

        if (closing) paragraph.Add(new XElement(W + "r", FieldChar("end")));

        return paragraph;
    }

    private static XElement FieldChar(string type)
        => new(W + "fldChar", new XAttribute(W + "fldCharType", type));
}
