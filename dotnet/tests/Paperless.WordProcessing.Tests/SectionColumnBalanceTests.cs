using Paperless.Core.Units;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Which multi-column sections balance their columns, which is decided by the section after them.
/// </summary>
/// <remarks>
/// Both of LibreOffice's Word importers say the same thing in the same shape. The WW8 one marks a
/// section unbalanced when <c>aNext == aEnd || !aNext-&gt;IsContinuous()</c>
/// (<c>sw/source/filter/ww8/ww8par.cxx</c>:4576); the DOCX one reaches
/// <c>pPrevSection-&gt;DontBalanceTextColumns()</c> from the branch handling every break type that is
/// "effectively a page break" (<c>dmapper/PropertyMap.cxx</c>:1919) and handles the last section in
/// <c>ApplyColumnProperties</c>. Both are switched off wholesale by the document's own
/// <c>noColumnBalance</c> compatibility flag.
/// </remarks>
public sealed class SectionColumnBalanceTests
{
    /// <summary>A two-column section followed by a continuous one balances.</summary>
    [Fact]
    public void AStretchClosedByAContinuousBreakBalances()
    {
        List<WritingSection> sections = [Columns(2), Continuous()];
        SectionColumnBalance.Apply(sections, noColumnBalance: false);

        sections[0].BalancesColumns.ShouldBeTrue();
    }

    /// <summary>A two-column section followed by a page break does not.</summary>
    /// <remarks>
    /// The distinction the whole rule is about: the same two-column stretch fills column one to the
    /// bottom of the page when a page break follows it, and shares its text when a continuous break does.
    /// </remarks>
    [Fact]
    public void AStretchClosedByAPageBreakDoesNot()
    {
        List<WritingSection> sections = [Columns(2), new WritingSection { Break = SectionBreak.NextPage }];
        SectionColumnBalance.Apply(sections, noColumnBalance: false);

        sections[0].BalancesColumns.ShouldBeFalse();
    }

    /// <summary>The document's last section never balances, whatever its break.</summary>
    [Fact]
    public void TheLastSectionNeverBalances()
    {
        List<WritingSection> sections = [Continuous(), Columns(2, SectionBreak.Continuous)];
        SectionColumnBalance.Apply(sections, noColumnBalance: false);

        sections[1].BalancesColumns.ShouldBeFalse();
    }

    /// <summary>A single-column section is never marked, because the flag would mean nothing on it.</summary>
    [Fact]
    public void ASingleColumnSectionIsNotMarked()
    {
        List<WritingSection> sections = [Columns(1), Continuous()];
        SectionColumnBalance.Apply(sections, noColumnBalance: false);

        sections[0].BalancesColumns.ShouldBeFalse();
    }

    /// <summary>The document's own compatibility flag turns the whole rule off.</summary>
    [Fact]
    public void TheDocumentsOwnFlagSuppressesEverything()
    {
        List<WritingSection> sections = [Columns(2), Continuous()];
        SectionColumnBalance.Apply(sections, noColumnBalance: true);

        sections[0].BalancesColumns.ShouldBeFalse();
    }

    /// <summary>
    /// A continuous break onto different paper is a page break, so the stretch before it does not balance.
    /// </summary>
    /// <remarks>
    /// "If two following sections are different in following properties, Word will interpret a continuous
    /// section break between them as if it was a section break next page" — the same compatibility test
    /// the geometry already applies, asked here so the two cannot disagree about what a break is.
    /// </remarks>
    [Fact]
    public void AContinuousBreakOntoDifferentPaperDoesNotCount()
    {
        WritingSection landscape = Continuous() with
        {
            Page = PageGeometry.Default with { IsLandscape = true },
        };

        List<WritingSection> sections = [Columns(2), landscape];
        SectionColumnBalance.Apply(sections, noColumnBalance: false);

        sections[0].BalancesColumns.ShouldBeFalse();
    }

    private static WritingSection Columns(int count, SectionBreak start = SectionBreak.NextPage)
        => new()
        {
            Break = start,
            Page = PageGeometry.Default with { Columns = count, ColumnGap = Length.FromTwips(708) },
        };

    private static WritingSection Continuous()
        => new() { Break = SectionBreak.Continuous };
}
