using System.Xml.Linq;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Text.Fonts;

namespace XRay.Office.WordProcessing.Ooxml;

/// <summary>
/// The character formatting in force for a run, resolved across every WordprocessingML layer.
/// </summary>
/// <remarks>
/// Only what extraction can act on is surfaced. The resolution behind it is the interesting
/// part — see <see cref="WordStyles.ResolveRunProperty"/> for the toggle rule that decides
/// whether bold inside a bold style is bold.
/// </remarks>
public sealed record WordCharacterFormat
{
    /// <summary>Nothing resolved.</summary>
    public static readonly WordCharacterFormat None = new();

    /// <summary>True when the run is bold.</summary>
    public bool IsBold { get; init; }

    /// <summary>True when the run is italic.</summary>
    public bool IsItalic { get; init; }

    /// <summary>How the run is underlined, if it is.</summary>
    /// <remarks>
    /// <c>ST_Underline</c> (ECMA-376 17.18.99) states the line's style and its count in one token,
    /// and only the count reaches the geometry: <c>double</c> and <c>wavyDouble</c> are the two
    /// two-line values and every other non-<c>none</c> value is one line.
    /// </remarks>
    public TextUnderline Underline { get; init; }

    /// <summary>True when the run is underlined by any underline style.</summary>
    public bool IsUnderlined => Underline != TextUnderline.None;

    /// <summary>True when the run is struck through, single or double.</summary>
    public bool IsStruckThrough { get; init; }

    /// <summary>True when the run is raised.</summary>
    public bool IsSuperscript { get; init; }

    /// <summary>True when the run is lowered.</summary>
    public bool IsSubscript { get; init; }

    /// <summary>True when the run is in small capitals or all capitals.</summary>
    public bool IsCapitalised { get; init; }

    /// <summary>
    /// True when the run is marked hidden (<c>w:vanish</c>), which means no reader displays it.
    /// </summary>
    public bool IsHidden { get; init; }

    /// <summary>The font size, when a layer sets one.</summary>
    public Length? FontSize { get; init; }

    /// <summary>The Latin-script font name, when a layer sets one.</summary>
    public string? FontName { get; init; }

    /// <summary>The text colour, when a layer sets one and it is not "automatic".</summary>
    public Colour? Colour { get; init; }

    /// <summary>The run's language as a BCP 47 tag, which is how OOXML already stores it.</summary>
    public string? Language { get; init; }

    /// <summary>The coarse emphasis flags the content tree records.</summary>
    public RunEmphasis Emphasis
    {
        get
        {
            RunEmphasis emphasis = RunEmphasis.None;
            if (IsBold) emphasis |= RunEmphasis.Bold;
            if (IsItalic) emphasis |= RunEmphasis.Italic;
            if (IsUnderlined) emphasis |= RunEmphasis.Underline;
            if (IsStruckThrough) emphasis |= RunEmphasis.Strikethrough;
            if (IsSuperscript) emphasis |= RunEmphasis.Superscript;
            if (IsSubscript) emphasis |= RunEmphasis.Subscript;
            return emphasis;
        }
    }

    /// <summary>
    /// Resolves the formatting of a run.
    /// </summary>
    /// <param name="styles">The document's styles.</param>
    /// <param name="directRunProperties">The run's own <c>w:rPr</c>, or null.</param>
    /// <param name="paragraphStyleId">The paragraph style in force, or null.</param>
    /// <param name="theme">
    /// The document's theme, for a <c>w:themeColor</c>. Null leaves a themed colour unresolved
    /// rather than guessing at it, which is what a package with no theme part deserves.
    /// </param>
    public static WordCharacterFormat Resolve(
        WordStyles styles,
        XElement? directRunProperties,
        string? paragraphStyleId,
        DrawingTheme? theme = null)
    {
        ArgumentNullException.ThrowIfNull(styles);

        // The character style is named by the run itself, inside the very property set being
        // resolved — so it has to be read before anything else can be.
        string? characterStyleId = Word.Value(directRunProperties, "rStyle");

        WordProperty verticalAlignment = Get("vertAlign");
        WordProperty size = Get("sz");
        WordProperty colour = Get("color");

        return new WordCharacterFormat
        {
            IsBold = Get("b").IsOn,
            IsItalic = Get("i").IsOn,
            // w:u carries the line style in w:val, so "none" is off and anything else is on.
            Underline = UnderlineOf(styles.RunPropertyLayers(
                "u", directRunProperties, paragraphStyleId, characterStyleId)),
            IsStruckThrough = Get("strike").IsOn || Get("dstrike").IsOn,
            IsSuperscript = verticalAlignment.Value == "superscript",
            IsSubscript = verticalAlignment.Value == "subscript",
            IsCapitalised = Get("caps").IsOn || Get("smallCaps").IsOn,
            IsHidden = Get("vanish").IsOn,
            // w:sz is in half-points, which is the unit that catches people out: 24 is 12pt.
            FontSize = size.IntegerValue is { } halfPoints and > 0
                ? Length.FromPoints(halfPoints / 2.0)
                : null,
            // Every layer, not the innermost one: w:rFonts is inherited attribute by attribute, so
            // a run naming only a complex-script face still has its style's Latin one.
            FontName = LatinFamily(
                styles.RunPropertyLayers(
                    "rFonts", directRunProperties, paragraphStyleId, characterStyleId),
                theme),
            Colour = WordThemeColour.Read(colour.Element, theme),
            Language = Get("lang").Value,
        };

        WordProperty Get(string localName)
            => styles.ResolveRunProperty(
                localName, directRunProperties, paragraphStyleId, characterStyleId);
    }

