using XRay.Office.Core.Graphics;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// A reference built for a substituted face carries the lean the request asked for.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="IGlyphFallbackResolver.ReferenceFor(OpenTypeFace)"/> is a reverse lookup from a face
/// and its own remark says what is wrong with it: it has <em>no request to compare against</em>, so
/// it cannot answer <c>LogicalFontInstance::NeedsArtificialItalic()</c> and never did. Every face
/// reached through <see cref="XRay.Office.Text.Itemisation.FontItemiser"/> therefore arrived at the
/// page upright, however italic the run around it was.
/// </para>
/// <para>
/// Measured against LibreOffice <b>26.2.4.2</b> on 41 authored two-run packages over six filters —
/// <c>.docx</c>, <c>.fodt</c>, <c>.fodp</c>, <c>.fods</c>, <c>.pptx</c>, <c>.xlsx</c> —
/// (<c>probes/words-r58/fallback-oblique.py</c> and <c>fallback-oblique-ooxml.py</c>). Every italic
/// case shears on the reference and none did here; four negative controls are nought on both sides
/// in all six. Corpus reach before the change: the words track leaned <b>0</b> of the 6 616 glyphs
/// it draws in faces no document names against the reference's 289 of 9 391; slides 4 of 5 530
/// against 345 of 5 242; sheets 0 of 16 663 against 4 of 17 159.
/// </para>
/// </remarks>
public sealed class FallbackObliqueReferenceTests
{
    private static SystemFontResolver Fonts { get; } = new(SystemFontIndex.Build());

    /// <summary>
    /// The overload asked through the interface, which is the only place it exists.
    /// </summary>
    /// <remarks>
    /// A default interface method is reachable only through the interface, and that is deliberate:
    /// the rule is stated once, in <c>IGlyphFallbackResolver</c>, so neither implementer can drift
    /// from it — and every production call site already holds the interface rather than the class.
    /// The cast is written inline rather than as an interface-typed field because CA1859 refuses
    /// the field.
    /// </remarks>
    private static FontReference? Ask(IFontResolver resolver, OpenTypeFace face, bool italic)
        => ((IGlyphFallbackResolver)resolver).ReferenceFor(face, italic);

    /// <summary>U+6C49 汉, which no Latin face installed for this project covers.</summary>
    private const int Han = 0x6C49;

    private static OpenTypeFace Cjk =>
        Fonts.FallbackFor(Han) ?? throw new InvalidOperationException("no CJK face is installed");

    private static OpenTypeFace RealItalic =>
        Fonts.LoadOpenType(Fonts.Resolve(new FontRequest("Liberation Serif", 400, true)));

    [Fact]
    public void TheFallbackFaceHasNoItalicOfItsOwn()
        // The premise. Were an italic CJK face to be installed, the assertion below would be
        // asserting the wrong half of the rule and would still pass.
        => Cjk.IsItalic.ShouldBeFalse();

    [Fact]
    public void AnItalicRequestLeansASubstituteThatHasNoItalic()
        => Ask(Fonts, Cjk, italic: true)
            .ShouldNotBeNull()
            .SyntheticOblique.ShouldBeTrue();

    [Fact]
    public void AnUprightRequestLeansNothing()
        => Ask(Fonts, Cjk, italic: false)
            .ShouldNotBeNull()
            .SyntheticOblique.ShouldBeFalse();

    [Fact]
    public void TheOneArgumentOverloadStillLeansNothing()
        // The old signature keeps its old answer, so a caller that has no request to give is not
        // silently changed by this: an implementation of the interface that only answers coverage
        // questions stays valid and stays upright.
        => Fonts.ReferenceFor(Cjk).ShouldNotBeNull().SyntheticOblique.ShouldBeFalse();

    [Fact]
    public void ASubstituteThatIsItselfItalicIsNotLeanedTwice()
    {
        // NeedsArtificialItalic() is "italic was asked for AND the face that answered has none".
        // Without the second half a fallback that does have an italic would be sheared on top of
        // its own slant, which is a visibly different page rather than a subtle one.
        //
        // Asked through a stub rather than through the system resolver, and not for convenience:
        // no family installed here has both an italic and a character the earlier entries of
        // LibreOffice's fallback list lack, so the real resolver cannot be made to return an
        // italic substitute on this machine. The stub makes the reverse lookup succeed and leaves
        // the rule itself — which is the default method's, not the resolver's — the only thing
        // under test.
        RealItalic.IsItalic.ShouldBeTrue("the fixture needs a face that really is italic");

        IGlyphFallbackResolver stub = new AlwaysAnswers();

        stub.ReferenceFor(RealItalic, isItalicRequested: true)
            .ShouldNotBeNull()
            .SyntheticOblique.ShouldBeFalse();

        // ... and the same stub does lean an upright face, so the assertion above is not simply
        // the stub answering false to everything.
        stub.ReferenceFor(Cjk, isItalicRequested: true)
            .ShouldNotBeNull()
            .SyntheticOblique.ShouldBeTrue();
    }

    /// <summary>A resolver whose reverse lookup always succeeds, and does nothing else.</summary>
    private sealed class AlwaysAnswers : IGlyphFallbackResolver
    {
        public OpenTypeFace? FallbackFor(int codePoint, int weight = 400, bool isItalic = false)
            => null;

        public FontReference? ReferenceFor(OpenTypeFace face)
            => new()
            {
                FamilyName = face.FamilyName ?? string.Empty,
                RequestedFamily = face.FamilyName ?? string.Empty,
                Weight = face.Weight,
                IsItalic = face.IsItalic,
                FaceKey = "stub",
            };
    }

    [Fact]
    public void TheLeaningReferenceStillNamesAFileSoItCanBeEmbedded()
    {
        // A face is enough to shape with and not enough to embed: the PDF writer opens the font
        // program through the face key. Rebuilding the reference to add the lean must not lose it.
        FontReference reference = Ask(Fonts, Cjk, italic: true).ShouldNotBeNull();

        reference.FaceKey.ShouldNotBeNullOrEmpty();
        reference.FamilyName.ShouldBe(Fonts.ReferenceFor(Cjk).ShouldNotBeNull().FamilyName);
    }

    [Fact]
    public void AFaceTheResolverNeverHandedOutIsStillUnanswerable()
    {
        // The reverse lookup's null is the caller's signal to fall back to naming the face itself,
        // and adding an overload must not turn that null into a reference with no face key behind
        // it — which would be announced in the PDF and not embedded.
        SystemFontResolver other = new(SystemFontIndex.Build());

        Ask(other, Cjk, italic: true).ShouldBeNull();
    }
}
