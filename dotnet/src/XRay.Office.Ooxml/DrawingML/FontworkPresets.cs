using System.Collections.Frozen;

namespace XRay.Office.Ooxml.DrawingML;

/// <summary>
/// The shape a WordArt preset warps its text along, in the binary WordArt vocabulary.
/// </summary>
/// <remarks>
/// <para>
/// These are transcriptions of the <c>mso_sptText*</c> tables in
/// <c>svx/source/customshapes/EnhancedCustomShapeGeometry.cxx</c>, one field at a time and in the
/// same order, and they are what the geometry has to be. LibreOffice does <em>not</em> read
/// DrawingML's own <c>presetTextWarpDefinitions</c>: <c>FontworkHelpers::putCustomShapeIntoTextPathMode</c>
/// (<c>oox/source/drawingml/fontworkhelpers.cxx:75-197</c>) maps the <c>a:prstTxWarp/@prst</c> name
/// onto one of these through <c>PresetGeometryTypeNames::GetFontworkType</c>
/// (<c>oox/source/drawingml/presetgeometrynames.cxx</c>) and converts the adjustment values into
/// the units these expect. Deriving the curves from the OOXML definitions instead would be a
/// different set of curves and would not match the reference.
/// </para>
/// <para>
/// The vocabulary is MS-ODRAW's. A vertex coordinate whose top bit is set — the C++ writes
/// <c>2 MSO_I</c> and this writes <c>2 | I</c> — is the index of a formula in
/// <see cref="FontworkPreset.Calculations"/> rather than a number. A formula's own parameters are
/// literal unless the matching flag bit says otherwise, when 0x400 + n is again a formula and 327 +
/// n is the n-th adjustment value. The segment words are MS-ODRAW path opcodes, decoded in
/// <see cref="FontworkGeometry"/>.
/// </para>
/// <para>
/// <strong>All forty of DrawingML's warps are here.</strong> The corpus states twenty-five of them
/// — the 24 on <c>WordArt_Shapes_Arrows_Catalog1.docx</c> plus <c>textPlain</c> — and seven more
/// share a table with one of those. The remaining eight were left out for a round on the ground
/// that <em>a table transcribed for a preset no document states is a transcription nothing
/// checks</em>, and they are here now because that stopped being true:
/// <c>fontwork-presets-default.docx</c> and <c>fontwork-presets-adjusted.docx</c> state one shape
/// per <c>ST_TextShapeType</c> value in the catalogue's own container, so every table is measured
/// against both LibreOffice references rather than transcribed and hoped for. That took the
/// fixture's nine-page mean absolute grey difference from <b>2.584 to 0.603</b>.
/// </para>
/// <para>
/// Two things about the eight are worth knowing, because the reason they were deferred was partly
/// wrong. <strong>Five of them needed nothing but their tables</strong>: the four <c>*Pour</c>
/// shapes and <c>mso-spt142</c> are drawn with <c>0xA304</c> and <c>0xA504</c>, the same arc
/// opcodes the arch family uses, and a pour is simply two concentric arcs with the text fitted into
/// the ring between them. <strong>Only <c>mso-spt143</c> needed a new path builder</strong> —
/// <c>ANGLEELLIPSE</c>, which <see cref="FontworkGeometry"/> now decodes, and whose angles that one
/// shape states in plain degrees where every other binary user of the opcode states 1/65536ths.
/// </para>
/// </remarks>
public static class FontworkPresets
{
    /// <summary>The bit MS-ODRAW sets on a coordinate that names a formula instead of a value.</summary>
    public const int FormulaFlag = unchecked((int)0x80000000);

    /// <summary>The first adjustment value's property id, <c>DFF_Prop_adjustValue</c>.</summary>
    /// <remarks><c>include/svx/msdffdef.hxx:151</c>. The n-th adjustment is this plus n.</remarks>
    public const int FirstAdjustmentProperty = 327;

    private const int I = FormulaFlag;

    private static readonly Lazy<FrozenDictionary<string, FontworkPreset>> Table =
        new(Build, LazyThreadSafetyMode.ExecutionAndPublication);

    /// <summary>The preset of that LibreOffice Fontwork name, or null when it is not one.</summary>
    /// <param name="fontworkType">
    /// A name from <c>presetgeometrynames.cxx</c> — <c>fontwork-arch-up-curve</c>, <c>mso-spt157</c>
    /// and the like — not an OOXML <c>prst</c> value. <see cref="Fontwork.FontworkTypeOf"/> maps one
    /// to the other.
    /// </param>
    public static FontworkPreset? Find(string? fontworkType)
        => fontworkType is not null && Table.Value.TryGetValue(fontworkType, out FontworkPreset? preset)
            ? preset
            : null;

