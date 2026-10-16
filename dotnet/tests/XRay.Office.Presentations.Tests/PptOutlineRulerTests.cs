using XRay.Office.MsBinary.Records;
using XRay.Office.Presentations.MsBinary;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// The ruler of a shape whose text it only refers to, and the one other thing that ruler decides:
/// whether the paragraph gets a numbering rule of its own or keeps its master's.
/// </summary>
/// <remarks>
/// <para>
/// A <c>.ppt</c> shape can hold its text by reference — an <c>OutlineTextRefAtom</c> naming an
/// entry of the document's slide list — while keeping its own <c>TextRulerAtom</c> in its client
/// textbox. The reference reads the two out of those two places: <c>PPTTextObj</c>'s constructor
/// locates the ruler and remembers its file offset <strong>before</strong> it patches the client
/// textbox header over to the referenced text, and builds its <c>PPTTextRulerInterpreter</c> from
/// that offset afterwards (<c>filter/source/msfilter/svdfppt.cxx</c>, the
/// <c>nTextRulerAtomOfs</c> block and its use).
/// </para>
/// <para>
/// <strong>This is where every ruler in the two decks it decides actually is.</strong> Counted
/// over their record trees: <c>ws_prod-…-M.017-(French)-France.ppt</c> holds 54
/// <c>TextRulerAtom</c> records and <c>ws_prod-g-doc-Events-Part-M-presentation.ppt</c> 3, and in
/// both files every single one sits in a client textbox that also holds an
/// <c>OutlineTextRefAtom</c>. Over the corpus's 51 <c>.ppt</c>, 121 rulers in 17 documents are in
/// that position (<c>probes/slides-bullet2-r112/rulercensus.py</c>).
/// </para>
/// <para>
/// <strong>What it is worth, measured at 26.2.4.2.</strong> The France deck's rulers state
/// <c>textOfs[0] = 336</c> master units, which is 1.482 cm; its Body master states 216, which is
/// 0.953 cm. The reference starts that deck's page-8 and page-16 bulleted text at x = 85.10 pt
/// and this tree started it at 70.09 — 43.09 plus one length or the other — and after this reads
/// 85.09 on both pages.
/// </para>
/// </remarks>
public class PptOutlineRulerTests
{
    /// <summary>One record with the eight-byte header the whole format uses.</summary>
    private static byte[] Record(ushort type, params byte[] payload)
        => [0, 0, (byte)type, (byte)(type >> 8), .. BitConverter.GetBytes((uint)payload.Length), .. payload];

    private static byte[] Word(int value) => BitConverter.GetBytes((ushort)value);

    /// <summary>A ruler stating a text offset for each of the levels named.</summary>
    private static byte[] Ruler(params int[] textOffsets)
    {
        uint flags = 0;
        for (int level = 0; level < textOffsets.Length; level++) flags |= 8u << level;

        List<byte> payload = [.. BitConverter.GetBytes(flags)];
        foreach (int offset in textOffsets) payload.AddRange(Word(offset));
        return Record(PptRecordTypes.TextRulerAtom, [.. payload]);
    }

    private static DffRecordBuffer Buffer(params byte[][] records)
        => new([.. records.SelectMany(r => r)]);

    // ------------------------------------------------------------------------ finding the ruler

    [Fact]
    public void ARulerBesideAnOutlineReferenceIsFound()
    {
        // The France deck's own shape, in the order the file writes it: the reference to the
        // slide list first, the ruler after it. Its flags word is 0x000000F8 -- all five text
        // offsets, no bullet offsets -- and its five values are these.
        DffRecordBuffer buffer = Buffer(
            Record(PptRecordTypes.OutlineTextRefAtom, BitConverter.GetBytes(0u)),
            Ruler(336, 576, 792, 1152, 1392));

        PptTextRuler ruler = PptTextReader.RulerIn(buffer, 0, buffer.Length).ShouldNotBeNull();

        ruler.TextOffset(0).ShouldBe((ushort)336);
        ruler.TextOffset(1).ShouldBe((ushort)576);
        ruler.TextOffset(4).ShouldBe((ushort)1392);
        ruler.BulletOffset(0).ShouldBeNull();
    }

    [Fact]
    public void ARulerBeforeTheReferenceIsFoundToo()
    {
        // Nothing in the format fixes the order inside a client textbox, and the reference
        // searches the whole record rather than the tail after the reference atom.
        DffRecordBuffer buffer = Buffer(
            Ruler(264),
            Record(PptRecordTypes.OutlineTextRefAtom, BitConverter.GetBytes(0u)));

        PptTextReader.RulerIn(buffer, 0, buffer.Length)
                     .ShouldNotBeNull().TextOffset(0).ShouldBe((ushort)264);
    }

    [Fact]
    public void ATextboxWithNoRulerStatesNone()
    {
        DffRecordBuffer buffer = Buffer(
            Record(PptRecordTypes.OutlineTextRefAtom, BitConverter.GetBytes(0u)));

        PptTextReader.RulerIn(buffer, 0, buffer.Length).ShouldBeNull();
    }

    [Fact]
    public void ARulerStatingNothingThisConsumesIsNotARuler()
    {
        // Flags zero: no default tab, no tab stops, no offsets. `ReadRuler` answers null rather
        // than an empty ruler, so a caller cannot mistake it for one that speaks.
        DffRecordBuffer buffer = Buffer(Record(PptRecordTypes.TextRulerAtom, BitConverter.GetBytes(0u)));

        PptTextReader.RulerIn(buffer, 0, buffer.Length).ShouldBeNull();
    }

    // ------------------------------------------- and what a ruler decides about the numbering

