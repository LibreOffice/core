using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Numbers;
using XRay.Office.OpenDocument.Styles;
using Shouldly;

namespace XRay.Office.OpenDocument.Tests;

/// <summary>
/// A multi-section ODF number format: several <c>number:*-style</c> elements linked by
/// <c>style:map</c>, assembled into one code.
/// </summary>
/// <remarks>
/// <para>
/// <strong>ODF does not write a multi-section format as one element</strong>, and nothing followed
/// the links — <c>OdfNumberFormat.Code</c> walked one element's <c>number:</c> children and
/// stopped, so one section was compiled and applied to every value. LibreOffice writes the section
/// the cell names as the element it names, with its own body <em>last</em> and the others in
/// volatile styles reached through its maps.
/// </para>
/// <para>
/// The markup in these tests is copied out of <c>features/sheet-odf-numfmt-sections.ods</c>, which
/// is 26.2.4.2's own export of a workbook stating six format codes — so the shapes asserted here
/// are the exporter's rather than an author's guess at it.
/// </para>
/// <para>
/// <strong>Three of the rules are not in the specification's prose</strong>, and each has an arm
/// below. A single <c>value()&gt;=0</c> map writes <em>no</em> condition into the code, because it
/// is the ordinary positive/negative pair and bracketing it would leave nothing to fall through
/// to (<c>xmlnumfi.cxx</c>:2149-2150). In a <c>number:text-style</c> — which is how an accounting
/// format arrives, the owner holding <c>@</c> and its three maps holding the numbers — the last
/// map is unconditional too (<c>:2152-2156</c>). And a condition that does not begin
/// <c>value()</c>, or that names a style nothing resolves, contributes <b>nothing at all</b>, not
/// even an empty section (<c>:2137-2140</c>).
/// </para>
/// </remarks>
public class OdfNumberFormatSectionsTests
{
    private const string N = "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0";
    private const string S = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";
    private const string F = "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0";

    /// <summary>LibreOffice's own extension namespace, which the accounting family's
    /// <c>blank-width-char</c> sits in.</summary>
    private const string L = "urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0";