    private static FrozenDictionary<string, FontworkPreset> Build()
    {
        Dictionary<string, FontworkPreset> presets = new(StringComparer.Ordinal);

        // Shared segment programmes, named as the C++ names them.
        int[] fadeSegments = [0x4000, 0x0001, 0x8000, 0x4000, 0x0001, 0x8000];
        int[] canUpSegments = [0x4000, 0x2002, 0x8000, 0x4000, 0x2002, 0x8000];
        int[] curveUpSegments = [0x4000, 0x2001, 0x8000, 0x4000, 0x2002, 0x8000];

        // Shared formula tables.
        FontworkFormula[] fadeCalc =
        [
            new(0x2000, FirstAdjustmentProperty, 0, 0),
            new(0x8000, 21600, 0, FirstAdjustmentProperty),
        ];
        FontworkFormula[] cascadeCalc =
        [
            new(0x2000, FirstAdjustmentProperty, 0, 0),
            new(0x8000, 21600, 0, FirstAdjustmentProperty),
            new(0x2001, 0x401, 1, 4),
        ];
        FontworkFormula[] archCurveCalc =
        [
            new(0x400a, 10800, FirstAdjustmentProperty, 0),
            new(0x4009, 10800, FirstAdjustmentProperty, 0),
            new(0x2000, 0x400, 10800, 0),
            new(0x2000, 0x401, 10800, 0),
            new(0x8000, 21600, 0, 0x402),
        ];
        FontworkFormula[] triangleCalc = [new(0x2000, FirstAdjustmentProperty, 0, 0)];
        FontworkFormula[] chevronCalc =
        [
            new(0x2000, FirstAdjustmentProperty, 0, 0),
            new(0x8000, 21600, 0, FirstAdjustmentProperty),
        ];
        FontworkFormula[] curveUpCalc =
        [
            new(0x2000, FirstAdjustmentProperty, 0, 0),
            new(0x4001, 14250, 0x400, 12170),
            new(0x4001, 12800, 0x400, 12170),
            new(0x4001, 6380, 0x400, 12170),
            new(0x8000, 21600, 0, 0x403),
        ];

        void Add(
            string name,
            int[] vertices,
            int[] segments,
            FontworkFormula[] calculations,
            int[] defaults)
            => presets[name] = new FontworkPreset(name, vertices, segments, calculations, defaults);

        // ---- fontwork-plain-text (textPlain) -------------------------------------------------
        Add(
            "fontwork-plain-text",
            [3 | I, 0, 5 | I, 0, 6 | I, 21600, 7 | I, 21600],
            fadeSegments,
            [
                new(0x2000, FirstAdjustmentProperty, 0, 10800),
                new(0x2001, 0x400, 2, 1),
                new(0x2003, 0x401, 0, 0),
                new(0xa006, 0x401, 0, 0x402),
                new(0x8000, 21600, 0, 0x402),
                new(0x6006, 0x401, 0x404, 21600),
                new(0x6006, 0x401, 0x402, 0),
                new(0xa006, 0x401, 21600, 0x404),
            ],
            [10800]);

        // ---- fontwork-stop (textStop) --------------------------------------------------------
        Add(
            "fontwork-stop",
            [
                0, 0 | I, 7200, 0, 14400, 0, 21600, 0 | I,
                0, 1 | I, 7200, 21600, 14400, 21600, 21600, 1 | I,
            ],
            [0x4000, 0x0003, 0x8000, 0x4000, 0x0003, 0x8000],
            [
                new(0x2000, FirstAdjustmentProperty, 0, 0),
                new(0x8000, 21600, 0, FirstAdjustmentProperty),
            ],
            [2700]);

        // ---- fontwork-triangle-up / -down (textTriangle, textTriangleInverted) ---------------
        Add(
            "fontwork-triangle-up",
            [0, 0 | I, 10800, 0, 21600, 0 | I, 0, 21600, 21600, 21600],
            [0x4000, 0x0002, 0x8000, 0x4000, 0x0001, 0x8000],
            triangleCalc,
            [10800]);

        Add(
            "fontwork-triangle-down",
            [0, 0, 21600, 0, 0, 0 | I, 10800, 21600, 21600, 0 | I],
            [0x4000, 0x0001, 0x8000, 0x4000, 0x0002, 0x8000],
            triangleCalc,
            [10800]);

        // ---- fontwork-chevron-up / -down (textChevron, textChevronInverted) ------------------
        Add(
            "fontwork-chevron-up",
            [0, 0 | I, 10800, 0, 21600, 0 | I, 0, 21600, 10800, 1 | I, 21600, 21600],
            [0x4000, 0x0002, 0x8000, 0x4000, 0x0002, 0x8000],
            chevronCalc,
            [5400]);

        Add(
            "fontwork-chevron-down",
            [0, 0, 10800, 1 | I, 21600, 0, 0, 0 | I, 10800, 21600, 21600, 0 | I],
            [0x4000, 0x0002, 0x8000, 0x4000, 0x0002, 0x8000],
            chevronCalc,
            [16200]);

        // ---- fontwork-fade-* (textFadeRight, textFadeLeft, textFadeUp, textFadeDown) ---------
        Add("fontwork-fade-right", [0, 0, 21600, 0 | I, 0, 21600, 21600, 1 | I], fadeSegments, fadeCalc, [7200]);
        Add("fontwork-fade-left", [0, 0 | I, 21600, 0, 0, 1 | I, 21600, 21600], fadeSegments, fadeCalc, [7200]);
        Add("fontwork-fade-up", [0 | I, 0, 1 | I, 0, 0, 21600, 21600, 21600], fadeSegments, fadeCalc, [7200]);
        Add("fontwork-fade-down", [0, 0, 21600, 0, 0 | I, 21600, 1 | I, 21600], fadeSegments, fadeCalc, [7200]);

        // ---- fontwork-slant-up / -down (textSlantUp, textSlantDown) --------------------------
        Add("fontwork-slant-up", [0, 0 | I, 21600, 0, 0, 21600, 21600, 1 | I], fadeSegments, fadeCalc, [12000]);
        Add("fontwork-slant-down", [0, 0, 21600, 1 | I, 0, 0 | I, 21600, 21600], fadeSegments, fadeCalc, [12000]);

        // ---- fontwork-fade-up-and-right / -left (textCascadeUp, textCascadeDown) -------------
        Add(
            "fontwork-fade-up-and-right",
            [0, 2 | I, 21600, 0, 0, 21600, 21600, 0 | I],
            fadeSegments,
            cascadeCalc,
            [9600]);

        Add(
            "fontwork-fade-up-and-left",
            [0, 0, 21600, 2 | I, 0, 0 | I, 21600, 21600],
            fadeSegments,
            cascadeCalc,
            [9600]);

        // ---- fontwork-arch-up-curve / -down-curve (textArchUp, textArchDown) -----------------
        Add(
            "fontwork-arch-up-curve",
            [0, 0, 21600, 21600, 2 | I, 3 | I, 4 | I, 3 | I],
            [0xA504, 0x8000],
            archCurveCalc,
            [180]);

        Add(
            "fontwork-arch-down-curve",
            [0, 0, 21600, 21600, 4 | I, 3 | I, 2 | I, 3 | I],
            [0xA304, 0x8000],
            archCurveCalc,
            [0]);

        // ---- fontwork-circle-curve (textCircle) ----------------------------------------------
        Add(
            "fontwork-circle-curve",
            [0, 0, 21600, 21600, 2 | I, 3 | I, 2 | I, 4 | I],
            [0xA504, 0x8000],
            [
                new(0x400a, 10800, FirstAdjustmentProperty, 0),
                new(0x4009, 10800, FirstAdjustmentProperty, 0),
                new(0x2000, 0x400, 10800, 0),
                new(0x2000, 0x401, 10800, 0),
                new(0x8000, 21600, 0, 0x403),
            ],
            [-179]);

        // ---- fontwork-open-circle-curve (textButton) -----------------------------------------
        Add(
            "fontwork-open-circle-curve",
            [
                0, 0, 21600, 21600, 2 | I, 3 | I, 4 | I, 3 | I,
                0, 10800, 21600, 10800,
                0, 0, 21600, 21600, 2 | I, 5 | I, 4 | I, 5 | I,
            ],
            [0xA504, 0x8000, 0x4000, 0x0001, 0x8000, 0xA304, 0x8000],
            [
                new(0x400a, 10800, FirstAdjustmentProperty, 0),
                new(0x4009, 10800, FirstAdjustmentProperty, 0),
                new(0x2000, 0x400, 10800, 0),
                new(0x2000, 0x401, 10800, 0),
                new(0x8000, 21600, 0, 0x402),
                new(0x8000, 21600, 0, 0x403),
            ],
            [180]);

        // ---- fontwork-curve-up / -down (textCurveUp, textCurveDown) --------------------------
        Add(
            "fontwork-curve-up",
            [
                0, 0 | I, 4900, 1 | I, 11640, 2 | I, 21600, 0,
                0, 4 | I, 3700, 21600, 8500, 21600, 10100, 21600, 14110, 21600, 15910, 21600, 21600, 4 | I,
            ],
            curveUpSegments,
            curveUpCalc,
            [9900]);

        Add(
            "fontwork-curve-down",
            [
                0, 0, 9960, 2 | I, 16700, 1 | I, 21600, 0 | I,
                0, 4 | I, 5690, 21600, 7490, 21600, 11500, 21600, 13100, 21600, 17900, 21600, 21600, 4 | I,
            ],
            curveUpSegments,
            curveUpCalc,
            [9900]);

        // ---- mso-spt174 / mso-spt175 (textCanUp, textCanDown) --------------------------------
        Add(
            "mso-spt174",
            [
                0, 1 | I, 900, 0, 7100, 0, 10800, 0, 14500, 0, 20700, 0, 21600, 1 | I,
                0, 21600, 900, 4 | I, 7100, 0 | I, 10800, 0 | I, 14500, 0 | I, 20700, 4 | I, 21600, 21600,
            ],
            canUpSegments,
            [
                new(0x2000, FirstAdjustmentProperty, 0, 0),
                new(0x8000, 21600, 0, FirstAdjustmentProperty),
                new(0x2000, FirstAdjustmentProperty, 0, 14400),
                new(0x4001, 5470, 0x402, 7200),
                new(0x4000, 16130, 0x403, 0),
            ],
            [18500]);

        Add(
            "mso-spt175",
            [
                0, 0, 900, 2 | I, 7100, 0 | I, 10800, 0 | I, 14500, 0 | I, 20700, 2 | I, 21600, 0,
                0, 1 | I, 900, 21600, 7100, 21600, 10800, 21600, 14500, 21600, 20700, 21600, 21600, 1 | I,
            ],
            canUpSegments,
            [
                new(0x2000, FirstAdjustmentProperty, 0, 0),
                new(0x8000, 21600, 0, FirstAdjustmentProperty),
                new(0x4001, 5470, 0x400, 7200),
            ],
            [3100]);

        // ---- fontwork-inflate / mso-spt161 (textInflate, textDeflate) ------------------------
        Add(
            "fontwork-inflate",
            [
                0, 0 | I, 4100, 1 | I, 7300, 0, 10800, 0, 14300, 0, 17500, 1 | I, 21600, 0 | I,
                0, 2 | I, 4100, 3 | I, 7300, 21600, 10800, 21600, 14300, 21600, 17500, 3 | I, 21600, 2 | I,
            ],
            canUpSegments,
            [
                new(0x2000, FirstAdjustmentProperty, 0, 0),
                new(0x4001, 1530, 0x400, 4650),
                new(0x8000, 21600, 0, 0x400),
                new(0x8000, 21600, 0, 0x401),
            ],
            [2950]);

        Add(
            "mso-spt161",
            [
                0, 0, 3500, 1 | I, 7100, 0 | I, 10800, 0 | I, 14500, 0 | I, 18100, 1 | I, 21600, 0,
                0, 21600, 3500, 3 | I, 7100, 2 | I, 10800, 2 | I, 14500, 2 | I, 18100, 3 | I, 21600, 21600,
            ],
            canUpSegments,
            [
                new(0x2000, FirstAdjustmentProperty, 0, 0),
                new(0x2001, 0x400, 5320, 7100),
                new(0x8000, 21600, 0, 0x400),
                new(0x8000, 21600, 0, 0x401),
            ],
            [8100]);

        // ---- mso-spt162 / mso-spt163 (textInflateBottom, textDeflateBottom) ------------------
        Add(
            "mso-spt162",
            [
                0, 0, 21600, 0,
                0, 0 | I, 3500, 3 | I, 7300, 21600, 10800, 21600, 14300, 21600, 18100, 3 | I, 21600, 0 | I,
            ],
            [0x4000, 0x0001, 0x8000, 0x4000, 0x2002, 0x8000],
            [
                new(0x2000, FirstAdjustmentProperty, 0, 0),
                new(0x2000, 0x400, 0, 11150),
                new(0x2001, 0x401, 3900, 10450),
                new(0x2000, 0x402, 17700, 0),
            ],
            [14700]);

        Add(
            "mso-spt163",
            [
                0, 0, 21600, 0,
                0, 21600, 2900, 3 | I, 7200, 0 | I, 10800, 0 | I, 14400, 0 | I, 18700, 3 | I, 21600, 21600,
            ],
            [0x4000, 0x0001, 0x8000, 0x4000, 0x2002, 0x8000],
            [
                new(0x2000, FirstAdjustmentProperty, 0, 0),
                new(0x2000, 0x400, 0, 1350),
                new(0x2001, 0x401, 12070, 20250),
                new(0x2000, 0x402, 9530, 0),
            ],
            [11500]);

        // ---- mso-spt164 / mso-spt165 (textInflateTop, textDeflateTop) ------------------------
        Add(
            "mso-spt164",
            [
                0, 0 | I, 3500, 1 | I, 7300, 0, 10800, 0, 14300, 0, 18100, 1 | I, 21600, 0 | I,
                0, 21600, 21600, 21600,
            ],
            [0x4000, 0x2002, 0x8000, 0x4000, 0x0001, 0x8000],
            [
                new(0x2000, FirstAdjustmentProperty, 0, 0),
                new(0x2001, 0x400, 3900, 10450),
            ],
            [6900]);

        Add(
            "mso-spt165",
            [
                0, 0, 2900, 1 | I, 7200, 0 | I, 10800, 0 | I, 14400, 0 | I, 18700, 1 | I, 21600, 0,
                0, 21600, 21600, 21600,
            ],
            [0x4000, 0x2002, 0x8000, 0x4000, 0x0001, 0x8000],
            [
                new(0x2000, FirstAdjustmentProperty, 0, 0),
                new(0x2001, 0x400, 12070, 20250),
            ],
            [10100]);

        // ---- the four *Pour shapes and mso-spt142 --------------------------------------------
        // A pour shape is an arch or a circle with a hole in it: two concentric arcs rather than
        // one, so the text is fitted into the ring between them instead of laid along a line. That
        // is the same even-rail envelope every other warp uses and needs no new path builder — the
        // arcs are `0xA504` and `0xA304`, which `FontworkGeometry` already decodes. `adj2` is the
        // inner radius, and `Fontwork.Adjustments` is what halves it (`fontworkhelpers.cxx:135-141`).
        int adj2 = FirstAdjustmentProperty + 1;

        FontworkFormula[] archPourCalc =
        [
            new(0x400a, 10800, FirstAdjustmentProperty, 0),
            new(0x4009, 10800, FirstAdjustmentProperty, 0),
            new(0x2000, 0x400, 10800, 0),
            new(0x2000, 0x401, 10800, 0),
            new(0x8000, 21600, 0, 0x402),
            new(0x8000, 10800, 0, adj2),
            new(0x600a, 0x405, FirstAdjustmentProperty, 0),
            new(0x6009, 0x405, FirstAdjustmentProperty, 0),
            new(0x2000, 0x406, 10800, 0),
            new(0x2000, 0x407, 10800, 0),
            new(0x8000, 21600, 0, 0x408),
            new(0x8000, 21600, 0, 0x405),
        ];

        Add(
            "fontwork-arch-up-pour",
            [
                0, 0, 21600, 21600, 2 | I, 3 | I, 4 | I, 3 | I,
                5 | I, 5 | I, 11 | I, 11 | I, 8 | I, 9 | I, 0xa | I, 9 | I,
            ],
            [0xA504, 0x8000, 0xA504, 0x8000],
            archPourCalc,
            [180, 5400]);

        Add(
            "fontwork-arch-down-pour",
            [
                5 | I, 5 | I, 11 | I, 11 | I, 0xa | I, 9 | I, 8 | I, 9 | I,
                0, 0, 21600, 21600, 4 | I, 3 | I, 2 | I, 3 | I,
            ],
            [0xA304, 0x8000, 0xA304, 0x8000],
            archPourCalc,
            [0, 5400]);

        // Not `archPourCalc`: 4 and 10 differ, which is the whole of the difference between an arch
        // and a full circle here.
        Add(
            "fontwork-circle-pour",
            [
                0, 0, 21600, 21600, 2 | I, 3 | I, 2 | I, 4 | I,
                5 | I, 5 | I, 11 | I, 11 | I, 8 | I, 9 | I, 8 | I, 0xa | I,
            ],
            [0xA504, 0x8000, 0xA504, 0x8000],
            [
                new(0x400a, 10800, FirstAdjustmentProperty, 0),
                new(0x4009, 10800, FirstAdjustmentProperty, 0),
                new(0x2000, 0x400, 10800, 0),
                new(0x2000, 0x401, 10800, 0),
                new(0x8000, 21600, 0, 0x403),
                new(0x8000, 10800, 0, adj2),
                new(0x600a, 0x405, FirstAdjustmentProperty, 0),
                new(0x6009, 0x405, FirstAdjustmentProperty, 0),
                new(0x2000, 0x406, 10800, 0),
                new(0x2000, 0x407, 10800, 0),
                new(0x8000, 21600, 0, 0x409),
                new(0x8000, 21600, 0, 0x405),
                new(0x0000, 21600, 0, 0),
            ],
            [-179, 5400]);

        Add(
            "fontwork-open-circle-pour",
            [
                0, 0, 21600, 21600, 2 | I, 3 | I, 4 | I, 3 | I,
                6 | I, 6 | I, 7 | I, 7 | I, 10 | I, 11 | I, 12 | I, 11 | I,
                0x16 | I, 16 | I, 0x15 | I, 16 | I,
                0x16 | I, 15 | I, 0x15 | I, 15 | I,
                6 | I, 6 | I, 7 | I, 7 | I, 10 | I, 13 | I, 12 | I, 13 | I,
                0, 0, 21600, 21600, 2 | I, 5 | I, 4 | I, 5 | I,
            ],
            [
                0xA504, 0x8000,
                0xA504, 0x8000,
                0x4000, 0x0001, 0x8000,
                0x4000, 0x0001, 0x8000,
                0xA304, 0x8000,
                0xA304, 0x8000,
            ],
            [
                new(0x400a, 10800, FirstAdjustmentProperty, 0),
                new(0x4009, 10800, FirstAdjustmentProperty, 0),
                new(0x2000, 0x400, 10800, 0),
                new(0x2000, 0x401, 10800, 0),
                new(0x8000, 21600, 0, 0x402),
                new(0x8000, 21600, 0, 0x403),
                new(0x8000, 10800, 0, adj2),
                new(0x8000, 21600, 0, 0x406),
                new(0x600a, adj2, FirstAdjustmentProperty, 0),
                new(0x6009, adj2, FirstAdjustmentProperty, 0),
                new(0x2000, 0x408, 10800, 0),
                new(0x2000, 0x409, 10800, 0),
                new(0x8000, 21600, 0, 0x40a),
                new(0x8000, 21600, 0, 0x40b),
                new(0x2001, 0x406, 1, 2),
                new(0x4000, 10800, 0x40e, 0),
                new(0x8000, 10800, 0, 0x40e),
                new(0x6001, 0x40e, 0x40e, 1),
                new(0x6001, adj2, adj2, 1),
                new(0xa000, 0x412, 0, 0x411),
                new(0x200d, 0x413, 0, 0),
                new(0x4000, 10800, 0x414, 0),
                new(0x8000, 10800, 0, 0x414),
            ],
            [180, 5400]);

        // mso-spt142 (textRingInside). Two clockwise arcs each drawn as an implicit-move arc
        // followed by an arc-to, which is `0xa604 0xa504` — both opcodes already decoded.
        Add(
            "mso-spt142",
            [
                0, 0, 21600, 2 | I, 0, 0 | I, 21600, 0 | I,
                0, 0, 21600, 2 | I, 21600, 0 | I, 0, 0 | I,
                0, 3 | I, 21600, 21600, 0, 1 | I, 21600, 1 | I,
                0, 3 | I, 21600, 21600, 21600, 1 | I, 0, 1 | I,
            ],
            [0xa604, 0xa504, 0x8000, 0xa604, 0xa504, 0x8000],
            [
                new(0x2001, FirstAdjustmentProperty, 1, 2),
                new(0x8000, 21600, 0, 0x400),
                new(0x2000, FirstAdjustmentProperty, 0, 0),
                new(0x8000, 21600, 0, FirstAdjustmentProperty),
            ],
            [13500]);

        // mso-spt143 (textRingOutside). Two full ellipses, drawn with `ANGLEELLIPSE` rather than
        // with the arc opcodes — the only WordArt table that uses it, and the reason
        // `FontworkGeometry` decodes 0xA1 and 0xA2 at all. Its angles are plain degrees where every
        // other binary `ANGLEELLIPSE` states 1/65536ths, which the reference special-cases by this
        // very name (`EnhancedCustomShape2d.cxx:2253-2258`).
        Add(
            "mso-spt143",
            [
                10800, 0 | I, 10800, 0 | I, 180, 359,
                10800, 1 | I, 10800, 0 | I, 180, 359,
            ],
            [0xA203, 0x8000, 0xA203, 0x8000],
            [
                new(0x2001, FirstAdjustmentProperty, 1, 2),
                new(0x8000, 21600, 0, 0x400),
            ],
            [13500]);

        // ---- mso-spt166 / mso-spt167 (textDeflateInflate, textDeflateInflateDeflate) ---------
        // The only two warps whose geometry is four and six rails rather than two, so the text is
        // split across two and three envelopes and `Distribute` deals the lines out between them.
        // A single-line body — which is every corpus warp — fills the first envelope and leaves the
        // others empty, which is what the reference draws too.
        Add(
            "mso-spt166",
            [
                0, 0, 21600, 0,
                0, 10100, 3300, 3 | I, 7100, 5 | I, 10800, 5 | I, 14500, 5 | I, 18300, 3 | I, 21600, 10100,
                0, 11500, 3300, 4 | I, 7100, 6 | I, 10800, 6 | I, 14500, 6 | I, 18300, 4 | I, 21600, 11500,
                0, 21600, 21600, 21600,
            ],
            [
                0x4000, 0x0001, 0x8000,
                0x4000, 0x2002, 0x8000,
                0x4000, 0x2002, 0x8000,
                0x4000, 0x0001, 0x8000,
            ],
            [
                new(0x2000, FirstAdjustmentProperty, 0, 0),
                new(0x8000, 10800, 0, 0x400),
                new(0x2001, 0x401, 5770, 9500),
                new(0x8000, 10100, 0, 0x402),
                new(0x8000, 11500, 0, 0x402),
                new(0x2000, 0x400, 0, 700),
                new(0x2000, 0x400, 700, 0),
            ],
            [6500]);

        Add(
            "mso-spt167",
            [
                0, 0, 21600, 0,
                0, 6600, 3600, 3 | I, 7250, 4 | I, 10800, 4 | I, 14350, 4 | I, 18000, 3 | I, 21600, 6600,
                0, 7500, 3600, 5 | I, 7250, 6 | I, 10800, 6 | I, 14350, 6 | I, 18000, 5 | I, 21600, 7500,
                0, 14100, 3600, 9 | I, 7250, 10 | I, 10800, 10 | I, 14350, 10 | I, 18000, 9 | I, 21600, 14100,
                0, 15000, 3600, 7 | I, 7250, 8 | I, 10800, 8 | I, 14350, 8 | I, 18000, 7 | I, 21600, 15000,
                0, 21600, 21600, 21600,
            ],
            [
                0x4000, 0x0001, 0x8000,
                0x4000, 0x2002, 0x8000,
                0x4000, 0x2002, 0x8000,
                0x4000, 0x2002, 0x8000,
                0x4000, 0x2002, 0x8000,
                0x4000, 0x0001, 0x8000,
            ],
            [
                new(0x2000, FirstAdjustmentProperty, 0, 850),
                new(0x2001, 0x400, 6120, 8700),
                new(0x2000, 0x401, 0, 4280),
                new(0x4000, 6600, 0x402, 0),
                new(0x2000, FirstAdjustmentProperty, 0, 450),
                new(0x2000, 0x403, 900, 0),
                new(0x2000, 0x404, 900, 0),
                new(0x8000, 21600, 0, 0x403),
                new(0x8000, 21600, 0, 0x404),
                new(0x8000, 21600, 0, 0x405),
                new(0x8000, 21600, 0, 0x406),
            ],
            [6050]);

        AddWaves(presets);
        return presets.ToFrozenDictionary(StringComparer.Ordinal);
    }

