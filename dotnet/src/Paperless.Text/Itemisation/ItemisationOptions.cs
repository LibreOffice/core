using Paperless.Text.Fonts;

namespace Paperless.Text.Itemisation;

/// <summary>
/// How a paragraph is cut into the sub-runs a shaper is handed.
/// </summary>
/// <remarks>
/// <para>
/// Everything here is off or neutral by default, so a paragraph of Latin prose is cut into exactly
/// one sub-run and shaped in exactly the call it was shaped in before any of this existed. That is
/// not a convenience: a paragraph split into runs it does not need loses the shaping context at each
/// boundary and measures very slightly wide, which is enough to move a line break — so the
/// no-op case has to be a genuine no-op rather than an equivalent-looking one.
/// </para>
/// <para>
/// Glyph fallback is opt-in for the same reason it is a separate interface: a caller who has not
/// supplied a resolver gets no coverage checks at all, which is both what the old behaviour was and
/// what an extraction-only caller wants.
/// </para>
/// </remarks>
public sealed record ItemisationOptions
{
    /// <summary>The neutral settings: left to right, no glyph fallback.</summary>
    public static ItemisationOptions Default { get; } = new();

    /// <summary>
    /// The direction the paragraph is declared to have.
    /// </summary>
    /// <remarks>
    /// The paragraph's own writing mode, not a guess from its text. LibreOffice takes it from the
    /// paragraph properties (<c>SwScriptInfo::InitScriptInfo</c> passes
    /// <c>m_nDefaultDir</c> straight to <c>ubidi_setPara</c>), so an English paragraph containing
    /// only Hebrew still starts at the left margin.
    /// </remarks>
    public BidiDirection BaseDirection { get; init; } = BidiDirection.LeftToRight;

    /// <summary>
    /// Where to look for a face when the run's own has no glyph, or null to not look.
    /// </summary>
    /// <remarks>
    /// <see cref="SystemFontResolver"/> implements this, so the usual thing to pass is the resolver
    /// that chose the run's face in the first place — which also collects the fallbacks it made
    /// where a caller comparing against a reference renderer can find them.
    /// </remarks>
    public IGlyphFallbackResolver? GlyphFallback { get; init; }

    /// <summary>Called once per character that needed a fallback face, resolved or not.</summary>
    public Action<GlyphFallback>? OnGlyphFallback { get; init; }
}
