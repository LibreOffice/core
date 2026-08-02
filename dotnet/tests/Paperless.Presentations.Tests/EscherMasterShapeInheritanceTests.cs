using Paperless.Core.Diagnostics;
using Paperless.MsBinary.Escher;
using Paperless.MsBinary.Records;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// What a shape inherits from the master shape its <c>hspMaster</c> names.
/// </summary>
/// <remarks>
/// The defect this covers made a corpus deck unreadable rather than merely inexact: PowerPoint
/// states a placeholder's fill and line once, on the master's copy of it, and a reader that
/// does not follow the reference falls back to "a rectangle is filled by default" and paints an
/// opaque white box over the slide's own background, leaving white text on white. It is
/// invisible to any text comparison, because every value involved is a colour or a fill.
/// </remarks>
public class EscherMasterShapeInheritanceTests
{
    /// <summary>The fill boolean group, written under <c>fNoFillHitTest</c>.</summary>
    private const ushort FillGroup = 447;

    [Fact]
    public void ABooleanTheShapeLeavesUnstatedIsTakenFromItsMaster()
    {
        // The child states only bit 0 of the fill group and says nothing about fFilled (bit 4).
        EscherPropertyTable child = Table((FillGroup, 0x0001_0000u));

        // The master states bit 0 and bit 4, and sets bit 4 to false — "this placeholder has no
        // fill", which is how PowerPoint writes a master's title and body.
        EscherPropertyTable master = Table((FillGroup, 0x0011_0001u));

        EscherPropertyTable merged = child.InheritFrom(master);

        // fFilled now counts as stated, and as false — so the shape draws no fill at all, where
        // before it fell back to "a rectangle is filled by default" and painted itself white.
        merged.StatesBoolean(EscherPropertyIds.Filled).ShouldBeTrue();
        merged.Boolean(EscherPropertyIds.Filled).ShouldBeFalse();
    }

    [Fact]
    public void ABooleanTheShapeStatesItselfSurvivesItsMastersDifferentValue()
    {
        // The child hard-states fFilled true; the master hard-states it false. The child wins,
        // and the rest of the group still comes from the master — which is the whole reason a
        // boolean group merges bit by bit rather than as a value.
        EscherPropertyTable child = Table((FillGroup, 0x0010_0010u));
        EscherPropertyTable master = Table((FillGroup, 0x0011_0001u));

        EscherPropertyTable merged = child.InheritFrom(master);

        merged.Boolean(EscherPropertyIds.Filled).ShouldBeTrue();

        // Bit 0 is fNoFillHitTest, which only the master stated; taking the master's whole entry
        // would have lost the child's bit 4, and ignoring the master would have lost this one.
        (merged.Value(FillGroup) & 1).ShouldBe(1u);
    }

    [Fact]
    public void AnOrdinaryPropertyIsInheritedOnlyWhereTheShapeIsSilent()
    {
        EscherPropertyTable child = Table((EscherPropertyIds.LineColour, 0x00FF0000u));
        EscherPropertyTable master = Table(
            (EscherPropertyIds.LineColour, 0x000000FFu),
            (EscherPropertyIds.FillColour, 0x08000001u));

        EscherPropertyTable merged = child.InheritFrom(master);

        merged.Value(EscherPropertyIds.LineColour).ShouldBe(0x00FF0000u);

        // The fill colour the master states is a scheme index, and it is the only statement of
        // the colour anywhere: the slide's copy of the placeholder carries no fillColor at all.
        merged.Value(EscherPropertyIds.FillColour).ShouldBe(0x08000001u);
    }

