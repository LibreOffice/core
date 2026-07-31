using System.Xml.Linq;
using Paperless.Presentations.Ooxml;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Tests placeholder identity and matching, which is where a slide's text finds the properties
/// it does not state.
/// </summary>
/// <remarks>
/// The rules are LibreOffice's, from <c>oox/source/ppt/pptshapecontext.cxx</c> and
/// <c>oox/source/ppt/pptshape.cxx:715-820</c>. They are tested here from markup literals rather
/// than through whole packages because there are five priority rungs, two fallback types and a
/// sentinel index, and building a package per case would obscure what each one is for.
/// </remarks>
public class PptxPlaceholderTests
{
    private const string P = "http://schemas.openxmlformats.org/presentationml/2006/main";
    private const string ADraw = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>A <c>p:sp</c> carrying a placeholder, named so the match can be identified.</summary>
    private static XElement Shape(string name, string? type = null, int? index = null)
    {
        string attributes = (type is null ? "" : $" type=\"{type}\"")
                            + (index is null ? "" : $" idx=\"{index}\"");
        return XElement.Parse(
            $"<p:sp xmlns:p=\"{P}\" xmlns:a=\"{ADraw}\" name=\"{name}\">"
            + $"<p:nvSpPr><p:cNvPr id=\"1\" name=\"{name}\"/><p:cNvSpPr/>"
            + $"<p:nvPr><p:ph{attributes}/></p:nvPr></p:nvSpPr></p:sp>");
    }

    private static string? NameOf(XElement? shape) => shape?.Attribute("name")?.Value;

    [Fact]
    public void APlaceholderWithNoTypeReadsAsObjBecauseThatIsWhatLibreOfficeReads()
    {
        // ECMA-376 gives CT_Placeholder/@type a default of "body"; LibreOffice reads it as "obj"
        // (pptshapecontext.cxx:68). Not hypothetical: LibreOffice's own PPTX export writes a bare
        // <p:ph/> for every outline placeholder, so the disagreement is in the first deck you
        // round-trip through Impress. Agreeing with the reference is the point of the exercise.
        PptxPlaceholder.Read(Shape("x"), master: null)!.Value.Type.ShouldBe("obj");
        PptxPlaceholder.Read(Shape("x"), master: null)!.Value.Index.ShouldBeNull();
    }

    [Fact]
    public void AShapeWithNoPlaceholderElementIsNotAPlaceholder()
    {
        XElement plain = XElement.Parse(
            $"<p:sp xmlns:p=\"{P}\"><p:nvSpPr><p:cNvPr id=\"1\" name=\"box\"/>"
            + "<p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr></p:sp>");

        PptxPlaceholder.Read(plain, master: null).ShouldBeNull();
    }

    [Fact]
    public void TheSentinelIndexMeansInheritNothing()
    {
        // 4294967295 is SAL_MAX_UINT32, an unsigned -1 (pptshapecontext.cxx:76-78). Real files
        // carry it. Read as a signed int it overflows; read as unsigned and kept it matches
        // nothing by accident. Either way it must not become a real index.
        XElement shape = Shape("x", "body", null);
        shape.Descendants(XName.Get("ph", P)).Single().SetAttributeValue("idx", "4294967295");

        PptxPlaceholder.Read(shape, master: null)!.Value.Index.ShouldBeNull();
    }

    [Fact]
    public void AnIndexWithNoTypeTakesItsTypeFromTheMastersPlaceholderOfThatIndex()
    {
        XElement master = Part(Shape("masterBody", "body", 1));
        XElement slide = Shape("slide", type: null, index: 1);

        // This is how a slide says "the second content box" without repeating what kind of box
        // the layout decided it was (pptshapecontext.cxx:82-89).
        PptxPlaceholder.Read(slide, master)!.Value.Type.ShouldBe("body");
    }

    [Theory]
    [InlineData("ctrTitle", "ctrTitle", "title")]
    [InlineData("subTitle", "subTitle", "body")]
    [InlineData("obj", "obj", "body")]
    [InlineData("title", "title", null)]
    [InlineData("sldNum", "sldNum", null)]
    public void ThreeTypesHaveAFallbackBecauseSlidesAndLayoutsNameTheSameSlotDifferently(
        string type, string first, string? second)
    {
        new PptxPlaceholder(type, null).MatchTypes.ShouldBe((first, second));
    }

