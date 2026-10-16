using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Model;

namespace XRay.Office.WordProcessing.Rtf;

/// <summary>
/// Accumulates RTF's page geometry as the control words go by.
/// </summary>
/// <remarks>
/// <para>
/// RTF states the same geometry twice, at two scopes, with two sets of control words. The
/// document-level ones (<c>\paperw</c>, <c>\margl</c>, <c>\headery</c>) are the defaults every section
/// starts from; the section-level ones (<c>\pgwsxn</c>, <c>\marglsxn</c>, <c>\guttersxn</c>) override
/// them for the section being read. <c>\sectd</c> resets the section back to the document's defaults,
/// and <c>\sect</c> ends one — so this is a small state machine rather than a parse of a subtree, and
/// it has to be fed in order.
/// </para>
/// <para>
/// The lesson from the merge handling elsewhere in the RTF reader applies again: what LibreOffice
/// <em>writes</em> and what the specification permits are different sets. LibreOffice writes the
/// document-level words and repeats the section-level ones on every section, while Word writes the
/// section-level ones only where a section differs. Reading both, with section overriding document, is
/// what handles either.
/// </para>
/// </remarks>
internal sealed class RtfPageGeometry
{
    /// <summary>Twips beyond which a page dimension is treated as a producer error.</summary>
    private const int MaxDimensionTwips = 22 * 1440;

    private readonly List<PageDefaults> _sections = [];

    private PageDefaults _document = PageDefaults.Initial;
    private PageDefaults _section = PageDefaults.Initial;
    private bool _sectionTouched;

    /// <summary>
    /// Which page-border side the <c>\brdr*</c> words now describe, or null when none is selected.
    /// </summary>
    /// <remarks>
    /// RTF states a border in two halves everywhere it states one — <c>\pgbrdrl</c> selects a side and
    /// the <c>\brdrw</c>, <c>\brdrcf</c>, <c>\brsp</c> and style words after it describe whichever
    /// side was selected last — and the same describing words serve a cell, a paragraph, a character
    /// and a page. So the selection is a small state machine, and it has to be *cleared* by any word
    /// that selects one of the other four scopes or the page border would swallow a table's.
    /// LibreOffice keeps the identical state as <c>RTFBorderState</c>
    /// (<c>sw/source/writerfilter/rtftok/rtfdocumentimpl.cxx</c>:167-215).
    /// </remarks>
    private int? _pageBorderSide;

    /// <summary>
    /// The sections read so far, plus the one still open.
    /// </summary>
    /// <remarks>
    /// The open one is included because RTF's last section has no <c>\sect</c> — the document simply
    /// ends. A reader that only collected closed sections would lose the geometry of every single-section
    /// document, which is nearly all of them.
    /// </remarks>
    internal IReadOnlyList<WritingSection> Sections => Resolve(_ => null);

    /// <summary>
    /// The sections read so far, plus the one still open, with border colours resolved.
    /// </summary>
    /// <remarks>
    /// A <c>\brdrcf</c> is an index into the <c>\colortbl</c>, which the document is free to write
    /// after the border that names it — the same ordering problem <c>\cf</c> has — so the index is
    /// carried and looked up here, once the whole file has been read.
    /// </remarks>
    /// <param name="colour">Resolves a colour table index, answering null for one out of range.</param>
    internal IReadOnlyList<WritingSection> Resolve(Func<int?, Colour?> colour)
    {
        ArgumentNullException.ThrowIfNull(colour);

        List<WritingSection> all = new(_sections.Count + 1);
        foreach (PageDefaults defaults in _sections) all.Add(defaults.ToSection(colour));
        all.Add(_section.ToSection(colour));
        return all;
    }

