using System.Collections.Concurrent;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// A face a cell's text is set in, with the metrics laying it out needs.
/// </summary>
/// <param name="Face">The face itself, for shaping and for advance widths.</param>
/// <param name="Reference">
/// How a backend names it. Carries the resolver's own <c>FaceKey</c> — a file path — rather than
/// the family name, which is what lets a PDF embed the face and advance the pen by the font's own
/// widths. See the remark on <see cref="SheetFonts"/>.
/// </param>
/// <param name="Metrics">Its line metrics, resolved by the shared precedence rules.</param>
internal readonly record struct SheetFace(
    OpenTypeFace Face,
    FontReference Reference,
    LineMetrics Metrics)
{
    /// <summary>The distance from the baseline to the top of the text, at a size.</summary>
    public Length AscentAt(Length size) => Metrics.ScaledAscent(size);

    /// <summary>The distance from the baseline to the bottom of the text, at a size.</summary>
    public Length DescentAt(Length size) => Metrics.ScaledDescent(size);

    /// <summary>
    /// How tall Calc considers one line of this face, at a size.
    /// </summary>
    /// <remarks>
    /// Ascent plus descent, with no line gap: Calc builds the text size from the font metric
    /// alone — <c>aTextSize.setHeight(aMetric.GetAscent() + aMetric.GetDescent())</c>,
    /// <c>sc/source/ui/view/output2.cxx:734</c> — where Writer adds the external leading. That is
    /// the difference between a wrapped cell's second line sitting 11.17 pt below its first and
    /// 11.50 pt below it in ten-point Liberation Sans, and it is why a cell cannot simply borrow
    /// the word processor's line height.
    /// </remarks>
    public Length LineHeightAt(Length size) => AscentAt(size) + DescentAt(size);

    /// <summary>
    /// The advance of the widest digit, which is what a column's capacity is counted in.
    /// </summary>
    /// <remarks>
    /// <c>ScDrawStringsVars::GetMaxDigitWidth</c>: the <c>General</c> format's decision to fall
    /// back to scientific notation, and the number of characters it is allowed, are both a count
    /// of digit widths rather than a measurement of the text
    /// (<c>SetTextToWidthOrHash</c>, <c>output2.cxx:645</c>).
    /// </remarks>
    public Length MaxDigitWidthAt(Length size)
    {
        int widest = 0;
        for (int digit = '0'; digit <= '9'; digit++)
        {
            int advance = Face.AdvanceForCharacter(digit);
            if (advance > widest) widest = advance;
        }

        int upem = Face.UnitsPerEm > 0 ? Face.UnitsPerEm : 1000;
        return size * ((double)widest / upem);
    }
}

/// <summary>
/// Resolves the faces a sheet's cells ask for, once each.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The reference must carry the resolver's face key, not the family name.</strong> The
/// key a <see cref="SystemFontResolver"/> produces is the font <em>file's</em> path, and the PDF
/// backend uses it to load and embed the face. A reference built by hand from
/// <c>face.FamilyName</c> loads nothing, so the backend has no <c>/Widths</c> to advance the pen
/// with and corrects every glyph with an explicit adjustment instead — a <c>TJ</c> array with
/// roughly -700 thousandths of an em between each pair of glyphs. The output looks right and
/// extracts as loose characters: <c>pdftotext</c> reads an adjustment that large as a word break,
/// so a fourteen-page workbook came out as 13 255 one-character "words" against LibreOffice's
/// 2 281 real ones. That is not a rendering bug at all — it is a searchability bug, and it is the
/// reason this type exists rather than a lazily-loaded single face.
/// </para>
/// <para>
/// Cached on the family, weight and posture together, because that triple is what the resolver
/// takes and a sheet asks for the same handful of them thousands of times.
/// </para>
/// </remarks>
internal static class SheetFonts
{
    /// <summary>
    /// The family a cell that names none is set in.
    /// </summary>
    /// <remarks>
    /// <c>DefaultFontType::LATIN_SPREADSHEET</c> resolves to Liberation Sans on Linux, so it is
    /// the face every reference rendering of a document that states no font is measured in.
    /// </remarks>
    public const string DefaultFamily = "Liberation Sans";

    private static readonly ConcurrentDictionary<(string Family, int Weight, bool Italic), SheetFace?>
        Cache = new();