    /// <summary>
    /// Which of <c>ST_Underline</c>'s values a <c>w:u</c> asks for.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>DomainMapper::handleUnderlineType</c>
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:5046-5106) maps ST_Underline's
    /// eighteen values onto nine <c>FontLineStyle</c>s, and the two distinctions this engine keeps
    /// are the ones that change the geometry: how many lines, and how thick.
    /// </para>
    /// <para>
    /// <c>double</c> and <c>wavyDouble</c> draw two. <c>thick</c> and the six heavy forms —
    /// <c>dottedHeavy</c>, <c>dashedHeavy</c>, <c>dashLongHeavy</c>, <c>dashDotHeavy</c>,
    /// <c>dashDotDotHeavy</c>, <c>wavyHeavy</c> — draw one at about twice the weight. The other
    /// nine draw one ordinary line.
    /// </para>
    /// <para>
    /// <strong>A <c>w:u</c> stating no <c>w:val</c> is not an underline, and it does not turn one
    /// off either — it is transparent.</strong> <c>CT_Underline</c>'s <c>val</c> is optional with no
    /// default (<c>sw/source/writerfilter/ooxml/model.xml</c>:17021-17027), so an element without it
    /// emits no <c>LN_CT_Underline_val</c> at all and <c>DomainMapper::lcl_sprm</c>'s case for it
    /// (<c>dmapper/DomainMapper.cxx</c>:364) never fires; the sibling case at <c>:368</c> takes the
    /// <c>w:color</c> and nothing else. So the run keeps whatever an outer layer stated, which is why
    /// this searches the layers for the innermost one carrying a <c>val</c> rather than taking the
    /// innermost <c>w:u</c> and reading its absent one as <c>single</c>.
    /// </para>
    /// <para>
    /// <strong>Reading it as <c>single</c> cost a whole document's ink.</strong>
    /// <c>f445896eb008d14c1746fc37d412dc22.docx</c> states <c>&lt;w:u w:color="000000"/&gt;</c> on four
    /// of its styles and <c>&lt;w:u w:color="666666"/&gt;</c> on a run, with no <c>val</c> anywhere, and
    /// this tree underlined every word of it: 108 filled rules per page against 26.2.4.2's none, on
    /// fourteen of its fifteen pages, at identical text, identical faces and identical line breaks.
    /// The gate cannot see an underline. <c>probes/underline-r176</c>.
    /// </para>
    /// </remarks>
    /// <param name="layers">
    /// Every <c>w:u</c> the run resolves through, innermost first — see
    /// <see cref="WordStyles.RunPropertyLayers"/>.
    /// </param>
    internal static TextUnderline UnderlineOf(IReadOnlyList<XElement> layers)
    {
        ArgumentNullException.ThrowIfNull(layers);

        foreach (XElement layer in layers)
        {
            if (layer.Attribute(Word.Name("val"))?.Value is not { Length: > 0 } stated) continue;

            return stated switch
            {
                "none" => TextUnderline.None,
                "double" or "wavyDouble" => TextUnderline.DoubleLine,
                "thick" or "dottedHeavy" or "dashedHeavy" or "dashLongHeavy" or "dashDotHeavy"
                    or "dashDotDotHeavy" or "wavyHeavy" => TextUnderline.BoldLine,
                _ => TextUnderline.SingleLine,
            };
        }

        return TextUnderline.None;
    }

    /// <summary>
    /// The Latin family the innermost layer to name one gives, or null when none does.
    /// </summary>
    /// <remarks>
    /// Only the ASCII slot, unlike the layout path's search: this is reported as "the font", and a
    /// document whose runs name only an East Asian face has no Latin font to report rather than an
    /// East Asian one masquerading as one.
    /// </remarks>
    private static string? LatinFamily(List<XElement> layers, DrawingTheme? theme)
    {
        foreach (XElement fonts in layers)
        {
            if (WordParagraphFormats.SlotFamily(fonts, theme?.Fonts, "ascii", "asciiTheme") is { } name)
            {
                return name;
            }
        }

        return null;
    }
}
