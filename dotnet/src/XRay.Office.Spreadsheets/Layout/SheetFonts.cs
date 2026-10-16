using System.Collections.Concurrent;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Shaping;

namespace XRay.Office.Spreadsheets.Layout;

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

    /// <summary>
    /// Every face a workbook has asked for, keyed by everything that can change the answer.
    /// </summary>
    /// <remarks>
    /// The declared class is part of the key rather than a detail of the lookup, because it is part
    /// of the <em>question</em>: two cells naming the same absent family and declaring different
    /// shapes for it resolve to two different faces, and a key without it would hand the second
    /// whichever the first happened to load. Present in almost no workbook, so the extra dimension
    /// costs nothing in practice.
    /// </remarks>
    private static readonly
        ConcurrentDictionary<(string Family, int Weight, bool Italic, FontFamilyClass Declared),
                             SheetFace?> Cache = new();

    /// <summary>The face a format asks for, or null when no face could be read at all.</summary>
    /// <param name="format">The cell's resolved format.</param>
    public static SheetFace? For(SheetCellFormat format)
    {
        ArgumentNullException.ThrowIfNull(format);

        string family = string.IsNullOrWhiteSpace(format.FontFamily)
            ? DefaultFamily
            : format.FontFamily;

        return Cache.GetOrAdd(
            (family, format.FontWeight, format.IsItalic, format.DeclaredFontClass), Load);
    }

    /// <summary>The upright regular face of one family, or null when none could be read.</summary>
    /// <remarks>
    /// For the callers that have a family name and nothing else — a shape's text, whose runs
    /// carry a typeface but whose weight and slant this path does not model. It shares the cache
    /// with <see cref="For(SheetCellFormat)"/> rather than keeping its own, because a workbook
    /// whose text boxes are set in the same face as its cells should resolve it once.
    /// </remarks>
    /// <param name="family">The family name, or null for the default.</param>
    public static SheetFace? ForFamily(string? family) => ForFamily(family, bold: false);

    /// <summary>The upright face of one family at one of two weights.</summary>
    /// <remarks>
    /// The weight is a <c>bool</c> rather than a number because the callers that have one have
    /// only that: BIFF's <c>bls</c> is 400 or 700 in every file of the corpus, and a chart's
    /// model carries the answer as <see cref="XRay.Office.Core.Charts.ChartPlot.IsTitleBold"/>. The
    /// resolver underneath takes a full weight and the cache is keyed on it, so widening this
    /// later costs nothing.
    /// </remarks>
    /// <param name="family">The family name, or null for the default.</param>
    /// <param name="bold">Whether the family's bold face is wanted.</param>
    public static SheetFace? ForFamily(string? family, bool bold)
        => ForFamily(family, bold, italic: false);

    /// <summary>One family at one of two weights, upright or slanted.</summary>
    /// <remarks>
    /// The slant is here for a header or footer band, whose face is the workbook's own default
    /// cell font — and that font can be italic, which the reference honours: a workbook whose
    /// <c>fonts[0]</c> carries <c>&lt;i/&gt;</c> has its band drawn in <c>LiberationSans-Italic</c>
    /// on 26.2.4.2 (<c>probes/sheets-r56</c>, seven authored workbooks keyed on the PDF's own
    /// font list rather than on advance widths, because the Liberation faces are
    /// metric-compatible and a bold band is the same width as an upright one).
    /// </remarks>
    /// <param name="family">The family name, or null for the default.</param>
    /// <param name="bold">Whether the family's bold face is wanted.</param>
    /// <param name="italic">Whether its italic face is wanted.</param>
    public static SheetFace? ForFamily(string? family, bool bold, bool italic)
        => Cache.GetOrAdd(
            // Unknown class: a chart's font is named directly and carries no generic-family
            // declaration for a fallback to honour, unlike a cell's, which comes from a
            // SpreadsheetML <font> that may state <family val="N"/>.
            (string.IsNullOrWhiteSpace(family) ? DefaultFamily : family, bold ? 700 : 400, italic,
             FontFamilyClass.Unknown),
            Load);

    /// <summary>
    /// How much of a twip a digit width has to carry before it is taken as the next one up.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Fitted, with no mechanism behind it, and said so deliberately.</strong> LibreOffice
    /// reports a digit width as a whole number of twips off its reference device
    /// (<c>UnitConverter::finalizeImport</c> asks <c>XFont::getCharWidth</c>, which returns an
    /// integer), and the device's own quantisation decides the last one. No simple rule
    /// reproduces every case: swept over 205 points — five faces at every half point from 6 to
    /// 26 — truncating agrees with the installed 26.2.4.2 on 119, rounding half up on 194, and
    /// the fractional part alone cannot decide it, since the reference truncates a fraction as
    /// large as 0.521 and carries one as small as 0.440.
    /// </para>
    /// <para>
    /// <strong>The constant is chosen against the configurations the corpus actually uses, not
    /// against that sweep.</strong> Enumerating the default font of all 171 sheets documents
    /// gives seventeen distinct family/size pairs, and a one-cell probe rendered through the
    /// installed binary gives the reference's digit width for each. Written as "truncate unless
    /// the fraction exceeds <c>c</c>", they constrain it to <c>0.5039 &lt;= c &lt; 0.6406</c>:
    /// Carlito 11 pt is 111.5039 and must truncate to 111 (the default font of 65 documents),
    /// while Carlito 12 pt is 121.6406 and must carry to 122 (7 documents). This is the midpoint
    /// of that window, and it also scores 190 of 205 on the independent sweep above.
    /// </para>
    /// <para>
    /// Exact metric → what 26.2.4.2 writes: Liberation Sans 111.23 → 111, 122.35 → 122,
    /// 133.48 → 133; Carlito 111.50 → 111 and 121.64 → 122; Liberation Serif 100.00 → 100;
    /// Liberation Mono 120.02 → 120; DejaVu Sans 127.25 → 127, 139.97 → 140 and 152.70 → 153.
    /// All ten hold at this value.
    /// </para>
    /// <para>
    /// <strong>Rounding half up is the obvious alternative and is wrong here.</strong> It scores
    /// better on the uniform sweep, but it takes Carlito 11 pt to 112 — and Carlito 11 pt is the
    /// default font of 65 corpus documents against Carlito 12 pt's 7. It would break fifty-one
    /// passing documents to fix six.
    /// </para>
    /// <para>
    /// <strong>This constant was 0.67, and that was right for LibreOffice 24.2.7.2.</strong> The
    /// figure it was fitted to recorded Carlito 121.64 → <em>121</em>; the installed 26.2.4.2
    /// answers 122 for the same face at the same size, measured off a filled cell's rectangle in
    /// its own PDF. Ground truth moved, so the old window <c>(0.64, 0.70]</c> and the new one no
    /// longer overlap.
    /// </para>
    /// <para>
    /// <strong>Re-checked against LibreOffice 26.2.4.2 on 2026-08-21 and correct.</strong> Thirty
    /// authored workbooks, six faces (Calibri, Liberation Sans, Times New Roman, Courier New at
    /// 11 pt and Calibri and Liberation Sans at 12 pt) against five stated column widths (8.43,
    /// 10, 12.5, 20, 30), each measuring the x of a glyph in the *next* column — which is the
    /// first column's width, reported by <c>pdftotext -bbox</c> without decoding a content
    /// stream. **Thirty of thirty agree to within 0.001 pt.**
    /// <c>dotnet/probes/sheets-r53-totalsrow/audit_colwidth.py</c> reproduces it. This site and
    /// the digit-width model below it are no longer 24.2.7.2 claims.
    /// </para>
    /// <para>
    /// A one-twip column width is normally invisible, which is why truncation survived several
    /// rounds. It stops being invisible twice over. On a fit-to-page sheet
    /// <c>ScPrintFunc::CalcZoom</c> bisects on <em>integer</em> percentages, so a 0.7% error in
    /// the total print width moves the answer a whole percent and takes a page with it — that is
    /// <c>dragon-175066A.xlsx</c>, default font 宋体, resolved to DejaVu Sans here, whose zoom was
    /// 38 against LibreOffice's 37 (unaffected by this change: 152.70 carries at both constants).
    /// And it decides how many columns fit a page when the fit is close —
    /// <c>sectors-defense-and-aerospace.xlsx</c> is 40 columns wide in Calibri 12, where one twip
    /// per digit is 2 pt per column and the difference between two reference columns needing
    /// 488.07 pt of a 487.73 pt page and two of ours needing 484.0 pt. That is one column per
    /// page against two, and 227 pages against 449.
    /// </para>
    /// </remarks>
    private const double DigitWidthCarry = 0.57;

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
    /// <strong>Neither truncated nor rounded.</strong> The device's own quantisation decides the
    /// last twip and no single rule reproduces it, so this truncates unless the fraction carries
    /// past <see cref="DigitWidthCarry"/> — which is fitted rather than derived, and whose
    /// remarks hold both the nine measured faces and the corpus sweep that justifies it.
    /// </para>
    /// </remarks>
    /// <param name="font">The workbook's default font, or null for the application's own.</param>
    public static double DigitWidthTwips(SheetDefaultFont? font)
    {
        if (font is null) return SheetColumnDigits.FallbackDigitWidthTwips;

        SheetFace? face = Cache.GetOrAdd(
            (string.IsNullOrWhiteSpace(font.Family) ? DefaultFamily : font.Family,
             font.Weight, font.IsItalic, font.DeclaredClass),
            Load);

        if (face is null || font.Size <= Length.Zero)
            return SheetColumnDigits.FallbackDigitWidthTwips;

        double twips = face.Value.MaxDigitWidthAt(font.Size).Emu / (double)Length.EmuPerTwip;
        if (twips < 1) return SheetColumnDigits.FallbackDigitWidthTwips;

        double whole = Math.Truncate(twips);
        return twips - whole > DigitWidthCarry ? whole + 1 : whole;
    }

    private static SheetFace? Load(
        (string Family, int Weight, bool Italic, FontFamilyClass Declared) key)
    {
        try
        {
            lock (Gate)
            {
                SystemFontResolver resolver = Shared;

                // The declared family only, not the declared pitch — the same half of the
                // declaration the DOCX and DOC paths pass, and for the same reason: only roman and
                // swiss were measured to move LibreOffice's answer.
                FontReference reference = resolver.Resolve(
                    new FontRequest(
                        key.Family, key.Weight, key.Italic, DeclaredClass: key.Declared));
                OpenTypeFace face = resolver.LoadOpenType(reference);

                return new SheetFace(
                    face, reference, LineSpacing.Resolve(face, MetricGrid.Spreadsheet));
            }
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

    // ------------------------------------------------------------------------ glyph fallback

    private static readonly object Gate = new();
    private static SystemFontResolver? _shared;

    /// <summary>
    /// Fallback faces already dressed, keyed by the face <em>and</em> by the request that reached
    /// it.
    /// </summary>
    /// <remarks>
    /// The slant is part of the key and not an afterthought: one substituted face answers both an
    /// upright run and an italic one, and the two differ by
    /// <see cref="FontReference.SyntheticOblique"/>. Keyed on the face alone, whichever run arrived
    /// first would decide the lean for every other run in the workbook that fell back to the same
    /// face.
    /// </remarks>
    private static readonly ConcurrentDictionary<(OpenTypeFace Face, bool Italic), SheetFace?>
        FallbackFaces = new();

    /// <summary>
    /// The one resolver a workbook's faces are loaded through.
    /// </summary>
    /// <remarks>
    /// Shared rather than built per lookup because glyph fallback needs the <em>same</em> resolver
    /// that loaded the primary face: <see cref="SystemFontResolver.ReferenceFor"/> answers only for
    /// faces that resolver itself handed out, and a fallback face named without a reference reaches
    /// the PDF writer with no font program behind it — announced in the file and not embedded.
    /// </remarks>
    private static SystemFontResolver Shared => _shared ??= SystemFontResolver.Build();

    /// <summary>
    /// Where a cell's face sends the characters it has no glyph for.
    /// </summary>
    /// <remarks>
    /// A cell's face is chosen from the format's family name, and coverage is a property of a
    /// character — so a spreadsheet whose cells name a Latin face and hold Japanese asks a face
    /// with no CJK coverage to draw ideographs. Without this it draws its missing-glyph box at its
    /// own <c>.notdef</c> advance, which is both visibly wrong and, being far narrower than a
    /// full-width ideograph, breaks the cell's lines in the wrong places and reserves too short a
    /// row.
    /// </remarks>
    public static IGlyphFallbackResolver Fallback { get; } = new Locked();

    /// <summary>The shaper cell text is measured with: the default one, plus glyph fallback.</summary>
    public static ITextShaper Shaper { get; } = new FallbackShaper(TextShaper.Default, Fallback);

    /// <summary>
    /// A fallback face dressed as a <see cref="SheetFace"/>, or null when it cannot be named.
    /// </summary>
    /// <param name="face">A face <see cref="Fallback"/> returned.</param>
    /// <param name="isItalicRequested">
    /// Whether the cell's own run asked for italic. A substituted face has no memory of the
    /// request, so without this a cell of italic text holding a character its face cannot draw
    /// has that character drawn upright beside its leaning neighbours — measured on 26.2.4.2 in
    /// <c>probes/words-r58/fallback-oblique-ooxml.py</c>, whose `.xlsx` arm shears six of six.
    /// </param>
    public static SheetFace? ForFallback(OpenTypeFace face, bool isItalicRequested = false)
    {
        ArgumentNullException.ThrowIfNull(face);

        return FallbackFaces.GetOrAdd((face, isItalicRequested), static key =>
        {
            FontReference? reference = Fallback.ReferenceFor(key.Face, key.Italic);
            return reference is null
                ? null
                : new SheetFace(
                    key.Face, reference, LineSpacing.Resolve(key.Face, MetricGrid.Spreadsheet));
        });
    }

    /// <summary>
    /// The shared resolver behind a lock.
    /// </summary>
    /// <remarks>
    /// <see cref="SystemFontResolver"/> caches into plain dictionaries, and <see cref="Cache"/> is
    /// concurrent — so two threads asking for two faces at once would otherwise be mutating one
    /// resolver. The lock is uncontended in the single-threaded layout that actually runs; it is
    /// here so that the test host's parallel classes cannot corrupt it.
    /// </remarks>
    private sealed class Locked : IGlyphFallbackResolver
    {
        public OpenTypeFace? FallbackFor(int codePoint, int weight = 400, bool isItalic = false)
        {
            lock (Gate) return Shared.FallbackFor(codePoint, weight, isItalic);
        }

        // Forwarded for the same reason `SymbolFallbackFor` is: the interface's default drops the
        // primary face, and with it the generic whose fontconfig preference list ranks the answer.
        public OpenTypeFace? FallbackFor(int codePoint, int weight, bool isItalic, OpenTypeFace? primary)
        {
            lock (Gate) return Shared.FallbackFor(codePoint, weight, isItalic, primary);
        }

        // Forwarded rather than inherited: the interface's default sends this to FallbackFor, which
        // would silently give a cell set in a pi face the fontconfig answer LibreOffice refuses it.
        public OpenTypeFace? SymbolFallbackFor(int codePoint, int weight = 400, bool isItalic = false)
        {
            lock (Gate) return Shared.SymbolFallbackFor(codePoint, weight, isItalic);
        }

        public FontReference? ReferenceFor(OpenTypeFace face)
        {
            lock (Gate) return Shared.ReferenceFor(face);
        }
    }
}
