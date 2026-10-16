using System.Xml.Linq;
using XRay.Office.Core.Numbers;
using XRay.Office.OpenDocument.Styles;
using Shouldly;

namespace XRay.Office.OpenDocument.Tests;

/// <summary>
/// What an ODF <c>number:*-style</c> compiles to, and what it then renders.
/// </summary>
/// <remarks>
/// <para>
/// ODF states a number format as a tree of elements and OOXML as a string. LibreOffice keeps one
/// formatter for both and reaches it from ODF by building a format string
/// (<c>xmloff/source/style/xmlnumfi.cxx</c>); this is that build, so the assertions are on the
/// string it produces <em>and</em> on what the shared engine then makes of it — a code that parses
/// but renders wrongly is the failure a code-only assertion misses.
/// </para>
/// <para>
/// The reason a chart needs this at all: an ODF axis names a data style through
/// <c>style:data-style-name</c> and caches no text of its own, so a percentage axis draws
/// <c>0.05</c> instead of <c>5%</c> without it.
/// </para>
/// </remarks>
public class OdfNumberFormatTests
{
    private const string N = "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0";
    private const string S = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";

    private static XElement Style(string inner, string kind = "number-style")
        => XElement.Parse($"<number:{kind} xmlns:number=\"{N}\" xmlns:style=\"{S}\">{inner}</number:{kind}>");

    [Fact]
    public void APlainIntegerCompilesToASingleZero()
        => OdfNumberFormat.Code(Style("""<number:number number:min-integer-digits="1"/>"""))
            .ShouldBe("0");

    [Fact]
    public void GroupingAndDecimalsBecomeTheFamiliarCode()
        => OdfNumberFormat.Code(Style(
                """
                <number:number number:decimal-places="2" number:min-decimal-places="2"
                               number:min-integer-digits="1" number:grouping="true"/>
                """))
            .ShouldBe("#,##0.00");

    /// <summary>
    /// A percentage style is a number followed by a bare per cent sign, and bare is the point.
    /// </summary>
    /// <remarks>
    /// ODF writes every suffix as a <c>number:text</c> and they are quoted, because an unquoted
    /// one whose characters happen to be <c>d</c>, <c>m</c> or <c>y</c> is read as a date
    /// directive. The per cent sign is the exception: it is the only thing in the compiled code
    /// that says "multiply by a hundred", so quoting it renders <c>0.05</c> as <c>0.1%</c>.
    /// </remarks>
    [Fact]
    public void APercentageStyleKeepsItsSignAsALiteral()
    {
        XElement style = Style(
            """
            <number:number number:decimal-places="1" number:min-decimal-places="1"
                           number:min-integer-digits="1"/>
            <number:text>%</number:text>
            """,
            "percentage-style");

        OdfNumberFormat.Code(style).ShouldBe("0.0%");

        NumberFormatCode code = OdfNumberFormat.Parse(style)!;
        NumberFormatter.Format(code, 0.05).ShouldBe("5.0%");
    }

    /// <summary>A currency style keeps its symbol and its grouping.</summary>
    [Fact]
    public void ACurrencyStyleRendersThroughTheSharedEngine()
    {
        XElement style = Style(
            """
            <number:currency-symbol>£</number:currency-symbol>
            <number:number number:decimal-places="2" number:min-decimal-places="2"
                           number:min-integer-digits="1" number:grouping="true"/>
            """,
            "currency-style");

        NumberFormatter.Format(OdfNumberFormat.Parse(style)!, 1234.5).ShouldBe("£1,234.50");
    }

    /// <summary>
    /// A date style's pieces are emitted in the order the file states them.
    /// </summary>
    /// <remarks>
    /// <strong>The named trap.</strong> <c>number:month</c> and <c>number:minutes</c> both compile
    /// to <c>M</c> — the same ambiguity the format-code language has, resolved the same way by
    /// what sits either side. So the pieces must go out in document order; gathering them by kind,
    /// or emitting the date part before the time part regardless of what the style says, turns
    /// <c>13:45</c> into month 45 of year 13.
    /// </remarks>
    [Fact]
    public void ADateStylesPiecesKeepTheirDocumentOrder()
    {
        XElement style = Style(
            """
            <number:day number:style="long"/>
            <number:text>/</number:text>
            <number:month number:style="long"/>
            <number:text>/</number:text>
            <number:year number:style="long"/>
            """,
            "date-style");

        // The separators are bare, not quoted: this asserted `DD"/"MM"/"YYYY` until the HTML
        // export made the code itself visible and the reference was measured writing
        // `sdnum="1033;0;MM/DD/YYYY"` for the same shape. Both codes render the line below
        // identically, which is why the quoting was wrong for as long as it was.
        OdfNumberFormat.Code(style).ShouldBe("DD/MM/YYYY");
        NumberFormatter.Format(OdfNumberFormat.Parse(style)!, 45000).ShouldBe("15/03/2023");
    }

