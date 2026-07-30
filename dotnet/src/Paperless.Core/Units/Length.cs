namespace Paperless.Core.Units;

/// <summary>
/// A device-independent length, stored as an exact integral number of EMUs
/// (English Metric Units, 914400 per inch).
/// </summary>
/// <remarks>
/// <para>
/// Office formats each pick a different native unit, and converting through
/// floating point loses the exactness that round-tripping needs. EMU is the
/// natural common denominator: 914400 = 2^5 x 3^2 x 5^2 x 127, which divides
/// evenly by every unit we care about.
/// </para>
/// <list type="table">
///   <listheader><term>Unit</term><description>EMU per unit — where it comes from</description></listheader>
///   <item><term>inch</term>       <description>914400 —</description></item>
///   <item><term>point (1/72")</term><description>12700 — RTF font sizes, PDF user space</description></item>
///   <item><term>twip (1/1440")</term><description>635 — Writer's internal unit, DOC/DOCX/RTF measurements</description></item>
///   <item><term>1/100 mm</term>   <description>360 — the draw layer's internal unit, ODF</description></item>
///   <item><term>EMU</term>        <description>1 — OOXML DrawingML</description></item>
/// </list>
/// <para>
/// A signed 64-bit EMU count covers roughly +/- 10^13 metres, so overflow is
/// not a practical concern; we use <see cref="long"/> rather than <see cref="int"/>
/// because a single inch is already 914400.
/// </para>
/// </remarks>
public readonly struct Length : IEquatable<Length>, IComparable<Length>
{
    /// <summary>EMUs per inch.</summary>
    public const long EmuPerInch = 914400;

    /// <summary>EMUs per PostScript point (1/72 inch).</summary>
    public const long EmuPerPoint = EmuPerInch / 72;

    /// <summary>EMUs per twip (1/1440 inch), the unit used throughout Writer and the MS binary formats.</summary>
    public const long EmuPerTwip = EmuPerInch / 1440;

    /// <summary>EMUs per 1/100 mm, the unit used by the draw layer and ODF.</summary>
    public const long EmuPerMm100 = EmuPerInch / 2540;

    /// <summary>EMUs per millimetre.</summary>
    public const long EmuPerMillimetre = EmuPerMm100 * 100;

    /// <summary>A zero length.</summary>
    public static Length Zero => default;

    private Length(long emu) => Emu = emu;

    /// <summary>The length in EMUs.</summary>
    public long Emu { get; }

    /// <summary>Creates a length from a raw EMU count.</summary>
    public static Length FromEmu(long emu) => new(emu);

    /// <summary>Creates a length from twips (1/1440 inch).</summary>
    public static Length FromTwips(long twips) => new(twips * EmuPerTwip);

    /// <summary>Creates a length from hundredths of a millimetre.</summary>
    public static Length FromMm100(long mm100) => new(mm100 * EmuPerMm100);

    /// <summary>Creates a length from PostScript points (1/72 inch).</summary>
    public static Length FromPoints(double points) => new((long)Math.Round(points * EmuPerPoint));

    /// <summary>Creates a length from inches.</summary>
    public static Length FromInches(double inches) => new((long)Math.Round(inches * EmuPerInch));

    /// <summary>Creates a length from millimetres.</summary>
    public static Length FromMillimetres(double mm) => new((long)Math.Round(mm * EmuPerMillimetre));

    /// <summary>Creates a length from device pixels at the given resolution.</summary>
    public static Length FromPixels(double pixels, double dpi) => new((long)Math.Round(pixels / dpi * EmuPerInch));

    /// <summary>The length in twips, rounded to nearest.</summary>
    public long Twips => DivideRounded(Emu, EmuPerTwip);

    /// <summary>The length in hundredths of a millimetre, rounded to nearest.</summary>
    public long Mm100 => DivideRounded(Emu, EmuPerMm100);

    /// <summary>The length in PostScript points.</summary>
    public double Points => (double)Emu / EmuPerPoint;

    /// <summary>The length in inches.</summary>
    public double Inches => (double)Emu / EmuPerInch;

    /// <summary>The length in millimetres.</summary>
    public double Millimetres => (double)Emu / EmuPerMillimetre;

    /// <summary>Converts to device pixels at the given resolution.</summary>
    public double ToPixels(double dpi) => Inches * dpi;

    private static long DivideRounded(long value, long divisor)
    {
        // Round half away from zero, matching how the office formats round.
        long half = divisor / 2;
        return value >= 0 ? (value + half) / divisor : (value - half) / divisor;
    }

    /// <inheritdoc/>
    public bool Equals(Length other) => Emu == other.Emu;

    /// <inheritdoc/>
    public override bool Equals(object? obj) => obj is Length other && Equals(other);

    /// <inheritdoc/>
    public override int GetHashCode() => Emu.GetHashCode();

    /// <inheritdoc/>
    public int CompareTo(Length other) => Emu.CompareTo(other.Emu);

    /// <inheritdoc/>
    public override string ToString() => $"{Points:0.###}pt";

    /// <summary>Adds two lengths.</summary>
    public static Length operator +(Length a, Length b) => new(a.Emu + b.Emu);

    /// <summary>Subtracts one length from another.</summary>
    public static Length operator -(Length a, Length b) => new(a.Emu - b.Emu);

    /// <summary>Negates a length.</summary>
    public static Length operator -(Length a) => new(-a.Emu);

    /// <summary>Scales a length.</summary>
    public static Length operator *(Length a, double factor) => new((long)Math.Round(a.Emu * factor));

    /// <summary>Scales a length.</summary>
    public static Length operator *(double factor, Length a) => a * factor;

    /// <summary>Divides a length.</summary>
    public static Length operator /(Length a, double divisor) => new((long)Math.Round(a.Emu / divisor));

    /// <summary>Compares two lengths.</summary>
    public static bool operator <(Length a, Length b) => a.Emu < b.Emu;

    /// <summary>Compares two lengths.</summary>
    public static bool operator >(Length a, Length b) => a.Emu > b.Emu;

    /// <summary>Compares two lengths.</summary>
    public static bool operator <=(Length a, Length b) => a.Emu <= b.Emu;

    /// <summary>Compares two lengths.</summary>
    public static bool operator >=(Length a, Length b) => a.Emu >= b.Emu;

    /// <summary>Compares two lengths for equality.</summary>
    public static bool operator ==(Length a, Length b) => a.Emu == b.Emu;

    /// <summary>Compares two lengths for inequality.</summary>
    public static bool operator !=(Length a, Length b) => a.Emu != b.Emu;
}