    /// <summary>The face a format asks for, or null when no face could be read at all.</summary>
    /// <param name="format">The cell's resolved format.</param>
    public static SheetFace? For(SheetCellFormat format)
    {
        ArgumentNullException.ThrowIfNull(format);

        string family = string.IsNullOrWhiteSpace(format.FontFamily)
            ? DefaultFamily
            : format.FontFamily;

        return Cache.GetOrAdd((family, format.FontWeight, format.IsItalic), Load);
    }

    /// <summary>The upright regular face of one family, or null when none could be read.</summary>
    /// <remarks>
    /// For the callers that have a family name and nothing else — a shape's text, whose runs
    /// carry a typeface but whose weight and slant this path does not model. It shares the cache
    /// with <see cref="For(SheetCellFormat)"/> rather than keeping its own, because a workbook
    /// whose text boxes are set in the same face as its cells should resolve it once.
    /// </remarks>
    /// <param name="family">The family name, or null for the default.</param>
    public static SheetFace? ForFamily(string? family)
        => Cache.GetOrAdd(
            (string.IsNullOrWhiteSpace(family) ? DefaultFamily : family, 400, false), Load);

    /// <summary>
    /// What one digit of a workbook's default font is worth, in twips.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The measurement a SpreadsheetML or BIFF column width is stated in multiples of, and the
    /// one thing about a spreadsheet's geometry that cannot be read out of the file. LibreOffice
    /// takes the widest of <c>'0'</c>–<c>'9'</c> from its reference device in whole units —
    /// <c>UnitConverter::finalizeImport</c> (<c>sc/source/filter/oox/unitconverter.cxx:113</c>)
    /// for OOXML and <c>XclRoot::SetCharWidth</c> (<c>xlroot.cxx:210</c>) for BIFF, which share a
    /// comment saying so — and this is that, measured from the face's own <c>hmtx</c> instead of
    /// from a device.
    /// </para>
    /// <para>
    /// <strong>Truncated, not rounded, and the difference is measured rather than assumed.</strong>
    /// Round-tripping one-column probe workbooks through LibreOffice 24.2.7.2 and reading the
    /// <c>style:column-width</c> it wrote gives, against the exact metric: Liberation Sans 10pt
    /// 111.23 → 111, 11pt 122.35 → 122, 12pt 133.48 → 133; Carlito 11pt 111.50 → <em>111</em>,
    /// 12pt 121.64 → <em>121</em>; Liberation Serif 10pt 100.00 → 100; Liberation Mono 10pt
    /// 120.02 → 120; DejaVu Sans 11pt 139.97 → <em>140</em>, 12pt 152.70 → <em>153</em>. Eight of
    /// the ten are exact either way. The two that are not disagree, so a device's quantisation is
    /// what decides them rather than a rounding rule, and truncation is the half that matters:
    /// Carlito at eleven points is what Excel's own default font resolves to here, and it is the
    /// default of 65 of the 171 corpus spreadsheets — where rounding would put every column 0.9%
    /// too wide, all of them currently correct. The DejaVu substitutions truncation costs a twip
    /// on are thirteen documents, and a twip is a 1440th of an inch.
    /// </para>
    /// </remarks>
    /// <param name="font">The workbook's default font, or null for the application's own.</param>
    public static double DigitWidthTwips(SheetDefaultFont? font)
    {
        if (font is null) return SheetColumnDigits.FallbackDigitWidthTwips;

        SheetFace? face = Cache.GetOrAdd(
            (string.IsNullOrWhiteSpace(font.Family) ? DefaultFamily : font.Family,
             font.Weight, font.IsItalic),
            Load);

        if (face is null || font.Size <= Length.Zero)
            return SheetColumnDigits.FallbackDigitWidthTwips;

        double twips = face.Value.MaxDigitWidthAt(font.Size).Emu / (double)Length.EmuPerTwip;
        return twips >= 1 ? Math.Truncate(twips) : SheetColumnDigits.FallbackDigitWidthTwips;
    }

    private static SheetFace? Load((string Family, int Weight, bool Italic) key)
    {
        try
        {
            SystemFontResolver resolver = SystemFontResolver.Build();
            FontReference reference = resolver.Resolve(
                new FontRequest(key.Family, key.Weight, key.Italic));
            OpenTypeFace face = resolver.LoadOpenType(reference);

            return new SheetFace(face, reference, LineSpacing.Resolve(face));
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // No readable face is not a reason to fail a layout — the pages, their count and
            // their geometry are all already decided, and only the ink is missing.
            return null;
        }
    }
}
