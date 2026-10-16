using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using Shouldly;

namespace XRay.Office.OpenDocument.Tests;

/// <summary>
/// What a bullet level's own <c>style:text-properties</c> say about its label.
/// </summary>
/// <remarks>
/// <para>
/// A list level carries the label's face, its relative size and its colour, and the colour was the
/// one of the three nothing read — so every bulleted ODF slide drew its markers in its text's
/// colour. It is DrawingML's <c>a:buClr</c>, which the deck reader has read since it was written.
/// <strong>22 436 bullet levels in all 302 of the converted corpus's <c>.odp</c> state
/// <c>fo:color</c>.</strong>
/// </para>
/// <para>
/// <c>style:use-window-font-color="true"</c> is ODF's <em>automatic</em> colour and LibreOffice
/// writes it on every level it generates that does not colour its bullet, so it has to be tested
/// before <c>fo:color</c> rather than instead of it.
/// </para>
/// <para>
/// The second half here is that <see cref="OdfListStyle.FormatLabel"/> is the extraction answer
/// and is not the rendering one: it collapses a Private Use Area character to U+2022, which is
/// right for an index and wrong for a page — the slide layer reads
/// <see cref="OdfListLevel.BulletCharacter"/> instead when the level names a face the symbol
/// tables cover.
/// </para>
/// </remarks>
public class OdfListLevelColourTests
{
    private static OdfListStyle Style(string levels)
    {
        XElement root = XElement.Parse($$"""
            <office:document-styles
                xmlns:office="{{OdfNamespaces.Office}}"
                xmlns:style="{{OdfNamespaces.Style}}"
                xmlns:fo="{{OdfNamespaces.FoCompatible}}"
                xmlns:text="{{OdfNamespaces.Text}}">
              <office:styles>
                <text:list-style style:name="Bullets">{{levels}}</text:list-style>
              </office:styles>
            </office:document-styles>
            """);

        OdfStyles styles = new();
        styles.AddDocument(root, null);
        return styles.FindListStyle("Bullets").ShouldNotBeNull();
    }

    private const string Slot = "\uF0FC";

    [Fact]
    public void AStatedColourIsRead()
    {
        OdfListLevel level = Style(
            $"""
             <text:list-level-style-bullet text:level="1" text:bullet-char="{Slot}">
               <style:text-properties style:font-name="Wingdings" fo:color="#9bbb59"
                                      fo:font-size="90%"/>
             </text:list-level-style-bullet>
             """).GetLevel(1).ShouldNotBeNull();

        level.Colour.ShouldBe(Colour.FromRgb(0x9BBB59));
        level.RelativeSize!.Value.ShouldBe(0.9, 0.0001);
        level.Typeface.ShouldBe("Wingdings");
    }

    /// <remarks>
    /// The automatic colour wins over a stated one, because a level may carry both and the
    /// automatic flag is what LibreOffice wrote last.
    /// </remarks>
    [Fact]
    public void TheWindowFontColourMeansTheItemsOwn()
    {
        OdfListLevel level = Style(
            $"""
             <text:list-level-style-bullet text:level="1" text:bullet-char="{Slot}">
               <style:text-properties fo:color="#9bbb59"
                                      style:use-window-font-color="true"/>
             </text:list-level-style-bullet>
             """).GetLevel(1).ShouldNotBeNull();

        level.Colour.ShouldBeNull();
    }

    [Fact]
    public void ALevelStatingNoColourAnswersNone()
        => Style(
            $"""
             <text:list-level-style-bullet text:level="1" text:bullet-char="{Slot}"/>
             """).GetLevel(1).ShouldNotBeNull().Colour.ShouldBeNull();

    /// <remarks>
    /// The two answers a level gives about its character, and the reason they differ: an index
    /// cannot use a Wingdings slot and a renderer must not lose it.
    /// </remarks>
    [Fact]
    public void TheFormattedLabelCollapsesAPrivateUseSlotAndTheRawCharacterKeepsIt()
    {
        OdfListStyle style = Style(
            $"""
             <text:list-level-style-bullet text:level="1" text:bullet-char="{Slot}">
               <style:text-properties style:font-name="Wingdings"/>
             </text:list-level-style-bullet>
             """);

        style.FormatLabel(1, [1]).ShouldBe("•");
        style.GetLevel(1).ShouldNotBeNull().BulletCharacter.ShouldBe(Slot);
    }
}
