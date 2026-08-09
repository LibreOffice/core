using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// How tall the band a header or footer occupies actually prints, which is not the height the
/// file states.
/// </summary>
/// <remarks>
/// <para>
/// SpreadsheetML and BIFF both state a header band as two margins — <c>top</c> and
/// <c>header</c> — whose difference is the band. Calc does <em>not</em> keep that difference as
/// the printed band. It splits it into a measured text height and a distance, keeps the
/// distance, and re-measures the text when it prints:
/// </para>
/// <list type="number">
/// <item>
/// At import the band's text is measured crudely — the height of a line is the largest
/// <em>stated point size</em> on it, with no ascent, descent or leading. The OOXML filter does
/// this in <c>HeaderFooterParser::getCurrHeight</c>, which returns
/// <c>maFontModel.mfHeight</c> (<c>sc/source/filter/oox/pagesettings.cxx:738-741</c>); the BIFF
/// filter does the identical thing in <c>XclImpHFConverter::GetMaxLineHeight</c>
/// (<c>sc/source/filter/excel/xihelper.cxx:504-508</c>). Call that the <em>nominal</em> height.
/// </item>
/// <item>
/// The filter stores <c>bodyDistance = statedBand - nominal</c> and keeps the stated band as a
/// minimum (<c>pagesettings.cxx:1029-1040</c>, <c>xipage.cxx:311-330</c>). A negative distance
/// means the text does not fit, and the band is then pinned rather than dynamic.
/// </item>
/// <item>
/// At print time <c>ScPrintFunc::UpdateHFHeight</c> throws the nominal figure away and asks the
/// EditEngine for the real laid-out height, then adds the stored distance and floors the result
/// at the stated band: <c>nHeight = nMaxHeight + nDistance</c>, then
/// <c>if (nHeight &lt; nManHeight) nHeight = nManHeight</c>
/// (<c>sc/source/ui/view/printfun.cxx:817-849</c>).
/// </item>
/// </list>
/// <para>
/// Composing those three, the printed band is
/// <c>statedBand + max(0, measured - nominal)</c> — the stated band plus however much the real
/// text height exceeds the sum of the bare point sizes. It is never smaller than the stated band
/// and, for ordinary one-line furniture, about a tenth of the font size larger.
/// </para>
/// <para>
/// That difference is small and it is not negligible, because it comes off the printable body on
/// <em>every</em> page. Measured on <c>RegChangeReport.xlsx</c>, whose footer is one 10 pt line:
/// the workbook's margins give a body of 684.0 pt, and greedy pagination over LibreOffice's own
/// row heights reproduces its page breaks only for a body in <c>[681.62, 682.14)</c>. The band
/// rule accounts for the difference and nothing else measured does — the file's own numbers
/// reproduce LibreOffice's flat-ODF export exactly, <c>fo:min-height="0.45in"</c> against a
/// stated band of 0.45 in and <c>fo:margin-top="0.311in"</c> against
/// <c>0.45 in - 10 pt = 790</c> hundredths of a millimetre.
/// </para>
/// <para>
/// This is a port of the two filters and <c>UpdateHFHeight</c> rather than a rule of its own, so
/// it walks the <c>&amp;</c>-code string a second time instead of reusing
/// <see cref="SheetHeaderFooter.ParseCodes"/>. The two answer different questions: that one asks
/// what prints, and drops the size and face codes precisely because they do not; this one asks
/// how tall the codes make each line, and the literal text is what it can ignore.
/// </para>
/// </remarks>
internal static class SheetBandHeight
{
    /// <summary>
    /// The band a header or footer prints in, given the band the file states for it.
    /// </summary>
    /// <remarks>
    /// Returns <paramref name="statedBand"/> unchanged when the text is taller than the band
    /// allows: Calc marks such a band fixed rather than dynamic and prints it at the stated
    /// height, cropping the text (<c>#i23296</c>, cited at both filters).
    /// </remarks>
    /// <param name="codes">The header or footer string as the file wrote it.</param>
    /// <param name="statedBand">
    /// The band the file's two margins imply — <c>top - header</c>, or <c>bottom - footer</c>.
    /// </param>
    /// <param name="defaultFont">
    /// The workbook's own default cell font, which is what a run naming none is set in. Not a
    /// fixed ten-point face: <c>ScPrintFunc::MakeEditEngine</c> fills the band's defaults from
    /// <c>getDefaultCellAttribute</c> (<c>printfun.cxx:1769-1774</c>), and both filters' parsers
    /// start from the workbook's first font rather than a constant
    /// (<c>XclImpHFConverter::ResetFontData</c>, <c>xihelper.cxx:534-542</c>).
    /// <para>
    /// Measured on <c>NAARMO_Mexico_RVSM_Approvals.xlsx</c>, whose header states no size and
    /// whose default font is Calibri 11: LibreOffice's flat-ODF export gives the header
    /// <c>fo:min-height="0.45in"</c> and <c>fo:margin-bottom="0.2972in"</c>, a difference of
    /// <strong>11.0 pt</strong> and not 10. Taking the nominal height as ten there makes the
    /// band a point too tall and costs the workbook a page.
    /// </para>
    /// </param>
    public static Length Printed(
        string? codes, Length statedBand, SheetDefaultFont? defaultFont = null)
    {
        if (string.IsNullOrEmpty(codes) || statedBand <= Length.Zero) return statedBand;

        (Length nominal, Length measured) = Measure(codes, defaultFont ?? SheetDefaultFont.Calc);

        // A band whose text already overflows it is pinned, not dynamic, so the stated height is
        // what prints. Testing the nominal height is deliberate: it is the figure the *filter*
        // compared when it decided, and using the measured one here would make a band dynamic or
        // fixed on a different test from Calc's.
        if (nominal > statedBand) return statedBand;

        Length growth = measured - nominal;
        return growth > Length.Zero ? statedBand + growth : statedBand;
    }