    /// <summary>The fixture's own styles, by name, exactly as 26.2.4.2 exported them.</summary>
    private static readonly Dictionary<string, string> Styles = new()
    {
        // `0.0;[Red]-0.0;[Blue]0.0` — three sections, so two maps, so neither is bare.
        ["N152"] = """
            <number:number-style style:name="N152">
              <style:text-properties fo:color="#0000ff"/>
              <number:number number:decimal-places="1" number:min-decimal-places="1" number:min-integer-digits="1"/>
              <style:map style:condition="value()&gt;0" style:apply-style-name="N152P0"/>
              <style:map style:condition="value()&lt;0" style:apply-style-name="N152P1"/>
            </number:number-style>
            """,
        ["N152P0"] = """
            <number:number-style style:name="N152P0" style:volatile="true">
              <number:number number:decimal-places="1" number:min-decimal-places="1" number:min-integer-digits="1"/>
            </number:number-style>
            """,
        ["N152P1"] = """
            <number:number-style style:name="N152P1" style:volatile="true">
              <style:text-properties fo:color="#ff0000"/>
              <number:text>-</number:text>
              <number:number number:decimal-places="1" number:min-decimal-places="1" number:min-integer-digits="1"/>
            </number:number-style>
            """,

        // `#,##0 ;[Red](#,##0)` — one map at >=0, and the two sections differ in TEXT.
        ["N153"] = """
            <number:number-style style:name="N153">
              <style:text-properties fo:color="#ff0000"/>
              <number:text>(</number:text>
              <number:number number:decimal-places="0" number:min-decimal-places="0" number:min-integer-digits="1" number:grouping="true"/>
              <number:text>)</number:text>
              <style:map style:condition="value()&gt;=0" style:apply-style-name="N153P0"/>
            </number:number-style>
            """,
        ["N153P0"] = """
            <number:number-style style:name="N153P0" style:volatile="true">
              <number:number number:decimal-places="0" number:min-decimal-places="0" number:min-integer-digits="1" number:grouping="true"/>
              <number:text> </number:text>
            </number:number-style>
            """,

        // `[>=100]"big" 0;[<0]"neg" 0;0.0` — conditions that are not the default pair.
        ["N154"] = """
            <number:number-style style:name="N154">
              <number:number number:decimal-places="1" number:min-decimal-places="1" number:min-integer-digits="1"/>
              <style:map style:condition="value()&gt;=100" style:apply-style-name="N154P0"/>
              <style:map style:condition="value()&lt;0" style:apply-style-name="N154P1"/>
            </number:number-style>
            """,
        ["N154P0"] = """
            <number:number-style style:name="N154P0" style:volatile="true">
              <number:text>big </number:text>
              <number:number number:decimal-places="0" number:min-decimal-places="0" number:min-integer-digits="1"/>
            </number:number-style>
            """,
        ["N154P1"] = """
            <number:number-style style:name="N154P1" style:volatile="true">
              <number:text>neg </number:text>
              <number:number number:decimal-places="0" number:min-decimal-places="0" number:min-integer-digits="1"/>
            </number:number-style>
            """,

        // `_(* #,##0_);_(* (#,##0);_(* "-"_);_(@_)` — the accounting format, whose owner is the
        // TEXT style and whose three maps are the numbers.
        ["N144"] = """
            <number:text-style style:name="N144">
              <number:text loext:blank-width-char="("> </number:text>
              <number:text-content/>
              <number:text loext:blank-width-char=")"> </number:text>
              <style:map style:condition="value()&gt;0" style:apply-style-name="N144P0"/>
              <style:map style:condition="value()&lt;0" style:apply-style-name="N144P1"/>
              <style:map style:condition="value()=0" style:apply-style-name="N144P2"/>
            </number:text-style>
            """,
        ["N144P0"] = """
            <number:number-style style:name="N144P0" style:volatile="true">
              <number:text loext:blank-width-char="("> </number:text>
              <number:fill-character> </number:fill-character>
              <number:number number:decimal-places="0" number:min-decimal-places="0" number:min-integer-digits="1" number:grouping="true"/>
              <number:text loext:blank-width-char=")"> </number:text>
            </number:number-style>
            """,
        ["N144P1"] = """
            <number:number-style style:name="N144P1" style:volatile="true">
              <number:text loext:blank-width-char="("> </number:text>
              <number:fill-character> </number:fill-character>
              <number:text>(</number:text>
              <number:number number:decimal-places="0" number:min-decimal-places="0" number:min-integer-digits="1" number:grouping="true"/>
              <number:text>)</number:text>
            </number:number-style>
            """,
        ["N144P2"] = """
            <number:number-style style:name="N144P2" style:volatile="true">
              <number:text loext:blank-width-char="("> </number:text>
              <number:fill-character> </number:fill-character>
              <number:text loext:blank-width-char=")1">- </number:text>
            </number:number-style>
            """,

        // `[COLOR10]0.0` — the palette index resolved to #dddddd, which is not one of the ten.
        ["N155"] = """
            <number:number-style style:name="N155">
              <style:text-properties fo:color="#dddddd"/>
              <number:number number:decimal-places="1" number:min-decimal-places="1" number:min-integer-digits="1"/>
            </number:number-style>
            """,
    };

    private static XElement Named(string name)
        // The fragments above are quoted verbatim out of the fixture, so the three prefixes they
        // use are declared on a wrapper rather than repeated on every one of them.
        //
        // `PreserveWhitespace` is load-bearing and its absence is silent: `<number:text> </>` is
        // a whitespace-only node, the default drops it, and the format's padding space then
        // disappears from the compiled code here while the reader — which loads with
        // `IgnoreWhitespace = false` through `OdfXml` — keeps it. A fixture parsed the default
        // way asserts a code the product never produces.
        => XElement.Parse(
            $"""<w xmlns:number="{N}" xmlns:style="{S}" xmlns:fo="{F}" xmlns:loext="{L}">{Styles[name]}</w>""",
            LoadOptions.PreserveWhitespace)
            .Elements().Single();

    private static XElement? Resolve(string name) => Styles.ContainsKey(name) ? Named(name) : null;

    private static string? Code(string name) => OdfNumberFormat.Code(Named(name), Resolve);

    [Fact]
    public void WithoutAResolverTheNamedElementAloneIsCompiled()
        => OdfNumberFormat.Code(Named("N152"))
            .ShouldBe("[BLUE]0.0", "one section, which is what every caller saw before this");

    [Fact]
    public void TwoMapsBothStateTheirConditionAndTheOwnersBodyIsLast()
        => Code("N152").ShouldBe("[>0]0.0;[<0][RED]-0.0;[BLUE]0.0");

    [Fact]
    public void ASingleGreaterOrEqualZeroMapStatesNoCondition()
        => Code("N153").ShouldBe("""#,##0" ";[RED](#,##0)""");

    [Fact]
    public void ConditionsThatAreNotTheDefaultPairAreBothKept()
        => Code("N154").ShouldBe("""[>=100]"big "0;[<0]"neg "0;0.0""");

