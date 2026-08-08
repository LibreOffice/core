using Paperless.Core.Units;
using Paperless.WordProcessing.Model;
using Paperless.WordProcessing.Ww8;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A DOC's section descriptor states only what differs from Word's own defaults.
/// </summary>
/// <remarks>
/// <para>
/// WW8's <c>SEP</c> is a structure with initial values, not a list of statements, and Word writes a sprm
/// only where a section departs from them. So a reader has to know the defaults: silence means "the usual
/// thing" and not "nothing".
/// </para>
/// <para>
/// The header and footer distances are the pair that bite hardest, because they place the running head
/// and foot <em>and</em> size the bands they sit in. LibreOffice states 720 twips twice over — in
/// <c>WW8_SEP</c>'s constructor (<c>ww8scan.cxx</c>, <c>dyaHdrTop(720), dyaHdrBottom(720)</c>) and again
/// as the fallback passed to <c>ReadUSprm</c> (<c>ww8par6.cxx</c>:1183).
/// </para>
/// </remarks>
public sealed class DocSectionDefaultsTests
{
    /// <summary>
    /// A section that says nothing about its header distance is asking for half an inch.
    /// </summary>
    /// <remarks>
    /// Reading the silence as zero pins the header to the paper edge — measured on
    /// <c>batch-019/doc/150_5335_5a.doc</c>, whose running head sat at y 780.80 against LibreOffice's
    /// 744.80 and whose page number sat at 2.60 against 38.60, exactly 36 pt each way on every page.
    /// </remarks>
    [Fact]
    public void ASectionStatingNoHeaderDistanceTakesHalfAnInch()
    {
        WritingSection section = Ww8SectionTable.ReadProperties(TopMarginOnly);

        section.Page.HeaderDistance.ShouldBe(Length.FromTwips(720));
        section.Page.FooterDistance.ShouldBe(Length.FromTwips(720));
    }

    /// <summary>
    /// The band a header sits in is the gap between its distance and the body's top margin.
    /// </summary>
    /// <remarks>
    /// Which is why the default reaches the body as well as the furniture: with the distance read as
    /// zero the band was the whole top margin, 36 pt too tall, so a running head of more than one line
    /// fitted where Writer would have pushed the text down.
    /// </remarks>
    [Fact]
    public void TheHeaderBandIsWhatTheTopMarginLeavesAboveTheBody()
    {
        WritingSection section = Ww8SectionTable.ReadProperties(TopMarginOnly);

        section.Page.Margins.Top.ShouldBe(Length.FromTwips(1440));
        section.Page.HeaderHeight.ShouldBe(Length.FromTwips(1440 - 720));
    }

    /// <summary>A stated distance still wins, so the default is a fallback and not a floor.</summary>
    [Fact]
    public void AStatedDistanceBeatsTheDefault()
    {
        byte[] stated = [.. TopMarginOnly.Span, 0x17, 0xB0, 0x40, 0x01];
        WritingSection section = Ww8SectionTable.ReadProperties(stated);

        section.Page.HeaderDistance.ShouldBe(Length.FromTwips(320));
        section.Page.FooterDistance.ShouldBe(Length.FromTwips(720));
    }

    /// <summary>
    /// <c>sprmSDyaTop</c> at Word's own inch, and nothing else.
    /// </summary>
    /// <remarks>
    /// 0x9023 with a two-byte operand of 1440. Stated rather than left to the geometry's own default so
    /// that the header band's arithmetic is asserted against a number this test can see.
    /// </remarks>
    private static ReadOnlyMemory<byte> TopMarginOnly => new byte[] { 0x23, 0x90, 0xA0, 0x05 };
}
