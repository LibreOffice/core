using System.Buffers.Binary;
using XRay.Office.WordProcessing.Ww8;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A WW8 toggle sprm's "as the style" and "against the style" operands are relative to <em>one
/// style's</em> own resolved value, never to the value the run is being layered onto.
/// </summary>
/// <remarks>
/// <para>
/// <c>SwWW8ImplReader::Read_BoldUsw</c> (<c>sw/source/filter/ww8/ww8par6.cxx</c>:3117-3156) starts at
/// <c>pSI = GetStyle(m_nCurrentColl)</c> — the paragraph style — replaces it outright when the CHPX
/// being read carries a <c>sprmCIstd</c>, and only then asks
/// <c>if (*pData &amp; 0x80) { if (pSI->m_n81Flags &amp; nMask) bOn = !bOn; }</c>.
/// <c>m_n81Flags</c> is seeded from the base style (<c>ww8par2.cxx</c>:3825) and then set or cleared
/// by the style's own CHPX, so it is that chain walked from nothing — and a style that states no
/// weight answers <em>not bold</em> rather than "whatever the paragraph is".
/// </para>
/// <para>
/// The witness is <c>150_5335_5a.doc</c> page 3, where 26.2.4.2 draws 523 characters in
/// <c>LiberationSerif-Bold</c> and this tree drew none: every contents entry's CHPX is
/// <c>sprmCIstd</c> 26 (<c>Hyperlink</c>, which states no weight) plus <c>sprmCFBold</c> 0x81, over a
/// <c>TOC 3</c> paragraph style that is itself bold. The document's own internal control is the two
/// runs beside it — <c>304A1A00 350881</c> on <c>Definition of ACN</c> against <c>350881 504A0000</c>
/// on the tab that follows it, the same operand in the same paragraph with and without the style —
/// which 26.2.4.2's <c>--convert-to fodt</c> writes out as <c>fo:font-weight="bold"</c> and
/// <c>fo:font-weight="normal"</c> respectively.
/// </para>
/// <para>
/// Unit rather than document tests: the corpus holds the only <c>.doc</c> that exercise the rule and
/// the repository holds no fixture with a styled toggle. <c>probes/ww8char-r135/results.md</c> has
/// the corpus measurement.
/// </para>
/// </remarks>
public sealed class Ww8ToggleBaseTests
{
    /// <summary><c>sprmCFBold</c>.</summary>
    private const ushort Bold = 0x0835;

    /// <summary><c>sprmCFItalic</c>.</summary>
    private const ushort Italic = 0x0836;

    /// <summary>"The opposite of the style's value".</summary>
    private const byte AgainstTheStyle = 0x81;

    /// <summary>"The style's value".</summary>
    private const byte AsTheStyle = 0x80;

    /// <summary>
    /// 0x81 over a style that states the toggle is off, and over one that does not is on.
    /// </summary>
    /// <remarks>
    /// The whole of the defect in one assertion. Both calls apply the same grpprl; they differ only in
    /// what the operand is measured against, and the answers are opposite.
    /// </remarks>
    [Fact]
    public void AToggleAgainstTheStyleIsMeasuredAgainstTheStyleItIsGiven()
    {
        Apply(Toggle(Bold, AgainstTheStyle), toggleBase: StatesNoWeight).IsBold.ShouldBe(true);
        Apply(Toggle(Bold, AgainstTheStyle), toggleBase: IsBold).IsBold.ShouldBe(false);
    }

    /// <summary>
    /// 0x80 is the same question answered the other way round.
    /// </summary>
    /// <remarks>
    /// Worth its own arm because 0x80 is the operand a reader is most likely to treat as a no-op: it
    /// means <em>take the style's value</em>, so over a bold style it turns a run bold that the
    /// accumulated value might have had regular, and over a plain one it turns a run regular that the
    /// paragraph style had made bold.
    /// </remarks>
    [Fact]
    public void AToggleAsTheStyleTakesTheStylesValueAndNotTheInheritedOne()
    {
        Apply(Toggle(Bold, AsTheStyle), toggleBase: IsBold).IsBold.ShouldBe(true);
        Apply(Toggle(Bold, AsTheStyle), toggleBase: StatesNoWeight).IsBold.ShouldBe(false);
    }

