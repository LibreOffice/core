using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.WordProcessing.Model;

namespace Paperless.WordProcessing.Rtf;

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

    private readonly List<WritingSection> _sections = [];

    private PageDefaults _document = PageDefaults.Initial;
    private PageDefaults _section = PageDefaults.Initial;
    private bool _sectionTouched;

    /// <summary>
    /// The sections read so far, plus the one still open.
    /// </summary>
    /// <remarks>
    /// The open one is included because RTF's last section has no <c>\sect</c> — the document simply
    /// ends. A reader that only collected closed sections would lose the geometry of every single-section
    /// document, which is nearly all of them.
    /// </remarks>
    internal IReadOnlyList<WritingSection> Sections
    {
        get
        {
            List<WritingSection> all = [.. _sections, Current()];
            return all;
        }
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
            case "colsx":
                return SetSection(d => d with { ColumnGap = Twips(parameter) });

            // ---- structure
            case "sectd":
                // Back to the document's defaults, keeping nothing the previous section said. The
                // header distance survives because it never belonged to the section.
                _section = _document;
                _sectionTouched = true;
                return true;

            case "sect":
                _sections.Add(Current());
                _section = _document;
                _sectionTouched = false;
                return true;

            default:
                return false;
        }
    }

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

    private WritingSection Current() => _section.ToSection();

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
        bool Landscape,
        bool Mirrored,
        bool DifferentFirstPage,
        int? RestartAt,
        SectionBreak Break)
    {
        /// <summary>Nothing stated yet, and one column.</summary>
        internal static PageDefaults Initial { get; } = new() { Columns = 1 };

        internal WritingSection ToSection()
        {
            PageMargins fallback = PageMargins.Default;

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
                    Size = new DocSize(
                        Width is { } w ? Length.FromTwips(w) : PageGeometry.Default.Size.Width,
                        Height is { } h ? Length.FromTwips(h) : PageGeometry.Default.Size.Height),
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
                    IsLandscape = Landscape,
                    HasMirroredMargins = Mirrored,
                },
                RestartPageNumberAt = RestartAt,
                HasDifferentFirstPage = DifferentFirstPage,
                Break = Break,
            };
        }
    }
}
