using System.Xml.Linq;
using XRay.Office.Core.Units;

namespace XRay.Office.Ooxml.OfficeMath;

/// <summary>
/// How tall a formula is, and where its baseline sits inside that — StarMath's answer, because
/// the reference's answer <em>is</em> StarMath's.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice does not lay an OMML formula out as text. Its importer builds a StarMath document
/// from the subtree and embeds it as an OLE object anchored as-character, so the line the formula
/// sits on takes the <em>object's</em> height — the <c>svg:height</c> of the <c>draw:frame</c>
/// that <c>--convert-to fodt</c> prints. A reader that emits the <c>m:t</c> leaves as ordinary
/// text gives the line its own font's height instead, which is right for a bare letter and short
/// by 15 pt for a fraction.
/// </para>
/// <para>
/// <strong>Two laws decide it, both measured on 26.2.4.2 rather than derived.</strong> The line's
/// height is exactly the object's <c>svg:height</c> — over ten shapes the difference between the
/// reference's paragraph pitch and that attribute is a constant 37.45 ± 0.02 pt, which is the two
/// plain paragraphs either side of it. And <strong><c>w:sz</c> is ignored</strong>: StarMath lays
/// the formula out at <em>its own</em> base size, 12 pt (<c>SmFormat::SmFormat</c>,
/// <c>starmath/source/format.cxx</c>:25), so <c>x</c> at 8 pt, 12 pt and 20 pt all come back
/// 13.349 pt tall. <c>probes/blankpage-r163/results.md</c> §4.
/// </para>
/// <para>
/// <strong>What is modelled is the vertical half of <c>SmNode::Arrange</c> and nothing else.</strong>
/// Widths, positions and the glyphs themselves are the drawing path's business; this exists to
/// answer how much room a line has to reserve. The arithmetic is in hundredths of a millimetre
/// with integer division, because that is <c>SmO3tlLengthUnit</c> and those divisions are the
/// reference's own — writing it in floating point loses the ±1 unit that separates
/// <c>frac-nest-num</c> from <c>frac-nest3</c>.
/// </para>
/// <para>
/// <strong>Scored against 26.2.4.2's own <c>svg:height</c> over 53 one-construct fixtures</strong>
/// (<c>probes/mathheight-r165/</c>, built from the real document's parts so they inherit its
/// compatibility defaults): worst |Δ| <strong>0.369 pt</strong>, 51 of 53 within 0.25 pt, 18 exact.
/// The two outliers are both nested radicals, where the sign's own glyph box is approximated by
/// the height <c>AdaptToY</c> asks for.
/// </para>
/// <para>
/// <strong>What the corpus actually states</strong>, counted over the six documents that hold any
/// OMML at all: <c>m:sSub</c> 156, <c>m:f</c> 55, <c>m:rad</c> 10, <c>m:d</c> 10, <c>m:sSup</c> 6,
/// <c>m:func</c> 1 — and no n-ary, matrix, accent or limit anywhere. Those six are the arms that
/// carry the corpus; the rest are modelled because the fixtures measure them and a document
/// outside the corpus will state one.
/// </para>
/// </remarks>
public static class OfficeMathBox
{
    /// <summary>Hundredths of a millimetre in one point — the unit StarMath computes in.</summary>
    private const double Mm100PerPoint = 2540.0 / 72.0;

    /// <summary>
    /// <c>SmFormat</c>'s base size: 12 pt, in hundredths of a millimetre.
    /// </summary>
    /// <remarks>
    /// <c>aBaseSize(0, o3tl::convert(12, o3tl::Length::pt, SmO3tlLengthUnit()))</c>,
    /// <c>starmath/source/format.cxx</c>:25. The document's own <c>w:sz</c> never reaches it.
    /// </remarks>
    private const int BaseSize = 423;

