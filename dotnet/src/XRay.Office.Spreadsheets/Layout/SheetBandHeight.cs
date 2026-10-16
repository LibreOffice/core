using XRay.Office.Core.Units;

namespace XRay.Office.Spreadsheets.Layout;

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
        => Printed(codes, statedBand, defaultFont, out _);

    /// <inheritdoc cref="Printed(string?, Length, SheetDefaultFont?)"/>
    /// <param name="codes">The header or footer string as the file wrote it.</param>
    /// <param name="statedBand">
    /// The band the file's two margins imply — <c>top - header</c>, or <c>bottom - footer</c>.
    /// </param>
    /// <param name="defaultFont">The workbook's own default cell font.</param>
    /// <param name="isDynamic">
    /// Whether Calc leaves the band <em>dynamic</em>, which is the branch both filters take when
    /// the text fits inside the stated band (<c>xipage.cxx:315-331</c>,
    /// <c>pagesettings.cxx:1032</c>). It is worth reporting because it decides whether the
    /// band has a minimum height at all: <c>UpdateHFHeight</c> returns before it reaches
    /// <c>nManHeight</c> for a band that is not dynamic (<c>printfun.cxx:793</c>), so a pinned
    /// band prints at exactly the stated height however short that is. The BIFF filter is the
    /// only one where this shows, because it is the only one that leaves a <em>page style
    /// default</em> in <c>ATTR_PAGE_SIZE</c> for the dynamic case — see
    /// <c>XlsPrintSetup.Band</c>.
    /// </param>
    public static Length Printed(
        string? codes, Length statedBand, SheetDefaultFont? defaultFont, out bool isDynamic)
    {
        isDynamic = true;
        if (string.IsNullOrEmpty(codes) || statedBand <= Length.Zero)
        {
            // A band the margins leave no room for is the #i23296 case at its extreme: the
            // filter pins it at nothing rather than making it dynamic.
            isDynamic = statedBand > Length.Zero;
            return statedBand;
        }

        (Length nominal, Length measured) = Measure(codes, defaultFont ?? SheetDefaultFont.Calc);

        // A band whose text already overflows it is pinned, not dynamic, so the stated height is
        // what prints. Testing the nominal height is deliberate: it is the figure the *filter*
        // compared when it decided, and using the measured one here would make a band dynamic or
        // fixed on a different test from Calc's.
        if (nominal > statedBand)
        {
            isDynamic = false;
            return statedBand;
        }

        Length growth = measured - nominal;
        return growth > Length.Zero ? statedBand + growth : statedBand;
    }

    /// <summary>
    /// The distance a band keeps between its text and the sheet — Calc's <c>nDistance</c>, which
    /// is a part of the band rather than an addition to it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Zero on a pinned band</strong>, and that is a port rather than a simplification.
    /// Both filters write the distance out as nothing when the band is already too short for its
    /// own text — <c>orHFData.mnBodyDist = max(mnBodyDist, 0)</c>
    /// (<c>sc/source/filter/oox/pagesettings.cxx:1040</c>) and
    /// <c>lclPutMarginItem(rHdrItemSet, EXC_ID_BOTTOMMARGIN, 0.0)</c>
    /// (<c>sc/source/filter/excel/xipage.cxx:322</c>) — because there is nothing left to give
    /// away.
    /// </para>
    /// <para>
    /// It matters because <see cref="SheetPrintSetup.FooterGap"/> is what separates a band's top
    /// edge from its text, and <c>SheetPageDecoration.DrawBand</c> lays the text into
    /// <c>bandHeight - gap</c>. <strong>XLSX and XLSB used to set no gap at all</strong> and so
    /// inherited <see cref="SheetPrintSetup"/>'s ODF default of 142 twips, which made that
    /// difference negative for every band under 7.1 pt and dropped the band outright — no ink and
    /// no words. Measured on six authored margin variants of
    /// <c>020_Free_Blood_Pressure_Chart…xlsx</c>, rendered both ways: at a stated band of 3.6 pt
    /// LibreOffice draws the footer and we drew nothing; at 7.2 pt and above both draw. The
    /// threshold was exactly 142 twips, which is the tell that the constant and not the geometry
    /// was the cause.
    /// </para>
    /// </remarks>
    /// <param name="codes">The band's own <c>&amp;</c>-code string.</param>
    /// <param name="statedBand">The band the file's two margins imply.</param>
    /// <param name="defaultFont">The workbook's own default cell font.</param>
    /// <param name="dynamicGap">The gap a band whose text fits inside it keeps.</param>
    public static Length BodyDistance(
        string? codes, Length statedBand, SheetDefaultFont? defaultFont, Length dynamicGap)
    {
        Printed(codes, statedBand, defaultFont, out bool isDynamic);
        return isDynamic ? dynamicGap : Length.Zero;
    }

    /// <summary>
    /// The band a header or footer prints in when the file states Calc's two terms directly,
    /// which is what ODF does: <c>max(minHeight, textHeight + distance)</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is <c>ScPrintFunc::UpdateHFHeight</c> with nothing composed away, because ODF needs
    /// nothing composed. <c>rParam.nHeight = nMaxHeight + rParam.nDistance</c> and then
    /// <c>if (rParam.nHeight &lt; rParam.nManHeight) rParam.nHeight = rParam.nManHeight</c>
    /// (<c>sc/source/ui/view/printfun.cxx:838</c> and <c>:848-849</c>). A page layout's
    /// <c>fo:min-height</c> is <c>nManHeight</c> — it reaches <c>ATTR_PAGE_SIZE</c>, which
    /// <c>lcl_FillHFParam</c> copies into <c>nManHeight</c> (<c>printfun.cxx:666</c>,
    /// <c>:683</c>) — and the header's own <c>fo:margin-bottom</c> (the footer's
    /// <c>fo:margin-top</c>) is <c>nDistance</c>, read from <c>ATTR_ULSPACE</c> at
    /// <c>printfun.cxx:898</c> and <c>:913</c>.
    /// </para>
    /// <para>
    /// <strong><c>OdsPrintSetup</c> read the declared height alone</strong>, on the
    /// reasoning that the gap is already inside it. That is true only while <c>text + gap</c>
    /// stays under the declared height, and an <c>.ods</c> converted from a workbook carries the
    /// workbook's own header margin as the gap, so the pair is routinely a 21.26 pt band with a
    /// 25.99 pt gap — a gap larger than the whole band, which 13 of the 307 converted
    /// <c>.ods</c> declare outright. Measured against 26.2.4.2 over 51 authored probes
    /// (<c>probes/ods-band-r75/</c>): six gaps from 0 to 2 cm move the first printed row point
    /// for point once the sum passes the declared height and not at all before it, and seven
    /// declared heights move it point for point once they pass the sum.
    /// </para>
    /// <para>
    /// <strong>The text term has a floor, and the floor is one line of the workbook's own
    /// default cell font.</strong> Calc measures all three areas of all three page variants and
    /// takes the greatest (<c>printfun.cxx:817-836</c>), and an area holding nothing is still an
    /// <c>EditTextObject</c> of one empty paragraph rather than a null pointer — <c>TextHeight</c>
    /// returns zero only for a null one (<c>printfun.cxx:777-785</c>) — so a header whose single
    /// line is smaller than a plain cell of that workbook is sized by the empty areas beside it.
    /// Measured on sixteen probes crossing four default sizes with four header sizes: a 6 pt
    /// header in a 20 pt workbook takes the 20 pt line, in a 14 pt workbook the 14 pt line, and
    /// in a 6 pt workbook its own; and two 6 pt lines in a 20 pt workbook still take one 20 pt
    /// line, which is what says the floor is the band's rather than each line's.
    /// </para>
    /// <para>
    /// The distance is <em>not</em> added when the minimum wins, and the difference shows in
    /// where the text sits rather than in the band: the text is centred in
    /// <c>nHeight - nDistance</c> whichever term won (<c>PrintHF</c>,
    /// <c>printfun.cxx:1876-1912</c>), which the seven <c>min_*</c> probes reproduce to 0.01 pt.
    /// </para>
    /// </remarks>
    /// <param name="minHeight">The declared height, Calc's <c>nManHeight</c>.</param>
    /// <param name="distance">The gap between the band's text and the sheet, <c>nDistance</c>.</param>
    /// <param name="defaultFont">
    /// The workbook's own default cell font, which is what an area naming none is set in and what
    /// an empty area is measured in.
    /// </param>
    /// <param name="bands">
    /// The band as each page variant states it — the shared one, and the left- and first-page
    /// ones where the master distinguishes them. Calc maximises over all of them.
    /// </param>
    public static Length Dynamic(
        Length minHeight,
        Length distance,
        SheetDefaultFont? defaultFont,
        params SheetHeaderFooter?[] bands)
    {
        ArgumentNullException.ThrowIfNull(bands);

        SheetDefaultFont font = defaultFont ?? SheetDefaultFont.Calc;

        Length text = Length.Zero;
        foreach (SheetHeaderFooter? band in bands)
        {
            if (band is not null) text = Length.Max(text, TextHeight(band, font));
        }

        return text > Length.Zero ? Length.Max(minHeight, text + distance) : minHeight;
    }

    /// <summary>
    /// How tall a band's text lays out: the tallest of its three areas, an empty area counting
    /// as one line of the default font.
    /// </summary>
    /// <remarks>
    /// The three areas share one rectangle and are each drawn into it separately, so the band
    /// is the greatest of the three and never their sum — <c>UpdateHFHeight</c>'s nine-way
    /// <c>std::max</c> (<c>sc/source/ui/view/printfun.cxx:817-836</c>). Nothing wraps here, for
    /// the same reason <see cref="Measure"/> does not: Calc gives the EditEngine the band's own
    /// paper width and a real header is one short line per area.
    /// </remarks>
    /// <param name="band">The three areas and their segments.</param>
    /// <param name="defaultFont">The workbook's default cell font.</param>
    public static Length TextHeight(SheetHeaderFooter band, SheetDefaultFont? defaultFont)
    {
        ArgumentNullException.ThrowIfNull(band);

        SheetDefaultFont font = defaultFont ?? SheetDefaultFont.Calc;
        SheetHeaderContext context = new();

        Length height = Length.Zero;
        foreach (SheetHeaderPart part in (SheetHeaderPart[])[band.Left, band.Centre, band.Right])
        {
            // An empty area is one empty paragraph, not nothing: `LineHeight` answers the
            // default font's line for a line carrying no pieces, which is exactly that.
            Length own = part.IsEmpty
                ? LineHeight([], 1.0, font)
                : PartHeight(part, context, 1.0, font);

            height = Length.Max(height, own);
        }

        return height;
    }

    /// <summary>How tall one part of a band is: the sum of its lines.</summary>
    /// <param name="part">The area.</param>
    /// <param name="context">What the fields resolve to on this page.</param>
    /// <param name="zoom">The print scale the band is drawn at.</param>
    /// <param name="bandFont">The face a piece naming none is drawn in.</param>
    public static Length PartHeight(
        SheetHeaderPart part, SheetHeaderContext context, double zoom, SheetDefaultFont bandFont)
    {
        ArgumentNullException.ThrowIfNull(part);
        if (part.IsEmpty) return Length.Zero;

        Length height = Length.Zero;
        foreach (IReadOnlyList<SheetHeaderPiece> line in part.Lines(context))
            height += LineHeight(line, zoom, bandFont);

        return height;
    }

    /// <summary>How tall one line of a band is: the tallest of the pieces on it.</summary>
    /// <remarks>
    /// An empty line — a bare break, which a footer written as <c>&amp;RPage &amp;P\n\nrest</c>
    /// contains — still takes a line, at the band font's own height. So does an area holding
    /// nothing at all, which is what makes this the floor
    /// <see cref="TextHeight(SheetHeaderFooter, SheetDefaultFont?)"/> uses.
    /// </remarks>
    /// <param name="line">The pieces on the line.</param>
    /// <param name="zoom">The print scale the band is drawn at.</param>
    /// <param name="bandFont">The face a piece naming none is drawn in.</param>
    public static Length LineHeight(
        IReadOnlyList<SheetHeaderPiece> line, double zoom, SheetDefaultFont bandFont)
    {
        ArgumentNullException.ThrowIfNull(line);
        ArgumentNullException.ThrowIfNull(bandFont);

        Length height = Length.Zero;
        foreach (SheetHeaderPiece piece in line)
        {
            SheetBandFace face = FaceOf(piece, bandFont);
            height = Length.Max(
                height,
                SheetBandText.LineHeightAt(
                    SizeOf(piece, zoom, bandFont), face.Family, face.Bold, face.Italic));
        }

        return height > Length.Zero
            ? height
            : SheetBandText.LineHeightAt(
                bandFont.Size * zoom,
                bandFont.Family,
                bandFont.Weight >= BoldWeight,
                bandFont.IsItalic);
    }

    /// <summary>The em size one piece of a band is drawn at, the page's zoom applied.</summary>
    /// <remarks>
    /// The fallback is the <em>workbook's</em> default cell font and not a fixed ten point — see
    /// <see cref="SheetPrintSetup.BandFont"/>, which carries the measurement.
    /// </remarks>
    /// <param name="piece">The piece.</param>
    /// <param name="zoom">The print scale the band is drawn at.</param>
    /// <param name="bandFont">The face a piece naming none is drawn in.</param>
    public static Length SizeOf(SheetHeaderPiece piece, double zoom, SheetDefaultFont bandFont)
    {
        ArgumentNullException.ThrowIfNull(bandFont);
        return (piece.Size ?? bandFont.Size) * zoom;
    }

    /// <summary>The family one piece of a band is drawn in.</summary>
    /// <remarks>
    /// The piece's own <c>&amp;"Family,Style"</c> if it states one, and the workbook's default
    /// cell family otherwise. A null answer means the furniture's own face, which is what
    /// <see cref="SheetBandText"/> resolves for a workbook that names nothing.
    /// </remarks>
    /// <param name="piece">The piece.</param>
    /// <param name="bandFont">The face a piece naming none is drawn in.</param>
    public static SheetBandFace FaceOf(SheetHeaderPiece piece, SheetDefaultFont bandFont)
    {
        ArgumentNullException.ThrowIfNull(bandFont);
        return new SheetBandFace(
            piece.Family ?? bandFont.Family,
            piece.Bold ?? bandFont.Weight >= BoldWeight,
            piece.Italic ?? bandFont.IsItalic);
    }

    /// <summary>The weight at which a workbook's default font makes its band bold.</summary>
    /// <remarks>
    /// Six hundred, the CSS threshold, and it never has to discriminate on this corpus: both
    /// Excel readers write 400 or 700 and nothing between.
    /// </remarks>
    private const int BoldWeight = 600;

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

                // &K is a colour and takes the six characters after it, hex or not — see the
                // twin in `SheetHeaderFooter.ParseCodes` for why the distinction matters. Every
                // other code is a toggle or one this does not know, and neither changes a height.
                case 'K':
                    at = Math.Min(codes.Length, at + 6);
                    break;

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
