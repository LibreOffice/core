using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.Presentations.Ooxml;
using Paperless.Text.Fonts;
using Shouldly;
using System.Xml.Linq;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Checks that a bullet stated in a symbol face is <em>drawn</em> as that face's glyph rather
/// than as U+2022.
/// </summary>
/// <remarks>
/// <para>
/// The reader's half of this is covered by <c>PptSymbolBulletTests</c>: the slot goes into the
/// Private Use Area. What is asserted here is the half that was missing — that the slot is then
/// recoded into the OpenSymbol code point holding the same picture, and drawn from OpenSymbol.
/// </para>
/// <para>
/// Measured against the reference on <c>Framing Europe.ppt</c> page 12, which states Wingdings
/// slot 0x6E: LibreOffice draws a glyph 13.72 pt wide at x = 61.09, and before this rule we drew
/// U+2022 at 6.30 pt wide at the same pen. Afterwards ours is 13.73 pt at 61.09.
/// </para>
/// </remarks>
public class SlideSymbolBulletGlyphTests
{
    private static readonly XNamespace A =
        "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static DocRect Area =>
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(200));

    /// <summary>
    /// A one-paragraph body whose bullet is <paramref name="character"/> in
    /// <paramref name="face"/>.
    /// </summary>
    private static SlideTextBody Body(string face, string character) =>
        PptxTextBody.Read(XElement.Parse(
            $"""
             <a:txBody xmlns:a="{A}">
               <a:bodyPr/>
               <a:p>
                 <a:pPr marL="342900" indent="-342900">
                   <a:buFont typeface="{face}" charset="2"/>
                   <a:buChar char="{character}"/>
                 </a:pPr>
                 <a:r><a:rPr lang="en-GB" sz="2400"/><a:t>Bulleted</a:t></a:r>
               </a:p>
             </a:txBody>
             """));

    /// <summary>The marker is the run drawn left of the text, so the leftmost of the two.</summary>
    private static GlyphRun Marker(SlideTextBody body)
    {
        List<PlacedGlyphRun> placed = SlideTextLayout.Place(body, Area, new SlideFonts());
        placed.Count.ShouldBe(2);
        return placed[0].Run.Origin.X <= placed[1].Run.Origin.X ? placed[0].Run : placed[1].Run;
    }

    [Fact]
    public void AWingdingsBulletIsDrawnAsItsOpenSymbolGlyph()
    {
        GlyphRun marker = Marker(Body("Wingdings", "\uF0D8"));

        marker.Text.ShouldBe("\uE49E");
        marker.Font.FamilyName.ShouldBe("OpenSymbol");
        marker.Glyphs.Count.ShouldBe(1);
    }

    /// <summary>
    /// The slot may be stated unaliased, which is how PowerPoint actually writes it — the
    /// character is whatever byte 0xD8 means in the file's encoding.
    /// </summary>
    [Fact]
    public void TheUnaliasedSlotReachesTheSameGlyph()
    {
        Marker(Body("Wingdings", "\u00D8")).Text.ShouldBe("\uE49E");
    }

    /// <summary>
    /// A symbol bullet is wider than the U+2022 that used to stand in for it, and the marker's
    /// advance is what the first line has to clear — so this is a layout change, not only a
    /// glyph change.
    /// </summary>
    [Fact]
    public void TheRecodedGlyphIsWiderThanTheBulletItReplaces()
    {
        GlyphRun symbol = Marker(Body("Wingdings", "\uF06E"));
        GlyphRun plain = Marker(Body("Calibri", "\u2022"));

        symbol.Text.ShouldBe("\uE439");
        symbol.Glyphs[0].Advance.Emu.ShouldBeGreaterThan(plain.Glyphs[0].Advance.Emu);
    }

    /// <summary>
    /// A face with no recode table keeps the old answer. This is the assertion that fails if the
    /// recode is applied to everything rather than to the fourteen faces LibreOffice lists.
    /// </summary>
    [Fact]
    public void AFaceWithNoTableStillFallsBackToU2022()
    {
        // charset="2" makes the reader treat it as a symbol position, so the slot reaches the
        // layout in the Private Use Area exactly as Wingdings' would; only the table is missing.
        GlyphRun marker = Marker(Body("Some Private Dingbat", "\uF0D8"));

        marker.Text.ShouldBe("\u2022");
    }

    /// <summary>
    /// The digits of a generated number are not symbol positions, whatever face the level names
    /// for its bullet — recoding them would make nonsense of the numbering.
    /// </summary>
    [Fact]
    public void AnAutoNumberIsNotRecoded()
    {
        SlideTextBody body = PptxTextBody.Read(XElement.Parse(
            $"""
             <a:txBody xmlns:a="{A}">
               <a:bodyPr/>
               <a:p>
                 <a:pPr marL="342900" indent="-342900">
                   <a:buFont typeface="Wingdings" charset="2"/>
                   <a:buAutoNum type="arabicPeriod"/>
                 </a:pPr>
                 <a:r><a:rPr lang="en-GB" sz="2400"/><a:t>Numbered</a:t></a:r>
               </a:p>
             </a:txBody>
             """));

        Marker(body).Text.ShouldBe("1.");
    }
}
