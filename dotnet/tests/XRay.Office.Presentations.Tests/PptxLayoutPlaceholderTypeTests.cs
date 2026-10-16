using System.Xml.Linq;
using XRay.Office.Presentations.Ooxml;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Where the "an untyped <c>p:ph</c> takes the type of the placeholder sharing its index" rule
/// applies, and where it must not.
/// </summary>
/// <remarks>
/// <para>
/// The rule belongs to a <em>slide's</em> placeholder alone. LibreOffice reaches it through
/// <c>mpSlidePersistPtr-&gt;getMasterPersist()</c> (<c>oox/source/ppt/pptshapecontext.cxx</c>
/// :68 and :82-90), and a layout has no master persist to get: the layout fragment is imported
/// <em>into the master's own</em> <c>SlidePersist</c> —
/// <c>LayoutFragmentHandler(rFilter, aLayoutFragmentPath, pMasterPersistPtr)</c> at
/// <c>oox/source/ppt/presentationfragmenthandler.cxx:287</c>, with the constructor taking that
/// argument straight into its <c>mpSlidePersistPtr</c>. Only slides and notes ever get a
/// <c>setMasterPersist</c>, at <c>:614</c> and <c>:643</c>. So the branch is simply not entered
/// while a layout's shapes are read, and a layout's bare <c>&lt;p:ph idx="4"/&gt;</c> keeps the
/// default <c>obj</c>.
/// </para>
/// <para>
/// Applying it to a layout as well is invisible on almost every deck, because the index the
/// layout uses for a content box is normally the index the master uses for a body box, and
/// <c>obj</c> falls back to <c>body</c> anyway. It bites where the master's placeholder at that
/// index is one no content box should ever inherit from — <c>dt</c>, <c>ftr</c> or
/// <c>sldNum</c>, the three the master keeps at low indices in its own footer row.
/// </para>
/// <para>
/// Measured on <c>slides/batch-004/pptx/solog_orientation_august_2019.pptx</c>. Slide 5's
/// right-hand content box is <c>&lt;p:ph sz="quarter" idx="4"/&gt;</c>, its
/// <c>slideLayout5.xml</c> counterpart is untyped at the same index, and
/// <c>slideMaster1.xml</c>'s index 4 is the <em>slide-number</em> placeholder, whose list style
/// is <c>sz="1200" algn="r"</c> over <c>schemeClr tx1</c> tinted to 75%. Five bulleted
/// paragraphs therefore drew flush right in <c>#8B8B8B</c> where the reference draws them flush
/// left in black. A blind reviewer who had seen neither the deck nor this brief ranked the grey
/// first and the right-alignment second among all differences on the page; they are one bug.
/// </para>
/// </remarks>
public class PptxLayoutPlaceholderTypeTests
{
    private const string P = "http://schemas.openxmlformats.org/presentationml/2006/main";

    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>A placeholder shape whose list style is identifiable by its <c>algn</c>.</summary>
    private static string Shape(string name, string? type, int? index, string alignment)
        => $"<p:sp><p:nvSpPr><p:cNvPr id=\"1\" name=\"{name}\"/><p:cNvSpPr/><p:nvPr>"
           + $"<p:ph{(type is null ? "" : $" type=\"{type}\"")}"
           + $"{(index is null ? "" : $" idx=\"{index}\"")}/></p:nvPr></p:nvSpPr>"
           + $"<p:txBody><a:bodyPr/><a:lstStyle><a:lvl1pPr algn=\"{alignment}\"/></a:lstStyle>"
           + "<a:p/></p:txBody></p:sp>";

    private static XElement Part(params string[] shapes) => XElement.Parse(
        $"<p:sldMaster xmlns:p=\"{P}\" xmlns:a=\"{A}\"><p:cSld><p:spTree>"
        + string.Concat(shapes) + "</p:spTree></p:cSld></p:sldMaster>");

    /// <summary>The deck's shape: master footer row at index 4, layout content box at index 4.</summary>
    private static (XElement Master, XElement Layout) Parts() => (
        Part(Shape("masterBody", "body", 1, "l"), Shape("masterSldNum", "sldNum", 4, "r")),
        Part(Shape("layoutBody", "body", 1, "l"), Shape("layoutContent", null, 4, "just")));

    private static XElement Slide(int index) => XElement.Parse(
        $"<p:sp xmlns:p=\"{P}\"><p:nvSpPr><p:cNvPr id=\"9\" name=\"slide\"/><p:cNvSpPr/>"
        + $"<p:nvPr><p:ph sz=\"quarter\" idx=\"{index}\"/></p:nvPr></p:nvSpPr></p:sp>");

    private static string? AlignmentOf(XElement? shape)
        => shape?.Descendants(XName.Get("lvl1pPr", A)).FirstOrDefault()?.Attribute("algn")?.Value;

    [Fact]
    public void ASlidesUntypedPlaceholderStillMatchesItsLayoutsBoxOfThatIndex()
    {
        (XElement master, XElement layout) = Parts();
        PptxTextStyles styles = new(layout, master, null, isNotesPage: false);

        (XElement? direct, _) = styles.Placeholders(
            PptxPlaceholder.Read(Slide(4), master, layout));

        // The layout is searched before the master, so the content box wins over the footer row.
        AlignmentOf(direct).ShouldBe("just");
    }

    [Fact]
    public void ALayoutsUntypedPlaceholderDoesNotBecomeTheMastersSlideNumberBox()
    {
        (XElement master, XElement layout) = Parts();
        PptxTextStyles styles = new(layout, master, null, isNotesPage: false);

        (_, XElement? inherited) = styles.Placeholders(
            PptxPlaceholder.Read(Slide(4), master, layout));

        // The layout's box is "obj", which falls back to "body" — so the master rung behind it is
        // the body placeholder, at any index, rather than the slide-number box that happens to
        // share index 4. Reading it as "sldNum" gave that box's right-aligned 12 pt grey.
        AlignmentOf(inherited).ShouldBe("l");
    }

    /// <summary>
    /// The second hop still finds the master's box when the two agree about the index.
    /// </summary>
    /// <remarks>
    /// The control for the fix. Removing the master from the lookup could plausibly have severed
    /// the layout-to-master hop altogether, which would cost every placeholder its master
    /// geometry and list style — a far larger regression than the one being fixed. It does not,
    /// because <c>obj</c>'s fallback to <c>body</c> reaches the master's box on its own.
    /// </remarks>
    [Fact]
    public void TheLayoutToMasterHopStillReachesTheMastersBodyPlaceholder()
    {
        (XElement master, XElement layout) = Parts();
        PptxTextStyles styles = new(layout, master, null, isNotesPage: false);

        (XElement? direct, XElement? inherited) = styles.Placeholders(
            PptxPlaceholder.Read(Slide(1), master, layout));

        AlignmentOf(direct).ShouldBe("l");
        inherited.ShouldNotBeNull();
    }

    /// <summary>
    /// A layout placeholder that states its own type is unaffected.
    /// </summary>
    /// <remarks>
    /// Asserted so the fix is pinned to the untyped case: nothing about a stated
    /// <c>type="sldNum"</c> on the layout should change, and a real footer row must still find
    /// the master's.
    /// </remarks>
    [Fact]
    public void ALayoutPlaceholderStatingItsTypeStillMatchesThatTypeOnTheMaster()
    {
        XElement master = Part(Shape("masterSldNum", "sldNum", 4, "r"));
        XElement layout = Part(Shape("layoutSldNum", "sldNum", 4, "just"));
        PptxTextStyles styles = new(layout, master, null, isNotesPage: false);

        XElement slide = XElement.Parse(
            $"<p:sp xmlns:p=\"{P}\"><p:nvSpPr><p:cNvPr id=\"9\" name=\"slide\"/><p:cNvSpPr/>"
            + "<p:nvPr><p:ph type=\"sldNum\" idx=\"4\"/></p:nvPr></p:nvSpPr></p:sp>");

        (XElement? direct, XElement? inherited) = styles.Placeholders(
            PptxPlaceholder.Read(slide, master, layout));

        AlignmentOf(direct).ShouldBe("just");
        AlignmentOf(inherited).ShouldBe("r");
    }
}
