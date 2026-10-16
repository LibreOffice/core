namespace XRay.Office.TestKit.LibreOffice;

/// <summary>
/// The resolution a font size survives at through a LibreOffice PDF.
/// </summary>
/// <remarks>
/// <para>
/// A <c>Tf</c> operator looks like the one number in a comparison that needs no tolerance, and it is
/// not: <b>26.2.4.2 prints every font size at a whole tenth of a point.</b> Measured on one rendering
/// holding an automatic superscript at each of 33 base sizes — every <c>Tf</c> is a whole tenth, and at
/// 21 of the 33 it is not the size the layout used, an eleven point superscript printing as 6.4 pt
/// where the layout set 127 twips, or 6.35. <c>probes/escsize-r153/results.md</c> §1.
/// </para>
/// <para>
/// So a size read out of a reference PDF is the writer's rounding of the layout's answer, and the way
/// to compare ours against it is to round ours the same way rather than to loosen a tolerance —
/// which would accept a size wrong by half a tenth in either direction. This tree's own writer prints
/// the size it laid out with, and is deliberately not changed to match: making our output coarser to
/// make a comparison exact is the trade `dotnet/CLAUDE.md` refuses elsewhere in this same family.
/// </para>
/// </remarks>
public static class PdfFontSizes
{
    /// <summary>The size as 26.2.4.2's PDF writer would print it: the nearest tenth of a point.</summary>
    /// <remarks>
    /// Rounded through the twip count rather than in floating point, where 6.35 reads back as
    /// 6.34999… and rounds <em>down</em>. Getting that wrong inverted the verdict on 81 of 108
    /// documents in the round that established the rule, so it is worth the two lines.
    /// </remarks>
    public static double AsLibreOfficePrintsIt(double points)
    {
        long twips = (long)Math.Floor(points * 20 + 0.5);
        return (twips / 2 + twips % 2) / 10.0;
    }
}
