using System.Xml.Linq;
using XRay.Office.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// Which branch of an <c>mc:AlternateContent</c> X-Ray.Office takes, and why the extended-chart
/// case is the one exception to "a choice we cannot read loses to the fallback".
/// </summary>
/// <remarks>
/// <para>
/// Excel writes an extended ("chartex") chart — Pareto, histogram, waterfall, treemap, sunburst,
/// box-and-whisker, funnel — as a <c>Requires="cx1"</c> choice with a generated rectangle beside
/// it whose only text tells the reader their Excel is too old. Taking the fallback, which is
/// otherwise the correct thing for a reader that cannot draw the choice, puts 26 words of English
/// advice on the page where the chart belongs. Measured against LibreOffice 26.2.4.2 on the two
/// corpus witnesses (<c>054_Problem_analysis_with_Pareto_chart</c> and
/// <c>051_Manufacturer_defect_analysis</c>): the reference draws none of that sentence, and
/// suppressing it took both documents from 87/61 and 95/69 extractable words to exact matches.
/// </para>
/// <para>
/// The slicer tests below are what keep this honest. A slicer's fallback has the *same* shape —
/// <c>id="0"</c>, an empty name, <c>noTextEdit</c>, a one-EMU outline and an advisory sentence —
/// and there the reference <em>does</em> draw it. So the rule may not be "suppress advisory
/// placeholders"; it has to be keyed on the chartex graphic-data URI and nothing else.
/// </para>
/// <para>
/// The slicer used to need a key of its own, and no longer does. It was written as an exception
/// because <c>a14</c> was in <c>UnderstoodExtensions</c>; <c>oox</c> and writerfilter both refuse
/// <c>a14</c> — see <see cref="OoxmlNamespaces.UnderstoodExtensions"/> — so the general rule now
/// reaches every one of the corpus's seven <c>a14</c> slicer choices, and the exception was
/// unreachable code. The tests are kept, because what they assert about the *outcome* is measured
/// and is what must not drift.
/// </para>
/// </remarks>
public sealed class OoxmlAlternateContentTests
{
    private const string Mce = "http://schemas.openxmlformats.org/markup-compatibility/2006";

    /// <summary>Builds a spreadsheet drawing whose choice holds a graphic frame of one URI.</summary>
    /// <param name="choiceUri">The graphic-data URI the choice's frame carries.</param>
    /// <param name="requires">The prefix the choice's <c>Requires</c> names.</param>
    /// <param name="prefix">The prefix bound to the markup-compatibility namespace.</param>
    /// <param name="requiresUri">
    /// What the <c>Requires</c> prefix is bound to. The default is a namespace nothing here
    /// understands, which is the ordinary "a choice we cannot read" case. Binding it to a
    /// namespace that <em>is</em> understood is what the corpus actually contains for a slicer,
    /// and leaving that untested is how the slicer assertion below passed for four rounds while
    /// the defect it names was live on three documents.
    /// </param>
    private static XElement Drawing(
        string choiceUri, string requires, string prefix = "mc",
        string requiresUri = "urn:the-extension")
        => XElement.Parse($"""
            <xdr:wsDr xmlns:xdr="http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"
                      xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">
              <xdr:twoCellAnchor>
                <{prefix}:AlternateContent xmlns:{prefix}="{Mce}">
                  <{prefix}:Choice xmlns:{requires}="{requiresUri}" Requires="{requires}">
                    <xdr:graphicFrame>
                      <a:graphic><a:graphicData uri="{choiceUri}" /></a:graphic>
                    </xdr:graphicFrame>
                  </{prefix}:Choice>
                  <{prefix}:Fallback>
                    <xdr:sp>
                      <xdr:txBody><a:p><a:r><a:t>Your version of Excel is too old.</a:t></a:r></a:p></xdr:txBody>
                    </xdr:sp>
                  </{prefix}:Fallback>
                </{prefix}:AlternateContent>
              </xdr:twoCellAnchor>
            </xdr:wsDr>
            """);

    private const string ExtendedChartUri = "http://schemas.microsoft.com/office/drawing/2014/chartex";
    private const string SlicerUri = "http://schemas.microsoft.com/office/drawing/2010/slicer";

    [Fact]
    public void AnExtendedChartChoiceBeatsItsAdvisoryFallback()
    {
        XElement root = Drawing(ExtendedChartUri, "cx1");
        OoxmlXml.Normalise(root);

        root.Descendants().Any(e => e.Name.LocalName == "graphicFrame").ShouldBeTrue();
        root.Value.ShouldNotContain("too old");
    }