    // The `SmFormat` defaults, `starmath/source/format.cxx`:31-60, as percentages.
    private const int RelIndex = 60;          // SIZ_INDEX — a sub- or superscript's size
    private const int RelLimits = 60;         // SIZ_LIMITS — an n-ary's limits
    private const int DistVertical = 5;       // DIS_VERTICAL — between the rows of a stack
    private const int DistScript = 20;        // DIS_SUBSCRIPT and DIS_SUPERSCRIPT
    private const int DistStrokeWidth = 5;    // DIS_STROKEWIDTH — a fraction's rule
    private const int DistLimit = 0;          // DIS_UPPERLIMIT and DIS_LOWERLIMIT
    private const int DistBracketSize = 5;    // DIS_BRACKETSIZE — a scaled bracket's oversize
    private const int DistMatrixRow = 3;      // DIS_MATRIXROW
    private const int DistOperatorSize = 50;  // DIS_OPERATORSIZE
    private const int DistRoot = 0;           // DIS_ROOT

    /// <summary>
    /// A row of ordinary maths text at the base size, in hundredths of a millimetre.
    /// </summary>
    /// <remarks>
    /// <c>SmTextNode::Arrange</c> takes <c>OutputDevice::GetTextHeight()</c> of the resolved face
    /// (<c>Times New Roman</c>, which resolves to Liberation Serif here), and the reference answers
    /// <strong>471</strong> for it at 12 pt — measured on nine one-run fixtures whose text is a
    /// letter, a capital, a digit, a Greek letter with a descender and a sum, all 471. Liberation
    /// Serif's own <c>hhea</c> predicts 468, and the three units between them are the device the
    /// object is formatted on; taking the measured value keeps the whole ladder on the reference's
    /// number rather than three units under it at every level.
    /// </remarks>
    private const int TextHeight = 471;

    /// <summary>That row's ascent.</summary>
    /// <remarks>
    /// Read off the <c>sup</c> arm rather than assumed: a superscript's top clears the base's by
    /// <c>alignT − scriptAlignT − nDist</c>, which pins the ascent at 376 and nothing else.
    /// Liberation Serif's <c>hhea</c> ascender predicts 377.
    /// </remarks>
    private const int TextAscent = 376;

    /// <summary>The vertical extent of a formula, and where its baseline sits inside it.</summary>
    /// <param name="Height">Top to bottom.</param>
    /// <param name="Ascent">From the top down to the formula's own baseline.</param>
    public readonly record struct Extent(Length Height, Length Ascent);

    /// <summary>
    /// Measures an <c>m:oMath</c> or <c>m:oMathPara</c>, or returns null for anything else.
    /// </summary>
    public static Extent? Measure(XElement? formula)
    {
        if (formula is null || formula.Name.NamespaceName != OoxmlNamespaces.OfficeMath) return null;
        if (formula.Name.LocalName is not ("oMath" or "oMathPara")) return null;

        Rect body = Row(formula.Elements(), BaseSize);
        if (body.Height <= 0) return null;

        return new Extent(Length.FromMm100(body.Height), Length.FromMm100(Baseline(body) - body.Top));
    }

    /// <summary>
    /// The formula's own baseline — <c>SmTableNode</c>'s <c>mnFormulaBaseline</c>, node.cxx:532-545.
    /// </summary>
    /// <remarks>
    /// A formula whose top node has a baseline of its own (a row of text, a script) states it. One
    /// that has not — a fraction, a stack — is hung from its middle instead, offset by the gap
    /// between a single letter's baseline and its own middle, which is what keeps a lone fraction's
    /// bar on the surrounding text's maths axis.
    /// </remarks>
    private static int Baseline(Rect rect)
        => rect.HasBaseline ? rect.Base : rect.AlignM + (TextAscent - AlignMiddle(BaseSize));

    /// <summary>
    /// <c>nAlignM = nBaseline - nFontHeight * 121 / 422</c> — <c>rect.cxx</c>:188, where the horizontal
    /// bars of <c>+</c> and <c>-</c> sit.
    /// </summary>
    private static int AlignMiddle(int fontHeight) => TextAscentAt(fontHeight) - fontHeight * 121 / 422;

    private static int TextAscentAt(int fontHeight)
        => (int)Math.Round((double)fontHeight * TextAscent / BaseSize);

    private static int TextHeightAt(int fontHeight)
        => (int)Math.Round((double)fontHeight * TextHeight / BaseSize);

