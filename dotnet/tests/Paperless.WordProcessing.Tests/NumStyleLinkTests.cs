using System.Xml.Linq;
using Paperless.WordProcessing.Ooxml;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// An abstract numbering that holds no levels, only the name of the style whose numbering it is.
/// </summary>
/// <remarks>
/// <para>
/// <c>w:styleLink</c> and <c>w:numStyleLink</c> are the two halves of one relationship: the definition
/// carrying the levels declares itself a style's numbering, and every definition that wants to share
/// them carries a <c>w:numStyleLink</c> naming the same style and nothing else at all. Word writes the
/// pair whenever a list is defined through the gallery, which is the ordinary way of making one.
/// </para>
/// <para>
/// Reading only the first half is a silent failure of an unusually complete kind: the paragraph keeps
/// its indent, its hanging indent and its text, and simply has no number. Five documents in the words
/// corpus take this route, and on <c>1528039320.docx</c> it was the last three tokens between the
/// rendering and its reference.
/// </para>
/// <para>
/// LibreOffice resolves it in <c>ListsManager::GetAbstractList</c>
/// (<c>sw/source/writerfilter/dmapper/NumberingManager.cxx:1140-1176</c>).
/// </para>
/// </remarks>
public sealed class NumStyleLinkTests
{
    [Fact]
    public void AnInstanceLinkedThroughAStyleFindsTheLevelsTheStyleDefines()
    {
        WordNumbering numbering = Read();

        WordNumberingLevel? level = numbering.FindLevel("1", 0);

        level.ShouldNotBeNull();
        level.LevelText.ShouldBe("%1.");
        level.NumberFormat.ShouldBe("decimal");
    }

    /// <summary>And the label counts from the linked definition, not from nothing.</summary>
    [Fact]
    public void TheLabelCountsThroughTheLink()
    {
        WordNumbering numbering = Read();

        numbering.Advance("1", 0).ShouldBe("1.");
        numbering.Advance("1", 0).ShouldBe("2.");
    }

    /// <summary>A definition that carries its own levels is unaffected by any of this.</summary>
    [Fact]
    public void ADefinitionWithItsOwnLevelsIsUnchanged()
    {
        Read().FindLevel("2", 0)!.LevelText.ShouldBe("%1)");
    }

    /// <summary>
    /// Two definitions pointing at each other terminate rather than hanging.
    /// </summary>
    /// <remarks>
    /// Not a shape Word produces, but the corpus is kept as found and a reader must survive it. The walk
    /// is bounded, so a cycle resolves to wherever it had reached — which draws no label, exactly as an
    /// unresolvable link should.
    /// </remarks>
    [Fact]
    public void ACycleBetweenTwoLinksTerminates()
    {
        WordNumbering numbering = new();
        numbering.Add(XElement.Parse(
            $"""
            <w:numbering xmlns:w="{Ns}">
              <w:abstractNum w:abstractNumId="0">
                <w:styleLink w:val="B"/><w:numStyleLink w:val="A"/>
              </w:abstractNum>
              <w:abstractNum w:abstractNumId="1">
                <w:styleLink w:val="A"/><w:numStyleLink w:val="B"/>
              </w:abstractNum>
              <w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num>
            </w:numbering>
            """));

        numbering.FindLevel("1", 0).ShouldBeNull();
    }

    private const string Ns = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    /// <summary>
    /// Two abstract definitions in the shape Word writes: one holding the levels and declaring itself
    /// the numbering of "Estilo1", one holding only the link to it.
    /// </summary>
    private static WordNumbering Read()
    {
        WordNumbering numbering = new();
        numbering.Add(XElement.Parse(
            $"""
            <w:numbering xmlns:w="{Ns}">
              <w:abstractNum w:abstractNumId="0">
                <w:styleLink w:val="Estilo1"/>
                <w:lvl w:ilvl="0">
                  <w:start w:val="1"/><w:numFmt w:val="decimal"/><w:lvlText w:val="%1."/>
                </w:lvl>
              </w:abstractNum>
              <w:abstractNum w:abstractNumId="1">
                <w:numStyleLink w:val="Estilo1"/>
              </w:abstractNum>
              <w:abstractNum w:abstractNumId="2">
                <w:lvl w:ilvl="0">
                  <w:start w:val="1"/><w:numFmt w:val="decimal"/><w:lvlText w:val="%1)"/>
                </w:lvl>
              </w:abstractNum>
              <w:num w:numId="1"><w:abstractNumId w:val="1"/></w:num>
              <w:num w:numId="2"><w:abstractNumId w:val="2"/></w:num>
            </w:numbering>
            """));

        return numbering;
    }
}
