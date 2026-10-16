using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// <c>a:blip/a:clrChange</c> — PowerPoint's <em>Set Transparent Color</em> — is read, and
/// <c>from == to</c> with a transparent destination is a knockout rather than a no-op.
/// </summary>
/// <remarks>
/// <para>
/// This was a route, not a rule. <see cref="ColourKnockout"/>, its per-channel box match, its
/// binary alpha and the decoder that applies it all already existed, and the binary <c>.ppt</c>
/// path had populated them from Escher property 263 for rounds. Nothing in the tree read
/// <c>a:clrChange</c>, so every OOXML picture was drawn with its stored pixels.
/// </para>
/// <para>
/// <c>social-media-app-bulletin-january.pptx</c> page 3 is the corpus instance, and it was found
/// by a reviewer looking at a page of a document that <strong>passes every gate column</strong>.
/// Its wordmark is a 450 × 95 PNG, <strong>91.6% pure #000000</strong>, colour type 2 with no
/// alpha channel and no <c>tRNS</c>, under
/// <c>&lt;a:clrChange&gt;&lt;a:clrFrom&gt;&lt;a:srgbClr val="000000"/&gt;…&lt;a:alpha val="0"/&gt;</c>.
/// Drawn as stored it is an opaque black slab covering the words <em>Social Media</em> in the
/// title — and the title is still in the text layer, so the word count never moves and the gate
/// reports the document correct.
/// </para>
/// <para>
/// The values pinned here are read from LibreOffice's own
/// <c>lclCheckAndApplyChangeColorTransform</c>,
/// <c>oox/source/drawingml/fillproperties.cxx</c>:236-276.
/// </para>
/// </remarks>
public class BlipColourChangeTests
{
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string R = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    /// <summary>A <c>p:blipFill</c> whose blip carries the stated colour change.</summary>
    private static XElement BlipFill(string from, string to, string? alpha)
    {
        XElement destination = new(XName.Get("srgbClr", A), new XAttribute("val", to));
        if (alpha is not null)
        {
            destination.Add(new XElement(XName.Get("alpha", A), new XAttribute("val", alpha)));
        }

        return new XElement(
            XName.Get("blipFill", A),
            new XElement(
                XName.Get("blip", A),
                new XAttribute(XName.Get("embed", R), "rId2"),
                new XElement(
                    XName.Get("clrChange", A),
                    new XElement(
                        XName.Get("clrFrom", A),
                        new XElement(XName.Get("srgbClr", A), new XAttribute("val", from))),
                    new XElement(XName.Get("clrTo", A), destination))));
    }

    [Fact]
    public void AClrChangeIsRead()
    {
        DrawingColourChange change = DrawingFill.ReadBlip(BlipFill("000000", "000000", "0"))
            !.ColourChange.ShouldNotBeNull();

        change.From.Resolve(theme: null).ShouldBe(new Colour(0, 0, 0));

        // The destination's a:alpha rides inside the colour as an ordinary DrawingML transform,
        // which is why nothing here parses the attribute a second time.
        change.To.Resolve(theme: null).ShouldBe(new Colour(0, 0, 0, 0));
    }

    [Fact]
    public void ABlipWithoutAClrChangeStatesNone()
        => DrawingFill.ReadBlip(new XElement(
               XName.Get("blipFill", A),
               new XElement(XName.Get("blip", A))))!.ColourChange.ShouldBeNull();

    /// <summary>
    /// The rule the whole fix turns on, and the one an "obvious" reading gets backwards.
    /// </summary>
    /// <remarks>
    /// <c>fillproperties.cxx</c>:240 applies the transform when
    /// <c>(nFromColor != nToColor) || maColorChangeTo.hasTransparency()</c>. Every one of the 93
    /// occurrences in this corpus is <c>from == to</c>, so a reader that skips equal colours
    /// implements exactly nothing while looking correct.
    /// </remarks>
    [Fact]
    public void EqualColoursStillKnockOutWhenTheDestinationIsTransparent()
        => DrawingPictureEffects.Knockout(
               DrawingFill.ReadBlip(BlipFill("000000", "000000", "0")),
               theme: null, Png).ShouldNotBeNull();

    /// <summary>Equal colours and an opaque destination ask for nothing.</summary>
    [Fact]
    public void EqualColoursWithNoAlphaProduceNoKnockout()
        => DrawingPictureEffects.Knockout(
               DrawingFill.ReadBlip(BlipFill("FF0000", "FF0000", null)),
               theme: null, Png).ShouldBeNull();

