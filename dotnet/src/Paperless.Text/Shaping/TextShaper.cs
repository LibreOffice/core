namespace Paperless.Text.Shaping;

/// <summary>
/// The shaper layout uses when the caller has no opinion.
/// </summary>
/// <remarks>
/// HarfBuzz when its native library is there, metrics-only when it is not. The fallback is the leniency
/// rule applied to a dependency rather than to a file: a machine without the native library should
/// still lay documents out, with line breaks that are wrong by the amount of the font's kerning, rather
/// than fail. <see cref="IsShapingAvailable"/> says which was chosen, so a caller comparing against a
/// reference renderer can tell whether a difference is worth investigating.
/// </remarks>
public static class TextShaper
{
    private static readonly HarfBuzzShaper? SharedHarfBuzz =
        HarfBuzzShaper.IsAvailable ? new HarfBuzzShaper() : null;

    /// <summary>The default shaper: HarfBuzz if it loaded, otherwise metrics-only.</summary>
    public static ITextShaper Default { get; } = SharedHarfBuzz ?? (ITextShaper)MetricsShaper.Instance;

    /// <summary>
    /// True when the default shaper applies kerning and ligatures.
    /// </summary>
    /// <remarks>
    /// False means measurements will be a little wide — around a quarter of an em per line of Latin
    /// prose — and that a line whose last word ends near the margin may break one word early.
    /// </remarks>
    public static bool IsShapingAvailable => SharedHarfBuzz is not null;
}
