using System.Buffers.Binary;
using XRay.Office.WordProcessing.Ww8;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A character attribute still open where a <c>TOC</c> or <c>INDEX</c> field begins is not drawn by
/// 26.2.4.2, and the boundary is that field's begin marker.
/// </summary>
/// <remarks>
/// <para>
/// <c>Read_F_Tox</c> inserts the index section and then moves the insertion point backwards into it —
/// <c>m_oPosAfterTOC.emplace(*m_pPaM, m_pPaM); (*m_pPaM).Move(fnMoveBackward);</c>
/// (<c>sw/source/filter/ww8/ww8par5.cxx</c>:3531-3533) — while every attribute of the CHPX still on
/// <c>m_xCtrlStck</c> is waiting to be closed, so each closes in a node that is not the one it opened
/// in and the range it would have covered is never set.
/// </para>
/// <para>
/// <strong>Measured at the reference rather than argued from the source.</strong> Six one-field
/// variants of <c>361400CSLegislation1RF01PUBLIC1.doc</c>, each read back through 26.2.4.2's own
/// <c>--convert-to fodt</c>: its <c>Table of Contents</c> heading is the CHPX at fc 2189-2207 whose
/// last character is the paragraph mark and whose next is the <c>TOC</c> field's <c>U+0013</c>. As
/// authored, no underline; with the field's type changed to one with no handler, underline; with the
/// run's end moved back one byte so it closes at the mark, underline; with <c>sprmCKul</c> replaced by
/// <c>sprmCIco</c>, no colour, and with that same colour and the retyped field, <c>#ff0000</c>. Type 8
/// (<c>INDEX</c>, the other <c>Read_F_Tox</c> slot) loses it; types 3 (<c>REF</c>) and 88
/// (<c>HYPERLINK</c>) keep it.
/// </para>
/// <para>
/// So the trigger is exactly the two types <see cref="Ww8DocumentReader.IndexFieldStarts"/> collects,
/// and what this file pins is that collection — which markers are index-field begins and which are
/// not. The paragraph style's own character half is untouched throughout, because it is on the node's
/// format rather than on the control stack. <c>probes/ww8char-r135/results.md</c> §2.
/// </para>
/// </remarks>
public sealed class Ww8IndexFieldBoundaryTests
{
    private const char Begin = '';
    private const char Separator = '';
    private const char End = '';

    /// <summary>The <c>TOC</c> field's type byte in the <c>PlcFld</c>.</summary>
    private const byte Toc = 13;

    /// <summary>The <c>INDEX</c> field's, the other <c>Read_F_Tox</c> slot.</summary>
    private const byte Index = 8;

    /// <summary>The <c>REF</c> field's, which arm F of the series shows keeps the attribute.</summary>
    private const byte Ref = 3;

    /// <summary>The <c>HYPERLINK</c> field's, which arm G shows keeps it too.</summary>
    private const byte Hyperlink = 88;

    /// <summary>
    /// Both of <c>Read_F_Tox</c>'s slots are index fields and nothing else is.
    /// </summary>
    /// <remarks>
    /// The two negative arms are the measurement rather than the specification: 26.2.4.2 draws the same
    /// heading underlined when the identical bytes name field type 3 or 88, and unadorned when they
    /// name 13 or 8. A rule keyed on "a field" rather than on those two types would strip the CHPX
    /// before **1526** of the corpus's field beginnings instead of three.
    /// </remarks>
    [Fact]
    public void OnlyTheTwoFieldTypesThatReachReadFToxAreIndexFieldStarts()
    {
        Starts(Toc).ShouldBe([0]);
        Starts(Index).ShouldBe([0]);
        Starts(Ref).ShouldBeEmpty();
        Starts(Hyperlink).ShouldBeEmpty();
    }

