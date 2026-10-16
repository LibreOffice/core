using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// What a paragraph takes from the <em>paragraph</em> formatting of the style it names, and from
/// that style's <c>\sbasedon</c> ancestors.
/// </summary>
/// <remarks>
/// <para>
/// The rule is not "the style's formatting cascades". A paragraph's direct formatting is rebuilt
/// from RTF's own defaults for every property the style it <em>names</em> states —
/// <c>cloneAndDeduplicateSprm</c>'s <em>"not found - try to override style with default"</em>
/// branch, <c>sw/source/writerfilter/rtftok/rtfsprm.cxx</c>:319-327, over the table in
/// <c>getDefaultSPRM</c> (:154-224) — and only the ancestors' statements escape it, because
/// <c>lcl_copyFlatten</c> (<c>rtfdocumentimpl.cxx</c>:490-514) hands that branch the named entry's
/// own <c>pPr</c> and nothing above it.
/// </para>
/// <para>
/// So the alignment and the space before are honoured from an ancestor and dropped from the named
/// style; the space after, the indents and the line spacing are dropped from both, because
/// <c>getDefaultSPRM</c> answers for them wherever they are found; and the keep-with-next is
/// honoured from either, because it answers for that one not at all. Measured over 53 one-page
/// probes at thirteen properties against LibreOffice 26.2.4.2 —
/// <c>dotnet/probes/rtf-resid-r80/genmatrix.py</c>, 53 of 53 reproduced.
/// </para>
/// </remarks>
public sealed class RtfStyleParagraphFormattingTests
{
    /// <summary>An ancestor's <c>\qc</c> centres the paragraph.</summary>
    [Fact]
    public void AnInheritedAlignmentReachesTheParagraph()
        => Format(@"\pard\plain\s3\fs20 MARKER\par",
                @"{\s2\snext2\qc Centred;}{\s3\sbasedon2\snext3 Child;}")
            .Alignment.ShouldBe(TextAlignment.Centre);

    /// <summary>The named style's own <c>\qc</c> does not.</summary>
    [Fact]
    public void AStylesOwnAlignmentDoesNot()
        => Format(@"\pard\plain\s2\fs20 MARKER\par", @"{\s2\snext2\qc Centred;}")
            .Alignment.ShouldBe(TextAlignment.Start);

    /// <summary>And it does not fall back on the ancestor's either — the default replaces both.</summary>
    /// <remarks>
    /// The witness is <c>150-5370-10H.rtf</c>'s <c>\s1 heading 1</c>, which states
    /// <c>\qc\sb720</c> over an <c>\s0</c> stating <c>\sb120</c>: 26.2.4.2 draws a paragraph naming
    /// <c>\s1</c> flush left with no space above it at all.
    /// </remarks>
    [Fact]
    public void AStatedPropertyReplacesTheInheritedOneWithTheDefault()
    {
        ParagraphFormat format = Format(
            @"\pard\plain\s3\fs20 MARKER\par",
            @"{\s2\snext2\qc\sb480 Centred;}{\s3\sbasedon2\snext3\ql\sb0 Flat;}");

        format.Alignment.ShouldBe(TextAlignment.Start);
        format.SpaceBefore.ShouldBe(Length.Zero);
    }

    /// <summary>An ancestor's <c>\sb</c> is the space above the paragraph.</summary>
    [Fact]
    public void AnInheritedSpaceBeforeReachesTheParagraph()
        => Format(@"\pard\plain\s3\fs20 MARKER\par",
                @"{\s2\snext2\sb480 Spaced;}{\s3\sbasedon2\snext3 Child;}")
            .SpaceBefore.ShouldBe(Length.FromTwips(480));

    /// <summary>
    /// Its sibling <c>\sa</c> reaches it from nowhere, and that asymmetry is the point.
    /// </summary>
    /// <remarks>
    /// <c>getDefaultSPRM</c>'s value for the whole <c>spacing</c> node is <c>after = 0</c> and is
    /// taken before the per-attribute recursion, so the space after is written over and the space
    /// before is never visited.
    /// </remarks>
    [Fact]
    public void AnInheritedSpaceAfterDoesNot()
        => Format(@"\pard\plain\s3\fs20 MARKER\par",
                @"{\s2\snext2\sa480 Spaced;}{\s3\sbasedon2\snext3 Child;}")
            .SpaceAfter.ShouldBe(Length.Zero);

