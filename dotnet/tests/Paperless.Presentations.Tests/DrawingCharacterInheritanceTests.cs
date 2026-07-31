using System.Xml.Linq;
using Paperless.Core.Extraction;
using Paperless.Ooxml.DrawingML;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Tests what a run inherits from the <c>a:defRPr</c> chain rather than stating itself.
/// </summary>
/// <remarks>
/// <para>
/// Only the half of the character chain extraction can observe is resolved: bold, italic,
/// underline, strikethrough, the raised or lowered baseline, and the language tag. Size, colour
/// and typeface are inherited by exactly the same walk, and are deliberately not read, because
/// nothing in the content tree reports them — they change where the text sits and what it looks
/// like, not what it says.
/// </para>
/// <para>
/// Markup literals rather than corpus decks, because the corpus cannot separate the cases. Both
/// LibreOffice-written feature decks state every run's emphasis on the run, so they exercise
/// none of this; the decks that do are PowerPoint-authored, and there the inheritance is
/// entangled with the placeholder chain that finds the <c>defRPr</c> in the first place.
/// </para>
/// </remarks>
public class DrawingCharacterInheritanceTests
{
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static XElement Parse(string inner)
        => XElement.Parse($"<a:txBody xmlns:a=\"{A}\">{inner}</a:txBody>");

    private static XElement Level(string inner)
        => XElement.Parse($"<a:lvl1pPr xmlns:a=\"{A}\">{inner}</a:lvl1pPr>");

    private static List<ContentRun> Read(XElement body, DrawingTextOptions? options = null)
    {
        ContentSection target = new() { Kind = SectionKind.Frame };
        DrawingTextBody.Read(body, target, options);
        return [.. target.Children.OfType<ContentParagraph>().SelectMany(p => p.Children).OfType<ContentRun>()];
    }

    [Fact]
    public void ARunWithNoPropertiesTakesItsEmphasisFromTheInheritedLevel()
    {
        XElement body = Parse("<a:p><a:r><a:t>Inherited</a:t></a:r></a:p>");

        // A PowerPoint-authored deck states a placeholder's boldness once, on the master, and
        // never on the slide. Reading only the run's own a:rPr reports every such title
        // unemphasised — the same symptom the binary format has with TxMasterStyleAtom.
        List<ContentRun> runs = Read(body, new DrawingTextOptions
        {
            InheritedLevelProperties = _ => [Level("<a:defRPr b=\"1\"/>")],
        });

        runs.Single().Emphasis.ShouldBe(RunEmphasis.Bold);
    }

    [Fact]
    public void ARunKeepsWhatItDoesNotStateAndOverridesWhatItDoes()
    {
        XElement body = Parse(
            "<a:p><a:r><a:rPr b=\"0\"/><a:t>Only bold is cancelled</a:t></a:r></a:p>");

        // Attribute by attribute, not element by element: a run that states b="0" and nothing
        // else has cancelled the boldness and kept the italic. Merging whole property sets
        // instead — the run's rPr wholesale replacing the defRPr — loses the italic.
        List<ContentRun> runs = Read(body, new DrawingTextOptions
        {
            InheritedLevelProperties = _ => [Level("<a:defRPr b=\"1\" i=\"1\"/>")],
        });

        runs.Single().Emphasis.ShouldBe(RunEmphasis.Italic);
    }

    [Fact]
    public void TheNearestSourceInTheChainWinsPropertyByProperty()
    {
        XElement body = Parse("<a:p><a:r><a:t>Mixed</a:t></a:r></a:p>");

        // Two sources, layout before master. Bold comes from the nearer one — which turns it
        // off — while the strikethrough the nearer one says nothing about survives from the
        // further one.
        List<ContentRun> runs = Read(body, new DrawingTextOptions
        {
            InheritedLevelProperties = _ =>
            [
                Level("<a:defRPr b=\"0\"/>"),
                Level("<a:defRPr b=\"1\" strike=\"sngStrike\"/>"),
            ],
        });

        runs.Single().Emphasis.ShouldBe(RunEmphasis.Strikethrough);
    }

    [Fact]
    public void TheParagraphsOwnDefaultRunPropertiesBeatEveryInheritedOne()
    {
        XElement body = Parse(
            "<a:p><a:pPr><a:defRPr i=\"1\"/></a:pPr><a:r><a:t>Paragraph default</a:t></a:r></a:p>");

        // a:pPr/a:defRPr is the most specific source short of the run itself. LibreOffice
        // applies it last of the four (oox/source/drawingml/textparagraph.cxx:66), after the
        // master's list style, the shape's text style and the body's list style.
        List<ContentRun> runs = Read(body, new DrawingTextOptions
        {
            InheritedLevelProperties = _ => [Level("<a:defRPr i=\"0\" b=\"1\"/>")],
        });

        runs.Single().Emphasis.ShouldBe(RunEmphasis.Bold | RunEmphasis.Italic);
    }

    [Fact]
    public void TheBodysOwnListStyleBeatsAnythingInherited()
    {
        XElement body = Parse(
            $"<a:lstStyle xmlns:a=\"{A}\"><a:lvl1pPr><a:defRPr u=\"none\"/></a:lvl1pPr></a:lstStyle>"
            + "<a:p><a:r><a:t>Body style</a:t></a:r></a:p>");

        // The shape's own a:lstStyle sits below its paragraphs and above everything the caller
        // inherits, so an underline the master asks for is cancelled here.
        List<ContentRun> runs = Read(body, new DrawingTextOptions
        {
            InheritedLevelProperties = _ => [Level("<a:defRPr u=\"sng\"/>")],
        });

        runs.Single().Emphasis.ShouldBe(RunEmphasis.None);
    }

    [Fact]
    public void ARaisedBaselineIsInheritedAsSuperscript()
    {
        XElement body = Parse("<a:p><a:r><a:t>th</a:t></a:r></a:p>");

        List<ContentRun> runs = Read(body, new DrawingTextOptions
        {
            InheritedLevelProperties = _ => [Level("<a:defRPr baseline=\"30000\"/>")],
        });

        runs.Single().Emphasis.ShouldBe(RunEmphasis.Superscript);
    }

    [Fact]
    public void ALanguageTagIsInheritedWhenTheRunStatesNone()
    {
        XElement body = Parse(
            "<a:p><a:r><a:t>Untagged</a:t></a:r>"
            + "<a:r><a:rPr lang=\"fr-FR\"/><a:t>Tagged</a:t></a:r></a:p>");

        List<ContentRun> runs = Read(body, new DrawingTextOptions
        {
            InheritedLevelProperties = _ => [Level("<a:defRPr lang=\"en-GB\"/>")],
        });

        runs[0].Language.ShouldBe("en-GB");
        runs[1].Language.ShouldBe("fr-FR");
    }

    [Fact]
    public void ADefaultRunPropertyForAnotherLevelIsNotApplied()
    {
        XElement body = Parse(
            "<a:p><a:pPr lvl=\"1\"/><a:r><a:t>Level two</a:t></a:r></a:p>");

        // The chain is asked for the paragraph's own level and answers for that level alone; a
        // reader that merged whole list styles would give a nested paragraph its parent's
        // emphasis as well as its parent's indent.
        List<ContentRun> runs = Read(body, new DrawingTextOptions
        {
            InheritedLevelProperties = level =>
                level == 0 ? [Level("<a:defRPr b=\"1\"/>")] : [],
        });

        runs.Single().Emphasis.ShouldBe(RunEmphasis.None);
    }
}