    /// <summary>
    /// The vertical half of an <c>SmRect</c>: its extent, its three alignment fences and its baseline.
    /// </summary>
    /// <remarks>
    /// <c>AlignT</c> and <c>AlignB</c> are what a script is hung from and what <c>ExtendBy</c> unions;
    /// <c>AlignM</c> is what two rects fall back to when one of them has no baseline. Keeping all four
    /// is not over-modelling — a fraction has no baseline, so <c>frac(a,b)</c> with a subscript is
    /// placed from <c>AlignB</c> and would sit 4 pt wrong if the baseline stood in for it.
    /// </remarks>
    private readonly record struct Rect(
        int Top, int Bottom, int AlignT, int AlignB, int AlignM, int Base, bool HasBaseline)
    {
        public int Height => Bottom - Top;

        public Rect Moved(int dy)
            => new(Top + dy, Bottom + dy, AlignT + dy, AlignB + dy, AlignM + dy, Base + dy, HasBaseline);

        /// <summary>The empty rect, which <c>ExtendBy</c> treats as "no information".</summary>
        public bool IsNothing => Bottom == Top && AlignT == 0 && AlignB == 0;
    }

    /// <summary>A row of text: the leaf of every formula.</summary>
    private static Rect Leaf(int fontHeight)
    {
        int ascent = TextAscentAt(fontHeight);
        return new Rect(
            0, TextHeightAt(fontHeight),
            ascent - fontHeight * 750 / 1000, ascent, AlignMiddle(fontHeight), ascent, true);
    }

    /// <summary>
    /// A rect with no alignment information of its own — <c>SmRect(nWidth, nHeight)</c>, rect.cxx:262,
    /// which is what a fraction's rule and a scaled bracket are.
    /// </summary>
    private static Rect Blank(int height) => new(0, height, 0, height, height / 2, 0, false);

    /// <summary>The whole of <c>SmRect::ExtendBy</c>'s vertical half — rect.cxx:457.</summary>
    /// <param name="self">The rect being extended.</param>
    /// <param name="other">The rect it is extended by.</param>
    /// <param name="mode">Whose baseline the union keeps — <c>RectCopyMBL</c>.</param>
    /// <param name="keep">
    /// <c>bKeepVerAlignParams</c>: a sub- or superscript may widen the union without moving the fences
    /// its siblings are hung from (rect.cxx:521).
    /// </param>
    private static Rect Extend(Rect self, Rect other, BaselineOf mode, bool keep = false)
    {
        if (other.IsNothing) return self;
        if (self.IsNothing) return other;

        int top = Math.Min(self.Top, other.Top);
        int bottom = Math.Max(self.Bottom, other.Bottom);
        int alignT = Math.Min(self.AlignT, other.AlignT);
        int alignB = Math.Max(self.AlignB, other.AlignB);
        int middle = self.AlignM;
        int baseline = self.Base;
        bool hasBaseline = self.HasBaseline;

        switch (mode)
        {
            case BaselineOf.Argument:
                baseline = other.Base;
                hasBaseline = other.HasBaseline;
                middle = other.AlignM;
                break;
            case BaselineOf.Neither:
                hasBaseline = false;
                middle = (alignT + alignB) / 2;
                break;
        }

        if (keep)
        {
            alignT = self.AlignT;
            alignB = self.AlignB;
            middle = self.AlignM;
            baseline = self.Base;
            hasBaseline = self.HasBaseline;
        }

        return new Rect(top, bottom, alignT, alignB, middle, baseline, hasBaseline);
    }

    private enum BaselineOf { This, Argument, Neither }

    // ---- the constructs ---------------------------------------------------

    /// <summary>
    /// One row, every part hung from the baseline where both have one and from the middle otherwise —
    /// <c>SmLineNode::Arrange</c>, node.cxx:578.
    /// </summary>
    private static Rect Row(IEnumerable<XElement> parts, int fontHeight)
    {
        Rect row = default;
        bool any = false;

        foreach (XElement part in parts)
        {
            if (Skipped(part)) continue;
            Rect next = Node(part, fontHeight);
            if (next.Height <= 0) continue;

            if (!any) { row = next; any = true; continue; }

            int dy = row.HasBaseline && next.HasBaseline
                ? row.Base - next.Base
                : row.AlignM - next.AlignM;
            row = Extend(row, next.Moved(dy), BaselineOf.This);
        }

        return any ? row : default;
    }

