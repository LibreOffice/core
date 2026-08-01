using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.MsBinary.Escher;
using Paperless.Presentations.MsBinary;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Tests the vertex-array path and the property-table rule it depends on, on hand-built tables.
/// </summary>
/// <remarks>
/// Synthetic rather than corpus input, and for a reason worth stating: the only PPT files this
/// project can produce are ones LibreOffice wrote, and <strong>LibreOffice never writes a vertex
/// array with literal coordinates</strong> — its exporter writes every preset shape's path as
/// references into a formula table instead. So the shape of array this code exists to read cannot
/// be obtained from the corpus at all, and a file authored by PowerPoint would have to be found
/// to get one. Building the six bytes by hand is the honest alternative.
/// </remarks>
public class PptCustomGeometryTests
{
    /// <summary>The identifiers this exercises, as the drawing layer numbers them.</summary>
    private const ushort Vertices = 325;
    private const ushort Segments = 326;
    private const ushort ShapeName = 896;

    [Fact]
    public void AnArrayPropertyMayLeaveItsOwnHeaderOutOfItsStatedLength()
    {
        // The trap. A writer may state an array's length as count x size, leaving the six bytes of
        // the array's own header out of the number — and complex values sit in one block in
        // property order, so under-counting one shifts every property after it. Here the shape's
        // name follows the vertex array; read the length as stated and the name comes back as the
        // last six bytes of the array followed by the first characters of the name, which is
        // plausible text rather than an error.
        EscherPropertyTable table = Build(
            (Vertices, Array(3, 8, [0, 0, 100, 0, 200, 200])),
            (ShapeName, Text("Freeform 7")));

        table.Text(ShapeName).ShouldBe("Freeform 7");

        ReadOnlySpan<byte> elements = table.Array(Vertices, out int count, out int size);
        count.ShouldBe(3);
        size.ShouldBe(8);
        elements.Length.ShouldBeGreaterThanOrEqualTo(24);
    }

    [Fact]
    public void AVertexArrayWithASegmentProgramBecomesThePathItDescribes()
    {
        // A right-angled triangle in the default 21600 view box: move to the bottom left, two
        // lines, close, end. Mapped onto a 144 x 72 pt shape it is (0,72), (0,0), (144,72).
        EscherPropertyTable table = Build(
            (Vertices, Array(3, 8, [0, 21600, 0, 0, 21600, 21600])),
            (Segments, Array(4, 2, [0x4000, 0x0002, 0x6000, 0x8000])));

        GraphicsPath path = PptCustomGeometry.Outline(
            table, new DocSize(Length.FromPoints(144), Length.FromPoints(72)))
            .ShouldNotBeNull();

        Corners(path).ShouldBe([(0, 72), (0, 0), (144, 72)]);
        path.Commands[^1].Verb.ShouldBe(PathVerb.Close);
    }

    [Fact]
    public void AVertexArrayWithNoSegmentProgramIsAClosedPolygon()
    {
        EscherPropertyTable table = Build(
            (Vertices, Array(4, 8, [0, 0, 21600, 0, 21600, 21600, 0, 21600])));

        GraphicsPath path = PptCustomGeometry.Outline(
            table, new DocSize(Length.FromPoints(100), Length.FromPoints(50)))
            .ShouldNotBeNull();

        Corners(path).ShouldBe([(0, 0), (100, 0), (100, 50), (0, 50)]);
    }

    [Fact]
    public void AVertexReferringToAGuideIsDeclinedRatherThanReadAsACoordinate()
    {
        // The top bit means "the low bits index the formula table", so the value is an expression
        // and not a position. Read as a signed coordinate it is about minus two thousand million,
        // which maps to a path several thousand kilometres across; declining leaves the caller to
        // draw the bounding rectangle instead. This is the shape of array LibreOffice's own PPT
        // export writes for every preset it emits.
        EscherPropertyTable table = Build(
            (Vertices, Array(3, 8, [0, unchecked((int)0x80000007), 0, 0, unchecked((int)0x80000009), 0])));

        PptCustomGeometry.Has(table).ShouldBeTrue();
        PptCustomGeometry.Outline(
            table, new DocSize(Length.FromPoints(144), Length.FromPoints(72))).ShouldBeNull();
    }

    private static List<(double X, double Y)> Corners(GraphicsPath path)
        => [.. path.Commands
            .Where(command => command.Verb != PathVerb.Close)
            .Select(command => (
                Math.Round(command.Point.X.Points, 3), Math.Round(command.Point.Y.Points, 3)))];

    /// <summary>
    /// An <c>IMsoArray</c> payload: three words of header, then the elements, with the header
    /// deliberately left out of the length the caller will state.
    /// </summary>
    private static byte[] Array(int count, int elementSize, int[] values)
    {
        List<byte> bytes =
        [
            .. BitConverter.GetBytes((ushort)count),
            .. BitConverter.GetBytes((ushort)count),
            .. BitConverter.GetBytes((ushort)elementSize),
        ];

        foreach (int value in values)
        {
            bytes.AddRange(elementSize >= 8
                ? BitConverter.GetBytes(value)
                : BitConverter.GetBytes((ushort)value));
        }

        return [.. bytes];
    }

    private static byte[] Text(string value)
        => System.Text.Encoding.Unicode.GetBytes(value + '\0');

    /// <summary>
    /// Builds a property table whose complex values follow the fixed entries in property order,
    /// with every array property's length stated the short way.
    /// </summary>
    private static EscherPropertyTable Build(params (ushort Id, byte[] Data)[] properties)
    {
        List<byte> entries = [];
        List<byte> complex = [];

        foreach ((ushort id, byte[] data) in properties)
        {
            // An array states its length without its own six-byte header; anything else states
            // the whole of it.
            bool isArray = id is Vertices or Segments;
            uint stated = (uint)(isArray ? data.Length - 6 : data.Length);

            entries.AddRange(BitConverter.GetBytes((ushort)(id | 0x8000)));
            entries.AddRange(BitConverter.GetBytes(stated));
            complex.AddRange(data);
        }

        return EscherPropertyTable.Read([.. entries, .. complex], properties.Length);
    }
}
