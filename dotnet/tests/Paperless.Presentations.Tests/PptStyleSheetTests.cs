using Paperless.Core.Extraction;
using Paperless.MsBinary.Records;
using Paperless.Presentations.MsBinary;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Tests the per-outline-level defaults a master's <c>TxMasterStyleAtom</c> records state.
/// </summary>
/// <remarks>
/// Synthetic atoms, because the corpus cannot separate the cases that matter. Every deck
/// LibreOffice writes states the same five levels for every instance with the same mask, so a
/// reader that got the first level's field order wrong, or that skipped the level-to-level
/// inheritance, would still read those files perfectly and then misread the first deck written
/// by anything else. The record has no length per field and no marker between them: one
/// misjudged optional field silently shifts everything after it, and the result parses.
/// </remarks>
public class PptStyleSheetTests
{
    [Fact]
    public void TheFirstLevelOfAnAtomUsesADifferentFieldOrderFromTheOnesAfterIt()
    {
        // Bit 0x200000 is a tab-stop array on the first level and a two-byte text direction on
        // every later one, so the two layouts differ in size and not merely in meaning. A reader
        // using the later layout throughout consumes ten bytes too few here and reads the
        // character mask out of the middle of the tab stops.
        AtomBuilder atom = new(levels: 2);
        atom.Paragraph(mask: 0x00200000, tabStops: 2);
        atom.Character(mask: 0x0001, flags: 0x0001);
        atom.Paragraph(mask: 0x00200000, fields: [0x0000]);
        atom.Character(mask: 0x0003, flags: 0x0002);

        PptStyleSheet sheet = Read(master: [(PptTextKind.Title, atom)]);

        sheet.Character(PptTextKind.Title, 0).Emphasis.ShouldBe(RunEmphasis.Bold);
        sheet.Character(PptTextKind.Title, 1).Emphasis.ShouldBe(RunEmphasis.Italic);
    }

    [Fact]
    public void ALevelInheritsEverythingTheLevelAboveItStated()
    {
        AtomBuilder atom = new(levels: 3);
        atom.Paragraph(mask: 0);
        atom.Character(mask: 0x00020001, flags: 0x0001, fields: [28]);   // bold, 28 point
        atom.Paragraph(mask: 0);
        atom.Character(mask: 0x00020000, fields: [24]);                  // only the size changes
        atom.Paragraph(mask: 0);
        atom.Character(mask: 0);

        PptStyleSheet sheet = Read(master: [(PptTextKind.Body, atom)]);

        sheet.Character(PptTextKind.Body, 0).FontHeight.ShouldBe((ushort)28);
        sheet.Character(PptTextKind.Body, 1).FontHeight.ShouldBe((ushort)24);
        sheet.Character(PptTextKind.Body, 2).FontHeight.ShouldBe((ushort)24);

        // Boldness was stated once, at the top, and reaches the deepest level through the copy
        // rather than through a second statement.
        sheet.Character(PptTextKind.Body, 2).Emphasis.ShouldBe(RunEmphasis.Bold);
    }

    [Fact]
    public void AMaskThatNamesAnAttributeWithItsFlagClearTurnsTheInheritedOneOff()
    {
        AtomBuilder atom = new(levels: 2);
        atom.Paragraph(mask: 0);
        atom.Character(mask: 0x0003, flags: 0x0003);   // bold and italic
        atom.Paragraph(mask: 0);
        atom.Character(mask: 0x0001, flags: 0x0000);   // bold named, and cleared

        PptStyleSheet sheet = Read(master: [(PptTextKind.Body, atom)]);

        sheet.Character(PptTextKind.Body, 0).Emphasis.ShouldBe(RunEmphasis.Bold | RunEmphasis.Italic);
        sheet.Character(PptTextKind.Body, 1).Emphasis.ShouldBe(RunEmphasis.Italic);
    }

