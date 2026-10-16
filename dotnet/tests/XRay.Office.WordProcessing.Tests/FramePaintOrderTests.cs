using System.Xml.Linq;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.MsBinary.Escher;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using XRay.Office.WordProcessing.Ww8;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Which floating frames are painted <em>before</em> the text, and which after.
/// </summary>
/// <remarks>
/// <para>
/// The defect these were written against renders a whole document as blank sheets while every gate
/// column reads normal. <c>words/extra-001/doc/info-bulletin-601.doc</c> carries one full-page opaque
/// letterhead raster on each of its pages, anchored in the header story with wrap 3; we emitted it after
/// the text it is meant to sit behind, so the PDF's five pages are five pictures with the text buried
/// underneath. <c>pdftotext</c> still found 1298 words of 1302 and the page count was the only column
/// that moved.
/// </para>
/// <para>
/// The rules are LibreOffice's, and the two formats state the thing differently enough that copying one
/// onto the other would be wrong in both directions — see <see cref="PageFrame.BehindText"/> for the
/// citations. What is tested here is that each reader answers what its own importer answers, and that
/// <see cref="PageDrawing"/> then emits the two groups in the right order.
/// </para>
/// </remarks>
public sealed class FramePaintOrderTests
{
    /// <summary>
    /// A header's shapes are painted before the body text, fills and outlines and all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// End to end, through the real reader, over <c>header-behind-text.docx</c> — authored by
    /// <c>probes/words-e1-01/make-fixture.py</c> and checked against LibreOffice's own rendering of it,
    /// which emits its content stream in exactly this order: the 50% alpha panel as a transparency
    /// group, then the grey box's fill and its red outline, and only then <c>BT … BODYLINE … ET</c>.
    /// </para>
    /// <para>
    /// The assertion is on the <em>interleaving</em>, which is why the sink here keeps one flat log
    /// instead of using <c>RecordingDrawingSink</c>'s per-kind lists: the defect was never a missing
    /// operation, it was operations in the wrong order, and a recorder that keeps fills and glyph runs
    /// apart cannot see that at all.
    /// </para>
    /// </remarks>
    [Fact]
    public void AHeadersShapesArePaintedBeforeTheBodyText()
    {
        Draw("header-behind-text.docx").Log
            .ShouldBe(["fill", "fill", "stroke", "glyph"]);
    }

    /// <summary>The fill and the outline are the ones the file states.</summary>
    /// <remarks>
    /// <para>
    /// Both are read off LibreOffice's own PDF of the same fixture rather than restated from the markup:
    /// it writes the box as <c>0.4666666667 0.4666666667 0.4666666667 rg … f*</c> — which is
    /// <c>777777</c> — and its outline as <c>1 0 0 RG</c> with <c>1 w</c>, a 12700 EMU line.
    /// </para>
    /// <para>
    /// The alpha panel is asserted too, because it is the half a grep of a content stream cannot find:
    /// LibreOffice exports an <c>a:alpha</c> fill as a transparency-group XObject rather than as a plain
    /// <c>re f</c>, which is why a round that looked for the fill in the operators concluded the
    /// reference drew none.
    /// </para>
    /// </remarks>
    [Fact]
    public void AShapesFillAndOutlineAreTheOnesTheFileStates()
    {
        OrderedSink sink = Draw("header-behind-text.docx");

        // 50% of black, from `<a:alpha val="50000"/>`, and then the opaque 0x777777 box.
        sink.Fills.ShouldBe([new Colour(0, 0, 0, 128), new Colour(0x77, 0x77, 0x77)]);

        Stroke outline = sink.Strokes.ShouldHaveSingleItem();
        outline.Paint.ShouldBe(Paint.Solid(new Colour(0xFF, 0, 0)));
        outline.Width.ShouldBe(Length.FromPoints(1));
    }

    /// <summary>
    /// A DOC shape anchored in the header story with wrap 3 goes behind the text.
    /// </summary>
    /// <remarks>
    /// The second half of <c>SwWW8ImplReader</c>'s <c>bMoveToBackground</c>
    /// (<c>sw/source/filter/ww8/ww8graf.cxx</c>:2833), and the half the corpus turns on: a Word letterhead
    /// states no <c>fBehindDocument</c> at all and relies entirely on being in the header with a
    /// run-through wrap.
    /// </remarks>
    [Fact]
    public void ADocShapeInTheHeaderWithARunThroughWrapGoesBehindTheText()
    {
        Placed(Anchor(header: true, wrap: 3), EscherPropertyTable.Empty)
            .BehindText.ShouldBeTrue();
    }

    /// <summary>The same shape in the body does not.</summary>
    /// <remarks>
    /// The discriminating half of the pair. A reader that answered "behind" to every wrap-3 shape would
    /// pass the test above and put every floating body picture under its own text.
    /// </remarks>
    [Fact]
    public void TheSameShapeInTheBodyStaysInFrontOfTheText()
    {
        Placed(Anchor(header: false, wrap: 3), EscherPropertyTable.Empty)
            .BehindText.ShouldBeFalse();
    }