    [Fact]
    public void AShapeNamingAnHspMasterInheritsFromItWhileTheReaderCanResolveOne()
    {
        // A slide placeholder as PowerPoint writes one: an hspMaster naming a shape on another
        // page, and almost nothing else.
        byte[] drawing = Drawing(
            (4100u, [(FillGroup, 0x0011_0001u), (EscherPropertyIds.FillColour, 0x08000002u)]),
            (7171u, [(FillGroup, 0x0001_0000u), (EscherPropertyIds.MasterShape, 4100u)]));

        DffRecordBuffer buffer = new(drawing);
        buffer.TryReadHeader(0, out DffRecordHeader container).ShouldBeTrue();

        // Without a resolver nothing is inherited, which is what every other host wants: Word and
        // Excel have no master shapes and must not pay for a lookup that can never hit.
        EscherDrawingReader plain = new(buffer, []);
        EscherShape unresolved = plain.ReadDrawing(container)[1];
        unresolved.Properties.StatesBoolean(EscherPropertyIds.Filled).ShouldBeFalse();

        // With one, the placeholder resolves exactly as the master's does.
        Dictionary<uint, EscherPropertyTable> byId =
            plain.ReadDrawing(container).ToDictionary(s => s.ShapeId, s => s.Properties);

        EscherDrawingReader resolving = new(buffer, [])
        {
            MasterShapeProperties = id => byId.GetValueOrDefault(id),
        };

        EscherShape resolved = resolving.ReadDrawing(container)[1];
        resolved.Properties.StatesBoolean(EscherPropertyIds.Filled).ShouldBeTrue();
        resolved.Properties.Boolean(EscherPropertyIds.Filled).ShouldBeFalse();
        resolved.Properties.Value(EscherPropertyIds.FillColour).ShouldBe(0x08000002u);
    }

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

    /// <summary>A <c>DgContainer</c> holding one group of leaf shapes with property tables.</summary>
    private static byte[] Drawing(params (uint Id, (ushort Id, uint Value)[] Properties)[] shapes)
    {
        List<byte> group = [];
        Record(group, EscherRecordTypes.ShapeContainer, container: true, 0,
               Shape(1, EscherShapeAttributes.Group | EscherShapeAttributes.Patriarch, []));

        foreach ((uint id, (ushort Id, uint Value)[] properties) in shapes)
        {
            Record(group, EscherRecordTypes.ShapeContainer, container: true, 0,
                   Shape(id, EscherShapeAttributes.HaveAnchor, properties));
        }

        List<byte> spgr = [];
        Record(spgr, EscherRecordTypes.ShapeGroupContainer, container: true, 0, [.. group]);

        List<byte> drawing = [];
        Record(drawing, EscherRecordTypes.DrawingContainer, container: true, 0, [.. spgr]);
        return [.. drawing];
    }

    private static byte[] Shape(
        uint id, EscherShapeAttributes flags, (ushort Id, uint Value)[] properties)
    {
        List<byte> shape = [];

        List<byte> sp = [];
        for (int i = 0; i < 4; i++) sp.Add((byte)(id >> (i * 8)));
        for (int i = 0; i < 4; i++) sp.Add((byte)((uint)flags >> (i * 8)));

        // Shape type 1 is a plain rectangle, which is filled by default — so a test that gets no
        // fill from its master is genuinely reading the master rather than falling through.
        Record(shape, EscherRecordTypes.Shape, container: false, instance: 1, [.. sp]);

        if (properties.Length > 0)
        {
            List<byte> table = [];
            foreach ((ushort pid, uint value) in properties)
            {
                table.Add((byte)pid);
                table.Add((byte)(pid >> 8));
                for (int i = 0; i < 4; i++) table.Add((byte)(value >> (i * 8)));
            }

            Record(shape, EscherRecordTypes.ShapeProperties, container: false,
                   (ushort)properties.Length, [.. table]);
        }

        return [.. shape];
    }

    private static void Record(
        List<byte> into, ushort type, bool container, ushort instance, byte[] payload)
    {
        ushort versionAndInstance = (ushort)((instance << 4) | (container ? 0x0F : 0x00));
        into.Add((byte)versionAndInstance);
        into.Add((byte)(versionAndInstance >> 8));
        into.Add((byte)type);
        into.Add((byte)(type >> 8));
        for (int i = 0; i < 4; i++) into.Add((byte)((uint)payload.Length >> (i * 8)));
        into.AddRange(payload);
    }
}