    [Fact]
    public void AnInstanceAboveTextInAShapePrefixesEveryLevelWithAWordAndNeverUsesTheFirstLayout()
    {
        // Instance five onwards writes an unexplained word before each level's paragraph mask
        // and reads every level with the later layout — including the first. Reading it as the
        // other five instances are read puts the paragraph mask two bytes early.
        AtomBuilder atom = new(levels: 2, prefixed: true);
        atom.Paragraph(mask: 0);
        atom.Character(mask: 0x0004, flags: 0x0004);
        atom.Paragraph(mask: 0);
        atom.Character(mask: 0x0100, flags: 0x0100);

        PptStyleSheet sheet = Read(master: [(PptTextKind.CentreBody, atom)]);

        sheet.Character(PptTextKind.CentreBody, 0).Emphasis.ShouldBe(RunEmphasis.Underline);

        // These four instances also skip the level-to-level copy the other five do, so the
        // underline stated at level zero does not reach level one.
        sheet.Character(PptTextKind.CentreBody, 1).Emphasis.ShouldBe(RunEmphasis.Strikethrough);
    }

    [Fact]
    public void AnInstanceTheMasterNeverStatesIsACopyOfTheOneItVariesOn()
    {
        AtomBuilder body = new(levels: 1);
        body.Paragraph(mask: 0);
        body.Character(mask: 0x00020001, flags: 0x0001, fields: [26]);

        // No atom for the half-body or the subtitle: both are variations on the body, and a
        // reader that left them at the bare defaults would report a two-column slide's text at
        // 32 point and unemphasised where LibreOffice reports 26 point and bold.
        PptStyleSheet sheet = Read(master: [(PptTextKind.Body, body)]);

        sheet.Character(PptTextKind.HalfBody, 0).FontHeight.ShouldBe((ushort)26);
        sheet.Character(PptTextKind.HalfBody, 0).Emphasis.ShouldBe(RunEmphasis.Bold);
        sheet.Character(PptTextKind.CentreBody, 0).FontHeight.ShouldBe((ushort)26);
    }

    [Fact]
    public void TheEnvironmentsCopyBeatsTheMastersForTextInAShape()
    {
        AtomBuilder environment = new(levels: 1);
        environment.Paragraph(mask: 0);
        environment.Character(mask: 0x00020001, flags: 0x0001, fields: [14]);

        AtomBuilder master = new(levels: 1);
        master.Paragraph(mask: 0);
        master.Character(mask: 0x00020002, flags: 0x0002, fields: [40]);

        // LibreOffice reads the environment's atom first and then refuses the master's, on the
        // observation that the document-wide one wins (svdfppt.cxx:4117-4197). Reading them the
        // other way round — or letting the master overwrite — takes the wrong size and the
        // wrong emphasis for every plain text box in the deck.
        PptStyleSheet sheet = Read(
            master: [(PptTextKind.Other, master)],
            environment: environment);

        sheet.Character(PptTextKind.Other, 0).FontHeight.ShouldBe((ushort)14);
        sheet.Character(PptTextKind.Other, 0).Emphasis.ShouldBe(RunEmphasis.Bold);
    }

    [Fact]
    public void AnInstanceWithNoAtomAnywhereKeepsTheFormatsOwnDefaults()
    {
        PptStyleSheet sheet = Read(master: []);

        // A deck whose master states nothing still has to report a title at 44 point and a
        // speaker note at 12; the defaults are in PPTCharSheet's constructor, not in the file.
        sheet.Character(PptTextKind.Title, 0).FontHeight.ShouldBe((ushort)44);
        sheet.Character(PptTextKind.Body, 0).FontHeight.ShouldBe((ushort)32);
        sheet.Character(PptTextKind.Notes, 0).FontHeight.ShouldBe((ushort)12);
        sheet.Character(PptTextKind.Other, 0).FontHeight.ShouldBe((ushort)24);

        // And a body level draws a bullet by default while a title does not.
        sheet.Paragraph(PptTextKind.Body, 0).HasBullet.ShouldBeTrue();
        sheet.Paragraph(PptTextKind.Title, 0).HasBullet.ShouldBeFalse();
    }