    [Fact]
    public void AnUnknownTypeIsNotLookedUpAtAll()
    {
        // Rather than falling back to matching anything at the same index, which would attach a
        // placeholder's properties to a shape the producer meant something else by.
        new PptxPlaceholder("somethingElse", 0).MatchTypes.ShouldBe((null, null));
        new PptxPlaceholder("somethingElse", 0).Find([Shape("a", "body", 0)]).ShouldBeNull();
    }

    [Fact]
    public void TheSameTypeAtTheSameIndexBeatsTheSameTypeAtAnother()
    {
        XElement[] shapes = [Shape("wrongIndex", "body", 7), Shape("right", "body", 2)];
        NameOf(new PptxPlaceholder("body", 2).Find(shapes)).ShouldBe("right");
    }

    [Fact]
    public void ThePreferredTypeAtAnyIndexBeatsTheFallbackTypeAtTheSameIndex()
    {
        XElement[] shapes = [Shape("fallbackSameIndex", "body", 3), Shape("preferred", "obj", 9)];

        // Priority 1 over priority 2: pptshape.cxx:743-754. Getting these two the wrong way round
        // is invisible on a simple deck and wrong on every layout with two content boxes.
        NameOf(new PptxPlaceholder("obj", 3).Find(shapes)).ShouldBe("preferred");
    }

    [Fact]
    public void AnythingAtTheSameIndexIsTheLastResort()
    {
        XElement[] shapes = [Shape("otherType", "pic", 4)];

        // The fifth rung is what makes a slide's idx="4" find the layout's placeholder even when
        // the two disagree about what kind of content it holds. LibreOffice's own comment calls
        // the ordering "probably unnecessarily complicated"; it is still the one to match.
        NameOf(new PptxPlaceholder("obj", 4).Find(shapes)).ShouldBe("otherType");
    }

    [Fact]
    public void TheLayoutsPlaceholderBeatsTheMastersAtTheSamePriority()
    {
        // LibreOffice imports a layout *into* the master's shape list and then searches that list
        // backwards (presentationfragmenthandler.cxx:287, pptshape.cxx:791). The reverse walk is
        // load-bearing: it is the only reason the layout wins.
        XElement[] shapes = [Shape("master", "title", null), Shape("layout", "title", null)];
        NameOf(new PptxPlaceholder("title", null).Find(shapes)).ShouldBe("layout");
    }

    [Fact]
    public void APlaceholderInsideAGroupIsStillFound()
    {
        XElement group = XElement.Parse($"<p:grpSp xmlns:p=\"{P}\"/>");
        group.Add(Shape("nested", "title", null));

        NameOf(new PptxPlaceholder("title", null).Find([group])).ShouldBe("nested");
    }

    [Theory]
    [InlineData("title", false, "titleStyle")]
    [InlineData("ctrTitle", false, "titleStyle")]
    [InlineData("body", false, "bodyStyle")]
    [InlineData("subTitle", false, "bodyStyle")]
    [InlineData("obj", false, "bodyStyle")]
    [InlineData("body", true, "notesStyle")]
    [InlineData("ftr", false, null)]
    public void OnlyThreeTypesReachAMasterTextStyle(string type, bool notes, string? expected)
    {
        // pptshape.cxx:116-139. A footer or slide-number placeholder falls straight through to
        // the presentation default, and a notes body resolves against notesStyle — get that one
        // wrong and every line of every speaker note comes out bulleted.
        new PptxPlaceholder(type, null).TextStyle(notes).ShouldBe(expected);
    }

    private static XElement Part(params XElement[] shapes)
    {
        XElement tree = XElement.Parse($"<p:spTree xmlns:p=\"{P}\"/>");
        foreach (XElement shape in shapes) tree.Add(shape);

        XElement common = XElement.Parse($"<p:cSld xmlns:p=\"{P}\"/>");
        common.Add(tree);

        XElement part = XElement.Parse($"<p:sldMaster xmlns:p=\"{P}\"/>");
        part.Add(common);
        return part;
    }
}
