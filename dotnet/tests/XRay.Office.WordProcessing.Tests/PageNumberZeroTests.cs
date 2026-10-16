using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A page numbered nought prints <c>0</c> in decimal and nothing at all in the other four sequences.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="NoteNumbering.Render"/> used to raise every value to one, on the reasoning that "none of
/// the sequences has a zeroth term". That is true of four of the five and false of the one that
/// matters. It is safe for a note — <see cref="NoteNumbering.Citation"/> clamps its own start before
/// calling, so a footnote never arrives below one — and wrong for a page, because
/// <c>w:pgNumType/@w:start</c> may legitimately be nought.
/// </para>
/// <para>
/// <strong>Measured on LibreOffice 26.2.4.2</strong>, ten three-page documents differing only in
/// <c>w:pgNumType</c>, reading the <c>PAGE</c> field off each page —
/// <c>probes/page-number-zero/</c>. The point the clamp got wrong is that a sequence with no zeroth
/// term writes <em>nothing</em> rather than its first term.
/// </para>
/// <para>
/// The corpus document it decides is <c>EHEST-SMS-Safety-Management-Manual-V2.docx</c>, whose first
/// page the reference numbers 0 and we numbered 1 — on that page alone, since every later page already
/// agreed, which is what made a formatting fault look like a counter one. Two more documents declare
/// the same thing: <c>final-technical-report-template.docx</c> and
/// <c>Technical_Issue_Report_Form.docx</c>.
/// </para>
/// </remarks>
public sealed class PageNumberZeroTests
{
    /// <summary>Decimal has a zeroth term and writes it.</summary>
    [Fact]
    public void DecimalWritesZeroAsZero()
    {
        NoteNumbering.Render(NoteNumberFormat.Arabic, 0).ShouldBe("0");
    }

    /// <summary>The other four have none, and write nothing rather than their first term.</summary>
    [Theory]
    [InlineData(NoteNumberFormat.LowerRoman)]
    [InlineData(NoteNumberFormat.UpperRoman)]
    [InlineData(NoteNumberFormat.LowerLetter)]
    [InlineData(NoteNumberFormat.UpperLetter)]
    [InlineData(NoteNumberFormat.Chicago)]
    public void ASequenceWithNoZerothTermWritesNothing(NoteNumberFormat format)
    {
        NoteNumbering.Render(format, 0).ShouldBe(string.Empty);
    }

    /// <summary>
    /// And it is emptiness rather than the first term, which is the distinction the clamp lost.
    /// </summary>
    [Theory]
    [InlineData(NoteNumberFormat.LowerRoman, "i")]
    [InlineData(NoteNumberFormat.UpperRoman, "I")]
    [InlineData(NoteNumberFormat.LowerLetter, "a")]
    [InlineData(NoteNumberFormat.UpperLetter, "A")]
    public void NoughtIsNotTheFirstTerm(NoteNumberFormat format, string first)
    {
        NoteNumbering.Render(format, 1).ShouldBe(first);
        NoteNumbering.Render(format, 0).ShouldNotBe(first);
    }

    /// <summary>Every sequence is unchanged from one upwards, which is the regression this guards.</summary>
    [Fact]
    public void TheSequencesAreUnchangedFromOneUpwards()
    {
        NoteNumbering.Render(NoteNumberFormat.Arabic, 3).ShouldBe("3");
        NoteNumbering.Render(NoteNumberFormat.LowerRoman, 3).ShouldBe("iii");
        NoteNumbering.Render(NoteNumberFormat.UpperRoman, 4).ShouldBe("IV");
        NoteNumbering.Render(NoteNumberFormat.LowerLetter, 2).ShouldBe("b");
        NoteNumbering.Render(NoteNumberFormat.UpperLetter, 27).ShouldBe("AA");
    }

    /// <summary>
    /// A note is unaffected: its own start is clamped before the sequence is asked, so a file stating
    /// nought still gets the first term and not an empty mark.
    /// </summary>
    [Theory]
    [InlineData(NoteNumberFormat.Arabic, "1")]
    [InlineData(NoteNumberFormat.LowerRoman, "i")]
    [InlineData(NoteNumberFormat.UpperLetter, "A")]
    public void ANoteStartingBelowOneStillGetsAMark(NoteNumberFormat format, string expected)
    {
        new NoteNumbering(format, StartAt: 0).Citation(0).ShouldBe(expected);
        new NoteNumbering(format, StartAt: -5).Citation(0).ShouldBe(expected);
    }

    /// <summary>A decimal page number below nought is written as it stands, not clamped.</summary>
    /// <remarks>
    /// Not reachable from <c>w:pgNumType</c>, whose value is unsigned, but the guard states which side
    /// of the boundary the arithmetic sits on rather than leaving it to be rediscovered.
    /// </remarks>
    [Fact]
    public void ANegativeDecimalIsWrittenAsItStands()
    {
        NoteNumbering.Render(NoteNumberFormat.Arabic, -3).ShouldBe("-3");
    }
}
