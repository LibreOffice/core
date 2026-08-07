using System.IO.Compression;
using System.Xml.Linq;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Ooxml;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A paragraph style that states one of <c>w:spacing/@w:before</c> and <c>@w:after</c> and
/// inherits the other.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice keeps both margins in one item, <c>SvxULSpaceItem</c>, and writerfilter sets them
/// through two separate UNO properties — so stating one is a read-modify-write of the pair, and
/// the half the file does not state is frozen as a <em>direct</em> value at whatever the parent
/// chain held when the style was applied. Styles are applied in declaration order, so a parent
/// declared further down <c>styles.xml</c> has not yet had its own definition applied and is
/// still sitting at Writer's pool default for the built-in style its <c>w:name</c> names.
/// </para>
/// <para>
/// Every figure here was measured on LibreOffice 24.2.7.2 from
/// <c>tests/corpus/features/style-one-sided-spacing.docx</c>, whose four styles differ in exactly
/// one thing each. The rendered baselines are 72.00, 97.50, 122.90, 160.40, 185.80, 199.30,
/// 224.70, 241.20 and 266.60 pt down an American-letter page with a one-inch margin and a
/// 13.45 pt line, which is where the four expectations below come from.
/// </para>
/// <para>
/// The corpus document this was found on is
/// <c>words/batch-007/docx/final-technical-report-template.docx</c>: its <c>Heading1</c> states
/// only <c>w:after="240"</c> and is based on its own <c>Heading2</c>, so every heading is 12 pt
/// lower than we drew it and the document is six pages rather than five.
/// </para>
/// </remarks>
public sealed class OneSidedStyleSpacingTests
{
    /// <summary>A parent declared later, naming a built-in heading, gives Writer's 12 pt.</summary>
    [Fact]
    public void ALaterHeadingParentGivesTwelvePointsAbove()
    {
        ParagraphFormat format = Resolve("KidLateHeading");

        format.SpaceBefore.ShouldBe(Length.FromPoints(12));
        format.SpaceAfter.ShouldBe(Length.FromPoints(12));
    }

    /// <summary>
    /// A parent declared earlier is an ordinary inheritance and keeps its own value.
    /// </summary>
    /// <remarks>
    /// The control that stops the rule being read as "a heading parent always means 12 pt".
    /// Both parents here state <c>w:before="480"</c>; only the one declared first is heard.
    /// </remarks>
    [Fact]
    public void AnEarlierHeadingParentIsInheritedNormally()
    {
        Resolve("KidEarlyHeading").SpaceBefore.ShouldBe(Length.FromPoints(24));
    }

    /// <summary>
    /// A parent declared later that Writer has no built-in style for gives nought — and that
    /// suppresses the parent's own value rather than falling through to it.
    /// </summary>
    [Fact]
    public void ALaterCustomParentGivesNothingAbove()
    {
        Resolve("KidLateCustom").SpaceBefore.ShouldBe(Length.Zero);
    }

    /// <summary>A style stating both halves is left alone.</summary>
    [Fact]
    public void AStyleStatingBothIsUntouched()
    {
        ParagraphFormat format = Resolve("KidBoth");

        format.SpaceBefore.ShouldBe(Length.FromPoints(3));
        format.SpaceAfter.ShouldBe(Length.FromPoints(12));
    }

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
            Corpus.Require("style-one-sided-spacing.docx"));
        using Stream part = archive.GetEntry("word/styles.xml")!.Open();

        WordStyles styles = new();
        styles.Add(XDocument.Load(part).Root!);
        return styles;
    }
}