    /// <summary>
    /// Takes one control word, returning true when it was a geometry word.
    /// </summary>
    /// <remarks>
    /// Returning whether it was consumed lets the caller keep its own dispatch free of thirty cases
    /// that all do the same thing. The parameterless forms matter: <c>\landscape</c> with no parameter
    /// means on, and <c>\landscape0</c> means off, which is RTF's convention for every flag.
    /// </remarks>
    internal bool Handle(string name, int? parameter)
    {
        bool flag = parameter is not 0;

        switch (name)
        {
            // ---- document defaults
            case "paperw": return SetDocument(d => d with { Width = Dimension(parameter) });
            case "paperh": return SetDocument(d => d with { Height = Dimension(parameter) });
            case "margl": return SetDocument(d => d with { Left = Twips(parameter) });
            case "margr": return SetDocument(d => d with { Right = Twips(parameter) });
            case "margt": return SetDocument(d => d with { Top = Twips(parameter) });
            case "margb": return SetDocument(d => d with { Bottom = Twips(parameter) });
            case "gutter": return SetDocument(d => d with { Gutter = Twips(parameter) });
            case "landscape": return SetDocument(d => d with { Landscape = flag });
            case "facingp": return SetDocument(d => d with { Mirrored = flag });

            // \headery and \footery have no section-level counterparts — they are document-wide in
            // RTF, unlike every other margin — so they are set on both scopes at once.
            case "headery": return SetBoth(d => d with { HeaderDistance = Twips(parameter) });
            case "footery": return SetBoth(d => d with { FooterDistance = Twips(parameter) });

            // ---- section overrides
            case "pgwsxn": return SetSection(d => d with { Width = Dimension(parameter) });
            case "pghsxn": return SetSection(d => d with { Height = Dimension(parameter) });
            case "marglsxn": return SetSection(d => d with { Left = Twips(parameter) });
            case "margrsxn": return SetSection(d => d with { Right = Twips(parameter) });
            case "margtsxn": return SetSection(d => d with { Top = Twips(parameter) });
            case "margbsxn": return SetSection(d => d with { Bottom = Twips(parameter) });
            case "guttersxn": return SetSection(d => d with { Gutter = Twips(parameter) });
            case "lndscpsxn": return SetSection(d => d with { Landscape = flag });
            case "facpgsxn": return SetSection(d => d with { Mirrored = flag });
            case "titlepg": return SetSection(d => d with { DifferentFirstPage = flag });

            // How the section starts. \sbknone is the continuous one; \sbkcol starts where the next column
            // would, which for a single-column section is the same page and so behaves as continuous without
            // being it — and in a multi-column section is the break that fills the rest of a column.
            case "sbknone":
                return SetSection(d => d with { Break = SectionBreak.Continuous });
            case "sbkcol":
                return SetSection(d => d with { Break = SectionBreak.NewColumn });
            case "sbkpage":
                return SetSection(d => d with { Break = SectionBreak.NextPage });
            case "sbkeven":
                return SetSection(d => d with { Break = SectionBreak.EvenPage });
            case "sbkodd":
                return SetSection(d => d with { Break = SectionBreak.OddPage });
            case "pgnstarts": return SetSection(d => d with { RestartAt = parameter });

            // A column count of one is the default and is written explicitly by some producers, so it
            // is not treated as absent.
            case "cols":
                return SetSection(d => d with { Columns = parameter is > 0 and < 64 ? parameter.Value : 1 });
            // \rtlsect and \ltrsect, the section's own direction: it reverses the order of the
            // columns and leaves the margins and the paragraphs alone.
            case "rtlsect":
                return SetSection(d => d with { IsRightToLeft = true });
            case "ltrsect":
                return SetSection(d => d with { IsRightToLeft = false });

            case "colsx":
                return SetSection(d => d with { ColumnGap = Twips(parameter) });

            // ---- the page border, which RTF states in two halves like every other border
            case "pgbrdrt": return SelectBorder(0);
            case "pgbrdrl": return SelectBorder(1);
            case "pgbrdrb": return SelectBorder(2);
            case "pgbrdrr": return SelectBorder(3);

            // \pgbrdropt packs three fields; bits 5-7 hold the offset origin, and 1 there is "from the
            // edge of the page". LibreOffice reads only that one value and leaves the default — which is
            // *from the text* — everywhere else (`rtfdispatchvalue.cxx`, RTFKeyword::PGBRDROPT).
            case "pgbrdropt":
                return SetSection(d => d with { BorderFromText = ((parameter ?? 0) & 0xE0) >> 5 != 1 });

            // Whether the border surrounds the header and the footer as well as the body. Writer has no
            // such distinction — its page border is the page's — so these are consumed and dropped
            // rather than left to fall through into the paragraph vocabulary.
            case "pgbrdrhead" or "pgbrdrfoot" or "pgbrdrsnap":
                return true;

            // ---- structure
            case "sectd":
                // Back to the document's defaults, keeping nothing the previous section said. The
                // header distance survives because it never belonged to the section.
                _section = _document;
                _sectionTouched = true;
                _pageBorderSide = null;
                return true;

            case "sect":
                _sections.Add(_section);
                _section = _document;
                _sectionTouched = false;
                _pageBorderSide = null;
                return true;

            default:
                return HandleBorderWord(name, parameter);
        }
    }

