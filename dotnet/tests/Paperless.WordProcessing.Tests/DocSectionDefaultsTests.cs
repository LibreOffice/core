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
    /// A section that says nothing about its column gap is asking for 1.25 cm, not for none.
    /// </summary>
    /// <remarks>
    /// <c>ReadUSprm(pSep, pIds[4], 708)</c> (<c>ww8par6.cxx</c>:987), commented "default distance
    /// 1.25 cm". It is the column <em>width</em> that makes this matter: the width is the measure less
    /// the gaps, so a zero gap widens every column of a two-column section by half the gap. Measured on
    /// <c>batch-016/150_5300_13_chg8.doc</c>, whose columns came out 252 pt against LibreOffice's 234.3
    /// — and whose own flat-ODF export states <c>fo:column-gap="0.4917in"</c>, which is 708 twips.
    /// </remarks>
    [Fact]
    public void ASectionStatingNoColumnGapTakesAnInchAndAQuarterCentimetre()
    {
        // sprmSCcolumns, 0x500B, with ccolM1 = 1: two columns and no gap stated.
        byte[] twoColumns = [.. TopMarginOnly.Span, 0x0B, 0x50, 0x01, 0x00];
        WritingSection section = Ww8SectionTable.ReadProperties(twoColumns);

        section.Page.Columns.ShouldBe(2);
        section.Page.ColumnGap.ShouldBe(Length.FromTwips(708));
    }

    /// <summary>A stated column gap still wins.</summary>
    [Fact]
    public void AStatedColumnGapBeatsTheDefault()
    {
        // sprmSCcolumns then sprmSDxaColumns, 0x900C, at 360 twips.
        byte[] stated = [.. TopMarginOnly.Span, 0x0B, 0x50, 0x01, 0x00, 0x0C, 0x90, 0x68, 0x01];
        WritingSection section = Ww8SectionTable.ReadProperties(stated);

        section.Page.ColumnGap.ShouldBe(Length.FromTwips(360));
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