    /// <summary>
    /// The four wave presets, which share two large formula tables between them.
    /// </summary>
    /// <remarks>
    /// <c>mso_sptWaveCalc</c> and <c>mso_sptDoubleWaveCalc</c> are the shared ones. A wave takes two
    /// adjustments — the crest height and the horizontal phase — which is why its <c>a:avLst</c>
    /// carries an <c>adj2</c> and why that one alone is converted about the centre rather than
    /// scaled straight (<c>fontworkhelpers.cxx:126-133</c>).
    /// </remarks>
    private static void AddWaves(Dictionary<string, FontworkPreset> presets)
    {
        FontworkFormula[] waveCalc =
        [
            new(0x2000, FirstAdjustmentProperty, 0, 0),          // 400 vertical adjust
            new(0x8000, 21600, 0, 0x400),                        // 401
            new(0x2000, FirstAdjustmentProperty + 1, 0, 0),      // 402 horizontal adjust
            new(0x2000, 0x402, 0, 10800),                        // 403
            new(0x2001, 0x403, 2, 1),                            // 404
            new(0x2003, 0x404, 0, 0),                            // 405
            new(0x8000, 4320, 0, 0x405),                         // 406
            new(0xa006, 0x403, 0, 0x405),                        // 407
            new(0x4001, 15800, 0x400, 4460),                     // 408
            new(0xa000, 0x400, 0, 0x408),                        // 409
            new(0x6000, 0x400, 0x408, 0),                        // 40a
            new(0x8000, 21600, 0, 0x404),                        // 40b
            new(0x6006, 0x403, 0x40b, 21600),                    // 40c
            new(0xa000, 0x40c, 0, 0x407),                        // 40d
            new(0x2001, 0x405, 1, 2),                            // 40e
            new(0xa000, 0x407, 7200, 0x40e),                     // 40f
            new(0x6000, 0x40c, 0x40e, 7200),                     // 410
            new(0x2001, 0x40d, 1, 2),                            // 411
            new(0x6000, 0x407, 0x411, 0),                        // 412
            new(0x8000, 21600, 0, 0x412),                        // 413
            new(0x2001, 0x405, 1, 2),                            // 414
            new(0x8000, 21600, 0, 0x414),                        // 415
            new(0x2001, 0x400, 2, 1),                            // 416
            new(0x8000, 21600, 0, 0x416),                        // 417
            new(0x8000, 21600, 0, 0x407),                        // 418
            new(0x8000, 21600, 0, 0x40f),                        // 419
            new(0x6000, 0x401, 0x408, 0),                        // 41a
            new(0x8000, 21600, 0, 0x410),                        // 41b
            new(0xa000, 0x401, 0, 0x408),                        // 41c
            new(0x8000, 21600, 0, 0x40c),                        // 41d
        ];

        FontworkFormula[] doubleWaveCalc =
        [
            .. waveCalc[..8],
            new(0x4001, 7900, 0x400, 2230),                      // 408, the one that differs
            .. waveCalc[9..15],
            new(0xa000, 0x407, 3600, 0x40e),                     // 40f
            new(0x6000, 0x40c, 0x40e, 3600),                     // 410
            .. waveCalc[17..],
            new(0xa000, 0x412, 3600, 0x40e),                     // 41e
            new(0x6000, 0x412, 0x40e, 3600),                     // 41f
            new(0xa000, 0x413, 3600, 0x40e),                     // 420
            new(0x6000, 0x413, 0x40e, 3600),                     // 421
        ];

        int[] waveSegments = [0x4000, 0x2001, 0x8000, 0x4000, 0x2001, 0x8000];
        int[] doubleWaveSegments = [0x4000, 0x2002, 0x8000, 0x4000, 0x2002, 0x8000];
        int[] waveDefaults = [1400, 10800];

        presets["fontwork-wave"] = new FontworkPreset(
            "fontwork-wave",
            [
                7 | I, 0 | I, 15 | I, 9 | I, 16 | I, 10 | I, 12 | I, 0 | I,
                29 | I, 1 | I, 27 | I, 28 | I, 25 | I, 26 | I, 24 | I, 1 | I,
            ],
            waveSegments,
            waveCalc,
            waveDefaults);

        presets["mso-spt157"] = new FontworkPreset(
            "mso-spt157",
            [
                7 | I, 0 | I, 15 | I, 10 | I, 16 | I, 9 | I, 12 | I, 0 | I,
                29 | I, 1 | I, 27 | I, 26 | I, 25 | I, 28 | I, 24 | I, 1 | I,
            ],
            waveSegments,
            waveCalc,
            waveDefaults);

        presets["mso-spt158"] = new FontworkPreset(
            "mso-spt158",
            [
                7 | I, 0 | I, 15 | I, 9 | I, 31 | I, 10 | I, 18 | I, 0 | I,
                30 | I, 9 | I, 16 | I, 10 | I, 12 | I, 0 | I,
                29 | I, 1 | I, 27 | I, 28 | I, 33 | I, 26 | I, 19 | I, 1 | I,
                32 | I, 28 | I, 25 | I, 26 | I, 24 | I, 1 | I,
            ],
            doubleWaveSegments,
            doubleWaveCalc,
            waveDefaults);

        presets["mso-spt159"] = new FontworkPreset(
            "mso-spt159",
            [
                7 | I, 0 | I, 15 | I, 10 | I, 31 | I, 9 | I, 18 | I, 0 | I,
                30 | I, 10 | I, 16 | I, 9 | I, 12 | I, 0 | I,
                29 | I, 1 | I, 27 | I, 26 | I, 33 | I, 28 | I, 19 | I, 1 | I,
                32 | I, 26 | I, 25 | I, 28 | I, 24 | I, 1 | I,
            ],
            doubleWaveSegments,
            doubleWaveCalc,
            waveDefaults);
    }
}