    /// <summary>
    /// The words that describe a border, when a page border's side is the one selected.
    /// </summary>
    /// <remarks>
    /// Returns false — so the caller's own dispatch sees them — whenever no page side is selected, and
    /// clears the selection for any word that selects one of the other four border scopes. That is the
    /// whole of what keeps <c>\clbrdrl\brdrw15</c> out of the page's box.
    /// </remarks>
    private bool HandleBorderWord(string name, int? parameter)
    {
        switch (name)
        {
            // Another scope claims the describing words from here on.
            case "clbrdrl" or "clbrdrr" or "clbrdrt" or "clbrdrb"
                or "brdrt" or "brdrl" or "brdrb" or "brdrr" or "brdrbtw"
                or "trbrdrt" or "trbrdrl" or "trbrdrb" or "trbrdrr" or "trbrdrh" or "trbrdrv"
                or "box" or "chbrdr"
                or "pard" or "cell" or "row" or "par":
                _pageBorderSide = null;
                return false;

            default:
                break;
        }

        if (_pageBorderSide is not { } at) return false;

        switch (name)
        {
            // \brdrw is in twips and dmapper wants eighths of a point, with the one-off that 2 twips
            // becomes 1 rather than 0 — `rtfdispatchvalue.cxx`, RTFKeyword::BRDRW, which is where the
            // odd-looking `* 2 / 5` comes from.
            case "brdrw":
            {
                int twips = Math.Max(0, parameter ?? 0);
                int eighths = twips == 2 ? 1 : twips > 1 ? twips * 2 / 5 : 0;
                return SetBorder(at, side => side with { EighthPoints = eighths });
            }

            case "brdrcf":
                return SetBorder(at, side => side with { ColourIndex = parameter });

            // \brsp is in twips and dmapper wants whole points, truncating — the same integer division
            // LibreOffice does, which is why a 15 twip spacing is nothing at all.
            case "brsp":
                return SetBorder(at, side => side with { SpacePoints = Math.Max(0, parameter ?? 0) / 20 });

            case "brdrsh":
                return SetBorder(at, side => side with { Shadow = true });

            default:
                break;
        }

        if (StyleOf(name) is not { } style) return false;

        return SetBorder(at, side => side with { Style = style });
    }

    /// <summary>
    /// The OOXML border style a <c>\brdr*</c> style word names, or null when it names none.
    /// </summary>
    /// <remarks>
    /// <c>rtfdispatchflag.cxx</c>:262-330, which maps each keyword onto an <c>ST_Border</c> token; the
    /// numbers here are <see cref="BorderRules.WordStyleOf"/>'s for the same tokens.
    /// <c>\brdrhair</c> is <em>single</em> and not a style of its own — "brdrhair and brdrs are the
    /// same, brdrw will make a difference" — and <c>\brdrart</c> is an art border, which maps to no
    /// line at all, exactly as an art <c>w:val</c> does.
    /// </remarks>
    private static int? StyleOf(string name) => name switch
    {
        "brdrs" or "brdrhair" => 1,
        "brdrdb" => 3,
        "brdrdot" => 6,
        "brdrdash" => 7,
        "brdrdashd" => 8,
        "brdrdashdd" => 9,
        "brdrtnthsg" => 11,
        "brdrthtnsg" => 12,
        "brdrtnthmg" => 14,
        "brdrthtnmg" => 15,
        "brdrtnthlg" => 17,
        "brdrthtnlg" => 18,
        "brdrdashsm" => 22,
        "brdremboss" => 24,
        "brdrengrave" => 25,
        "brdroutset" => 26,
        "brdrinset" => 27,
        "brdrnone" or "brdrnil" or "brdrtbl" => 255,
        "brdrart" => 0,
        _ => null,
    };

    /// <summary>Selects a page border side for the words that follow it.</summary>
    private bool SelectBorder(int side)
    {
        _pageBorderSide = side;

        // The side exists from the moment it is named, even if nothing describes it: `\pgbrdrt\brdrs`
        // with no width is a hairline top border, not the absence of one.
        return SetSection(d => d.WithBorder(side, d.Border(side)));
    }

    private bool SetBorder(int side, Func<RtfBorderSide, RtfBorderSide> change)
        => SetSection(d => d.WithBorder(side, change(d.Border(side))));

    private bool SetDocument(Func<PageDefaults, PageDefaults> change)
    {
        _document = change(_document);

        // A document-level word before the first \sectd is also the open section's value: RTF's
        // preamble sets both, and a document that never writes \sectd — which is most of them — would
        // otherwise get default geometry.
        if (!_sectionTouched) _section = _document;
        return true;
    }

    private bool SetSection(Func<PageDefaults, PageDefaults> change)
    {
        _section = change(_section);
        _sectionTouched = true;
        return true;
    }

