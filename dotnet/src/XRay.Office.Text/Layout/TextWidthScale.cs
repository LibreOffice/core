using XRay.Office.Core.Units;

namespace XRay.Office.Text.Layout;

/// <summary>
/// How far a run's glyphs are squeezed across when a format states a character width.
/// </summary>
/// <remarks>
/// <para>
/// <c>w:rPr/w:w</c> in WordprocessingML and <c>\charscalex</c> in RTF, both of which VCL applies by
/// setting the font's width away from its height — <c>Font::SetAverageFontWidth</c>, whose value is a
/// <c>tools::Long</c> in the map mode's own unit. Writer's map mode is twips, so the width the face is
/// actually built at is an integer number of them and the effective scale is that integer over the
/// height rather than the percentage itself.
/// </para>
/// <para>
/// It shows on the corpus's commonest value and nowhere else. 12 pt is 240 twips; 99 per cent of it is
/// 237.6, truncated to 237, so the run is drawn at <b>0.98750</b> and not 0.99. Measured against
/// 24.2.7.2 in <c>dotnet/probes/words-character-scale/</c>: <c>Hamburgefonstiv 12345</c> at 12 pt is
/// 83.928 pt unscaled and 82.879 pt at <c>w:w="99"</c>, a ratio of 0.98750 to five places, while 95,
/// 90, 50, 150 and 200 per cent each come out at exactly their own figure because each divides 240.
/// Of the corpus's 1440 scaled runs, <b>1226 say 99</b>.
/// </para>
/// </remarks>
public static class TextWidthScale
{
    /// <summary>A percentage that means "as the face is drawn".</summary>
    public const int Natural = 100;

    /// <summary>
    /// The factor a run's advances are multiplied by.
    /// </summary>
    /// <param name="emSize">The run's em size, whose twips are the grid the width lands on.</param>
    /// <param name="perCent">The stated percentage, 100 for none.</param>
    /// <returns>One when the run is unscaled, so the ordinary path multiplies by nothing.</returns>
    public static double Of(Length emSize, int perCent)
    {
        if (perCent == Natural || perCent <= 0) return 1.0;

        long height = emSize.Twips;
        if (height <= 0) return perCent / 100.0;

        long width = height * perCent / 100;

        return width <= 0 ? 1.0 : (double)width / height;
    }
}