    /// <summary>A header shape that leaves a hole in the text is not behind it.</summary>
    /// <remarks>
    /// <c>nwr</c> is tested for 3 exactly, not for "some wrap": the C++ writes <c>aFSFA.nwr == 3</c>. A
    /// shape the text flows around is one the text is not underneath.
    /// </remarks>
    [Fact]
    public void AHeaderShapeThatWrapsTextAroundItselfIsNotBehindIt()
    {
        Placed(Anchor(header: true, wrap: 2), EscherPropertyTable.Empty)
            .BehindText.ShouldBeFalse();
    }

    /// <summary>
    /// The Escher <c>fBehindDocument</c> bit puts a body shape behind the text on its own.
    /// </summary>
    /// <remarks>
    /// Bit 5 of the <c>DFF_Prop_fPrint</c> group, which is property 954 in the group-relative numbering
    /// <see cref="EscherPropertyTable.Boolean"/> uses. The mask below sets the value bit and its
    /// "is stated" companion sixteen places up, which is how the format says a boolean is present.
    /// </remarks>
    [Fact]
    public void TheEscherBehindDocumentBitPutsABodyShapeBehindTheText()
    {
        Placed(Anchor(header: false, wrap: 1), Table((BooleanGroup, 0x0020_0020u)))
            .BehindText.ShouldBeTrue();
    }

    /// <summary>
    /// The neighbouring bit in the same group is <c>fHidden</c> and must not be read as this one.
    /// </summary>
    /// <remarks>
    /// The failure a bit-shift makes: 0x02 is <c>fHidden</c> and 0x20 is <c>fBehindDocument</c>, four
    /// places apart in one 32-bit word. A shape stating only the first is a shape Word prints normally.
    /// </remarks>
    [Fact]
    public void TheHiddenBitInTheSameGroupIsNotTheBehindDocumentBit()
    {
        Build(Anchor(header: false, wrap: 1), Table((BooleanGroup, 0x0002_0002u)))
            .ShouldBeNull("a hidden shape is not placed at all");

        Placed(Anchor(header: false, wrap: 1), Table((BooleanGroup, 0x0002_0000u)))
            .BehindText.ShouldBeFalse("stated false is still not behind the text");
    }

    /// <summary>
    /// A DOCX drawing anchored in a header is behind the text without saying so.
    /// </summary>
    /// <remarks>
    /// <c>m_bOpaque</c> is initialised to <c>!IsInHeaderFooter()</c>
    /// (<c>GraphicImport.cxx</c>:342) and nothing in an ordinary header drawing puts it back, so the
    /// default is the rule rather than an edge case.
    /// </remarks>
    [Fact]
    public void ADocxDrawingInAHeaderIsBehindTheTextWithoutSayingSo()
    {
        Frame(behindDoc: null, wrap: "wrapSquare", inHeaderFooter: true, compatibility: 15)
            .BehindText.ShouldBeTrue();

        Frame(behindDoc: null, wrap: "wrapSquare", inHeaderFooter: false, compatibility: 15)
            .BehindText.ShouldBeFalse();
    }

    /// <summary>
    /// <c>behindDoc</c> with <c>wrapNone</c> is honoured whatever the compatibility mode.
    /// </summary>
    /// <remarks>
    /// tdf#137850's exception: the restoring branch is on the four wraps that leave a hole, and
    /// <c>wrapNone</c> is not one of them, so a modern file's <c>behindDoc</c> still means what it says
    /// there.
    /// </remarks>
    [Fact]
    public void BehindDocWithNoWrapIsHonouredUnderEveryCompatibilityMode()
    {
        Frame(behindDoc: "1", wrap: "wrapNone", inHeaderFooter: false, compatibility: 15)
            .BehindText.ShouldBeTrue();

        Frame(behindDoc: "1", wrap: "wrapNone", inHeaderFooter: false, compatibility: 12)
            .BehindText.ShouldBeTrue();
    }

    /// <summary>
    /// With a wrap that leaves a hole, <c>behindDoc</c> is honoured before Word 2013 and ignored after.
    /// </summary>
    /// <remarks>
    /// The one case where the compatibility mode decides paint order, and the reason the reader is given
    /// the mode at all. Both halves are asserted, because a reader that ignored the mode would pass
    /// whichever half its fixed answer happened to be.
    /// </remarks>
    [Fact]
    public void AWrapThatLeavesAHoleMakesBehindDocDependOnTheCompatibilityMode()
    {
        Frame(behindDoc: "1", wrap: "wrapSquare", inHeaderFooter: false, compatibility: 14)
            .BehindText.ShouldBeTrue();

        Frame(behindDoc: "1", wrap: "wrapSquare", inHeaderFooter: false, compatibility: 15)
            .BehindText.ShouldBeFalse();
    }

