using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A drawing canvas or group anchored <em>as character</em>, and where its members land.
/// </summary>
/// <remarks>
/// <para>
/// A <c>wpc:wpc</c> is a drawing canvas — what Word writes when a user draws several shapes on a
/// canvas rather than grouping them — and <c>DocxFrames.Members</c> flattens it, like a
/// <c>wpg:wgp</c>, into an envelope plus one frame per leaf carrying
/// <see cref="PageFrame.GroupSize"/> and <see cref="PageFrame.GroupOffset"/>.
/// </para>
/// <para>
/// <c>FrameLayout.Place</c>, which positions an anchored drawing, has always added both.
/// <c>FrameLayout.HangInline</c>, which hangs an as-character one on its line, added neither — so
/// every member of an inline canvas or group was drawn at the drawing's own top-left corner, all of
/// them on one spot, with only the last painted visible. Censused over the words corpus: 9
/// <c>wpc:wpc</c> across 4 documents and 28 inline <c>wpg:wgp</c> across 2, every one of them
/// inline.
/// </para>
/// <para>
/// Measured in <c>dotnet/probes/words-inline-canvas/</c> on this fixture's own shape — three
/// coloured rectangles stepped diagonally inside a 4 x 2 in canvas — where both installed
/// references put them at (72.00, 85.92), (180.00, 121.92) and (288.00, 157.92) PDF points, and we
/// drew two of the three under the third.
/// </para>
/// </remarks>
public sealed class FrameInlineCanvasTests
{
    /// <summary>Every member of an inline canvas is offset inside the drawing, not stacked at it.</summary>
    /// <remarks>
    /// A canvas states no child space of its own, so the member offsets are one-to-one with the
    /// <c>a:off</c> the file gives — which is what makes it the cleaner of the two fixtures for
    /// reading the placement rather than the transform.
    /// </remarks>
    [Fact]
    public void AnInlineCanvasLaysItsMembersOutInsideItself()
    {
        IReadOnlyList<PlacedFrame> members = Members();

        members.Count.ShouldBe(3);

        // Stepped by 1.5 in across and 0.5 in down, exactly as the fixture states them.
        members[1].Area.X.ShouldBe(members[0].Area.X + Length.FromInches(1.5));
        members[2].Area.X.ShouldBe(members[0].Area.X + Length.FromInches(3));
        members[1].Area.Y.ShouldBe(members[0].Area.Y + Length.FromInches(0.5));
        members[2].Area.Y.ShouldBe(members[0].Area.Y + Length.FromInches(1));

        foreach (PlacedFrame member in members)
        {
            member.Area.Width.ShouldBe(Length.FromInches(1));
            member.Area.Height.ShouldBe(Length.FromInches(0.5));
        }
    }

    /// <summary>
    /// The whole canvas hangs on the line, so the member at the canvas's own origin is at its top.
    /// </summary>
    /// <remarks>
    /// The half that is not the offset: what rests on the baseline is the <em>envelope's</em>
    /// rectangle and not the member's. Hanging each member by its own height instead would put the
    /// bottom of all three on the baseline, which is a different wrong answer from stacking them at
    /// the corner and looks almost right on a canvas whose members are all the same height.
    /// </remarks>
    [Fact]
    public void TheEnvelopeIsWhatRestsOnTheLine()
    {
        IReadOnlyList<PlacedFrame> all = Frames();
        PlacedFrame envelope = all.Single(frame => frame.Frame.GroupSize is null);
        IReadOnlyList<PlacedFrame> members = [.. all.Where(frame => frame.Frame.GroupSize is not null)];

        envelope.Area.Width.ShouldBe(Length.FromInches(4));
        envelope.Area.Height.ShouldBe(Length.FromInches(2));

        // The first member states `a:off` of nought, so it is the envelope's own corner.
        members[0].Area.X.ShouldBe(envelope.Area.X);
        members[0].Area.Y.ShouldBe(envelope.Area.Y);

        // And the last one is inside the envelope rather than below it, which is what hanging each
        // member by its own height would give.
        members[2].Area.Bottom.ShouldBeLessThanOrEqualTo(envelope.Area.Bottom);
    }

    private static IReadOnlyList<PlacedFrame> Members()
        => [.. Frames().Where(frame => frame.Frame.GroupSize is not null)];

    private static IReadOnlyList<PlacedFrame> Frames()
    {
        using MemoryStream package = BuildPackage();
        using DocumentSource source = DocumentSource.FromStream(package, "inline-canvas.docx");
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return [.. pages.Pages.SelectMany(page => page.Frames)
                     .Where(frame => frame.Frame.Anchor == FrameAnchor.AsCharacter)];
    }

    private static MemoryStream BuildPackage()
    {
        // 914400 EMU to the inch: three 1 x 0.5 in rectangles stepped diagonally inside a 4 x 2 in
        // canvas, so that no two share a row or a column and a stack cannot be mistaken for a
        // layout.
        string members = string.Concat(
            Rectangle("FF0000", 0, 0),
            Rectangle("00FF00", 1371600, 457200),
            Rectangle("0000FF", 2743200, 914400));

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}"
                        xmlns:wpc="{Wpc}">
              <w:body>
                <w:p>
                  <w:r>
                    <w:drawing>
                      <wp:inline distT="0" distB="0" distL="0" distR="0">
                        <wp:extent cx="3657600" cy="1828800"/>
                        <wp:effectExtent l="0" t="0" r="0" b="0"/>
                        <wp:docPr id="1" name="Canvas"/>
                        <a:graphic><a:graphicData uri="{Wpc}">
                          <wpc:wpc><wpc:bg><a:noFill/></wpc:bg><wpc:whole/>{members}</wpc:wpc>
                        </a:graphicData></a:graphic>
                      </wp:inline>
                    </w:drawing>
                  </w:r>
                </w:p>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/settings.xml", Settings);
            Write(archive, "word/document.xml", document);
        }

        result.Position = 0;
        return result;

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(Encoding.UTF8.GetBytes(content));
        }
    }

    private static string Rectangle(string colour, int x, int y)
        => $"""
           <wps:wsp>
             <wps:cNvPr id="0" name="m{colour}"/><wps:cNvSpPr/>
             <wps:spPr>
               <a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="914400" cy="457200"/></a:xfrm>
               <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
               <a:solidFill><a:srgbClr val="{colour}"/></a:solidFill>
               <a:ln w="0"><a:noFill/></a:ln>
             </wps:spPr>
             <wps:bodyPr/>
           </wps:wsp>
           """;

    private const string ContentTypes = """
        <?xml version="1.0" encoding="UTF-8"?>
        <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
          <Default Extension="rels"
                   ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
          <Default Extension="xml" ContentType="application/xml"/>
          <Override PartName="/word/document.xml"
                    ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
          <Override PartName="/word/settings.xml"
                    ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
        </Types>
        """;

    private const string RootRelationships = """
        <?xml version="1.0" encoding="UTF-8"?>
        <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
          <Relationship Id="rId1" Target="word/document.xml"
                        Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
        </Relationships>
        """;

    private const string DocumentRelationships = """
        <?xml version="1.0" encoding="UTF-8"?>
        <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
          <Relationship Id="rId1" Target="settings.xml"
                        Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"/>
        </Relationships>
        """;

    private const string Settings = """
        <?xml version="1.0" encoding="UTF-8"?>
        <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
          <w:compat>
            <w:compatSetting w:name="compatibilityMode"
                             w:uri="http://schemas.microsoft.com/office/word" w:val="15"/>
          </w:compat>
        </w:settings>
        """;

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
    private const string Wpc = "http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas";
}