    [Theory]
    // `(100)` in red is the second section of N153, and a reader that compiled the named element
    // alone drew the parentheses on the positive too.
    [InlineData("N153", 150.0, "150 ", null)]
    [InlineData("N153", -100.0, "(100)", 0xFF0000)]
    [InlineData("N152", 150.0, "150.0", null)]
    [InlineData("N152", -100.0, "-100.0", 0xFF0000)]
    [InlineData("N152", 0.0, "0.0", 0x0000FF)]
    [InlineData("N154", 150.0, "big 150", null)]
    [InlineData("N154", -100.0, "neg 100", null)]
    [InlineData("N154", 0.0, "0.0", null)]
    public void TheSectionAValueSelectsIsWhatIsRenderedAndColoured(
        string name, double value, string expected, int? colour)
    {
        NumberFormatCode code = NumberFormatCode.Parse(Code(name)!);
        NumberFormatSection section = code.SelectFor(value);

        NumberFormatter.Format(code, value).ShouldBe(expected);
        section.Colour.ShouldBe(colour is { } rgb ? Colour.FromRgb((uint)rgb) : null);
    }

    [Fact]
    public void AColourThatIsNotOneOfTheTenIsDropped()
    {
        // `[COLOR10]` is the palette at the running installation's own standard.soc, which
        // 26.2.4.2 resolves to #dddddd and its own ODF export writes as an RGB value. AddColor
        // matches that against ten constants and nothing else, so the reference drops it on
        // re-import too — a reader resolving arbitrary RGB here would paint ink it does not.
        Code("N155").ShouldBe("0.0");
        NumberFormatCode.Parse(Code("N155")!).Sections[0].Colour.ShouldBeNull();
    }

    [Fact]
    public void AMapNamingAStyleNothingResolvesContributesNoSectionAtAll()
    {
        XElement style = Named("N153");
        style.Elements().Last().SetAttributeValue(XName.Get("apply-style-name", S), "nope");

        // Not an empty section and not a semicolon: AddCondition returns before appending
        // anything, so what is left is the owner's body alone.
        OdfNumberFormat.Code(style, Resolve).ShouldBe("[RED](#,##0)");
    }

    [Fact]
    public void AConditionThatIsNotAValueComparisonIsSkipped()
    {
        XElement style = Named("N153");
        style.Elements().Last().SetAttributeValue(XName.Get("condition", S), "cell-content()>=0");

        OdfNumberFormat.Code(style, Resolve).ShouldBe("[RED](#,##0)");
    }

    [Theory]
    // The right-hand column is 26.2.4.2's OWN assembled code for the same style, taken from its
    // HTML export of the fixture: `sdnum` is "<language>;<system>;<code>", and the third field is
    // the string `SvNumberFormatter` holds once `CreateAndInsert` has prepended the conditions.
    // It is a stronger instrument than a rendered page for this question, because it states the
    // answer rather than showing a consequence of it — `probes/odfnumfmt-r141/reference-sdnum.tsv`.
    [InlineData("N152", "[>0]0.0;[<0][RED]-0.0;[BLUE]0.0")]
    [InlineData("N153", """#,##0" ";[RED](#,##0)""")]
    [InlineData("N154", """[>=100]"big "0;[<0]"neg "0;0.0""")]
    [InlineData("N155", "0.0")]
    public void TheAssembledCodeIsTheReferencesOwn(string name, string expected)
        => Code(name).ShouldBe(expected);

    [Fact]
    public void TheAccountingFormatIsTheReferencesOwnCodeToo()
    {
        // 26.2.4.2's own `sdnum` for this style, and the hardest of the six: three conditions
        // with the last bare, a `number:text-style` owner whose body is the text section, an
        // `*` fill character, and four `_x` blanks reconstructed from `blank-width-char`
        // attributes and the spaces the exporter wrote in their place.
        Code("N144").ShouldBe("""[>0]_(* #,##0_);[<0]_(* (#,##0);_(* "-"_);_(@_)""");
    }

    [Fact]
    public void AStyleThatMapsToItselfIsNotFollowedForever()
    {
        XElement style = Named("N153");
        style.Elements().Last().SetAttributeValue(XName.Get("apply-style-name", S), "N153");

        // `CreateAndInsert` keeps a stack of the styles it is building and refuses one already on
        // it -- "invalid style:map references containing style", xmlnumfi.cxx:1592-1596. Without
        // that this is a stack overflow rather than a diagnostic.
        OdfNumberFormat.Code(style, name => name == "N153" ? style : Resolve(name))
            .ShouldBe("[RED](#,##0)");
    }
}
