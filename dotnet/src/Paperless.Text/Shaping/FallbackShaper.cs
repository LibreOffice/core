using Paperless.Text.Fonts;
using Paperless.Text.Itemisation;

namespace Paperless.Text.Shaping;

/// <summary>
/// A shaper that hands the characters its face cannot draw to a face that can.
/// </summary>
/// <remarks>
/// <para>
/// <strong>For measuring, not for drawing.</strong> The result splices glyphs taken from several
/// faces into one <see cref="ShapedText"/>, whose glyph identifiers are therefore only meaningful
/// beside the face each came from — which this type deliberately does not report. What it does get
/// right is every <em>advance</em>, and an advance is what a line breaker asks for. A caller that
/// needs the glyphs themselves has to itemise for itself and shape each run, which is what
/// <c>MeasuredParagraph</c> and the spreadsheet cell painter do.
/// </para>
/// <para>
/// It exists because a face is chosen once per run and coverage is a property of a character. A
/// Japanese title in a cell whose format names a Latin face measures, without this, as a row of
/// <c>.notdef</c> advances — narrow, arbitrary, and nothing like the full-width ideographs the
/// reference renderer lays out after its own fallback. Measured on
/// <c>seihon_zassi_kikou_20221215.xlsx</c>: 121 rows come out one line short, every one of them by
/// exactly one line pitch.
/// </para>
/// <para>
/// The fallback face's advances are rescaled into the <em>primary</em> face's design grid, because
/// <see cref="ShapedText"/> carries one <see cref="ShapedText.UnitsPerEm"/> for the whole run and
/// every caller scales through it. Rescaling per glyph rather than at the end is unavoidable here
/// and costs at most half a design unit a glyph, which at 2048 units to the em is a thousandth of a
/// point at ordinary sizes.
/// </para>
/// </remarks>
public sealed class FallbackShaper : ITextShaper
{
    private readonly ITextShaper _inner;
    private readonly IGlyphFallbackResolver _fallback;

    /// <summary>Wraps a shaper so that uncovered characters are shaped by a face that covers them.</summary>
    /// <param name="inner">The shaper doing the work, once a face has been chosen.</param>
    /// <param name="fallback">Where to look when the run's own face has no glyph.</param>
    public FallbackShaper(ITextShaper inner, IGlyphFallbackResolver fallback)
    {
        ArgumentNullException.ThrowIfNull(inner);
        ArgumentNullException.ThrowIfNull(fallback);
        _inner = inner;
        _fallback = fallback;
    }

    /// <inheritdoc/>
    public ShapedText Shape(OpenTypeFace face, ReadOnlySpan<char> text, ShapingOptions options = default)
    {
        ArgumentNullException.ThrowIfNull(face);

        if (text.Length == 0) return _inner.Shape(face, text, options);

        List<FaceRun> runs = FontItemiser.Split(text, 0, text.Length, face, _fallback);

        // The overwhelmingly common case: everything is in the face that was asked for, and the run
        // reaches the shaper in exactly the call it did before this existed. Shaping is contextual,
        // so a run split it did not need would measure very slightly differently.
        if (runs.Count == 1 && !runs[0].IsFallback) return _inner.Shape(face, text, options);

        int units = face.UnitsPerEm > 0 ? face.UnitsPerEm : 1000;
        List<ShapedGlyph> spliced = new(text.Length);

        foreach (FaceRun run in runs)
        {
            if (run.Length <= 0) continue;

            ShapedText part = _inner.Shape(run.Face, text.Slice(run.Start, run.Length), options);
            int partUnits = part.UnitsPerEm > 0 ? part.UnitsPerEm : units;

            foreach (ShapedGlyph glyph in part.Glyphs)
            {
                spliced.Add(glyph with
                {
                    Cluster = glyph.Cluster + run.Start,
                    Advance = partUnits == units
                        ? glyph.Advance
                        : (int)Math.Round((double)glyph.Advance * units / partUnits),
                    OffsetX = partUnits == units
                        ? glyph.OffsetX
                        : (int)Math.Round((double)glyph.OffsetX * units / partUnits),
                    OffsetY = partUnits == units
                        ? glyph.OffsetY
                        : (int)Math.Round((double)glyph.OffsetY * units / partUnits),
                });
            }
        }

        ShapedGlyph[] glyphs = [.. spliced];
        return new ShapedText(glyphs, ShapedText.PrefixWidths(glyphs, text.Length), units);
    }
}
