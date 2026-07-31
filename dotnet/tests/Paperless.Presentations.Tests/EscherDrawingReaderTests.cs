using System.Text;
using Paperless.Core.Diagnostics;
using Paperless.MsBinary.Escher;
using Paperless.MsBinary.Records;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Tests the shared Escher (MS-ODRAW) reader on hand-built drawings.
/// </summary>
/// <remarks>
/// The reader lives in <c>Paperless.MsBinary</c> because DOC and XLS delegate their drawings to
/// the same records, so it is tested on synthetic input rather than on a PowerPoint file: what
/// is under test is the format, not one host's use of it. The cases chosen are the ones a real
/// file cannot easily produce — a nesting depth meant to exhaust the stack, and a record whose
/// declared length runs past the buffer.
/// </remarks>
public class EscherDrawingReaderTests
{
    [Fact]
    public void AGroupsOwnShapeRecordIsTheGroupRatherThanAShapeInsideIt()
    {
        DrawingBuilder builder = new();
        builder.Drawing(dg =>
        {
            dg.Group(patriarch =>
            {
                patriarch.Shape(1, shapeType: 0, EscherShapeAttributes.Group | EscherShapeAttributes.Patriarch);
                patriarch.Shape(2, shapeType: 202, EscherShapeAttributes.HaveAnchor);
                patriarch.Group(inner =>
                {
                    inner.Shape(3, shapeType: 0, EscherShapeAttributes.Group | EscherShapeAttributes.Child);
                    inner.Shape(4, shapeType: 1, EscherShapeAttributes.Child);
                    inner.Shape(5, shapeType: 2, EscherShapeAttributes.Child);
                });
            });
        });

        IReadOnlyList<EscherShape> shapes = builder.Read();

        // The patriarch's own shape describes the drawing, and the inner group's own shape is
        // the group; only three real shapes exist, one of which holds two others.
        shapes.Select(s => s.ShapeId).ShouldBe([2u, 3u]);
        shapes[1].IsGroup.ShouldBeTrue();
        shapes[1].Children.Select(s => s.ShapeId).ShouldBe([4u, 5u]);
        shapes.SelectMany(s => s.SelfAndDescendants()).Count().ShouldBe(4);
    }

    [Fact]
    public void ADrawingNestedBeyondTheDepthCapIsTruncatedRatherThanOverflowingTheStack()
    {
        DrawingBuilder builder = new();
        builder.Drawing(dg => dg.Group(patriarch =>
        {
            patriarch.Shape(1, 0, EscherShapeAttributes.Group | EscherShapeAttributes.Patriarch);
            Nest(patriarch, EscherDrawingReader.MaxGroupDepth + 20);
        }));

        List<Diagnostic> diagnostics = [];
        IReadOnlyList<EscherShape> shapes = builder.Read(diagnostics);

        shapes.Count.ShouldBe(1);

        int depth = 0;
        for (EscherShape? shape = shapes[0]; shape is not null;
             shape = shape.Children.Count > 0 ? shape.Children[0] : null)
        {
            depth++;
        }

        depth.ShouldBeLessThanOrEqualTo(EscherDrawingReader.MaxGroupDepth + 1);
        diagnostics.ShouldContain(d => d.Code == "PL2350");

        static void Nest(DrawingBuilder.Container parent, int remaining)
        {
            if (remaining == 0) return;
            parent.Group(group =>
            {
                group.Shape(1, 0, EscherShapeAttributes.Group | EscherShapeAttributes.Child);
                Nest(group, remaining - 1);
            });
        }
    }

    [Fact]
    public void ARecordWhoseLengthOverrunsTheBufferYieldsWhatDidFit()
    {
        DrawingBuilder builder = new();
        builder.Drawing(dg => dg.Group(patriarch =>
        {
            patriarch.Shape(1, 0, EscherShapeAttributes.Group | EscherShapeAttributes.Patriarch);
            patriarch.Shape(2, 202, EscherShapeAttributes.HaveAnchor);
        }));

        // Truncating the buffer is what a partially downloaded or damaged file looks like:
        // every enclosing container now declares more than the stream holds.
        byte[] truncated = builder.Bytes[..^4];
        DffRecordBuffer buffer = new(truncated);
        buffer.TryReadHeader(0, out DffRecordHeader drawing).ShouldBeTrue();

        List<Diagnostic> diagnostics = [];
        IReadOnlyList<EscherShape> shapes =
            new EscherDrawingReader(buffer, diagnostics).ReadDrawing(drawing);

        // The shape's identifier survived; its flags word did not, and reads as nothing rather
        // than throwing or being taken from whatever followed.
        shapes.Count.ShouldBe(1);
        shapes[0].ShapeId.ShouldBe(2u);
        shapes[0].Flags.ShouldBe(EscherShapeAttributes.None);
    }

