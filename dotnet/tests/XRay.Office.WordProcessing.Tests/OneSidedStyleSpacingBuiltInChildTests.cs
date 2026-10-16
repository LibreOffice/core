using System.IO.Compression;
using System.Xml.Linq;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A half-stated <c>w:spacing</c> on a style whose own <c>w:name</c> is one of Writer's, where
/// the unstated margin comes from Writer's own hierarchy under that style because the parent the
/// file gave it is one Writer has never heard of.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="OneSidedStyleSpacingTests"/> covers the other half of the same rule, and every one
/// of its four styles is custom-named — so an unrecognised parent there really does mean nought,
/// and nothing in it can test what happens when the style's own name is one Writer knows. The
/// corpus case that rule was fitted to cannot test it either: a <c>heading 1</c> based on a
/// <c>heading 2</c> reads 12 pt whichever end answers.
/// </para>
/// <para>
/// Every expectation here is a measurement of LibreOffice 26.2.4.2 rather than a prediction,
/// taken from <c>tests/corpus/features/style-one-sided-spacing-builtin-child.docx</c> — which
/// <c>dotnet/probes/words-pagination-01/make-builtin-child-corpus.py</c> both authors and reads
/// back through <c>soffice --convert-to fodt</c>, so the file and the numbers below come from
/// one script and cannot drift apart. All five styles state only <c>w:before="480"</c> and all
/// five parents are declared after every child, so the 24 pt above is a control: it is the value
/// the file states, and an implementation that mirrored it would put 24 pt below too.
/// </para>
/// <para>
/// The document behind this is the pair of FAA Holdover Tables, whose <c>Heading4</c> is named
/// <c>heading 4</c>, is based on a custom <c>Notes/Cautions Heading</c> declared 182 styles
/// later, and states only <c>w:before="120"</c>. Reading the parent's row gave nought below;
/// the reference keeps 6 pt, and those 6 pt per NOTES heading are their shared 13-page deficit.
/// </para>
/// </remarks>
public sealed class OneSidedStyleSpacingBuiltInChildTests
{
    /// <summary>
    /// A <c>heading 4</c> over a parent Writer does not recognise keeps Writer's own 6 pt below,
    /// where the parent alone would have given nought.
    /// </summary>
    [Fact]
    public void ABuiltInHeadingChildKeepsItsOwnSixPointsBelow()
    {
        ParagraphFormat format = Resolve("H4Custom");

        format.SpaceBefore.ShouldBe(Length.FromPoints(24));
        format.SpaceAfter.ShouldBe(Length.FromPoints(6));
    }

    /// <summary>
    /// Every heading level answers with the same 6 pt, because what survives is their shared
    /// <c>Heading</c> base and not the per-level rows Writer's pool declares.
    /// </summary>
    /// <remarks>
    /// <c>DocumentStylePoolManager.cxx</c>:850 gives Heading 2 ten points above and :857 gives
    /// Heading 3 seven, and neither is reachable here. This style's parent is a <c>heading 2</c>,
    /// which Writer does recognise, so it is the parent that answers — and it answers with the
    /// same number, which is exactly why the older probe could not tell the two ends apart.
    /// </remarks>
    [Fact]
    public void EveryHeadingLevelAnswersWithTheSameSixPoints()
        => Resolve("H5Heading").SpaceAfter.ShouldBe(Length.FromPoints(6));

    /// <summary>
    /// A style named <c>Body Text</c> over an unrecognised parent answers with nothing, though a
    /// <c>Body Text</c> <em>parent</em> answers with 7 pt.
    /// </summary>
    /// <remarks>
    /// The control that keeps the rule from being read as "a built-in name answers wherever it
    /// sits". Only the heading family answers from this end; <c>Caption</c>, <c>List</c> and
    /// <c>Quote</c> measured nought here too.
    /// </remarks>
    [Fact]
    public void ABuiltInBodyTextChildAnswersWithNothing()
        => Resolve("BodyKid").SpaceAfter.ShouldBe(Length.Zero);

    /// <summary>
    /// A custom style over a <c>heading 2</c> parent still reads the parent's 6 pt, which is the
    /// behaviour this change must leave alone — the change is additive and fires only where the
    /// parent gave nought.
    /// </summary>
    [Fact]
    public void ACustomChildStillReadsAHeadingParent()
        => Resolve("CustomHeading").SpaceAfter.ShouldBe(Length.FromPoints(6));

    /// <summary>Neither end being one of Writer's gives nought, as before.</summary>
    [Fact]
    public void NeitherEndBuiltInGivesNothing()
        => Resolve("CustomCustom").SpaceAfter.ShouldBe(Length.Zero);

    private static ParagraphFormat Resolve(string styleId)
    {
        WordStyles styles = LoadStyles();
        XElement properties = new(
            XName.Get("pPr", "http://schemas.openxmlformats.org/wordprocessingml/2006/main"),
            new XElement(
                XName.Get("pStyle", "http://schemas.openxmlformats.org/wordprocessingml/2006/main"),
                new XAttribute(
                    XName.Get("val", "http://schemas.openxmlformats.org/wordprocessingml/2006/main"),
                    styleId)));

        return WordParagraphFormats.Resolve(styles, properties, Length.FromTwips(720));
    }

    private static WordStyles LoadStyles()
    {
        using ZipArchive archive = ZipFile.OpenRead(
            Corpus.Require("style-one-sided-spacing-builtin-child.docx"));
        using Stream part = archive.GetEntry("word/styles.xml")!.Open();

        WordStyles styles = new();
        styles.Add(XDocument.Load(part).Root!);
        return styles;
    }
}
