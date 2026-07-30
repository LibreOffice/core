using System.Text;

namespace Paperless.Text.Encodings;

/// <summary>
/// Resolves the legacy Windows and IBM code pages that RTF, DOC, XLS and PPT store 8-bit text
/// in.
/// </summary>
/// <remarks>
/// <para>
/// Every legacy Microsoft format names its encoding by number, and the number comes from the
/// file: an <c>\ansicpg</c> control word in RTF, a <c>CODEPAGE</c> record in BIFF, an LCID in
/// the WW8 FIB. Reading text without honouring it produces the failure the
/// <c>extraction-comparison</c> skill describes as systematic corruption of every non-ASCII
/// character while ASCII stays perfect.
/// </para>
/// <para>
/// .NET Core ships only UTF-8, UTF-16, ASCII and Latin-1 as built-in encodings; the rest live
/// behind <see cref="CodePagesEncodingProvider"/>, which is part of the framework on .NET 10
/// and needs registering once. That is what this type exists to do — registering it lazily
/// rather than at start-up, and in one place rather than in each reader that discovers the need.
/// </para>
/// <para>
/// <strong>Latin-1 is not Windows-1252.</strong> They differ across 0x80–0x9F, which is exactly
/// where curly quotes, en and em dashes, the ellipsis and the euro sign live — so substituting
/// one for the other silently mangles the punctuation of ordinary Western documents rather than
/// failing visibly.
/// </para>
/// </remarks>
public static class LegacyCodePages
{
    /// <summary>
    /// Windows-1252, the default for Western legacy documents and the fallback when a file
    /// names a code page that cannot be resolved.
    /// </summary>
    public const int WindowsWestern = 1252;

    /// <summary>
    /// Code page 1200: not an 8-bit encoding at all, but the marker BIFF8 uses to say its
    /// strings are natively UTF-16 and must not be reinterpreted.
    /// </summary>
    /// <remarks>
    /// Per <c>research/05-infrastructure.md</c> section F.2 this is the single most important
    /// special case: treating it as a code page would reinterpret already-correct UTF-16 text.
    /// </remarks>
    public const int Utf16Marker = 1200;

    private static readonly Lazy<bool> Registration = new(() =>
    {
        Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);
        return true;
    });

    /// <summary>
    /// The encoding for a code page number, or <see cref="Fallback"/> when it cannot be
    /// resolved.
    /// </summary>
    /// <param name="codePage">
    /// The number as recorded in the file. Zero, negative and unknown values fall back rather
    /// than throwing: a file naming a code page this platform lacks is still worth reading, and
    /// the fallback gets the ASCII range right in every case.
    /// </param>
    /// <param name="resolved">
    /// False when the requested code page could not be honoured, so a caller can record a
    /// diagnostic — a silent substitution is how encoding bugs stay hidden.
    /// </param>
    public static Encoding Get(int codePage, out bool resolved)
    {
        _ = Registration.Value;

        if (codePage is <= 0 or Utf16Marker)
        {
            resolved = false;
            return Fallback;
        }

        try
        {
            resolved = true;
            return Encoding.GetEncoding(codePage);
        }
        catch (ArgumentException)
        {
            // Not a code page this platform knows.
            resolved = false;
            return Fallback;
        }
        catch (NotSupportedException)
        {
            resolved = false;
            return Fallback;
        }
    }

    /// <inheritdoc cref="Get(int, out bool)"/>
    public static Encoding Get(int codePage) => Get(codePage, out _);

    /// <summary>
    /// The encoding used when a file's own code page cannot be honoured.
    /// </summary>
    /// <remarks>
    /// Windows-1252 rather than Latin-1 or UTF-8: it is what the overwhelming majority of
    /// legacy documents actually are, it agrees with ASCII everywhere, and it is what
    /// LibreOffice falls back to as well
    /// (<c>research/05-infrastructure.md</c> section F.2).
    /// </remarks>
    public static Encoding Fallback
    {
        get
        {
            _ = Registration.Value;
            try
            {
                return Encoding.GetEncoding(WindowsWestern);
            }
            catch (ArgumentException)
            {
                // Should not happen once the provider is registered, but a reader must not fail
                // outright because a platform is unusually stripped down.
                return Encoding.Latin1;
            }
        }
    }

    /// <summary>
    /// The Windows ANSI code page a language would have used, for formats that record a language
    /// rather than an encoding.
    /// </summary>
    /// <remarks>
    /// The WW8 FIB names a language id, not a code page, so the encoding has to be inferred from
    /// it — which is what LibreOffice's <c>utl_getWinTextEncodingFromLangStr</c> does. Keyed on
    /// the ISO language prefix, since that is what decides the code page; the country does not.
    /// Anything unlisted is Western, which is both the common case and the safest guess.
    /// </remarks>
    /// <param name="isoLanguage">
    /// A language tag or bare ISO 639 code, e.g. <c>ru</c> or <c>ja-JP</c>.
    /// </param>
    public static int FromLanguage(string? isoLanguage)
    {
        if (string.IsNullOrWhiteSpace(isoLanguage)) return WindowsWestern;

        int separator = isoLanguage.AsSpan().IndexOfAny('-', '_');
        string language = (separator < 0 ? isoLanguage : isoLanguage[..separator]).ToLowerInvariant();

        return language switch
        {
            "ja" => 932,
            "zh" => 936,   // Simplified; traditional locales are corrected by the caller's country
            "ko" => 949,
            "th" => 874,
            "ru" or "uk" or "be" or "bg" or "sr" or "mk" or "kk" or "ky" or "tt" or "mn" => 1251,
            "cs" or "pl" or "sk" or "sl" or "hu" or "hr" or "ro" or "sq" or "bs" => 1250,
            "el" => 1253,
            "tr" or "az" => 1254,
            "he" or "yi" => 1255,
            "ar" or "fa" or "ur" or "ps" => 1256,
            "et" or "lv" or "lt" => 1257,
            "vi" => 1258,
            _ => WindowsWestern,
        };
    }
}
