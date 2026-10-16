using XRay.Office.Text.Fonts;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// The generic family a workbook declares beside a font's name, in each format's own spelling.
/// </summary>
/// <remarks>
/// <para>
/// One place rather than three, because the three spreadsheet readers state the same fact and
/// LibreOffice funnels them into one enumeration too: SpreadsheetML's <c>&lt;family val="N"/&gt;</c>
/// and BIFF's <c>FONT</c> family byte are both read into <c>Font::maModel.mnFamily</c> against the
/// <c>OOX_FONTFAMILY_*</c> constants (<c>sc/source/filter/oox/stylesbuffer.cxx:110-116</c>, the
/// attribute at <c>:616</c> and the byte at <c>:672</c>), and ODF states the same five as words.
/// </para>
/// <para>
/// <strong>Only roman and swiss are carried across.</strong> That is the rule the word-processing
/// side arrived at by probing 26.2.4.2 with the family name held constant, and nothing about a
/// spreadsheet changes it: modern, script and decorative each leave the fallback exactly where an
/// undeclared request left it, so mapping modern onto a monospaced face — which its name invites —
/// would invent a divergence rather than remove one.
/// </para>
/// </remarks>
internal static class SheetDeclaredFonts
{
    /// <summary>The shape a Windows <c>FF_*</c> family code names.</summary>
    /// <remarks>
    /// 0 dontcare, 1 roman, 2 swiss, 3 modern, 4 script, 5 decorative. Shared by SpreadsheetML,
    /// XLSB and BIFF, and by the DOC <c>FFN</c> on the word-processing side.
    /// </remarks>
    /// <param name="code">The code as the file states it.</param>
    public static FontFamilyClass FromWindowsCode(int code) => code switch
    {
        1 => FontFamilyClass.Serif,
        2 => FontFamilyClass.SansSerif,
        _ => FontFamilyClass.Unknown,
    };

    /// <summary>The shape an ODF <c>style:font-family-generic</c> names.</summary>
    /// <remarks>
    /// ODF spells the same five as words — <c>roman</c>, <c>swiss</c>, <c>modern</c>,
    /// <c>decorative</c>, <c>script</c> — plus <c>system</c>, which names the application's own UI
    /// face rather than a shape. Compared case-insensitively because the attribute is an enumeration
    /// and real files still shout it.
    /// </remarks>
    /// <param name="generic">The attribute's value, or null when the face declares none.</param>
    public static FontFamilyClass FromOdfGeneric(string? generic)
        => generic is null
            ? FontFamilyClass.Unknown
            : generic.Equals("roman", StringComparison.OrdinalIgnoreCase)
                ? FontFamilyClass.Serif
                : generic.Equals("swiss", StringComparison.OrdinalIgnoreCase)
                    ? FontFamilyClass.SansSerif
                    : FontFamilyClass.Unknown;
}