    /// <summary>A <c>wp:inline</c> is never behind the text, wherever it sits.</summary>
    /// <remarks>
    /// An as-character frame takes room on its own line rather than floating over anything, so its layer
    /// decides nothing visible. Asserted so that the header default above cannot leak into it.
    /// </remarks>
    [Fact]
    public void AnInlineDrawingIsNeverBehindTheText()
    {
        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}">
              <wp:inline>
                <wp:extent cx="914400" cy="457200"/>
                <a:graphic><a:graphicData><wps:wsp/></a:graphicData></a:graphic>
              </wp:inline>
            </w:drawing>
            """);

        DocxFrames.ReadAll(drawing, null, anchorOffset: 0, pictures: null,
                           new DocxFrameContext(InHeaderFooter: true, CompatibilityMode: 15))
            .ShouldHaveSingleItem()
            .BehindText.ShouldBeFalse();
    }

    /// <summary>The <c>DFF_Prop_fPrint</c> boolean group's own identifier.</summary>
    private const ushort BooleanGroup = 959;

    private static PageFrame Frame(
        string? behindDoc, string wrap, bool inHeaderFooter, int compatibility)
    {
        string attribute = behindDoc is null ? string.Empty : $""" behindDoc="{behindDoc}" """;
        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}">
              <wp:anchor{attribute}>
                <wp:extent cx="914400" cy="457200"/>
                <wp:{wrap}/>
                <a:graphic><a:graphicData><wps:wsp/></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        return DocxFrames
            .ReadAll(drawing, null, anchorOffset: 0, pictures: null,
                     new DocxFrameContext(null, inHeaderFooter, compatibility))
            .ShouldHaveSingleItem();
    }

    private static PageFrame Placed(Ww8ShapeAnchor anchor, EscherPropertyTable properties)
        => Build(anchor, properties).ShouldNotBeNull("the shape is placeable");

    private static PageFrame? Build(Ww8ShapeAnchor anchor, EscherPropertyTable properties)
        => Ww8Frames.Build(
            anchor,
            new EscherShape
            {
                ShapeId = 1,
                ShapeType = EscherShapeTypes.TextBox,
                Properties = properties,
            },
            offset: 0,
            blocks: []);

    private static Ww8ShapeAnchor Anchor(bool header, int wrap) => new(
        Position: 0,
        ShapeId: 1,
        Left: 0,
        Top: 0,
        Right: 4000,
        Bottom: 500,
        IsHeaderAnchor: header,
        HorizontalOrigin: Ww8ShapeOrigin.Page,
        VerticalOrigin: Ww8ShapeOrigin.Page,
        Wrap: wrap,
        WrapSide: 0,
        IsPageRelative: false,
        IsBelowText: false);

    /// <summary>A property table holding exactly these entries.</summary>
    private static EscherPropertyTable Table(params (ushort Id, uint Value)[] entries)
    {
        List<byte> content = [];
        foreach ((ushort id, uint value) in entries)
        {
            content.Add((byte)id);
            content.Add((byte)(id >> 8));
            for (int i = 0; i < 4; i++) content.Add((byte)(value >> (i * 8)));
        }

        return EscherPropertyTable.Read(content.ToArray(), entries.Length);
    }

    /// <summary>Every drawing operation a fixture's rendering emits, in order.</summary>
    private static OrderedSink Draw(string name)
    {
        OrderedSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(name)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return sink;
    }

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";

    /// <summary>
    /// A sink that records the order operations arrived in, and nothing else.
    /// </summary>
    /// <remarks>
    /// Deliberately not <c>RecordingDrawingSink</c>: that one keeps fills, strokes and glyph runs in
    /// separate lists, which answers "what was drawn" and cannot answer "in what order" — the only
    /// question here.
    /// </remarks>
    private sealed class OrderedSink : IDrawingSink
    {
        public List<string> Log { get; } = [];

        public List<Colour> Fills { get; } = [];

        public List<Stroke> Strokes { get; } = [];

        public void BeginPage(DocSize size) { }

        public void EndPage() { }

        public void DrawGlyphRun(GlyphRun run, Paint paint) => Log.Add("glyph");

        public void DrawImage(RasterImage image, DocRect destination, double opacity = 1.0)
            => Log.Add("image");

        public void FillPath(GraphicsPath path, Paint paint, FillRule rule = FillRule.NonZero)
        {
            Log.Add("fill");
            if (paint is SolidPaint solid) Fills.Add(solid.Colour);
        }

        public void StrokePath(GraphicsPath path, Stroke stroke)
        {
            Log.Add("stroke");
            Strokes.Add(stroke);
        }

        public void ClipPath(GraphicsPath path, FillRule rule = FillRule.NonZero) { }

        public void Transform(AffineTransform transform) { }

        public void Save() { }

        public void Restore() { }

        public void BeginTransparencyGroup(double opacity) { }

        public void EndTransparencyGroup() { }
    }
}