    /// <summary>The properties elements, which state formatting and occupy no room.</summary>
    private static bool Skipped(XElement element)
        => element.Name.NamespaceName != OoxmlNamespaces.OfficeMath
           || element.Name.LocalName.EndsWith("Pr", StringComparison.Ordinal)
           || element.Name.LocalName is "mcs" or "mc" or "argSz";

    private static Rect Node(XElement element, int fontHeight)
    {
        string name = element.Name.LocalName;
        return name switch
        {
            "r" or "t" => Leaf(fontHeight),
            "sSub" => Script(element, fontHeight, sub: "sub"),
            "sSup" => Script(element, fontHeight, sup: "sup"),
            "sSubSup" or "sPre" => Script(element, fontHeight, sub: "sub", sup: "sup"),
            "f" => Fraction(element, fontHeight),
            "rad" => Radical(element, fontHeight),
            "d" => Delimiter(element, fontHeight),
            "nary" => Nary(element, fontHeight),
            "m" => Matrix(element, fontHeight),
            "eqArr" => Stack([.. Parts(element, "e")], fontHeight),
            "limLow" => Limit(element, fontHeight, above: false),
            "limUpp" => Limit(element, fontHeight, above: true),
            "groupChr" => Limit(element, fontHeight, above: false, bracket: true),
            _ => Row(element.Elements(), fontHeight),
        };
    }

    private static IEnumerable<XElement> Parts(XElement element, string name)
        => element.Elements(XName.Get(name, OoxmlNamespaces.OfficeMath));

    private static XElement? Part(XElement element, string name)
        => element.Element(XName.Get(name, OoxmlNamespaces.OfficeMath));

    private static Rect Of(XElement? part, int fontHeight)
        => part is null ? default : Row(part.Elements(), fontHeight);

    /// <summary>The value of an OMML property — stated as <c>m:val</c>, never <c>w:val</c>.</summary>
    private static string? Value(XElement? properties, string name)
        => properties is null
            ? null
            : Part(properties, name)?.Attribute(XName.Get("val", OoxmlNamespaces.OfficeMath))?.Value;

    /// <summary>
    /// An OMML boolean: stated as <c>m:val</c>, and true by its mere presence when it states nothing.
    /// </summary>
    private static bool Flag(XElement? properties, string name)
    {
        if (properties is null) return false;
        if (Part(properties, name) is not { } child) return false;

        string? value = child.Attribute(XName.Get("val", OoxmlNamespaces.OfficeMath))?.Value;
        return value is null || value is not ("0" or "false" or "off");
    }

    /// <summary>
    /// <c>SmSubSupNode::Arrange</c> — node.cxx:1143. A script is set at
    /// <see cref="RelIndex"/> of the body's size, hung from the body's own top or bottom fence and
    /// then clamped so that it does not cross the line 40 % of the way up.
    /// </summary>
    private static Rect Script(XElement element, int fontHeight, string? sub = null, string? sup = null)
    {
        Rect body = Of(Part(element, "e"), fontHeight);
        if (body.Height <= 0) return default;

        // "prevent sub-/supscripts from diminishing in size" — node.cxx:1184.
        int scriptHeight = fontHeight > BaseSize / 3 ? fontHeight * RelIndex / 100 : fontHeight;
        Rect result = body;
        int delimiter = body.AlignB + (int)(0.4 * (body.AlignT - body.AlignB));

        if (sup is not null && Of(Part(element, sup), scriptHeight) is { Height: > 0 } above)
        {
            int y = body.AlignT - above.AlignT - fontHeight * DistScript / 100;
            if (y + above.Height > delimiter) y = delimiter - above.Height;
            result = Extend(result, above.Moved(y), BaselineOf.This, keep: true);
        }

        if (sub is not null && Of(Part(element, sub), scriptHeight) is { Height: > 0 } below)
        {
            int y = body.AlignB - below.AlignB + fontHeight * DistScript / 100;
            if (y < delimiter) y = delimiter;
            result = Extend(result, below.Moved(y), BaselineOf.This, keep: true);
        }

        return result;
    }

