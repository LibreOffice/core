namespace XRay.Office.Text.Fonts;

/// <summary>
/// The metric-compatible faces shipped beside the library: a floor under the machine's font set,
/// not an override of it.
/// </summary>
/// <remarks>
/// <para>
/// <strong>What they are for.</strong> A rendering that silently depends on what happens to be
/// installed is not reproducible, and this project has measured the cost. <c>dotnet/CLAUDE.md</c>
/// records a container that lost <c>fonts-dejavu-core</c>: holding LibreOffice constant and
/// varying only the font set moved <b>53 of 534 page counts and 426 pages</b> — the same order as
/// a whole reference-version change. Nothing declared the font set, so it survived an entire pass
/// unnoticed, and <c>fc-match</c> could not have warned about it because it never fails; it always
/// returns <em>something</em>. Shipping Carlito, Caladea, Liberation and DejaVu means a machine
/// that lacks them substitutes nothing.
/// </para>
/// <para>
/// <strong>Installed wins, and that direction was measured rather than assumed.</strong> The
/// obvious design is the opposite one — prefer ours, so every machine renders identically — and it
/// is wrong. Measured 2026-09-03 against LibreOffice 26.2.4.2 from the TDF tarball, the same build
/// these files came from, <c>XRay.Office.Fidelity.Tests</c> over 552 comparisons:
/// </para>
/// <list type="table">
///   <item><description>bundle as a fallback (installed wins) — <b>36 failed</b></description></item>
///   <item><description>bundle preferred over installed — <b>68 failed</b></description></item>
/// </list>
/// <para>
/// Preferring them is twice as bad, because <strong>LibreOffice does not read its own bundled
/// fonts either</strong>: it resolves through fontconfig, which sees <c>/usr/share/fonts</c>, and
/// the copies it ships are its own floor for systems without them. Preferring ours therefore
/// moves us away from the reference rather than towards it.
/// </para>
/// <para>
/// It matters for exactly one family. Comparing the shipped files against Ubuntu 24.04's by their
/// own <c>hmtx</c>, Carlito and Liberation Sans are <em>metrically identical</em> — bundling them
/// changes no advance at all — while <b>Caladea genuinely differs</b>: <c>A</c> is 599 units
/// installed against 623 shipped, <c>o</c> 480 against 531, <c>M</c> 888 against 815. Up to 10% on
/// a glyph is a line-break difference, which is a pagination difference. So the whole of the
/// preference question is Caladea, and the answer is to defer to the machine.
/// </para>
/// <para>
/// <see cref="Preferred"/> reverses it for the case where reproducibility is worth more than
/// agreement with a local LibreOffice — rendering the same document identically on machines whose
/// font sets differ. It is opt-in because the measurement above says the default cannot be.
/// </para>
/// <para>
/// Content files rather than embedded resources. A face has to be a path by the time anything
/// draws with it — <c>SystemFontResolver.LoadFace</c>, <c>FileFontProvider</c> and
/// <c>SkiaDrawingSink.TypefaceFor</c> all open one, which is the whole reason
/// <see cref="EmbeddedFontStore"/> writes a document's own embedded faces out to disk. Embedding
/// these would mean extracting 17 MB to a temporary directory on first use to arrive back where
/// copying them beside the assembly already is.
/// </para>
/// </remarks>
public static class BundledFonts
{
    /// <summary>The directory the faces are copied into, beside the assembly.</summary>
    private const string FolderName = "fonts";

    /// <summary>The variable that turns the bundle off, or promotes it above the machine's.</summary>
    /// <remarks>
    /// <c>0</c>, <c>false</c> or <c>no</c> switches them off entirely; <c>prefer</c> puts them
    /// ahead of the installed faces; anything else, including unset, leaves them as the fallback.
    /// Read that way round so a variable set to an unexpected value cannot silently disable them
    /// or silently change what a document renders as.
    /// </remarks>
    public const string Variable = "XRAY_OFFICE_BUNDLED_FONTS";

    private static readonly Lazy<string?> Located = new(Locate, isThreadSafe: true);

    private static string? Setting => Environment.GetEnvironmentVariable(Variable);

    /// <summary>Whether the shipped faces are available at all.</summary>
    public static bool Enabled => Setting is not ("0" or "false" or "no");

    /// <summary>
    /// Whether they are searched <em>before</em> the machine's rather than after.
    /// </summary>
    /// <remarks>
    /// Off by default. Turning it on trades agreement with a locally installed LibreOffice for
    /// identical output across machines; the remarks on this class carry the measurement that
    /// decides which of those a given round wants.
    /// </remarks>
    public static bool Preferred =>
        Enabled && string.Equals(Setting, "prefer", StringComparison.OrdinalIgnoreCase);

    /// <summary>
    /// Where the shipped faces are, or null when they were not deployed or are switched off.
    /// </summary>
    public static string? Directory => Enabled ? Located.Value : null;

    private static string? Locate()
    {
        // AppContext.BaseDirectory rather than the assembly's own location: the two differ for a
        // single-file publish, and it is the base directory the content files are copied beside.
        string? baseDirectory = AppContext.BaseDirectory;
        if (string.IsNullOrEmpty(baseDirectory)) return null;

        string candidate = Path.Combine(baseDirectory, FolderName);
        return System.IO.Directory.Exists(candidate) ? candidate : null;
    }
}