    /// <summary>
    /// It is the field's <em>begin</em> marker, not its separator and not its result.
    /// </summary>
    /// <remarks>
    /// Arm B of the series settles this: keeping the <c>TOC</c> field and moving the run's end back by
    /// one byte, so that it closes at the paragraph mark rather than at the <c>U+0013</c>, brings the
    /// underline back. The result is where O81's own rule lives and is a different position entirely —
    /// several hundred characters later on the witness.
    /// </remarks>
    [Fact]
    public void ThePositionIsTheBeginMarkerAndNotTheResult()
    {
        string text = "above\r" + Begin + " TOC " + Separator + "One" + End + "after";
        List<int> starts = Ww8DocumentReader.IndexFieldStarts(
            text, 0, Fields((text.IndexOf(Begin, StringComparison.Ordinal), Toc)), 0);

        starts.ShouldHaveSingleItem().ShouldBe(text.IndexOf(Begin, StringComparison.Ordinal));
        starts[0].ShouldBeLessThan(text.IndexOf(Separator, StringComparison.Ordinal));
    }

    /// <summary>
    /// Every index field's begin counts, nested ones included.
    /// </summary>
    /// <remarks>
    /// Each is its own <c>Read_F_Tox</c> call and it is the call that moves the insertion point, so a
    /// walk that took only the outermost — which is the right answer for the cached <em>result</em>'s
    /// extent, <c>IndexResultRanges</c>' own rule — would miss the inner boundary.
    /// </remarks>
    [Fact]
    public void ANestedIndexFieldIsABoundaryOfItsOwn()
    {
        string text = Begin + " TOC " + Separator + "One" + Begin + " INDEX " + Separator + "x" + End
                      + End;
        int outer = text.IndexOf(Begin, StringComparison.Ordinal);
        int inner = text.IndexOf(Begin, outer + 1);

        Ww8DocumentReader.IndexFieldStarts(text, 0, Fields((outer, Toc), (inner, Index)), 0)
            .ShouldBe([outer, inner]);
    }

    /// <summary>
    /// Positions are the walk's own, so a header's or a footnote's field table still pairs.
    /// </summary>
    /// <remarks>
    /// The same reconciliation <c>IndexResultRanges</c> makes, for the same reason: look a global
    /// position up in a story-relative table and every field in that story reads as type 0, so no
    /// boundary is ever found and the rule silently does nothing in exactly the stories it is hardest
    /// to notice in.
    /// </remarks>
    [Fact]
    public void AStoryThatDoesNotStartAtNoughtPairsAgainstItsOwnBase()
    {
        const int start = 5000;
        const int fieldBase = 4000;
        string text = "a\r" + Begin + " TOC " + Separator + "One" + End;
        int at = text.IndexOf(Begin, StringComparison.Ordinal);

        Ww8DocumentReader.IndexFieldStarts(text, start, Fields((start + at - fieldBase, Toc)), fieldBase)
            .ShouldBe([start + at]);
    }

    /// <summary>A story with no field table has no boundaries.</summary>
    [Fact]
    public void AStoryWithNoFieldsHasNoBoundaries()
    {
        Ww8DocumentReader.IndexFieldStarts("plain text", 0, Ww8FieldTypes.Empty, 0).ShouldBeEmpty();
    }

    private static List<int> Starts(byte type)
    {
        string text = Begin + " F " + Separator + "One" + End;
        return Ww8DocumentReader.IndexFieldStarts(text, 0, Fields((0, type)), 0);
    }

    /// <summary>A <c>PlcFld</c> naming one type per field beginning.</summary>
    /// <remarks>
    /// Two four-byte positions and one two-byte record per field half, which is the shape
    /// <see cref="Ww8FieldTypes.Parse"/> reads: <c>FLD.ch</c> in the low five bits of the first byte,
    /// and for a beginning the type in the second.
    /// </remarks>
    private static Ww8FieldTypes Fields(params (int Position, byte Type)[] beginnings)
    {
        List<byte> positions = [];
        List<byte> records = [];

        foreach ((int position, byte type) in beginnings)
        {
            byte[] four = new byte[4];
            BinaryPrimitives.WriteInt32LittleEndian(four, position);
            positions.AddRange(four);
            records.AddRange([19, type]);
        }

        positions.AddRange([0xFF, 0xFF, 0x7F, 0x7F]);
        return Ww8FieldTypes.Parse([.. positions, .. records]);
    }
}