    [Fact]
    public void APropertyTableFindsItsComplexValuesAfterTheFixedEntries()
    {
        // Two simple properties and one complex, whose four bytes state the length of the text
        // that follows the fixed entries rather than the text's value.
        byte[] name = Encoding.Unicode.GetBytes("Rounded rectangle\0");
        List<byte> content = [];
        AddEntry(content, EscherPropertyIds.Rotation, 90u << 16, complex: false);
        AddEntry(content, EscherPropertyIds.FillColour, 0x00FF7F3Fu, complex: false);
        AddEntry(content, EscherPropertyIds.ShapeName, (uint)name.Length, complex: true);
        content.AddRange(name);

        EscherPropertyTable table = EscherPropertyTable.Read(content.ToArray(), propertyCount: 3);

        table.Value(EscherPropertyIds.FillColour).ShouldBe(0x00FF7F3Fu);
        table.Value(EscherPropertyIds.Rotation).ShouldBe(90u << 16);

        // The stored NUL is inside the declared length, so a reader that keeps it produces a
        // name that compares unequal to the same text read anywhere else.
        table.Text(EscherPropertyIds.ShapeName).ShouldBe("Rounded rectangle");
        table.Has(EscherPropertyIds.Hidden).ShouldBeFalse();

        static void AddEntry(List<byte> into, ushort id, uint value, bool complex)
        {
            ushort raw = (ushort)(id | (complex ? 0x8000 : 0));
            into.Add((byte)raw);
            into.Add((byte)(raw >> 8));
            for (int i = 0; i < 4; i++) into.Add((byte)(value >> (i * 8)));
        }
    }

    [Fact]
    public void APropertyCountThatCouldNotFitIsClampedRatherThanTrusted()
    {
        // A count larger than the payload would put the complex block past the end and make
        // every complex value read from the wrong place, so it is clamped to what fits.
        byte[] content = new byte[12];
        EscherPropertyTable table = EscherPropertyTable.Read(content, propertyCount: 4000);
        table.Count.ShouldBeLessThanOrEqualTo(2);
    }

    /// <summary>Assembles an Escher drawing container.</summary>
    private sealed class DrawingBuilder
    {
        private readonly Container _root = new();

        public byte[] Bytes { get; private set; } = [];

        public void Drawing(Action<Container> build)
        {
            Container drawing = new();
            build(drawing);
            _root.Add(EscherRecordTypes.DrawingContainer, container: true, 0, drawing.Payload);
            Bytes = _root.Payload;
        }

        public IReadOnlyList<EscherShape> Read(List<Diagnostic>? diagnostics = null)
        {
            DffRecordBuffer buffer = new(Bytes);
            buffer.TryReadHeader(0, out DffRecordHeader drawing);
            return new EscherDrawingReader(buffer, diagnostics ?? []).ReadDrawing(drawing);
        }

        /// <summary>A record container being built.</summary>
        public sealed class Container
        {
            private readonly List<byte> _bytes = [];

            public byte[] Payload => [.. _bytes];

            public void Group(Action<Container> build)
            {
                Container group = new();
                build(group);
                Add(EscherRecordTypes.ShapeGroupContainer, container: true, 0, group.Payload);
            }

            public void Shape(uint id, ushort shapeType, EscherShapeAttributes flags)
            {
                Container shape = new();
                List<byte> payload = [];
                Append(payload, id);
                Append(payload, (uint)flags);
                shape.Add(EscherRecordTypes.Shape, container: false, shapeType, [.. payload]);
                Add(EscherRecordTypes.ShapeContainer, container: true, 0, shape.Payload);
            }

            public void Add(ushort type, bool container, ushort instance, byte[] payload)
            {
                ushort versionAndInstance = (ushort)((instance << 4) | (container ? 0x0F : 0x00));
                _bytes.Add((byte)versionAndInstance);
                _bytes.Add((byte)(versionAndInstance >> 8));
                _bytes.Add((byte)type);
                _bytes.Add((byte)(type >> 8));
                Append(_bytes, (uint)payload.Length);
                _bytes.AddRange(payload);
            }

            private static void Append(List<byte> into, uint value)
            {
                for (int i = 0; i < 4; i++) into.Add((byte)(value >> (i * 8)));
            }
        }
    }
}
