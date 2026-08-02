using System.Buffers.Binary;
using System.Text;
using Paperless.WordProcessing.Ww8;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// The geometry a WW8 list level states in its own <c>grpprlPapx</c>.
/// </summary>
/// <remarks>
/// <para>
/// Word writes a level's indent, its hanging indent and the tab that follows its label into the
/// <c>LVL</c>'s paragraph grpprl, and normally repeats them on every paragraph in the list as well. The
/// grpprl used to be stepped over unread, which worked for exactly as long as the repetition held: on a
/// document that does not repeat it, every item came out with a nought indent and no hanging one, so the
/// label was drawn where the item's first word starts and <c>pdftotext</c> read <c>-Stills</c> as one
/// word where LibreOffice reads two.
/// </para>
/// <para>
/// Built as bytes rather than taken from a corpus file, because the subject is the record layout: three
/// sprms among the several a level may carry, each in two spellings, plus the <c>ixchFollow</c> byte
/// beside them in the fixed header.
/// </para>
/// </remarks>
public sealed class Ww8ListLevelTests
{
    /// <summary>The level's own indents and tab reach the parsed definition.</summary>
    [Fact]
    public void ALevelStatesItsIndentsInItsParagraphGrpprl()
    {
        Ww8ListLevel level = ParseLevel(
            follow: 0,
            paragraphProperties:
            [
                .. Sprm(0x840F, 720),   // sprmPDxaLeft
                .. Sprm(0x8411, unchecked((ushort)-360)),  // sprmPDxaLeft1
            ]);

        level.IndentAt.ShouldBe(720);
        level.FirstLineIndent.ShouldBe(-360);
        level.Follow.ShouldBe((byte)0);
    }

    /// <summary>The later spelling of the same two sprms is read as well.</summary>
    [Fact]
    public void TheLaterSpellingOfTheIndentSprmsIsRead()
    {
        Ww8ListLevel level = ParseLevel(
            follow: 1,
            paragraphProperties:
            [
                .. Sprm(0x845E, 1080),
                .. Sprm(0x8460, unchecked((ushort)-540)),
            ]);

        level.IndentAt.ShouldBe(1080);
        level.FirstLineIndent.ShouldBe(-540);
        level.Follow.ShouldBe((byte)1);
    }

    /// <summary>
    /// A negative left indent is taken as a distance, which is what <c>ReadLVL</c> does with it.
    /// </summary>
    [Fact]
    public void ANegativeLeftIndentIsTakenAsADistance()
    {
        Ww8ListLevel level = ParseLevel(
            follow: 2, paragraphProperties: [.. Sprm(0x840F, unchecked((ushort)-720))]);

        level.IndentAt.ShouldBe(720);
        level.Follow.ShouldBe((byte)2);
    }

    /// <summary>
    /// The tab the label's follower aims at, in the one shape a level ever writes it.
    /// </summary>
    [Fact]
    public void ALevelsListTabIsRead()
    {
        // sprmPChgTabs, operand: cch, then delete nothing and insert one stop of type 6.
        byte[] tabs = [0x15, 0xC6, 5, 0, 1, 0xD0, 0x02, 6];

        Ww8ListLevel level = ParseLevel(follow: 0, paragraphProperties: tabs);

        level.TabPosition.ShouldBe(720);
    }

    /// <summary>A level whose grpprl says nothing about indents reports none.</summary>
    [Fact]
    public void ALevelWithNoParagraphGrpprlReportsNoIndents()
    {
        Ww8ListLevel level = ParseLevel(follow: 0, paragraphProperties: []);

        level.IndentAt.ShouldBe(0);
        level.FirstLineIndent.ShouldBe(0);
        level.TabPosition.ShouldBe(0);
    }

    /// <summary>A two-byte sprm: its id, then its operand.</summary>
    private static byte[] Sprm(ushort id, ushort operand)
    {
        byte[] bytes = new byte[4];
        BinaryPrimitives.WriteUInt16LittleEndian(bytes, id);
        BinaryPrimitives.WriteUInt16LittleEndian(bytes.AsSpan(2), operand);
        return bytes;
    }

    /// <summary>
    /// A <c>PlcfLst</c> holding one simple list of one level, parsed back out again.
    /// </summary>
    private static Ww8ListLevel ParseLevel(byte follow, byte[] paragraphProperties)
    {
        List<byte> stream = [];

        // The count, then one 28-byte LSTF: an lsid, a tplc, nine style indexes, then the flags whose
        // low bit is fSimpleList.
        stream.AddRange([1, 0]);

        byte[] definition = new byte[28];
        BinaryPrimitives.WriteInt32LittleEndian(definition, 42);
        for (int slot = 0; slot < 9; slot++)
        {
            BinaryPrimitives.WriteUInt16LittleEndian(definition.AsSpan(8 + (2 * slot)), 4095);
        }

        definition[26] = 0x01;
        stream.AddRange(definition);

        // One LVL: the 28-byte LVLF, the paragraph grpprl, no character grpprl, then the template.
        byte[] header = new byte[28];
        BinaryPrimitives.WriteInt32LittleEndian(header, 1);
        header[4] = 0;              // nfc: decimal
        header[15] = follow;        // ixchFollow
        header[24] = 0;             // cbGrpprlChpx
        header[25] = (byte)paragraphProperties.Length;
        stream.AddRange(header);
        stream.AddRange(paragraphProperties);

        const string Template = "%";
        stream.AddRange([(byte)Template.Length, 0]);
        stream.AddRange(Encoding.Unicode.GetBytes(Template));

        // One LFO naming that list, so the level can be asked for by instance.
        List<byte> overrides = [1, 0, 0, 0];
        byte[] instance = new byte[16];
        BinaryPrimitives.WriteInt32LittleEndian(instance, 42);
        overrides.AddRange(instance);

        Ww8Numbering numbering = Ww8Numbering.Parse([.. stream], [.. overrides]);

        Ww8ListLevel? level = numbering.FindLevel(1, 0);
        level.ShouldNotBeNull();
        return level!.Value;
    }
}