    [Fact]
    public void ADepthBeyondTheStatedLevelsResolvesAgainstTheDeepestOneTheFormatAllows()
    {
        AtomBuilder atom = new(levels: 5);
        for (int level = 0; level < 5; level++)
        {
            atom.Paragraph(mask: 0);
            atom.Character(mask: 0x00020000, fields: [(ushort)(30 - (level * 2))]);
        }

        PptStyleSheet sheet = Read(master: [(PptTextKind.Body, atom)]);

        // A character run clamps to level four however deep its paragraph claims to be, which
        // is not the same clamp the paragraph properties use — LibreOffice allows those nine.
        sheet.Character(PptTextKind.Body, 4).FontHeight.ShouldBe((ushort)22);
        sheet.Character(PptTextKind.Body, 9).FontHeight.ShouldBe((ushort)22);
    }

    /// <summary>Reads a synthetic master container, and optionally an environment beside it.</summary>
    private static PptStyleSheet Read(
        (PptTextKind Kind, AtomBuilder Atom)[] master, AtomBuilder? environment = null)
    {
        List<byte> bytes = [];

        int masterStart = bytes.Count;
        List<byte> masterBody = [];
        foreach ((PptTextKind kind, AtomBuilder atom) in master)
        {
            masterBody.AddRange(
                Record(PptRecordTypes.TxMasterStyleAtom, (ushort)kind, atom.Build()));
        }

        bytes.AddRange(Container(PptRecordTypes.MainMaster, masterBody));

        int environmentStart = bytes.Count;
        if (environment is not null)
        {
            bytes.AddRange(Container(
                PptRecordTypes.Environment,
                Record(PptRecordTypes.TxMasterStyleAtom, (ushort)PptTextKind.Other, environment.Build())));
        }

        DffRecordBuffer buffer = new([.. bytes]);
        buffer.TryReadHeader(masterStart, out DffRecordHeader masterHeader).ShouldBeTrue();

        DffRecordHeader? environmentHeader = null;
        if (environment is not null)
        {
            buffer.TryReadHeader(environmentStart, out DffRecordHeader header).ShouldBeTrue();
            environmentHeader = header;
        }

        return PptStyleSheet.Read(buffer, masterHeader, environmentHeader);
    }

    private static List<byte> Container(ushort type, IEnumerable<byte> payload)
    {
        List<byte> body = [.. payload];
        List<byte> bytes =
        [
            0x0F, 0x00,
            (byte)type, (byte)(type >> 8),
            .. BitConverter.GetBytes((uint)body.Count),
            .. body,
        ];
        return bytes;
    }

    private static List<byte> Record(ushort type, ushort instance, byte[] payload)
    {
        ushort versionAndInstance = (ushort)(instance << 4);
        List<byte> bytes =
        [
            (byte)versionAndInstance, (byte)(versionAndInstance >> 8),
            (byte)type, (byte)(type >> 8),
            .. BitConverter.GetBytes((uint)payload.Length),
            .. payload,
        ];
        return bytes;
    }

    /// <summary>Assembles one <c>TxMasterStyleAtom</c>: a level count, then level after level.</summary>
    private sealed class AtomBuilder
    {
        private readonly List<byte> _bytes;
        private readonly bool _prefixed;

        public AtomBuilder(int levels, bool prefixed = false)
        {
            _bytes = [.. BitConverter.GetBytes((ushort)levels)];
            _prefixed = prefixed;
        }

        /// <summary>Adds a level's paragraph properties.</summary>
        public void Paragraph(uint mask, ushort[]? fields = null, int tabStops = 0)
        {
            if (_prefixed) _bytes.AddRange(BitConverter.GetBytes((ushort)0));

            _bytes.AddRange(BitConverter.GetBytes(mask));
            foreach (ushort field in fields ?? []) _bytes.AddRange(BitConverter.GetBytes(field));

            if (tabStops <= 0) return;
            _bytes.AddRange(BitConverter.GetBytes((ushort)tabStops));
            for (int i = 0; i < tabStops; i++) _bytes.AddRange(BitConverter.GetBytes(0u));
        }

        /// <summary>Adds a level's character properties, closing the level.</summary>
        public void Character(uint mask, ushort flags = 0, ushort[]? fields = null)
        {
            _bytes.AddRange(BitConverter.GetBytes(mask));
            if ((mask & 0xFFFF) != 0) _bytes.AddRange(BitConverter.GetBytes(flags));
            foreach (ushort field in fields ?? []) _bytes.AddRange(BitConverter.GetBytes(field));
        }

        public byte[] Build() => [.. _bytes];
    }
}
