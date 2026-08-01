using System.Xml.Linq;
using Paperless.Core.Numbers;
using Paperless.OpenDocument.Styles;
using Shouldly;

namespace Paperless.OpenDocument.Tests;

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

        OdfNumberFormat.Code(style).ShouldBe("""DD"/"MM"/"YYYY""");
        NumberFormatter.Format(OdfNumberFormat.Parse(style)!, 45000).ShouldBe("15/03/2023");
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

    /// <summary>A style with nothing this compiles yields null rather than an empty code.</summary>
    [Fact]
    public void AnEmptyStyleIsNotAFormat()
    {
        OdfNumberFormat.Code(Style(string.Empty)).ShouldBeNull();
        OdfNumberFormat.Parse(null).ShouldBeNull();
    }
}