    [Fact]
    public void TheExtendedChartExceptionDoesNotDependOnTheMarkupCompatibilityPrefix()
    {
        // ECMA-376 1st edition bound the namespace to `ve`, and eight corpus documents still do.
        XElement root = Drawing(ExtendedChartUri, "cx1", prefix: "ve");
        OoxmlXml.Normalise(root);

        root.Descendants().Any(e => e.Name.LocalName == "graphicFrame").ShouldBeTrue();
        root.Value.ShouldNotContain("too old");
    }

    [Fact]
    public void ASlicerChoiceStillLosesToItsFallback()
    {
        // The reference draws the slicer placeholder, so this must NOT follow the chartex rule.
        // Here the Requires prefix is bound to a namespace nothing understands, so the fallback
        // wins by the *general* rule; the test below is the one that exercises the slicer key.
        XElement root = Drawing(SlicerUri, "a14");
        OoxmlXml.Normalise(root);

        root.Descendants().Any(e => e.Name.LocalName == "graphicFrame").ShouldBeFalse();
        root.Value.ShouldContain("too old");
    }

    /// <summary>
    /// A slicer choice written the way the corpus writes it — <c>Requires="a14"</c> with
    /// <c>a14</c> bound to DrawingML 2010 — loses to its fallback.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is what the corpus contains and what the test above does not reach, because that one
    /// binds the prefix to a namespace nothing has ever understood. All three witnesses write
    /// <c>Requires="a14"</c> with <c>a14</c> bound to
    /// <see cref="OoxmlNamespaces.DrawingML2010"/>. LibreOffice 26.2.4.2 draws the fallback
    /// rectangle: measured, its PDF holds the advisory 3 times on
    /// <c>049_Expenses_calculator</c>, 2 on <c>037_Personal_money_tracker</c> and 1 on
    /// <c>DynamicBubbleChart</c>, against 0 in ours before the fix that made it so.
    /// </para>
    /// <para>
    /// It reached that outcome twice, by two different routes, and the second is the right one.
    /// First by a key on the slicer graphic-data URI, written when <c>a14</c> was understood and
    /// the choice was being taken; now by <c>a14</c> not being understood at all, which is what
    /// <c>ContextHandler2Helper::prepareMceContext</c> does. The test is unchanged in what it
    /// asserts and that is the point: the outcome was measured, the mechanism was a guess, and
    /// only the mechanism moved.
    /// </para>
    /// </remarks>
    [Fact]
    public void ASlicerChoiceLosesToItsFallbackWhenItsRequiresIsA14()
    {
        XElement root = Drawing(SlicerUri, "a14", requiresUri: OoxmlNamespaces.DrawingML2010);
        OoxmlXml.Normalise(root);

        root.Descendants().Any(e => e.Name.LocalName == "graphicFrame").ShouldBeFalse();
        root.Value.ShouldContain("too old");
    }

    /// <summary>
    /// An understood choice that is <em>not</em> a slicer still wins over its fallback.
    /// </summary>
    /// <remarks>
    /// The guard against widening. 108 word-processing documents write a
    /// <c>Requires="wps"</c> choice with a VML fallback beside it, and every one of them must keep
    /// taking the choice — a rule that made any understood choice with a fallback lose would
    /// silently swap the shape content of a third of the words corpus for its VML twin.
    /// </remarks>
    [Fact]
    public void AnUnderstoodChoiceThatIsNotASlicerStillBeatsItsFallback()
    {
        XElement root = Drawing(
            "http://schemas.microsoft.com/office/word/2010/wordprocessingShape", "wps",
            requiresUri: OoxmlNamespaces.WordShape);
        OoxmlXml.Normalise(root);

        root.Descendants().Any(e => e.Name.LocalName == "graphicFrame").ShouldBeTrue();
        root.Value.ShouldNotContain("too old");
    }