    /// <summary>
    /// An opaque recolour produces no knockout, because <see cref="ColourKnockout"/> cannot
    /// express one and drawing the picture as stored is what we did before.
    /// </summary>
    [Fact]
    public void AnOpaqueRecolourProducesNoKnockout()
        => DrawingPictureEffects.Knockout(
               DrawingFill.ReadBlip(BlipFill("FFFFFF", "00FF00", null)),
               theme: null, Png).ShouldBeNull();

    [Fact]
    public void AKnockoutResolvesToItsFromColour()
    {
        ColourKnockout knockout = DrawingPictureEffects.Knockout(
            DrawingFill.ReadBlip(BlipFill("000000", "000000", "0")), theme: null, Png)
            .ShouldNotBeNull();

        knockout.Colour.ShouldBe(new Colour(0, 0, 0));
        knockout.Matches(0, 0, 0).ShouldBeTrue();
    }

    /// <summary>
    /// The tolerance is chosen by the stored format, and it is not one number.
    /// </summary>
    /// <remarks>
    /// <c>fillproperties.cxx</c>:245-264, the fix for tdf#149670. A lossy JPEG smears a flat
    /// background into a cloud of near-matches, so an exact match leaves a halo; a lossless PNG
    /// stores it exactly, so a wide tolerance eats real picture content. Note 15 and 1 are both
    /// different from the 9 the binary Escher call site passes, which is
    /// <see cref="ColourKnockout.DefaultTolerance"/> and is also correct — for that call site.
    /// </remarks>
    [Theory]
    [InlineData(new byte[] { 0x89, (byte)'P', (byte)'N', (byte)'G', 13, 10, 26, 10 }, 1)]
    [InlineData(new byte[] { 0xFF, 0xD8, 0xFF, 0xE0 }, 15)]
    [InlineData(new byte[] { (byte)'B', (byte)'M', 0, 0 }, 0)]
    [InlineData(new byte[] { (byte)'I', (byte)'I', 0x2A, 0x00 }, 1)]
    [InlineData(new byte[] { (byte)'M', (byte)'M', 0x00, 0x2A }, 1)]
    [InlineData(new byte[] { (byte)'G', (byte)'I', (byte)'F', (byte)'8' }, 9)]
    public void TheToleranceIsChosenByTheStoredFormat(byte[] encoded, int expected)
        => DrawingPictureEffects.ToleranceFor(encoded).ShouldBe(expected);

    /// <summary>
    /// The knockout the corpus actually asks for, end to end, at the tolerance a PNG gets.
    /// </summary>
    /// <remarks>
    /// Tolerance 1 on a PNG means a pixel one step off pure black still goes — and one two steps
    /// off does not. That is the boundary tdf#149670 moved, and pinning it is what stops a
    /// future change quietly restoring the flat 9, which on this deck's anti-aliased wordmark
    /// would eat the dark blue of the letter <em>chat</em>.
    /// </remarks>
    [Fact]
    public void APngKnockoutMatchesOneStepOffButNotTwo()
    {
        ColourKnockout knockout = DrawingPictureEffects.Knockout(
            DrawingFill.ReadBlip(BlipFill("000000", "000000", "0")), theme: null, Png)
            .ShouldNotBeNull();

        knockout.Tolerance.ShouldBe(1);
        knockout.Matches(1, 1, 1).ShouldBeTrue();
        knockout.Matches(2, 0, 0).ShouldBeFalse();
    }

    /// <summary>
    /// A picture that already carries an alpha channel is not knocked out at all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Measured, not reasoned. <c>Graphic::colorChange</c> branches on <c>aBitmap.HasAlpha()</c>
    /// (<c>vcl/source/graphic/UnoGraphic.cxx</c>:188-208): an alpha-bearing bitmap takes
    /// <c>ChangeColorAlpha</c>, and only a bitmap without alpha reaches the
    /// <c>CreateAlphaMask(aColorFrom, nTolerance)</c> branch that is the knockout.
    /// </para>
    /// <para>
    /// Confirmed on the installed 26.2.4.2 with two authored one-shape decks differing in
    /// exactly one thing — the same pixels and the same <c>clrChange</c>, saved once as RGB PNG
    /// and once as RGBA. The RGB deck renders the colour knocked out; the RGBA deck renders it
    /// untouched.
    /// </para>
    /// <para>
    /// Without this, <c>vv_summit_SAIC-PRESENTATION*.pptx</c> page 13 — an RGBA PNG that is
    /// 66.1% <c>F4F4F4</c> — went from an <strong>exact</strong> page to 0.28 unaccounted ink.
    /// </para>
    /// </remarks>
    [Fact]
    public void AnAlphaBearingPictureIsNotKnockedOut()
        => DrawingPictureEffects.Knockout(
               DrawingFill.ReadBlip(BlipFill("F4F4F4", "F4F4F4", "0")),
               theme: null, Rgba).ShouldBeNull();