    /// <summary>
    /// Walks the code string and totals both heights: the filters' nominal one and the laid-out
    /// one Calc re-measures at print time.
    /// </summary>
    /// <remarks>
    /// Both are a sum down the lines of one portion and a maximum across the three portions,
    /// which is what <c>XclImpHFConverter::GetTotalHeight</c> (<c>xihelper.cxx:496-500</c>) and
    /// <c>UpdateHFHeight</c>'s three-way <c>std::max</c> each do. An <em>empty</em> portion still
    /// contributes one line — the filters add <c>GetMaxLineHeight</c> for all three portions
    /// unconditionally (<c>xihelper.cxx:479-481</c>) and an empty EditEngine is one empty
    /// paragraph — so a band is never shorter than a single line of its own font.
    /// </remarks>
    private static (Length Nominal, Length Measured) Measure(
        string codes, SheetDefaultFont defaultFont)
    {
        // Left, centre, right. Text before any switch belongs to the centre, which is where
        // both filters' parsers start.
        Length[] nominal = [Length.Zero, Length.Zero, Length.Zero];
        Length[] measured = [Length.Zero, Length.Zero, Length.Zero];
        Length[] lineSize = [Length.Zero, Length.Zero, Length.Zero];
        Length[] lineHeight = [Length.Zero, Length.Zero, Length.Zero];

        int part = 1;
        Length size = defaultFont.Size;
        string? family = defaultFont.Family;

        int at = 0;
        while (at < codes.Length)
        {
            char c = codes[at];
            if (c != '&' || at + 1 >= codes.Length)
            {
                if (c == '\n')
                {
                    // A line break banks the line and starts a fresh one, taking nothing with it
                    // (`InsertLineBreak`, xihelper.cxx:565-573).
                    Bank(part);
                }
                else
                {
                    Text(part);
                }

                at++;
                continue;
            }

            char code = codes[at + 1];
            at += 2;

            switch (code)
            {
                // A section switch resets the font to the workbook's own default —
                // `ResetFontData` (xihelper.cxx:534-542), `setNewPortion` (pagesettings.cxx:868).
                case 'L': part = 0; size = defaultFont.Size; family = defaultFont.Family; break;
                case 'C': part = 1; size = defaultFont.Size; family = defaultFont.Family; break;
                case 'R': part = 2; size = defaultFont.Size; family = defaultFont.Family; break;

                // The fields. Each is inserted as one character and counts as text, because
                // `InsertField` calls `UpdateCurrMaxLineHeight` exactly as `InsertText` does
                // (xihelper.cxx:557-563).
                case 'P' or 'N' or 'D' or 'T' or 'A' or 'F' or 'Z':
                    Text(part);
                    break;

                case '&': Text(part); break;
                case '\n': Bank(part); break;

                // &"Family,Style" — the family is everything up to the first comma or the closing
                // quotation mark. A leading "-" means "keep the current face", which is what
                // Excel writes when it states only a style.
                case '"':
                {
                    int end = codes.IndexOf('"', at);
                    string spec = end < 0 ? codes[at..] : codes[at..end];
                    at = end < 0 ? codes.Length : end + 1;

                    int comma = spec.IndexOf(',');
                    string named = (comma < 0 ? spec : spec[..comma]).Trim();
                    if (named.Length > 0 && named != "-") family = named;
                    break;
                }

                // A font size: a run of digits, in points.
                case >= '0' and <= '9':
                {
                    int start = at - 1;
                    while (at < codes.Length && char.IsAsciiDigit(codes[at])) at++;
                    if (int.TryParse(codes[start..at], out int points) && points > 0)
                        size = Length.FromPoints(points);
                    break;
                }

                // &K is a colour and takes six hex digits with it; every other code is a toggle
                // or one this does not know, and neither changes a height.
                case 'K':
                {
                    int taken = 0;
                    while (taken < 6 && at < codes.Length && Uri.IsHexDigit(codes[at]))
                    {
                        at++;
                        taken++;
                    }

                    break;
                }

                default: break;
            }
        }

        Length totalNominal = Length.Zero;
        Length totalMeasured = Length.Zero;
        for (int i = 0; i < 3; i++)
        {
            Bank(i);
            if (nominal[i] > totalNominal) totalNominal = nominal[i];
            if (measured[i] > totalMeasured) totalMeasured = measured[i];
        }

        return (totalNominal, totalMeasured);

        // Text on a line raises that line's two heights to the run's, which is what
        // `UpdateMaxLineHeight` does with the font in effect at the moment the run is inserted.
        void Text(int which)
        {
            if (size > lineSize[which]) lineSize[which] = size;

            Length height = SheetBandText.LineHeightAt(size, family);
            if (height > lineHeight[which]) lineHeight[which] = height;
        }

        // Bank the current line. A line with no text on it still stands at the height of the
        // font in effect, which is `GetMaxLineHeight`'s fallback to `mxFontData->mnHeight`.
        void Bank(int which)
        {
            nominal[which] += lineSize[which] > Length.Zero ? lineSize[which] : size;
            measured[which] += lineHeight[which] > Length.Zero
                ? lineHeight[which]
                : SheetBandText.LineHeightAt(size, family);

            lineSize[which] = Length.Zero;
            lineHeight[which] = Length.Zero;
        }
    }
}