    /// <summary>
    /// <c>SmBinVerNode::Arrange</c> — node.cxx:831. The numerator sits on the rule and the
    /// denominator hangs from it, with <c>DIS_NUMERATOR</c> and <c>DIS_DENOMINATOR</c> both nought.
    /// </summary>
    /// <remarks>
    /// The rule is not a hairline: <c>SmRectangleNode::Arrange</c> adds its font's border width on
    /// each side (node.cxx:1799), so the gap between the terms is
    /// <c>H×5/100 + 2×(H/20)</c> — 63 hundredths of a millimetre at the base size, against the 62
    /// the reference reports, the difference being one unit of its own integer arithmetic.
    /// </remarks>
    private static Rect Fraction(XElement element, int fontHeight)
    {
        XElement? properties = Part(element, "fPr");
        string kind = Value(properties, "type") ?? "bar";

        Rect numerator = Of(Part(element, "num"), fontHeight);
        Rect denominator = Of(Part(element, "den"), fontHeight);
        if (numerator.Height <= 0 && denominator.Height <= 0) return default;

        // A linear fraction is written `a/b` on one line and a stacked one with no rule is a
        // two-row stack, which is `SmTableNode`'s spacing rather than a rule's.
        if (kind == "lin") return Row(new[] { Part(element, "num"), Part(element, "den") }
                                          .Where(p => p is not null).SelectMany(p => p!.Elements()),
                                      fontHeight);
        if (kind == "noBar") return Stack([Part(element, "num"), Part(element, "den")], fontHeight);

        Rect rule = Blank(fontHeight * DistStrokeWidth / 100 + 2 * (fontHeight / 20));
        rule = rule.Moved(numerator.Bottom - rule.Top);
        Rect below = denominator.Moved(rule.Bottom - denominator.Top);

        return Extend(Extend(numerator, below, BaselineOf.Neither), rule, BaselineOf.Neither)
            with { AlignM = (rule.Top + rule.Bottom) / 2 };
    }

    /// <summary>
    /// <c>SmRootNode::Arrange</c> — node.cxx:725, with <c>lcl_GetHeightVerOffset</c>.
    /// </summary>
    /// <remarks>
    /// The sign is as tall as the radicand less half the radicand's own descender, plus the tenth
    /// <c>SmRootSymbolNode::AdaptToY</c> adds (node.cxx:1765), and its foot sits that same half
    /// descender above the radicand's. A degree is placed 52 % of the way down the sign
    /// (<c>lcl_GetExtraPos</c>).
    /// </remarks>
    private static Rect Radical(XElement element, int fontHeight)
    {
        Rect body = Of(Part(element, "e"), fontHeight);
        if (body.Height <= 0) return default;

        int offset = (body.Bottom - 1 - body.AlignB) / 2;
        int wanted = body.Height - offset + DistRoot * fontHeight / 100;
        Rect sign = Blank(wanted + wanted / 10);
        Rect result = Extend(body, sign.Moved(body.Bottom - offset - sign.Bottom), BaselineOf.This);

        XElement? degree = Part(element, "deg");
        if (degree is null || !degree.Elements().Any() || Flag(Part(element, "radPr"), "degHide"))
        {
            return result;
        }

        int scriptHeight = fontHeight > BaseSize / 3 ? fontHeight * RelIndex / 100 : fontHeight;
        Rect extra = Of(degree, scriptHeight);
        if (extra.Height <= 0) return result;

        int y = result.Top + sign.Height * 52 / 100 - extra.Height;
        return Extend(result, extra.Moved(y - extra.Top), BaselineOf.This, keep: true);
    }

    /// <summary>
    /// <c>SmBraceNode::Arrange</c> — node.cxx:1256. A scaled bracket oversizes its body by
    /// <see cref="DistBracketSize"/> at each end and is centred on it.
    /// </summary>
    private static Rect Delimiter(XElement element, int fontHeight)
    {
        Rect body = Row(Parts(element, "e").SelectMany(e => e.Elements()), fontHeight);
        if (body.Height <= 0) return default;

        int height = body.Height + 2 * (body.Height * DistBracketSize / 100);
        Rect bracket = Blank(height);
        int centre = (body.Top + body.Bottom) / 2;
        return Extend(body, bracket.Moved(centre - height / 2), BaselineOf.This);
    }

