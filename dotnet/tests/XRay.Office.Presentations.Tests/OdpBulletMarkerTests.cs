using XRay.Office.Core;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using Shouldly;
using Xunit;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// What an ODF bullet level's character and colour reach the page as.
/// </summary>
/// <remarks>
/// <para>
/// A bullet level almost always names a symbol face and states its character as that face's own
/// slot in the Private Use Area, which means nothing anywhere else. LibreOffice substitutes
/// OpenSymbol for a symbol face it has not got and recodes the slot to the StarSymbol code point
/// holding the same picture (<c>unotools/source/misc/fontcvt.cxx</c>:185 and :1325); this tree
/// does the same in <c>SlideTextLayout</c>, and the deck reader has kept the slot for it since
/// symbol bullets were implemented.
/// </para>
/// <para>
/// The ODF path did not, because it took its label from <c>OdfListStyle.FormatLabel</c>, which is
/// the <em>extraction</em> answer and collapses a Private Use Area character to U+2022 — right for
/// an index, and a black dot where the reference draws a green check mark.
/// <strong>3598 bullet levels in 74 of the converted corpus's 302 <c>.odp</c> state a Private Use
/// Area character</strong>, every one of them in the F000 block, and every family they name has a
/// recode table but one.
/// </para>
/// <para>
/// The colour is a second reading and a wider one: <c>fo:color</c> on the level's own
/// <c>style:text-properties</c>, with <c>style:use-window-font-color="true"</c> meaning the item's
/// own colour instead. <strong>22 436 bullet levels in all 302 state one.</strong>
/// </para>
/// <para>
/// The fixture's three slides and the 26.2.4.2 figures they are checked against are in its own
/// header.
/// </para>
/// </remarks>
public class OdpBulletMarkerTests
{
    /// <summary>The recode of Wingdings slot 0xFC — a check mark — in OpenSymbol.</summary>
    private const string CheckMark = "\uE4C2";

    private static SlidePages Layout()
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("odp-list-bullet-symbol.fodp")));

        document.ShouldBeAssignableTo<IPaginatedDocument>();
        return (SlidePages)((IPaginatedDocument)document).Layout();
    }

    /// <summary>The marker run of a slide: the leftmost run, which is the label.</summary>
    private static PlacedGlyphRun Marker(LaidOutSlide slide)
    {
        List<PlacedGlyphRun> runs =
        [
            .. slide.Shapes.Where(shape => shape.Text is not null)
                .SelectMany(shape => shape.Text!.Runs)
                .OrderBy(run => run.Run.Origin.X.Emu),
        ];

        runs.Count.ShouldBeGreaterThan(1);
        return runs[0];
    }

    /// <remarks>
    /// The picture, not the code point: 26.2.4.2 states the slot in its own <c>ToUnicode</c>
    /// because it recodes when it draws, and this tree recodes when it lays out. A 600 dpi crop of
    /// the two renderings of the corpus document this was found on is byte-identical.
    /// </remarks>
    [Fact]
    public void APrivateUseSlotIsRecodedRatherThanCollapsedToABullet()
    {
        Marker(Layout().Slides[0]).Run.Text.ShouldBe(CheckMark);
        Marker(Layout().Slides[1]).Run.Text.ShouldBe(CheckMark);
    }

    /// <remarks>
    /// The level's <c>fo:color</c>, which nothing read before: the item's text is black and its
    /// bullet is not.
    /// </remarks>
    [Fact]
    public void TheLevelsOwnColourIsWhatTheMarkerIsDrawnIn()
    {
        Marker(Layout().Slides[0]).Colour.ShouldBe(Colour.FromRgb(0x9BBB59));

        // The item's own text, on the same slide, is not that colour.
        Layout().Slides[0].Shapes.Where(shape => shape.Text is not null)
            .SelectMany(shape => shape.Text!.Runs)
            .OrderBy(run => run.Run.Origin.X.Emu)
            .Skip(1)
            .First()
            .Colour.ShouldBe(Colour.Black);
    }

    /// <remarks>
    /// <c>style:use-window-font-color="true"</c> is ODF's <em>automatic</em> colour and is written
    /// on every level LibreOffice generates that does not colour its bullet, so it has to be read
    /// before <c>fo:color</c> rather than instead of it.
    /// </remarks>
    [Fact]
    public void UseWindowFontColourLeavesTheMarkerTheItemsOwnColour()
        => Marker(Layout().Slides[1]).Colour.ShouldBe(Colour.Black);

    /// <remarks>
    /// The control. A character that is not a slot is not recoded, and the level's
    /// <c>fo:font-size</c> — 45% here against 90% above — still reaches the marker.
    /// </remarks>
    [Fact]
    public void ACharacterThatIsNotASlotIsDrawnAsItStands()
    {
        Marker(Layout().Slides[2]).Run.Text.ShouldBe("\u25CF");
        Marker(Layout().Slides[2]).Run.FontSize.Points.ShouldBe(7.2, 0.05);
        Marker(Layout().Slides[0]).Run.FontSize.Points.ShouldBe(14.4, 0.05);
    }
}