    /// <summary>
    /// An <c>a14</c> choice with no fallback beside it is <strong>dropped</strong>, and the
    /// anchor with it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This test used to assert the opposite, under the name
    /// <c>ASlicerChoiceWithNoFallbackBesideItIsStillTaken</c>, and its stated reason was that
    /// "dropping the choice would lose an anchor rather than gain a placeholder". That is an
    /// argument, not a measurement, and it is wrong: MCE says select the first choice you
    /// understand, else the fallback, else nothing, and <c>oox</c> does exactly that.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2 rather than reasoned about, on precisely this shape.
    /// <c>013_Contextures_chart_sample</c>'s drawing part is an <c>a14</c> choice holding an
    /// <c>xdr:pic</c> beside an <strong>empty</strong> fallback, and the picture on its page comes
    /// from the legacy VML instead. Delete the sheet's <c>legacyDrawing</c> relationship — one
    /// edit, nothing else changed — and the reference's page 1 goes from 23 extractable words to
    /// <strong>5</strong>: the a14 choice's picture is not drawn, because nothing usable is beside
    /// it. <c>probes/sheets-r55/probe-vml-camera.py</c>.
    /// </para>
    /// </remarks>
    [Fact]
    public void AnA14ChoiceWithNoFallbackBesideItIsDropped()
    {
        XElement root = XElement.Parse($"""
            <xdr:wsDr xmlns:xdr="http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"
                      xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">
              <mc:AlternateContent xmlns:mc="{Mce}">
                <mc:Choice xmlns:a14="{OoxmlNamespaces.DrawingML2010}" Requires="a14">
                  <xdr:graphicFrame>
                    <a:graphic><a:graphicData uri="{SlicerUri}" /></a:graphic>
                  </xdr:graphicFrame>
                </mc:Choice>
              </mc:AlternateContent>
            </xdr:wsDr>
            """);
        OoxmlXml.Normalise(root);

        root.Descendants().Any(e => e.Name.LocalName == "graphicFrame").ShouldBeFalse();
        root.Descendants().Any(e => e.Name.NamespaceName == Mce).ShouldBeFalse();
    }

    /// <summary>
    /// An <c>a14</c> choice loses to its fallback whatever it wraps — this one wraps an ordinary
    /// picture, not a slicer, so nothing about it is keyed on a graphic-data URI.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ContextHandler2Helper::prepareMceContext</c> lists the MCE namespaces the <c>oox</c>
    /// filters honour and carries <c>a14</c> commented out with the reason attached — "we do not
    /// currently support inline formulas and other a14 stuff". Writerfilter keeps its own list
    /// (<c>wps</c>, <c>wpg</c>, <c>w14</c>, <c>wpc</c>) and <c>a14</c> is not on that one either.
    /// </para>
    /// <para>
    /// Measured: unwrapping the <c>mc:AlternateContent</c> around
    /// <c>013_Contextures_chart_sample</c>'s camera picture makes 26.2.4.2 draw that picture
    /// <strong>twice</strong> — once at x = 129.5 from the DrawingML anchor it can now see, once
    /// at 133.8 from the legacy VML shape it was already drawing — 41 extractable words against
    /// 23. With the wrapper in place it draws the VML one alone.
    /// </para>
    /// </remarks>
    [Fact]
    public void AnA14ChoiceLosesToItsFallbackWhateverItWraps()
    {
        XElement root = Drawing(
            "urn:some-graphic", "a14", requiresUri: OoxmlNamespaces.DrawingML2010);
        OoxmlXml.Normalise(root);

        root.Descendants().Any(e => e.Name.LocalName == "graphicFrame").ShouldBeFalse();
        root.Value.ShouldContain("too old");
    }

    /// <summary>
    /// The guard requires a fallback to exist, so it cannot change the no-fallback case — and the
    /// no-fallback case drops the element, which is what MCE says to do: select the first choice
    /// you understand, else the fallback, else nothing. Asserted because it is *not* obvious, and
    /// because a future widening of the chartex exception must not silently start resurrecting
    /// frames here.
    /// </summary>
    [Fact]
    public void AnUnreadableChoiceWithNoFallbackBesideItIsStillDropped()
    {
        XElement root = XElement.Parse($"""
            <xdr:wsDr xmlns:xdr="http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"
                      xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">
              <mc:AlternateContent xmlns:mc="{Mce}">
                <mc:Choice xmlns:cx1="urn:the-extension" Requires="cx1">
                  <xdr:graphicFrame>
                    <a:graphic><a:graphicData uri="{ExtendedChartUri}" /></a:graphic>
                  </xdr:graphicFrame>
                </mc:Choice>
              </mc:AlternateContent>
            </xdr:wsDr>
            """);
        OoxmlXml.Normalise(root);

        root.Descendants().Any(e => e.Name.LocalName == "graphicFrame").ShouldBeFalse();
        root.Descendants().Any(e => e.Name.NamespaceName == Mce).ShouldBeFalse();
    }
}