    /// <summary>
    /// <c>SmOperNode::Arrange</c> — node.cxx:1515. The operator is set at
    /// <see cref="DistOperatorSize"/> over the body's size and its limits go above and below it.
    /// </summary>
    private static Rect Nary(XElement element, int fontHeight)
    {
        XElement? properties = Part(element, "naryPr");
        Rect body = Of(Part(element, "e"), fontHeight);
        Rect symbol = Blank(TextHeightAt(fontHeight * (100 + DistOperatorSize) / 100));

        int dy = body.Height > 0 ? (body.Top + body.Bottom) / 2 - (symbol.Top + symbol.Bottom) / 2 : 0;
        Rect result = body.Height > 0
            ? Extend(body, symbol.Moved(dy), BaselineOf.This)
            : symbol;

        int limitHeight = fontHeight > BaseSize / 3 ? fontHeight * RelLimits / 100 : fontHeight;
        int gap = fontHeight * DistLimit / 100;

        if (!Flag(properties, "supHide") && Of(Part(element, "sup"), limitHeight) is { Height: > 0 } upper)
        {
            result = Extend(result, upper.Moved(result.Top - upper.Bottom - gap), BaselineOf.This, keep: true);
        }

        if (!Flag(properties, "subHide") && Of(Part(element, "sub"), limitHeight) is { Height: > 0 } lower)
        {
            result = Extend(result, lower.Moved(result.Bottom - lower.Top + gap), BaselineOf.This, keep: true);
        }

        return result;
    }

    /// <summary>
    /// <c>SmTableNode::Arrange</c> — node.cxx:491. One column, <see cref="DistVertical"/> apart.
    /// </summary>
    private static Rect Stack(IReadOnlyList<XElement?> rows, int fontHeight)
        => Column(rows.Select(r => Of(r, fontHeight)), DistVertical * fontHeight / 100);

    /// <summary>
    /// <c>SmMatrixNode::Arrange</c> — node.cxx:1941. Its row gap is taken from a "norm distance"
    /// of three font heights rather than from one.
    /// </summary>
    private static Rect Matrix(XElement element, int fontHeight)
        => Column(
            Parts(element, "mr").Select(row => Row(Parts(row, "e").SelectMany(e => e.Elements()), fontHeight)),
            3 * fontHeight * DistMatrixRow / 100);

    private static Rect Column(IEnumerable<Rect> rows, int gap)
    {
        Rect result = default;
        bool any = false;
        int y = 0;

        foreach (Rect row in rows)
        {
            if (row.Height <= 0) continue;
            if (any) y += gap;
            Rect placed = row.Moved(y - row.Top);
            y = placed.Bottom;
            result = any ? Extend(result, placed, BaselineOf.Neither) : placed;
            any = true;
        }

        return any ? result : default;
    }

    /// <summary>
    /// <c>m:limLow</c>, <c>m:limUpp</c> and <c>m:groupChr</c> — a body with one line set under or
    /// over it, and for a group character a bracket between the two.
    /// </summary>
    private static Rect Limit(XElement element, int fontHeight, bool above, bool bracket = false)
    {
        Rect body = Of(Part(element, "e"), fontHeight);
        if (body.Height <= 0) return default;

        Rect result = body;
        if (bracket)
        {
            Rect mark = Blank(fontHeight * DistBracketSize / 100 * 2);
            result = Extend(result, mark.Moved(result.Bottom - mark.Top), BaselineOf.This, keep: true);
        }

        int limitHeight = fontHeight > BaseSize / 3 ? fontHeight * RelLimits / 100 : fontHeight;
        Rect limit = Of(Part(element, "lim"), limitHeight);
        if (limit.Height <= 0) return result;

        int gap = fontHeight * DistLimit / 100;
        return Extend(
            result,
            above ? limit.Moved(result.Top - limit.Bottom - gap)
                  : limit.Moved(result.Bottom - limit.Top + gap),
            BaselineOf.This,
            keep: true);
    }
}