    /// <summary>
    /// A ruler speaking for a level makes that paragraph's numbering its own.
    /// </summary>
    /// <remarks>
    /// <c>ReadParaProps</c>' tail writes the ruler's value into the property set <em>and sets the
    /// mask bit with it</em> (<c>svdfppt.cxx</c>:5062-5068), so <c>PPT_ParaAttr_TextOfs</c> is
    /// hard for every paragraph of a shape carrying one, and <c>nHardCount</c> is therefore
    /// non-zero however little the paragraph itself states.
    /// </remarks>
    [Fact]
    public void ARulerMakesTheParagraphsNumberingItsOwn()
    {
        PptTextRuler ruler = new(null, [336, null, null, null, null], [null, null, null, null, null]);

        // 0x1800 is the France deck's own page-8 paragraph mask: alignment and line feed, and
        // none of the seven attributes the count is over.
        PptParagraphRun paragraph = new(Length: 4, Depth: 0, HasBullet: null, BulletCharacter: null, Mask: 0x1800);

        PptTextBody.NumberingIsOwn(paragraph, ruler, extended: null, depth: 0).ShouldBeTrue();
        PptTextBody.NumberingIsOwn(paragraph, ruler: null, extended: null, depth: 0).ShouldBeFalse();

        // The ruler speaks for level 0 and for no other, so a depth-1 paragraph of the same
        // shape is not made hard by it.
        PptTextBody.NumberingIsOwn(paragraph, ruler, extended: null, depth: 1).ShouldBeFalse();
    }

    /// <summary>
    /// Each of the seven attributes makes the numbering the paragraph's own on its own.
    /// </summary>
    /// <remarks>
    /// The mask bits are the file's, which <c>ReadParaProps</c> keeps as
    /// <c>nMask &amp; 0x207df7</c> and which coincide with the <c>PPT_ParaAttr_*</c> numbers for
    /// all four bullet fields. <c>BulletOn</c> is bit zero, and reading it as not one of the seven
    /// is what left round 111 unable to separate the two decks.
    /// </remarks>
    [Theory]
    [InlineData(0x0001u, true)]     // BulletOn
    [InlineData(0x0010u, true)]     // BulletFont
    [InlineData(0x0020u, true)]     // BulletColour
    [InlineData(0x0080u, true)]     // BulletChar
    [InlineData(0x0100u, true)]     // TextOfs, as pfLeftMargin
    [InlineData(0x0400u, true)]     // BulletOfs, as pfIndent
    [InlineData(0x0800u, false)]    // alignment
    [InlineData(0x1000u, false)]    // line feed
    [InlineData(0x2000u, false)]    // space before
    [InlineData(0x1800u, false)]    // both of the France deck's
    [InlineData(0x0000u, false)]
    public void OneOfTheSevenIsEnoughAndNothingElseIs(uint mask, bool expected)
    {
        PptParagraphRun paragraph = new(Length: 4, Depth: 0, HasBullet: null, BulletCharacter: null, Mask: mask);

        PptTextBody.NumberingIsOwn(paragraph, ruler: null, extended: null, depth: 0).ShouldBe(expected);
    }

    /// <summary>
    /// A stated bullet size counts only behind <c>BuHardHeight</c>, in the mask and in the flags.
    /// </summary>
    /// <remarks>
    /// <c>ReadParaProps</c> clears <c>mnAttrSet</c>'s <c>PPT_ParaAttr_BulletHeight</c> again unless
    /// both are set (<c>:4903-4912</c>), which is the only one of the three hard-flags that gates
    /// the mask rather than the value.
    /// </remarks>
    [Theory]
    [InlineData(0x0040u, (ushort)0x0000, false)]    // buSize with no BuHardHeight anywhere
    [InlineData(0x0048u, (ushort)0x0000, false)]    // the mask says hard, the flags word does not
    [InlineData(0x0040u, (ushort)0x0008, false)]    // the flags word says hard, the mask does not
    [InlineData(0x0048u, (ushort)0x0008, true)]     // both
    [InlineData(0x0008u, (ushort)0x0008, false)]    // hard-height alone, with no size stated
    public void AStatedBulletSizeIsOnlyHardBehindItsOwnFlag(uint mask, ushort flags, bool expected)
    {
        PptParagraphRun paragraph = new(
            Length: 4, Depth: 0, HasBullet: null, BulletCharacter: null, Mask: mask, BulletFlags: flags);

        PptTextBody.NumberingIsOwn(paragraph, ruler: null, extended: null, depth: 0).ShouldBe(expected);
    }

    /// <summary>
    /// A paragraph carrying an extended entry that states anything is hard by that alone.
    /// </summary>
    /// <remarks>
    /// The eighth term of the count is <c>ImplGetExtNumberFormat</c>'s own return, which is true
    /// as soon as <c>pPara->mxParaSet->mnExtParagraphMask</c> is non-zero (<c>:3409-3419</c>).
    /// </remarks>
    [Fact]
    public void AnExtendedEntryStatingAnythingIsHardOnItsOwn()
    {
        PptParagraphRun paragraph = new(Length: 4, Depth: 0, HasBullet: null, BulletCharacter: null);

        PptTextBody.NumberingIsOwn(
            paragraph, ruler: null,
            extended: new PptExtendedParagraph(0x00800000, 3, false, 0), depth: 0).ShouldBeTrue();

        PptTextBody.NumberingIsOwn(
            paragraph, ruler: null,
            extended: new PptExtendedParagraph(0, PptExtendedParagraph.NoBulletBlip, false, 0),
            depth: 0).ShouldBeFalse();
    }
}