    /// <summary>Nor does an inherited indent, for the same reason one level down.</summary>
    [Fact]
    public void AnInheritedIndentDoesNot()
        => Format(@"\pard\plain\s3\fs20 MARKER\par",
                @"{\s2\snext2\li1440\lin1440 Indented;}{\s3\sbasedon2\snext3 Child;}")
            .StartIndent.ShouldBe(Length.Zero);

    /// <summary>
    /// <c>\keepn</c> is the exception: the style that states it keeps the paragraph with the next
    /// whether the paragraph names that style or a child of it.
    /// </summary>
    /// <remarks>
    /// <c>keepNext</c> is a plain value with neither a default nor children, so the reset branch
    /// writes nothing over it. Measured on a page filled to its last line: the reference moves the
    /// paragraph to the following page in all three of <c>\keepn</c> direct, from the named style
    /// and from its parent.
    /// </remarks>
    [Fact]
    public void KeepWithNextReachesTheParagraphFromEitherLevel()
    {
        Format(@"\pard\plain\s2\fs20 MARKER\par", @"{\s2\snext2\keepn Kept;}")
            .KeepWithNext.ShouldBeTrue();

        Format(@"\pard\plain\s3\fs20 MARKER\par",
                @"{\s2\snext2\keepn Kept;}{\s3\sbasedon2\snext3 Child;}")
            .KeepWithNext.ShouldBeTrue();
    }

    /// <summary>The paragraph's own control words still beat the style's.</summary>
    [Fact]
    public void DirectFormattingAfterTheStyleWins()
        => Format(@"\pard\plain\s3\ql\fs20 MARKER\par",
                @"{\s2\snext2\qc\sb480 Centred;}{\s3\sbasedon2\snext3 Child;}")
            .Alignment.ShouldBe(TextAlignment.Start);

    /// <summary>
    /// A <c>\sbasedon</c> naming a style the sheet has not defined yet is not a parent at all.
    /// </summary>
    /// <remarks>
    /// The importer resolves it to a style <em>name</em> where it stands
    /// (<c>rtfdispatchvalue.cxx</c>:131-134), and a style not yet read has no name to give, so
    /// nothing is set as the parent. Measured: a child declared before its parent takes none of its
    /// centring, and an unrelated entry between the two changes nothing.
    /// </remarks>
    [Fact]
    public void AForwardSbasedonIsNotAParent()
    {
        Format(@"\pard\plain\s3\fs20 MARKER\par",
                @"{\s3\sbasedon2\snext3 Child;}{\s2\snext2\qc Centred;}")
            .Alignment.ShouldBe(TextAlignment.Start);

        Format(@"\pard\plain\s3\fs20 MARKER\par",
                @"{\s2\snext2\qc Centred;}{\s9\snext9\li720 Other;}{\s3\sbasedon2\snext3 Child;}")
            .Alignment.ShouldBe(TextAlignment.Centre);
    }

    /// <summary>The first body paragraph's format, as layout resolved it.</summary>
    private static ParagraphFormat Format(string body, string styles)
    {
        string rtf =
            @"{\rtf1\ansi\deff0{\fonttbl{\f0\froman Liberation Serif;}}"
            + @"{\stylesheet{\s0\snext0\ql Normal;}" + styles + "}"
            + @"\paperw11906\paperh16838\margl1440\margr1440\margt1440\margb1440\sectd"
            + body
            + "}";

        using DocumentSource source = DocumentSource.FromStream(
            new MemoryStream(Encoding.ASCII.GetBytes(rtf)), "styles.rtf");
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return pages.Paragraphs.First(paragraph => paragraph.Text.Contains("MARKER")).DeclaredFormat;
    }
}
