using System.IO.Compression;
using System.Xml.Linq;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// 26.2.4.2 voids the direct paragraph formatting of every paragraph in a built-in
/// <c>heading N</c> that a <c>TOC \t</c> switch names — and this tree deliberately does not.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Word honours the paragraph, so by default this tree does too.</strong> A <c>TOC</c>
/// carrying <c>\t "Heading 2,1,Heading 3,2"</c> registers those styles as the index's source styles
/// (<c>DomainMapper_Impl::handleToc</c>, <c>DomainMapper_Impl.cxx</c>:7663-7707), and from then on
/// 26.2.4.2 imports every paragraph in one of them, anywhere in the document, with its <c>w:pPr</c>
/// beyond the <c>w:pStyle</c> discarded. That is a quirk of one importer rather than anything the
/// format says, and reproducing it means drawing a document differently from the application that
/// wrote it. <c>DocxTocStyles.Variable</c> switches it on for a run scored against LibreOffice;
/// <c>dotnet/TODO.word-parity.md</c> records what leaving it off costs on the corpus.
/// </para>
/// <para>
/// Both halves are pinned here, because the expensive half of this round was the measurement and it
/// should not have to be made twice. The first group asserts what this tree draws — the paragraph's
/// own formatting. The second asserts the reference's answer through
/// <c>DocxTocStyles.Resolve</c>/<c>Prune</c>, against 26.2.4.2's own <c>--convert-to fodt</c> of the
/// committed fixture <c>tests/corpus/features/words-toc-template-styles.docx</c>
/// (<c>probes/tocstyle-r160/reference-arms.txt</c>).
/// </para>
/// <para>
/// Three further controls could not live in that fixture, because the registration is document-wide
/// and its arms would void them: a <c>\t</c> naming only <c>Heading 2</c>, a <c>\o</c> switch with
/// no <c>\t</c>, and a document with no <c>TOC</c> at all. Each is its own file under
/// <c>probes/tocstyle-r160/</c> and each keeps its direct formatting in the reference too.
/// </para>
/// </remarks>
public sealed class DocxTocTemplateStyleTests
{
    private const string Fixture = "words-toc-template-styles.docx";

    /// <summary>The 6 pt the fixture's styles state and whose arms state 0.</summary>
    private static readonly Length StyleSpacing = Length.FromTwips(120);

    /// <summary>
    /// A paragraph in a named heading keeps the spacing it states, which is Word's answer.
    /// </summary>
    /// <remarks>
    /// Arm A, and the one that decides a page: the reference draws the style's 6 pt here, and on
    /// <c>24-25_FAA_Holdover_Tables.docx</c> those 6 pt are a whole page.
    /// </remarks>
    [Fact]
    public void ADirectSpacingIsHonoured() => Arm("A-named-spacing").SpaceAfter.ShouldBe(Length.Zero);

    /// <summary>And the alignment it states, over a style that centres.</summary>
    [Fact]
    public void ADirectAlignmentIsHonoured()
        => Arm("B-named-jc").Alignment.ShouldBe(TextAlignment.Start);

    /// <summary>And the indent.</summary>
    [Fact]
    public void ADirectIndentIsHonoured()
        => Arm("C-named-ind").StartIndent.ShouldBe(Length.FromTwips(720));

    /// <summary>A page break is kept either way, so this arm cannot tell the two apart.</summary>
    /// <remarks>
    /// Kept as a regression on the arm rather than on the rule: <c>w:pageBreakBefore</c> is the one
    /// property 26.2.4.2 does <em>not</em> discard, so a reader that got the exception backwards
    /// would show up here and nowhere else.
    /// </remarks>
    [Fact]
    public void APageBreakIsKept() => Arm("D-named-break").StartsNewPage.ShouldBeTrue();

    /// <summary>A paragraph in a heading the switch does not name is unaffected either way.</summary>
    [Fact]
    public void AnUnnamedHeadingKeepsItsFormatting()
        => Arm("G-other-style").SpaceAfter.ShouldBe(Length.Zero);

    /// <summary>
    /// The reference's rule, pinned against its own resolved view of the same fixture.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Five arms are voided and four are not, and the four are what make the rule narrow enough to
    /// carry: <c>heading 4</c> while the switch names <c>heading 3</c>; a <c>w:customStyle</c>
    /// wearing the name <c>heading 5</c>; a <c>w:customStyle</c> with an outline level of its own;
    /// and the built-in <c>Title</c>. So it is the built-in heading, and not the name, the outline
    /// level, or any built-in style.
    /// </para>
    /// <para>
    /// Through <c>Resolve</c> rather than through the layout, because the switch is an environment
    /// variable and every other test in the process would see it.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheReferenceVoidsOnlyTheBuiltInHeadingsTheSwitchNames()
    {
        (XElement body, WordStyles styles) = Package();
        IReadOnlySet<string> voided = DocxTocStyles.Resolve(body, styles);

        voided.ShouldBe(["Heading3"], ignoreOrder: true);

        foreach (string label in new[]
                 { "A-named-spacing", "B-named-jc", "C-named-ind", "E-named-before" })
        {
            Pruned(body, voided, label).ShouldBe([], $"{label} keeps nothing of its own");
        }

        // The one exception, and the only thing that survives the pruning.
        Pruned(body, voided, "D-named-break").ShouldBe(["pageBreakBefore"]);

        foreach (string label in new[]
                 { "G-other-style", "I-custom-name", "J-custom-outline", "K-builtin-title" })
        {
            Pruned(body, voided, label).ShouldBe(["spacing"], $"{label} is untouched");
        }
    }

    /// <summary>The names of the properties an arm's <c>w:pPr</c> keeps, <c>w:pStyle</c> aside.</summary>
    private static List<string> Pruned(XElement body, IReadOnlySet<string> voided, string label)
    {
        XElement paragraph =
            body.Descendants().First(e => e.Name.LocalName == "p" && e.Value.StartsWith(label, StringComparison.Ordinal));

        XElement? properties = DocxTocStyles.Prune(Word.Child(paragraph, "pPr"), voided);
        return
        [
            .. properties!.Elements()
                .Select(child => child.Name.LocalName)
                .Where(name => name != "pStyle"),
        ];
    }

    private static (XElement Body, WordStyles Styles) Package()
    {
        using ZipArchive zip = ZipFile.OpenRead(Corpus.Require(Fixture));

        XElement document = Load(zip, "word/document.xml");
        WordStyles styles = new();
        styles.Add(Load(zip, "word/styles.xml"));

        return (Word.Child(document, "body")!, styles);

        static XElement Load(ZipArchive archive, string name)
        {
            using Stream stream = archive.GetEntry(name)!.Open();
            return XElement.Load(stream);
        }
    }

    private static ParagraphFormat Arm(string label)
    {
        using DocumentSource source = DocumentSource.FromFile(Corpus.Require(Fixture));
        using IDocument document = new WordProcessingReader().Read(source);

        var pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        PageParagraph paragraph =
            pages.Paragraphs.FirstOrDefault(p => p.Text == label)
            ?? throw new InvalidOperationException($"the fixture states no arm '{label}'");

        return paragraph.DeclaredFormat;
    }
}