    /// <summary>
    /// The inherited value decides nothing once a base is given, in either direction.
    /// </summary>
    /// <remarks>
    /// This is the arm that fails at the base of this round: the old reading took
    /// <c>format.IsBold</c>, so a bold paragraph style and a plain one gave opposite answers for the
    /// same file bytes. A run whose CHPX names <c>Hyperlink</c> comes out bold whether its paragraph
    /// style is bold or not.
    /// </remarks>
    [Fact]
    public void TheValueBeingLayeredOntoDoesNotDecideAToggle()
    {
        Apply(Toggle(Bold, AgainstTheStyle), inherited: IsBold, toggleBase: StatesNoWeight)
            .IsBold.ShouldBe(true);
        Apply(Toggle(Bold, AgainstTheStyle), inherited: StatesNoWeight, toggleBase: StatesNoWeight)
            .IsBold.ShouldBe(true);
    }

    /// <summary>
    /// With no base given the accumulated value stands in, which is what a style chain resolves against.
    /// </summary>
    /// <remarks>
    /// A style's own <c>m_n81Flags</c> is its base style's flags with its own CHPX applied over them,
    /// so walking a chain outermost first and accumulating <em>is</em> the rule — the parameter exists
    /// for the one place the accumulated value is the wrong answer, and must not change the other.
    /// </remarks>
    [Fact]
    public void WithNoBaseTheAccumulatedValueIsTheStandIn()
    {
        Apply(Toggle(Bold, AgainstTheStyle), inherited: IsBold).IsBold.ShouldBe(false);
        Apply(Toggle(Bold, AgainstTheStyle), inherited: StatesNoWeight).IsBold.ShouldBe(true);
    }

    /// <summary>
    /// Two toggles in one grpprl are each measured against the base, not against each other.
    /// </summary>
    /// <remarks>
    /// <c>Read_BoldUsw</c> reads <c>pSI-&gt;m_n81Flags</c> afresh on every call, so a CHPX stating the
    /// same sprm twice resolves both against the style and the last one wins. The italic arm is the
    /// control: the two toggles are separate bits and neither reaches the other.
    /// </remarks>
    [Fact]
    public void EveryToggleInOneGrpprlIsMeasuredAgainstTheSameBase()
    {
        Ww8LayoutFormat twice = Apply(
            [.. Toggle(Bold, AgainstTheStyle), .. Toggle(Bold, AgainstTheStyle)],
            toggleBase: StatesNoWeight);
        twice.IsBold.ShouldBe(true);

        Ww8LayoutFormat both = Apply(
            [.. Toggle(Bold, AgainstTheStyle), .. Toggle(Italic, AgainstTheStyle)],
            toggleBase: IsBold);
        both.IsBold.ShouldBe(false);
        both.IsItalic.ShouldBe(true, "italic is a different bit of the same style's flags");
    }

    /// <summary>
    /// A plain 0 or 1 operand states the value outright and consults no style at all.
    /// </summary>
    /// <remarks>
    /// <c>bOn = *pData &amp; 1</c> is set before the style is looked at and the inversion is guarded by
    /// <c>*pData &amp; 0x80</c>, so only the two high-bit operands are relative to anything.
    /// </remarks>
    [Fact]
    public void AnAbsoluteOperandConsultsNoStyle()
    {
        Apply(Toggle(Bold, 1), toggleBase: IsBold).IsBold.ShouldBe(true);
        Apply(Toggle(Bold, 1), toggleBase: StatesNoWeight).IsBold.ShouldBe(true);
        Apply(Toggle(Bold, 0), toggleBase: IsBold).IsBold.ShouldBe(false);
        Apply(Toggle(Bold, 0), toggleBase: StatesNoWeight).IsBold.ShouldBe(false);
    }

    private static Ww8LayoutFormat IsBold => new() { IsBold = true };

    /// <summary>A style that states no weight at all, which is what <c>Hyperlink</c> is.</summary>
    /// <remarks>
    /// Its <c>m_n81Flags</c> bold bit is clear, so it answers <em>not bold</em> — not "whatever the
    /// paragraph resolved to", which is the whole of the defect.
    /// </remarks>
    private static Ww8LayoutFormat StatesNoWeight => default;

    /// <summary>A one-byte-operand toggle sprm.</summary>
    private static byte[] Toggle(ushort identifier, byte operand)
    {
        byte[] bytes = new byte[3];
        BinaryPrimitives.WriteUInt16LittleEndian(bytes, identifier);
        bytes[2] = operand;
        return bytes;
    }

    private static Ww8LayoutFormat Apply(
        byte[] grpprl, Ww8LayoutFormat inherited = default, Ww8LayoutFormat? toggleBase = null)
        => Ww8DocumentReader.ApplyLayoutSprms(
            inherited, grpprl.AsMemory(), Ww8DocumentProperties.Default, toggleBase);
}