    private bool SetBoth(Func<PageDefaults, PageDefaults> change)
    {
        _document = change(_document);
        _section = change(_section);
        return true;
    }

    private static int? Dimension(int? parameter)
        => parameter is > 0 and <= MaxDimensionTwips ? parameter : null;

    /// <summary>
    /// A twip measurement, keeping a negative one.
    /// </summary>
    /// <remarks>
    /// Negative margins are legal and used: a header that hangs above the page's top edge is written
    /// as a negative <c>\margt</c>, and clamping it moves the body text.
    /// </remarks>
    private static int? Twips(int? parameter) => parameter;

    /// <summary>The room left between the furniture's edge and the body's, never negative.</summary>
    private static Core.Units.Length Gap(Core.Units.Length furnitureEdge, Core.Units.Length bodyEdge)
    {
        Core.Units.Length gap = bodyEdge - furnitureEdge;
        return gap > Core.Units.Length.Zero ? gap : Core.Units.Length.Zero;
    }

    /// <summary>
    /// The geometry as RTF states it, in twips, before conversion.
    /// </summary>
    /// <remarks>
    /// Nullable fields throughout, so "the document did not say" stays distinguishable from "the
    /// document said zero" — a zero margin is meaningful and a missing one has to fall back.
    /// </remarks>
    private readonly record struct PageDefaults(
        int? Width,
        int? Height,
        int? Left,
        int? Right,
        int? Top,
        int? Bottom,
        int? Gutter,
        int? HeaderDistance,
        int? FooterDistance,
        int Columns,
        int? ColumnGap,
        bool IsRightToLeft,
        bool Landscape,
        bool Mirrored,
        bool DifferentFirstPage,
        int? RestartAt,
        SectionBreak Break)
    {
        /// <summary>Nothing stated yet, and one column.</summary>
        internal static PageDefaults Initial { get; } = new() { Columns = 1, BorderFromText = true };

        /// <summary>The four page border sides, in top, left, bottom, right order.</summary>
        /// <remarks>
        /// Four separate fields rather than an array because <see cref="PageDefaults"/> is a value
        /// type whose whole point is that <c>\sectd</c> can restore a section by assignment: an array
        /// would be shared between the document's defaults and every section copied from them, so a
        /// border stated in one section would appear in all of them.
        /// </remarks>
        RtfBorderSide BorderTop { get; init; }

        RtfBorderSide BorderLeft { get; init; }

        RtfBorderSide BorderBottom { get; init; }

        RtfBorderSide BorderRight { get; init; }

        /// <summary>True when the border is measured from the text rather than the paper's edge.</summary>
        /// <remarks>
        /// RTF's default, and the opposite of DOCX's: a file that states no <c>\pgbrdropt</c> means
        /// from the text.
        /// </remarks>
        internal bool BorderFromText { get; init; }

        internal RtfBorderSide Border(int side) => side switch
        {
            0 => BorderTop,
            1 => BorderLeft,
            2 => BorderBottom,
            _ => BorderRight,
        };

        internal PageDefaults WithBorder(int side, RtfBorderSide value) => side switch
        {
            0 => this with { BorderTop = value },
            1 => this with { BorderLeft = value },
            2 => this with { BorderBottom = value },
            _ => this with { BorderRight = value },
        };

        internal WritingSection ToSection(Func<int?, Colour?> colour)
        {
            // RTF and DOCX share `SectionPropertyMap`, so they share its defaults: Letter with
            // one-inch margins, whatever the machine's own default paper is. See
            // `PageGeometry.Letter`.
            PageMargins fallback = PageGeometry.Letter.Margins;

            Core.Units.Length top = Top is { } t ? Length.FromTwips(t) : fallback.Top;
            Core.Units.Length bottom = Bottom is { } b ? Length.FromTwips(b) : fallback.Bottom;
            Core.Units.Length headerDistance =
                HeaderDistance is { } hd ? Length.FromTwips(hd) : Core.Units.Length.Zero;
            Core.Units.Length footerDistance =
                FooterDistance is { } fd ? Length.FromTwips(fd) : Core.Units.Length.Zero;

            return new WritingSection
            {
                Page = new PageGeometry
                {
                    // Fitted to the nearest standard paper dimension when it is within 0.44 mm of
                    // one. RTF reaches the same rule as DOCX rather than a parallel one:
                    // `rtfdispatchvalue.cxx`:1274-1289 dispatches \paperw, \paperh, \pgwsxn and
                    // \pghsxn through `LN_CT_PageSz_w`/`_h`, which is where `DomainMapper` applies
                    // the fit. See `Model.PaperSizes`. The corpus's words track holds no RTF, so
                    // this arm is reasoned rather than measured, and is written that way on purpose:
                    // an unfitted RTF page would be the one Word-family reader out of step.
                    Size = new DocSize(
                        Width is { } w
                            ? Model.PaperSizes.SloppyFit(Length.FromTwips(w))
                            : PageGeometry.Letter.Size.Width,
                        Height is { } h
                            ? Model.PaperSizes.SloppyFit(Length.FromTwips(h))
                            : PageGeometry.Letter.Size.Height),
                    Margins = new PageMargins(
                        Left is { } l ? Length.FromTwips(l) : fallback.Left,
                        Right is { } r ? Length.FromTwips(r) : fallback.Right,
                        top,
                        bottom),
                    Gutter = Gutter is { } g ? Length.FromTwips(g) : Length.Zero,
                    HeaderDistance = headerDistance,
                    FooterDistance = footerDistance,

                    // \margt is the body's top margin and \headery the header's distance from the
                    // page edge, exactly as in DOCX and DOC, so the header's height is the gap.
                    HeaderHeight = Gap(headerDistance, top),
                    FooterHeight = Gap(footerDistance, bottom),
                    Columns = Columns > 0 ? Columns : 1,
                    ColumnGap = ColumnGap is { } cg ? Length.FromTwips(cg) : Length.Zero,
                    IsRightToLeft = IsRightToLeft,
                    IsLandscape = Landscape,
                    HasMirroredMargins = Mirrored,
                    Borders = Borders(colour),
                },
                RestartPageNumberAt = RestartAt,
                HasDifferentFirstPage = DifferentFirstPage,
                Break = Break,
            };
        }

        /// <summary>
        /// The page border the four sides come to, or null when none of them draws.
        /// </summary>
        /// <remarks>
        /// The shadow is the <em>right</em> side's, as it is in DOCX and DOC: RTF puts <c>\brdrsh</c>
        /// on each side and Writer reads only <c>m_bBorderShadows[BORDER_RIGHT]</c>, taking that side's
        /// own width for the offset.
        /// </remarks>
        private PageBorders? Borders(Func<int?, Colour?> colour)
        {
            PageBorderSide right = BorderRight.Resolve(colour);

            PageBorders borders = new()
            {
                Top = BorderTop.Resolve(colour),
                Left = BorderLeft.Resolve(colour),
                Bottom = BorderBottom.Resolve(colour),
                Right = right,
                OffsetFromText = BorderFromText,
                Shadow = BorderRight.Shadow ? right.Width : Core.Units.Length.Zero,
            };

            return borders.Draws ? borders : null;
        }
    }