    [Fact]
    public void APictureWithoutAlphaIsKnockedOut()
        => DrawingPictureEffects.Knockout(
               DrawingFill.ReadBlip(BlipFill("F4F4F4", "F4F4F4", "0")),
               theme: null, Rgb).ShouldNotBeNull();

    [Theory]
    [InlineData(6, true)]    // RGBA
    [InlineData(4, true)]    // grey + alpha
    [InlineData(2, false)]   // truecolour
    [InlineData(0, false)]   // greyscale
    [InlineData(3, false)]   // palette, no tRNS
    public void APngsColourTypeDecidesWhetherItCarriesAlpha(byte colourType, bool expected)
        => DrawingPictureEffects.HasAlphaChannel(PngHeader(colourType)).ShouldBe(expected);

    /// <summary>A palette PNG carries alpha through a <c>tRNS</c> chunk rather than its type.</summary>
    [Fact]
    public void APaletteePngWithATrnsChunkCarriesAlpha()
        => DrawingPictureEffects.HasAlphaChannel(PngHeader(3, "tRNS")).ShouldBeTrue();

    /// <summary>No <c>tRNS</c> may follow the first <c>IDAT</c>, so the scan stops there.</summary>
    [Fact]
    public void ATrnsAfterTheFirstIdatIsNotLookedFor()
        => DrawingPictureEffects.HasAlphaChannel(PngHeader(3, "IDAT", "tRNS")).ShouldBeFalse();

    [Fact]
    public void AJpegNeverCarriesAlpha()
        => DrawingPictureEffects.HasAlphaChannel([0xFF, 0xD8, 0xFF, 0xE0, 0, 0, 0, 0]).ShouldBeFalse();

    /// <summary>
    /// <c>useA="0"</c> makes the reference discard the destination's transparency, so the
    /// knockout becomes nothing.
    /// </summary>
    /// <remarks>
    /// <c>ColorChangeContext::~ColorChangeContext</c> calls
    /// <c>maColorChangeTo.clearTransparence()</c> when <c>useA</c> is false
    /// (<c>oox/source/drawingml/misccontexts.cxx</c>:266-270). All 93 corpus occurrences state
    /// no <c>useA</c> and so default to true, so this reaches nothing today — it is pinned
    /// because the attribute is the whole difference between knocking a colour out and not.
    /// </remarks>
    [Fact]
    public void UseAlphaFalseProducesNoKnockout()
    {
        XElement fill = BlipFill("000000", "000000", "0");
        fill.Descendants().First(e => e.Name.LocalName == "clrChange")
            .Add(new XAttribute("useA", "0"));

        DrawingFill.ReadBlip(fill)!.ColourChange!.Value.UseAlpha.ShouldBeFalse();
        DrawingPictureEffects.Knockout(DrawingFill.ReadBlip(fill), theme: null, Rgb).ShouldBeNull();
    }

    [Fact]
    public void UseAlphaDefaultsToTrue()
        => DrawingFill.ReadBlip(BlipFill("000000", "000000", "0"))
               !.ColourChange!.Value.UseAlpha.ShouldBeTrue();

    /// <summary>A PNG signature plus a complete IHDR, then the named zero-length chunks.</summary>
    private static byte[] PngHeader(byte colourType, params string[] chunks)
    {
        List<byte> bytes =
        [
            0x89, (byte)'P', (byte)'N', (byte)'G', 13, 10, 26, 10,
            0, 0, 0, 13, (byte)'I', (byte)'H', (byte)'D', (byte)'R',
            0, 0, 0, 1, 0, 0, 0, 1,      // 1x1
            8, colourType, 0, 0, 0,
            0, 0, 0, 0,                  // CRC
        ];

        foreach (string chunk in chunks)
        {
            bytes.AddRange([0, 0, 0, 0]);
            bytes.AddRange(chunk.Select(c => (byte)c));
            bytes.AddRange([0, 0, 0, 0]);
        }

        return [.. bytes];
    }

    private static ReadOnlySpan<byte> Png => Rgb;
    private static ReadOnlySpan<byte> Rgb => PngHeader(2);
    private static ReadOnlySpan<byte> Rgba => PngHeader(6);
}
