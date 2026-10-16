using XRay.Office.Core.Units;

namespace XRay.Office.WordProcessing.Model;

/// <summary>
/// Snaps a stated page dimension onto a standard paper dimension when it is close enough,
/// the way the Word-family importers do.
/// </summary>
/// <remarks>
/// <para>
/// A document that says its page is 11912 twips wide is saying 21012 hundredths of a millimetre,
/// which is A4 plus a tenth of a millimetre — a rounding scar left by a round trip through inches
/// or through some other application's units, not a decision anyone made. Word's own file format
/// carries these constantly, and LibreOffice's importers erase them: every Word-family reader
/// passes each page dimension through a nearest-standard-dimension fit before the value reaches
/// the layout.
/// </para>
/// <para>
/// The three call sites are <c>DomainMapper.cxx</c>:827 and :836 for <c>w:pgSz</c> (which serves
/// RTF as well, since <c>rtfdispatchvalue.cxx</c>:1274-1289 dispatches <c>\paperw</c> and friends
/// through the same <c>LN_CT_PageSz_w</c>/<c>_h</c> cases) and <c>ww8par6.cxx</c>:521 and :1083 for
/// DOC. ODF is deliberately not in that list: an ODF document states its page in the same
/// hundredths of a millimetre the model uses, so there is no scar to erase.
/// </para>
/// <para>
/// Three details of the rule matter and none of them is guessable from its name.
/// </para>
/// <para>
/// **It fits one dimension at a time, against every dimension in the table.** A width is compared
/// against each standard paper's *height* as well as its width, so a page 364 mm wide snaps to
/// 364 mm — B4(JIS)'s height, and no paper's width. Measured on the installed 26.2.4.2: a stated
/// 20638 × 25000 twips comes back 1031.81 pt wide, which is 364 mm exactly, while 20500 twips at
/// the same height comes back unchanged. There is no notion of a matching *format* here, and a
/// page can leave this function as the width of one paper and the height of another.
/// </para>
/// <para>
/// **The window is 0.44 mm and the comparison is strict.** <c>MAXSLOPPY</c> is
/// <c>PT2MM100(1.25)</c>, which is 44 hundredths of a millimetre after its round-half-up, and the
/// test is <c>&lt; 44</c> rather than <c>&lt;=</c>. Measured on 26.2.4.2 by sweeping a stated A4
/// height one twip at a time: 16814 through 16862 twips all come back 841.89 pt, 16813 and 16863
/// come back unchanged. Those are exactly the twips whose hundredths-of-a-millimetre value lands
/// strictly inside 29700 ± 44.
/// </para>
/// <para>
/// **The comparison happens in hundredths of a millimetre, not in twips**, so the input's own
/// conversion rounding is part of the rule. 16813 twips is 29656.26 hundredths, which rounds to
/// 29656 and is 44 away — outside. A twip-domain window would have let it in.
/// </para>
/// <para>
/// The table is <c>aDinTab</c> from <c>i18nutil/source/utility/paper.cxx</c>, transcribed in its
/// own order and verified entry for entry against that file. **The order is load-bearing**: the
/// fit returns the first entry within the window, and eleven pairs of distinct dimensions in the
/// table sit closer together than two window widths — 21519 and 21590 (Quarto's width and
/// Letter's) are 0.71 mm apart, so a page between them can be within reach of both and the answer
/// is whichever comes first. The <c>PAPER_USER</c> row, which is 0 × 0 and would swallow every
/// small dimension, is skipped there and is simply absent here.
/// </para>
/// </remarks>
internal static class PaperSizes
{
    /// <summary>
    /// Half-open window, in hundredths of a millimetre, inside which a stated dimension is taken
    /// to be the standard one. <c>PT2MM100(1.25)</c> in <c>paper.cxx</c>:169.
    /// </summary>
    private const long MaxSloppyMm100 = 44;

    /// <summary>
    /// Every standard paper's width and height, in hundredths of a millimetre, in
    /// <c>aDinTab</c>'s order.
    /// </summary>
    private static readonly (long Width, long Height)[] Standard =
    [
        (84100, 118900), (59400, 84100), (42000, 59400), (29700, 42000), (21000, 29700),
        (14800, 21000), (25000, 35300), (17600, 25000), (21590, 27940), (21590, 35560), (27940, 43180),
        (12500, 17600), (22900, 32400), (16200, 22900), (11400, 16200), (11400, 22900), (11000, 22000),
        (18000, 27000), (21000, 28000), (43180, 55880), (55880, 86360), (86360, 111760),
        (18415, 26670), (21590, 33020), (9843, 19050), (9208, 16510), (9843, 22543), (10478, 24130),
        (11430, 26353), (12065, 27940), (18400, 26000), (13000, 18400), (14000, 20300), (25700, 36400),
        (18200, 25700), (12800, 18200), (43180, 27940), (13970, 21590), (21519, 27517), (25400, 35560),
        (13970, 29210), (32400, 45800), (11000, 23000), (37783, 27940), (21590, 33020), (10000, 14800),
        (22860, 27940), (25400, 27940), (38100, 27940), (22000, 22000), (22700, 35600), (30500, 48700),
        (21590, 32233), (21000, 33000), (20000, 14800), (10500, 14800), (30480, 27940), (7400, 10500),
        (5200, 7400), (3700, 5200), (2600, 3700), (100000, 141400), (70700, 100000), (50000, 70700),
        (35300, 50000), (8800, 12500), (6200, 8800), (4400, 6200), (3100, 4400), (45800, 64800),
        (8100, 11400), (5700, 8100), (22860, 30480), (30480, 45720), (45720, 60960), (60960, 91440),
        (91440, 121920), (15750, 28000), (17500, 28000), (19500, 27000), (19700, 27300),
        (19050, 33866), (19050, 25400), (14288, 25400), (15875, 25400),
    ];

    /// <summary>
    /// The nearest standard paper dimension within 0.44 mm of <paramref name="stated"/>, or
    /// <paramref name="stated"/> itself when none is that close.
    /// </summary>
    /// <remarks>
    /// The result is rebuilt from twips rather than from hundredths of a millimetre because that is
    /// the unit Writer lays a page out in: the DOC reader converts the fitted value straight back
    /// (<c>SvxPaperInfo::GetSloppyPaperDimension</c>), and the DOCX one hands hundredths to a page
    /// style whose frame size Writer then holds in twips. Both arrive at the same 11906 twips for
    /// A4's 21000, which is what the reference PDF's 595.304 pt media box reads back as.
    /// </remarks>
    internal static Length SloppyFit(Length stated)
    {
        long mm100 = stated.Mm100;

        foreach ((long width, long height) in Standard)
        {
            if (Math.Abs(width - mm100) < MaxSloppyMm100) return FromMm100ViaTwips(width);
            if (Math.Abs(height - mm100) < MaxSloppyMm100) return FromMm100ViaTwips(height);
        }

        return stated;
    }

    /// <summary>
    /// Hundredths of a millimetre to the nearest twip, then to a <see cref="Length"/>.
    /// </summary>
    private static Length FromMm100ViaTwips(long mm100)
        => Length.FromTwips((mm100 * 72 + 127 / 2) / 127);
}