    /// <summary>
    /// One page border side as RTF states it: an unresolved style, width, colour index and distance.
    /// </summary>
    /// <remarks>
    /// Kept unresolved because RTF states the four in any order and may state the colour table after
    /// them, so the side cannot become a <see cref="PageBorderSide"/> until the file has been read.
    /// </remarks>
    /// <param name="Style">
    /// The OOXML border style number the <c>\brdr*</c> word named — 0 for none stated, which is also
    /// what an art border comes to.
    /// </param>
    /// <param name="EighthPoints">The <c>\brdrw</c> width, converted to OOXML's eighths of a point.</param>
    /// <param name="ColourIndex">The <c>\brdrcf</c> index into the colour table.</param>
    /// <param name="SpacePoints">The <c>\brsp</c> distance, in whole points.</param>
    /// <param name="Shadow">Whether <c>\brdrsh</c> asked this side for a shadow.</param>
    private readonly record struct RtfBorderSide(
        int Style,
        int EighthPoints,
        int? ColourIndex,
        int SpacePoints,
        bool Shadow)
    {
        /// <summary>The side as the model wants it, or a side that draws nothing.</summary>
        internal PageBorderSide Resolve(Func<int?, Colour?> colour)
        {
            // A side that was selected and given a width but never a style is a plain single rule:
            // `\pgbrdrt\brdrw90` alone is what several producers write.
            int style = Style == 0 && EighthPoints > 0 ? 1 : Style;

            Core.Units.Length width = EighthPoints > 0
                ? Core.Units.Length.FromPoints(EighthPoints / 8.0)
                : Core.Units.Length.FromPoints(0.5);

            if (BorderRules.FromWord(style, width) is not { } rule) return default;

            return new PageBorderSide(
                rule.Width,
                colour(ColourIndex) ?? Colour.Black,
                Core.Units.Length.FromPoints(Math.Min(SpacePoints, 31)));
        }
    }
}