    /// <summary>
    /// <c>number:day-of-week</c> compiles to <c>NN</c> or <c>NNN</c>, and never to <c>NNNN</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The three <c>N</c> keys are short name, long name and long name with the locale's
    /// day-of-week separator, in that order (<c>svl/source/numbers/zformat.cxx</c>:3983-4004) —
    /// so the long element is <c>NNN</c> and not the <c>NNNN</c> its name suggests.
    /// <c>SvXMLNumFormatContext::AddNfKeyword</c> makes the point outright: it rewrites an
    /// incoming <c>NNNN</c> to <c>NNN</c> and only restores the separator when the following
    /// <c>&lt;number:text&gt;</c> holds exactly it
    /// (<c>xmloff/source/style/xmlnumfi.cxx</c>:2037-2041 and :955-970).
    /// </para>
    /// <para>
    /// Measured with a hand-built flat ODS through both installed binaries
    /// (<c>dotnet/probes/numfmt-r68/dow.fods</c>): a short day-of-week draws <c>Sun</c> and a
    /// long one draws <c>Sunday</c>, with no trailing comma. This compiled to <c>NNN</c> and
    /// <c>NNNN</c> until round 68, which was invisible only because the <c>N</c> keys were not
    /// implemented at all and both spellings came out as literal letters.
    /// </para>
    /// <para>
    /// The corpus holds no ODF, so this is unmeasurable there; the path that reaches it is a
    /// chart axis, where there is no cached display string to fall back on.
    /// </para>
    /// </remarks>
    [Fact]
    public void ADayOfWeekIsShortAtNnAndLongAtNnn()
    {
        XElement shortDay = Style("""<number:day-of-week/>""", "date-style");
        XElement longDay = Style("""<number:day-of-week number:style="long"/>""", "date-style");

        OdfNumberFormat.Code(shortDay).ShouldBe("NN");
        OdfNumberFormat.Code(longDay).ShouldBe("NNN");

        // 44794 is Sunday 21 August 2022.
        NumberFormatter.Format(OdfNumberFormat.Parse(shortDay)!, 44794).ShouldBe("Sun");
        NumberFormatter.Format(OdfNumberFormat.Parse(longDay)!, 44794).ShouldBe("Sunday");
    }

    /// <summary>A time style's minutes stay minutes because they follow the hours.</summary>
    [Fact]
    public void MinutesAfterHoursAreMinutesAndNotMonths()
    {
        XElement style = Style(
            """
            <number:hours number:style="long"/>
            <number:text>:</number:text>
            <number:minutes number:style="long"/>
            """,
            "time-style");

        NumberFormatter.Format(OdfNumberFormat.Parse(style)!, 0.5730).ShouldBe("13:45");
    }

    /// <summary>
    /// A per cent sign with a space in front of it is one literal, and the space alone is quoted.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>0.00 %</c> is the commonest percentage format there is, and ODF writes its suffix as the
    /// single two-character <c>number:text</c> <c>" %"</c> rather than as a space and a sign. A
    /// compiler that quotes a literal whole therefore produces <c>0.00" %"</c>, in which nothing
    /// says "multiply by a hundred" any more: 0.05 renders <c>0.05 %</c>.
    /// </para>
    /// <para>
    /// The reference quotes around the sign instead (<c>lcl_EnquoteIfNecessary</c>,
    /// <c>xmlnumfi.cxx</c>:552-587). Its own HTML export of this style, measured, states
    /// <c>sdnum="1033;0;0.00&amp;quot; &amp;quot;%"</c> and renders the cell <c>5.00 %</c>.
    /// </para>
    /// </remarks>
    [Fact]
    public void APercentageSignBehindASpaceStillMultiplies()
    {
        XElement style = Style(
            """
            <number:number number:decimal-places="2" number:min-decimal-places="2"
                           number:min-integer-digits="1"/>
            <number:text> %</number:text>
            """,
            "percentage-style");

        OdfNumberFormat.Code(style).ShouldBe("0.00\" \"%");

        NumberFormatCode code = OdfNumberFormat.Parse(style)!;
        NumberFormatter.Format(code, 0.05).ShouldBe("5.00 %");
    }

    /// <summary>
    /// A lone separator stays bare where its format type can carry one, and is quoted where it
    /// cannot.
    /// </summary>
    /// <remarks>
    /// <c>lcl_ValidChar</c> (<c>xmlnumfi.cxx</c>:480-531) admits <c>/</c> unquoted in a date, time
    /// or currency style only, because in a number style the same character is the fraction bar.
    /// Both codes render the same text, so this is invisible until something states the code
    /// itself — which the HTML export's <c>sdnum</c> does.
    /// </remarks>
    [Fact]
    public void ASeparatorIsBareInADateStyleAndQuotedInANumberOne()
    {
        OdfNumberFormat.Code(Style(
                """
                <number:month number:style="long"/>
                <number:text>/</number:text>
                <number:day number:style="long"/>
                """,
                "date-style"))
            .ShouldBe("MM/DD");

        OdfNumberFormat.Code(Style(
                """
                <number:number number:min-integer-digits="1"/>
                <number:text>/</number:text>
                """))
            .ShouldBe("0\"/\"");
    }

    /// <summary>Two or more characters are quoted whatever they are.</summary>
    /// <remarks>
    /// The bare cases are one character, or two of which the second is a space — enough for the
    /// separators a date or currency format is built from, and no more. <c>" kg"</c> is a suffix,
    /// and unquoted its <c>k</c> would be read as the thousands directive.
    /// </remarks>
    [Fact]
    public void ALongerLiteralIsQuoted()
        => OdfNumberFormat.Code(Style(
                """
                <number:number number:min-integer-digits="1"/>
                <number:text> kg</number:text>
                """))
            .ShouldBe("0\" kg\"");

        /// <summary>A style with nothing this compiles yields null rather than an empty code.</summary>
    [Fact]
    public void AnEmptyStyleIsNotAFormat()
    {
        OdfNumberFormat.Code(Style(string.Empty)).ShouldBeNull();
        OdfNumberFormat.Parse(null).ShouldBeNull();
    }
}