/// <summary>One preset's geometry, in the units the WordArt tables use.</summary>
/// <param name="Name">The LibreOffice Fontwork type name.</param>
/// <param name="Vertices">
/// The coordinate list as x,y pairs. A value carrying <see cref="FontworkPresets.FormulaFlag"/>
/// names a formula rather than stating a number.
/// </param>
/// <param name="Segments">The MS-ODRAW path opcodes that consume those coordinates.</param>
/// <param name="Calculations">The formulae, in evaluation order; formula n is 0x400 + n.</param>
/// <param name="Defaults">
/// The adjustment values to use where the document states none. Their count is also how many
/// adjustments the preset has.
/// </param>
public sealed record FontworkPreset(
    string Name,
    IReadOnlyList<int> Vertices,
    IReadOnlyList<int> Segments,
    IReadOnlyList<FontworkFormula> Calculations,
    IReadOnlyList<int> Defaults);

/// <summary>
/// One MS-ODRAW formula: an operation in the low byte of <paramref name="Flags"/>, three
/// parameters, and a bit per parameter saying whether it is a reference rather than a number.
/// </summary>
/// <param name="Flags">
/// <c>0x2000</c>, <c>0x4000</c> and <c>0x8000</c> mark parameters one, two and three as references;
/// the low byte is the operation, decoded in <see cref="FontworkGeometry"/> exactly as
/// <c>EnhancedCustomShape2d::GetEquation</c> decodes it
/// (<c>svx/source/customshapes/EnhancedCustomShape2d.cxx:88-300</c>).
/// </param>
/// <param name="P1">First parameter.</param>
/// <param name="P2">Second parameter.</param>
/// <param name="P3">Third parameter.</param>
public readonly record struct FontworkFormula(int Flags, int P1, int P2, int P3);
